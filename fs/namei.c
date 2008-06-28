#include <fs/fs.h>
#include <lib/string.h>

fs_node *namei(char *filename)
{
	char sname[NODE_NAME_LEN], *end=filename, tmp;
	fs_node *node = fs_root;
	
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
		if (!*sname) continue;		//A "//" is also valid
		node=finddir_fs(node,sname);
		if (!node) return 0;
		if (node->flags&FS_MOUNTPOINT)
			node=node->ptr;
	}
	return node;
}
