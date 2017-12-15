/* ************************************************************************
*  file: save.c      Implementation of save files         Part of Copper3 *
*  Usage : load_char/save_char                                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#ident	"@(#) $Id:$\n"
/* $Log:$
 */

/*
 Crashproof saving of players and items, some parts and ideas
 originally by Taquin Ho (prometheus) and Jeff Stile (Abaddon),
 others from original db.c - Mostly rewritten for Copper III

 The call chain looks like this:

    Loading
    =======

    load_char(name):
	<read from file>
	store_to_char();
	load_obj(file);

    load_obj(file):
        store_to_obj();
        <recurse>


    Saving
    ======

    save_char(ch):
	char_to_store(ch,<store>);
	<write to file>
	save_obj(file);

    save_obj(file):
        obj_to_store(obj);
        <recurse>

*/



#include CONFIG

#if HAVE_STRINGS_H

#include <strings.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif


#include <ctype.h>

#include "structs.h"
#include "save.h"
#include "utils.h"
#include "comm.h"
#include "player.h"
#include "db.h"
#include "error.h"
#include "proto.h"

#include <sys/timeb.h>

void obj_material_to_store(struct obj_material *om, struct obj_material_u *st);
void obj_affect_to_store(struct obj_affect *oa, struct obj_affect_u *st);
void org_to_store(struct char_org_data *org, struct org_u *st);
void skill_to_store(struct char_skill_data *sk, struct skill_u *st);
void phys_to_store(struct char_phys_data *ph, struct phys_u *st);
void affect_to_store(struct affected_type *af, struct affect_u *st);
void obj_to_store(struct obj_data *obj, struct obj_file_elem *object);
void char_to_store(struct char_data *ch, struct char_file_u *st);
void store_to_org(struct org_u *st, struct char_org_data *org);
void store_to_skill(struct skill_u *st, struct char_skill_data *sk);
void store_to_phys(struct phys_u *st, struct char_phys_data *ph);
void store_to_affect(struct affect_u *st, struct affected_type *af);
void store_to_obj_material(struct obj_material_u *st, struct obj_material *om);
void store_to_obj_affect(struct obj_affect_u *st, struct obj_affect *oa);
void store_to_obj(struct obj_data *obj, struct obj_file_elem *object);
void store_to_char(struct char_file_u *st, struct char_data *ch);
void save_obj(struct obj_data *obj,FILE *fl);
struct obj_data *load_obj(FILE *fl);
void save_string(char *str,FILE *fl);
char *load_string(FILE *fl);

int unlink(char *path);

extern struct obj_index_data *obj_index;
extern struct room_data *world;
extern int top_of_world;
extern char log_buf[];
extern size_t obj_info_size[];


/**** Loading Routines ****/

void load_world_obj()
{
    FILE *fl;
    struct obj_data *o;
    int to_room;

    if(!(fl=fopen("save/world.objs","r"))) {
        perror("fopen");
        log("BUG: Cannot open world object file");
        return;
    }

    while(!feof(fl)) {
        o=load_obj(fl);
        if(o) {
            to_room=o->in_room;
            o->in_room=NOWHERE;
            obj_to_room(o,to_room);
        }
    }
}

struct obj_data *load_obj(FILE *fl)
{
    int j;
    struct obj_data *obj,*contained;
    struct obj_file_elem object;
    struct obj_material_u material_elem;
    struct obj_material *material;
    struct obj_affect_u affect_elem;
    struct obj_affect *affect;
    struct obj_info type,*info;

    if(!fread(&object, sizeof(struct obj_file_elem), 1, fl))
        return NULL;

    obj = read_object(object.item_number, VIRTUAL);
    if(!obj) {
        log("BUG: Cannot load saved object");
        return NULL;
    }
    store_to_obj(obj,&object);

    for(j=0;j < object.num_materials;j++) {
	fread(&material_elem,sizeof(material_elem),1,fl);
	CREATE(material,struct obj_material,1);
        store_to_obj_material(&material_elem,material);
        material->next = obj->material;
        obj->material = material;
    }

