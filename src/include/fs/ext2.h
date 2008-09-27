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

#ifndef _EXT2_H
#define _EXT2_H

#include <fs/vfs.h>

#define EXT2_MAGIC	0xEF53

#define EXT2_TYPE_FIFO	0x1000
#define EXT2_TYPE_CHAR	0x2000
#define EXT2_TYPE_DIR	0x4000
#define EXT2_TYPE_BLOCK	0x6000
#define EXT2_TYPE_FILE	0x8000
#define EXT2_TYPE_LINK	0xA000
#define EXT2_TYPE_SOCK	0xC000

#define EXT2_N_BLOCKS		15
#define EXT2_NDIR_BLOCKS	12

#define EXT2_BAD_INO		1
#define EXT2_ROOT_INO		2
#define EXT2_UNDEL_DIR_INO	6
#define EXT2_FIRST_INO		11

#ifndef _EXT2_DISCR
#define _EXT2_DISCR
typedef struct _ext2_discr ext2_discr;
#endif

#ifndef _EXT2_INODE
#define _EXT2_INODE
typedef struct _ext2_inode ext2_inode;
#endif

struct _ext2_inode {
	USHORT i_mode;
	USHORT i_uid;
	UINT i_size;
	UINT i_atime;
	UINT i_ctime;
	UINT i_mtime;
	UINT i_dtime;
	USHORT i_gid;
	USHORT i_nlinks;
	UINT i_blocks;
	UINT i_flags;
	UINT i_unused1;
	UINT i_block[EXT2_N_BLOCKS];
	UINT i_generation;
	UINT i_file_acl;
	UINT i_dir_acl;
	UINT i_faddr;
	UCHAR i_findx;
	UCHAR i_fsize;
	UCHAR i_unused2;
	USHORT i_upper_uid;
	USHORT i_upper_gid;
	UINT i_unused3;
};

typedef struct _ext2_group_dt {
	UINT bg_block_bitmap;
	UINT bg_inode_bitmap;
	UINT bg_inode_table;
	USHORT bg_free_blocks_count;
	USHORT bg_free_inodes_count;
	USHORT bg_used_dirs_count;
	USHORT bg_pad;
	UINT bg_unused[3];
} ext2_group_dt;

typedef struct _ext2_group {
	char *inode_bitmap;
	char *block_bitmap;
	ext2_group_dt phys;
} ext2_group;

typedef struct _ext2_sb {
	UINT s_inodes_count;
	UINT s_blocks_count;
	UINT s_r_blocks_count;
	UINT s_free_blocks_count;
	UINT s_free_inodes_count;
	UINT s_first_data_block;
	UINT s_log_block_size;
	UINT s_log_frag_size;
	UINT s_blocks_per_group;
	UINT s_frags_per_group;
	UINT s_inodes_per_group;
	UINT s_mtime;
	UINT s_wtime;
	USHORT s_mnt_count;
	USHORT s_max_mnt_count;
	USHORT s_magic;
	USHORT s_state;
	USHORT s_errors;
	USHORT s_minor_ver;
	UINT s_lastcheck;
	UINT s_checkinterval;
	UINT s_creator_os;
	UINT s_major_ver;
	USHORT s_res_uid;
	USHORT s_res_gid;
	UINT s_nres_inode;
	USHORT s_ino_size;
	USHORT s_blockgroup;
	UINT s_comp_feat;
	UINT s_incomp_feat;
	UINT s_ro_feat;
	UCHAR s_fs_id[16];
	UCHAR s_fs_name[16];
	UCHAR s_last_path[64];
} ext2_sb;

struct _ext2_discr {
	ext2_sb *pysb;
	ext2_group *groups;
	UINT group_count;
	UINT inodes_per_block;
};

#endif
