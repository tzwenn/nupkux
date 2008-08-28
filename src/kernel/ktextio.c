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

#include <kernel/ktextio.h>
#include <lib/stdarg.h>
#include <mm.h>
#include <kernel/dts.h>
#include <lib/string.h>
#include <drivers/tty.h>

extern fs_node *current_tty;

static int _ksetcursor(UCHAR x, UCHAR y);
static int _kout(fs_node *node, off_t offset, size_t size, const char *output);
int (*ktexto)(fs_node *,off_t,size_t,const char*) = _kout;

tty_cursor _cursor_pos;

char *mem = (char *) VIDEO_MEM_ENTRY;

extern int vsprintf(char *buf, const char *fmt, va_list args);

int printf(const char *fmt, ...)
{
	char str[STRLEN];
	int res;
	va_list ap;

	va_start(ap,fmt);
	res=vsprintf(str,fmt,ap);
	ktexto(current_tty,0,strlen(str),str);
	va_end(ap);
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

static int _ksetcursor(UCHAR x, UCHAR y)
{
	USHORT position;

	if (x==TXT_WIDTH) {
		if (y==TXT_HEIGHT-1) return 0;
		y++;
		x=0;
	}
	if (x>TXT_WIDTH) {
		if (!y) return 0;
		x=TXT_WIDTH-1;
		y--;
	}
	if (y>TXT_HEIGHT) y=0;
	if (y==TXT_HEIGHT) y--;
	position=(y*TXT_WIDTH)+x;
	outportb(0x3D4,0x0F);
	outportb(0x3D5,(USHORT) (position & 0xFF));
	outportb(0x3D4,0x0E);
	outportb(0x3D5,(USHORT) ((position>>8) & 0xFF));
	_cursor_pos.x=x;
	_cursor_pos.y=y;
	return 0;
}

int _kclear()
{
	int i=0;

	while (i<(TXT_WIDTH*TXT_HEIGHT*2)) {
		mem[i++]=' ';
		mem[i++]=TXT_COL_WHITE;
	}
	_ksetcursor(0,0);
	return 0;
}

static int _kiomove(int x, int y, int len)
{
	int i;

	for (i=2*TXT_HEIGHT*TXT_WIDTH;i>=2*(y*TXT_WIDTH+x+len-1);i--)
		mem[i]=mem[i-2*len];
	for (i=(y*TXT_WIDTH+x+len-1);i>=(y*TXT_WIDTH+x);i--) {
		mem[2*i]=' ';
		mem[2*i+1]=TXT_COL_WHITE;
	}
	return 0;
}

static int _kout(fs_node *node, off_t offset, size_t size, const char *output)
{
	UCHAR x = _cursor_pos.x, y = _cursor_pos.y;
	int i;

	while (size--) {
		switch (*output) {
		  case '\n':	_kiomove(x,y,TXT_WIDTH);
				#ifdef NEWLINE_RETURN
				for (i=2*((y+1)*TXT_WIDTH);i<2*TXT_HEIGHT*TXT_WIDTH;i++)
					mem[i]=mem[i+2*x];
				x=0;
				#endif
				y++;
				break;
		  case '\r':	x=0;
				break;
		  case '\t':	i=TAB_WIDTH-(x%TAB_WIDTH);
				_kiomove(x,y,i);
				x+=i+1;
				break;
		  case '\b':	if (!x) {
					if (!y) break;
					y--;
					x=TXT_WIDTH;
				}
				x--;
				for (i=2*(y*TXT_WIDTH+x);i<2*TXT_HEIGHT*TXT_WIDTH;i++)
					mem[i]=mem[i+2];
				break;
		  case '\v':	_kiomove(x,y,TXT_WIDTH);
				y++;
				break;
		  case '\f':	_kiomove(x,y,TXT_WIDTH);
				y++;
				break;
		  default: 	_kiomove(x,y,1);
					mem[2*(y*TXT_WIDTH+x)]=*output;
					mem[2*(y*TXT_WIDTH+x)+1]=TXT_COL_WHITE;
					x++;
					break;
		}
		if (x>=TXT_WIDTH) {
			x-=TXT_WIDTH;
			y++;
		}
		if (y>=TXT_HEIGHT) {
			y=0;
			while (y<TXT_HEIGHT-1) {
				memcpy(mem+2*(y*TXT_WIDTH),mem+2*((y+1)*TXT_WIDTH),(UINT) 2*TXT_WIDTH);
				y++;
			}
			for (y=0;y<TXT_WIDTH;y++) {
				mem[2*((TXT_HEIGHT-1)*TXT_WIDTH+y)]=' ';
				mem[2*((TXT_HEIGHT-1)*TXT_WIDTH+y)+1]=TXT_COL_WHITE;
			}
			y=TXT_HEIGHT-1;
		}
		output++;
	}
	_ksetcursor(x,y);
	return 0;
}

int _kputc(const char chr)
{
	return ktexto(current_tty,0,1,(char *)&chr);
}
