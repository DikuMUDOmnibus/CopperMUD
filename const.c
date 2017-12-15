
/* ************************************************************************
*  file: constants.c                                      Part of DIKUMUD *
*  Usage: For constants used by the game.                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include "structs.h"
#include "magic.h"
#include "time.h"
#include "vt100.h"
#include "bio.h"

#define NULL 0

const size_t obj_info_size[] = {
 sizeof(struct obj_info_light), /* LIGHT */
 sizeof(struct obj_info_scroll), /* SCROLL */
 sizeof(struct obj_info_wand), /* WAND */
 sizeof(struct obj_info_staff), /* STAFF */
 sizeof(struct obj_info_weapon), /* WEAPON */
 sizeof(struct obj_info), /* FIREWEAPON */
 sizeof(struct obj_info), /* MISSILE */
 sizeof(struct obj_info_potion), /* POTION */
 sizeof(struct obj_info_wear), /* WORN */
 sizeof(struct obj_info), /* TRAP */
 sizeof(struct obj_info_container), /* CONTAINER */
 sizeof(struct obj_info), /* NOTE */
 sizeof(struct obj_info_drink), /* DRINKCON */
 sizeof(struct obj_info_key), /* KEY */
 sizeof(struct obj_info_food), /* FOOD */
 sizeof(struct obj_info_money), /* MONEY */
 sizeof(struct obj_info), /* PEN */
 sizeof(struct obj_info_boat), /* BOAT */
 sizeof(struct obj_info), /* BOOK */
 sizeof(struct obj_info_poison), /* POISON */
 sizeof(struct obj_info_teleport), /* TELEPORT */
 sizeof(struct obj_info), /* TIMER */
 sizeof(struct obj_info), /* STONE */
 sizeof(struct obj_info_exit), /* EXIT */
 sizeof(struct obj_info) /* TRASH */
};

const struct material_info materials[] = {
 {"unknown","mystical strange bizzare",0,COLOR_GREY,1.0},
 {"metal","hard shiny smooth",MAT_MULTICOLOR,COLOR_BLACK|COLOR_GREY,10.0},
 {"cloth","soft",MAT_MULTICOLOR|MAT_FLAMMABLE,COLOR_ANY,0.2},
 {"stone","hard dull",MAT_MULTICOLOR,COLOR_BLACK|COLOR_GREY|COLOR_WHITE,8.0},
 {"plant","organic",MAT_FLAMMABLE,COLOR_ANY,0.4},
 {"water","liquid clear",0,COLOR_TRANSL,1.0},
 {"wood","hard rough",MAT_FLAMMABLE,COLOR_BROWN,0.8},
 {"clay","??",0,COLOR_BROWN,3.0},
 {"dirt","grainy loose",0,COLOR_BROWN,2.5},
 {"energy","pulsating",0,COLOR_TRANSL,4.0},
 {"crystal","sparkling faceted",MAT_MULTICOLOR,COLOR_ANY|COLOR_TRANSL,3.5},
 {"flesh","organic",MAT_FLAMMABLE,COLOR_ANY,1.5},
 {"chemical","liquid pungeant",MAT_FLAMMABLE|MAT_MULTICOLOR,COLOR_ANY|COLOR_TRANSL,1.2},
 {"oil","liquid viscous",MAT_FLAMMABLE,COLOR_BLACK|COLOR_TRANSL,2.0}
};

