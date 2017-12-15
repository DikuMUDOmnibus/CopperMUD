/* ************************************************************************
*  file: act.other.c , Implementation of commands.        Part of DIKUMUD *
*  Usage : Other commands.                                                *
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
#include "skills.h"
#include "player.h"
#include "error.h"
#include "proto.h"

#define MAX_TITLE_LEN    45

/* extern variables */

extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];




int do_qui(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("You have to write quit - no less, to quit!\n\r",ch);
    return ERROR_FAILED;
}

int do_quit(struct char_data *ch, char *argument, int cmd)
{
    int do_save(struct char_data *ch, char *argument, int cmd);
    void die(struct char_data *ch);

    if (IS_NPC(ch) || !ch->desc)
	return ERROR_FAILED;

    if (ch->specials.fighting) {
	send_to_char("No way! You are fighting.\n\r", ch);
	return ERROR_FAILED;
    }

    if (GET_POS(ch) < POS_STUNNED) {
	send_to_char("You die before your time!\n\r", ch);
	die(ch);
	return OKAY;
    }

    act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n has left the game.", TRUE, ch,0,0,TO_ROOM);
    save_char(ch);
/*    extract_char_eq(ch);*/
    extract_char(ch);
    return OKAY;
}



int do_save(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];
/*
    if (IS_NPC(ch))
	return ERROR_FAILED;
*/

    sprintf(buf, "Saving %s.\n\r", GET_NAME(ch));
    send_to_char(buf, ch);
    save_char(ch);
    return OKAY;
}



int do_sneak(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    byte percent;

    if(1) {
	send_to_char("You're no thief!\n\r", ch);
	return ERROR_NO_KNOWLEDGE;
    }
    send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);
/*
    if (IS_AFFECTED(ch, AFF_SNEAK))
	affect_from_char(ch, SKILL_SNEAK);*/

    percent=number(1,101); /* 101% is a complete failure */

    if (percent > ch->skills[SKILL_SNEAK].learned +
      dex_app_skill[GET_DEX(ch)].sneak)
	return ERROR_FAILED;

    af.type = SKILL_SNEAK;
    af.duration = 5;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SNEAK;
    affect_to_char(ch, &af);
    return OKAY;
}



int do_hide(struct char_data *ch, char *argument, int cmd)
{
    byte percent;

    if(1) {
	send_to_char("You're no thief!\n\r", ch);
	return ERROR_NO_KNOWLEDGE;
    }
    send_to_char("You attempt to hide yourself.\n\r", ch);
/*
    if (IS_AFFECTED(ch, AFF_HIDE))
	REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);*/

    percent=number(1,101); /* 101% is a complete failure */

    if (percent > ch->skills[SKILL_HIDE].learned +
      dex_app_skill[GET_DEX(ch)].hide)
	return ERROR_FAILED;
/*
    SET_BIT(ch->specials.affected_by, AFF_HIDE);*/
    return OKAY;
}


