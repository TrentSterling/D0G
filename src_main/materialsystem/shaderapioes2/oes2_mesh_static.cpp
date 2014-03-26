//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 static mesh.
//
//===========================================================================//
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

int CMeshOES2::s_nPrims;
const CPrimList *CMeshOES2::s_pPrims;

CMeshOES2::CMeshOES2(const char *pTextureGroupName)
{
	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
	m_pColorMesh = NULL;
	m_pFlexVertexBuffer = NULL;

	m_Type = MATERIAL_TRIANGLES;
	m_Mode = GL_TRIANGLES;

	m_NumIndices = 0;

	m_IsVBLocked = false;
	m_IsIBLocked = false;

	m_pTextureGroupName = pTextureGroupName;
}

CMeshOES2::~CMeshOES2(void)
{
	if (!(MeshMgr()->IsDynamicMesh(this)))
	{
		if (m_pVertexBuffer)
		{
			delete m_pVertexBuffer;
			m_pVertexBuffer = NULL;
		}
		if (m_pIndexBuffer)
		{
			delete m_pIndexBuffer;
			m_pIndexBuffer = NULL;
		}
	}
}

void CMeshOES2::Draw(int nFirstIndex, int nIndexCount)
{
	if (!(ShaderUtil()->OnDrawMesh(this, nFirstIndex, nIndexCount)))
	{
		MarkAsDrawn();
		return;
	}

	CPrimList primList;
	if (!nIndexCount || (nFirstIndex == -1))
	{
		primList.m_FirstIndex = 0;
		primList.m_NumIndices = m_NumIndices;
	}
	else
	{
		primList.m_FirstIndex = nFirstIndex;
		primList.m_NumIndices = nIndexCount;
	}
	DrawInternal(&primList, 1);
}

void CMeshOES2::Draw(CPrimList *pLists, int nLists)
{
	if (!(ShaderUtil()->OnDrawMesh(this, pLists, nLists)))
	{
		MarkAsDrawn();
		return;
	}
	DrawInternal(pLists, nLists);
}

void CMeshOES2::DrawInternal(const CPrimList *pLists, int nLists)
{
	int i;
	for (i = 0; i < nLists; ++i)
	{
		if (pLists[i].m_NumIndices > 0)
			break;
	}
	if (i == nLists)
		return;
	Assert(!(ShaderAPI()->IsInSelectionMode()));
	if (!SetRenderState(0))
		return;
	s_nPrims = nLists;
	s_pPrims = pLists;
	ShaderAPI()->DrawMesh(this);
}

bool CMeshOES2::Lock(int nVertexCount, bool bAppend, VertexDesc_t &desc)
{
	Assert(!m_IsVBLocked);
	if (!nVertexCount || IsDeviceDeactivated())
	{
		CVertexBufferOES2::ComputeVertexDescription(NULL, 0, desc);
		return false;
	}
	if (!m_pVertexBuffer)
		m_pVertexBuffer = new CVertexBufferOES2(SHADER_BUFFER_TYPE_STATIC, m_VertexFormat, nVertexCount, m_pTextureGroupName);
	if (!(m_pVertexBuffer->Lock(nVertexCount, bAppend, desc)))
	{
		int maxVerts, maxIndices;
		MeshMgr()->GetMaxToRender(this, false, &maxVerts, &maxIndices);
		if (nVertexCount > maxVerts)
		{
			Error("Too many verts for a dynamic vertex buffer (%d>%d) Tell a programmer to up VERTEX_BUFFER_SIZE.\n",
				nVertexCount, maxVerts);
		}
		// ComputeVertexDescription is already done by CVertexBufferOES2::Lock.
		return false;
	}
	m_IsVBLocked = true;
	return true;
}

bool CMeshOES2::Lock(int nMaxIndexCount, bool bAppend, IndexDesc_t &desc)
{
	Assert(!m_IsIBLocked);
	if (!nMaxIndexCount || IsDeviceDeactivated())
	{
		desc.m_pIndices = g_ScratchIndexBuffer;
		desc.m_nIndexSize = 0;
		return false;
	}
	if (!m_pIndexBuffer)
		m_pIndexBuffer = new CIndexBufferOES2(SHADER_BUFFER_TYPE_STATIC, IndexFormat(), nMaxIndexCount, m_pTextureGroupName);
	if (!(m_pIndexBuffer->Lock(nMaxIndexCount, bAppend, desc)))
	{
		Error("failed to lock index buffer in CMeshOES2::LockIndexBuffer\n");
		return false;
	}
	m_IsIBLocked = true;
	return true;
}

void CMeshOES2::LockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc)
{
	ShaderUtil()->SyncMatrices();
	VPROF("CMeshOES2::LockMesh");
	Lock(nVertexCount, false, *(static_cast<VertexDesc_t *>(&desc)));
	if ((nIndexCount >= 0) && (m_Type != MATERIAL_POINTS))
	{
		Lock(nIndexCount, false, *(static_cast<IndexDesc_t *>(&desc)));
	}
	else
	{
		desc.m_pIndices = g_ScratchIndexBuffer;
		desc.m_nIndexSize = 0;
	}
}

