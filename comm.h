/* ************************************************************************
*  file: comm.h , Communication module.                   Part of DIKUMUD *
*  Usage: Prototypes and structures for communcation functions            *
************************************************************************* */

#ifndef COMM_H
#define COMM_H

/* ***********************************************************
*  The following structures are related to descriptor_data   *
*********************************************************** */


struct txt_block
{
    char *text;
    struct txt_block *next;
};

struct txt_q
{
    struct txt_block *head;
    struct txt_block *tail;
};



/* modes of connectedness */

#define CON_PLYNG    0
#define CON_NME	     1
#define CON_NEWPL    2
#define CON_PWDNRM   3
#define CON_INTRO1   4
#define CON_INTRO2   5
#define CON_QSEX     6
#define CON_RMOTD    7
#define CON_SLCT     8
#define CON_PWDNEW   11
#define CON_PWDNEW2  12
#define CON_PWDNCNF  13
#define CON_CLOSE    14
#define CON_READERS  15
#define CON_DELETE   16
#define CON_INTRIN   17 /* Final death sequence */
#define CON_TOMB     18
#define CON_TOMB2    19
#define CON_IDQ      20 /* Ident query */

struct snoop_data
{
    struct descriptor_data *snooping;
	/* Who is this char snooping */
    struct descriptor_data *snoop_by;
	/* And who is snooping on this char */
};

struct reader_type {
    int number;
    char *title;
    char *text;
    int items;
    int select[10];
    struct reader_type *next;
};

struct descriptor_data
{
    int descriptor;	              /* file descriptor for socket */
    char *name;                   /* Copy of the player name (pw bug) */
    char host[50];                /* hostname                   */
    char user[16];                /* user on that host          */
    int numeric;                  /* number of host             */
    int pos;                      /* position in player-file    */
    int connected;                /* mode of 'connectedness'    */
    int idle;                     /* how long with no command?  */
    int wait;                     /* wait for how many loops    */
    bool newline;                 /* newlines in input          */
    int page_size;                /* when to "press return"     */
    char *showstr_head;           /* for paging through texts   */
    char *showstr_point;          /*       -                    */
    char **str;                   /* for the modify-str system  */
    int max_str;                  /* -                          */
    int prompt_mode;              /* control of prompt-printing */
    char buf[MAX_STRING_LENGTH];  /* buffer for raw input       */
    char last_input[MAX_INPUT_LENGTH];/* the last input         */
    struct txt_q output;          /* q of strings to send       */
    struct txt_q input;           /* q of unprocessed input     */
    struct char_data *character;  /* linked to char             */
    struct char_data *original;   /* original char              */
    struct snoop_data snoop;      /* to snoop people.           */
    struct reader_type *reader;   /* current reader             */
    struct descriptor_data *next; /* link to next descriptor    */
};

#define TO_ROOM    0
#define TO_VICT    1
#define TO_NOTVICT 2
#define TO_CHAR    3

#define SEND_TO_Q(messg, desc)  write_to_q((messg), &(desc)->output)

#define OVL_PULSE         18  /* Heartbeat counts for overload prevention */
#define OVL_LIMIT          5  /* Max intrusive acts during ovl period */

#endif /* !defined(COMM_H) */
