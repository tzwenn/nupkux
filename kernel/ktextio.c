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

static int _ksetcursor(UCHAR x, UCHAR y);
int _kin(char *instr, int maxlen);
int _kclear();
static UCHAR _kkeyboard_layout(UCHAR key, int layout);
static UCHAR _kinterpret_key(UCHAR key, int layout);

static int _kout(char *output);

CURSOR_POS _cursor_pos;
CURSOR_POS _cursor_max;
UINT _line_buffer_pos = 0, _line_buffer_end = 0;
UCHAR *_line_buffer = 0;
char _key_states[128];
char _print_pressed_keys = 0;

char *mem = (char *) VIDEO_MEM_ENTRY;

inline void outportb(USHORT port, UCHAR value) 
{
    asm volatile ("outb %%al,%%dx"::"d" (port), "a" (value));
}

inline UCHAR inportb(USHORT port)  
{
 	UCHAR value;

	asm volatile ("inb %%dx,%%al":"=a" (value):"d"(port));
	return value;
}

inline void outportw(USHORT port, USHORT value) 
{
    asm volatile ("outw %%ax,%%dx"::"d"(port), "a"(value));
}

inline USHORT inportw(USHORT port)  
{
 	USHORT value;

	asm volatile ("inw %%dx,%%ax":"=a"(value):"d"(port));
	return value;
}

static int _kline_buffer_up() 
{
	if (((_line_buffer_end>=0) && (_line_buffer_pos==_line_buffer_end)) || 
	   ((_line_buffer_end<0) && (_line_buffer_pos==-_line_buffer_end-1) )) return 0;
	
	_line_buffer_pos++;
	printf("+(%d)",_line_buffer_pos);	
	return 1;
}

static int _kline_buffer_down() 
{
	if (!_line_buffer_pos)	return 0;

	_line_buffer_pos--;
	printf("-(%d)",_line_buffer_pos);
	return 1;
}


static int _kline_buffer_reset() 
{
	//while (_kline_buffer_down());
	return 1;
}

extern int vsprintf(char *buf, const char *fmt, va_list args);

int printf(const char *fmt, ...)
{
	char str[STRLEN];
	int res;
	va_list ap;

	va_start(ap,fmt);
	res=vsprintf(str,fmt,ap);
	_kout(str);
	va_end(ap);
	return res;
}

void setup_ktexto()
{
	_line_buffer=(UCHAR *)calloc(2*TXT_WIDTH,LINE_BUFFER_LEN); //becomes obsolete if we have got tty
}

int str2d(char *str)
{
	int res = 0;

	while (*str) {
		res=10*(res)+(*str-48);
		str++;
	}
	return res;
}

static UCHAR _kkeyboard_layout(UCHAR key, int layout)
{
	UCHAR german_keymap[90] = 	{0,1,'1','2','3','4','5','6','7','8','9','0',223/*ß*/,0/*´*/,'\b',
					'\t','q','w','e','r','t','z','u','i','o','p',0/*ü*/,'+','\n',
					0,'a','s','d','f','g','h','j','k','l',0/*ö*/,0/*ä*/,'^',0,'#',
					'y','x','c','v','b','n','m',',','.','-',
					0,0,0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,'<',0,0,0};

	if ((layout==KEYBOARD_LAY_DE) && (key<=90)) return german_keymap[key];
	return 0;
}

extern void reboot();

