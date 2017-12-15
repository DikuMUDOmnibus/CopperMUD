
/* ************************************************************************
*  file: force.c     Implementation of core magic         Part of Copper3 *
*  Usage : Very, very carefully...                                        *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include CONFIG

#include <stdio.h>
#include <strings.h>
#include <ctype.h>

#include "structs.h"
#include "comm.h"
#include "utils.h"
#include "magic.h"
#include "error.h"
#include "proto.h"

/*
#######################################################################
This is a tricky file... it seems once timing is understood, it'll fall
together...how should PERM IMBUE STR differ from IMBUE PERM STR???
#######################################################################
*/

/* For new "core" magic system -
 *
 * This is intended to be entirely private, hence the structures and
 * defines are declared internal to this file, with only the prototypes
 * of the functions to be declared, in proto.h and the ATTRIB_* values
 * (in magic.h??)
 *
 * Part of the aim is to have, at least vaguely, object-oriented magic.
 * This should be easier to graft onto diku than other fundamental
 * changes, since the magic system is mostly independent already from
 * the rest of the code.
 *
 * Spells, or other magical effects, should interface to these routines
 * like this:
 *
 * struct magic *m;
 * int error;
 *
 * if((error=create_magic(<creator of the magic>,&m))!=OKAY) *note ptr to ptr*
 *    return(error);
 *
 * if((error=bind_magic_to_object(m,object))!=OKAY)
 *    return(error);
 * * Perhaps the magic could, on failure, revert to the "caster" *
 * *             if(object<>caster) bind_magic_to_object(m,caster);...
 *
 * * a_to_m does return an error code, but we can ignore it here *
 * attribute_to_magic(m,ATTRIB_VISIBILITY,-5);
 * attribute_to_magic(m,ATTRIB_ODOR,+30);
 *
 * * If the magic is triggered later on, do not call this *
 * expend_magic(m);
 */

/* #define ATTRIB_ define these elsewhere... */

struct attrib {
    int type;
    int value;
    struct attrib *next;
};

/* Magic flags */

#define MAGIC_LOCAL       1 /* Not sure what I meant by this */
#define MAGIC_WAIT        2 /* Delay the application of the attributes */
#define MAGIC_DIFFUSE     4 /* Not identifiable to the source, but will */
                            /* appear in the region near the source */
#define MAGIC_INHERENT    8 /* Something part of the object - not to be */
                            /* dispelled without dispelling the object */
                            /* itself...(don't use this one lightly) */

struct magic {
    struct attrib *attribs;
    struct generic target;
    int label;
    int flags;
    int power;
    int entire_period;    /* Do these make sense? */
    int period_remaining;
    struct magic *next;
};


int create_magic(struct generic *initiator,struct magic **m)
{
    CREATE((*m),struct magic,1);

    switch(initiator->attach_type) {
	case ATTACH_OBJ:
/*            (*m)->power = initiator->attached_to.obj->power; / * (range?);*/
            break;
	case ATTACH_CHAR:
/*            (*m)->power = initiator->attached_to.char->power; / * (range?);*/
            break;
	case ATTACH_ROOM:
/*            (*m)->power = initiator->attached_to.room->power; / * (range?);*/
            break;

        default:
            free(*m);
            return ERROR_INTERNAL;
            break;
    }
    /*TEMP TEMP TEMP*/
    (*m)->power = 30; /*COMPLETELY ARBITRARY VALUE */
    return OKAY;
}

int bind_magic_to_object(struct magic *m, struct generic *target)
{
    if(!m||!target)
        return ERROR_INTERNAL;

    if(number(1,100) > power_over_object(m,target))
        return ERROR_FAILED;

    /* whole structure copy */
    m->target= *target;

    switch(target->attach_type) {
	case ATTACH_ROOM:
            m->next = target->attached_to.room->magic;
            target->attached_to.room->magic = m;
	    break;
	case ATTACH_OBJ:
            m->next = target->attached_to.obj->magic;
            target->attached_to.obj->magic = m;
	    break;
	case ATTACH_CHAR:
            m->next = target->attached_to.ch->magic;
            target->attached_to.ch->magic = m;
	    break;
	default:
	    return ERROR_INTERNAL;
	    break;
    }
    return OKAY;
}

