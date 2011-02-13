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

#include <fs/devfs.h>

static vnode *devfs_lookup(vnode *dir, const char *name)
{
	devfs_handle *handle = dir->u.devfs_i;
	if (!handle) return 0;
	devfs_d_entry *entries = handle->pdata;
	UINT i = dir->size / sizeof(devfs_d_entry);
	while (i--) {
		if (namei_match(name, entries[i].filename))
			return iget(dir->sb, entries[i].inode);
	}
	return 0;
}

extern file_operations devfs_dir_fop;

inode_operations devfs_i_ops = {
f_op:
	&devfs_dir_fop,
lookup:
	&devfs_lookup,
};
