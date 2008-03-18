#include <fs/fat32.h>
#include <fs/fs.h>
#include <mm.h>
#include <lib/string.h>

#include <kernel/devices/fdc.h>
#include <kernel/ktextio.h>

struct dirent *fat32_readdir(fs_node *node, UINT index);
fs_node *fat32_finddir(fs_node *node, char *name);
UINT fat32_findfileentry(fs_node *node, fat32fileentry *result);
UINT fat32_read(fs_node *node, UINT offset, UINT size, UCHAR *buffer);

struct dirent dirent;

//#define FAT32_ClusEntryVal(N,discr) ((*((DWORD *) &((discr).FAT[(discr).BPB.BPB_ResvdSecCnt*(discr).BPB.BPB_BytsPerSec+FAT32_FATOffset(N)+FAT32_ThisFATEntOffset(N,(discr))])))&0x0FFFFFFF)

UINT FAT32_ClusEntryVal(UINT N, fat32discr *discr)
{
	UCHAR *SecBuff=discr->FAT+(FAT32_FATOffset(N)/discr->BPB.BPB_BytsPerSec)*discr->BPB.BPB_BytsPerSec;
	
	return (*((UINT *) &SecBuff[FAT32_ThisFATEntOffset(N,discr)])) & 0x0FFFFFFF;
}

/*UINT device_read(char *device, UINT pos, UINT n, UCHAR* buffer)
{
	UINT len = (n%SECTORSZ)?(n/SECTORSZ+4):(n/SECTORSZ);
	UCHAR *buf = (UCHAR *)malloc(len*SECTORSZ);

	printf("Read n=%d;len=%d sectors from pos=%d (%d)\n",n,len,pos,pos/SECTORSZ);	
	pos/=SECTORSZ;
	if (!fdc_read_block(pos,buf,len)) {
		free(buf);
		return 0;
	}
	memcpy(buffer,buf,n);
	free(buf);
	return n;
}*/

UINT device_read(char *device, UINT pos, UINT n, UCHAR* buffer)
{
	UINT len;
	UCHAR *buf;

	len=((pos+n)/SECTORSZ-(pos)/SECTORSZ);
	if (!len) len=4;
	printf("Read n=%d;len=%d sectors from pos=%d (%d)\n",n,len,pos,pos/SECTORSZ);
	buf=(UCHAR *)malloc(len*SECTORSZ);
	if (!fdc_read_block(pos/SECTORSZ,buf,len)) {
		free(buf);
		return 0;
	}
	memcpy(buffer,buf+pos%SECTORSZ,n);
	free(buf);
	return n;
}

UINT fat32_read_BPB(char *device, fat32BPB *BPB)
{
	UCHAR sector[SECTORSZ];

	if (!device_read(device,0,SECTORSZ,sector)) return 0;
	BPB->BPB_SecPerClus=sector[13];
	BPB->BPB_NumFATs=sector[16];
	BPB->BPB_Media=sector[21];
	BPB->BS_DrvNum=sector[64];
	memcpy(BPB->BS_VolLab,sector+71,11);
	BPB->BPB_BytsPerSec=*((USHORT*) (sector+11));
	BPB->BPB_ResvdSecCnt=*((USHORT*) (sector+14));
	BPB->BPB_RootEntCnt=*((USHORT*) (sector+17));
	BPB->BPB_SecPerTrk=*((USHORT*) (sector+24));
	BPB->BPB_NumHeads=*((USHORT*) (sector+26));
	BPB->BPB_ExtFlags=*((USHORT*) (sector+40));
	BPB->BPB_FSVer=*((USHORT*) (sector+42));
	BPB->BPB_BkBootSec=*((USHORT*) (sector+50));
	BPB->BPB_HiddSec=*((UINT*) (sector+28));
	BPB->BPB_TotSec32=*((UINT*) (sector+32)); 
	//Seeeehr gefährlich! Sobald wir schreiben muss das weg!
	if (!BPB->BPB_TotSec32) BPB->BPB_TotSec32=*((USHORT*) (sector+19)); 
	BPB->BPB_FATSz32=*((UINT*) (sector+36));  
	BPB->BPB_RootClus=*((UINT*) (sector+44));
	return 1;
}

