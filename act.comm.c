/* ************************************************************************
*  file: act.comm.c , Implementation of commands.  Part of Copper DikuMUD *
*  Usage : Communication.                                                 *
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
#include "interp.h"
#include "db.h"
#include "player.h"
#include "error.h"
#include "proto.h"

/* extern variables */

extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct descriptor_data *descriptor_list;
extern char log_buf[];


bool is_overload(struct char_data *ch)
{
    if(++ch->specials.ovl_count > OVL_LIMIT) {
	send_to_char("SHUT UP ALREADY!!! Your actions lose meaning at this speed!\n\r",ch);
	/* So that nobody will be locked out for too long */
	ch->specials.ovl_count=MIN(ch->specials.ovl_count,
			2*OVL_LIMIT);
	return(TRUE);
    }
    return(FALSE);
}

bool is_silent(struct char_data *ch,bool showit)
{
    if(IS_SET(zone_table[world[ch->in_room].zone].flags,ZONE_SILENT)) {
	if(showit)
	    send_to_char("The world is deathly silent, and no noise forms...\n\r",ch);
	return(TRUE);
    }
    return(FALSE);
}

bool can_talk(struct char_data *ch)
{
/*	if(IS_HUMANOID(ch) || GET_CLASS(ch)==CLASS_DRAGON ||
	    GET_CLASS(ch)==CLASS_GIANT || GET_CLASS(ch)==CLASS_DEMON
	    || GET_CLASS(ch)==CLASS_OTHER)
	return(TRUE);
    act("$n tries to communicate something but can't!",TRUE,ch,0,0,TO_ROOM);
    act("Who said you could talk?",FALSE,ch,0,0,TO_CHAR);
    return(FALSE);*/
    return(TRUE);
}

int do_say(struct char_data *ch, char *argument, int cmd)
{
    int i;
    char buf[MAX_STRING_LENGTH];

    for (i = 0; *(argument + i) == ' '; i++);

    if (!*(argument + i)) {
	send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
	return ERROR_SYNTAX;
    }
    if(is_overload(ch)||is_silent(ch,TRUE)||!can_talk(ch))
        return ERROR_FAILED;

    sprintf(buf,"$n says '%s'", argument + i);
    act(buf,FALSE,ch,0,0,TO_ROOM);
    if(ch->prefs && IS_SET(ch->prefs->flags,PLR_MSGECHO)) {
        sprintf(buf,"You say '%s'", argument + i);
        act(buf,FALSE,ch,0,0,TO_CHAR);
    }
    return OKAY;
}


/* Changed to only shout to people in your zone */
/* Changed it back. I like global shouts. Changed so only level 2 or above */
/* can use. -Swiftest */

int do_shout(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    struct descriptor_data *i;


    for (; *argument == ' '; argument++);

    if (!(*argument)) {
	send_to_char("Shout? Yes! Fine! Shout we must, but WHAT??\n\r", ch);
        return ERROR_SYNTAX;
    }
    if(is_overload(ch)||is_silent(ch,TRUE)||!can_talk(ch))
        return ERROR_FAILED;

    sprintf(buf, "$n shouts '%s'", argument);

    for(i = descriptor_list; i; i = i->next)
   	if(i->character != ch&&!i->connected&&!is_silent(i->character,FALSE))
/* && 
      (world[i->character->in_room].zone == 
      world[ch->in_room].zone)
*/
		act(buf, 0, ch, 0, i->character, TO_VICT);

    
    if(ch->desc)
	ch->desc->last_input[0]='\0';

    if(ch->prefs && IS_SET(ch->prefs->flags,PLR_MSGECHO)) {
        sprintf(buf,"You shout '%s'", argument);
        act(buf,FALSE,ch,0,0,TO_CHAR);
    }
    return OKAY;
}

