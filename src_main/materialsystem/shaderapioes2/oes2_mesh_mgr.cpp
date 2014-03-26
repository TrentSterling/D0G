//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 mesh manager.
//
//===========================================================================//
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

ConVar mat_debugalttab("mat_debugalttab", "0", FCVAR_CHEAT);

CMeshMgr::CMeshMgr(void) : m_DynamicTempMesh(true)
{
	memset(m_BoundBuffers, 0, sizeof(m_BoundBuffers));
	m_pDynamicIndexBuffer = NULL;
	memset(m_StreamState, 0, sizeof(m_StreamState));
}

void CMeshMgr::BindOESBuffer(OESBufferTarget_t target, unsigned int buffer)
{
	if ((m_BoundBuffers[target] == buffer) || IsDeviceDeactivated())
		return;
	m_BoundBuffers[target] = buffer;
	// Depends on OESBufferTarget_t.
	glBindBuffer((target == OES_BUFFER_TARGET_INDEX) ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER, buffer);
}

void CMeshMgr::DeleteOESBuffer(OESBufferTarget_t target, unsigned int buffer)
{
	if (!buffer || !(ShaderAPI()->IsUsingGraphics()))
		return;
	if (m_BoundBuffers[target] == buffer)
		m_BoundBuffers[target] = 0;
	if (target == OES_BUFFER_TARGET_VERTEX)
	{
		int i;
		for (i = NUM_VERTEX_STREAMS; i--; )
		{
			if (m_StreamState[i].m_Buffer == buffer)
			{
				m_StreamState[i].m_Buffer = 0;
				MarkStreamStateDirty();
			}
		}
	}
	glDeleteBuffers(1, &buffer);
}

void CMeshMgr::DestroyVertexBuffers(void)
{
	int i;
	IVertexBufferOES2 *pBuffer;
	for (i = m_DynamicVertexBuffers.Count(); i-- > 0; )
	{
		pBuffer = m_DynamicVertexBuffers[i];
		if (pBuffer)
			delete pBuffer;
	}
	m_DynamicVertexBuffers.RemoveAll();
	m_DynamicMesh.Reset();
	m_DynamicFlexMesh.Reset();
}

void CMeshMgr::DiscardVertexBuffers(void)
{
	if (IsDeviceDeactivated())
		return;
	int i;
	for (i = m_DynamicVertexBuffers.Count(); i-- > 0; )
		m_DynamicVertexBuffers[i]->Flush();
	m_pDynamicIndexBuffer->Flush();
}

IVertexBufferOES2 *CMeshMgr::FindOrCreateVertexBuffer(int dynamicBufferId, VertexFormat_t fmt)
{
	int bufferMemory = ShaderAPI()->GetCurrentDynamicVBSize();
	while (m_DynamicVertexBuffers.Count() <= dynamicBufferId)
	{
		m_DynamicVertexBuffers.AddToTail(
			new CVertexBufferOES2(SHADER_BUFFER_TYPE_DYNAMIC, VERTEX_FORMAT_UNKNOWN, bufferMemory, NULL));
	}
	IVertexBufferOES2 *pBuffer = m_DynamicVertexBuffers[dynamicBufferId];
	if (pBuffer->GetVertexFormat() != fmt)
		pBuffer->SetVertexFormat(fmt);
	return pBuffer;
}

