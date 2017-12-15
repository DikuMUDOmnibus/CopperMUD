/* ************************************************************************
*  file: phys.c, Physical routines                        Part of DIKUMUD *
*  Usage : Various physical checks/changes                                *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */



#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interp.h"
#include "db.h"
#include "bio.h"
#include "error.h"
#include "proto.h"


/* Global data */

extern struct room_data *world;
extern char *spell_wear_off_msg[];

void check_fall(struct char_data *ch);

/* Extern procedures */

char *strdup(char *str);

/* Extern procedures */


void check_fall(struct char_data *ch)
{
    int dam=0;
    int to_room;
    struct obj_info_exit *exit;

    /* Now handle the possibility of falling */

    while(world[ch->in_room].sector_type==SECT_NO_GROUND &&
	    GET_POS(ch) <POS_LEVITATE) {

	act("$n falls downward.",TRUE,ch,0,0,TO_ROOM);
	if(GET_POS(ch)>POS_SLEEP)
	    act("You fall, shapes and sounds shredding past you.",FALSE,ch,0,0,TO_CHAR);

        exit = get_exit(ch->in_room,5);
	if(exit && (exit->to_room!=ch->in_room)) {
	    to_room=exit->to_room;
	    char_from_room(ch);
	    char_to_room(ch,to_room,5);
	    act("$n falls here from above.",TRUE,ch,0,0,TO_ROOM);
	    if(dam)
		dam = dam << 1;
	    else
		dam = 10;
	} else {
	    /* Death from falling */
	    act("You do not survive the fall...", FALSE,ch,0,0,TO_CHAR);
	    death_cry(ch);
	    char_from_room(ch);
	    char_to_room(ch,0,5);
	    extract_char(ch);
	    dam=0;
	    break; /* get out of the while loop */
	}
    }

    if(dam) {
	damage(ch,ch,dam,DAM_FALLING);
	GET_POS(ch)=POS_SIT;
    }
}



void clone_obj(struct obj_data *obj)
{
    struct obj_data *clone;


    CREATE(clone, struct obj_data, 1);

    *clone = *obj;

    clone->name               = strdup(obj->name);
    clone->description        = strdup(obj->description);
    clone->short_description  = strdup(obj->short_description);
    clone->action_description = strdup(obj->action_description);
    clone->ex_description     = 0;

    /* REMEMBER EXTRA DESCRIPTIONS */
    clone->carried_by         = 0;
    clone->in_obj             = 0;
    clone->contains           = 0;
    clone->next_content       = 0;
    clone->next               = 0;

    /* VIRKER IKKE ENDNU */
}



/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data *ch, struct char_data *victim)
{
    struct char_data *k;

    for(k=victim; k; k=k->master) {
	if (k == ch)
	    return(TRUE);
    }

    return(FALSE);
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data *ch)
{
    struct follow_type *j, *k;

    if(!ch->master) {
        log("BUG: No master in stop_follower");
        return;
    }
#if 0
    if (IS_AFFECTED(ch, AFF_CHARM)) {
	act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
	act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
	act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
	if (affected_by_spell(ch, SPELL_CHARM_PERSON))
	    affect_from_char(ch, SPELL_CHARM_PERSON);
    } else {
	act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
	act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
	act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
    }
#endif

    if (ch->master->followers->follower == ch) { /* Head of follower-list? */
	k = ch->master->followers;
	ch->master->followers = k->next;
	free(k);
    } else { /* locate follower who is not head of list */
	for(k = ch->master->followers; k->next->follower!=ch; k=k->next)  ;

	j = k->next;
	k->next = j->next;
	free(j);
    }

    ch->master = 0;
/*    REMOVE_BIT(ch->specials.affected_by, AFF_CHARM);*/
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
    struct follow_type *j, *k;

    if (ch->master)
	stop_follower(ch);

    for (k=ch->followers; k; k=j) {
	j = k->next;
	stop_follower(k->follower);
    }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data *ch, struct char_data *leader)
{
    struct follow_type *k;

    if(ch->master) {
        log("BUG: Master in add_follower");
        return;
    }

    ch->master = leader;

    CREATE(k, struct follow_type, 1);

    k->follower = ch;
    k->next = leader->followers;
    leader->followers = k;

    act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
    act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
    act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}

