/* ************************************************************************
*  file: pulse.c , Things that happen regularly           Part of DIKUMUD *
*  Usage: Procedures controling gain and limit.          (orig. limits.c) *
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
#include "skills.h"
#include "comm.h"
#include "db.h"
#include "event.h"
#include "error.h"
#include "proto.h"

extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct obj_data *object_list;
extern struct room_data *world;
extern struct zone_data *zone_table;
extern char *spell_wear_off_msg[];

/* External procedures */

void update_pos( struct char_data *victim );                 /* in fight.c */
void damage(struct char_data *ch, struct char_data *victim,  /*    do      */
      int damage, int weapontype);
struct time_info_data age(struct char_data *ch);



/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

    if (age < 15)
	return(p0);                               /* < 15   */
    else if (age <= 29) 
	return (int) (p1+(((age-15)*(p2-p1))/15));  /* 15..29 */
    else if (age <= 44)
	return (int) (p2+(((age-30)*(p3-p2))/15));  /* 30..44 */
    else if (age <= 59)
	return (int) (p3+(((age-45)*(p4-p3))/15));  /* 45..59 */
    else if (age <= 79)
	return (int) (p4+(((age-60)*(p5-p4))/20));  /* 60..79 */
    else
	return(p6);                               /* >= 80 */
}


/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */
int mana_limit(struct char_data *ch)
{
    int max;

    max = (100);  /* + (graf(age(ch).year, 0,0,10,30,50,70,60)); */
    max = MAX(ch->physical->max_mana, max);
    return(max);
}


int hit_limit(struct char_data *ch)
{
    int max;

    max = (ch->physical->max_hit) +
	   (graf(age(ch).year, 2,4,17,14,8,4,3));


    /* Class/Level calculations */

    /* Skill/Spell calculations */
    
    return (max);
}


int move_limit(struct char_data *ch)
{
    int max;

    /* HERE SHOULD BE CON CALCULATIONS INSTEAD */
    max = graf(age(ch).year, 50,70,160,120,100,40,20);
    max = MAX(ch->physical->max_move, max);

    /* Class/Level calculations */

    /* Skill/Spell calculations */

    return (max);
}




/* manapoint gain pr. game hour */
int mana_gain(struct char_data *ch)
{
    int gain;

    gain = graf(age(ch).year, 2,4,6,8,6,5,8);

    /* Local energy calculation */
    gain = (zone_table[world[ch->in_room].zone].conditions.free_energy*gain)/10000;

    /* Class calculations */

    /* Skill/Spell calculations */

    /* Position calculations    */
    switch (GET_POS(ch)) {
	case POS_SLEEP:
	    gain += gain;
	    break;
	case POS_REST:
	    gain+= (gain>>1);  /* Divide by 2 */
	    break;
	case POS_SIT:
	    gain += (gain>>2); /* Divide by 4 */
	    break;
    }

    gain += gain;
/*
    if (IS_AFFECTED(ch,AFF_POISON))
	gain >>= 2;*/

    if(!ch->physical->hunger || ch->physical->thirst)
	gain >>= 2;

    if(zone_table[world[ch->in_room].zone].conditions.free_energy <= 0)
	gain=0;
    else if(gain <= 1)
	gain=2;

    /* Room mana gains should be absolute */
    if((world[ch->in_room].mana_alignment==MANA_ALL_ALIGNS) ||
	(world[ch->in_room].mana_alignment==MANA_GOOD &&
	    IS_GOOD(ch)) ||
	(world[ch->in_room].mana_alignment==MANA_NEUTRAL &&
	    IS_NEUTRAL(ch)) ||
	(world[ch->in_room].mana_alignment==MANA_EVIL &&
	    IS_EVIL(ch)))
	gain += world[ch->in_room].mana;

    if(GET_MANA(ch)+gain > mana_limit(ch))
	gain=mana_limit(ch)-GET_MANA(ch);

    if(GET_MANA(ch)+gain < 0)
	gain = -GET_MANA(ch);

    zone_table[world[ch->in_room].zone].conditions.free_energy -= gain;

 return (gain);
}


