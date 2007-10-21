#ifndef _UINSTD_H
#define _UINSTD_H

//Will there be ever somthing "nearly POSIX" by me?
#define _POSIX_VERSION 199009L

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef int ssize_t;
#endif

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

//file access 
#define F_OK	0
#define X_OK	1
#define W_OK	2
#define R_OK	4

//lseek starts
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#endif
