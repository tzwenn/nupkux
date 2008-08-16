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

#include <fs/devfs.h>
#include <mm.h>
#include <lib/string.h>

struct dirent dirent;

static fs_node *devfs_get_node(devfs_discr *discr, UINT inode)
{
	fs_node *node = discr->nodes;
	
	while (inode>=DEVFS_INODES_PER_BLOCK) {
		node=&(node[DEVFS_INODES_PER_BLOCK]);  //We've to alloc just one more ...
		if (node->inode!=DEVFS_INODE_LINK) return 0; //It's a kind of stupid
		node=node->ptr;
		if (!node) return 0;
		inode-=DEVFS_INODES_PER_BLOCK;
	}
	return &(node[inode]);
}

static void devfs_add_d_entry(fs_node *dir, const char *name, UINT inode)
{
	UINT entr_num = (dir->size/sizeof(devfs_d_entry));
	
	dir->size=(entr_num+1)*sizeof(devfs_d_entry);
	dir->p_data=realloc(dir->p_data,dir->size);
	((devfs_d_entry*)dir->p_data)[entr_num].inode=inode;
	strncpy(((devfs_d_entry*)dir->p_data)[entr_num].filename,name,DEVFS_FILENAME_LEN);
	((devfs_d_entry*)dir->p_data)[entr_num].filename[DEVFS_FILENAME_LEN-1]=0;
}

static void del_add_d_entry(fs_node *dir, UINT index)
{
	devfs_d_entry *entries = (devfs_d_entry *) dir->p_data;
	UINT count=dir->size/sizeof(devfs_d_entry);
	
	if (index>count) return;
	while (index<count-1) {
		entries[index]=entries[index+1];
		index++;
	}
	dir->size-=sizeof(devfs_d_entry);
	dir->p_data=realloc(dir->p_data,dir->size);
}

static void devfs_dirck(fs_node *dir)
{
	devfs_d_entry *entries = (devfs_d_entry *) dir->p_data;
	devfs_discr *discr = (devfs_discr *)(dir->mi->discr);
	UINT i=0;
	fs_node *node;
	
	while (i<dir->size/sizeof(devfs_d_entry)) {
		node=devfs_get_node(discr,entries[i].inode);
		if (!node || node->inode==DEVFS_INODE_FREE)
			del_add_d_entry(dir,i);
			else i++;
	}
}

static void devfs_fsck(fs_node *nodes)
{
	
	UINT i, val_nodes;
	fs_node *pre=0;
	
	while (1) {
		val_nodes=0;
		for (i=0;i<DEVFS_INODES_PER_BLOCK;i++) {
			if (nodes[i].inode!=DEVFS_INODE_FREE) {
				if (nodes[i].flags==FS_DIRECTORY) devfs_dirck(&(nodes[i]));
				val_nodes++;
			} else nodes[i].p_data=0;
		}
		if (nodes[DEVFS_INODES_PER_BLOCK].inode!=DEVFS_INODE_LINK) return;
		if (!nodes[DEVFS_INODES_PER_BLOCK].ptr) {
			if (!val_nodes) {
				pre[DEVFS_INODES_PER_BLOCK].ptr=0;
				free(nodes);
			}
			return;
		}
		pre=nodes;
		nodes=nodes[DEVFS_INODES_PER_BLOCK].ptr;
	}
	
}

static struct dirent *devfs_readdir(fs_node *node, UINT index)
{
	if (node->flags!=FS_DIRECTORY) return 0;
	devfs_discr *discr = (devfs_discr *)(node->mi->discr);
	devfs_d_entry *entries = (devfs_d_entry *) (node->p_data);
	fs_node *file_node;
	
	if (index>=node->size/sizeof(devfs_d_entry)) return 0;
	file_node=devfs_get_node(discr,entries[index].inode);
	strncpy(dirent.d_name,entries[index].filename,DEVFS_FILENAME_LEN);
	dirent.d_name[DEVFS_FILENAME_LEN]=0;
	dirent.d_namlen=strlen(dirent.d_name);
	dirent.d_ino=entries[index].inode;
	dirent.d_type=file_node->flags;
	return &dirent;
}

static fs_node *devfs_finddir(fs_node *node, const char *name)
{
	if (node->flags!=FS_DIRECTORY) return 0;
	devfs_discr *discr = (devfs_discr *)(node->mi->discr);
	devfs_d_entry *entries = (devfs_d_entry *) (node->p_data);
	UINT i = node->size/sizeof(devfs_d_entry);
	
	while (i--) 
		if (!strcmp(name,entries[i].filename)) 
			return devfs_get_node(discr,entries[i].inode);
	
	return 0;
}