const struct bio_info bio_attrs[] = {
 {"null",0,0,NULL},
 {"str",0,25,BI_INTRINSIC | BI_DICE},
 {"int",0,25,BI_INTRINSIC | BI_DICE},
 {"wis",0,25,BI_INTRINSIC | BI_DICE},
 {"dex",0,25,BI_INTRINSIC | BI_DICE},
 {"con",0,25,BI_INTRINSIC | BI_DICE},
 {"chr",0,25,BI_INTRINSIC | BI_DICE},
 {"hp",1,50, BI_RANGE},
 {"hd",1,500,BI_INTRINSIC | BI_RANGE},
 {"mana",0,2000,BI_INTRINSIC | BI_RANGE},
 {"move",0,2000,BI_INTRINSIC | BI_RANGE},
 {"limbs",0,10,NULL},
 {"plimbs",0,10,NULL},
 {"glimbs",0,10,NULL},
 {"tails",0,5,NULL},
 {"sight",0,1,NULL},
 {"hear",0,1,NULL},
 {"smell",0,1,NULL},
 {"odor",0,10,NULL},
 {"odor_str",0,100,NULL},
 {"size",0,100,NULL},
 {"res1",0,1,NULL},
 {"res2",0,1,NULL},
 {"res3",0,1,NULL},
 {"res4",0,1,NULL},
 {"res5",0,1,NULL},
 {"sleep",0,1,NULL},
 {"rest",0,1,NULL},
 {"sit",0,1,NULL},
 {"swim",0,1,NULL},
 {"stand",0,1,NULL},
 {"levitate",0,1,NULL},
 {"fly",0,1,NULL},
 {"mobile",0,1,NULL},
 {"gestation",0,2000,NULL},
 {"litter",1,1000,NULL},
 {"sexes",1,5,NULL},
 {"ratio",1,99,NULL},
 {"diet",1,20,NULL},
 {"pref_temp",-100,100,NULL},
 {"pref_humid",0,100,NULL},
 {"temp_toler",5,100},
 {"humid_toler",10,100},
 {"calories",0,20000},
 {"\n",0,0,NULL}        /* marker for search_block_offset() */
};

const char *errors[] = {
 "Okay",
 "Internal Error - please report",
 "Syntax Error - poorly specified command",
 "Position Error - cannot perform action",
 "Can't find target",
 "Problem with location",
 "Command requires more moves than available",
 "Command requires more mana than available",
 "Something physical prevents such action",
 "Something magical prevents such action",
 "You don't know how",
 "Capacity reached",
 "Failed",
 "Range Error",
 "Authorization problem - who says you can do that?",
 "That makes no sense",
 "Already done - can't redo",
 "Body can't do that"
};

const char *org_kind[] = {
 "Hometown",
 "Profession",
 "Religion",
 "Society",
 "",
 "",
 "",
 "",
 "Other",
 "Admin"
};

const char *skills[] = {
 "learn",
 "offense",
 "defense",
 "social",
 "cast",
 "0",
 "0",
 "0",
 "0",
 "0",
 "levitate", /* 10 */
 "fly",
 "\n"
};

const struct mattrib_info mattribs[] = {
 {"null",ATTRIB_NULL, 0,0},
 {"origin_t",ATTRIB_ORIGIN_T,0,0},
 {"origin_id",ATTRIB_ORIGIN_ID,0,0},

 {"str",ATTRIB_STR,15,5},
 {"int",ATTRIB_INT,10,8},
 {"wis",ATTRIB_WIS,10,8},
 {"dex",ATTRIB_DEX,10,5},
 {"con",ATTRIB_CON,10,8},
 {"chr",ATTRIB_CHR,10,5},

 {"imbue",ATTRIB_IMBUE,50,10},
 {"ignore",ATTRIB_IGNORE,1,0},
 {"toggle",ATTRIB_TOGGLE_IGNORE,1,0},
 {"transform",ATTRIB_TRANSFORM,100,15},
 {"halve",ATTRIB_HALVE,0,1},
 {"decimate",ATTRIB_DECIMATE,0,3},

 {"trig_touch",ATTRIB_TRIG_TOUCH,20,5},
 {"perm",ATTRIB_PERMANENT,500,20},

 /* place marker for the end of array */
 {"\n",-1,-1,-1}
};

