//===== Copyright © 1996-2013, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: OpenGL ES header inclusion.
//
//===========================================================================//
#ifndef OES2_GL_H
#define OES2_GL_H

#ifdef SHADERAPIOES3 // Compiler -Definition - needs Android API 18 at least.
#include <GLES3/gl3.h>
#else
#include <GLES2/gl2.h>
#endif

#endif // !OES2_GL_H