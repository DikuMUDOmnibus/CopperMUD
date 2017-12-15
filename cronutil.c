/* Header stuff later... */

/*
working comment area:

The hard part here is going to be conditional operations, trapping events
that the scripts might care about. - perhaps we could have a cron_proc
attached to mobs (rooms/items?) to figure those out...
*/



#include <string.h>
#include <ctype.h>

#include "structs.h" /* need this? */
#include "db.h"
#include "cron.h"
#include "utils.h"
#include "interp.h"
#include "error.h"
#include "proto.h"

/* extern procedures */

struct cron_var *get_var(struct cron *c,int type,char *name);
void send_admin(char *text);
char *strdup();

/* local procedures */

struct exp *read_exp(char **input);
char *read_arg(char **input);
char *read_one_arg(char **input);
void read_bool(struct cron_quantum *c,char **input);
struct cron *new_cron();
struct script *get_script(int num);
void init_list(struct cron_list *l);
void enqueue_cron(struct cron *c,int where);
void unqueue_cron(struct cron *c);
bool eval_bool(struct cron *c,struct cron_quantum *q);

/* extern variables */

extern char log_buf[];

/* local variables */

struct cronbase cronbase;

char *cron_commands[] = {
    "nop",
    "var",
    "log",
    "actor",
    "setdoor",
    "load",
    "echo",
    "message",
    "speed",
    "restart",
    "run",
    "set",
    "if",
    "endif",
    "while",
    "endwhile",
    "wait",
    "\n"
};

void boot_cron()
{
    FILE *fl;
    int number,index;
    struct script *newscript;
    struct cron_quantum *newquant;
    char *temp,buffer[100],buf1[100],*txt;


    /* Initialize the cron lists */
    init_list(&cronbase.fast_crons);
    init_list(&cronbase.medium_crons);
    init_list(&cronbase.slow_crons);

    cronbase.scripts = NULL;

    /* load in the scripts */
    if(!(fl = fopen(CRON_FILE,"r"))) {
	perror("cron");
	exit(0);
    }

    while(1) {
	fscanf(fl," #%d\n",&number);
	temp=fread_string(fl);
	if(*temp=='$')
	    break;

	CREATE(newscript,struct script,1);

	newscript->name = temp;
	newscript->number = number;
	newscript->next_script = cronbase.scripts;
	cronbase.scripts = newscript;
	newscript->first = newscript->last = NULL;

	while(1) {
	    if(!(fgets(buffer,99,fl)))
		break;
/*log(buffer);*/

	    for(index=0;buffer[index];index++)
		if(buffer[index]=='\n') {
		    buffer[index]='\0';
		    break;
		}

	    for(txt=buffer;*txt && isspace(*txt);txt++)
		;
	    if(!*txt)            /* A blank line */
		continue;

	    if(*txt == '*') /* A comment line */
		continue;

	    if(*txt == 'S') /* End of script */
		break;


	    for(index=0;!isspace(txt[index]);index++)
		buf1[index]=txt[index];
	    buf1[index]='\0';
	    txt += index;

	    index=search_block(buf1,cron_commands,TRUE);

	    if(index==-1) {      /* An unintelligible line */
		sprintf(log_buf,
		    "CRON: Unintelligible script line:\n --> %s", buffer);
		log(log_buf);
		continue;
	    }

	    /* Insert the command */
	    CREATE(newquant,struct cron_quantum,1);
	    newquant->next = NULL;
	    if(newscript->last) {
		newscript->last->next = newquant;
		newscript->last = newquant;
	    } else
		newscript->first=newscript->last=newquant;

	    newquant->cmd = index;

	    switch(index) {
		case QC_LOG:
		    newquant->arg = read_arg(&txt);
		    break;

		case QC_ACTOR_COMMAND:
		/*	newquant->exp1 = read_exp(&txt);*/
		/*    newquant->data = atoi(txt);
		    while(isdigit(*txt)||isspace(*txt))
			txt++;*/
		    newquant->name = read_one_arg(&txt);
		    newquant->arg = read_arg(&txt);
		    break;

		case QC_SET_DOOR:
		    break;

		case QC_LOAD:
		/*	half_chop(...,buf1,buf2);*/
		    if(!strcmp(buf1,"mob"))
			newquant->data=LOAD_MOB;
		    else if(!strcmp(buf1,"obj"))
			newquant->data=LOAD_OBJ;
		    else {
			log("CRON: Invalid load");
			continue;
		    }
		    /* Which mob */
		    newquant->exp1 = read_exp(&txt);
		    newquant->exp2 = read_exp(&txt);
		    break;

		case QC_ECHO:
		    txt=one_argument(txt,buf1);
		    if(!strcmp(buf1,"room"))
			newquant->data=ECHO_ROOM;
		    else if(!strcmp(buf1,"zone"))
			newquant->data=ECHO_ZONE;
		    else {
			log("CRON: Invalid echo");
			continue;
		    }
		    newquant->exp1 = read_exp(&txt);
		    newquant->arg = read_arg(&txt);
		    break;
		case QC_MESSAGE:
		/*	...get_var?(&txt);*/
		    newquant->arg = read_arg(&txt);
		    break;
		case QC_SPEED:
		    if(!strcmp(txt,"fast"))
			newquant->data=SPEED_FAST;
		    else if(!strcmp(txt,"medium"))
			newquant->data=SPEED_MEDIUM;
		    else if(!strcmp(txt,"slow"))
			newquant->data=SPEED_SLOW;
		    break;
		case QC_RESTART: /* Args can be ignored */
		    break;
		case QC_IF:
		    read_bool(newquant,&txt);
		    break;
		case QC_WHILE:
		    read_bool(newquant,&txt);
		    break;
		default:
		    break;
	    }
	}
    }
    fclose(fl);
}

