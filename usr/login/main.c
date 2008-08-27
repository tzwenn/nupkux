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
#include <unistd.h>

void acknowledgement(void)
{
	printf("Welcome to Nupkux build %s %s\n\n",__DATE__,__TIME__);
	printf("The Nupkux kernel and userspace programs are free software, but come\n");
	printf("WITHOUT ANY WARRANTY; see the GNU General Public License for more details.\n\n");
}

int main(int argc, char *argv[], char *envp[])
{
	acknowledgement();
	execve("/bin/sh",0,0);
	return 0;
}
