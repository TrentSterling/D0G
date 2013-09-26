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

#include <cstd/wchar.h>

// To simulate the return value of WideCharToMultiByte, the buffer size must be
// checked against the return value.
// If ret<n, it's ret+1.
// If ret==n, it's ret+1 if there's null terminator or 0 if there's no.

size_t d0g_wcstombs(char *s, const wchar_t *pwcs, size_t n)
{
	wchar_t c;
	size_t ret = 0;
	if (!s)
	{
		while (*pwcs)
		{
			c = *(pwcs++);
			if (c <= 0x7f)
				++ret;
			else if (c <= 0x7ff)
				ret += 2;
			else
				ret += 3;
		}
		return ret;
	}
	if (!n)
		return 0;
	while (*pwcs)
	{
		c = *(pwcs++);
		if (c <= 0x7f)
		{
			*(s++) = (char)c;
			if (++ret >= n)
				return ret;
			continue;
		}
		if (c <= 0x7ff)
		{
			*(s++) = (char)(c >> 6) | 0xc0;
			if (++ret >= n)
				return ret;
			*(s++) = (char)(c & 0x3f) | 0x80;
			if (++ret >= n)
				return ret;
			continue;
		}
		*(s++) = (char)(c >> 12) | 0xe0;
		if (++ret >= n)
			return ret;
		*(s++) = (char)((c >> 6) & 0x3f) | 0x80;
		if (++ret >= n)
			return ret;
		*(s++) = (char)(c & 0x3f) | 0x80;
		if (++ret >= n)
			return ret;
	}
	*s = 0;
	return ret;
}