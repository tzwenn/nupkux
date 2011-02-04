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
#include <fcntl.h>
#include <string.h>

char *sh_gets(char *buf)
{
	int i=0;
	printf("\e[?25h");
	for (;;) {
		buf[i]=getchar();
		switch (buf[i]) {
			case '\n':
				buf[i]='\0';
				goto end;
				break;
			case '\b':
				i-=2;
				if (i>=-1) printf("\b");
				if (i<0) i=-1;
				break;
			case '\t':
				break;
			default:
				printf("%c",buf[i]);
				break;
		}
		i++;
	}
end:
	printf("\n\e[?25l");
	return buf;

}

void print_welcome(void)
{
	printf("This shell session is connected to the Nupkux intern shell (nish)\n");
	printf("running in kernel mode, so \e[33mBE CAREFULL\e[m!\n");

}

int main(int argc, char *argv[], char *envp[])
{
	char input[255] = {0,};
	print_welcome();
	int fd=open("/dev/nish",O_RDWR,0);
	if (fd<0) {
		printf("\e[31mCannot connect to nish\e[m\n");
		exit(1);
	}
	for (;;) {
		printf("\e[97msh#\e[m ");
		sh_gets(input);
		write(fd,input,strlen(input)+1);
	}
	close(fd);
	return 0;
}