int hit_gain(struct char_data *ch)
/* Hitpoint gain pr. game hour */
{
    int gain,temp_dam;

    gain = graf(age(ch).year, 2,5,10,18,6,4,2);

    /* Class/Level calculations */

    /* Skill/Spell calculations */

    /* Position calculations    */

    switch (GET_POS(ch)) {
	case POS_SLEEP:
	    gain += (gain>>1); /* Divide by 2 */
	    break;
	case POS_REST:
	    gain+= (gain>>2);  /* Divide by 4 */
	    break;
	case POS_SIT:
	    gain += (gain>>3); /* Divide by 8 */
	    break;
    }

/*    gain >>= 1;*/
/*
    if (IS_AFFECTED(ch,AFF_POISON)) {
	gain >>= 2;
        damage(ch,ch,2,SKILL_POISON);
    }*/

    /* Weather affects hit points */
    if(!(world[ch->in_room].room_flags & INDOORS)) {
	temp_dam=(ch->specials.warmth/10 +
	    zone_table[world[ch->in_room].zone].conditions.temp);
	if(IS_NPC(ch)) /* NPC's shouldn't be so affected */
	    if(temp_dam > 25)
		temp_dam -= 15;
	    else
		temp_dam += 15;

	if(temp_dam <-3) {
            add_event_char(ch,EVENT_2COLD,0,ID_NOBODY,ID_NOBODY);
/*	    damage(ch,ch,-temp_dam/4,DAMAGE_FROSTBITE);*/
	} else if(temp_dam > 50) {
            add_event_char(ch,EVENT_2HOT,0,ID_NOBODY,ID_NOBODY);
/*	    damage(ch,ch,(temp_dam-50)/4+1,DAMAGE_HEATSTROKE);*/
        }
    }

    if(!ch->physical->hunger || !ch->physical->thirst)
	gain >>= 2;


    return (gain);
}



int move_gain(struct char_data *ch)
/* move gain pr. game hour */
{
    int gain;

    gain = graf(age(ch).year, 12,18,22,21,14,10,6);

    /* Class/Level calculations */

    /* Skill/Spell calculations */

    /* Position calculations    */
    switch (GET_POS(ch)) {
	case POS_SLEEP:
	    gain += gain+2;
	    break;
	case POS_REST:
	    gain+= (gain>>1)+1;
	    break;
	case POS_SIT:
	    gain += (gain>>2);
	    break;
    }
/*
    if (IS_AFFECTED(ch,AFF_POISON))
	gain >>= 2;*/

    if(!ch->physical->hunger || ch->physical->thirst)
	gain >>= 2;

    return (gain);
}



/* Gain maximum in various points */
void advance_level(struct char_data *ch)
{
    int add_hp;

    extern struct con_app_type con_app[];

    
    add_hp = con_app[GET_CON(ch)].hitp;
/*
    switch(GET_CLASS(ch)) {

	case CLASS_MAGIC_USER : {
	    add_hp += number(3, 8);
	    add_mana = number(GET_LEVEL(ch),(int)(1.5*GET_LEVEL(ch))); 
	    add_mana = MIN(add_mana, 10);
	} break;

	case CLASS_CLERIC : {
	    add_hp += number(5, 10);
	    add_mana = number(GET_LEVEL(ch),(int)(1.5*GET_LEVEL(ch)));
	    add_mana = MIN(add_mana, 10);
	} break;

	case CLASS_THIEF : {
	    add_hp += number(7,13);
	    add_mana = 0;
	} break;

	case CLASS_WARRIOR : {
	    add_hp += number(10,15);
	    add_mana = 0;
	} break;
    }

    ch->physical->max_hit += MAX(1, add_hp);
    if ((GET_LEVEL(ch) != 1) && (ch->physical->max_mana < 160)) {
 	   ch->physical->max_mana = ch->physical->max_mana + (sh_int)add_mana;
 	} * if *

    if (GET_CLASS(ch) == CLASS_MAGIC_USER || GET_CLASS(ch) == CLASS_CLERIC)
	ch->specials.spells_to_learn += MAX(2, wis_app[GET_WIS(ch)].bonus);
    else
	ch->specials.spells_to_learn += MIN(2,MAX(1, wis_app[GET_WIS(ch)].bonus));
*/
}


void set_title(struct char_data *ch)
{
    char *def_title="the personage";
/*	if (GET_TITLE(ch))
	RECREATE(GET_TITLE(ch),char,strlen(READ_TITLE(ch))+1);
    else
	CREATE(GET_TITLE(ch),char,strlen(READ_TITLE(ch)));
*/
    if (GET_TITLE(ch))
	RECREATE(GET_TITLE(ch),char,strlen(def_title)+1);
    else
	CREATE(GET_TITLE(ch),char,strlen(def_title));
    strcpy(GET_TITLE(ch), def_title);
}



void gain_exp(struct char_data *ch, int gain)
{
    if (gain > 0 && GET_EXP(ch)<1000000) {

	GET_EXP(ch) += gain;
    }

    if (gain < 0) {
	gain = MAX(-500000, gain);  /* Never loose more than 1/2 mil */
	GET_EXP(ch) += gain;
	if (GET_EXP(ch) < 0)
	    GET_EXP(ch) = 0;
    }
}


void gain_exp_regardless(struct char_data *ch, int gain)
{

    if (gain > 0) {
	GET_EXP(ch) += gain;
    }
    if (gain < 0) 
	GET_EXP(ch) += gain;
    if (GET_EXP(ch) < 0)
	GET_EXP(ch) = 0;
}

