//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 dynamic light.
//
//===========================================================================//
#include <cstd/string.h>
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Public interface.
//-----------------------------------------------------------------------------

void CShaderAPIOES2::CommitPixelShaderLighting(int pshReg)
{
	if (m_PixelShaderLightingChanged)
	{
		m_PixelShaderLightingChanged = false;

		int lightIndex[MAX_NUM_LIGHTS];
		SortLights(lightIndex);

		const float farAway = 10000.0f;

		int i;
		for (i = 6; i--; )
			m_PixelShaderLighting[i].Init();

		int numLights = m_DynamicState.m_NumLights;
		if (numLights > 0)
		{
			LightDesc_t *light = &(m_DynamicState.m_LightDescs[lightIndex[0]]);
			m_PixelShaderLighting[0].AsVector3D() = light->m_Color;
			if (light->m_Type == MATERIAL_LIGHT_DIRECTIONAL)
				m_PixelShaderLighting[1].AsVector3D() = m_DynamicState.m_LightingOrigin - light->m_Direction * farAway;
			else
				m_PixelShaderLighting[1].AsVector3D() = light->m_Position;

			if (numLights > 1)
			{
				light = &(m_DynamicState.m_LightDescs[lightIndex[1]]);
				m_PixelShaderLighting[2].AsVector3D() = light->m_Color;
				if (light->m_Type == MATERIAL_LIGHT_DIRECTIONAL)
					m_PixelShaderLighting[3].AsVector3D() = m_DynamicState.m_LightingOrigin - light->m_Direction * farAway;
				else
					m_PixelShaderLighting[3].AsVector3D() = light->m_Position;

// Having 4 lights (6 constants) decreases vertex throughput badly, also on DX9 pre-SM2.0b cards, only 2 lights are enabled.
#if MAX_NUM_PIXEL_LIGHT_CONSTANTS > 4
				if (numLights > 2)
				{
					light = &(m_DynamicState.m_LightDescs[lightIndex[2]]);
					m_PixelShaderLighting[4].AsVector3D() = light->m_Color;
					if (light->m_Type == MATERIAL_LIGHT_DIRECTIONAL)
						m_PixelShaderLighting[5].AsVector3D() = m_DynamicState.m_LightingOrigin - light->m_Direction * farAway;
					else
						m_PixelShaderLighting[5].AsVector3D() = light->m_Position;

					if (numLights > 3)
					{
						light = &(m_DynamicState.m_LightDescs[lightIndex[3]]);
						m_PixelShaderLighting[0].w = light->m_Color[0];
						m_PixelShaderLighting[1].w = light->m_Color[1];
						m_PixelShaderLighting[2].w = light->m_Color[2];
						if (light->m_Type == MATERIAL_LIGHT_DIRECTIONAL)
						{
							Vector pos = m_DynamicState.m_LightingOrigin - light->m_Direction * farAway;
							m_PixelShaderLighting[3].w = pos.x;
							m_PixelShaderLighting[4].w = pos.y;
							m_PixelShaderLighting[5].w = pos.z;
						}
						else
						{
							m_PixelShaderLighting[3].w = light->m_Position.x;
							m_PixelShaderLighting[4].w = light->m_Position.y;
							m_PixelShaderLighting[5].w = light->m_Position.z;
						}
					}
				}
#endif
			}
		}
	}
	Uniform4fv(pshReg, MAX_NUM_PIXEL_LIGHT_CONSTANTS, m_PixelShaderLighting[0].Base());
}

void CShaderAPIOES2::DisableAllLocalLights(void)
{
	bool flushed = false;
	int i;
	for (i = MAX_NUM_LIGHTS; i--; )
	{
		if (!(m_DynamicState.m_LightEnable[i]))
			continue;
		if (!flushed)
		{
			FlushBufferedPrimitives();
			m_PixelShaderLightingChanged = true;
			m_VertexShaderLightingChanged = true;
			flushed = true;
		}
		m_DynamicState.m_LightDescs[i].m_Type = MATERIAL_LIGHT_DISABLE;
		m_DynamicState.m_LightEnable[i] = false;
	}
}

float CShaderAPIOES2::GetAmbientLightCubeLuminance(void)
{
	Vector4D vLuminance(0.3f, 0.59f, 0.11f, 0.0f);
	float fLuminance = 0.0f;
	int i;
	for (i = 6; i--; )
		fLuminance += vLuminance.Dot(m_DynamicState.m_AmbientLightCube[i]);
	return fLuminance * (1.0f / 6.0f);
}

