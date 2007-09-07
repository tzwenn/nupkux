#include <kernel/sish.h>
#include <kernel/ktextio.h>
#include <time.h>
#include <string.h>

int sish();

int sish_help()
{
	printf("List of built-in commands:\n");
	printf("\ttest\t\tRun the current development function\n");
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
	printf("Timestamp:\t%d\n",timestamp);
	return 1;
}

void lba28_init(USHORT controller, UCHAR drive, UINT addr) 
{
	outportb(controller | 0x01,0x00);
	outportb(controller | 0x02,0x01); //UCHAR sectorcount
	outportb(controller | 0x03,(UCHAR) (addr & 0xFF));
	outportb(controller | 0x04,(UCHAR) ((addr>>8) & 0xFF));
	outportb(controller | 0x05,(UCHAR) ((addr>>16) & 0xFF));
	outportb(controller | 0x06,0xE0 | (drive << 4) | ((addr >> 24) & 0x0F));
}

int lba28_read(UCHAR buf[512], USHORT controller, UCHAR drive, UINT addr)
{
	int i;
	USHORT value;
//	UCHAR test[2] = " ";
	time_t timestmp;

	controller=controller & 0xFFF0;
	if ((controller!=0x1F0) && (controller!=0x170)) return 0;
	if ((drive!=0xA0) && (drive!=0xB0)) return 0;
	lba28_init(controller,addr,drive);
	outportb(controller | 0x07,0x20);
	timestmp=time(0);
	while ((inportb(controller | 0x07) & 0x08)==0x08) {if (!(time(0)-timestmp-2)) return 0;}
	timestmp=time(0);
	while ((inportb(controller | 0x07) & 0x0F)!=0x08) {if (!(time(0)-timestmp-2)) return 0;}
	for (i=0;i<256;i++) {
		value=inportw(controller);
		buf[i*2]=(UCHAR) (value & 0xFF);
		buf[i*2+1]=(UCHAR) ((value & 0xFF00) >> 8);
/*		test[0]=buf[i*2];
		printf("%s",test);
		test[0]=buf[i*2+1];
		printf("%s",test);*/
		printf("%.4X",value);
	}
	return 1;
}

int lba28_write(UCHAR buf[512], USHORT controller, UCHAR drive, UINT addr)
{
	int i;
	USHORT value;
	time_t timestmp;

	controller=controller & 0xFFF0;
	if ((controller!=0x1F0) && (controller!=0x170)) return 0;
	if ((drive!=0xA0) && (drive!=0xB0)) return 0;
	lba28_init(controller,addr,drive);
	outportb(controller | 0x07,0x30);
	timestmp=time(0);
	while ((inportb(controller | 0x07) & 0x08)==0x08) {if (!(time(0)-timestmp-2)) return 0;}
	timestmp=time(0);
	while ((inportb(controller | 0x07) & 0x0F)!=0x08) {if (!(time(0)-timestmp-2)) return 0;}
	for (i=0;i<256;i++) {
		value=buf[i*2];
		value|=buf[i*2+1] << 8;
		outportw(controller,value);
	}
	return 1;
}

int test_lba()
{
#define sish_test_sleep() timestamp=time(0); \
	while (time(0)==timestamp)

	UINT controller = 0;
	USHORT tmpword = 0;
	time_t timestamp;
	UCHAR buffer[512];

	printf("Detect Devices ...\n");
	outportb(0x1F6,0xA0);
	sish_test_sleep();
	tmpword=inportb(0x1F7);
	if (tmpword & 0x40) {
		controller=0x01;
		printf("Primary master exists.\n");
		for (tmpword=0;tmpword<512;tmpword++)
			buffer[tmpword]=tmpword;	
		if (!lba28_write(buffer,0x1F0,0xA0,0)) printf("Write-Error\n");
		if (!lba28_read(buffer,0x1F0,0xA0,0)) printf("Read-Error\n");
		printf("Reading address 0 of primary master finished.\n");
	}
	outportb(0x1F6,0xB0);
	sish_test_sleep();
	tmpword=inportb(0x1F7);
	if (tmpword & 0x40) {
		controller=controller | 0x02;
		printf("Primary slave exists.\n");
		if (!lba28_read(buffer,0x1F0,0xB0,0)) printf("Read-Error");
		printf("\nReading address 0 of primary slave finished.\n");
	}
	outportb(0x176,0xA0);
	sish_test_sleep();
	tmpword=inportb(0x177);
	if (tmpword & 0x40) {
		controller=controller | 0x10;
		printf("Secondary master exists.\n");
		if (!lba28_read(buffer,0x170,0xA0,0)) printf("Read-Error");
		printf("\nReading address 0 of secondary master finished.\n");
	}
	outportb(0x176,0xB0);
	sish_test_sleep();
	tmpword=inportb(0x177);
	if (tmpword & 0x40) {
		controller=controller | 0x20;
		printf("Secondary Slave exists.\n");
		if (!lba28_read(buffer,0x170,0xB0,0)) printf("Read-Error");
		printf("\nReading address 0 of secondary slave finished.\n");
	}
	printf("Finished.\n");
	return 1;
}

int sish_test()
{
	printf("Floppydriver called ...\n");
	printf("Init DMA ...\n");
	outportb(0x0A,0x06);	//Disable channel 2
	outportb(
}

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

int _sish_interpret(char *cmd) 
{
	char args[STRLEN];

	_sish_split_par(cmd,args);
	if (!strcmp(cmd,"test")) return sish_test();
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