    for(j=0;j < object.num_affects;j++) {
	fread(&affect_elem,sizeof(affect_elem),1,fl);
        CREATE(affect,struct obj_affect,1);
	store_to_obj_affect(&affect_elem,affect);
        affect->next = obj->affected;
        obj->affected = affect;
    }


    /* This could use some explanation:
     *
     * First, we read the obj_info type, so we know what kind
     * of record to expect.
     *
     * Then, we allocate that kind of info for the object, and
     * load into it starting after the obj_type entry the
     * remaining part of the record.
     */
    for(j=0;j< object.num_info;j++) {
        fread(&type,sizeof(type),1,fl);
        info=new_obj_info(type.obj_type,obj);

        fread(info+sizeof(struct obj_info),
              obj_info_size[type.obj_type]-sizeof(struct obj_info),
              1,fl);
    }

    /* Take care of contained items */
    for(j=0;j< object.num_contains;j++) {
	contained=load_obj(fl);
        if(contained)
            obj_to_obj(contained,obj);
    }
    return(obj);
}



/* Load a char, TRUE if loaded, FALSE if not */
struct char_data * load_char(char *name)
{
    FILE *fl;
    char buf[30];
    int result,i,eq_pos;
    struct char_file_u char_element;
    struct org_u org_elem;
    struct char_org_data *org;
    struct skill_u sk_elem;
    struct char_skill_data *sk;
    struct phys_u ph_elem;
    struct char_phys_data *ph, **phs;
    struct affect_u af_elem;
    struct affected_type *af;
    struct char_data *ch;
    struct obj_data *obj;

    CAP(name);

    sprintf(buf,"save/%s",name);
    if (!(fl = fopen(buf, "r")))
	return(NULL);

    ch = bare_char();

    result = fread(&char_element, sizeof(struct char_file_u), 1, fl);
    store_to_char(&char_element,ch);

    /*** Load all variable length items... ***/

    /* Affects */
    for(i=0;i<char_element.num_affect;i++) {
        result = fread(&af_elem,sizeof(struct affect_u),1,fl);
        CREATE(af,struct affected_type,1);
        store_to_affect(&af_elem,af);
        af->next=ch->affected;
        ch->affected=af;
    }


    /* Organizational information */
    ch->orgs=NULL;
    for(i=0;i<char_element.num_org;i++) {
        result = fread(&org_elem,sizeof(struct org_u),1,fl);
        CREATE(org,struct char_org_data,1);
        store_to_org(&org_elem,org);
        org->next=ch->orgs;
        ch->orgs=org;
    }

    /* Skills */
    for(i=0;i<char_element.num_skill;i++) {
        result = fread(&sk_elem,sizeof(struct skill_u),1,fl);
        CREATE(sk,struct char_skill_data,1);
        store_to_skill(&sk_elem,sk);
        sk->next=ch->skills;
        ch->skills=sk;
    }

    /* Physical */
    phs=&ch->physical;
    for(i=0;i<char_element.num_phys;i++) {
        result = fread(&ph_elem,sizeof(struct phys_u),1,fl);
        CREATE(ph,struct char_phys_data,1);
        store_to_phys(&ph_elem,ph);
        *phs = ph;
        ph->next=NULL;
        phs = &ph->next;
    }

    /* Player information */
    if(!(ch->specials.act & ACT_ISNPC)) {
        CREATE(ch->prefs,struct player_prefs,1);
        result = fread(ch->prefs,sizeof(struct player_prefs),1,fl);
    } else {
        ch->prefs = NULL;
    }

    ch->tmpabilities = ch->physical->abilities;

    /* Now load the objects */