IMeshOES2 *CMeshMgr::GetDynamicMesh(IMaterial* pMaterial, VertexFormat_t fmt, int nHWSkinBoneCount, bool buffered,
	IMesh *pVertexOverride, IMesh *pIndexOverride)
{
	IMaterialInternal *pMatInternal = static_cast<IMaterialInternal *>(pMaterial);
	Assert(!pMaterial || pMatInternal->IsRealTimeVersion());

	if (pVertexOverride || pIndexOverride)
		buffered = false;
	if (m_BufferedMode && (m_BufferedMode != buffered))
		m_BufferedMesh.SetMesh(NULL);
	m_BufferedMode = buffered;

	IMeshOES2 *pMesh;
	if (ShaderAPI()->IsInSelectionMode())
	{
		Assert(!pVertexOverride);
		if (pIndexOverride)
		{
			IMeshOES2 *pSrcIndexMesh = static_cast<IMeshOES2 *>(pIndexOverride);
			Assert(!(pSrcIndexMesh->IsDynamic()));
			CMeshBuilder dstMeshBuilder;
			dstMeshBuilder.Begin(&m_DynamicTempMesh, pSrcIndexMesh->GetPrimitiveType(), 0, pSrcIndexMesh->IndexCount());
			pSrcIndexMesh->GetIndexBuffer()->CopyToTempMeshIndexBuffer(dstMeshBuilder);
			dstMeshBuilder.End();
		}
		pMesh = &m_DynamicTempMesh;
	}
	else
	{
		pMesh = &m_DynamicMesh;
	}

	if (buffered)
	{
		Assert(m_BufferedMesh.WasRendered());
		m_BufferedMesh.SetMesh(pMesh);
		pMesh = &m_BufferedMesh;
	}

	if (pVertexOverride)
	{
		pMesh->SetVertexFormat(pVertexOverride->GetVertexFormat());
	}
	else
	{
		if (fmt)
		{
			int nVertexFormatBoneWeights = NumBoneWeights(fmt);
			if (nHWSkinBoneCount < nVertexFormatBoneWeights)
				nHWSkinBoneCount = nVertexFormatBoneWeights;
		}
		else
		{
			fmt = pMatInternal->GetVertexFormat();
		}
		fmt &= ~VERTEX_BONE_WEIGHT_MASK;
		if (nHWSkinBoneCount)
			fmt |= VERTEX_BONEWEIGHT(2) | VERTEX_BONE_INDEX;
		pMesh->SetVertexFormat(fmt);
	}

	if (pMesh == &m_DynamicMesh)
	{
		IMeshOES2 *pBase = static_cast<IMeshOES2 *>(pVertexOverride);
		if (pBase)
			m_DynamicMesh.OverrideVertexBuffer(pBase->GetVertexBuffer());
		pBase = static_cast<IMeshOES2 *>(pIndexOverride);
		if (pBase)
			m_DynamicMesh.OverrideIndexBuffer(pBase->GetIndexBuffer());
	}

	return pMesh;
}

void CMeshMgr::GetMaxToRender(const IMesh *pMesh, bool maxUntilFlush, int *pMaxVerts, int *pMaxIndices)
{
	const IMeshOES2 *pBaseMesh = static_cast<const IMeshOES2 *>(pMesh);
	if (!pBaseMesh)
	{
		*pMaxVerts = 0;
		*pMaxIndices = m_pDynamicIndexBuffer->IndexCount();
		return;
	}

	if (pMesh == &m_BufferedMesh)
	{
		pBaseMesh = (static_cast<const CBufferedMeshOES2 *>(pMesh))->GetMesh();
		pMesh = pBaseMesh;
	}

	if (!IsDynamicMesh(pMesh))
	{
		*pMaxVerts = 65535;
		*pMaxIndices = 65535;
		return;
	}

	const IVertexBufferOES2 *pVertexBuffer = pBaseMesh->GetVertexBuffer();
	if (!pVertexBuffer)
	{
		*pMaxVerts = 0;
		*pMaxIndices = 0;
		return;
	}
	const IIndexBufferOES2 *pIndexBuffer = pBaseMesh->GetIndexBuffer();

	if (!maxUntilFlush)
	{
		*pMaxVerts = ShaderAPI()->GetCurrentDynamicVBSize() / pVertexBuffer->VertexSize();
		if (*pMaxVerts > 65535)
			*pMaxVerts = 65535;
		*pMaxIndices = pIndexBuffer ? pIndexBuffer->IndexCount() : 0;
		return;
	}

	*pMaxVerts = pVertexBuffer->GetRoomRemaining();
	if (!(*pMaxVerts))
		*pMaxVerts = ShaderAPI()->GetCurrentDynamicVBSize() / pVertexBuffer->VertexSize();
	if (*pMaxVerts > 65535)
		*pMaxVerts = 65535;

	if (pIndexBuffer)
	{
		*pMaxIndices = pIndexBuffer->GetRoomRemaining();
		if (!(*pMaxIndices))
			*pMaxIndices = pIndexBuffer->IndexCount();
	}
	else
	{
		*pMaxIndices = 0;
	}
}

