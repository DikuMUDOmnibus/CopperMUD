
/* ************************************************************************
*  file: handler.c , Handler module.                      Part of DIKUMUD *
*  Usage: Various routines for moving about objects/players               *
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
#include "db.h"
#include "skills.h"
#include "error.h"
#include "proto.h"

extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct obj_data  *object_list;
extern struct char_data *character_list;
extern struct mob_index_data *mob_index;
extern struct obj_index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern char *menu;
extern int rev_dir[];
extern int max_dir[];
extern char log_buf[];

/* External procedures */

int str_cmp(char *arg1, char *arg2);
void free_char(struct char_data *ch);
void stop_fighting(struct char_data *ch);
void remove_follower(struct char_data *ch);
extern char *index();
void clearMemory(struct char_data *ch);
void update_crash(struct char_data *ch);

char *fname(char *namelist)
{
    static char holder[30];
    register char *point;

    if(!namelist) {
	log("BUG: null pointer into fname!");
	*point='\0';
	return(point);
    }

    for (point = holder; isalpha(*namelist); namelist++, point++)
	*point = *namelist;

    *point = '\0';

    return(holder);
}

int isname(char *str, char *namelist)
{
    register char *curname, *curstr;

    if(!str || !namelist)
        return NULL;

    curname = namelist;
    for (;;)
    {
	for (curstr = str;; curstr++, curname++)
	{
	    if (!*curstr && !isalpha(*curname))
		return(1);

	    if (!*curname)
		return(0);

	    if (!*curstr || *curname == ' ')
		break;

	    if (LOWER(*curstr) != LOWER(*curname))
		break;
	}

	/* skip to next name */

	for (; isalpha(*curname); curname++);
	if (!*curname)
	    return(0);
	curname++;			/* first char of new name */
    }
}



void affect_modify(struct char_data *ch,byte loc, byte mod, long bitv, bool add)
{
    int maxabil;

    if (!add) {
	mod = -mod;
    }


    maxabil = (IS_NPC(ch) ? 25:18);

    switch(loc)
    {
	case APPLY_NONE:
	    break;

	case APPLY_STR:
	    GET_STR(ch) += mod;
	    break;

	case APPLY_DEX:
	    GET_DEX(ch) += mod;
	    break;

	case APPLY_INT:
	    GET_INT(ch) += mod;
	    break;

	case APPLY_WIS:
	    GET_WIS(ch) += mod;
	    break;

	case APPLY_CON:
	    GET_CON(ch) += mod;
	    break;

	case APPLY_SEX:
	    /* ??? GET_SEX(ch) += mod; */
	    break;

	case APPLY_CLASS:
	    /* ??? GET_CLASS(ch) += mod; */
	    break;

	case APPLY_LEVEL:
	    /* ??? GET_LEVEL(ch) += mod; */
	    break;

	case APPLY_AGE:
/*			ch->player.time.birth += mod; */
	    break;

	case APPLY_CHAR_WEIGHT:
	    GET_WEIGHT(ch) += mod;
	    break;

	case APPLY_CHAR_HEIGHT:
	    GET_HEIGHT(ch) += mod;
	    break;

	case APPLY_MANA:
	    ch->physical->max_mana += mod;
	    if(GET_MANA(ch)>mana_limit(ch))
		GET_MANA(ch)=mana_limit(ch);
	    break;

	case APPLY_HIT:
	    ch->physical->max_hit += mod;
	    if(GET_HIT(ch)>hit_limit(ch))
		GET_HIT(ch)=hit_limit(ch);
	    break;

	case APPLY_MOVE:
	    ch->physical->max_move += mod;
	    break;

	case APPLY_GOLD:
	    break;

	case APPLY_EXP:
	    break;

	case APPLY_AC:
	    GET_AC(ch) += mod;
	    break;

	case APPLY_HITROLL:
	    GET_HITROLL(ch) += mod;
	    break;

	case APPLY_DAMROLL:
	    GET_DAMROLL(ch) += mod;
	    break;

	case APPLY_SAVING_PARA:
	    ch->specials.apply_saving_throw[0] += mod;
	    break;

	case APPLY_SAVING_ROD:
	    ch->specials.apply_saving_throw[1] += mod;
	    break;

	case APPLY_SAVING_PETRI:
	    ch->specials.apply_saving_throw[2] += mod;
	    break;

	case APPLY_SAVING_BREATH:
	    ch->specials.apply_saving_throw[3] += mod;
	    break;

	case APPLY_SAVING_SPELL:
	    ch->specials.apply_saving_throw[4] += mod;
	    break;

	default:
	    log("BUG: Unknown apply adjust attempt (handler.c, affect_modify)");
	    break;

    } /* switch */
}



