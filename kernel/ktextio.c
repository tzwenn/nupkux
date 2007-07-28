#include <kernel/ktextio.h>
#include <memory.h>
#include <stdarg.h>

int _ksetcursor(UCHAR x, UCHAR y);
int _kin(char *instr, int maxlen);
int _kclear();
UCHAR _kkeyboard_layout(UCHAR key, int layout);
UCHAR _kinterpret_key(UCHAR key, int layout);

int _kout(char *output);

CURSOR_POS _cursor_pos;
CURSOR_POS _cursor_max;
UINT _line_buffer_pos = 0;
UCHAR _line_buffer[LINE_BUFFER_LEN][2*TXT_WIDTH];
UINT _line_buffer_len = 0;
char _key_states[128];

inline void outportb(USHORT port, UCHAR value)  // Output a byte to a port
{
    asm volatile ("outb %%al,%%dx"::"d" (port), "a" (value));
}

inline UCHAR inportb(USHORT port)  
{
 	UCHAR value;

	asm volatile ("inb %%dx,%%al":"=a" (value):"d"(port));
	return value;
}

int _kline_buffer_up() 
{
	if (_line_buffer_pos==_line_buffer_len) return 1;
		

	_line_buffer_pos++;
	return 1;
}

int _kline_buffer_down() 
{
	if (!_line_buffer_pos)	return 1;
	
	_line_buffer_pos--;
	return 1;
}


int _kline_buffer_reset() 
{
	while (_line_buffer_pos)
		_kline_buffer_down();
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

UCHAR _kkeyboard_layout(UCHAR key, int layout)
{
	switch (key) {
//F-Keys
/*
//		case  1: return 1; //'ESC';
		case 59 = F1;
		case 68 = F10;
		87 = F11;
		88 = F12; 
*/		
//numbers
		case 41: return '^';
		case  2: return '1';
		case  3: return '2';
		case  4: return '3';
		case  5: return '4';
		case  6: return '5';
		case  7: return '6';
		case  8: return '7';
		case  9: return '8';
		case 10: return '9';
		case 11: return '0';
		case 12: return 223;//'ß';
//		case 13: return '´';
		case 14: return '\b';
//first row
		case 15: return '\t';
		case 16: return 'q';
		case 17: return 'w';
		case 18: return 'e';
		case 19: return 'r';
		case 20: return 't';		
		case 21: return 'z';
		case 22: return 'u';
		case 23: return 'i';
		case 24: return 'o';
		case 25: return 'p';
//		case 26: return 'ü';
		case 27: return '+';
		case 28: return '\n';
//second row
		case 30: return 'a';
		case 31: return 's';
		case 32: return 'd';
		case 33: return 'f';
		case 34: return 'g';
		case 35: return 'h';
		case 36: return 'j';
		case 37: return 'k';
		case 38: return 'l';
		//case 39: return 246;//'ö';
		//case 40: return 228;//'ä';
		case 43: return '#';
//last row
		case 86: return '<';
		case 44: return 'y';
		case 45: return 'x';
		case 46: return 'c';
		case 47: return 'v';
		case 48: return 'b';
		case 49: return 'n';
		case 50: return 'm';
		case 51: return ',';
		case 52: return '.';
		case 53: return '-';
//others
		case 57: return ' ';
		default: return 0;
	}
}

UCHAR _kinterpret_key(UCHAR key, int layout)
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
			case SPEC_KEY_DELET: if ((_cursor_pos.x!=TXT_WIDTH-1) && (_cursor_pos.y!=TXT_HEIGHT-1)) {
						_ksetcursor(_cursor_pos.x+1,_cursor_pos.y);
						_kout("\b");
					     }
					     break;
		}
	key=_kkeyboard_layout(key,layout);
	if (!key) return 0;
	if (_key_states[SPEC_KEY_SHIFT]) {
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


int _ksetcursor(UCHAR x, UCHAR y)
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
        char *mem = (char *) VIDEO_MEM_ENTRY;
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

int _kiomove(int x, int y, int len)
{
	char *mem = (char *) VIDEO_MEM_ENTRY;	
	int i;

	for (i=2*TXT_HEIGHT*TXT_WIDTH;i>=2*(y*TXT_WIDTH+x+len-1);i--)
		mem[i]=mem[i-2*len];
	for (i=(y*TXT_WIDTH+x+len-1);i>=(y*TXT_WIDTH+x);i--) {
		mem[2*i]=' ';
		mem[2*i+1]=TXT_COL_WHITE;
	}
	return 0;
}

int _kin(char *instr, int maxlen)
{
	CURSOR_POS cstart = _cursor_pos;	
	char *mem = (char *) VIDEO_MEM_ENTRY;
	char keyprint[2];

	UCHAR input = 0;
	int i = 0;

	instr[0]=0;
	keyprint[0]=' ';
	keyprint[1]=0;
	input=128;
	while (input--)
		_key_states[input]=0;
	while (1) {
		input=inportb(0x60);
		if (input<128) {
			if (!_key_states[input]) {
				if ((!maxlen) && (input!=SPEC_KEY_RETURN)) continue;
				keyprint[0]=_kinterpret_key(input,KEYBOARD_LAY_DE);
				_key_states[input]=1;
				if (keyprint[0]=='\n') continue;
				_kout(keyprint);
			}
		} else if (_key_states[input-128]) {
			if ((!maxlen) && (input-128!=SPEC_KEY_RETURN)) continue;
			keyprint[0]=_kinterpret_key(input-128,KEYBOARD_LAY_DE);
			_key_states[input-128]=0;
			if (keyprint[0]=='\n') break;
		}
		if (_cursor_pos.y<cstart.y) _ksetcursor(_cursor_pos.x,cstart.y);
		if ((_cursor_pos.y==cstart.y) && (_cursor_pos.x<cstart.x)) _ksetcursor(cstart.x,_cursor_pos.y);
	}
	while ((instr) && (i<maxlen-1) && (i<(_cursor_pos.y*TXT_WIDTH+_cursor_pos.x)-(cstart.y*TXT_WIDTH+cstart.x))) {
		instr[i]=mem[2*(cstart.y*TXT_WIDTH+cstart.x+i)];
		instr[++i]=0;
	}
	#ifdef NEWLINE_KIN
	_kout("\n");
	#endif
	return 0;
}

int _kout(char *output) 
{
	char *mem = (char *) VIDEO_MEM_ENTRY;
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
				/*mem[2*(y*TXT_WIDTH+x)]=' ';
				mem[2*(y*TXT_WIDTH+x)+1]=TXT_COL_WHITE;*/	
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
			y=LINE_BUFFER_LEN;
			while (y--) 
				memcpy((_line_buffer+2*y*TXT_WIDTH),(_line_buffer+2*(y-1)*TXT_WIDTH),(UINT) 2*TXT_WIDTH);
			memcpy(_line_buffer,mem,(UINT) 2*TXT_WIDTH);
			if (_line_buffer_len<LINE_BUFFER_LEN-1) _line_buffer_len++;
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
