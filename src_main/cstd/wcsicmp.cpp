//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: MSVCRT wcsicmp for Android.
//
// $NoKeywords: $
//
//===========================================================================//

#include <wchar.h>

extern "C" int _wcsicmp(const wchar_t *string1, const wchar_t *string2)
{
	wchar_t c1, c2;
	for (; *string1; string1++, string2++)
	{
		c1 = towlower(*string1);
		c2 = towlower(*string2);
		if (c1 != c2)
			return (int)c1 - c2;
	}
	return -*string2;
}

extern "C" int _wcsnicmp(const wchar_t *string1, const wchar_t *string2, size_t count)
{
	wchar_t c1, c2;
	if (!count)
		return 0;
	for (; *string1; string1++, string2++)
	{
		c1 = towlower(*string1);
		c2 = towlower(*string2);
		if (c1 != c2)
			return (int)c1 - c2;
		if (!(--count))
			return 0;
	}
	return -*string2;
}