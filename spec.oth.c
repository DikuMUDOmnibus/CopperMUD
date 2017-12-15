/* ************************************************************************
*  file: spec_other.c , Special module.                   Part of DIKUMUD *
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
#include "error.h"
#include "proto.h"

/*   external vars  */

extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct obj_index_data *obj_index;
extern struct time_info_data time_info;
extern struct command_info commands[];

/* extern procedures */

void hit(struct char_data *ch, struct char_data *victim, int type);
void gain_exp(struct char_data *ch, int gain);
char *strdup(char *source);


