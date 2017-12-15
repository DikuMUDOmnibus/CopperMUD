/* ************************************************************************
*  file: act.vio.c,        Implementation of commands.    Part of DIKUMUD *
*  Usage : Fighting  commands.                                            *
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
#include "comm.h"
#include "interp.h"
#include "db.h"
#include "skills.h"
#include "time.h"
#include "error.h"
#include "proto.h"

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;
extern struct command_info commands[];
extern int max_dir[];


void raw_kill(struct char_data *ch);
void lose_exp_by_flight(struct char_data *ch);
bool nokill(struct char_data *ch, struct char_data *victim);

/* Nokill--user can toggle this option so they don't inadvertently
get the flag for killing a player. Swiftest.
*/
	

int do_hit(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    struct char_data *victim;

    one_argument(argument, arg);

    if (*arg) {
	victim = get_char_room_vis(ch, arg);
	if (victim) {
	    if (victim == ch) {
		send_to_char("You hit yourself..OUCH!.\n\r", ch);
		act("$n hits $mself, and says 'OUCH!'", FALSE, ch, 0, victim, TO_ROOM);
	    } else {
/*
		if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
		    act("$N is just such a good friend, you simply can't hit $M.",
		     FALSE, ch,0,victim,TO_CHAR);
		    return;
		}
*/
		if ((GET_POS(ch)>=POS_STAND) &&
		  (victim != ch->specials.fighting)) {
		    hit(ch, victim, TYPE_UNDEFINED);
		} else {
		    send_to_char("You do the best you can!\n\r",ch);
		}
	    }
	} else {
	    send_to_char("They aren't here.\n\r", ch);
            return ERROR_MISSING_TARGET;
	}
    } else {
	send_to_char("Hit whom?\n\r", ch);
        return ERROR_SYNTAX;
    }
    return OKAY;
}


int do_murder(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    struct char_data *victim;

    one_argument(argument, arg);

    if (*arg) {
	victim = get_char_room_vis(ch, arg);
	if (victim) {
	    if (victim == ch) {
		send_to_char("You hit yourself..OUCH!.\n\r", ch);
		act("$n hits $mself, and says OUCH!", FALSE, ch, 0, victim, TO_ROOM);
	    } else {
/*
		if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
		    act("$N is just such a good friend, you simply can't hit $M.",
		     FALSE, ch,0,victim,TO_CHAR);
		    return;
		}
*/

		if ((GET_POS(ch)==POS_STAND) &&
		  (victim != ch->specials.fighting)) {
		    hit(ch, victim, TYPE_UNDEFINED);
		} else {
		    send_to_char("You do the best you can!\n\r",ch);
		}
	    }
	} else {
	    send_to_char("They aren't here.\n\r", ch);
            return ERROR_MISSING_TARGET;
	}
    } else {
	send_to_char("Hit whom?\n\r", ch);
        return ERROR_SYNTAX;
    }
    return OKAY;
}

int do_kill(struct char_data *ch, char *argument, int cmd)
{
/*    char arg[MAX_STRING_LENGTH];
    struct char_data *victim;*/

    return do_hit(ch, argument, 0);
}


int do_backstab(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct obj_data *weapon;
    struct obj_info_weapon *ow;
    struct affected_type af;
    char name[256];
    byte percent;

    one_argument(argument, name);


    if (!(victim = get_char_room_vis(ch, name))) {
	send_to_char("Backstab who?\n\r", ch);
	return ERROR_MISSING_TARGET;
    }
    
    if (victim == ch) {
	send_to_char("How can you sneak up on yourself?\n\r", ch);
	return ERROR_NO_SENSE;
    }

    if(!get_skill(ch,SKILL_BACKSTAB)) {
	send_to_char("You're no thief!\n\r",ch);
	return ERROR_NO_KNOWLEDGE;
    }

    if (!(weapon = get_equip_used(ch,WIELD))) {
	send_to_char("You need to wield a weapon, to make it a succes.\n\r",ch);
	return ERROR_FAILED;
    }

    if (!(ow=(struct obj_info_weapon *)get_obj_info(weapon,ITEM_WEAPON)) ||
            (ow->damage_type != 11)) {
	send_to_char("Only piercing weapons can be used for backstabbing.\n\r",ch);
	return ERROR_FAILED;
    }

    if (victim->specials.fighting) {
	send_to_char("You can't backstab a fighting person, too alert!\n\r", ch);
	return ERROR_FAILED;
    }

    percent=number(1,101); /* 101% is a complete failure */

/*
    if(IS_AFFECTED(victim,AFF_AWARE) && GET_POS(victim)>POS_REST) {
	act("$N seems way too alert to catch off guard.",FALSE,ch,0,victim,TO_CHAR);
	return ERROR_FAILED;
    }*/

    if (AWAKE(victim) && (percent > ch->skills[SKILL_BACKSTAB].learned))
	damage(ch, victim, 0, SKILL_BACKSTAB);
    else {
	hit(ch,victim,SKILL_BACKSTAB);
	/* After getting hit once, this guy will be wary next time */
	af.type=SKILL_AWARENESS;
	af.duration=1;
	af.modifier=0;
	af.location=APPLY_NONE;
	af.bitvector=AFF_AWARE;
	affect_join(victim,&af,FALSE,FALSE);
    }
    return OKAY;
}



