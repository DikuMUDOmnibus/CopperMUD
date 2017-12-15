
/* This file is derived from nethack, its copyright is below */

/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* block some unused #defines to avoid overloading some cpp's */

#include "structs.h"
#include "comm.h"
#include "utils.h"
#include "proto.h"

static const char *deaths[] = {		/* the array of death */
	"smacked around", "carved up", "gouged", "gassed",
	"poisoned", "turned to stone", "heatstroke", "frozen",
	"poor diet", "viral problems", "bacteria", "bled",
	"organ failure", "burned", "electricuted", "drowning",
	"choked", "quit" };

static const char *ends[] = {		/* "when you..." */
	"died", "died", "died", "were gassed", "were poisoned", 
	"turned to stone", "fainted", "froze", "starved", "sneezed",
	"evacuated", "died", "lost it", "burned", "did the electric slide",
	"drowned", "were choked", "turned chicken" };

#if 0
void dms_show_msg(ch,killer,death)
{
    char *who_str;

}

char * done_in_by(struct char_data *ch,struct char_data *killer)
{
	static char buf[MAX_STRING_LENGTH];
	char *nam;

	buf[0] = '\0';
	if(!killer)
		return buf;
	if(GET_NAME(killer))
		nam=GET_NAME(killer);
	else if(bio_index[killer->phys->species].name)
		nam=bio_index[killer->phys->species].name;
	else
		nam="whatever";
	sprintf(buf,"the mighty %s",nam);
	return buf;
}


void present_end(struct char_data *ch,struct char_data *killer,int how)
{
	struct permonst *upmon;
	char kilbuf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
	/* kilbuf: used to copy killer in case it comes from something like
	 *	xname(), which would otherwise get overwritten when we call
	 *	xname() when listing possessions
	 * buf2: same as player name, except it is capitalized
	 */
	/* Avoid killed by "a" burning or "a" starvation */
	if (!killer && (how == STARVING || how == BURNING))
		killer_format = KILLED_BY;
	strcpy(kilbuf, (!killer || how >= PANICKED ? deaths[how] : killer));
	killer = kilbuf;
	else if(how == STONED)

	if (how == ESCAPED || how == PANICKED)
		killer_format = NO_KILLER_PREFIX;

	disclose(how,taken);

	strcpy(buf2, plname);
	if(!done_stopprint)
	    printf("Goodbye %s the %s...\n\n", buf2, pl_character);
	if(how == ESCAPED) {
		register struct monst *mtmp;
		register struct obj *otmp, *otmp2, *prevobj;
		struct obj *jewels = (struct obj *)0;
		long i;
		register unsigned int worthlessct = 0;

		/* put items that count into jewels chain
		 * rewriting the fcobj and invent chains here is safe,
		 * as they'll never be used again
		 */
		for(otmp = fcobj; otmp; otmp = otmp2) {
			otmp2 = otmp->nobj;
			if(carried(otmp->cobj)
					&& ((otmp->olet == GEM_SYM &&
					     otmp->otyp < LUCKSTONE)
					    || otmp->olet == AMULET_SYM)) {
				if(otmp == fcobj)
					fcobj = otmp->nobj;
				else
					prevobj->nobj = otmp->nobj;
				otmp->nobj = jewels;
				jewels = otmp;
			} else
				prevobj = otmp;
		}
		for(otmp = invent; otmp; otmp = otmp2) {
			otmp2 = otmp->nobj;
			if((otmp->olet == GEM_SYM && otmp->otyp < LUCKSTONE)
					    || otmp->olet == AMULET_SYM) {
				if(otmp == invent)
					invent = otmp->nobj;
				else
					prevobj->nobj = otmp->nobj;
				otmp->nobj = jewels;
				jewels = otmp;
			} else
				prevobj = otmp;
		}

		/* add points for jewels */
		for(otmp = jewels; otmp; otmp = otmp->nobj) {
			if(otmp->olet == GEM_SYM)
				u.urexp += (long) otmp->quan *
					    objects[otmp->otyp].g_val;
			else 	/* amulet */
				u.urexp += (otmp->spe < 0) ? 2 :
					otmp->otyp == AMULET_OF_YENDOR ?
							5000 : 500;
		}

		if(mtmp) {
			if(!done_stopprint) printf("You");
			while(mtmp) {
				if(!done_stopprint)
					printf(" and %s", mon_nam(mtmp));
				if(mtmp->mtame)
					u.urexp += mtmp->mhp;
				mtmp = mtmp->nmon;
			}
		} else
		if(!done_stopprint)
		  printf("You %s with %ld points,\n",
			how==ASCENDED ? "went to your reward"
		    u.urexp);

		/* print jewels chain here */
		for(otmp = jewels; otmp; otmp = otmp->nobj) {
			makeknown(otmp->otyp);
			if(otmp->olet == GEM_SYM && otmp->otyp < LUCKSTONE) {
				i = (long) otmp->quan *
					objects[otmp->otyp].g_val;
				if(i == 0) {
					worthlessct += otmp->quan;
					continue;
				}
				printf("        %s (worth %ld zorkmids),\n",
				    doname(otmp), i);
			} else {		/* amulet */
				otmp->known = 1;
				i = (otmp->spe < 0) ? 2 :
					otmp->otyp == AMULET_OF_YENDOR ?
							5000 : 500;
				printf("        %s (worth %ld zorkmids),\n",
				    doname(otmp), i);
			}
		}
		if(worthlessct)
		  printf("        %u worthless piece%s of colored glass,\n",
			worthlessct, plur((long)worthlessct));
	} else
		if(!done_stopprint) {
		    printf("You %s ", ends[how]);
		    printf("with %ld points,\n", u.urexp);
		}
	if(!done_stopprint)
	  printf("and %ld piece%s of gold, after %ld move%s.\n",
	    u.ugold, plur(u.ugold), moves, plur(moves));
	if(!done_stopprint)
  printf("You were level %u with a maximum of %d hit points when you %s.\n",
	    u.ulevel, u.uhpmax, ends[how]);
	{
		if (!done_stopprint) {
			getret();
			cls();
		}
/* "So when I die, the first thing I will see in Heaven is a score list?" */
		topten(how);
	}
	if(done_stopprint) printf("\n\n");
	exit(0);
}
#endif

