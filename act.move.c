/* ************************************************************************
*  file: act.movement.c , Implementation of commands      Part of DIKUMUD *
*  Usage : Movement commands, close/open & lock/unlock doors.             *
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
#include "event.h"
#include "bio.h"
#include "skills.h"
#include "error.h"
#include "proto.h"

/*   external vars  */

extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct obj_index_data *obj_index;
extern int rev_dir[];
extern int max_dir[];
extern char *dirs[]; 
extern int movement_loss[];
extern struct command_info commands[];
/* external functs */

int special(struct char_data *ch, int cmd, char *arg);
void death_cry(struct char_data *ch);
struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name,
    struct obj_data *list);

/* I think we'll need a way to determine exit-objects from a direction */

struct obj_info_exit *get_exit(int room,int dir)
{
    struct obj_data *o;
    struct obj_info_exit *exit;

    for(o=world[room].contents;o;o=o->next_content)
        if((exit= (struct obj_info_exit *) get_obj_info(o,ITEM_EXIT)))
            if(exit->dir==dir)
                return exit;
    return NULL;
}

/* This is a routine to check for problems with particular exits which */
/* don't apply room-wide    -Sman */

int leave_by_exit(struct char_data *ch,int cmd)
{
    char firstdir=cmd,found=0;
    struct char_data *k,*in_way=0;
    struct weather_data *cond;
    struct obj_info_exit *exit_info;
    int i;

    if(!(exit_info = get_exit(ch->in_room,cmd))) {
        return ERROR_LOCATION;
    }

    /* First check if we're crowded in a SINGLE_FILE room */
    if(world[ch->in_room].room_flags & SINGLE_FILE) {
	for(i=0;i<max_dir[zone_table[world[ch->in_room].zone].dir_system] &&
                   !found;i++)
	    if(get_exit(ch->in_room,i)) {
		firstdir=i;
		found=1;
	    }
	if(!found) {
	    log("Invalid direction in leave_by_exit");
	    return ERROR_LOCATION;
	}
	if(firstdir==cmd) {
	    if(world[ch->in_room].people!=ch) {
		for(k=world[ch->in_room].people;!in_way && k;
			k=k->next_in_room) {
		    in_way=k;
		    if(k->next_in_room==ch)
			break;
		}
	    }
	} else if(ch->next_in_room) {
	    in_way=ch->next_in_room;
	}
	if(in_way) {
	    act("Oof! It seems that $N is in your way.",FALSE,ch,0,in_way,TO_CHAR);
	    act("Oof! $n runs into you.",FALSE,ch,0,in_way,TO_VICT);
	    act("$n bumps into $N.",TRUE,ch,0,in_way,TO_NOTVICT);
	    return ERROR_PHYS_PREVENTS;
	}
    }

    /* This is for the wind direction, so the wind can */
    /* be too strong to walk against it... */
    if(!(world[ch->in_room].room_flags & INDOORS)) {
	cond=&zone_table[world[ch->in_room].zone].conditions;
	if(cond->wind_dir == cmd && cond->windspeed > 70) {
	    send_to_char("The wind's too strong.\n\r",ch);
	    return ERROR_PHYS_PREVENTS;
	}
    }

    if(world[exit_info->to_room].sector_type == SECT_NO_GROUND) {
	if(cmd==4 && GET_POS(ch) < POS_FLY) {
	    send_to_char("You'd have to fly there!\n\r\n\r",ch);
	    return ERROR_PHYS_PREVENTS;
	}
    }
    return OKAY;
}

