//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 shader setup interface.
//
//===========================================================================//
#ifndef OES2_SHADOW_H
#define OES2_SHADOW_H

#include "shaderapi/ishadershadow.h"

struct ShadowState_t
{
	struct BlendFuncs_t
	{
		unsigned int m_Src;
		unsigned int m_Dest;
		unsigned int m_SrcAlpha;
		unsigned int m_DestAlpha;
	};

	unsigned int m_ZFunc;

	BlendFuncs_t m_BlendFuncs;

	ShaderFogMode_t m_FogMode;

	bool m_ZEnable : 1; // 1 bit in total.
	unsigned char m_ZWriteEnable : 1; // 2
	unsigned char m_ZBias : 2; // 4

	unsigned char m_ColorWriteEnable : 1; // 5
	unsigned char m_AlphaWriteEnable : 1; // 6

	bool m_AlphaBlendEnable : 1; // 7
	bool m_SeparateAlphaBlendEnable : 1; // 8

	bool m_AlphaTestEnable : 1; // 9

	bool m_CullEnable : 1; // 10

	// D0GHDR: m_DisableFogGammaCorrection.

	unsigned int m_Reserved : 22;
};

struct ShadowShaderState_t
{
	ShaderProgramHandle_t m_ShaderProgram;
	VertexFormat_t m_VertexUsage;
	unsigned int m_Reserved; // For CRC.
};

abstract_class IShaderShadowOES2 : public IShaderShadow
{
public:
	virtual void Init(void) = 0;
	virtual const ShadowState_t &GetShadowState(void) = 0;
	virtual const ShadowShaderState_t &GetShadowShaderState(void) = 0;
	virtual void ComputeAggregateShadowState(void) = 0;
};

extern IShaderShadowOES2 *g_pShaderShadowOES2;
inline IShaderShadowOES2 *ShaderShadow(void) { return g_pShaderShadowOES2; }

#endif // !OES2_SHADOW_H