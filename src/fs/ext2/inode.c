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
#include <lib/string.h>

extern inline int ext2_read_block(super_block *sb, UINT block, char *buffer);

static int file_ext2_block(ext2_inode *node, int block)
{
	if (block < EXT2_NDIR_BLOCKS) return node->i_block[block];
	else return 0;
}

static int ext2_read(vnode *node, off_t offset, size_t size, char *buffer)
{
	int blocksize = node->sb->blocksize;
	if (offset > node->size)
		return 0;
	if (offset + size > node->size)
		size = node->size - offset;
	int b_s = offset / blocksize, boff = offset % blocksize, b_e = (offset + size - 1) / blocksize, block;
	int bsize = blocksize - boff;
	char *buf = malloc(blocksize);
	if (ext2_read_block(node->sb, file_ext2_block(node->u.ext2_i, b_s), buf) <= 0) {
		free(buf);
		return -EIO;
	}
	memcpy(buffer, buf + boff, bsize);
	buffer += bsize;
	for (block = b_s + 1; block < b_e; block++) {
		if (ext2_read_block(node->sb, file_ext2_block(node->u.ext2_i, block), buf) <= 0) {
			free(buf);
			return -EIO;
		}
		memcpy(buffer, buf, blocksize);
		buffer += blocksize;
	}
	if (b_s != b_e) {
		if (ext2_read_block(node->sb, file_ext2_block(node->u.ext2_i, b_e), buf) <= 0) {
			free(buf);
			return -EIO;
		}
		memcpy(buffer, buf, (offset + size) % blocksize);
	}
	free(buf);
	return size;
}

static int ext2_readdir(vnode *dir, off_t index, struct dirent *buf)
{
	int err;
	char *buffer = malloc(dir->size);
	ext2_d_entry *ent = (ext2_d_entry *)buffer;
	int read_size = dir->size;
	if ((err = ext2_read(dir, 0, dir->size, buffer)) <= 0) goto end;
	err = -EINVAL;
	while (index--) {
		read_size -= ent->rec_len;
		if (read_size <= 0  || !ent->rec_len) goto end;
		ent = (ext2_d_entry *)((UINT)ent + ent->rec_len);
	}
	if (!ent->inode) goto end;
	buf->d_ino = ent->inode;
	strncpy(buf->d_name, ent->name, VFS_NAME_LEN);
	buf->d_namlen = strlen(buf->d_name);
	buf->d_name[ent->name_len] = 0;
	err = 0;
end:
	free(buffer);
	return err;
}

static vnode *ext2_lookup(vnode *dir, const char *name)
{
	ULONG ino;
	char *buffer = malloc(dir->size);
	ext2_d_entry *ent = (ext2_d_entry *)buffer;
	int read_size = dir->size;
	if (ext2_read(dir, 0, dir->size, buffer) <= 0) goto end;
	while (read_size > 0) {
		if (namei_nmatch(name, ent->name, ent->name_len)) {
			ino = ent->inode;
			free(buffer);
			return iget(dir->sb, ino);
		}
		if (!ent->rec_len) goto end;
		ent = (ext2_d_entry *)((UINT)ent + ent->rec_len);
		read_size -= ent->rec_len;
	}
end:
	free(buffer);
	return 0;
}

static file_operations ext2_f_ops = {
read:
	&ext2_read,
readdir:
	&ext2_readdir,
};

inode_operations ext2_i_ops = {
f_op:
	&ext2_f_ops,
lookup:
	&ext2_lookup,
};
