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

static int ltrim(char *cmd)
{
	char *str = cmd;

	while ((*str > 0) && (*str <= 32)) str++;
	strcpy(cmd, str);
	return str - cmd;
}

static int rtrim(char *cmd)
{
	char *str = cmd;

	if (!*str) return 0;
	while (*str) str++;
	str--;
	while ((*str > 0) && (*str <= 32) && (str >= cmd)) str--;
	str++;
	*str = 0;
	return str - cmd;
}

static int _nish_split_par(char *cmd, char *args)
{
	char *astart;

	ltrim(cmd);
	astart = strchr(cmd, ' ');
	if (astart) {
		*astart = 0;
		strcpy(args, astart + 1);
		ltrim(args);
		rtrim(args);
	}
	return 0;
}

static int split_to_argv(char *str, char *argv[])
{	//I've done something with strtok() but it didn't work
	if (!str) return -1;
	if (!*str) return 0;
	char cmd[STRLEN], args[STRLEN];
	UINT i = 0;
	strncpy(cmd, str, STRLEN);
	do {
		memset(args, 0, STRLEN);
		_nish_split_par(cmd, args);
		strcpy(argv[i], cmd);
		strcpy(cmd, args);
		i++;
	} while (*cmd);
	argv[i] = 0;
	return i;
}

