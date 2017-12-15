/* ************************************************************************
*  file: signals.c , trapping of signals from Unix.       Part of DIKUMUD *
*  Usage : Signal Trapping.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <signal.h>

#include <sys/time.h>

#include "structs.h"
#include "interp.h"
#include "utils.h"
#include "comm.h"
#include "proto.h"

extern int process_output(struct descriptor_data *t);

/* compiler complaints here - is there a good portable solution? */
void checkpointing(int dummy);
void shutdown_request(int dummy);
void logsig(int dummy);
void hupsig(int dummy);
void shutdownsave(void);

void signal_setup(void)
{
    struct itimerval itime;
    struct timeval interval;

    signal(SIGUSR2, shutdown_request);

    /* just to be on the safe side: */

    signal(SIGHUP, hupsig);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, hupsig);
    signal(SIGALRM, logsig);
    signal(SIGTERM, hupsig);

    /* set up the deadlock-protection */

    interval.tv_sec = 900;    /* 15 minutes */
    interval.tv_usec = 0;
    itime.it_interval = interval;
    itime.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &itime, 0);
    signal(SIGVTALRM, checkpointing);
}



void checkpointing(int dummy)
{
    extern int tics;
    
    if (!tics)
    {
	log("CHECKPOINT shutdown: tics not updated");
	abort();
    }
    else
	tics = 0;
}




void shutdown_request(int dummy)
{
    extern int shutdown;

    log("Received USR2 - shutdown request");
    shutdown = 1;
}

/* kick out players etc */
void hupsig(int dummy)
{
    log("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
    shutdownsave();
    exit(0);   /* something more elegant should perhaps be substituted */
}

/* Save all players before shutting down */
void shutdownsave(void)
{
    extern struct descriptor_data *descriptor_list;
    struct descriptor_data *i;
    struct char_data *vict;
    static char *buf=
    "\n\rThe Copper SAVE Daemon has forced you to 'save.' Good-bye!\n\r";

    for(i=descriptor_list; i; i=i->next){
	if(!i->connected){
	    vict=i->character;
	    write_to_descriptor(i->descriptor, buf);
	    do_save(vict, "", 0);
	}
    }
    /*
    for(c=character_list;c;c=c->next)
        save_char(c);
    */
    log("SAVE Daemon invoked to save all players.");
    save_world_obj(SO_START);
    save_world_obj(SO_SAVEALL);
    save_world_obj(SO_DONE);
    update_time();
}

void logsig(int dummy)
{
    log("Signal received. Ignoring.");
}