/* Needed this for teleports and such */
int can_enter_room(struct char_data *ch,int room, bool show_msg)
{
    bool has_boat;
    struct obj_data *obj;

    if (world[room].sector_type == SECT_WATER_NOSWIM &&
		GET_POS(ch) < POS_LEVITATE) {
	has_boat = FALSE;
	/* See if char is carrying a boat */
	for (obj=ch->carrying; obj; obj=obj->next_content)
	    if (get_obj_info(obj,ITEM_BOAT))
		has_boat = TRUE;
	if (!has_boat) {
	    if(show_msg)
		send_to_char("You need a boat to go there.\n\r\n\r", ch);
	    return ERROR_PHYS_PREVENTS;
	}
    }

    if(world[room].sector_type == SECT_NO_GROUND) {
	if(GET_POS(ch) < POS_LEVITATE) {
	    if(show_msg)
		send_to_char("You'd have to walk on air!",ch);
	    return ERROR_PHYS_PREVENTS;
	}
    }
    /* If we get this far, it's okay to go this way */
    return OKAY;
}

int can_go(struct char_data *ch, int dir)
{
    return NULL;
}

int do_simple_move(struct char_data *ch, int cmd, int following)
/* Assumes, 
    1. That there is no master and no followers.
    2. That the direction exists. 

*/
{
    char tmp[80],buf[MAX_STRING_LENGTH];
    int was_in;
    int need_movement;
    int to_room;
    int retval;
    struct obj_info_exit *exit_info;
    struct bio_attr *ba;

    ba=find_bio_attr(ch->physical->species,BIO_MOBILE);
    if(!ba || !ba->value1) {
        send_to_char("Your body cannot move!\n\r",ch);
        return ERROR_BODY;
    }

    /* The order for this is debateable -Sman */
    retval = leave_by_exit(ch,cmd);
    if (retval != OKAY) {
	return retval;
    }

    exit_info = get_exit(ch->in_room,cmd);

    to_room = exit_info->to_room;

    retval=can_enter_room(ch,to_room,TRUE);
    if(retval != OKAY)
	return retval;

    need_movement = (movement_loss[world[ch->in_room].sector_type]+
    movement_loss[world[to_room].sector_type]) / 2;


    if(GET_MOVE(ch)<need_movement)
    {
	if(!following)
	    send_to_char("You are too exhausted.\n\r",ch);
	else
	    send_to_char("You are too exhausted to follow.\n\r",ch);

	return ERROR_NO_MOVES;
    }

    GET_MOVE(ch) -= need_movement;

    /* You may notice th order of event queueing is odd - this is
       so the moving person doesn't learn of its own movements */

/*    if (!IS_AFFECTED(ch, AFF_SNEAK)) {*/
	sprintf(tmp, "$n leaves %s.", dirs[cmd]);
	act(tmp, TRUE, ch, 0,0,TO_ROOM);
        add_event_room(to_room,EVENT_ARRIVE,0,ch->id,ID_NOBODY);
/*    }*/

    was_in = ch->in_room;

    char_from_room(ch);

    char_to_room(ch, to_room,cmd);

    buf[0]=0;
/*    if (!IS_AFFECTED(ch, AFF_SNEAK)) {*/
	strcat(buf,"$n has arrived.");
	act(buf, TRUE, ch, 0,0, TO_ROOM);
        add_event_room(was_in,EVENT_DEPART,0,ch->id,ID_NOBODY);
/*    }*/

    do_look(ch, "\0",15);

    return OKAY;
}

