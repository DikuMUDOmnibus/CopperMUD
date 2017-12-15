/* ************************************************************************
*  file: Interp.c , Command interpreter module.           Part of DIKUMUD *
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
#include "db.h"
#include "org.h"
#include "utils.h"
#include "event.h"
#include "error.h"
#include "proto.h"

#define NOT !
#define AND &&
#define OR ||

extern int log_all,req_passwd,override;
extern char *story,*menu,*welcome;
extern char motd[MAX_STRING_LENGTH],log_buf[];
extern struct mob_index_data *mob_index;
extern struct obj_index_data *obj_index;
extern struct room_data *world;

/* external fcntls */

void set_title(struct char_data *ch);
void init_char(struct char_data *ch);
int special(struct char_data *ch, int cmd, char *arg);
void log(char *str);
int default_loc(int hometown);




/* This max_command bit is a bit silly, but how to fix it? */

#define MAX_COMMANDS 279

struct command_info commands[MAX_COMMANDS] =
{
    /* Related to movement */
    { "look",      15,  POS_REST,   do_look,   CMD_ACTIVE|CMD_GUEST },
    { "north",     0,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "east",      1,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "south",     2,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "west",      3,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "up",        4,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "down",      5,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "northeast", 6,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "ne",        6,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "southeast", 7,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "se",        7,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "southwest", 8,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "sw",        8,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "northwest", 9,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "nw",        9,   POS_STAND,  do_move,   CMD_ACTIVE|CMD_GUEST },
    { "exits",     7,   POS_REST,   do_exits,  CMD_ACTIVE|CMD_GUEST },
    { "stand",     POS_STAND,    POS_REST,   do_chgpos,  CMD_ACTIVE|CMD_GUEST },
    { "sit",       POS_SIT,      POS_REST,   do_chgpos,  CMD_ACTIVE|CMD_GUEST },
    { "rest",      POS_REST,     POS_REST,   do_chgpos,  CMD_ACTIVE|CMD_GUEST },
    { "fly",       POS_FLY,      POS_STAND,  do_chgpos,  CMD_ACTIVE },
    { "levitate",  POS_LEVITATE, POS_STAND,  do_chgpos,  CMD_ACTIVE },
    { "sleep",     45,  POS_SLEEP,  do_sleep,  CMD_ACTIVE|CMD_GUEST },
    { "wake",      46,  POS_SLEEP,  do_wake,   CMD_ACTIVE|CMD_GUEST },

    /* Communication */
    { "say",       17,  POS_REST,   do_say,    CMD_ACTIVE|CMD_GUEST },
    { "shout",     18,  POS_REST,   do_shout,  CMD_ACTIVE },
    { "tell",      19,  POS_DEAD,   do_tell,   CMD_ACTIVE|CMD_GUEST },
    { "whisper",   83,  POS_REST,  do_whisper,  CMD_ACTIVE },
    { "ask",       86,  POS_REST,  do_ask,  CMD_ACTIVE },
    { "order",     87,  POS_REST,  do_order,  CMD_ACTIVE },
    { "'",         169, POS_REST,  do_say,  CMD_ACTIVE|CMD_GUEST },
    { "yell",      241, POS_REST,  do_yell,  CMD_ACTIVE },

