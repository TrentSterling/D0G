//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 temporary (selection) mesh.
//
//===========================================================================//
#include <cstd/string.h>
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

CTempMeshOES2::CTempMeshOES2(bool isDynamic)
{
	m_VertexSize = 0xffff;
	m_IsDynamic = isDynamic;
}

void CTempMeshOES2::CopyToMeshBuilder(int iStartVert, int nVerts,
	int iStartIndex, int nIndices, int indexOffset, CMeshBuilder &builder)
{
	int startOffset = iStartVert * m_VertexSize;
	int endOffset = (iStartVert + nVerts) * m_VertexSize;
	Assert((startOffset >= 0) && (startOffset <= m_VertexData.Count()));
	Assert((endOffset >= 0) && (endOffset <= m_VertexData.Count()) && (endOffset >= startOffset));
	if (endOffset > startOffset)
	{
		// DX FIXME:
		// Make this a method of CMeshBuilder (so the "Position" pointer accessor can be removed).
		// Make sure it takes a VertexFormat_t parameter for src/dest match validation.
		memcpy((void *)(builder.Position()), &(m_VertexData[startOffset]), endOffset - startOffset);
		builder.AdvanceVertices(nVerts);
	}
	while (nIndices-- > 0)
	{
		builder.Index(m_IndexData[iStartIndex++] + indexOffset);
		builder.AdvanceIndex();
	}
}

void CTempMeshOES2::Draw(int nFirstIndex, int nIndexCount)
{
	Assert(ShaderAPI()->IsInSelectionMode()); // Otherwise RenderPass will be called, which is Assert(0).
	if (!(ShaderUtil()->OnDrawMesh(this, nFirstIndex, nIndexCount)))
	{
		MarkAsDrawn();
		return;
	}
	if (m_VertexData.Count() <= 0)
		return;
	if (!IsDeviceDeactivated())
		TestSelection();
	if (m_IsDynamic)
	{
		m_VertexData.RemoveAll();
		m_IndexData.RemoveAll();
	}
}

void CTempMeshOES2::LockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc)
{
	ShaderUtil()->SyncMatrices();
	m_LockedVerts = nVertexCount;
	m_LockedIndices = nIndexCount;
	if (nVertexCount > 0)
	{
		int vertexByteOffset = m_VertexData.AddMultipleToTail(m_VertexSize * nVertexCount);
		desc.m_nFirstVertex = vertexByteOffset / m_VertexSize;
		CVertexBufferOES2::ComputeVertexDescription(&(m_VertexData[vertexByteOffset]), m_VertexFormat, desc);
	}
	else
	{
		desc.m_nFirstVertex = 0;
		CVertexBufferOES2::ComputeVertexDescription(0, 0, desc);
	}
	if ((nIndexCount > 0) && (m_Type != MATERIAL_POINTS))
	{
		int firstIndex = m_IndexData.AddMultipleToTail(nIndexCount);
		desc.m_pIndices = &(m_IndexData[firstIndex]);
		desc.m_nIndexSize = 1;
	}
	else
	{
		desc.m_pIndices = g_ScratchIndexBuffer;
		desc.m_nIndexSize = 0;
	}
}

void CTempMeshOES2::SetVertexFormat(VertexFormat_t fmt)
{
	CBaseMeshOES2::SetVertexFormat(fmt);
	m_VertexSize = CVertexBufferOES2::VertexFormatSize(fmt);
}

void CTempMeshOES2::UnlockMesh(int nVertexCount, int nIndexCount, MeshDesc_t &desc)
{
	int toRemove = m_LockedVerts - nVertexCount;
	if (toRemove)
		m_VertexData.RemoveMultiple(m_VertexData.Count() - toRemove, toRemove);
	toRemove = m_LockedIndices - nIndexCount;
	if (toRemove)
		m_IndexData.RemoveMultiple(m_IndexData.Count() - toRemove, toRemove);
}

//------------------------------------------------------------------------------
// Selection mode.
//------------------------------------------------------------------------------
static int g_NumClipVerts;
static Vector g_ClipVerts[16];

