
/* ************************************************************************
*  file: spec_generic.c , Special module.                 Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
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


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interp.h"
#include "db.h"
#include "event.h"
#include "error.h"
#include "proto.h"

/*   external vars  */

extern struct room_data *world;
extern int top_of_world;
extern struct zone_data *zone_table;
extern struct obj_index_data *obj_index;
extern struct time_info_data time_info;

/* extern procedures */

void hit(struct char_data *ch, struct char_data *victim, int type);
char *strdup(char *source);

/* what in the world would this return? char * is silly, methinks... */

int teach(struct char_data *ch,struct event *e)
{
    return OKAY;
}

int guild_administrator(struct char_data *ch,struct event *e)
{
    return OKAY;
}
