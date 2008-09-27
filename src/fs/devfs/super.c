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

#include <fs/devfs.h>
#include <lib/string.h>

extern inode_operations devfs_i_ops;
extern file_operations devfs_dir_fop;
extern ULONG next_inode;

static super_block *read_devfs_sb(super_block *sb, void *data, int verbose);

static super_block *devfs_sb = 0; //With this variable I forbit more than one instance of DevFS
devfs_handle *devfs_root = 0;

filesystem_t devfs_fs_type = {
		name: "devfs",
		flags: 0,
		read_super: &read_devfs_sb,
		next: NULL
};

void devfs_handle2vnode(vnode *node, devfs_handle *handle)
{
	if (!node || !handle) return;
	node->flags=handle->flags;
	node->atime=node->ctime=node->mtime=0;
	node->gid=handle->gid;
	node->uid=handle->uid;
	node->mode=handle->mode;
	node->nlinks=handle->nlinks;
	node->size=handle->size;
	if (!node->i_op) node->i_op=malloc(sizeof(inode_operations));
	memcpy(node->i_op,&devfs_i_ops,sizeof(inode_operations));
	node->i_op->f_op=handle->f_op;
	handle->node=node;
	node->u.devfs_i=handle;
}

static devfs_handle *devfs_empty_dir(ULONG ino, ULONG parent)
{
	devfs_handle *h=calloc(1,sizeof(devfs_handle));
	devfs_d_entry *entr=calloc(2,sizeof(devfs_d_entry));

	entr[0].inode=ino;
	entr[1].inode=parent;
	strcpy(entr[0].filename,".");
	strcpy(entr[1].filename,"..");
	h->pdata=(void *)entr;
	h->flags=FS_DIRECTORY;
	h->ino=ino;
	h->mode=755;
	h->size=sizeof(devfs_d_entry)*2;
	h->nlinks=1;
	h->f_op=&devfs_dir_fop;
	h->parent=devfs_iget(parent);
	devfs_add_to_cache(h);
	return h;
}

static void devfs_read_inode(vnode *node)
{
	devfs_handle2vnode(node,devfs_iget(node->ino));
}

static void devfs_put_inode(vnode *node)
{
	if (node->count==1) {
		free(node->i_op);
		node->u.devfs_i->node=0;
	}
}

static void devfs_put_super(super_block *sb)
{
	devfs_free_cache();
	devfs_root=0;
	next_inode=0;
	devfs_sb=0;
}

static super_operations devfs_s_ops = {
		read_inode: &devfs_read_inode,
		put_inode: &devfs_put_inode,
		put_super: &devfs_put_super,
};

static super_block *read_devfs_sb(super_block *sb, void *data, int verbose)
{
	if (devfs_sb) return 0;
	devfs_sb=sb;
	sb->s_op=&devfs_s_ops;
	sb->blocksize=1;
	sb->blocksize_bits=0;
	devfs_create_cache();
	devfs_root=devfs_empty_dir(0,0);
	sb->root=iget(sb,0);
	return sb;
}
