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

int d0g_wcscmp(const char *s1, const char *s2)
{
	wchar_t c1, c2;
	do
	{
		c1 = *(s1++);
		c2 = *(s2++);
		if (!c2)
			return c1 - c2;
	} while (c1 == c2);
	return c1 < c2 ? -1 : 1;
}

int d0g_wcsncmp(const char *s1, const char *s2, size_t n)
{
	wint_t c1 = 0, c2 = 0;
	if (n >= 4)
	{
		size_t n4 = n >> 2;
		do
		{
			c1 = (wint_t)(*(s1++));
			c2 = (wint_t)(*(s2++));
			if (!c1 || (c1 != c2))
				return c1 - c2;
			c1 = (wint_t)(*(s1++));
			c2 = (wint_t)(*(s2++));
			if (!c1 || (c1 != c2))
				return c1 - c2;
			c1 = (wint_t)(*(s1++));
			c2 = (wint_t)(*(s2++));
			if (!c1 || (c1 != c2))
				return c1 - c2;
			c1 = (wint_t)(*(s1++));
			c2 = (wint_t)(*(s2++));
			if (!c1 || (c1 != c2))
				return c1 - c2;
		} while (--n4 > 0);
		n &= 3;
	}
	while (n > 0)
	{
		c1 = (wint_t)(*(s1++));
		c2 = (wint_t)(*(s2++));
		if (!c1 || (c1 != c2))
			return c1 - c2;
		--n;
	}
	return c1 - c2;
}