void CMeshOES2::RenderPass(void)
{
	Assert(m_Type != MATERIAL_HETEROGENOUS);
	VPROF("CMeshOES2::RenderPass");

	int i;
	const CPrimList *pPrim = s_pPrims;
	bool applied = false;
	if (m_Type == MATERIAL_POINTS)
	{
		for (i = 0; i < s_nPrims; ++i, ++pPrim)
		{
			if (!(pPrim->m_NumIndices))
				continue;
			if (!applied)
			{
				applied = true;
				MeshMgr()->ApplyStreamState();
			}
			glDrawArrays(GL_POINTS, 0, pPrim->m_NumIndices);
		}
	}
	else
	{
		MeshMgr()->BindOESBuffer(OES_BUFFER_TARGET_INDEX, m_pIndexBuffer->GetOESBuffer());
		for (i = 0; i < s_nPrims; ++i, ++pPrim)
		{
			if (!(pPrim->m_NumIndices))
				continue;
#ifdef VPROF_ENABLED
			int numPrimitives;
			switch (m_Type)
			{
			case MATERIAL_LINES:
				numPrimitives = pPrim->m_NumIndices >> 1; break;
			case MATERIAL_TRIANGLES:
				numPrimitives = pPrim->m_NumIndices / 3; break;
			case MATERIAL_TRIANGLE_STRIP:
				numPrimitives = pPrim->m_NumIndices - 2; break;
			NO_DEFAULT
			}
			VPROF("glDrawElements");
			VPROF_INCREMENT_COUNTER("DrawElements", 1);
			VPROF_INCREMENT_COUNTER("numPrimitives", numPrimitives);
#endif
			if (!applied)
			{
				applied = true;
				MeshMgr()->ApplyStreamState();
			}
			glDrawElements(m_Mode, pPrim->m_NumIndices, GL_UNSIGNED_SHORT, (void *)(pPrim->m_FirstIndex << 1));
		}
	}
}

void CMeshOES2::SetColorMesh(IMesh *pColorMesh, int nVertexOffset)
{
	if (!(ShaderUtil()->OnSetColorMesh(this, pColorMesh, nVertexOffset)))
		return;
	Assert(pColorMesh || !nVertexOffset);
	m_pColorMesh = static_cast<IMeshOES2 *>(pColorMesh);
	m_ColorMeshVertOffsetInBytes = nVertexOffset;
}

void CMeshOES2::SetFlexMesh(IMesh *pMesh, int nVertexOffset)
{
	if (!(ShaderUtil()->OnSetFlexMesh(this, pMesh, nVertexOffset)))
		return;
	m_FlexVertOffsetInBytes = nVertexOffset;
	if (pMesh)
	{
		pMesh->MarkAsDrawn();
		m_pFlexVertexBuffer = (static_cast<IMeshOES2 *>(pMesh))->GetVertexBuffer();
	}
	else
	{
		m_pFlexVertexBuffer = NULL;
	}
}

void CMeshOES2::SetPrimitiveType(MaterialPrimitiveType_t type)
{
	Assert(type != MATERIAL_INSTANCED_QUADS);
	if (!(ShaderUtil()->OnSetPrimitiveType(this, type)))
		return;
	m_Type = type;
	switch (type)
	{
	case MATERIAL_POINTS:
		m_Mode = GL_POINTS;
		break;
	case MATERIAL_LINES:
		m_Mode = GL_LINES;
		break;
	case MATERIAL_TRIANGLES:
		m_Mode = GL_TRIANGLES;
		break;
	case MATERIAL_TRIANGLE_STRIP:
		m_Mode = GL_TRIANGLE_STRIP;
		break;
	case MATERIAL_HETEROGENOUS:
		m_Mode = 0;
		break;
	NO_DEFAULT
	}
}

bool CMeshOES2::SetRenderState(int nVertexOffsetInBytes)
{
	if (IsDeviceDeactivated())
		return false;

	int frame = ShaderAPI()->GetCurrentFrameCounter();

	IVertexBufferOES2 *pVB = m_pVertexBuffer;
	Assert(m_pVertexBuffer);
	if (MeshMgr()->SetStreamState(0, pVB->GetOESBuffer(), nVertexOffsetInBytes, pVB->VertexSize()))
		pVB->HandlePerFrameTextureStats(frame);

	if (m_pColorMesh)
	{
		pVB = m_pColorMesh->GetVertexBuffer();
		if (MeshMgr()->SetStreamState(1, pVB->GetOESBuffer(), m_ColorMeshVertOffsetInBytes, pVB->VertexSize()))
			pVB->HandlePerFrameTextureStats(frame);
	}
	else
		MeshMgr()->SetStreamState(1, 0, 0, 0);

	pVB = m_pFlexVertexBuffer;
	if (m_pFlexVertexBuffer)
	{
		if (MeshMgr()->SetStreamState(2, pVB->GetOESBuffer(), m_FlexVertOffsetInBytes, pVB->VertexSize()))
			pVB->HandlePerFrameTextureStats(frame);
	}
	else
		MeshMgr()->SetStreamState(2, 0, 0, 0);

	Assert(m_pIndexBuffer);
	m_pIndexBuffer->HandlePerFrameTextureStats(frame); // D0GTODO: Do this only if index buffer or offset changes!!!

	return true;
}

void CMeshOES2::Unlock(int nWrittenVertexCount, VertexDesc_t &desc)
{
	if (!m_IsVBLocked)
		return;
	Assert(m_pVertexBuffer);
	m_pVertexBuffer->Unlock(nWrittenVertexCount, desc);
	m_IsVBLocked = false;
}

void CMeshOES2::Unlock(int nWrittenIndexCount, IndexDesc_t &desc)
{
	if (!m_IsIBLocked)
		return;
	Assert(m_pIndexBuffer);
	m_pIndexBuffer->Unlock(nWrittenIndexCount, desc);
	m_IsIBLocked = false;
}

void CMeshOES2::UnlockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc)
{
	VPROF("CMeshOES2::UnlockMesh");
	if (m_IsVBLocked)
		Unlock(nVertexCount, *(static_cast<VertexDesc_t *>(&desc)));
	if (m_IsIBLocked)
		Unlock(nIndexCount, *(static_cast<IndexDesc_t *>(&desc)));
	m_NumIndices = nIndexCount;
}