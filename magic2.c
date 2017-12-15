/* ************************************************************************
*  file: magic2.c ,     Implementation of magic           Part of DIKUMUD *
*  Usage : Basic ways to access core routines                             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#ident	"@(#) $Id:$\n"
/* $Log:$
 */


#include <strings.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interp.h"
#include "db.h"
#include "skills.h"
#include "magic.h"
#include "error.h"
#include "proto.h"

/* local variables */

/* extern variables */

extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern char log_buf[];

/* extern procedures */


/* local procedures */


int invis_spell(struct char_data *
