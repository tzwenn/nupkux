#include <fs/fs.h>
#include <lib/string.h>
#include <mm.h>

#include <kernel/ktextio.h>

static fs_node *copy_node(fs_node *node)
{
	if (!node) return 0;
	fs_node *res=malloc(sizeof(fs_node));
	
	memcpy(res,node,sizeof(fs_node));
	return res;
}

fs_node *namei(char *filename)
{
	char sname[NODE_NAME_LEN], *end=filename, tmp;
	fs_node *node = get_fs_root_node(), *newnode;
	
	if (!node) return 0;
	if (!filename) return 0;
	if (!strcmp(filename,"/")) return node;
	if (filename[0]=='/') filename++;
	while (end) {
		if ((end=strchr(filename,'/'))) {
			tmp=*end;
			*end=0;
			strcpy(sname,filename);
			*end=tmp;
		} else strcpy(sname,filename);
		filename=end+1;
		newnode=finddir_fs(node,sname);
		free(node);
		if (!newnode) return 0;
		//FIXME: According to this every file could be a mountpoint
		if (newnode->flags&FS_MOUNTPOINT) {
			node=copy_node(newnode->ptr);
			free(newnode);
		} else node=newnode;
	}
	printf("\n");
	return node;
}