static fs_node *devfs_find_free_node(devfs_discr *discr)
{
	fs_node *nodes = discr->nodes;
	UINT i,block;
	
	for (block=0;;block++) {
		for (i=0;i<DEVFS_INODES_PER_BLOCK;i++)
			if (nodes[i].inode==DEVFS_INODE_FREE) {
				nodes[i].inode=DEVFS_INODES_PER_BLOCK*block+i;
				return &(nodes[i]);
			}
		if (!nodes[DEVFS_INODES_PER_BLOCK].ptr) {
			nodes[DEVFS_INODES_PER_BLOCK].ptr=calloc(DEVFS_INODES_PER_BLOCK+1,sizeof(fs_node));
			nodes=nodes[DEVFS_INODES_PER_BLOCK].ptr;
			i=DEVFS_INODES_PER_BLOCK;
			while (i--)
				nodes[i].inode=DEVFS_INODE_FREE;
			nodes[DEVFS_INODES_PER_BLOCK].inode=DEVFS_INODE_LINK;
			nodes[DEVFS_INODES_PER_BLOCK].ptr=0;
			nodes[0].inode=DEVFS_INODES_PER_BLOCK*block;
			return nodes;
		} else nodes=nodes[DEVFS_INODES_PER_BLOCK].ptr;
	}
	return 0;
}

fs_node *devfs_register_device(fs_node *dir, const char *name, UINT mode, UINT uid, UINT gid, UINT type, node_operations *f_op)
{
	if (!dir || dir->flags!=FS_DIRECTORY || dir->mi->fs_type!=FS_TYPE_DEVFS) return 0;
	devfs_discr *discr = (devfs_discr *) dir->mi->discr;
	fs_node *node = devfs_find_free_node(discr);
	
	node->mode=mode;
	node->uid=uid;
	node->gid=gid;
	node->flags=type;
	node->f_op=f_op;
	node->size=1; //We must have something here
	node->ptr=0;
	node->nlinks=1;
	node->mi=dir->mi;
	node->p_data=(void *)1; //lock it!
	devfs_add_d_entry(dir,name,node->inode);
	node->p_data=(void *)0; //unlock
	
	return node;
}

void devfs_unregister_device(fs_node *device)
{
	if (!device || (device->flags&0x07)==FS_DIRECTORY || device->mi->fs_type!=FS_TYPE_DEVFS) return;
	devfs_discr *discr = (devfs_discr *) device->mi->discr;
	
	device->inode=DEVFS_INODE_FREE;
	devfs_fsck(discr->nodes);
}

node_operations devfs_dir_operations = {0,0,0,0,&devfs_readdir,&devfs_finddir};

fs_node *setup_devfs(fs_node *mountpoint)
{
	devfs_discr *discr = (devfs_discr *) malloc(sizeof(devfs_discr));
	devfs_d_entry *rootentr;
	fs_node *root, *nodes;
	mountinfo *mi;
	UINT i = DEVFS_INODES_PER_BLOCK;
	
	root=nodes=calloc(DEVFS_INODES_PER_BLOCK+1,sizeof(fs_node));
	discr->nodes=nodes;
	discr->root=root;
	while (i--)
		nodes[i].inode=DEVFS_INODE_FREE;
	nodes[DEVFS_INODES_PER_BLOCK].inode=DEVFS_INODE_LINK;
	nodes[DEVFS_INODES_PER_BLOCK].ptr=0;
	mi=fs_add_mountpoint(FS_TYPE_DEVFS,(void *)discr,mountpoint,0,root);
	
	root->size=2*sizeof(devfs_d_entry);
	rootentr=calloc(2,sizeof(devfs_d_entry));
	rootentr[0].inode=rootentr[1].inode=0;
	strcpy(rootentr[0].filename,".");
	strcpy(rootentr[1].filename,"..");
	root->p_data=(void *)rootentr;
	
	root->mode=0755;
	root->uid=FS_UID_ROOT;
	root->gid=FS_GID_ROOT;
	root->flags=FS_DIRECTORY;
	root->inode=0;
	root->nlinks=1;
	root->mi=mi;
	root->ptr=0;
	root->f_op=&devfs_dir_operations;
	return root;
}

UINT remove_devfs(fs_node *devfs)
{
	if (!devfs) return 2;
	
	devfs_discr *discr = (devfs_discr *)(devfs->mi->discr);
	if (devfs!=discr->root) return 1;
	mountinfo *mi = devfs->mi;
	fs_del_mountpoint(mi);
	UINT i;
	fs_node *block = discr->nodes,*tmp;
	while (block) {
		for (i=0;i<DEVFS_INODES_PER_BLOCK;i++) {
			if (block[i].flags==FS_DIRECTORY) free(block[i].p_data);
		}
		tmp=block[DEVFS_INODES_PER_BLOCK].ptr;
		free(block);
		block=tmp;
	}
	free(discr);
	free(mi);
	return 0;
}