int do_move(struct char_data *ch, char *argument, int cmd)
{
    char tmp[MAX_INPUT_LENGTH+20];
    int was_in;
    struct follow_type *k, *next_dude;
    struct obj_data *o;
    struct obj_info_exit *exit_info;
    bool found=FALSE;

    if(ch->specials.fighting) {
        send_to_char("You seem to be engaged in something at the moment.\n\r",ch);
        return ERROR_FAILED;
    }

    for(o=world[ch->in_room].contents;o;o=o->next_content)
        if((exit_info= (struct obj_info_exit *) get_obj_info(o,ITEM_EXIT)))
            if(exit_info->dir==cmd) {
                found++; /* Test if more than one way out?? */
                break;
            }
    if (!found) {
	send_to_char("Alas, you cannot go that way...\n\r", ch);
        return ERROR_FAILED;
    }

    if (IS_SET(exit_info->exit_info, EX_CLOSED)) {
        if (o->name) {
	    sprintf(tmp, "The %s seems to be closed.\n\r",
		    fname(o->name));
            send_to_char(tmp, ch);
        } else {
            send_to_char("It seems to be closed.\n\r", ch);
	}
        return ERROR_PHYS_PREVENTS;
    }
    if (exit_info->to_room == NOWHERE) {
        send_to_char("Alas, that way leads nowhere.\n\r", ch);
        return ERROR_FAILED;
    }
    if (!ch->followers && !ch->master)
        return(do_simple_move(ch,cmd,FALSE));
/*
    if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master) && 
	     (ch->in_room == ch->master->in_room)) {
        send_to_char("The thought of leaving your master makes you weep.\n\r",
                ch);
        act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
        return ERROR_MAGIC_PREVENTS;
    }*/

    was_in = ch->in_room;
    if (do_simple_move(ch, cmd, TRUE) == OKAY) { /* Move the character */
        if (ch->followers) { /* If success move followers */
            for(k = ch->followers; k; k = next_dude) {
                next_dude = k->next;
                if ((was_in == k->follower->in_room) &&
			      (GET_POS(k->follower) >= POS_STAND)) {
                    act("You follow $N.", FALSE, k->follower, 0, ch, TO_CHAR);
                    send_to_char("\n\r", k->follower);
                    /*do_move(k->follower, argument, cmd);*/
                    sprintf(tmp,"%s %s",commands[cmd+1].command_name,argument);
                    command_interpreter(k->follower,tmp);
		}
            }
	}
        return OKAY;
    }
    return ERROR_FAILED;
}


struct obj_data *match_door(struct obj_info_exit *door,int from, int to)
{
    struct obj_data *o,*best=NULL;
    struct obj_info_exit *exit;

    for(o=world[to].contents;o;o=o->next) 
        if((exit = (struct obj_info_exit *) get_obj_info(o,ITEM_EXIT))) {
            if(exit->to_room == from) {
                best = o;
                if(exit->dir == rev_dir[door->dir]) /* Optimal answer */
                    return o;
            }
        }
    return best;
}