/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(struct char_data *ch)
{
    struct affected_type *af;
    struct obj_affect *oa;
    struct obj_data *i;
    int j;

    for(i=ch->carrying; i; i=i->next_content) {
	if(i->equipped_as != UNEQUIPPED)
	    for(oa=i->affected;oa;oa=oa->next)
		affect_modify(ch, oa->location,
		   oa->modifier,
		   i->obj_flags.bitvector, FALSE);
    }


    for(af = ch->affected; af; af=af->next)
	affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

    ch->tmpabilities = ch->physical->abilities;

    for(i=ch->carrying; i; i=i->next_content) {
	if(i->equipped_as != UNEQUIPPED)
	    for(oa=i->affected;oa;oa=oa->next)
		affect_modify(ch, oa->location,
		   oa->modifier,
		   i->obj_flags.bitvector, TRUE);
    }


    for(af = ch->affected; af; af=af->next)
	affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

    /* Make certain values are between 0..25, not < 0 and not > 25! */

    j = (IS_NPC(ch) ? 25 :18);

    GET_DEX(ch) = MAX(0,MIN(GET_DEX(ch), j));
    GET_INT(ch) = MAX(0,MIN(GET_INT(ch), j));
    GET_WIS(ch) = MAX(0,MIN(GET_WIS(ch), j));
    GET_CON(ch) = MAX(0,MIN(GET_CON(ch), j));
    GET_STR(ch) = MAX(0,MIN(GET_STR(ch), j));
}



/* Insert an affect_type in a char_data structure
 Automatically sets apropriate bits and apply's */
void affect_to_char( struct char_data *ch, struct affected_type *af )
{
    struct affected_type *affected_alloc;

    CREATE(affected_alloc, struct affected_type, 1);

    *affected_alloc = *af;
    affected_alloc->next = ch->affected;
    ch->affected = affected_alloc;

    affect_modify(ch, af->location, af->modifier,
	   af->bitvector, TRUE);
    affect_total(ch);
}



/* Remove an affected_type structure from a char (called when duration
 reaches zero). Pointer *af must never be NIL! Frees mem and calls 
 affect_location_apply                                                */
void affect_remove( struct char_data *ch, struct affected_type *af )
{
    struct affected_type *hjp;

    affect_modify(ch, af->location, af->modifier,
	   af->bitvector, FALSE);


    /* remove structure *af from linked list */

    if (ch->affected == af) {
	/* remove head of list */
	ch->affected = af->next;
    } else {

	for(hjp = ch->affected; (hjp->next) && (hjp->next != af); hjp = hjp->next);

	if (hjp->next != af) {
	    log("BUG: Could not locate affected_type in ch->affected. (affect_remove)");
	    return;
	}
	hjp->next = af->next; /* skip the af element */
    }

    free ( af );

    affect_total(ch);
}



/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char( struct char_data *ch, byte skill)
{
    struct affected_type *hjp;

    for(hjp = ch->affected; hjp; hjp = hjp->next)
	if (hjp->type == skill)
	    affect_remove( ch, hjp );

}



/* Return if a char is affected by a spell (SPELL_XXX), NULL indicates 
 not affected                                                        */
bool affected_by_spell( struct char_data *ch, byte skill )
{
    struct affected_type *hjp;

    for (hjp = ch->affected; hjp; hjp = hjp->next)
	if ( hjp->type == skill )
	    return( TRUE );

    return( FALSE );
}



