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

//                       Based on code from Bran's and JamesM'S kernel development tutorials.


#ifndef _DTS_H
#define _DTS_H

#include <kernel.h>

void setup_dts();

typedef struct _gdt_entry gdt_entry;
typedef struct _gdt_pointer gdt_pointer;
typedef struct _idt_entry idt_entry;
typedef struct _idt_pointer idt_pointer;

struct _gdt_entry {
	USHORT limit_low;
	USHORT base_low;
	UCHAR base_middle;
	UCHAR access;
	UCHAR granularity;
	UCHAR base_high;
} __attribute__ ((packed));

struct _gdt_pointer {
	USHORT limit;
	UINT base;
} __attribute__((packed));

struct _idt_entry {
	USHORT base_lo;
	USHORT sel;
	UCHAR  always0;
	UCHAR  flags;
	USHORT base_hi;
} __attribute__((packed));

struct _idt_pointer {
    USHORT limit;
    UINT base;
} __attribute__((packed));

#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

typedef struct _registers {
	UINT ds;
	UINT edi,esi,ebp,esp,ebx,edx,ecx,eax;
	UINT int_no,err_code;
	UINT eip,cs,eflags,useresp,ss;
} registers;

typedef void (*isr_t)(registers);
void register_interrupt_handler(UCHAR n, isr_t handler);

#endif
