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

#include "devfs.h"
#include <lib/string.h>

extern void devfs_handle2vnode(vnode *node, devfs_handle *handle);
extern devfs_handle *devfs_root;

ULONG next_inode=1;

static void devfs_add_d_entry(devfs_handle *dir, const char *name, ULONG inode)
{
	UINT entr_num = (dir->size/sizeof(devfs_d_entry));

	dir->size=(entr_num+1)*sizeof(devfs_d_entry);
	dir->pdata=realloc(dir->pdata,dir->size);
	((devfs_d_entry*)dir->pdata)[entr_num].inode=inode;
	strncpy(((devfs_d_entry*)dir->pdata)[entr_num].filename,name,DEVFS_FILENAME_LEN);
	((devfs_d_entry*)dir->pdata)[entr_num].filename[DEVFS_FILENAME_LEN]=0;
	if (dir->node)
		devfs_handle2vnode(dir->node,dir);
}

static void devfs_del_d_entry(devfs_handle *dir, ULONG ino)
{
	if (!dir) return;
	UINT index = 0;
	devfs_d_entry *entries = (devfs_d_entry *) dir->pdata;
	UINT count=dir->size/sizeof(devfs_d_entry);

	for (;index<count;index++) //Optimize: Skip . and ..
		if (entries[index].inode==ino) break;
	if (index>=count) return;
	while (index<count-1) {
		entries[index]=entries[index+1];
		index++;
	}
	dir->size-=sizeof(devfs_d_entry);
	dir->pdata=realloc(dir->pdata,dir->size);
	if (dir->node)
		devfs_handle2vnode(dir->node,dir);
}


devfs_handle *devfs_register_device_v2(devfs_handle *dir, const char *name, UINT mode, UINT uid, UINT gid, UINT type, file_operations *f_op)
{
	if (!dir) dir=devfs_root;
	if (!dir || !IS_DIR2(dir)) return 0;
	devfs_handle *handle=calloc(1,sizeof(devfs_handle));
	handle->ino=next_inode++;
	handle->parent=dir;
	handle->mode=mode;
	handle->uid=uid;
	handle->gid=gid;
	handle->flags=type;
	handle->f_op=f_op;
	handle->size=1; //Maybe we change to something like count of blocks or so
	handle->nlinks=1;
	devfs_add_d_entry(dir,name,handle->ino);
	devfs_add_to_cache(handle);
	return handle;
}

void devfs_unregister_device_v2(devfs_handle *device)
{
	if (!device) return;
	devfs_del_d_entry(device->parent,device->ino);
	devfs_del_from_cache(device);
}

file_operations devfs_dir_fop = {
		0,
};