void affect_join( struct char_data *ch, struct affected_type *af,
	 bool avg_dur, bool avg_mod )
{
    struct affected_type *hjp;
    bool found = FALSE;

    for (hjp = ch->affected; !found && hjp; hjp = hjp->next) {
	if ( hjp->type == af->type ) {
	    
	    af->duration += hjp->duration;
	    if (avg_dur)
		af->duration /= 2;

	    af->modifier += hjp->modifier;
	    if (avg_mod)
		af->modifier /= 2;

	    affect_remove(ch, hjp);
	    affect_to_char(ch, af);
	    found = TRUE;
	}
    }
    if (!found)
	affect_to_char(ch, af);
}

/* move a player out of a room */
void char_from_room(struct char_data *ch)
{
    struct char_data *i;

    if (ch->in_room == NOWHERE) {
	log("BUG: NOWHERE extracting char from room (char_from_room)");
	return;
    }

    world[ch->in_room].light -= ch->specials.lights_carried;

    if (ch == world[ch->in_room].people)  /* head of list */
	world[ch->in_room].people = ch->next_in_room;

    else    /* locate the previous element */
    {
	for (i = world[ch->in_room].people; 
	    i->next_in_room != ch; i = i->next_in_room);

    	i->next_in_room = ch->next_in_room;
    }

    ch->in_room = NOWHERE;
    ch->next_in_room = 0;
}


/* place a character in a room */
void char_to_room(struct char_data *ch, int room,int dir)
{
    int t_dir,j,other_exit=-1;
    struct char_data *k;


    if(!(world[room].room_flags & SINGLE_FILE)) { /* Don't care about order*/
	ch->next_in_room = world[room].people;
	world[room].people = ch;
    } else {
	t_dir=rev_dir[dir];
	for(j=0;j<max_dir[zone_table[world[room].zone].dir_system];j++)
            if(get_exit(room,j) && j!=t_dir)
		other_exit=j;
	if(other_exit==-1 || other_exit > t_dir) {
	    ch->next_in_room = world[room].people;
	    world[room].people = ch;
	} else {
	    if(!(k=world[room].people))
		world[room].people=ch;
	    else {
		while(k->next_in_room)
		    k=k->next_in_room;
		k->next_in_room=ch;
	    }
	}
    }
    ch->in_room = room;

    world[room].light += ch->specials.lights_carried;
}


/* give an object to a char   */
void obj_to_char(struct obj_data *object, struct char_data *ch)
{
    struct obj_info_light *light;

    if(!object)
	log("BUG: No object in obj_to_char!");
    if(!ch)
	log("BUG: No character in obj_to_char!");
    if(!object || !ch)
	return;

    object->next_content = ch->carrying;
    ch->carrying = object;
    object->carried_by = ch;
    object->in_room = NOWHERE;
    IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(ch)++;

    if((light = (struct obj_info_light *)get_obj_info(object,ITEM_LIGHT))) {
	ch->specials.lights_carried+=light->brightness;
	if(ch->in_room!=NOWHERE)
	    world[ch->in_room].light+=light->brightness;
    }
}


/* take an object from a char */
void obj_from_char(struct obj_data *object)
{
    struct obj_data *tmp;
    struct obj_info_light *light;

    if(!object->carried_by) {
        log("BUG: obj_from_char when obj not carried");
        return;
    }

    if((light = (struct obj_info_light *)get_obj_info(object,ITEM_LIGHT))) {
	object->carried_by->specials.lights_carried-=light->brightness;
	if(object->carried_by->in_room!=NOWHERE)
	    world[object->carried_by->in_room].light-=light->brightness;
    }

    if (object->carried_by->carrying == object)   /* head of list */
	object->carried_by->carrying = object->next_content;

    else
    {
	for (tmp = object->carried_by->carrying; 
	    tmp && (tmp->next_content != object); 
	   tmp = tmp->next_content); /* locate previous */

	tmp->next_content = object->next_content;
    }

    IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(object->carried_by)--;
    object->carried_by = 0;
    object->next_content = 0;
}