int do_gen_opening(struct char_data *ch, char *argument, int cmd)
{
    int other_room,ret;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    struct obj_data *obj,*back;
    struct obj_info_exit *exit,*back_e;
    struct obj_info_container *container;

    char *openstr[] = {"open","close","lock","unlock","pick"};

    argument_interpreter(argument, type, dir);

    if (!*type) {
        sprintf(buf,"%s what?\n\r",openstr[cmd-1]);
        (void)CAP(buf);
	send_to_char(buf, ch);
	return ERROR_SYNTAX;
    }

    ret=generic_find(argument, FIND_OBJ_INV|FIND_OBJ_ROOM, ch, (void *)&obj);
    if(!ret) {
        send_to_char("Can't find it.\n\r",ch);
        return ERROR_MISSING_TARGET;
    }

    /*** Containers ***/
    if((container=(struct obj_info_container *)get_obj_info(obj,ITEM_CONTAINER)))
    {
        switch(cmd) {
            case OPENING_OPEN:
                if (!IS_SET(container->lock_state, CONT_CLOSED)) {
                    send_to_char("But it's already open!\n\r", ch);
                    return ERROR_FAILED;
                }
                if (!IS_SET(container->lock_state, CONT_CLOSEABLE)) {
                    send_to_char("You can't do that.\n\r", ch);
                    return ERROR_FAILED;
                }
                if (IS_SET(container->lock_state, CONT_LOCKED)) {
                    send_to_char("It seems to be locked.\n\r", ch);
                    return ERROR_FAILED;
                }

                REMOVE_BIT(container->lock_state, CONT_CLOSED);
                act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM);
                return OKAY;
                break;
            case OPENING_CLOSE:
                if (IS_SET(container->lock_state, CONT_CLOSED)) {
                    send_to_char("But it's already closed!\n\r", ch);
                    return ERROR_FAILED;
                }
                if (!IS_SET(container->lock_state, CONT_CLOSEABLE)) {
                    send_to_char("You can't do that.\n\r", ch);
                    return ERROR_FAILED;
                }

                SET_BIT(container->lock_state, CONT_CLOSED);
                act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM);
                return OKAY;
                break;
            case OPENING_LOCK:
                if (!IS_SET(container->lock_state, CONT_CLOSED)) {
                    send_to_char("Maybe you should close it first...\n\r", ch);
                    return ERROR_FAILED;
                }
                if (container->lock_number < 0) {
                    send_to_char("That thing can't be locked.\n\r", ch);
                    return ERROR_FAILED;
                }
                if (!has_key(ch, container->lock_number)){
                    send_to_char("You don't seem to have the right key.\n\r",ch);	
                    return ERROR_FAILED;
                }
                if (IS_SET(container->lock_state, CONT_LOCKED)) {
                    send_to_char("It is locked already.\n\r", ch);
                    return ERROR_FAILED;
                }
                SET_BIT(container->lock_state, CONT_LOCKED);
                send_to_char("*Click*\n\r",ch);
                act("$n locks $p - 'click', it says.",FALSE,ch,obj,0,TO_ROOM);
                return OKAY;

                break;
            case OPENING_UNLOCK:
                if (container->lock_number < 0) {
                    send_to_char("That can't be locked in the first place.\n\r", ch);
                    return ERROR_FAILED;
                }
                if (!has_key(ch, container->lock_number)){
                    send_to_char("You don't seem to have the right key.\n\r",ch);	
                    return ERROR_FAILED;
                }
                if (!IS_SET(container->lock_state, CONT_LOCKED)) {
                    send_to_char("It is unlocked already.\n\r", ch);
                    return ERROR_FAILED;
                }
                REMOVE_BIT(container->lock_state, CONT_LOCKED);
                send_to_char("*Click*\n\r", ch);
                act("$n unlocks $p - 'click', it says.", FALSE, ch, obj, 0,TO_ROOM);
                return OKAY;

                break;
            case OPENING_PICK:
        }

    /*** exits ***/
    } else if ((exit = (struct obj_info_exit *) get_obj_info(obj, ITEM_EXIT))) {
        switch(cmd) {
            case OPENING_OPEN:
                if (!IS_SET(exit->exit_info, EX_ISDOOR)) {
                    send_to_char("That's impossible, I'm afraid.\n\r", ch);
                    return ERROR_FAILED;
                }
                if (!IS_SET(exit->exit_info, EX_CLOSED)) {
                    send_to_char("It's already open!\n\r", ch);
                    return ERROR_FAILED;
                }
                if (IS_SET(exit->exit_info, EX_LOCKED)) {
                    send_to_char("It seems to be locked.\n\r", ch);
                    return ERROR_FAILED;
                }

                REMOVE_BIT(exit->exit_info, EX_CLOSED);
                if(obj->name)
                    act("$n opens the $F.",FALSE,ch,0,obj->name,TO_ROOM);
                else
                    act("$n opens the door.", FALSE, ch, 0, 0, TO_ROOM);

                /* now for opening the OTHER side of the door! */
                if ((other_room = exit->to_room) != NOWHERE) {
                    if ((back = match_door(exit,ch->in_room,other_room))) {
                        back_e = (struct obj_info_exit *)get_obj_info(back,ITEM_EXIT);
        	        if (back_e->to_room == ch->in_room) {
                             REMOVE_BIT(back_e->exit_info, EX_CLOSED);
                             if (back->name) {
	                         sprintf(buf,"The %s is opened from the other side.\n\r", fname(back->name));
	        		    send_to_room(buf, exit->to_room);
                            } else {
    		        	send_to_room("The door is opened from the other side.\n\r",
    		        	exit->to_room);
		            }
                        }
                    }
	        }
                return OKAY;
                break;
            case OPENING_CLOSE:
                if (!IS_SET(exit->exit_info, EX_ISDOOR)) {
                    send_to_char("That's impossible, I'm afraid.\n\r", ch);
                    return ERROR_FAILED;
                }
                if (IS_SET(exit->exit_info, EX_CLOSED)) {
                    send_to_char("It's already closed!\n\r", ch);
                    return ERROR_FAILED;
                }
    
                SET_BIT(exit->exit_info, EX_CLOSED);
                if(obj->name)
                    act("$n closes the $F.",FALSE,ch,0,obj->name,TO_ROOM);
                else
                    act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM);
    
                /* now for closing the OTHER side of the door! */
                if ((other_room = exit->to_room) != NOWHERE) {
                	if((back = match_door(exit,ch->in_room,other_room))) {
                            back_e = (struct obj_info_exit *)get_obj_info(back,ITEM_EXIT);
        	            if (back_e->to_room == ch->in_room) {
                            SET_BIT(back_e->exit_info, EX_CLOSED);
                            if (back->name) {
	        	            sprintf(buf,"The %s is closed from the other side.\n\r", fname(back->name));
	        		    send_to_room(buf, exit->to_room);
                            } else {
        			send_to_room("The door is closed from the other side.\n\r",
        			exit->to_room);
    		        }
                        }
                    }
	        }
                return OKAY;
                break;
            case OPENING_LOCK:
                if (!IS_SET(exit->exit_info, EX_ISDOOR)) {
                    send_to_char("That's absurd.\n\r", ch);
                    return ERROR_FAILED;
                }
                if (!IS_SET(exit->exit_info, EX_CLOSED)) {
                    send_to_char("You have to close it first, I'm afraid.\n\r", ch);
                    return ERROR_FAILED;
                }
                if (exit->key < 0) {
                    send_to_char("There does not seem to be any keyholes.\n\r", ch);
                    return ERROR_FAILED;
                }
                if (!has_key(ch, exit->key)) {
                    send_to_char("You don't have the proper key.\n\r", ch);
                    return ERROR_FAILED;
                }
                if (IS_SET(exit->exit_info, EX_LOCKED)) {
                    send_to_char("It's already locked!\n\r", ch);
                    return ERROR_FAILED;
                }
                SET_BIT(exit->exit_info, EX_LOCKED);

                if (obj->name)
                    act("$n locks the $F.",0,ch,0,obj->name, TO_ROOM);
                else
                    act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM);

                send_to_char("*Click*\n\r", ch);
                /* now for locking the other side, too */
                if ((other_room = exit->to_room) != NOWHERE)
                	if ((back = match_door(exit,ch->in_room,other_room))){
                            back_e = (struct obj_info_exit *)get_obj_info(back,ITEM_EXIT);
                            if (back_e->to_room == ch->in_room)
                                SET_BIT(back_e->exit_info, EX_LOCKED);
                        }
    
                return OKAY;
                break;
            case OPENING_UNLOCK:
                if (!IS_SET(exit->exit_info, EX_ISDOOR)) {
                    send_to_char("That's absurd.\n\r", ch);
                    return ERROR_FAILED;
                }
                if (!IS_SET(exit->exit_info, EX_CLOSED)) {
                    send_to_char("You have to close it first, I'm afraid.\n\r", ch);
                    return ERROR_FAILED;
                }
                if (exit->key < 0) {
                    send_to_char("There does not seem to be any keyholes.\n\r", ch);
                    return ERROR_FAILED;
                }
                if (!has_key(ch, exit->key)) {
                    send_to_char("You don't have the proper key.\n\r", ch);
                    return ERROR_FAILED;
                }
                if (IS_SET(exit->exit_info, EX_LOCKED)) {
                    send_to_char("It's already locked!\n\r", ch);
                    return ERROR_FAILED;
                }
                SET_BIT(exit->exit_info, EX_LOCKED);
    
                if (obj->name)
                    act("$n locks the $F.",0,ch,0,obj->name, TO_ROOM);
                else
                    act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM);

                send_to_char("*Click*\n\r", ch);
                /* now for locking the other side, too */
                if ((other_room = exit->to_room) != NOWHERE)
                    if((back = match_door(exit,ch->in_room,other_room))) {
                        back_e = (struct obj_info_exit *)get_obj_info(back,ITEM_EXIT);
                        if (back_e->to_room == ch->in_room)
                            SET_BIT(back_e->exit_info, EX_LOCKED);
                    }
    
                return OKAY;
                break;
            case OPENING_PICK:
        }
    }
    /*** huh? ***/
    send_to_char("That's not possible.\n\r",ch);
    return ERROR_FAILED;
}

