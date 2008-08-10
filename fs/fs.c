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

#include <fs/fs.h>
#include <mm.h>
#include <errno.h>

static fs_node *fs_root = 0;
static mountinfo *mountinfos = 0;

void open_fs(fs_node *node, UCHAR read, UCHAR write)
{
	if (node->f_op && node->f_op->open) return node->f_op->open(node);
}

UINT read_fs(fs_node *node, off_t offset, size_t size, UCHAR *buffer)
{
	if (node->f_op && node->f_op->read) return node->f_op->read(node,offset,size,buffer);
		else return 0;
}

UINT write_fs(fs_node *node, off_t offset, size_t size, UCHAR *buffer)
{
	if (node->f_op && node->f_op->write) return node->f_op->write(node,offset,size,buffer);
		else return 0;
}

void close_fs(fs_node *node)
{
	if (node->f_op && node->f_op->close) return node->f_op->close(node);
}

struct dirent *readdir_fs(fs_node *node, UINT index)
{
	if ((node->flags&FS_DIRECTORY) && (node->f_op && node->f_op->readdir)) return node->f_op->readdir(node,index);
		else return 0;
}

fs_node *finddir_fs(fs_node *node, char *name)
{
	if ((node->flags&FS_DIRECTORY) && (node->f_op && node->f_op->finddir)) return node->f_op->finddir(node,name);
		else return 0;
}

UINT setup_vfs()
{
	if (fs_root) return 1;
	fs_root=calloc(1,sizeof(fs_node)); //Sets zero everywhere too
	fs_root->mode=(FS_MODE_RWX << FS_MODE_USER) | (FS_MODE_RX << FS_MODE_GROUP) | (FS_MODE_RX << FS_MODE_OTHER);
	fs_root->uid=FS_UID_ROOT;
	fs_root->gid=FS_GID_ROOT;
	fs_root->flags=FS_DIRECTORY;
	
	return 0;
}

mountinfo *fs_add_mountpoint(UINT fs_type, void *discr, fs_node *mountpoint, fs_node *device, fs_node *root)
{
	mountinfo *mi = (mountinfo *) malloc(sizeof(mountinfo));
	
	mi->fs_type=fs_type;
	mi->discr=discr;
	mi->mountpoint=mountpoint;
	mi->device=device;
	mi->root=root;
	mi->next=mountinfos;
	mountinfos=mi;
	if (mountpoint) {
		/*FIXME I could mount another time on this node and make a lot of trouble
		  UPDATE: namei() resolves the problem in a good way:
			  B is mounted on A and C also (for the caller) on A, it will
		          found itself on B (understood? sorry ...)
		*/
		if (mountpoint->flags!=FS_DIRECTORY) {
			errno=-ENOTDIR;
			return 0;
		}
		mountpoint->flags|=FS_MOUNTPOINT;
		mountpoint->ptr=root;
	}

	return mi;
}

UINT fs_del_mountpoint(mountinfo *mi)
{
	mountinfo *pre=0,*tmp=mountinfos;
	
	while (tmp) {
		if (tmp==mi) break;
		pre=tmp;
		tmp=tmp->next;
	}
	if (!tmp) return 0;
	if (!pre) mountinfos=tmp->next;
		else pre->next=tmp->next;
	if (mi->mountpoint) {
		mi->mountpoint->flags&=~FS_MOUNTPOINT;
		mi->mountpoint->ptr=0;
	}
	return 1;
}

UINT close_vfs()
{
	free(fs_root);
	fs_root=0;
	
	return 0;
}

fs_node *resolve_node(fs_node *node)
{
	if (!node) return 0;
	if (node->flags&FS_MOUNTPOINT || node->flags==FS_SYMLINK)
		return resolve_node(node->ptr);
	else return node;
}

fs_node *get_root_fs_node()
{
	return resolve_node(fs_root);
}

int sys_close(int fd)
{
	return 0;
}