    /* Related to items */
    { "get",       10,  POS_REST,   do_get,    CMD_ACTIVE },
    { "drink",     11,  POS_REST,   do_drink,  CMD_ACTIVE },
    { "eat",       12,  POS_REST,   do_eat,    CMD_ACTIVE },
    { "wear",      13,  POS_REST,   do_wear,   CMD_ACTIVE },
    { "wield",     14,  POS_REST,   do_wield,  CMD_ACTIVE },
    { "drop",      60,  POS_REST,  do_drop,  CMD_ACTIVE },
    { "read",      63,  POS_REST,  do_read,  CMD_ACTIVE|CMD_GUEST },
    { "pour",      64,  POS_STAND,  do_pour,  CMD_ACTIVE },
    { "grab",      65,  POS_REST,  do_grab,  CMD_ACTIVE },
    { "remove",    66,  POS_REST,  do_remove,  CMD_ACTIVE },
    { "put",       67,  POS_REST,  do_put,  CMD_ACTIVE },
    { "give",      72,  POS_REST,  do_give, CMD_ACTIVE },
    { "sip",       88,  POS_REST,  do_sip,  CMD_ACTIVE },
    { "taste",     89,  POS_REST,  do_taste,  CMD_ACTIVE },
    { "open",      OPENING_OPEN,  POS_SIT,  do_gen_opening,  CMD_ACTIVE },
    { "close",     OPENING_CLOSE, POS_SIT,  do_gen_opening,  CMD_ACTIVE },
    { "lock",      OPENING_LOCK, POS_SIT,  do_gen_opening,  CMD_ACTIVE },
    { "unlock",    OPENING_UNLOCK, POS_SIT,  do_gen_opening,  CMD_ACTIVE },
    { "write",     149, POS_STAND,  do_write,  CMD_ACTIVE },
    { "hold",      150, POS_REST,  do_grab,  CMD_ACTIVE },
    { "take",      167, POS_REST,  do_get,  CMD_ACTIVE  },
    { "fill",      240, POS_STAND,  do_fill,  CMD_ACTIVE },
    { "rub",       259, POS_DEAD,  do_rub,  CMD_ACTIVE },
    { "light",     276, POS_SIT,  do_light, 0 },
    { "extinguish",277, POS_SIT,  do_extinguish, 0 },

    /* Informational */
    { "score",     16,  POS_DEAD,   do_score,  CMD_GUEST },
    { "skills",    16,  POS_DEAD,   do_skills, CMD_GUEST },
    { "inventory", 20,  POS_DEAD,   do_inventory,  CMD_ACTIVE },
    { "help",      38,  POS_DEAD,   do_help,   CMD_GUEST },
    { "who",       39,  POS_DEAD,   do_who,    CMD_GUEST },
    { "news",      54,  POS_SLEEP,  do_news, CMD_GUEST },
    { "equipment", 55,  POS_SLEEP,  do_equipment,  CMD_ACTIVE },
    { "weather",   62,  POS_REST,  do_weather,0 },
    { "time",      76,  POS_DEAD,  do_time, CMD_GUEST },
    { "examine",   166, POS_SIT,  do_examine,  CMD_ACTIVE|CMD_GUEST },
    { "info",      168, POS_SLEEP,  do_info,0|CMD_GUEST },
    { "credits",   212, POS_DEAD,  do_credits,0|CMD_GUEST },
    { "localwho",  224, POS_DEAD,  do_localwho,0 },

    /* Miscellaneous */
    { "qui",       21,  POS_DEAD,   do_qui,    CMD_GUEST },
    { "insult",    33,  POS_REST,   do_insult, CMD_ACTIVE },
    { "buy",       56,  POS_REST,   do_buy,  CMD_ACTIVE },
    { "sell",      57,  POS_STAND,  do_sell,  CMD_ACTIVE },
    { "offer",     93,  POS_STAND,  do_offer,  CMD_ACTIVE },
    { "accept",    94,  POS_STAND,  do_accept,  CMD_ACTIVE },
    { "save",      69,  POS_SLEEP,  do_save,0 },
    { "quit",      73,  POS_DEAD,   do_quit, CMD_GUEST },
    { "idea",      80,  POS_DEAD,   do_idea,0 },
    { "typo",      81,  POS_DEAD,   do_typo,0 },
    { "bug",       82,  POS_DEAD,   do_bug,0 },
    { "follow",    91,  POS_REST,   do_follow,  CMD_ACTIVE },
    { "practice",  164, POS_SIT,    do_practice,  CMD_ACTIVE },
    { "practise",  170, POS_SIT,    do_practice,  CMD_ACTIVE },
    { "learn",     174, POS_SIT,    do_learn,  CMD_ACTIVE },
    { "brief",     199, POS_DEAD,   do_brief,   CMD_GUEST },
    { "quaff",     206, POS_REST,   do_quaff,  CMD_ACTIVE },
    { "recite",    207, POS_REST,   do_recite,  CMD_ACTIVE },
    { "compact",   213, POS_DEAD,   do_compact,CMD_GUEST },
    { "mode",      213, POS_DEAD,   do_mode,   CMD_GUEST },
    { "discon",    0,   POS_DEAD,   do_discon, 0 },
    { "msgecho",   0,   POS_DEAD,   do_msgecho,CMD_GUEST },
    { "display",   214, POS_DEAD,   do_display,CMD_GUEST },
    { "join",      228, POS_DEAD,   do_join,  CMD_ACTIVE },
    { "title",     246, POS_DEAD,  do_title,0 },
    { "search",    268, POS_STAND,  do_search,  CMD_ACTIVE },
    { "calendar",  274, POS_DEAD,  do_calendar, CMD_GUEST },
    { "admit",     278, POS_SIT, do_admit, 0 },
    { "learn",     279, POS_SIT, do_learn, 0 },
    { "promote",   280, POS_SIT, do_promote, 0 },
    { "demote",    281, POS_SIT, do_demote, 0 },

