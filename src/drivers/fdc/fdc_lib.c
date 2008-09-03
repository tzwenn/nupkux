/*
GazOS Operating System
Copyright (C) 1999  Gareth Owen <gaz@athene.co.uk>
Copyright (C) 2007,2008  Sven Köhler

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "dma.h"
#include <drivers/fdc.h>
#include <time.h>
#include <lib/memory.h>
#include <kernel/dts.h>
#include <errno.h>

/* globals */
static volatile UCHAR done = 0;
static volatile pid_t thepid;
static UCHAR dchange = 0;
static UINT motor = 0;
static int mtick = 0;
static int tmout = 0;
static UCHAR status[7] = {0};
static UCHAR statsz = 0;
static UCHAR sr0 = 0;
static UCHAR fdc_track = 0xff;
static DrvGeom geometry = {DG144_HEADS,DG144_TRACKS,DG144_SPT};

static unsigned long tbaddr = 0x80000L;    /* physical address of track buffer located below 1M */

static void recalibrate(void);
static UINT flseek(int track);
static UINT fdc_rw(int block,char *blockbuff,UINT read,ULONG nosectors);

/* helper functions */
static void sendbyte(int byte)
{
	volatile int msr;
	int tmo;

	for (tmo=0;tmo<128;tmo++) {
		msr = inportb(FDC_MSR);
		if ((msr&0xc0)==0x80) {
			outportb(FDC_DATA,byte);
			return;
		}
		inportb(0x80);
	}
}

/* getbyte() routine from intel manual */
static int getbyte(void)
{
	volatile int msr;
	int tmo;

	for (tmo=0;tmo<128;tmo++) {
		msr = inportb(FDC_MSR);
		if ((msr&0xd0)==0xd0) {
			return inportb(FDC_DATA);
		}
		inportb(0x80);   /* delay */
	}
	return -1;   /* read timeout */
}

/* this waits for FDC command to complete */
static UINT waitfdc(UINT sensei)
{
	tmout = 1000;   /* set timeout to 1 second */
	/* wait for IRQ6 handler to signal command finished */
	thepid=sys_getpid();
	while (!done && tmout) sys_pause();
	/* read in command result bytes */
	statsz = 0;
	while ((statsz<7) && (inportb(FDC_MSR)&(1<<4))) {
		status[statsz++] = getbyte();
	}
	if (sensei) { /* send a "sense interrupt status" command */
		sendbyte(CMD_SENSEI);
		sr0=getbyte();
		fdc_track=getbyte();
	}
	if (!tmout) { /* timed out! */
		if (inportb(FDC_DIR) & 0x80)  /* check for diskchange */
		dchange=1;
		return 0;
	} else return 1;
}

static void FloppyIRQ(registers *regs)
{
	sys_kill(thepid,SIGCONT);
	outportb(0x20,0x20);
}

static void block2hts(int block,int *head,int *track,int *sector)
{
	*head=(block%(geometry.spt*geometry.heads))/(geometry.spt);
	*track=block/(geometry.spt*geometry.heads);
	*sector=block%geometry.spt+1;
}

/**** disk operations ****/

/* this gets the FDC to a known state */
static void reset(void)
{
	outportb(FDC_DOR,0);
	mtick=0;
	motor=0;
	outportb(FDC_DRS,0);
	outportb(FDC_DOR,0x0c);
	done=1;
	sendbyte(CMD_SPECIFY);
	sendbyte(0xdf);  /* SRT = 3ms, HUT = 240ms */
	sendbyte(0x02);  /* HLT = 16ms, ND = 0 */
	flseek(1);
	recalibrate();
	dchange = 0;
}

static void motoron(void)
{
	if (!motor) {
		mtick=-1;
		outportb(FDC_DOR,0x1c);
		sleep(500);
		motor=1;
	}
}

static void motoroff(void)
{
	if (motor) {
		mtick=2000;
	}
}

static void recalibrate(void) /* recalibrate the drive */
{
	/* send actual command bytes */
	sendbyte(CMD_RECAL);
	sendbyte(0);
	/* wait until seek finished */
	waitfdc(1);
}

/* seek to track */
static UINT flseek(int track)
{
	if (fdc_track==track) return 1;	/* already there? */
	if(!motor) motoron();
	/* send actual command bytes */
	sendbyte(CMD_SEEK);
	sendbyte(0);
	sendbyte(track);
	/* wait until seek finished */
	if (!waitfdc(1)) return 0;	/* timeout! */
	/* now let head settle for 15ms */
	sleep(15);
	/* check that seek worked */
	return  !((sr0!=0x20) || (fdc_track!=track));
}

