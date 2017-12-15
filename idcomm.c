
/* ************************************************************************
*  file: idcomm.c , Communication module.                 Part of Copper3 *
*  Usage: Communication, central game loop.                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
*  Using *any* part of DikuMud without having read license.doc is         *
*  violating our copyright.
************************************************************************* */

/* This is very much comm.c in new clothes... The idea is to multiplex
   on connections going to ident servers, so that slow/bogus servers
   can't keep new people from logging in. */

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
#include <sys/timeb.h>
#include <fcntl.h>
#include <signal.h>

#include "id.h"

#ifndef sigmask
#define dismask(m)	(1 << ((m) - 1))
#endif

/* For setrlimit */
#if CONFIG_SYSV
#include <sys/resource.h>
#endif

#define MAX_NAME_LENGTH 15
#define MAX_HOSTNAME   256
#define OPT_USEC 250000       /* time delay corresponding to 4 passes/sec */



int maxdesc, avail_descs;

int init_socket(int port);
void close_sockets(int s);
void close_socket(struct descriptor_data *d);
struct timeval timediff(struct timeval *a, struct timeval *b);
void nonblock(int s);


/* Init sockets, handle requests, and cleanup sockets */
int ?????(int port)
{
    int s; 

    int retval;
    fd_set input_set, output_set, exc_set;
    struct timeval last_time, now, timespent, timeout, null_time;
    static struct timeval opt_time;
    char *comm;
    char buf[100];
#if CONFIG_SYSV
    struct rlimit limit = {100,105};
#endif


    null_time.tv_sec = 0;
    null_time.tv_usec = 0;

    opt_time.tv_usec = OPT_USEC;  /* Init time values */
    opt_time.tv_sec = 0;
    gettimeofday(&last_time, (struct timeval *) 0);

    maxdesc = s;

#if CONFIG_BSD
#if HAVe_SeTDTABLeSIZe
    retval = setdtablesize(100);
    if (retval == -1)
	log("unable to set table size");
    else {
	sprintf(buf, "%s %d\n", "dtablesize set to: ", retval);
	log(buf);
    }
#endif

    avail_descs = getdtablesize() - 8;  /*  !! Change if more needed !! */
#endif

#if CONFIG_SYSV

        retval = setrlimit(RLIMIT_NOFILE,&limit);
        if (retval == -1)
                log("unable to set resource limit");
        else {
                sprintf(buf, "%s %d\n", "rlimit size set to: ", retval);
                log(buf);
        }

        getrlimit(RLIMIT_NOFILE,&limit);
        avail_descs = limit.rlim_cur - 9;  /*  !! Change if more needed !! */
#endif


    sprintf(buf,"avail_descs set to: %d",avail_descs);
    log(buf);

    mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
	sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
	sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP);

    /* Main loop */
    while (!shutdown)
    {
	/* Check what's happening out there */
	FD_ZERO(&input_set);
	FD_ZERO(&output_set);
	FD_ZERO(&exc_set);
/*	FD_SET(s, &input_set); (incoming desc? */
	FD_SET(0, &input_set); /* check for commands */
	for (point = descriptor_list; point; point = point->next)
	{
	    FD_SET(point->descriptor, &input_set);
	    FD_SET(point->descriptor, &exc_set);
	    FD_SET(point->descriptor, &output_set);
	}

	/* check out the time */
	gettimeofday(&now, (struct timeval *) 0);
	timespent = timediff(&now, &last_time);
	timeout = timediff(&opt_time, &timespent);
	last_time.tv_sec = now.tv_sec + timeout.tv_sec;
	last_time.tv_usec = now.tv_usec + timeout.tv_usec;
	if (last_time.tv_usec >= 1000000)
	{
	    last_time.tv_usec -= 1000000;
	    last_time.tv_sec++;
	}

	sigsetmask(mask);

	if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) 
	    < 0)
	{
	    perror("Select poll");
	    return;
	}

	if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0)
	{
	    perror("Select sleep");
	    exit(1);
	}

	sigsetmask(0);

	/* Respond to whatever might be happening */
	if (FD_ISSET(0, &input_set))
            read_new_req();
	for (point = descriptor_list; point; point = next_point) 
	{
	    next_point = point->next;
	    if (FD_ISSET(point->descriptor, &input_set)) {
    		if (process_input(point) < 0) 
		    close_socket(point);
	    }
	}

	/* kick out the freaky folks */
	for (point = descriptor_list; point; point = next_point)
	{
	    next_point = point->next;   
	    if (FD_ISSET(point->descriptor, &exc_set))
	    {
		FD_CLR(point->descriptor, &input_set);
		FD_CLR(point->descriptor, &output_set);
		close_socket(point);
	    }
	}
    }
}