void temp_cron_hack()
{
    /* TMEP TEMP TEMP TEMP */
    struct cron *temp_cron;
    struct char_data *mob;
    struct cron_var *temp_var;

    /* TEMP TEMP TEMP TEMP TEMP TEMP */
    /* way to force a test cron to be created before we create the new zones*/
    temp_cron=new_cron();
    mob=load_bio_to(701,VIRTUAL,real_room(3001));
    free(mob->player.name);
    free(mob->player.short_descr);
    mob->player.name = strdup("mayor");
    mob->player.short_descr = strdup("Hey, it's the mayor!");
    mob->id = 12345;

    CREATE(temp_var, struct cron_var, 1);

    temp_var->type = VAR_ACTOR;
    temp_var->num = 1;
    temp_var->value = 12345;
    temp_var->next = NULL;
    temp_var->name = "1";

    temp_cron->vars = temp_var;
    temp_cron->mode = CRON_GO;

    temp_cron->script = get_script(3000);

    temp_cron->executing = temp_cron->script->first;

    enqueue_cron(temp_cron,SPEED_FAST);
}

struct script *get_script(int num)
{
    struct script *s;

    for(s=cronbase.scripts;s;s=s->next_script)
	if(num==s->number)
	    return(s);
    return(NULL);
}

struct cron *new_cron()
{
    struct cron *new;

    CREATE(new,struct cron,1);

    new->vars=NULL;
    new->stack=NULL;

    return(new);
}

void free_cron(struct cron *c)
{
    struct cron_var *v,*temp;

    /* Eliminate vars */
    for(v=c->vars;v;v=temp) {
	temp=v->next;
	free(v);
    }

    /* Unlink the cron */
    unqueue_cron(c);

    free(c);
}

void init_list(struct cron_list *l)
{
    l->Head = (struct cron *)&l->Tail;
    l->Tail = NULL;
    l->TailPred = (struct cron *)&l->Head;
}

void enqueue_cron(struct cron *c,int where)
{
    switch(where) {
	case SPEED_FAST:
	    c->Succ = (struct cron *)&cronbase.fast_crons.Tail;
	    c->Pred = cronbase.fast_crons.TailPred;
	    cronbase.fast_crons.TailPred->Succ = c;
	    cronbase.fast_crons.TailPred = c;
	    break;
	case SPEED_MEDIUM:
	    c->Succ = (struct cron *)&cronbase.medium_crons.Tail;
	    c->Pred = cronbase.medium_crons.TailPred;
	    cronbase.medium_crons.TailPred->Succ = c;
	    cronbase.medium_crons.TailPred = c;
	    break;
	case SPEED_SLOW:
	    c->Succ = (struct cron *)&cronbase.slow_crons.Tail;
	    c->Pred = cronbase.slow_crons.TailPred;
	    cronbase.slow_crons.TailPred->Succ = c;
	    cronbase.slow_crons.TailPred = c;
	    break;
    }
}

