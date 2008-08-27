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

#include <stdio.h>
#include <fcntl.h>

#define switch_to_user_mode() asm volatile(	"cli\n\t" \
			"movw $0x23, %ax\n\t"	\
			"movw %ax, %ds\n\t"	\
			"movw %ax, %es\n\t"	\
			"movw %ax, %fs\n\t"	\
			"movw %ax, %gs\n\t"	\
			"movl %esp, %eax\n\t"	\
			"pushl $0x23\n\t"	\
			"pushl %eax\n\t"	\
			"pushf\n\t"	\
			"popl %eax\n\t"	\
			"orl $0x200,%eax\n\t"	\
			"pushl %eax\n\t"	\
			"pushl $0x1B\n\t"	\
			"pushl $1f\n\t"	\
			"iret\n\t"	\
			"1:");

#define TTY_SETUPS	4

int main(void)
{
	//switch_to_user_mode();
	open("/dev/tty0",O_WRONLY,0);
	open("/dev/tty0",O_RDONLY,0);
	open("/dev/tty0",O_RDONLY,0);
	printf("\e[32mStarting Nupkux INIT ...\e[m\n");
	const char *argv[3] = {0,};
	char cmd[6] = "getty";
	char device[4] = {0,};
	int i;
	argv[0]=cmd;
	argv[1]=device;
	for (i=0;i<TTY_SETUPS;i++) {
		sprintf(device,"%d",i);
		if (!fork())
			exit(execve("/bin/getty",argv,0));
	}
	for (;;);
	return 0;
}
