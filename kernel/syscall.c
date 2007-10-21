#include <kernel/syscall.h>
#include <kernel/ktextio.h>

int sys_putchar(struct regs *r)
{
	printf("%c",r->ebx);
	return 0;
}

int SysCallHandler(struct regs *r)
{
	switch (r->eax) {
		case SYS_PUTCHAR: return sys_putchar(r);
				  break;
		default: printf("SysCall 0x%X (%d)\n",r->eax,r->eax);
	}
	return 0;
}