    /* As yet, this does not properly equip the character */
    for(i=0;i < char_element.num_item;i++) {
        obj=load_obj(fl);
        if(obj) {
            if(obj->equipped_as != UNEQUIPPED) {
                /* Need to juggle this to stay sane */
                eq_pos = obj->equipped_as;
                obj->equipped_as = UNEQUIPPED;
                equip_char(ch,obj,eq_pos);
            } else {
        	    obj_to_char(obj,ch);
            }
        }
    }

    fclose(fl);

    return ch;
}






/****                 ****/
/**** Saving Routines ****/
/****                 ****/

int save_world_obj(int save_obj_state)
{
    static FILE *so_fl=NULL;
    struct obj_data *o;
    static int saving_room;
    int max;

    switch(save_obj_state) {
        case SO_START:
            if(!(so_fl=fopen("save/world.objs","w"))) {
                perror("fopen");
                log("BUG: Cannot save world objects");
                return -1;
            }
            saving_room=0;
            break;

        case SO_SAVE:
            max=saving_room+50;
            for(;saving_room<max && saving_room < top_of_world;saving_room++)
                for(o=world[saving_room].contents;o;o=o->next_content)
                    if(!(o->obj_flags.extra_flags & ITEM_IGNORE))
                        save_obj(o,so_fl);
            if(saving_room >= top_of_world)
                return 1;
            break;

        case SO_DONE:
            fclose(so_fl);
            break;

        case SO_SAVEALL:
            for(saving_room=0;saving_room < top_of_world;saving_room++)
                for(o=world[saving_room].contents;o;o=o->next_content)
                    if(!(o->obj_flags.extra_flags & ITEM_IGNORE))
                        save_obj(o,so_fl);
            break;
    }
    return 0;
}

/*
* Assumes file is open with write privilege
*/

void save_obj(struct obj_data *obj,FILE *fl)
{
    struct obj_data *tmp;
    struct obj_file_elem object;
    struct obj_material_u material_elem;
    struct obj_material *om;
    struct obj_affect_u affect_elem;
    struct obj_affect *oa;
    struct obj_info *info;

    obj_to_store(obj, &object);
    if (fwrite(&object, sizeof(struct obj_file_elem), 1, fl) < 1) {
	perror("writing save data -save_obj");
	exit(1);
    }

    for(om=obj->material;om;om=om->next) {
        obj_material_to_store(om,&material_elem);
        fwrite(&material_elem,sizeof(struct obj_material_u),1,fl);
    }

    for(oa=obj->affected;oa;oa=oa->next) {
        obj_affect_to_store(oa,&affect_elem);
        fwrite(&affect_elem,sizeof(struct obj_affect_u),1,fl);
    }

    for(info=obj->info;info;info=info->next) {
        /* This mumbo-jumbo is so we don't have *_u records for each
           info_type there is... and to save 4 bytes per record */

	/* NOTE: upon further consideration, things like the strings
	 * in obj_info_exit suggest we will have to use *_u records...
         * oh well...
         */
        if(fwrite(&info->obj_type,
               obj_info_size[info->obj_type] - sizeof(void *),
               1,fl) < 1) {
            perror("writing save data -save_obj infos");
            exit(1);
        }
    }

    for(tmp=obj->contains;tmp;tmp=tmp->next_content) {
	save_obj(tmp,fl);
    }
}


