#include <kernel/sish.h>
#include <kernel/ktextio.h>
#include <time.h>
#include <string.h>

int sish();

int sish_help()
{
	printf("List of built-in commands:\n");
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
	char *wdays[7] = {
		"Sunday","Monday","Thursday","Wednesday","Tuesday","Friday","Saturday"
	};
	
	now=getrtctime();
	printf("Time:\t\t%d:%d:%d\n",now.tm_hour,now.tm_min,now.tm_sec);
	printf("Date:\t\t%s %d-%d-%d\n",wdays[now.tm_wday],now.tm_year,now.tm_mon,now.tm_mday);
	removetimezone(&now);
	timestamp=mktime(&now);	
	printf("Timestamp:\t%d\n",timestamp);
	return 1;
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
