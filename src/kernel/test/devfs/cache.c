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

#include "devfs.h"

static devfs_handle *cache = 0;

void devfs_add_to_cache(devfs_handle *handle)
{
	handle->cache_next=cache;
	cache=handle;
}

void devfs_del_from_cache(devfs_handle *handle)
{
	devfs_handle *tmp=cache,*prev=0;
	while (tmp) {
		if (handle==tmp) break;
		prev=tmp;
		tmp=tmp->cache_next;
	}
	if (!prev) cache=handle->cache_next;
		else prev->cache_next=handle->cache_next;
	if (IS_DIR2(handle)) free(handle->pdata); //Other devices?
	free(handle);
}

devfs_handle *devfs_iget(ULONG ino)
{
	devfs_handle *handle=cache;
	while (handle) {
		if (handle->ino==ino) return handle;
		handle=handle->cache_next;
	}
	return 0;
}

void devfs_free_cache(void)
{
	devfs_handle *tmp;
	while (cache) {
		tmp=cache->cache_next;
		if (IS_DIR2(cache)) free(cache->pdata);
		free(cache);
		cache=tmp;
	}
}