    /* Combat */
    { "kill",      25,  POS_SIT,  do_kill,   CMD_ACTIVE },
    { "hit",   70,  POS_SIT,  do_hit, CMD_ACTIVE },
    { "flee",  151, POS_SIT,  do_flee,  CMD_ACTIVE },
    { "consider", 201, POS_REST,  do_consider,  CMD_ACTIVE },
    { "murder", 236, POS_SIT,  do_murder,  CMD_ACTIVE },

    /* Skills */
    { "cast",  84,  POS_SIT,  do_cast,  CMD_ACTIVE },
    { "sneak", 152, POS_STAND,  do_sneak,  CMD_ACTIVE },
    { "hide",  153, POS_REST,  do_hide,  CMD_ACTIVE },
    { "backstab", 154, POS_STAND,  do_backstab,  CMD_ACTIVE },
    { "pick",  OPENING_PICK, POS_STAND,  do_gen_opening,  CMD_ACTIVE },
    { "steal", 156, POS_STAND,  do_steal,  CMD_ACTIVE },
    { "bash",  157, POS_STAND,  do_bash,  CMD_ACTIVE },
    { "rescue",158, POS_STAND,  do_rescue,  CMD_ACTIVE },
    { "kick",  159, POS_STAND,  do_kick,  CMD_ACTIVE },

    /* Administrative */
    { "echo",  41,  POS_SLEEP,  do_echo,  CMD_ACTIVE | CMD_ADMIN },
    { "force", 47,  POS_SLEEP,  do_force, CMD_ADMIN },
    { "transfer", 48,  POS_SLEEP,  do_trans,  CMD_ACTIVE | CMD_ADMIN },
    { "goto",  61,  POS_SLEEP,  do_goto,  CMD_ACTIVE | CMD_ADMIN },
    { "shutdow", 68,  POS_DEAD,  do_shutdow, CMD_ADMIN },
    { "string", 71,  POS_SLEEP,  do_string, CMD_ADMIN },
    { "stat",  74,  POS_DEAD,  do_stat, CMD_ADMIN },
    { "load",  77,  POS_DEAD,  do_load, CMD_ACTIVE | CMD_ADMIN },
    { "purge", 78,  POS_DEAD,  do_purge,  CMD_ACTIVE | CMD_ADMIN },
    { "shutdown", 79,  POS_DEAD,  do_shutdown, CMD_ADMIN },
    { "at",    85,  POS_DEAD,  do_at, CMD_ADMIN },
    { "snoop", 90,  POS_DEAD,  do_snoop, CMD_ADMIN },
    { "where", 173, POS_DEAD,  do_where, CMD_ADMIN },
    { "reroll",175, POS_DEAD,  do_reroll,CMD_ADMIN },
    { "restore",203, POS_DEAD,  do_restore,  CMD_ACTIVE | CMD_ADMIN },
    { "return",204, POS_DEAD,  do_return,0 },
    { "switch",205, POS_DEAD,  do_switch, CMD_ADMIN },
    { "users", 208, POS_DEAD,  do_users, CMD_ADMIN },
    { "teleport",217, POS_DEAD,  do_teleport,  CMD_ACTIVE | CMD_ADMIN },
    { "gecho",  218, POS_DEAD,  do_gecho,  CMD_ACTIVE | CMD_ADMIN },
    { "start",  219, POS_DEAD,  do_start,  CMD_ACTIVE },
    { "setstat",223, POS_DEAD,  do_setstat, CMD_ADMIN },
    { "wizlock", 226, POS_DEAD,  do_wizlock, CMD_ADMIN },
    { "systats", 232, POS_DEAD,  do_systats, CMD_ADMIN },
    { "sockets", 234, POS_DEAD,  do_sockets, CMD_ADMIN },
    { "release", 235, POS_DEAD,  do_release, CMD_ADMIN },
    { "ban",    238, POS_DEAD,  do_ban, CMD_ADMIN },
    { "allow",  239, POS_DEAD,  do_allow, CMD_ADMIN },
    { "passwords", 260, POS_DEAD,  do_passwords, CMD_ADMIN },
    { "setzone",   261, POS_DEAD,  do_setzone, CMD_ADMIN},
    { "override",  263, POS_DEAD,  do_override, CMD_ADMIN },
    { "silently",  265, POS_DEAD,  do_silently, CMD_ADMIN },
    { "alert",     266, POS_DEAD,  do_alert,  CMD_ACTIVE | CMD_ADMIN },
    { "jail",      267, POS_DEAD,  do_jail,  CMD_ACTIVE | CMD_ADMIN },
    { "secret",    271, POS_STAND,  do_secret, CMD_ADMIN },
    { "lookup",    272, POS_DEAD,  do_lookup, CMD_ADMIN },
    { "setobj",    273, POS_DEAD,  do_setobj, CMD_ADMIN },
    { "cstats",    275, POS_DEAD,  do_cstat, CMD_ADMIN },
    { "orginfo",   276, POS_DEAD,  do_orginfo, CMD_ADMIN },
    { "bioinfo",   277, POS_DEAD,  do_bioinfo, CMD_ADMIN },

