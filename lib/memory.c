/*
 *  Copyright (C) 2007,2008 Sven Köhler
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

#include <lib/memory.h>

void *memcpy(void *target, const void *src, size_t count)
{	
	char *tmp = (char *) target, *source = (char *)src;
	while (count--) {
		*tmp=*source;
		tmp++;
		source++;
	}
	return target;
}

void *memset(void *target, int value, size_t count)
{
	char *tmp = (char *)target;
	while (count--) *(tmp++)=value;
	return target;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	if (n) {
		register const UCHAR *p1 = s1, *p2 = s2;
		do {
			if (*p1++ != *p2++)
				return (*--p1 - *--p2);
		} while (--n);
	}
	return 0;
}
