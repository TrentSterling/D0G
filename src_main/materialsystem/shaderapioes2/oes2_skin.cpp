//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 skeletal animation skinning.
//
//===========================================================================//
#include <cstd/string.h>
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

void CShaderAPIOES2::LoadBoneMatrix(int boneIndex, const float *m)
{
	if (IsDeactivated())
		return;
	memcpy(m_BoneMatrix[boneIndex], m, sizeof(float) * 12);
	if (boneIndex == 0)
	{
		MatrixMode(MATERIAL_MODEL);
		VMatrix matrix;
		memcpy(matrix.Base(), m, sizeof(float) * 12);
		matrix[3][0] = matrix[3][1] = matrix[3][2] = 0.0f;
		matrix[3][3] = 1.0f;
		LoadMatrix(matrix);
	}
	if (boneIndex < m_DynamicState.m_NumBones)
		StandardConstantChanged(OES2_SHADER_CONST_MODEL);
}

void CShaderAPIOES2::SetNumBoneWeights(int numBones)
{
	int oldNumBones = m_DynamicState.m_NumBones;
	if (oldNumBones == numBones)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_NumBones = numBones;
	StandardConstantChanged(OES2_SHADER_CONST_MODEL);
	if ((!oldNumBones && numBones) || (oldNumBones && !numBones))
		MeshMgr()->MarkStreamStateDirty(); // Always use VertexAttribZero when there are 0 bones.
}

void CShaderAPIOES2::SetSkinningMatrices(void)
{
	if (m_DynamicState.m_NumBones && CommitStandardConstant(OES2_SHADER_CONST_MODEL))
		glUniform4fv(GetStandardConstLocation(OES2_SHADER_CONST_MODEL), m_DynamicState.m_NumBones * 3, m_BoneMatrix[0]);
}