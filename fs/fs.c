#include <fs/fs.h>
#include <mm.h>

fs_node fs_root;
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

mountinfo *fs_add_mountpoint(UINT filesystem, void *discr, fs_node *mountpoint, char *device)
{
	mountinfo *mi = malloc(sizeof(mountinfo));
	
	mi->filesystem=filesystem;
	mi->discr=discr;
	mi->mountpoint=mountpoint;
	mi->device=device;
	mi->next=mountinfos;
	mountinfos=mi;

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
