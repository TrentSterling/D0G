//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: Clears color/depth, but obeys stencil while doing so.
//
//===========================================================================//
#include "BaseOES2Shader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_OES_SHADER_FLAGS(BufferClearObeyStencil, "", SHADER_NOT_EDITABLE)
	BEGIN_SHADER_PARAMS
		SHADER_PARAM(CLEARCOLOR, SHADER_PARAM_TYPE_INTEGER, "1", "activates clearing of color")
		SHADER_PARAM(CLEARDEPTH, SHADER_PARAM_TYPE_INTEGER, "1", "activates clearing of depth")
	END_SHADER_PARAMS

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			bool enableColorWrites = params[CLEARCOLOR]->GetIntValue() != 0;
			pShaderShadow->DepthFunc(SHADER_DEPTHFUNC_ALWAYS);
			pShaderShadow->EnableDepthWrites(params[CLEARDEPTH]->GetIntValue() != 0);
			pShaderShadow->EnableColorWrites(enableColorWrites);
			pShaderShadow->EnableAlphaWrites(enableColorWrites);
			SET_PROGRAM(BufferClearObeyStencil, enableColorWrites ? 1 : 0)
				if (enableColorWrites)
				{
					SET_PROGRAM_STATIC("USESCOLOR")
					SET_PROGRAM_INPUT(OES2_SHADER_INPUT_COLOR, Color)
				}
				SET_PROGRAM_VS_SOURCE(bufferclearobeystencil_v)
				SET_PROGRAM_PS_SOURCE(bufferclearobeystencil_p)
				SET_PROGRAM_INPUT(OES2_SHADER_INPUT_POSITION, Pos)
			END_SET_PROGRAM
		}
		Draw();
	}
END_SHADER