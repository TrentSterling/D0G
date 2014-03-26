//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 dynamic mesh.
//
//===========================================================================//
#include "oes2.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

void CDynamicMeshOES2::Draw(int nFirstIndex, int nIndexCount)
{
	if (!(ShaderUtil()->OnDrawMesh(this, nFirstIndex, nIndexCount)))
	{
		MarkAsDrawn();
		return;
	}
	VPROF("CDynamicMeshOES2::Draw");
	m_HasDrawn = true;
	if (!(m_IndexOverride || m_VertexOverride ||
		((m_TotalVertices > 0) && ((m_TotalIndices > 0) || (m_Type == MATERIAL_POINTS))) ))
		return;

	if (!SetRenderState((m_VertexOverride || HasFlexMesh()) ? 0 : m_FirstVertex))
		return;
	int baseIndex = (!m_IndexOverride || (m_pIndexBuffer == MeshMgr()->GetDynamicIndexBuffer())) ? m_FirstIndex : 0;

	if (nIndexCount && (nFirstIndex != -1))
	{
		nFirstIndex += baseIndex;
	}
	else
	{
		nFirstIndex = baseIndex;
		if (m_IndexOverride)
			nIndexCount = m_pIndexBuffer->IndexCount();
		else
			nIndexCount = (m_Type == MATERIAL_POINTS) ? m_TotalVertices : m_TotalIndices;
	}
	Assert(nIndexCount);

	CPrimList prim(nFirstIndex, nIndexCount);
	s_nPrims = 1;
	s_pPrims = &prim;

	ShaderAPI()->DrawMesh(this);
}

void CDynamicMeshOES2::LockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc)
{
	ShaderUtil()->SyncMatrices();
	PreLock();
	if (m_VertexOverride)
		nVertexCount = 0;
	if (m_IndexOverride)
		nIndexCount = 0;
	Lock(nVertexCount, false, *(static_cast<VertexDesc_t *>(&desc)));
	m_FirstVertex = (static_cast<VertexDesc_t *>(&desc))->m_nOffset;
	if (m_Type != MATERIAL_POINTS)
	{
		Lock(nIndexCount, false, *(static_cast<IndexDesc_t *>(&desc)));
		m_FirstIndex = (static_cast<IndexDesc_t *>(&desc))->m_nOffset >> 1;
	}
	else
	{
		desc.m_pIndices = g_ScratchIndexBuffer;
		desc.m_nIndexSize = 0;
	}
}

void CDynamicMeshOES2::SetVertexFormat(VertexFormat_t fmt)
{
	if (IsDeviceDeactivated())
		return;
	if ((fmt != m_VertexFormat) || m_VertexOverride || m_IndexOverride)
	{
		m_VertexFormat = fmt;
		UseVertexBuffer(MeshMgr()->FindOrCreateVertexBuffer(m_BufferId, fmt));
		if (!m_BufferId)
			UseIndexBuffer(MeshMgr()->GetDynamicIndexBuffer());
		m_VertexOverride = m_IndexOverride = false;
	}
}

void CDynamicMeshOES2::Reset(void)
{
	m_VertexFormat = 0;
	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
	ResetVertexAndIndexCounts();
}

void CDynamicMeshOES2::UnlockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc)
{
	m_TotalVertices += nVertexCount;
	m_TotalIndices += nIndexCount;
	CMeshOES2::UnlockMesh(nVertexCount, nIndexCount, desc);
}