//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 shader class interface.
//
//===========================================================================//
#ifndef ISHADEROES2_H
#define ISHADEROES2_H

#include <cstd/string.h>
#include "shaderapi/shareddefs.h"
#include "imesh.h"
#include "IShader.h"
#include "materialsystem/ishaderapi.h"
#include "tier0/commonmacros.h"
#include "tier0/platform.h"

#define OES2_SHADER_MAX_PIXEL_SAMPLERS 16
#define OES2_SHADER_MAX_VERTEX_SAMPLERS 4
#define OES2_SHADER_MAX_SAMPLERS (OES2_SHADER_MAX_PIXEL_SAMPLERS + OES2_SHADER_MAX_VERTEX_SAMPLERS)

#define OES2_SHADER_FORMAT MAKEID('E','S','0','1')

#define OES2_SHADER_MAX_FILES 31 // To let file-dependent booleans use a bitfield.

// Standard vertex shader constants.
enum
{
	OES2_SHADER_CONST_MODELVIEWPROJ, // f4x4
	OES2_SHADER_CONST_VIEWPROJ, // f4x4
	OES2_SHADER_CONST_VIEWMODEL, // f4x4
	OES2_SHADER_CONST_AMBIENT_LIGHT, // f3[3][2] as f3[6]
	OES2_SHADER_CONST_LIGHTS_COLOR, // f4[4]
	OES2_SHADER_CONST_LIGHTS_DIRECTION, // f4[4]
	OES2_SHADER_CONST_LIGHTS_POSITION, // f3[4]
	OES2_SHADER_CONST_LIGHTS_SPOT, // f4[4]
	OES2_SHADER_CONST_LIGHTS_ATTENUATION, // f3[4]
	OES2_SHADER_CONST_LIGHTS_ENABLE, // i1
	OES2_SHADER_CONST_MODEL, // f4x3[16] as f4[48]
	OES2_SHADER_CONST_COUNT
};
typedef uint32 OES2ShaderConstantUsage_t;

enum
{
	// If matrices are added here, the shader loading and drawing must account that because matrices take multiple locations!
	OES2_SHADER_INPUT_POSITION,
	OES2_SHADER_INPUT_POSITION_FLEX,
	OES2_SHADER_INPUT_BONE_WEIGHT,
	OES2_SHADER_INPUT_BONE_INDEX,
	OES2_SHADER_INPUT_NORMAL,
	OES2_SHADER_INPUT_NORMAL_FLEX,
	OES2_SHADER_INPUT_COLOR,
	OES2_SHADER_INPUT_SPECULAR,
	OES2_SHADER_INPUT_TEXCOORD0,
	OES2_SHADER_INPUT_TEXCOORD1,
	OES2_SHADER_INPUT_TEXCOORD2,
	OES2_SHADER_INPUT_TEXCOORD3,
	OES2_SHADER_INPUT_TEXCOORD4,
	OES2_SHADER_INPUT_TEXCOORD5,
	OES2_SHADER_INPUT_TEXCOORD6,
	OES2_SHADER_INPUT_TEXCOORD7,
	OES2_SHADER_INPUT_TANGENT_S,
	OES2_SHADER_INPUT_TANGENT_T,
	OES2_SHADER_INPUT_USERDATA,
	OES2_SHADER_INPUT_COUNT
};

#define OES2_SHADER_MAX_CONSTANTS 32 // Should be enough for all shader-specific constants.

enum OES2ShaderLanguage_t
{
	OES2_SHADER_LANGUAGE_ESSL2 = 100,
	OES2_SHADER_LANGUAGE_ESSL3 = 300
};

struct OES2ShaderCreationData_t
{
	// Files are loaded and concatenated sequentially.
	// Prefix . - load the file from FS.
	// Prefix / - cache the file globally.
	// No prefix - include in place (must end with \n).
	// Using pointers instead of arrays because most strings are constant.
	const char *m_VertexFiles[OES2_SHADER_MAX_FILES];
	int m_VertexCount;
	const char *m_PixelFiles[OES2_SHADER_MAX_FILES];
	int m_PixelCount;

	OES2ShaderConstantUsage_t m_StandardConstants; // A bitfield of OES2_SHADER_CONST_ values.
	const char *m_Constants[OES2_SHADER_MAX_CONSTANTS]; // An array of used constant names.
	const char *m_Samplers[OES2_SHADER_MAX_SAMPLERS]; // Sampler uniform names. Will ignore samplers not supported by hardware.

	const char *m_Attributes[OES2_SHADER_INPUT_COUNT]; // An array of the names of used vertex inputs.
	unsigned int m_TexCoordSizes[VERTEX_MAX_TEXTURE_COORDINATES]; // Number of floats per texture coordinate.
	unsigned int m_UserDataSize; // Number of userdata attribute floats [0-4].

	OES2ShaderLanguage_t m_Language; // GLSL version of the shader.

	OES2ShaderCreationData_t(void)
	{
		m_VertexCount = 0;
		m_PixelCount = 0;
		m_StandardConstants = 0;
		memset(m_Constants, 0, sizeof(m_Constants));
		memset(m_Samplers, 0, sizeof(m_Samplers));
		memset(m_Attributes, 0, sizeof(m_Attributes));
		memset(m_TexCoordSizes, 0, sizeof(m_TexCoordSizes));
		m_UserDataSize = 0;
		m_Language = OES2_SHADER_LANGUAGE_ESSL2;
	}
};

#endif // !ISHADEROES2_H