int attribute_to_magic(struct magic *m,int attribute,int value)
{
    struct attrib *a;

    for(a=m->attribs;a;a=a->next)
        if(a->type==attribute) {
            if(a->value==value)
                return OKAY;
            else
                return ERROR_NO_SENSE;
        }

    CREATE(a,struct attrib,1);

    a->type=attribute;
    a->value=value;
    a->next=m->attribs;
    m->attribs=a;
    return OKAY;
}

int query_attribute(struct magic *m,int attribute)
{
    struct attrib *a;

    for(a=m->attribs;a;a=a->next)
        if(a->type==attribute)
            return(a->value);

    return(ERROR_INTERNAL); /* Not a valid place to return ERROR_*... */
}

/* return a percentage of "normal" force that can be exerted on the object*/
int power_over_object(struct magic *m, struct generic *target)
{
    int percent = 100;

    switch(target->attach_type) {
	case ATTACH_ROOM: /* Previous spells? */
            break;
	case ATTACH_OBJ: /* Previous spells? */
            break;
	case ATTACH_CHAR: /* Check magic resistance, level, int/wis */
            break;
        default:
            return(ERROR_INTERNAL); /* Again, bad place for an error */
            break;
    }
    return(percent);
}


int expend_magic(struct magic *m,struct generic *target)
{
    struct attrib *a;
    int imbue=ATTRIB_DIR_SELF; /* A handy default value */
    int duration=0; /* not too handy, but better than random dust */

    for(a=m->attribs;a;a=a->next) {
        switch(a->type) {
            case ATTRIB_NULL: /* nop it */
            case ATTRIB_ORIGIN_T:
            case ATTRIB_ORIGIN_ID:
                break;
            case ATTRIB_STR:
            case ATTRIB_INT:
            case ATTRIB_WIS:
            case ATTRIB_DEX:
            case ATTRIB_CON:
            case ATTRIB_CHR:
                /* This is a key part,  how to do it? 
                attrib_attach(m->target,a->type,a->value,imbue); *****/
                break;
            case ATTRIB_IMBUE:
                imbue=a->value;
                break;
            case ATTRIB_PERMANENT:
                duration=-1;
                break;
            default:
                log("BUG: Bad attribute type in expend_magic");
                break;
        }
    }
    return OKAY;
}

int magical_attack(struct magic *m,struct generic *target)
{
    return OKAY;
}

/* This seems silly - if someone needs to loop, let them loop in this file
int next_attribute(struct magic *m,last)
{
}***/

int remove_magic(struct magic *m)
{
    struct magic *i,**head;
    struct attrib *a;

    switch(m->target.attach_type) {
	case TARGET_ROOM:
            head= &(m->target.attached_to.room->magic);
            break;
	case TARGET_OBJ:
            head= &(m->target.attached_to.obj->magic);
            break;
	case TARGET_CHAR:
            head= &(m->target.attached_to.ch->magic);
            break;
        default:
            return ERROR_INTERNAL;
            break;
    }

    /* Check to see if it's the head of the list */
    if(*head==m) {
        *head = m->next;

    } else {
        /* Get to here, and we have to walk through the list */
        i=*head;
        while(i) {
            if(i->next == m) {
                i->next = m->next;
                break;
            }
            i=i->next;
        }
        if(!i) {
            log("BUG: Bad attachment of magic, remove_magic()");
            return ERROR_INTERNAL;
        }
    }

    while(m->attribs) {
	a = m->attribs->next;
	free(m->attribs);
	m->attribs = a;
    }

    free(m);

    return OKAY;
}
