/* Header stuff later... 


working comment area:

The hard part here is going to be conditional operations, trapping events
that the scripts might care about. - perhaps we could have a cron_proc
attached to mobs (rooms/items?) to figure those out...

*/

#include <ctype.h>


#include <string.h>

#include "cron.h"
#include "structs.h" /* need this? */
#include "interp.h"
#include "utils.h"
#include "comm.h"
#include "error.h"
#include "proto.h"

/* global variables */
extern char log_buf[];


/* local variables */
extern struct cronbase cronbase;

/* extern procedures */
extern struct char_data *get_char_by_id(int id);

/* local procedures */
void exec_cron(struct cron *c);
void check_wait(struct cron *c);
void execute_cron(struct cron *c);
struct cron_var *get_var(struct cron *c,int type,char *name);
char *expand_arg(struct cron *c,char *arg);
struct cron_quantum *scan_for_match(struct cron_quantum *q, int match);
void push_stack(struct cron *c,struct stack_unit *frame);
struct stack_unit *pop_stack(struct cron *c);


/* The processing routines */
void scheduler()
{
    static int normal_timer=0,slow_timer=0;
    struct cron *c;

    for(c=cronbase.fast_crons.Head;c->Succ;c=c->Succ)
	exec_cron(c);

    if(++normal_timer==CRON_MEDIUM_TICKS) {
	normal_timer=0;
	for(c=cronbase.medium_crons.Head;c->Succ;c=c->Succ)
	    exec_cron(c);
    }

    if(++slow_timer==CRON_SLOW_TICKS) {
	slow_timer=0;
	for(c=cronbase.slow_crons.Head;c->Succ;c=c->Succ)
	    exec_cron(c);
    }
}

void exec_cron(struct cron *c)
{
    switch(c->mode) {
	case CRON_WAIT:
	    check_wait(c);
	    break;
	case CRON_GO:
	    execute_cron(c);
	    break;
	case CRON_WACKY:
	    /* Don't do anything at all */
	    break;
	default:
	    log("Bad cron type in exec_cron_list!");
	    c->mode=CRON_WACKY;
	    break;
    }
}

void check_wait(struct cron *c)
{

}

/* This is the real meat of the whole thing */
void execute_cron(struct cron *c)
{
    struct cron_quantum *q,*next;
    struct cron_var *var;
    struct char_data *ch;
    struct stack_unit *frame;

    q=c->executing;
    next=q->next;
/*
sprintf(log_buf,"EXECUTING %d: %s",q->cmd,q->arg);
log(log_buf);
*/
    switch(q->cmd) {
	case QC_NOP:
	    break; /* What do you expect? */
	case QC_LOG:
	    log(expand_arg(c,q->arg));
	    break;
	case QC_VAR:
	    break;
	case QC_ACTOR_COMMAND:
	    var=get_var(c,VAR_ACTOR, q->name);
	    if(var) {
		ch=get_char_from_id(var->value);
		if(ch) {
		    command_interpreter(ch,
			  expand_arg(c,q->arg));
		} else {
		    /* Check condition of lost actor */
		    log("Lost actor");
		}
	    } else
		log("cron: No actor found");
	    break;
	case QC_SET_DOOR:
	    break;
	case QC_LOAD:
	    if(q->data==LOAD_MOB) {
	    } else if(q->data==LOAD_OBJ) {
	    }
	    break;
	case QC_ECHO:
	    if(q->data==ECHO_ROOM) {
		send_to_room(q->arg,
		      real_room(eval_exp(c,q->exp1)));
	    } else if(q->data==ECHO_ZONE) {
/*				send_to_zone(q->arg,
		      real_zone(eval_exp(c,q->exp1)));
*/			}
	    break;
	case QC_MESSAGE:
	    break;
	case QC_SPEED:
	    /* Remove from the current list */
	    /* and place in the proper list */
	    unqueue_cron(c);
	    enqueue_cron(c,q->data);
	    break;
	case QC_RESTART:
	    /* This one will be hard to define...
	    do we dump uninit'd vars or all vars?
	    We DEFINITELY dump the stack...*/
	    next = c->script->first;
	    break;
	case QC_WAIT:
	    /* First to check to see if the wait is not */
	    /* already satisfied, then stick in the wait list */
	    break;
	case QC_IF:
	    if(eval_bool(c,q)) {
		CREATE(frame,struct stack_unit,1);
		frame->type = STACK_IF_LOOP;
		frame->ref = q;
		push_stack(c,frame);
	    } else {
		next = scan_for_match(q,QC_ENDIF);
		if(!next) {
		    c->mode = CRON_WACKY;
		    sprintf(log_buf,"CRON: no match for IF in '%s'", c->script->name);
		    log(log_buf);
		} else
		    next = next->next;
	    }
	    break;
	case QC_ENDIF:
	    frame = pop_stack(c);
	    if(frame->type != STACK_IF_LOOP) {
		c->mode = CRON_WACKY;
		log("CRON: mismatched if/endif");
		return;
	    }
	    free(frame);
	    break;
	case QC_WHILE:
	    if(eval_bool(c,q)) {
		CREATE(frame,struct stack_unit,1);
		frame->type = STACK_WHILE_LOOP;
		frame->ref = q;
		push_stack(c,frame);
	    } else {
		next = scan_for_match(q,QC_ENDWHILE);
		if(!next) {
		    c->mode = CRON_WACKY;
		    sprintf(log_buf,"CRON: no match for WHILE in '%s'", c->script->name);
		    log(log_buf);
		} else
		    next = next->next;
	    }
	    break;
	case QC_ENDWHILE:
	    frame = pop_stack(c);
	    if(frame->type != STACK_WHILE_LOOP) {
		c->mode = CRON_WACKY;
		log("CRON: mismatched while/endwhile");
		return;
	    }
	    free(frame);
	    break;
	default:
	    sprintf(log_buf,"Unknown command in script %s",c->script->name);
	    log(log_buf);
	    break;
    }
    if(!next)
	free_cron(c);
    else
	c->executing = next;
}

