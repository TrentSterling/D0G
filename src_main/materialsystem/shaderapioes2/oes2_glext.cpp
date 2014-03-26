//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2014, SiPlus, MIT licensed. =============//
//
// Purpose: Used OpenGL ES 2.0 extension function pointers.
//
//===========================================================================//
#include "oes2.h"
#include "oes2_glext.h"
// NOTE: This must be the last file included!
#include "tier0/memdbgon.h"

// EXT_occlusion_query_boolean
#ifndef SHADERAPIOES3
PFNGLGENQUERIESEXTPROC glextGenQueriesEXT;
PFNGLDELETEQUERIESEXTPROC glextDeleteQueriesEXT;
PFNGLBEGINQUERYEXTPROC glextBeginQueryEXT;
PFNGLENDQUERYEXTPROC glextEndQueryEXT;
PFNGLGETQUERYOBJECTUIVEXTPROC glextGetQueryObjectuivEXT;
#endif

// OES_texture_3D
#ifndef SHADERAPIOES3
PFNGLTEXIMAGE3DOESPROC glextTexImage3DOES;
PFNGLTEXSUBIMAGE3DOESPROC glextTexSubImage3DOES;
#endif