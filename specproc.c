
/* ************************************************************************
*  file: specproc.c      Special module.                  Part of DIKUMUD *
*  Usage: Procedures that are just so special.                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */


#include "structs.h"
#include "db.h"
#include "error.h"
#include "proto.h"

extern struct room_data *world;
extern struct mob_index_data *mob_index;
extern struct obj_index_data *obj_index;
extern int top_of_world;
extern struct obj_data *object_list;
extern char log_buf[];

/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
   /* mob_index[real_mobile(12502)].func = waiter; */
}



/* assign special procedures to objects */
void assign_objects(void)
{
    int board(struct obj_data *obj,struct char_data *ch, int cmd, char *arg);
}



/* assign special procedures to rooms */
void assign_rooms(void)
{
/*    int chalice(int room, struct char_data *ch, int cmd, char *arg);

    world[real_room(14088)].funct = barrier_click;*/
}


/* ********************************************************************
*  Handling funcs                                                     *
******************************************************************** */

void room_procs()
{
    int i;

    for(i=0;i<top_of_world;i++)
	if(world[i].funct)
	    (*world[i].funct)(i,NULL,0,NULL);
}

void item_procs()
{
    struct obj_data *i;

    for(i=object_list;i;i=i->next)
	if(i->item_number >=0 && obj_index[i->item_number].func)
	    (*obj_index[i->item_number].func)(i,NULL,0,NULL);
}
