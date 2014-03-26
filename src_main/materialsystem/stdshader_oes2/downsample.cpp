//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: Downsample shader.
//
//===========================================================================//
#include "BaseOES2Shader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_OES_SHADER_FLAGS(Downsample, "Help for Downsample", SHADER_NOT_EDITABLE)
	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	SHADER_INIT
	{
		LoadTexture(BASETEXTURE);
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableDepthWrites(false);
			pShaderShadow->EnableAlphaWrites(true);
			SET_PROGRAM(Downsample, 0)
				SET_PROGRAM_VS_SOURCE(downsample_v)
				SET_PROGRAM_PS_SOURCE(downsample_p)
				SET_PROGRAM_CONSTANT(0, TapOffs)
				SET_PROGRAM_SAMPLER(0, Tex)
				SET_PROGRAM_INPUT(OES2_SHADER_INPUT_POSITION, Pos)
				SET_PROGRAM_TEXCOORD(0, BaseTexCoord, 2)
			END_SET_PROGRAM
		}
		DYNAMIC_STATE
		{
			BindTexture(SHADER_SAMPLER0, BASETEXTURE, -1);
			int width, height;
			pShaderAPI->GetBackBufferDimensions(width, height);
			float dX = 1.0f / (float)width, dY = 1.0f / (float)height;
			float tapOffs[8] = {-dX, -dY, -dX, dY, dX, -dY, dX, dY};
			pShaderAPI->Uniform4fv(0, 1, tapOffs);
		}
		Draw();
	}
END_SHADER