int do_yell(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    struct descriptor_data *i;


    if (!IS_NPC(ch) && GET_EXP(ch)<1500) {
	send_to_char("Due to misuse, you need to have at least 1,500 exp to yell.\n\r",ch);
	return ERROR_FAILED;
    }

    for (; *argument == ' '; argument++);

    if (!(*argument)) {
	send_to_char("So, what do you want to yell, bud?\n\r", ch);
        return ERROR_SYNTAX;
    }
    if(is_overload(ch)||is_silent(ch,TRUE)||!can_talk(ch))
        return ERROR_FAILED;

    sprintf(buf, "$n yells '%s'", argument);

    for (i = descriptor_list; i; i = i->next)
        if (i->character != ch && !i->connected &&
	(world[i->character->in_room].zone == 
	world[ch->in_room].zone) &&
	(!(world[ch->in_room].room_flags & INDOORS) ||
	i->character->in_room==ch->in_room))
            act(buf, 0, ch, 0, i->character, TO_VICT);

    if(ch->prefs && IS_SET(ch->prefs->flags,PLR_MSGECHO)) {
        sprintf(buf,"You yell '%s'", argument);
        act(buf,FALSE,ch,0,0,TO_CHAR);
    }

    return OKAY;
}

int do_tell(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char name[MAX_INPUT_LENGTH], message[MAX_STRING_LENGTH],
	buf[MAX_STRING_LENGTH];

    half_chop(argument,name,message);

    if(!*name || !*message) {
	send_to_char("Who do you wish to tell what??\n\r", ch);
        return ERROR_SYNTAX;
    }
    if(is_overload(ch) || is_silent(ch,TRUE) || !can_talk(ch))
	return ERROR_FAILED;
    if (!(vict = get_char_vis(ch, name))) {
	send_to_char("No-one by that name here..\n\r", ch);
        return ERROR_MISSING_TARGET;
    }
    if (ch == vict) {
	send_to_char("You try to tell yourself something.\n\r", ch);
        return OKAY;
    }
    if (((GET_POS(vict) == POS_SLEEP) ||
	(is_silent(vict,FALSE)) || (!IS_NPC(vict) && !vict->desc))) {
	act("$E can't hear you.",FALSE,ch,0,vict,TO_CHAR);
        return ERROR_FAILED;
    }
    sprintf(buf,"%s tells you '%s'\n\r",
	 (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),message);
    send_to_char(buf, vict);

    if(ch->prefs && IS_SET(ch->prefs->flags,PLR_MSGECHO)) {
        sprintf(buf,"You tell $N '%s'.", message);
        act(buf,FALSE,ch,0,vict,TO_CHAR);
    }

    return OKAY;
}



int do_whisper(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char name[MAX_INPUT_LENGTH], message[MAX_STRING_LENGTH],
	buf[MAX_STRING_LENGTH];

    half_chop(argument,name,message);

    if(!*name || !*message) {
	send_to_char("Who do you want to whisper to.. and what??\n\r", ch);
        return ERROR_SYNTAX;
    }
    if (!(vict = get_char_room_vis(ch, name))) {
	send_to_char("No-one by that name here..\n\r", ch);
        return ERROR_MISSING_TARGET;
    }
    if(is_overload(ch) || is_silent(ch,TRUE) || !can_talk(ch))
	return ERROR_FAILED;

    if(vict == ch) {
	send_to_char(
	    "You can't seem to get your mouth close enough to your ear...\n\r",
	    ch);
        return ERROR_FAILED;
    }
    sprintf(buf,"$n whispers to you, '%s'.",message);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    act("$n whispers something to $N.", FALSE, ch, 0, vict, TO_NOTVICT);

    if(ch->prefs && IS_SET(ch->prefs->flags,PLR_MSGECHO)) {
        sprintf(buf,"You whisper '%s' to $N.", message);
        act(buf,FALSE,ch,0,vict,TO_CHAR);
    }

    return OKAY;
}


