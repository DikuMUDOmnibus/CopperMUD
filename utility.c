/* ************************************************************************
*  file: utility.c, Utility module.                       Part of DIKUMUD *
*  Usage: Utility procedures                                              *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */




#include CONFIG

#if HAVE_STRINGS_H

#include <strings.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#include <ctype.h>

#include <time.h>

#include "structs.h"
#include "db.h"
#include "utils.h"
#include "time.h"
#include "proto.h"

extern struct time_data time_info;
extern struct room_data *world;
extern struct zone_data *zone_table;


int MIN(int a, int b)
{
    return a < b ? a:b;
}


int MAX(int a, int b)
{
    return a > b ? a:b;
}

/* Choose a random bit from the number supplied */
int choose_bit(int mask)
{
    int count=0,i,choose;

    for(i=0;i<32;i++)
        if(mask & (2 << i))
            count++;

    if(!count)
        return -1;

    choose=number(0,count-1);

    count=0;
    for(i=0;i<32;i++)
        if(mask & (2 << i))
            if(count++==choose)
                return 2 << i;
    return -1;
}

/* Turn a 2^n into n (if not a power of 2, it will return the lowest bit) */
int bitcard(int number)
{
    int i=0;

    if(!number)
        return -1;

    for(i=0;i<32;i++)
        if(number & (2 << i))
            return i;
    return -1;
}

char *rating(int number,int scale,int mode)
{
    int percentile;

    percentile = (number*100)/scale;

    if(percentile < 5) {
        return("no");
    }
    if(percentile < 15) {
        return("hardly any");
    }
    if(percentile < 25) {
        return("poor");
    }
    if(percentile < 35) {
        return("some");
    }
    if(percentile < 65) {
        return("roughly average");
    }
    if(percentile < 75) {
        return("good");
    }
    if(percentile < 85) {
        return("pretty good");
    }
    if(percentile < 85) {
        return("outstanding");
    }
    return("excellent");
}

bool check_hide(struct char_data *hiding,struct char_data *seeing)
{
    int i;
/*    struct skill_data *sk;*/

    i=number(0,100);

   /* sk=get_skill(hiding,<HIDE>);
    if(number < (sk->... + modifiers...))*/
        return TRUE;
    return FALSE;
}

bool CAN_SEE(struct char_data *ch,struct char_data *vict)
{
#if 0
    /* can character see *anything*? */
    if(IS_AFFECTED(ch,AFF_BLIND))
	return(FALSE);
    if(!IS_AFFECTED(ch,AFF_INFRARED) && !IS_LIGHT(vict->in_room))
	return(FALSE);

    /* is victim magically hidden? */
    if(IS_AFFECTED(vict,AFF_INVISIBLE) && !IS_AFFECTED(vict,AFF_INFRARED)
	    && !IS_AFFECTED(vict,AFF_DETECT_INVISIBLE))
	return(FALSE);

    /* is victim trying to hide? */
    if(IS_AFFECTED(vict,AFF_HIDE) && check_hide(vict,ch))
        return(FALSE);
#endif
    return(TRUE);
}

bool CAN_SEE_OBJ(struct char_data *ch,struct obj_data *obj)
{
/*
    if(IS_AFFECTED(ch,AFF_BLIND))
	return(FALSE);*/

    /* Check the visibility at the location of the item */
    if(obj->in_room!=NOWHERE) {
	if(!IS_LIGHT(obj->in_room))
	    return(FALSE);
    } else if(obj->carried_by) {
	if(!IS_LIGHT(obj->carried_by->in_room))
	    return(FALSE);
    } else if(obj->in_obj) {
	if(!CAN_SEE_OBJ(ch,obj->in_obj))
	    return(FALSE);
    }
/*
    if(IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE) &&
	    !IS_AFFECTED(ch,AFF_DETECT_INVISIBLE))
	return(FALSE);*/
    if(IS_SET(obj->obj_flags.extra_flags, ITEM_SECRET))
	return(FALSE);
    return(TRUE);
}

/* creates a random number in interval [from;to] */
int number(int from, int to) 
{
    int range;

    range = abs(to - from) + 1;
    return((random() % range) + MIN(to,from));
}



/* simulates dice roll */
int dice(int number, int size) 
{
    int r;
    int sum = 0;

    if(size < 1) {
        log("BUG: dice with number < 1");
        return 1; /* relatively safe number */
    }

    for (r = 1; r <= number; r++) sum += ((random() % size)+1);
        return(sum);
}


#if !(HAVE_STRDUP)
/* Create a duplicate of a string */
char *strdup(char *source)
{
    char *new;

    CREATE(new, char, strlen(source)+1);
    return(strcpy(new, source));
}
#endif



/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
    int chk, i;

    for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
	if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
	    if (chk < 0)
		return (-1);
	    else 
		return (1);
    return(0);
}



/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(char *arg1, char *arg2, int n)
{
    int chk, i;

    for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n>0); i++, n--)
	if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
	    if (chk < 0)
		return (-1);
	    else 
		return (1);

    return(0);
}



/* writes a string to the log */
void log(char *str)
{
    long ct;
    char *tmstr;

    ct = time(0);
    tmstr = asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    fprintf(stderr, "%s :: %s\n", tmstr, str);
}
    


