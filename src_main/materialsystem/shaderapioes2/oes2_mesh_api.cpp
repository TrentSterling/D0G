//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 mesh ShaderAPI functions.
//
//===========================================================================//
#include "oes2.h"
#include "datacache/idatacache.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

void CShaderAPIOES2::BeginPass(StateSnapshot_t snapshot)
{
	if (IsDeactivated())
		return;
	VPROF("CShaderAPIOES2::BeginPass");
	m_CurrentSnapshot = snapshot;
	m_TransitionTable.UseSnapshot(m_CurrentSnapshot); // Moved from RenderPass for dynamic constants.

	CommitFastClipPlane();
	CommitVertexShaderTransforms();
	if (m_pMaterial && m_pMaterial->IsVertexLit())
		CommitVertexShaderLighting();

	if (m_pRenderMesh)
		m_pRenderMesh->BeginPass();
}

void CShaderAPIOES2::ComputeVertexDescription(unsigned char *pBuffer, VertexFormat_t vertexFormat, MeshDesc_t &desc) const
{
	CVertexBufferOES2::ComputeVertexDescription(pBuffer, vertexFormat, desc);
}

IMesh *CShaderAPIOES2::CreateStaticMesh(VertexFormat_t vertexFormat, const char *pTextureBudgetGroup, IMaterial *pMaterial)
{
	CMeshOES2 *pNewMesh = new CMeshOES2(pTextureBudgetGroup);
	pNewMesh->SetVertexFormat(vertexFormat);
	return pNewMesh;
}

void CShaderAPIOES2::DestroyStaticMesh(IMesh *pMesh)
{
	Assert(!(MeshMgr()->IsDynamicMesh(pMesh)));
	CBaseMeshOES2 *pMeshImp = static_cast<CBaseMeshOES2 *>(pMesh);
	if (pMeshImp)
		delete pMeshImp;
}

void CShaderAPIOES2::DestroyVertexBuffers(bool bExitingLevel)
{
	MeshMgr()->DestroyVertexBuffers();
	m_DynamicVBSize = bExitingLevel ? DYNAMIC_VERTEX_BUFFER_MEMORY_SMALL : DYNAMIC_VERTEX_BUFFER_MEMORY;
}

void CShaderAPIOES2::DrawMesh(IMeshOES2 *pMesh)
{
	if (ShaderUtil()->GetConfig().m_bSuppressRendering)
		return;
	Assert(pMesh && m_pMaterial);
	VPROF("CShaderAPIOES2::DrawMesh");
	m_pRenderMesh = pMesh;
	MeshMgr()->SetVertexDecl(pMesh->GetVertexFormat());
	CommitSetViewports();
	CommitSetScissorRect();
	m_pMaterial->DrawMesh(VERTEX_COMPRESSION_NONE);
	m_pRenderMesh = NULL;
}

void CShaderAPIOES2::FlushBufferedPrimitives(void)
{
	if (!(ShaderUtil()) || ShaderUtil()->OnFlushBufferedPrimitives())
		FlushBufferedPrimitivesInternal();
}

void CShaderAPIOES2::FlushBufferedPrimitivesInternal(void)
{
	Assert(!m_pRenderMesh);
	MaterialMatrixMode_t tempStack = m_CurrStack;
	MeshMgr()->Flush();
	m_CurrStack = tempStack;
}

IMesh *CShaderAPIOES2::GetDynamicMesh(IMaterial *pMaterial, int nHWSkinBoneCount, bool bBuffered,
	IMesh *pVertexOverride, IMesh *pIndexOverride)
{
	return MeshMgr()->GetDynamicMesh(pMaterial, 0, nHWSkinBoneCount, bBuffered, pVertexOverride, pIndexOverride);
}

IMesh *CShaderAPIOES2::GetDynamicMeshEx(IMaterial *pMaterial, VertexFormat_t vertexFormat, int nHWSkinBoneCount, bool bBuffered,
	IMesh *pVertexOverride, IMesh *pIndexOverride)
{
	return MeshMgr()->GetDynamicMesh(pMaterial, vertexFormat, nHWSkinBoneCount, bBuffered, pVertexOverride, pIndexOverride);
}

IMesh *CShaderAPIOES2::GetFlexMesh(void)
{
	return MeshMgr()->GetFlexMesh();
}

void CShaderAPIOES2::GetMaxToRender(IMesh *pMesh, bool bMaxUntilFlush, int *pMaxVerts, int *pMaxIndices)
{
	MeshMgr()->GetMaxToRender(pMesh, bMaxUntilFlush, pMaxVerts, pMaxIndices);
}

int CShaderAPIOES2::GetMaxVerticesToRender(IMaterial *pMaterial)
{
	VertexFormat_t fmt =
		((IMaterialInternal *)pMaterial)->GetRealTimeVersion()->GetVertexFormat() & ~VERTEX_FORMAT_COMPRESSED;
	int maxVerts = GetCurrentDynamicVBSize() / CVertexBufferOES2::VertexFormatSize(fmt);
	if (maxVerts > 65535)
		return 65535;
	return maxVerts;
}

void CShaderAPIOES2::RenderPass(int nPass, int nPassCount)
{
	if (IsDeactivated())
		return;
	Assert(m_CurrentSnapshot != -1);
	Assert(m_pRenderMesh);
#ifdef TEST_CACHE_LOCKS
	g_pDataCache->Flush();
#endif
	m_pRenderMesh->RenderPass();
	m_CurrentSnapshot = -1;
}