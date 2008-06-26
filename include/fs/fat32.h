#ifndef _FAT32_H
#define _FAT32_H

#include <squaros.h>
#include <fs/fs.h>

#define FAT32_FSVer	0x0000
#define SECTORSZ	512
#define FAT_DIR_SZ	32
#define FAT32_EOC	0x0FFFFFF8
#define FAT32_BADCLUSTER 0x0FFFFFF7

#define FirstSectorofCluster(N,discr) (((N-2)*discr->BPB.BPB_SecPerClus)+discr->FirstDataSector)
#define FAT32_FATOffset(N) ((N)*4)
#define FAT32_ThisFATEntOffset(N,discr) (FAT32_FATOffset(N)%(discr)->BPB.BPB_BytsPerSec)

typedef UCHAR* fat32fat;
typedef struct _fat32BPB fat32BPB;
typedef struct _fat32discr fat32discr;
typedef struct _fat32fileentry fat32fileentry;

struct _fat32BPB {
	UCHAR BPB_SecPerClus;	//offset 13
	UCHAR BPB_NumFATs;	//offset 16
	UCHAR BPB_Media;	//offset 21
	UCHAR BS_DrvNum;	//offset 64
	UCHAR BS_VolLab[11];	//offset 71
	USHORT BPB_BytsPerSec;  //offset 11
	USHORT BPB_ResvdSecCnt;	//offset 14
	USHORT BPB_RootEntCnt;	//offset 17
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

struct _fat32discr {
	fs_node *device;
	fat32fat FAT;
	UINT FatSz;
	UINT FirstDataSector;
	UINT RootDirSectors;
	UINT DataSec;
	UINT CountofClusters;
	fat32BPB BPB;
};

struct _fat32fileentry {
	UCHAR name[11];
	UCHAR attr;
	UINT cluster;
	UINT offset;
	UINT startcluster;
	UINT size;
};

extern UINT fat32_read_discr(fs_node *device, fat32discr *discr);
extern fs_node *fat32_mount(fs_node *device, fs_node *mountpoint);
extern UINT fat32_umount(fs_node *node);

#endif
