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

.extern isr_handler
.extern irq_handler

#define ISR_NOERRCODE(NR)	.global isr##NR; \
					isr##NR:; \
						cli; \
						pushl	$0; \
						pushl	$##NR; \
						jmp	isr_common_stub

#define ISR_ERRCODE(NR)	.global isr##NR; \
				isr##NR:; \
					cli; \
					pushl	$##NR; \
					jmp	isr_common_stub

#define IRQ(NR,INTNR)	.global irq##NR; \
				irq##NR:; \
					cli; \
					pushl	$0; \
					pushl	$##INTNR; \
					jmp	irq_common_stub

.global gdt_flush
.global idt_flush
.global tss_flush

gdt_flush:
	movl	4(%esp),%eax
	lgdt	(%eax)
	movw	$0x10,%ax
	movw	%ax,%ds
	movw	%ax,%es
	movw	%ax,%fs
	movw	%ax,%gs
	movw	%ax,%ss
	jmp	$0x08,$1f
1:
	ret

idt_flush:
	movl	4(%esp),%eax
	lidt	(%eax)
	ret

tss_flush:
	movw	$0x2B,%ax
	ltr	%ax
	ret

ISR_NOERRCODE(0)
ISR_NOERRCODE(1)
ISR_NOERRCODE(2)
ISR_NOERRCODE(3)
ISR_NOERRCODE(4)
ISR_NOERRCODE(5)
ISR_NOERRCODE(6)
ISR_NOERRCODE(7)
ISR_ERRCODE  (8)
ISR_NOERRCODE(9)
ISR_ERRCODE  (10)
ISR_ERRCODE  (11)
ISR_ERRCODE  (12)
ISR_ERRCODE  (13)
ISR_ERRCODE  (14)
ISR_NOERRCODE(15)
ISR_NOERRCODE(16)
ISR_NOERRCODE(17)
ISR_NOERRCODE(18)
ISR_NOERRCODE(19)
ISR_NOERRCODE(20)
ISR_NOERRCODE(21)
ISR_NOERRCODE(22)
ISR_NOERRCODE(23)
ISR_NOERRCODE(24)
ISR_NOERRCODE(25)
ISR_NOERRCODE(26)
ISR_NOERRCODE(27)
ISR_NOERRCODE(28)
ISR_NOERRCODE(29)
ISR_NOERRCODE(30)
ISR_NOERRCODE(31)
ISR_NOERRCODE(128)
IRQ(0,32)
IRQ(1,33)
IRQ(2,34)
IRQ(3,35)
IRQ(4,36)
IRQ(5,37)
IRQ(6,38)
IRQ(7,39)
IRQ(8,40)
IRQ(9,41)
IRQ(10,42)
IRQ(11,43)
IRQ(12,44)
IRQ(13,45)
IRQ(14,46)
IRQ(15,47)

isr_common_stub:
	pusha
	movw	%ds,%ax
	pushl	%eax
	movw	$0x10,%ax
	movw	%ax,%ds
	movw	%ax,%es
    	movw	%ax,%fs
    	movw	%ax,%gs
	call	isr_handler
	popl	%eax
	movw	%ax,%ds
	movw	%ax,%es
    	movw	%ax,%fs
    	movw	%ax,%gs
	popa
	addl	$8,%esp
	sti
	iret

irq_common_stub:
	pusha
	movw	%ds,%ax
	pushl	%eax
	movw	$0x10,%ax
	movw	%ax,%ds
	movw	%ax,%es
    	movw	%ax,%fs
    	movw	%ax,%gs
	call	irq_handler
	popl	%eax
	movw	%ax,%ds
	movw	%ax,%es
    	movw	%ax,%fs
    	movw	%ax,%gs
	popa
	addl	$8,%esp
	sti
	iret