int do_ask(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char name[MAX_INPUT_LENGTH], message[MAX_STRING_LENGTH],
	buf[MAX_STRING_LENGTH];

    half_chop(argument,name,message);

    if(!*name || !*message) {
	send_to_char("Who do you want to ask something.. and what??\n\r", ch);
        return ERROR_SYNTAX;
    }
    if (!(vict = get_char_room_vis(ch, name))) {
	send_to_char("No-one by that name here..\n\r", ch);
        return ERROR_MISSING_TARGET;
    }
    if (vict == ch)
    {
	act("$n quietly asks $mself a question.",FALSE,ch,0,0,TO_ROOM);
	send_to_char("You think about it for a while...\n\r", ch);
        return OKAY;
    }
    if(is_overload(ch)||is_silent(ch,TRUE) || !can_talk(ch))
        return ERROR_FAILED;
    sprintf(buf,"$n asks you '%s'",message);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    act("$n asks $N a question.",FALSE,ch,0,vict,TO_NOTVICT);

    if(ch->prefs && IS_SET(ch->prefs->flags,PLR_MSGECHO)) {
        sprintf(buf,"You ask $N '%s'", message);
        act(buf,FALSE,ch,0,vict,TO_CHAR);
    }

    return OKAY;
}



#define MAX_NOTE_LENGTH 1000      /* arbitrary */

/*** THIS NEEDS REWORKING ***/
int do_write(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *paper = 0, *pen = 0;
    char papername[MAX_INPUT_LENGTH], penname[MAX_INPUT_LENGTH],
	buf[MAX_STRING_LENGTH];

    argument_interpreter(argument, papername, penname);

    if (!ch->desc)
	return ERROR_FAILED;

    if (!*papername)  /* nothing was delivered */
    {   
	send_to_char(
	    "Write? with what? ON what? what are you trying to do??\n\r", ch);
	return ERROR_SYNTAX;
    }
    if (*penname) /* there were two arguments */
    {
	if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
	{
	    sprintf(buf, "You have no %s.\n\r", papername);
	    send_to_char(buf, ch);
	    return ERROR_MISSING_TARGET;
	}
	if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))
	{
	    sprintf(buf, "You have no %s.\n\r", papername);
	    send_to_char(buf, ch);
	    return ERROR_MISSING_TARGET;
	}
    }
    else  /* there was one arg.let's see what we can find */
    {			
	if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
	{
	    sprintf(buf, "There is no %s in your inventory.\n\r", papername);
	    send_to_char(buf, ch);
	    return ERROR_MISSING_TARGET;
	}
	if (get_obj_info(paper,ITEM_PEN))  /* oops, a pen.. */
	{
	    pen = paper;
	    paper = 0;
	}
	else if (!(get_obj_info(paper,ITEM_NOTE)))
	{
	    send_to_char("That thing has nothing to do with writing.\n\r", ch);
	    return ERROR_FAILED;
	}

	/* one object was found. Now for the other one. */
	if (!(pen=get_equip_used(ch,HOLD)))
	{
	    sprintf(buf, "You can't write with a %s alone.\n\r", papername);
	    send_to_char(buf, ch);
	    return ERROR_FAILED;
	}
	if (!CAN_SEE_OBJ(ch, pen))
	{
	    send_to_char("The stuff in your hand is invisible! Yeech!!\n\r", ch);
	    return ERROR_FAILED;
	}
	
	if (pen)
	    paper = get_equip_used(ch,HOLD);
	else
	    pen = get_equip_used(ch,HOLD);
    }
	    
    /* ok.. now let's see what kind of stuff we've found */
    if (!(get_obj_info(pen,ITEM_PEN)))
    {
	act("$p is no good for writing with.",FALSE,ch,pen,0,TO_CHAR);
        return ERROR_FAILED;
    }
    else if (!(get_obj_info(paper,ITEM_NOTE)))
    {
	act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
        return ERROR_FAILED;
    }
    else if (paper->action_description) {
	send_to_char("There's something written on it already.\n\r", ch);
        return ERROR_FAILED;
    }

    /* we can write - hooray! */
		
    send_to_char("Ok.. go ahead and write.. end the note with a @.\n\r", ch);
    act("$n begins to jot down a note.", TRUE, ch, 0,0,TO_ROOM);
    ch->desc->str = &paper->action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;

    return OKAY;
}
