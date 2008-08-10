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

#ifndef _KERNEL_H
#define _KERNEL_H

#define true	1
#define false	0

#define _kaborted	0xFF

typedef unsigned char  byte;
typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned int   UINT;
typedef unsigned long  ULONG;

typedef unsigned int size_t;
typedef unsigned int off_t;

extern char _kabort_func;

#define _kabort_func_break()	if (_kabort_func) { \
					_kabort_func=0; \
					break; \
				}
#define _kabort_func_return(val) if (_kabort_func) { \
					_kabort_func=0; \
					return val; \
				}

#define sti() asm volatile ("sti\n\t")
#define cli() asm volatile ("cli\n\t")
#define hlt() asm volatile ("hlt\n\t")

#endif
