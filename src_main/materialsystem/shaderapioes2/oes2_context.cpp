//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: EGL and OpenGL ES context management.
//
//===========================================================================//
#include <android/window.h>
#include <EGL/egl.h>
#include "android_system.h"
#include "oes2.h"
#include "oes2_gl.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

void CShaderAPIOES2::EnableVSync_360(bool bEnable)
{
	if (!m_DeviceActive || (m_WaitForVSync == bEnable))
		return;
	m_WaitForVSync = bEnable;
	eglSwapInterval(m_EGLDisplay, bEnable ? 1 : 0);
}

void CShaderAPIOES2::InitDevice(void)
{
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int w, h, oldw, oldh;
	ConVarRef mat_vsync("mat_vsync");

	ANativeWindow *window = ANDR_GetApp()->window;
	if (!window)
		return;

	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (display == EGL_NO_DISPLAY)
	{
		Warning("CShaderAPIOES2::InitContext: eglGetDisplay failed.\n");
		return;
	}
	if (!eglInitialize(display, NULL, NULL))
	{
		Warning("CShaderAPIOES2::InitContext: eglInitialize failed.\n");
		return;
	}

	const int attribs[] =
	{
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_STENCIL_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};
	const int attribsDepth16[] = // For devices that don't support 24-bit depth buffers.
	{
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_STENCIL_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};
	EGLConfig config;
	int numConfigs, format;
	if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs) || !numConfigs)
	{
		if (!eglChooseConfig(display, attribsDepth16, &config, 1, &numConfigs) || !numConfigs)
		{
			Warning("CShaderAPIOES2::InitContext: eglChooseConfig failed.\n");
			goto fail_config;
		}
	}
	if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format))
	{
		Warning("CShaderAPIOES2::InitContext: eglGetConfigAttrib failed.\n");
		goto fail_config;
	}
	ANativeWindow_setBuffersGeometry(window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, window, NULL);
	if (surface == EGL_NO_SURFACE)
	{
		Warning("CShaderAPIOES2::InitContext: eglCreateWindowSurface failed.\n");
		goto fail_config;
	}

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
		context = eglCreateContext(display, config, NULL, contextAttribs);
		if (context == EGL_NO_CONTEXT)
		{
			Warning("CShaderAPIOES2::InitContext: eglCreateContext failed.\n");
			goto fail_surface;
		}
	}
	if (!eglMakeCurrent(display, surface, surface, context))
	{
		Warning("CShaderAPIOES2::InitContext: eglMakeCurrent failed.\n");
		goto fail_context;
	}

	w = h = 0;
	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);
	if (!w || !h) // Shouldn't happen really, but still, it's better to check.
	{
		Assert(0);
		w = 640;
		h = 480;
	}

	ANativeActivity_setWindowFlags(ANDR_GetApp()->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);

	m_EGLDisplay = display;
	m_EGLSurface = surface;
	m_EGLContext = context;
	oldw = m_BackBufferWidth, oldh = m_BackBufferHeight;
	m_BackBufferWidth = w;
	m_BackBufferHeight = h;
	m_DeviceActive = true;
	m_WaitForVSync = mat_vsync.GetBool();
	if (!m_WaitForVSync)
		eglSwapInterval(display, 0);

	HardwareConfig()->DetermineHardwareCaps();

	{
		int i;
		VMatrix m;
		m.Identity();
		for (i = NUM_MATRIX_MODES; i--; )
			m_MatrixStack[i].Push(m);
	}
	ShaderShadow()->Init();
	InitRenderState();
	ClearBuffers(true, true, true, -1, -1);
	// MeshMgr()->Init() and ResetRenderState() are done by ReacquireResources.
	ReacquireResourcesInternal(true, true, "NeedsReset", (w != oldw) || (h != oldh));
	return;

fail_context:
	eglDestroyContext(display, context);
fail_surface:
	eglDestroySurface(display, surface);
fail_config:
	eglTerminate(display);
}

