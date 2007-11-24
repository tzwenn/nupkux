#ifndef _STRING_H
#define _STRING_H

typedef long size_t;

extern int strcmp(char *s1, char *s2);
extern size_t strlen(char *str);
extern char *strchr(char *str, char chr);
extern char *strcpy(char *dest, char *src);

#endif