/* write the vital data of a player to the player file */
void save_char(struct char_data *ch)
{
    int result;
    struct char_file_u st;
    struct obj_data *obj;
    struct skill_u skill_elem;
    struct org_u org_elem;
    struct affect_u af_elem;
    struct phys_u phys_elem;
    struct char_skill_data *sk;
    struct char_phys_data *ph;
    struct char_org_data *org;
    struct affected_type *af;
    FILE *fl;
    char buf[30];

/*    if (IS_NPC(ch) || !ch->desc)
	return;
*/

    if(ch->specials.act & ACT_NOSAVE) {
	log("BUG: Trying to save a guest character");
	return;
    }

    char_to_store(ch, &st);

    if (IS_NPC(ch))
        sprintf(buf,"save/%d",ch->id);
    else
        sprintf(buf,"save/%s",GET_NAME(ch));
    if (!(fl = fopen(buf, "w")))
    {
	perror("save char");
	exit(1);
    }

    if(!(fwrite(&st, sizeof(struct char_file_u), 1, fl))) {
	perror("fwrite in save_char()");
	exit(1);
    }

    /*** Do variable length items here ***/

    /* Affects */
    for(af=ch->affected;af;af=af->next) {
        affect_to_store(af,&af_elem);
        result = fwrite(&af_elem,sizeof(struct affect_u),1,fl);
    }

    /* Organizational information */
    for(org=ch->orgs;org;org=org->next) {
        org_to_store(org,&org_elem);
        result = fwrite(&org_elem,sizeof(struct org_u),1,fl);
    }

    /* Skills */
    for(sk=ch->skills;sk;sk=sk->next) {
        skill_to_store(sk,&skill_elem);
        result = fwrite(&skill_elem,sizeof(struct skill_u),1,fl);
    }

    /* Physical */
    for(ph=ch->physical;ph;ph=ph->next) {
        phys_to_store(ph,&phys_elem);
        result = fwrite(&phys_elem,sizeof(struct phys_u),1,fl);
    }

    /* Player information */
    if(!(ch->specials.act & ACT_ISNPC)) {
        result = fwrite(ch->prefs,sizeof(struct player_prefs),1,fl);
    }

    /* Now save the objects */

    for(obj=ch->carrying;obj;obj=obj->next_content)
	save_obj(obj, fl);

    fclose(fl);
}


/*** Structure translations (store_to_* && *_store) ***/

void store_to_org(struct org_u *st, struct char_org_data *org)
{
    int i;

    org->org_id = st->org_id;
    org->member_level = st->member_level;
    org->permissions = st->permissions;
    org->participation = st->participation;
    org->experience = st->experience;
    for(i=0;i<MAX_DOMAINS;i++)
        org->domains[i] = st->domains[i];
}

void org_to_store(struct char_org_data *org, struct org_u *st)
{
    int i;

    st->org_id = org->org_id;
    st->member_level = org->member_level;
    st->permissions = org->permissions;
    st->participation = org->participation;
    st->experience = org->experience;
    for(i=0;i<MAX_DOMAINS;i++)
        st->domains[i] = org->domains[i];
}

void store_to_skill(struct skill_u *st, struct char_skill_data *sk)
{
    sk->skill_num = st->skill_num;
    sk->learned = st->learned;
    sk->period = st->period;
    sk->period_type = st->period_type;
    sk->last = st->last;
}

void store_to_phys(struct phys_u *st, struct char_phys_data *ph)
{
    memcpy(ph,st,sizeof(struct phys_u));
}

void skill_to_store(struct char_skill_data *sk, struct skill_u *st)
{
    st->skill_num = sk->skill_num;
    st->learned = sk->learned;
    st->period = sk->period;
    st->period_type = sk->period_type;
    st->last = sk->last;
}

void phys_to_store(struct char_phys_data *ph, struct phys_u *st)
{
    memcpy(st,ph,sizeof(struct phys_u));
}

void store_to_affect(struct affect_u *st, struct affected_type *af)
{
    af->type = st->type;
    af->duration = st->duration;
    af->modifier = st->modifier;
    af->location = st->location;
    af->bitvector = st->bitvector;
}

void store_to_obj_material(struct obj_material_u *st, struct obj_material *om)
{
    om->type = st->type;
    om->subtype = st->subtype;
    om->purity = st->purity;
    om->weight = st->weight;
}

void obj_material_to_store(struct obj_material *om, struct obj_material_u *st)
{
    st->type = om->type;
    st->subtype = om->subtype;
    st->purity = om->purity;
    st->weight = om->weight;
}

void store_to_obj_affect(struct obj_affect_u *st, struct obj_affect *oa)
{
    oa->location = st->location;
    oa->modifier = st->modifier;
}

