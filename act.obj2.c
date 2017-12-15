/* ************************************************************************
*  file: act.obj2.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Commands mainly using objects.                                 *
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
#include "error.h"
#include "proto.h"

/* extern variables */

extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern char *drinks[];
extern int drink_aff[][3];

/* extern functions */
struct obj_data *get_equip_used(struct char_data *ch,int use);
char *strdup(char *source);


int do_extinguish(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj;
    struct obj_info_light *light;
    char buf[MAX_INPUT_LENGTH];

    one_argument(argument,buf);
    if(!(obj=get_obj_in_list_vis(ch,buf,ch->carrying))) {
	send_to_char("Extinguish what?\n\r",ch);
	return ERROR_MISSING_TARGET;
    }

    if(!(light = (struct obj_info_light *)get_obj_info(obj,ITEM_LIGHT))) {
	send_to_char("But that can't be lit!\n\r",ch);
	return ERROR_NO_SENSE;
    }

    if(!light->brightness) {
	send_to_char("It's already out!\n\r",ch);
	return ERROR_ALREADY_DONE;
    }

    off_light(obj);
    act("$n extinguishes $p.",FALSE,ch,obj,NULL,TO_ROOM);
    return OKAY;
}

int do_light(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj;
    struct obj_info_light *light;
    char buf[MAX_INPUT_LENGTH];

    one_argument(argument,buf);
    if(!(obj=get_obj_in_list_vis(ch,buf,ch->carrying))) {
	send_to_char("Light what?\n\r",ch);
	return ERROR_SYNTAX;
    }

    if(!(light = (struct obj_info_light *)get_obj_info(obj,ITEM_LIGHT))) {
	send_to_char("But that can't be lit!\n\r",ch);
	return ERROR_NO_SENSE;
    }

    if(light->brightness) {
	send_to_char("It's already lit!\n\r",ch);
	return ERROR_ALREADY_DONE;
    }

    on_light(obj);
    act("$n lights $p.",FALSE,ch,obj,NULL,TO_ROOM);
    return OKAY;
}

void weight_change_object(struct obj_data *obj, int weight)
{
    struct obj_data *tmp_obj;
    struct char_data *tmp_ch;

    if (obj->in_room != NOWHERE) {
	GET_OBJ_WEIGHT(obj) += weight;
    } else if ((tmp_ch = obj->carried_by)) {
	obj_from_char(obj);
	GET_OBJ_WEIGHT(obj) += weight;
	obj_to_char(obj, tmp_ch);
    } else if ((tmp_obj = obj->in_obj)) {
	obj_from_obj(obj);
	GET_OBJ_WEIGHT(obj) += weight;
	obj_to_obj(obj, tmp_obj);
    } else {
	log("Unknown attempt to subtract weight from an object.");
    }
}



void name_from_drinkcon(struct obj_data *obj)
{
    int i;
    char *new_name;

    for(i=0; (*((obj->name)+i)!=' ') && (*((obj->name)+i)!='\0'); i++)  ;

    if (*((obj->name)+i)==' ') {
	new_name=strdup((obj->name)+i+1);
	free(obj->name);
	obj->name=new_name;
    }
}



void name_to_drinkcon(struct obj_data *obj,int type)
{
    char *new_name;
    extern char *drinknames[];

    CREATE(new_name,char,strlen(obj->name)+strlen(drinknames[type])+2);
    sprintf(new_name,"%s %s",drinknames[type],obj->name);
    free(obj->name);
    obj->name=new_name;
}


