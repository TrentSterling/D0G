//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: Extensions for LibC wchar.h.
//
// $NoKeywords: $
//
//===========================================================================//

#ifndef CSTD_WCHAR_H
#define CSTD_WCHAR_H

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
extern int _wcsicmp(const wchar_t *string1, const wchar_t *string2);

inline int wcsicmp(const wchar_t *string1, const wchar_t *string2)
{
	_wcsicmp(string1, string2);
}

extern int _wcsnicmp(const wchar_t *string1, const wchar_t *string2, size_t count);

inline int wcsnicmp(const wchar_t *string1, const wchar_t *string2, size_t count)
{
	_wcsnicmp(string1, string2, count);
}
#endif // !_WIN32

#ifdef __cplusplus
}
#endif

#endif // CSTD_WCHAR_H
