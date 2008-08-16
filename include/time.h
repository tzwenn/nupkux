/*
 *  Copyright (C) 2007,2008 Sven Köhler
 *
 *  This file is part of Nupkux.
 *
 *  Nupkux is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Nupkux is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Nupkux.  If not, see <http://www.gnu.org/licenses/>.
 */

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

extern ULONG ticks;
extern time_t time(time_t *tp);	
extern time_t mktime(struct tm *timeptr); 
//Write a ctime, it will make things much easyer
extern struct tm getrtctime(void);
extern int removetimezone(struct tm *timeptr);
extern void sleep(UINT msecs);
extern void setup_timer(void);

#endif
