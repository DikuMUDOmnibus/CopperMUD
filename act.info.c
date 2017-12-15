/* ************************************************************************
*  file: act.info.c ,        Implementation of commands.  Part of DIKUMUD *
*  Usage : Informative commands.                                          *
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
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interp.h"
#include "db.h"
#include "org.h"
#include "time.h"
#include "player.h"
#include "error.h"
#include "proto.h"

/* extern variables */

extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct dayspec holidays[];
extern char credits[MAX_STRING_LENGTH];
extern char news[MAX_STRING_LENGTH];
extern char info[MAX_STRING_LENGTH];
extern char *dirs[]; 
extern char *where[];
extern char *color_liquid[];
extern char *fullness[];
extern const char *month_name[];
extern const int num_holiday;
extern struct time_info_data time_info;
extern const char *weekdays[];

/* extern functions */

struct time_info_data age(struct char_data *ch);
void page_string(struct descriptor_data *d, char *str, int keep_internal);
char get_season(struct zone_data *zone);
time_t time(time_t *tloc);
int strcasecmp(char *s1,char *s2);
int fseek(FILE *stream,long offset,int ptrname);

/* intern functions */

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int islong,
    bool show);


/* Procedures related to 'look' */

void argument_split_2(char *argument, char *first_arg, char *second_arg) {
    int look_at, found, begin;
    found = begin = 0;

    /* Find first non blank */
    for ( ;*(argument + begin ) == ' ' ; begin++);

    /* Find length of first word */
    for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

    /* Make all letters lower case, AND copy them to first_arg */
    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
    *(first_arg + look_at) = '\0';
    begin += look_at;

    /* Find first non blank */
    for ( ;*(argument + begin ) == ' ' ; begin++);

    /* Find length of second word */
    for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

    /* Make all letters lower case, AND copy them to second_arg */
    *(second_arg + look_at) = LOWER(*(argument + begin + look_at));
    *(second_arg + look_at)='\0';
    begin += look_at;
}

char *find_ex_description(char *word, struct extra_descr_data *list)
{
    struct extra_descr_data *i;

    for (i = list; i; i = i->next)
	if (isname(word,i->keyword))
	    return(i->description);

    return(0);
}

void show_obj_to_char_short(struct obj_data *object, struct char_data *ch)
{
    char buffer[MAX_STRING_LENGTH];
    struct obj_info_light *light;

    if (object->obj_flags.extra_flags & ITEM_IGNORE)
        return;
    buffer[0]='\0';
    if (object->short_description) {
	strcpy(buffer,object->short_description);
	if(object->equipped_as!=UNEQUIPPED) {
	    strcat(buffer,"   ");
	    strcat(buffer,where[object->equipped_as]);
        }
        if((light=(struct obj_info_light *)get_obj_info(object,ITEM_LIGHT)))
            if(light->brightness)
                strcat(buffer,"   <emits light>");
    }
    strcat(buffer, "\n\r");
    page_string(ch->desc, buffer, 1);
}


void show_obj_to_char_long(struct obj_data *object, struct char_data *ch)
{
    bool found;
    char buffer[MAX_STRING_LENGTH];
    struct obj_info *oi;
    struct obj_info_light *light;

    if (object->obj_flags.extra_flags & ITEM_IGNORE)
        return;
    buffer[0]='\0';
    if (object->description)
        strcpy(buffer,object->description);
    for(oi=object->info;oi;oi=oi->next)
        switch(oi->obj_type) {
            case ITEM_NOTE:
                if (object->action_description) {
                    strcat(buffer, "There is something written upon it:\n\r\n\r");
                    strcat(buffer, object->action_description);
        	}
        	else
        	    act("It's blank.", FALSE, ch,0,0,TO_CHAR);
                break;
            case ITEM_DRINKCON:
                strcat(buffer, "It looks like a drink container.");
                break;
            case ITEM_LIGHT:
                light=(struct obj_info_light *)oi;
                if(light->brightness > 20)
                    strcat(buffer,"It radiates vast amounts of light.");
                else if(light->brightness > 10)
                    strcat(buffer,"It shines brightly.");
                else if(light->brightness > 5)
                    strcat(buffer,"It emits light.");
                else if(light->brightness)
                    strcat(buffer,"It is faintly glowing.");
                else
                    strcat(buffer,"It's not providing any light.");
        }
    if(!buffer[0]) {
	    strcpy(buffer,"You see nothing special..");
    }

	found = FALSE;
	if (IS_OBJ_STAT(object,ITEM_INVISIBLE)) {
	    strcat(buffer,"(invisible)");
	    found = TRUE;
	}
	if (IS_OBJ_STAT(object,ITEM_HUM)) {
	    strcat(buffer,"..It emits a faint humming sound!");
	    found = TRUE;
	}

    strcat(buffer, "\n\r");
    send_to_char(buffer,ch);
/*
    if (((mode == 2) || (mode == 4)) && (GET_ITEM_TYPE(object) == 
	ITEM_CONTAINER)) {
	strcpy(buffer,"The ");
	strcat(buffer,fname(object->name));
	strcat(buffer," contains:\n\r");
	send_to_char(buffer, ch);
	if (mode == 2) list_obj_to_char(object->contains, ch, 1,TRUE);
	if (mode == 4) list_obj_to_char(object->contains, ch, 3,TRUE);
    }
*/
}

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int islong, 
    bool show) {
    struct obj_data *i;
    bool found;

    found = FALSE;
    for ( i = list ; i ; i = i->next_content ) { 
	if (CAN_SEE_OBJ(ch,i)) {
            if(islong)
	        show_obj_to_char_long(i, ch);
            else
	        show_obj_to_char_short(i, ch);
	    found = TRUE;
	}    
    }  
    if ((! found) && (show)) send_to_char("Nothing\n\r", ch);
}