const char *spell_wear_off_msg[] = {
 "RESERVED DB.C",
 "You feel less protected.",
 "!Teleport!",
 "You feel less righteous.",
 "You feel a cloak of blindness disolve.",
 "!Burning Hands!",
 "!Call Lightning",
 "You feel more self-confident.",
 "!Chill Touch!",
 "!Clone!",
 "!Color Spray!",
 "!Control Weather!",
 "!Create Food!",
 "!Create Water!",
 "!Cure Blind!",
 "!Cure Critic!",
 "!Cure Light!",
 "You feel better.",
 "You sense the red in your vision disappear.",
 "The detect invisible wears off.",
 "The detect magic wears off.",
 "The detect poison wears off.",
 "!Dispel Evil!",
 "!Earthquake!",
 "!Enchant Weapon!",
 "!Energy Drain!",
 "!Fireball!",
 "!Harm!",
 "!Heal",
 "You feel yourself exposed.",
 "!Lightning Bolt!",
 "!Locate object!",
 "!Magic Missile!",
 "You feel less sick.",
 "You feel less protected.",
 "!Remove Curse!",
 "The white aura around your body fades.",
 "!Shocking Grasp!",
 "You feel less tired.",
 "You feel weaker.",
 "!Summon!",
 "!Ventriloquate!",
 "!Word of Recall!",
 "!Remove Poison!",
 "You feel less aware of your suroundings.",
 "",  /* NO MESSAGE FOR SNEAK*/
 "!Hide!",
 "!Steal!",
 "!Backstab!",
 "!Pick Lock!",
 "!Kick!",
 "!Bash!",
 "!Rescue!",
 "!UNUSED!",
 "You stop levitating.",
 "You stop flying."
 "Your heightened awareness fades."
};

const int max_dir[] = /* Maximum directions for a zone */
{
    6, /* DIRS_COMPAT */
   10, /* DIRS_NORMAL */
    0, /* DIRS_NONE */
    6  /* DIRS_SHIP */
};

const int rev_dir[] = /* The opposite way from the index dir...-Sman */
{
    2,
    3,
    0,
    1,	
    5,
    4,
    8,
    9,
    6,
    7
}; 

const int movement_loss[]=
{
    1,	/* Inside     */
    2,  /* City       */
    2,  /* Field      */
    3,  /* Forest     */
    4,  /* Hills      */
    5,  /* Mountains  */
    4,  /* Swimming   */
    1,  /* Unswimable */
    4   /* No ground  */
};

const char *dirs[] = 
{
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "northeast",
    "southeast",
    "southwest",
    "northwest",
    "\n"
};

/* Keep this sorted, in case we do a binary search later or something.. */
const struct dayspec holidays[] = {
    {1,1,   {"New","Year's","Day","",""}},
    {2,15,  {"Dead","Rulers","Day","",""}},
    {3,14,  {"Dawn","of","Reluctant","Hearts",""}},
    {3,16,  {"Day","for","Memory","",""}},
    {4,1,   {"Fool","Day","","",""}},
    {4,10,  {"Mammoth","Day","","(Northern)",""}},
    {5,23,  {"Circle","Time","","",""}},
    {6,15,  {"Festival","of","Sprouts","",""}},
    {7,5,   {"Fifth","of","Roses","",""}},
    {7,26,  {"Martyr","Day","","",""}},
    {8,22,  {"NachStil","","","",""}},
    {9,1,   {"Imni","Fest","","",""}},
    {9,12,  {"Pasta","Day","","",""}},
    {10,9,  {"Choxo","no","Sekku","",""}},
    {10,25, {"Day","of","Mynsgra","",""}},
    {11,10, {"Da","Sedge","Merge","",""}},
    {12,9,  {"Cheese","Day","","",""}},
    {12,32, {"Homage","of","Past","Souls",""}},
    {13,31, {"Chaos","Night","","",""}},
    {14,3,  {"Hero's","Welcome","","",""}},
    {14,14, {"Festival","of","Language","",""}},
    {15,32, {"New", "Year's","Eve","",""}}
};

