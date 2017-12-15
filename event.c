


#include "structs.h"
#include "db.h"
#include "utils.h"
#include "event.h"
#include "proto.h"

extern struct room_data *world;
extern struct zone_data *zone_table;
extern char log_buf[];

/* some defines for local use */
#define METHOD_ROOM	1
#define METHOD_NEARBY	2
#define METHOD_ZONE	3

struct event *spare_events=NULL;

void add_event_char(struct char_data *ch,int type,int data,
                    CHAR_ID from,CHAR_ID to)
{
    struct event *new,*j;

    /* can't act on events you have no knowledge of */
    if(!AWAKE(ch) && type!=EVENT_NOISE)
        return;

    if(!spare_events)
        CREATE(new,struct event,1);
    else {
        new = spare_events;
        spare_events = spare_events->next;
    }

    new->next=NULL;
    new->type=type;
    new->data=data;
    new->initiator=from;
    new->receiver=to;
    if(!ch->events)
        ch->events=new;
    else {
        j=ch->events;
        while(j->next)
            j=j->next;
        j->next=new;
    }
}

void add_event_zone(int zon,int type,int data,CHAR_ID from, CHAR_ID to)
{
    int i;

    for(i=zone_table[zon].real_bottom;i<=zone_table[zon].real_top;i++)
        add_event_room(i,type,data,from,to);
}

void add_event_room(int loc,int type,int data,CHAR_ID from, CHAR_ID to)
{
    struct char_data *i;

    for(i=world[loc].people;i;i=i->next_in_room) {
        add_event_char(i,type,data,from,to);
    }
}

void add_event(int loc,int type,int data,CHAR_ID from,CHAR_ID to)
{
    int method;

    switch(type) {
        case EVENT_NOISE:
            method = METHOD_NEARBY;
            break;
        default:
            method = METHOD_ROOM;
            break;
    }

    switch(method) {
        case METHOD_ZONE:
            add_event_zone(world[loc].zone,type,data,from,to);
            break;
        case METHOD_NEARBY:
            /* archive bit would be good */
            break;
        case METHOD_ROOM:
            add_event_room(loc,type,data,from,to);
            break;
        default:
            break;
    }
}

void free_event(struct event *e)
{
    e->next = spare_events;
    spare_events = e;
}
