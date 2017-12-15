/* ************************************************************************
*  file: save.h, save file format      .                  Part of Copper3 *
*  Usage: Declarations of on-disk data structures                         *
*  Version for Copper III                                                 *
************************************************************************* */


#ifndef SAVE_H
#define SAVE_H

/* We definitely need some definitions from here */
#ifndef STRUCTS_H
#include "structs.h"
#endif

/*************************************************************************
* file element for player files. BEWARE: Changing it will ruin the files *
*************************************************************************/

struct char_file_u
{
    CHAR_ID id;
    int sex;
    time_t birth;  /* Time of birth of character     */
    int played;    /* Number of secs played in total */

    int weight;
    int height;

    char title[80];
    sh_int hometown;
    char description[240];
    int talks[MAX_TOUNGE];

    sh_int load_room;            /* Which room to place char in           */

    struct char_point_data points;

    /* specials */

    int spells_to_learn;  
    int alignment;     
    int position;

    time_t last_logon;  /* Time (in secs) of last logon */
    unsigned long act;

    /* char data */
    char name[20];
    sh_int apply_saving_throw[5];

    int num_org, num_skill, num_affect, num_item, num_phys;
};

struct phys_u
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
};

struct org_u {
    int org_id;                    /* Which organization?         */
    int member_level;             /* For those .orgs with levels */
    long permissions;              /* What can this person do?    */
    int participation;            /* How active is this person?  */
    int experience;                /* Overall in-the-field total  */
    int domains[MAX_DOMAINS];     /* Where is there power?       */
};

struct skill_u {
    short skill_num; /* org?? */
    int learned;           /* % chance for success 0 = not learned   */
    int period;
    int period_type;
    long int last;
};

struct affect_u {
    int type;           /* The type of spell that caused this      */
    sh_int duration;      /* For how long its effects will last      */
    int modifier;       /* This is added to apropriate ability     */
    int location;        /* Tells which ability to change(APPLY_XXX)*/
    long bitvector;       /* Tells which bits to set (AFF_XXX)       */
};

/* ***********************************************************************
*  file element for object file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */

/* Where to load in the item to */
/* (only valid when loading to char ) */
#define LOAD_TO_INV  1
#define LOAD_TO_OBJ  2 /* but which obj? */
#define LOAD_TO_EQ   3
#define LOAD_TO_ROOM 4

struct obj_file_elem 
{
    sh_int load_code;
    int load_to;
    int num_contains;

    sh_int item_number;

    int extra_flags;
    int weight;
    long bitvector;
    int hits, maxhits;
    int num_materials,num_affects,num_info;
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */

/* NOTE: We actually save more than this, but since info records
 * are of variable size, we have to determine length from the type
 * before loading the whole thing. The const array obj_info_size
 * is used for this.
 */
struct obj_info_u
{
    sh_int obj_type;
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_material_u
{
    int type;
    int subtype;
    int purity;
    int weight;
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affect_u {
    int location;      /* Which ability to change (APPLY_XXX) */
    int modifier;     /* How much it changes by              */
};

#endif /* !defined(SAVE_H) */
