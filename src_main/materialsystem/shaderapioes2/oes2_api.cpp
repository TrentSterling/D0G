//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 shader API.
//
//===========================================================================//
#include <float.h>
#include <cstd/string.h>
#include <EGL/egl.h>
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Some API functions.
//-----------------------------------------------------------------------------
CShaderAPIOES2::CShaderAPIOES2(void) :
	m_Framebuffers(32),
	m_Textures(32),
	m_Programs(32),
	m_ToneMappingScale(1.0f, 1.0f, 1.0f)
{
	int i;

	m_CurrentFrame = 0;

	m_ShadowSlopeScaleDepthBias = m_ShadowDepthBias = 0.0f;

	m_BackBufferWidth = m_BackBufferHeight = 0;
	m_DeviceActive = false;
	m_EGLContext = EGL_NO_CONTEXT;
	m_EGLDisplay = EGL_NO_DISPLAY;
	m_EGLSurface = EGL_NO_SURFACE;
	m_ReleaseResourcesRefCount = 0;

	memset(&m_DynamicState, 0, sizeof(m_DynamicState));

	// Flashlight state is assumed not to be used until it's set.

	m_FramebufferColorTexture = m_FramebufferDepthTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;

	m_CurrStack = (MaterialMatrixMode_t)(-1);

#ifdef _DEBUG
	m_OcclusionQuery = INVALID_SHADERAPI_OCCLUSION_QUERY_HANDLE;
#endif

	m_DynamicVBSize = DYNAMIC_VERTEX_BUFFER_MEMORY;
	m_pMaterial = NULL;
	m_pRenderMesh = NULL;

	memset(m_FloatRenderingParameters, 0, sizeof(m_FloatRenderingParameters));
	memset(m_IntRenderingParameters, 0, sizeof(m_IntRenderingParameters));
	memset(m_VectorRenderingParameters, 0, sizeof(m_VectorRenderingParameters));

	m_InSelectionMode = false;
	m_NumHits = 0;
	m_SelectionMaxZ = FLT_MIN;
	m_SelectionMinZ = FLT_MAX;
	m_pSelectionBuffer = m_pSelectionBufferEnd = NULL;

	m_BoundProgram = SHADER_PROGRAM_HANDLE_INVALID;
	memset(&m_StandardConstUpdateFrame, 0xff, sizeof(m_StandardConstUpdateFrame));
	memset(&m_StandardConstUpdate, 0xff, sizeof(m_StandardConstUpdate));

	m_ModifyTextureHandle = INVALID_SHADERAPI_TEXTURE_HANDLE;
	for (i = TEXTURE_MAX_STD_TEXTURES; i--; )
		m_StdTextureHandles[i] = INVALID_SHADERAPI_TEXTURE_HANDLE;

	m_EnableDebugTextureList = m_DebugGetAllTextures = m_DebugTexturesRendering = false;
	m_pDebugTextureList = NULL;
	m_TextureMemoryUsedLastFrame = m_TextureMemoryUsedTotal = m_TextureMemoryUsedPicMip1 = m_TextureMemoryUsedPicMip2 = 0;
	m_DebugDataExportFrame = 0;

	ResetRenderState(true);
}

CShaderAPIOES2::~CShaderAPIOES2(void)
{
	if (m_pDebugTextureList)
	{
		m_pDebugTextureList->deleteThis();
		m_pDebugTextureList = NULL;
	}
}

void CShaderAPIOES2::Bind(IMaterial *pMaterial)
{
	IMaterialInternal *pMatInt = static_cast<IMaterialInternal *>(pMaterial);
	bool bMaterialChanged;
	if (m_pMaterial && pMatInt && m_pMaterial->InMaterialPage() && pMatInt->InMaterialPage())
	{
		bMaterialChanged = (m_pMaterial->GetMaterialPage() != pMatInt->GetMaterialPage());
	}
	else
	{
		bMaterialChanged = (m_pMaterial != pMatInt) ||
			(m_pMaterial && m_pMaterial->InMaterialPage()) || (pMatInt && pMatInt->InMaterialPage());
	}
	if (bMaterialChanged)
	{
		FlushBufferedPrimitives();
		m_pMaterial = pMatInt;
	}
}

void CShaderAPIOES2::ClearSnapshots(void)
{
	FlushBufferedPrimitives();
	m_TransitionTable.Reset();
	InitRenderState();
}

void CShaderAPIOES2::FlushHardware(void)
{
	FlushBufferedPrimitives();
	MeshMgr()->DiscardVertexBuffers();
	ForceHardwareSync();
}

ConVar mat_frame_sync_enable("mat_frame_sync_enable", "1", FCVAR_CHEAT);
void CShaderAPIOES2::ForceHardwareSync(void)
{
	if (!(mat_frame_sync_enable.GetInt()))
		return;
	FlushBufferedPrimitives();
	if (!IsDeactivated())
		glFlush();
}

float CShaderAPIOES2::GetFloatRenderingParameter(int parm_number) const
	{ return (parm_number < MAX_FLOAT_RENDER_PARMS) ? m_FloatRenderingParameters[parm_number] : 0.0f; }
int CShaderAPIOES2::GetIntRenderingParameter(int parm_number) const
	{ return (parm_number < MAX_INT_RENDER_PARMS) ? m_IntRenderingParameters[parm_number] : 0; }
Vector CShaderAPIOES2::GetVectorRenderingParameter(int parm_number) const
	{ return (parm_number < MAX_VECTOR_RENDER_PARMS) ? m_VectorRenderingParameters[parm_number] : Vector(0.0f, 0.0f, 0.0f); }
void CShaderAPIOES2::SetFloatRenderingParameter(int parm_number, float value)
{
	if (parm_number < MAX_FLOAT_RENDER_PARMS)
		m_FloatRenderingParameters[parm_number] = value;
}
void CShaderAPIOES2::SetIntRenderingParameter(int parm_number, int value)
{
	if (parm_number < MAX_INT_RENDER_PARMS)
		m_IntRenderingParameters[parm_number] = value;
}
void CShaderAPIOES2::SetVectorRenderingParameter(int parm_number, const Vector &value)
{
	if (parm_number < MAX_VECTOR_RENDER_PARMS)
		m_VectorRenderingParameters[parm_number] = value;
}

const char *OESErrorString(void)
{
	if (!(ShaderAPI()->IsUsingGraphics()))
		return "CONTEXT_LOST";
	return OESErrorString(glGetError());
}

const char *OESErrorString(unsigned int error)
{
	switch (error)
	{
	case GL_INVALID_ENUM:
		return "INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "OUT_OF_MEMORY";
	}
	return "NO_ERROR";
}

//-----------------------------------------------------------------------------
// Class factory.
//-----------------------------------------------------------------------------
static CShaderAPIOES2 s_ShaderAPIOES2;
CShaderAPIOES2 *g_pShaderAPIOES2 = &s_ShaderAPIOES2;
IShaderUtil *g_pShaderUtil;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CShaderAPIOES2, IShaderAPI, SHADERAPI_INTERFACE_VERSION, s_ShaderAPIOES2);
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CShaderAPIOES2, IShaderDevice, SHADER_DEVICE_INTERFACE_VERSION, s_ShaderAPIOES2);
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CShaderAPIOES2, IDebugTextureInfo, DEBUG_TEXTURE_INFO_VERSION, s_ShaderAPIOES2);