static void format_mode(vnode *node, char *output)
{
	strcpy(output, "??????????");

	if (!node) return;
	UINT mode = node->mode, i = 3;
	switch (node->flags & 0x7) {
	case FS_DIRECTORY:
		output[0] = 'd';
		break;
	case FS_CHARDEVICE:
		output[0] = 'c';
		break;
	case FS_BLOCKDEVICE:
		output[0] = 'b';
		break;
	case FS_PIPE:
		output[0] = 'p';
		break;
	case FS_SYMLINK:
		output[0] = 'l';
		break;
	default:
		output[0] = '-';
		break;
	}
	while (i--) {
		output[(i+1)*3] = (mode & 1) ? 'x' : '-';
		output[i*3+2] = (mode & 2) ? 'w' : '-';
		output[i*3+1] = (mode & 4) ? 'r' : '-';
		mode >>= 3;
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

	now = getrtctime();
	printf("Time:\t\t%.2d:%.2d:%.2d\n", now.tm_hour, now.tm_min, now.tm_sec);
	printf("Date:\t\t%.2d-%.2d-%.2d\n", now.tm_year, now.tm_mon, now.tm_mday);
	removetimezone(&now);
	timestamp = mktime(&now);
	return 1;
}

static int nish_cat(int argc, char *argv[])
{
	vnode *node;
	char *buf;
	UINT i;

	if (argc == 1) return 1;
	node = namei(argv[1], 0);
	if (node) {
		open_fs(node, 0);
		buf = (char *)malloc(node->size);
		read_fs(node, 0, node->size, buf);
		for (i = 0; i < node->size; i++)
			_kputc(buf[i]);
		free(buf);
		close_fs(node);
		iput(node);
	} else printf("Error: Cannot find file %s.\n", argv[1]);
	return 1;
}

static int nish_ls(int argc, char *argv[])
{
	vnode *node, *tmp;
	UINT i;
	char the_mode[11];
	struct dirent DirEnt;

	if (argc == 1) node = namei(".", 0);
	else node = namei(argv[1], 0);
	if (node) {
		i = 0;
		printf("Inode\tMode\t\tUID\tGID\tSize\tName\n");
		if (!IS_DIR(node)) {
			tmp = node;
			format_mode(tmp, the_mode);
			printf("%d\t%s\t%d\t%d\t%d\t%s\n", tmp->ino, the_mode, tmp->uid, tmp->gid, tmp->size, argv[1]);
		}  else while (!readdir_fs(node, i++, &DirEnt)) {
				if (DirEnt.d_name[0] == '.') continue;
				tmp = node->i_op->lookup(node, DirEnt.d_name); //also stat
				format_mode(tmp, the_mode);
				printf("%d\t%s\t%d\t%d\t%d\t%s\n", tmp->ino, the_mode, tmp->uid, tmp->gid, tmp->size, DirEnt.d_name);
				iput(tmp);
			}
		iput(node);
	} else printf("Error: Could not find file %s.\n", argv[1]);
	return 1;
}

static int nish_cd(int argc, char *argv[])
{
	if (argc == 1) {
		sys_chdir("/");
		return 1;
	}
	int ret = sys_chdir(argv[1]);
	switch (ret) {
	case -ENOENT:
		printf("cd: %s: No such file or directory\n", argv[1]);
		break;
	case -ENOTDIR:
		printf("cd: %s: Is no directory\n", argv[1]);
		break;
	}
	return 1;
}

static int nish_test(int argc, char **argv)
{
/*	printf("---ext2 test: Mount /dev/ram0 on /mnt---\n");
	sys_mount("/dev/ram0", "/mnt", "ext2", 0, 0);*/
	printf("No tests today.\n");
	return 1;
}

static int _nish_interpret(char *str)
{
	if (*str < 32) return 0;
	char **argv = calloc(MAX_ARGS, sizeof(char *));
	int i, ret = 0, argc;
	for (i = 0; i < MAX_ARGS; i++)
		argv[i] = calloc(STRLEN, sizeof(char));
	argc = split_to_argv(str, argv);
	if (!strcmp(argv[0], "test")) ret = nish_test(argc, argv);
	else if (!strcmp(argv[0], "clear")) ret = printf("\e[2J\e[H");
	else if (!strcmp(argv[0], "ls")) ret = nish_ls(argc, argv);
	else if (!strcmp(argv[0], "cat")) ret = nish_cat(argc, argv);
	else if (!strcmp(argv[0], "cd")) ret = nish_cd(argc, argv);
	else if (!strcmp(argv[0], "time")) ret = nish_time();
	else if (!strcmp(argv[0], "halt")) ret = sys_reboot(0x04);
	else if (!strcmp(argv[0], "reboot")) ret = sys_reboot(0x02);
	else if (!strcmp(argv[0], "help")) ret = nish_help();
	else if (!strcmp(argv[0], "exit")) ret = NISH_EXIT;
	else printf("nish: %s: command not found.\n", argv[0]);
	for (i = 0; i < MAX_ARGS; i++) {
		free(argv[i]);
	}
	free(argv);
	return ret;
}

//#define DIRECT_NISH  //If you only want to use one single nish-session uncomment this line

#ifdef DIRECT_NISH

static vnode *ttynode = 0;

static void _kgets(char *buf)
{
	int i = 0;
	printf("\e[?25h");
	for (;;) {
		read_fs(ttynode, 0, 1, buf + i);
		switch (buf[i]) {
		case '\n':
			buf[i] = '\0';
			goto end;
			break;
		case '\b':
			i -= 2;
			if (i >= -1) printf("\b");
			if (i < 0) i = -1;
			break;
		case '\t':
			break;
		default:
			_kputc(buf[i]);
			break;
		}
		i++;
	}
end:
	printf("\n\e[?25l");
}

#else

static char nish_buf[STRLEN] = {0,};
static int buf_pos = 0;

static int nish_write(vnode *node, off_t offset, size_t size, const char *buffer)
{
	if (buf_pos + size + 1 > STRLEN) size = STRLEN - 1 - buf_pos;
	size_t i = size;
	while (i--) {
		nish_buf[buf_pos++] = *buffer;
		if (!*buffer) {
			_nish_interpret(nish_buf);
			memset(nish_buf, 0, STRLEN);
			buf_pos = 0;
		}
		buffer++;
	}

	return size;
}

static file_operations nish_ops = {
write:
	&nish_write,
};

#endif

int nish()
{
#ifdef DIRECT_NISH
	char input[STRLEN];
	int ret;
	ttynode = namei("/dev/tty0", &ret);
	if (!ttynode) {
		printf("Cannot open tty0: %d\n", -ret);
		for (;;);
	}
	printf("\nNupkux intern shell (nish) started.\nType \"help\" for a list of built-in commands.\n\n");
	while (1) {
		printf("# ");
		memset(input, 0, STRLEN);
		_kgets(input);
		ret = _nish_interpret(input);
		if ((ret & 0xF0) == 0xE0) break;
	}
	iput(ttynode);
	sys_reboot(0x04);
	return ret;
#else
	devfs_register_device(NULL, "nish", 0660, FS_UID_ROOT, FS_GID_ROOT, FS_CHARDEVICE, &nish_ops);
	return 0;
#endif
}
