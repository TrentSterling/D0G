//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 base mesh.
//
//===========================================================================//
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

void CBaseMeshOES2::Spew(int nVertexCount, const VertexDesc_t &desc)
{
	Warning("\nVerts %d (First %d, Offset %d) :\n", nVertexCount, desc.m_nFirstVertex, desc.m_nOffset);
	CVertexBufferOES2::SpewVertexBuffer(nVertexCount, desc);
}

void CBaseMeshOES2::Spew(int nIndexCount, const IndexDesc_t &desc)
{
	Warning("\nIndices: %d (First %d, Offset %d)\n", nIndexCount, desc.m_nFirstIndex, desc.m_nOffset);
	CIndexBufferOES2::SpewIndexBuffer(nIndexCount, desc);
}

void CBaseMeshOES2::Spew(int nVertexCount, int nIndexCount, const MeshDesc_t &desc)
{
	Warning("\nVerts: (Vertex Format %x)\n", m_VertexFormat);
	CVertexBufferOES2::PrintVertexFormat(m_VertexFormat);
	CVertexBufferOES2::SpewVertexBuffer(nVertexCount, *(static_cast<const VertexDesc_t *>(&desc)));
	Warning("\nIndices: %d\n", nIndexCount);
	CIndexBufferOES2::SpewIndexBuffer(nIndexCount, *(static_cast<const IndexDesc_t *>(&desc)));
}

void CBaseMeshOES2::ValidateData(int nVertexCount, const VertexDesc_t &desc)
{
#ifdef VALIDATE_DEBUG
	CVertexBufferOES2::ValidateVertexBufferData(nVertexCount, desc, m_pMaterial->GetVertexUsage());
#endif
}

void CBaseMeshOES2::ValidateData(int nVertexCount, int nIndexCount, const MeshDesc_t &desc)
{
#ifdef VALIDATE_DEBUG
	CVertexBufferOES2::ValidateVertexBufferData(nVertexCount, *(static_cast<VertexDesc_t *>(&desc)),
		m_pMaterial->GetVertexUsage());
#endif
}