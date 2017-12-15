/* ************************************************************************
*  file: act.obj1.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Commands mainly moving around objects.                         *
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
	
/* extern functions */
char *fname(char *namelist);
int isname(char *arg, char *arg2);
bool put(struct char_data *ch,struct obj_data *obj_object,
    struct obj_data *sub_object,bool showit);


/* procedures related to get */
void get(struct char_data *ch, struct obj_data *obj_object, 
    struct obj_data *sub_object,bool showit)
{
    struct obj_data *i;
    struct obj_info_money *om1,*om2;

    if (sub_object) {
	obj_from_obj(obj_object);
	obj_to_char(obj_object, ch);
	if (sub_object->carried_by == ch) {
	    act("You get $p from $P.", 0, ch, obj_object, sub_object,
		TO_CHAR);
	    if(showit)
		act("$n gets $p from $s $P.", 1, ch, obj_object, sub_object, TO_ROOM);
	} else {
	    act("You get $p from $P.", 0, ch, obj_object, sub_object,
		TO_CHAR);

	    if(showit)
		act("$n gets $p from $P.", 1, ch, obj_object, sub_object, TO_ROOM);
	}
    } else {
	obj_from_room(obj_object);
	obj_to_char(obj_object, ch);
	act("You get $p.", 0, ch, obj_object, 0, TO_CHAR);
	if(showit)
	    act("$n gets $p.", 1, ch, obj_object, 0, TO_ROOM);
    }

    if(obj_object->info && obj_object->info->obj_type==ITEM_MONEY &&
            !obj_object->info->next) { /** candidate for combination **/
        for(i=ch->carrying;i;i=i->next)
            if(i!=obj_object && i->info && i->info->obj_type==ITEM_MONEY &&
                    !i->info->next) { /* Found a possible match */
                om1=(struct obj_info_money *)i->info;
                om2=(struct obj_info_money *)obj_object->info;
                if(om1->money_type==om2->money_type) { /* exact match */
                    om1->amount += om2->amount;
                    
                    obj_from_char(obj_object);
	            extract_obj(obj_object);
	            send_to_char("You add the money to your pile.\n\r",ch);
                    break;
                }
            }
    }
    save_char(ch);
}