int do_order(struct char_data *ch, char *argument, int cmd)
{
    char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int org_room;
    struct char_data *victim;
    struct follow_type *k;

    half_chop(argument, name, message);

    if(GET_EXP(ch) < 1500) {
	send_to_char("Due to abuse, you must have at least 1500 experience to use this command.\n\r",ch);
	return ERROR_FAILED;
    }
    if (!*name || !*message)
	send_to_char("Order who to do what?\n\r", ch);
    else if (!(victim = get_char_room_vis(ch, name)) &&
	str_cmp("follower", name) && str_cmp("followers", name))
	    send_to_char("That person isn't here.\n\r", ch);
    else if (ch == victim)
	send_to_char("You obviously suffer from schitzophrenia.\n\r", ch);

    else {
/*
	if (IS_AFFECTED(ch, AFF_CHARM)) {
	    send_to_char("Your superior would not aprove of you giving orders.\n\r",ch);
	    return;
	}
*/
	if (victim) {
	    sprintf(buf, "$N orders you to '%s'", message);
	    act(buf, FALSE, victim, 0, ch, TO_CHAR);
	    act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);

	    if ( (victim->master!=ch) /*|| !IS_AFFECTED(victim, AFF_CHARM)*/ )
		act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
	    else {
		send_to_char("Ok.\n\r", ch);
		command_interpreter(victim, message);
	    }
	} else {  /* This is order "followers" */
	    sprintf(buf, "$n issues the order '%s'.", message);
	    act(buf, FALSE, ch, 0, victim, TO_ROOM);

	    org_room = ch->in_room;

	    for (k = ch->followers; k; k = k->next) {
/*
		if (org_room == k->follower->in_room)
		    if (IS_AFFECTED(k->follower, AFF_CHARM)) {
			found = TRUE;
			command_interpreter(k->follower, message);
		    }*/
	    }
	    if (!found) {
		send_to_char("Nobody here is a loyal subject of yours!\n\r", ch);
		return ERROR_MISSING_TARGET;
	    }
	}
    }
    return OKAY;
}



int do_flee(struct char_data *ch, char *argument, int cmd)
{
    int i, attempt, start_room, orig_room;
    char buf[MAX_INPUT_LENGTH];
    struct char_data *was_fighting;

    void gain_exp(struct char_data *ch, int gain);
    int special(struct char_data *ch, int cmd, char *arg);

    if (!(ch->specials.fighting)) {
	for(i=0; i<6; i++) {
            /* Select a random direction */
	    attempt = number(0, max_dir[zone_table[world[ch->in_room].zone].dir_system]-1);
	    if (can_go(ch, attempt)) {
		act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
		/*if ((die = do_simple_move(ch, attempt, FALSE))== 1) */
		send_to_char("You attempt to flee...\n\r",ch);
		start_room=ch->in_room;
		strcpy(buf,commands[attempt].command_name);
		command_interpreter(ch,buf);
		if(start_room == ch->in_room) {
		    act("$n tries to flee, but can't make it out of here!", TRUE, ch, 0, 0, TO_ROOM);
		    send_to_char("PANIC! You couldn't escape!\n\r", ch);
		    return ERROR_FAILED;
		}
	    }
	} /* for */
        if(start_room == ch->in_room)
            return ERROR_FAILED;
        else return OKAY;
    }

    orig_room=ch->in_room;
    for(i=0; i<6; i++) {
	/* Select a random direction */
	attempt=number(0,max_dir[zone_table[world[ch->in_room].zone].dir_system]-1);
	if (can_go(ch, attempt)) {
	    act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
	    send_to_char("You attempt to flee...\n\r",ch);
	    start_room=ch->in_room;
	    /* This is dubious, but works */
	    if(ch->specials.fighting) {
		was_fighting=ch->specials.fighting;
		stop_fighting(ch);
	    }
	    strcpy(buf,commands[attempt].command_name);
	    command_interpreter(ch,buf);
	    if(start_room != ch->in_room) {
	   /* The escape has succeded */
		lose_exp_by_flight(ch);


		/* Insert later when using huntig system        */
		/* ch->specials.fighting->specials.hunting = ch */

		return OKAY;
	    } else {
		act("$n tries to flee, but can't make it out of here!", TRUE, ch, 0, 0, TO_ROOM);
		send_to_char("PANIC! You couldn't escape!\n\r", ch);
		if(orig_room==ch->in_room)
		    set_fighting(ch,was_fighting);
		return ERROR_FAILED;
	    }
	}
    } /* for */
    return OKAY;
}