/* Return the effect of a piece of armor in position eq_pos */
int apply_ac(struct char_data *ch, struct obj_data *obj)
{
    struct obj_info_wear *ow;

    if(!obj) {
        log("BUG: Null obj in apply_ac");
        return 0;
    }

    if (!(ow = (struct obj_info_wear *)get_obj_info(obj,ITEM_WORN)))
	return 0;

    switch (obj->equipped_as) {

    case WEAR_BODY:
	return (3*ow->ac);  /* 30% */
    case WEAR_HEAD:
	return (2*ow->ac);  /* 20% */
    case WEAR_LEGS:
	return (2*ow->ac);  /* 20% */
    case WEAR_FEET:
	return (ow->ac);    /* 10% */
    case WEAR_HANDS:
	return (ow->ac);    /* 10% */
    case WEAR_ARMS:
	return (ow->ac);    /* 10% */
    case HOLD_HAND1:
    case HOLD_HAND2:
	return (ow->ac);    /* 10% */
    case WEAR_WAIST:
	return (ow->ac);    /* 10% */
    }
    return 0;
}



void equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
    struct obj_affect *oa;
    struct obj_info_wear *ow;

    if (obj->in_room!=NOWHERE) {
	log("BUG: Obj is in_room when equip.");
	return;
    }

    if(obj->carried_by && obj->carried_by != ch) {
	log("BUG: Obj is carried by someone else when equip.");
	return;
    }

    if(obj->equipped_as!=UNEQUIPPED) {
	log("BUG: Obj is already equipped.");
	return;
    }

    if(!obj->carried_by)
	obj_to_char(obj,ch);

    if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
	if (ch->in_room != NOWHERE) {

	    act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
	    act("$n is zapped by $p and instantly drops it.", FALSE, ch, obj, 0, TO_ROOM);
	    obj_from_char(obj);
	    obj_to_room(obj, ch->in_room);
	    return;
	} else {
	    log("BUG: ch->in_room = NOWHERE when equipping char.");
	}
    }

    obj->equipped_as = pos;

    GET_AC(ch) -= apply_ac(ch, obj);

    if((ow = (struct obj_info_wear *)get_obj_info(obj,ITEM_WORN)))
	ch->specials.warmth += ow->warmth;

    for(oa=obj->affected;oa;oa=oa->next)
	affect_modify(ch, oa->location, oa->modifier,
	 obj->obj_flags.bitvector, TRUE);

    affect_total(ch);
}



struct obj_data *unequip_char(struct char_data *ch, struct obj_data *obj)
{
    struct obj_affect *oa;
    struct obj_info_wear *ow;

    GET_AC(ch) += apply_ac(ch, obj);

    if((ow = (struct obj_info_wear *)get_obj_info(obj,ITEM_WORN)))
	ch->specials.warmth -= ow->warmth;

    for(oa=obj->affected;oa;oa=oa->next)
	affect_modify(ch, oa->location,
	 oa->modifier,
	 obj->obj_flags.bitvector, FALSE);

    obj->equipped_as = UNEQUIPPED;

    affect_total(ch);

    return(obj);
}


int get_number(char **name) {

    int i;
    char *ppos;
    char number[MAX_INPUT_LENGTH+30] = "";

    if(strlen(*name)>30)
	return(0);

    if ((ppos = index(*name, '.'))) {
	*ppos++ = '\0';
	strcpy(number,*name);
	strcpy(*name, ppos);

	for(i=0; *(number+i); i++)
	    if (!isdigit(*(number+i)))
		return(0);

	return(atoi(number));
    }

    return(1);
}

struct obj_data *get_equip_used(struct char_data *ch,int use)
{
    struct obj_data *obj;

    for(obj=ch->carrying;obj;obj=obj->next_content) {
	if(obj->equipped_as != UNEQUIPPED) {
	    if(use >=0) {
		if(obj->equipped_as==use)
		    return(obj);
	    } else {
		switch(use) {
		}
	    }
	}
    }
    return(NULL);
}


/* Search a given list for an object, and return a pointer to that object */
struct obj_data *get_obj_in_list(char *name, struct obj_data *list)
{
    struct obj_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp))) return(0);

    for (i = list, j = 1; i && (j <= number); i = i->next_content)
	if (isname(tmp, i->name)) {
	    if (j == number) 
		return(i);
	    j++;
	}

    return(0);
}



