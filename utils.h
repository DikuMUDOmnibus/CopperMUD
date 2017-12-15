
/* ************************************************************************
*  file: utils.h, Utility module.                         Part of DIKUMUD *
*  Usage: Utility macros                                                  *
************************************************************************* */

#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

/* Functions from utility.c */
#if !HAVE_MAXMIN
int MAX(int a, int b);
int MIN(int a, int b);
#endif
bool CAN_SEE(struct char_data *ch,struct char_data *vict);
bool CAN_SEE_OBJ(struct char_data *ch,struct obj_data *obj);
void log(char *str);
int number(int from,int to);
int dice(int number,int size);
int str_cmp(char *arg1, char *arg2);
int strn_cmp(char *arg1, char *arg2, int n);
void sprintbit(long vektor, char *names[], char *result);
void sprinttype(int type, char *names[], char *result);
struct time_info_data real_time_passed(time_t t2, time_t t1);
struct time_info_data mud_time_passed(time_t t2, time_t t1);
struct time_info_data age(struct char_data *ch);
int default_loc(int hometown);
int jail_hometown(int hometown);
void strToLower(char *s1);

#define IS_AFFECTED(x,y,z) (x > y > z > hfsjdhfks)

#define TRUE  1

#define FALSE 0

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))

#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 

#define IF_STR(st) ((st) ? (st) : "\0")

#define CAP(st)  (*(st) = UPPER(*(st)), st)

#define CREATE(result, type, number)  do {\
    if (!((result) = (type *) calloc ((number), sizeof(type))))\
	{ perror("malloc failure"); abort(); } } while(0)

#define RECREATE(result,type,number) do {\
 if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
	{ perror("realloc failure"); abort(); } } while(0)

#define IS_SET(flag,bit)  ((flag) & (bit))

#define SWITCH(a,b) { (a) ^= (b); \
	   (b) ^= (a); \
	   (a) ^= (b); }

#define IS_DARK(room)  ( !world[room].light && \
	    (IS_SET(world[room].room_flags, DARK) || \
	     ( ( world[room].sector_type != SECT_INSIDE && \
	       world[room].sector_type != SECT_CITY ) && \
	      (zone_table[world[room].zone].conditions.ambient_light < 20)) ) )

#define IS_LIGHT(room)  (!IS_DARK(room))

#define SET_BIT(var,bit)  ((var) = (var) | (bit))

#define REMOVE_BIT(var,bit)  ((var) = (var) & ~(bit) )

#define GET_REQ(i) (i<2  ? "Awful" :(i<4  ? "Bad"     :(i<7  ? "Poor"      :\
(i<10 ? "Average" :(i<14 ? "Fair"    :(i<20 ? "Good"    :(i<24 ? "Very good" :\
    "Superb" )))))))

#define HSHR(ch) (GET_SEX(ch) ?					\
    ((GET_SEX(ch) == 1) ? "his" : "her") : "its")

#define HSSH(ch) (GET_SEX(ch) ?					\
    ((GET_SEX(ch) == 1) ? "he" : "she") : "it")

#define HMHR(ch) (GET_SEX(ch) ? 					\
    ((GET_SEX(ch) == 1) ? "him" : "her") : "it")	

#define ANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")

#define SANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define IS_NPC(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC))

#define IS_MOB(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC) && ((ch)->nr >-1))

#define IS_FIGHTING(ch) (ch->specials.fighting)

#define IS_HUMANOID(ch) (!IS_NPC(ch) || GET_CLASS(ch)==CLASS_HUMANOID)

#define GET_POS(ch)     ((ch)->specials.position)

#define GET_NAME(ch)    ((ch)->player.name)

#define GET_TITLE(ch)   ((ch)->player.title)

#define GET_CLASS(ch)   ((ch)->player.class)

#define GET_HOME(ch)	((ch)->player.hometown)

#define GET_AGE(ch)     (age(ch).year)

#define GET_STR(ch)     ((ch)->tmpabilities.str)

#define GET_DEX(ch)     ((ch)->tmpabilities.dex)

#define GET_INT(ch)     ((ch)->tmpabilities.intel)

#define GET_WIS(ch)     ((ch)->tmpabilities.wis)

#define GET_CON(ch)     ((ch)->tmpabilities.con)

#define GET_CHR(ch)     ((ch)->tmpabilities.chr)

#define GET_AC(ch)      ((ch)->physical->armor)

#define GET_HIT(ch)     ((ch)->physical->hit)

#define GET_MAX_HIT(ch) (hit_limit(ch))

#define GET_MOVE(ch)    ((ch)->physical->move)

#define GET_MAX_MOVE(ch) (move_limit(ch))

#define GET_MANA(ch)    ((ch)->physical->mana)

#define GET_MAX_MANA(ch) (mana_limit(ch))

#define GET_LUCK(ch)    ((ch)->points.luck)

#define GET_EXP(ch)     ((ch)->points.exp)

#define GET_SPECIES(ch) ((ch)->physical->species)

#define GET_SIZE(ch)    ((ch)->physical->size)

#define GET_HEIGHT(ch)  ((ch)->physical->height)

#define GET_WEIGHT(ch)  ((ch)->physical->weight)

#define GET_SEX(ch)     ((ch)->physical->sex)

#define GET_HITROLL(ch) ((ch)->points.hitroll)

#define GET_DAMROLL(ch) ((ch)->points.damroll)

#define AWAKE(ch) (GET_POS(ch) > POS_SLEEP)



/* Object And Carry related macros */

#define GET_OBJ_WEIGHT(obj) ((obj)->obj_flags.weight)

#define CAN_CARRY_W(ch) (str_app[GET_STR(ch)].carry_w)

#define CAN_CARRY_N(ch) (5+GET_DEX(ch)/2)

#define IS_CARRYING_W(ch) ((ch)->specials.carry_weight)

#define IS_CARRYING_N(ch) ((ch)->specials.carry_items)

#define CAN_CARRY_OBJ(ch,obj)  \
 (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
  ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
 (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&          \
  CAN_SEE_OBJ((ch),(obj)))

#define IS_OBJ_STAT(obj,stat) (IS_SET((obj)->obj_flags.extra_flags,stat))



/* char name/short_desc(for mobs) or someone?  */

#define PERS(ch, vict)   (                                          \
    CAN_SEE(vict, ch) ?						                                    \
     (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) :	\
     "someone")

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
    (obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
    fname((obj)->name) : "something")

#define OUTSIDE(ch) (!IS_SET(world[(ch)->in_room].room_flags,INDOORS))

#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_OUTLAW(ch) ( IS_SET(ch->specials.act,ACT_OUTLAW))

#endif /* !defined(UTILS_H) */
