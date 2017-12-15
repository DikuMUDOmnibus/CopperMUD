/* ************************************************************************
*  file: mob.c ,     Definitions for mobile creatures     Part of DIKUMUD *
*  Usage : Accessed through ch->mob_info                                  *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#ident	"@(#) $Id:$\n"
/* $Log:$
 */

#ifndef MOB_H
#define MOB_H

/* We need the structure of time_info_data */
#ifndef STRUCTS_H
#include "structs.h"
#endif

#define CONCERN_AUTONOMIC 0 /* that which is not considered */
#define CONCERN_SAFETY 1
#define CONCERN_FEED 2
#define CONCERN_COMMUNICATE 3
#define CONCERN_GOTO 4
#define CONCERN_ACQUIRE 5
#define CONCERN_SHELTER 6
#define CONCERN_SELL 7
#define CONCERN_BUY 8 /* part of acquire? */
#define CONCERN_ADVENTURE 9 /* learn new things to satisfy curiousity */
#define CONCERN_FELLOWSHIP 10 /* companions to be concerned about */
#define CONCERN_AID 11        /* bring aid to somebody */
#define CONCERN_IDLE_CONV 12  /* talk about whatever */

struct concern {
    int type;
    int data1,data2;
    int pri;
    int failures;
    /* how to show subordination? */
    struct concern *next;
};

struct memory {
    int type;
    struct time_info_data when;
    int data1,data2;    /* Needs to be generalized greatly */
    struct memory *next;
};

/* Moods for mobs */

#define MOOD_NORMAL       0
#define MOOD_ANGER        1
#define MOOD_MOB          2

#define ACTIVITY_NONE       0
#define ACTIVITY_KILL       1
#define ACTIVITY_SUBDUE     2
#define ACTIVITY_TORTURE    3
#define ACTIVITY_BARGAIN    4
#define ACTIVITY_PRAY       5
#define ACTIVITY_MEDITATE   6
#define ACTIVITY_xxxx

#define MOOD_RESTLESS 1
#define MOOD_ANGRY 2

struct char_mob_data
{
    /* updated stats */
    int evaltime;
    int retval;
    int mood;
    int goal; /* ?? */

    /* constant stats */
    char disposition;     /* 0-100 */
    char appearance;      /* 0-10 */
    char sanity;          /* 0-100 */
    char sensitivity;     /* 0-100 */
    char bravery;         /* 0-100 */
    char morals;          /* 0-100 */
    char greed;           /* 0-100 */

    struct concern *concerns;
    struct memory *short_mem, *long_mem; /* perhaps silly, but... */
};

#endif /* !defined(MOB_H) */
