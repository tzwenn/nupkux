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

//A REALLY good source: http://www.nondot.org/sabre/os/files/Communication/ser_port.txt

#define COM1	0x3F8
#define COM2	0x2F8
#define COM3	0x3E8
#define COM4	0x2E8

#define RBR	0	//Receive Buffer Register (read only)
#define THR	0	//Transmitter Holding Register (write only)
#define IER	1	//Interrupt Enable Register
#define FCR	2	//FIFO Control Register
#define LCR	3	//Line Control Register
#define MCR	4	//Modem Control Register
#define LSR	5	//Line Status Register
#define MSR	6	//Modem Status Register
#define DL_LO	0	//Divisor Latch (low byte)
#define DL_HI	1	//Divisor Latch (high byte)

#define NO_INTERRUPTS	0x00	//Disable interrupts
#define ERBFI		0x01	//Enable Receiver Buffer Full Interrupt
#define SET_DLAB	0x80	//Set Divisor Latch Access Bit
#define RX_TRIGGER	0xC0	//RX FIFO trigger level select
#define XFres		0x04	//Transmitter FIFO reset
#define RFres		0x02	//Receiver FIFO reset
#define FCR_enable	0x01	//Should be clear
#define OUT2		0x08	//Enable IRQ
#define RTS		0x02	//Request To Send
#define	DTR		0x01	//Data Terminal Ready
#define	MCR_LOOP	0x10	//Start loopback (for check)
#define LOOP_CLEAR	0x00	//Clear loopback

#define THRE		0x20	//Transmitter Holding Register Empty (THR ready)
#define RBF		0x01	//Receiver Buffer Full (Data Available)

#define WORD_8		0x03	//8n1

#define divisor		1	//115200 baud

static int detect_serial_port(USHORT port)
{
	outportb(port+MCR,MCR_LOOP);
	if ((inportb(port+MSR)&0xF0)) return 0;
	outportb(port+MCR,MCR_LOOP | 0x0F);
	if ((inportb(port+MSR)&0xF0)!=0xF0) return 0;
	return 1;
}

static void serial_open(fs_node *node)
{
	USHORT port=(USHORT) ((UINT) node->p_data);
	outportb(port+IER,NO_INTERRUPTS);
	outportb(port+LCR,SET_DLAB);
	outportb(port+DL_LO,divisor & 0xFF);
	outportb(port+DL_HI,(divisor >> 8) & 0xFF);
	outportb(port+LCR,WORD_8);
	outportb(port+MCR,LOOP_CLEAR);
	outportb(port+FCR,RX_TRIGGER | RFres | XFres | FCR_enable);
	outportb(port+MCR,OUT2 | RTS | DTR);
	//By now this is my first "real hardware" driver, nupkux is without "real" scheduling
	//For this reason this is non-interrupt-driven and therefore ugly one
	//outportb(port+IER,ERBFI);
	node->nlinks++;
}

static int serial_write(fs_node *node, off_t offset, size_t size, const char *buffer)
{
	size_t i=size;
	USHORT port=(USHORT) ((UINT) node->p_data);

	while (i--)  {
		while (!(inportb(port+LSR)&THRE));
		outportb(port+THR,*(buffer++));
	}
	return size;
}

static int serial_read(fs_node *node, off_t offset, size_t size, char *buffer)
{
	size_t i=size;
	USHORT port=(USHORT) ((UINT) node->p_data);

	while (i--) {
		while (!inportb(port+LSR)&RBF);
		*(buffer++)=inportb(port+RBR);
	}
	return size;
}

static void serial_close(fs_node *node)
{
	USHORT port=(USHORT) ((UINT) node->p_data);

	outportb(port+IER,NO_INTERRUPTS);
	outportb(port+MCR,0x00);
	node->nlinks--;
}

static node_operations serial_ops = {
		open: &serial_open,
		read: &serial_read,
		write: &serial_write,
		close: &serial_close,
};

void setup_serial(fs_node *devfs)
{
	if (detect_serial_port(COM1))
		devfs_register_device(devfs,"ttyS0",0660,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&serial_ops)->p_data=(void *)COM1;
	if (detect_serial_port(COM2))
		devfs_register_device(devfs,"ttyS1",0660,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&serial_ops)->p_data=(void *)COM2;
	if (detect_serial_port(COM3))
		devfs_register_device(devfs,"ttyS2",0660,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&serial_ops)->p_data=(void *)COM3;
	if (detect_serial_port(COM4))
		devfs_register_device(devfs,"ttyS3",0660,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&serial_ops)->p_data=(void *)COM4;
}
