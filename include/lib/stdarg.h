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

#ifndef _STDARG_H
#define _STDARG_H

typedef char *va_list;

#define _va_sizeof(VALUE) (((sizeof(VALUE)+sizeof(int)-1)/sizeof(int))*sizeof(int))
#define va_start(AP,LAST) AP=(((char *)(&LAST)+_va_sizeof(LAST)))
#define va_arg(AP,TYPE) (AP+=_va_sizeof(TYPE), \
			*((TYPE *)(AP-_va_sizeof(TYPE))))
#define va_end(AP)

#endif
