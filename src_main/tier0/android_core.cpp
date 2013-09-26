//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//============= D0G modifications © 2013, SiPlus, MIT licensed. =============//
//
// Purpose: Android language
//
//===========================================================================//

#ifdef __ANDROID__

#include <sys/system_properties.h>
#include "tier0/platform.h"
#include "android_core.h"

#define ANDR_LANGUAGE(l, h) ((l) | ((h) << 8))

struct ANDR_Language_t
{
	unsigned short m_Name;
	const char *m_ShortName;
};

static ANDR_Language_t s_ANDR_Languages[] =
{
	{ANDR_LANGUAGE('e', 'n'), "english"},
	{ANDR_LANGUAGE('d', 'e'), "german"},
	{ANDR_LANGUAGE('f', 'r'), "french"},
	{ANDR_LANGUAGE('i', 't'), "italian"},
	{ANDR_LANGUAGE('k', 'o'), "koreana"},
	{ANDR_LANGUAGE('e', 's'), "spanish"},
	{ANDR_LANGUAGE('z', 'h'), "tchinese"}, // Traditional because of no Google Play in the mainland
	{ANDR_LANGUAGE('r', 'u'), "russian"},
	{ANDR_LANGUAGE('t', 'h'), "thai"},
	{ANDR_LANGUAGE('j', 'a'), "japanese"},
	{ANDR_LANGUAGE('p', 't'), "portuguese"},
	{ANDR_LANGUAGE('p', 'l'), "polish"},
	{ANDR_LANGUAGE('d', 'a'), "danish"},
	{ANDR_LANGUAGE('n', 'l'), "dutch"},
	{ANDR_LANGUAGE('f', 'i'), "finnish"},
	{ANDR_LANGUAGE('n', 'b'), "norwegian"},
	{ANDR_LANGUAGE('s', 'v'), "swedish"}
};

const char *ANDR_GetLanguageString(void)
{
	char language[PROP_VALUE_MAX];
	int length = __system_property_get("persist.sys.language", language);
	if (length == 2)
	{
		int i;
		unsigned short name = ANDR_LANGUAGE(language[0], language[1]);
		for (i = 0; i < (sizeof(s_ANDR_Languages) / sizeof(ANDR_Language_t)); ++i)
		{
			if (s_ANDR_Languages[i].m_Name == name)
				return s_ANDR_Languages[i].m_ShortName;
		}
	}
	return "english";
}

#endif