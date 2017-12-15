/* ************************************************************************
*  file: act.nego.c , Implementation of commands.  Part of Copper Copper3 *
*  Usage : Negotiation                                                    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

/* $Id: $ */




#include CONFIG

#if HAVE_STRINGS_H

#include <strings.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "event.h"
#include "interp.h"
#include "db.h"
#include "error.h"
#include "proto.h"

/* extern variables */

extern struct room_data *world;
extern struct zone_data *zone_table;

int do_offer(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *to;
    char charname[MAX_INPUT_LENGTH], what[MAX_INPUT_LENGTH];

    argument_interpreter(argument,charname,what);

    to=get_char_room_vis(ch,charname);

    if(!to) {
        send_to_char("Offer to whom?\n",ch);
        return ERROR_MISSING_TARGET;
    }

    /* Determine offering */

    /* Add event */
/*    add_event(ch->in_room,EVENT_OFFER,??,ch->id,to->id);*/
    return OKAY;
}

int do_accept(struct char_data *ch, char *argument, int cmd)
{
    struct event *e;
    char buf[MAX_INPUT_LENGTH];
    struct char_data *from;

    one_argument(argument,buf);
/*
    from=get_char_room_vis(ch,buf);
    if(!from) {
        ...
        return ERROR_MISSING_TARGET;
    }*/

    for(e=ch->events;e;e=e->next) {
        if(e->type==EVENT_OFFER && e->initiator==from->id &&
                (e->receiver==ch->id || e->receiver==ID_NOBODY)) {
            /* Do it... */
        }
    }
    return OKAY;
}

int do_buy(struct char_data *ch, char *argument, int cmd)
{
    return OKAY;
}

int do_sell(struct char_data *ch, char *argument, int cmd)
{
    return OKAY;
}
