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

#include <kernel/dts.h>
#include <lib/memory.h>
#include <kernel/ktextio.h>

extern void gdt_flush(UINT);
extern void idt_flush(UINT);
static void init_gdt();
static void init_idt();
static void gdt_set_gate(int,UINT,UINT,UCHAR,UCHAR);
static void idt_set_gate(UCHAR,UINT,USHORT,UCHAR);

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

gdt_entry gdt_entries[5];
gdt_pointer gdt_ptr;
idt_entry idt_entries[256];
idt_pointer idt_ptr;
isr_t interrupt_handlers[256];

void setup_dts()
{

	init_gdt();
	init_idt();
	memset(&interrupt_handlers,0,sizeof(isr_t)*256);
}

static void init_gdt()
{
	gdt_ptr.limit=(sizeof(gdt_entry)*5)-1;
	gdt_ptr.base=(UINT)&gdt_entries;
	gdt_set_gate(0,0,0,0,0);
	gdt_set_gate(1,0,0xFFFFFFFF,0x9A,0xCF);
	gdt_set_gate(2,0,0xFFFFFFFF,0x92,0xCF);
	gdt_set_gate(3,0,0xFFFFFFFF,0xFA,0xCF);
	gdt_set_gate(4,0,0xFFFFFFFF,0xF2,0xCF);
	gdt_flush((UINT)&gdt_ptr);
}


static void gdt_set_gate(int num, UINT base, UINT limit, UCHAR access, UCHAR gran)
{
	gdt_entries[num].base_low    =(base & 0xFFFF);
	gdt_entries[num].base_middle =(base >> 16) & 0xFF;
	gdt_entries[num].base_high   =(base >> 24) & 0xFF;
	gdt_entries[num].limit_low   =(limit & 0xFFFF);
	gdt_entries[num].granularity =(limit >> 16) & 0x0F;
	gdt_entries[num].granularity|=gran & 0xF0;
	gdt_entries[num].access      =access;
}

static void init_idt()
{
	idt_ptr.limit=sizeof(idt_entry)*256-1;
	idt_ptr.base=(UINT)&idt_entries;
	memset(&idt_entries,0,sizeof(idt_entry)*256);
	outportb(0x20,0x11);
	outportb(0xA0,0x11);
	outportb(0x21,0x20);
	outportb(0xA1,0x28);
	outportb(0x21,0x04);
	outportb(0xA1,0x02);
	outportb(0x21,0x01);
	outportb(0xA1,0x01);
	outportb(0x21,0x0);
	outportb(0xA1,0x0);
	idt_set_gate( 0,(UINT)isr0,0x08,0x8E);
	idt_set_gate( 1,(UINT)isr1,0x08,0x8E);
	idt_set_gate( 2,(UINT)isr2,0x08,0x8E);
	idt_set_gate( 3,(UINT)isr3,0x08,0x8E);
	idt_set_gate( 4,(UINT)isr4,0x08,0x8E);
	idt_set_gate( 5,(UINT)isr5,0x08,0x8E);
	idt_set_gate( 6,(UINT)isr6,0x08,0x8E);
	idt_set_gate( 7,(UINT)isr7,0x08,0x8E);
	idt_set_gate( 8,(UINT)isr8,0x08,0x8E);
	idt_set_gate( 9,(UINT)isr9,0x08,0x8E);
	idt_set_gate(10,(UINT)isr10,0x08,0x8E);
	idt_set_gate(11,(UINT)isr11,0x08,0x8E);
	idt_set_gate(12,(UINT)isr12,0x08,0x8E);
	idt_set_gate(13,(UINT)isr13,0x08,0x8E);
	idt_set_gate(14,(UINT)isr14,0x08,0x8E);
	idt_set_gate(15,(UINT)isr15,0x08,0x8E);
	idt_set_gate(16,(UINT)isr16,0x08,0x8E);
	idt_set_gate(17,(UINT)isr17,0x08,0x8E);
	idt_set_gate(18,(UINT)isr18,0x08,0x8E);
	idt_set_gate(19,(UINT)isr19,0x08,0x8E);
	idt_set_gate(20,(UINT)isr20,0x08,0x8E);
	idt_set_gate(21,(UINT)isr21,0x08,0x8E);
	idt_set_gate(22,(UINT)isr22,0x08,0x8E);
	idt_set_gate(23,(UINT)isr23,0x08,0x8E);
	idt_set_gate(24,(UINT)isr24,0x08,0x8E);
	idt_set_gate(25,(UINT)isr25,0x08,0x8E);
	idt_set_gate(26,(UINT)isr26,0x08,0x8E);
	idt_set_gate(27,(UINT)isr27,0x08,0x8E);
	idt_set_gate(28,(UINT)isr28,0x08,0x8E);
	idt_set_gate(29,(UINT)isr29,0x08,0x8E);
	idt_set_gate(30,(UINT)isr30,0x08,0x8E);
	idt_set_gate(31,(UINT)isr31,0x08,0x8E);
	idt_set_gate(32,(UINT)irq0,0x08,0x8E);
	idt_set_gate(33,(UINT)irq1,0x08,0x8E);
	idt_set_gate(34,(UINT)irq2,0x08,0x8E);
	idt_set_gate(35,(UINT)irq3,0x08,0x8E);
	idt_set_gate(36,(UINT)irq4,0x08,0x8E);
	idt_set_gate(37,(UINT)irq5,0x08,0x8E);
	idt_set_gate(38,(UINT)irq6,0x08,0x8E);
	idt_set_gate(39,(UINT)irq7,0x08,0x8E);
	idt_set_gate(40,(UINT)irq8,0x08,0x8E);
	idt_set_gate(41,(UINT)irq9,0x08,0x8E);
	idt_set_gate(42,(UINT)irq10,0x08,0x8E);
	idt_set_gate(43,(UINT)irq11,0x08,0x8E);
	idt_set_gate(44,(UINT)irq12,0x08,0x8E);
	idt_set_gate(45,(UINT)irq13,0x08,0x8E);
	idt_set_gate(46,(UINT)irq14,0x08,0x8E);
	idt_set_gate(47,(UINT)irq15,0x08,0x8E);
	idt_flush((UINT)&idt_ptr);
}

static void idt_set_gate(UCHAR num, UINT base, USHORT sel, UCHAR flags)
{
	idt_entries[num].base_lo=base & 0xFFFF;
	idt_entries[num].base_hi=(base >> 16) & 0xFFFF;
	idt_entries[num].sel=sel;
	idt_entries[num].always0=0;
	idt_entries[num].flags=flags; //|0x60
}

//Interrupt Service Routines and related stuff

void register_interrupt_handler(UCHAR n, isr_t handler)
{
    interrupt_handlers[n]=handler;
}

void isr_handler(registers regs)
{
    if (interrupt_handlers[regs.int_no]) {
        isr_t handler=interrupt_handlers[regs.int_no];
        handler(regs);
    } else {
        printf("unhandled interrupt: %d\n",regs.int_no);
    }
}

void irq_handler(registers regs)
{
    if (regs.int_no >= 40) {
        outportb(0xA0, 0x20);
    }
    outportb(0x20,0x20);
    if (interrupt_handlers[regs.int_no]) {
        isr_t handler=interrupt_handlers[regs.int_no];
        handler(regs);
    }
}
