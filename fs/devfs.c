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
