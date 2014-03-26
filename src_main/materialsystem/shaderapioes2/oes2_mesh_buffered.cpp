//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 buffered mesh.
//
//===========================================================================//
#include "oes2.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

CBufferedMeshOES2::CBufferedMeshOES2(void)
{
	m_FlushNeeded = false;
	m_IsFlushing = false;
	m_LastIndex = 0;
	m_pMesh = NULL;
	m_WasRendered = true;
}

void CBufferedMeshOES2::Draw(int nFirstIndex, int nIndexCount)
{
	if (!(ShaderUtil()->OnDrawMesh(this, nFirstIndex, nIndexCount)))
	{
		m_WasRendered = true;
		MarkAsDrawn();
		return;
	}
	Assert(!m_IsFlushing && !m_WasRendered);
	Assert(!nIndexCount && (nFirstIndex == -1));
	m_WasRendered = true;
	m_FlushNeeded = true;
	if (m_pMesh->HasFlexMesh() || !(ShaderUtil()->GetConfig().bBufferPrimitives))
		ShaderAPI()->FlushBufferedPrimitives();
}

void CBufferedMeshOES2::Flush(void)
{
	if (!m_pMesh || m_IsFlushing || !m_FlushNeeded)
		return;
	VPROF("CBufferedMeshOES2::Flush");
	m_IsFlushing = true;
	(static_cast<IMesh *>(m_pMesh))->Draw();
	m_IsFlushing = false;
	m_FlushNeeded = false;
	m_pMesh->SetFlexMesh(NULL, 0);
}

void CBufferedMeshOES2::LockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc)
{
	Assert(m_pMesh);
	Assert(m_WasRendered);
	ShaderUtil()->SyncMatrices();
	m_pMesh->PreLock();

	m_ExtraIndices = 0;
	bool tristripFixup = m_pMesh->IndexCount() && (m_pMesh->GetPrimitiveType() == MATERIAL_TRIANGLE_STRIP);
	if (tristripFixup)
	{
		m_ExtraIndices = (m_pMesh->IndexCount() & 1) ? 3 : 2;
		nIndexCount += m_ExtraIndices;
	}
	if (!(m_pMesh->HasEnoughRoom(nVertexCount, nIndexCount)))
		ShaderAPI()->FlushBufferedPrimitives();

	m_pMesh->LockMesh(nVertexCount, nIndexCount, desc);

	if (tristripFixup && desc.m_nIndexSize)
	{
		*(desc.m_pIndices++) = m_LastIndex;
		if (m_ExtraIndices == 3)
			*(desc.m_pIndices++) = m_LastIndex;
		++desc.m_pIndices;
	}

	m_WasRendered = false;
}

void CBufferedMeshOES2::SetFlexMesh(IMesh *pMesh, int nVertexOffset)
{
	ShaderAPI()->FlushBufferedPrimitives(); // Read DX FIXME on this.
	m_pMesh->SetFlexMesh(pMesh, nVertexOffset);
}

void CBufferedMeshOES2::SetMesh(IMeshOES2 *pMesh)
{
	if (m_pMesh == pMesh)
		return;
	ShaderAPI()->FlushBufferedPrimitives();
	m_pMesh = pMesh;
}

void CBufferedMeshOES2::SetPrimitiveType(MaterialPrimitiveType_t type)
{
	Assert((type != MATERIAL_INSTANCED_QUADS) && (type != MATERIAL_HETEROGENOUS));
	if (GetPrimitiveType() == type)
		return;
	ShaderAPI()->FlushBufferedPrimitives();
	m_pMesh->SetPrimitiveType(type);
}

void CBufferedMeshOES2::SetVertexFormat(VertexFormat_t fmt)
{
	Assert(m_pMesh);
	if (!(m_pMesh->NeedsVertexFormatReset(fmt)))
		return;
	ShaderAPI()->FlushBufferedPrimitives();
	m_pMesh->SetVertexFormat(fmt);
}

void CBufferedMeshOES2::UnlockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc)
{
	Assert(m_pMesh);
	if (desc.m_nIndexSize && (m_pMesh->GetPrimitiveType() == MATERIAL_TRIANGLE_STRIP))
	{
		if (m_ExtraIndices)
			*(desc.m_pIndices - 1) = *(desc.m_pIndices);
		m_LastIndex = desc.m_pIndices[nIndexCount - 1];
		nIndexCount += m_ExtraIndices;
	}
	m_pMesh->UnlockMesh(nVertexCount, nIndexCount, desc);
}