#include <fs/fs.h>
#include <mm.h>

fs_node *fs_root = 0;
mountinfo *mountinfos = 0;

void open_fs(fs_node *node, UCHAR read, UCHAR write)
{
	if (node->open) return node->open(node);
}

UINT read_fs(fs_node *node, UINT offset, UINT size, UCHAR *buffer)
{
	if (node->read) return node->read(node,offset,size,buffer);
		else return 0;
}

UINT write_fs(fs_node *node, UINT offset, UINT size, UCHAR *buffer)
{
	if (node->write) return node->write(node,offset,size,buffer);
		else return 0;
}

void close_fs(fs_node *node)
{
	if (node->close) return node->close(node);
}

struct dirent *readdir_fs(fs_node *node, UINT index)
{
	if ((node->flags&FS_DIRECTORY) && (node->readdir)) return node->readdir(node,index);
		else return 0;
}

fs_node *finddir_fs(fs_node *node, char *name)
{
	if ((node->flags&FS_DIRECTORY) && (node->finddir)) return node->finddir(node,name);
		else return 0;
}

mountinfo *fs_add_mountpoint(UINT fs_type, void *discr, fs_node *mountpoint, fs_node *device, fs_node *root)
{
	mountinfo *mi = malloc(sizeof(mountinfo));
	
	mi->fs_type=fs_type;
	mi->discr=discr;
	mi->mountpoint=mountpoint;
	mi->device=device;
	mi->next=mountinfos;
	mountinfos=mi;
	if (mountpoint) {
		mountpoint->flags|=FS_MOUNTPOINT;
		mountpoint->ptr=root;
	}

	return mi;
}

UINT fs_del_mountpoint(mountinfo *mi)
{
	mountinfo *pre=0,*tmp=mountinfos;
	
	while (tmp) {
		if (tmp==mi) break;
		pre=tmp;
		tmp=tmp->next;
	}
	if (!tmp) return 0;
	if (!pre) mountinfos=tmp->next;
		else pre->next=tmp->next;
	free(mi);
	return 1;
}

fs_node *get_fs_root_node()
{
	if (!fs_root) return 0;
	fs_node *res=malloc(sizeof(fs_node));
	
	memcpy(res,fs_root,sizeof(fs_node));
	return res;
}

fs_node *set_fs_root_node(fs_node* new_root)
{
	if (fs_root) free(fs_root);
	fs_root=0;
	if (!new_root) return 0;
	fs_root=malloc(sizeof(fs_node));
	memcpy(fs_root,new_root,sizeof(fs_node));
	
	return new_root;
}
