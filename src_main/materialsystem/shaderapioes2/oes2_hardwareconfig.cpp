//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 hardware configuration info.
//
//===========================================================================//
#include <cstd/string.h>
#include <EGL/egl.h>
#include "oes2.h"
#include "oes2_gl.h"
#include "oes2_glext.h"
#include "tier0/icommandline.h"
#include "tier1/checksum_crc.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

void CHardwareConfig::DetermineHardwareCaps(void)
{
	if (m_CapsDetermined)
		return;
	m_CapsDetermined = true;

	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &m_Caps.m_MaxCubeMapTextureSize);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_Caps.m_MaxTextureSize);
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &m_Caps.m_MaxRenderTargetSize);
	// Make max render target size power-of-2. 
	int bits = 0;
	while (m_Caps.m_MaxRenderTargetSize > 1)
	{
		m_Caps.m_MaxRenderTargetSize >>= 1;
		++bits;
	}
	if (bits < 8) // In case of a broken driver. 256x256 should be supported everywhere.
		bits = 8;
	m_Caps.m_MaxRenderTargetSize = 1 << bits;
	if (m_Caps.m_MaxRenderTargetSize > m_Caps.m_MaxTextureSize)
		m_Caps.m_MaxRenderTargetSize = m_Caps.m_MaxTextureSize;
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &m_Caps.m_NumPixelShaderConstants);
	glGetIntegerv(GL_MAX_VARYING_VECTORS, &m_Caps.m_NumPixelShaderInputs);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &m_Caps.m_NumVertexShaderConstants);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_Caps.m_NumVertexShaderInputs);
	if (m_Caps.m_NumVertexShaderInputs > OES2_SHADER_INPUT_COUNT)
		m_Caps.m_NumVertexShaderInputs = OES2_SHADER_INPUT_COUNT;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_Caps.m_NumPixelShaderSamplers);
	if (m_Caps.m_NumPixelShaderSamplers > OES2_SHADER_MAX_PIXEL_SAMPLERS)
		m_Caps.m_NumPixelShaderSamplers = OES2_SHADER_MAX_PIXEL_SAMPLERS;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &m_Caps.m_NumVertexShaderSamplers);
	if (m_Caps.m_NumVertexShaderSamplers > OES2_SHADER_MAX_VERTEX_SAMPLERS)
		m_Caps.m_NumVertexShaderSamplers = OES2_SHADER_MAX_VERTEX_SAMPLERS;
	m_Caps.m_NumCombinedSamplers = m_Caps.m_NumPixelShaderSamplers + m_Caps.m_NumVertexShaderSamplers;

	const char *vendor = (const char *)(glGetString(GL_VENDOR));
	if (vendor)
		m_Caps.m_VendorID = (unsigned int)(CRC32_ProcessSingleBuffer(vendor, strlen(vendor)));
	else
		vendor = "";

	const char *renderer = (const char *)(glGetString(GL_RENDERER));
	if (renderer)
		m_Caps.m_DeviceID = (unsigned int)(CRC32_ProcessSingleBuffer(renderer, strlen(renderer)));
	else
		renderer = "";

	Q_snprintf(m_Caps.m_pDriverName, MATERIAL_ADAPTER_NAME_LENGTH, "%s %s", vendor, renderer);

	if (!(CommandLine()->CheckParm("-noglextensions")))
	{
		const char *ext = (const char *)(glGetString(GL_EXTENSIONS));
		if (ext)
		{
			if (strstr(ext, "GL_EXT_texture_filter_anisotropic"))
			{
				float maxAnisotropy;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
				m_Caps.m_MaxAnisotropy = (int)maxAnisotropy;
			}

#ifdef SHADERAPIOES3
			{
#else
			if (strstr(ext, "GL_OES_texture_3D"))
			{
				glextTexImage3DOES = (PFNGLTEXIMAGE3DOESPROC)(eglGetProcAddress("glTexImage3DOES"));
				glextTexSubImage3DOES = (PFNGLTEXSUBIMAGE3DOESPROC)(eglGetProcAddress("glTexSubImage3DOES"));
#endif
				glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE_OES, &m_Caps.m_MaxTextureDepth);
			}

#ifndef SHADERAPIOES3
			if (strstr(ext, "GL_OES_depth24"))
			{
				m_Caps.m_DepthStencilFormat = GL_DEPTH_COMPONENT24_OES;
				m_Caps.m_DepthTextureFormat = GL_UNSIGNED_INT;
			}
			else
			{
				m_Caps.m_DepthTextureFormat = GL_UNSIGNED_SHORT;
			}
			if (strstr(ext, "GL_OES_packed_depth_stencil"))
			{
				m_Caps.m_DepthStencilFormat = GL_DEPTH24_STENCIL8_OES;
				m_Caps.m_DepthTextureFormat = GL_UNSIGNED_INT_24_8_OES;
			}
			if (!strstr(ext, "GL_OES_depth_texture"))
				m_Caps.m_DepthTextureFormat = 0;
#endif

			if (strstr(ext, "GL_NV_occlusion_query_samples"))
				m_Caps.m_OcclusionQuery = GL_SAMPLES_PASSED_NV;
#ifndef SHADERAPIOES3
			else if (strstr(ext, "GL_EXT_occlusion_query_boolean"))
				m_Caps.m_OcclusionQuery = GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT;
			if (m_Caps.m_OcclusionQuery)
			{
				glextGenQueriesEXT = (PFNGLGENQUERIESEXTPROC)(eglGetProcAddress("glGenQueriesEXT"));
				glextDeleteQueriesEXT = (PFNGLDELETEQUERIESEXTPROC)(eglGetProcAddress("glDeleteQueriesEXT"));
				glextBeginQueryEXT = (PFNGLBEGINQUERYEXTPROC)(eglGetProcAddress("glBeginQueryEXT"));
				glextEndQueryEXT = (PFNGLENDQUERYEXTPROC)(eglGetProcAddress("glEndQueryEXT"));
				glextGetQueryObjectuivEXT = (PFNGLGETQUERYOBJECTUIVEXTPROC)(eglGetProcAddress("glGetQueryObjectuivEXT"));
			}
#endif

			m_Caps.m_SupportsBorderColor = (strstr(ext, "GL_NV_texture_border_clamp") != NULL);

			if (strstr(ext, "GL_EXT_texture_compression_s3tc") || strstr(ext, "GL_NV_texture_compression_s3tc"))
				m_Caps.m_SupportsCompressedTextures = COMPRESSED_TEXTURES_DXT;
			else if (strstr(ext, "GL_EXT_texture_compression_dxt1") || strstr(ext, "GL_NV_texture_compression_dxt1"))
				m_Caps.m_SupportsCompressedTextures = COMPRESSED_TEXTURES_DXT1;
		}
		else
		{
			Warning("CHardwareConfig::DetermineHardwareCaps: glGetString(GL_EXTENSIONS) failed.\n");
		}
	}