void CShaderAPIOES2::GetDX9LightState(LightState_t *state) const
{
	Assert(m_pRenderMesh);
	// D0GTODO: Count zeros in SetAmbientLightCube maybe?
	state->m_bAmbientLight = !(
		(m_DynamicState.m_AmbientLightCube[0][0] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[0][1] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[0][2] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[1][0] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[1][1] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[1][2] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[2][0] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[2][1] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[2][2] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[3][0] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[3][1] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[3][2] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[4][0] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[4][1] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[4][2] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[5][0] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[5][1] == 0.0f) &&
		(m_DynamicState.m_AmbientLightCube[5][2] == 0.0f));
	state->m_nNumLights = m_DynamicState.m_NumLights;
	state->m_bStaticLight = m_pRenderMesh->HasColorMesh();
}

void CShaderAPIOES2::SetAmbientLightCube(Vector4D cube[6])
{
	if (!memcmp(m_DynamicState.m_AmbientLightCube, cube, 6 * sizeof(Vector4D)))
		return;
	memcpy(m_DynamicState.m_AmbientLightCube, cube, 6 * sizeof(Vector4D));
	StandardConstantChanged(OES2_SHADER_CONST_AMBIENT_LIGHT);
}

void CShaderAPIOES2::SetFlashlightStateEx(const FlashlightState_t &state, const VMatrix &worldToTexture,
		ITexture *pFlashlightDepthTexture)
{
	FlushBufferedPrimitives();
	m_FlashlightState = state;
	m_FlashlightWorldToTexture = worldToTexture;
	if (SupportsShadowDepthTextures())
		m_pFlashlightDepthTexture = pFlashlightDepthTexture;
	else
		m_FlashlightState.m_bEnableShadows = false;
}

void CShaderAPIOES2::SetLight(int lightNum, const LightDesc_t &desc)
{
	Assert((lightNum >= 0) && (lightNum < MAX_NUM_LIGHTS));
	Assert((desc.m_Range >= 0.0f) && (desc.m_Range <= sqrt(FLT_MAX)));
	FlushBufferedPrimitives();
	if (desc.m_Type == MATERIAL_LIGHT_DISABLE)
	{
		if (m_DynamicState.m_LightEnable[lightNum])
		{
			m_DynamicState.m_LightEnable[lightNum] = false;
			m_PixelShaderLightingChanged = true;
			m_VertexShaderLightingChanged = true;
		}
		return;
	}
	m_DynamicState.m_LightEnable[lightNum] = true;
	LightDesc_t &pLight = m_DynamicState.m_LightDescs[lightNum];
	memcpy(&pLight, &(const_cast<LightDesc_t &>(desc)), sizeof(LightDesc_t));
	if (pLight.m_Phi > (float)M_PI)
		pLight.m_Phi = (float)M_PI;
	if ((pLight.m_Theta - pLight.m_Phi) > -0.001f)
		pLight.m_Theta = pLight.m_Phi - 0.001f;
	m_PixelShaderLightingChanged = true;
	m_VertexShaderLightingChanged = true;
}

void CShaderAPIOES2::SetLightingOrigin(Vector vLightingOrigin)
{
	if (vLightingOrigin == m_DynamicState.m_LightingOrigin)
		return;
	FlushBufferedPrimitives();
	m_DynamicState.m_LightingOrigin = vLightingOrigin;
	m_PixelShaderLightingChanged = true;
}

void CShaderAPIOES2::SetPixelShaderStateAmbientLightCube(int pshReg, bool bForceToBlack)
{
	if (bForceToBlack)
	{
		float tempCube[24];
		memset(tempCube, 0, sizeof(tempCube));
		Uniform4fv(pshReg, 6, tempCube);
	}
	else
	{
		Uniform4fv(pshReg, 6, m_DynamicState.m_AmbientLightCube[0].Base());
	}
}

void CShaderAPIOES2::SetVertexShaderStateAmbientLightCube(void)
{
	if (CommitStandardConstant(OES2_SHADER_CONST_AMBIENT_LIGHT))
	{
		glUniform4fv(GetStandardConstLocation(OES2_SHADER_CONST_AMBIENT_LIGHT), 6,
			m_DynamicState.m_AmbientLightCube[0].Base());
	}
}

//-----------------------------------------------------------------------------
// Private API functions.
//-----------------------------------------------------------------------------

