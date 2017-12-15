
/* ************************************************************************
*  File: vio.c , Combat module.                           Part of DIKUMUD *
*  Usage: Violence system and messages.                                   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/* Thoughts on combat:

damage() should only be called from a minimum of places, directly off
the main loop if possible. also, damage() shouldn't need to handle
0 (non-) damage, nor messages for that matter.

perhaps an action_message(ch,vict,type) for combat messages, and maybe
even socials, if they can be combined

hitting to subdue should be handled in hit() somewhere

*/



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
#include "db.h"
#include "player.h"
#include "skills.h"
#include "event.h"
#include "error.h"
#include "proto.h"

/* Structures */

struct char_data *combat_list = 0;      /* head of l-list of fighting chars*/
struct char_data *combat_next_dude = 0; /* Next dude global trick           */

/* External structures */

extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct obj_data  *object_list;
extern struct obj_index_data *obj_index;
extern struct str_app_type str_app[];

/* External procedures */

char *fread_string(FILE *f1);
void stop_follower(struct char_data *ch);
void forget(char *name, struct char_data *ch);
void remember(char *name, struct char_data *ch);

void death_stat_handler(struct char_data *ch);
int to_hit_char(struct char_data *ch,struct char_data *victim,int adj);

struct msg_trio {
    char *msg_attacker;
    char *msg_victim;
    char *msg_room;
    struct msg_trio *next;
};

struct fight_msg {
    int type;
    int total;
    struct msg_trio *list;
    struct fight_msg *next;
};



/* The Fight related routines */
void load_messages(void)
{
#if 0
    FILE *f1;
    int i,type;
    struct message_type *messages;
    char chk[100];

    if (!(f1 = fopen(MESS_FILE, "r"))){
	perror("read messages");
	exit(0);
    }

    for (i = 0; i < MAX_MESSAGES; i++)
    { 
	fight_messages[i].a_type = 0;
	fight_messages[i].number_of_attacks=0;
	fight_messages[i].msg = 0;
    }

    fscanf(f1, " %s \n", chk);

    while(*chk == 'M')
    {
	fscanf(f1," %d\n", &type);
	for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type!=type) &&
	    (fight_messages[i].a_type); i++);
	if(i>=MAX_MESSAGES){
	    log("Too many combat messages.");
	    exit(0);
	}

	CREATE(messages,struct message_type,1);
	fight_messages[i].number_of_attacks++;
	fight_messages[i].a_type=type;
	messages->next=fight_messages[i].msg;
	fight_messages[i].msg=messages;

	messages->die_msg.attacker_msg      = fread_string(f1);
	messages->die_msg.victim_msg        = fread_string(f1);
	messages->die_msg.room_msg          = fread_string(f1);
	messages->miss_msg.attacker_msg     = fread_string(f1);
	messages->miss_msg.victim_msg       = fread_string(f1);
	messages->miss_msg.room_msg         = fread_string(f1);
	messages->hit_msg.attacker_msg      = fread_string(f1);
	messages->hit_msg.victim_msg        = fread_string(f1);
	messages->hit_msg.room_msg          = fread_string(f1);
	fscanf(f1, " %s \n", chk);
    }

    fclose(f1);
#endif
}


void update_pos( struct char_data *victim )
{
    /* Handle hit point positions */
    if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
	;
    else if (GET_HIT(victim) > 0 )
	GET_POS(victim) = POS_STAND;
    else if (GET_HIT(victim) <= -11)
	GET_POS(victim) = POS_DEAD;
    else if (GET_HIT(victim) <= -6)
	GET_POS(victim) = POS_MORTALLYW;
    else if (GET_HIT(victim) <= -3)
	GET_POS(victim) = POS_INCAP;
    else
	GET_POS(victim) = POS_STUNNED;
}


/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
    if(ch->specials.fighting) {
        log("BUG: Character already fighting in set_fighting");
        return;
    }

    ch->next_fighting = combat_list;
    combat_list = ch;
/*
    if(IS_AFFECTED(ch,AFF_SLEEP))
	affect_from_char(ch,SPELL_SLEEP);*/

    ch->specials.fighting = vict;

    add_event(vict->in_room,EVENT_ATTACK,NULL,vict->id,ch->id);

    if(GET_POS(ch)<POS_STAND) /* Need a bodytype bitvector for positions? */
	GET_POS(ch)=POS_STAND; /* VEHICLE, perhaps? */
}


