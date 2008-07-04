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
#include "fdc.h"
#include <kernel/ktextio.h>
#include <time.h>
#include <fs/devfs.h>
#include <lib/memory.h>

void ResetFloppy();
void reset(void);
UINT diskchange(void);
void motoron(void);
void motoroff(void);
void recalibrate(void);
UINT flseek(int track);
UINT log_disk(DrvGeom *g);
UINT format_track(UCHAR track,DrvGeom *g);
UINT fdc_read_block(int block,UCHAR *blockbuff, ULONG nosectors);

/* globals */
static volatile UCHAR done = 0;
static UCHAR dchange = 0;
UINT motor = 0;
int mtick = 0;
int tmout = 0;
static UCHAR status[7] = { 0 };
static UCHAR statsz = 0;
static UCHAR sr0 = 0;
static UCHAR fdc_track = 0xff;
static DrvGeom geometry = { DG144_HEADS,DG144_TRACKS,DG144_SPT };

unsigned long tbaddr = 0x80000L;    /* physical address of track buffer located below 1M */

/* prototypes */
extern void floppy_ISR();
extern void _int1c();

void sendbyte(int byte);
int getbyte();
void FloppyIRQ( struct regs* r );
UINT waitfdc(UINT sensei);
UINT fdc_rw(int block,UCHAR *blockbuff,UINT read,ULONG nosectors);

/* helper functions */

void sendbyte(int byte)
{
   volatile int msr;
   int tmo;
   
   for (tmo = 0;tmo < 128;tmo++) {
      msr = inportb(FDC_MSR);
      if ((msr & 0xc0) == 0x80) {
	 outportb(FDC_DATA,byte);
	 return;
      }
      inportb(0x80);  
   }
}

/* getbyte() routine from intel manual */
int getbyte()
{
   volatile int msr;
   int tmo;
   
   for (tmo = 0;tmo < 128;tmo++) {
      msr = inportb(FDC_MSR);
      if ((msr & 0xd0) == 0xd0) {
	 return inportb(FDC_DATA);
      }
      inportb(0x80);   /* delay */
   }

   return -1;   /* read timeout */
}

/* this waits for FDC command to complete */
UINT waitfdc(UINT sensei)
{
   tmout = 1000;   /* set timeout to 1 second */
     
   /* wait for IRQ6 handler to signal command finished */
   while (!done && tmout )
     ;
   /* read in command result bytes */
   statsz = 0;
   while ((statsz < 7) && (inportb(FDC_MSR) & (1<<4))) {
      status[statsz++] = getbyte();
   }

   if (sensei) {
      /* send a "sense interrupt status" command */
      sendbyte(CMD_SENSEI);
      sr0 = getbyte();
      fdc_track = getbyte();
   }
   
   done = 0;
   
   if (!tmout) {
      /* timed out! */
      if (inportb(FDC_DIR) & 0x80)  /* check for diskchange */
	dchange = 1;
      return 0;
   } else
     return 1;
}

/* This is the IRQ6 handler */
void FloppyIRQ( struct regs* r )
{
   /* signal operation finished */
   done = 1;

   /* EOI the PIC */
   outportb(0x20,0x20);
}

/*
 * converts linear block address to head/track/sector
 * 
 * blocks are numbered 0..heads*tracks*spt-1
 * blocks 0..spt-1 are serviced by head #0
 * blocks spt..spt*2-1 are serviced by head 1
 * 
 * WARNING: garbage in == garbage out
 */
void block2hts(int block,int *head,int *track,int *sector)
{
   *head = (block % (geometry.spt * geometry.heads)) / (geometry.spt);
   *track = block / (geometry.spt * geometry.heads);
   *sector = block % geometry.spt + 1;
}

/**** disk operations ****/

// reset the floppy disk - this is MY way!
void ResetFloppy()
{
	// reset the drive
	outportb( FDC_DOR, 0x00 );
	outportb( FDC_DOR, 0x0C );
	outportb( FDC_CCR, 0x00 );

	// set the flag
	done = 1;

	// wait for the interrupt...
	while( done == 1 );

	outportb( FDC_DOR, 0x0C );
}

/* this gets the FDC to a known state */
void reset(void)
{
   outportb(FDC_DOR,0);
   mtick = 0;
   motor = 0;
   outportb(FDC_DRS,0);
   outportb(FDC_DOR,0x0c);
   done = 1;
   sendbyte(CMD_SPECIFY);
   sendbyte(0xdf);  /* SRT = 3ms, HUT = 240ms */
   sendbyte(0x02);  /* HLT = 16ms, ND = 0 */
   flseek(1);
   recalibrate();
   dchange = 0;
}

/* this returns whether there was a disk change */
UINT diskchange()
{
	return dchange;
}

void motoron()
{
	if (!motor) {
		mtick=-1;
		outportb(FDC_DOR,0x1c);
		sleep(500);
		motor=1;
	}
}

void motoroff()
{
	if (motor) {
		mtick=2000;
	}
}

