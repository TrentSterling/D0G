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

extern "C" wchar_t *d0g_wcsstr(const wchar_t *haystack, const wchar_t *needle)
{
	wchar_t b, c;
	if ((b = *needle))
	{
		--haystack;
		do
		{
			if (!(c = *(++haystack)))
				goto ret0;
		} while (c != b);
		if (!(c = *(++needle)))
			goto foundneedle;
		++needle;
		goto jin;
		for (;;)
		{
			wchar_t a;
			const wchar_t *rhaystack, *rneedle;
			do
			{
				if (!(a = *(++haystack)))
					goto ret0;
				if (a == b)
					break;
				if (!(a = *(++haystack)))
					goto ret0;
shloop: ;
			} while (a != b);
jin:
			if (!(a = *(++haystack)))
				goto ret0;
			if (a != c)
				goto shloop;
			if (*(rhaystack = haystack-- + 1) == (a = *(rneedle = needle)))
			{
				do
				{
					if (!a)
						goto foundneedle;
					if (*++rhaystack != (a = *++needle))
						break;
					if (!a)
						goto foundneedle;
				} while (*++rhaystack == (a = *(++needle)));
			}
			needle = rneedle;
			if (!a)
				break;
		}
	}
foundneedle:
	return (wchar_t *)(haystack);
ret0:
	return 0;
}