const int num_holiday = 22;

const char *weekdays[7] = { 
    "Dimanche",
    "Lundi",
    "Mardi",
    "Mercredi",
    "Jeudi",
    "Vendredi",
    "Samedi"
};

const char *month_name[15] = {
    "Nimrias",
    "Phol",
    "Murghidri",
    "Heat",
    "Stamphor",
    "ThuHurca",
    "Lunamber",
    "Famine",
    "Deimias",
    "Wandering",
    "Coloscor",
    "Pindrop",
    "Laments",
    "Runes",
    "Finiar"
};

const int sharp[] = {
 0,
 0,
 0,
 1,    /* Slashing */
 0,
 0,
 0,
 0,    /* Bludgeon */
 0,
 0,
 0,
 0 };  /* Pierce   */

const char *where[] = {
    "",                         /* unequipped */
    "<held in right hand>",
    "<held in left hand>",
    "<worn on right finger>",
    "<worn on left finger>",
    "<worn around neck>",
    "<worn around neck>",
    "<worn on body>",
    "<worn on head>",
    "<worn on legs>",
    "<worn on feet>",
    "<worn on hands>",
    "<worn on arms>",
    "<worn about body>",
    "<worn about waist>",
    "<worn around right wrist>",
    "<worn around left wrist>"
}; 

const char *drinks[]=
{
    "water",
    "beer",
    "wine",
    "ale",
    "dark ale",
    "whisky",
    "lemonade",
    "firebreather",
    "local speciality",
    "slime mold juice",
    "milk",
    "tea",
    "coffee",
    "blood",
    "salt water",
    "coca cola"
};

const char *drinknames[]=
{
    "water",
    "beer",
    "wine",
    "ale",
    "ale",
    "whisky",
    "lemonade",
    "firebreather",
    "local",
    "juice",
    "milk",
    "tea",
    "coffee",
    "blood",
    "salt",
    "cola"
};

const int drink_aff[][3] = {
    { 0,1,10 },  /* Water    */
    { 3,2,5 },   /* beer     */
    { 5,2,5 },   /* wine     */
    { 2,2,5 },   /* ale      */
    { 1,2,5 },   /* ale      */
    { 6,1,4 },   /* Whiskey  */
    { 0,1,8 },   /* lemonade */
    { 10,0,0 },  /* firebr   */
    { 3,3,3 },   /* local    */
    { 0,4,-8 },  /* juice    */
    { 0,3,6 },
    { 0,1,6 },
    { 0,1,6 },
    { 0,2,-1 },
    { 0,1,-2 },
    { 0,1,5 }
};

const char *color_liquid[]=
{
    "clear",
    "brown",
    "clear",
    "brown",
    "dark",
    "golden",
    "red",
    "green",
    "clear",
    "light green",
    "white",
    "brown",
    "black",
    "red",
    "clear",
    "black"
};

const char *fullness[] =
{
    "less than half ",
    "about half ",
    "more than half ",
    ""
};

const char *item_types[] = {
    "light",
    "scroll",
    "wand",
    "staff",
    "weapon",
    "fireweapon",
    "missile",
    "potion",
    "worn",
    "trap",
    "container",
    "note",
    "drinkcon",
    "key",
    "food",
    "money",
    "pen",
    "boat",
    "book",
    "poison",
    "teleport",
    "timer",
    "stone",
    "exit",
    "trash",
    "\n"
};

const char *wear_bits[] = {
    "FINGER",
    "NECK",
    "BODY",
    "HEAD",
    "LEGS",
    "FEET",
    "HANDS",
    "ARMS",
    "SHIELD",
    "ABOUT",
    "WAIST",
    "WRIST",
    "WIELD",
    "\n"
};