static UCHAR _kinterpret_key(UCHAR key, int layout)
{
	if (_key_states[key])
		switch (key) {
			case SPEC_KEY_LEFT:  _ksetcursor(_cursor_pos.x-1,_cursor_pos.y);
					     break;
			case SPEC_KEY_RIGHT: _ksetcursor(_cursor_pos.x+1,_cursor_pos.y);
					     break;
			case SPEC_KEY_UP:    _ksetcursor(_cursor_pos.x,_cursor_pos.y-1);
					     break;
			case SPEC_KEY_DOWN:  _ksetcursor(_cursor_pos.x,_cursor_pos.y+1);
					     break;
			case SPEC_KEY_HOME:  if (_key_states[SPEC_KEY_CTRLL]) _ksetcursor(0,0);
						else _ksetcursor(0,_cursor_pos.y);
					     break;
			case SPEC_KEY_END:   if (_key_states[SPEC_KEY_CTRLL]) _ksetcursor(TXT_WIDTH-1,TXT_HEIGHT-1);
						else _ksetcursor(TXT_WIDTH-1,_cursor_pos.y);
					     break;
			case SPEC_KEY_NEXT:  if (_key_states[SPEC_KEY_SHIFTL] || _key_states[SPEC_KEY_SHIFTR]) _kline_buffer_up();
					     break;
			case SPEC_KEY_PRIOR: if (_key_states[SPEC_KEY_SHIFTL] || _key_states[SPEC_KEY_SHIFTR]) _kline_buffer_down();
					     break;
			case SPEC_KEY_DELET: if (_key_states[SPEC_KEY_CTRLL] && _key_states[SPEC_KEY_ALT]) {
						reboot();
						return 0;
					     }
					     if ((_cursor_pos.x!=TXT_WIDTH-1) && (_cursor_pos.y!=TXT_HEIGHT-1)) {
						_ksetcursor(_cursor_pos.x+1,_cursor_pos.y);
						_kout("\b");
					     }
					     break;
			case 46: 	     if (_key_states[SPEC_KEY_CTRLL]) {
						_kabort_func=1;  //^C
	 				        return 0;
					     }
					     break;
		}
	key=_kkeyboard_layout(key,layout);
	if (!key) return 0;
	if (_key_states[SPEC_KEY_SHIFTL] || _key_states[SPEC_KEY_SHIFTR]) {
        	if ((key>=97) && (key<=123)) return key & 0xDF;
		switch (key) {
			case '1': return '!';
			case '2': return '"';
			//case '3': return 167;//'§';
			case '4': return '$';
			case '5': return '%';
			case '6': return '&';
			case '7': return '/';
			case '8': return '(';
			case '9': return ')';
			case '0': return '=';
			case 223: return '?'; //(223=ß)
			case '+': return '*';
			case '#': return '\'';
			case ',': return ';';
			case '.': return ':';
			case '-': return '_';
			//case '^': return 176;//'°';
			case '<': return '>';
		}
	}
	if (_key_states[SPEC_KEY_ALTGR]) {
		switch (key) {
			case 'q': return '@';
			case '7': return '{';
			case '8': return '[';
			case '9': return ']';
			case '0': return '}';
			case 223: return '\\';
			case '+': return '~';
			case '<': return '|';

		}
	}
	if (key>127) return 0;
	return key;
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

	_kline_buffer_reset();
        while (i<(TXT_WIDTH*TXT_HEIGHT*2)) {
        	mem[i]=' ';
                i++;
                mem[i]=TXT_COL_WHITE;
                i++;
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

void irq_keyboard(registers regs)
{
	UCHAR input = inportb(0x60);
	char keyprint[2] = " ";

	if (!(input & 0x80)) {
		if (!_key_states[input]) {
			_key_states[input]=1;
			keyprint[0]=_kinterpret_key(input,KEYBOARD_LAY_DE);
			if ((_print_pressed_keys) && (keyprint[0]!='\n')) _kout(keyprint);
		}
	} else if (_key_states[input-128]) {
		_key_states[input-128]=0;
	}
}

void setup_input()
{
	int i = 0;

	register_interrupt_handler(IRQ1,&irq_keyboard);
	i=128;
	while (i--)
		_key_states[i]=0;
}

int _kin(char *instr, int maxlen)
{
	CURSOR_POS cstart = _cursor_pos;	

	int i = 0;

	_print_pressed_keys=1;
	instr[0]=0;
	while (!_key_states[SPEC_KEY_RETURN]) {
		_kabort_func_return(_kaborted);
		if (_cursor_pos.y<cstart.y) _ksetcursor(_cursor_pos.x,cstart.y);
		if ((_key_states[SPEC_KEY_DOWN]) && (_cursor_pos.y>cstart.y)) _ksetcursor(_cursor_pos.x,cstart.y);
		if ((_cursor_pos.y==cstart.y) && (_cursor_pos.x<cstart.x)) _ksetcursor(cstart.x,_cursor_pos.y);
	}
	_print_pressed_keys=0;
	while (_key_states[SPEC_KEY_RETURN]);
	while ((instr) && (i<maxlen-1) && (i<(_cursor_pos.y*TXT_WIDTH+_cursor_pos.x)-(cstart.y*TXT_WIDTH+cstart.x))) {
		instr[i]=mem[2*(cstart.y*TXT_WIDTH+cstart.x+i)];
		instr[++i]=0;
	}
	#ifdef NEWLINE_KIN
	_kout("\n");
	#endif
	return 0;
}

static int _kout(char *output)
{
	UCHAR x = _cursor_pos.x, y = _cursor_pos.y;
	int i;

	_kline_buffer_reset();
	while (*output) {
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
		  default:	if (*output>=32) {
					_kiomove(x,y,1);
					mem[2*(y*TXT_WIDTH+x)]=*output;
					mem[2*(y*TXT_WIDTH+x)+1]=TXT_COL_WHITE;
					x++;
				}
				break;
		} 
		if (x>=TXT_WIDTH) {
			x-=TXT_WIDTH;
			y++;
		}
		if (y>=TXT_HEIGHT) {
			/*ToDo: line_buffer*/
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
	char buf[2] = {chr,0};
	return _kout(buf);
}
