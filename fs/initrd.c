#include <fs/initrd.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/ktextio.h>
#include <mm.h>

fs_node *filetonode(UINT inode, fs_node *node, char *name, initrd_discr *discr, mountinfo *mi);

struct dirent dirent;

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
	fs_node *res = malloc(sizeof(fs_node));
	initrd_discr *discr = (initrd_discr *)(node->mi->discr);
	initrd_inode inode = discr->initrd_inodes[node->inode];
	initrd_folder_entry *entries = (initrd_folder_entry *) (inode.offset+discr->location);
	UINT i = inode.size/sizeof(initrd_folder_entry);
	
	while (i--)
		if (!strcmp(name,entries[i].filename)) {
			filetonode(entries[i].inode,res,name,node->mi->discr,node->mi);
			return res;
		}
	return 0;
}

fs_node *filetonode(UINT inode, fs_node *node, char *name, initrd_discr *discr, mountinfo *mi)
{
	initrd_inode *initrd_inodes = discr->initrd_inodes;
	
	node->mode=initrd_inodes[inode].mode;
	node->gid=initrd_inodes[inode].gid;
	node->uid=initrd_inodes[inode].uid;
	node->flags=initrd_inodes[inode].flags;
	node->inode=initrd_inodes[inode].inode;
	node->size=initrd_inodes[inode].size;
	strcpy(node->name,name);
	node->read=&initrd_read;
	node->write=0;
	node->open=0;
	node->close=0;
	if (initrd_inodes[inode].flags==FS_DIRECTORY) {
		node->readdir=&initrd_readdir;
		node->finddir=&initrd_finddir;
	} else {
		node->readdir=0;
		node->finddir=0;
	}
	node->ptr=0;
	node->mi=mi;
	
	return node;
}

fs_node *setup_initrd(UINT location)
{
	initrd_discr *discr = malloc(sizeof(initrd_discr));
	
	discr->location=location;
	discr->initrdheader=(initrd_header *)location;
	discr->initrd_inodes=(initrd_inode *)(location+sizeof(initrd_header));
	
	discr->initrd_root=malloc(sizeof(fs_node));
	filetonode(0,discr->initrd_root,"initrd",discr,0);
	discr->initrd_root->mi=fs_add_mountpoint(FS_TYPE_INITRD,(void *)discr,0,0,discr->initrd_root);
	return discr->initrd_root;
}

UINT remove_initrd(fs_node *node)
{
	if (node!=((initrd_discr *)(node->mi->discr))->initrd_root) return 1;
	free(node->mi->discr);
	fs_del_mountpoint(node->mi);
	free(node);
	return 0;
}