void gain_condition(struct char_data *ch,int condition,int value)
{
    bool intoxicated;
    int newval;

    switch(condition) {
        case DRUNK:
		newval = ch->physical->impairment;
                break;
        case FULL:
		newval = ch->physical->hunger;
                break;
        case THIRST:
		newval = ch->physical->thirst;
                break;
        case SLEEP:
		newval = ch->physical->exhaustion;
                break;
        default:
                log("BUG: Bad call to gain_condition");
		return;
    }


    if(newval==-1) /* No change */
	return;

    intoxicated=(ch->physical->impairment > 0);

    newval  += value;

    newval= MAX(0,newval);
    newval= MIN(24,newval);

    switch(condition){
	case FULL :
	    if(!(ch->physical->hunger=newval)) {
	        send_to_char("You are hungry.\n\r",ch);
                add_event_char(ch,EVENT_HUNGER,0,ID_NOBODY,ID_NOBODY);
            }
            break;
	case THIRST :
	    if(!(ch->physical->thirst=newval)) {
	        send_to_char("You are thirsty.\n\r",ch);
                add_event_char(ch,EVENT_THIRST,0,ID_NOBODY,ID_NOBODY);
            }
            break;
	case DRUNK :
	    if(!(ch->physical->impairment=newval) &&(intoxicated))
	        send_to_char("You are now sober.\n\r",ch);
            break;
        case SLEEP :
	    if(!(ch->physical->exhaustion=newval) && GET_POS(ch)!=POS_SLEEP) {
                send_to_char("You collapse from exhaustion!\n",ch);
                act("$n collapses from exhaustion!",TRUE,ch,0,0,TO_ROOM);
                GET_POS(ch)=POS_SLEEP;
	    } else if(ch->physical->exhaustion==1)
                send_to_char("You are absolutely exhausted.\n",ch);
            else if(ch->physical->exhaustion<4) {
                send_to_char("You are getting sleepy.\n",ch);
                /* we should be more exact about this event */
                add_event_char(ch,EVENT_SLEEPY,0,ID_NOBODY,ID_NOBODY);
            } else if(ch->physical->exhaustion>8 && GET_POS(ch)==POS_SLEEP) {
                send_to_char("You wake up naturally.\n",ch);
                act("$n blinks $s eyes and wakes up.",FALSE,ch,0,0,TO_ROOM);
                GET_POS(ch)=POS_REST;
                add_event_char(ch,EVENT_WAKE,0,ch->id,ID_NOBODY);
            }
            break;
	default :
            break;
    }

}


void check_idling()
{
    struct descriptor_data *d;
    struct char_data *ch;

    for(d=descriptor_list;d;d=d->next) {
        if (++d->idle > 32)
        {
            ch = d->character;
            close_socket(d);

            if(ch) {
                save_char(ch);
                extract_char_eq(ch);
                extract_char(ch);
            }
        } else if(d->idle > 25 && d->character) {
            switch(number(0,4)) {
                case 0:
                    send_to_char("Wake up, bub!\n\r",d->character);
                    break;
                case 1:
                    send_to_char("No loitering!\n\r",d->character);
                    break;
                case 2:
                    send_to_char("You feel idle.\n\r",d->character);
                    break;
                case 3:
                    send_to_char("Anybody home?\n\r",d->character);
                    break;
                case 4:
                    send_to_char("You are idle, miscreant!\n\r",d->character);
                    break;
            }
        } else if(d->idle > 10 && d->connected!=CON_PLYNG) {
            close_socket(d);
        } else if(d->idle > 2 && d->connected==CON_NME) {
            close_socket(d);
        } else if(d->idle > 4 && d->connected==CON_PLYNG && d->character) {
            if(number(0,20) > GET_LUCK(d->character)) {
                /* Play with them a bit... */
                /*(make encounter...)*/
            }
        }
    }
}





