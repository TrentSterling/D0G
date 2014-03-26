//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 fog.
//
//===========================================================================//
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Public interface.
//-----------------------------------------------------------------------------

void CShaderAPIOES2::ApplyFogMode(ShaderFogMode_t fogMode)
{
	// D0GHDR: A lot of fog gamma correction stuff.
	if (fogMode == SHADER_FOGMODE_DISABLED)
		return;
	float *color = m_DynamicState.m_PixelFogColor;
	switch (fogMode)
	{
	case SHADER_FOGMODE_BLACK:
		color[0] = color[1] = color[2] = 0.0f;
		break;
	case SHADER_FOGMODE_OO_OVERBRIGHT:
	case SHADER_FOGMODE_GREY:
		color[0] = color[1] = color[2] = 0.5f;
		break;
	case SHADER_FOGMODE_WHITE:
		color[0] = color[1] = color[2] = 1.0f;
		break;
	case SHADER_FOGMODE_FOGCOLOR:
		{
			unsigned char rgb[3];
			GetSceneFogColor(rgb);
			color[0] = ((float)(rgb[0])) * (1.0f / 255.0f);
			color[1] = ((float)(rgb[1])) * (1.0f / 255.0f);
			color[2] = ((float)(rgb[2])) * (1.0f / 255.0f);
			break;
		}
	NO_DEFAULT
	}
}

void CShaderAPIOES2::FogEnd(float flEnd)
{
	if (m_DynamicState.m_FogEnd == flEnd)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_FogEnd = flEnd;
	UpdateFogConstant();
}

void CShaderAPIOES2::FogMaxDensity(float flMaxDensity)
{
	if (m_DynamicState.m_FogMaxDensity = flMaxDensity)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_FogMaxDensity = flMaxDensity;
	UpdateFogConstant();
}

void CShaderAPIOES2::FogStart(float flStart)
{
	if (m_DynamicState.m_FogStart == flStart)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_FogStart = flStart;
	UpdateFogConstant();
}

void CShaderAPIOES2::GetFogDistances(float *fStart, float *fEnd, float *fFogZ)
{
	if (fStart)
		*fStart = m_DynamicState.m_FogStart;
	if (fEnd)
		*fEnd = m_DynamicState.m_FogEnd;
	if (fFogZ)
		*fFogZ = m_DynamicState.m_FogZ;
}

void CShaderAPIOES2::SceneFogColor3ub(unsigned char r, unsigned char g, unsigned char b)
{
	if ((m_SceneFogColor[0] == r) && (m_SceneFogColor[1] == g) && (m_SceneFogColor[2] == b))
		return;
	FlushBufferedPrimitives();
	m_SceneFogColor[0] = r;
	m_SceneFogColor[1] = g;
	m_SceneFogColor[2] = b;
	const ShadowState_t *pShadow = m_TransitionTable.CurrentShadowState();
	if (pShadow)
		ApplyFogMode(pShadow->m_FogMode);
}

void CShaderAPIOES2::SceneFogMode(MaterialFogMode_t fogMode)
{
	if (m_SceneFogMode == fogMode)
		return;
	FlushBufferedPrimitives();
	m_SceneFogMode = fogMode;
	// ApplyFogMode is not required because FogMode is FF.
}

void CShaderAPIOES2::SetFogZ(float fogZ)
{
	if (m_DynamicState.m_FogZ == fogZ)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_FogZ = fogZ;
	UpdateFogConstant();
}

void CShaderAPIOES2::SetPixelShaderFogParams(int reg)
{
	float fogParams[8];
	if ((m_TransitionTable.CurrentShadowState()->m_FogMode != SHADER_FOGMODE_DISABLED) &&
		(m_SceneFogMode != MATERIAL_FOG_NONE))
	{
		fogParams[0] = m_FogConstant[0];
		fogParams[1] = m_FogConstant[1];
		// Negative z means water fog (to avoid using additional boolean constant).
		fogParams[2] = (m_SceneFogMode == MATERIAL_FOG_LINEAR) ? m_FogConstant[2] : -1.0f;
		fogParams[3] = m_FogConstant[3];
	}
	else
	{
		fogParams[0] = 1.0f;
		fogParams[1] = m_DynamicState.m_FogZ;
		fogParams[2] = 1.0f;
		fogParams[3] = 0.0f;
	}
	fogParams[4] = m_DynamicState.m_PixelFogColor[0];
	fogParams[5] = m_DynamicState.m_PixelFogColor[1];
	fogParams[6] = m_DynamicState.m_PixelFogColor[2];
	fogParams[7] = 0.0f;
	Uniform4fv(reg, 2, fogParams);
}

//-----------------------------------------------------------------------------
// Private API functions.
//-----------------------------------------------------------------------------

void CShaderAPIOES2::UpdateFogConstant(void)
{
	float ooFogRange = 1.0f;
	float fStart = m_DynamicState.m_FogStart;
	float fEnd = m_DynamicState.m_FogEnd;
	if (fEnd != fStart)
		ooFogRange /= fEnd - fStart;
	m_FogConstant[0] = 1.0f - (ooFogRange * fEnd); // 1 - (end / (fogEnd - fogStart)).
	m_FogConstant[1] = m_DynamicState.m_FogZ; // Water height.
	m_FogConstant[2] = clamp(m_DynamicState.m_FogMaxDensity, 0.0f, 1.0f); // Max fog density.
	m_FogConstant[3] = ooFogRange; // 1 / (fogEnd - fogStart).
}