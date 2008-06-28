#ifndef _TIME_H
#define _TIME_H

#include <kernel.h>

#ifndef _TIME_T
#define _TIME_T
  typedef long time_t;
#endif
typedef long clock_t;

struct tm {
   int tm_sec;      /* Sekunden - [0,59] */
   int tm_min;      /* Minuten - [0,59] */
   int tm_hour;     /* Stunden - [0,23] */
   int tm_mday;     /* Tag des Monats - [1,31] */
   int tm_mon;      /* Monat im Jahr - [0,11] */
   int tm_year;     /* Jahr (2 Stellen) */
   int tm_wday;     /* Tage seit Sonntag (Wochentag) - [0,6] */
   int tm_yday;     /* Tage seit Neujahr (1.1.) - [0,365] */
   int tm_isdst;    /* Sommerzeit-Flag */
};

extern time_t time(time_t *tp);	
extern time_t mktime(struct tm *timeptr); 
//Write a ctime, it will make things much easyer
extern struct tm getrtctime();
extern int removetimezone(struct tm *timeptr);
extern void sleep(UINT msecs);

#endif
