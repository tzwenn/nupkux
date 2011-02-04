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

#include <fs/vfs.h>
#include <mm.h>

static vnode *create_empty_inode(super_block *sb, ULONG ino)
{
	vnode *res=calloc(1,sizeof(vnode));
	res->sb=sb;
	res->ino=ino;
	res->dev=sb->dev;
	res->count=1;
	sb->s_op->read_inode(res); //TODO: Error checking
	res->cache_next=sb->cache;
	sb->cache=res;
	return res;
}

static void free_inode(vnode *node)
{
	vnode *tmp=node->sb->cache, *prev=0;
	while (tmp) {
		if (tmp==node) break;
		prev=tmp;
		tmp=tmp->cache_next;
	}
	if (!prev)
		node->sb->cache=node->cache_next;
	else prev->cache_next=node->cache_next;
	free(node);
}

vnode *vfs_create_cache(void)
{
	return 0;
}

void free_sb_inodes(super_block *sb)
{
	if (!sb) return;
	vnode *node=sb->cache,*tmp;
	while (node) {
		tmp=node->cache_next;
		node->count=1; //We are going to free all instances ... Well, let the filesystem think so.
		if (sb->s_op->put_inode)
			sb->s_op->put_inode(node);
		free(node);
		node=tmp;
	}
}

vnode *iget(super_block *sb, ULONG ino)
{
	if (!sb) return 0;
	vnode *node=sb->cache;
	while (node) {
		if (node->ino==ino) {
			node->count++;
			return node;
		}
		node=node->cache_next;
	}
	if (!sb->s_op || !sb->s_op->read_inode) return 0;
	return create_empty_inode(sb,ino);
}

void iput(vnode *node)
{
	if (!node || !node->sb || !node->sb->s_op) return;
	if (!node->count) return;
	if (node->sb->s_op->put_inode)
		node->sb->s_op->put_inode(node);
	node->count--;
	if (!node->count) free_inode(node);
}