const char *extra_bits[] = {
    "take",
    "hum",
    "dark",
    "lock",
    "evil",
    "invisible",
    "magic",
    "nodrop",
    "ignore",
    "anti-good",
    "anti-evil",
    "anti-neutral",
    "secret",
    "float",
    "hidden",
    "snooper",
    "snoopee",
    "archive",
    "\n"
};

const char *room_bits[] = {
    "DARK",
    "DEATH",
    "NO_MOB",
    "INDOORS",
    "LAWFULL",
    "NEUTRAL",
    "CHAOTOC",
    "NO_MAGIC",
    "TUNNEL",
    "PRIVATE",
    "ARENA",
    "SAFE",
    "NO_PRECIP",
    "SINGLE_FILE",
    "JAIL",
    "NO_TELEPORT",
    "\n"
};

const char *zone_bits[] = {
    "SILENT",
    "SAFE",
    "HOMETOWN",
    "NEW_RESET",
    "\n"
};

const char *exit_bits[] = {
    "IS-DOOR",
    "CLOSED",
    "LOCKED",
    "\n"
};

const char *sector_types[] = {
    "Inside",
    "City",
    "Field",
    "Forest",
    "Hills",
    "Mountains",
    "Water Swim",
    "Water NoSwim",
    "No Ground",
    "\n"
};

const char *equipment_types[] = {
    "Special",
    "Worn on right finger",
    "Worn on left finger",
    "First worn around Neck",
    "Second worn around Neck",
    "Worn on body",
    "Worn on head",
    "Worn on legs",
    "Worn on feet",
    "Worn on hands",
    "Worn on arms",
    "Worn as shield",
    "Worn about body",
    "Worn around waist",
    "Worn around right wrist",
    "Worn around left wrist",
    "Wielded",
    "Held",
    "\n"
};
    
const char *affected_bits[] = 
{	"BLIND",
    "INVISIBLE",
    "DETECT-EVIL",
    "DETECT-INVISIBLE",
    "DETECT-MAGIC",
    "SENCE-LIFE",
    "HOLD",
    "SANCTUARY",
    "xxxxx",
    "UNUSED",
    "CURSE",
    "FLAMING-HANDS",
    "POISON",
    "PROTECT-EVIL",
    "PARALYSIS",
    "MORDENS-SWORD",
    "FLAMING-SWORD",
    "SLEEP",
    "DODGE",
    "SNEAK",
    "HIDE",
    "FEAR",
    "CHARM",
    "FOLLOW",
    "WIMPY",
    "INFRARED",
    "LEVITATE",
    "FLY",
    "AWARE",
    "\n"
};

const char *apply_types[] = {
    "NONE",
    "STR",
    "DEX",
    "INT",
    "WIS",
    "CON",
    "SEX",
    "CLASS",
    "LEVEL",
    "AGE",
    "CHAR_WEIGHT",
    "CHAR_HEIGHT",
    "MANA",
    "HIT",
    "MOVE",
    "GOLD",
    "EXP",
    "ARMOR",
    "HITROLL",
    "DAMROLL",
    "SAVING_PARA",
    "SAVING_ROD",
    "SAVING_PETRI",
    "SAVING_BREATH",
    "SAVING_SPELL",
    "\n"
};

const char *action_bits[] = {
    "SPEC",
    "SENTINEL",
    "SCAVENGER",
    "ISNPC",
    "NICE-THIEF",
    "AGGRESSIVE",
    "STAY-ZONE",
    "WIMPY",
    "AGGRESSIVE_EVIL",
    "AGGRESSIVE_GOOD",
    "AGGRESSIVE_NEUTRAL",
	"MEMORY",
    "IS_HUMANOID",
    "IS_DRAGON",
    "IS_DAEMON",
    "IS_ANIMAL",
    "IS_INSECT",
    "CAN_FLY",
    "CAN_SWIM",
    "HAS_CL",
    "HAS_MU",
    "HAS_TH",
    "HAS_WA",
    "SPEC_DIE",
    "OUTLAW",
    "\n"
};