int do_steal(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct obj_data *obj;
    char victim_name[240];
    char obj_name[240];
    char buf[240];
    int percent;
    bool ohoh = FALSE;

    if(1) {
	send_to_char("You're no thief!\n\r", ch);
	return ERROR_NO_KNOWLEDGE;
    }

    argument = one_argument(argument, obj_name);
    one_argument(argument, victim_name);


    if (!(victim = get_char_room_vis(ch, victim_name))) {
	send_to_char("Steal what from who?\n\r", ch);
	return ERROR_SYNTAX;
    }
    if (victim == ch) {
	send_to_char("Come on now, that's rather stupid!\n\r", ch);
	return ERROR_FAILED;
    }

    if ((GET_EXP(ch) < 1250) && !IS_NPC(victim)) {
     send_to_char("Due to misuse of steal, you can't steal from other players\n\r", ch);
     send_to_char("unless you have at least 1,250 experience points.\n\r", ch);
     return ERROR_FAILED;
    }

    if (IS_SET(world[ch->in_room].room_flags,ARENA)) {
	send_to_char("The Arena Gods frown upon stealing!\n\r",ch);
	return ERROR_FAILED;
    }

    /* 101% is a complete failure */
    percent=number(1,101) - dex_app_skill[GET_DEX(ch)].p_pocket;

    if (GET_POS(victim) < POS_SLEEP)
	percent = -1; /* ALWAYS SUCCESS */

    if (str_cmp(obj_name, "coins") && str_cmp(obj_name,"gold")) {

	obj = get_obj_in_list_vis(victim, obj_name, victim->carrying);
	if(!obj) {
	    act("$E does not have that item.",FALSE,ch,0,victim,TO_CHAR);
	    return ERROR_MISSING_TARGET;
	}

	percent += GET_OBJ_WEIGHT(obj); /* Make heavy harder */

	if((AWAKE(victim)&&(percent >ch->skills[SKILL_STEAL].learned))){
	    ohoh = TRUE;
	    act("Oops..", FALSE, ch,0,0,TO_CHAR);
	    act("$n tried to steal something from you!",FALSE,
		    ch,0,victim,TO_VICT);
	    act("$n tries to steal something from $N.", TRUE,
		    ch, 0, victim, TO_NOTVICT);
	} else { /* Steal the item */
	    if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
		if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
		    if(obj->equipped_as != UNEQUIPPED) {
			unequip_char(victim,obj);
			send_to_char("You unequip it...",ch);
		    }
		    obj_from_char(obj);
		    obj_to_char(obj, ch);
		    send_to_char("Got it!\n\r", ch);
		}
	    } else {
		send_to_char("You cannot carry that much.\n\r", ch);
                return ERROR_FULL;
            }
	}
    } else { /* Steal some coins */
	if ((AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned))) {
	    ohoh = TRUE;
	    act("Oops..", FALSE, ch,0,0,TO_CHAR);
	    act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT);
	    act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT);
	} else {
	    /* Steal some gold coins */
	/*	gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
	    gold = MIN(1782, gold);
	    if (gold > 0) {
		GET_GOLD(ch) += gold;
		GET_GOLD(victim) -= gold;
		sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
		send_to_char(buf, ch);
	    } else {
		send_to_char("You couldn't get any gold...\n\r", ch);
                return ERROR_FAILED;
	    }*/
	}
    }

    if (ohoh && IS_NPC(victim) && AWAKE(victim)) {
	hit(victim, ch, TYPE_UNDEFINED);
    } else if(ohoh && !IS_NPC(victim) && AWAKE(victim)) {
        send_to_char("** You are branded a thief! **\n\r",ch);
        sprintf(buf,"%s is a bloody thief!\n\r", GET_NAME(ch));
        send_to_char(buf,victim);
    }
    return OKAY;
}


#ifdef JKL

int do_pick(struct char_data *ch, char *argument, int cmd)
{
    byte percent;

    if(1) {
	send_to_char("You're no thief!\n\r", ch);
	return;
    }
    percent=number(1,101); /* 101% is a complete failure */

    if (percent > (ch->skills[SKILL_PICK_LOCK].learned +
      dex_app_skill[GET_DEX(ch)].p_locks)) {
	send_to_char("You failed to pick the lock.\n\r", ch);
	return;
    }
}


#endif

int do_idea(struct char_data *ch, char *argument, int cmd)
{
    FILE *fl;
    char str[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
	send_to_char("Monsters can't have ideas - Go away.\n\r", ch);
	return ERROR_FAILED;
    }

    /* skip whites */
    for (; isspace(*argument); argument++);

    if (!*argument)
    {
	send_to_char("That doesn't sound like a good idea to me.. Sorry.\n\r",
	    ch);
	return ERROR_SYNTAX;
    }

    if (!(fl = fopen(IDEA_FILE, "a")))
    {
	perror ("do_idea");
	send_to_char("Could not open the idea file.\n\r", ch);
        log("BUG: Cannot open idea file");
	return ERROR_INTERNAL;
    }

    sprintf(str, "**%s: %s\n", GET_NAME(ch), argument);
    fputs(str, fl);
    fclose(fl);
    send_to_char("Ok. Thanks.\n\r", ch);
    return OKAY;
}







int do_typo(struct char_data *ch, char *argument, int cmd)
{
    FILE *fl;
    char str[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
	send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
	return ERROR_FAILED;
    }

    /* skip whites */
    for (; isspace(*argument); argument++);

    if (!*argument)
    {
	send_to_char("I beg your pardon?\n\r", ch);
	return ERROR_SYNTAX;
    }

    if (!(fl = fopen(TYPO_FILE, "a")))
    {
	perror ("do_typo");
	send_to_char("Could not open the typo file.\n\r", ch);
        log("BUG: Cannot open typo file");
	return ERROR_INTERNAL;
    }

    sprintf(str, "**%s[%d]: %s\n",
	GET_NAME(ch), world[ch->in_room].number, argument);
    fputs(str, fl);
    fclose(fl);
    send_to_char("Ok. thanks.\n\r", ch);
    return OKAY;
}