void CShaderAPIOES2::Present(void)
{
	if (IsDeactivated())
		return;
	FlushBufferedPrimitives();
	if (!(eglSwapBuffers(m_EGLDisplay, m_EGLSurface)) && (eglGetError() == EGL_CONTEXT_LOST))
	{
		ShutdownDevice();
		InitDevice();
	}
	MeshMgr()->DiscardVertexBuffers();
	if ((m_DeviceActive) && (ShaderUtil()->GetConfig().bMeasureFillRate || ShaderUtil()->GetConfig().bVisualizeFillRate))
		ClearBuffers(true, true, true, -1, -1);
}

void CShaderAPIOES2::ReacquireResourcesInternal(bool resetState, bool forceReacquire, const char *forceReason, bool resize)
{
	if (forceReacquire)
	{
		if (m_ReleaseResourcesRefCount > 1)
		{
			Warning("Forcefully resetting device (%s), resources release level was %d.\n",
				forceReason ? forceReason : "unspecified", m_ReleaseResourcesRefCount);
			Assert(0);
		}
		m_ReleaseResourcesRefCount = 0;
	}
	else if (--m_ReleaseResourcesRefCount)
	{
		Warning("ReacquireResources has no effect, now at level %d.\n", m_ReleaseResourcesRefCount);
		DevWarning("ReacquireResources being discarded is a bug: use IsDeactivated to check for a valid device.\n");
		Assert(0);
		if (m_ReleaseResourcesRefCount < 0)
			m_ReleaseResourcesRefCount = 0;
		return;
	}
	if (resetState)
		ResetRenderState(false);
	MeshMgr()->RestoreBuffers();
	if (resize)
		g_pShaderDeviceMgrOES2->InvokeModeChangeCallbacks();
	ShaderUtil()->RestoreShaderObjects(CShaderDeviceMgrOES2::ShaderInterfaceFactory);
}

void CShaderAPIOES2::ReleaseResources(void)
{
	if (m_ReleaseResourcesRefCount++)
	{
		Warning("ReleaseResources has no effect, now at level %d.\n", m_ReleaseResourcesRefCount);
		DevWarning("ReleaseResources called twice is a bug: use IsDeactivated to check for a valid device.\n");
		Assert(0);
		return;
	}
	ShaderUtil()->ReleaseShaderObjects();
	MeshMgr()->ReleaseBuffers();
}

bool CShaderAPIOES2::SetMode(void *hwnd, int nAdapter, const ShaderDeviceInfo_t &info)
{
	Assert(!nAdapter);
	ShutdownDevice();
	// hwnd!=0 => APP_CMD_INIT_WINDOW, hwnd==0 => APP_CMD_TERM_WINDOW.
	if (hwnd)
	{
		Assert(hwnd == ANDR_GetApp());
		InitDevice();
	}
	return m_DeviceActive;
}

void CShaderAPIOES2::ShutdownDevice(void)
{
	if (!m_DeviceActive)
		return;

	// Yo Source, drop it hard!
	int i;
	m_DeviceActive = false;
	ReleaseResources();
	for (i = TEXTURE_MAX_STD_TEXTURES; i--; )
		m_StdTextureHandles[i] = INVALID_SHADERAPI_TEXTURE_HANDLE;
	// Textures are unbound in ResetShaderState.
	m_Textures.Purge();
	m_Framebuffers.Purge();
	for (i = NUM_MATRIX_MODES; i--; )
		m_MatrixStack[i].Purge();
	m_TransitionTable.Reset();
	MeshMgr()->Shutdown();
	m_Programs.Purge();

	if (m_EGLDisplay == EGL_NO_DISPLAY)
		return;
	eglMakeCurrent(m_EGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	if (m_EGLContext != EGL_NO_CONTEXT)
	{
		eglDestroyContext(m_EGLDisplay, m_EGLContext);
		m_EGLContext = EGL_NO_CONTEXT;
	}
	if (m_EGLSurface != EGL_NO_SURFACE)
	{
		eglDestroySurface(m_EGLDisplay, m_EGLSurface);
		m_EGLSurface = EGL_NO_SURFACE;
	}
	eglTerminate(m_EGLDisplay);
	m_EGLDisplay = EGL_NO_DISPLAY;
}