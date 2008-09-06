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

#include "vfs.h"
#include <lib/string.h>
#include <errno.h>

static filesystem_t *filesystems = 0;

filesystem_t *vfs_get_fs(const char *name)
{
	filesystem_t *fs=filesystems;

	if (!name) return 0;
	while (fs) {
		if (!strcmp(fs->name,name)) break;
		fs=fs->next;
	}
	return fs;
}

int init_vfs(void)
{
	return 0;
}

int register_filesystem(filesystem_t *fs)
{
	if (!fs) return -EINVAL;
	if (fs->next || vfs_get_fs(fs->name)) return -EBUSY;
	fs->next=filesystems;
	filesystems=fs;
	return 0;
}

int unregister_filesystem(filesystem_t *fs)
{
	filesystem_t *prev=0, *tmp=filesystems;

	if (!fs || !fs->name) return -EINVAL;
	while (tmp) {
		if (!strcmp(fs->name,tmp->name)) break;
		prev=tmp;
		tmp=tmp->next;
	}
	if (!tmp) return -EINVAL;
	if (!prev) filesystems=filesystems->next;
		else {
			prev->next=tmp->next;
			tmp->next=0;
		}
	return 0;
}