UINT fat32_read_discr(char *device, fat32discr *discr)
{
	discr->device=device;
	if (!fat32_read_BPB(device,&discr->BPB)) return 0;
	discr->RootDirSectors=((discr->BPB.BPB_RootEntCnt*FAT_DIR_SZ)+(discr->BPB.BPB_BytsPerSec-1))/discr->BPB.BPB_BytsPerSec;
	discr->FatSz=discr->BPB.BPB_FATSz32;
	discr->FirstDataSector=discr->BPB.BPB_ResvdSecCnt+(discr->BPB.BPB_NumFATs*discr->FatSz)+discr->RootDirSectors;
	discr->DataSec=discr->BPB.BPB_TotSec32-(discr->BPB.BPB_ResvdSecCnt+(discr->BPB.BPB_NumFATs*discr->FatSz)+discr->RootDirSectors);
	discr->CountofClusters=discr->DataSec/discr->BPB.BPB_SecPerClus;
	discr->FAT=malloc(discr->FatSz*SECTORSZ);
	if (!device_read(device,discr->BPB.BPB_ResvdSecCnt*SECTORSZ,discr->FatSz,discr->FAT)) {
		free(discr->FAT);
		return 0;	
	}
	
	return 1;
}

UINT fat32_findfileentry(fs_node *node, fat32fileentry *entry)
{
	fat32discr *discr = (fat32discr *) (node->filesystem)->discr;
	char *device = (char *) node->filesystem->device;
	UCHAR *clusterbuf = (UCHAR *) malloc(discr->BPB.BPB_SecPerClus*SECTORSZ);

	entry->cluster=2;
	entry->offset=11*FAT_DIR_SZ;

	if (!device_read(device,FirstSectorofCluster(entry->cluster,discr)*SECTORSZ,discr->BPB.BPB_SecPerClus*SECTORSZ,clusterbuf)) {
		free(clusterbuf);
		return 0;	
	}

	memcpy(entry->name,clusterbuf+entry->offset,11);
	entry->attr=clusterbuf[entry->offset+11];
	entry->size=*((UINT *)(clusterbuf+entry->offset+28));
	entry->startcluster=*((USHORT *)(clusterbuf+entry->offset+20)) << 16;
	entry->startcluster|=*((USHORT *)(clusterbuf+entry->offset+26));

	free(clusterbuf);
	return 1;
}


UINT fat32_read(fs_node *node, UINT offset, UINT size, UCHAR *buffer)
{
	UINT cluster,clusnum,bufsize=0;
	UCHAR *clusterbuf = 0;

	fat32fileentry entry;
	fat32discr *discr = (fat32discr *) (node->filesystem)->discr;
	char *device = (char *) node->filesystem->device;

	if (!fat32_findfileentry(node,&entry)) return 0;
	printf("%s in cluster %d (size: %d)\n",entry.name,entry.startcluster,entry.size);
	cluster=entry.startcluster;
	clusnum=offset/(discr->BPB.BPB_SecPerClus*discr->BPB.BPB_BytsPerSec);
	while ((cluster<FAT32_BADCLUSTER) && cluster && (clusnum--)) {
		cluster=FAT32_ClusEntryVal(cluster,discr);
	}
	if (clusnum) return 0;
	bufsize=discr->BPB.BPB_SecPerClus-offset%(discr->BPB.BPB_SecPerClus*discr->BPB.BPB_BytsPerSec);
	clusterbuf=malloc(bufsize);
	if (!device_read(device,FirstSectorofCluster(cluster,discr)*SECTORSZ+offset%(discr->BPB.BPB_SecPerClus*discr->BPB.BPB_BytsPerSec),
			bufsize,clusterbuf)) {
		free(clusterbuf);
		return 0;
	}
	memcpy(buffer,clusterbuf,bufsize);
	free(clusterbuf);
	return 1;
}

struct dirent *fat32_readdir(fs_node *node, UINT index)
{
	
	return &dirent;
}


fs_node *fat32_finddir(fs_node *node, char *name)
{

	return 0;
}

fs_node *fat32_mount(char *device, fs_node *mountpoint)
{
	fs_node *root = (fs_node*) malloc(sizeof(fs_node));
	fat32discr *discr = (fat32discr *) malloc(sizeof(fat32discr));
	
	if (!fat32_read_discr(device,discr)) {
		free(root);
		free(discr);
		return 0;
	}
	strcpy(root->name,"fat32root");
	root->mask=0x1ED;	//rwxr-xr-x
	root->flags=FS_DIRECTORY;
	root->uid=root->gid=root->inode=root->size=0;
	root->read=0;
	root->write=0;
	root->open=0;
	root->close=0;
	root->readdir=&fat32_readdir;
	root->finddir=&fat32_finddir;
	root->ptr=0;
	root->filesystem=fs_add_mountpoint(FS_TYPE_FAT32,(void *)discr,mountpoint,device);

	return root;
}

UINT fat32_umount(fs_node *node)
{
	fat32discr *discr = (fat32discr *) node->filesystem->discr;

	if (!fs_del_mountpoint(node->filesystem)) return 0;
	free(discr->FAT);
	free(discr);
	free(node);
	
	return 1;
}
