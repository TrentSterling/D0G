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

int d0g_iswcntrl(wchar_t c)
{
	return (c <= 0x1f) ||
		((unsigned int)(c - 0x7f) <= 0x20) ||
		((unsigned int)(c - 0x200b) <= 0x4) ||
		((unsigned int)(c - 0x202a) <= 0x4) ||
		((unsigned int)(c - 0xfff9) <= 0x2);
}

int d0g_iswdigit(wchar_t c)
{
	if ((c == 0xb2) || (c == 0xb3) || (c == 0xb9))
		return 1;
	int r = c & 0xf;
	c >>= 4;
	int check09 = r <= 0x9;
	if (check09 && ((c == 0x3) || (c == 0xa62) || (c == 0xa8d) ||
		(c == 0xa90) || (c == 0xa9d) || (c == 0xaa5) || (c == 0xabf)))
		return 1;
	if ((r >= 0x6) && ((c == 0x194) || (c == 0x1d4) || (((unsigned int)(c - 0x96) <= 0x40) && ((c & 7) == 6)))) // 6-f digits
		return 1;
	if ((c < 0x66) || (c > 0x1c5))
		return 0;
	static const unsigned int ranges09[] = { // Bit array defining does a range contain 0-9 digits
		0x400201, 0x0, 0x0, 0x80000000, 0x40001080, 0x8, 0x0, 0x0, 0x9000000, 0x800000, 0xc020800c
	};
	return check09 && (ranges09[(c - 0x66) >> 5] & (1 << ((c - 6) & 31)));
}

int d0g_iswspace(wchar_t c)
{
	return ((unsigned int)(c - 0x9) <= 0x4) ||
		(c == 0x20) || (c == 0xa0) ||
		(c == 0x1680) || (c == 0x180e) ||
		((unsigned int)(c - 0x2000) <= 0xa) ||
		(c == 0x2028) || (c == 0x2029) ||
		(c == 0x205f) || (c == 0x3000);
}

int d0g_iswxdigit(wchar_t c)
{
	return ((unsigned int)(c - L'0') < 10) || ((unsigned int)((c | 0x20) - L'a') < 6);
}