    /* Social commands */
    { "kiss",      0,   POS_REST,   do_social, CMD_ACTIVE },
    { "smile",     1,  POS_REST,   do_social, CMD_ACTIVE },
    { "cackle",    2,  POS_REST,   do_social, CMD_ACTIVE },
    { "laugh",     3,  POS_REST,   do_social, CMD_ACTIVE },
    { "giggle",    4,  POS_REST,   do_social, CMD_ACTIVE },
    { "shake",     5,  POS_REST,   do_social, CMD_ACTIVE },
    { "growl",     6,  POS_REST,   do_social, CMD_ACTIVE },
    { "scream",    7,  POS_REST,   do_social, CMD_ACTIVE },
    { "comfort",   8,  POS_REST,   do_social, CMD_ACTIVE },
    { "nod",       9,  POS_REST,   do_social, CMD_ACTIVE },
    { "sigh",      10,  POS_REST,   do_social, CMD_ACTIVE },
    { "hug",       11,  POS_REST,  do_social,  CMD_ACTIVE },
    { "nuzzle",    12,  POS_REST,  do_social,  CMD_ACTIVE },
    { "cry",       13,  POS_REST,  do_social,  CMD_ACTIVE },
    { "poke",      14,  POS_REST,  do_social,  CMD_ACTIVE },
    { "grin",      15,  POS_REST,  do_social,  CMD_ACTIVE },
    { "bow",       16,  POS_STAND,  do_social,  CMD_ACTIVE },
    { "applaud",   17, POS_REST,  do_social,  CMD_ACTIVE },
    { "burp",      18, POS_REST,  do_social,  CMD_ACTIVE },
    { "chuckle",   19, POS_REST,  do_social,  CMD_ACTIVE },
    { "clap",      20, POS_REST,  do_social,  CMD_ACTIVE },
    { "cough",     21, POS_REST,  do_social,  CMD_ACTIVE },
    { "curtsey",   22, POS_STAND,  do_social,  CMD_ACTIVE },
    { "frown",     23, POS_REST,  do_social,  CMD_ACTIVE },
    { "gasp",      24, POS_REST,  do_social,  CMD_ACTIVE },
    { "glare",     25, POS_REST,  do_social,  CMD_ACTIVE },
    { "groan",     26, POS_REST,  do_social,  CMD_ACTIVE },
    { "moan",      27, POS_REST,  do_social,  CMD_ACTIVE },
    { "pout",      28, POS_REST,  do_social,  CMD_ACTIVE },
    { "shiver",    29, POS_REST,  do_social,  CMD_ACTIVE },
    { "shrug",     30, POS_REST,  do_social,  CMD_ACTIVE },
    { "slap",      31, POS_REST,  do_social,  CMD_ACTIVE },
    { "smirk",     32, POS_REST,  do_social,  CMD_ACTIVE },
    { "sneeze",    33, POS_REST,  do_social,  CMD_ACTIVE },
    { "snicker",   34, POS_REST,  do_social,  CMD_ACTIVE },
    { "sniff",     35, POS_REST,  do_social,  CMD_ACTIVE },
    { "spit",      36, POS_STAND,  do_social,  CMD_ACTIVE },
    { "stare",     37, POS_REST,  do_social,  CMD_ACTIVE },
    { "strut",     38, POS_STAND,  do_social,  CMD_ACTIVE },
    { "thank",     39, POS_REST,  do_social,  CMD_ACTIVE },
    { "twiddle",   40, POS_REST,  do_social,  CMD_ACTIVE },
    { "wave",      41, POS_REST,  do_social,  CMD_ACTIVE },
    { "wink",      42, POS_REST,  do_social,  CMD_ACTIVE },
    { "yawn",      43, POS_REST,  do_social,  CMD_ACTIVE },
    { "tickle",    44, POS_REST,  do_social,  CMD_ACTIVE },
    { "pat",       45, POS_REST,  do_social,  CMD_ACTIVE },
    { "beg",       46, POS_REST,  do_social,  CMD_ACTIVE },
    { "cringe",    47, POS_REST,  do_social,  CMD_ACTIVE },
    { "grovel",    48, POS_REST,  do_social,  CMD_ACTIVE },
    { "nudge",     49, POS_REST,  do_social,  CMD_ACTIVE },
    { "peer",      50, POS_REST,  do_social,  CMD_ACTIVE },
    { "point",     51, POS_REST,  do_social,  CMD_ACTIVE },
    { "ponder",    52, POS_REST,  do_social,  CMD_ACTIVE },
    { "snarl",     53, POS_REST,  do_social,  CMD_ACTIVE },
    { "taunt",     54, POS_REST,  do_social,  CMD_ACTIVE },
    { "think",     55, POS_REST,  do_social,  CMD_ACTIVE },
    { "whine",     56, POS_REST,  do_social,  CMD_ACTIVE },
    { "yodel",     57, POS_REST,  do_social,  CMD_ACTIVE },
    { "tap",       58, POS_REST,  do_social,  CMD_ACTIVE },
    { "wince",     59, POS_DEAD,  do_social,  CMD_ACTIVE },
    { "drool",     60, POS_DEAD,  do_social,  CMD_ACTIVE },
    { "embrace",   61, POS_DEAD,  do_social,  CMD_ACTIVE },

/* marker */
    { "\n",        -1, POS_DEAD,  NULL,       CMD_ACTIVE }
};

