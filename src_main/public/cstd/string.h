//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: Extensions for LibC string.h.
//
// $NoKeywords: $
//
//===========================================================================//

#ifndef CSTD_STRING_H
#define CSTD_STRING_H

#include <string.h>

#ifndef _WIN32
#include <strings.h>

#ifdef __cplusplus
extern "C" {
#endif

inline int _stricmp(const char *string1, const char *string2)
{
	strcasecmp(string1, string2);
}

inline int stricmp(const char *string1, const char *string2)
{
	strcasecmp(string1, string2);
}

inline int _strnicmp(const char *string1, const char *string2, size_t count)
{
	strncasecmp(string1, string2, count);
}

inline int strnicmp(const char *string1, const char *string2, size_t count)
{
	strncasecmp(string1, string2, count);
}

#ifdef __cplusplus
}
#endif

#endif // _WIN32

#endif // CSTD_STRING_H
