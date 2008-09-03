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

/*
 * I don't provide a real tty here, its just a crappy console called "tty"
 * I have to work on an terminos struct
 */

#include <drivers/tty.h>
#include <kernel/dts.h>
#include <mm.h>
#include <errno.h>

#define TTY_POS(TTY,X,Y)	((TTY)->width*((TTY)->scrln+(Y)-(((TTY)->scrln+(Y)>=(TTY)->memlines)?(TTY)->memlines:0))+(X))
#define is_digit(C)		((UINT) ((C)-'0')<10u)
#define TTY_ESCAPE_END(C)	((C)>0x40 && (C)<0x7E)
#define TTY_ESCAPE_CHR(C)	(TTY_ESCAPE_END(C) || (C)==';' || (C)=='?' || is_digit(C))

static UCHAR ansi_col_map[8] = {TTY_COL_BLACK,TTY_COL_RED,TTY_COL_GREEN,TTY_COL_BROWN,TTY_COL_BLUE,TTY_COL_MAGENTA,TTY_COL_CYAN,TTY_COL_GRAY};
fs_node *current_tty = 0;
extern int (*ktexto)(fs_node *,off_t,size_t,const char*);
extern tty_cursor _cursor_pos;
extern void irq_tty_keyboard(registers *regs);

tty *ttys[NR_TTYS] = {0,};

extern int sprintf(char *str, const char *fmt, ...);

static int atoi(const char *str)
{
	int res = 0;

	while (*str) {
		if is_digit(*str)
			res=10*(res)+(*str-'0');
		str++;
	}
	return res;
}

static void tty_showcusor(tty *atty)
{
	USHORT pos;

	if (atty->show_cursor && atty->viewln==atty->scrln) pos=atty->cursor.y*atty->width+atty->cursor.x;
		else pos=atty->height*atty->width;
	outportb(0x3D4,0x0F);
	outportb(0x3D5,(UCHAR)(pos&0xFF));
	outportb(0x3D4,0x0E);
	outportb(0x3D5,(UCHAR)(pos>>8));
}

static void print_tty(void)
{
	tty *atty=(tty *)device_pdata(current_tty);

	if (atty->memlines-atty->viewln<atty->height) {
		memcpy((char *)VIDEO_MEM,(char *)(atty->mem+atty->viewln*atty->width),(atty->memlines-atty->viewln)*atty->width*sizeof(USHORT));
		memcpy((char *)(VIDEO_MEM+(atty->memlines-atty->viewln)*atty->width*sizeof(USHORT)),atty->mem,(atty->height-(atty->memlines-atty->viewln))*atty->width*sizeof(USHORT));
	} else memcpy((char *)VIDEO_MEM,(char *)(atty->mem+atty->viewln*atty->width),atty->height*atty->width*sizeof(USHORT));
	tty_showcusor(atty);
}

static void tty_select_graphic_rendition(tty *atty, int n)
{
	UCHAR fg=((atty->attr >> 8) & 0xFF),bg=((atty->attr >> 12) & 0xFF);
	if (!n) {
		fg=TTY_COL_FG_DEF;
		bg=TTY_COL_BG_DEF;
	} else if (n>=30 && n<=37) {
		fg=ansi_col_map[n-30];
	} else if (n==39) {
		fg=TTY_COL_FG_DEF;
	} else if (n>=40 && n<=47) {
		bg=ansi_col_map[n-40];
	} else if (n==49) {
		bg=TTY_COL_BG_DEF;
	} else if (n>=90 && n<=97) {
		fg=ansi_col_map[n-90] | TTY_COL_BRIGHT;
	} else if (n==99) {
		fg=TTY_COL_FG_DEF | TTY_COL_BRIGHT;
	} else if (n>=100 && n<=107) {
		bg=ansi_col_map[n-100] | TTY_COL_BRIGHT;
	} else if (n==109) {
		bg=TTY_COL_BG_DEF  | TTY_COL_BRIGHT;
	}
	atty->attr=(bg << 12) | (fg << 8);
}

