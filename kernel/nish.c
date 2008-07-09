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

#include <kernel/nish.h>
#include <kernel/ktextio.h>
#include <time.h>
#include <lib/string.h>
#include <drivers/drivers.h>
#include <drivers/ata.h>
#include <task.h>
#include <mm.h>

int nish();

static int ltrim(char *cmd)
{
	char *str = cmd;

	while ((*str<=32) && (*str)) str++;
	strcpy(cmd,str);
	return str-cmd;
}

static int rtrim(char *cmd)
{
	char *str = cmd;

	if (!*str) return 0;
	while (*str) str++;
	str--;
	while ((*str<=32) && (str>=cmd)) str--;
	str++;
	*str=0;
	return str-cmd;
}

static int _nish_split_par(char *cmd, char *args)
{
	char *astart;

	ltrim(cmd);
	astart=strchr(cmd,' ');
	if (astart) {
		*astart=0;
		strcpy(args,astart+1);
		ltrim(args);
		rtrim(args);
	}
	return 0;
}

static void format_mode(fs_node *node, char *output)
{
	strcpy(output,"??????????");
	
	if (!node) return;
	UINT mode = node->mode, i=3;
	switch (node->flags&0x7) {
		case FS_DIRECTORY:	output[0]='d';
		break;
		case FS_CHARDEVICE:	output[0]='c';
		break;
		case FS_BLOCKDEVICE:	output[0]='b';
		break;
		case FS_PIPE:		output[0]='p';
		break;
		case FS_SYMLINK:	output[0]='l';
		break;
		default:		output[0]='-';
		break;
	}
	while (i--) {
		output[(i+1)*3]=(mode&1)?'x':'-';
		output[i*3+2]=(mode&2)?'w':'-';
		output[i*3+1]=(mode&4)?'r':'-';
		mode>>=3;
	}
}

static int nish_help()
{
	printf("List of built-in commands:\n");
	printf("\ttest\t\tRun the current development function\n");
	printf("\tclear\t\tClean up the mess on the screen\n");
	printf("\tlba28\t\tAccess IDE-ATA drives\n");
	printf("\tls\t\tList directory contents\n");
	printf("\tcat\t\tShow file content on stdout\n");
	printf("\tcd\t\tChange working directory\n");
	printf("\ttime\t\tGive information about time and date\n");
	printf("\texit\t\tQuit nish\n"); 
	printf("\thalt\t\tHalt system\n");
	printf("\treboot\t\tReboot system\n");
	printf("\thelp\t\tWhat do you read right now?\n");
	return 0;
}

static int nish_time()
{
	struct tm now;
	time_t timestamp;
	
	now=getrtctime();
	printf("Time:\t\t%.2d:%.2d:%.2d\n",now.tm_hour,now.tm_min,now.tm_sec);
	printf("Date:\t\t%.2d-%.2d-%.2d\n",now.tm_year,now.tm_mon,now.tm_mday);
	removetimezone(&now);
	timestamp=mktime(&now);	
	return 1;
}

static int nish_lba28()
{
	UCHAR idelist = 0,drv,tcalc;
	USHORT tmpword = 0, contrl;
	UINT addr;
	UCHAR buffer[512];
	char input[STRLEN];

	printf("Detect Devices ...\n");
	outportb(0x1F6,0xA0);
	sleep(10);
	tmpword=inportb(0x1F7);
	if (tmpword & 0x40) {
		idelist=0x01;
		printf("Primary master exists.\n");
	}
	outportb(0x1F6,0xB0);
	sleep(10);
	tmpword=inportb(0x1F7);
	if (tmpword & 0x40) {
		idelist|=0x02;
		printf("Primary slave exists.\n");
	}
	outportb(0x176,0xA0);
	sleep(10);
	tmpword=inportb(0x177);
	if (tmpword & 0x40) {
		idelist|=0x10;
		printf("Secondary master exists.\n");
	}
	outportb(0x176,0xB0);
	sleep(10);
	tmpword=inportb(0x177);
	if (tmpword & 0x40) {
		idelist|=0x20;
		printf("Secondary Slave exists.\n");
	}
	printf("Finished. Start loop, [^C] stops.\n");
	while (1) {
		printf("Controller (1/2): ");
		if (_kin(input,STRLEN)==_kaborted) break;
		if (!strcmp(input,"1")) contrl=0x1F0;
			else if (!strcmp(input,"2")) contrl=0x170;
			else {
				printf("Unknown controller.\n");
				continue;
			     }
		printf("Drive (A/B): ");
		if (_kin(input,STRLEN)==_kaborted) break;
		if (!strcmp(input,"A")) drv=0x00;
			else if (!strcmp(input,"B")) drv=0x01;
			else {
				printf("Unknown drive.\n");
				continue;
			     }
		tcalc=drv+1;
		if (contrl==0x170) tcalc<<=4;
		if (!(idelist & tcalc)) {
			printf("No device attached.\n");
			continue;
		}
		printf("Enter address: ");
		if (_kin(input,STRLEN)==_kaborted) break;
		addr=str2d(input);
		if (!lba28_read(buffer,contrl,drv,addr,0)) {
			printf("Read-Error\n");
			continue;
		} else printf("Reading finished.\n");
		for (addr=0;addr<512;addr++)
			printf("%.2X",buffer[addr]);
		printf("\n");
	}
	printf("\n");
	return 1;
}

