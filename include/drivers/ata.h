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

#ifndef _ATA_H
#define _ATA_H

#include <drivers/drivers.h>

#define ATA_IDE_1	0x1F0
#define ATA_IDE_2	0x170

#define ATA_MASTER	0x00
#define ATA_SLAVE	0x01

#define ATA_BASE	0
#define ATA_FEAT	1
#define ATA_SECS	2
#define ATA_SNUM	3
#define ATA_CLOW	4
#define ATA_CHIH	5
#define ATA_HDEV	6
#define ATA_CMD		7
#define ATA_CHK2	0x206

#define ATA_READ_CMD	0x20
#define ATA_WRITE_CMD	0x30

extern int lba28_read(UCHAR* buf, USHORT controller, UCHAR drive, UINT addr, UCHAR sectorcount);
extern int lba28_write(UCHAR* buf, USHORT controller, UCHAR drive, UINT addr, UCHAR sectorcount);

#endif
