
/* ************************************************************************
*  file: error.h     Definition of error codes.           Part of DIKUMUD *
*  Usage : return these values anywhere in a command stream               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#ifndef ERROR_H
#define ERROR_H

#ident	"@(#) $Id:$\n"
/* $Log:$
 */



#define OKAY			0 /* Everything's fine */

#define ERROR_INTERNAL          1 /* Probably a bug of some sort */
#define ERROR_SYNTAX		2 /* Error in specification of a command */
#define ERROR_POSITION		3 /* Can't do that while xxx-ing */
#define ERROR_MISSING_TARGET	4 /* Who? What? */
#define ERROR_LOCATION		5 /* Can't do that in this room */
#define ERROR_NO_MOVES		6 /* Need (more) moves to do that */
#define ERROR_NO_MANA		7 /* Need (more) mana to do that */
#define ERROR_PHYS_PREVENTS     8 /* A log in the road, for example */
#define ERROR_MAGIC_PREVENTS	9 /* Mysterious forces involved */
#define ERROR_NO_KNOWLEDGE     10 /* You don't know about that - forget it */
#define ERROR_FULL             11 /* limit of capacity (orgs/items carried)*/
#define ERROR_FAILED           12 /* if there was a chance of success */
#define ERROR_RANGE            13 /* outside physical limits */
#define ERROR_AUTH             14 /* no authority to accomplish it */
#define ERROR_NO_SENSE         15 /* how impossible can you get, idiot? */
                                  /* for example, putting a bag inside itself*/
#define ERROR_ALREADY_DONE     16 /* What's the point in redoing it? */
#define ERROR_BODY             17 /* something that can't be done in the body */

#endif /* !defined(ERROR_H) */
