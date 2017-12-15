/* ************************************************************************
*  file: structs.h , Structures        .                  Part of DIKUMUD *
*  Usage: Declarations of central data structures                         *
*  Version for Copper III                                                 *
************************************************************************* */


#ifndef STRUCTS_H
#define STRUCTS_H

#include <sys/types.h>

typedef signed char sbyte;
typedef unsigned char ubyte;
typedef signed short int sh_int;
typedef unsigned short int ush_int;
typedef char bool;
typedef char byte;

typedef signed long int CHAR_ID;
#define ID_NOBODY 0

#define MAX_STRING_LENGTH   4096
#define MAX_INPUT_LENGTH     160 

/* This is a struct to keep lists of things handy (mostly char's) */

#define ATTACH_OBJ      1
#define ATTACH_CHAR     2
#define ATTACH_ROOM     3

struct generic {
    short attach_type;
    union {
        struct char_data *ch;
        struct obj_data *obj;
        struct room_data *room;
    } attached_to;
};


struct list_elem {
    struct generic data;
    struct list_elem *next;
};


/* The following defs are for obj_data  */

/* For 'obj_type' */

#define ITEM_LIGHT      0
#define ITEM_SCROLL     1
#define ITEM_WAND       2
#define ITEM_STAFF      3
#define ITEM_WEAPON     4
#define ITEM_FIREWEAPON 5
#define ITEM_MISSILE    6
#define ITEM_POTION     7
#define ITEM_WORN       8
#define ITEM_TRAP       9
#define ITEM_CONTAINER 10
#define ITEM_NOTE      11
#define ITEM_DRINKCON  12
#define ITEM_KEY       13
#define ITEM_FOOD      14
#define ITEM_MONEY     15
#define ITEM_PEN       16
#define ITEM_BOAT      17
#define ITEM_BOOK      18
#define ITEM_POISON    19
#define ITEM_TELEPORT  20 /* To teleport to specific places */
#define ITEM_TIMER     21 /* A standard interface for "timer" objects */
#define ITEM_STONE     22
#define ITEM_EXIT      23
#define ITEM_TRASH     24 /* Here, because I *might* remove it */

/* Bitvector For 'wear_flags' */

/* I debate whether this should be a bitvector... how many things can
really be worn in multiple places?? We can save memory and make it byte
*/
#define ITEM_WEAR_FINGER       1
#define ITEM_WEAR_NECK         2
#define ITEM_WEAR_BODY         4
#define ITEM_WEAR_HEAD         8
#define ITEM_WEAR_LEGS        16
#define ITEM_WEAR_FEET        32
#define ITEM_WEAR_HANDS       64 
#define ITEM_WEAR_ARMS       128
#define ITEM_WEAR_SHIELD     256
#define ITEM_WEAR_ABOUT      512 
#define ITEM_WEAR_WAISTE    1024
#define ITEM_WEAR_WRIST     2048
#define ITEM_WIELD          4096
#define ITEM_HOLD           8192

/* Bitvector for 'extra_flags' */

#define ITEM_TAKE            1 
#define ITEM_HUM             2
#define ITEM_DARK            4
#define ITEM_LOCK            8
#define ITEM_EVIL           16
#define ITEM_INVISIBLE      32
#define ITEM_MAGIC          64
#define ITEM_NODROP        128
#define ITEM_IGNORE        256
#define ITEM_ANTI_GOOD     512 /* not usable by good people    */
#define ITEM_ANTI_EVIL    1024 /* not usable by evil people    */
#define ITEM_ANTI_NEUTRAL 2048 /* not usable by neutral people */
#define ITEM_SECRET	  4096 /* Can only be found by search command */
#define ITEM_FLOAT        8192 /* Will it float in a liquid */
#define ITEM_HIDDEN      16384 /* Even detectable at all? */
#define ITEM_SNOOPER     32768 /* Can "see" through another item */
#define ITEM_SNOOPEE     65536 /* Can be "seen" through by another item */
#define ITEM_ARCHIVE    131072 /* Used in item counting routines */