void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
    char buffer[MAX_STRING_LENGTH];
    int found, percent;
    struct obj_data *tmp_obj;

    if (mode == 0) {
/*
	if ((IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch,i))){
	    if (IS_AFFECTED(ch, AFF_SENSE_LIFE))
		send_to_char("You sense a hidden life form in the room.\n\r", ch);
	    return;
	}*/

	if (1 /*!(i->player.long_descr)*/){
	    /* A player char or a mobile without long descr, or not in default pos. */
	    if (!IS_NPC(i)) {	
		sprintf(buffer,"%s %s",GET_NAME(i),GET_TITLE(i));
	    } else {
		strcpy(buffer,i->player.short_descr);
		(void)CAP(buffer);
	    }
/*
	    if ( IS_AFFECTED(i,AFF_INVISIBLE))
	     strcat(buffer," (invisible)");*/

	    if(i->desc && i->desc->str)
		strcat(buffer," is writing a message here.");
            else if (i->specials.fighting) {
		strcat(buffer," is here, fighting ");
		if (i->specials.fighting == ch)
		    strcat(buffer," YOU!");
		else {
		    if (i->in_room == i->specials.fighting->in_room)
			if (IS_NPC(i->specials.fighting))
			    strcat(buffer, i->specials.fighting->player.short_descr);
			else
			    strcat(buffer, GET_NAME(i->specials.fighting));
		    else
			strcat(buffer, "someone who has already left.");
		}
	    }
	    else
	      switch(GET_POS(i)) {
		case POS_STUNNED  : 
		    strcat(buffer," is lying here, stunned."); break;
		case POS_INCAP    : 
		    strcat(buffer," is lying here, incapacitated."); break;
		case POS_MORTALLYW: 
		    strcat(buffer," is lying here, mortally wounded.");
		    break;
		case POS_DEAD     : 
		    strcat(buffer," is lying here, dead.");
		    break;
		case POS_STAND : 
		    strcat(buffer," is standing here.");
		    break;
		case POS_SIT  : 
		    strcat(buffer," is sitting here.");
		    break;
		case POS_REST  : 
		    strcat(buffer," is resting here.");
		    break;
		case POS_SWIM:
		    strcat(buffer," is swimming here.");
		    break;
		case POS_SLEEP : 
		    strcat(buffer," is sleeping here.");
		    break;
		case POS_LEVITATE  : 
		    strcat(buffer," is levitating here.");
		    break;
		case POS_FLY : 
		    strcat(buffer," is flying here.");
		    break;
		default : strcat(buffer," is floating here."); break;
	      }
/*
	    if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
		if (IS_EVIL(i))
		    strcat(buffer, " (Red Aura)");
	    }*/

	    strcat(buffer,"\n\r");
	    send_to_char(buffer, ch);
	}
	else  /* npc with long */
	{/*
	    if (IS_AFFECTED(i,AFF_INVISIBLE))
		strcpy(buffer,"*");
	    else*/
		*buffer = '\0';
/*
	    if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
		if (IS_EVIL(i))
		    strcat(buffer, " (Red Aura)");
	    }*/

	    strcat(buffer, i->player.long_descr);

	    send_to_char(buffer, ch);
	}
			    /*
	if (IS_AFFECTED(i,AFF_SANCTUARY))
	    act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);*/

    } else if (mode == 1) {

	if (i->player.description)
	    send_to_char(i->player.description, ch);
	else {
	    act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
	}

	/* Show a character to another */

	if (GET_MAX_HIT(i) > 0)
	    percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
	else
	    percent = -1; /* How could MAX_HIT be < 1?? */

	if (IS_NPC(i))
	    strcpy(buffer, i->player.short_descr);
	else
	    strcpy(buffer, GET_NAME(i));

	if (percent >= 100)
	    strcat(buffer, " is in an excellent condition.\n\r");
	else if (percent >= 90)
	    strcat(buffer, " has a few scratches.\n\r");
	else if (percent >= 75)
	    strcat(buffer, " has some small wounds and bruises.\n\r");
	else if (percent >= 50)
	    strcat(buffer, " has quite a few wounds.\n\r");
	else if (percent >= 30)
	    strcat(buffer, " has some big nasty wounds and scratches.\n\r");
	else if (percent >= 15)
	    strcat(buffer, " looks pretty hurt.\n\r");
	else if (percent >= 0)
	    strcat(buffer, " is in an awful condition.\n\r");
	else
	    strcat(buffer, " is bleeding awfully from big wounds.\n\r");

	send_to_char(buffer, ch);

	found = FALSE;
	act("You try to see what $E has:",FALSE,ch,0,i,TO_CHAR);
	for(tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
	    if(CAN_SEE_OBJ(ch, tmp_obj) && (number(0,20)<10)){
		show_obj_to_char_short(tmp_obj, ch);
		found = TRUE;
	    }
	}
	if (!found)
	    send_to_char("You can't see anything.\n\r", ch);

    } else if (mode == 2) {

	/* Lists inventory */
	act("$n has:", FALSE, i, 0, ch, TO_VICT);
	list_obj_to_char(i->carrying,ch,FALSE,TRUE);
    }
}



void list_char_to_char(struct char_data *list, struct char_data *ch, 
    int mode) {
    struct char_data *i;

    for (i = list; i ; i = i->next_in_room) {
	if ( (ch!=i)/* && 
	  ((IS_AFFECTED(ch, AFF_SENSE_LIFE) || 
	  (CAN_SEE(ch,i) && !IS_AFFECTED(i, AFF_HIDE))))*/) {
	    show_char_to_char(i,ch,0);
	}
	else {
	 if (ch!=i && (IS_DARK(ch->in_room))/* && 
	   (IS_AFFECTED(i, AFF_INFRARED))*/) {
	   /* Monster with infra red : can't see him */
	   act("You see a pair of glowing red eyes looking your way.", FALSE,
               ch,0,0,TO_CHAR);
	 } /* if */
	} /* if */
    } 
}