const char *player_bits[] = {
    "BRIEF",
    "GUEST",
    "COMPACT",
    "DONTSET",
    "",
    "TAG_FLAG",
    "OUTLAW",
    "MSG_ECHO",
    "NOSUMMON",
    "\n"
};


const char *position_types[] = {
    "Dead",
    "Mortally wounded",
    "Incapacitated",
    "Stunned",
    "Sleeping",
    "Resting",
    "Sitting",
    "Fighting",
    "Standing",
    "Swiming",
    "Levitating",
    "Flying",
    "Aware",
    "\n"
};

const char *connected_types[]	=	{
    "Playing",
    "Get name",
    "New player",
    "Read password",
    "Get new password",
    "Confirm new password",
    "Get sex",
    "Read messages of today",
    "Read Menu",
    "Get extra description",
    "Get class",
    "Link dead",
    "Check old password",
    "Replace password",
    "Confirm replacement password",
    "Closing connection",
    "Get hometown",
    "New password confirm",
    "\n"
};

/* [class], [level] (all) */
const int thaco[4][25] = {
    { 100,20,20,20,19,19,19,18,18,18,17,17,17,16,16,16,15,15,15,14,14,14,13,13,13},
 { 100,20,20,20,18,18,18,16,16,16,14,14,14,12,12,12,10,10,10, 8, 8, 8, 6, 6, 6},
 { 100,20,20,19,19,18,18,17,17,16,16,15,15,14,13,13,12,12,11,11,10,10, 9, 9, 8},
 { 100,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1}
};

/* [ch] strength apply (all) */
const struct str_app_type str_app[31] = {
    { -5,-4,   0,  0 },  /* 0  */
    { -5,-4,   3,  1 },  /* 1  */
    { -3,-2,   3,  2 },
    { -3,-1,  10,  3 },  /* 3  */
    { -2,-1,  25,  4 },
    { -2,-1,  55,  5 },  /* 5  */
    { -1, 0,  80,  6 },
    { -1, 0,  90,  7 },
    {  0, 0, 100,  8 },
    {  0, 0, 100,  9 },
    {  0, 0, 115, 10 }, /* 10  */
    {  0, 0, 115, 11 },
    {  0, 0, 140, 12 },
    {  0, 0, 140, 13 },
    {  0, 0, 170, 14 },
    {  0, 0, 170, 15 }, /* 15  */
    {  0, 1, 195, 16 },
    {  1, 1, 220, 18 },
    {  1, 2, 255, 20 }, /* 18  */
    {  3, 7, 640, 40 },
    {  3, 8, 700, 40 }, /* 20  */
    {  4, 9, 810, 40 },
    {  4,10, 970, 40 },
    {  5,11,1130, 40 },
    {  6,12,1440, 40 },
    {  7,14,1750, 40 }, /* 25            */
    {  1, 3, 280, 22 }, /* 18/01-50      */
    {  2, 3, 305, 24 }, /* 18/51-75      */
    {  2, 4, 330, 26 }, /* 18/76-90      */
    {  2, 5, 380, 28 }, /* 18/91-99      */
    {  3, 6, 480, 30 }  /* 18/100   (30) */
};

/* [dex] skill apply (thieves only) */
const struct dex_skill_type dex_app_skill[26] = {
    {-99,-99,-90,-99,-60},   /* 0 */
    {-90,-90,-60,-90,-50},   /* 1 */
    {-80,-80,-40,-80,-45},
    {-70,-70,-30,-70,-40},
    {-60,-60,-30,-60,-35},
    {-50,-50,-20,-50,-30},   /* 5 */
    {-40,-40,-20,-40,-25},
    {-30,-30,-15,-30,-20},
    {-20,-20,-15,-20,-15},
    {-15,-10,-10,-20,-10},
    {-10, -5,-10,-15, -5},   /* 10 */
    { -5,  0, -5,-10,  0},
    {  0,  0,  0, -5,  0},
    {  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0},   /* 15 */
    {  0,  5,  0,  0,  0},
    {  5, 10,  0,  5,  5},
    { 10, 15,  5, 10, 10},
    { 15, 20, 10, 15, 15},
    { 15, 20, 10, 15, 15},   /* 20 */
    { 20, 25, 10, 15, 20},
    { 20, 25, 15, 20, 20},
    { 25, 25, 15, 20, 20},
    { 25, 30, 15, 25, 25},
    { 25, 30, 15, 25, 25}    /* 25 */
};