/* Some different kind of liquids */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_COKE       15
#define LIQ_ORANGE_J   16
#define LIQ_TOMATO_J   17
#define LIQ_APPLE_J    18
#define LIQ_GUAVA_J    19

/* for containers */

#define CONT_CLOSEABLE      1
#define CONT_PICKPROOF      2
#define CONT_CLOSED         4
#define CONT_LOCKED         8
#define CONT_MAGICPROOF     16
#define CONT_WIZLOCKED      32
#define CONT_CORPSE         64 /* gruesome, ain't it? */

struct ban_t /* Struct to ban certain sites */
{
    char *name;		/* Name of site */
    struct ban_t *next;	/* next in list */
};

struct extra_descr_data
{
    char *keyword;                 /* Keyword in look/examine          */
    char *description;             /* What to see                      */
    struct extra_descr_data *next; /* Next in list                     */
};

#define OBJ_NOTIMER    -7000000

/* This will be the base structure which takes over for value[4]     */
/* - each item will contain a list of these, which will really be    */
/* unique structure for that item type - for example, obj_info_armor */
/* or obj_info_weapon...                                             */

struct obj_info
{
    struct obj_info *next;
    sh_int obj_type;
};

struct obj_info_boat
{
    struct obj_info *next;
    sh_int obj_type;
};

struct obj_info_wand
{
    struct obj_info *next;
    sh_int obj_type;
};

struct obj_info_weapon
{
    struct obj_info *next;
    sh_int obj_type;

    int dice_num, dice_size; /* how much damage? */
    int damage_type;
};

#define MONEY_COPPER_HUMAN	0
#define MONEY_SILVER_HUMAN	1
#define MONEY_GOLD_HUMAN	2
#define MONEY_COPPER_ELF	10

struct obj_info_money
{
    struct obj_info *next;
    sh_int obj_type;

    int money_type;
    int amount;
};

struct obj_info_staff
{
    struct obj_info *next;
    sh_int obj_type;
};

struct obj_info_scroll
{
    struct obj_info *next;
    sh_int obj_type;
};

struct obj_info_potion
{
    struct obj_info *next;
    sh_int obj_type;
};

struct obj_info_wear
{
    struct obj_info *next;
    sh_int obj_type;

    int wear_flags;     /* Where you can wear it */
    int ac;
    int warmth; /* keep prestige?? */
};

struct obj_info_light
{
    struct obj_info *next;
    sh_int obj_type;

    int brightness;
    int light_flags;
    int hours_left;
};

struct obj_info_exit
{
    struct obj_info *next;
    int obj_type;

    char *general_description;       /* When look DIR.                  */
    sh_int dir;                      /* which way is the exit?          */
    sh_int exit_info;                /* Exit info                       */
    sh_int key;                      /* Key's number (-1 for no key)    */
    sh_int to_room;                  /* Where direction leeds (NOWHERE) */
};

struct obj_info_key
{
    struct obj_info *next;
    int obj_type;

    int key_number;
};

struct obj_info_container
{
    struct obj_info *next;
    int obj_type;

    int lock_state;
    int lock_number;
    int capacity; /* how much can it contain, anyway? */
};

struct obj_info_food
{
    struct obj_info *next;
    int obj_type;

    int food_type;
    int filling;
};

struct obj_info_drink
{
    struct obj_info *next;
    int obj_type;

    int drink_type;
    int capacity;
    int how_full;
};

struct obj_info_poison
{
    struct obj_info *next;
    int obj_type;

    int poison_type;
    int quantity;
};

/* This should enable a better interface than in c2 */
struct obj_info_teleport
{
    struct obj_info *next;
    int obj_type;

    int to_location;
    char trig_type;
    char charges;
    int trigger;
    int data1;
};

