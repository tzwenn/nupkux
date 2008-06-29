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
