#include <fs/fs.h>

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
