//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 shader setup interface.
//
//===========================================================================//
#include <cstd/string.h>
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

#define SHADOWOES2_DUMMY(name) virtual void name(bool bEnable) {}
#define SHADOWOES2_BOOLEAN(name, state) virtual void name(bool bEnable) { m_ShadowState.state = bEnable; }
#define SHADOWOES2_GLBOOLEAN(name, state) virtual void name(bool bEnable) { m_ShadowState.state = (bEnable ? 1 : 0); }

class CShaderShadowOES2 : public IShaderShadowOES2
{
public:
	CShaderShadowOES2(void) { Init(); }

	virtual void SetDefaultState(void);

	virtual void DepthFunc(ShaderDepthFunc_t depthFunc);
	SHADOWOES2_BOOLEAN(EnableDepthWrites, m_ZWriteEnable);
	SHADOWOES2_BOOLEAN(EnableDepthTest, m_ZEnable);
	virtual void EnablePolyOffset(PolygonOffsetMode_t nOffsetMode) { m_ShadowState.m_ZBias = nOffsetMode; }

	// Stencil is a part of the shader API, not the shadow state.
	SHADOWOES2_DUMMY(EnableStencil);
	virtual void StencilFunc(ShaderStencilFunc_t stencilFunc) {}
	virtual void StencilPassOp(ShaderStencilOp_t stencilOp) {}
	virtual void StencilFailOp(ShaderStencilOp_t stencilOp) {}
	virtual void StencilDepthFailOp(ShaderStencilOp_t stencilOp) {}
	virtual void StencilReference(int nReference) {}
	virtual void StencilMask(int nMask) {}
	virtual void StencilWriteMask(int nMask) {}

	SHADOWOES2_GLBOOLEAN(EnableColorWrites, m_ColorWriteEnable);
	SHADOWOES2_GLBOOLEAN(EnableAlphaWrites, m_AlphaWriteEnable);

	SHADOWOES2_BOOLEAN(EnableBlending, m_AlphaBlendEnable);
	virtual void BlendFunc(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor)
	{
		m_BlendFuncs.m_Src = BlendFuncValue(srcFactor);
		m_BlendFuncs.m_Dest = BlendFuncValue(dstFactor);
	}

	SHADOWOES2_BOOLEAN(EnableAlphaTest, m_AlphaTestEnable); // To tell opacity check if we're going to discard.
	virtual void AlphaFunc(ShaderAlphaFunc_t alphaFunc, float alphaRef) {}

	virtual void PolyMode(ShaderPolyModeFace_t face, ShaderPolyMode_t polyMode) {} // OES2 allows only GL_FILL.

	SHADOWOES2_BOOLEAN(EnableCulling, m_CullEnable);

	SHADOWOES2_DUMMY(EnableConstantColor); // FF.

	// Shader creation attribute table defines the vertex format.
	virtual void VertexShaderVertexFormat(unsigned int nFlags, int nTexCoordCount, int* pTexCoordDimensions,
	                                      int nUserDataSize) {}

	// Using SetShaderProgram instead.
	virtual void SetVertexShader(const char *pFileName, int nStaticVshIndex) { Assert(0); }
	virtual void SetPixelShader(const char *pFileName, int nStaticPshIndex) { Assert(0); }

	SHADOWOES2_DUMMY(EnableLighting); // FF.
	SHADOWOES2_DUMMY(EnableSpecular); // Unused.

	// D0GSRGB
	SHADOWOES2_DUMMY(EnableSRGBWrite);
	virtual void EnableSRGBRead(Sampler_t sampler, bool bEnable) {}