int do_look(struct char_data *ch, char *argument, int cmd)
{
    char buffer[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    int keyword_no;
    int bits, temp;
    bool found;
    struct obj_data *tmp_object, *found_object;
    struct obj_info *oi;
    struct obj_info_container *cont;
    struct obj_info_drink *drink;
    struct char_data *tmp_char;
    char *tmp_desc;
    void *target = NULL;

    static char *keywords[]= { 
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"in",
	"at",
	"",  /* Look at '' case */
	"\n" };

    if (!ch->desc)
	return OKAY;

    if (GET_POS(ch) < POS_SLEEP) {
	send_to_char("You can't see anything but stars!\n\r", ch);
        return ERROR_FAILED;
    }
    if (GET_POS(ch) == POS_SLEEP) {
	send_to_char("You can't see anything, you're sleeping!\n\r", ch);
        return ERROR_FAILED;
    }
/*
    if ( IS_AFFECTED(ch, AFF_BLIND) ) {
	send_to_char("You can't see a damn thing, you're blinded!\n\r", ch);
        return ERROR_FAILED;
    }*/
    if ( IS_DARK(ch->in_room)) {
	send_to_char("It is pitch black...\n\r", ch);
	list_char_to_char(world[ch->in_room].people, ch, 0);
        return OKAY;
    } 
    argument_split_2(argument,arg1,arg2);
    keyword_no = search_block(arg1, keywords, FALSE); /* Partiel Match */

    if ((keyword_no == -1) && *arg1) {
        keyword_no = 7;
        strcpy(arg2, arg1); /* Let arg2 become the target object (arg1) */
    }

    found        = FALSE;
    tmp_object   = 0;
    tmp_char	 = 0;
    tmp_desc	 = 0;

    switch(keyword_no) {
        /* look <dir> */
        case 0 :
        case 1 :
        case 2 : 
        case 3 : 
        case 4 :
        case 5 :
/**** substitute once method is known...
	    if (EXIT(ch, keyword_no)) {
		if (EXIT(ch, keyword_no)->general_description) {
		    send_to_char(EXIT(ch, keyword_no)-> general_description,ch);
		} else {
		    send_to_char("You see nothing special.\n\r", ch);
		}

		if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CLOSED) && 
			(EXIT(ch, keyword_no)->keyword)) {
		    sprintf(buffer, "The %s is closed.\n\r",
		        fname(EXIT(ch, keyword_no)->keyword));
		    send_to_char(buffer, ch);
		} else {
		    if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_ISDOOR) &&
			  EXIT(ch, keyword_no)->keyword) {
		    sprintf(buffer, "The %s is open.\n\r",
				fname(EXIT(ch, keyword_no)->keyword));
		    send_to_char(buffer, ch);
		    }
	        }
	    } else {
		send_to_char("Nothing special there...\n\r", ch);
	    }
	    break;
*****/

	/* look 'in'	*/
	case 6:
	    if (*arg2) {
	        /* Item carried */

	        bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, (void *) &tmp_object);

	        if (bits) { /* Found something */
                    for(oi=tmp_object->info;oi;oi=oi->next)
	              switch(oi->obj_type) {
	                case ITEM_DRINKCON:
                        drink=(struct obj_info_drink *)oi;
		        if (drink->how_full == 0) {
		            act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
			} else {
			    if(drink->how_full<0)
			        temp=3;
			    else
			        temp=((drink->how_full*3)/drink->capacity);
			    sprintf(buffer,"It's %sfull of a %s liquid.\n\r",
			    fullness[temp],color_liquid[drink->drink_type]);
			    send_to_char(buffer, ch);
			}
		        case ITEM_CONTAINER:
                        cont=(struct obj_info_container *)oi;
		        if(!IS_SET(cont->lock_state,CONT_CLOSED)){
			    send_to_char(fname(tmp_object->name), ch);
			    switch (bits) {
			        case FIND_OBJ_INV :
			            send_to_char(" (carried) : \n\r", ch);
				    break;
				case FIND_OBJ_ROOM :
				    send_to_char(" (here) : \n\r", ch);
				    break;
			    }
		            list_obj_to_char(tmp_object->contains, ch, 0, TRUE);
			} else
			    send_to_char("It is closed.\n\r", ch);
		      }
                    if(!tmp_object->info)
		        send_to_char("That is not a container.\n\r", ch);
		} else { /* wrong argument */
	            send_to_char("You do not see that item here.\n\r", ch);
		}
	    } else { /* no argument */
	        send_to_char("Look in what?!\n\r", ch);
	    }
	    break;

	/* look 'at'	*/
	case 7 : {
	    if (*arg2) {
		bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
		   FIND_CHAR_ROOM, ch,&target);

                switch(bits) {
                    case  FIND_CHAR_ROOM:
                        tmp_char=(struct char_data *)target;
                        break;
                    case  FIND_OBJ_ROOM:
                    case  FIND_OBJ_INV:
                        found_object=(struct obj_data *)target;
                        break;
                    default:
                        break;
                }

		if (tmp_char) {
		    show_char_to_char(tmp_char, ch, 1);
		    if (ch != tmp_char) {
		        act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
		        act("$n looks at $N.",TRUE,ch,0, tmp_char, TO_NOTVICT);
		    }
		    return OKAY;
		}


		/* Search for Extra Descriptions in room and items */

		/* Extra description in room?? */
		if (!found) {
		    tmp_desc = find_ex_description(arg2, 
		        world[ch->in_room].ex_description);
		    if (tmp_desc) {
		        page_string(ch->desc, tmp_desc, 0);
		        return OKAY;
                        /* RETURN SINCE IT WAS A ROOM DESCRIPTION */
		        /* Old system was: found = TRUE; */
		    }
		}

		/* Search for extra descriptions in items */

		/* In inventory */

		if (!found) {
		    for(tmp_object = ch->carrying; tmp_object && !found; 
			    tmp_object = tmp_object->next_content) {
		        if(CAN_SEE_OBJ(ch, tmp_object)) {
			    tmp_desc = find_ex_description(arg2, 
			        tmp_object->ex_description);
			    if (tmp_desc) {
			        page_string(ch->desc, tmp_desc, 1);
			        found = TRUE;
			    }
			}
                    }
		}

		/* Object In room */

		if (!found) {
		    for(tmp_object = world[ch->in_room].contents; 
			    tmp_object && !found; 
			    tmp_object = tmp_object->next_content) {
		        if(CAN_SEE_OBJ(ch, tmp_object)) {
		            tmp_desc = find_ex_description(arg2, 
				    tmp_object->ex_description);
			    if (tmp_desc) {
			        page_string(ch->desc, tmp_desc, 1);
			        found = TRUE;
			    }
			}
		    }
		}
		/* wrong argument */

/* NOTE NOTE NOTE NOTE - these next 2 calls to s-o-t-c need changing */
		if (bits) { /* If an object was found */
		    if (!found)
		        show_obj_to_char_short(found_object, ch); /* Show no-description */
		    else
		        show_obj_to_char_long(found_object, ch); /* Find hum, glow etc */
		} else if (!found) {
		    send_to_char("You do not see that here.\n\r", ch);
		}
	    } else {
	        /* no argument */

	        send_to_char("Look at what?\n\r", ch);
                return ERROR_SYNTAX;
	    }
	    break;


	/* look ''		*/ 
	case 8 :
	    send_to_char(world[ch->in_room].name, ch);
	    send_to_char("\n\r", ch);

            if (ch->prefs && !IS_SET(ch->prefs->flags, PLR_BRIEF))
                send_to_char(world[ch->in_room].description[zone_table[world[ch->in_room].zone].desc_mode], ch);

	    list_obj_to_char(world[ch->in_room].contents, ch, TRUE,FALSE);

	    list_char_to_char(world[ch->in_room].people, ch, 0);
	    break;

	/* wrong arg	*/
	case -1 : 
	    send_to_char("Sorry, I didn't understand that!\n\r", ch);
            return ERROR_SYNTAX;
	    break;
	}
    }
    return OKAY;
}