int do_get(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    struct obj_data *sub_object;
    struct obj_data *obj_object;
    struct obj_data *next_obj;
    struct obj_data *remember;
    bool found = FALSE;
    bool fail  = FALSE;
    int type   = 3;
    int total=0;
    bool alldot = FALSE;
    char allbuf[MAX_STRING_LENGTH];
    struct obj_info_container *cont;


    argument_interpreter(argument, arg1, arg2);

    /* get type */
    if (!*arg1) {
	type = 0;
    }
    if (*arg1 && !*arg2) {
	alldot = FALSE;
	allbuf[0] = '\0';
	if ((str_cmp(arg1, "all") != 0) &&
	  (sscanf(arg1, "all.%s", allbuf) != 0)) {
	    strcpy(arg1, "all");
	    alldot = TRUE;
	}
	if (!str_cmp(arg1,"all")) {
	    type = 1;
	} else {
	    type = 2;
	}
    }

    if (*arg1 && *arg2) {
	alldot = FALSE;
	allbuf[0] = '\0';
	if ((str_cmp(arg1,"all") != 0) &&
	  (sscanf(arg1, "all.%s", allbuf) != 0)) {
	    strcpy(arg1, "all");
	    alldot = TRUE;
	}
	if (!str_cmp(arg1,"all")) {
	    if (!str_cmp(arg2,"all")) {
		type = 3;
	    } else {
		type = 4;
	    }
	} else {
	    if (!str_cmp(arg2,"all")) {
		type = 5;
	    } else {
		type = 6;
	    }
	}
    }

    switch (type) {
	/* get */
	case 0:{ 
	    send_to_char("Get what?\n\r", ch); 
            return ERROR_SYNTAX;
	} break;
	/* get all */
	case 1:{ 
	    sub_object = 0;
	    found = FALSE;
	    fail = FALSE;
	    for(obj_object = world[ch->in_room].contents;
		obj_object;
		obj_object = next_obj) {
		next_obj = obj_object->next_content;

		/* IF all.obj, only get those named "obj" */
		if (alldot && !isname(allbuf,obj_object->name)) {
		 continue;
		} /* if */

		if (CAN_SEE_OBJ(ch,obj_object)) {
		    if ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)) {
			if ((IS_CARRYING_W(ch)+obj_object->obj_flags.weight) <=
                                CAN_CARRY_W(ch)) {
			    if (obj_object->obj_flags.extra_flags & ITEM_TAKE) {
				get(ch,obj_object,sub_object,TRUE);
				total++;
			    } else {
				send_to_char("You can't take that\n\r", ch);
				fail = TRUE;
			    }
			} else {
			    sprintf(buffer,"%s : You can't carry that much weight.\n\r", 
				fname(obj_object->name));
				send_to_char(buffer, ch);
			    fail = TRUE;
			}
		    } else {
			sprintf(buffer,"%s : You can't carry that many items.\n\r", 
			    fname(obj_object->name));
			send_to_char(buffer, ch);
			fail = TRUE;
		    }
		}
	    }
	    if (total) {
		sprintf(buffer,"You got %d item(s).\n\r",total);
		send_to_char(buffer, ch);
                return OKAY;
	    } else {
		if (!fail) send_to_char("You see nothing here.\n\r", ch);
                    return ERROR_MISSING_TARGET;
	    }
	} break;
	/* get ??? */
	case 2:{
	    sub_object = 0;
	    found = FALSE;
	    fail	= FALSE;
	    obj_object = get_obj_in_list_vis(ch, arg1, 
		world[ch->in_room].contents);
	    if (obj_object) {
		if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
		    if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < 
			CAN_CARRY_W(ch)) {
			if (obj_object->obj_flags.extra_flags & ITEM_TAKE) {
			    get(ch,obj_object,sub_object,TRUE);
			    found = TRUE;
			} else {
			    send_to_char("You can't take that\n\r", ch);
			    fail = TRUE;
			}
		    } else {
			sprintf(buffer,"%s : You can't carry that much weight.\n\r", 
			    fname(obj_object->name));
			send_to_char(buffer, ch);
			fail = TRUE;
		    }
		} else {
		    sprintf(buffer,"%s : You can't carry that many items.\n\r", 
			fname(obj_object->name));
		    send_to_char(buffer, ch);
		    fail = TRUE;
		}
	    } else {
		sprintf(buffer,"You do not see a %s here.\n\r", arg1);
		send_to_char(buffer, ch);
		fail = TRUE;
	    }
	} break;
	/* get all all */
	case 3:{ 
	    send_to_char("You must be joking?!\n\r", ch);
            return ERROR_NO_SENSE;
	} break;
	/* get all ??? */
	case 4:{
	    found = FALSE;
	    fail = FALSE; 
	    sub_object = get_obj_in_list_vis(ch, arg2, 
		world[ch->in_room].contents);
	    if (!sub_object){
		sub_object = get_obj_in_list_vis(ch, arg2, ch->carrying);
	    }
	    if (sub_object) {
		if((cont=(struct obj_info_container *)
                            get_obj_info(sub_object,ITEM_CONTAINER))) {
		    if (IS_SET(cont->lock_state, CONT_CLOSED)) {
		     sprintf(buffer,"The %s is closed.\n\r", fname(sub_object->name));
		     send_to_char(buffer, ch);
		     return ERROR_FAILED;
		    } /* if */
		    for(obj_object = sub_object->contains;
			obj_object;
			obj_object = next_obj) {
			next_obj = obj_object->next_content;
			
			/* IF all.obj, only get those named "obj" */
			if (alldot && !isname(allbuf,obj_object->name) ) {
			 continue;
			} /* if */

			if (CAN_SEE_OBJ(ch,obj_object)) {
			    if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
				if (((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < CAN_CARRY_W(ch)) || sub_object->carried_by==ch) {
				    if (obj_object->obj_flags.extra_flags,ITEM_TAKE) {
					get(ch,obj_object,sub_object,FALSE);
					total++;
					remember=obj_object;
				    } else {
					send_to_char("You can't take that\n\r", ch);
					fail = TRUE;
				    }
				} else {
				    sprintf(buffer,"%s : You can't carry that much weight.\n\r", 
					fname(obj_object->name));
				    send_to_char(buffer, ch);
				    fail = TRUE;
				}
			    } else {
				sprintf(buffer,"%s : You can't carry that many items.\n\r", 
				    fname(obj_object->name));
				send_to_char(buffer, ch);
				fail = TRUE;
			    }
			}
		    }
		    if (!total && !fail) {
			sprintf(buffer,"You do not see anything in the %s.\n\r", 
			    fname(sub_object->name));
			send_to_char(buffer, ch);
			fail = TRUE;
		    } else if(total==1)
			act("$n gets $o",TRUE,ch,remember,0,TO_ROOM);
		    else if(total>1 && total <6)
			act("$n gets some stuff from $o.",TRUE,ch,sub_object,0,TO_ROOM);
		    else if(total>5)
			act("$n gets a bunch of stuff from $o.",TRUE,ch,sub_object,0,TO_ROOM);
		} else {
		    sprintf(buffer,"The %s is not a container.\n\r",
			fname(sub_object->name));
		    send_to_char(buffer, ch);
		    fail = TRUE;
		}
	    } else { 
		sprintf(buffer,"You do not see or have the %s.\n\r", arg2);
		send_to_char(buffer, ch);
		fail = TRUE;
                return ERROR_MISSING_TARGET;
	    }
	} break;
	case 5:{ 
	    send_to_char("You can't take a thing from more than one container.\n\r", 
		ch);
            return ERROR_NO_SENSE;
	} break;
	case 6:{
	    found = FALSE;
	    fail = FALSE;
	    sub_object = get_obj_in_list_vis(ch, arg2, 
		world[ch->in_room].contents);
	    if (!sub_object){
		sub_object = get_obj_in_list_vis(ch, arg2, ch->carrying);
	    }
	    if (sub_object) {
		if((cont=(struct obj_info_container *)
                            get_obj_info(sub_object,ITEM_CONTAINER))) {
		    if (IS_SET(cont->lock_state, CONT_CLOSED)) {
		     sprintf(buffer,"The %s is closed.\n\r", fname(sub_object->name));
		     send_to_char(buffer, ch);
		     return ERROR_PHYS_PREVENTS;
		    } /* if */
		    obj_object = get_obj_in_list_vis(ch, arg1, sub_object->contains);
		    if (obj_object) {
			if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
			    if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < CAN_CARRY_W(ch) || sub_object->carried_by==ch) {
				if (obj_object->obj_flags.extra_flags &
ITEM_TAKE) {
				    get(ch,obj_object,sub_object,TRUE);
				    found = TRUE;
				} else {
				    send_to_char("You can't take that\n\r", ch);
				    fail = TRUE;
				}
			    } else {
				sprintf(buffer,"%s : You can't carry that much weight.\n\r", 
				    fname(obj_object->name));
				send_to_char(buffer, ch);
				fail = TRUE;
			    }
			} else {
			    sprintf(buffer,"%s : You can't carry that many items.\n\r", 
				fname(obj_object->name));
			    send_to_char(buffer, ch);
			    fail = TRUE;
			}
		    } else {
			sprintf(buffer,"The %s does not contain the %s.\n\r", 
			    fname(sub_object->name), arg1);
			send_to_char(buffer, ch);
			fail = TRUE;
		    }
		} else {
		    sprintf(buffer,"The %s is not a container.\n\r", fname(sub_object->name));
		    send_to_char(buffer, ch);
		    fail = TRUE;
		}
	    } else {
		sprintf(buffer,"You do not see or have the %s.\n\r", arg2);
		send_to_char(buffer, ch);
		fail = TRUE;
	    }
	} break;
    }
    return ERROR_INTERNAL; /* that's what I'd call it - yuccch */
}


