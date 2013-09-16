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

size_t d0g_wcslen(const wchar_t *s)
{
	const wchar_t *wcp = s;
	while (*s)
	{
		if (!(*(++s)))
			break;
		if (!(*(++s)))
			break;
		if (!(*(++s)))
			break;
		++s;
	}
	return s - wcp;
}

size_t d0g_wcsnlen(const wchar_t *s, size_t maxlen)
{
	const wchar_t *wcp = s;
	while (maxlen && *s)
	{
		++s;
		if (!(--maxlen && *s))
			break;
		++s;
		if (!(--maxlen && *s))
			break;
		++s;
		if (!(--maxlen && *s))
			break;
		++s;
		--maxlen;
	}
	return s - wcp;
}