/* end of look */




int do_read(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH+6];

    /* This is just for now - To be changed later.! */
    sprintf(buf,"at %s",argument);
    return(do_look(ch,buf,15));
}



int do_examine(struct char_data *ch, char *argument, int cmd)
{
    char name[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH+4];
    int bits,temp;
    struct obj_data *tmp_object;

    sprintf(buf,"at %s",argument);
    temp = do_look(ch,buf,15);

    one_argument(argument, name);

    if (!*name)
    {
	send_to_char("Examine what?\n\r", ch);
	return ERROR_SYNTAX;
    }

    bits = generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM ,
       ch, (void **)&tmp_object);

    if (tmp_object) {
	if ((get_obj_info(tmp_object,ITEM_DRINKCON)) ||
	  (get_obj_info(tmp_object,ITEM_CONTAINER))) {
	    send_to_char("When you look inside, you see:\n\r", ch);
	    sprintf(buf,"in %s",argument);
	    do_look(ch,buf,15);
	}
        return OKAY;
    }
    return temp;
}



int do_exits(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    struct obj_data *o;
    struct obj_info_exit *e;
    char *exits[] =
    {
	"North",
	"East ",
	"South",
	"West ",
	"Up   ",
	"Down ",
        "Northeast",
        "Southeast",
        "Southwest",
        "Northwest"
    };

    *buf = '\0';

    for(o = world[ch->in_room].contents;o;o=o->next_content)
        if ((e = (struct obj_info_exit *)get_obj_info(o,ITEM_EXIT)))
	    if (e->to_room != NOWHERE && !IS_SET(e->exit_info, EX_CLOSED))
		if (IS_DARK(e->to_room)) {
		    if(!IS_DARK(ch->in_room))
			sprintf(buf + strlen(buf), "%10s - Too dark to tell\n\r", exits[e->dir]);
		} else
		    sprintf(buf + strlen(buf), "%10s - %s\n\r", exits[e->dir],
			world[e->to_room].name);

    send_to_char("Obvious exits:\n\r", ch);

    if (*buf)
	send_to_char(buf, ch);
    else
	send_to_char("None.\n\r", ch);
    return OKAY;
}


int do_score(struct char_data *ch, char *argument, int cmd)
{
    struct char_org_data *i;
    bool found;
    struct time_info_data playing_time;
    struct org_type *org;
    char buf[160];

    struct time_info_data real_time_passed(time_t t2, time_t t1);

    sprintf(buf, "You are %d years old.", GET_AGE(ch));

    if ((age(ch).month == 0) && (age(ch).day == 0))
	strcat(buf," It's your birthday today.\n\r");
    else
	strcat(buf,"\n\r");
    send_to_char(buf, ch);

    if (ch->physical->impairment >10)
	send_to_char("You are intoxicated.\n\r", ch);

    sprintf(buf, 
	"You have %d(%d) hit, %d(%d) mana and %d(%d) movement points.\n\r",
	GET_HIT(ch),GET_MAX_HIT(ch),
	GET_MANA(ch),GET_MAX_MANA(ch),
	GET_MOVE(ch),GET_MAX_MOVE(ch));
    send_to_char(buf,ch);
    sprintf(buf,"You have %s strength, %s dexterity, %s intelligence,\n\r%s wisdom, and %s constitution.\n\r",
	rating(GET_STR(ch),25,0), rating(GET_DEX(ch),25,0),
	rating(GET_INT(ch),25,0), rating(GET_WIS(ch),25,0),
	rating(GET_CON(ch),25,0));
    send_to_char(buf,ch);

    buf[0]=0;
    if(IS_GOOD(ch)){
	strcat(buf,"Your alignment is good");
	if(ch->specials.alignment <400)
	    strcat(buf,", leaning toward neutral");
    } else if (IS_EVIL(ch)) {
	strcat(buf,"Your alignment is evil");
	if(ch->specials.alignment >-400)
	    strcat(buf,", leaning toward neutral");
    } else {
	strcat(buf,"You are neutrally aligned");
	if(ch->specials.alignment >300)
	    strcat(buf,", leaning toward good");
	else if(ch->specials.alignment <-300)
	    strcat(buf,", leaning toward evil");
    }
    strcat(buf,".");
    send_to_char(buf,ch);

    sprintf(buf," Your Armor Class is %d.\n\r", GET_AC(ch));
    send_to_char(buf,ch);

    sprintf(buf,"You have scored %d exp.\n\r",
	GET_EXP(ch));
    send_to_char(buf,ch);

    sprintf(buf,"You have made %d kills.\n\r",ch->points.kills);
    send_to_char(buf,ch);

    playing_time = real_time_passed((time(0)-ch->player.time.logon) +
     ch->player.time.played, 0);
    sprintf(buf,"You have been playing for %d days and %d hours.\n\r",
	playing_time.day,
	playing_time.hours);		
    send_to_char(buf, ch);		

    sprintf(buf,"This ranks you as %s %s.\n\r",
	GET_NAME(ch),
	GET_TITLE(ch));
    send_to_char(buf,ch);

    send_to_char("You belong to:\n\r",ch);
    found=FALSE;
    for(i=ch->orgs;i;i=i->next) {
	org=get_org_by_id(i->org_id);
	if(org) {
	    found=TRUE;
            if(org->org_flags & ORGF_LEVELS)
                sprintf(buf,"  %s [level %d]\n\r",org->name,i->member_level);
            else
                sprintf(buf,"  %s\n\r",org->name);
	    send_to_char(buf,ch);
	}
    }
    if(!found)
	send_to_char("  Nothing!\n\r",ch);

    switch(GET_POS(ch)) {
	case POS_DEAD : 
	    send_to_char("You are DEAD!\n\r", ch); break;
	case POS_MORTALLYW :
	    send_to_char("You are mortally wounded!, you should seek help!\n\r", ch); break;
	case POS_INCAP : 
	    send_to_char("You are incapacitated, slowly fading away\n\r", ch); break;
	case POS_STUNNED : 
	    send_to_char("You are stunned! You can't move\n\r", ch); break;
	case POS_SLEEP : 
	    send_to_char("You are sleeping.\n\r",ch); break;
	case POS_REST  : 
	    send_to_char("You are resting.\n\r",ch); break;
	case POS_SIT  : 
	    send_to_char("You are sitting.\n\r",ch); break;
	case POS_STAND : 
	    send_to_char("You are standing.\n\r",ch); break;
	case POS_LEVITATE:
	    send_to_char("You are Levitating.\n\r",ch); break;
	case POS_FLY:
	    send_to_char("You are flying.\n\r",ch); break;
	default :
	    send_to_char("You can't seem to figure out yourself.\n\r",ch); break;
    }
    if (ch->specials.fighting)
	act("You are fighting $N.\n\r", FALSE, ch, 0,
        ch->specials.fighting, TO_CHAR);

    if(IS_OUTLAW(ch))
	send_to_char("You are an outlaw!\n\r",ch);

/*
    if(IS_AFFECTED(ch,AFF_SNEAK)){
	send_to_char("Sneak\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_HIDE)){
	send_to_char("Hide\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_DODGE)){
	send_to_char("Dodge\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_BLIND)){
	send_to_char("Blind\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_INVISIBLE)){
	send_to_char("Invisible\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_DETECT_EVIL)){
	send_to_char("Detect Evil\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_DETECT_INVISIBLE)){
	send_to_char("Detect Invisible\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_DETECT_MAGIC)){
	send_to_char("Detect Magic\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_SENSE_LIFE)){
	send_to_char("Sense Life\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_HOLD)){
	send_to_char("Hold\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_SANCTUARY)){
	send_to_char("Sanctuary\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_CURSE)){
	send_to_char("Curse\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_FLAMING)){
	send_to_char("Flaming\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_POISON)){
	send_to_char("Poison\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_PROTECT_EVIL)){
	send_to_char("Protection from Evil\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_PARALYSIS)){
	send_to_char("Paralysis\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_MORDEN_SWORD)){
	send_to_char("Morden's sword\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_FLAMING_SWORD)){
	send_to_char("Flaming sword\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_SLEEP)){
	send_to_char("Sleep\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_FEAR)){
	send_to_char("Fear\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_CHARM)){
	send_to_char("Charm\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_FOLLOW)){
	send_to_char("Follow\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_WIMPY)){
	send_to_char("Wimpy\n\r",ch);
    }
    if(IS_AFFECTED(ch,AFF_INFRARED)){
	send_to_char("Infrared\n\r",ch);
    }
*/

    return OKAY;
}