void CMeshMgr::Init(void)
{
	m_DynamicMesh.Init(0);
	m_DynamicFlexMesh.Init(1);
	m_pDynamicIndexBuffer = new CIndexBufferOES2(SHADER_BUFFER_TYPE_DYNAMIC, MATERIAL_INDEX_FORMAT_16BIT,
		INDEX_BUFFER_SIZE, TEXTURE_GROUP_DYNAMIC_INDEX_BUFFER);
	m_BufferedMode = true;

	m_StreamStateProgram = VERTEX_FORMAT_UNKNOWN;
	CVertexBufferOES2::ComputeVertexDescription(NULL, VERTEX_FORMAT_UNKNOWN, m_VertexDecl);
	m_VertexDeclFormat = VERTEX_FORMAT_UNKNOWN;

	m_EnabledAttributeArrays = 0;
	if (!IsDeviceDeactivated())
	{
		int i;
		for (i = HardwareConfig()->Caps().m_NumVertexShaderInputs; i-- > 0; )
		{
			glDisableVertexAttribArray(i);
			glVertexAttrib4f(i, 0.0f, 0.0f, 0.0f, 1.0f);
		}
	}
}

void CMeshMgr::ReleaseBuffers(void)
{
	if (mat_debugalttab.GetBool())
		Warning("mat_debugalttab: CMeshMgr::ReleaseBuffers\n");
	CleanUp();
	m_DynamicMesh.Reset();
	m_DynamicFlexMesh.Reset();
}

void CMeshMgr::RestoreBuffers(void)
{
	if (mat_debugalttab.GetBool())
		Warning("mat_debugalttab: CMeshMgr::RestoreBuffers\n");
	Init();
}

//-----------------------------------------------------------------------------
// Stream state.
//-----------------------------------------------------------------------------

typedef void (*StreamStateFunc_t)(const VertexDesc_t &desc, const ShaderProgram_t &program);

static void StreamState_Position(const VertexDesc_t &desc, const ShaderProgram_t &program)
{
	if (desc.m_VertexSize_Position)
	{
		MeshMgr()->VertexAttribPointer(0, program.m_VertexInputs[OES2_SHADER_INPUT_POSITION],
			3, GL_FLOAT, desc.m_pPosition);
	}
	else
		MeshMgr()->VertexAttribZero(program.m_VertexInputs[OES2_SHADER_INPUT_POSITION]);
}

static void StreamState_PositionFlex(const VertexDesc_t &desc, const ShaderProgram_t &program)
{
	if (MeshMgr()->HasFlexStream())
		MeshMgr()->VertexAttribPointer(2, program.m_VertexInputs[OES2_SHADER_INPUT_POSITION_FLEX], 3, GL_FLOAT, (void *)0);
	else
		MeshMgr()->VertexAttribZero(program.m_VertexInputs[OES2_SHADER_INPUT_POSITION_FLEX]);
}

static void StreamState_BoneWeight(const VertexDesc_t &desc, const ShaderProgram_t &program)
{
	if (desc.m_VertexSize_BoneWeight && ShaderAPI()->GetCurrentNumBones())
	{
		MeshMgr()->VertexAttribPointer(0, program.m_VertexInputs[OES2_SHADER_INPUT_BONE_WEIGHT],
			NumBoneWeights(program.m_VertexUsage), GL_FLOAT, desc.m_pBoneWeight);
	}
	else
		MeshMgr()->VertexAttribZero(program.m_VertexInputs[OES2_SHADER_INPUT_BONE_WEIGHT]);
}

static void StreamState_BoneIndex(const VertexDesc_t &desc, const ShaderProgram_t &program)
{
	if (desc.m_VertexSize_BoneMatrixIndex && ShaderAPI()->GetCurrentNumBones())
	{
		MeshMgr()->VertexAttribPointer(0, program.m_VertexInputs[OES2_SHADER_INPUT_BONE_INDEX],
			4, GL_UNSIGNED_BYTE, desc.m_pBoneMatrixIndex);
	}
	else
		MeshMgr()->VertexAttribZero(program.m_VertexInputs[OES2_SHADER_INPUT_BONE_INDEX]);
}

