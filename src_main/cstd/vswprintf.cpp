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

#include <alloca.h>
#include <stdarg.h>
#include <cstd/stdio.h>
#include <cstd/wchar.h>

// THIS IS AN AWFUL HACK FOR ASCII ONLY - DO NOT USE WIDE STRINGS IN FORMATS!!!
// REPLACE WIDE STRING FORMATTING WITH WCSCPY/WCSCAT ON SIGHT!!!

extern "C" int d0g_vsnwprintf(wchar_t *s, size_t n, const wchar_t *format, va_list args)
{
	if (!n)
		return 0;
	size_t size = d0g_wcstombs(NULL, format, 0);
	if (!size)
	{
		s[0] = 0;
		return 0;
	}
	char *mbs = (char *)(alloca(size + 1));
	d0g_wcstombs(mbs, format, size + 1);
	char *c = (char *)s;
	int ret = vsnprintf(c, n, mbs, args);
	if (ret < 0)
		return ret;
	int i = ret;
	while (i--)
		s[i] = c[i];
	if (ret < n)
		s[ret] = 0;
	return ret;
}

extern "C" int d0g_vswprintf(wchar_t *s, const wchar_t *format, va_list args)
{
	size_t size = d0g_wcstombs(NULL, format, 0);
	if (!size)
	{
		s[0] = 0;
		return 0;
	}
	char *mbs = (char *)(alloca(size + 1));
	d0g_wcstombs(mbs, format, size + 1);
	char *c = (char *)s;
	int ret = vsprintf(c, mbs, args);
	if (ret < 0)
		return ret;
	int i = ret;
	while (i--)
		s[i] = c[i];
	s[ret] = 0;
	return ret;
}

extern "C" int d0g_snwprintf(wchar_t *s, size_t n, const wchar_t *format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	int ret = d0g_vsnwprintf(s, n, format, argptr);
	va_end(argptr);
	return ret;
}

extern "C" int d0g_swprintf(wchar_t *s, const wchar_t *format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	int ret = d0g_vswprintf(s, format, argptr);
	va_end(argptr);
	return ret;
}