struct dayspec *get_holiday(int month,int day)
{
    int i=0;

    while(i<num_holiday) {
	if(holidays[i].month==month && holidays[i].day==day)
	    return(&holidays[i]);
	i++;
    }

    return(NULL);
}


int do_time(struct char_data *ch, char *argument, int cmd)
{
    char buf[100], *suf;
    int weekday, day, i;
    struct dayspec *holiday;

    sprintf(buf, "It is %d o'clock %s, on ",
	((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	((time_info.hours >= 12) ? "pm" : "am") );

    weekday = ((MUD_DAYS_PER_MONTH*time_info.month) +time_info.day/*+1*/)
	% MUD_DAYS_PER_WEEK;

    strcat(buf,weekdays[weekday]);
    strcat(buf,"\n\r");
    send_to_char(buf,ch);

    day = time_info.day + 1;   /* day in [1..MUD_DAYS_PER_MONTH] */

    if (day == 1)
	suf = "st";
    else if (day == 2)
	suf = "nd";
    else if (day == 3)
	suf = "rd";
    else if (day < 20)
	suf = "th";
    else if ((day % 10) == 1)
	suf = "st";
    else if ((day % 10) == 2)
	suf = "nd";
    else if ((day % 10) == 3)
	suf = "rd";
    else
	suf = "th";

    sprintf(buf, "The %d%s Day of the %s, Year %d of the Current Era.\n\r",
	day,
	suf,
	month_name[(int)time_info.month],
	time_info.year);

    send_to_char(buf,ch);

    if((holiday=get_holiday(time_info.month,day))) {
        strcpy(buf,"Today is ");
        for(i=0;i<5;i++)
            if(*holiday->line[i])
                strcat(buf,holiday->line[i]);
        strcat(buf,"!\n\r");
        send_to_char(buf,ch);
    }
    return OKAY;
}

int do_calendar(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    int i,row,week,day,numweeks,month,format;
    struct dayspec *spec;

    char *line=
	"+---------+---------+---------+---------+---------+---------+---------+\n";
    char *post=
	"|";
    char *spaces=
	"         ";

    /* parse some args for short/long/whatever */
    while(isspace(*argument))
	argument++;

    month = time_info.month;

    if(!*argument)
	format=0;
    else {
	one_argument(argument,buf);
	if(!strcasecmp(buf,"long"))
	    format=1;
	else if(!strcasecmp(buf,"short"))
	    format=0;
	/* Can stuff month-arg in here */
    }

    buf[0]='\0';
    i=32-strlen(month_name[month])/2;
    while(i-- > 0)
	strcat(buf," ");

    strcat(buf,month_name[month]);
    strcat(buf,"\n");
    send_to_char(buf,ch);

    day = -(((MUD_DAYS_PER_MONTH*month) +1) % MUD_DAYS_PER_WEEK) + 1;

    /* set numweeks to the number of weeks in the month */
    numweeks = MUD_DAYS_PER_MONTH/MUD_DAYS_PER_WEEK;

    if(MUD_DAYS_PER_MONTH % MUD_DAYS_PER_WEEK)
	numweeks++;

    if((MUD_DAYS_PER_MONTH % MUD_DAYS_PER_WEEK)-day >=MUD_DAYS_PER_WEEK)
	numweeks++;

    if(format==0) {
	/* Now figure out how many weeks to print */
	if(time_info.day-day>=21) {
	    numweeks -=2;
	    day +=14;
	} else if(time_info.day-day>=14) {
	    numweeks -=1;
	    day +=7;
	}
	numweeks=MIN(numweeks,3);
    }

    for(week=0;week< numweeks;week++,day+=7) {

	send_to_char(line,ch);

	for(row=1;row<6;row++) { /* Each vertical post of a week */
	    for(i=1;i<=MUD_DAYS_PER_WEEK;i++) {
		day++;
		send_to_char(post,ch);
		if(day <1 || day > MUD_DAYS_PER_MONTH) {
		    send_to_char(spaces,ch);
		    continue;
		}
		switch(row) {
		    case 1:
			if(day==time_info.day+1){
			    send_to_char(
			    "  TODAY  ",ch);
			} else {
			    sprintf(buf, " %-8d",
				day);
			    send_to_char(buf,ch);
			}
			break;
		    default:
			if((spec=get_holiday(month+1,day))) {
			    sprintf(buf,"%-9s",
			    spec->line[row-2]);
			    send_to_char(buf,ch);
			} else 
			    send_to_char(spaces,ch);
			break;
		}
	    }
	    day -=7;
	    send_to_char(post,ch);
	    send_to_char("\n",ch);
	}
    }
    send_to_char(line,ch);
    return OKAY;
}

int do_weather(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    struct weather_data *cond;
    struct climate *clime;
    char *season_patterns[8]= {
	"One season. ",
	"Two equal seasons. ",
	"Two seasons (first long). ",
	"Two seasons (second long). ",
	"Three equal seasons. ",
	"Four equal seasons. ",
	"Four seasons (even long). ",
	"Four seasons (odd long). "
    };
    char *wind_patterns[7]= {
	"Calm ",
	"Breezy ",
	"Unsettled ",
	"Windy ",
	"Chinook ",
	"Violent ",
	"Hurricane "
    };
    char *wind_dir[4] = {"North. ","East. ","South. ","West. "};
    char *precip_patterns[9]= {
	"No precip. ",
	"Arid. ",
	"Dry. ",
	"Low precip. ",
	"Avg precip. ",
	"High precip. ",
	"Stormy. ",
	"Torrent. ",
	"Const precip. "
    };
    char *temp_patterns[11]= {
	"Frostbite. ",
	"Nippy. ",
	"Freezing. ",
	"Cold. ",
	"Cool. ",
	"Mild. ",
	"Warm. ",
	"Hot. ",
	"Blustery. ",
	"Heatstroke. ",
	"Boiling. "
    };
    int seas;

    clime=&zone_table[world[ch->in_room].zone].climate;
    cond=&zone_table[world[ch->in_room].zone].conditions;

  if(1) {
    seas=get_season(&zone_table[world[ch->in_room].zone]);
    send_to_char("Weather Dump\n\r   Season Chars: ",ch);

    send_to_char(season_patterns[clime->season_pattern-1],ch);
    sprintf(buf,"(#%d)\n\r                 ",seas);
    send_to_char(buf,ch);
    send_to_char(wind_patterns[clime->season_wind[seas]-1],ch);
    if(clime->season_wind_variance[seas-1])
	send_to_char("variable. ",ch);
    else {
	send_to_char("from ",ch);
	send_to_char(wind_dir[(int)clime->season_wind_dir[seas]],ch);
    }
    send_to_char(precip_patterns[clime->season_precip[seas]-1],ch);
    send_to_char(temp_patterns[clime->season_temp[seas]-1],ch);

    sprintf(buf,"\n\rTemp: %d  Humidity: %d  Pressure: %d\n\r", cond->temp,
	cond->humidity,cond->pressure);

    send_to_char(buf,ch);

    sprintf(buf,"Windspeed: %d  Direction: %d  Precip Rate: %d\n\r",
	cond->windspeed,cond->wind_dir,cond->precip_rate);

    send_to_char(buf,ch);

    sprintf(buf,"Light: %d  Energy: %d  Pressure change: %d  Precip change: %d\n\r",
	cond->ambient_light,cond->free_energy,cond->pressure_change,
	cond->precip_change);

    send_to_char(buf,ch);
  }
    if(!OUTSIDE(ch)) {
	send_to_char("How can you know what the weather's like when you are inside?\n\r\n\r",ch);
	return ERROR_LOCATION;
    }
    buf[0]=0;
    if(cond->precip_rate) {
	if(cond->temp<=0)
	    strcat(buf,"It's snowing");
	else
	    strcat(buf,"It's raining");
	if(cond->precip_rate>65)
	    strcat(buf," extremely hard");
	else if(cond->precip_rate>50)
	    strcat(buf," very hard");
	else if(cond->precip_rate>30)
	    strcat(buf," hard");
	else if(cond->precip_rate<15)
	    strcat(buf," lightly");
	strcat(buf,", ");
    } else {
	if(cond->humidity>80)
	    strcat(buf,"It's very cloudy, ");
	else if(cond->humidity>55)
	    strcat(buf,"It's cloudy, ");
	else if(cond->humidity>25)
	    strcat(buf,"It's partly cloudy, ");
	else if(cond->humidity)
	    strcat(buf,"It's mostly clear, ");
	else
	    strcat(buf,"It's clear, ");
    }
    if(cond->temp > 100)
	strcat(buf,"boiling, ");
    else if(cond->temp > 80)
	strcat(buf,"blistering, ");
    else if(cond->temp > 50)
	strcat(buf,"incredibly hot, ");
    else if(cond->temp > 40)
	strcat(buf,"very, very hot, ");
    else if(cond->temp > 30)
	strcat(buf,"very hot, ");
    else if(cond->temp > 24)
	strcat(buf,"hot, ");
    else if(cond->temp > 18)
	strcat(buf,"warm, ");
    else if(cond->temp > 9)
	strcat(buf,"mild, ");
    else if(cond->temp > 3)
	strcat(buf,"cool, ");
    else if(cond->temp > -1)
	strcat(buf,"cold, ");
    else if(cond->temp > -10)
	strcat(buf,"freezing, ");
    else if(cond->temp > -25)
	strcat(buf,"well past freezing, ");
    else
	strcat(buf,"numbingly frozen, ");

    strcat(buf,"and ");

    if(cond->windspeed<=0)
	strcat(buf,"there is absolutely no wind");
    else if(cond->windspeed<10)
	strcat(buf,"calm");
    else if(cond->windspeed<20)
	strcat(buf,"breezy");
    else if(cond->windspeed<35)
	strcat(buf,"windy");
    else if(cond->windspeed<50)
	strcat(buf,"very windy");
    else if(cond->windspeed<70)
	strcat(buf,"very, very windy");
    else if(cond->windspeed<100)
	strcat(buf,"there is a gale blowing");
    else
	strcat(buf,"the wind is unbelievable");
    strcat(buf,".\n\r");
    send_to_char(buf,ch);
    /*if(GET_CLASS(ch)==CLASS_CLERIC || GET_CLASS(ch)==CLASS_MAGIC_USER) {*/
	if(cond->free_energy>40000)
	    send_to_char("Wow! This place is bursting with energy!\n\r",ch);
	else if(cond->free_energy>30000)
	    send_to_char("The environs tingle your magical senses.\n\r",ch);
	else if(cond->free_energy>20000)
	    send_to_char("The area is rich with energy.\n\r",ch);
	else if(cond->free_energy<4000)
	    send_to_char("There is almost no magical energy here.\n\r",ch);
	else if(cond->free_energy<5000)
	    send_to_char("Your magical senses are dulled by the scarceness of energy here.\n\r",ch);
    /*}*/
    send_to_char("\n\r",ch);
    return OKAY;
}

int do_help(struct char_data *ch, char *argument, int cmd)
{
    extern int top_of_helpt;
    extern struct help_index_element *help_index;
    extern FILE *help_fl;
    extern char help[MAX_STRING_LENGTH];

    int chk, bot, top, mid, minlen;
    char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];


    if (!ch->desc)
	return OKAY;

    for(;isspace(*argument); argument++)  ;


    if (*argument)
    {
	if (!help_index)
	{
	    send_to_char("No help available.\n\r", ch);
            log("BUG: No help table");
	    return ERROR_INTERNAL;
	}
	bot = 0;
	top = top_of_helpt;

	for (;;)
	{
	    mid = (bot + top) / 2;
	    minlen = strlen(argument);

	    if (!(chk = strn_cmp(argument, help_index[mid].keyword, minlen)))
	    {
		fseek(help_fl, help_index[mid].pos, 0);
		*buffer = '\0';
		for (;;)
		{
		    fgets(buf, 80, help_fl);
		    if (*buf == '#')
			break;
		    strcat(buffer, buf);
		    strcat(buffer, "\r");
		}
		page_string(ch->desc, buffer, 1);
		return OKAY;
	    }
	    else if (bot >= top)
	    {
		send_to_char("There is no help on that word.\n\r", ch);
		return ERROR_FAILED;
	    }
	    else if (chk > 0)
		bot = ++mid;
	    else
		top = --mid;
	}
    }


    send_to_char(help, ch);
    return OKAY;
}

char *make_who(struct char_data *ch)
{
    static char buf[MAX_STRING_LENGTH];

    sprintf(buf,"%s %s", GET_NAME(ch), ch->player.title);
/*
    if (IS_AFFECTED(ch, AFF_INVISIBLE)){
	strcat(buf," (invis)");
    }*/
    strcat(buf,"\n\r");

    return(buf);
}

int do_who(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *d;
    char buf[256], *line;
    int number=0;

    if(IS_NPC(ch))
	return OKAY;

    send_to_char("-------\n\rPlayers\n\r-------\n\r", ch);

    for (d = descriptor_list; d; d = d->next)
    {
	if (!d->connected && CAN_SEE(ch, d->character))
	{
	    number++;
	    if(d->original) { /* If switched */			
		if(!CAN_SEE(ch,d->original)){
		    number--;
		    continue;
		}
		line=make_who(d->original);
	    } else
		line=make_who(d->character);

	    send_to_char(line, ch);
	}
    }
    sprintf(buf,"\n\rThere are %d visible players.\n\r",number);
    send_to_char(buf,ch);
    return OKAY;
}


int do_localwho(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *d;
    char buf[256];
    char team[80];

/*	if(IS_NPC(ch)) return; */
    send_to_char("--------------------\n\rPlayers in your Zone\n\r--------------------\n\r", ch);

    for (d = descriptor_list; d; d = d->next) {
	if (!d->connected && CAN_SEE(ch, d->character) &&
	(world[d->character->in_room].zone == 
	world[ch->in_room].zone)) {
	    if(!CAN_SEE(ch, d->character))
		continue;
	    if(d->original) { 	/* If switched */
		sprintf(buf, "%15s %-30s\n\r", 
		 GET_NAME(d->original),
		 	d->original->player.title);
	    } else {
		sprintf(team,"%s",GET_NAME(d->character));

		sprintf(buf, "%-30s - %s\n\r", team,
		 world[d->character->in_room].name);	
	    }
     		send_to_char(buf, ch);
	}
    }
    return OKAY;
}

int do_sockets(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int num_can_see=0,numeric=0;

    struct descriptor_data *d;

    one_argument(argument,arg);

    if(!strcasecmp(arg,"numeric"))
        numeric=1;

    strcpy(buf, "Socket Stats:\n\r------------\n\r");
    send_to_char(buf,ch);
    
    for (d = descriptor_list; d; d = d->next)
    {
	if (d->character && d->character->player.name 
	  && !CAN_SEE(ch,d->character)){
	    continue;
	}
	num_can_see++;

        if(numeric)
            sprintf(arg, "%d.%d.%d.%d",
                (d->numeric & 0x000000FF),
                (d->numeric & 0x0000FF00) >> 8,
                (d->numeric & 0x00FF0000) >> 16,
                (d->numeric & 0xFF000000) >> 24);

        sprintf(buf,"%3d: %-30s/ %-14s [%d] ",
	      d->descriptor,
	      (numeric ? arg : d->host),
	      (d->original) ? d->original->player.name :
		      d->character->player.name, d->idle);

	switch(d->connected){
	 case CON_PLYNG:
	    break;
	 case CON_NME:
	    strcat(buf,"input name");
            break;
	 case CON_NEWPL:
	    strcat(buf,"new player name");
            break;
	 case CON_PWDNRM:
	    strcat(buf,"pw known player");
            break;
	 case CON_QSEX:
	    strcat(buf,"query sex");
            break;
	 case CON_RMOTD:
	    strcat(buf,"reading motd");
            break;
	 case CON_SLCT:
	    strcat(buf,"slct");
            break;
	 case CON_PWDNEW:
	    strcat(buf,"pwdnew");
            break;
	 case CON_PWDNCNF:
	    strcat(buf,"pwdncnf");
            break;
	 case CON_CLOSE:
	    strcat(buf,"closed");
            break;
	 case CON_READERS:
            strcat(buf,"readers - ");
            strcat(buf,d->reader->title);
            break;
	 case CON_DELETE:
	    strcat(buf,"delete");
            break;
	 case CON_INTRIN:
	    strcat(buf,"intrinsics");
            break;
	 case CON_TOMB:
	    strcat(buf,"tombstone");
            break;
	 case CON_TOMB2:
	    strcat(buf,"tombstone2");
            break;
	 default:
	    sprintf(arg,"Display Mode %d",d->connected);
	    strcat(buf,arg);
	    break;
	}
	strcat(buf,"\n\r");
	send_to_char(buf, ch);
    }
    sprintf(buf,"\n\rThere are %d users playing\n\r",
	num_can_see);
    send_to_char(buf,ch);
    return OKAY;
}

int do_users(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH], line[200],who[80];
    int count;
    struct descriptor_data *d;

    strcpy(buf, "Connections:\n\r------------\n\r");
    
    for (count=0, d = descriptor_list; d; d = d->next)
    {
	if (d->character && d->character->player.name)
	{
	    count++;
	    if(d->original)
		sprintf(line, "%-16s: ", d->original->player.name);
	    else
		sprintf(line, "%-16s: ", d->character->player.name);
	} else {
	    count++;
	    strcpy(line, "UNDEFINED       : ");
	}
        if(d->user[0])
            strcpy(who,d->user);
        else
            strcpy(who,"<unknown>");
        strcat(who,"@");
	if (d->host[0])
	    strcat(who, d->host);
	else
	    strcat(who, "unknown.huh");

	strcat(buf, line);
        strcat(buf, who);
        strcat(buf, "\n\r");
    }
    send_to_char(buf,ch);
    sprintf(buf,"\n\rThere are %d users playing.\n\r",count);
    send_to_char(buf, ch);
    return OKAY;
}



