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

// To simulate the return value of MultiByteToWideChar, the buffer size must be
// checked against the return value.
// If ret<n, it's ret+1.
// If ret==n, it's ret+1 if there's null terminator or 0 if there's no.

size_t d0g_mbstowcs(wchar_t *s, const char *pmbs, size_t n)
{
	int bytesNeeded = 0;
	wchar_t c;
	size_t ret = 0;
	if (!s)
	{
		while (*pmbs)
		{
			c = *(pmbs++);
			if (bytesNeeded)
			{
				if ((c & 0xc0) != 0x80)
					return (size_t)(-1);
				--bytesNeeded;
				continue;
			}
			if (!(c & 0x80))
			{
				++ret;
				continue;
			}
			if ((c & 0xe0) == 0xc0)
			{
				bytesNeeded = 1;
				++ret;
				continue;
			}
			if ((c & 0xf0) == 0xe0)
			{
				bytesNeeded = 2;
				++ret;
				continue;
			}
			return (size_t)(-1);
		}
		if (bytesNeeded)
			return (size_t)(-1);
		return ret;
	}
	if (!n)
		return 0;
	while (*pmbs)
	{
		c = *(pmbs++);
		if (bytesNeeded)
		{
			if ((c & 0xc0) != 0x80)
				return (size_t)(-1);
			if (--bytesNeeded)
				*s |= (c & 0x3f) << 6;
			else
				*(s++) |= (c & 0x3f);
			continue;
		}
		if (ret >= n)
			return ret;
		if (!(c & 0x80))
		{
			*(s++) = c & 0x7f;
			++ret;
			continue;
		}
		if ((c & 0xe0) == 0xc0)
		{
			bytesNeeded = 1;
			*s = (c & 0x1f) << 6;
			++ret;
			continue;
		}
		if ((c & 0xf0) == 0xe0)
		{
			bytesNeeded = 2;
			*s = (c & 0x1f) << 12;
			++ret;
			continue;
		}
		return (size_t)(-1);
	}
	if (bytesNeeded)
		return (size_t)(-1);
	if (ret < n)
		*s = 0;
	return ret;
}