/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list)
{
    struct obj_data *i;

    for (i = list; i; i = i->next_content)
	if (i->item_number == num) 
	    return(i);
	
    return(0);
}




/*search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
    struct obj_data *i;

    for (i = object_list; i; i = i->next)
	if (i->item_number == nr) 
	    return(i);

    return(0);
}


/* return a char_data structure for an id number */
struct char_data *get_char_from_id(CHAR_ID id)
{
    struct char_data *i;

    for(i=character_list;i;i=i->next)
	if(i->id == id)
	    return(i);

    /* We didn't find it in the game */


    /* Doesn't exist at all */
    return(NULL);
}



/* search a room for a char, and return a pointer if found..  */
struct char_data *get_char_room(char *name, int room)
{
    struct char_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    if(!*name)
        return(NULL);

    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);

    for (i = world[room].people, j = 1; i && (j <= number); i = i->next_in_room)
	if (isname(tmp, GET_NAME(i))) {
	    if (j == number)
    return(i);
	    j++;
	}

    return(0);
}


/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(int nr)
{
    struct char_data *i;

    for (i = character_list; i; i = i->next)
	if (i->nr == nr)
	    return(i);

    return(0);
}

/* put an object in a room */
void obj_to_room(struct obj_data *object, int room)
{
    struct char_data *i;
    struct obj_info_exit *exit;
    struct obj_info_light *light;

    if(!object) {
	log("BUG: Null object in obj_to_room!");
	return;
    }

    object->next_content = world[room].contents;
    world[room].contents = object;
    object->in_room = room;
    object->carried_by = 0;

    /* Is it lit? */
    if((light = (struct obj_info_light *)get_obj_info(object,ITEM_LIGHT)))
        world[room].light+=light->brightness;

    if(object->obj_flags.extra_flags & ITEM_IGNORE)
        return;

    if((world[room].sector_type==SECT_WATER_SWIM ||
	    world[room].sector_type==SECT_WATER_NOSWIM) &&
	    !IS_SET(object->obj_flags.extra_flags,ITEM_FLOAT)) {
	for(i=world[room].people;i;i=i->next_in_room)
	    if(CAN_SEE_OBJ(i,object))
		act("$p sinks into the water.",TRUE,i,object,0,TO_CHAR);
	extract_obj(object);
    } else if(world[room].sector_type==SECT_NO_GROUND) {
	for(i=world[room].people;i;i=i->next_in_room)
	    if(CAN_SEE_OBJ(i,object))
		act("$p falls downward.",TRUE,i,object,0,TO_CHAR);
        exit = get_exit(room,5);
	if(exit && exit->to_room!=room){
	    obj_from_room(object);

	    /* Need to do this before obj_to_room so that */
	    /* when the falling obj goes through more than */
	    /* one room, it doesn't fall out before it arrives */
	    for(i=world[exit->to_room].people; i;i=i->next_in_room)
		if(CAN_SEE_OBJ(i,object))
		    act("$p falls here from above.",TRUE,i,object,0,TO_CHAR);
	    obj_to_room(object,exit->to_room);
	} else
	    extract_obj(object);
    }
}

/* Take an object from a room */
void obj_from_room(struct obj_data *object)
{
    struct obj_data *i;
    struct obj_info_light *light;

    /* Is it lit? */
    if((light = (struct obj_info_light *)get_obj_info(object,ITEM_LIGHT)))
        world[object->in_room].light-=light->brightness;

    /* remove object from room */

    if (object == world[object->in_room].contents)  /* head of list */
     world[object->in_room].contents = object->next_content;

    else     /* locate previous element in list */
    {
	for (i = world[object->in_room].contents; i && 
	 (i->next_content != object); i = i->next_content);

	i->next_content = object->next_content;
	}

    object->in_room = NOWHERE;
    object->next_content = 0;
}


