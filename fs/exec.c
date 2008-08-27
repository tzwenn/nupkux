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

#include <paging.h>
#include <fs/fs.h>
#include <errno.h>
#include <elf.h>
#include <kernel/dts.h>
#include <lib/string.h>

#define USER_STACK_POS	0x80000000
#define USER_STACK_SIZE 0x1000

#define STACK_NOMEM		0xFFFFFFFF

extern registers *glob_regs;

static char *write_vector(const char **vec, char *buf)
{ //TODO: access_ok
	if (!vec) return 0;
	int i,tmp,argc=0;
	char *oldbuf=buf;
	while (vec[argc++]);
	i=argc-1;
	while (i--) {
		tmp=strlen(vec[i])+1;
		buf-=tmp;
		strncpy(buf,vec[i],tmp);;
	}
	buf-=sizeof(UINT *);
	*((UINT *)buf)=0;
	i=argc-1;
	while (i--) {
		tmp=strlen(vec[i])+1;
		oldbuf-=tmp;
		buf-=sizeof(UINT *);
		*((UINT *)buf)=(UINT)oldbuf;
	}
	return buf;
}

UINT new_stack(UINT pos, UINT size)
{
	UINT i=0;
	for (i=pos;i>=(pos-size);i-=FRAME_SIZE)
		make_page(i,PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USERMODE,current_directory,1);
	flush_tlb();
	return 0;
}

UINT make_new_stack(const char **argv,const char **envp, UINT pos)
{
	char *esp=(char *)pos,*args,*envs;
	if ((envs=write_vector(envp,esp))==(char *)STACK_NOMEM) return STACK_NOMEM;
	if (envs) esp=envs;
	if ((args=write_vector(argv,esp))==(char *)STACK_NOMEM) return STACK_NOMEM;
	if (args) esp=args;
	UINT i=0;
	while (argv[i++]);
	esp-=3*sizeof(UINT);
	((UINT *)esp)[0]=i-1;
	((UINT *)esp)[1]=(UINT) args;
	((UINT *)esp)[2]=(UINT) envs;
	return (UINT)esp;
}

#include <kernel/ktextio.h>

int sys_execve(const char *file,const char **argv,const char **envp)
{
	//TODO: permission check; freeing old core image & stack
	if (!access_ok(VERIFY_READ,file,VERIFY_STRLEN)) return -EFAULT;
	char *buf;
	UINT entry,stack;
	fs_node *node=namei(file);

	if (!node) return -ENOENT;
	open_fs(node,1,0);
	buf=malloc(node->size);
	read_fs(node,0,node->size,buf);
	close_fs(node);
	new_stack(USER_STACK_POS,USER_STACK_SIZE);
	if ((stack=make_new_stack(argv,envp,USER_STACK_POS))==STACK_NOMEM) {
		free(buf); //FIXME: The Stack is destroyed
		return -EFAULT;
	}
	if (load_elf(buf,&entry,1)) {
		free(buf);
		return -ENOEXEC;
	}
	if (load_elf(buf,&entry,0)) {
			free(buf);
			return -ENOEXEC;
	}
	free(buf);
	if (glob_regs) {
		glob_regs->eip=entry;
		glob_regs->ebx=stack;
		//glob_regs->useresp=stack;
	}
	return 0;
}
