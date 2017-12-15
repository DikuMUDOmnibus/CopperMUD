/* ************************************************************************
*  file: db.h , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars booting world.                             *
************************************************************************* */

#ifndef DB_H
#define DB_H

/* data files used by the game system */

#define DFLT_DIR          "lib"           /* default data directory     */

#define WORLD_FILE        "world.wld"     /* room definitions            */
#define MOB_FILE          "world.mob"     /* monster prototypes          */
#define BIO_FILE          "world.bio"     /* biology prototypes          */
#define OBJ_FILE          "world.obj"     /* object prototypes           */
#define ZONE_FILE         "world.zon"     /* zone defs & command tables  */
#define ORG_FILE          "world.org"     /* organizational information  */
#define ORG_FIGURE_FILE   "world.fig"     /* organizational figures      */
#define RUMOR_FILE        "world.rumor"   /* current state of rumors     */

#define SYL_FILE          "syllables"     /* magic syllables             */
#define CREDITS_FILE      "credits"       /* for the 'credits' command   */
#define NEWS_FILE         "news"          /* for the 'news' command      */
#define MOTD_FILE         "motd"          /* messages of today           */
#define TIME_FILE         "time"          /* game calendar information   */
#define ID_FILE           "id"            /* keeping track of id's       */

#define IDEA_FILE         "ideas"         /* for the 'idea'-command      */
#define TYPO_FILE         "typos"         /*         'typo'              */
#define BUG_FILE          "bugs"          /*         'bug'               */

#define MESS_FILE         "messages"      /* damage message              */
#define SOCMESS_FILE      "actions"       /* messgs for social acts      */
#define HELP_KWRD_FILE    "help_table"    /* for HELP <keywrd>           */
#define HELP_PAGE_FILE    "help"          /* for HELP <CR>               */
#define INFO_FILE         "info"          /* for INFO                    */

#define MOB_LOOKUP        "lookup.mob"    /* for 'lookup'-command        */
#define OBJ_LOOKUP        "lookup.obj"    /* for 'lookup'-command        */

#define REAL 0
#define VIRTUAL 1

/* structure for the reset commands */
struct reset_com
{
    char command;   /* current command                      */ 
    sh_int if_flag;   /* if TRUE: exe only if preceding exe'd */
    int arg1;       /*                                      */
    int arg2;       /* Arguments to the command             */
    int arg3;       /*                                      */

    /* 
    *  Commands:              *
    *  'M': Read a mobile     *
    *  'O': Read an object    *
    *  'G': Give obj to mob   *
    *  'P': Put obj in obj    *
    *  'G': Obj to char       *
    *  'E': Obj to char equip *
    *  'D': Set state of door *
    */
};

#include "weather.h"   /* For the new weather system */

/* Flags for zones (non weather related) */
#define ZONE_SILENT     1
#define ZONE_SAFE       2
#define ZONE_HOMETOWN   4
#define ZONE_NEW_RESET  8       /* Need to write this version sometime */

/* All major terrains appearing in the zone */
/* These are used to generate random monsters */
#define TER_CITY               1
#define TER_VILLAGE            2
#define TER_CAVES              4
#define TER_DUNGEON            8

#define TER_FOREST            32
#define TER_PLAIN             64
#define TER_HILLS            128
#define TER_MOUNTAIN         256

#define TER_LAKE            2048
#define TER_MARSH           4096
#define TER_SWAMP           8192
#define TER_RIVER          16384
#define TER_SHALLOW        32768
#define TER_DEEPWATER      65536

#define TER_ASTRAL        131072
#define TER_ETHER         262144
#define TER_UNDERWORLD    524288
#define TER_OTHERPLANE   1048576

#define TER_PLANE    (TER_ASTRAL | TER_ETHER | TER_UNDERWORLD | TER_OTHERPLANE)
#define TER_SPECIAL  ( TER_PLANE )

#define MAX_TER_TYPES         22

/* The direction system used in the zone */
#define DIRS_COMPAT 0
#define DIRS_NORMAL 1
#define DIRS_NONE   2
#define DIRS_SHIP   3

/* zone definition structure. for the 'zone-table'   */
struct zone_data
{
    int number;             /* v-num of zone                      */
    char *name;             /* name of this zone                  */
    char *author;           /* Qui avez-faire cette zone?         */
    int lifespan;           /* how long between resets (minutes)  */
    int age;                /* current age of this zone (minutes) */
    int top;                /* upper limit for rooms in this zone */
    int real_top,real_bottom;/* real numbers for each end of zone */
    int flags;             /* For zonewide effects               */

    int reset_mode;         /* conditions for reset (see below)   */
    struct reset_com *cmd;  /* command table for reset	          */

    int terrain_type;
    int dir_system;
    int desc_mode;          /* Which room description to print    */
    struct climate climate;
    struct weather_data conditions;
    /*
    *  Reset mode:                              *
    *  0: Don't reset, and don't update age.    *
    *  1: Reset if no PC's are located in zone. *
    *  2: Just reset.                           *
    */
};

#define FREQ_COMMON	0
#define FREQ_UNCOMMON	1
#define FREQ_RARE	2
#define FREQ_VERYRARE	3
#define FREQ_SPECIAL	4
#define FREQ_UNIQUE	5
#define FREQ_NEVER      6

#define MAX_FREQ	7

/* element in monster index table   */
struct mob_index_data
{
    int virtual;    /* virtual number of this mob               */
    long pos;       /* file position of this field              */
    int number;     /* number of existing units of this mob    	*/
    int (*func)();  /* special procedure for this mob           */
    char *name;     /* easier way to do lookups                 */
    int loc_flags;  /* where does the mob hang out?             */
    int frequency;  /* how often does one come across them?     */
};


/* element in object index table   */
struct obj_index_data
{
    int virtual;    /* virtual number of this obj               */
    long pos;       /* file position of this field              */
    int number;     /* number of existing units of this obj	*/
    int (*func)();  /* special procedure for this obj           */
    char *name;     /* easier way to do lookups                 */
};




/* for queueing zones for update   */
struct reset_q_element
{
    int zone_to_reset;            /* ref to zone_data */
    struct reset_q_element *next;	
};



/* structure for the update queue     */
struct reset_q_type
{
    struct reset_q_element *head;
    struct reset_q_element *tail;
} reset_q;




struct help_index_element
{
    char *keyword;
    long pos;
};

#endif /* !defined(DB_H) */
