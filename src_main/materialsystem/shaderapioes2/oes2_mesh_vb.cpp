//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 mesh vertex buffer.
//
//===========================================================================//
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

unsigned char CVertexBufferOES2::s_DynamicLockedData[DYNAMIC_VERTEX_BUFFER_MEMORY];

CVertexBufferOES2::CVertexBufferOES2(ShaderBufferType_t type, VertexFormat_t fmt, int vertexCount,
	const char *pBudgetGroupName)
{
	Assert(vertexCount > 0);

	m_BufferSize = vertexCount;
	m_FirstUnwrittenOffset = 0;
	m_Flush = false;
	m_IsDynamic = (type == SHADER_BUFFER_TYPE_DYNAMIC) || (type == SHADER_BUFFER_TYPE_DYNAMIC_TEMP);
	m_pLockedData = NULL;
	m_VertexBuffer = 0;
	m_VertexFormat = fmt;
	if (fmt == VERTEX_FORMAT_UNKNOWN)
	{
		m_VertexCount = 0;
	}
	else
	{
		m_BufferSize *= VertexSize();
		m_VertexCount = vertexCount;
	}

#ifdef VPROF_ENABLED
	if (m_IsDynamic)
	{
		m_pGlobalCounter = g_VProfCurrentProfile.FindOrCreateCounter(
			"TexGroup_global_" TEXTURE_GROUP_DYNAMIC_VERTEX_BUFFER, COUNTER_GROUP_TEXTURE_GLOBAL);
	}
	else
	{
		char name[256];
		Q_strcpy(name, "TexGroup_global_");
		Q_strcat(name, pBudgetGroupName, sizeof(name));
		m_pGlobalCounter = g_VProfCurrentProfile.FindOrCreateCounter(name, COUNTER_GROUP_TEXTURE_GLOBAL);
		Q_strcpy(name, "TexGroup_frame_");
		Q_strcat(name, pBudgetGroupName, sizeof(name));
		m_pFrameCounter = g_VProfCurrentProfile.FindOrCreateCounter(name, COUNTER_GROUP_TEXTURE_PER_FRAME);
	}
	m_VProfFrame = -1;
#endif
}

CVertexBufferOES2::~CVertexBufferOES2(void)
{
	if (!m_VertexBuffer)
		return;

	if (m_pLockedData) // Lock cannot be performed if failed to Allocate, so checking after m_VertexBuffer.
	{
		delete[] m_pLockedData;
		m_pLockedData = NULL;
	}

#ifdef VPROF_ENABLED
	if (!m_IsDynamic)
	{
		Assert(m_pGlobalCounter);
		*m_pGlobalCounter -= m_BufferSize;
	}
#endif

	MeshMgr()->DeleteOESBuffer(OES_BUFFER_TARGET_VERTEX, m_VertexBuffer);
	m_VertexBuffer = 0;
}

