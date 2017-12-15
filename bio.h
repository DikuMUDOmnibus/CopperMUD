/* ************************************************************************
*  file: bio.c ,     Definitions for biological systems   Part of Copper3 *
*  Usage :                                                                *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#ident	"@(#) $Id:$\n"
/* $Log:$
 */

#ifndef BIO_H
#define BIO_H

/**** Bio attributes ****/
#define BIO_NULL           0
/* physical */
#define BIO_STR            1
#define BIO_INT            2
#define BIO_WIS            3
#define BIO_DEX            4
#define BIO_CON            5
#define BIO_CHR            6

#define BIO_HITPOINTS      7
/* HitDice takes precedence over HitPoints */
#define BIO_HITDICE        8

#define BIO_MANA           9
#define BIO_MOVES          10

/* Body information */
#define BIO_LIMBS          11
#define BIO_LIMBS_PROPEL   12
#define BIO_LIMBS_GRASP    13
#define BIO_TAILS          14
#define BIO_SIGHTED        15
#define BIO_HEARING        16
#define BIO_SMELLING       17
#define BIO_ODOR_TYPE      18
#define BIO_ODOR_STRENGTH  19
#define BIO_SIZE           20
#define BIO_RESERVED1      21
#define BIO_RESERVED2      22
#define BIO_RESERVED3      23
#define BIO_RESERVED4      24
#define BIO_RESERVED5      25

/* Position - what positions can they logically hold? */
#define BIO_POS_SLEEP      26
#define BIO_POS_REST       27
#define BIO_POS_SIT        28
#define BIO_POS_SWIM       29
#define BIO_POS_STAND      30
#define BIO_POS_LEVITATE   31
#define BIO_POS_FLY        32
/* mobility */
#define BIO_MOBILE         33

/* Reproduction */
#define BIO_GESTATION      34
#define BIO_LITTER_SIZE    35
#define BIO_SEXES          36
#define BIO_SEX_RATIO      37

/* Living conditions */
#define BIO_DIET           38
#define BIO_PREF_TEMP      39
#define BIO_PREF_HUMIDITY  40
#define BIO_TEMP_TOLERATE  41
#define BIO_HUMID_TOLERATE 42
#define BIO_DAILY_CALS     43

#define MAX_BIO_ATTR       44

/* bio-info flags */
#define BI_INTRINSIC 1

#define BI_RANGE 2    /* These two are mutually exclusive */
#define BI_DICE 4

/* one per attribute */
struct bio_info {
    char *name;
    int low,high;
    int flags;
};

/* attribute information for particular bios */
struct bio_attr {
    int type;
    int value1,value2;
    struct bio_attr *next;
};

/* element in biology index table   */
struct bio_index_data
{
    int virtual;    /* virtual number of this mob               */
    int inherits;   /* what is the parent?                      */
    int count;      /* number of existing units of this bio     */
    char *name;     /* easier way to do lookups                 */
    char *sdesc;    /* simple description                       */
    char *ldesc;    /* long description                         */
    int loc;        /* where does the mob hang out?             */
    int freq;       /* how often does one come across them?     */
    struct bio_attr *attribs;
};

/* Where to put this is debatable, unless there is ever a fight.h */
#define DAM_BLOW	0	/* A "hit" with force */
#define DAM_SLASH	1
#define DAM_PIeRCe	2
#define DAM_GAS		3
#define DAM_POISON	4
#define DAM_PARALYSIS	5
#define DAM_HeAT	6
#define DAM_COLD	7
#define DAM_NUTRITION	8
#define DAM_VIRUS	9
#define DAM_BACTeRIA	10
#define DAM_BLeeD	11
#define DAM_ORGAN	12
#define DAM_FALLING     13


#endif /* !defined(BIO_H) */
