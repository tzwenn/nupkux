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

#ifndef _TTY_H
#define _TTY_H

#include <drivers/drivers.h>
#include <lib/string.h>

#define VIDEO_MEM	0xB8000
#define TTY_WIDTH	80
#define TTY_HEIGHT	25
#define TTY_LINES	75	//25 visible lines + 50 linebuffer
#define TTY_ESCAPE_LEN	128
#define TTY_INBUF_LEN	16

#define TTY_COL_BLACK	0
#define TTY_COL_BLUE	1
#define TTY_COL_GREEN	2
#define TTY_COL_CYAN	3
#define TTY_COL_RED	4
#define TTY_COL_MAGENTA	5
#define TTY_COL_BROWN	6
#define TTY_COL_GRAY	7
#define TTY_COL_BRIGHT	8

#define TTY_COL_FG_DEF	7
#define TTY_COL_BG_DEF	0

#define NR_TTYS		4

typedef struct {
  char x;
  char y;
} tty_cursor;

typedef struct _tty {
	UCHAR is_esc:1;
	UCHAR show_cursor:1;
	UCHAR echo: 1;
	UCHAR nr: 5;
	USHORT width,height,memlines,viewln,scrln,esc_ind,attr;
	USHORT *mem,in_s,in_e;
	devfs_handle *dev;
	tty_cursor cursor,_cursor;
	USHORT input_buffer[TTY_INBUF_LEN];
	char escape_seq[TTY_ESCAPE_LEN];
} tty;

extern int setup_tty(void);
extern devfs_handle *set_current_tty(int nr);

#endif