/* [level] backstab multiplyer (thieves only) */
const byte backstab_mult[25] = {
    1,   /* 0 */
    2,   /* 1 */
    2,
    2,
    2,
    2,   /* 5 */
    2,
    2,
    3,
    3,
    3,   /* 10 */
    3,
    3,
    3,
    4,
    4,   /* 15 */
    4,
    4,
    4,
    4,
    4,   /* 20 */
    5,
    5,
    5,
    5    /* 25 */
};

/* [dex] apply (all) */
struct dex_app_type dex_app[26] = {
    {-7,-7, 6},   /* 0 */
    {-6,-6, 5},   /* 1 */
    {-4,-4, 5},
    {-3,-3, 4},
    {-2,-2, 3},
    {-1,-1, 2},   /* 5 */
    { 0, 0, 1},
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0, 0},   /* 10 */
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0,-1},   /* 15 */
    { 1, 1,-2},
    { 2, 2,-3},
    { 2, 2,-4},
    { 3, 3,-4},
    { 3, 3,-4},   /* 20 */
    { 4, 4,-5},
    { 4, 4,-5},
    { 4, 4,-5},
    { 5, 5,-6},
    { 5, 5,-6}    /* 25 */
};

/* [con] apply (all) */
struct con_app_type con_app[26] = {
    {-4,20},   /* 0 */
    {-3,25},   /* 1 */
    {-2,30},
    {-2,35},
    {-1,40},
    {-1,45},   /* 5 */
    {-1,50},
    { 0,55},
    { 0,60},
    { 0,65},
    { 0,70},   /* 10 */
    { 0,75},
    { 0,80},
    { 0,85},
    { 0,88},
    { 1,90},   /* 15 */
    { 2,95},
    { 2,97},
    { 3,99},
    { 3,99},
    { 4,99},   /* 20 */
    { 5,99},
    { 5,99},
    { 5,99},
    { 6,99},
    { 7,100}   /* 25 */
};

/* [int] apply (all) */
struct int_app_type int_app[26] = {
    {3},
    {5},    /* 1 */
    {7},
    {8},
    {9},
    {10},   /* 5 */
    {11},
    {12},
    {13},
    {15},
    {17},   /* 10 */
    {19},
    {22},
    {25},
    {30},
    {35},   /* 15 */
    {40},
    {45},
    {50},
    {53},
    {55},   /* 20 */
    {56},
    {60},
    {70},
    {80},
    {99}    /* 25 */
};

/* [wis] apply (all) */
struct wis_app_type wis_app[26] = {
    {0},   /* 0 */
    {0},   /* 1 */
    {0},
    {0},
    {0},
    {0},   /* 5 */
    {0},
    {0},
    {0},
    {0},
    {0},   /* 10 */
    {0},
    {2},
    {2},
    {3},
    {3},   /* 15 */
    {3},
    {4},
    {5},   /* 18 */
    {6},
    {6},   /* 20 */
    {6},
    {6},
    {6},
    {6},
    {6}   /* 25 */
};

/* These were in db.h and were a pain to change, since most everything    */
/* would need recompiling...so I changed these defines to constants -Sman */

