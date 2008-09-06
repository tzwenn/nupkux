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
#include <task.h>
#include <mm.h>
#include <kernel/syscall.h>
#include <drivers/drivers.h>
#include <signal.h>

#define MAX_ARGS	16

extern int init(void);

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
	printf("\tls\t\tList directory contents\n");
	printf("\tcat\t\tShow file content on stdout\n");
	printf("\tcd\t\tChange working directory\n");
	printf("\ttime\t\tGive information about time and date\n");
	printf("\thalt\t\tHalt system\n");
	printf("\treboot\t\tReboot system\n");
	printf("\thelp\t\tWhat do you read right now?\n");
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
	} else printf("Error: Cannot find file %s.\n",argv[1]);
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

extern int do_vfs_test(int argc, char **argv);

static int nish_test(int argc, char **argv)
{
	return do_vfs_test(argc,argv);
}

static char nish_buf[STRLEN] = {0,};
static int buf_pos=0;

static int _nish_interpret(char *str)
{
	if (*str<32) return 0;
	char **argv=calloc(MAX_ARGS,sizeof(char *));
	int i,ret=0,argc;
	for (i=0;i<MAX_ARGS;i++)
		argv[i]=calloc(STRLEN,sizeof(char));
	argc=split_to_argv(str,argv);
	memset(nish_buf,0,STRLEN);
	buf_pos=0;
	if (!strcmp(argv[0],"test")) ret=nish_test(argc,argv);
		else if (!strcmp(argv[0],"clear")) ret=printf("\e[2J\e[H");
		else if (!strcmp(argv[0],"ls")) ret=nish_ls(argc,argv);
		else if (!strcmp(argv[0],"cat")) ret=nish_cat(argc,argv);
		else if (!strcmp(argv[0],"cd")) ret=nish_cd(argc,argv);
		else if (!strcmp(argv[0],"time")) ret=nish_time();
		else if (!strcmp(argv[0],"halt")) ret=sys_reboot(0x04);
		else if (!strcmp(argv[0],"reboot")) ret=sys_reboot(0x02);
		else if (!strcmp(argv[0],"help")) ret=nish_help();
		else printf("nish: %s: command not found.\n",argv[0]);
	for (i=0;i<MAX_ARGS;i++) {
		free(argv[i]);
	}
	free(argv);
	return ret;
}

static int nish_write(fs_node *node, off_t offset, size_t size, const char *buffer)
{
	if (buf_pos+size+1>STRLEN) size=STRLEN-1-buf_pos;
	size_t i=size;
	while (i--) {
		nish_buf[buf_pos++]=*buffer;
		if (!*buffer)
			_nish_interpret(nish_buf);
		buffer++;
	}

	return size;
}

static node_operations nish_ops = {
		write: &nish_write,
};

int nish()
{
	devfs_register_device(namei("/dev"),"nish",0660,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&nish_ops);
	return 0;
	/*char input[STRLEN];
	int ret;

	printf("\nNupkux intern shell (nish) started.\nType \"help\" for a list of built-in commands.\n\n");
	while (1) {
		printf("# ");
		memset(input,0,STRLEN);
		_kgets(input,STRLEN);
		ret=_nish_interpret(input);
		if ((ret & 0xF0)==0xE0) break;
	}
	return ret;*/
}