int has_key(struct char_data *ch, int key)
{
    return 0;
}
#if 0

int do_lock(struct char_data *ch, char *argument, int cmd)
{
    int other_room;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
    struct room_direction_data *door,*back;
    struct obj_data *obj;


    }
    return ERROR_SYNTAX;
}

int do_unlock(struct char_data *ch, char *argument, int cmd)
{
    int other_room;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
    struct room_direction_data *door,*back;
    struct obj_data *obj;



int do_pick(struct char_data *ch, char *argument, int cmd)
{
    byte percent;
    int other_room;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
    struct room_direction_data *door, *back;
    struct obj_data *obj;
    struct obj_info_container *cont;
    void *target;

    if(1 /*GET_CLASS(ch)!=CLASS_THIEF &&*/) {
	send_to_char("You're no thief!\n\r", ch);
	return ERROR_NO_KNOWLEDGE;
    }
    argument_interpreter(argument, type, dir);

    percent=number(1,101); /* 101% is a complete failure */

/*
    if (percent > (ch->skills[SKILL_PICK_LOCK].learned)) {
        send_to_char("You failed to pick the lock.\n\r", ch);
        return ERROR_FAILED;
    }*/

    if (!*type) {
	send_to_char("Pick what?\n\r", ch);
        return ERROR_SYNTAX;
    }
    switch (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
	ch, &target)) {

        case FIND_OBJ_INV:
        case FIND_OBJ_ROOM:

            /* this is an object */
            obj = (struct obj_data *)target;

            if (!(cont=(struct obj_info_container *)
                        get_obj_info(obj,ITEM_CONTAINER))) {
                send_to_char("That's not a container.\n\r", ch);
                return ERROR_FAILED;
            }
            if (!IS_SET(container->lock_state, CONT_CLOSED)) {
                send_to_char("Silly - it ain't even closed!\n\r", ch);
                return ERROR_FAILED;
            }
            if (container->lock_number < 0) {
                send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
                return ERROR_FAILED;
            }
            if (!IS_SET(container->lock_state, CONT_LOCKED)) {
                send_to_char("Oho! This thing is NOT locked!\n\r", ch);
                return ERROR_FAILED;
            }
            if (IS_SET(container->lock_state, CONT_PICKPROOF)) {
                send_to_char("It resists your attempts at picking it.\n\r", ch);
                return ERROR_FAILED;
            }
            REMOVE_BIT(container->lock_state, CONT_LOCKED);
            send_to_char("*Click*\n\r", ch);
            act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
            break;

        case FIND_DOOR: /* NOTE WARNING DANGER YIP DOODLE! not valid pbbbt */

            door = (struct room_direction_data *)target;

            if (!IS_SET(door->exit_info, EX_ISDOOR)) {
                send_to_char("That's absurd.\n\r", ch);
                return ERROR_FAILED;
            }
            if (!IS_SET(door->exit_info, EX_CLOSED)) {
                send_to_char("You realize that the door is already open.\n\r", ch);
                return ERROR_FAILED;
            }
            if (door->key < 0) {
                send_to_char("You can't seem to spot any lock to pick.\n\r", ch);
                return ERROR_FAILED;
            }
            if (!IS_SET(door->exit_info, EX_LOCKED)) {
                send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
                return ERROR_FAILED;
            }
            if (IS_SET(door->exit_info, EX_PICKPROOF)) {
                send_to_char("You seem to be unable to pick this lock.\n\r", ch);
                return ERROR_FAILED;
            }
            REMOVE_BIT(door->exit_info, EX_LOCKED);
            if (obj->name)
                act("$n skillfully picks the lock of the $F.", 0, ch, 0,
            	    obj->name, TO_ROOM);
            else
                act("$n picks the lock.", TRUE, ch, 0, 0, TO_ROOM);
            send_to_char("The lock quickly yields to your skills.\n\r", ch);
            /* now for unlocking the other side, too */
            if ((other_room = door->to_room) != NOWHERE)
            	if (back = match_door(door,other_room))
	            if (back->to_room == ch->in_room)
	                REMOVE_BIT(back->exit_info, EX_LOCKED);
            return OKAY;
            break;
    }
    return ERROR_FAILED;
}

