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

#ifndef _ELF_H
#define _ELF_H

//I've only written supported values
#define ELF_MAGIC	0x464C457F //Just for low-endian
#define ELFCLASS32	1
#define	ELFDATA2LSB	1
#define EV_CURRENT	1
#define ET_EXEC		2
#define EM_386		3

#define PT_NULL		0
#define PT_LOAD		1
#define PT_DYNAMIC	2
#define PT_INTERP	3
#define PT_NOTE		4
#define PT_SHLIB	5
#define PT_PHDR		6

#define	PF_READ		0x04	/* readable */
#define	PF_WRITE	0x02	/* writable */
#define	PF_EXEC		0x01	/* executable */
#define	SF_BSS		(SF_ZERO | SF_READ | SF_WRITE)

typedef UINT	Elf32_Addr;
typedef USHORT	Elf32_Half;
typedef UINT	Elf32_Off;
typedef int		Elf32_Sword;
typedef UINT	Elf32_Word;

typedef struct _elf32_ehdr {
	UINT ei_magic;
	UCHAR ei_class;
	UCHAR ei_data;
	UCHAR ei_version;
	UCHAR ei_reserved[9];
	//Things above also called "UCHAR e_ident[16]"
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} elf32_ehdr;

typedef struct _elf32_phdr {
	Elf32_Word p_type;
	Elf32_Off p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} elf32_phdr;

#define LOAD_ELF_SUCCESS	0
#define LOAD_ELF_INVARG		1
#define LOAD_ELF_NOELF		2
#define LOAD_ELF_MACHINE	3
#define LOAD_ELF_SUPPORT	4
#define LOAD_ELF_DYNAMIC	5
#define LOAD_ELF_LOAD		6

extern int load_elf(char *image, UINT *entry, int pretend);

#endif