static void tty_interpret_escape(tty *atty) //According to http://en.wikipedia.org/wiki/ANSI_escape_code
{
	char *params=atty->escape_seq+1, *sep, cmd=atty->escape_seq[atty->esc_ind];
	atty->escape_seq[atty->esc_ind]=0;
	int n,m,tmp;
	if (!*params) n=1;
		else n=atoi(params);
	switch (cmd) {
		case 'A':
			if (n>=atty->cursor.y) atty->cursor.y=0;
				else atty->cursor.y-=n;
			break;
		case 'B':
			if (atty->cursor.y+n>=atty->height) atty->cursor.y=atty->height-1;
				else atty->cursor.y+=n;
			break;
		case 'C':
			if (atty->cursor.x+n>=atty->width) atty->cursor.x=atty->width-1;
				else atty->cursor.x+=n;
			break;
		case 'D':
			if (n>=atty->cursor.x) atty->cursor.x=0;
				else atty->cursor.x-=n;
			break;
		case 'E':
			if (atty->cursor.y+n>=atty->height) atty->cursor.y=atty->height-1;
				else atty->cursor.y+=n;
			atty->cursor.x=0;
			break;
		case 'F':
			if (n>=atty->cursor.y) atty->cursor.y=0;
				else atty->cursor.y-=n;
			atty->cursor.x=0;
			break;
		case 'G':
			if (!n) n=1;
				else if (n>atty->width) n=atty->width;
			atty->cursor.x=n-1;
			break;
		case 'H':
		case 'f':
			if ((sep=strchr(params,';'))) {
				*(sep++)=0;
				if (!*sep) m=1;
					else m=atoi(sep);
				if (!*params) n=1;
					else n=atoi(params);
			} else m=1;
			if (n<1) n=1;
				else if (n>atty->width) n=atty->width;
			if (m<1) m=1;
				else if (m>atty->height) m=atty->height;
			atty->cursor.x=n-1;
			atty->cursor.y=m-1;
			break;
		case 'J':
		case 'K':
			if (!*params) n=0;
			tmp=atty->attr | 0x20;
			switch (n) {
				case 0:
					memsetw((USHORT *)(atty->mem+TTY_POS(atty,atty->cursor.x,atty->cursor.y)),tmp,atty->width-atty->cursor.x);
					if (cmd=='J')
						for (m=atty->cursor.y+1;m<atty->height;m++)
							memsetw((USHORT *)(atty->mem+TTY_POS(atty,0,m)),tmp,atty->width);
					break;
				case 1:
					memsetw((USHORT *)(atty->mem+TTY_POS(atty,0,atty->cursor.y)),tmp,atty->cursor.x+1);
					if (cmd=='J')
						for (m=0;m<atty->cursor.y;m++)
							memsetw((USHORT *)(atty->mem+TTY_POS(atty,0,m)),tmp,atty->width);
					break;
				case 2:
					if (cmd=='J')
						for (m=0;m<atty->height;m++)
							memsetw((USHORT *)(atty->mem+TTY_POS(atty,0,m)),tmp,atty->width);
						else memsetw((USHORT *)(atty->mem+TTY_POS(atty,0,atty->cursor.y)),tmp,atty->width);
					break;
			}
			break;
		case 'S': //Not implemented
			break;
		case 'T': //Not implemented
			break;
		case 'm':
			while ((sep=strchr(params,';'))) {
				*sep=0;
				tty_select_graphic_rendition(atty,atoi(params)); //atoi(NULL)=0
				params=sep+1;
			}
			tty_select_graphic_rendition(atty,atoi(params));
			break;
		case 'n':
			if (n==6) {
				//"Input" not implemented
			}
			break;
		case 's':
			atty->_cursor=atty->cursor;
			break;
		case 'u':
			atty->cursor=atty->_cursor;
			break;
		case 'l':
			if (!strcmp(params,"?25")) atty->show_cursor=0;
			break;
		case 'h':
			if (!strcmp(params,"?25")) atty->show_cursor=1;
			break;
		case 'e': //This is not ANSI but I'm missing good ioctrl with tcgetattr
			atty->echo=(n)?1:0;
			break;
	}
}