#endif


int do_chgpos(struct char_data *ch, char *argument, int cmd)
{
    /* Sleeping people can't change position (outside of "wake") */
    if(GET_POS(ch)==POS_SLEEP) {
        send_to_char("You'll need to wake up first.\n",ch);
        return ERROR_POSITION;
    }

    /* Let the idiots know who they are */
    if(cmd==GET_POS(ch)) {
        send_to_char("But you're already there.\n",ch);
        return ERROR_ALREADY_DONE;
    }

    /* determine if the character can change to the new position */
    if(!can_have_pos(ch,cmd)) {
        switch(cmd) {
            case POS_REST:
                send_to_char("This body wasn't built for resting.\n",ch);
                break;
            case POS_SIT:
                send_to_char("This body wasn't built for sitting.\n",ch);
                break;
            case POS_STAND:
                send_to_char("This body wasn't built for standing.\n",ch);
                break;
            case POS_LEVITATE:
            case POS_FLY:
            default:
                send_to_char("That would not seem possible.\n",ch);
                break;
        }
        return ERROR_NO_SENSE;
    }

    /* fetch message from... somewhere */
    switch(cmd) {
        case POS_REST:
            send_to_char("You achieve a position of relaxation.\n",ch);
            act("$n sits down and rests.",TRUE,ch,0,0,TO_ROOM);
            break;
        case POS_SIT:
            if(GET_POS(ch)<POS_SIT) {
                send_to_char("You rise to a sitting position.\n",ch);
                act("$n rises to a sitting position.",TRUE,ch,0,0,TO_ROOM);
            } else {
                send_to_char("You sit down.\n",ch);
                act("$n sits down.",TRUE,ch,0,0,TO_ROOM);
            }
            break;
        case POS_STAND:
            send_to_char("You assume your feet.\n",ch);
            act("$n assumes $s feet.",TRUE,ch,0,0,TO_ROOM);
            break;
        case POS_LEVITATE:
            send_to_char("You float motionless above the ground.\n",ch);
            act("$n floats above the ground, motionless.",TRUE,ch,0,0,TO_ROOM);
            break;
        case POS_FLY:
            send_to_char("You lift yourself to the sky.\n",ch);
            act("$n begins to fly around.",TRUE,ch,0,0,TO_ROOM);
            break;
        default:
            break;
    }

    /* now, we attain the position */
    GET_POS(ch)=cmd;

    return OKAY;
}


