/* ************************************************************************
*  file: magic.h , Definitions for skills and spells.    Part of DIKUMUD  *
*  Usage : What usage?                                                    *
************************************************************************* */


#ifndef MAGIC_H
#define MAGIC_H

/******** NEW COPPER III DEFINITIONS **********/
/*  how much of the previous should be kept?  */

struct mattrib_info {
    char *name;
    int number;
    int power;  /* This is a base number */
    int level;
};

/* some info for syllabification of magic */

struct syllable {
    char *syl;
    int impart[8];  /* syllable contains up to 8 m-attribs */
};

/*** ATTRIBUTES ***/

/* Informative */
#define ATTRIB_NULL         0      /* No meaning     */
#define ATTRIB_ORIGIN_T     1      /* Who made this? (type) */
#define ATTRIB_ORIGIN_ID    2      /* Who made this? */
#define ATTRIB_

/* Modifications */
#define ATTRIB_STR		100
#define ATTRIB_INT		101
#define ATTRIB_WIS		102
#define ATTRIB_DEX		103
#define ATTRIB_CON		104
#define ATTRIB_CHR		105

/* Actions */
#define ATTRIB_IMBUE		200
#define ATTRIB_IGNORE           201 /* Ignore next attribute */
#define ATTRIB_TOGGLE_IGNORE    202 /* Ignore until next toggle */
#define ATTRIB_TRANSFORM        203 /* Turn one thing into another */
#define ATTRIB_HALVE            204 /* Decrease power by half */
#define ATTRIB_DECIMATE         205 /* (original def) reduce to 9 in 10 */

/* Direction */
#define ATTRIB_DIR_SELF         500
#define ATTRIB_DIR_ORIGIN       501
#define ATTRIB_DIR_FOES         502 /* Those fighting me */
#define ATTRIB_DIR_ANTI_FOE     503 /* The enemy of my enemy is my friend */
#define ATTRIB_DIR_CHAR_ID      504 /* Following is char ID of target */
#define ATTRIB_DIR_OBJ_ID       505 /* Following is obj ID of target */

/* Qualities */
#define ATTRIB_TRIG_TOUCH	1000
#define ATTRIB_PERMANENT	1001

/* Target types */
#define TARGET_CHAR	1
#define TARGET_OBJ	2
#define TARGET_ROOM	3
#define TARGET_DOOR	4

#endif /* !defined(MAGIC_H) */