void unqueue_cron(struct cron *c)
{
    c->Succ->Pred = c->Pred;
    c->Pred->Succ = c->Succ;
    c->Pred = c->Succ = NULL;
}

char *read_one_arg(char **input)
{
    int i;
    char *new=NULL,*start;

    while(isspace(**input))
	(*input)++;

    start=*input;

    for(i=0;**input && **input!='+' && **input!=' ';i++,(*input)++)
	;

    if(i) {
	CREATE(new,char,i+1);
	strncpy(new,start,i);
	new[i]='\0';
    }

    return(new);
}

char *read_arg(char **input)
{
    int i;
    char *new=NULL,*start;

    while(isspace(**input))
	(*input)++;

    start=*input;

    for(i=0;**input && **input!='+' && **input!=';';i++,(*input)++)
	;

    if(i) {
	CREATE(new,char,i+1);
	strncpy(new,start,i);
	new[i]='\0';
    }

    return(new);
}

void read_bool(struct cron_quantum *c,char **input)
{
    char *i;

    while(isspace(**input))
	(*input)++;

    if(!strncasecmp(*input,"true",4)) {
	c->data = BOOL_TRUE;
	*input +=4;
	return;
    } else if(!strncasecmp(*input,"false",5)) {
	c->data = BOOL_FALSE;
	*input +=5;
	return;
    }
return;

    c->exp1 = read_exp(input);

    while(isspace(**input))
	(*input)++;

    /* Now bite off the operator, if it exists */

    i=*input;

    if(!strncmp(i,"<>",2)) {
	c->data = BOOL_NOT_EQUAL;
	i +=2;
    } else if(!strncmp(i,"=",1)) {
	c->data = BOOL_EQUAL;
	i++;
    }

    c->exp2 = read_exp(input);
}

int read_op(char **input)
{
    return OP_PLUS; /* not being used, so it doesn't matter */
}

bool end_exp(char **text)
{
    char *check;

    check=*text;

    while(*check==' ' || *check=='\t')
	check++;

    if(*check==':' || *check=='\n' || *check=='\r' || *check=='\0') {
	*text=++check;
	return(TRUE);
    }

    return(FALSE);
}

struct exp *read_term(char **input)
{
    struct exp *new;
    int i;
    char buf[100];

    while(isspace(**input))
	(*input)++;

    if(isdigit(**input) || **input=='-') {
	i=atoi(*input);
	CREATE(new,struct exp,1);
	new->type=EXP_CONSTANT;
	new->data=i;

	/* Now, wind the pointer past the number */
	if(**input=='-')
	    (*input)++;
	while(isdigit(**input))
	    (*input)++;

	/* And return our value */
	return(new);
    } else if(**input=='$') {
	i=0;
	while(isalpha(**input)) {
	    buf[i]=**input;
	    i++; (*input)++;
	}
	buf[i]='\0';
	new->type = EXP_VARIABLE;
	new->name = strdup(buf);
    } /*other types here */

    return(NULL);
}

struct exp *read_factor(char **input)
{
    struct exp *new,*res;

    if(**input=='(') {
	res=read_exp(input);
	/* check for matching paren */
	return(res);
    }

    res=read_term(input);

    while(**input==' ' || **input=='\t')
	(*input)++;

    if(**input=='*' || **input=='/') {
	CREATE(new,struct exp,1);
	new->left=res;
	new->right=read_exp(input);
	new->type=EXP_OPERATOR;
	new->data=(**input=='*' ? OP_MULT : OP_DIVIDE);
	return(new);
    }
    return(res);
}

struct exp *read_exp(char **input)
{
    struct exp *new,*res;

    res=read_term(input);

    if(end_exp(input))
	return(res);

    if(**input=='+' || **input=='-') {
	CREATE(new,struct exp,1);
	new->left=res;
	new->right=read_exp(input);
	new->type=EXP_OPERATOR;
	new->data=(**input=='+' ? OP_PLUS : OP_MINUS);
    } else {
	log("error in read_exp");
	return(res);
    }
    return(new);
}

