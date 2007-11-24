#include <lib/string.h>

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
	if (!(*str)) return 0;
	return str;
}

char *strcpy(char *dest, char *src)
{
	char *tmp = dest;
	while (*src) *(tmp++)=*(src++);
	*tmp=0;
	return dest;
}
