//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: VertexLitGeneric shader.
//
//===========================================================================//
#include "BaseOES2Shader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define CONST_VERTEXCONTROLS 0
#define CONST_BASETEXCOORDTRANSFORM 1
#define CONST_DETAILTEXCOORDTRANSFORM 2
#define CONST_EYEPOS 3

#define CONST_PIXELFOG 4
#define CONST_PIXELCONTROLS 5
#define CONST_DIFFUSEMODULATION 6
#define CONST_SELFILLUMTINT 7
#define CONST_ENVMAPTINT 8
#define CONST_ENVMAPSATURATION 9

BEGIN_OES_SHADER(VertexLitGeneric, "Help for VertexLitGeneric")
	BEGIN_SHADER_PARAMS
		SHADER_PARAM(ALBEDO, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "albedo (Base texture with no baked lighting)")

		SHADER_PARAM(SELFILLUMTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint")
		SHADER_PARAM(SELFILLUM_ENVMAPMASK_ALPHA, SHADER_PARAM_TYPE_FLOAT, "0.0", "defines that self illum value comes from env map mask alpha")
		SHADER_PARAM(SELFILLUMMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "If we bind a texture here, it overrides base alpha (if any) for self illum")

		SHADER_PARAM(DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture")
		SHADER_PARAM(DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail")
		SHADER_PARAM(DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture")
		SHADER_PARAM(DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "1", "blend amount for detail texture.")
		SHADER_PARAM(DETAILTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "detail texture tint")

		SHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap")
		SHADER_PARAM(ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "envmap frame number")
		SHADER_PARAM(ENVMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "envmap mask")
		SHADER_PARAM(ENVMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "")
		SHADER_PARAM(ENVMAPMASKTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$envmapmask texcoord transform")
		SHADER_PARAM(ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint")
		SHADER_PARAM(ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal")

		SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.7", "")

		SHADER_PARAM(SEPARATEDETAILUVS, SHADER_PARAM_TYPE_BOOL, "0", "Use texcoord1 for detail texture")
	END_SHADER_PARAMS

	// D0GTODO: VertexLitGeneric bump.

	SHADER_INIT_PARAMS()
	{
		if (!params[ALPHATESTREFERENCE]->IsDefined())
			params[ALPHATESTREFERENCE]->SetFloatValue(0.7f);
		if (!params[DETAILBLENDFACTOR]->IsDefined())
			params[DETAILBLENDFACTOR]->SetFloatValue(1.0f);
		if (!params[DETAILSCALE]->IsDefined())
			params[DETAILSCALE]->SetFloatValue(4.0f);
		// D0GTODO: Albedo->basetexture shen bumpmapping is active.
		SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
		SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);
		if (!params[BASETEXTURE]->IsDefined())
		{
			CLEAR_FLAGS(MATERIAL_VAR_SELFILLUM);
			CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
		}
		// D0GTODO: $normalmapalphaenvmapmask.
		if (!g_pConfig->UseSpecular() && params[ENVMAP]->IsDefined() && params[BASETEXTURE]->IsDefined())
			params[ENVMAP]->SetUndefined();
	}

	SHADER_INIT
	{
		bool selfIllumMask = IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) && params[SELFILLUMMASK]->IsDefined();
		if (params[BASETEXTURE]->IsDefined())
		{
			LoadTexture(BASETEXTURE);
			if (!params[BASETEXTURE]->GetTextureValue()->IsTranslucent())
			{
				if (!selfIllumMask)
					CLEAR_FLAGS(MATERIAL_VAR_SELFILLUM);
				CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
			}
		}
		if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK))
			CLEAR_FLAGS(MATERIAL_VAR_ALPHATEST);
		if (params[DETAIL]->IsDefined())
			LoadTexture(DETAIL);
		if (params[ENVMAP]->IsDefined())
			LoadCubeMap(ENVMAP);
		if (params[ENVMAPMASK]->IsDefined())
			LoadTexture(ENVMAPMASK);
		if (selfIllumMask && !IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK))
			LoadTexture(SELFILLUMMASK);
	}

	SHADER_DRAW
	{
		bool detail = params[DETAIL]->IsTexture();
		bool envMap = params[ENVMAP]->IsTexture();
		bool envMapMask = params[ENVMAPMASK]->IsTexture();
		bool selfIllumInEnvMapMask = IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) && envMapMask && params[SELFILLUM_ENVMAPMASK_ALPHA]->GetFloatValue();
		bool selfIllumMask = !selfIllumInEnvMapMask && params[SELFILLUMMASK]->IsTexture();

		SHADOW_STATE
		{
			bool isAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST);
			if (isAlphaTested)
				pShaderShadow->EnableAlphaTest(true);
			SetBlendingShadowState(params[BASETEXTURE]->IsTexture() ?
				EvaluateBlendRequirements(BASETEXTURE, true) : EvaluateBlendRequirements(ENVMAPMASK, false));

			// 0 DETAILTEXTURE
				// 1 SEPARATE_DETAIL_UVS
			// 2 SELFILLUM
				// 3 SELFILLUM_ENVMAPMASK_ALPHA
				// 4 SELFILLUMMASK
			// 5 CUBEMAP
				// 6 BASEALPHAENVMAPMASK
				// 7 ENVMAPMASK
			// 8 ALPHATEST
			// 9 NOFOG

			int combo = 0;
			if (detail)
			{
				combo |= 1;
				if (params[SEPARATEDETAILUVS]->GetIntValue())
					combo |= 1 << 1;
			}
			if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM))
			{
				combo |= 1 << 2;
				if (selfIllumInEnvMapMask)
					combo |= 1 << 3;
				else if (selfIllumMask)
					combo |= 1 << 4;
			}
			if (envMap)
			{
				combo |= 1 << 5;
				if (IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK))
					combo |= 1 << 6;
				else if (envMapMask)
					combo |= 1 << 7;
			}
			if (isAlphaTested)
				combo |= 1 << 8;
			if (IS_FLAG_SET(MATERIAL_VAR_NOFOG))
				combo |= 1 << 9;

			SET_PROGRAM(VertexLitGeneric, combo)
				SetProgramVertexShaderLight(creationData);
				SET_PROGRAM_SKINNING
				if (detail)
				{
					SET_PROGRAM_STATIC("DETAILTEXTURE")
					SET_PROGRAM_SAMPLER(1, Detail)
					if (params[SEPARATEDETAILUVS]->GetIntValue())
					{
						SET_PROGRAM_STATIC("SEPARATE_DETAIL_UVS")
						SET_PROGRAM_TEXCOORD(1, DetailTexCoord, 2)
					}
					else
					{
						SET_PROGRAM_CONSTANT(CONST_DETAILTEXCOORDTRANSFORM, DetailTexCoordTransform)
					}
				}
				if (selfIllumInEnvMapMask || (envMap && envMapMask))
					SET_PROGRAM_SAMPLER(2, EnvmapMask)
				if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM))
				{
					SET_PROGRAM_PS_STATIC("SELFILLUM")
					SET_PROGRAM_CONSTANT(CONST_SELFILLUMTINT, SelfIllumTint)
					if (selfIllumInEnvMapMask)
					{
						SET_PROGRAM_PS_STATIC("SELFILLUM_ENVMAPMASK_ALPHA")
					}
					else if (selfIllumMask)
					{
						SET_PROGRAM_PS_STATIC("SELFILLUMMASK")
						SET_PROGRAM_SAMPLER(3, SelfIllumMask)
					}
				}
				if (envMap)
				{
					SET_PROGRAM_STATIC("CUBEMAP")
					SET_PROGRAM_CONSTANT(CONST_EYEPOS, EyePos)
					SET_PROGRAM_CONSTANT(CONST_ENVMAPTINT, EnvmapTint)
					SET_PROGRAM_CONSTANT(CONST_ENVMAPSATURATION, EnvmapSaturation)
					SET_PROGRAM_SAMPLER(4, Envmap)
					if (IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK))
						SET_PROGRAM_PS_STATIC("BASEALPHAENVMAPMASK")
					else if (envMapMask)
						SET_PROGRAM_PS_STATIC("ENVMAPMASK")
				}
				if (isAlphaTested)
					SET_PROGRAM_PS_STATIC("ALPHATEST")
				SET_PROGRAM_PIXEL_FOG(CONST_PIXELFOG)
				SET_PROGRAM_VS_INCLUDE(vertexlitgeneric_v) // Include because of lots of combos.
				SET_PROGRAM_PS_INCLUDE(vertexlitgeneric_p)
				SET_PROGRAM_CONSTANT(CONST_VERTEXCONTROLS, VertexControls)
				SET_PROGRAM_CONSTANT(CONST_BASETEXCOORDTRANSFORM, BaseTexCoordTransform)
				SET_PROGRAM_CONSTANT(CONST_PIXELCONTROLS, PixelControls)
				SET_PROGRAM_CONSTANT(CONST_DIFFUSEMODULATION, DiffuseModulation)
				SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_VIEWPROJ)
				SET_PROGRAM_SAMPLER(0, Base)
				SET_PROGRAM_INPUT(OES2_SHADER_INPUT_POSITION, Pos)
				SET_PROGRAM_INPUT(OES2_SHADER_INPUT_NORMAL, Normal)
				SET_PROGRAM_INPUT(OES2_SHADER_INPUT_SPECULAR, Specular)
				SET_PROGRAM_TEXCOORD(0, BaseTexCoord, 2)
			END_SET_PROGRAM

			DefaultFog();
		}

		DYNAMIC_STATE
		{
			if (params[BASETEXTURE]->IsTexture())
				BindTexture(SHADER_SAMPLER0, BASETEXTURE, FRAME);
			else
				pShaderAPI->BindStandardTexture(SHADER_SAMPLER0, envMap ? TEXTURE_BLACK : TEXTURE_WHITE);
			SetTextureTransform(CONST_BASETEXCOORDTRANSFORM, BASETEXTURETRANSFORM);
			if (detail)
			{
				BindTexture(SHADER_SAMPLER1, DETAIL, DETAILFRAME);
				if (!params[SEPARATEDETAILUVS]->GetIntValue())
					SetTextureScaledTransform(CONST_DETAILTEXCOORDTRANSFORM, BASETEXTURETRANSFORM, DETAILSCALE);
			}
			if (selfIllumInEnvMapMask || (envMap && envMapMask))
				BindTexture(SHADER_SAMPLER2, ENVMAPMASK, ENVMAPMASKFRAME);
			if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM))
			{
				if (selfIllumMask)
					BindTexture(SHADER_SAMPLER3, SELFILLUMMASK, -1);
				pShaderAPI->Uniform3fv(CONST_SELFILLUMTINT, 1, params[SELFILLUMTINT]->GetVecValue());
			}
			float eyePos[3];
			pShaderAPI->GetWorldSpaceCameraPosition(eyePos);
			if (envMap)
			{
				BindTexture(SHADER_SAMPLER4, ENVMAP, ENVMAPFRAME);
				pShaderAPI->Uniform3fv(CONST_EYEPOS, 1, eyePos);
				SetEnvMapTintConstant(CONST_ENVMAPTINT, ENVMAPTINT);
				// D0GHDR: Multiply tint by 16 for HDR_TYPE_INTEGER.
				float envMapSaturation[3];
				params[ENVMAPSATURATION]->GetVecValue(envMapSaturation, 3);
				pShaderAPI->Uniform3fv(CONST_ENVMAPSATURATION, 1, envMapSaturation);
			}
			LightState_t lightState;
			pShaderAPI->GetDX9LightState(&lightState);
			pShaderAPI->Uniform3f(CONST_VERTEXCONTROLS,
				(pShaderAPI->GetCurrentNumBones() > 0) ? 1.0f : 0.0f,
				lightState.m_bStaticLight ? 1.0f : 0.0f,
				(float)(lightState.HasDynamicLight()) * (IS_FLAG_SET(MATERIAL_VAR_HALFLAMBERT) ? -1.0f : 1.0f));
			if (!IS_FLAG_SET(MATERIAL_VAR_NOFOG))
				pShaderAPI->SetPixelShaderFogParams(CONST_PIXELFOG);
			pShaderAPI->Uniform3f(CONST_PIXELCONTROLS,
				params[DETAILBLENDFACTOR]->GetFloatValue(),
				params[ALPHATESTREFERENCE]->GetFloatValue(),
				eyePos[2]);
			float color[4];
			ComputeModulationColor(color);
			pShaderAPI->Uniform4fv(CONST_DIFFUSEMODULATION, 1, color);
		}

		Draw();
	}
END_SHADER