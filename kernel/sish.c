#include <kernel/sish.h>
#include <kernel/ktextio.h>
#include <time.h>
#include <lib/string.h>
#include <fs/initrd.h>
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

int rtrim(char *cmd)
{
	char *str = cmd;

	if (!*str) return 0;
	while (*str) str++;
	str--;
	while ((*str<=32) && (str>=cmd)) str--;
	str++;
	*str=0;
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
		rtrim(args);
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

extern UINT initrd_location;

static void format_mode(fs_node *node, char *output)
{
	strcpy(output,"??????????");
	
	if (!node) return;
	UINT mode = node->mode, i=3;
	switch (node->flags) {
		case FS_DIRECTORY:	output[0]='d';
					break;
		case FS_CHARDEVICE:	output[0]='c';
					break;
		case FS_BLOCKDEVICE:	output[0]='b';
					break;
		case FS_PIPE:		output[0]='p';
					break;
		case FS_SYMLINK:	output[0]='l';
					break;
		default:		output[0]='-';
					break;
	}
	while (i--) {
		output[(i+1)*3]=(mode&1)?'x':'-';
		output[i*3+2]=(mode&2)?'w':'-';
		output[i*3+1]=(mode&4)?'r':'-';
		mode>>=3;
	}
}

int sish_test()
{
	printf("---initrd test---\n\n");
	UCHAR *buf;
	UINT i;
	char input[STRLEN],args[STRLEN],the_mode[11];
	fs_node *root,*testfile,*tmp;
	struct dirent *pDirEnt;
	
	root=setup_initrd(initrd_location);
	set_fs_root_node(root);
	
	while (1) {
		printf("> ");
		_kin(input,STRLEN);
		memset(args,0,STRLEN);
		_sish_split_par(input,args);
		if (!(*input)) {
			printf("\n");
			continue;
		}
		if (!strcmp(input,"exit")) break;
		if (!strcmp(input,"read")) {
			testfile=namei(args);
			if (testfile) {
				buf=(UCHAR *)malloc(testfile->size);
				printf("----Open  file \"%s\"----\n",testfile->name);
				read_fs(testfile,0,testfile->size,buf);
				for (i=0;i<testfile->size;i++)
					printf("%c",buf[i]);
				printf("----Close file \"%s\"----\n",testfile->name);
				free(testfile);
			} else printf("Error: Could not open file.\n");
		}
		if (!strcmp(input,"ls")) {
			if (!*args) testfile=root;
				else testfile=namei(args);
			if (testfile) {
				i=0;
				printf("Inode\tMode\t\tUID\tGID\tSize\tName\n");
				if (testfile->flags!=FS_DIRECTORY) {
					tmp=testfile;
					format_mode(tmp,the_mode);
					printf("%d\t%s\t%d\t%d\t%d\t%s\n",tmp->inode,the_mode,tmp->uid,tmp->gid,tmp->size,tmp->name);
				} else while ((pDirEnt=readdir_fs(testfile,i))) {
					tmp=finddir_fs(testfile,pDirEnt->d_name);
					format_mode(tmp,the_mode);
					printf("%d\t%s\t%d\t%d\t%d\t%s\n",tmp->inode,the_mode,tmp->uid,tmp->gid,tmp->size,tmp->name);
					free(tmp);
					i++;
				}
				if (*args) free(testfile);
			} else printf("Error: Could not find file.\n");
		}
	}
			
	remove_initrd(root);
	set_fs_root_node(0);
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