/* recalibrate the drive */
void recalibrate(void)
{
   /* send actual command bytes */
   sendbyte(CMD_RECAL);
   sendbyte(0);
   /* wait until seek finished */
   waitfdc(1);
   /* turn the motor off */
  // motoroff();
}

/* seek to track */
UINT flseek(int track)
{
   if (fdc_track == track)  /* already there? */
     return 1;
   if( !motor )
     motoron();
   /* send actual command bytes */
   sendbyte(CMD_SEEK);
   sendbyte(0);
   sendbyte(track);
   /* wait until seek finished */
   if (!waitfdc(1))
     return 0;     /* timeout! */
   /* now let head settle for 15ms */
   sleep(15);
   /* check that seek worked */
   return  !((sr0 != 0x20) || (fdc_track != track));
}

/* checks drive geometry - call this after any disk change */
UINT log_disk(DrvGeom *g)
{
   /* get drive in a known status before we do anything */
   reset();

   /* assume disk is 1.68M and try and read block #21 on first track */
   geometry.heads = DG168_HEADS;
   geometry.tracks = DG168_TRACKS;
   geometry.spt = DG168_SPT;

   if (fdc_read_block(20,0,1)) {
      /* disk is a 1.68M disk */
      if (g) {
	 g->heads = geometry.heads;
	 g->tracks = geometry.tracks;
	 g->spt = geometry.spt;
      }
      return 1;
   }
   /* OK, not 1.68M - try again for 1.44M reading block #18 on first track */
   geometry.heads = DG144_HEADS;
   geometry.tracks = DG144_TRACKS;
   geometry.spt = DG144_SPT;

   if (fdc_read_block(17,0,1)) {
      /* disk is a 1.44M disk */
      if (g) {
	 g->heads = geometry.heads;
	 g->tracks = geometry.tracks;
	 g->spt = geometry.spt;
      }
      return 1;
   }
   
   /* it's not 1.44M or 1.68M - we don't support it */
   return 1;
}

/* read block (blockbuff is 512 byte buffer) */
UINT fdc_read_block(int block,UCHAR *blockbuff, ULONG nosectors)
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

/* write block (blockbuff is 512 byte buffer) */
UINT fdc_write_block(int block,UCHAR *blockbuff, ULONG nosectors)
{
	return fdc_rw(block,blockbuff,0,nosectors);
}

/*
 * since reads and writes differ only by a few lines, this handles both.  This
 * function is called by read_block() and write_block()
 */
UINT fdc_rw(int block,UCHAR *blockbuff,UINT read,ULONG nosectors)
{
   int head,track,sector,tries, copycount = 0;
   UCHAR *p_tbaddr = (UCHAR *)0x80000;
   UCHAR *p_blockbuff = blockbuff;

   /* convert logical address into physical address */
   block2hts(block,&head,&track,&sector);

   /* spin up the disk */
   motoron();

   if (!read && blockbuff) {
      /* copy data from data buffer into track buffer */
      for(copycount=0; copycount<(nosectors*512); copycount++) {
      	*p_tbaddr = *p_blockbuff;
      	p_blockbuff++;
      	p_tbaddr++;
      }
   }

   for (tries = 0;tries < 3;tries++) {
      /* check for diskchange */
      if (inportb(FDC_DIR) & 0x80) {
	 dchange = 1;
	 flseek(1);  /* clear "disk change" status */
	 recalibrate();
	 motoroff();
	 
	 return fdc_rw(block, blockbuff, read, nosectors);
      }
      /* move head to right track */
      if (!flseek(track)) {
	 motoroff();
	 return 0;
      }
      
      /* program data rate (500K/s) */
      outportb(FDC_CCR,0);
      
      /* send command */
      if (read) {
	 dma_xfer(2,tbaddr,nosectors*512,0);
	 sendbyte(CMD_READ);
      } else {
	 dma_xfer(2,tbaddr,nosectors*512,1);
	 sendbyte(CMD_WRITE);
      }

      sendbyte(head << 2);
      sendbyte(track);
      sendbyte(head);
      sendbyte(sector);
      sendbyte(2);               /* 512 bytes/sector */
      sendbyte(geometry.spt);
      if (geometry.spt == DG144_SPT)
	sendbyte(DG144_GAP3RW);  /* gap 3 size for 1.44M read/write */
      else
	sendbyte(DG168_GAP3RW);  /* gap 3 size for 1.68M read/write */
      sendbyte(0xff);            /* DTL = unused */
      
      /* wait for command completion */
      /* read/write don't need "sense interrupt status" */
      if (!waitfdc(1)) {
	reset();
	return fdc_rw(block, blockbuff, read, nosectors);
      }
      
      if ((status[0] & 0xc0) == 0) break;   /* worked! outta here! */

      recalibrate();  /* oops, try again... */
   }
   
   /* stop the motor */
   motoroff();

   if (read && blockbuff) {
      /* copy data from track buffer into data buffer */
      p_blockbuff = blockbuff;
      p_tbaddr = (UCHAR *) 0x80000;
      for(copycount=0; copycount<(nosectors*512); copycount++) {
      	*p_blockbuff = *p_tbaddr;
      	p_blockbuff++;
      	p_tbaddr++;
      }
   }

   return (tries != 3);
}

