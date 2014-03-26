//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: DepthWrite shader.
//
//===========================================================================//
#include "BaseOES2Shader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_OES_SHADER(DepthWrite, "Help for DepthWrite")
	BEGIN_SHADER_PARAMS
		SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "", "Alpha reference value")
	END_SHADER_PARAMS

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
		bool isAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST);

		SHADOW_STATE
		{
			pShaderShadow->EnablePolyOffset(SHADER_POLYOFFSET_SHADOW_BIAS);
			pShaderShadow->EnableColorWrites(false);
			pShaderShadow->EnableAlphaWrites(false);
			pShaderShadow->EnableCulling(isAlphaTested && !IS_FLAG_SET(MATERIAL_VAR_NOCULL));
			SET_PROGRAM(DepthWrite, isAlphaTested ? 1 : 0)
				SET_PROGRAM_SKINNING
				if (isAlphaTested)
				{
					SET_PROGRAM_STATIC("ALPHATEST")
					SET_PROGRAM_CONSTANT(1, AlphaThreshold)
					SET_PROGRAM_SAMPLER(0, Base)
					SET_PROGRAM_TEXCOORD(0, BaseTexCoord, 2)
				}
				SET_PROGRAM_VS_SOURCE(depthwrite_v)
				SET_PROGRAM_PS_SOURCE(depthwrite_p)
				SET_PROGRAM_CONSTANT(0, Skinning)
				SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_VIEWPROJ)
				SET_PROGRAM_INPUT(OES2_SHADER_INPUT_POSITION, Pos)
			END_SET_PROGRAM
		}

		DYNAMIC_STATE
		{
			pShaderAPI->Uniform1f(0, (pShaderAPI->GetCurrentNumBones() > 0) ? 1.0f : 0.0f);
			if (isAlphaTested)
			{
				BindTexture(SHADER_SAMPLER0, BASETEXTURE, FRAME);
				float alphaThreshold = params[ALPHATESTREFERENCE]->GetFloatValue();
				pShaderAPI->Uniform1f(1, (alphaThreshold > 0.0f) ? alphaThreshold : 0.7f);
			}
		}

		Draw();
	}
END_SHADER