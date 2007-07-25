#include <kernel/sish.h>
#include <kernel/ktextio.h>

int sish();

typedef UINT size_t;

int strcmp(char *s1, char *s2)
{	
	while ((*s1) && (*s2)) {
		if (*s1<*s2) return -1;
		if (*s1>*s2) return 1;
		s1++;
		s2++;
	}
	if ((!*s1) && (*s2)) return -1;
	if ((*s1) && (!*s2)) return 1;
	return 0;
}

size_t strlen(char *str)
{
	size_t res = 0;
	
	while (*(str++)) res++;
	return res;
}

char *strchr(char *str, char chr)
{
	while ((*str) && (*str!=chr)) str++;
	return str;
}

int sish_help()
{
	_kout("List of built-in commands:\n");
	_kout("\texit\t\tQuit sish\n"); 
	_kout("\thalt\t\tHalt system\n");
	_kout("\treboot\t\tReboot system\n");
	_kout("\thelp\t\tWhat do you read right now?\n");
	return 0;
}

int _sish_split_par(char *cmd, char **argv)
{
	return 0;
}

int _sish_interpret(char *cmd) 
{
	if (!strcmp(cmd,"help")) return sish_help();
	if (!strcmp(cmd,"exit")) return SISH_EXIT;
	if (!strcmp(cmd,"halt")) return SISH_HALT;
	if (!strcmp(cmd,"reboot")) return SISH_REBOOT;
	_kout("sish: ");
	_kout(cmd);
	_kout(": command not found\n");
	return 0;
}

int sish()
{
	char input[STRLEN];
	int ret;

	_kout("\nSquaros intern shell (sish) started.\n\n");
	while (1) {
		_kout("$ ");
		_kin(input,STRLEN);
		ret=_sish_interpret(input);
		if ((ret & 0xF0)==0xE0) break;
	}
	return ret;
}
