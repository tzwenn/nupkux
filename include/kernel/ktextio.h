/*
	Kernel Textmode Output
*/

#ifndef _KTEXTIO_H
#define _KTEXTIO_H

#include <squaros.h>

#define NEWLINE_RETURN
#define NEWLINE_KIN
#define STRLEN		255

#define VIDEO_MEM_ENTRY	0xB8000

#define TXT_COL_WHITE	0x07

#define TXT_WIDTH	80	
#define TXT_HEIGHT	25
#define LINE_BUFFER_LEN	50//818

#define TAB_WIDTH	8

#define KEYBOARD_LAY_DE 10

#define SPEC_KEY_ESC 	1
#define SPEC_KEY_F1 	59
#define SPEC_KEY_F10 	68
#define SPEC_KEY_RETURN 28
#define SPEC_KEY_SHIFT	54
#define SPEC_KEY_ALTGR  56
#define SPEC_KEY_CTRLL  29
#define SPEC_KEY_CTRLR  29
#define SPEC_KEY_SUPRL	91
#define SPEC_KEY_SUPRR	91
#define SPEC_KEY_POPUP  93
#define SPEC_KEY_ROLLN  70
#define SPEC_KEY_INSRT  82
#define SPEC_KEY_DELET  83
#define SPEC_KEY_HOME  	71
#define SPEC_KEY_NEXT  	73
#define SPEC_KEY_PRIOR  81
#define SPEC_KEY_END  	79
#define SPEC_KEY_LEFT  	75
#define SPEC_KEY_RIGHT  77
#define SPEC_KEY_UP	72
#define SPEC_KEY_DOWN	80

typedef struct {
  UCHAR x;
  UCHAR y;
} CURSOR_POS;

extern inline void outportb(USHORT port, UCHAR value);
extern inline UCHAR inportb(USHORT port);
extern int _ksetcursor(UCHAR x, UCHAR y);
extern int _kin(char *instr, int maxlen);
extern int _kclear();
extern int printf(const char *fmt, ...);
extern UCHAR _kinterpret_key(UCHAR key, int layout);

extern CURSOR_POS _cursor_pos;
extern CURSOR_POS _cursor_max;
extern UINT _line_buffer_pos;
extern UINT _line_buffer_len;
extern UCHAR _line_buffer[LINE_BUFFER_LEN][TXT_WIDTH*2];
extern char _key_states[128];

#endif