/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
    struct char_data *tmp;

    if(!ch->specials.fighting) {
        log("BUG: Character not fighting in stop_fighting");
        return;
    }

    if (ch == combat_next_dude)
	combat_next_dude = ch->next_fighting;

    if (combat_list == ch)
     combat_list = ch->next_fighting;
    else
    {
	for (tmp = combat_list; tmp && (tmp->next_fighting != ch); 
	    tmp = tmp->next_fighting);
	if (!tmp) {
	    log("BUG: Char fighting not found Error (stop_fighting)");
	    return;
	}
	tmp->next_fighting = ch->next_fighting;
    }

    ch->next_fighting = 0;
    ch->specials.fighting = 0;
    update_pos(ch);
}



#define MAX_CORPSE_TIME 10

void make_corpse(struct char_data *ch)
{
    struct obj_data *corpse, *o;
    struct obj_info_container *cont;
    char buf[MAX_STRING_LENGTH];

    char *strdup(char *source);

    CREATE(corpse, struct obj_data, 1);
    clear_object(corpse);

    
    corpse->item_number = NOWHERE;
    corpse->in_room = NOWHERE;
    corpse->name = strdup("corpse");

    sprintf(buf, "Corpse of %s is lying here.", 
     (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->description = strdup(buf);

    sprintf(buf, "Corpse of %s",
     (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->short_description = strdup(buf);

    corpse->obj_flags.extra_flags = ITEM_TAKE;

    cont = (struct obj_info_container *)new_obj_info(ITEM_CONTAINER,corpse);
    cont->capacity = 0; /* You can't store stuff in a corpse */
    cont->lock_state = CONT_CORPSE; /* corpse identifyer */
    corpse->obj_flags.weight = GET_WEIGHT(ch)+IS_CARRYING_W(ch);
/*    corpse->obj_flags.timer = MAX_CORPSE_TIME; */

    if(!IS_NPC(ch)&&IS_SET(world[ch->in_room].room_flags,ARENA)){
	obj_to_room(corpse, ch->in_room);
	corpse->next = object_list;
	object_list = corpse;
	return;

    } /* create an 'empty' token corpse in the arena */

    while((o=ch->carrying)) {
        obj_from_char(o);
        obj_to_obj(o,corpse);
    }

    obj_to_room(corpse, ch->in_room);
    save_char(ch);
}


/* When ch kills victim */
void change_alignment(struct char_data *ch, struct char_data *victim)
{
    int align;
    struct obj_data *obj,*next;

    /* Increment the kill count */
    ch->points.kills++;

    align = GET_ALIGNMENT(ch) - (GET_ALIGNMENT(victim)/ch->points.kills);

    if (align > 0) {
	if (align > 650)
	    GET_ALIGNMENT(ch) = MIN(1000,GET_ALIGNMENT(ch) + ((align-650) >> 3));
	else
	    GET_ALIGNMENT(ch) >>= 1;
    } else {
	if (align < -650)
	    GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) + ((align+650) >> 3));
	else
	    GET_ALIGNMENT(ch) >>= 1;
    }

    for(obj=ch->carrying;obj;obj=next) {
	next = obj->next_content;

	if(obj->equipped_as==UNEQUIPPED)
	    continue;
    
	if(((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
	  (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
	  (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) &&
	  (ch->in_room != NOWHERE)) {
	    act("You are zapped by $p and instantly drop it.",FALSE,ch,obj,0,TO_CHAR);
	    act("$n is zapped by $p and instantly drops it.",FALSE,ch,obj,0,TO_ROOM);
	    obj=unequip_char(ch,obj);
	    obj_to_room(obj,ch->in_room);
	}
    }
}



void death_cry(struct char_data *ch)
{
    int was_in;
    struct obj_data *o;
    struct obj_info_exit *exit;

    act("Your blood freezes as you hear $n's death cry.", FALSE,ch,0,0,TO_ROOM);
    was_in = ch->in_room;

    for(o=world[was_in].contents;o;o=o->next_content) {
        if((exit= (struct obj_info_exit *) get_obj_info(o,ITEM_EXIT))) {
            if(exit->to_room!=was_in) {
	        ch->in_room = exit->to_room;
	        act("Your blood freezes as you hear someones death cry.",
                FALSE,ch,0,0,TO_ROOM);
                ch->in_room = was_in;
            }
	}
    }
}


extern struct mob_index_data *mob_index;

void raw_kill(struct char_data *ch)
{
/*    char buf[MAX_STRING_LENGTH];*/

    if (ch->specials.fighting)
	stop_fighting(ch);

    death_cry(ch);
    make_corpse(ch);
    extract_char(ch);
}



void die(struct char_data *ch)
{
    gain_exp(ch, -(GET_EXP(ch)/2));

    add_event_room(ch->in_room,EVENT_DEATH,0,ch->id,ID_NOBODY);
    if(!IS_NPC(ch)) {
	raw_kill(ch);
	death_stat_handler(ch);
	return;
    }
    raw_kill(ch);
}

static char *dead_forever =
"   You feel your life force slowly escape from your body, and\n\r\
escape into the ethereal place of spirit-matter, separating into\n\r\
its constituent parts and mingling with the spirits of enemies,\n\r\
friends, lovers, great people, and the wicked...\n\r\n\r\
Rest In Peace.\n\r\n\r";

void death_stat_handler(struct char_data *ch)
{
    /* Put in check for org-death handling */

    if(!IS_NPC(ch)){
	/* Clear out the outlaw flag 
	REMOVE_BIT(ch->specials.act,ACT_OUTLAW);*/
	if(!--ch->physical->abilities.con) {
	    /* Mark character as gone, forever */
            SET_BIT(ch->prefs->flags,PLR_DEAD);
	    send_to_char(dead_forever,ch);
            if(ch->desc) {
                SEND_TO_Q("Do you wish to see your intrinsics? ",ch->desc);
                ch->desc->connected=CON_INTRIN;
            } else 
                delete_char(ch);
	    return; /* don't make a ghost */
	}
    } /* !isnpc */

    /* Make a ghost under certain conditions */
    if(GET_INT(ch) > 7 && GET_CON(ch) > 6) {
    }
}


char *replace_string(char *str, char *weapon)
{
    static char buf[256];
    char *cp;

    cp = buf;

    for (; *str; str++) {
	if (*str == '#') {
	    switch(*(++str)) {
		case 'W' : 
		    for (; *weapon; *(cp++) = *(weapon++));
		    break;
		default :
		    *(cp++) = '#';
		    break;
	    }
	} else {
	    *(cp++) = *str;
	}

	*cp = 0;
    } /* For */

    return(buf);
}



void dam_message(int dam, struct char_data *ch, struct char_data *victim,
	int w_type)
{
    struct obj_data *wield;
/*    char *buf;*/
    int dam_level;

#if 0
    static struct dam_weapon_type {
	char *to_room;
	char *to_char;
	char *to_victim;
    } dam_weapons[] = {

    {"$n misses $N with $s #W.",                           /*    0    */
     "You miss $N with your #W.",
     "$n misses you with $s #W." },

 {"$n tickles $N with $s #W.",                          /*  1.. 2  */
  "You tickle $N as you #W $M.",
  "$n tickles you as $e #W you." },

 {"$n barely #W $N.",                                   /*  3.. 4  */
  "You barely #W $N.",
  "$n barely #W you."},

    {"$n #W $N.",                                          /*  5.. 6  */
  "You #W $N.",
  "$n #W you."}, 

    {"$n #W $N hard.",                                     /*  7..10  */
     "You #W $N hard.",
  "$n #W you hard."},

    {"$n #W $N very hard.",                                /* 11..14  */
     "You #W $N very hard.",
     "$n #W you very hard."},

    {"$n #W $N extremely hard.",                          /* 15..20  */
     "You #W $N extremely hard.",
     "$n #W you extremely hard."},

    {"$N staggers from a fearsome #W from $n.",        /* 20..40 */
     "$N staggers from your fearsome #W.",
     "You stagger from a fearsome #W from $n."},

    {"$n massacres $N to small fragments with $s #W.",     /* 40..60  */
     "You massacre $N to small fragments with your #W.",
     "$n massacres you to small fragments with $s #W."},

				/* 60..100 */
    {"$N is enshrouded in a mist of blood after receiving $n's #W.",
     "$N is enshrouded in a mist of blood after receiving your #W.",
     "You are enshrouded in a mist of blood after $n's #W."},

    {"$n nearly rips $N apart from the force of $s #W.",   /* 100+ */
     "You nearly rip $N apart from the force of your #W.",
     "$n nearly rips you apart from the force of $s #W."}
    };
#endif
/*
    w_type -= TYPE_HIT;   * Change to base of table with text */

    wield = get_equip_used(ch,WIELD);

    if (dam == 0) {
	dam_level=0;
    } else if (dam <= 2) {
	dam_level=1;
    } else if (dam <= 4) {
	dam_level=2;
    } else if (dam <= 6) {
	dam_level=3;
    } else if (dam <= 10) {
	dam_level=4;
    } else if (dam <= 15) {
	dam_level=5;
    } else if (dam <= 20) {
	dam_level=6;
    } else if (dam <= 40) {
	dam_level=7;
    } else if (dam <= 60) {
	dam_level=8;
    } else if (dam <= 100) {
	dam_level=9;
    } else {
	dam_level=10;
    }
#if 0
    buf = replace_string(dam_weapons[dam_level].to_room, attack_hit_text[w_type].singular);
    act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
    buf = replace_string(dam_weapons[dam_level].to_char, attack_hit_text[w_type].singular);
    act(buf, FALSE, ch, wield, victim, TO_CHAR);
    buf = replace_string(dam_weapons[dam_level].to_victim, attack_hit_text[w_type].singular);
    act(buf, FALSE, ch, wield, victim, TO_VICT);
#endif
}



void damage(struct char_data *ch, struct char_data *victim,
      int dam, int attacktype)
{
    char buf[MAX_STRING_LENGTH];
#if 0
    struct message_type *messages;
#endif
    struct obj_data *weapon;
    int max_hit,exp;

    int hit_limit(struct char_data *ch);

    if (GET_POS(victim)<=POS_DEAD){
	fprintf(stderr,"BUG: damage() already dead failed by %s vs. %s.\n\r",
	 GET_NAME(ch),GET_NAME(victim));
	send_to_char("He's dead already, report this bug.\n\r",ch);
	send_to_char("You're dead. Report bug, please.\n\r",victim);
	return;
    }

    if (victim != ch) {
/*
	if (IS_NPC(ch)&&IS_AFFECTED(ch, AFF_CHARM) &&
	  !IS_NPC(victim)&&(victim->specials.fighting!=ch)){
	    send_to_char("You cannot harm another player!\n\r",ch);
	    return;
	}*/
#if 0
	if (IS_NPC(ch) && IS_NPC(victim) &&
	  victim->master &&
	  !number(0,10) && IS_AFFECTED(victim, AFF_CHARM) &&
	  (victim->master->in_room == ch->in_room)) {
	    if (ch->specials.fighting)
		stop_fighting(ch);
	    hit(ch, victim->master, TYPE_UNDEFINED);
	    return;
	}
#endif
    }

    if (victim->master == ch)
	stop_follower(victim);
	    
    dam=MIN(dam,300);

    dam=MAX(dam,0);

    GET_HIT(victim)-=dam;

    /* No exp for arena fighting */
    gain_exp(ch,dam);

    update_pos(victim);

    weapon=get_equip_used(ch,WIELD);
#if 0
    if ((attacktype >= TYPE_HIT) && (attacktype <= TYPE_CRUSH)) {
	if (!weapon) {
	    dam_message(dam, ch, victim, TYPE_HIT);
	} else {
	    dam_message(dam, ch, victim, attacktype);
	}
    } else {
/*
     if(!weapon) {
	log("Hit messages in fight.c with no weapon!");
	return;
     }*/
     for(i = 0; i < MAX_MESSAGES; i++) {
	if (fight_messages[i].a_type == attacktype) {
	    nr=dice(1,fight_messages[i].number_of_attacks);
	    for(j=1,messages=fight_messages[i].msg;(j<nr)&&(messages);j++)
		messages=messages->next;

	    if (dam != 0) {
		if (GET_POS(victim) == POS_DEAD) {
		    act(messages->die_msg.attacker_msg, FALSE, ch, weapon, victim, TO_CHAR);
		    act(messages->die_msg.victim_msg, FALSE, ch, weapon, victim, TO_VICT);
		    act(messages->die_msg.room_msg, FALSE, ch, weapon, victim, TO_NOTVICT);
		} else {
		    act(messages->hit_msg.attacker_msg, FALSE, ch, weapon, victim, TO_CHAR);
		    act(messages->hit_msg.victim_msg, FALSE, ch, weapon, victim, TO_VICT);
		    act(messages->hit_msg.room_msg, FALSE, ch, weapon, victim, TO_NOTVICT);
		}
	    } else { /* Dam == 0 */
		act(messages->miss_msg.attacker_msg, FALSE, ch, weapon, victim, TO_CHAR);
		act(messages->miss_msg.victim_msg, FALSE, ch, weapon, victim, TO_VICT);
		act(messages->miss_msg.room_msg, FALSE, ch, weapon, victim, TO_NOTVICT);
	    }
	}
    }
    }
#endif
    switch (GET_POS(victim)) {
	/*
	* Use send_to_char, because act() doesn't send
	* message if you are DEAD.
		*/
	case POS_MORTALLYW:
	    act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
	    send_to_char("You are mortally wounded, and will die soon, if not aided.", victim);
	    break;
	case POS_INCAP:
	    act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
	    send_to_char("You are incapacitated an will slowly die, if not aided.", victim);
	    break;
	case POS_STUNNED:
	    act("$n is stunned, but might regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
	    send_to_char("You're stunned, but you might regain consciousness again.", victim);
	    break;
	case POS_DEAD:
	    act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
       		send_to_char("You are dead!  Sorry...", victim);
	    break;

	default:  /* >= POS_SLEEP */

	    max_hit=hit_limit(victim);

	    if (dam > (max_hit/5))
		act("Ouch! That Really did HURT!",FALSE, victim, 0, 0, TO_CHAR);

	    if (GET_HIT(victim) < (max_hit/5)) {
	    
		act("You wish that your wounds would stop BLEEDING so much!",FALSE,victim,0,0,TO_CHAR);
	    }
	    break;		
    }

    if (!IS_NPC(victim) && !(victim->desc)) {
	if (!victim->specials.fighting) {
	    act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
	    victim->specials.was_in_room = victim->in_room; char_from_room(victim);
	    char_to_room(victim, 0,0);
	}
    }

    if (GET_POS(victim) < POS_STUNNED)
	if (ch->specials.fighting == victim)
	    stop_fighting(ch);

    if (!AWAKE(victim))
	if (victim->specials.fighting)
	    stop_fighting(victim);

    if (GET_POS(victim) == POS_DEAD) {
	if (IS_NPC(victim) || victim->desc)
	  /* Cannot get exp in the arena */
	  if(!IS_SET(world[ch->in_room].room_flags,ARENA)) {
	    /* Calculate level-difference bonus */
	    exp = GET_EXP(victim)/3;
	    exp = MAX(exp, 1);
	    gain_exp(ch, exp);
	    change_alignment(ch, victim);
	 } /* if not in arena area */
	if (!IS_NPC(victim) && /* No logging if arena 'death' */
	  !IS_SET(world[ch->in_room].room_flags,ARENA)) {
	    sprintf(buf, "%s killed by %s at %s",
		GET_NAME(victim),
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
		world[victim->in_room].name);
	    log(buf);
	}
#if 0
	if (IS_NPC(ch) /*&& !IS_NPC(victim)*/) {/* Ouch, this isn't */
	    forget(victim->player.name,ch);/* a perfect solution*/
	} /* if */                             /* to memory prob... */
#endif
	die(victim);
    }
}



void hit(struct char_data *ch, struct char_data *victim, int type)
{

    struct obj_data *wielded = 0;
    struct obj_data *held = 0;
    struct obj_info_weapon *weapon;
    int w_type;
    int dam,hit_factor,dam_factor;

    extern byte backstab_mult[];

    if (ch->in_room != victim->in_room) {
	log("BUG: NOT SAME ROOM WHEN FIGHTING!");
	return;
    }

    held = get_equip_used(ch,HOLD);
    wielded = get_equip_used(ch,WIELD);

    if(!(weapon=(struct obj_info_weapon *)get_obj_info(wielded,ITEM_WEAPON))) {
        /* Okay, wielded item is not a weapon...hmmm */
        log("BUG: Wielded item not a weapon - think about this later");
        return;
    }

/*
    if(wielded) {
	switch (weapon->damage_type) {
	    case 0  :
	    case 1  :
	    case 2  : w_type = TYPE_WHIP; break;
	    case 3  : w_type = TYPE_SLASH; break;
	    case 4  :
	    case 5  :
	    case 6  : w_type = TYPE_CRUSH; break;
	    case 7  : w_type = TYPE_BLUDGEON; break;
	    case 8  :
	    case 9  :
	    case 10 : w_type = TYPE_CLAW; break;
	    case 11 : w_type = TYPE_PIERCE; break;

	    default : w_type = TYPE_HIT; break;
	}
    }	else {
	if (IS_NPC(ch) && (ch->specials.attack_type >= TYPE_HIT))
	    w_type = ch->specials.attack_type;
	else
	    w_type = TYPE_HIT;
    }
*/
    /* Handle specific attack types */
    switch(type) {
        case SKILL_BASH:
        case SKILL_KICK:
            hit_factor =-5;
            dam_factor = 5;
            break;
        default:
            hit_factor = 0; /* Weapon hit factor? */
            dam_factor = 0; /* Weapon dam factor? */
            break;
    }

    if (GET_POS(victim) > POS_STUNNED) {
        if (!(victim->specials.fighting)) 
        set_fighting(victim, ch);
    }

    dam_factor += to_hit_char(ch,victim,hit_factor);
    if(!dam_factor) {
        /* Express the miss some other way */
act("You miss $N.",FALSE,ch,0,victim,TO_CHAR);
act("$n misses you.",FALSE,ch,0,victim,TO_VICT);
act("$n misses $N.",FALSE,ch,0,victim,TO_NOTVICT);
/*
	if (type == SKILL_BACKSTAB)
	    damage(ch, victim, 0, SKILL_BACKSTAB);
	else
	    damage(ch, victim, 0, w_type);*/
    } else {

	dam  = str_app[GET_STR(ch)].todam;
	dam += GET_DAMROLL(ch)+dam_factor;

	if (!wielded) {
	    if (IS_NPC(ch))
		dam += dice(ch->specials.damnodice, ch->specials.damsizedice);
	    else
		dam += number(0,2);  /* Max. 2 dam with bare hands */
	} else {
	    dam += dice(weapon->dice_num, weapon->dice_size);
	}

	if (GET_POS(victim) < POS_STAND)
	    dam *= 1+(POS_STAND-GET_POS(victim))/3;
	/* Position  sitting  x 1.33 */
	/* Position  resting  x 1.66 */
	/* Position  sleeping x 2.00 */
	/* Position  stunned  x 2.33 */
	/* Position  incap    x 2.66 */
	/* Position  mortally x 3.00 */

act("You hit $N.",FALSE,ch,0,victim,TO_CHAR);
act("$n hit you.",FALSE,ch,0,victim,TO_VICT);
act("$n hits $N.",FALSE,ch,0,victim,TO_NOTVICT);

	dam = MAX(1, dam);  /* Not less than 0 damage */

	if (type == SKILL_BACKSTAB) {
	    dam *= backstab_mult[10];
	    damage(ch, victim, dam, SKILL_BACKSTAB);
	} else {
	  if (wielded && obj_index[wielded->item_number].func) {
	   if(! (*obj_index[wielded->item_number].func)
	      (wielded, ch, -(dam*1000+w_type) , (char *) victim))
		 damage(ch,victim,dam,w_type);
	  } else
	    damage(ch, victim, dam, w_type);
	}
    }
}

void lose_exp_by_flight(struct char_data *ch)
{
    int lose;

    if(IS_NPC(ch) || IS_SET(world[ch->in_room].room_flags,ARENA))
	return;

    lose=GET_MAX_HIT(ch->specials.fighting)-GET_HIT(ch->specials.fighting);

    gain_exp(ch,-lose);
}

/* control the fights going on */
void perform_violence(void)
{
    struct char_data *ch;

    for (ch = combat_list; ch; ch=combat_next_dude)
    {
	combat_next_dude = ch->next_fighting;
        if(!ch->specials.fighting) {
            log("BUG: non-fighting character in combat list");
            continue;
        }

	if(AWAKE(ch) && (ch->in_room==ch->specials.fighting->in_room)) {
	    hit(ch, ch->specials.fighting, ch->specials.attack);
	} else { /* Not in same room */
	    stop_fighting(ch);
	}
    }
}

/* New generic hitting calculators */
int to_hit(struct char_data *ch, int vict_ac, int adj)
{
    int hit,diceroll;
    struct char_skill_data *sk;

    if((sk=get_skill(ch,SKILL_OFFENSE)))
        hit = 20 - sk->learned/10;
    else
        hit = 20;

    hit -= str_app[GET_STR(ch)].tohit;
    hit -= GET_HITROLL(ch);
    hit -= vict_ac;
    diceroll = number(1,20)+adj;

    if((diceroll-hit)<0)
        return(0);
    else
        return(diceroll-hit+1);
}

int to_hit_char(struct char_data *ch,struct char_data *victim,int adj)
{
    int victim_ac;
    extern struct dex_app_type dex_app[];

    victim_ac  = GET_AC(victim)/10;

    if (AWAKE(victim))
	victim_ac += dex_app[GET_DEX(victim)].defensive;
/*
    if(IS_EVIL(ch) && IS_AFFECTED(victim,AFF_PROTECT_EVIL))
	victim_ac -= 1;*/

    victim_ac = MAX(-10, victim_ac);  /* -10 is lowest */

    return(to_hit(ch,victim_ac,adj));
}
