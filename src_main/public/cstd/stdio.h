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

#ifndef CSTD_STDIO_H
#define CSTD_STDIO_H

#include <stdio.h>

#ifndef _WIN32
#include <stdarg.h>

inline int _snprintf(char *buffer, size_t count, const char *format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	int ret = vsnprintf(buffer, count, format, argptr);
	va_end(argptr);
	return ret;
}

inline int _vsnprintf(char *buffer, size_t count, const char *format, va_list argptr)
{
	return vsnprintf(buffer, count, format, argptr);
}

#endif // !_WIN32

#endif // CSTD_STDIO_H
