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

.global read_eip
.global clone_page

read_eip:
	popl	%eax
	jmp	*%eax

clone_page:
	pushl	%ebx
	pushf
	cli
	movl	12(%esp),%ebx
	movl	16(%esp),%ecx
	movl	%cr0,%edx
	andl	$0x7fffffff,%edx
	movl	%edx,%cr0
	movl	$0x400,%edx
loop:
	movl	(%ebx),%eax
	movl	%eax,(%ecx)
	addl	$4,%ebx
	addl	$4,%ecx
	dec	%edx
	jnz	loop
	movl	%cr0,%edx
	orl	$0x80000000,%edx
	movl	%edx,%cr0
	popf
	popl	%ebx
	ret

