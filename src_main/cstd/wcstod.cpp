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
#include <stdlib.h>
#include <cstd/wchar.h>

double d0g_wcstod(const wchar_t *nptr, wchar_t **endptr)
{
	while (d0g_iswspace(*nptr))
		++nptr;
	const wchar_t *c = nptr;
	if ((*c == L'+') || (*c == L'-'))
		++c;
	if ((*c == L'0') && ((c[1] | 0x20) == L'x'))
	{
		c += 2;
		while (((unsigned int)(*c - L'0') < 10) || ((unsigned int)((*c | 0x20) - L'a') < 6))
			++c;
		if (*c == L'.')
		{
			++c;
			while (((unsigned int)(*c - L'0') < 10) || ((unsigned int)((*c | 0x20) - L'a') < 6))
				++c;
		}
	}
	else
	{
		while ((unsigned int)(*c - L'0') < 10)
			++c;
		if (*c == L'.')
		{
			++c;
			while ((unsigned int)(*c - L'0') < 10)
				++c;
		}
		if ((unsigned int)((*c | 0x20) - L'd') <= 1)
		{
			++c;
			if ((*c == L'+') || (*c == L'-'))
				++c;
			while ((unsigned int)(*c - L'0') < 10)
				++c;
		}
	}
	size_t length = c - nptr;
	char *mbs = (char *)(alloca(length + 1));
	mbs[length] = 0;
	while (length--)
		mbs[length] = (char)(nptr[length]);
	char *endptr2;
	double ret = strtod((const char *)(mbs), &endptr2);
	if (endptr)
		*endptr = ((wchar_t *)nptr) + (endptr2 - mbs);
	return ret;
}