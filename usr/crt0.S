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

.global _start
.extern main
_start:
# I didn't stop getting "General Fault" ,"Invalid Opcode" or "Debug" Exceptions with
# my selfmade stack, so I decided to keep the old one and pass the args in %ebx
	pushl 8(%ebx)
	pushl 4(%ebx)
	pushl (%ebx)

# Jump into programs main function
	call main

#Unwind selfmade stack
	popl %ebx
	popl %ebx
	popl %ebx

# Call syscall exit(%eax)
	mov %eax,%ebx
	mov $0x01,%eax
	int $0x80

# Repeat until rescheduled:
rep:
	nop
	jmp rep
