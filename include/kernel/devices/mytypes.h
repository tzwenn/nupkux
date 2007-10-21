/*
 * mytypes.h
 * 
 * assorted portability-inducing datatypes
 * 
 * Copyright (C) 1998  Fabian Nunez
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * The author can be reached by email at: fabian@cs.uct.ac.za
 * 
 * or by airmail at: Fabian Nunez
 *                   10 Eastbrooke
 *                   Highstead Road
 *                   Rondebosch 7700
 *                   South Africa
 */

#ifndef MYTYPES_H_
#define MYTYPES_H_

/* hardware datatypes */
typedef unsigned char BYTE;           /* 8-bit byte */
typedef unsigned short WORD;          /* 16-bit word */
typedef unsigned long DWORD;          /* 32-bit dword */
typedef unsigned long ADDR;           /* address that should not be deref'd */

/* integer types */
typedef unsigned char UINT8;          /* 8-bit unsigned integer */
typedef signed char INT8;             /* 8-bit signed integer */
typedef unsigned short UINT16;        /* 16-bit unsigned integer */
typedef signed short INT16;           /* 16-bit signed integer */
typedef unsigned long UINT32;         /* 32-bit unsigned integer */
typedef signed long INT32;            /* 32-bit signed integer */ 

// boolean types
typedef unsigned int BOOL;
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef true
#define true 1
#define false 0
#endif
 
/* logical datatypes */
typedef unsigned char CHAR;           /* ISO 8859-1 character */
typedef unsigned char *STRPTR;        /* C-style NUL-terminated string */
// typedef enum { FALSE=0,TRUE=1 } BOOL; /* boolean value */

/* constants */
#ifndef NULL
#define NULL ((void *)0)
#endif /* NULL */

/* useful macros */
#define BITFIELD(name,width) unsigned int name : width
#define HIBYTE(x) ((BYTE)((x) >> 8))
#define LOBYTE(x) ((BYTE)((x) & 0xff))
#define ABS(x) ((x) < 0 ? -(x) : (x))    /* NB: multiple evaluations! */

#endif /* MYTYPES_H_ */
