#ifndef _FAT32_H
#define _FAT32_H

#include <squaros.h>

#define FAT32_FSVer 0x0000

typedef struct _fat32discr fat32discr;
typedef UCHAR* fat32fat;

struct _fat32discr {
	UCHAR BPB_SecPerClus;	//offset 13
	UCHAR BPB_NumFATs;	//offset 16
	UCHAR BPB_Media;	//offset 21
	UCHAR BS_DrvNum;	//offset 64
	UCHAR BS_VolLab[11];	//offset 71
	USHORT BPB_BytsPerSec;  //offset 11
	USHORT BPB_RsvdSecC;	//offset 14
	USHORT BPB_SecPerTrk;	//offset 24
	USHORT BPB_NumHeads;	//offset 26
	USHORT BPB_ExtFlags;	//offset 40
	USHORT BPB_FSVer;	//offset 42 -> soll 0:0 sein
	USHORT BPB_BkBootSec;	//offset 50
	UINT BPB_HiddSec;	//offset 28 
	UINT BPB_TotSec32;	//offset 32
	UINT BPB_FATSz32;	//offset 36
	UINT BPB_RootClus;	//offset 44	
};

extern fat32discr fat32_read_discr(UCHAR *device);

#endif
