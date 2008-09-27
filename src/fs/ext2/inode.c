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

#include <fs/ext2.h>
#include <errno.h>
#include <kernel/ktextio.h>

extern inline int ext2_read_block(super_block *sb, UINT block, char *buffer);

static int ext2_read(vnode *node, off_t offset, size_t size, char *buffer)
{
	char buf[1024];
	if (offset>node->size)
		return 0;
	if (offset+size>node->size)
		size=node->size-offset;
	int block=offset/node->sb->blocksize;
	ext2_read_block(node->sb,node->u.ext2_i->i_block[block],buf);
	memcpy(buffer,buf,size);
	return size;
}

static int ext2_readdir(vnode *dir, off_t index, struct dirent *buf)
{
	printf("Readdir on inode %d\n",dir->ino);

	return -EINVAL;
}

static vnode *ext2_lookup(vnode *dir, const char *name)
{
	printf("Lookup on inode %d for %s\n",dir->ino,name);
	return 0;
}

static file_operations ext2_f_ops = {
		read: &ext2_read,
		readdir: &ext2_readdir,
};

inode_operations ext2_i_ops = {
		f_op: &ext2_f_ops,
		lookup: &ext2_lookup,
};
