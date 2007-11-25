#include <kernel/sish.h>
#include <kernel/ktextio.h>
#include <time.h>
#include <lib/string.h>
#include <kernel/devices/fdc.h>
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
	printf("\tclear\t\tCleans up the mess on the screen.\n");
	printf("\tlba28\t\tAccess IDE-ATA drives\n");
	printf("\tpaging\t\tCreates a pagefault and crashes the kernel\n");
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

int sish_test()
{
#define read_start 0x1AC 
#define read_count 1
	UCHAR buf[read_count*512];
	int i;
	
	int *a=(int *)malloc(sizeof(int));
	int *b=(int *)malloc(sizeof(int)),*c;
	if (!a || !b) return 1;
	*a=0;
	*b=0;
	printf("_a @ 0x%X; _b @ 0x%X\n",(UINT)a-sizeof(mm_header),(UINT)b-sizeof(mm_header));
	printf("hdr: %d; ftr: %d\na:",sizeof(mm_header),sizeof(mm_footer));
	for (i=(UINT)a-sizeof(mm_header);i<(UINT)a;i++)
		printf("%.2X-",*((UCHAR *)i));
	printf("\b|");
	for (i=(UINT)a+4;i<(UINT)a+4+sizeof(mm_footer);i++)
		printf("%.2X-",*((UCHAR *)i));
	printf("\b\nb:");
	for (i=(UINT)b-sizeof(mm_header);i<(UINT)b;i++)
		printf("%.2X-",*((UCHAR *)i));
	printf("\b|");
	for (i=(UINT)b+4;i<(UINT)b+4+sizeof(mm_footer);i++)
		printf("%.2X-",*((UCHAR *)i));
	printf("\b\n");
	c=malloc(3*sizeof(int));
	free(a);
	free(b);
	*c=0;
	printf("_c @ 0x%X\n",(UINT)c-sizeof(mm_header));
	free(c);
	return 1;

	printf("Programm/module from floppy\n");
	if (!fdc_read_block(read_start,buf,read_count)) {
		printf("Can't read floppy\n");
		return 1;
	}
	for (i=0;i<read_count*512;i++) {
		if (i==512) printf("|");
		printf("%X",buf[i]);
	}
	/*buf[6]=0xCD;
	buf[7]=0x80;
	buf[8]=0xC3;*/
	printf("\nCall ... \n");
	asm ("call *%%ebx\n\t"::"b"(buf));
	printf("\n\n");
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

	printf("\nSquaros intern shell (sish) started.\nType \"help\" for a list of built-in commands.\n\n");
	while (1) {
		printf("$ ");
		_kin(input,STRLEN);
		ret=_sish_interpret(input);
		if ((ret & 0xF0)==0xE0) break;
	}
	return ret;
}
