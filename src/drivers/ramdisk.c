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

#include <drivers/ramdisk.h>

static int ramdisk_request(vnode *node, int cmd, ULONG sector, ULONG count, char *buffer)
{
	device_lock(node);
	if (sector > RAMDISK_SECTOR_COUNT) return 0;
	if (sector + count > RAMDISK_SECTOR_COUNT)
		count = RAMDISK_SECTOR_COUNT - sector;
	size_t size = count * RAMDISK_SECTOR_SIZE;
	off_t off = sector * RAMDISK_SECTOR_SIZE;
	switch (cmd) {
	case REQUEST_READ:
		memcpy(buffer, ((char *)node->u.devfs_i->pdata) + off, size);
		break;
	case REQUEST_WRITE:
		memcpy(((char *)node->u.devfs_i->pdata) + off, buffer, size);
		break;
	}
	device_unlock(node);
	return count;
}

static void ramdisk_free_pdata(void *pdata)
{
	free(pdata);
}

file_operations ramdisk_ops = {
request:
	ramdisk_request,
free_pdata:
	ramdisk_free_pdata,
};

void setup_ramdisk(void)
{
	devfs_handle *dev = devfs_register_device(NULL, "ram0", 0660, FS_UID_ROOT, FS_GID_ROOT, FS_BLOCKDEVICE, &ramdisk_ops);
	dev->pdata = calloc(RAMDISK_SECTOR_COUNT, RAMDISK_SECTOR_SIZE);
	dev->bcount = RAMDISK_SECTOR_COUNT;
	dev->bsize = RAMDISK_SECTOR_SIZE;
	/*////////////////////////////////
	vnode *floppy = namei("/dev/fd0", 0);
	if (floppy) {
		request_fs(floppy, REQUEST_READ, 0, RAMDISK_SECTOR_COUNT, dev->pdata);
	}
	iput(floppy);
	////////////////////////////////*/
}
