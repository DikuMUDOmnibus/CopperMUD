/* ************************************************************************
*  file: act.org.c , Implementation of commands.          Part of Copper3 *
*  Usage : Commands for organizations                                     *
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
#include "org.h"
#include "event.h"
#include "error.h"
#include "proto.h"

/* extern variables */

extern struct room_data *world;
extern struct zone_data *zone_table;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct org_type *org_list;
extern char *org_kind[];
extern char *skills[];

/* extern procedures */

void start_org(struct char_data *member,struct org_type *org);
struct org_type *get_org_by_name(char *name);
struct char_skill_data *get_skill(struct char_data *ch,int type);

int do_join(struct char_data *ch, char *argument, int cmd)
{
    struct org_type *org;
    char buf[MAX_STRING_LENGTH];

    /* This one is tough. */

    while(isspace(*argument))
	argument++;

    if(!*argument){
	send_to_char("You need to join *something*!\n\r",ch);
	return ERROR_SYNTAX;
    }

    /* Seems to me we could use some semantics based on who/where/when */
    if(!(org=get_org_by_name(argument))) {
        send_to_char("Join what?\n",ch);
        return ERROR_MISSING_TARGET;
    }

    sprintf(buf,"You express your interest in joining %s.",org->name);
    act(buf,FALSE,ch,0,0,TO_CHAR);
    sprintf(buf,"$n expresses interest in joining %s.",org->name);
    act(buf,FALSE,ch,0,0,TO_ROOM);
    add_event(ch->in_room,EVENT_JOIN,org->org_id,ch->id,ID_NOBODY);

    return OKAY;
}

int do_resign(struct char_data *ch, char *argument, int cmd)
{
    /* not sure if this is necessary yet */
    return OKAY;
}

int do_admit(struct char_data *ch, char *argument, int cmd)
{
    char charname[MAX_INPUT_LENGTH],orgname[MAX_INPUT_LENGTH],buf[200];
    struct org_type *org;
/*    struct org_require *req;*/
    struct char_data *candidate;
/*    int found, i;*/

    argument_interpreter(argument,charname,orgname);

    candidate=get_char_room_vis(ch,charname);

    if(!candidate) {
	send_to_char("That person isn't here.\n\r",ch);
	return ERROR_MISSING_TARGET;
    }

    /* locate org somehow */
    org=get_org_by_name(orgname);

    if(!org) {
	act("Admit $N to what?",FALSE,ch,0,candidate,TO_CHAR);
	return ERROR_SYNTAX;
    }

    /* Check qualifications of the admitting agent */
    if(!get_char_org(ch,org->org_id) && !get_char_org(ch,ORG_ID_ADMIN)) {
        send_to_char("You aren't even a member yourself!\n\r",ch);
        return ERROR_AUTH;
    }

    if(get_char_org(candidate,org->org_id)) {
        act("$N is already a member!",FALSE,ch,0,candidate,TO_CHAR);
        return ERROR_ALREADY_DONE;
    }

    /* How do we know "level" necessary, or whatever? */

/*	for(req=org->requires;req;req=req->next) {
	/ * Would be nice to have feedback here...* /
	if(

    }*/

    /* If we got this far, let's get into the org */
    start_org(candidate,org);
    sprintf(buf,"$n is now a member of %s.",org->name);
    act(buf,FALSE,candidate,0,0,TO_ROOM);
    sprintf(buf,"You are now a member of %s.",org->name);
    act(buf,FALSE,candidate,0,0,TO_CHAR);

    return OKAY;
}

int do_practice(struct char_data *ch, char *argument, int cmd)
{
    struct char_skill_data *sk;
    int skill_num;

    /* Question of where to practice, how we know it's possible, etc. */

    for(; *argument == ' ';argument++) ;

    if(!*argument) {
	send_to_char("Practice what?\n\r",ch);
	return ERROR_SYNTAX;
    }

    /*** parse arg to skill number ***/
    /*skill_num = ???;*/
    if(!skill_num || !(sk=get_skill(ch,skill_num))) {
	send_to_char("You don't know how to practice that.\n\r",ch);
	return ERROR_MISSING_TARGET;
    }

    if(1/*some pract_cost magic values*/) {
	send_to_char("You don't need to practice that.\n\r",ch);
	return ERROR_NO_SENSE;
    }

    if(sk->learned==100) {
	send_to_char("You are already learned in that category.\n",ch);
	return ERROR_FULL;
    }

    /* Do the actual practicing */

    return OKAY;
}

/* What was this command supposed to do? */
int do_privileges(struct char_data *ch, char *argument, int cmd)
{

    return OKAY;
}

int do_learn(struct char_data *ch, char *argument, int cmd)
{

    return OKAY;
}

int do_promote(struct char_data *ch, char *argument, int cmd)
{
    ;
    return OKAY;
}

int do_demote(struct char_data *ch, char *argument, int cmd)
{

    return OKAY;
}

int do_skills(struct char_data *ch, char *argument, int cmd)
{
    struct char_skill_data *sk;
    int i=0;
    char buf[120],buf2[120];

    for(sk=ch->skills;sk;sk=sk->next) {
        /* Get english for this sometime */
        sprintf(buf,"Skill %s: %s, ", skills[sk->skill_num],
                                    rating(sk->learned,100,0));
        switch(sk->period_type) {
            case PERIOD_MANA:
                sprintf(buf2,"requires minimum %d mana per use.\n",sk->period);
                strcat(buf,buf2);
                break;
            case PERIOD_DAILY:
                sprintf(buf2,"can be used %d times per day.\n",sk->period);
                strcat(buf,buf2);
                break;
            case PERIOD_NONE:
                strcat(buf,"unlimited use.\n");
                break;
            case PERIOD_USEMOVES:
                sprintf(buf2,"uses %d movement points each use.\n",sk->period);
                strcat(buf,buf2);
                break;
            case PERIOD_PERMOVE:
                sprintf(buf2,"uses %d movement points on each location change.\n",sk->period);
                strcat(buf,buf2);
                break;
            default:
                strcat(buf,"and you have no idea how to use it!\n");
                break;
        }
        send_to_char(buf,ch);
        i++;
    }
    sprintf(buf,"You have %d skill%s.\n",i,(i==1 ? "" : "s"));
    send_to_char(buf,ch);

    return OKAY;
}

int do_orginfo(struct char_data *ch, char *argument, int cmd)
{
    struct org_type *o;
    char buf[256];

    for(o=org_list;o;o=o->next) {
        sprintf(buf,"%3d> %s [%s] %d\n",o->org_id,o->name,org_kind[o->org_type],
            o->org_flags);
        send_to_char(buf,ch);
    }
    return OKAY;
}