int do_inventory(struct char_data *ch, char *argument, int cmd)
{

    send_to_char("You have:\n\r", ch);
    list_obj_to_char(ch->carrying, ch, FALSE, TRUE);
    return OKAY;
}


int do_equipment(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *j;
    bool found;

    send_to_char("You are using:\n\r", ch);
    found = FALSE;
    for (j=ch->carrying; j; j=j->next_content) {
	if(j->equipped_as != UNEQUIPPED) {
	    if (CAN_SEE_OBJ(ch,j)) {
		show_obj_to_char_short(j,ch);
		found = TRUE;
	    } else {
		send_to_char("Something.\n\r",ch);
		found = TRUE;
	    }
	}
    }
    if(!found) {
	send_to_char(" Nothing.\n\r", ch);
    }
    return OKAY;
}


int do_credits(struct char_data *ch, char *argument, int cmd) {

    page_string(ch->desc, credits, 0);
    return OKAY;
}


int do_news(struct char_data *ch, char *argument, int cmd) {

    page_string(ch->desc, news, 0);
    return OKAY;
}


int do_info(struct char_data *ch, char *argument, int cmd) {

    page_string(ch->desc, info, 0);
    return OKAY;
}



int do_where(struct char_data *ch, char *argument, int cmd)
{
    char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], buf2[256];
    register struct char_data *i;
    register struct obj_data *k;
    struct descriptor_data *d;

    one_argument(argument, name);
    if (!*name) {
	if (0)
	{
	    /*send_to_char("What are you looking for?\n\r", ch);*/
	    /* Do a local who instead */
	    return(do_localwho(ch,argument,cmd));
	}
	else
	{
	    strcpy(buf, "Players:\n\r--------\n\r");
	
	    for (d = descriptor_list; d; d = d->next) {
		if (d->character && d->connected == CON_PLYNG && d->character->in_room != NOWHERE) {
		    if (d->original)   /* If switched */
			sprintf(buf, "%-20s - [%d] %s (In body of %s)\n\r",
			 d->original->player.name,
			 world[d->character->in_room].number,
			 world[d->character->in_room].name,
			 fname(d->character->player.name));
		    else
			sprintf(buf, "%-20s - [%d] %s \n\r",
			 d->character->player.name,
			 world[d->character->in_room].number,
			 world[d->character->in_room].name);
			
		    send_to_char(buf, ch);
		}
	    }
	    return OKAY;
	}
    }

    *buf = '\0';

    for (i = character_list; i; i = i->next)
	if (isname(name, i->player.name) && CAN_SEE(ch, i) )
	{
	    if(i->in_room != NOWHERE && (get_char_org(ch,ORG_ID_ADMIN) ||
	      (world[i->in_room].zone == world[ch->in_room].zone /*&& !IS_AFFECTED(i,AFF_HIDE)*/))) {

		if (IS_NPC(i))
		    sprintf(buf, "%-30s- %s ", i->player.short_descr,
			world[i->in_room].name);
		else
		    sprintf(buf, "%-30s- %s ", i->player.name,
			world[i->in_room].name);

		if (0)
		    sprintf(buf2,"[%d]\n\r", world[i->in_room].number);
		else
		    strcpy(buf2, "\n\r");

		strcat(buf, buf2);
		send_to_char(buf, ch);

	    }
	}

    if (0){
	for (k = object_list; k; k = k->next)
	    if (isname(name, k->name) && CAN_SEE_OBJ(ch, k) && 
		(k->in_room != NOWHERE)) {
		    sprintf(buf, "%-30s- %s [%d]\n\r",
			k->short_description,
			world[k->in_room].name,
			world[k->in_room].number);
			send_to_char(buf, ch);
		}
    }

    if (!*buf) {
	send_to_char("Couldn't find any such thing.\n\r", ch);
        return ERROR_MISSING_TARGET;
    }
    return OKAY;
}



