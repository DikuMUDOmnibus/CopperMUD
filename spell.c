/* ************************************************************************
*  file: spell.c          Basic routines and parsing      Part of DIKUMUD *
*  Usage : Interpreter of cast magic                                      *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */


#include CONFIG

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interp.h" 
#include "skills.h"
#include "magic.h"
#include "error.h"
#include "proto.h"

#if HAVE_STRINGS_H
#include <strings.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif


/* Global data */

extern struct room_data *world;
extern char *spell_wear_off_msg[];
extern struct mattrib_info mattribs[];
extern char log_buf[];
extern char *errors[];

/* Extern procedures */

char *strdup(char *str);

/* Extern procedures */



const byte saving_throws[4][5][25] = {
{
 {16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,10,10,10,10,10, 8, 6, 4, 0},
 {13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 0},
 {15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 0},
 {17,15,15,15,15,15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 5, 3, 0},
 {14,12,12,12,12,12,10,10,10,10,10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 0}
}, {
 {11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 0},
 {16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 0},
 {15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 0},
 {18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 0},
 {17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 0}
}, {
 {15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 7, 6, 0},
 {16,14,14,14,14,12,12,12,12,10,10,10,10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 0},
 {14,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 0},
 {18,16,16,16,16,15,15,15,15,14,14,14,14,13,13,13,13,12,12,12,12,11, 9, 5, 0},
 {17,15,15,15,15,13,13,13,13,11,11,11,11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 0}
}, {
 {16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 0},
 {18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 0},
 {17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 0},
 {20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0},
 {19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 0}
}
};


int USE_MANA(struct char_data *ch,int sn)
{
    int mana;
/*	if(GET_LEVEL(ch)>=SPELL_LEVEL(ch,sn))
      mana=((float)spell_info[sn].min_usesmana * 
	(1.0+(1.0/(float)(GET_LEVEL(ch)-SPELL_LEVEL(ch,sn)+1.0))));
    else
      mana=2*spell_info[sn].min_usesmana+SPELL_LEVEL(ch,sn)-GET_LEVEL(ch);
*/
mana=number(5,60);
    mana=MAX(2,MIN(100,mana));
    return(mana);
}


/* I am loathe to make it too easy to decode spells, but it shouldn't
 be impossible either - just enough not impossible so small gains can
 be made without endangering vast portions of the scheme. I consider
 an index which randomly changes the order of syllables taken, yet
 remain consistent for the spell ("aaabbb" becoming "ytytytooo" and
 "pipipildaldalda" on different castings, for example)...
*/
void say_spell( struct char_data *ch, char *name )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    int j, offs;
    struct char_data *temp_char;


    struct s_syllable {
	char org[10];
	char new[10];
    };

    struct s_syllable syls[] = {
    { " ", " " },
    { "ar", "abra"   },
    { "au", "kada"    },
    { "bles", "fid" },
    { "bl", "no" },
    { "ind", "se" },
    { "bur", "mosa" },
    { "cu", "judi" },
    { "de", "oculo"},
    { "en", "unso" },
    { "light", "dies" },
    { "lo", "hi" },
    { "mor", "zak" },
    { "move", "sido" },
    { "ness", "locri" },
    { "ning", "illa" },
    { "per", "duda" },
    { "ra", "gru"   },
    { "re", "candus" },
    { "son", "sabru" },
    { "tect", "infra" },
    { "tri", "cula" },
    { "ven", "nofo" },
    { ":", "heh" },
    { ",", "uh"},
    { ".", "eh"},
    {"a", "a"},{"b","b"},{"c","q"},{"d","e"},{"e","z"},{"f","y"},{"g","o"},
    {"h", "p"},{"i","u"},{"j","y"},{"k","t"},{"l","r"},{"m","w"},{"n","i"},
    {"o", "a"},{"p","s"},{"q","d"},{"r","f"},{"s","g"},{"t","h"},{"u","j"},
    {"v", "z"},{"w","x"},{"x","n"},{"y","l"},{"z","k"}, {"",""}
    };



    strcpy(buf, "");

    offs = 0;

    while(*(name+offs)) {
	for(j=0; *(syls[j].org); j++)
	    if (strncmp(syls[j].org, name+offs, strlen(syls[j].org))==0) {
		strcat(buf, syls[j].new);
		if (strlen(syls[j].org))
		    offs+=strlen(syls[j].org);
		else
		    ++offs;
                break;
	    }
        if(!*(syls[j].org)) { /* we didn't match */
            ++offs;
        }
    }


    sprintf(buf2, "You utter the words, '%s'", buf);
    act(buf2, FALSE, ch, 0, temp_char, TO_CHAR);

    sprintf(buf2,"$n utters the words, '%s'", buf);
    for(temp_char = world[ch->in_room].people;
	temp_char;
	temp_char = temp_char->next_in_room)
	if(temp_char != ch) {
	    act(buf2, FALSE, ch, 0, temp_char, TO_VICT);
	}
}



