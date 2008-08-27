/*
 *  Copyright (C) 2008 Sven Köhler
 *
 *  This file is part of Nupkux.
 *
 *  Nupkux is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Nupkux is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Nupkux.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdarg.h>

static FILE __stdin=STDIN_FILENO;
static FILE __stdout=STDOUT_FILENO;
static FILE __stderr=STDERR_FILENO;

FILE *stdin = &__stdin;
FILE *stdout = &__stdout;
FILE *stderr = &__stderr;

int getc(FILE *stream)
{
	char res;
	read(*stream,&res,1);
	return res;
}

extern int vsprintf(char *buf, const char *fmt, va_list args);

#define BUFSIZE 1024

int printf(const char *fmt, ...)
{
	char str[BUFSIZE]={0,};
	int res;
	va_list ap;

	va_start(ap,fmt);
	res=vsprintf(str,fmt,ap);
	va_end(ap);
	write(STDOUT_FILENO,str,res);
	return res;
}

int sprintf(char *str, const char *fmt, ...)
{
	int res;
	va_list ap;

	va_start(ap,fmt);
	res=vsprintf(str,fmt,ap);
	va_end(ap);
	return res;
}
