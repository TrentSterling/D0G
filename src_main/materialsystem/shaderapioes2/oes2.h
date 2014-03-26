//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES 2.0 shader API globals.
//
//===========================================================================//
#ifndef OES2_H
#define OES2_H

#include "tier0/dbg.h"
#include "tier0/icommandline.h"
#include "tier0/platform.h"
#include "tier0/vprof.h"
#include "tier1/convar.h"
#include "mathlib/vector.h"
#include "materialsystem/itexture.h"
#include "materialsystem/materialsystem_config.h"
#include "shaderapi/shareddefs.h"
#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishaderdevice.h"
#include "shaderapi/ishaderdynamic.h"
#include "shaderapi/ishaderutil.h"
#include "../materialsystem/imaterialinternal.h"

extern IShaderUtil *g_pShaderUtil;
inline IShaderUtil *ShaderUtil(void) { return g_pShaderUtil; }

typedef int ShaderProgramHandle_t;
#define SHADER_PROGRAM_HANDLE_INVALID 0

#include "materialsystem/ishaderoes2.h"
#include "oes2_hardwareconfig.h"
#include "oes2_shadow.h"
#include "oes2_transition.h" // ^ ShadowState_t
#include "oes2_api.h" // ^ CTransitionTable
#include "oes2_devicemgr.h"
#include "oes2_mesh.h"

//-----------------------------------------------------------------------------
// Memory debugging (not supported on Android yet).
//-----------------------------------------------------------------------------
#ifdef _DEBUG
#define MEM_ALLOC_OES_CREDIT() MEM_ALLOC_CREDIT_("OES:" __FILE__)
#define BEGIN_OES_ALLOCATION() MemAlloc_PushAllocDbgInfo("OES:" __FILE__, __LINE__)
#define END_OES_ALLOCATION() MemAlloc_PopAllocDbgInfo()
#else
#define MEM_ALLOC_OES_CREDIT() ((void)0)
#define BEGIN_OES_ALLOCATION() ((void)0)
#define END_OES_ALLOCATION() ((void)0)
#endif

#endif // !OES2_H