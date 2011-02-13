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

#include <drivers/drivers.h>
#include <kernel/ktextio.h>

int fs_read_block(super_block *sb, ULONG block, ULONG count, char *buffer)
{
	UINT start_sector = sb->skip_bytes / sb->dev->u.devfs_i->bsize;
	UINT sec_block = sb->blocksize / sb->dev->u.devfs_i->bsize;
	//FIXME: I assume sb->blocksize > sb->dev->u.devfs_i->bsize
	return request_fs(sb->dev, REQUEST_READ, block * sec_block + start_sector, count * sec_block, buffer);
}
