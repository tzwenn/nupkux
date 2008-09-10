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

extern vnode *root_vnode;

extern UINT initrd_location;
extern filesystem_t initrd_fs_type;

int do_vfs_test(int argc, char **argv)
{
	vnode *node=0;
	char *buf;
	int i;

	setup_vfs_v2();
	register_filesystem(&initrd_fs_type);
	sys_mount("initrd","/","initrdfs",0,(char *)initrd_location);
	sys_mount("initrd","/mnt","initrdfs",0,(char *)initrd_location);
	if (argc>=2) node=namei_v2(argv[1],&i);
	printf("---\e[32mStart\e[m---\n\n");
	if (node) {
		buf=malloc(node->size);
		node->i_op->f_op->read(node,0,node->size,buf);
		for (i=0;i<node->size;i++)
			_kputc(buf[i]);
		free(buf);
		iput(node);
	} else printf("%s: not found (errno=%d)\n",argv[1],-i);
	sys_umount("/mnt");
	printf("\n---\e[32mFinished!\e[m---\n");
	sys_umount("/");
	unregister_filesystem(&initrd_fs_type);
	close_vfs_v2();
	return 1;
}
