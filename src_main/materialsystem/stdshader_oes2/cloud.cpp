//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: Cloud shader.
//
//===========================================================================//
#include "BaseOES2Shader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_OES_SHADER(Cloud, "Help for Cloud")
	BEGIN_SHADER_PARAMS
		SHADER_PARAM_OVERRIDE(BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/cloud", "cloud texture", 0)
		SHADER_PARAM(CLOUDALPHATEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/cloudalpha", "cloud alpha texture")
		SHADER_PARAM(CLOUDSCALE, SHADER_PARAM_TYPE_VEC2, "[1 1]", "cloudscale")
		SHADER_PARAM(MASKSCALE, SHADER_PARAM_TYPE_VEC2, "[1 1]", "maskscale")
	END_SHADER_PARAMS

	SHADER_INIT
	{
		LoadTexture(BASETEXTURE);
		LoadTexture(CLOUDALPHATEXTURE);
		if (!params[CLOUDSCALE]->IsDefined())
			params[CLOUDSCALE]->SetVecValue(1.0f, 1.0f);
		if (!params[MASKSCALE]->IsDefined())
			params[MASKSCALE]->SetVecValue(1.0f, 1.0f);
	}

	SHADER_DRAW
	{
		bool noFog = IS_FLAG_SET(MATERIAL_VAR_NOFOG);
		SHADOW_STATE
		{
			pShaderShadow->EnableDepthWrites(false);
			pShaderShadow->EnableBlending(true);
			if (IS_FLAG_SET(MATERIAL_VAR_ADDITIVE))
				pShaderShadow->BlendFunc(SHADER_BLEND_ONE, SHADER_BLEND_ONE);
			else
				pShaderShadow->BlendFunc(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
			SET_PROGRAM(Cloud, noFog ? 1 : 0)
				SET_PROGRAM_VS_SOURCE(cloud_v)
				SET_PROGRAM_PIXEL_FOG(0)
				SET_PROGRAM_PS_SOURCE(cloud_p)
				SET_PROGRAM_CONSTANT(1, BaseTextureTransform)
				SET_PROGRAM_CONSTANT(2, MaskTransform)
				if (!noFog)
					SET_PROGRAM_CONSTANT(3, EyePosZ)
				SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_VIEWPROJ)
				SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_MODEL)
				SET_PROGRAM_SAMPLER(0, Base)
				SET_PROGRAM_SAMPLER(1, Alpha)
				SET_PROGRAM_INPUT(OES2_SHADER_INPUT_POSITION, Pos)
				SET_PROGRAM_TEXCOORD(0, TexCoord0, 2)
				SET_PROGRAM_TEXCOORD(1, TexCoord1, 2)
			END_SET_PROGRAM
			DefaultFog();
		}
		DYNAMIC_STATE
		{
			BindTexture(SHADER_SAMPLER0, BASETEXTURE, FRAME);
			BindTexture(SHADER_SAMPLER1, CLOUDALPHATEXTURE);
			if (!noFog)
			{
				pShaderAPI->SetPixelShaderFogParams(0);
				pShaderAPI->Uniform1f(3, GetEyeZ());
			}
			SetTextureScaledTransform(1, BASETEXTURETRANSFORM, CLOUDSCALE);
			SetTextureScale(2, MASKSCALE);
		}
		Draw();
	}
END_SHADER