struct obj_flag_data
{
    int hits;           /* How much use/damage for the obj  */
    int maxhits;        /* Original value for the object    */
/*    int value[4];       / * Values of the item TO BE REMOVED */
    int extra_flags;    /* If it hums,glows etc             */
    int weight;         /* Weight, what else?               */
    int cost;           /* Value when sold (gp.)            */
    long bitvector;     /* To set chars bits                */
};

/* material types */
#define MATERIAL_UNKNOWN 0
#define MATERIAL_METAL   1
#define MATERIAL_CLOTH   2
#define MATERIAL_STONE   3
#define MATERIAL_PLANT   4
#define MATERIAL_WATER   5
#define MATERIAL_WOOD    6
#define MATERIAL_CLAY    7
#define MATERIAL_DIRT    8
#define MATERIAL_ENERGY  9
#define MATERIAL_CRYSTAL 10
#define MATERIAL_FLESH   11
#define MATERIAL_CHEM    12
#define MATERIAL_OIL     13

#define SUBMAT_GENERIC   0

#define MAT_MULTICOLOR   1
#define MAT_FLAMMABLE    2

#define COLOR_WHITE      1
#define COLOR_BLACK      2
#define COLOR_GREY       4
#define COLOR_BLUE       8
#define COLOR_RED        16
#define COLOR_GREEN      32
#define COLOR_YELLOW     64
#define COLOR_PURPLE     128
#define COLOR_ORANGE     256
#define COLOR_BROWN      512
#define COLOR_TRANSL     1024  /* see-through */

/* this discludes translucent */
#define COLOR_ANY (COLOR_WHITE|COLOR_BLACK|COLOR_GREY|COLOR_BLUE|COLOR_RED| \
                   COLOR_GREEN|COLOR_YELLOW|COLOR_PURPLE|COLOR_ORANGE| \
                   COLOR_BROWN)