void CShaderAPIOES2::CommitVertexShaderLighting(void)
{
	int i;
	if (m_VertexShaderLightingChanged)
	{
		m_VertexShaderLightingChanged = false;

		int oldNumLights = m_DynamicState.m_NumLights;
		int lightIndex[MAX_NUM_LIGHTS];
		SortLights(lightIndex);

		LightDesc_t &light = m_DynamicState.m_LightDescs[lightIndex[i]];
		float *lightState;
		for (i = 0; i < m_DynamicState.m_NumLights; ++i)
		{
			lightState = m_VertexShaderLighting.m_Color[i];
			lightState[0] = light.m_Color[0];
			lightState[1] = light.m_Color[1];
			lightState[2] = light.m_Color[2];
			lightState[3] = (light.m_Type == MATERIAL_LIGHT_DIRECTIONAL) ? 1.0f : 0.0f;

			lightState = m_VertexShaderLighting.m_Direction[i];
			lightState[0] = light.m_Direction[0];
			lightState[1] = light.m_Direction[1];
			lightState[2] = light.m_Direction[2];
			lightState[3] = (light.m_Type == MATERIAL_LIGHT_SPOT) ? 1.0f : 0.0f;

			lightState = m_VertexShaderLighting.m_Position[i];
			lightState[0] = light.m_Position[0];
			lightState[1] = light.m_Position[1];
			lightState[2] = light.m_Position[2];

			lightState = m_VertexShaderLighting.m_Spot[i];
			if (light.m_Type == MATERIAL_LIGHT_SPOT)
			{
				float stopdot = cos(light.m_Theta * 0.5f);
				float stopdot2 = cos(light.m_Phi * 0.5f);
				lightState[0] = light.m_Falloff;
				lightState[1] = stopdot;
				lightState[2] = stopdot2;
				lightState[3] = (stopdot > stopdot2) ? 1.0f / (stopdot - stopdot2) : 0.0f;
			}
			else
			{
				lightState[0] = 0.0f;
				lightState[1] = lightState[2] = lightState[3] = 0.0f;
			}

			lightState = m_VertexShaderLighting.m_Attenuation[i];
			lightState[0] = light.m_Attenuation0;
			lightState[1] = light.m_Attenuation1;
			lightState[2] = light.m_Attenuation2;
		}

		StandardConstantChanged(OES2_SHADER_CONST_LIGHTS_COLOR); // No need to mark other light constants as changed.
	}

	if (CommitStandardConstant(OES2_SHADER_CONST_LIGHTS_COLOR))
	{
		i = m_DynamicState.m_NumLights;
		glUniform4fv(GetStandardConstLocation(OES2_SHADER_CONST_LIGHTS_COLOR), i, m_VertexShaderLighting.m_Color[0]);
		glUniform4fv(GetStandardConstLocation(OES2_SHADER_CONST_LIGHTS_DIRECTION), i, m_VertexShaderLighting.m_Direction[0]);
		glUniform3fv(GetStandardConstLocation(OES2_SHADER_CONST_LIGHTS_POSITION), i, m_VertexShaderLighting.m_Position[0]);
		glUniform4fv(GetStandardConstLocation(OES2_SHADER_CONST_LIGHTS_SPOT), i, m_VertexShaderLighting.m_Spot[0]);
		glUniform3fv(GetStandardConstLocation(OES2_SHADER_CONST_LIGHTS_ATTENUATION), i, m_VertexShaderLighting.m_Attenuation[0]);
		glUniform1i(GetStandardConstLocation(OES2_SHADER_CONST_LIGHTS_ENABLE), i);
	}

	SetVertexShaderStateAmbientLightCube();
}

VertexShaderLightTypes_t CShaderAPIOES2::ComputeLightType(int i) const
{
	if (!(m_DynamicState.m_LightEnable[i]))
		return LIGHT_NONE;
	switch (m_DynamicState.m_LightDescs[i].m_Type)
	{
	case MATERIAL_LIGHT_POINT:
		return LIGHT_POINT;
	case MATERIAL_LIGHT_DIRECTIONAL:
		return LIGHT_DIRECTIONAL;
	case MATERIAL_LIGHT_SPOT:
		return LIGHT_SPOT;
	}
	Assert(0);
	return LIGHT_NONE;
}

void CShaderAPIOES2::SortLights(int *index)
{
	m_DynamicState.m_NumLights = 0;
	int i;
	for (i = 0; i < MAX_NUM_LIGHTS; ++i)
	{
		VertexShaderLightTypes_t type = ComputeLightType(i);
		if (type == LIGHT_NONE)	
			continue;
		int j = m_DynamicState.m_NumLights;
		while (--j >= 0)
		{
			if (m_DynamicState.m_LightType[j] <= type)
				break;
			m_DynamicState.m_LightType[j + 1] = m_DynamicState.m_LightType[j];
			index[j + 1] = index[j];
		}
		++j;
		m_DynamicState.m_LightType[j] = type;
		index[j] = i;
		++m_DynamicState.m_NumLights;
	}
}