#ifndef _MEMORY_H
#define _MEMORY_H

#include <kernel.h>

extern void *memcpy(void *target, void *src, UINT count);
extern void *memset(void *target, char value, UINT count);

#endif
