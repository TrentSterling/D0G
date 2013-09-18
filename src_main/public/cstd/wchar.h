/* Copyright (c) 2013 SiPlus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

// This is a very limited implementation of C wchar for en-US locale, with
// UTF-16 being the wchar codepage and UTF-8 being the multibyte codepage.
// It must be used with -DWCHAR_MAX=(65535) -DWCHAR_MIN=(0) -fshort-wchar
// C compiler options because Linux systems use 32-bit wchar by default.
// This library also contains mbstowcs and wcstombs, which are supposed to
// be located in stdlib, so if they are used, cstd/wchar must be included
// AFTER stdlib.

#ifndef CSTD_WCHAR_H
#define CSTD_WCHAR_H

#ifdef _WIN32
#include <wchar.h>
#include <wctype.h>
#else

#ifndef _WCHAR_H_
#define _WCHAR_H_
#endif

#include <malloc.h>
#include <stdarg.h>
#include <stddef.h>
#include <time.h>

#ifdef wint_t
#undef wint_t
#endif
#define wint_t wchar_t

#ifdef WINT_MAX
#undef WINT_MAX
#endif
#ifdef WINT_MIN
#undef WINT_MIN
#endif
#define WINT_MAX ((wchar_t)(0xffff))
#define WINT_MIN ((wchar_t)(0))

#ifdef WEOF
#undef WEOF
#endif
#define WEOF ((wchar_t)(0xffff))

#ifdef __cplusplus
extern "C" {
#endif
extern int               d0g_iswcntrl(wchar_t);
#define                      iswcntrl d0g_iswcntrl
extern int               d0g_iswdigit(wchar_t);
#define                      iswdigit d0g_iswdigit
extern int               d0g_iswspace(wchar_t);
#define                      iswspace d0g_iswspace
extern int               d0g_iswxdigit(wchar_t);
#define                      iswxdigit d0g_iswxdigit

extern size_t            d0g_mbstowcs(wchar_t *, const char *, size_t);
#define                      mbstowcs d0g_mbstowcs
// THIS IS AN AWFUL HACK FOR ASCII ONLY - DO NOT USE WIDE STRINGS IN FORMATS!!!
// REPLACE WIDE STRING FORMATTING WITH WCSCPY/WCSCAT ON SIGHT!!!
extern int               d0g_swprintf(wchar_t *, size_t, const wchar_t *, ...);
#define                      swprintf d0g_swprintf
#define                    _snwprintf d0g_swprintf
extern wchar_t           d0g_towlower(wchar_t);
#define                      towlower d0g_towlower
extern wchar_t           d0g_towupper(wchar_t);
#define                      towupper d0g_towupper
// THIS IS AN AWFUL HACK!!! READ SWPRINTF COMMENT!!!
extern int               d0g_vswprintf(wchar_t *, size_t, const wchar_t *, va_list);
#define                      vswprintf d0g_vswprintf
#define                    _vswnprintf d0g_vswprintf

extern wchar_t          *d0g_wcscat(wchar_t *, const wchar_t *);
#define                      wcscat d0g_wcscat
extern int               d0g_wcscasecmp(const wchar_t *, const wchar_t *);
#define                      wcscasecmp d0g_wcscasecmp
#define                      wcsicmp d0g_wcscasecmp
#define                     _wcsicmp d0g_wcscasecmp
inline wchar_t          *d0g_wcschr(const wchar_t *wcs, wchar_t wc)
{
	do
	{
		if (*wcs == wc)
			return (wchar_t *)(wcs);
	} while (*(wcs++));
	return 0;
}
#define                      wcschr d0g_wcschr
extern int               d0g_wcscmp(const wchar_t *, const wchar_t *);
#define                      wcscmp d0g_wcscmp
extern wchar_t          *d0g_wcscpy(wchar_t *, const wchar_t *);
#define                      wcscpy d0g_wcscpy
extern size_t            d0g_wcslen(const wchar_t *);
#define                      wcslen d0g_wcslen
extern wchar_t          *d0g_wcsncat(wchar_t *, const wchar_t *, size_t);
#define                      wcsncat d0g_wcsncat
extern int               d0g_wcsncmp(const wchar_t *, const wchar_t *, size_t);
#define                      wcsncmp d0g_wcsncmp
extern int               d0g_wcsncasecmp(const wchar_t *, const wchar_t *, size_t);
#define                      wcsncasecmp d0g_wcsncasecmp
#define                      wcsnicmp d0g_wcsncasecmp
#define                     _wcsnicmp d0g_wcsncasecmp
extern wchar_t          *d0g_wcsncpy(wchar_t *, const wchar_t *, size_t);
#define                      wcsncpy d0g_wcsncpy
extern size_t            d0g_wcsnlen(const wchar_t *, size_t);
#define                      wcsnlen d0g_wcsnlen
extern wchar_t          *d0g_wcsstr(const wchar_t *, const wchar_t *);
#define                      wcsstr d0g_wcsstr
#define                      wcswcs d0g_wcsstr
extern double            d0g_wcstod(const wchar_t *, wchar_t **);
#define                      wcstod d0g_wcstod
extern size_t            d0g_wcstombs(char *, const wchar_t *, size_t);
#define                      wcstombs d0g_wcstombs
#ifdef __cplusplus
}
#endif

#endif // _WIN32

#endif // CSTD_WCHAR_H