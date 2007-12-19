#include <kernel/sish.h>
#include <kernel/ktextio.h>
#include <time.h>
#include <lib/string.h>
#include <fs/fat32.h>
#include <kernel/devices/ata.h>
#include <mm.h>

int sish();

int ltrim(char *cmd)
{
	char *str = cmd;

	while ((*str<=32) && (*str)) str++;
	strcpy(cmd,str);
	return str-cmd;
}

int _sish_split_par(char *cmd, char *args)
{
	char *astart;

	ltrim(cmd);
	astart=strchr(cmd,' ');
	if (astart) {
		*astart=0;
		strcpy(args,astart+1);
		ltrim(args);
	}
	return 0;
}


int sish_help()
{
	printf("List of built-in commands:\n");
	printf("\ttest\t\tRun the current development function\n");
	printf("\tclear\t\tClean up the mess on the screen.\n");
	printf("\tlba28\t\tAccess IDE-ATA drives\n");
	printf("\tpaging\t\tCreate a pagefault and crashes the kernel\n");
	printf("\ttime\t\tGive information about time and date\n");
	printf("\texit\t\tQuit sish\n"); 
	printf("\thalt\t\tHalt system\n");
	printf("\treboot\t\tReboot system\n");
	printf("\thelp\t\tWhat do you read right now?\n");
	return 0;
}

int sish_time()
{
	struct tm now;
	time_t timestamp;
	
	now=getrtctime();
	printf("Time:\t\t%.2d:%.2d:%.2d\n",now.tm_hour,now.tm_min,now.tm_sec);
	printf("Date:\t\t%.2d-%.2d-%.2d\n",now.tm_year,now.tm_mon,now.tm_mday);
	removetimezone(&now);
	timestamp=mktime(&now);	
	return 1;
}

int sish_lba28()
{
	UCHAR idelist = 0,drv,tcalc;
	USHORT tmpword = 0, contrl;
	UINT addr;
	UCHAR buffer[512];
	char input[STRLEN];

	printf("Detect Devices ...\n");
	outportb(0x1F6,0xA0);
	sleep(10);
	tmpword=inportb(0x1F7);
	if (tmpword & 0x40) {
		idelist=0x01;
		printf("Primary master exists.\n");
	}
	outportb(0x1F6,0xB0);
	sleep(10);
	tmpword=inportb(0x1F7);
	if (tmpword & 0x40) {
		idelist|=0x02;
		printf("Primary slave exists.\n");
	}
	outportb(0x176,0xA0);
	sleep(10);
	tmpword=inportb(0x177);
	if (tmpword & 0x40) {
		idelist|=0x10;
		printf("Secondary master exists.\n");
	}
	outportb(0x176,0xB0);
	sleep(10);
	tmpword=inportb(0x177);
	if (tmpword & 0x40) {
		idelist|=0x20;
		printf("Secondary Slave exists.\n");
	}
	printf("Finished. Start loop, [^C] stops.\n");
	while (1) {
		printf("Controller (1/2): ");
		if (_kin(input,STRLEN)==_kaborted) break;
		if (!strcmp(input,"1")) contrl=0x1F0;
			else if (!strcmp(input,"2")) contrl=0x170;
			else {
				printf("Unknown controller.\n");
				continue;
			     }
		printf("Drive (A/B): ");
		if (_kin(input,STRLEN)==_kaborted) break;
		if (!strcmp(input,"A")) drv=0x00;
			else if (!strcmp(input,"B")) drv=0x01;
			else {
				printf("Unknown drive.\n");
				continue;
			     }
		tcalc=drv+1;
		if (contrl==0x170) tcalc<<=4;
		if (!(idelist & tcalc)) {
			printf("No device attached.\n");
			continue;
		}
		printf("Enter address: ");
		if (_kin(input,STRLEN)==_kaborted) break;
		addr=str2d(input);
		if (!lba28_read(buffer,contrl,drv,addr,0)) {
			printf("Read-Error\n");
			continue;
		} else printf("Reading finished.\n");
		for (addr=0;addr<512;addr++)
			printf("%.2X",buffer[addr]);
		printf("\n");
	}
	printf("\n");
	return 1;
}

