/* ************************************************************************
*  file: org.h, organizational structures                 Part of Copper3 *
*  Usage: Declarations of central data structures                         *
*  Version for Copper III                                                 *
************************************************************************* */

#ifndef ORG_H
#define ORG_H

#include <sys/types.h>

/* For period type */

#define PERIOD_MANA     1       /* Value is minimum mana, calc as normal */
#define PERIOD_DAILY    2       /* Value is number of uses per day       */
#define PERIOD_NONE     3       /* Unlimited skill - ignore value        */
#define PERIOD_USEMOVES 4       /* Value is taken from moves per use     */
/*(could be used for those beings with global shouts)*/
#define PERIOD_PERMOVE  5       /* Value is taken from moves each move   */
/*(could be used for levitating, flying, swimming)*/

struct org_ability_spec
{
    sh_int min_level;
    int skill_num;
    sh_int pract_cost;
    int period;
    sh_int period_type;
    struct org_ability_spec *next;
};

#define REQ_STAT    1
#define REQ_ALIGN   2
#define REQ_ORG     3
#define REQ_ITEM?

struct org_req
{
    int type;
    int which;
    int value;
};

struct org_figure_spec
{
    char *title;
    int level;
    int flags;
    struct org_ability_spec *abilities;
    struct org_figure_spec *next;
};

/* basic org types */

#define ORG_HOMETOWN      0
#define ORG_PROFESSION    1
#define ORG_RELIGION      2
#define ORG_SOCIETY       3

#define ORG_OTHER         8
#define ORG_ADMIN         9        /* These guys can do anything */

#define ORG_ID_ADMIN      1        /* A single hardcoded org id */

/* org flags */
#define ORGF_LEVELS       1        /* Does this org have levels? */
#define ORGF_AUTOJOIN     2        /* Can people admit themselves? */

struct org_type
{
    int org_id;
    char *name;                    /* What's it called?            */
    char *namelist;                /* list of keywords for it      */
    unsigned long org_flags;
    sh_int org_type;                 /* What basic use does it have? */
    struct org_ability_spec *skills;
    struct org_figure_spec *members;
    struct org_type *next;
};


/* ======================================================================== */



struct org_figure
{
    int char_id;
    int org_id;
    int figure_id;
    int term;
    struct org_figure *next;
};

/* ***********************************************************************
*  file element for figure file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */

struct org_figure_u
{
    int char_id;
    int org_id;
    int figure_id;
    int term;
};

#endif /* !defined(ORG_H) */