int tty_write(fs_node *node, off_t offset, size_t size, const char *buffer)
{
	size_t i=size;
	tty *atty=(tty *)device_pdata(node);

	atty->viewln=atty->scrln;
	while (i--) {
		if (atty->is_esc) {
			if (!atty->esc_ind) {
				if (*buffer!='[') atty->is_esc=0;
			} else {
				if (atty->esc_ind>=TTY_ESCAPE_LEN || !TTY_ESCAPE_CHR(*buffer)) atty->is_esc=0;
				else {
					atty->escape_seq[atty->esc_ind]=*buffer;
					if (TTY_ESCAPE_END(*buffer)) {
						atty->is_esc=0;
						tty_interpret_escape(atty);
					}
				}
			}
			atty->esc_ind++;
		} else switch (*buffer) {
			case '\0': break;
			case '\b':
				if (atty->cursor.x) atty->cursor.x--;
				atty->mem[TTY_POS(atty,atty->cursor.x,atty->cursor.y)]=atty->attr | 0x20;
				break;
			case '\t':
				atty->cursor.x=(atty->cursor.x+8)&0xF8;
				if (atty->cursor.x>=atty->width) atty->cursor.x=atty->width-1;
				break;
			case '\n':
				atty->cursor.x=0;
				atty->cursor.y++;
				break;
			case '\v':
			case '\f':
				atty->cursor.y++;
				break;
			case '\r':
				atty->cursor.x=0;
				break;
			case '\e':
				atty->is_esc=1;
				atty->esc_ind=0;
				memset(atty->escape_seq,0,TTY_ESCAPE_LEN);
				atty->escape_seq[0]='[';
				break;
			default:
				atty->mem[TTY_POS(atty,atty->cursor.x++,atty->cursor.y)]=atty->attr | (UCHAR) (*buffer);
				break;
		}
		if (atty->cursor.x>=atty->width) {
			atty->cursor.x=0;
			atty->cursor.y++;
		}
		if (atty->cursor.y==atty->height) {
			atty->cursor.y--;
			if (++atty->scrln==atty->memlines)
				atty->scrln=0;
			atty->viewln=atty->scrln;
			memsetw((USHORT *)(atty->mem+TTY_POS(atty,0,atty->height-1)),atty->attr | 0x20,atty->width);
		}
		buffer++;
	}
	if (node==current_tty) print_tty();
	return size;
}

static int tty_read(fs_node *node, off_t offset, size_t size, char *buffer)
{
	size_t i=size;
	tty *atty=(tty *)device_pdata(node);
	while (i) {
		sti(); //FIXME: Just delete it
		if (atty->in_e!=atty->in_s) {
			*buffer=(char)atty->input_buffer[atty->in_s++];
			if (atty->echo) tty_write(node,0,1,buffer);
			if (atty->in_s==TTY_INBUF_LEN) atty->in_s=0;
			i--;
			buffer++;
		}
	}
	return size;
}

static void tty_free_pdata(fs_node *node)
{
	free(((tty *)device_pdata(node))->mem);
	free(device_pdata(node));
}

static int tty_ioctl(fs_node *node, UINT cmd, ULONG arg)
{
	return -ENOTTY;
}

static tty *create_tty(int nr)
{
	tty *res=malloc(sizeof(tty));
	res->cursor.x=0;
	res->cursor.y=0;
	res->_cursor=res->cursor;
	res->height=TTY_HEIGHT;
	res->width=TTY_WIDTH;
	res->memlines=TTY_LINES;
	res->viewln=res->scrln=0;
	res->attr=(TTY_COL_BG_DEF << 12) | (TTY_COL_FG_DEF << 8);
	res->nr=nr;
	res->is_esc=0;
	res->show_cursor=0;
	res->mem=calloc(res->width*res->memlines,sizeof(USHORT));
	res->in_e=res->in_s=0;
	res->echo=1;
	res->node=0;
	return res;
}

static node_operations tty_ops = {
		read: &tty_read,
		write: &tty_write,
		free_pdata: &tty_free_pdata,
		ioctl: &tty_ioctl,
};

fs_node *set_current_tty(int nr)
{
	if (nr<0 || nr>=NR_TTYS || !ttys[nr]) return 0;
	current_tty=ttys[nr]->node;
	print_tty();
	return current_tty;
}

int setup_tty(fs_node *devfs) // Also replaces ktextio stuff with tty0
{
	int i;
	char name[6];
	fs_node *node;

	ttys[0]=create_tty(0);
	current_tty=devfs_register_device(devfs,"tty0",0660,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&tty_ops);
	set_device_pdata(current_tty,ttys[0]);
	ttys[0]->node=current_tty;
	for (i=1;i<NR_TTYS;i++) {
		sprintf(name,"tty%d",i);
		node=devfs_register_device(devfs,name,0660,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&tty_ops);
		ttys[i]=create_tty(i);
		set_device_pdata(node,ttys[i]);
		ttys[i]->node=node;
	}
	memcpy(ttys[0]->mem,(char *)VIDEO_MEM,ttys[0]->width*ttys[0]->height*sizeof(USHORT));
	ttys[0]->cursor=_cursor_pos;
	ktexto=tty_write;
	register_interrupt_handler(IRQ1,&irq_tty_keyboard);
	return 0;
}
