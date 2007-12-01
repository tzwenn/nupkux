#ifndef _SQUAROS_H
#define _SQUAROS_H

#define true	1
#define false	0

#define _kaborted	0xFF

typedef unsigned char  byte;
typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned int   UINT;
typedef unsigned long  ULONG;

struct regs {
	UINT gs, fs, es, ds;
	UINT edi, esi, ebp, esp, ebx, edx, ecx, eax;	UINT int_no, err_code;
	UINT eip, cs, eflags, useresp, ss;    
};

extern void gdt_set_gate(int num, ULONG base, ULONG limit, UCHAR access, UCHAR granularity);
extern void init_ktexto();
extern void gdt_install();
extern void idt_set_gate(UCHAR num, ULONG base, USHORT sel, UCHAR flags);
extern void idt_install();
extern void isrs_install();
extern void irq_install_handler(int irq, void (*handler)(struct regs *r));
extern void irq_uninstall_handler(int irq);					//To MACRO
extern void irq_install();
extern void timer_install();
extern void init_floppy();
extern void paging_setup();

extern char _kabort_func;

#define _kabort_func_break()	if (_kabort_func) { \
					_kabort_func=0; \
					break; \
				}
#define _kabort_func_return(val) if (_kabort_func) { \
					_kabort_func=0; \
					return val; \
				}

#define sti() asm volatile ("sti\n\t")
#define cli() asm volatile ("cli\n\t")
#define hlt() asm volatile ("hlt\n\t")

#endif
