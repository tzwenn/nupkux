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
#include <errno.h>
#include <kernel/ktextio.h>
#include <time.h>
#include <lib/string.h>
#include <drivers/ata.h>
#include <task.h>
#include <mm.h>
#include <kernel/syscall.h>

#define MAX_ARGS	16

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

int split_to_argv(char *str, char *argv[])
{ //I've done something with strtok() but it didn't work
	if (!str) return -1;
	if (!*str) return 0;
	char cmd[STRLEN],args[STRLEN];
	UINT i=0;
	strncpy(cmd,str,STRLEN);
	do {
		memset(args,0,STRLEN);
		_nish_split_par(cmd,args);
		strcpy(argv[i],cmd);
		strcpy(cmd,args);
		i++;
	} while (*cmd);
	argv[i]=0;
	return i;
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

static int nish_help(void)
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
	printf("  Any other command is tried to be executed from /bin.\n");
	return 0;
}

static int nish_time(void)
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

static int nish_lba28(void)
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
		if (_kgets(input,STRLEN)==_kaborted) break;
		if (!strcmp(input,"1")) contrl=0x1F0;
			else if (!strcmp(input,"2")) contrl=0x170;
			else {
				printf("Unknown controller.\n");
				continue;
			     }
		printf("Drive (A/B): ");
		if (_kgets(input,STRLEN)==_kaborted) break;
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
		if (_kgets(input,STRLEN)==_kaborted) break;
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

static int nish_cat(int argc, char *argv[])
{
	fs_node *node;
	char *buf;
	UINT i;

	if (argc==1) return 1;
	node=namei(argv[1]);
	if (node) {
		open_fs(node,1,0);
		buf=(char *)malloc(node->size);
		read_fs(node,0,node->size,buf);
		for (i=0;i<node->size;i++)
			_kputc(buf[i]);
		free(buf);
		close_fs(node);
	} else printf("Error: Cannot find file.\n");
	return 1;
}

static int nish_ls(int argc, char *argv[])
{
	fs_node *node, *tmp;
	UINT i;
	char the_mode[11];
	struct dirent *pDirEnt;

	if (argc==1) node=current_task->pwd;
		else node=namei(argv[1]);
	if (node) {
		i=0;
		printf("Inode\tMode\t\tUID\tGID\tSize\tName\n");
		if ((node->flags&0x7)!=FS_DIRECTORY) {
			tmp=node;
			format_mode(tmp,the_mode);
			printf("%d\t%s\t%d\t%d\t%d\t%s\n",tmp->inode,the_mode,tmp->uid,tmp->gid,tmp->size,argv[1]);
		} else while ((pDirEnt=readdir_fs(node,i++))) {
			if (pDirEnt->d_name[0]=='.') continue;
			tmp=finddir_fs(node,pDirEnt->d_name);
			format_mode(tmp,the_mode);
			printf("%d\t%s\t%d\t%d\t%d\t%s\n",tmp->inode,the_mode,tmp->uid,tmp->gid,tmp->size,pDirEnt->d_name);
		}
	} else printf("Error: Could not find file %s.\n",argv[1]);

	return 1;
}

static int nish_cd(int argc, char *argv[])
{
	if (argc==1) {
		sys_chdir("/");
		return 1;
	}
	int ret = sys_chdir(argv[1]);
	switch (ret) {
		case -ENOENT: 	printf("cd: %s: No such file or directory\n",argv[1]);
				break;
		case -ENOTDIR: 	printf("cd: %s: Is no directory\n",argv[1]);
				break;
	}
	return 1;
}

static int nish_run(const char *cmd, char **argv)
{
	int ret;
	pid_t pid;

	if (!(pid=sys_fork())) {
		ret=sys_execve(cmd,argv,0);
		if (ret==-ENOENT) {
			printf("nish: %s: command not found\n",cmd);
		}
		sys_exit(ret);
	} else sys_waitpid(pid,&ret,0);
	return ret;
}

extern void run_test(void);

static int nish_test(int argc, char **argv)
{
	printf("---tty test---\n\n");
	run_test();
	return 1;
}

static int _nish_interpret(char *str)
{
	if (*str<32) return 0;
	char **argv=calloc(MAX_ARGS,sizeof(char *));
	int i,ret=0,argc;
	for (i=0;i<MAX_ARGS;i++)
		argv[i]=calloc(STRLEN,sizeof(char));
	argc=split_to_argv(str,argv);
	if (!strcmp(argv[0],"test")) ret=nish_test(argc,argv);
		else if (!strcmp(argv[0],"clear")) ret=_kclear();
		else if (!strcmp(argv[0],"lba28")) ret=nish_lba28();
		else if (!strcmp(argv[0],"ls")) ret=nish_ls(argc,argv);
		else if (!strcmp(argv[0],"cat")) ret=nish_cat(argc,argv);
		else if (!strcmp(argv[0],"cd")) ret=nish_cd(argc,argv);
		else if (!strcmp(argv[0],"time")) ret=nish_time();
		else if (!strcmp(argv[0],"exit")) ret=NISH_EXIT;
		else if (!strcmp(argv[0],"halt")) ret=NISH_HALT;
		else if (!strcmp(argv[0],"reboot")) ret=NISH_REBOOT;
		else if (!strcmp(argv[0],"help")) ret=nish_help();
		else ret=nish_run(argv[0],argv);
	for (i=0;i<MAX_ARGS;i++) {
		free(argv[i]);
	}
	free(argv);
	return ret;
}

int nish()
{
	char input[STRLEN];
	int ret;

	printf("\nNupkux intern shell (nish) started.\nType \"help\" for a list of built-in commands.\n\n");
	while (1) {
		printf("# ");
		memset(input,0,STRLEN);
		_kgets(input,STRLEN);
		ret=_nish_interpret(input);
		if ((ret & 0xF0)==0xE0) break;
	}
	return ret;
}