#ifndef SHADERAPIOES3
	int range[2], precision;
	glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, range, &precision);
	m_Caps.m_SupportsFragmentHighPrecision = (range[0] && range[1] && precision);
#endif
}

void CHardwareConfig::DetermineHardwareCapsFromPbuffer(void)
{
	if (m_CapsDetermined)
		return;
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (display == EGL_NO_DISPLAY)
		return;
	if (eglInitialize(display, NULL, NULL))
	{
		const int configAttribs[] =
		{
			EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_NONE
		};
		EGLConfig config;
		int numConfigs;
		if (eglChooseConfig(display, configAttribs, &config, 1, &numConfigs) && numConfigs)
		{
			const int attribs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
			EGLSurface surface = eglCreatePbufferSurface(display, config, attribs);
			if (surface != EGL_NO_SURFACE)
			{
				const int contextAttribs[] =
				{
#ifdef SHADERAPIOES3
					EGL_CONTEXT_CLIENT_VERSION, 3,
#else
					EGL_CONTEXT_CLIENT_VERSION, 2,
#endif
					EGL_NONE
				};
				EGLContext context = eglCreateContext(display, config, NULL, contextAttribs);
				if (context)
				{
					if (eglMakeCurrent(display, surface, surface, context))
					{
						DetermineHardwareCaps();
						eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
					}
					eglDestroyContext(display, context);
				}
				eglDestroySurface(display, surface);
			}
		}
		eglTerminate(display);
	}
}

