/* ************************************************************************
*  file: item.c , Some utilities for messing        Part of CopperDIKUMUD *
*  with items, especially for counting them in lists                      *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */




#include CONFIG

#if HAVE_STRINGS_H

#include <strings.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif


#include "structs.h"
#include "utils.h"
#include "db.h"
#include "error.h"
#include "comm.h"
#include "proto.h"

/* put this in a .h file sometime */
#define LOOK_OBJS 32768

/* extern variables */

extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct room_data *world;

/* lib protos */
int strcasecmp(char *s1,char *s2);
	
/* for modes */
#define SINGLE        1
#define ALL           2
#define ALL_OF_TYPE   3

/********** How to use... ************

struct obj_data *marker,*this_item;

marker=first_in_some_list;
this_item=NULL;

 *** For a get everything type command ***

while(count=count_next(&this_item,&marker)) {
 sprintf(buf,"There are %d %s here.\n\r",count,this_item->short_desc);
 send_to_char(buf,ch);
}
clear_item_list(first_in_some_list);

 *** For a get all.blue type command ***

marker=get_next_blank_of_type(.....);

if(marker) {
 while(marker) {
   count=count_next(&this_item,&marker);
   sprintf(buf,"You munge %d %s(s)\n\r",count,this_item->short_desc);
   send_to_char(buf,ch);
   marker=get_next_blank_of_type(.....);
 }
}
*/





void clear_item_list(struct obj_data *obj)
{
 struct obj_data *k;
 for(k=obj;k;k=k->next_content)
   REMOVE_BIT(k->obj_flags.extra_flags,ITEM_ARCHIVE);
}
 
int count_next(struct char_data *ch,struct obj_data **out_item,
       struct obj_data **marker)
{
 struct obj_data *k;
 int count=0;
 bool blanks_yet=FALSE;
 
 *out_item=*marker;
 for(k=*marker;k;k=k->next_content)
   if(!CAN_SEE_OBJ(ch,k)) {
    SET_BIT(k->obj_flags.extra_flags,ITEM_ARCHIVE);
   } else if(isname((*marker)->name,k->name)) {
    SET_BIT(k->obj_flags.extra_flags,ITEM_ARCHIVE);
    count++;
   } else if(!blanks_yet) {
    blanks_yet=TRUE;
    *marker=k;
   }
 return(count);
}

struct obj_data *fetch_next_item(struct char_data *ch,struct obj_data *start,
	struct obj_data *match)
{
    struct obj_data *k;

    for(k=start;k;k=k->next_content) {
	if(!CAN_SEE_OBJ(ch,k))
	    SET_BIT(k->obj_flags.extra_flags,ITEM_ARCHIVE);
	else if(k->item_number==match->item_number &&
	!IS_SET(k->obj_flags.extra_flags,ITEM_ARCHIVE)) {
	    SET_BIT(k->obj_flags.extra_flags,ITEM_ARCHIVE);
	    return(k);
	}
    }
    return(NULL);
}

struct obj_data *fetch_next_text(struct char_data *ch,struct obj_data *start,
	char *text)
{
    struct obj_data *k;

    for(k=start;k;k=k->next_content) {
	if(!CAN_SEE_OBJ(ch,k))
	    SET_BIT(k->obj_flags.extra_flags,ITEM_ARCHIVE);
	else if(isname(text,k->name) &&
	!IS_SET(k->obj_flags.extra_flags,ITEM_ARCHIVE)) {
	    SET_BIT(k->obj_flags.extra_flags,ITEM_ARCHIVE);
	    return(k);
	}
    }
    return(NULL);
}

struct obj_data *find_unarchived(struct char_data *ch,struct obj_data *start)
{
    struct obj_data *k;

    for(k=start;k;k=k->next_content)
	if(!IS_SET(k->obj_flags.extra_flags,ITEM_ARCHIVE) &&
		CAN_SEE_OBJ(ch,k))
	    return(k);

    return(NULL);
}

/* Macros for different types of the following */
#define SINGLE_ITEM 1
#define ALL_ITEMS   2
#define ALL_TYPE    3

/* This is a generic way of handling items for commands which require just
 one "object" -> things like get, drop, wear, etc. ("put" on the other
 hand would not work).

 "flags" corresponds to a bitvector identical to that of generic_find
 (the CHAR bits are ignored), which correspond to what "places" items
 can be at for the command (foolish to "get" something already carried).

 The function is passed a pointer to another function to call when it
 has an item which satisfies the argument and parameters of the call.
*/

void single_item_processor(struct char_data *ch, char *argument, int cmd,
    int flags, bool func(struct char_data *ch, struct obj_data *obj))
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *sub_object;
    struct obj_data *j,*k;
    struct obj_data *start_at,*this_item;
    int mode,bits;
    bool alldot = FALSE;
    char allbuf[MAX_STRING_LENGTH];


    argument_interpreter(argument, arg1, arg2);

    /* get type */
    if (!*arg1) {
	send_to_char("This interpreter is not a mind reader.\n\r\n\r",ch);
	return;
    }
    alldot = FALSE;
    allbuf[0] = '\0';
    if (sscanf(arg1, "all.%s", allbuf) != 0) {
	mode=ALL_OF_TYPE;
    } else if (!str_cmp(arg1,"all")) {
	mode=ALL;
    } else {
	mode=SINGLE;
    }

    /* get the "first" item here */
    if(!*arg2) {
	bits=generic_find(arg1,flags,ch,(void **)&start_at);
	if(!start_at) {
	    send_to_char("There's nothing appropriate available.\n\r",ch);
	    return;
	}
    } else if(!strcasecmp(arg2,"room")) {
	if(!(flags & FIND_OBJ_ROOM)) {
	    send_to_char("You can't do that to items in the room.\n\r",ch);
	    return;
	}
	start_at=world[ch->in_room].contents;
    } else if(!strcasecmp(arg2,"inventory")) {
	if(!(flags & FIND_OBJ_INV)) {
	    send_to_char("You can't do that to items in your inventory.\n\r",ch);
	    return;
	}
	start_at=ch->carrying;
    } else if((sub_object=get_obj_in_list_vis(ch,arg2,
                    world[ch->in_room].contents))) { /*add "from" option? */
	if(!(flags & LOOK_OBJS)) {
	    send_to_char("You can't do that with items in containers.\n\r",ch);
	    return;
	}
	start_at=sub_object->contains;
    } else {
	sprintf(buf,"What is %s?",arg2);
	send_to_char(buf,ch);
	return;
    }

    switch(mode) {
	case ALL:
	    this_item=start_at;
	    do {
		k=this_item;
		do {
		    func(ch,k);
		} while((k=fetch_next_item(ch,k,this_item)));
	    } while((j=find_unarchived(ch,j)));
	    clear_item_list(start_at);
	    break;
	case ALL_OF_TYPE:
	    k=this_item;
	    do {
		func(ch,k);
	    } while((k=fetch_next_text(ch,this_item,allbuf)));
	    clear_item_list(start_at);
	    break;
	case SINGLE:
	    func(ch,start_at);
	    break;
	default:
	    log("BUG: Bad mode in single_item_processor!");
	    send_to_char("Bug! Please report\n\r",ch);
	    break;
    }
}
