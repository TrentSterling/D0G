//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: This is what all OpenGL ES 2.0 shaders inherit from.
//
//===========================================================================//
#include "BaseOES2Shader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar mat_fullbright("mat_fullbright", "0", FCVAR_CHEAT);

BlendType_t CBaseOES2Shader::EvaluateBlendRequirements(int textureVar, bool isBaseTexture, int detailTextureVar)
{
	IMaterialVar **params = s_ppParams;
	bool isTranslucent = IsAlphaModulating() ||
		(CurrentMaterialVarFlags() & MATERIAL_VAR_VERTEXALPHA) ||
		(!(CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST) && TextureIsTranslucent(textureVar, isBaseTexture)) ||
		((detailTextureVar != -1) && TextureIsTranslucent(detailTextureVar, isBaseTexture));
	if (CurrentMaterialVarFlags() & MATERIAL_VAR_ADDITIVE)
		return isTranslucent ? BT_BLENDADD : BT_ADD;
	return isTranslucent ? BT_BLEND : BT_NONE;
}

void CBaseOES2Shader::SetProgramVertexShaderLight(OES2ShaderCreationData_t &creationData)
{
	SET_PROGRAM_VS_INCLUDE(common_light_v)
	SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_AMBIENT_LIGHT);
	SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_LIGHTS_COLOR);
	SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_LIGHTS_DIRECTION);
	SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_LIGHTS_POSITION);
	SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_LIGHTS_SPOT);
	SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_LIGHTS_ATTENUATION);
	SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_LIGHTS_POSITION);
	SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_LIGHTS_ENABLE);
}

void CBaseOES2Shader::SetTextureScale(int reg, int scaleVar)
{
	float scale[2] = {1.0f, 1.0f};
	IMaterialVar *pScaleVar = s_ppParams[scaleVar];
	if (pScaleVar)
	{
		if (pScaleVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
			pScaleVar->GetVecValue(scale, 2);
		else if (pScaleVar->IsDefined())
			scale[0] = scale[1] = pScaleVar->GetFloatValue();
	}
	Vector4D scaleMatrix[2];
	scaleMatrix[0].Init(scale[0], 0.0f, 0.0f, 0.0f);
	scaleMatrix[1].Init(0.0f, scale[1], 0.0f, 0.0f);
	s_pShaderAPI->Uniform4fv(reg, 2, scaleMatrix[0].Base()); 
}

void CBaseOES2Shader::SetTextureScaledTransform(int reg, int transformVar, int scaleVar)
{
	Vector4D transformation[2];
	IMaterialVar *pTransformationVar = s_ppParams[transformVar];
	if (pTransformationVar && (pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX))
	{
		const VMatrix &mat = pTransformationVar->GetMatrixValue();
		transformation[0].Init(mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
		transformation[1].Init(mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
	}
	else
	{
		transformation[0].Init(1.0f, 0.0f, 0.0f, 0.0f);
		transformation[1].Init(0.0f, 1.0f, 0.0f, 0.0f);
	}
	float scale[2] = {1.0f, 1.0f};
	IMaterialVar *pScaleVar = s_ppParams[scaleVar];
	if (pScaleVar)
	{
		if (pScaleVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
			pScaleVar->GetVecValue(scale, 2);
		else if (pScaleVar->IsDefined())
			scale[0] = scale[1] = pScaleVar->GetFloatValue();
	}
	transformation[0][0] *= scale[0];
	transformation[0][1] *= scale[1];
	transformation[1][0] *= scale[0];
	transformation[1][1] *= scale[1];
	transformation[0][3] *= scale[0];
	transformation[1][3] *= scale[1];
	s_pShaderAPI->Uniform4fv(reg, 2, transformation[0].Base());
}

void CBaseOES2Shader::SetTextureTransform(int reg, int transformVar)
{
	Vector4D transformation[2];
	IMaterialVar *pTransformationVar = s_ppParams[transformVar];
	if (pTransformationVar && (pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX))
	{
		const VMatrix &mat = pTransformationVar->GetMatrixValue();
		transformation[0].Init(mat[0][0], mat[0][1], mat[0][2], mat[0][3]);
		transformation[1].Init(mat[1][0], mat[1][1], mat[1][2], mat[1][3]);
	}
	else
	{
		transformation[0].Init(1.0f, 0.0f, 0.0f, 0.0f);
		transformation[1].Init(0.0f, 1.0f, 0.0f, 0.0f);
	}
	s_pShaderAPI->Uniform4fv(reg, 2, transformation[0].Base());
}