

#ifndef EVENT_H
#define EVENT_H

#ifndef STRUCTS_H
#include "structs.h"
#endif


/*** Event Types ***/

/* generic */
#define EVENT_NOISE     1
#define EVENT_ACTION    2
#define EVENT_DEATH     3
#define EVENT_ATTACK    4

/* movement */
#define EVENT_ARRIVE    10
#define EVENT_DEPART    11

/* negotiation */
#define EVENT_OFFER     20
#define EVENT_ACCEPT    21
#define EVENT_DECLINE   22
#define EVENT_WITHDRAW  23
#define EVENT_JOIN      24 /* Threatens to make OFFER too complicated */

/* personal */
#define EVENT_SOCIAL    30
#define EVENT_SPEECH    31
#define EVENT_RELATE    31

/* physical */
#define EVENT_HUNGER    40
#define EVENT_THIRST    41
#define EVENT_SLEEPY    42
#define EVENT_IMPAIR    43
#define EVENT_SHOCK     44
#define EVENT_2HOT      45
#define EVENT_2COLD     46
#define EVENT_NOBREATHE 47
#define EVENT_WAKE      48

/* other */
#define EVENT_DISCON	90 /* we lost a player character */

/*** data for specific events ***/

/* NOISE */
#define NOISE_CLANG     20

#define NOISE_BURP      50
#define NOISE_SNEEZE    51
#define NOISE_COUGH     52

struct event {
    int type;
    int data;
    CHAR_ID initiator,receiver;
    char *text; /* if any? */
    struct event *next;
};


#define NEGO_SALE 1
#define NEGO_JOIN 2

struct nego_data {
    int type;
    int price;
    struct obj_data *obj; /* I hate using a pointer...how else? OBJ_ID? */
};

#endif /* EVENT_H */