struct cron_quantum *scan_for_match(struct cron_quantum *q, int match)
{
    while(q) {
	if(q->cmd==QC_IF)
	    q = scan_for_match(q,QC_ENDIF);
	else if(q->cmd==QC_WHILE)
	    q = scan_for_match(q,QC_ENDWHILE);
	else if(q->cmd==match)
	    return(q);
	if(q)
	    q = q->next;
    }
    return(NULL);
}

void push_stack(struct cron *c,struct stack_unit *frame)
{
    frame->next = c->stack;
    c->stack = frame;
}

struct stack_unit *pop_stack(struct cron *c)
{
    struct stack_unit *frame;

    frame = c->stack;
    if(!frame) {
	c->mode = CRON_WACKY;
	sprintf(log_buf,"CRON: popping an empty stack in '%s'",
	    c->script->name);
	log(log_buf);
	return(NULL);
    }
    c->stack = frame->next;
    return(frame);
}

struct cron_var *get_var(struct cron *c,int type,char *name)
{
    struct cron_var *var;

    for(var=c->vars;var;var=var->next) {
fprintf(stderr,"Args: %d %s  Var: %d %s\n",type,name,var->type,var->name);
	if(var->type==type)
            if((var->name && name && !strcmp(var->name,name)))
	        return(var);
    }
    sprintf(log_buf,"cron_var %s not found!",name);
    log(log_buf);
    return(NULL);
}

char *expand_arg(struct cron *c,char *arg)
{
    static char buf[MAX_INPUT_LENGTH];
    int marker=0,num,sub_type;
    struct cron_var *var;

    while(*arg) {
	if(*arg == '@') {
	    sub_type = 0 /*<some default type>*/;

	/*	if(arg[1]=='...') {
	    }

	    if(isdigit(arg[1])) {
		arg++;
		num = atoi(arg);
		var = get_var(c,VAR_ACTOR,num);
		switch(sub_type) {

		}
		marker++;
		while(isdigit(arg[1]))
		    arg++;
	    } */
	} else
	    buf[marker++] = *arg;
	arg++;
    }

    /* Take care of that terminating null */
    buf[marker]='\0'; /* I hope that's right */

    return(buf);
}