char *fill[]=
{ "in",
 "from",
 "with",
 "the",
 "on",
 "at",
 "to",
 "\n"
};

int search_block(char *arg, char **list, bool exact)
{
    register int i,l;

    l=strlen(arg);

    if(exact) {
	for(i=0; **(list+i) != '\n'; i++)
	    if (!strcasecmp(arg, *(list+i)))
		return(i);

    } else {
	if (!l)
	    l=1; /* Avoid "" to match the first available string */

	for(i=0; **(list+i) != '\n'; i++)
	    if (!strncasecmp(arg, *(list+i), l))
		return(i);

    }

    return(-1);
}

int search_block_offset(char *arg, char **list, bool exact,int size,int offset)
{
    register int i,l;

    l=strlen(arg);

    if(exact) {
	for(i=0; **(char **)(i*size+offset+(int)list) != '\n'; i++) {
	    if (!strcasecmp(arg, *(char **)(i*size+offset+(int)list)))
		return(i);
        }

    } else {
	if (!l)
	    l=1; /* Avoid "" to match the first available string */

	for(i=0; **(char **)(i*size+offset+(int)list) != '\n'; i++)
	    if (!strncasecmp(arg, *(char **)(i*size+offset+(int)list), l))
		return(i);

    }

    return(-1);
}


int old_search_block(char *argument,int begin,int length,char **list,int mode)
{
    int guess, found, search;
    
    /* If the word contain 0 letters, then a match is already found */
    found = (length < 1);

    guess = 0;

    /* Search for a match */

    if(mode)
    while ( NOT found AND *(list[guess]) != '\n' )
    {
	found=(length==strlen(list[guess]));
	for(search=0;( search < length AND found );search++)
	    found=(*(argument+begin+search)== *(list[guess]+search));
	guess++;
    } else {
	while ( NOT found AND *(list[guess]) != '\n' ) {
	    found=1;
	    for(search=0;( search < length AND found );search++)
		found=(*(argument+begin+search)== *(list[guess]+search));
	    guess++;
	}
    }

    return ( found ? guess : -1 ); 
}