int do_bash(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct char_skill_data *skill;
    char name[256];

    one_argument(argument, name);

    if(!(skill = get_skill(ch,SKILL_BASH))) {
	send_to_char("You better leave all the martial arts to fighters.\n\r", ch);
	return ERROR_NO_KNOWLEDGE;
    }

    if (!(victim = get_char_room_vis(ch, name))) {
	if (ch->specials.fighting &&
		ch->specials.fighting->in_room==ch->in_room) {
	    victim = ch->specials.fighting;
	} else {
	    send_to_char("Bash whom?\n\r", ch);
	    return ERROR_MISSING_TARGET;
	}
    }
    
    if (victim == ch) {
	send_to_char("Aren't we funny today...\n\r", ch);
	return ERROR_NO_SENSE;
    }

    if(world[ch->in_room].sector_type >= SECT_WATER_SWIM) {
	send_to_char("You have no footing here!\n\r",ch);
	return ERROR_LOCATION;
    }

    ch->specials.attack = SKILL_BASH;
    return OKAY;
}



int do_rescue(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim, *tmp_ch;
    struct char_skill_data *skill;
    int percent;
    char victim_name[240];

    one_argument(argument, victim_name);

    if (!(victim = get_char_room_vis(ch, victim_name))) {
	send_to_char("Who do you want to rescue?\n\r", ch);
	return ERROR_MISSING_TARGET;
    }

    if (victim == ch) {
	send_to_char("What about fleeing instead?\n\r", ch);
	return ERROR_NO_SENSE;
    }

    if (ch->specials.fighting == victim) {
	send_to_char("How can you rescue someone you are trying to kill?\n\r",ch);
	return ERROR_NO_SENSE;
    }

    for (tmp_ch=world[ch->in_room].people; tmp_ch &&
      (tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room)  ;

    if (!tmp_ch) {
	act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
	return ERROR_NO_SENSE;
    }

    if(!(skill = get_skill(ch,SKILL_BASH))) {
	send_to_char("But only true warriors can do this!", ch);
	return ERROR_NO_KNOWLEDGE;
    } else {
	percent=number(1,101); /* 101% is a complete failure */
	if (percent > ch->skills[SKILL_RESCUE].learned) {
	    send_to_char("You fail the rescue.\n\r", ch);
	    return ERROR_FAILED;
	}

	send_to_char("Banzai! To the rescue...\n\r", ch);
	act("You are rescued by $N, you are confused!", FALSE, victim, 0, ch, TO_CHAR);
	act("$n heroically rescues $N.", FALSE, ch, 0, victim, TO_NOTVICT);

	if (victim->specials.fighting == tmp_ch)
	    stop_fighting(victim);
	if (tmp_ch->specials.fighting)
	    stop_fighting(tmp_ch);
	if (ch->specials.fighting)
	    stop_fighting(ch);

	set_fighting(ch, tmp_ch);
	set_fighting(tmp_ch, ch);

    }
    return OKAY;
}



int do_kick(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct char_skill_data *skill;
    char name[256];
/*    byte percent;
    int learned;*/

    if(!(skill = get_skill(ch,SKILL_BASH))) {
	send_to_char("You better leave all the martial arts to fighters.\n\r", ch);
	return ERROR_NO_KNOWLEDGE;
    }

    one_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name))) {
	if (ch->specials.fighting &&
		ch->specials.fighting->in_room==ch->in_room) {
	    victim = ch->specials.fighting;
	} else {
	    send_to_char("Kick whom?\n\r", ch);
	    return ERROR_MISSING_TARGET;
	}
    }

    if (victim == ch) {
	send_to_char("Aren't we funny today...\n\r", ch);
	return ERROR_NO_SENSE;
    }

    ch->specials.attack = SKILL_KICK;
    return OKAY;
}