int do_drink(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    struct obj_data *temp;
    struct obj_info_drink *drink;
    int amount;
    int weight;


    one_argument(argument,buf);

    if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying)))
    {
	if(!(temp=get_obj_in_list_vis(ch,buf,world[ch->in_room].contents))) {
	    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	    return ERROR_MISSING_TARGET;
	} else if(temp->obj_flags.extra_flags & ITEM_TAKE) {
	    act("You must get it first!",FALSE,ch,0,0,TO_CHAR);
	    return ERROR_NO_SENSE;
	}
    }

    if (!(drink = (struct obj_info_drink *)get_obj_info(temp,ITEM_DRINKCON)))
    {
	act("You can't drink from that!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_NO_SENSE;
    }

    if(ch->physical->impairment>10 && ch->physical->thirst>0)/* drunk */
    {
	act("You simply fail to reach your mouth!", FALSE, ch, 0, 0, TO_CHAR);
	act("$n tried to drink but missed $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
	return ERROR_FAILED;
    }

    if(ch->physical->hunger>20 && ch->physical->thirst>0) /* Stomach full */
    {
	act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_FULL;
    }

    if(drink->how_full) {  /* Not empty */
        sprintf(buf,"$n drinks %s from $p",drinks[drink->drink_type]);
        act(buf, TRUE, ch, temp, 0, TO_ROOM);
        sprintf(buf,"You drink the %s.\n\r",drinks[drink->drink_type]);
        send_to_char(buf,ch);

        if (drink_aff[drink->drink_type][DRUNK] > 0 )
            amount = (25-ch->physical->thirst)/drink_aff[drink->drink_type][0];
        else    /* Why is this random? --Sman */
            amount = number(3,10);

        if(drink->how_full>0)
            amount = MIN(amount, drink->how_full);
        /*
        * You can't subtract more than the object weighs
        */
        weight = MIN(amount, temp->obj_flags.weight);

        if(drink->how_full>=0)
            weight_change_object(temp, -weight);  /* Subtract amount */

        gain_condition(ch,DRUNK,(int)((int)drink_aff
            [drink->drink_type][DRUNK]*amount)/4);

        gain_condition(ch,FULL,(int)((int)drink_aff
            [drink->drink_type][FULL]*amount)/4);

        gain_condition(ch,THIRST,(int)((int)drink_aff
            [drink->drink_type][THIRST]*amount)/4);

        if(ch->physical->impairment>10)
            act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);

        if(ch->physical->thirst>20)
            act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);

        if(ch->physical->hunger>20)
            act("You are full.",FALSE,ch,0,0,TO_CHAR);

#if 0
        if(temp->obj_flags.value[3]) /* The shit was poisoned ! */
        {
            act("Ooops, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR);
            act("$n chokes and utters some strange sounds.",
             TRUE,ch,0,0,TO_ROOM);
            af.type = SKILL_POISON;
            af.duration = amount*3;
            af.modifier = 0;
            af.location = APPLY_NONE;
            af.bitvector = AFF_POISON;
            affect_join(ch,&af, FALSE, FALSE);
        }
#endif

        if(drink->how_full<0) /* Infinite liquid */
            return OKAY;

        /* empty the container, and no longer poison. */
        drink->how_full-= amount;
        if(!drink->how_full) {  /* The last bit */
            drink->drink_type=0;
/*            temp->obj_flags.value[3]=0;*/
            name_from_drinkcon(temp);
            if(temp->obj_flags.weight < 10) {
                act("It is now empty, and it magically disappears in a puff of smoke!",FALSE,ch,0,0,TO_CHAR);
                extract_obj(temp);
            }
        }
        return OKAY;

    }

    act("It's empty already.",FALSE,ch,0,0,TO_CHAR);
    
    return ERROR_NO_SENSE;
}



