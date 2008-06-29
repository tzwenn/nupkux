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

#include <fs/initrd.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <mm.h>

fs_node *initrd_inode_to_fs_node(UINT inode, fs_node *node, initrd_discr *discr, mountinfo *mi);

struct dirent dirent;

static void initrd_open(fs_node *node)
{
	node->nlinks++;
}

static UINT initrd_read(fs_node *node, UINT offset, UINT size, UCHAR *buffer)
{
	initrd_discr *discr = (initrd_discr *)(node->mi->discr);
	initrd_inode inode = discr->initrd_inodes[node->inode];
	if (offset>inode.size)
		return 0;
	if (offset+size>inode.size)
		size=inode.size-offset;
	memcpy(buffer,(UCHAR*)(inode.offset+offset+discr->location),size);
	return size;
}

static UINT initrd_write(fs_node *node, UINT offset, UINT size, UCHAR *buffer)
{
	return 0;
}

static void initrd_close(fs_node *node)
{
	node->nlinks--;
}

static struct dirent *initrd_readdir(fs_node *node, UINT index)
{
	if (node->flags!=FS_DIRECTORY) return 0;
	initrd_discr *discr = (initrd_discr *)(node->mi->discr);
	initrd_inode inode = discr->initrd_inodes[node->inode], file_inode;
	initrd_folder_entry *entries = (initrd_folder_entry *) (inode.offset+discr->location);
	UINT count = inode.size/sizeof(initrd_folder_entry);
	
	if (index>=count) return 0;
	file_inode=discr->initrd_inodes[entries[index].inode];
	strcpy(dirent.d_name,entries[index].filename);
	dirent.d_namlen=strlen(dirent.d_name);
	dirent.d_ino=entries[index].inode;
	dirent.d_type=file_inode.flags;
	return &dirent;
}


static fs_node *initrd_finddir(fs_node *node, char *name)
{	
	initrd_discr *discr = (initrd_discr *)(node->mi->discr);
	initrd_inode inode = discr->initrd_inodes[node->inode];
	initrd_folder_entry *entries = (initrd_folder_entry *) (inode.offset+discr->location);
	UINT i = inode.size/sizeof(initrd_folder_entry);
	
	while (i--) 
		if (!strcmp(name,entries[i].filename)) 
			return &(discr->nodes->nodes[entries[i].inode]);
	return 0;
}

node_operations initrd_operations = {&initrd_open,&initrd_read,&initrd_write,&initrd_close,&initrd_readdir,&initrd_finddir};

fs_node *initrd_inode_to_fs_node(UINT inode, fs_node *node, initrd_discr *discr, mountinfo *mi)
{
	initrd_inode d_inode = discr->initrd_inodes[inode];
	
	node->mode=d_inode.mode;
	node->gid=d_inode.gid;
	node->uid=d_inode.uid;
	node->flags=d_inode.flags;
	node->inode=d_inode.inode;
	node->size=d_inode.size;
	node->nlinks=1;
	node->ptr=0;
	node->mi=mi;
	node->f_op=&initrd_operations;
	
	return node;
}

fs_node *setup_initrd(UINT location, fs_node *mountpoint)
{
	if (!location) return 0;
	
	initrd_discr *discr = malloc(sizeof(initrd_discr));
	vfs_nodes *nodes = malloc(sizeof(vfs_nodes));
	fs_node *root;
	mountinfo *mi;
	UINT i;
	
	discr->location=location;
	discr->initrdheader=(initrd_header *)location;
	discr->initrd_inodes=(initrd_inode *)(location+sizeof(initrd_header));
	
	nodes->nodes=malloc(sizeof(fs_node)*discr->initrdheader->inodecount);
	memset(nodes->nodes,0,discr->initrdheader->inodecount);
	nodes->root=&(nodes->nodes[0]);
	discr->nodes=nodes;
	root=nodes->root;
	initrd_inode_to_fs_node(0,root,discr,0);
	mi=fs_add_mountpoint(FS_TYPE_INITRD,(void *)discr,mountpoint,0,nodes);
	root->mi=mi;
	
	i=discr->initrdheader->inodecount;
	while (--i)
		initrd_inode_to_fs_node(i,&(nodes->nodes[i]),discr,mi);
	
	return root;
}

UINT remove_initrd(fs_node *node)
{	
	if (!node) return 2;
	if (node!=node->mi->nodes->root) return 1;
	fs_del_mountpoint(node->mi);
	free(node->mi->discr);
	free(node->mi->nodes->nodes);
	free(node->mi->nodes);
	free(node->mi);
	return 0;
}
