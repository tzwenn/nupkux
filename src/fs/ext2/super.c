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
#include <fs/mount.h>

extern inode_operations ext2_i_ops;

static super_block *read_ext2_sb(super_block *sb, void *data, int verbose);

inline int ext2_read_block(super_block *sb, UINT block, char *buffer)
{
	if (!block) {
		memset(buffer, 0, 1024);
		return 2;
	}
	return fs_read_block(sb, block - 1, 1, buffer);
}

static inline int ext2_inode_used(char *inode_bitmap, int group_off)
{
	return inode_bitmap[group_off/8] & (1 << (group_off % 8));
}

static void ext2_read_inode(vnode *node)
{
	ext2_discr discr = *(node->sb->u.ext2_s);
	if (!node->ino || node->ino > discr.pysb->s_inodes_count) return;
	UINT group = (node->ino - 1) / discr.pysb->s_inodes_per_group;
	if (group >= discr.group_count) return;
	UINT goff = (node->ino - 1) % (discr.pysb->s_inodes_per_group);
	if (!ext2_inode_used(discr.groups[group].inode_bitmap, goff)) return;
	char *buf = malloc(node->sb->blocksize);
	if (ext2_read_block(node->sb, discr.groups[group].phys.bg_inode_table + goff / discr.inodes_per_block, buf) <= 0) {
		free(buf);
		return;
	}
	ext2_inode *p_node = (ext2_inode *)((UINT)buf + ((goff % discr.inodes_per_block) * discr.pysb->s_ino_size));
	node->atime = p_node->i_atime;
	node->ctime = p_node->i_ctime;
	node->mtime = p_node->i_mtime;
	node->mode = p_node->i_mode & 0x1FF;
	switch (p_node->i_mode & 0xF000) {
	case EXT2_TYPE_FIFO:
		node->flags = FS_PIPE;
		break;
	case EXT2_TYPE_CHAR:
		node->flags = FS_CHARDEVICE;
		break;
	case EXT2_TYPE_DIR:
		node->flags = FS_DIRECTORY;
		break;
	case EXT2_TYPE_BLOCK:
		node->flags = FS_BLOCKDEVICE;
		break;
	case EXT2_TYPE_FILE:
		node->flags = FS_FILE;
		break;
	case EXT2_TYPE_LINK:
		node->flags = FS_SYMLINK;
		break;
	default:
		free(buf);
		return;
	}
	node->uid = p_node->i_uid;
	node->gid = p_node->i_gid;
	node->nlinks = p_node->i_nlinks;
	node->size = p_node->i_size;
	node->i_op = &ext2_i_ops;
	node->u.ext2_i = malloc(sizeof(ext2_inode));
	memcpy(node->u.ext2_i, p_node, sizeof(ext2_inode));
	free(buf);
}

static void ext2_put_inode(vnode *node)
{
	if (node->count == 1)
		free(node->u.ext2_i);
}

static void free_ext2_discr(ext2_discr *discr)
{
	int i;
	for (i = 0; i < discr->group_count; i++) {
		free(discr->groups[i].inode_bitmap);
		free(discr->groups[i].block_bitmap);
	}
	free(discr->groups);
	free(discr->pysb);
	free(discr);
}

static void ext2_put_super(super_block *sb)
{
	free_ext2_discr(sb->u.ext2_s);
}

static super_operations ext2_s_ops = {
read_inode:
	&ext2_read_inode,
put_inode:
	&ext2_put_inode,
put_super:
	&ext2_put_super,
};

filesystem_t ext2_fs_type = {
name: "ext2"
	,
flags:
	MNT_FLAG_REQDEV,
read_super:
	&read_ext2_sb,
next:
	NULL
};

static super_block *read_ext2_sb(super_block *sb, void *data, int verbose)
{
	char buf[1024];
	int i;
	sb->blocksize = 0x400; //Just the first super block ...
	sb->skip_bytes = 0x400;
	ext2_sb *py_super = malloc(sizeof(ext2_sb));
	if (ext2_read_block(sb, 1, buf) <= 0) {
		free(py_super);
		return 0;
	}
	memcpy(py_super, buf, sizeof(ext2_sb));
	if (py_super->s_magic != EXT2_MAGIC) {
		free(py_super);
		return 0;
	}
	sb->blocksize = 0x400 << py_super->s_log_block_size;
	sb->blocksize_bits = py_super->s_log_block_size + 10;
	ext2_discr *discr = malloc(sizeof(ext2_discr));
	discr->pysb = py_super;
	sb->u.ext2_s = discr;
	discr->inodes_per_block = sb->blocksize / py_super->s_ino_size;
	discr->group_count = (py_super->s_blocks_count / py_super->s_blocks_per_group) + 1;
	discr->groups = calloc(discr->group_count, sizeof(ext2_group));
	for (i = 0; i < discr->group_count; i++) {
		if (ext2_read_block(sb, py_super->s_first_data_block + py_super->s_blocks_per_group * i + 1, buf) <= 0) {
			free_ext2_discr(discr);
			return 0;
		}
		memcpy(&(discr->groups[i].phys), buf, sizeof(ext2_group_dt));
		discr->groups[i].inode_bitmap = malloc(sb->blocksize);
		if (ext2_read_block(sb, discr->groups[i].phys.bg_inode_bitmap, discr->groups[i].inode_bitmap) <= 0) {
			free_ext2_discr(discr);
			return 0;
		}
		discr->groups[i].block_bitmap = malloc(sb->blocksize);
		if (ext2_read_block(sb, discr->groups[i].phys.bg_block_bitmap, discr->groups[i].block_bitmap) <= 0) {
			free_ext2_discr(discr);
			return 0;
		}
	}
	sb->s_op = &ext2_s_ops;
	sb->root = iget(sb, EXT2_ROOT_INO);
	return sb;
}