void sprintbit(long vektor, char *names[], char *result)
{
    long nr;

    *result = '\0';

    for(nr=0; vektor; vektor>>=1)
    {
	if (IS_SET(1, vektor))
	    if (*names[nr] != '\n') {
		strcat(result,names[nr]);
		strcat(result," ");
	    } else {
		strcat(result,"UNDEFINED");
		strcat(result," ");
	    }
	if (*names[nr] != '\n')
	 nr++;
    }

    if (!*result)
	strcat(result, "NOBITS");
}



void sprinttype(int type, char *names[], char *result)
{
    int nr;

    for(nr=0;(*names[nr]!='\n');nr++);
    if(type < nr)
	strcpy(result,names[type]);
    else
	strcpy(result,"UNDEFINED");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
    long secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

 now.hours = (secs/SECS_PER_REAL_HOUR) % 24;  /* 0..23 hours */
 secs -= SECS_PER_REAL_HOUR*now.hours;

 now.day = (secs/SECS_PER_REAL_DAY);          /* 0..34 days  */
 secs -= SECS_PER_REAL_DAY*now.day;

    now.month = -1;
 now.year  = -1;

    return now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
    long secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs/SECS_PER_MUD_HOUR) % MUD_HOURS_PER_DAY;
    secs -= SECS_PER_MUD_HOUR*now.hours;

    now.day = (secs/SECS_PER_MUD_DAY) % MUD_DAYS_PER_MONTH;
    secs -= SECS_PER_MUD_DAY*now.day;

    now.month = (secs/SECS_PER_MUD_MONTH) % MUD_MONTHS_PER_YEAR;
    secs -= SECS_PER_MUD_MONTH*now.month;

    now.year = (secs/SECS_PER_MUD_YEAR);        /* 0..XX? years */

    return now;
}



struct time_info_data age(struct char_data *ch)
{
    struct time_info_data player_age;

    player_age = mud_time_passed(time(0),ch->player.time.birth);

    player_age.year += 17;   /* All players start at 17 */

    return player_age;
}

int default_loc(int hometown)
{
    switch(hometown) {
	case 0:
	    return(1200);
	    break;
	case 1:
	    return(3001);
	    break;
	case 2:
	    return(12591);
	    break;
	default:
	    log("Bad hometown!");
	    break;
    }
    return(0);
}

int jail_hometown(int hometown)
{
    switch(hometown) {
	case 0:
	    return(10);
	    break;
	case 1:
	    return(3067);
	    break;
	case 2:
	    return(12585);
	    break;
	default:
	    log("Bad hometown!");
	    break;
    }
    return(0);
}

void strToLower(char *s1)
{
 int i, len;

 len = strlen(s1);
 for (i = 0; i < len; i++) {
    if(s1[i] >= 'A' && s1[i] <= 'Z')
	s1[i] += 'a'-'A';
 }
} /* strToLower */

#ifndef HAVE_STRSTR
/*
* PURPOSE:
*    strstr() locates the first occurrence in the string pointed to by s1 of
*    the sequence of characters in the string pointed to by s2.  A NULL value
*    is returned if s2 was not found within s1.
*/

char *strstr (s1, s2)
 char *s1;
 char *s2;
{
 char *s1Ptr; /* A pointer into string s1 */
 char *s2Ptr; /* A pointer into string s2 */

 if (*s2 == '\0') {
  return s1;
 } /* if */

 while (*s1 != '\0') {
   if (*s1 == *s2) {

    s1Ptr = s1;
    s2Ptr = s2;

    while ((*s1Ptr != '\0') && (tolower(*s1Ptr) == tolower(*s2Ptr))) {
      ++s1Ptr;
      ++s2Ptr;
    } /* while */

    if (*s2Ptr == '\0') {
      /*
      * At this point the entire s2 string has been processed.
      * This implies that the substring was found in s1.
      */
      return s1;
    } /* if */
    else if (*s1Ptr == '\0') {
      /*
      * If were at the end of s1 and not at the end of s2
      * the substring doesn't exist.
      */
      return NULL;
    } /* if */
   } /* if */
   ++s1;
 } /* while */

 return NULL;

} /* strstr */
#endif

/* this is a case independent version of the above */
char *stristr (s1, s2)
 char *s1;
 char *s2;
{
 char *s1Ptr; /* A pointer into string s1 */
 char *s2Ptr; /* A pointer into string s2 */

 if (*s2 == '\0') {
  return s1;
 } /* if */

 while (*s1 != '\0') {
   if (tolower(*s1) == tolower(*s2)) {

    s1Ptr = s1;
    s2Ptr = s2;

    while ((*s1Ptr != '\0') && (tolower(*s1Ptr) == tolower(*s2Ptr))) {
      ++s1Ptr;
      ++s2Ptr;
    } /* while */

    if (*s2Ptr == '\0') {
      /*
      * At this point the entire s2 string has been processed.
      * This implies that the substring was found in s1.
      */
      return s1;
    } /* if */
    else if (*s1Ptr == '\0') {
      /*
      * If were at the end of s1 and not at the end of s2
      * the substring doesn't exist.
      */
      return NULL;
    } /* if */
   } /* if */
   ++s1;
 } /* while */

 return NULL;

} /* strstr */
