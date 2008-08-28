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

#include <drivers/tty.h>
#include <kernel/dts.h>
#include <task.h>
#include "keymap.h"
#include <kernel/ktextio.h>

#define KEYBD_PORT	0x60

extern fs_node *current_tty;
extern keymap_t german_keymap;
keymap_t *keymap=&german_keymap;

static int key_state=0;
static int shift=0;
static int alt=0;
static int altgr=0;
static int ctrl=0;
static int escape=0;

static void create_key_state(void)
{
	key_state=0;
	if (shift)	key_state=1;
	if (alt)	key_state+=2;
	if (altgr)	key_state=3;
	if (ctrl)	key_state=5;
}

extern int tty_write(fs_node *node, off_t offset, size_t size, const char *buffer);

static void handle_key(UCHAR *scan_code, USHORT *key, int up)
{
	//tty *atty=(tty *)current_tty->p_data;
	*key=(*keymap)[(*scan_code&0x7F)*KEYMAP_STATES+key_state];
	if (escape) {
		if (*key==ALT) altgr=!up;
		*scan_code|=KEYUP;
		escape=0;
	}
	if (*scan_code==0xE0) escape=1;
	int i;
	switch (*key) {
		case SHIFT:
			shift=!up;
			create_key_state();
			break;
		case CTRL:
			ctrl=!up;
			create_key_state();
			break;
		case ALT:
			alt=!up;
			create_key_state();
			break;
		case CALOCK:
			shift=!shift;
			create_key_state();
			break;
		case C('C'): //TODO
			break;
		case AF1:
		case AF2:
		case AF3:
		case AF4:
		case AF5:
		case AF6:
		case AF7:
		case AF8:
		case AF9:
		case AF10:
		case AF11:
		case AF12:
			if (!(*scan_code&KEYUP)) {
				i=((*key)&(~ALT))-0x10;
				*scan_code|=KEYUP;
				set_current_tty(i);
			}
			*scan_code|=KEYUP;
			break;
	}
	if (*key&0xFF00) *scan_code|=KEYUP;
}

void irq_tty_keyboard(registers *regs)
{
	UCHAR scan_code = inportb(KEYBD_PORT);
	USHORT input;
	tty *atty=(tty *)current_tty->p_data;
	handle_key(&scan_code,&input,scan_code&KEYUP);
	if (escape) return;
	if (!(scan_code&KEYUP)) {
		atty->input_buffer[atty->in_e++]=input;
		if (atty->in_e==TTY_INBUF_LEN) atty->in_e=0;
		if (atty->in_e==atty->in_s)  //Buffer overflow
			if (++atty->in_s==TTY_INBUF_LEN) atty->in_s=0;
	}
}