static bool PointInsidePlane(Vector &vert, int normalInd, float val, bool nearClip)
{
	if (nearClip || (val > 0.0f))
		return (val - vert[normalInd]) >= 0.0f;
	return (vert[normalInd] - val) >= 0.0f;
}

static void IntersectPlane(Vector &start, Vector &end,
	int normalInd, float val, Vector &outVert)
{
	Vector dir = end - start;
	Assert(dir[normalInd] != 0.0f);
	outVert = start + dir * ((val - start[normalInd]) / dir[normalInd]);
	outVert[normalInd] = val; // Avoid any precision problems.
}

static int ClipTriangleAgainstPlane(Vector **ppVert, int vertexCount,
	Vector **ppOutVert, int normalInd, float val, bool nearClip = false)
{
	// Ye Olde Sutherland-Hodgman clipping algorithm.
	int numOutVerts = 0;
	Vector &start = *(ppVert[vertexCount - 1]);
	bool startInside = PointInsidePlane(start, normalInd, val, nearClip);
	int i;
	for (i = 0; i < vertexCount; ++i)
	{
		Vector &end = *(ppVert[i]);
		bool endInside = PointInsidePlane(end, normalInd, val, nearClip);
		if (endInside)
		{
			if (!startInside)
			{
				Vector &clipVert = g_ClipVerts[g_NumClipVerts++];
				IntersectPlane(start, end, normalInd, val, clipVert);
				ppOutVert[numOutVerts++] = &clipVert;
			}
			ppOutVert[numOutVerts++] = &end;
		}
		else if (startInside)
		{
			Vector &clipVert = g_ClipVerts[g_NumClipVerts++];
			IntersectPlane(start, end, normalInd, val, clipVert);
			ppOutVert[numOutVerts++] = &clipVert;
		}
		start = end;
		startInside = endInside;
	}
	return numOutVerts;
}

void CTempMeshOES2::ClipTriangle(Vector **ppVert, float zNear, VMatrix &projection)
{
	int i;
	int vertexCount = 3;
	Vector *ppClipVert1[10], *ppClipVert2[10];

	g_NumClipVerts = 0;

	// Clip against the near plane in view space to prevent negative w; clip against each plane.
	vertexCount = ClipTriangleAgainstPlane(ppVert, vertexCount, ppClipVert1, 2, zNear, true);
	if (vertexCount < 3)
		return;

	// Sucks that this has to be done, but near plane has to be clipped in view space.
	// Clipping in projection space is screwy when w < 0.
	// Transform the clipped points into projection space.
	Assert(g_NumClipVerts <3);
	for (i = 0; i < vertexCount; ++i)
	{
		if (ppClipVert1[i] == &(g_ClipVerts[0]))
		{
			Vector3DMultiplyPositionProjective(projection, *(ppClipVert1[i]), g_ClipVerts[0]);
		}
		else if (ppClipVert1[i] == &(g_ClipVerts[1]))
		{
			Vector3DMultiplyPositionProjective(projection, *(ppClipVert1[i]), g_ClipVerts[1]);
		}
		else
		{
			Vector3DMultiplyPositionProjective(projection, *(ppClipVert1[i]), g_ClipVerts[g_NumClipVerts]);
			ppClipVert1[i] = &(g_ClipVerts[g_NumClipVerts++]);
		}
	}

	vertexCount = ClipTriangleAgainstPlane(ppClipVert1, vertexCount, ppClipVert2, 2, 1.0f);
	if (vertexCount < 3)
		return;
	vertexCount = ClipTriangleAgainstPlane(ppClipVert2, vertexCount, ppClipVert1, 0, 1.0f);
	if (vertexCount < 3)
		return;
	vertexCount = ClipTriangleAgainstPlane(ppClipVert1, vertexCount, ppClipVert2, 0, -1.0f);
	if (vertexCount < 3)
		return;
	vertexCount = ClipTriangleAgainstPlane(ppClipVert2, vertexCount, ppClipVert1, 1, 1.0f);
	if (vertexCount < 3)
		return;
	vertexCount = ClipTriangleAgainstPlane(ppClipVert1, vertexCount, ppClipVert2, 1, -1.0f);
	if (vertexCount < 3)
		return;

	float minz = ppClipVert2[0]->z;
	float maxz = ppClipVert2[0]->z;
	for (i = 1; i < vertexCount; ++i)
	{
		if (ppClipVert2[i]->z < minz)
			minz = ppClipVert2[i]->z;
		else if (ppClipVert2[i]->z > maxz)
			maxz = ppClipVert2[i]->z;
	}
	ShaderAPI()->RegisterSelectionHit(minz, maxz);
}