int do_bug(struct char_data *ch, char *argument, int cmd)
{
    FILE *fl;
    char str[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
	send_to_char("You are a monster! Bug off!\n\r", ch);
	return ERROR_FAILED;
    }

    /* skip whites */
    for (; isspace(*argument); argument++);

    if (!*argument)
    {
	send_to_char("Pardon?\n\r", ch);
	return ERROR_SYNTAX;
    }

    if (!(fl = fopen(BUG_FILE, "a")))
    {
	perror ("do_bug");
	send_to_char("Could not open the bug-file.\n\r", ch);
        log("BUG: Cannot open bug file");
	return ERROR_INTERNAL;
    }

    sprintf(str, "**%s[%d]: %s\n",
	GET_NAME(ch), world[ch->in_room].number, argument);
    fputs(str, fl);
    fclose(fl);
    return OKAY;
}



int do_brief(struct char_data *ch, char *argument, int cmd)
{
    if (!ch->prefs)
	return ERROR_FAILED;

    if (IS_SET(ch->prefs->flags, PLR_BRIEF))
    {
	send_to_char("Brief mode off.\n\r", ch);
	REMOVE_BIT(ch->prefs->flags, PLR_BRIEF);
    }
    else
    {
	send_to_char("Brief mode on.\n\r", ch);
	SET_BIT(ch->prefs->flags, PLR_BRIEF);
    }
    return OKAY;
}


int do_compact(struct char_data *ch, char *argument, int cmd)
{
    if (!ch->prefs)
	return ERROR_FAILED;

    if (IS_SET(ch->prefs->flags, PLR_COMPACT))
    {
	send_to_char("You are now in the uncompacted mode.\n\r", ch);
	REMOVE_BIT(ch->prefs->flags, PLR_COMPACT);
    }
    else
    {
	send_to_char("You are now in compact mode.\n\r", ch);
	SET_BIT(ch->prefs->flags, PLR_COMPACT);
    }
    return OKAY;
}

int do_mode(struct char_data *ch, char *argument, int cmd)
{
    if (!ch->prefs)
	return ERROR_FAILED;

    if (IS_SET(ch->prefs->flags, PLR_STATUS))
    {
	send_to_char("You are no longer in status bar mode.\n\r", ch);
	REMOVE_BIT(ch->prefs->flags, PLR_STATUS);
    }
    else
    {
	send_to_char("You are now in status bar mode.\n\r", ch);
	SET_BIT(ch->prefs->flags, PLR_STATUS);
    }
    return OKAY;
}

int do_msgecho(struct char_data *ch, char *argument, int cmd)
{
    if (!ch->prefs)
	return ERROR_FAILED;

    if (IS_SET(ch->prefs->flags, PLR_MSGECHO))
    {
	send_to_char("Messages will no longer be echoed.\n\r", ch);
	REMOVE_BIT(ch->prefs->flags, PLR_MSGECHO);
    }
    else
    {
	send_to_char("Messages will now be echoed.\n\r", ch);
	SET_BIT(ch->prefs->flags, PLR_MSGECHO);
    }
    return OKAY;
}

int do_quaff(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    struct obj_data *temp;
    struct obj_info_potion *potion;
    bool equipped;

    equipped = FALSE;

    one_argument(argument,buf);

    if (!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
	temp = get_equip_used(ch,HOLD);
	equipped = TRUE;
	if ((temp==0) || !isname(buf, temp->name)) {
	    act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
            return ERROR_MISSING_TARGET;
    	 }
    }

    if(!(potion = (struct obj_info_potion *)get_obj_info(temp,ITEM_POTION))){
	act("You can only quaff potions.",FALSE,ch,0,0,TO_CHAR);
	return ERROR_FAILED;
    }

    act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
    act("You quaff $p which dissolves.",FALSE,ch,temp,0,TO_CHAR);

    if(IS_SET(world[ch->in_room].room_flags,NO_MAGIC)) {
	send_to_char("You feel a slight gathering of magic within you, but it fades.\n\r",ch);
        return ERROR_FAILED;
    }
    if (equipped)
	unequip_char(ch, temp);

    extract_obj(temp);
    return OKAY;
}