int do_drop(struct char_data *ch, char *argument, int cmd) {
    char arg[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    struct obj_data *tmp_object;
    struct obj_data *next_obj;
    bool test = FALSE;

    argument=one_argument(argument, arg);
#if 0
/* Should we bother? */
    if(is_number(arg))
    {
	if(strlen(arg)>7){
	    send_to_char("Number field too big.\n\r",ch);
	    return ERROR_SYNTAX;
	}
	amount = atoi(arg);
	argument=one_argument(argument,arg);
	if (str_cmp("coins",arg) && str_cmp("coin",arg))
	{
	    send_to_char("Sorry, you can't do that (yet)...\n\r",ch);
	    return ERROR_INTERNAL;
	}
	if(amount<0)
	{
	    send_to_char("Sorry, you can't do that!\n\r",ch);
	    return ERROR_NO_SENSE;
	}
/*		if(GET_GOLD(ch)<amount)
	{
	    send_to_char("You haven't got that many coins!\n\r",ch);
	    return ERROR_NO_SENSE;
	}*/
	send_to_char("OK.\n\r",ch);
	if(amount==0)
	    return OKAY;
	
	act("$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM);
	if(amount>50000) {
	    sprintf(buffer,"WIZ: Large transaction: %s drops %d coins",GET_NAME(ch),amount);
	    log(buffer);
	}
/*		if(0!=(tmp_object=
	 get_obj_in_list("gold", world[ch->in_room].contents))){
	    amount+=tmp_object->obj_flags.value[0];
	    extract_obj(tmp_object);
	    send_to_char("You add your gold to the pile.\n\r",ch);
	}
	obj_to_room(tmp_object,ch->in_room);*/
	return OKAY;
    }
#endif

    if (*arg) {
	if (!str_cmp(arg,"all")) {
	    for(tmp_object = ch->carrying;
		tmp_object;
		tmp_object = next_obj) {
		next_obj = tmp_object->next_content;
                if (tmp_object->equipped_as!=UNEQUIPPED)
                    continue;
		if (!IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) {
		    if (CAN_SEE_OBJ(ch, tmp_object)) {
			sprintf(buffer, "You drop the %s.\n\r", fname(tmp_object->name));
			send_to_char(buffer, ch);
		    } else {
			send_to_char("You drop something.\n\r", ch);
		    }
		    act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
		    obj_from_char(tmp_object);
		    obj_to_room(tmp_object,ch->in_room);
		    test = TRUE;
		} else {
		    if (CAN_SEE_OBJ(ch, tmp_object)) {
			sprintf(buffer, "You can't drop the %s, it must be CURSED!\n\r", fname(tmp_object->name));
			send_to_char(buffer, ch);
			test = TRUE;
		    }
		}
	    }
	    if (!test) {
		send_to_char("You do not seem to have anything.\n\r", ch);
	    }
    } else {
	    tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
	    if (tmp_object) {
		if (! IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) {
		    sprintf(buffer, "You drop the %s.\n\r", fname(tmp_object->name));
		    send_to_char(buffer, ch);
		    act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
		    obj_from_char(tmp_object);
		    obj_to_room(tmp_object,ch->in_room);
		} else {
		    send_to_char("You can't drop it, it must be CURSED!\n\r", ch);
		}
	    } else {
		send_to_char("You do not have that item.\n\r", ch);
                return ERROR_MISSING_TARGET;
	    }
	}
    } else {
	send_to_char("Drop what?\n\r", ch);
        return ERROR_SYNTAX;
    }
    return ERROR_INTERNAL; /* that's what I'd call it - yuccch */
}

int do_putalldot(struct char_data *ch, char *name, char *target, int cmd)
{
    struct obj_data *tmp_object;
    struct obj_data *next_object;
    struct obj_data *obj_into;
    struct obj_data *remember;
    char buf[MAX_STRING_LENGTH];
    int bits, total=0;

    bits=generic_find(target,FIND_OBJ_INV|FIND_OBJ_ROOM,ch,(void *)&obj_into);
    if(!obj_into) {
	send_to_char("Into what?",ch);
	return ERROR_SYNTAX;
    }

    /* If "put all.object bag", get all carried items
    * named "object", and put each into the bag.
    */
    for (tmp_object = ch->carrying; tmp_object;
      tmp_object = next_object) {
      next_object = tmp_object->next_content;
     if (isname(name, tmp_object->name) ) {
	if(put(ch,tmp_object,obj_into,FALSE)) {
	    remember=tmp_object;
	    total++;
	} else /* Let them see why */
	    put(ch,tmp_object,obj_into,TRUE);
     }
  	}	      
    if (total) {
	sprintf(buf,"You put %d %s(s) in $o",total,name);
	act(buf,FALSE,ch,obj_into,0,TO_CHAR);
	if(total == 1)
	    strcpy(buf,"$n puts $O in $o.");
	else if(total < 6)
	    sprintf(buf,"$n puts some %s(s) in $o.",name);
	else
	    sprintf(buf,"$n puts a bunch of %ss in $o.",name);
	act(buf,TRUE,ch,obj_into,remember,TO_ROOM);
    } else {
	send_to_char("You don't have anything to put in it.", ch);
    }
    return ERROR_INTERNAL; /* that's what I'd call it - yuccch */
}

/* Just a rewrite of put-all-dot, for everything in inventory. - Sman */
int do_putallinv(struct char_data *ch, char *target, int cmd)
{
    struct obj_data *tmp_object;
    struct obj_data *next_object;
    struct obj_data *obj_into;
    char buf[MAX_STRING_LENGTH];
    int bits,total=0;

    /* If "put all bag", get all carried items
    * and put each into the bag.
    */
    bits=generic_find(target,FIND_OBJ_INV|FIND_OBJ_ROOM,ch,(void *)&obj_into);
    if(!obj_into) {
	send_to_char("Into what?",ch);
	return ERROR_SYNTAX;
    }
    for (tmp_object = ch->carrying; tmp_object;tmp_object = next_object) {
      next_object = tmp_object->next_content;
      if(obj_into!=tmp_object) {
	if(put(ch,tmp_object,obj_into,FALSE))
	    total++;
	else /* Let them see why */
	    put(ch,tmp_object,obj_into,TRUE);
      }
  	}	      
    if (total) {
	sprintf(buf,"You put in %d items in $o",total);
	act(buf,FALSE,ch,obj_into,0,TO_CHAR);
	if(total < 6)
	    act("$n puts some stuff in $o.",TRUE,ch,obj_into,0,TO_ROOM);
	else
	    act("$n puts a bunch of stuff in $o.",TRUE,ch,obj_into,0,TO_ROOM);
        return OKAY;
    } else {
	send_to_char("You don't have anything to put in it.", ch);
        return ERROR_FAILED;
    }
}

int do_put(struct char_data *ch, char *argument, int cmd)
{
    char buffer[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    struct obj_data *obj_object;
    struct obj_data *sub_object;
    int bits;
    char allbuf[MAX_STRING_LENGTH];
	
    argument_interpreter(argument, arg1, arg2);
    if (*arg1) { /* This cascading needs adjustment for retvals */
	if (*arg2) {
	    allbuf[0] = '\0';
	    if(!strcmp(arg1,"all")) {
		return do_putallinv(ch,arg2,cmd);
	    } else if (sscanf(arg1, "all.%s", allbuf) != 0) {
		return do_putalldot(ch, allbuf, arg2, cmd);
	    }
	    obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
	    if (obj_object) {
		bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM,
			  ch, (void *)&sub_object);
		if (sub_object) {
		    put(ch,obj_object,sub_object,TRUE);
		} else {
		    sprintf(buffer, "You dont have the %s.\n\r", arg2);
		    send_to_char(buffer, ch);
		}
	    } else {
		sprintf(buffer, "You dont have the %s.\n\r", arg1);
		send_to_char(buffer, ch);
	    }
	} else {
	    sprintf(buffer, "Put %s in what?\n\r", arg1);
	    send_to_char(buffer, ch);
	}
    } else {
	send_to_char("Put what in what?\n\r",ch);
    }
    return ERROR_SYNTAX;
}

bool put(struct char_data *ch,struct obj_data *obj_object,
    struct obj_data *sub_object,bool showit)
{
    char buffer[MAX_STRING_LENGTH];
    struct obj_info_container *cont;

    if((cont=(struct obj_info_container *)
                get_obj_info(sub_object,ITEM_CONTAINER))) {
	if (!IS_SET(cont->lock_state, CONT_CLOSED)) {
	    if (obj_object == sub_object) {
		if(showit)
		    send_to_char("You attempt to fold it into itself, but fail.\n\r", ch);
		return(FALSE);
	    }
	    if (((sub_object->obj_flags.weight) + 
		(obj_object->obj_flags.weight)) <
		(cont->capacity)) {
		if(showit)
		    send_to_char("Ok.\n\r", ch);
		if(obj_object->carried_by) {
		    obj_from_char(obj_object);
		} else {
		    obj_from_room(obj_object);
		}
		obj_to_obj(obj_object, sub_object);
		if(showit)
		    act("$n puts $p in $P",TRUE, ch, obj_object, sub_object, TO_ROOM);
		return(TRUE);
	    } else {
		if(showit)
		    send_to_char("It won't fit.\n\r", ch);
	    }
	} else {
	    if(showit)
		send_to_char("It seems to be closed.\n\r", ch);
	}
    } else {
	if(showit) {
	    sprintf(buffer,"The %s is not a container.\n\r", fname(sub_object->name));
	    send_to_char(buffer, ch);
	}
    }
    return(FALSE);
}


int do_give(struct char_data *ch, char *argument, int cmd)
{
    char obj_name[MAX_INPUT_LENGTH], vict_name[MAX_INPUT_LENGTH];
    struct char_data *vict;
    struct obj_data *obj;

    argument=one_argument(argument,obj_name);
#if 0
    if(is_number(obj_name))
    {
	if(strlen(obj_name)>7){ 
	    send_to_char("Number field too large.\n\r",ch);
	    return ERROR_SYNTAX;
    	}
	amount = atoi(obj_name);
	argument=one_argument(argument, arg);
	if (str_cmp("coins",arg) && str_cmp("coin",arg))
	{
	    send_to_char("Sorry, you can't do that (yet)...\n\r",ch);
	    return ERROR_INTERNAL;
	}
	if(amount<0)
	{
	    send_to_char("Sorry, you can't do that!\n\r",ch);
	    return ERROR_NO_SENSE;
	}
/*		if(GET_GOLD(ch)<amount)
	{
	    send_to_char("You haven't got that many coins!\n\r",ch);
	    return ERROR_NO_SENSE;
	}*/
	argument=one_argument(argument, vict_name);
	if(!*vict_name)
	{
	    send_to_char("To who?\n\r",ch);
	    return ERROR_SYNTAX;
	}
	if (!(vict = get_char_room_vis(ch, vict_name)))
	{
	    send_to_char("To who?\n\r",ch);
	    return ERROR_MISSING_TARGET;
	}
/*		if(IS_SET(vict->specials.act,PLR_GUEST))
	{
	    send_to_char("That's a guest player - no can do.\n\r",ch);
	    return ERROR_FAILED;
	}*/
	send_to_char("Ok.\n\r",ch);
	sprintf(buf,"%s gives you %d gold coins.\n\r",PERS(ch,vict),amount);
	send_to_char(buf,vict);
	act("$n gives some gold to $N.", 1, ch, 0, vict, TO_NOTVICT);

	if(amount>50000) {
	    sprintf(buf,"WIZ: Large transaction: %s gives %d coins to %s",GET_NAME(ch),amount,GET_NAME(vict));
	    log(buf);
	}
/*	GET_GOLD(ch)-=amount;
	GET_GOLD(vict)+=amount; */
	return OKAY;
    }
#endif

    argument=one_argument(argument, vict_name);


    if (!*obj_name || !*vict_name)
    {
	send_to_char("Give what to who?\n\r", ch);
	return ERROR_SYNTAX;
    }
    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
    {
	send_to_char("You do not seem to have anything like that.\n\r",
	 ch);
	return ERROR_MISSING_TARGET;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP))
    {
	send_to_char("You can't let go of it! Yeech!!\n\r", ch);
	return ERROR_FAILED;
    }
    if (!(vict = get_char_room_vis(ch, vict_name)))
    {
	send_to_char("No one by that name around here.\n\r", ch);
	return ERROR_MISSING_TARGET;
    }
/*	if(IS_SET(vict->specials.act,PLR_GUEST))
    {
	send_to_char("That's a guest player - no can do.\n\r",ch);
	return ERROR_FAILED;
    }*/

    if ((1+IS_CARRYING_N(vict)) > CAN_CARRY_N(vict))
    {
	act("$N seems to have $S hands full.", 0, ch, 0, vict, TO_CHAR);
	return ERROR_FULL;
    }
    if (obj->obj_flags.weight + IS_CARRYING_W(vict) > CAN_CARRY_W(vict))
    {
	act("$E can't carry that much weight.", 0, ch, 0, vict, TO_CHAR);
	return ERROR_FULL;
    }
    obj_from_char(obj);
    obj_to_char(obj, vict);
    act("$n gives $p to $N.", 1, ch, obj, vict, TO_NOTVICT);
    act("$n gives you $p.", 0, ch, obj, vict, TO_VICT);
    send_to_char("Ok.\n\r", ch);
    save_char(ch);
    save_char(vict);
    return OKAY;
}
