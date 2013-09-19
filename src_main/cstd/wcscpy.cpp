/* Copyright (C) 1995-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.

   D0G modifications (C) 2013, SiPlus, MIT licensed. */

#include <cstd/wchar.h>

extern "C" wchar_t *d0g_wcscpy(wchar_t *dest, const wchar_t *src)
{
	wint_t c;
	wchar_t *wcp = dest;
	do
	{
		c = *(src++);
		*(wcp++) = c;
	} while (c);
	return dest;
}

extern "C" wchar_t *d0g_wcsncpy(wchar_t *dest, const wchar_t *src, size_t n)
{
	wint_t c;
	wchar_t *const s = dest--;
	if (n >= 4)
	{
		size_t n4 = n >> 2;
		for (;;)
		{
			c = *(src++);
			*(++dest) = c;
			if (!c)
				break;
			c = *(src++);
			*(++dest) = c;
			if (!c)
				break;
			c = *(src++);
			*(++dest) = c;
			if (!c)
				break;
			c = *(src++);
			*(++dest) = c;
			if (!c)
				break;
			if (!(--n4))
				goto last_chars;
		}
		n = n - (dest - s) - 1;
		if (!n)
			return s;
		goto zero_fill;
    }
last_chars:
	n &= 3;
	if (!n)
		return s;
	do
	{
		c = *(src++);
		*(++dest) = c;
		if (!(--n))
			return s;
	} while (c);
zero_fill:
	do
		*(++dest) = 0;
	while (--n);
	return s;
}