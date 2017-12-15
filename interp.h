/* ************************************************************************
*  file: Interp.h , Command interpreter module.           Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
************************************************************************* */

#ifndef INTERP_H
#define INTERP_H

int command_interpreter(struct char_data *ch, char *argument);
int search_block(char *arg, char **list, bool exact);
int old_search_block(char *argument,int begin,int length,char **list,int mode);
char lower( char c );
void argument_interpreter(char *argument, char *first_arg, char *second_arg);
char *one_argument(char *argument,char *first_arg);
int fill_word(char *argument);
void half_chop(char *string, char *arg1, char *arg2);
int is_abbrev(char *arg1, char *arg2);

/* Command flags */

#define CMD_ACTIVE   1  /* Commands that are "physical world" actions, as */
	    /* opposed to things like "who", "score", etc.    */

#define CMD_GUEST    2  /* Commands that guests can use */

#define CMD_ADMIN    4  /* Commands only for ADMIN chars */

typedef int COMMAND   (struct char_data *ch, char *argument, int cmd);

struct command_info {
    char *command_name;			/* String of command  */
    int command_number;			/* Num sent to proc   */
    byte min_position;			/* Min pos to do cmd  */
    COMMAND *command_pointer;		/* Pointer to command */
    byte flags;				/* Flags              */
};

#define OPENING_OPEN 1
#define OPENING_CLOSE 2
#define OPENING_LOCK 3
#define OPENING_UNLOCK 4
#define OPENING_PICK 5

COMMAND do_accept;
COMMAND do_admit;
COMMAND do_advance;
COMMAND do_alert;
COMMAND do_allow;
COMMAND do_ask;
COMMAND do_at;
COMMAND do_auction;
COMMAND do_backstab;
COMMAND do_balance;
COMMAND do_ban;
COMMAND do_bash;
COMMAND do_bioinfo;
COMMAND do_brief;
COMMAND do_bug;
COMMAND do_buy;
COMMAND do_calendar;
COMMAND do_cast;
COMMAND do_chgpos;
COMMAND do_close;
COMMAND do_compact;
COMMAND do_consider;
COMMAND do_credits;
COMMAND do_cstat;
COMMAND do_demote;
COMMAND do_deposit;
COMMAND do_discon;
COMMAND do_display;
COMMAND do_drink;
COMMAND do_drop;
COMMAND do_eat;
COMMAND do_echo;
COMMAND do_emote;
COMMAND do_enter;
COMMAND do_equipment;
COMMAND do_examine;
COMMAND do_exit;
COMMAND do_exits;
COMMAND do_extinguish;
COMMAND do_fill;
COMMAND do_flee;
COMMAND do_follow;
COMMAND do_force;
COMMAND do_gecho;
COMMAND do_gen_opening;
COMMAND do_get;
COMMAND do_give;
COMMAND do_goto;
COMMAND do_grab;
COMMAND do_group;
COMMAND do_help;
COMMAND do_hide;
COMMAND do_hit;
COMMAND do_idea;
COMMAND do_info;
COMMAND do_insult;
COMMAND do_inventory;
COMMAND do_jail;
COMMAND do_join;
COMMAND do_kick;
COMMAND do_kill;
COMMAND do_learn;
COMMAND do_leave;
COMMAND do_levels;
COMMAND do_light;
COMMAND do_load;
COMMAND do_localwho;
COMMAND do_lock;
COMMAND do_look;
COMMAND do_lookup;
COMMAND do_mode;
COMMAND do_move;
COMMAND do_msgecho;
COMMAND do_multiclass;
COMMAND do_murder;
COMMAND do_news;
COMMAND do_not_here;
COMMAND do_offer;
COMMAND do_open;
COMMAND do_order;
COMMAND do_orginfo;
COMMAND do_override;
COMMAND do_pardon;
COMMAND do_passwords;
COMMAND do_pick;
COMMAND do_poofIn;
COMMAND do_poofOut;
COMMAND do_pose;
COMMAND do_pour;
COMMAND do_practice;
COMMAND do_promote;
COMMAND do_purge;
COMMAND do_put;
COMMAND do_quaff;
COMMAND do_qui;
COMMAND do_quit;
COMMAND do_read;
COMMAND do_recite;
COMMAND do_release;
COMMAND do_remove;
COMMAND do_rent;
COMMAND do_reroll;
COMMAND do_rescue;
COMMAND do_restore;
COMMAND do_return;
COMMAND do_rub;
COMMAND do_save;
COMMAND do_say;
COMMAND do_score;
COMMAND do_search;
COMMAND do_secret;
COMMAND do_sell;
COMMAND do_setobj;
COMMAND do_setskill;
COMMAND do_setstat;
COMMAND do_setzone;
COMMAND do_shout;
COMMAND do_shutdow;
COMMAND do_shutdown;
COMMAND do_silently;
COMMAND do_sip;
COMMAND do_skills;
COMMAND do_sleep;
COMMAND do_sneak;
COMMAND do_snoop;
COMMAND do_social;
COMMAND do_sockets;
COMMAND do_start;
COMMAND do_stat;
COMMAND do_steal;
COMMAND do_string;
COMMAND do_switch;
COMMAND do_systats;
COMMAND do_taste;
COMMAND do_teleport;
COMMAND do_tell;
COMMAND do_time;
COMMAND do_title;
COMMAND do_trans;
COMMAND do_typo;
COMMAND do_unlock;
COMMAND do_users;
COMMAND do_wake;
COMMAND do_wear;
COMMAND do_weather;
COMMAND do_where;
COMMAND do_whisper;
COMMAND do_who;
COMMAND do_wield;
COMMAND do_wimpy;
COMMAND do_withdraw;
COMMAND do_wizearmuffs;
COMMAND do_wizhelp;
COMMAND do_wizlock;
COMMAND do_write;
COMMAND do_yell;

#endif /* !defined(INTERP_H)*/