void obj_affect_to_store(struct obj_affect *oa, struct obj_affect_u *st)
{
    st->location = oa->location;
    st->modifier = oa->modifier;
}

void affect_to_store(struct affected_type *af, struct affect_u *st)
{
    st->type = af->type;
    st->duration = af->duration;
    st->modifier = af->modifier;
    st->location = af->location;
    st->bitvector = af->bitvector;
}

void store_to_obj(struct obj_data *obj, struct obj_file_elem *object)
{
    obj->obj_flags.extra_flags = object->extra_flags;
    obj->obj_flags.weight      = object->weight;
    obj->obj_flags.bitvector   = object->bitvector;

    if(object->load_code==LOAD_TO_EQ)
        obj->equipped_as=object->load_to;
    else if(object->load_code==LOAD_TO_ROOM)
        obj->in_room=object->load_to;
}

void obj_to_store(struct obj_data *obj, struct obj_file_elem *object)
{
    struct obj_data *j;
    struct obj_affect *oa;
    struct obj_material *om;
    struct obj_info *oi;
    int i;

    object->item_number = obj_index[obj->item_number].virtual;
    object->extra_flags = obj->obj_flags.extra_flags;
    object->weight  = obj->obj_flags.weight;
    object->bitvector  = obj->obj_flags.bitvector;

    if(obj->equipped_as!=UNEQUIPPED) {
        object->load_code = LOAD_TO_EQ;
        object->load_to = obj->equipped_as;
    } else if(obj->in_room != NOWHERE) {
        object->load_code = LOAD_TO_ROOM;
        object->load_to = obj->in_room;
    } else if(obj->carried_by) {
        object->load_code = LOAD_TO_INV;
    } else if(obj->in_obj) {
        object->load_code = LOAD_TO_OBJ;
    }

    i=0;
    for(j=obj->contains;j;j=j->next_content)
        i++;
    object->num_contains = i;

    i=0;
    for(om=obj->material;om;om=om->next)
        i++;
    object->num_materials = i;

    i=0;
    for(oa=obj->affected;oa;oa=oa->next)
        i++;
    object->num_affects = i;

    i=0;
    for(oi=obj->info;oi;oi=oi->next)
        i++;
    object->num_info = i;
}


/* copy data from the file structure to a char struct */	
void store_to_char(struct char_file_u *st, struct char_data *ch)
{
    int i;

    ch->id = st->id;
    GET_SEX(ch) = st->sex;
    /*GET_CLASS(ch) = st->class;
    GET_LEVEL(ch) = st->level;*/

    ch->player.short_descr = 0;
    ch->player.long_descr = 0;

    if (*st->title)
    {
	CREATE(ch->player.title, char, strlen(st->title) + 1);
	strcpy(ch->player.title, st->title);
    }
    else
	GET_TITLE(ch) = 0;

    if (*st->description)
    {
	CREATE(ch->player.description, char, 
	    strlen(st->description) + 1);
	strcpy(ch->player.description, st->description);
    }
    else
	ch->player.description = 0;

    ch->player.hometown = st->hometown;

    ch->player.time.birth = st->birth;
    ch->player.time.played = st->played;
    ch->player.time.logon  = time(0);

    for (i = 0; i <= MAX_TOUNGE - 1; i++)
	ch->player.talks[i] = st->talks[i];

    GET_WEIGHT(ch) = st->weight;
    GET_HEIGHT(ch) = st->height;

    ch->points = st->points;
    if (ch->physical->max_mana < 100) {
        ch->physical->max_mana = 100;
    } /* if */

    ch->specials.alignment    = st->alignment;
    ch->specials.position     = st->position;

    ch->specials.act          = st->act;
    ch->specials.carry_weight = 0;
    ch->specials.carry_items  = 0;
    ch->physical->armor          = 100;
    ch->points.hitroll        = 0;
    ch->points.damroll        = 0;

    CREATE(GET_NAME(ch), char, strlen(st->name) +1);
    strcpy(GET_NAME(ch), st->name);

    /* Not used as far as I can see (Michael) */
    for(i = 0; i <= 4; i++)
     ch->specials.apply_saving_throw[i] = st->apply_saving_throw[i];

    ch->in_room = real_room(st->load_room);
    affect_total(ch);
} /* store_to_char */

