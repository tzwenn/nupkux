/*
 *  Copyright (C) 2008 Sven Köhler
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

#include <task.h>
#include <fs/fs.h>
#include <errno.h>

int sys_execve(const char *file,char **argv,char **envp)
{
	//TODO: permission check, PATH, only './' for programs in .
	UCHAR *buf;
	int ret,argc=0;
	int (*main)(int,char **,char **);
	fs_node *node=namei(file);

	if (!node)
		node=finddir_fs(namei("/bin"),file);
	if (!node)
		return -ENOENT;
	open_fs(node,1,0);
	buf=(UCHAR *)malloc(node->size);
	read_fs(node,0,node->size,buf);
	while (argv[argc]) argc++;
	main=(int (*)(int,char **,char **))buf;
	ret=main(argc,argv,envp);
	free(buf);
	close_fs(node);
	return ret;
}