void CShaderAPIOES2::SpewDriverInfo(void) const
{
	if (!HardwareConfig()->CapsDetermined())
		Warning("SHADER API DRIVER INFO NOT DETERMINED!\n\n");
	const HardwareCaps_t &caps = HardwareConfig()->Caps();

	Warning("Shader API Driver Info:\n\nDriver Description: %s\nVendor CRC: %08x\nRenderer CRC: %08x\n",
		caps.m_pDriverName, caps.m_VendorID, caps.m_DeviceID);
	Warning("OpenGL ES version: %d.%d\n", caps.m_nDriverVersionHigh, caps.m_nDriverVersionLow);

	int width, height;
	GetWindowSize(width, height);
	Warning("\nDisplay mode: %d x %d\n", width, height);

	Warning("\nSHADERAPI CAPS:\n");
	Warning("m_MaxAnisotropy: %d\n", caps.m_MaxAnisotropy);
	Warning("m_MaxCubeMapTextureSize: %d\n", caps.m_MaxCubeMapTextureSize);
	Warning("m_NumCombinedSamplers: %d\n", caps.m_NumCombinedSamplers);
	Warning("m_MaxRenderTargetSize: %d\n", caps.m_MaxRenderTargetSize);
	Warning("m_MaxTextureDepth: %d\n", caps.m_MaxTextureDepth);
	Warning("m_MaxTextureSize: %d\n", caps.m_MaxTextureSize);
	Warning("m_NumPixelShaderConstants: %d\n", caps.m_NumPixelShaderConstants);
	Warning("m_NumPixelShaderInputs: %d\n", caps.m_NumPixelShaderInputs);
	Warning("m_NumPixelShaderSamplers: %d\n", caps.m_NumPixelShaderSamplers);
	Warning("m_NumVertexShaderConstants: %d\n", caps.m_NumVertexShaderConstants);
	Warning("m_NumVertexShaderInputs: %d\n", caps.m_NumVertexShaderInputs);
	Warning("m_NumVertexShaderSamplers: %d\n", caps.m_NumVertexShaderSamplers);
	Warning("m_DepthStencilFormat: ");
	switch (caps.m_DepthStencilFormat)
	{
	case GL_DEPTH_COMPONENT16: Warning("DEPTH_COMPONENT16\n"); break;
	case GL_DEPTH_COMPONENT24_OES: Warning("DEPTH_COMPONENT24\n"); break;
	case GL_DEPTH24_STENCIL8_OES: Warning("DEPTH24_STENCIL8\n"); break;
	NO_DEFAULT;
	}
	Warning("m_DepthTextureFormat: ");
	switch (caps.m_DepthTextureFormat)
	{
	case 0: Warning("no\n"); break;
	case GL_UNSIGNED_SHORT: Warning("UNSIGNED_SHORT\n"); break;
	case GL_UNSIGNED_INT: Warning("UNSIGNED_INT\n"); break;
	case GL_UNSIGNED_INT_24_8_OES: Warning("UNSIGNED_INT_24_8\n"); break;
	NO_DEFAULT;
	}
	Warning("m_OcclusionQuery: ");
	switch (caps.m_OcclusionQuery)
	{
	case 0: Warning("no\n"); break;
	case GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT: Warning("ANY_SAMPLES_PASSED_CONSERVATIVE\n"); break;
	case GL_SAMPLES_PASSED_NV: Warning("SAMPLES_PASSED\n"); break;
	NO_DEFAULT;
	}
	Warning("m_ShaderDLL: %s\n", caps.m_ShaderDLL);
	Warning("m_SupportsCompressedTextures: ");
	switch (caps.m_SupportsCompressedTextures)
	{
	case COMPRESSED_TEXTURES_OFF: Warning("no\n"); break;
	case COMPRESSED_TEXTURES_DXT1: Warning("DXT1\n"); break;
	case COMPRESSED_TEXTURES_DXT: Warning("DXT\n"); break;
	NO_DEFAULT;
	}
	Warning("m_SupportsBorderColor: %s\n", caps.m_SupportsBorderColor ? "yes" : "no");
	Warning("m_SupportsFragmentHighPrecision: %s\n", caps.m_SupportsFragmentHighPrecision ? "yes" : "no");
}

static CHardwareConfig s_HardwareConfig;
CHardwareConfig *g_pHardwareConfig = &s_HardwareConfig;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CHardwareConfig, IMaterialSystemHardwareConfig, 
	MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION, s_HardwareConfig)