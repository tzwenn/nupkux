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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

typedef unsigned char UCHAR;
typedef unsigned int UINT;

#define INITRD_FILENAME_LEN	64
#define FS_FILE		0x01
#define FS_DIRECTORY	0x02
#define HARDLINKS	2
#define INITRD_MAGIC	0x54494E49

typedef struct _initrd_header
{
	UINT magic;
	UINT inodecount;
	UINT entries;
} initrd_header;

typedef struct _initrd_inode
{
	UINT inode;
	UINT offset;
	UINT mode;
	UINT gid;
	UINT uid;
	UCHAR flags;
	UINT size;
} initrd_inode;

typedef struct _initrd_folder_entry
{
	UINT inode;
	char filename[INITRD_FILENAME_LEN];

} initrd_folder_entry;

UINT tmp_inode = 0;
initrd_inode *inodes = 0;
initrd_header header;
UCHAR *filebuf = 0; //Ich könnte auch fseek nehmen, doch irgendwie ...
UINT filelen = 0;

UINT savedir(char *directory, UINT parent_inode);
UINT savefile(char *filename, UCHAR d_type, UINT parent_inode);

UINT count_of_inodes(char *directory, int recursive)
{
	UINT res = 0, tmp;
	DIR *pDIR;
	struct dirent *pDirEnt;
	char filename[255];

	pDIR = opendir(directory);
	if (!pDIR) return -1;
	pDirEnt = readdir(pDIR);
	while (pDirEnt) {
		if (strcmp(".", pDirEnt->d_name) && strcmp("..", pDirEnt->d_name)) {
			if ((pDirEnt->d_type == DT_DIR) && (recursive)) {
				strcpy(filename, directory);
				strcat(filename, "/");
				strcat(filename, pDirEnt->d_name);
				tmp = count_of_inodes(filename, 1);
				if (tmp == -1) return -1;
				res += tmp;
			}
			res++;
		}
		pDirEnt = readdir(pDIR);
	}
	closedir(pDIR);
	return res;
}

UCHAR *expand_initrd(UINT diff)
{
	UINT oldsize = filelen;

	filelen += diff;
	filebuf = realloc(filebuf, filelen);
	inodes = (initrd_inode *)((UINT)filebuf + sizeof(initrd_header));

	return (UCHAR *)(((UINT) filebuf) + oldsize);
}

void createinode(char *filename, initrd_inode *inode, UCHAR d_type)
{
	struct stat infobuf;

	stat(filename, &infobuf);
	inode->inode = tmp_inode++;
	inode->mode = infobuf.st_mode;
	inode->gid = infobuf.st_gid;
	inode->uid = infobuf.st_uid;
	inode->flags = d_type;
	inode->size = infobuf.st_size;
	inode->offset = filelen;	//In der Hoffnung, dass dies nur aufgerufen wird, wenn es nötig ist
	//Name muss in den Ordner
}

void backup(UCHAR *buf, UINT len)
{
	FILE *f = fopen("backup.img", "w");
	UINT i;
	for (i = 0; i < len; i++) {
		putc(buf[i], f);
	}
	fclose(f);
}

UINT savefile(char *filename, UCHAR d_type, UINT parent_inode)
{
	if (d_type == DT_DIR)
		return savedir(filename, parent_inode);

	initrd_inode inode;
	UCHAR *data;
	FILE *f;

	createinode(filename, &inode, FS_FILE);
	fprintf(stderr, "\tOpen file %s\n", filename);
	header.entries++;
	data = expand_initrd(inode.size);

	f = fopen(filename, "r");
	fread(data, inode.size, 1, f);
	fclose(f);

	inodes[inode.inode] = inode;
	return inode.inode;
}

UINT savedir(char *directory, UINT parent_inode)
{
	initrd_inode inode;
	initrd_folder_entry *entries;
	UINT tmp, i = HARDLINKS;
	DIR *pDIR;
	struct dirent *pDirEnt;
	char filename[255];

	header.entries++;
	createinode(directory, &inode, FS_DIRECTORY);
	fprintf(stderr, "Enter directory %s (inode %d)\n", directory, inode.inode);
	inode.size = sizeof(initrd_folder_entry) * (count_of_inodes(directory, 0) + HARDLINKS);
	entries = (initrd_folder_entry *)expand_initrd(inode.size);

	entries[0].inode = inode.inode;
	strcpy(entries[0].filename, ".");	//HARDLINK auf mich selbst
	header.entries++;

	entries[1].inode = (parent_inode == -1) ? inode.inode : parent_inode; //Wenn Wurzelknoten
	strcpy(entries[1].filename, "..");	//HARDLINK auf Elternknoten
	header.entries++;

	pDIR = opendir(directory);
	if (!pDIR) return -1;
	pDirEnt = readdir(pDIR);
	while (pDirEnt) {
		if (strcmp(".", pDirEnt->d_name) && strcmp("..", pDirEnt->d_name)) {
			memset(entries[i].filename, 0, INITRD_FILENAME_LEN);
			strcpy(entries[i].filename, pDirEnt->d_name);
			strcpy(filename, directory);
			strcat(filename, "/");
			strcat(filename, pDirEnt->d_name);
			tmp = savefile(filename, pDirEnt->d_type, inode.inode);
			entries = (initrd_folder_entry *)((UINT)filebuf + inode.offset);
			entries[i].inode = tmp;
			i++;
		}
		pDirEnt = readdir(pDIR);
	}
	closedir(pDIR);

	inodes[inode.inode] = inode;
	return inode.inode;
}

int main(int argc, char* argv[])
{
	char dirname[256];
	UINT len;
	FILE *f = stdout;


	if (argc < 2) {
		fprintf(stderr, "Please specify directory.\n");
		return 1;
	}
	strcpy(dirname, argv[1]);
	header.magic = INITRD_MAGIC;
	header.inodecount = count_of_inodes(dirname, 1) + 1;
	header.entries = 0;
	if (!header.inodecount) return 2;
	len = sizeof(initrd_header) + header.inodecount * sizeof(initrd_inode);
	expand_initrd(len);
	memset(inodes, 0, header.inodecount * sizeof(initrd_inode));
	savedir(dirname, -1);
	*((initrd_header*)(filebuf)) = header;
	//f=fopen("initrd.img","w");
	fwrite(filebuf, filelen, 1, f);
	//fclose(f);
	return 0;
}
