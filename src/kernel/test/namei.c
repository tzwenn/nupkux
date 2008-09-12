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

#include "vfs.h"
#include <lib/string.h>
#include <errno.h>
#include <unistd.h>
#include <task.h>

extern vnode *root_vnode;

static int node_permission(vnode *node, int mask)
{
	int mode = node->mode;

	if (I_AM_ROOT()) mode=0777;
		else if (current_task->uid==node->uid || current_task->euid==node->uid)	mode >>= 6;
		else if (current_task->gid==node->gid || current_task->egid==node->gid)	mode >>= 3;

	return mode & mask & 0007;
}

static inline int is_end_chr(char chr)
{
	return (!chr || chr=='/');
}

int namei_match(const char *s1, const char *s2)
{
	if ((!s1) || (!s2)) return 0;
	while (!is_end_chr(*s1) && !is_end_chr(*s2)) {
		if (*s1++!=*s2++) return 0;
	}
	if (is_end_chr(*s1)!=is_end_chr(*s2)) return 0;
	return 1;
}

static vnode *getdentry(vnode *node, const char *filename, int *status, int frommount)
{
	vnode *newnode;
	node->count++;
	if (IS_MNT2(node) && !frommount) {
		newnode=node->mount;
		iput(node);
		return getdentry(newnode,filename,status,0);
	}
	if (IS_MNTED2(node) && namei_match(filename,"..")) {
		newnode=node->cover;
		iput(node);
		return getdentry(newnode,filename,status,1);
	}
	if (!node->i_op || !node->i_op->lookup) {
		if (status) *status=-EGENERIC;
		iput(node);
		return 0;
	}
	/*
	 * TODO: Symlinks go here
	 */
	if (!IS_DIR2(node)) {
		if (status) *status=-ENOTDIR;
		iput(node);
		return 0;
	}
	if (!node_permission(node,X_OK)) {
		if (status) *status=-EACCES;
		iput(node);
		return 0;
	}
	newnode=node->i_op->lookup(node,filename);
	if (status) *status=(newnode)?0:-ENOENT;
	iput(node);
	return newnode;
}

static vnode *igetdir(vnode *node, const char *filename, const char **name, int *length, int *status)
{
	const char *pos;
	int len;
	while ((pos=strchr(filename,'/'))) {
		if ((len=pos-filename))
			if (!(node=getdentry(node,filename,status,0))) return 0;
		filename=pos+1;
	}
	if (name) *name=filename;
	if (length) *length=strlen(filename);
	return node;
}

vnode *namei_v2(const char *filename, int *status)
{
	vnode *node;
	int len;
	const char *basename;
	node=root_vnode; //TODO: Open pwd/root (!node)
	if (filename[0]=='/') filename++;
	if (!(node=igetdir(node,filename,&basename,&len,status)))
		return 0;
	if (status) *status=0;
	if (!len) return node; // we ended with '/' (p.e. "/bin/")
	return getdentry(node,basename,status,0);
}
