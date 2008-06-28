#include <fs/devfs.h>
#include <mm.h>

fs_node *mkdevfs()
{
	/*fs_node *dev = (fs_node*) malloc(sizeof(fs_node));
	dev->mode=0x1ED;	//rwxr-xr-x
	dev->flags=FS_DIRECTORY;
	dev->uid=dev->gid=dev->inode=dev->size=0;
	dev->read=0;
	dev->write=0;
	dev->open=0;
	dev->close=0;
	//dev->readdir=&fat32_readdir;
	//dev->finddir=&fat32_finddir;
	dev->ptr=0;
	
	return dev;*/
	return 0;
}
