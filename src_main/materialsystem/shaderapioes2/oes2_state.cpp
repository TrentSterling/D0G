//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 state variables.
//
//===========================================================================//
#include <cstd/string.h>
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

void CShaderAPIOES2::ApplyCullEnable(bool bEnable)
{
	if (m_DynamicState.m_CullEnabled == bEnable)
		return;
	m_DynamicState.m_CullEnabled = bEnable;
	if (bEnable)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void CShaderAPIOES2::ApplyZBias(const ShadowState_t &state)
{
	MaterialSystem_Config_t &config = ShaderUtil()->GetConfig();
	float slopeScaleDepthBias, depthBias;
	switch (state.m_ZBias)
	{
	case SHADER_POLYOFFSET_DECAL:
		slopeScaleDepthBias = (config.m_SlopeScaleDepthBias_Decal != 0.0f) ? (1.0f / config.m_SlopeScaleDepthBias_Decal) : 0.0f;
		depthBias = (config.m_DepthBias_Decal != 0.0f) ? (1.0f / config.m_DepthBias_Decal) : 0.0f;
		break;
	case SHADER_POLYOFFSET_SHADOW_BIAS:
		slopeScaleDepthBias = m_ShadowSlopeScaleDepthBias;
		depthBias = m_ShadowDepthBias;
		break;
	default:
		slopeScaleDepthBias = (config.m_SlopeScaleDepthBias_Normal != 0.0f) ? (1.0f / config.m_SlopeScaleDepthBias_Normal) : 0.0f;
		depthBias = (config.m_DepthBias_Normal != 0.0f) ? (1.0f / config.m_DepthBias_Normal) : 0.0f;
	}
	bool enabled = slopeScaleDepthBias || depthBias;
	if (enabled)
	{
		if (!m_DynamicState.m_DepthBiasEnabled)
			glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(slopeScaleDepthBias, depthBias);
	}
	else if (m_DynamicState.m_DepthBiasEnabled)
		glDisable(GL_POLYGON_OFFSET_FILL);
	m_DynamicState.m_DepthBiasEnabled = enabled;
}

void CShaderAPIOES2::ApplyZWriteEnable(bool bEnable)
{
	m_ZWriteEnable = bEnable;
	glDepthMask(bEnable ? GL_TRUE : GL_FALSE);
}

void CShaderAPIOES2::CullMode(MaterialCullMode_t cullMode)
{
	unsigned int newFrontFace;
	switch (cullMode)
	{
	case MATERIAL_CULLMODE_CCW:
		// Culls backfacing polys (normal).
		newFrontFace = GL_CW;
		break;
	case MATERIAL_CULLMODE_CW:
		// Culls frontfacing polys.
		newFrontFace = GL_CCW;
		break;
	default:
		Warning("CShaderAPIOES2::CullMode: invalid cullMode\n");
		return;
	}
	if (m_DynamicState.m_FrontFace != newFrontFace)
	{
		FlushBufferedPrimitives();
		m_DynamicState.m_FrontFace = newFrontFace;
		if (!IsDeactivated())
			glFrontFace(newFrontFace);
	}
}

unsigned int CShaderAPIOES2::GetFrontFace(void) const
{
	Assert(m_pMaterial);
	if (!(m_DynamicState.m_CullEnabled) || m_pMaterial->GetMaterialVarFlag(MATERIAL_VAR_NOCULL))
		return 0;
	return m_DynamicState.m_FrontFace;
}

void CShaderAPIOES2::InitRenderState(void)
{
	ShaderShadow()->SetDefaultState();
	m_TransitionTable.TakeDefaultStateSnapshot();
	if (!IsDeactivated())
		ResetRenderState(false);
}

void CShaderAPIOES2::ResetRenderState(bool bFullReset)
{
	int i;

	if (!IsDeactivated())
	{
		glFrontFace(GL_CW);
		glEnable(GL_CULL_FACE);
		// glCullFace(GL_BACK); // Never changed.
		glDisable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
		glStencilMask(0xffffffff);
		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.0f, 0.0f);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		// glBlendColor(1.0f, 1.0f, 1.0f, 1.0f); // Constant color blending modes are not used.
		// glBlendEquation(GL_FUNC_ADD); // Never changed.
		glActiveTexture(GL_TEXTURE0);
		glClearDepthf(1.0f);
		glClearStencil(0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	}

	m_CurrentSnapshot = -1;

	m_CachedFastClipProjectionMatrix.Identity();

	m_SceneFogColor[0] = m_SceneFogColor[1] = m_SceneFogColor[2] = 0;
	m_SceneFogMode = MATERIAL_FOG_NONE;

	m_DynamicState.m_ClearColor[0] = m_DynamicState.m_ClearColor[1] =
		m_DynamicState.m_ClearColor[2] = m_DynamicState.m_ClearColor[3] = 0;
	m_ClearColorChanged = true;
	m_ClearReverseDepth = false;
	m_ClearStencil = 0;
	m_ZWriteEnable = true;

	StandardConstantChanged(OES2_SHADER_CONST_AMBIENT_LIGHT);

	m_WorldSpaceCameraPosition.Init(0.0f, 0.0f, 0.0f);

	m_DynamicState.m_PixelFogColor[0] = m_DynamicState.m_PixelFogColor[1] = m_DynamicState.m_PixelFogColor[2] = 0.0f;
	m_DynamicState.m_FogStart = 0.0f;
	m_DynamicState.m_FogEnd = 0.0f;
	m_DynamicState.m_FogMaxDensity = 1.0f;
	m_DynamicState.m_FogZ = 1.0f;
	UpdateFogConstant();

	// Shadow state applied in ShaderAPI.
	m_DynamicState.m_CullEnabled = true;
	m_DynamicState.m_FrontFace = GL_CW;
	m_DynamicState.m_DepthBiasEnabled = false;

	m_DynamicState.m_NumBones = 0;

	for (i = HardwareConfig()->Caps().m_NumCombinedSamplers; i-- > 0; )
		m_BoundTextures[i] = INVALID_SHADERAPI_TEXTURE_HANDLE;

	m_PixelShaderLightingChanged = true;
	m_VertexShaderLightingChanged = true;
	for (i = MAX_NUM_LIGHTS; i--; )
		m_DynamicState.m_LightEnable[i] = false;

	for (i = NUM_MATRIX_MODES; i--; )
		m_DynamicState.m_TransformType[i] = TRANSFORM_IS_GENERAL;
	StandardConstantChanged(OES2_SHADER_CONST_MODELVIEWPROJ);
	StandardConstantChanged(OES2_SHADER_CONST_VIEWPROJ);
	StandardConstantChanged(OES2_SHADER_CONST_MODEL);

	m_TransitionTable.UseDefaultState();
	SetDefaultState();

	if (bFullReset)
	{
		SetAnisotropicLevel(1);

		float *pBoneMatrix;
		for (i = 16; i--; )
		{
			pBoneMatrix = m_BoneMatrix[i];
			memset(pBoneMatrix, 0, sizeof(float) * 12);
			pBoneMatrix[0] = 1.0f;
			pBoneMatrix[5] = 1.0f;
			pBoneMatrix[10] = 1.0f;
		}
		MatrixMode(MATERIAL_VIEW);
		LoadIdentity();
		MatrixMode(MATERIAL_PROJECTION);
		LoadIdentity();
	}

	int width, height;
	GetBackBufferDimensions(width, height);
	m_Viewport.Init(0, 0, width, height);
	m_ViewportChanged = true;
	m_ViewportZChanged = true;

	m_ScissorEnabled = m_DynamicState.m_ScissorEnabled = false;
	m_DynamicState.m_ScissorRect.x = m_DynamicState.m_ScissorRect.y =
		m_DynamicState.m_ScissorRect.width = m_DynamicState.m_ScissorRect.height = -1;
	m_ScissorRectChanged = true;

	EnableFastClip(false);
	float fakePlane[4];
	int fakePlaneVal = -1;
	fakePlane[0] = fakePlane[1] = fakePlane[2] = fakePlane[3] = *((float *)(&fakePlaneVal));
	SetFastClipPlane(fakePlane);
	m_DynamicState.m_FastClipPlaneChanged = true;

	m_pRenderMesh = NULL;
	MeshMgr()->SetVertexDecl(VERTEX_FORMAT_UNKNOWN);

	SetRenderTarget(SHADER_RENDERTARGET_BACKBUFFER, SHADER_RENDERTARGET_DEPTHBUFFER);

	m_ActiveTexture = SHADER_SAMPLER0;
	m_UnpackAlignment = 4;
}