int do_consider(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char name[256];
    int diff;

    one_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name))) {
	send_to_char("Consider killing who?\n\r", ch);
	return ERROR_SYNTAX;
    }

    if (victim == ch) {
	send_to_char("Easy! Very easy indeed!\n\r", ch);
	return OKAY;
    }

    if (!IS_NPC(victim) && !IS_NPC(ch)) {
	send_to_char("Would you like to borrow a cross and a shovel?\n\r", ch);
	return OKAY;
    }

    diff = /*(GET_LEVEL(victim)-GET_LEVEL(ch));*/ 99;

    if (diff <= -10)
	send_to_char("Now where did that chicken go?\n\r", ch);
    else if (diff <= -5)
	send_to_char("You could do it with a needle!\n\r", ch);
    else if (diff <= -2)
	send_to_char("Easy.\n\r", ch);
    else if (diff <= -1)
	send_to_char("Fairly easy.\n\r", ch);
    else if (diff == 0)
	send_to_char("The perfect match!\n\r", ch);
    else if (diff <= 1)
	send_to_char("You would need some luck!\n\r", ch);
    else if (diff <= 2)
	send_to_char("You would need a lot of luck!\n\r", ch);
    else if (diff <= 3)
	send_to_char("You would need a lot of luck and great equipment!\n\r", ch);
    else if (diff <= 5)
	send_to_char("Do you feel lucky, punk?\n\r", ch);
    else if (diff <= 10)
	send_to_char("Are you mad!?\n\r", ch);
    else if (diff <= 15)
	send_to_char("You ARE mad!\n\r", ch);
    else if (diff <= 20)
	send_to_char("Why don't you just lie down and pretend you're dead instead?\n\r",ch);
    else if (diff <= 25)
	send_to_char("What do you want your epitaph to say?!?\n\r",ch);
    else if (diff <= 100)
	send_to_char("Your epitaph will read: Here lies one dead and very dumb Diku-player.\n\r",ch);
    return OKAY;
}