int command_interpreter(struct char_data *ch, char *argument) 
{
    int look_at, cmd, begin, i, retcode;
    char swif_buf[MAX_INPUT_LENGTH+30];

    bool check_item_teleport(struct char_data *ch,char *arg,int cmd);

    sprintf(swif_buf,"CMD: %s %s",ch->player.name,argument);

    cmdlog(swif_buf);

    if(log_all){
	log(swif_buf);
    }

    /* Find first non blank */
	for (begin = 0 ; (*(argument + begin ) == ' ' ) ; begin++ );
    
    /* Find length of first word */
    for (look_at = 0; *(argument + begin + look_at ) > ' ' ; look_at++)

   		/* Make all letters lower case AND find length */
	*(argument + begin + look_at) = 
	LOWER(*(argument + begin + look_at));

    if(!look_at) {
        send_to_char("[no comment]\n\r",ch);
	return ERROR_SYNTAX;
    }

    
/*	cmd = old_search_block(argument,begin,look_at,command,0);*/

    cmd=-1;
    for(i=0;i<MAX_COMMANDS && *(commands[i].command_name) != '\n';i++) {
	if(!strncmp(commands[i].command_name,argument,look_at)) {
	    cmd=i;
	    break;
	}
    }

    if(cmd==-1) {
	send_to_char("Huh?!?\n\r",ch);
	return ERROR_SYNTAX;
    }

    /* Check admin commands */
    if((IS_NPC(ch) || !get_char_org(ch,ORG_ID_ADMIN)) &&
	    IS_SET(commands[cmd].flags,CMD_ADMIN)) {
	send_to_char("Huh?!?\n\r",ch);
	return ERROR_SYNTAX;
    }

    /* Check guests */
    if(!IS_NPC(ch) && IS_SET(ch->specials.act,ACT_NOSAVE) &&
	    !IS_SET(commands[cmd].flags,CMD_GUEST)) {
	send_to_char("You cannot use this command yet.\n\r",ch);
	return ERROR_FAILED;
    }

    if ( cmd>-1 && (commands[cmd].command_pointer != 0))
    {
	if( GET_POS(ch) < commands[cmd].min_position )
	    switch(GET_POS(ch))
	    {
		case POS_DEAD:
		    send_to_char("Lie still; you are DEAD!!! :-( \n\r", ch);
		break;
		case POS_INCAP:
		case POS_MORTALLYW:
		    send_to_char(
			"You are in a pretty bad shape, unable to do anything!\n\r",
			ch);
		break;

		case POS_STUNNED:
		    send_to_char(
		    "All you can do right now, is think about the stars!\n\r", ch);
		break;
		case POS_SLEEP:
		    send_to_char("In your dreams, or what?\n\r", ch);
		break;
		case POS_REST:
		    send_to_char("Nah... You feel too relaxed to do that..\n\r",
			ch);
		break;
		case POS_SIT:
		    send_to_char("Maybe you should get on your feet first?\n\r",ch);
		break;
	    }
	else {
/*
            if(commands[cmd].flags & CMD_ACTIVE)
                REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);*/

	    /* Hack for item_teleport objects */
	    if(check_item_teleport(ch, argument+begin+look_at,cmd))
		return; /* ??? what to return? how? */

	    /* If we get this far, call the actual command */
	    retcode = ((*commands[cmd].command_pointer)
		 (ch, argument + begin + look_at,
		 commands[cmd].command_number));
	    if(retcode==OKAY) {
		send_to_char("Ok.",ch);
                if(commands[cmd].flags & CMD_ACTIVE)
                    add_event(ch->in_room,EVENT_ACTION,cmd,ch->id,ID_NOBODY);
            } else if(retcode==ERROR_INTERNAL)
                fprintf(stderr,"BUG: Command was [%s] %s\n",GET_NAME(ch),
                        argument);
	}
    } else if ( cmd>-1 && (commands[cmd].command_pointer == 0)) {
	send_to_char(
	"Sorry, but that command has yet to be implemented...\n\r",
	    ch);
        return ERROR_INTERNAL;
    } else {
	send_to_char("Huh!?!\n\r", ch);
        return ERROR_SYNTAX;
    }
    return retcode;
}

