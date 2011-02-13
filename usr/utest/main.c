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

typedef unsigned int UINT;

int main(int argc, char *argv[], char *envp[])
{
	int i = 0;
	if (argv) {
		printf("%d Params:\n", argc);
		while (argv[i]) {
			printf("\targv[%d]=%s\n", i, argv[i]);
			i++;
		}
	}
	printf("\nEnvp:\n");
	if (envp) {
		i = 0;
		while (envp[i]) {
			printf("\tenvp[%d]=%s\n", i, envp[i]);
			i++;
		}
	}
	return 0;
}

