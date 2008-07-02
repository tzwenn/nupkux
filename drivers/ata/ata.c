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

#include <time.h>
#include <kernel/ktextio.h>
#include <drivers/ata.h>

int lba28_init(USHORT controller, UCHAR drive, UINT addr, UCHAR sectorcount) 
{
	while ((inportb(controller + ATA_CHK2) & 0x80)) {_kabort_func_return(0)};
	outportb(controller | ATA_FEAT,0x00);
	outportb(controller | ATA_SECS,sectorcount); 
	outportb(controller | ATA_SNUM,(UCHAR) addr);
	outportb(controller | ATA_CLOW,(UCHAR) (addr>>8));
	outportb(controller | ATA_CHIH,(UCHAR) (addr>>16));
	outportb(controller | ATA_HDEV,0xE0 | (UCHAR) (drive << 4) | (UCHAR) ((addr >> 24) & (0x0F)));
	return 1;
}

/*void lba48_init(USHORT controller, UCHAR drive, ULONG addr, USHORT sectorcount) 
{
	outportb(controller | 0x01,0x00);
	outportb(controller | 0x01,0x00);
	outportb(controller | 0x02,(UCHAR) ((sectorcount>>8) & 0xFF)); 
	outportb(controller | 0x02,(UCHAR) (sectorcount & 0xFF)); 
	outportb(controller | 0x03,(UCHAR) ((addr>>24) & 0xFF));
	outportb(controller | 0x03,(UCHAR) (addr & 0xFF));
	outportb(controller | 0x04,(UCHAR) ((addr>>32) & 0xFF));
	outportb(controller | 0x04,(UCHAR) ((addr>>8) & 0xFF));
	outportb(controller | 0x05,(UCHAR) ((addr>>40) & 0xFF));
	outportb(controller | 0x05,(UCHAR) ((addr>>16) & 0xFF));
	outportb(controller | 0x06,0x40 | (drive << 4));
}*/

int lba28_read(UCHAR* buf, USHORT controller, UCHAR drive, UINT addr, UCHAR sectorcount)
{
	int i;
	USHORT value;

	controller=controller & 0xFFF0;
	if ((controller!=ATA_IDE_1) && (controller!=ATA_IDE_2)) return 0;
	if ((drive!=ATA_MASTER) && (drive!=ATA_SLAVE)) return 0;
	if (!lba28_init(controller,drive,addr,sectorcount)) return 0;
	outportb(controller | ATA_CMD,ATA_READ_CMD);
	while (!(inportb(controller | ATA_CMD) & 0x08)) {_kabort_func_return(0)};
	for (i=0;i<256*(sectorcount+1);i++) {
		value=inportw(controller);
		buf[i*2]=(UCHAR) (value & 0xFF);
		buf[i*2+1]=(UCHAR) ((value & 0xFF00) >> 8);
	}
	return 1;
}

int lba28_write(UCHAR* buf, USHORT controller, UCHAR drive, UINT addr, UCHAR sectorcount)
{
	int i;
	USHORT value;

	controller=controller & 0xFFF0;
	if ((controller!=ATA_IDE_1) && (controller!=ATA_IDE_2)) return 0;
	if ((drive!=ATA_MASTER) && (drive!=ATA_SLAVE)) return 0;
	if (!lba28_init(controller,drive,addr,sectorcount)) return 0;
	outportb(controller | ATA_CMD,ATA_WRITE_CMD);
	while (!(inportb(controller | ATA_CMD) & 0x08)) {_kabort_func_return(0)};
	for (i=0;i<256*(sectorcount+1);i++) {
		value=buf[i*2];
		value|=buf[i*2+1] << 8;
		outportw(controller,value);
	}
	return 1;
}
