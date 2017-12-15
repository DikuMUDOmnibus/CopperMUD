
#include <errno.h>


#include <ctype.h>

#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>


#include CONFIG

#if HAVE_STRINGS_H

#include <strings.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "time.h"
#include "player.h"
#include "proto.h"

extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;

/* called once per game_loop */

void loop_debug()
{
	FILE *fl;
	struct descriptor_data *d;

	fl=fopen("loop.debug","w");
	
	for(d=descriptor_list;d;d=d->next) {
		if(d->character)
			fprintf(fl,"%s m[%d] r[%d] v[%d]\n",GET_NAME(d->character),d->connected,d->character->in_room,world[d->character->in_room].number);
		else
			fprintf(fl,"[No name] m[%d]\n",d->connected);
	}
	fclose(fl);
}

void hour_debug()
{
}

extern FILE *cmdfile;


void cmdlog(char *str)
{
        static logcount = 0;

        if(cmdfile) {
                if(!(++logcount % 50)) {
                        rewind(cmdfile);
                }
                fprintf(cmdfile,"[%d] %s\n",logcount,str);
                fflush(cmdfile);
        }
}

