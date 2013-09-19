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

extern "C" wchar_t *d0g_wcscat(wchar_t *dest, const wchar_t *src)
{
	wchar_t *s1 = dest;
	wchar_t c;
	do
		c = *(s1++);
	while (c);
	s1 -= 2;
	do
	{
		c = *(src++);
		*(++s1) = c;
	} while (c);
	return dest;
}

extern "C" wchar_t *d0g_wcsncat(wchar_t *dest, const wchar_t *src, size_t n)
{
	wchar_t c;
	wchar_t *const s = dest;
	do
		c = *(dest++);
	while (c);
	dest -= 2;
	if (n >= 4)
	{
		size_t n4 = n >> 2;
		do
		{
			c = *(src++);
			*(++dest) = c;
			if (!c)
				return s;
			c = *(src++);
			*(++dest) = c;
			if (!c)
				return s;
			c = *(src++);
			*(++dest) = c;
			if (!c)
				return s;
			c = *(src++);
			*(++dest) = c;
			if (!c)
				return s;
		} while (--n4 > 0);
		n &= 3;
	}
	while (n)
	{
		c = *(src++);
		*(++dest) = c;
		if (!c)
			return s;
		--n;
	}
	if (c)
		*(++dest) = 0;
	return s;
}