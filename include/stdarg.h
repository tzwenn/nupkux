#ifndef _STDARG_H
#define _STDARG_H

typedef char *va_list;

#define _va_sizeof(VALUE) (((sizeof(VALUE)+sizeof(int)-1)/sizeof(int))*sizeof(int))
#define va_start(AP,LAST) AP=(((char *)(&LAST)+_va_sizeof(LAST)))
#define va_arg(AP,TYPE) (AP+=_va_sizeof(TYPE), \
			*((TYPE *)(AP-_va_sizeof(TYPE))))
#define va_end(AP)

#endif