/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to)
{
    struct obj_data *tmp_obj;

    obj->next_content = obj_to->contains;
    obj_to->contains = obj;
    obj->in_obj = obj_to;

    for(tmp_obj = obj->in_obj; tmp_obj; tmp_obj = tmp_obj->in_obj) {
	GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);
	if(!tmp_obj->in_obj && tmp_obj->carried_by)
	    IS_CARRYING_W(tmp_obj->carried_by) += GET_OBJ_WEIGHT(obj);
    }
}


/* remove an object from an object */
void obj_from_obj(struct obj_data *obj)
{
    struct obj_data *tmp, *obj_from;

    if (obj->in_obj) {
	obj_from = obj->in_obj;
	if (obj == obj_from->contains)   /* head of list */
	 obj_from->contains = obj->next_content;
	else {
	    for (tmp = obj_from->contains; 
		tmp && (tmp->next_content != obj);
		tmp = tmp->next_content); /* locate previous */

	    if (!tmp) {
		perror("Fatal error in object structures.");
		abort();
	    }

	    tmp->next_content = obj->next_content;
	}


	/* Subtract weight from containers container */
	for(tmp = obj->in_obj; tmp->in_obj; tmp = tmp->in_obj)
	    GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

	GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

	/* Subtract weight from char that carries the object */
	if (tmp->carried_by)
	    IS_CARRYING_W(tmp->carried_by) -= GET_OBJ_WEIGHT(obj);

	obj->in_obj = 0;
	obj->next_content = 0;
    } else {
	perror("Trying to object from object when in no object.");
	abort();
    }
}


/* Extract an object from the world */
void extract_obj(struct obj_data *obj)
{
    struct obj_data *temp1;

    if(obj->in_room != NOWHERE)
	obj_from_room(obj);
    else if(obj->carried_by)
	obj_from_char(obj);
    else if(obj->in_obj)
	obj_from_obj(obj);
/*	else if(obj->in_obj) What sense does this make? * * * * * *
    {
	temp1 = obj->in_obj;
	if(temp1->contains == obj)   * head of list *
	    temp1->contains = obj->next_content;
	else
	{
	    for( temp2 = temp1->contains ;
		temp2 && (temp2->next_content != obj);
		temp2 = temp2->next_content );

	    if(temp2) {
		temp2->next_content =
		    obj->next_content; }
	}
    }
*/

    for( ; obj->contains; extract_obj(obj->contains)); 
	/* leaves nothing ! */

    if (object_list == obj )       /* head of list */
	object_list = obj->next;
    else
    {
	for(temp1 = object_list; 
	    temp1 && (temp1->next != obj);
	    temp1 = temp1->next);
	
	if(temp1)
	    temp1->next = obj->next;
    }

    if(obj->item_number>=0)
	(obj_index[obj->item_number].number)--;
    free_obj(obj);
}

/* Remove a char's "belongings" completely from the world, without saving */
void extract_char_eq(struct char_data *ch)
{
    while(ch->carrying) {
	if(ch->carrying->equipped_as != UNEQUIPPED)
	    unequip_char(ch,ch->carrying);
	extract_obj(ch->carrying);
    }
}