bool saves_spell(struct char_data *ch, sh_int save_type)
{
    int save;

    /* Negative apply_saving_throw makes saving throw better! */

    save = ch->specials.apply_saving_throw[save_type];

    if (!IS_NPC(ch)) {
	/*save += saving_throws[GET_CLASS(ch)-1][save_type][GET_LEVEL(ch)];*/
    }

    return(MAX(1,save) < number(1,20));
}

int count_lines(FILE *fl)
{
    char buf[100];
    int count=0;

    rewind(fl);
    while(fgets(buf,99,fl)) {
        count++;
    }
    rewind(fl);
    return(count);
}


int top_syllable;
struct syllable *syllables;

void boot_syllables()
{
    FILE *fl;
    int i,count;
    char buf[80],*token;

    if(!(fl=fopen(SYL_FILE,"r"))) {
        log("BUG: Can't open syllables");
        return; /* or exit? */
    }

    i = count_lines(fl) - count_char('#',fl);
    CREATE(syllables,struct syllable,i);

    top_syllable=0;

    while(fgets(buf,80,fl)) {

        if(buf[0]=='#') /* comment */
            continue;

        token=strtok(buf," \n");

        syllables[top_syllable].syl = strdup(token);

        count=0;
        while((token=strtok(NULL,",\n "))) {
            for(i=0;*mattribs[i].name!='\n';i++) {
                if(!strcasecmp(token,mattribs[i].name)) {
                    syllables[top_syllable].impart[count++]=i;
                    break;
                }
            }
            if(*mattribs[i].name=='\n') {
                sprintf(log_buf,"BUG: Unknown magic attribute %s in syl file",
                    token);
                log(log_buf);
            }
        }

        for(i=count;i<8;i++)
            syllables[top_syllable].impart[i]=-1;

        top_syllable++;
    }
}

struct syllable *match_syl(char *input)
{
    int i;

    for(i=0;i<top_syllable;i++)
        if(!(strcmp(syllables[i].syl,input)))
            return &syllables[i];

    return(NULL);
}

char *skip_spaces(char *string)
{
    for(;*string && (*string)==' ';string++);

    return(string);
}


/* Assumes that *argument does start with first letter of chopped string */