/* Update both PC's & NPC's and objects*/
void point_update( void )
{	
    struct char_data *i, *next_dude;
    struct obj_data *j, *next_thing/*, *jj, *next_thing2*/;
    struct obj_info *oi,*next_oi;
    struct obj_info_light *light;
    struct obj_info_container *cont;

    /* characters */
    for (i = character_list; i; i = next_dude) {
	next_dude = i->next;
	if (GET_POS(i) > POS_STUNNED) {
	    GET_HIT(i)  = MIN(GET_HIT(i)  + hit_gain(i),  hit_limit(i));
	    GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
	    GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
	} else if (GET_POS(i) == POS_STUNNED) {
	    GET_HIT(i)  = MIN(GET_HIT(i)  + hit_gain(i),  hit_limit(i));
	    GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
	    GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
	    update_pos( i );
	}/* else if	(GET_POS(i) == POS_INCAP)
	    damage(i, i, 1, TYPE_SUFFERING);
	else if (GET_POS(i) == POS_MORTALLYW)
	    damage(i, i, 2, TYPE_SUFFERING);*/
	gain_condition(i,FULL,-1);
	gain_condition(i,DRUNK,-1);
	gain_condition(i,THIRST,-1);
        if(GET_POS(i) == POS_SLEEP)
            gain_condition(i,SLEEP,4);
        else if(GET_POS(i) == POS_REST)
            gain_condition(i,SLEEP,1);
        else
	    gain_condition(i,SLEEP,-1);
    } /* for */

    /* objects */
    for(j = object_list; j ; j = next_thing) {
	next_thing = j->next; /* Next in object list */

        for(oi=j->info;oi && j;oi=next_oi) {
            next_oi = oi->next;
            switch(oi->obj_type) {
                case ITEM_CONTAINER:
                cont=(struct obj_info_container *)oi;
            	/* If this is a corpse */
            	if(cont->lock_state & CONT_CORPSE) {
            	    /* timer count down */
#if 0
                    < how to do this?? >
            	    if(j->obj_flags.timer > 0)
            		j->obj_flags.timer--;

            	    if (!j->obj_flags.timer) {
            
            		if (j->carried_by)
            		    act("$p decay in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
            		else if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
            		    act("A quivering hoard of maggots consume $p.", TRUE, world[j->in_room].people, j, 0, TO_ROOM);
            		    act("A quivering hoard of maggots consume $p.", TRUE, world[j->in_room].people, j, 0, TO_CHAR);
            		}
            
            		for(jj = j->contains; jj; jj = next_thing2) {
            		    next_thing2 = jj->next_content; /* Next in inventory */
            		    obj_from_obj(jj);
            
            		    if (j->in_obj)
            			obj_to_obj(jj,j->in_obj);
            		    else if (j->carried_by)
            			obj_to_room(jj,j->carried_by->in_room);
            		    else if (j->in_room != NOWHERE)
            			obj_to_room(jj,j->in_room);
            		}
            		extract_obj(j);
            		j=NULL; /* so I don't need to use a goto... */
            	    }
#endif
            	}
            	break;
                
                case ITEM_LIGHT:
                light=(struct obj_info_light *)oi;
            	if(light->brightness>0) {
            	    if(light->hours_left>0) {
            		if(!--light->hours_left) {
            		    off_light(j);
            		}
            	    }
            	}
            	break;
            }
        }
    }
}

void affect_update( void )
{
    static struct affected_type *af, *next_af_dude;
    static struct char_data *i;

    for (i = character_list; i; i = i->next) {
        for (af = i->affected; af; af = next_af_dude) {
            next_af_dude = af->next;
            if (af->duration >= 1)
                af->duration--;
            else if (af->duration == -1)
                /* No action */
                af->duration = -1;  /* GODs only! unlimited */
            else {
                if ((af->type > 0) && (af->type <= 52)) /* It must be a spell */
                    if (!af->next || (af->next->type != af->type) ||
                      (af->next->duration > 0))
                        if (*spell_wear_off_msg[af->type]) {
                            send_to_char(spell_wear_off_msg[af->type], i);
                            send_to_char("\n\r", i);
                        }

                affect_remove(i, af);
            }
        }
/*
        if((GET_POS(i)==POS_FLY && !IS_AFFECTED(i,AFF_FLY))
                || (GET_POS(i)==POS_LEVITATE &&
                !IS_AFFECTED(i,AFF_FLY) &&
                !IS_AFFECTED(i,AFF_LEVITATE))) {

            GET_POS(i)=POS_STAND;
            if(world[i->in_room].sector_type==SECT_NO_GROUND) {
                check_fall(i);
            }
        }*/
    }
}

void char_pulse(int pulse)
{
    struct char_data *i,*next,*prev=NULL;
    struct obj_data *o;

    for(i=character_list;i;i=next) {
        next = i->next;

        /* This updates values for everybody in the character list which */
        /* control their ability to "speak" without overloading the system */
        if(i->specials.ovl_count > 0) {
            if(++(i->specials.ovl_timer) > OVL_PULSE) {
                i->specials.ovl_timer = 0;
                i->specials.ovl_count -= OVL_LIMIT;
            }
        }

        check_fall(i);

        if(i->specials.act & ACT_CLEANUP) {
            while(i->carrying) {
                o=i->carrying;
                if(o->equipped_as != UNEQUIPPED)
                    unequip_char(i,o);
                extract_obj(o);
            }
            char_from_room(i); /* Fetch from the void */
            if(!prev)
                character_list = i->next;
            else
                prev->next = i->next;
            free_char(i);
            continue;
        } else {
            prev=i;
        }
    }
}