/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char(struct char_data *ch)
{
    struct char_data *k, *next_char;
    struct descriptor_data *t_desc;
    int was_in;

    extern struct char_data *combat_list;

    void do_return(struct char_data *ch, char *argument, int cmd);

    void die_follower(struct char_data *ch);

    if(!ch) {
	log("BUG: extract_char(*NULL*)");
	return;
    }

    if(!IS_NPC(ch) && !ch->desc)
    {
	for(t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
	    if(t_desc->original==ch)
		do_return(t_desc->character, "", 0);
    }

    if (ch->in_room == NOWHERE) {
	log("BUG: NOWHERE extracting char. (extract_char)");
    /*	exit(1); */ return; /* Must we really crash here? */
    }

    if (ch->followers || ch->master)
	die_follower(ch);

/*** This shouldn't be necessary
    while(ch->carrying) {
	i=ch->carrying;
	obj_from_char(i);
	obj_to_room(i,ch->in_room);
    }
***/


    
    if (ch->specials.fighting)
	stop_fighting(ch);

    for (k = combat_list; k ; k = next_char) {
	next_char = k->next_fighting;
	if (k->specials.fighting == ch)
	    stop_fighting(k);
    }

    /* Get rid of arrest information */
    if(ch->specials.arrest_by) {
	if(ch->specials.arrest_by->specials.arrest_link==ch)
	    ch->specials.arrest_by->specials.arrest_link=ch->specials.arrest_link;
	else
	    for(k=ch->specials.arrest_by->specials.arrest_link;k;k=k->specials.arrest_link)
		if(k->specials.arrest_link==ch) {
		    k->specials.arrest_link=ch->specials.arrest_link;
		    break;
		}
    }

    if(ch->in_room!=NOWHERE) {
        /* Must remove from room before removing the equipment! */
        was_in = ch->in_room;
        char_from_room(ch);
        char_to_room(ch,0,0);
    }

    GET_AC(ch) = 100;
    if (affected_by_spell(ch, SKILL_ARMOR))
      GET_AC(ch) -= 20;
    if (affected_by_spell(ch, SKILL_BLINDNESS))
      GET_AC(ch) += 40;

    if (ch->desc)
	if (ch->desc->original)
	    do_return(ch, "", 0);

    if (IS_NPC(ch)) 
    {
	if (ch->nr > -1) /* if mobile */
	    mob_index[ch->nr].number--;
/*                clearMemory(ch);    Only NPC's can have memory */
        SET_BIT(ch->specials.act,ACT_CLEANUP);
	return;
    }

    if (ch->desc) {
	ch->desc->connected = CON_SLCT;
	SEND_TO_Q(menu, ch->desc);
        remove_char(ch);
    } else
        SET_BIT(ch->specials.act,ACT_CLEANUP);
}

void remove_char(struct char_data *ch)
{
    struct char_data *i;

    if(ch->in_room!=NOWHERE)
        char_from_room(ch);

    if(character_list==ch)
        character_list=ch->next;
    else
        for(i=character_list;i->next;i=i->next)
            if(i->next==ch) {
                i->next=ch->next;
                break;
            }

    ch->next=NULL;
    reset_char(ch);
}

/* ***********************************************************************
 Here follows high-level versions of some earlier routines, ie functions
 which incorporate the actual player-data.
 *********************************************************************** */


struct char_data *get_char_room_vis(struct char_data *ch, char *name)
{
    struct char_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH+50];
    char *tmp;

    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);

    if(!str_cmp(name,"me") || !str_cmp(name,"self"))
	return(ch);

    for (i = world[ch->in_room].people, j = 1; i && (j <= number); i = i->next_in_room)
	if (isname(tmp, GET_NAME(i)))
	    if (CAN_SEE(ch, i))	{
		if (j == number) 
		    return(i);
		j++;
	    }

    return(0);
}





struct char_data *get_char_vis(struct char_data *ch, char *name)
{
    struct char_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    /* check location */
    if ((i = get_char_room_vis(ch, name)))
	return(i);

    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);

    for (i = character_list, j = 1; i && (j <= number); i = i->next)
	if (isname(tmp, GET_NAME(i)))
	    if (CAN_SEE(ch, i))	{
		if (j == number)
		    return(i);
		j++;
	    }

    return(0);
}






struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, 
		struct obj_data *list)
{
    struct obj_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH+50];
    char *tmp;

    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);

    for (i = list, j = 1; i && (j <= number); i = i->next_content)
	if (isname(tmp, i->name))
	    if (CAN_SEE_OBJ(ch, i)) {
		if (j == number)
		    return(i);
		j++;
	    }
    return(0);
}





/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data *ch, char *name)
{
    struct obj_data *i;
    int j, number;
 char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    /* scan items carried */
    if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
	return(i);

    /* scan room */
    if ((i = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)))
	return(i);

 strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);

    /* ok.. no luck yet. scan the entire obj list   */
    for (i = object_list, j = 1; i && (j <= number); i = i->next)
	if (isname(tmp, i->name))
	    if (CAN_SEE_OBJ(ch, i)) {
		if (j == number)
		    return(i);
		j++;
	    }
    return(0);
}

struct room_data *get_room_vis(struct char_data *ch, char *name)
{
    return NULL;
}

