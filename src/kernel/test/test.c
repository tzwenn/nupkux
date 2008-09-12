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

/* This file contains stuff related to the new virtual filesystem */

#include "vfs.h"
#include "initrdfs/initrdfs.h"
#include "mount.h"
#include <unistd.h>
#include <kernel/ktextio.h>
#include <mm.h>
#include "devfs/devfs.h"

extern vnode *root_vnode;

extern UINT initrd_location;
extern filesystem_t initrd_fs_type;
extern filesystem_t devfs_fs_type;

static int test_read(vnode *node, off_t offset, size_t size, char *buffer)
{
	memset(buffer,'A',size);
	return size;
}

file_operations ops = {
		read: &test_read,
};

int do_vfs_test(int argc, char **argv)
{
	vnode *node=0;
	char *buf;
	int i;

	setup_vfs_v2();
	register_filesystem(&initrd_fs_type);
	sys_mount("initrd","/","initrdfs",0,(char *)initrd_location);
	register_filesystem(&devfs_fs_type);
	sys_mount("devfs","/dev","devfs",0,0);
	devfs_handle *test=devfs_register_device_v2(NULL,"test",664,0,0,FS_CHARDEVICE,&ops);
	printf("---\e[32mStart\e[m---\n\n");
	if (argc>=2) node=namei_v2(argv[1],&i);
	if (node) {
		if (node->i_op && node->i_op->f_op && node->i_op->f_op->read) {
			buf=malloc(node->size);
			node->i_op->f_op->read(node,0,node->size,buf);
			for (i=0;i<node->size;i++)
				_kputc(buf[i]);
			free(buf);
		} else printf("File does not support reading!\n");
		iput(node);
	} else printf("%s: not found (errno=%d)\n",argv[1],-i);
	printf("\n---\e[32mFinished!\e[m---\n");
	devfs_unregister_device_v2(test);
	sys_umount("/dev");
	unregister_filesystem(&devfs_fs_type);
	sys_umount("/");
	unregister_filesystem(&initrd_fs_type);
	close_vfs_v2();
	return 1;
}
