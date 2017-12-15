/* ************************************************************************
*  file: player.c , Player action module.                 Part of DIKUMUD *
*  Usage: Procedures generating 'intelligent' behavior in the mobiles.    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */



#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "event.h"
#include "skills.h"
#include "interp.h"
#include "error.h"
#include "player.h"
#include "proto.h"

extern struct room_data *world;
extern struct zone_data *zone_table;
extern char log_buf[];

int player_action(struct char_data *ch)
{
    struct event *e;
    int action_decided=FALSE;
    char command[MAX_INPUT_LENGTH];

    while((e=ch->events) && !action_decided) {
        /* Take care of anything we care about */
        switch(e->type) {
            /*** These are governed by the discon strings ***/
            case EVENT_HUNGER:
                if(!ch->desc && ch->prefs->discon[DISCON_HUNGER][0]) {
                    strcpy(command,ch->prefs->discon[DISCON_HUNGER]);
                    action_decided=TRUE;
                }
                break;
            case EVENT_THIRST:
                if(!ch->desc && ch->prefs->discon[DISCON_THIRST][0]) {
                    strcpy(command,ch->prefs->discon[DISCON_THIRST]);
                    action_decided=TRUE;
                }
                break;
            case EVENT_NOISE:
                if(!ch->desc && ch->prefs->discon[DISCON_NOISE][0]) {
                    strcpy(command,ch->prefs->discon[DISCON_NOISE]);
                    action_decided=TRUE;
                }
                /* Wake up the player? */
                break;
            case EVENT_ATTACK:
                if(!ch->desc && ch->prefs->discon[DISCON_ATTACK][0]) {
                    strcpy(command,ch->prefs->discon[DISCON_ATTACK]);
                    action_decided=TRUE;
                }
                break;

            case EVENT_ACTION:
                /* spec procs might be processed here... */
                break;
            case EVENT_DEATH:
                break;
            case EVENT_ARRIVE:
                /* Aggressive mobs should attack, scared ones flee */
                break;
            case EVENT_DEPART:
                /* Following/tracking follow */
                break;

            /* Negotiation events */
            case EVENT_OFFER:
                /* Decline if not linked */
                break;
            case EVENT_ACCEPT:
                break;
            case EVENT_DECLINE:
                break;
            case EVENT_WITHDRAW:
                break;

            /* Personal interaction */
            case EVENT_SOCIAL:
                
                break;
            case EVENT_SPEECH:
                break;

            default:
                break;
        }

        /* Then dispose of the event */
        ch->events=e->next;
        free_event(e);
    }
    if(action_decided) {
        command_interpreter(ch,command);
        return TRUE;
    }
    return FALSE;
}

/* selection of what to do on events */
int do_discon(struct char_data *ch,char *argument,int cmd)
{
    char kind[MAX_INPUT_LENGTH],action[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    int event;

    if(!ch->prefs) {
        send_to_char("You are unable to do such things.\n\r",ch);
        return ERROR_INTERNAL;
    }

    half_chop(argument,kind,action);

    /* either no args */
    if(!*kind && !*action) {
        sprintf(buf,"HUNGER: %s\n\r",ch->prefs->discon[DISCON_HUNGER]);
        send_to_char(buf,ch);
        sprintf(buf,"THIRST: %s\n\r",ch->prefs->discon[DISCON_THIRST]);
        send_to_char(buf,ch);
        sprintf(buf,"NOISE: %s\n\r",ch->prefs->discon[DISCON_NOISE]);
        send_to_char(buf,ch);
        sprintf(buf,"ATTACK: %s\n\r",ch->prefs->discon[DISCON_ATTACK]);
        send_to_char(buf,ch);
        return OKAY;
    }

    /* or "two" args */
    if(!*kind || !*action) {
        send_to_char("When do you want to do what during disconnect?\n\r",ch);
        return ERROR_SYNTAX;
    }

    if(!strcmp(kind,"hunger"))
        event=DISCON_HUNGER;
    else if(!strcmp(kind,"thirst"))
        event=DISCON_THIRST;
    else if(!strcmp(kind,"attack"))
        event=DISCON_ATTACK;
    else if(!strcmp(kind,"noise"))
        event=DISCON_NOISE;
    else {
        send_to_char("Unknown disconnect event.\n\r",ch);
        return ERROR_SYNTAX;
    }

    /* We might want to check the syntax of the action string sometime */

    strcpy(ch->prefs->discon[event],action);
    return OKAY;
}