	// FF stuff.
	SHADOWOES2_DUMMY(EnableVertexBlend);
	virtual void OverbrightValue(TextureStage_t stage, float value) {}
	virtual void EnableTexture(Sampler_t sampler, bool bEnable) {}
	virtual void EnableTexGen(TextureStage_t stage, bool bEnable) {}
	virtual void TexGen(TextureStage_t stage, ShaderTexGenParam_t param) {}
	SHADOWOES2_DUMMY(EnableCustomPixelPipe);
	virtual void CustomTextureStages(int stageCount) {}
	virtual void CustomTextureOperation(TextureStage_t stage, ShaderTexChannel_t channel, 
		ShaderTexOp_t op, ShaderTexArg_t arg1, ShaderTexArg_t arg2) {}
	virtual void DrawFlags(unsigned int drawFlags) {}
	SHADOWOES2_DUMMY(EnableAlphaPipe);
	SHADOWOES2_DUMMY(EnableConstantAlpha);
	SHADOWOES2_DUMMY(EnableVertexAlpha);
	virtual void EnableTextureAlpha(TextureStage_t stage, bool bEnable) {}

	SHADOWOES2_BOOLEAN(EnableBlendingSeparateAlpha, m_SeparateAlphaBlendEnable);
	virtual void BlendFuncSeparateAlpha(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor)
	{
		m_BlendFuncs.m_SrcAlpha = BlendFuncValue(srcFactor);
		m_BlendFuncs.m_DestAlpha = BlendFuncValue(dstFactor);
	}

	virtual void FogMode(ShaderFogMode_t fogMode)
	{
		Assert((fogMode >= 0) && (fogMode < SHADER_FOGMODE_NUMFOGMODES));
		m_ShadowState.m_FogMode = fogMode;
	}

	virtual void SetDiffuseMaterialSource(ShaderMaterialSource_t materialSource) {} // Unused.

	virtual void SetMorphFormat(MorphFormat_t flags) {}

	SHADOWOES2_DUMMY(DisableFogGammaCorrection); // D0GHDR: Fog gamma correction.

	// Alpha to coverage is not required in OES2, FXAA will be used instead.
	SHADOWOES2_DUMMY(EnableAlphaToCoverage);

	virtual void SetShadowDepthFiltering(Sampler_t stage) {} // No ARB_texture_gather in OES2.

	virtual bool SetShaderProgram(const char *pName, int combo, int format, void *pData);

	virtual void Init(void);
	virtual const ShadowState_t &GetShadowState(void) { return m_ShadowState; }
	virtual const ShadowShaderState_t &GetShadowShaderState(void) { return m_ShadowShaderState; }
	virtual void ComputeAggregateShadowState(void);

private:
	FORCEINLINE unsigned int BlendFuncValue(ShaderBlendFactor_t factor) const;

	ShadowState_t::BlendFuncs_t m_BlendFuncs;

	ShadowState_t m_ShadowState;
	ShadowShaderState_t m_ShadowShaderState;
};

void CShaderShadowOES2::Init(void)
{
	memset(&m_ShadowState, 0, sizeof(m_ShadowState));
	m_ShadowShaderState.m_ShaderProgram = SHADER_PROGRAM_HANDLE_INVALID;
	m_ShadowShaderState.m_VertexUsage = 0;
}

void CShaderShadowOES2::SetDefaultState(void)
{
	DepthFunc(SHADER_DEPTHFUNC_NEAREROREQUAL);
	EnableDepthTest(true);
	EnableDepthWrites(true);
	EnablePolyOffset(SHADER_POLYOFFSET_DISABLE);

	EnableBlending(false);
	BlendFunc(SHADER_BLEND_ZERO, SHADER_BLEND_ZERO);
	EnableBlendingSeparateAlpha(false);
	BlendFuncSeparateAlpha(SHADER_BLEND_ZERO, SHADER_BLEND_ZERO);

	EnableColorWrites(true);
	EnableAlphaWrites(false);

	EnableCulling(true);

	// D0GHDR: DisableFogGammaCorrection(false).

	// D0GSRGB

	m_ShadowShaderState.m_ShaderProgram = SHADER_PROGRAM_HANDLE_INVALID;
	m_ShadowShaderState.m_VertexUsage = 0;
}

