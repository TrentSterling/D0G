//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: AccumBuff5Sample shader.
//
//===========================================================================//
#include "BaseOES2Shader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_OES_SHADER_FLAGS(AccumBuff5Sample, "Help for AccumBuff5Sample", SHADER_NOT_EDITABLE)
	BEGIN_SHADER_PARAMS
		SHADER_PARAM(TEXTURE0, SHADER_PARAM_TYPE_TEXTURE, "", "")
		SHADER_PARAM(TEXTURE1, SHADER_PARAM_TYPE_TEXTURE, "", "")
		SHADER_PARAM(TEXTURE2, SHADER_PARAM_TYPE_TEXTURE, "", "")
		SHADER_PARAM(TEXTURE3, SHADER_PARAM_TYPE_TEXTURE, "", "")
		SHADER_PARAM(TEXTURE4, SHADER_PARAM_TYPE_TEXTURE, "", "")
		SHADER_PARAM(WEIGHTS, SHADER_PARAM_TYPE_VEC4, "", "Weight for Samples")
	END_SHADER_PARAMS

	SHADER_INIT
	{
		LoadTexture(TEXTURE0);
		LoadTexture(TEXTURE1);
		LoadTexture(TEXTURE2);
		LoadTexture(TEXTURE3);
		LoadTexture(TEXTURE4);
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableDepthWrites(false);
			pShaderShadow->EnableDepthTest(false);
			pShaderShadow->EnableAlphaWrites(false);
			pShaderShadow->EnableBlending(false);
			pShaderShadow->EnableCulling(false);
			SET_PROGRAM(AccumBuff5Sample, 0)
				SET_PROGRAM_SCREENSPACEEFFECT
				SET_PROGRAM_PS_SOURCE(accumbuff5sample_p)
				SET_PROGRAM_CONSTANT(0, Weights)
				SET_PROGRAM_SAMPLER(0, Tex0)
				SET_PROGRAM_SAMPLER(1, Tex1)
				SET_PROGRAM_SAMPLER(2, Tex2)
				SET_PROGRAM_SAMPLER(3, Tex3)
				SET_PROGRAM_SAMPLER(4, Tex4)
			END_SET_PROGRAM
		}
		DYNAMIC_STATE
		{
			BindTexture(SHADER_SAMPLER0, TEXTURE0, -1);
			BindTexture(SHADER_SAMPLER1, TEXTURE1, -1);
			BindTexture(SHADER_SAMPLER2, TEXTURE2, -1);
			BindTexture(SHADER_SAMPLER3, TEXTURE3, -1);
			BindTexture(SHADER_SAMPLER4, TEXTURE4, -1);
			float weights[2];
			params[WEIGHTS]->GetVecValue(weights, 2);
			pShaderAPI->Uniform2fv(0, 1, weights);
		}
		Draw();
	}
END_SHADER