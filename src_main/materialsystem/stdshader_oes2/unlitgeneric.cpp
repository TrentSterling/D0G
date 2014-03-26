//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: UnlitGeneric shader.
//
//===========================================================================//
#include "BaseOES2Shader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define CONST_SKINNING 0
#define CONST_BASETEXCOORDTRANSFORM 1
#define CONST_DETAILTEXCOORDTRANSFORM 2
#define CONST_PIXELFOG 3
#define CONST_PIXELCONTROLS 4

BEGIN_OES_SHADER(UnlitGeneric, "Help for UnlitGeneric")
	BEGIN_SHADER_PARAMS
		SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.7", "")

		SHADER_PARAM(DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture")
		SHADER_PARAM(DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail")
		SHADER_PARAM(DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture")
		SHADER_PARAM(DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "1", "blend amount for detail texture.")

		SHADER_PARAM(SEPARATEDETAILUVS, SHADER_PARAM_TYPE_BOOL, "0", "Use texcoord1 for detail texture")
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		if (!params[ALPHATESTREFERENCE]->IsDefined())
			params[ALPHATESTREFERENCE]->SetFloatValue(0.7f);
		if (!params[DETAILBLENDFACTOR]->IsDefined())
			params[DETAILBLENDFACTOR]->SetFloatValue(1.0f);
		if (!params[DETAILSCALE]->IsDefined())
			params[DETAILSCALE]->SetFloatValue(4.0f);
		SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
	}

	SHADER_INIT
	{
		if (params[BASETEXTURE]->IsDefined())
			LoadTexture(BASETEXTURE);
		if (params[DETAIL]->IsDefined())
			LoadTexture(DETAIL);
	}

	SHADER_DRAW
	{
		bool detail = params[DETAIL]->IsDefined();

		SHADOW_STATE
		{
			bool isAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST);
			if (isAlphaTested)
				pShaderShadow->EnableAlphaTest(true);
			SetBlendingShadowState(EvaluateBlendRequirements(BASETEXTURE, true));

			bool vertexColor = IS_FLAG_SET(MATERIAL_VAR_VERTEXCOLOR) || IS_FLAG_SET(MATERIAL_VAR_VERTEXALPHA);

			// 0 DETAILTEXTURE
				// 1 SEPARATE_DETAIL_UVS
			// 2 VERTEXCOLOR
			// 3 NOFOG
			// 4 ALPHATEST			

			int combo = 0;
			if (detail)
			{
				combo |= 1;
				if (params[SEPARATEDETAILUVS]->GetIntValue())
					combo |= 1 << 1;
			}
			if (vertexColor)
				combo |= 1 << 2;
			if (IS_FLAG_SET(MATERIAL_VAR_NOFOG))
				combo |= 1 << 3;
			if (isAlphaTested)
				combo |= 1 << 4;

			SET_PROGRAM(UnlitGeneric, combo)
				SET_PROGRAM_SKINNING
				if (detail)
				{
					SET_PROGRAM_STATIC("DETAILTEXTURE")
					if (params[SEPARATEDETAILUVS]->GetIntValue())
					{
						SET_PROGRAM_STATIC("SEPARATE_DETAIL_UVS")
						SET_PROGRAM_TEXCOORD(1, DetailTexCoord, 2)
					}
					SET_PROGRAM_CONSTANT(CONST_DETAILTEXCOORDTRANSFORM, DetailTexCoordTransform)
					SET_PROGRAM_SAMPLER(1, Detail)
				}
				if (vertexColor)
				{
					SET_PROGRAM_STATIC("VERTEXCOLOR")
					SET_PROGRAM_INPUT(OES2_SHADER_INPUT_COLOR, Color)
				}
				SET_PROGRAM_PIXEL_FOG(CONST_PIXELFOG)
				if (isAlphaTested)
					SET_PROGRAM_PS_STATIC("ALPHATEST")
				SET_PROGRAM_VS_SOURCE(unlitgeneric_v)
				SET_PROGRAM_PS_SOURCE(unlitgeneric_p)
				SET_PROGRAM_CONSTANT(CONST_SKINNING, Skinning)
				SET_PROGRAM_CONSTANT(CONST_BASETEXCOORDTRANSFORM, BaseTexCoordTransform)
				SET_PROGRAM_CONSTANT(CONST_PIXELCONTROLS, PixelControls)
				SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_VIEWPROJ)
				SET_PROGRAM_SAMPLER(0, Base)
				SET_PROGRAM_INPUT(OES2_SHADER_INPUT_POSITION, Pos)
				SET_PROGRAM_TEXCOORD(0, BaseTexCoord, 2)
			END_SET_PROGRAM

			DefaultFog();
		}

		DYNAMIC_STATE
		{
			if (params[BASETEXTURE]->IsDefined())
				BindTexture(SHADER_SAMPLER0, BASETEXTURE, FRAME);
			else
				pShaderAPI->BindStandardTexture(SHADER_SAMPLER0, TEXTURE_WHITE);
			if (!IS_FLAG_SET(MATERIAL_VAR_NOFOG))
				pShaderAPI->SetPixelShaderFogParams(CONST_PIXELFOG);
			pShaderAPI->Uniform1f(CONST_SKINNING, (pShaderAPI->GetCurrentNumBones() > 0) ? 1.0f : 0.0f);
			SetTextureTransform(CONST_BASETEXCOORDTRANSFORM, BASETEXTURETRANSFORM);
			pShaderAPI->Uniform4f(CONST_PIXELCONTROLS,
				params[DETAILBLENDFACTOR]->GetFloatValue(),
				IS_FLAG_SET(MATERIAL_VAR_VERTEXALPHA) ? 1.0f : 0.0f,
				params[ALPHATESTREFERENCE]->GetFloatValue(),
				GetEyeZ());
			if (detail)
			{
				BindTexture(SHADER_SAMPLER1, DETAIL, DETAILFRAME);
				SetTextureScaledTransform(CONST_DETAILTEXCOORDTRANSFORM, BASETEXTURETRANSFORM, DETAILSCALE);
			}
		}

		Draw();
	}
END_SHADER