void argument_interpreter(char *argument,char *first_arg,char *second_arg )
{
    int look_at, found, begin;

    found = begin = 0;

    do
    {
	/* Find first non blank */
	for ( ;*(argument + begin ) == ' ' ; begin++);

	/* Find length of first word */
	for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

	    /* Make all letters lower case,
	     AND copy them to first_arg */
	    *(first_arg + look_at) =
	    LOWER(*(argument + begin + look_at));

	*(first_arg + look_at)='\0';
	begin += look_at;

    }
    while( fill_word(first_arg));

    do
    {
	/* Find first non blank */
	for ( ;*(argument + begin ) == ' ' ; begin++);

	/* Find length of first word */
	for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

	    /* Make all letters lower case,
	     AND copy them to second_arg */
	    *(second_arg + look_at) =
	    LOWER(*(argument + begin + look_at));

	*(second_arg + look_at)='\0';
	begin += look_at;

    }
    while( fill_word(second_arg));
}

int is_number(char *str)
{
    int look_at;

    if(*str=='\0')
	return(0);

    for(look_at=0;*(str+look_at) != '\0';look_at++)
	if((*(str+look_at)<'0')||(*(str+look_at)>'9'))
	    return(0);
    return(1);
}

/*  Quinn substituted a new one-arg for the old one.. I thought returning a 
  char pointer would be neat, and avoiding the func-calls would save a
  little time... If anyone feels pissed, I'm sorry.. Anyhow, the code is
  snatched from the old one, so it outta work..

void one_argument(char *argument,char *first_arg )
{
    static char dummy[MAX_STRING_LENGTH];

    argument_interpreter(argument,first_arg,dummy);
}

*/


/* find the first sub-argument of a string, return pointer to first char in
 primary argument, following the sub-arg			            */
char *one_argument(char *argument, char *first_arg )
{
    int found, begin, look_at;

    found = begin = 0;

    do
    {
	/* Find first non blank */
	for ( ;isspace(*(argument + begin)); begin++);

	/* Find length of first word */
	for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

	    /* Make all letters lower case,
	     AND copy them to first_arg */
	    *(first_arg + look_at) =
	    LOWER(*(argument + begin + look_at));

	*(first_arg + look_at)='\0';
	begin += look_at;
    }
    while (fill_word(first_arg));

    return(argument+begin);
}
    
    






int fill_word(char *argument)
{
    return ( search_block(argument,fill,TRUE) >= 0);
}





/* determine if a given string is an abbreviation of another */
int is_abbrev(char *arg1, char *arg2)
{
    if (!*arg1)
     return(0);

    for (; *arg1; arg1++, arg2++)
     if (LOWER(*arg1) != LOWER(*arg2))
       return(0);

    return(1);
}




/* return first 'word' plus trailing substring of input string */
void half_chop(char *string, char *arg1, char *arg2)
{
    for (; isspace(*string); string++);

    for (; !isspace(*arg1 = *string) && *string; string++, arg1++);

    *arg1 = '\0';

    for (; isspace(*string); string++);

    for (; (*arg2 = *string); string++, arg2++);
}



int special(struct char_data *ch, int cmd, char *arg)
{
    register struct obj_data *i;
    register struct char_data *k;

    /* special in room? */
    if (world[ch->in_room].funct)
     if ((*world[ch->in_room].funct)(ch->in_room, ch, cmd, arg))
       return(1);

    /* special in inventory? */
    for (i = ch->carrying; i; i = i->next_content)
	if (i->item_number>=0)
	    if (obj_index[i->item_number].func)
		if ((*obj_index[i->item_number].func)
			(i, ch, cmd, arg))
		    return(1);

    /* special in mobile present? */
    for (k = world[ch->in_room].people; k; k = k->next_in_room)
     if ( IS_MOB(k) )
       if (mob_index[k->nr].func)
	if ((*mob_index[k->nr].func)(k, ch, cmd, arg))
	  return(1);

    /* special in object present? */
    for (i = world[ch->in_room].contents; i; i = i->next_content)
     if (i->item_number>=0)
       if (obj_index[i->item_number].func)
	if ((*obj_index[i->item_number].func)(i, ch, cmd, arg))
	  return(1);

    return(0);
}