static int nish_cat(char *args)
{
	fs_node *node;
	UCHAR *buf;
	UINT i;
	
	node=namei(args);
	if (node) {
		open_fs(node,1,0);
		buf=(UCHAR *)malloc(node->size);
		read_fs(node,0,node->size,buf);
		for (i=0;i<node->size;i++)
			_kputc(buf[i]);
		free(buf);
		close_fs(node);
	} else printf("Error: Cannot find file.\n");
	return 1;	
}

static int nish_ls(char *args)
{
	fs_node *node, *tmp;
	UINT i;
	char the_mode[11];
	struct dirent *pDirEnt;
	
	if (!*args) node=current_task->pwd;
		else node=namei(args);
	if (node) {
		i=0;
		printf("Inode\tMode\t\tUID\tGID\tSize\tName\n");
		if ((node->flags&0x7)!=FS_DIRECTORY) {
			tmp=node;
			format_mode(tmp,the_mode);
			printf("%d\t%s\t%d\t%d\t%d\t%s\n",tmp->inode,the_mode,tmp->uid,tmp->gid,tmp->size,args);
		} else while ((pDirEnt=readdir_fs(node,i++))) {
			if (pDirEnt->d_name[0]=='.') continue;
			tmp=finddir_fs(node,pDirEnt->d_name);
			format_mode(tmp,the_mode);
			printf("%d\t%s\t%d\t%d\t%d\t%s\n",tmp->inode,the_mode,tmp->uid,tmp->gid,tmp->size,pDirEnt->d_name);
		}
	} else printf("Error: Could not find file %s.\n",args);
	
	return 1;
}

static int nish_cd(char *args)
{
	fs_node *node;
	
	cli();
	node=namei(args);
	if (node) {
		if ((node->flags&0x07)==FS_DIRECTORY) {
			current_task->pwd=node;
		} else printf("Error: \"%s\" isn't a directory.\n");
	} else printf("Error: Could not find directory.\n");
	sti();
	return 1;
}

extern UINT initrd_location;

static int nish_test()
{
	printf("---multitasking test---\n\n");
	
	if (!fork()) {
		while (1) {
			printf(".");
		}
	} else {
		if (!fork()) {
			while (1) {
				printf("x");
			}
		} else {
			while (1) {
				printf("O");
			}
		}
	}	
	return 1;
}

static int _nish_interpret(char *cmd)
{
	char args[STRLEN];

	_nish_split_par(cmd,args);
	if (!(*cmd)) {
		printf("\n");
		return 0;
	}
	if (!strcmp(cmd,"switch")) {
		//switch_task();
		printf("%d ticks \n",ticks);
		return 1;
	}
	if (!strcmp(cmd,"test")) return nish_test();
	if (!strcmp(cmd,"clear")) return _kclear();
	if (!strcmp(cmd,"lba28")) return nish_lba28();
	if (!strcmp(cmd,"ls")) return nish_ls(args);
	if (!strcmp(cmd,"cat")) return nish_cat(args);
	if (!strcmp(cmd,"cd")) return nish_cd(args);
	if (!strcmp(cmd,"time")) return nish_time();
	if (!strcmp(cmd,"exit")) return NISH_EXIT;
	if (!strcmp(cmd,"halt")) return NISH_HALT;
	if (!strcmp(cmd,"reboot")) return NISH_REBOOT;
	if (!strcmp(cmd,"help")) return nish_help();
	printf("nish: %s: command not found\n",cmd);
	return 0;
}

int nish()
{
	char input[STRLEN];
	int ret;

	printf("\nNupkux intern shell (nish) started.\nType \"help\" for a list of built-in commands.\n\n");
	while (1) {
		printf("# ");
		memset(input,0,STRLEN);
		_kin(input,STRLEN);
		ret=_nish_interpret(input);
		if ((ret & 0xF0)==0xE0) break;
	}
	return ret;
}
