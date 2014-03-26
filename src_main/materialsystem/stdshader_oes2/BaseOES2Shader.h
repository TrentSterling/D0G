//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: Base OpenGL ES 2.0 shader header.
//
//===========================================================================//
#ifndef BASEOES2SHADER_H
#define BASEOES2SHADER_H

#include "shaderlib/cshader.h"
#include "shaderlib/baseshader.h"
#include "materialsystem/ishaderoes2.h"
#include "ConVar.h"
#include <renderparm.h>


extern ConVar mat_fullbright;


#define BEGIN_OES_SHADER(_name, _help) __BEGIN_SHADER_INTERNAL(CBaseOES2Shader, _name, _help, 0)
#define BEGIN_OES_SHADER_FLAGS(_name, _help, _flags) __BEGIN_SHADER_INTERNAL(CBaseOES2Shader, _name, _help, _flags)


#define SET_PROGRAM(_name, _combo) \
			if (!(pShaderShadow->SetShaderProgram(#_name, _combo, 0, NULL))) \
			{ \
				const char *creationName = #_name; \
				int creationCombo = _combo; \
				OES2ShaderCreationData_t creationData; \
				creationData.m_VertexFiles[creationData.m_VertexCount++] = "/common_v"; \
				creationData.m_PixelFiles[creationData.m_PixelCount++] = "/common_p";

#define SET_PROGRAM_VS_INCLUDE(_name) \
				creationData.m_VertexFiles[creationData.m_VertexCount++] = "/" #_name;
#define SET_PROGRAM_VS_SOURCE(_name) \
				creationData.m_VertexFiles[creationData.m_VertexCount++] = "." #_name;
#define SET_PROGRAM_VS_STATIC(_def) \
				creationData.m_VertexFiles[creationData.m_VertexCount++] = "#define " _def "\n";

#define SET_PROGRAM_PS_INCLUDE(_name) \
				creationData.m_PixelFiles[creationData.m_PixelCount++] = "/" #_name;
#define SET_PROGRAM_PS_SOURCE(_name) \
				creationData.m_PixelFiles[creationData.m_PixelCount++] = "." #_name;
#define SET_PROGRAM_PS_STATIC(_def) \
				creationData.m_PixelFiles[creationData.m_PixelCount++] = "#define " _def "\n";

#define SET_PROGRAM_STATIC(_def) \
				{ \
					creationData.m_VertexFiles[creationData.m_VertexCount++] = "#define " _def "\n"; \
					creationData.m_PixelFiles[creationData.m_PixelCount++] = "#define " _def "\n"; \
				}

#define SET_PROGRAM_CONSTANT(_index, _name) \
				creationData.m_Constants[_index] = "c_" #_name;
#define SET_PROGRAM_STANDARD_CONSTANT(_index) \
				creationData.m_StandardConstants |= (1 << _index);
#define SET_PROGRAM_SAMPLER(_index, _name) \
				creationData.m_Samplers[_index] = "t_" #_name;

#define SET_PROGRAM_INPUT(_index, _name) \
				creationData.m_Attributes[_index] = "i_" #_name;
#define SET_PROGRAM_TEXCOORD(_index, _name, _size) \
				{ \
					creationData.m_Attributes[OES2_SHADER_INPUT_TEXCOORD0 + _index] = "i_" #_name; \
					creationData.m_TexCoordSizes[_index] = _size; \
				}
#define SET_PROGRAM_USERDATA(_name, _size) \
				{ \
					creationData.m_Attributes[OES2_SHADER_INPUT_USERDATA] = "i_" #_name; \
					creationData.m_UserDataSize = _size; \
				}

#define SET_PROGRAM_USE_OES3_SL \
				creationData.m_Language = OES2_SHADER_LANGUAGE_ESSL3;

#define END_SET_PROGRAM \
				pShaderShadow->SetShaderProgram(creationName, creationCombo, OES2_SHADER_FORMAT, &creationData); \
			}


#define SET_PROGRAM_PIXEL_FOG(_reg) \
				{ \
					if (IS_FLAG_SET(MATERIAL_VAR_NOFOG)) \
					{ \
						SET_PROGRAM_STATIC("NOFOG") \
					} \
					else \
					{ \
						SET_PROGRAM_PS_INCLUDE(common_fog_p) \
						SET_PROGRAM_CONSTANT(_reg, FogParams) \
					} \
				}

#define SET_PROGRAM_SCREENSPACEEFFECT \
				{ \
					SET_PROGRAM_VS_INCLUDE(screenspaceeffect_v) \
					SET_PROGRAM_INPUT(OES2_SHADER_INPUT_POSITION, Pos) \
					SET_PROGRAM_TEXCOORD(0, BaseTexCoord, 2) \
				}

#define SET_PROGRAM_SKINNING \
				{ \
					SET_PROGRAM_VS_INCLUDE(common_skin_v) \
					SET_PROGRAM_STANDARD_CONSTANT(OES2_SHADER_CONST_MODEL) \
					SET_PROGRAM_INPUT(OES2_SHADER_INPUT_BONE_WEIGHT, BoneWeights) \
					SET_PROGRAM_INPUT(OES2_SHADER_INPUT_BONE_INDEX, BoneIndices) \
				}


class CBaseOES2Shader : public CBaseShader
{
public:
	BlendType_t EvaluateBlendRequirements(int textureVar, bool isBaseTexture, int detailTextureVar = -1);
	FORCEINLINE float GetEyeZ(void) { float pos[3]; s_pShaderAPI->GetWorldSpaceCameraPosition(pos); return pos[2]; }
	FORCEINLINE void SetEnvMapTintConstant(int reg, int tintVar)
	{
		if (!g_pConfig->bShowSpecular || (mat_fullbright.GetInt() == 2))
		{
			s_pShaderAPI->Uniform3f(reg, 0.0f, 0.0f, 0.0f);
			return;
		}
		float color[3];
		s_ppParams[tintVar]->GetVecValue(color, 3);
		s_pShaderAPI->Uniform3fv(reg, 1, color);
	}
	void SetProgramVertexShaderLight(OES2ShaderCreationData_t &creationData);
	void SetTextureScale(int reg, int scaleVar);
	void SetTextureScaledTransform(int reg, int transformVar, int scaleVar);
	void SetTextureTransform(int reg, int transformVar);
};


#endif // !BASEOES2SHADER_H