int do_sleep(struct char_data *ch, char *argument, int cmd)
{
    if(ch->physical->exhaustion>4) {
        send_to_char("You aren't tired enough - the best you can do is just rest.\n",ch);
        return ERROR_PHYS_PREVENTS;
    }

    switch(GET_POS(ch)) {
	case POS_STAND : 
	case POS_SIT  :
	case POS_REST  : {
	    send_to_char("You go to sleep.\n\r", ch);
	    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
	    GET_POS(ch) = POS_SLEEP;
            return OKAY;
	} break;
	case POS_SLEEP : {
	    send_to_char("You are already sound asleep.\n\r", ch);
	} break;
	default : {
	    if(world[ch->in_room].sector_type >=SECT_WATER_SWIM) 
		act("Here? That's not too wise.",FALSE,ch,0,0,TO_CHAR);
	    else {
		act("You stop floating around, and lie down to sleep.",
	    	 FALSE, ch, 0, 0, TO_CHAR);
		act("$n stops floating around, and lie down to sleep.",
		 TRUE, ch, 0, 0, TO_ROOM);
		GET_POS(ch) = POS_SLEEP;
                return OKAY;
	    }
	} break;
    }
    return ERROR_FAILED;
}


int do_wake(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *tmp_char;
    char arg[MAX_STRING_LENGTH];


    one_argument(argument,arg);
    if (*arg) {
	if (GET_POS(ch) == POS_SLEEP) {
	    act("You can't wake people up if you are asleep yourself!",
		FALSE, ch,0,0,TO_CHAR);
            return ERROR_POSITION;
	}
	tmp_char = get_char_room_vis(ch, arg);
	if (tmp_char) {
	    if (tmp_char == ch) {
	        act("If you want to wake yourself up, just type 'wake'",
			FALSE, ch,0,0,TO_CHAR);
                return ERROR_SYNTAX;
	    }
	    if (GET_POS(tmp_char) == POS_SLEEP) {
		act("You wake $M up.", FALSE, ch, 0, tmp_char, TO_CHAR);
/*		if(IS_AFFECTED(tmp_char, AFF_SLEEP)){
		    affect_from_char(tmp_char, SKILL_SLEEP);
		}*/
		GET_POS(tmp_char) = POS_SIT;
		act("You are awakened by $n.", FALSE, ch, 0, tmp_char, TO_VICT);
                return OKAY;
	    } else {
		act("$N is already awake.",FALSE,ch,0,tmp_char, TO_CHAR);
                return ERROR_ALREADY_DONE;
	    }
	} else {
	    send_to_char("You do not see that person here.\n\r", ch);
            return ERROR_MISSING_TARGET;
	}
    }
    if (/*IS_AFFECTED(ch,AFF_SLEEP) ||*/ ch->physical->exhaustion==0) {
        send_to_char("You're too tired!\n\r", ch);
        return ERROR_FAILED;
    }
    if (GET_POS(ch) > POS_SLEEP) {
	send_to_char("You are already awake...\n\r", ch);
        return ERROR_ALREADY_DONE;
    }
    send_to_char("You wake, and sit up.\n\r", ch);
    act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SIT;
    return OKAY;
}