bool CVertexBufferOES2::Allocate(void)
{
	Assert(!m_VertexBuffer);
	m_FirstUnwrittenOffset = 0;

	glGenBuffers(1, &m_VertexBuffer);
	if (!m_VertexBuffer)
	{
		Warning("CVertexBufferOES2::Allocate: glGenBuffers failed: %s.\n", OESErrorString());
		return false;
	}
	MeshMgr()->BindOESBuffer(OES_BUFFER_TARGET_VERTEX, m_VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_BufferSize, NULL, m_IsDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

#ifdef VPROF_ENABLED
	if (!m_IsDynamic)
	{
		Assert(m_pGlobalCounter);
		(*m_pGlobalCounter) += m_BufferSize;
	}
#endif

	return true;
}

void CVertexBufferOES2::BeginCastBuffer(VertexFormat_t format)
{
	Assert(!m_VertexFormat || (m_VertexFormat == format));
	SetVertexFormat(format);
}

void CVertexBufferOES2::ComputeVertexDescription(unsigned char *pBuffer, VertexFormat_t vertexFormat, VertexDesc_t &desc)
{
	int *pVertexSizesToSet[64];
	int nVertexSizesToSet = 0;
	static ALIGN32 ModelVertexDX8_t temp[4];
	float *dummyData = (float *)(&temp); // Should be larger than any CMeshBuilder command can set.

	VertexCompressionType_t compression = CompressionType(vertexFormat);
	desc.m_CompressionType = compression;

	int offset;

	Assert(!(vertexFormat & VERTEX_WRINKLE) || (vertexFormat & VERTEX_POSITION));
	if (vertexFormat & VERTEX_POSITION)
	{
		desc.m_pPosition = (float *)pBuffer;
		offset = GetVertexElementSize(VERTEX_ELEMENT_POSITION, compression);
		pVertexSizesToSet[nVertexSizesToSet++] = &(desc.m_VertexSize_Position);

		if (vertexFormat & VERTEX_WRINKLE)
		{
			desc.m_pWrinkle = (float *)(pBuffer + offset);
			offset = GetVertexElementSize(VERTEX_ELEMENT_WRINKLE, compression);
			pVertexSizesToSet[nVertexSizesToSet++] = &(desc.m_VertexSize_Wrinkle);
		}
		else
		{
			desc.m_pWrinkle = dummyData;
			desc.m_VertexSize_Wrinkle = 0;
		}
	}
	else
	{
		offset = 0;
		desc.m_pPosition = dummyData;
		desc.m_VertexSize_Position = 0;
	}

	desc.m_NumBoneWeights = NumBoneWeights(vertexFormat);
	if (vertexFormat & VERTEX_BONE_INDEX)
	{
		// Always exactly two weights.
		Assert(desc.m_NumBoneWeights == 2);
		desc.m_pBoneWeight = (float *)(pBuffer + offset);
		offset += GetVertexElementSize(VERTEX_ELEMENT_BONEWEIGHTS2, compression);
		pVertexSizesToSet[nVertexSizesToSet++] = &(desc.m_VertexSize_BoneWeight);

		desc.m_pBoneMatrixIndex = pBuffer + offset;
		offset += GetVertexElementSize(VERTEX_ELEMENT_BONEINDEX, compression);
		pVertexSizesToSet[nVertexSizesToSet++] = &(desc.m_VertexSize_BoneMatrixIndex);
	}
	else
	{
		Assert(desc.m_NumBoneWeights == 0);
		desc.m_pBoneWeight = dummyData;
		desc.m_VertexSize_BoneWeight = 0;
		desc.m_pBoneMatrixIndex = (unsigned char *)dummyData;
		desc.m_VertexSize_BoneMatrixIndex = 0;
	}

	if (vertexFormat & VERTEX_NORMAL)
	{
		desc.m_pNormal = (float *)(pBuffer + offset);
		// See PackNormal_[SHORT2|UBYTE4|HEND3N] in mathlib.h for the compression algorithm.
		offset += GetVertexElementSize(VERTEX_ELEMENT_NORMAL, compression);
		pVertexSizesToSet[nVertexSizesToSet++] = &(desc.m_VertexSize_Normal);
	}
	else
	{
		desc.m_pNormal = dummyData;
		desc.m_VertexSize_Normal = 0;
	}

	if (vertexFormat & VERTEX_COLOR)
	{
		desc.m_pColor = pBuffer + offset;
		offset += GetVertexElementSize(VERTEX_ELEMENT_COLOR, compression);
		pVertexSizesToSet[nVertexSizesToSet++] = &(desc.m_VertexSize_Color);
	}
	else
	{
		desc.m_pColor = (unsigned char *)dummyData;
		desc.m_VertexSize_Color = 0;
	}

	if (vertexFormat & VERTEX_SPECULAR)
	{
		desc.m_pSpecular = pBuffer + offset;
		offset += GetVertexElementSize(VERTEX_ELEMENT_SPECULAR, compression);
		pVertexSizesToSet[nVertexSizesToSet++] = &(desc.m_VertexSize_Specular);
	}
	else
	{
		desc.m_pSpecular = (unsigned char *)dummyData;
		desc.m_VertexSize_Specular = 0;
	}

	int i;
	for (i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; ++i)
	{
		// DX FIXME: Compress texcoords to SHORT2N/SHORT4N, with a scale rolled into the texture transform.
		const VertexElement_t texCoordElements[4] =
		{
			VERTEX_ELEMENT_TEXCOORD1D_0,
			VERTEX_ELEMENT_TEXCOORD2D_0,
			VERTEX_ELEMENT_TEXCOORD3D_0,
			VERTEX_ELEMENT_TEXCOORD4D_0
		};
		int nSize = TexCoordSize(i, vertexFormat);
		if (nSize)
		{
			desc.m_pTexCoord[i] = (float *)(pBuffer + offset);
			VertexElement_t texCoordElement = (VertexElement_t)(texCoordElements[nSize - 1] + i);
			offset += GetVertexElementSize(texCoordElement, compression);
			pVertexSizesToSet[nVertexSizesToSet++] = &(desc.m_VertexSize_TexCoord[i]);
		}
		else
		{
			desc.m_pTexCoord[i] = dummyData;
			desc.m_VertexSize_TexCoord[i] = 0;
		}
	}

	if (vertexFormat & VERTEX_TANGENT_S)
	{
		// DX UNDONE: Use normal compression here (use mem_dumpvballocs to see if this uses much memory).
		desc.m_pTangentS = (float *)(pBuffer + offset);
		offset += GetVertexElementSize(VERTEX_ELEMENT_TANGENT_S, compression);
		pVertexSizesToSet[nVertexSizesToSet++] = &(desc.m_VertexSize_TangentS);
	}
	else
	{
		desc.m_pTangentS = dummyData;
		desc.m_VertexSize_TangentS = 0;
	}
	if (vertexFormat & VERTEX_TANGENT_T)
	{
		// DX UNDONE: Use normal compression here (use mem_dumpvballocs to see if this uses much memory).
		desc.m_pTangentT = (float *)(pBuffer + offset);
		offset += GetVertexElementSize(VERTEX_ELEMENT_TANGENT_T, compression);
		pVertexSizesToSet[nVertexSizesToSet++] = &(desc.m_VertexSize_TangentT);
	}
	else
	{
		desc.m_pTangentT = dummyData;
		desc.m_VertexSize_TangentT = 0;
	}

	int userDataSize = UserDataSize(vertexFormat);
	if (userDataSize > 0)
	{
		desc.m_pUserData = (float *)(pBuffer + offset);
		// DX: See PackNormal_[SHORT2|UBYTE4|HEND3N] in mathlib.h for the compression algorithm.
		offset += GetVertexElementSize((VertexElement_t)(VERTEX_ELEMENT_USERDATA1 + (userDataSize - 1)), compression);
		pVertexSizesToSet[nVertexSizesToSet++] = &(desc.m_VertexSize_UserData);
	}
	else
	{
		desc.m_pUserData = dummyData;
		desc.m_VertexSize_UserData = 0;
	}

	if (!(vertexFormat & VERTEX_FORMAT_USE_EXACT_FORMAT) && (offset > 16))
		offset = (offset + 15) & (~15);
	desc.m_ActualVertexSize = offset;

	Assert(nVertexSizesToSet < (sizeof(pVertexSizesToSet) / sizeof(pVertexSizesToSet[0])));
	while (nVertexSizesToSet-- > 0)
		*(pVertexSizesToSet[nVertexSizesToSet]) = offset;
}

void CVertexBufferOES2::EndCastBuffer(void)
{
	Assert(m_IsDynamic && m_VertexFormat);
	m_VertexFormat = 0;
	m_VertexCount = 0;
}

void CVertexBufferOES2::HandlePerFrameTextureStats(int frame)
{
#ifdef VPROF_ENABLED
	if ((m_VProfFrame != frame) && !m_IsDynamic)
	{
		m_VProfFrame = frame;
		*m_pFrameCounter += m_BufferSize;
	}
#endif
}

bool CVertexBufferOES2::Lock(int nMaxVertexCount, bool bAppend, VertexDesc_t &desc)
{
	Assert(!m_pLockedData && (nMaxVertexCount != 0) && (nMaxVertexCount <= m_VertexCount));

	// DX FIXME: Why do we need to sync matrices now?
	ShaderUtil()->SyncMatrices();

	VPROF("CVertexBufferOES2::Lock");

	desc.m_nFirstVertex = 0;

	int memoryRequired;
	bool hasEnoughMemory;

	if (!nMaxVertexCount || (m_VertexFormat == VERTEX_FORMAT_UNKNOWN) || IsDeviceDeactivated())
		goto fail;

	if (nMaxVertexCount > m_VertexCount)
	{
		Warning("Too many vertices for vertex buffer. . tell a programmer (%d>%d)\n", nMaxVertexCount, m_VertexCount);
		goto fail;
	}

	if (!m_VertexBuffer && !Allocate())
		goto fail;

	memoryRequired = nMaxVertexCount * VertexSize();
	hasEnoughMemory = ((m_FirstUnwrittenOffset + memoryRequired) <= m_BufferSize);

	if (bAppend)
	{
		Assert(!m_Flush);
		if (!hasEnoughMemory)
			goto fail;
	}
	else if (m_Flush || !hasEnoughMemory || !m_IsDynamic)
	{
		m_FirstUnwrittenOffset = 0;
		m_Flush = false;
	}

	if (m_IsDynamic)
	{
		ComputeVertexDescription(s_DynamicLockedData, m_VertexFormat, desc);
	}
	else
	{
		m_pLockedData = new unsigned char[memoryRequired];
		ComputeVertexDescription(m_pLockedData, m_VertexFormat, desc);
	}
	desc.m_nOffset = m_FirstUnwrittenOffset;
	return true;

fail:
	ComputeVertexDescription(NULL, 0, desc);
	desc.m_nOffset = 0; 
	return false;
}

void CVertexBufferOES2::PrintVertexFormat(VertexFormat_t fmt)
{
	VertexCompressionType_t compression = CompressionType(fmt);
	int size;
	if (fmt & VERTEX_POSITION)
		Warning("VERTEX_POSITION|");
	if (fmt & VERTEX_NORMAL)
	{
		// DX FIXME: Genericise using VertexElement_t data tables (so funcs like "just work" if we make compression changes).
		if (compression != VERTEX_COMPRESSION_NONE)
			Warning("VERTEX_NORMAL[COMPRESSED]|");
		else
			Warning("VERTEX_NORMAL|");
	}
	if (fmt & VERTEX_COLOR)
		Warning("VERTEX_COLOR|");
	if (fmt & VERTEX_SPECULAR)
		Warning("VERTEX_SPECULAR|");
	if (fmt & VERTEX_TANGENT_S)
		Warning("VERTEX_TANGENT_S|");
	if (fmt & VERTEX_TANGENT_T)
		Warning("VERTEX_TANGENT_T|");
	if (fmt & VERTEX_BONE_INDEX)
		Warning("VERTEX_BONE_INDEX|");
	if (fmt & VERTEX_FORMAT_VERTEX_SHADER)
		Warning("VERTEX_FORMAT_VERTEX_SHADER|");
	size = NumBoneWeights(fmt);
	if (size > 0)
		Warning("VERTEX_BONEWEIGHT(%d)%s|", size, (compression ? "[COMPRESSED]" : ""));
	size = UserDataSize(fmt);
	if (size > 0)
		Warning("VERTEX_USERDATA_SIZE(%d)|", size);
	int i;
	for (i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; ++i)
	{
		size = TexCoordSize(i, fmt);
		if (size)
			Warning("VERTEX_TEXCOORD_SIZE(%d,%d)", i, size);
	}
	Warning("\n");
}

void CVertexBufferOES2::SetVertexFormat(VertexFormat_t fmt)
{
	Assert(m_IsDynamic);
	Assert((fmt != VERTEX_FORMAT_UNKNOWN) && (fmt != VERTEX_FORMAT_INVALID));
	m_VertexFormat = fmt;
	m_VertexCount = m_BufferSize / VertexSize();
}

void CVertexBufferOES2::Spew(int nVertexCount, const VertexDesc_t &desc)
{
	Warning("\nVerts %d (First %d, Offset %d) :\n", nVertexCount, desc.m_nFirstVertex, desc.m_nOffset);
	SpewVertexBuffer(nVertexCount, desc);
}

void CVertexBufferOES2::SpewVertexBuffer(int vertexCount, const VertexDesc_t &desc)
{
	char tempBuf[1024];
	int i, j, len = 0;
	for (i = 0; i < vertexCount; ++i)
	{
		len += Q_snprintf(tempBuf + len, sizeof(tempBuf) - len, "[%4d] ", i + desc.m_nFirstVertex);
		if (desc.m_VertexSize_Position)
		{
			Vector &pos = *((Vector *)((unsigned char *)(desc.m_pPosition) + i * desc.m_VertexSize_Position));
			len += Q_snprintf(tempBuf + len, sizeof(tempBuf) - len, "P %8.2f %8.2f %8.2f ", pos.x, pos.y, pos.z);
		}
		if (desc.m_VertexSize_Wrinkle)
		{
			len += Q_snprintf(tempBuf + len, sizeof(tempBuf) - len, "Wr %8.2f ",
				*((float *)((unsigned char *)(desc.m_pWrinkle) + i * desc.m_VertexSize_Wrinkle)));
		}
		if (desc.m_NumBoneWeights > 0)
		{
			Assert(desc.m_CompressionType == VERTEX_COMPRESSION_NONE);
			len += Q_snprintf(tempBuf + len, sizeof(tempBuf) - len, "BW ");
			float *pWeight = (float *)((unsigned char *)(desc.m_pBoneWeight) + i * desc.m_VertexSize_BoneWeight);
			for (j = 0; j < desc.m_NumBoneWeights; ++j)
				len += Q_snprintf(tempBuf + len, sizeof(tempBuf) - len, "%1.2f ", pWeight[j]);
		}
		if (desc.m_VertexSize_BoneMatrixIndex)
		{
			unsigned char *pIndex = desc.m_pBoneMatrixIndex + i * desc.m_VertexSize_BoneMatrixIndex;
			len += Q_snprintf(tempBuf + len, sizeof(tempBuf) - len, "BI %d %d %d %d ",
				pIndex[0], pIndex[1], pIndex[2], pIndex[3]);
			Assert((pIndex[0] >= 0) && (pIndex[0] < NUM_MODEL_TRANSFORMS));
			Assert((pIndex[1] >= 0) && (pIndex[1] < NUM_MODEL_TRANSFORMS));
			Assert((pIndex[2] >= 0) && (pIndex[2] < NUM_MODEL_TRANSFORMS));
			Assert((pIndex[3] >= 0) && (pIndex[3] < NUM_MODEL_TRANSFORMS));
		}
		if (desc.m_VertexSize_Normal)
		{
			Assert(desc.m_CompressionType == VERTEX_COMPRESSION_NONE);
			Vector &normal = *((Vector *)((unsigned char *)(desc.m_pNormal) + i * desc.m_VertexSize_Normal));
			len += Q_snprintf(tempBuf + len, sizeof(tempBuf) - len, "N %1.2f %1.2f %1.2f ", normal.x, normal.y, normal.z);
		}
		if (desc.m_VertexSize_Color)
		{
			unsigned char *pColor = desc.m_pColor + i * desc.m_VertexSize_Color;
			len += Q_snprintf(tempBuf + len, sizeof(tempBuf) - len, "C b %3d g %3d r %3d a %3d ",
				pColor[0], pColor[1], pColor[2], pColor[3]);
		}
		for (j = 0; j < VERTEX_MAX_TEXTURE_COORDINATES; ++j)
		{
			if (!(desc.m_VertexSize_TexCoord[j]))
				continue;
			Vector2D &texcoord = *((Vector2D *)((unsigned char *)(desc.m_pTexCoord[j]) + i * desc.m_VertexSize_TexCoord[j]));
			len += Q_snprintf(tempBuf + len, sizeof(tempBuf) - len, "T%d %.2f %.2f ", j, texcoord.x, texcoord.y);	
		}
		if (desc.m_VertexSize_TangentS)
		{
			Vector &tangentS = *((Vector *)((unsigned char *)(desc.m_pTangentS) + i * desc.m_VertexSize_TangentS));
			len += Q_snprintf(tempBuf + len, sizeof(tempBuf) - len, "S %1.2f %1.2f %1.2f ", tangentS.x, tangentS.y, tangentS.z);
		}
		if (desc.m_VertexSize_TangentT)
		{
			Vector &tangentT = *((Vector *)((unsigned char *)(desc.m_pTangentT) + i * desc.m_VertexSize_TangentT));
			len += Q_snprintf(tempBuf + len, sizeof(tempBuf) - len, "T %1.2f %1.2f %1.2f ", tangentT.x, tangentT.y, tangentT.z);
		}
		Q_snprintf(tempBuf + len, sizeof(tempBuf) - len, "\n");
		Warning(tempBuf);
		len = 0;
	}
}

void CVertexBufferOES2::Unlock(int nWrittenVertexCount, VertexDesc_t &desc)
{
	if (!m_IsDynamic && !m_pLockedData)
		return;
	Assert((m_FirstUnwrittenOffset + nWrittenVertexCount) <= m_VertexCount);
	MeshMgr()->BindOESBuffer(OES_BUFFER_TARGET_VERTEX, m_VertexBuffer);
	int size = nWrittenVertexCount * VertexSize();
	if (m_IsDynamic)
	{
		if (nWrittenVertexCount)
			glBufferSubData(GL_ARRAY_BUFFER, m_FirstUnwrittenOffset, size, s_DynamicLockedData);
	}
	else
	{
		if (nWrittenVertexCount)
			glBufferSubData(GL_ARRAY_BUFFER, m_FirstUnwrittenOffset, size, m_pLockedData);
		delete[] m_pLockedData;
		m_pLockedData = NULL;
	}
	m_FirstUnwrittenOffset += size;
}

void CVertexBufferOES2::ValidateData(int nVertexCount, const VertexDesc_t &desc)
{
#ifdef VALIDATE_DEBUG
	ValidateVertexBufferData(nVertexCount, desc, m_pMaterial->GetVertexUsage());
#endif
}

void CVertexBufferOES2::ValidateVertexBufferData(int nVertexCount, const VertexDesc_t &desc, VertexFormat_t fmt)
{
#ifdef VALIDATE_DEBUG
	int i, j, numBoneWeights = NumBoneWeights(fmt);
	for (i = 0; i < nVertexCount; ++i)
	{
		if (fmt & VERTEX_POSITION)
		{
			Vector &pos = *((Vector *)((unsigned char *)(desc.m_pPosition) + i * desc.m_VertexSize_Position));
			Assert(IsFinite(pos.x) && IsFinite(pos.y) && IsFinite(pos.z));
		}
		if (fmt & VERTEX_WRINKLE)
		{
			Assert(fmt & VERTEX_POSITION);
			Assert(IsFinite(*((float *)((unsigned char *)(desc.m_pWrinkle) + i * desc.m_VertexSize_Wrinkle))));
		}
		if (numBoneWeights > 0)
		{
			float *pWeight = (float *)((unsigned char *)(desc.m_pBoneWeight) + i * desc.m_VertexSize_BoneWeight);
			for (j = numBoneWeights; j-- > 0; )
				Assert((pWeight[j] >= 0.0f) && (pWeight[j] <= 1.0f));
		}
		if (fmt & VERTEX_BONE_INDEX)
		{
			unsigned char *pIndex = desc.m_pBoneMatrixIndex + i * desc.m_VertexSize_BoneMatrixIndex;
			Assert((pIndex[0] >= 0) && (pIndex[0] < NUM_MODEL_TRANSFORMS));
			Assert((pIndex[1] >= 0) && (pIndex[1] < NUM_MODEL_TRANSFORMS));
			Assert((pIndex[2] >= 0) && (pIndex[2] < NUM_MODEL_TRANSFORMS));
			Assert((pIndex[3] >= 0) && (pIndex[3] < NUM_MODEL_TRANSFORMS));
		}
		if (fmt & VERTEX_NORMAL)
		{
			Assert(desc.m_CompressionType == VERTEX_COMPRESSION_NONE);
			Vector &normal = *((Vector *)((unsigned char *)(desc.m_pNormal) + i * desc.m_VertexSize_Normal));
			Assert((normal[0] >= -1.05f) && (normal[0] <= 1.05f));
			Assert((normal[1] >= -1.05f) && (normal[1] <= 1.05f));
			Assert((normal[2] >= -1.05f) && (normal[2] <= 1.05f));
		}
		for (j = VERTEX_MAX_TEXTURE_COORDINATES; j--; )
		{
			if (TexCoordSize(j, fmt) <= 0)
				continue;
			Vector2D &texcoord = *((Vector2D *)((unsigned char *)(desc.m_pTexCoord[j]) + i * desc.m_VertexSize_TexCoord[j]));
			Assert(IsFinite(texcoord.x) && IsFinite(texcoord.y));
		}
		if (fmt & VERTEX_TANGENT_S)
		{
			Vector &tangentS = *((Vector *)((unsigned char *)(desc.m_pTangentS) + i * desc.m_VertexSize_TangentS));
			Assert(IsFinite(tangentS.x) && IsFinite(tangentS.y) && IsFinite(tangentS.z));
		}
		if (fmt & VERTEX_TANGENT_T)
		{
			Vector &tangentT = *((Vector *)((unsigned char *)(desc.m_pTangentT) + i * desc.m_VertexSize_TangentT));
			Assert(IsFinite(tangentT.x) && IsFinite(tangentT.y) && IsFinite(tangentT.z));
		}
	}
#endif
}

int CVertexBufferOES2::VertexFormatSize(VertexFormat_t fmt)
{
	// Depends on VertexElement_t!

	// Always exactly two weights if any are present.
	Assert(((NumBoneWeights(fmt) == 2) && (fmt & VERTEX_BONE_INDEX))
		|| ((NumBoneWeights(fmt) == 0) && !(fmt & VERTEX_BONE_INDEX)));

	int size = 0;

	Assert(!(fmt & VERTEX_WRINKLE) || (fmt & VERTEX_POSITION));
	if (fmt & VERTEX_POSITION)
	{
		size += 3 * sizeof(float);
		if (fmt & VERTEX_WRINKLE)
			size += 1 * sizeof(float);
	}

	if (fmt & VERTEX_COLOR)
		size += 4 * sizeof(unsigned char);
	if (fmt & VERTEX_SPECULAR)
		size += 4 * sizeof(unsigned char);

	int i;
	for (i = VERTEX_MAX_TEXTURE_COORDINATES; i--; )
		size += TexCoordSize(i, fmt) * sizeof(float);

	if (fmt & VERTEX_TANGENT_S)
		size += 3 * sizeof(float);
	if (fmt & VERTEX_TANGENT_T)
		size += 3 * sizeof(float);

	int userDataSize = UserDataSize(fmt);
	if (CompressionType(fmt) == VERTEX_COMPRESSION_ON)
	{
		if (fmt & VERTEX_BONE_INDEX)
			size += 2 * sizeof(short) + 4 * sizeof(unsigned char);
#if COMPRESSED_NORMALS_TYPE == COMPRESSED_NORMALS_SEPARATETANGENTS_SHORT2
		if (fmt & VERTEX_NORMAL)
			size += 2 * sizeof(short);
		if (userDataSize == 4)
			size += 2 * sizeof(short);
		else
			size += userDataSize * sizeof(float);
#else // COMPRESSED_NORMALS_TYPE == COMPRESSED_NORMALS_COMBINEDTANGENTS_UBYTE4
		if (fmt & VERTEX_NORMAL)
			size += 4 * sizeof(unsigned char);
		if (userDataSize != 4)
			size += userDataSize * sizeof(float);
#endif
	}
	else
	{
		if (fmt & VERTEX_BONE_INDEX)
			size += 2 * sizeof(float) + 4 * sizeof(unsigned char);
		if (fmt & VERTEX_NORMAL)
			size += 3 * sizeof(float);
		size += userDataSize * sizeof(float);
	}

	if (!(fmt & VERTEX_FORMAT_USE_EXACT_FORMAT) && (size > 16))
		size = (size + 15) & (~15);

	return size;
}