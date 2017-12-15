/* ************************************************************************
*  file: time.h , Structures                              Part of DIKUMUD *
*  Usage: Declarations of time information and structures                 *
*  Version for Copper III                                                 *
************************************************************************* */

#ifndef TIME_H
#define TIME_H

#define PULSE_ZONE     240
#define PULSE_MOBILE    40 
#define PULSE_VIOLENCE  9
#define WAIT_SEC       4
#define WAIT_ROUND     4

#define MUD_HOURS_PER_DAY    24
#define MUD_DAYS_PER_WEEK    7
#define MUD_DAYS_PER_MONTH   32
#define MUD_MONTHS_PER_YEAR  15

#define SECS_PER_MUD_HOUR    71
#define SECS_PER_MUD_DAY     (MUD_HOURS_PER_DAY*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH   (MUD_DAYS_PER_MONTH*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR    (MUD_MONTHS_PER_YEAR*SECS_PER_MUD_MONTH)

#define SECS_PER_REAL_MIN    60
#define SECS_PER_REAL_HOUR   (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY    (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR   (365*SECS_PER_REAL_DAY)

/* For special days */

struct dayspec {
    int month, day;
    char *line[5]; /* holds description lines for calendar */
};

#endif /* !defined(TIME_H) */