int sish_paging()
{
	UINT *ptr = (UINT*) 0xA0000000;

	printf("I will crash, If I work fine ...\n");
	printf("*ptr: 0x%X, btw: Kernel is still running, so there must be a problem.\n",*ptr); 	//Kernel should never print this
	return 1; 	
}

extern UINT device_read(char *device, UINT pos, UINT n, UCHAR* buffer);
extern UINT fat32_read(fs_node *node, UINT offset, UINT size, UCHAR *buffer);

int sish_test()
{
	fat32discr discr;
	UCHAR dir0[512];
//	void *dummy;
	fs_node *node;
#define	DIR_NUM	14

/*	dummy=malloc(50);
	printf("dummy at 0x%X\n",dummy);
	free(dummy);*/
	node=fat32_mount("/dev/fd0",0);
	printf("---FAT32 Driver for floppy devices---\n\n");
	discr=*((fat32discr *) ((mountinfo *) node->filesystem)->discr);
	if (!node) {
		printf("Can not access floppy drive: Aborting.\n");
		return 1;
	}
	printf("FirstDataSector: %d\nBPB_BytsPerSec: %d\n",discr.FirstDataSector,discr.BPB.BPB_BytsPerSec);
	printf("BPB_FATSz32: 0x%X\nRootDirSectors: %d\n",discr.BPB.BPB_FATSz32,discr.RootDirSectors);
	printf("CountofClusters: %d\nBPB_SecPerClus: %d\n",discr.CountofClusters,discr.BPB.BPB_SecPerClus);	
	/*UINT i,j,k;
	for (k=discr.FirstDataSector;k<discr.FirstDataSector+1;k++) {
		if (!device_read("/dev/fd0",(k)*512,512,dir0)) {
			printf("Error\n");
			return 1;
		}
		for (j=0;j<DIR_NUM;j++) {
			if ((!dir0[j*32]) || (dir0[j*32]=='A' && dir0[j*32+1]>'Z')) continue;
			for (i=j*32;i<j*32+11;i++)
				printf("%c",dir0[i]);
			printf("\n");
		}
	}*/
	printf("--------------------------\n");
	if (!fat32_read(node,0,0,dir0)) printf("Error on read\n");
	if (!fat32_umount(node)) printf("Error on unmount!\n");
	/*dummy=malloc(50);
	printf("dummy at 0x%X\n",dummy);
	free(dummy);*/
	return 1;
}

int _sish_interpret(char *cmd) 
{
	char args[STRLEN];

	_sish_split_par(cmd,args);
	if (!(*cmd)) {
		printf("\n");
		return 0;
	}
	if (!strcmp(cmd,"test")) return sish_test();
	if (!strcmp(cmd,"clear")) return _kclear();
	if (!strcmp(cmd,"lba28")) return sish_lba28();
	if (!strcmp(cmd,"paging")) return sish_paging();
	if (!strcmp(cmd,"time")) return sish_time();
	if (!strcmp(cmd,"exit")) return SISH_EXIT;
	if (!strcmp(cmd,"halt")) return SISH_HALT;
	if (!strcmp(cmd,"reboot")) return SISH_REBOOT;
	if (!strcmp(cmd,"help")) return sish_help();
	printf("sish: %s: command not found\n",cmd);
	return 0;
}

int sish()
{
	char input[STRLEN];
	int ret;

	asm ("int $0x21\n\t");
	printf("\nSquaros intern shell (sish) started.\nType \"help\" for a list of built-in commands.\n\n");
	while (1) {
		printf("$ ");
		_kin(input,STRLEN);
		ret=_sish_interpret(input);
		if ((ret & 0xF0)==0xE0) break;
	}
	return ret;
}