static void StreamState_Normal(const VertexDesc_t &desc, const ShaderProgram_t &program)
{
	if (MeshMgr()->HasFlexStream())
		return;
	if (desc.m_VertexSize_Normal)
		MeshMgr()->VertexAttribPointer(0, program.m_VertexInputs[OES2_SHADER_INPUT_NORMAL], 3, GL_FLOAT, desc.m_pNormal);
	else
		MeshMgr()->VertexAttribZero(program.m_VertexInputs[OES2_SHADER_INPUT_NORMAL]);
}

static void StreamState_NormalFlex(const VertexDesc_t &desc, const ShaderProgram_t &program)
{
	if (MeshMgr()->HasFlexStream())
	{
		MeshMgr()->VertexAttribPointer(2, program.m_VertexInputs[OES2_SHADER_INPUT_NORMAL_FLEX],
			3, GL_FLOAT, (void *)(3 * sizeof(float)));
	}
	else
		MeshMgr()->VertexAttribZero(program.m_VertexInputs[OES2_SHADER_INPUT_NORMAL_FLEX]);
}

static void StreamState_Color(const VertexDesc_t &desc, const ShaderProgram_t &program)
{
	if (desc.m_VertexSize_Color)
	{
		MeshMgr()->VertexAttribPointer(0, program.m_VertexInputs[OES2_SHADER_INPUT_COLOR],
			4, GL_UNSIGNED_BYTE, desc.m_pColor, GL_TRUE);
	}
	else
		MeshMgr()->VertexAttribZero(program.m_VertexInputs[OES2_SHADER_INPUT_COLOR]);
}

static void StreamState_Specular(const VertexDesc_t &desc, const ShaderProgram_t &program)
{
	if (MeshMgr()->HasColorStream())
	{
		MeshMgr()->VertexAttribPointer(1, program.m_VertexInputs[OES2_SHADER_INPUT_SPECULAR],
			3, GL_UNSIGNED_BYTE, (void *)0, GL_TRUE);
	}
	else if (desc.m_VertexSize_Specular)
	{
		MeshMgr()->VertexAttribPointer(0, program.m_VertexInputs[OES2_SHADER_INPUT_SPECULAR],
			3, GL_UNSIGNED_BYTE, desc.m_pSpecular, GL_TRUE);
	}
	else
		MeshMgr()->VertexAttribZero(program.m_VertexInputs[OES2_SHADER_INPUT_SPECULAR]);
}

static void StreamState_TexCoord(const VertexDesc_t &desc, const ShaderProgram_t &program, int index)
{
	if (desc.m_VertexSize_TexCoord[index])
	{
		MeshMgr()->VertexAttribPointer(0, program.m_VertexInputs[OES2_SHADER_INPUT_TEXCOORD0 + index],
			TexCoordSize(index, program.m_VertexUsage), GL_FLOAT, desc.m_pTexCoord[index]);
	}
	else
		MeshMgr()->VertexAttribZero(program.m_VertexInputs[OES2_SHADER_INPUT_TEXCOORD0 + index]);
}
#define STREAM_STATE_TEXCOORD(index) static void StreamState_TexCoord##index \
	(const VertexDesc_t &desc, const ShaderProgram_t &program) \
	{ StreamState_TexCoord(desc, program, index); }
STREAM_STATE_TEXCOORD(0)
STREAM_STATE_TEXCOORD(1)
STREAM_STATE_TEXCOORD(2)
STREAM_STATE_TEXCOORD(3)
STREAM_STATE_TEXCOORD(4)
STREAM_STATE_TEXCOORD(5)
STREAM_STATE_TEXCOORD(6)
STREAM_STATE_TEXCOORD(7)
#undef STREAM_STATE_TEXCOORD

static void StreamState_TangentS(const VertexDesc_t &desc, const ShaderProgram_t &program)
{
	if (desc.m_VertexSize_TangentS)
	{
		MeshMgr()->VertexAttribPointer(0, program.m_VertexInputs[OES2_SHADER_INPUT_TANGENT_S],
			3, GL_FLOAT, desc.m_pTangentS);
	}
	else
		MeshMgr()->VertexAttribZero(program.m_VertexInputs[OES2_SHADER_INPUT_TANGENT_S]);
}
static void StreamState_TangentT(const VertexDesc_t &desc, const ShaderProgram_t &program)
{
	if (desc.m_VertexSize_TangentT)
	{
		MeshMgr()->VertexAttribPointer(0, program.m_VertexInputs[OES2_SHADER_INPUT_TANGENT_T],
			3, GL_FLOAT, desc.m_pTangentT);
	}
	else
		MeshMgr()->VertexAttribZero(program.m_VertexInputs[OES2_SHADER_INPUT_TANGENT_T]);
}