int do_cast(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *tar_obj;
    struct char_data *tar_char;
    struct char_skill_data *sk;
    struct syllable *syl;
    char name[MAX_STRING_LENGTH], sbuf[MAX_INPUT_LENGTH], *token, *say_save;
    int qend, ret, i;
    int last_impart,impart[100]; /* should be large enough */
    bool target_ok;
    void *target;
    struct magic *m;
    struct generic object,creator;
    int error;

    if(!(sk=get_skill(ch,SKILL_CAST))) {
        send_to_char("You don't seem to know how.\n",ch);
        return ERROR_NO_KNOWLEDGE;
    }

    /* Let's consider how to pass arguments here...
     *
     * spell vs. attrib?
     *
     * cast 'spell' target  (with book/scroll in hand?)
     *
     * cast 'attrib' target - this seems wrong, somehow...
     * - a spell is a list of attributes - perhaps a way
     * to make a list? (shall we delay the caster by the
     * complexity of spell/attrib-list?)
     *
     */

    argument = skip_spaces(argument);

    /* If there is no chars in argument */
    if (!(*argument)) {
	send_to_char("Cast which what where?\n\r", ch);
	return ERROR_SYNTAX;
    }

    if (*argument != '\'') {
	send_to_char("Magic must always be enclosed by the holy magic symbols : '\n\r",ch);
	return ERROR_SYNTAX;
    }

    /* Locate the last quote && lowercase the magic words (if any) */

    for (qend=1; *(argument+qend) && (*(argument+qend) != '\'') ; qend++)
	*(argument+qend) = LOWER(*(argument+qend));

    if (*(argument+qend) != '\'') {
	send_to_char("Magic must always be enclosed by the holy magic symbols : '\n\r",ch);
	return ERROR_SYNTAX;
    }

    /* Save that magic while we can */
    say_save=argument+1;
    say_save[qend]='\0';
    strncpy(sbuf,argument+1,qend-1);
    sbuf[qend]='\0';

    /* Syllabificate that bastard! */
    last_impart=0;
    token=strtok(sbuf," ,.:!");
    while(token) {
        /* If no match, we add garbage */
        if(!(syl=match_syl(token))) {
            impart[last_impart++] = 0; /*garbage it when I have time */
        } else {
            /* Eventually check to see if this is even known... */
            for(i=0;i<8 && syl->impart[i]!=-1;i++) {
                impart[last_impart++] = syl->impart[i];
            }
        }

        token=strtok(NULL," ,.:!");
    }


    say_spell(ch, say_save);

    argument+=qend+1;	/* Point to the last ' */
    for(;*argument == ' '; argument++);

    /* **************** Locate targets **************** */

    target_ok = FALSE;
    tar_char = 0;
    tar_obj = 0;
    target = 0;

    one_argument(argument, name);

    if(*name) {
        ret = generic_find(name, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
                       FIND_ROOM, ch,&target);

        if(!ret) {
            /* we're we thinking of something? */
            /* if not, then... */
            send_to_char("You must cast on something.\n\r",ch);
            return ERROR_MISSING_TARGET;
        }
        object.attach_type = (ret==FIND_CHAR_ROOM ? ATTACH_CHAR :
                        (ret==FIND_OBJ_INV || ret==FIND_OBJ_ROOM ? ATTACH_OBJ :
                        ATTACH_ROOM));
        object.attached_to.ch=(struct char_data *)target;
    } else {
        /** no arg means cast on yourself, for now **/
        object.attach_type = ATTACH_CHAR;
        object.attached_to.ch = ch;
    }

/*TEMP TEMP
    if (number(0,100) > sk->learned) {
        send_to_char("You lost your concentration!\n\r", ch);
		    GET_MANA(ch) -= (USE_MANA(ch, spl)>>1);
		    return ERROR_FAILED;
    }
***/

    if(IS_SET(world[ch->in_room].room_flags,NO_MAGIC))
        send_to_char("The magic gathers, then fades away.\n\r",ch);

    creator.attach_type = ATTACH_CHAR;
    creator.attached_to.ch = ch;
    if((error=create_magic(&creator,&m))!=OKAY)
        return(error);

    if((error=bind_magic_to_object(m,&object))!=OKAY)
        return(error);

    for(i=0;i<last_impart;i++) {
        sprintf(sbuf,"Syl %d = %s[%d]\n",i,mattribs[impart[i]].name,impart[i]);
        send_to_char(sbuf,ch);
        
        if((error = attribute_to_magic(m,mattribs[impart[i]].number,-5))) {
            sprintf(sbuf,"Error: %s\n",errors[error]);
            send_to_char(sbuf,ch);
        }
    }

    expend_magic(m,&object);

    return OKAY;

}