int do_recite(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    struct obj_data *scroll, *obj;
    struct obj_info_scroll *scr;
    struct char_data *victim;
    int bits;
    bool equipped;
    void *target;

    equipped = FALSE;
    obj = 0;
    victim = 0;

    argument = one_argument(argument,buf);

    if (!(scroll = get_obj_in_list_vis(ch,buf,ch->carrying))) {
	scroll = get_equip_used(ch,HOLD);
	equipped = TRUE;
        if ((scroll==0) || !isname(buf, scroll->name)) {
	    act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
	    return ERROR_MISSING_TARGET;
 	}
    }

    if(!(scr = (struct obj_info_scroll *)get_obj_info(scroll,ITEM_SCROLL)))
    {
        act("Recite is normally used for scroll's.",FALSE,ch,0,0,TO_CHAR);
        return ERROR_FAILED;
    }

    if (*argument) {
     bits = generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM |
     FIND_CHAR_ROOM, ch, &target);
	if (bits == 0) {
	    send_to_char("No such thing around to recite the scroll on.\n\r", ch);
	    return ERROR_MISSING_TARGET;
	} else if(bits==FIND_CHAR_ROOM)
            victim = (struct char_data *)target;
        else
            obj = (struct obj_data *)target;
    } else {
	victim = ch;
    }

    act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
    if(IS_SET(world[ch->in_room].room_flags,NO_MAGIC)) {
	act("You recite $p, but nothing happens.\n\r",FALSE,ch,scroll,0,TO_CHAR);
	return OKAY;
    }
    act("You recite $p which dissolves.",FALSE,ch,scroll,0,TO_CHAR);

    if (equipped)
	unequip_char(ch, scroll);

    extract_obj(scroll);
    return OKAY;
}


/*****************************************************************/
/* New code : by Dionysos.                                       */
/*****************************************************************/

int do_display(struct char_data *ch, char *argument, int cmd)
{
 char option[256];

/*	if (IS_NPC(ch))
	return;
*/

 one_argument(argument, option);

 if (option[0] != '\0') {
   if (strcmp(option, "hp") == 0) {
    if (ch->specials.dispflags & DISP_HP) {
      send_to_char("Hit point display off.\n\r", ch);
      ch->specials.dispflags &= ~DISP_HP;
    }
    else {
      send_to_char("Hit point display on.\n\r", ch);
      ch->specials.dispflags |= DISP_HP;
    } /* if */
   } 
   else if (strcmp(option, "mana") == 0) {
    if (ch->specials.dispflags) {
      send_to_char("Mana display off.\n\r", ch);
      ch->specials.dispflags &= ~DISP_MA;
    }
    else {
      send_to_char("Mana display on.\n\r", ch);
      ch->specials.dispflags |= DISP_MA;
    } /* if */
   } 
   else if (strcmp(option, "move") == 0) {
    if (ch->specials.dispflags) {
      send_to_char("Movement point display off.\n\r", ch);
      ch->specials.dispflags &= ~DISP_MV;
    }
    else {
      send_to_char("Movement point display on.\n\r", ch);
      ch->specials.dispflags |= DISP_MV;
    } /* if */
   } 
   else if (strcmp(option, "all") == 0) {
    send_to_char("Full display on.\n\r", ch);
    ch->specials.dispflags = DISP_HP|DISP_MV|DISP_MA;
   } 
   else if (strcmp(option, "none") == 0) {
    send_to_char("Display off.\n\r", ch);
    ch->specials.dispflags = NULL;
   } 
   else {
    send_to_char("Display what???\n\r", ch);     
   } /* if */
 }
 else {
   send_to_char("Display usage : display [hp mana move all none].", ch);
 } /* if */

 return OKAY;
} /* do_display */
/*
int do_nokill(struct char_data *ch, char *argument, int cmd)
{
    if(ch->specials.nokill==TRUE){
	ch->specials.nokill=FALSE;
	send_to_char("You may now attack players, you bully!\n\r",ch);
    } else {
	ch->specials.nokill=TRUE;
	send_to_char("You will be prevented from attacking players now, you nice person.\n\r",ch);
    }
}
*/

int do_title(struct char_data *ch, char *argument, int cmd) {

    bool found=FALSE;
	char *look;

  if (IS_NPC(ch))
     return ERROR_FAILED;

  while(*argument==' ')
    argument++;

  if (strlen(argument) > MAX_TITLE_LEN) {
     send_to_char("Title is too long.\n\r", ch);
     return ERROR_FAILED;
  }
    look=argument;
    while(*look) {
	if(*look=='('||*look==')')
	    found=TRUE;
	look++;
    }

    if(found) {
	send_to_char("You can't use parentheses in a title!\n\r",ch);
	return ERROR_FAILED;
    }
    if(zone_table[world[ch->in_room].zone].flags & ZONE_SILENT) {
	send_to_char("You can't seem to right now...\n\r",ch);
	return ERROR_FAILED;
    }
    free(GET_TITLE(ch));
    CREATE(GET_TITLE(ch),char,strlen(argument)+1);
    strcpy(GET_TITLE(ch), argument);
    return OKAY;

}

int do_rub(struct char_data *ch, char *argument, int cmd) 
{
    return ERROR_FAILED;
}