/*since reads and writes differ only by a few lines, this handles both.*/
static UINT fdc_rw(int block,char *blockbuff,UINT cmd_read,ULONG nosectors)
{
	int head,track,sector,tries, copycount = 0;
	char *p_tbaddr = (char *)0x80000;
	char *p_blockbuff = blockbuff;

	block2hts(block,&head,&track,&sector); /* convert logical address into physical address */
	motoron(); /* spin up the disk */
	if (!cmd_read && blockbuff) {
		/* copy data from data buffer into track buffer */
		for(copycount=0;copycount<(nosectors*FLOPPY_SECTOR_SIZE);copycount++) {
			*p_tbaddr=*p_blockbuff;
			p_blockbuff++;
			p_tbaddr++;
		}
	}
	for (tries=0;tries<3;tries++) {
		if (inportb(FDC_DIR) & 0x80) { /* check for diskchange */
			dchange=1;
			flseek(1);  /* clear "disk change" status */
			recalibrate();
			motoroff();
			return fdc_rw(block, blockbuff, cmd_read, nosectors);
		}
		if (!flseek(track)) {/* move head to right track */
			motoroff();
			return 0;
		}
		outportb(FDC_CCR,0);/* program data rate (500K/s) */
		if (cmd_read) { /* send command */
			dma_xfer(2,tbaddr,nosectors*FLOPPY_SECTOR_SIZE,0);
			sendbyte(CMD_READ);
		} else {
			dma_xfer(2,tbaddr,nosectors*FLOPPY_SECTOR_SIZE,1);
			sendbyte(CMD_WRITE);
		}
		sendbyte(head<<2);
		sendbyte(track);
		sendbyte(head);
		sendbyte(sector);
		sendbyte(2);               /* 512 bytes/sector */
		sendbyte(geometry.spt);
		if (geometry.spt == DG144_SPT) sendbyte(DG144_GAP3RW);  /* gap 3 size for 1.44M read/write */
			else sendbyte(DG168_GAP3RW);  /* gap 3 size for 1.68M read/write */
		sendbyte(0xFF);            /* DTL = unused */
		/* wait for command completion */
		/* read/write don't need "sense interrupt status" */
		if (!waitfdc(1)) {
			reset();
			return fdc_rw(block, blockbuff, cmd_read, nosectors);
		}
		if (!(status[0]&0xC0)) break;   /* worked! outta here! */
		recalibrate();  /* oops, try again... */
	}
	motoroff();
	if (cmd_read && blockbuff) {
		/* copy data from track buffer into data buffer */
		p_blockbuff=blockbuff;
		p_tbaddr = (char *) 0x80000;
		for(copycount=0; copycount<(nosectors*FLOPPY_SECTOR_SIZE);copycount++) {
			*p_blockbuff=*p_tbaddr;
			p_blockbuff++;
			p_tbaddr++;
		}
	}
	return (tries!=3);
}

/* read block (blockbuff is 512 byte buffer) */
UINT fdc_read_block(int block,char *blockbuff, ULONG nosectors)
{
	int track=0, sector=0, head=0, track2=0, result=0, loop=0;
	block2hts(block, &head, &track, &sector);
	block2hts(block+nosectors, &head, &track2, &sector);

	if(track!=track2)
	{
		for(loop=0; loop<nosectors; loop++)
			result = fdc_rw(block+loop, blockbuff+(loop*512), 1, 1);
		return result;
	}
	return fdc_rw(block,blockbuff,1,nosectors);
}

static int floppy_request(fs_node *node, int cmd, ULONG sector, ULONG count, char *buffer)
{
	if (sector>=FLOPPY_SECTOR_COUNT) return -EINVAL;
	device_lock(node);
	if (sector+count>=FLOPPY_SECTOR_COUNT)
		count=FLOPPY_SECTOR_COUNT-sector;
	switch (cmd) {
		case REQUEST_READ:
			fdc_read_block(sector,buffer,count);
			break;
		case REQUEST_WRITE:
			fdc_rw(sector,buffer,0,count);
			break;
	}
	device_unlock(node);
	return count;
}

node_operations floppy_ops = {
		request: floppy_request,};

void setup_floppy(fs_node *devfs)
{
	outportb(0x70,0x10);
	if (!inportb(0x71)) return;
	register_interrupt_handler(IRQ6,FloppyIRQ);
	reset();
	device_t *dev=device_discr(devfs_register_device(devfs,"fd0",0660,FS_UID_ROOT,FS_GID_ROOT,FS_BLOCKDEVICE,&floppy_ops));
	dev->bcount=FLOPPY_SECTOR_COUNT;
	dev->bsize=FLOPPY_SECTOR_SIZE;
}
