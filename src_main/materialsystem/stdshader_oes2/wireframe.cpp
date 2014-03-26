//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: Wireframe shader.
//
//===========================================================================//
#include "BaseOES2Shader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER(Wireframe_DX6, Wireframe)

BEGIN_OES_SHADER(Wireframe, "Help for Wireframe")
	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			bool vertexColor = IS_FLAG_SET(MATERIAL_VAR_VERTEXCOLOR);
			SET_PROGRAM(Wireframe, vertexColor ? 1 : 0)
				SET_PROGRAM_SKINNING
				if (vertexColor)
				{
					SET_PROGRAM_STATIC("VERTEXCOLOR")
					SET_PROGRAM_INPUT(OES2_SHADER_INPUT_COLOR, Color)
				}
				SET_PROGRAM_VS_SOURCE(wireframe_v)
				SET_PROGRAM_PS_SOURCE(wireframe_p)
				SET_PROGRAM_CONSTANT(0, Skinning)
				SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_VIEWPROJ)
				SET_PROGRAM_INPUT(OES2_SHADER_INPUT_POSITION, Pos)
			END_SET_PROGRAM
		}

		DYNAMIC_STATE
		{
			pShaderAPI->Uniform1f(0, (pShaderAPI->GetCurrentNumBones() > 0) ? 1.0f : 0.0f);
		}

		Draw();
	}
END_SHADER