struct material_info {
    char *name;
    char *adjlist;
    int flags;
    int colors;
    float density;
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_material
{
    int type;
    int subtype;
    int purity; /* if 90% pure, then 10% is unknown, burned off if purified */
    int weight;
    struct obj_material *next;
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affect {
    int location;      /* Which ability to change (APPLY_XXX) */
    int modifier;     /* How much it changes by              */
    struct obj_affect *next;
};

/* ======================== Structure for object ========================= */
struct obj_data
{
    sh_int item_number;            /* Where in data-base               */
    sh_int in_room;                /* In what room -1 when conta/carr  */ 
    struct obj_flag_data obj_flags;/* Object information               */
    struct obj_material *material; /* What's it made from?             */
    struct obj_affect *affected;   /* Which abilities in PC to change  */
    struct obj_info *info;         /* list of info structs             */

    char *name;                    /* Title of object :get etc.        */
    char *description ;            /* When in room                     */
    char *short_description;       /* when worn/carry/in cont.         */
    char *action_description;      /* What to write when used          */
    struct extra_descr_data *ex_description; /* extra descriptions     */
    struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
    int equipped_as;               /* How is it equipped?              */

    struct obj_data *in_obj;       /* In what object NULL when none    */
    struct obj_data *contains;     /* Contains objects                 */
    struct obj_data *snoop;        /* For SNOOPER/SNOOPEE flags        */

    struct magic *magic;

    struct obj_data *next_content; /* For 'contains' lists             */
    struct obj_data *next;         /* For the object list              */
};
/* ======================================================================= */

/* The following defs are for room_data  */

#define NOWHERE    -1    /* nil reference for room-database    */

/* Bitvector For 'room_flags' */
/* signed short int for these */
#define DARK            1    /* Off really means just lit by Sun/Moon */
#define INDOORS         8
#define HOLY           16

#define NO_MAGIC      128
#define ARENA        1024
#define NO_PRECIP    4096    /* Only applies to outdoor rooms (canopies...) */
#define SINGLE_FILE  8192    /* Only for rooms with 1 or 2 exits */
#define JAIL        16384    /* Incarceration!!! - Can't quit out of room */
#define NO_TELEPORT 32768    /* Don't teleport in */

/* For 'dir_option' */

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5
#define NORTHEAST      6    /* New ones */
#define SOUTHEAST      7
#define SOUTHWEST      8
#define NORTHWEST      9

#define EX_ISDOOR		1
#define EX_CLOSED		2
#define EX_LOCKED		4
#define EX_RSCLOSED	8
#define EX_RSLOCKED	16
#define EX_PICKPROOF 32

/* For 'Sector types' */

#define SECT_INSIDE          0
#define SECT_CITY            1
#define SECT_FIELD           2
#define SECT_FOREST          3
#define SECT_HILLS           4
#define SECT_MOUNTAIN        5
#define SECT_WATER_SWIM      6
#define SECT_WATER_NOSWIM    7
#define SECT_NO_GROUND       8

/* For Alignment of Room Mana Gain */
#define MANA_ALL_ALIGNS      0
#define MANA_GOOD            1
#define MANA_NEUTRAL         2
#define MANA_EVIL            3


/* trap information */

#define TRAP_NONE    0
#define TRAP_PIT     1
#define TRAP_ARROW   2
#define TRAP_DART    3
#define TRAP_GAS     4
#define TRAP_ACID    5
#define TRAP_FLAME   6

/* This will have to be defined in terms of the exit object (LATeR)... */
struct current_data
{
    char *msg_char;                  /* What is seen when flowing away  */
    char *msg_to_room;
    char *msg_from_room;
    char *msg_item_to_room;
    char *msg_item_from_room;
    int strength;                   /* The strength of the flow        */
    int chance;                     /* The likelihood of flow          */
    int flags;                      /* Whatever I think of later...    */
};

#define RDESC_NORMAL 0
#define RDESC_FIRE   1
#define RDESC_FLOOD  2

#define MAX_RDESC    8

/* ========================= Structure for room ========================== */
struct room_data
{
    sh_int number;               /* Rooms number                       */
    sh_int zone;                 /* Room zone (for resetting)          */
    int sector_type;             /* sector type (move/hide)            */
    char *name;                  /* Rooms name 'You are ...'           */
    char *description[MAX_RDESC];/* Shown when entered                 */
    struct extra_descr_data *ex_description; /* for examine/look       */
    int room_flags;              /* DEATH,DARK ... etc                 */
    int light;                   /* Number of lightsources in room     */
    int chance_fall;             /* % Chance of falling                */
    int fall_flags;              /* <not defined yet>                  */
    int mana;                    /* Room mana gain                     */
    int mana_alignment;          /* Alignment for room mana gain       */
    int trap;                    /* Traps placed in room               */
    int (*funct)();              /* special procedure                  */
    
    struct obj_data *contents;   /* List of items in room              */
    struct char_data *people;    /* List of NPC / PC in room           */
    struct magic *magic;
};
/* ======================================================================== */

/* The following defs and structures are related to char_data   */

/* For 'equipment' */

#define UNEQUIPPED      0

#define HOLD_HAND1      1
#define HOLD_HAND2      2
#define WEAR_FINGER_R   3
#define WEAR_FINGER_L   4
#define WEAR_NECK_1     5
#define WEAR_NECK_2     6
#define WEAR_BODY       7
#define WEAR_HEAD       8
#define WEAR_LEGS       9
#define WEAR_FEET      10
#define WEAR_HANDS     11
#define WEAR_ARMS      12
#define WEAR_ABOUT     13
#define WEAR_WAIST     14
#define WEAR_WRIST_R   15
#define WEAR_WRIST_L   16

#define HOLD           -1 /* Generic versions */
#define WEAR_FINGER    -2
#define WEAR_NECK      -3
#define WEAR_WRIST     -4
#define WIELD          -5


/* For 'char_payer_data' */

#define MAX_TOUNGE  3     /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_SKILLS  53    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_WEAR    16
#define MAX_AFFECT  25    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

/* Bitvector for 'affected_by' */
#define AFF_BLIND             1
#define AFF_INVISIBLE         2
#define AFF_DETECT_EVIL       4
#define AFF_DETECT_INVISIBLE  8
#define AFF_DETECT_MAGIC      16
#define AFF_SENSE_LIFE        32
#define AFF_HOLD              64
#define AFF_SANCTUARY         128

#define AFF_CURSE             1024
#define AFF_FLAMING           2048
#define AFF_POISON            4096
#define AFF_PROTECT_EVIL      8192
#define AFF_PARALYSIS         16384
#define AFF_MORDEN_SWORD      32768
#define AFF_FLAMING_SWORD     65536

#define AFF_SLEEP             131072
#define AFF_DODGE             262144
#define AFF_SNEAK             524288
#define AFF_HIDE              1048576
#define AFF_FEAR              2097152
#define AFF_CHARM             4194304
#define AFF_FOLLOW            8388608
#define AFF_WIMPY            16777216

#define AFF_INFRARED         33554432
#define AFF_LEVITATE         67108864
#define AFF_FLY             134217728
#define AFF_AWARE           268435456

/* modifiers to char's abilities */

#define APPLY_NONE              0
#define APPLY_STR               1
#define APPLY_DEX               2
#define APPLY_INT               3
#define APPLY_WIS               4
#define APPLY_CON               5
#define APPLY_SEX               6
#define APPLY_CLASS             7
#define APPLY_LEVEL             8
#define APPLY_AGE               9
#define APPLY_CHAR_WEIGHT      10
#define APPLY_CHAR_HEIGHT      11
#define APPLY_MANA             12
#define APPLY_HIT              13
#define APPLY_MOVE             14
#define APPLY_GOLD             15
#define APPLY_EXP              16
#define APPLY_AC               17
#define APPLY_ARMOR            17
#define APPLY_HITROLL          18
#define APPLY_DAMROLL          19
#define APPLY_SAVING_PARA      20
#define APPLY_SAVING_ROD       21
#define APPLY_SAVING_PETRI     22
#define APPLY_SAVING_BREATH    23
#define APPLY_SAVING_SPELL     24


/* 'class' for PC's */
#define CLASS_MAGIC_USER  1
#define CLASS_CLERIC      2
#define CLASS_THIEF       3
#define CLASS_WARRIOR     4

/* 'class' for NPC's */
#define CLASS_OTHER       0 /* These are not yet used!   */
#define CLASS_UNDEAD      1 /* But may soon be so        */
#define CLASS_HUMANOID    2 /* Idea is to use general    */
#define CLASS_ANIMAL      3 /* monster classes           */
#define CLASS_DRAGON      4 /* Then for example a weapon */
#define CLASS_GIANT       5 /* of dragon slaying is pos. */
#define CLASS_DEMON       6
#define CLASS_BIRD        7
#define CLASS_INSECT      8

/* sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* positions */
#define POS_DEAD      0
#define POS_MORTALLYW 1
#define POS_INCAP     2
#define POS_STUNNED   3
#define POS_SLEEP     4
#define POS_REST      5
#define POS_SIT       6
#define POS_SWIM      7
#define POS_STAND     8
#define POS_LEVITATE  9 /* Just a bit above the ground */
#define POS_FLY      10 /* Able to go upwards without support */

#define NUM_POS      11


/* for mobile actions: specials.act */
#define ACT_ISNPC        8     /* This bit is set for use with IS_NPC()   */
#define ACT_NICE_THIEF  16     /* Set if a thief should NOT be killed     */
#define ACT_OUTLAW       64    /* For jailing pc's */
	       /* aggressive only attack sleeping players */
#define ACT_NOSAVE     256     /* Don't save this */
#define ACT_CLEANUP    512     /* Call extract_char() next time around */

#define ACT_SPEC_DIE            8388608 /* Overrides corpse-making, calls */
		    /* Spec_proc instead (only when   */
		    /* ACT_SPEC is true) -Sman        */



/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data
{
    sh_int hours, day, month;
    sh_int year;
};

/* These data contain information about a players time data */
struct time_data
{
 time_t birth;    /* This represents the characters age                */
 time_t logon;    /* Time of the last logon (used to calculate played) */
 int played;      /* This is the total accumulated time played in secs */
};

struct char_player_data
{
    char *name;    	    /* PC / NPC s name (kill ...  )         */
    char *short_descr;  /* for 'actions'                        */
    char *long_descr;   /* for 'look'.. Only here for testing   */
    char *description;  /* Extra descriptions                   */
    char *title;        /* PC / NPC s title                     */
    int hometown;       /* PC s Hometown (zone)                 */
    int talks[MAX_TOUNGE]; /* PC s Tounges 0 for NPC           */
    struct time_data time; /* PC s AGE in days                 */
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_ability_data
{
    sh_int str;
    sh_int intel;
    sh_int wis;
    sh_int dex;
    sh_int con;
    sh_int chr;
};

#define SIZE_TINY 1
#define SIZE_SMALL 2
#define SIZE_MEDIUM 3
#define SIZE_LARGE 4
#define SIZE_HUGE 5
#define SIZE_GIGANTIC 6

/* for gain_condition() */
#define DRUNK 0
#define FULL 1
#define THIRST 2
#define SLEEP 3

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_WOUNDS 5
struct char_phys_data
{
    struct char_ability_data abilities;
    short int species;
    int size;
    int sex;
    int weight;
    int height;
    int thirst, hunger, exhaustion, impairment;

    sh_int mana;         
    sh_int max_mana;     /* Not useable may be erased upon player file renewal*/
    sh_int hit;   
    sh_int max_hit;      /* Max hit for NPC                         */
    sh_int move;  
    sh_int max_move;     /* Max move for NPC                        */

    sh_int armor;        /* Internal -100..100, external -10..10 AC */

    int wounds[MAX_WOUNDS][2];

    struct char_phys_data *next; /* Do we want this here? */
};



/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data
{
    int exp;             /* The experience of the player            */
    sh_int kills;        /* How many things has this person killed? */
    int score;           /* How many "points" scored?               */

    sh_int hitroll;       /* Any bonus or penalty to the hit roll    */
    sh_int damroll;       /* Any bonus or penalty to the damage roll */

    int luck;            /* Nasty concept, but let's see what happens*/
};

#define DISP_MV 1
#define DISP_MA 2
#define DISP_HP 4

struct char_special_data
{
    struct char_data *fighting; /* Opponent                             */
    int attack;                 /* Attack mode chosen                   */
    struct char_data *hunting;  /* Hunting person..                     */

    int position;           /* Standing or ...                         */
    unsigned long act;       /* administrative flags                    */
    int activity;           /* Current primary activity                */

    int skills_to_learn;    /* How many can you learn yet this level   */

    int carry_weight;        /* Carried weight                          */
    int carry_items;        /* Number of items carried                 */
    int lights_carried;     /* How many active lights are carried      */
    sh_int was_in_room;      /* storage of location for linkdead people */
    sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)             */

    int damnodice;           /* The number of damage dice's            */
    int damsizedice;         /* The size of the damage dice's          */
    int last_direction;      /* The last direction the monster went    */
    int attack_type;          /* The Attack Type Bitvector for NPC's    */
    int alignment;            /* +-1000 for alignments                  */

    int dispflags;
 
    int spec[4];		/* Some misc values for spec_proc mobs */

    sh_int ovl_timer;	/* For preventing (n)pc's from overloading */
    sh_int ovl_count;	/* the mud from saying, shouting, etc */

    int warmth;		/* So people can protect themselves from */
		/* extreme heat and cold */

    int jail_time;          /* How long left to spend in jail */
    struct char_data *arrest_by; /* Is this person on the way to jail?*/
    struct char_data *arrest_link; /* for lists of arrested people */
/*	char arrest_flags (resist arrest?) For future use */
    struct char_data *witnessing;
    struct char_data *witness_vict;
    int witness_cmd;

    struct list_elem *cron_vars; 
};

#define ARENA_NOTPLAYING	0
#define ARENA_BLUE_PLR		1
#define ARENA_BLUE_CAPT		2
#define ARENA_RED_PLR		3
#define ARENA_RED_CAPT		4
#define ARENA_WATCHER		5	/* Unimplemented at this time*/

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_skill_data
{
    short skill_num; /* org?? */
    sh_int learned;           /* % chance for success 0 = not learned   */
    int period;
    sh_int period_type;
    long int last;

    struct char_skill_data *next;
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

#define MAX_DOMAINS      5

struct char_org_data
{
    int org_id;                    /* Which organization?         */
    int member_level;             /* For those .orgs with levels */
    long permissions;              /* What can this person do?    */
    int participation;            /* How active is this person?  */
    int experience;                /* Overall in-the-field total  */
    int domains[MAX_DOMAINS];     /* Where is there power?       */
    struct char_org_data *next;
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct affected_type
{
    sh_int type;
    sh_int duration;      /* For how long its effects will last      */
    sh_int modifier;       /* This is added to apropriate ability     */
    sh_int location;        /* Tells which ability to change(APPLY_XXX)*/
    long bitvector;       /* Tells which bits to set (AFF_XXX)       */

    struct affected_type *next;
};

struct follow_type
{
    struct char_data *follower;
    struct follow_type *next;
};

/* ================== Structure for player/non-player ===================== */
struct char_data
{
    sh_int nr;                            /* monster nr (pos in file)     */
    CHAR_ID id;                           /* a *unique* id per player/mob */
    sh_int in_room;                       /* Location                     */

    struct char_player_data player;       /* Normal data                  */
    struct char_phys_data *physical;      /* Physical characteristics     */
    struct char_ability_data tmpabilities;/* The abilities we will use    */
    struct char_point_data points;        /* Points                       */
    struct char_special_data specials;    /* Special plaing constants     */
    struct char_skill_data *skills;       /* Skills                  */
    struct char_org_data *orgs;           /* Organizational info     */

    struct affected_type *affected;       /* affected by what spells      */
    struct magic *magic;
    struct event *events;

    struct obj_data *carrying;            /* Head of list                 */
    struct descriptor_data *desc;         /* NULL for mobiles             */
    struct player_prefs *prefs;           /* What players prefer          */
    struct char_mob_data *mobinfo;        /* Info only for mobs           */

    struct char_data *next_in_room;     /* For room->people - list        */
    struct char_data *next;             /* For either monster or ppl-list */
    struct char_data *next_fighting;    /* For fighting list              */

    struct follow_type *followers;        /* List of chars followers      */
    struct char_data *master;             /* Who is char following?       */
};


/* ======================================================================== */

struct dex_skill_type
{
    sh_int p_pocket;
    sh_int p_locks;
    sh_int traps;
    sh_int sneak;
    sh_int hide;
};

struct dex_app_type
{
    sh_int reaction;
    sh_int miss_att;
    sh_int defensive;
};

struct str_app_type
{
    sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
    sh_int todam;    /* Damage Bonus/Penalty                */
    sh_int carry_w;  /* Maximum weight that can be carrried */
    sh_int wield_w;  /* Maximum weight that can be wielded  */
};

struct wis_app_type
{
    byte bonus;       /* how many bonus skills a player can */
	     /* practice pr. level                 */
};

struct int_app_type
{
    byte learn;       /* how many % a player learns a spell/skill */
};

struct con_app_type
{
    sh_int hitp;
    sh_int shock;
};

#endif /* !defined(STRUCTS_H) */
