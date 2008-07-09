#*
#*  Copyright (C) 2007,2008 Sven Köhler
#*
#*  This file is part of Nupkux.
#*
#*  Nupkux is free software: you can redistribute it and/or modify
#*  it under the terms of the GNU General Public License as published by
#*  the Free Software Foundation, either version 3 of the License, or
#*  (at your option) any later version.
#*
#*  Nupkux is distributed in the hope that it will be useful,
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#*  GNU General Public License for more details.
#*
#*  You should have received a copy of the GNU General Public License
#*  along with Nupkux.  If not, see <http://www.gnu.org/licenses/>.
#*

.global _loader           #Entry

# multiboot header 
.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set FLAGS_MBH, ALIGN | MEMINFO  
.set MAGIC_MBH, 0x1BADB002
.set CHECKSUM,-(MAGIC_MBH+FLAGS_MBH) 

.align 4
.long MAGIC_MBH
.long FLAGS_MBH
.long CHECKSUM

_loader:
	push  %eax
	push  %esp
	push  %ebx
	cli

#Start Kernel's main-function
	call  _kmain
	cli
	hlt
