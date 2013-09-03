//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: Extensions for LibC stdio.h.
//
// $NoKeywords: $
//
//===========================================================================//

#ifndef CSTD_STDIO_H
#define CSTD_STDIO_H

#include <stdio.h>

#ifndef _WIN32
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

inline int _snprintf(char *buffer, size_t count, const char *format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buffer, count, format, argptr);
	va_end(argptr);
}

inline int _vsnprintf(char *buffer, size_t count, const char *format, va_list argptr)
{
	vsnprintf(buffer, count, format, argptr);
}

#ifdef __cplusplus
}
#endif

#endif // !_WIN32

#endif // CSTD_STDIO_H