/* ******************************************************************
*  general utility stuff (for local use)                            *
****************************************************************** */




struct timeval timediff(struct timeval *a, struct timeval *b)
{
    struct timeval rslt, tmp;

    tmp = *a;

    if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0)
    {
	rslt.tv_usec += 1000000;
	--(tmp.tv_sec);
    }
    if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0)
    {
	rslt.tv_usec = 0;
	rslt.tv_sec =0;
    }
    return(rslt);
}









/* ******************************************************************
*  socket handling							 *
****************************************************************** */




int init_socket(int port)
{
    int s;
    char *opt;
    char hostname[MAX_HOSTNAME+1];
    struct sockaddr_in sa;
    struct hostent *hp;
    struct linger ld;

    bzero(&sa, sizeof(struct sockaddr_in));
    gethostname(hostname, MAX_HOSTNAME);
    hp = gethostbyname(hostname);
    if (hp == NULL)
    {
	perror("gethostbyname");
	exit(1);
    }
    sa.sin_family = hp->h_addrtype;
    sa.sin_port	= htons(port);
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) 
    {
	perror("Init-socket");
	exit(1);
	}
    if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR,
	(char *) &opt, sizeof (opt)) < 0) 
    {
	perror ("setsockopt REUSEADDR");
	exit (1);
    }

    ld.l_onoff = 0;
    ld.l_linger = 0;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0)
    {
	perror("setsockopt LINGER");
	exit(1);
    }
    if (bind(s, &sa, sizeof(sa), 0) < 0)
    {
	perror("bind");
	close(s);
	exit(1);
    }
    listen(s, 3);
    return(s);
}



int write_to_descriptor(int desc, char *txt)
{
    int sofar, thisround, total;

    total = strlen(txt);
    sofar = 0;

    do
    {
	thisround = write(desc, txt + sofar, total - sofar);
	if (thisround < 0)
	{
	    perror("Write to socket");
	    return(-1);
	}
	sofar += thisround;
    } 
    while (sofar < total);

    return(0);
}





int process_input(struct descriptor_data *t)
{
    int sofar, thisround, begin, squelch, i, k, flag;
    char tmp[MAX_INPUT_LENGTH+2], buffer[MAX_INPUT_LENGTH + 60];

    sofar = 0;
    flag = 0;
    begin = strlen(t->buf);

    /* Read in some stuff */
    do
    {
	if ((thisround = read(t->descriptor, t->buf + begin + sofar, 
	    MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0)
	    sofar += thisround;		
	else
	    if (thisround < 0)
		if(errno != EWOULDBLOCK)
					{
		    perror("Read1 - ERROR");
		    return(-1);
		}
		else
		    break;
	    else
	    {
		log("EOF encountered on socket read.");
		return(-1);
	    }
    }
    while (!ISNEWL(*(t->buf + begin + sofar - 1)));	

    *(t->buf + begin + sofar) = 0;

    /* if no newline is contained in input, return without proc'ing */
    for (i = begin; !ISNEWL(*(t->buf + i)); i++)
	if (!*(t->buf + i))
	    return(0);

    /* input contains 1 or more newlines; process the stuff */
    
    return(1);
}




void close_sockets(int s)
{
    log("Closing all sockets.");

    while (descriptor_list)
	close_socket(descriptor_list);

    close(s);
}





void close_socket(struct descriptor_data *d)
{
    struct descriptor_data *tmp;
    char buf[100];

    close(d->descriptor);
    flush_queues(d);
    if (d->descriptor == maxdesc)
	--maxdesc;



    if (next_to_process == d)		/* to avoid crashing the process loop */
	next_to_process = next_to_process->next;   

    if (d == descriptor_list) /* this is the head of the list */
	descriptor_list = descriptor_list->next;
    else  /* This is somewhere inside the list */
    {
	/* Locate the previous element */
	for (tmp = descriptor_list; (tmp->next != d) && tmp; 
	    tmp = tmp->next);
	
	tmp->next = d->next;
    }
    if (d->showstr_head)
	free(d->showstr_head);
    free(d);
}





void nonblock(int s)
{
#if CONFIG_SYSV
    if (fcntl(s, F_SETFL, O_NONBLOCK) == -1)
#else
    if (fcntl(s, F_SETFL, FNDELAY) == -1)
#endif
    {
	perror("Noblock");
	exit(1);
    }
}

