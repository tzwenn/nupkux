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

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[])
{
	if (argc < 2) return 1;
	char device[15];
	sprintf(device, "/dev/tty%s", argv[1]);
	close(STDIN_FILENO); //Get rid of old stdin
	open(device, O_RDWR, 0); //Open the tty as new stdin
	dup2(STDIN_FILENO, STDOUT_FILENO); //Replace old stdout with tty
	dup2(STDIN_FILENO, STDERR_FILENO);
	printf("\nNupkux tty%s\n\n", argv[1]);
	execve("/bin/login", 0, 0);
	return 0;
}