/* Now how to evaluate this thing */
int eval_exp(struct cron *c,struct exp *exp)
{
    int result;
    struct cron_var *var;

    if(!exp) { /* Should we get this at all? */
	log("BUG: Null expression passed to eval_exp!");
	return(0);
    }

    switch(exp->type) {
	case EXP_CONSTANT:
	    return(exp->data);
	    break;
	case EXP_OPERATOR:
	    switch(exp->data) {
		case OP_PLUS:
		    return(eval_exp(c,exp->left) +
		       eval_exp(c,exp->right));
		    break;
		case OP_MINUS:
		    return(eval_exp(c,exp->left) -
		       eval_exp(c,exp->right));
		    break;
		case OP_MULT:
		    return(eval_exp(c,exp->left) *
		       eval_exp(c,exp->right));
		    break;
		case OP_DIVIDE:
		    result=eval_exp(c,exp->right);
		    if(result == 0) {
			sprintf(log_buf,"CRON: Divide by zero in %s",c->script->name);
			log(log_buf);
			return(0);
		    }
		    return(eval_exp(c,exp->left) / result);
		    break;
		case OP_MODULUS:
		    return(eval_exp(c,exp->left) %
		       eval_exp(c,exp->right));
		    break;
		default:
		    break;
	    }
	    break;
	case EXP_FUNCTION:
	    switch(exp->data) {
		case FUNC_ROOM_OF:
		    break;
		case FUNC_ZONE_OF:
		    break;
		default:
		    break;
	    }
	    break;
	case EXP_VARIABLE:
	    var = get_var(c,VAR_NUM,exp->name);
	    return(var->value);
	    break;
	default:
	    break;
    }
    return(NULL);
}

bool eval_bool(struct cron *c,struct cron_quantum *q)
{
    int res1,res2;

    if(q->data!=BOOL_TRUE && q->data!=BOOL_FALSE) {
        res1 = eval_exp(c,q->exp1);
        res2 = eval_exp(c,q->exp2);
    }

    switch(q->data) {
	case BOOL_EQUAL:
	    return(res1==res2);
	    break;
	case BOOL_NOT_EQUAL:
	    return(res1!=res2);
	    break;
	case BOOL_GREATER:
	    return(res1>res2);
	    break;
	case BOOL_LESS:
	    return(res1<res2);
	    break;
	case BOOL_EQUAL_GREATER:
	    return(res1>=res2);
	    break;
	case BOOL_EQUAL_LESS:
	    return(res1<=res2);
	    break;
	case BOOL_TRUE:
	    return TRUE;
	    break;
	case BOOL_FALSE:
	    return FALSE;
	    break;
	default:
	    log("CRON: Bad comparator in eval_bool!");
	    break;
    }
    return(FALSE);
}

/* Support routine for do_cstat */

void cstat_list(struct char_data *ch,struct cron_list *l)
{
    int total=0,wacky=0,running=0,waiting=0,suspended=0;
    struct cron *c;
    char buf[MAX_STRING_LENGTH];

    for(c=l->Head;c->Succ;c=c->Succ) {
	total++;
	switch(c->mode) {
	    case CRON_GO:
		running++;
		break;
	    case CRON_WACKY:
		wacky++;
		break;
	    case CRON_WAIT:
		waiting++;
		break;
	    case CRON_SUSPENDED:
		suspended++;
		break;
	}
    }
    
    sprintf(buf,"%5d  %5d  %5d  %5d  %5d\n",running,waiting,suspended,
	wacky,total);
    send_to_char(buf,ch);
}

/* A command for showing stats about the cron system */
int do_cstat(struct char_data *ch,char *argument,int cmd)
{
    int i;
    char buf[MAX_STRING_LENGTH];
    struct script *s;

    send_to_char("Cron Stats    Run    Wait   Susp   Wacky  Total\n\n",ch);

    send_to_char("Fast crons:   ",ch);
    cstat_list(ch,&cronbase.fast_crons);

    send_to_char("Medium crons: ",ch);
    cstat_list(ch,&cronbase.medium_crons);

    send_to_char("Slow crons:   ",ch);
    cstat_list(ch,&cronbase.slow_crons);

    for(i=0,s=cronbase.scripts;s;s=s->next_script)
	i++;
    sprintf(buf,"\nThere are %d scripts available.\n\r",i);
    send_to_char(buf,ch);
    return OKAY;
}