char *rip_txt[] = {
"                       ----------",
"                      /          \\",
"                     /    REST    \\",
"                    /      IN      \\",
"                   /     PEACE      \\",
"                  /                  \\",
"                  |                  |",
"                  |                  |",
"                  |                  |",
"                  |                  |",
"                  |                  |",
"                  |       1001       |",
"                 *|     *  *  *      | *",
"        _________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______\n",
0
};

char **rip;

static void
center(line, text)
int line;
char *text;
{
	register char *ip,*op;
	ip = text;
	op = &rip_txt[line][28 - ((strlen(text)+1)>>1)];
	while(*ip) *op++ = *ip++;
}

void clean_rip()
{
	int i;

	for(i=6;i<12;i++)
		center(i,"                 ");
}

void outrip(struct char_data *ch)
{
	char buf[MAX_STRING_LENGTH];
	register int y;
	extern struct time_info_data time_info;
/*	int killed_by_line = 0;*/

/*	cls();*/
	sprintf(buf, "%s", GET_NAME(ch));
	buf[16] = 0;
	center(6, buf);
	sprintf(buf, "-- Au");
	center(7, buf);
#if 0
	if (killer_format != NO_KILLER_PREFIX) {
		killed_by_line = 1;
		strcpy(buf, "killed by");
		if (killer_format == KILLED_BY_AN)
			strcat(buf, index(vowels, *killer) ? " an" : " a");
		center(8, buf);
	}
	strcpy(buf, killer);
	if(strlen(buf) > 16) {
	    register int i,i0,i1;
		i0 = i1 = 0;
		for(i = 0; i <= 16; i++)
			if(buf[i] == ' ') i0 = i, i1 = i+1;
		if(!i0) i0 = i1 = 16;
		buf[i1 + 16] = 0;
		center(9 + killed_by_line, buf+i1);
		buf[i0] = 0;
	}
	center(8 + killed_by_line, buf);
#endif
	sprintf(buf, "%4dC.E.", time_info.year);
	center(11, buf);
	for(y=0;rip_txt[y];y++) {
		sprintf(buf,"              %s\n",rip_txt[y]);
		SEND_TO_Q(buf,ch->desc);
	}
	clean_rip();
}

