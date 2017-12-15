/* ************************************************************************
*  file: skills.h , Definitions for skills and spells.    Part of DIKUMUD *
*  Usage : What usage?                                                    *
************************************************************************* */

#ifndef SKILLS_H
#define SKILLS_H

/* Skill numbers */
#define SKILL_LEARN                   0
#define SKILL_OFFENSE                 1
#define SKILL_DEFENSE                 2
#define SKILL_SOCIAL                  3
#define SKILL_CAST                    4

#define SKILL_LEVITATE               10
#define SKILL_FLY                    11

/* Blow are old skills, possible to reuse some of them */

#define TYPE_UNDEFINED                0 /* temporary..need to remove sometime*/

#define SKILL_ARMOR                   1 
#define SKILL_TELEPORT                2 
#define SKILL_BLESS                   3 
#define SKILL_BLINDNESS               4 
#define SKILL_BURNING_HANDS           5 
#define SKILL_CALL_LIGHTNING          6 
#define SKILL_CHARM_PERSON            7 
#define SKILL_CHILL_TOUCH             8 
#define SKILL_CLONE                   9 
#define SKILL_COLOUR_SPRAY           10 
#define SKILL_CONTROL_WEATHER        11 
#define SKILL_CREATE_FOOD            12 
#define SKILL_CREATE_WATER           13 
#define SKILL_CURE_BLIND             14 
#define SKILL_CURE_CRITIC            15 
#define SKILL_CURE_LIGHT             16 
#define SKILL_CURSE                  17 
#define SKILL_DETECT_EVIL            18 
#define SKILL_DETECT_INVISIBLE       19 
#define SKILL_DETECT_MAGIC           20 
#define SKILL_DETECT_POISON          21 
#define SKILL_DISPEL_EVIL            22 
#define SKILL_EARTHQUAKE             23 
#define SKILL_ENCHANT_WEAPON         24 
#define SKILL_ENERGY_DRAIN           25 
#define SKILL_FIREBALL               26 
#define SKILL_HARM                   27 
#define SKILL_HEAL                   28 
#define SKILL_INVISIBLE              29 
#define SKILL_LIGHTNING_BOLT         30 
#define SKILL_LOCATE_OBJECT          31 
#define SKILL_MAGIC_MISSILE          32 
#define SKILL_POISON                 33 
#define SKILL_PROTECT_FROM_EVIL      34 
#define SKILL_REMOVE_CURSE           35 
#define SKILL_SANCTUARY              36 
#define SKILL_SHOCKING_GRASP         37 
#define SKILL_SLEEP                  38 
#define SKILL_STRENGTH               39 
#define SKILL_SUMMON                 40 
#define SKILL_VENTRILOQUATE          41 
#define SKILL_REMOVE_POISON          43 
#define SKILL_SENSE_LIFE             44 

#define SKILL_SNEAK                  45 
#define SKILL_HIDE                   46 
#define SKILL_STEAL                  47 
#define SKILL_BACKSTAB               48 
#define SKILL_PICK_LOCK              49 

#define SKILL_KICK                   50 
#define SKILL_BASH                   51 
#define SKILL_RESCUE                 52

#define SKILL_IDENTIFY               53
#define SKILL_ANIMATE_DEAD           54
#define SKILL_FEAR                   55
#define SKILL_FIRE_BREATH            56
#define SKILL_GAS_BREATH             57
#define SKILL_FROST_BREATH           58
#define SKILL_ACID_BREATH            59
#define SKILL_LIGHTNING_BREATH       60
#define SKILL_AWARENESS              56
#define SKILL_QUICKNESS              57


#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO    64 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512


/* Possible Targets:

 bit 1 : PC/NPC in room
 bit 2 : PC/NPC in world
 bit 3 : Object held
 bit 4 : Object in inventory
 bit 5 : Object in room
 bit 6 : Object in world
 bit 7 : If fighting, and no argument, select tar_char as self
 bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
 bit 9 : If no argument, select self, if argument check that it IS self.

*/

#endif /* !defined(SKILLS_H) */