int do_eat(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    struct obj_data *temp;
    struct obj_info_food *food;

    one_argument(argument,buf);

    if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying)))
    {
	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_MISSING_TARGET;
    }

    if(!(food=(struct obj_info_food *)get_obj_info(temp,ITEM_FOOD)))
    {
	act("You try fitting it in your mouth, but can't swallow it!",
	    FALSE,ch,0,0,TO_CHAR);
	act("$n sticks $p in $s mouth, but can't swallow it!",
	    TRUE,ch,temp,0,TO_ROOM);
	return ERROR_NO_SENSE;
    }

    if(ch->physical->hunger>20) /* Stomach full */
    {	
	act("You are too full to eat more!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_FULL;
    }

    act("$n eats $p",TRUE,ch,temp,0,TO_ROOM);
    act("You eat $p.",FALSE,ch,temp,0,TO_CHAR);

    gain_condition(ch,FULL,food->filling);

    if(ch->physical->hunger>20) /* Stomach full */
	act("You are full.",FALSE,ch,0,0,TO_CHAR);

#if 0
    if(temp->obj_flags.value[3]) /* The shit was poisoned ! */
    {
	act("Ooops, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR);
	act("$n coughs and utters some strange sounds.",FALSE,ch,0,0,TO_ROOM);

	af.type = SKILL_POISON;
	af.duration = temp->obj_flags.value[0]*2;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_POISON;
	affect_join(ch,&af, FALSE, FALSE);
    }
#endif

    extract_obj(temp);
    return OKAY;
}


int do_pour(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *from_obj;
    struct obj_data *to_obj;
    struct obj_info_drink *dr1,*dr2;
    int amount;

    argument_interpreter(argument, arg1, arg2);

    if(!*arg1) /* No arguments */
    {
	act("What do you want to pour from?",FALSE,ch,0,0,TO_CHAR);
	return ERROR_SYNTAX;
    }

    if(!(from_obj = get_obj_in_list_vis(ch,arg1,ch->carrying)))
    {
	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_MISSING_TARGET;
    }

    if(!(dr1=(struct obj_info_drink *)get_obj_info(from_obj,ITEM_DRINKCON)))
    {
	act("You can't pour from that!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_NO_SENSE;
    }

    if(dr1->how_full==0)
    {
	act("The $p is empty.",FALSE, ch,from_obj, 0,TO_CHAR);
	return ERROR_NO_SENSE;
    }

    if(!*arg2)
    {
	act("Where do you want it? Out or in what?",FALSE,ch,0,0,TO_CHAR);
	return ERROR_SYNTAX;
    }

    if(!str_cmp(arg2,"out"))
    {
	act("$n empties $p", TRUE, ch,from_obj,0,TO_ROOM);
	act("You empty the $p.", FALSE, ch,from_obj,0,TO_CHAR);

	weight_change_object(from_obj, -dr1->how_full); /* Empty */

	dr1->how_full=0;
	dr1->drink_type=0;
/*	from_obj->obj_flags.value[3]=0; remove poison */
	name_from_drinkcon(from_obj);
	
	return OKAY;
    }

    if(!(to_obj = get_obj_in_list_vis(ch,arg2,ch->carrying)))
    {
	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_MISSING_TARGET;
    }

    if(!(dr2=(struct obj_info_drink *)get_obj_info(to_obj,ITEM_DRINKCON)))
    {
	act("You can't pour anything into that.",FALSE,ch,0,0,TO_CHAR);
	return ERROR_NO_SENSE;
    }

    if (to_obj == from_obj) 
    {
	act("A most unproductive effort.",FALSE,ch,0,0,TO_CHAR);
	return ERROR_NO_SENSE;
    }

    if((dr2->how_full!=0)&& (dr2->drink_type!=dr1->drink_type))
    {
	act("There is already another liquid in it!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_NO_SENSE;
    }

    if(!(dr2->how_full<dr2->capacity))
    {
	act("There is no room for more.",FALSE,ch,0,0,TO_CHAR);
	return ERROR_FULL;
    }

    sprintf(buf,"You pour the %s into the %s.",
	drinks[dr1->drink_type],arg2);
    send_to_char(buf,ch);

    /* New alias */
    if (dr2->how_full==0) 
	name_to_drinkcon(to_obj,dr1->drink_type);

    /* First same type liq. */
    dr2->drink_type=dr1->drink_type;

    /* Then how much to pour */
    dr1->how_full -= (amount = (dr2->capacity-dr2->how_full));

    dr2->how_full=dr2->capacity;

#if 0
    /* Then the poison boogie */
    to_obj->obj_flags.value[3]=
	(to_obj->obj_flags.value[3]||from_obj->obj_flags.value[3]);
#endif

    if(dr1->how_full<0)    /* There was to little */
    {
	dr2->how_full+=dr1->how_full;
	amount += dr1->how_full;
	dr1->how_full=0;
	dr1->drink_type=0;
/*	from_obj->obj_flags.value[3]=0;*/
	name_from_drinkcon(from_obj);
    }

    /* And the weight boogie */

    weight_change_object(from_obj, -amount);
    weight_change_object(to_obj, amount);   /* Add weight */

    return OKAY;
}

int do_fill(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *from_obj;
    struct obj_data *to_obj;
    struct obj_info_drink *dr1,*dr2;
    int amount;

    argument_interpreter(argument, arg1, arg2);

    if(!*arg1) /* No arguments */
    {
	act("What do you want to fill?",FALSE,ch,0,0,TO_CHAR);
	return ERROR_SYNTAX;
    }

    if(!(to_obj = get_obj_in_list_vis(ch,arg1,ch->carrying)))
    {
	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_MISSING_TARGET;
    }

    if(!(dr1=(struct obj_info_drink *)get_obj_info(to_obj,ITEM_DRINKCON)))
    {
	act("You can't fill that!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_NO_SENSE;
    }

    if(dr1->how_full==dr1->capacity)
    {
	act("The $p is full.",FALSE, ch,to_obj, 0,TO_CHAR);
	return ERROR_FULL;
    }

    if(!*arg2)
    {
	/* This will be for obvious in-room containers */
	act("For the moment, you need to type the source.",FALSE,ch,0,0,TO_CHAR);
	return ERROR_SYNTAX;
    }

    if(!(from_obj = get_obj_in_list_vis(ch,arg2,ch->carrying)))
    {
	if(!(from_obj = get_obj_in_list_vis(ch,arg2,world[ch->in_room].contents))) {
	    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	    return ERROR_MISSING_TARGET;
	} else if(from_obj->obj_flags.extra_flags & ITEM_TAKE) {
	    act("You must get it first!",FALSE,ch,0,0,TO_CHAR);
	    return ERROR_NO_SENSE;
	}
    }

    if(!(dr2=(struct obj_info_drink *)get_obj_info(from_obj,ITEM_DRINKCON)))
    {
	act("You can't get anything out of that.",FALSE,ch,0,0,TO_CHAR);
	return ERROR_NO_SENSE;
    }

    if (to_obj == from_obj) 
    {
	act("A most unproductive effort.",FALSE,ch,0,0,TO_CHAR);
	return ERROR_NO_SENSE;
	}

    if((dr1->how_full!=0)&& (dr1->drink_type!=dr2->drink_type))
    {
	act("There is already another liquid in it!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_NO_SENSE;
    }

    if(!(dr1->how_full<dr1->capacity) || dr1->how_full < 0)
    {
	act("There is no room for more.",FALSE,ch,0,0,TO_CHAR);
	return ERROR_FULL;
    }

    sprintf(buf,"You fill the %s with the %s.",arg1,
	drinks[dr2->drink_type]);
    act(buf,FALSE,ch,0,0,TO_CHAR);

    /* New alias */
    if (dr1->how_full==0) 
	name_to_drinkcon(to_obj,dr2->drink_type);

    /* First same type liq. */
    dr1->drink_type=dr2->drink_type;

#if 0
    /* Then the poison boogie */
    to_obj->obj_flags.value[3]=
	(to_obj->obj_flags.value[3]||from_obj->obj_flags.value[3]);
#endif

    /* Then how much to pour */
    if(dr2->how_full > 0) {
	dr2->how_full -= (amount = (dr1->capacity-dr1->how_full));

	dr1->how_full=dr1->capacity;

	if(dr2->how_full)    /* There was too little */
	{
	    dr1->how_full += dr2->how_full;
	    amount += dr2->how_full;
	    dr2->how_full=0;
	    dr2->drink_type=0;
/*	    from_obj->obj_flags.value[3]=0;*/
	    name_from_drinkcon(from_obj);
	}
    } else {
	amount=dr1->capacity-dr1->how_full;
	dr1->how_full+=amount;
    }


    /* And the weight boogie */

    if(dr2->how_full>=0)
	weight_change_object(from_obj, -amount);
    weight_change_object(to_obj, amount);   /* Add weight */

    return OKAY;
}

int do_sip(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *temp;
    struct obj_info_drink *drink;

    one_argument(argument,arg);

    if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
    {
	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_MISSING_TARGET;
    }

    if (!(drink = (struct obj_info_drink *)get_obj_info(temp,ITEM_DRINKCON)))
    {
	act("You can't sip from that!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_NO_SENSE;
    }

    if(ch->physical->impairment>10) /* The pig is drunk ! */
    {
	act("You simply fail to reach your mouth!",FALSE,ch,0,0,TO_CHAR);
	act("$n tries to sip, but fails!",TRUE,ch,0,0,TO_ROOM);
	return ERROR_FAILED;
    }

    if(!drink->how_full)  /* Empty */
    {
	act("But there is nothing in it?",FALSE,ch,0,0,TO_CHAR);
	return ERROR_NO_SENSE;
    }

    act("$n sips from the $o",TRUE,ch,temp,0,TO_ROOM);
    sprintf(buf,"It tastes like %s.\n\r",drinks[drink->drink_type]);
    send_to_char(buf,ch);

    gain_condition(ch,DRUNK,(int)(drink_aff[drink->drink_type][DRUNK]/4));

    gain_condition(ch,FULL,(int)(drink_aff[drink->drink_type][FULL]/4));

    gain_condition(ch,THIRST,(int)(drink_aff[drink->drink_type][THIRST]/4));

    if(drink->how_full>0) {
	weight_change_object(temp, -1);  /* Subtract one unit */
	drink->how_full--;
    }


    if(ch->physical->impairment>10)
	act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);

    if(ch->physical->thirst>20)
	act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);

    if(ch->physical->hunger>20)
	act("You are full.",FALSE,ch,0,0,TO_CHAR);

#if 0
    if(temp->obj_flags.value[3]/*&&!IS_AFFECTED(ch,AFF_POISON)*/) /* The shit was poisoned ! */
    {
	act("But it also had a strange taste!",FALSE,ch,0,0,TO_CHAR);

	af.type = SKILL_POISON;
	af.duration = 3;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_POISON;
	affect_to_char(ch,&af);
    }
#endif

    if(!drink->how_full)  /* The last bit */
    {
	drink->drink_type=0;
/*	temp->obj_flags.value[3]=0;*/
	name_from_drinkcon(temp);
    }

    return OKAY;

}


int do_taste(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    struct obj_data *temp;
    struct obj_info_food *food;

    one_argument(argument,arg);

    if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
    {
	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_MISSING_TARGET;
    }

    if (get_obj_info(temp,ITEM_DRINKCON))
    {
	return do_sip(ch,argument,0);
    }

    if(!(food=(struct obj_info_food *)get_obj_info(temp,ITEM_FOOD)))
    {
	act("Taste that?!? Your stomach refuses!",FALSE,ch,0,0,TO_CHAR);
	return ERROR_NO_SENSE;
    }

    act("$n tastes the $o", FALSE, ch, temp, 0, TO_ROOM);
    act("You taste the $o", FALSE, ch, temp, 0, TO_CHAR);

    gain_condition(ch,FULL,1);

    if(ch->physical->hunger>20)
	act("You are full.",FALSE,ch,0,0,TO_CHAR);

#if 0
    if(temp->obj_flags.value[3]/*&&!IS_AFFECTED(ch,AFF_POISON)*/) /* The shit was poisoned ! */
    {
	act("Ooops, it did not taste good at all!",FALSE,ch,0,0,TO_CHAR);

	af.type = SKILL_POISON;
	af.duration = 2;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_POISON;
	affect_to_char(ch,&af);
    }
#endif

    if(food->filling>0)
	food->filling--;

    if(!food->filling)	/* Nothing left */
    {
	act("There is nothing left now.",FALSE,ch,0,0,TO_CHAR);
	extract_obj(temp);
    }

    return OKAY;

}



/* functions related to wear */

void perform_wear(struct char_data *ch, struct obj_data *obj_object, int use)
{
    struct obj_data *old;

    /* Handle some silly little things to keep us sane */
    if((use==WEAR_FINGER || use==WEAR_FINGER_L || use==WEAR_FINGER_R) &&
	    (old=get_equip_used(ch,WEAR_HANDS))) {
	act("You'll need to take off $p first.",FALSE,ch,old,0,TO_CHAR);
	return;
    }

    /* Find out if there's room to wear it */
    if(use<0)
	switch(use) {
	    case HOLD:
		if(get_equip_used(ch,HOLD_HAND1) &&
		 get_equip_used(ch,HOLD_HAND2)) {
		    send_to_char("Your hands are full.\n\r", ch);
		    return;
		}
		break;
	    case WEAR_FINGER:
		if(get_equip_used(ch,WEAR_FINGER_L) &&
		 get_equip_used(ch,WEAR_FINGER_R)) {
		    send_to_char("Your fingers are occupied.\n\r", ch);
		    return;
		}
		break;
	    case WEAR_NECK:
		if(get_equip_used(ch,WEAR_NECK_1) &&
		 get_equip_used(ch,WEAR_NECK_2)) {
		    send_to_char("You would strangle yourself with so much around your neck.\n\r", ch);
		    return;
		}
		break;
	    case WEAR_WRIST:
		if(get_equip_used(ch,WEAR_WRIST_L) &&
		 get_equip_used(ch,WEAR_WRIST_R)) {
		    send_to_char("Your wrists are full.\n\r", ch);
		    return;
		}
		break;
	}
    else if((old=get_equip_used(ch,use))) {
	act("You've already got $p there.",FALSE, ch, old, 0,TO_CHAR);
	return;
    }

    /* See if the use matches the kind of object - if not, wear it */
    switch(use) {
	case HOLD:
	case HOLD_HAND1:
	case HOLD_HAND2:
	    if(!can_wear(obj_object,ITEM_HOLD)) {
		act("You can't hold $p.",FALSE,ch,obj_object,0,TO_CHAR);
		return;
	    }
	    if(use==HOLD) {
		if(get_equip_used(ch,HOLD_HAND1))
		    use=HOLD_HAND2;
		else
		    use=HOLD_HAND1;
	    }
	    act("$n grasps $p.", TRUE, ch, obj_object,0,TO_ROOM);
	    break;
	case WEAR_FINGER:
	case WEAR_FINGER_L:
	case WEAR_FINGER_R:
	    if(!can_wear(obj_object,ITEM_WEAR_FINGER)) {
		act("You can't wear $p on your finger.",FALSE,ch,obj_object,0,TO_CHAR);
		return;
	    }
	    if(use==WEAR_FINGER) {
		if(get_equip_used(ch,WEAR_FINGER_L))
		    use=WEAR_FINGER_R;
		else
		    use=WEAR_FINGER_L;
	    }
	    act("$n wears $p on $s finger.", TRUE, ch, obj_object,0,TO_ROOM);
	    break;
	case WEAR_NECK:
	case WEAR_NECK_1:
	case WEAR_NECK_2:
	    if(!can_wear(obj_object,ITEM_WEAR_NECK)) {
		act("You can't wear $p around your neck.",FALSE,ch,obj_object,0,TO_CHAR);
		return;
	    }
	    if(use==WEAR_NECK) {
		if(get_equip_used(ch,WEAR_NECK_1))
		    use=WEAR_NECK_2;
		else
		    use=WEAR_NECK_1;
	    }
	    act("$n wears $p around $s neck.", TRUE, ch, obj_object,0,TO_ROOM);
	    break;
	case WEAR_BODY:
	    if(!can_wear(obj_object,ITEM_WEAR_BODY)) {
		act("You can't wear $p on your body.",FALSE,ch,obj_object,0,TO_CHAR);
		return;
	    }
	    act("$n wears $p on $s body.", TRUE, ch, obj_object,0,TO_ROOM);
	    break;
	case WEAR_HEAD:
	    if(!can_wear(obj_object,ITEM_WEAR_HEAD)) {
		act("You can't wear $p on your head.",FALSE,ch,obj_object,0,TO_CHAR);
		return;
	    }
	    act("$n wears $p on $s head.",TRUE, ch, obj_object,0,TO_ROOM);
	    break;
	case WEAR_LEGS:
	    if(!can_wear(obj_object,ITEM_WEAR_LEGS)) {
		act("You can't wear $p on your legs.",FALSE,ch,obj_object,0,TO_CHAR);
		return;
	    }
	    act("$n wears $p on $s legs.",TRUE, ch, obj_object,0,TO_ROOM);
	    break;
	case WEAR_FEET:
	    if(!can_wear(obj_object,ITEM_WEAR_FEET)) {
		act("You can't wear $p on your feet.",FALSE,ch,obj_object,0,TO_CHAR);
		return;
	    }
	    act("$n wears $p on $s feet.",TRUE, ch, obj_object,0,TO_ROOM);
	    break;
	case WEAR_HANDS:
	    if(!can_wear(obj_object,ITEM_WEAR_HANDS)) {
		act("You can't wear $p on your hands.",FALSE,ch,obj_object,0,TO_CHAR);
		return;
	    }
	    act("$n wears $p on $s hands.",TRUE, ch, obj_object,0,TO_ROOM);
	    break;
	case WEAR_ARMS:
	    if(!can_wear(obj_object,ITEM_WEAR_ARMS)) {
		act("You can't wear $p on your arms.",FALSE,ch,obj_object,0,TO_CHAR);
		return;
	    }
	    act("$n wears $p on $s arms.",TRUE, ch, obj_object,0,TO_ROOM);
	    break;
	case WEAR_ABOUT:
	    if(!can_wear(obj_object,ITEM_WEAR_ABOUT)) {
		act("You can't wear $p about your body.",FALSE,ch,obj_object,0,TO_CHAR);
		return;
	    }
	    act("$n wears $p about $s body.",TRUE, ch, obj_object,0,TO_ROOM);
	    break;
	case WEAR_WAIST:
	    if(!can_wear(obj_object,ITEM_WEAR_WAISTE)) {
		act("You can't wear $p on your waist.",FALSE,ch,obj_object,0,TO_CHAR);
		return;
	    }
	    act("$n wears $p about $s waist.",TRUE, ch, obj_object,0,TO_ROOM);
	    break;
	case WEAR_WRIST:
	case WEAR_WRIST_L:
	case WEAR_WRIST_R:
	    if(!can_wear(obj_object,ITEM_WEAR_WRIST)) {
		act("You can't wear $p on your wrist.",FALSE,ch,obj_object,0,TO_CHAR);
		return;
	    }
	    if(use==WEAR_WRIST) {
		if(get_equip_used(ch,WEAR_WRIST_L))
		    use=WEAR_WRIST_R;
		else
		    use=WEAR_WRIST_L;
	    }
	    act("$n wears $p around $s wrist.",TRUE, ch, obj_object,0,TO_ROOM);
	    break;
    }

    /* If we get here, it's okay to wear */
    obj_from_char(obj_object);
    equip_char(ch,obj_object,use);
    act("Ok.",FALSE,ch,0,0,TO_CHAR);
}

int guess_location(struct obj_data *obj_object)
{
    int keyword=-99;

    if (can_wear(obj_object,ITEM_WEAR_FINGER))
	keyword = WEAR_FINGER;
    if (can_wear(obj_object,ITEM_WEAR_NECK))
	keyword = WEAR_NECK;
    if (can_wear(obj_object,ITEM_WEAR_WRIST))
	keyword = WEAR_WRIST;
    if (can_wear(obj_object,ITEM_WEAR_WAISTE))
	keyword = WEAR_WAIST;
    if (can_wear(obj_object,ITEM_WEAR_ARMS))
	keyword = WEAR_ARMS;
    if (can_wear(obj_object,ITEM_WEAR_HANDS))
	keyword = WEAR_HANDS;
    if (can_wear(obj_object,ITEM_WEAR_FEET))
	keyword = WEAR_FEET;
    if (can_wear(obj_object,ITEM_WEAR_LEGS))
	keyword = WEAR_LEGS;
    if (can_wear(obj_object,ITEM_WEAR_ABOUT))
	keyword = WEAR_ABOUT;
    if (can_wear(obj_object,ITEM_WEAR_HEAD))
	keyword = WEAR_HEAD;
    if (can_wear(obj_object,ITEM_WEAR_BODY))
	keyword = WEAR_BODY;

    return(keyword);
}

int do_wear(struct char_data *ch, char *argument, int cmd) {
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[256];
    char buffer[MAX_STRING_LENGTH];
    struct obj_data *obj_object, *next_obj;
    int keyword;
    static char *keywords[] = {
	"finger",
	"neck",
	"body",
	"head",
	"legs",
	"feet",
	"hands",
	"arms",
	"about",
	"waist",
	"wrist",
	"shield",
	"\n"
    };

    argument_interpreter(argument, arg1, arg2);
    if (*arg1 && str_cmp(arg1, "all")) {
	obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
	if (obj_object) {
	    if (*arg2) {
		keyword = search_block(arg2, keywords, FALSE);
		/* Partial Match */
		if (keyword == -1) {
		    sprintf(buf, "%s is an unknown body location.\n\r", arg2);
		    send_to_char(buf, ch);
		} else {
		    perform_wear(ch, obj_object, keyword+1);
		}
	    } else {
		keyword = guess_location(obj_object);

		if(keyword==-99) {
		    send_to_char("You can't wear that.\n\r",ch);
		} else
		    perform_wear(ch, obj_object, keyword);
	    }
	} else {
	    sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
	    send_to_char(buffer,ch);
            return ERROR_MISSING_TARGET;
	}
        return OKAY;
    }

    if(!arg1 || str_cmp(arg1, "all")){
	send_to_char("Wear what?\n\r", ch);
        return ERROR_SYNTAX;
    }

    /* WEAR ALL */
    for(obj_object= ch->carrying;obj_object;obj_object= next_obj) {
        next_obj=obj_object->next_content;
        keyword = guess_location(obj_object);

        if(keyword == -99)
            send_to_char("You can't wear that.\n\r",ch);
         else
            perform_wear(ch, obj_object, keyword);
    }
    return OKAY;
}


int do_wield(struct char_data *ch, char *argument, int cmd) {
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    struct obj_data *obj_object;

    argument_interpreter(argument, arg1, arg2);

    if (!*arg1) {
	send_to_char("Wield what?\n\r", ch);
        return ERROR_SYNTAX;
    }

    obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
    if (!obj_object) {
        sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
        send_to_char(buffer,ch);
        return ERROR_MISSING_TARGET;
    }
    perform_wear(ch, obj_object, HOLD);
    return OKAY;
}


int do_grab(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    struct obj_data *obj_object;


    argument_interpreter(argument, arg1, arg2);

    if (!*arg1) {
	send_to_char("Hold what?\n\r", ch);
        return ERROR_SYNTAX;
    }

    obj_object = get_obj_in_list(arg1, ch->carrying);
    if (!obj_object) {
        sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
        send_to_char(buffer,ch);
        return ERROR_MISSING_TARGET;
    }
    perform_wear(ch, obj_object, HOLD);
    return OKAY;
}


int do_remove(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    struct obj_data *obj_object;

    one_argument(argument, arg1);

    if (!*arg1) {
	send_to_char("Remove what?\n\r", ch);
        return ERROR_SYNTAX;
    }

    obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
    if (!obj_object) {
        send_to_char("You are not using it.\n\r", ch);
        return ERROR_MISSING_TARGET;
    }

    if(obj_object->equipped_as==UNEQUIPPED) {
        act("$p isn't equipped by you.",FALSE,ch,obj_object,0,TO_CHAR);
        return ERROR_ALREADY_DONE;
    }
    unequip_char(ch, obj_object);

    act("You stop using $p.",FALSE,ch,obj_object,0,TO_CHAR);
    act("$n stops using $p.",TRUE,ch,obj_object,0,TO_ROOM);

    return OKAY;
}

int do_search(struct char_data *ch,char *argument, int cmd)
{
    struct obj_data *k;
    bool found_something=FALSE;
    char name[MAX_STRING_LENGTH];
    struct obj_info_container *cont;

    one_argument(argument,name);

    if(!*name) {  /* No argument: search room */
	k=world[ch->in_room].contents;
    } else {
	generic_find(name,FIND_OBJ_INV|FIND_OBJ_ROOM,ch,(void *)&k);
	if(!k) {
	    send_to_char("Search what?\n\r",ch);
	    return ERROR_MISSING_TARGET;
	} else if(!(cont=(struct obj_info_container *)
                    get_obj_info(k,ITEM_CONTAINER))) {
	    send_to_char("There's no way to search that.\n\r",ch);
	    return ERROR_NO_SENSE;
	}
	k=k->contains;
    }

    for(;k;k=k->next_content)
	if(IS_SET(k->obj_flags.extra_flags,ITEM_SECRET) &&
		number(1,25)<GET_INT(ch)) {
	    REMOVE_BIT(k->obj_flags.extra_flags,ITEM_SECRET);
	    if(CAN_SEE_OBJ(ch,k)) { /* Was it found? */
		act("You find $p!",FALSE,ch,k,0,TO_CHAR);
		act("$n finds $p!",FALSE,ch,k,0,TO_ROOM);
		found_something=TRUE;
	    } else  /* Make it secret again */
		SET_BIT(k->obj_flags.extra_flags,ITEM_SECRET);
	}

    if(!found_something)
	send_to_char("You don't find anything you didn't see before.\n\r",ch);

    return OKAY;
}