static void StreamState_UserData(const VertexDesc_t &desc, const ShaderProgram_t &program)
{
	if (desc.m_VertexSize_UserData)
	{
		MeshMgr()->VertexAttribPointer(0, program.m_VertexInputs[OES2_SHADER_INPUT_USERDATA],
			UserDataSize(program.m_VertexUsage), GL_FLOAT, desc.m_pUserData);
	}
	else
		MeshMgr()->VertexAttribZero(program.m_VertexInputs[OES2_SHADER_INPUT_USERDATA]);
}

StreamStateFunc_t s_StreamStateFunctionTable[] = // Must match StreamStateOp_t!
{
	// Stream 0.
	StreamState_Position,
	StreamState_BoneWeight,
	StreamState_BoneIndex,
	StreamState_Normal,
	StreamState_Color,
	StreamState_TexCoord0,
	StreamState_TexCoord1,
	StreamState_TexCoord2,
	StreamState_TexCoord3,
	StreamState_TexCoord4,
	StreamState_TexCoord5,
	StreamState_TexCoord6,
	StreamState_TexCoord7,
	StreamState_TangentS,
	StreamState_TangentT,
	StreamState_UserData,
	// Stream 0/1.
	StreamState_Specular,
	// Stream 2.
	StreamState_PositionFlex,
	StreamState_NormalFlex
};

void CMeshMgr::ApplyStreamState(void)
{
	ShaderProgramHandle_t hProgram = ShaderAPI()->GetBoundProgramHandle();
	Assert(hProgram != SHADER_PROGRAM_HANDLE_INVALID);
	if (m_StreamStateProgram == hProgram)
		return;
	const ShaderProgram_t *program =
		(const ShaderProgram_t *)(ShaderAPI()->GetShaderProgramData(hProgram, OES2_SHADER_FORMAT));
	m_StreamStateProgram = hProgram;

	COMPILE_TIME_ASSERT(sizeof(s_StreamStateFunctionTable) == (sizeof(StreamStateFunc_t) * STREAM_STATE_OP_COUNT));
	m_CurrentAttributeArrays = 0;

	int i;
	VertexDesc_t &desc = m_VertexDecl;
	for (i = program->m_StreamOpCount; i-- > 0; )
		s_StreamStateFunctionTable[program->m_StreamOps[i]](desc, *program);

	unsigned int disable = m_EnabledAttributeArrays & ~m_CurrentAttributeArrays;
	if (disable)
	{
		m_EnabledAttributeArrays = m_CurrentAttributeArrays;
		for (i = HardwareConfig()->Caps().m_NumVertexShaderInputs; i-- > 0; )
		{
			if (disable & (1 << i))
				glDisableVertexAttribArray(i);
		}
	}
}

void CMeshMgr::VertexAttribPointer(unsigned int stream, unsigned int index,
	int size, unsigned int type, const void *ptr, int normalized)
{
	StreamState_t &streamState = m_StreamState[stream];
	BindOESBuffer(OES_BUFFER_TARGET_VERTEX, streamState.m_Buffer);
	unsigned int mask = 1 << index;
	m_CurrentAttributeArrays |= mask;
	if (!(m_EnabledAttributeArrays & mask))
	{
		glEnableVertexAttribArray(index);
		m_EnabledAttributeArrays |= mask;
	}
	glVertexAttribPointer(index, size, type, normalized, streamState.m_Stride, ((unsigned char *)ptr) + streamState.m_Offset);
}

void CMeshMgr::VertexAttribZero(unsigned int index)
{
	unsigned int mask = 1 << index;
	if (!(m_EnabledAttributeArrays & mask))
		return;
	glDisableVertexAttribArray(index);
	m_EnabledAttributeArrays &= ~mask;
	glVertexAttrib4f(index, 0.0f, 0.0f, 0.0f, 1.0f);
}

static CMeshMgr s_MeshMgr;
CMeshMgr *g_pMeshMgr = &s_MeshMgr;