void CTempMeshOES2::TestSelection(void)
{
	// Note that this doesn't take into account any vertex modification
	// done in a vertex shader. Also it doesn't take into account any clipping
	// done in hardware.

	// Blow off points and lines; they don't matter.
	if ((m_Type != MATERIAL_TRIANGLES) && (m_Type != MATERIAL_TRIANGLE_STRIP))
		return;

	VMatrix model, view, modelToView, projection;
	ShaderAPI()->GetMatrix(MATERIAL_MODEL, model);
	ShaderAPI()->GetMatrix(MATERIAL_VIEW, view);
	MatrixMultiply(model, view, modelToView);
	ShaderAPI()->GetMatrix(MATERIAL_PROJECTION, projection);
	float zNear = -(projection.m[3][2]) / projection.m[2][2];

	Vector *pPos[3];
	Vector normal;

	int numTriangles;
	if (m_Type == MATERIAL_TRIANGLES)
		numTriangles = m_IndexData.Count() / 3;
	else
		numTriangles = m_IndexData.Count() - 2;

	float cullFactor;
	switch (ShaderAPI()->GetFrontFace())
	{
	case 0:
		cullFactor = 0.0f;
		break;
	case GL_CW:
		cullFactor = -1.0f;
		break;
	case GL_CCW:
		cullFactor = 1.0f;
		break;
	NO_DEFAULT
	}

	// Makes the lovely loop simpler.
	if (m_Type == MATERIAL_TRIANGLE_STRIP)
		cullFactor *= -1.0f;

	// We'll need some temporary memory to tell us if we've transformed the vert.
	int vertexCount = m_VertexData.Count() / m_VertexSize;
	static CUtlVector<unsigned char> transformedVert;
	int transformedVertSize = (vertexCount + 7) >> 3;
	transformedVert.RemoveAll();
	transformedVert.EnsureCapacity(transformedVertSize);
	transformedVert.AddMultipleToTail(transformedVertSize);
	memset(transformedVert.Base(), 0, transformedVertSize);

	int i, indexPos;
	for (i = 0; i < numTriangles; ++i)
	{
		// Get the three indices
		if (m_Type == MATERIAL_TRIANGLES)
		{
			indexPos = i * 3;
		}
		else
		{
			Assert(m_Type == MATERIAL_TRIANGLE_STRIP);
			cullFactor *= -1.0f;
			indexPos = i;
		}

		// BAH. Gotta clip to the near clip plane in view space to prevent
		// negative w coords; negative coords throw off the projection-space clipper.

		// Get the three positions in view space.
		int j, inFrontIdx = -1;
		for (j = 0; j < 3; ++j)
		{
			int index = m_IndexData[indexPos];
			Vector *pPosition = (Vector *)(&(m_VertexData[index * m_VertexSize]));
			if ((transformedVert[index >> 3] & (1 << (index & 0x7))) == 0)
			{
				Vector3DMultiplyPositionProjective(modelToView, *pPosition, *pPosition);
				transformedVert[index >> 3] |= (1 << (index & 0x7));
			}

			pPos[j] = pPosition;
			if (pPos[j]->z < 0.0f)
				inFrontIdx = j;
			++indexPos;
		}

		// All points are behind the camera.
		if (inFrontIdx < 0)
			continue;

		// Backface cull.
		CrossProduct(*(pPos[1]) - *(pPos[0]), *(pPos[2]) - *(pPos[0]), normal);
		if ((DotProduct(normal, *(pPos[inFrontIdx])) * cullFactor) > 0.0f)
			continue;

		// Clip to viewport.
		ClipTriangle(pPos, zNear, projection);
	}
}