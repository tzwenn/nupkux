#ifndef _INITRD_H
#define _INITRD_H

#include <squaros.h>
#include <fs/fs.h>

#define INITRD_FILENAME_LEN	64

typedef struct _initrd_header
{
	UINT inodecount;
	UINT entries;
} initrd_header;


typedef struct _initrd_inode
{
	UINT inode;
	UINT offset;
	UINT mode;
	UINT gid;
	UINT uid;
	UCHAR flags;
	UINT size;
} initrd_inode;

typedef struct _initrd_folder_entry
{
	UINT inode;
	char filename[INITRD_FILENAME_LEN];
	
} initrd_folder_entry;

typedef struct _initrd_discr
{
	UINT location;
	initrd_header *initrdheader;
	initrd_inode *initrd_inodes;
	fs_node *initrd_root;
	UINT initrd_inode_count;
} initrd_discr;

extern fs_node *setup_initrd(UINT location);
extern UINT remove_initrd(fs_node *node);

#endif
