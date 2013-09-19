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

// The original GLibC code uses towlower, but towupper is probably faster
// because it can exit the loop early for lowercase characeters (they're
// obviously more common than uppercase ones) and is 1 range shorter.

extern "C" int d0g_wcscasecmp(const wchar_t *s1, const wchar_t *s2)
{
	wchar_t c1, c2;
	if (s1 == s2)
		return 0;
	do
	{
		c1 = d0g_towupper(*(s1++));
		c2 = d0g_towupper(*(s2++));
		if (!c1)
			return c1 - c2;
	} while (c1 == c2);
	return c1 - c2;
}

extern "C" int d0g_wcsncasecmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
	wchar_t c1, c2;
	if ((s1 == s2) || !n)
		return 0;
	do
	{
		c1 = d0g_towupper(*(s1++));
		c2 = d0g_towupper(*(s2++));
		if (!c1 || (c1 != c2))
			return c1 - c2;
	} while (--n);
	return c1 - c2;
}