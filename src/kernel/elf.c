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

#include <task.h>
#include <mm.h>
#include <lib/string.h>
#include <elf.h>

int elf_load_segment(char *image, elf32_phdr* seg)
{
	/*FIXME: By know I don't know how to cope p_vaddr AND p_align,
		So I ignore the latter and also p_flags which are only for ONE FRAME */
	UINT i,size=seg->p_filesz;
	for (i=ALIGN_DOWN(seg->p_vaddr);i<seg->p_vaddr+seg->p_memsz;i+=FRAME_SIZE)
		make_page(i,PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USERMODE,current_task->directory,1);
	flush_tlb(); //TODO: If make_page() fails we need a "not enough core"
	if (size>seg->p_memsz) size=seg->p_memsz;
	memcpy((void *)seg->p_vaddr,(void *)((UINT)image+seg->p_offset),size);
	if (seg->p_filesz<size)
		memset((void *)((UINT)image+seg->p_offset+seg->p_filesz),0,size-seg->p_filesz);
	return 0;
}

int load_elf(char *image, UINT *entry, int pretend)
{
	if (!image || !entry) return LOAD_ELF_INVARG;
	elf32_ehdr* hdr=(elf32_ehdr *)image;
	elf32_phdr* seg;
	UINT i;
	if (hdr->ei_magic!=ELF_MAGIC) return LOAD_ELF_NOELF;
	if (hdr->ei_version!=EV_CURRENT || hdr->e_version!=EV_CURRENT) return LOAD_ELF_SUPPORT;
	if (hdr->ei_class!=ELFCLASS32 || hdr->ei_data!=ELFDATA2LSB || hdr->e_machine!=EM_386) return LOAD_ELF_MACHINE;
	if (hdr->ei_version!=EV_CURRENT || hdr->e_version!=EV_CURRENT || hdr->e_type!=ET_EXEC) return LOAD_ELF_SUPPORT;
	*entry=hdr->e_entry;
	for (i=0;i<hdr->e_phnum;i++) {
		seg=(elf32_phdr* )((UINT)image+hdr->e_phoff+i*(UINT)hdr->e_phentsize);
		switch (seg->p_type) {
			case PT_DYNAMIC:
			case PT_SHLIB:
				return LOAD_ELF_DYNAMIC;
				break;
			case PT_LOAD:
				if (!pretend)
					if (elf_load_segment(image,seg)) return LOAD_ELF_LOAD;
				break;
			default:
				break;
		}
	}
	return 0;
}