/* NOTE WARNING HEY HI THERE - this function isn't being called
   REMOVE later if no good reason is found to keep it
*/
struct obj_data *create_money( int amount )
{
    struct obj_data *obj;
    struct obj_info_money *money;
    struct extra_descr_data *new_descr;
    char buf[80];

    char *strdup(char *str);

    if(amount<=0)
    {
	log("BUG: Try to create negative money.");
	exit(1);
    }

    CREATE(obj, struct obj_data, 1);
    CREATE(new_descr, struct extra_descr_data, 1);
    clear_object(obj);

    if(amount==1)
    {
	obj->name = strdup("coin gold");
	obj->short_description = strdup("a gold coin");
	obj->description = strdup("One miserable gold coin.");

	new_descr->keyword = strdup("coin gold");
	new_descr->description = strdup("One miserable gold coin.");
    }
    else
    {
	obj->name = strdup("coins gold");
	obj->short_description = strdup("gold coins");
	obj->description = strdup("A pile of gold coins.");

	new_descr->keyword = strdup("coins gold");
	if(amount<10) {
	    sprintf(buf,"There are %d coins.",amount);
	    new_descr->description = strdup(buf);
	} 
	else if (amount<100) {
	    sprintf(buf,"There are about %d coins",10*(amount/10));
	    new_descr->description = strdup(buf);
	}
	else if (amount<1000) {
	    sprintf(buf,"It looks like something around %d coins",100*(amount/100));
	    new_descr->description = strdup(buf);
	}
	else if (amount<100000) {
	    sprintf(buf,"You guess there are %d coins",1000*(amount/1000));
	    new_descr->description = strdup(buf);
	}
	else 
	    new_descr->description = strdup("There are A LOT of coins");			
    }

    new_descr->next = 0;
    obj->ex_description = new_descr;

    obj->obj_flags.extra_flags = ITEM_TAKE;

    money = (struct obj_info_money *)new_obj_info(ITEM_MONEY,obj);
    money->amount = amount;
    money->money_type = MONEY_GOLD_HUMAN; /* default? value */
    obj->obj_flags.cost = amount;
    obj->item_number = -1;

    obj->next = object_list;
    object_list = obj;

    return(obj);
}



/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar    Will be NULL if nothing was found, otherwise points          */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(char *arg, int bitvector, struct char_data *ch, void **tar)
{
    static char *ignore[] = {
	"the",
	"in",
	"on",
	"at",
	"\n" };

    int i;
    char name[256];
    bool found;

    found = FALSE;

    /* Eliminate spaces and "ignore" words */
    while (*arg && !found) {

	for(; *arg == ' '; arg++)   ;

	for(i=0; (name[i] = *(arg+i)) && (name[i]!=' '); i++)   ;
	name[i] = 0;
	arg+=i;
	if (search_block(name, ignore, TRUE) > -1)
	    found = TRUE;

    }
    if (!name[0])
	return(0);

    *tar = 0;

    if (IS_SET(bitvector, FIND_CHAR_ROOM)) {      /* Find person in room */
	if ((*tar = (void *) get_char_room_vis(ch, name))) {
	    return(FIND_CHAR_ROOM);
	}
    }

    if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
	if ((*tar = (void *) get_char_vis(ch, name))) {
	    return(FIND_CHAR_WORLD);
	}
    }

    if (IS_SET(bitvector, FIND_OBJ_INV)) {
	if ((*tar = (void *) get_obj_in_list_vis(ch, name, ch->carrying))) {
	    return(FIND_OBJ_INV);
	}
    }

    if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
	if ((*tar = (void *) get_obj_in_list_vis(ch, name, world[ch->in_room].contents))) {
	    return(FIND_OBJ_ROOM);
	}
    }

    if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
	if ((*tar = (void *) get_obj_vis(ch, name))) {
	    return(FIND_OBJ_WORLD);
	}
    }

    if (IS_SET(bitvector, FIND_ROOM)) {
	if ((*tar = (void *) get_room_vis(ch, name))) {
	    return(FIND_ROOM);
	}
    }

    return(0);
}