void off_light(struct obj_data *obj)
{
    struct obj_info_light *light;

    if(!(light=(struct obj_info_light *)get_obj_info(obj,ITEM_LIGHT))) {
        log("BUG: non-light passed to off_light()");
        return;
    }
    light->brightness=0;
    if(obj->carried_by) {
        obj->carried_by->specials.lights_carried--;
        world[obj->carried_by->in_room].light--;
    } else if(obj->in_room != NOWHERE)
        world[obj->in_room].light--;
}

void on_light(struct obj_data *obj)
{
    struct obj_info_light *light;

    if(!(light=(struct obj_info_light *)get_obj_info(obj,ITEM_LIGHT))) {
        log("BUG: non-light passed to off_light()");
        return;
    }

    light->brightness=1;
    if(obj->carried_by) {
        obj->carried_by->specials.lights_carried++;
        world[obj->carried_by->in_room].light++;
    } else if(obj->in_room != NOWHERE)
        world[obj->in_room].light++;
}

bool can_have_pos(struct char_data *ch,int pos)
{
    /* Naturally, this will be a bit more complicated later */
    return TRUE;
}

int can_wear(struct obj_data *obj,int bit)
{
    struct obj_info_wear *ow;

    if(!(ow=(struct obj_info_wear *)get_obj_info(obj,ITEM_WORN)))
        return 0;

    return(ow->wear_flags & bit);
}

/** These may be subsumed by new functions in force.c sometime **/

void teleport_to(struct char_data *ch,int to_room)
{

    act("$n slowly fades out of existence.", FALSE,ch,0,0,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, to_room,0);
    act("$n slowly fades into existence.", FALSE,ch,0,0,TO_ROOM);

    do_look(ch, "", 0);
}

/* A routine for ITEM_TELEPORT objects */

bool check_item_teleport(struct char_data *ch,char *arg,int cmd)
{
    struct obj_data *obj=NULL;
    int bits;
    struct obj_info_teleport *tele;

    bits=generic_find(arg,FIND_OBJ_INV|FIND_OBJ_ROOM,ch,(void **)&obj);
    if(!obj)
	return FALSE;

    if(!(tele = (struct obj_info_teleport *)get_obj_info(obj,ITEM_TELEPORT)))
	return FALSE;

    if(cmd!=tele->trigger) /* will be more complicated later */
	return FALSE;

    if(IS_SET(world[ch->in_room].room_flags,NO_MAGIC)) {
	send_to_char("That doesn't seem to do anything!\n\r",ch);
	return FALSE;
    }

    /* Ignore argument for now...eventually we could "throw dust Dbra"..*/
    if(!tele->charges || (IS_SET(world[ch->in_room].room_flags,ARENA)!=IS_SET(world[real_room(tele->to_location)].room_flags,ARENA))) {
	send_to_char("Nothing happens.\n\r\n\r",ch);
	return FALSE;
    }
    act("$p in $n's hands suddenly glows brightly!",FALSE,ch,obj,0,TO_ROOM);
    act("$p in your hands suddenly glows brightly!",FALSE,ch,obj,0,TO_CHAR);
    teleport_to(ch,real_room(tele->to_location));
    if(tele->charges>0) {
	if(!--tele->charges) {
	    act("$p in $n's hands shatters and the pieces disappear in smoke.",TRUE,ch,obj,0,TO_ROOM);
	    act("$p in your hands shatters and the pieces disappear in smoke.",TRUE,ch,obj,0,TO_CHAR);
	    extract_obj(obj);
	}
    }
    return TRUE;
}