/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data *ch, struct char_file_u *st)
{
    int i;
    struct affected_type *af;
    struct char_skill_data *sk;
    struct char_org_data *org;
    struct char_phys_data *ph;
    struct obj_data *obj;

    /* Unaffect everything a character can be affected by */
/*
    for(j=ch->carrying;j;j=j->next_content) {
	/ * save in case we erase in unequip_char * /
	i = ch->equipment->equipped_as;
	temp = unequip_char(ch, ch->equipment);
	temp->equipped_as = i;
	temp->next_content = char_eq;
	char_eq = temp;
    }*/

    ch->tmpabilities = ch->physical->abilities;

    st->birth      = ch->player.time.birth;
    st->played     = ch->player.time.played;
    st->played    += (long) (time(0) - ch->player.time.logon);
    st->last_logon = time(0);

    st->load_room = world[ch->in_room].number;

    st->id = ch->id;
    st->hometown = ch->player.hometown;
    st->weight   = GET_WEIGHT(ch);
    st->height   = GET_HEIGHT(ch);
    st->sex      = GET_SEX(ch);
    st->points    = ch->points;
    st->alignment       = ch->specials.alignment;
    st->position        = ch->specials.position;
    st->act             = ch->specials.act;

    st->points.hitroll =  0;
    st->points.damroll =  0;

    if (GET_TITLE(ch))
	strcpy(st->title, GET_TITLE(ch));
    else
	*st->title = '\0';

    if (ch->player.description)
	strcpy(st->description, ch->player.description);
    else
	*st->description = '\0';


    for (i = 0; i < MAX_TOUNGE; i++)
	st->talks[i] = ch->player.talks[i];

    strcpy(st->name, GET_NAME(ch) );

    for(i = 0; i <= 4; i++)
     st->apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

    for(af = ch->affected,i = 0; af; af=af->next) {
        i++;
    }
    st->num_affect = i;

    for(sk = ch->skills,i = 0; sk; sk=sk->next) {
        i++;
    }
    st->num_skill = i;

    for(org = ch->orgs,i = 0; org; org=org->next) {
        i++;
    }
    st->num_org = i;

    for(ph = ch->physical,i = 0; ph; ph=ph->next) {
        i++;
    }
    st->num_phys = i;

    for(obj = ch->carrying,i = 0; obj; obj=obj->next_content) {
        i++;
    }
    st->num_item = i;

    affect_total(ch);
} /* Char to store */

/* String routines */

void save_string(char *str,FILE *fl)
{
	size_t size;

	if(str)
		size = strlen(str);
	else
		size = 0;

	if(fwrite(&size,sizeof(size),1,fl)!=1) {
		log("BUG: Can't write in save_string");
		return;
	}

	if(!size)
		return;

	if(fwrite(str,sizeof(char),size,fl)!=size) {
		log("BUG: Can't write in save_string");
		return;
	}
}

char *load_string(FILE *fl)
{
	char buf[MAX_STRING_LENGTH];
	size_t size;

	if(fread(&size,sizeof(size),1,fl)!=1) {
		log("BUG: Can't read in load_string");
		return NULL;
	}

	if(!size)
		return NULL;

	if(fread(buf,sizeof(char),size,fl)!=1) {
		log("BUG: Can't read in load_string");
		return NULL;
	}

	buf[size]='\0';

	return (char *)strdup(buf);
}

/**** In cases where we don't care anymore... ****/

void delete_char(struct char_data *ch)
{
    char buf[100];
    int ret;

    sprintf(buf,"save/%s",GET_NAME(ch));
    ret = unlink(buf);
}
