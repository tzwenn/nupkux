#include <memory.h>

void *memcpy(void *target, void *src, UINT count);
void *memset(void *target, char value, UINT count);

void *memcpy(void *target, void *src, UINT count)
{	
	char *tmp = (char *) target;
	while (count--) {
		*tmp=*((char *) src);
		tmp++;
		src++;
	}
	return target;
}

void *memset(void *target, char value, UINT count)
{
	char *tmp = (char *)target;
	while (count--) *(tmp++)=value;
	return target;
}