char *menu=
"\n\r          Welcome to Dikumud at Copper\n\r\
            Stella says 'Same to ya'\n\r\n\
+---------------------------  0) Exit from Copper.\n\r\
| Greetings. To the right is  1) Enter the game.\n\r\
| the list of main menu       2) Online Readers\n\r\
| selections.  If this is     3) Change password.\n\r\
| your first visit to C3, we  4) Delete this char.\n\r\
| suggest browsing through\n\r\
| the online readers first.   Make your choice: ";

char *greetings=
"\n\r\n\r" 
VT_BOLDTEX 
"Copper III (alpha)\n\ra.k.a. Damn Stella Mud\n\rThe mud for carbon-based life forms.\n\r\n\r"
VT_NORMALT
"DikuMUD Created by\n\r\
Hans Henrik Staerfeldt, Katja Nyboe, Tom Madsen\n\r\
Michael Seifert, and Sebastian Hammer\n\r\n\r\
                 ~~~~~\n\r\
 ___              ~~                                     /\\\n\r\
/_ \\\\____       ~~                                      /  \\/\\\n\r\
V \\ \\    \\______^                                      /   /  \\\n\r\
 |\\  (o         )                                    /   /    \\  /\\\n\r\
 |        VVVVVV                                    /  /\\  /\\  \\/  \\\n\r\
  >       ^^^^^^                                   /  /  \\/  \\ /    \\\n\r\
 /      _______/                                  /  /    \\   /      \\\n\r\
/      /                                         /  /      \\ /      ct\\\n\r\
This is a dragon...                              These are rocks...\n\r\
                           Cuprum Tertius\n\r\
[address correspondence to abradfor@scooter.denver.colorado.edu]\n\r\
\n\r\
Hi! What's your name?\n\r\
('new' to make a new character)\n\r\
Name>";


#if 0
greetings="another possibility...\n\r\
  __  _   __  __  __  __\n\r\
 /   /_\\ |_/ |_/ /__ |_/\n\r\
 \\__ \\_/ |   |   \\__ | \\\n\r\
\n\r\
 |.:.:.:.:.:.:.:.:.:.:.:|\n\r\
 |::::::::::::::::::::::|\n\r\
 |::                  ::|\n\r\
 |:::::  :::  :::  :::::|\n\r\
 |:::::  :::  :::  :::::|\n\r\
 |:::::  :::  :::  :::::|\n\r\
 |:::::  :::  :::  :::::|\n\r\
 |:::::  :::  :::  :::::|\n\r\
 |::                  ::|\n\r\
 |::::::::::::::::::::::|\n\r\
 |::::::::::::::::::::::|\n\r\
_____________________________\n\r\
Il y a une auto sur moi aussi\n\r";
#endif


char *story=
"A long time ago, when the worlds were not yet created, the\n\r\
gods gathered to discuss plans for their creation. Being only\n\r\
a part of their own unreality, their viewpoints and arguments\n\r\
flowed around each other, coalescing into several competing\n\r\
factions.\n\r\n\r\
One group, whose size and power overwhelmed the others', extolled\n\r\
the virtues of responsibility and hard work. Unfortunately for\n\r\
them, they were unable to agree upon the details of managing\n\r\
and creating a world based exclusively on these principles. The\n\r\
violence of the differing drafts they spewed at each other was\n\r\
such that they disassociated from one another and left to create\n\r\
their own worlds, alone.\n\r\n\r\
The remaining gods were stunned, as much as a god can be stunned.\n\r\
They agreed that, no matter how much they disagreed, they would\n\r\
not draw absolute lines between their spheres of influence, nor\n\r\
forbid anyone's participation, whether mortal or immortal.\n\r\n\r\
So they went to work, loosely pulling together space, time,\n\r\
and emotion in a comprehensive network. They even hid entry\n\r\
points in the realms of the isolationists, so that no mortal,\n\r\
anywhere, be without respite from the dreary oppressions of\n\r\
responsibility and hard work.\n\r\n\r";

/*...a bit more left-wing than intended, but serviceable...Sman */