int do_follow(struct char_data *ch, char *argument, int cmd)
{
    char name[160];
    struct char_data *leader;

    void stop_follower(struct char_data *ch);
    void add_follower(struct char_data *ch, struct char_data *leader);


    one_argument(argument, name);

    if (*name) {
	if (!(leader = get_char_room_vis(ch, name))) {
	    send_to_char("I see no person by that name here!\n\r", ch);
	    return ERROR_MISSING_TARGET;
	}
    } else {
	send_to_char("Who do you wish to follow?\n\r", ch);
	return ERROR_SYNTAX;
    }
/*
    if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master)) {

	act("But you only feel like following $N!",
	 FALSE, ch, 0, ch->master, TO_CHAR);
        return ERROR_MAGIC_PREVENTS;

    }*/
    /* Not Charmed follow person */

    if (leader == ch) {
        if (!ch->master) {
            send_to_char("You are already following yourself.\n\r", ch);
	    return ERROR_FAILED;
	}
	stop_follower(ch);
        return OKAY;
    }
    if (circle_follow(ch, leader)) {
	act("Sorry, but following in 'loops' is not allowed", FALSE, ch, 0, 0, TO_CHAR);
	return ERROR_FAILED;
    }
    if (ch->master)
	stop_follower(ch);

    add_follower(ch, leader);
    return OKAY;
}