void CShaderShadowOES2::DepthFunc(ShaderDepthFunc_t depthFunc)
{
	// Mapped to ShaderDepthFunc_t enum!
	if ((depthFunc < SHADER_DEPTHFUNC_NEVER) || (depthFunc > SHADER_DEPTHFUNC_ALWAYS))
	{
		Warning("DepthFunc: invalid param\n");
		m_ShadowState.m_ZFunc = GL_ALWAYS;
		return;
	}
	const unsigned int zFuncs[2][8] =
	{
		{
			GL_NEVER,
			GL_LESS,
			GL_EQUAL,
			GL_LEQUAL,
			GL_GREATER,
			GL_NOTEQUAL,
			GL_GEQUAL,
			GL_ALWAYS
		},
		{
			GL_NEVER,
			GL_GREATER,
			GL_EQUAL,
			GL_GEQUAL,
			GL_LESS,
			GL_NOTEQUAL,
			GL_LEQUAL,
			GL_ALWAYS
		}
	};
	m_ShadowState.m_ZFunc =
		zFuncs[ShaderUtil()->GetConfig().bReverseDepth ? 1 : 0][(int)depthFunc - (int)SHADER_DEPTHFUNC_NEVER];
}

unsigned int CShaderShadowOES2::BlendFuncValue(ShaderBlendFactor_t factor) const
{
	// Mapped to ShaderBlendFactor_t enum!
	if ((factor < SHADER_BLEND_ZERO) || (factor > SHADER_BLEND_ONE_MINUS_SRC_COLOR))
	{
		Warning("BlendFunc: invalid factor\n");
		return GL_ONE;
	}
	const unsigned int factors[] =
	{
		GL_ZERO,
		GL_ONE,
		GL_DST_COLOR,
		GL_ONE_MINUS_DST_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA,
		GL_SRC_ALPHA_SATURATE,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR
	};
	return factors[(int)factor - (int)SHADER_BLEND_ZERO];
}

bool CShaderShadowOES2::SetShaderProgram(const char *pName, int combo, int format, void *pData)
{
	m_ShadowShaderState.m_ShaderProgram = ShaderAPI()->GetShaderProgram(pName, combo);
	if (m_ShadowShaderState.m_ShaderProgram == SHADER_PROGRAM_HANDLE_INVALID)
	{
		if (!pData)
			return false;
		m_ShadowShaderState.m_ShaderProgram = ShaderAPI()->CreateShaderProgram(pName, combo, format, pData);
		if (m_ShadowShaderState.m_ShaderProgram == SHADER_PROGRAM_HANDLE_INVALID)
		{
			m_ShadowShaderState.m_VertexUsage = VERTEX_FORMAT_INVALID;
			return false;
		}
	}
	const ShaderProgram_t *program =
		(const ShaderProgram_t *)(ShaderAPI()->GetShaderProgramData(m_ShadowShaderState.m_ShaderProgram, OES2_SHADER_FORMAT));
	m_ShadowShaderState.m_VertexUsage = (program ? program->m_VertexUsage : VERTEX_FORMAT_INVALID);
	return true;
}

void CShaderShadowOES2::ComputeAggregateShadowState(void)
{
	if (m_ShadowState.m_AlphaBlendEnable)
	{
		m_ShadowState.m_BlendFuncs.m_Src = m_BlendFuncs.m_Src;
		m_ShadowState.m_BlendFuncs.m_Dest = m_BlendFuncs.m_Dest;
	}
	else
	{
		m_ShadowState.m_BlendFuncs.m_Src = GL_ONE;
		m_ShadowState.m_BlendFuncs.m_Dest = GL_ZERO;
	}

	if (m_ShadowState.m_SeparateAlphaBlendEnable)
	{
		m_ShadowState.m_BlendFuncs.m_SrcAlpha = m_BlendFuncs.m_SrcAlpha;
		m_ShadowState.m_BlendFuncs.m_DestAlpha = m_BlendFuncs.m_DestAlpha;
	}
	else
	{
		m_ShadowState.m_BlendFuncs.m_SrcAlpha = GL_ONE;
		m_ShadowState.m_BlendFuncs.m_DestAlpha = GL_ZERO;
	}
}

static CShaderShadowOES2 s_ShaderShadowOES2;
IShaderShadowOES2 *g_pShaderShadowOES2 = &s_ShaderShadowOES2;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CShaderShadowOES2, IShaderShadow,
	SHADERSHADOW_INTERFACE_VERSION, s_ShaderShadowOES2);