/* this formats a track, given a certain geometry */
UINT format_track(UCHAR track,DrvGeom *g)
{
   int i,h,r,r_id,split;
   UCHAR tmpbuff[256];
   UCHAR *p_tbaddr = (UCHAR *)0x8000;
   UINT copycount = 0;

   /* check geometry */
   if (g->spt != DG144_SPT && g->spt != DG168_SPT)
     return 0;
   
   /* spin up the disk */
   motoron();

   /* program data rate (500K/s) */
   outportb(FDC_CCR,0);

   flseek(track);  /* seek to track */

   /* precalc some constants for interleave calculation */
   split = g->spt / 2;
   if (g->spt & 1) split++;
   
   for (h = 0;h < g->heads;h++) {
      if (inportb(FDC_DIR) & 0x80) {
	 dchange = 1;
	 flseek(1);  /* clear "disk change" status */
	 recalibrate();
	 motoroff();
	 return 0;
      }

      i = 0;   /* reset buffer index */
      for (r = 0;r < g->spt;r++) {
	 /* for each sector... */

	 /* calculate 1:2 interleave (seems optimal in my system) */
	 r_id = r / 2 + 1;
	 if (r & 1) r_id += split;
	 
	 /* add some head skew (2 sectors should be enough) */
	 if (h & 1) {
	    r_id -= 2;
	    if (r_id < 1) r_id += g->spt;
	 }
      
	 /* add some track skew (1/2 a revolution) */
	 if (track & 1) {
	    r_id -= g->spt / 2;
	    if (r_id < 1) r_id += g->spt;
	 }
	 
	 /**** interleave now calculated - sector ID is stored in r_id ****/

	 /* fill in sector ID's */
	 tmpbuff[i++] = track;
	 tmpbuff[i++] = h;
	 tmpbuff[i++] = r_id;
	 tmpbuff[i++] = 2;
      }

      /* copy sector ID's to track buffer */
      for(copycount = 0; copycount<i; copycount++) {
      	*p_tbaddr = tmpbuff[copycount];
      	p_tbaddr++;
      }
//      movedata(_my_ds(),(long)tmpbuff,_dos_ds,tbaddr,i);
      
      /* start dma xfer */
      dma_xfer(2,tbaddr,i,1);
      
      /* prepare "format track" command */
      sendbyte(CMD_FORMAT);
      sendbyte(h << 2);
      sendbyte(2);
      sendbyte(g->spt);
      if (g->spt == DG144_SPT)      
	sendbyte(DG144_GAP3FMT);    /* gap3 size for 1.44M format */
      else
	sendbyte(DG168_GAP3FMT);    /* gap3 size for 1.68M format */
      sendbyte(0);     /* filler byte */
	 
      /* wait for command to finish */
      if (!waitfdc(0))
	return 0;
      if (status[0] & 0xc0) {
	 motoroff();
	 return 0;
      }
   }
   motoroff();
   return 1;
}

static UINT drv_floppy_read(fs_node *node, UINT offset, UINT size, UCHAR *buffer)
{
	UCHAR tmpbuf[FLOPPY_SECTOR_SIZE];
	UINT block_s = offset/FLOPPY_SECTOR_SIZE,block_e;
	
	if (node->p_data) return 0; //A loop would be better on multitasking
	node->p_data=(void *)1;
	if (offset>FLOPPY_144IN_SIZE) return 0;
	if (offset+size>FLOPPY_144IN_SIZE) size=FLOPPY_144IN_SIZE-offset;
	block_e=(offset+size)/FLOPPY_SECTOR_SIZE;
	
	fdc_read_block(block_e,tmpbuf,1);
	memcpy(buffer+(((offset+size)/FLOPPY_SECTOR_SIZE)*FLOPPY_SECTOR_SIZE-offset),tmpbuf,FLOPPY_SECTOR_SIZE-(offset%FLOPPY_SECTOR_SIZE));
	
	fdc_read_block(block_s,tmpbuf,1);
	memcpy(buffer,tmpbuf+(offset%FLOPPY_SECTOR_SIZE),FLOPPY_SECTOR_SIZE-(offset%FLOPPY_SECTOR_SIZE));
	node->p_data=0;
	return 0;
}

static UINT drv_floppy_write(fs_node *node, UINT offset, UINT size, UCHAR *buffer)
{
	return 0;
}

node_operations floppy_ops = {0,&drv_floppy_read,&drv_floppy_write,0,0,0};

void init_floppy(fs_node *devfs)
{
	outportb(0x70,0x10);
	if (!inportb(0x71)) return;
	irq_install_handler(6,FloppyIRQ);
	reset();
	devfs_register_device(devfs,"fd0",0660,FS_UID_ROOT,FS_GID_ROOT,FS_BLOCKDEVICE,&floppy_ops);
}
