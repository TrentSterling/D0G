//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: Cable shader.
//
//===========================================================================//
#include "BaseOES2Shader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_OES_SHADER(Cable, "Help for Cable shader")
	BEGIN_SHADER_PARAMS
		SHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "cable/cablenormalmap", "bumpmap texture")
	END_SHADER_PARAMS

	SHADER_INIT
	{
		LoadTexture(BUMPMAP);
		LoadTexture(BASETEXTURE);
	}

	SHADER_DRAW
	{
		bool noFog = IS_FLAG_SET(MATERIAL_VAR_NOFOG);

		SHADOW_STATE
		{
			if (IS_FLAG_SET(MATERIAL_VAR_TRANSLUCENT))
			{
				pShaderShadow->EnableDepthWrites(false);
				pShaderShadow->EnableBlending(true);
				pShaderShadow->BlendFunc(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
			}
			bool isAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST);
			pShaderShadow->EnableAlphaTest(isAlphaTested);

			int combo = 0;
			if (noFog)
				combo |= 1;
			if (isAlphaTested)
				combo |= 2;
			SET_PROGRAM(Cable, combo)
				SET_PROGRAM_VS_SOURCE(cable_v)
				SET_PROGRAM_PIXEL_FOG(0)
				if (isAlphaTested)
					SET_PROGRAM_PS_STATIC("ALPHATEST")
				SET_PROGRAM_PS_SOURCE(cable_p)
				if (!noFog)
					SET_PROGRAM_CONSTANT(1, EyePosZ)
				SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_VIEWPROJ)
				SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_MODEL)
				SET_PROGRAM_SAMPLER(0, Normal)
				SET_PROGRAM_SAMPLER(1, Base)
				SET_PROGRAM_INPUT(OES2_SHADER_INPUT_POSITION, Pos)
				SET_PROGRAM_INPUT(OES2_SHADER_INPUT_COLOR, DirectionalLightColor)
				SET_PROGRAM_TEXCOORD(0, TexCoord0, 2)
				SET_PROGRAM_TEXCOORD(1, TexCoord1, 2)
			END_SET_PROGRAM
			FogToFogColor();
		}
		DYNAMIC_STATE
		{
			BindTexture(SHADER_SAMPLER0, BUMPMAP);
			if (mat_fullbright.GetInt() == 2 && !IS_FLAG_SET(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
				pShaderAPI->BindStandardTexture(SHADER_SAMPLER1, TEXTURE_GREY);
			else
				BindTexture(SHADER_SAMPLER1, BASETEXTURE);
			if (!noFog)
			{
				pShaderAPI->SetPixelShaderFogParams(0);
				pShaderAPI->Uniform1f(1, GetEyeZ());
			}
		}
		Draw();
	}
END_SHADER