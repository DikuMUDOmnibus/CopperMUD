/*
** An ident server query daemon, built off of the pidentd test1.c
** program by Peter Eriksson <pen@lysator.liu.se>
**
** Last modified: 5 Feb 1994
**
** Rationale: Sometimes people, particularly in the context of muds,
** hide behind veils of anonymity, as they do things which they
** might not do in other circumstances. This addition should make
** it easier to know which user at a machine is logging into your
** mud, and you can then provide more information to whatever
** administrators you come across in your search for justice. It
** is better that the few spoil things for the few, rather than the
** many.
**
** This runs as a separate program because it opens many descriptors
** (multiplexes queries), and most muds are short on descriptors
** anyway. It also makes it harder for people to mess up your mud by
** creating "strange" ident servers on their own machines.
**
** Be aware that this program is only as good as the servers it connects
** to. Somone who is root can make it look like there are many
** people online, when there's only one.
*/


#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>

#define IDNTPORT 113

int Toupper(c)
  int c;
{
  if (islower(c))
    return toupper(c);
  else
    return c;
}


/*
** Compare two strings, case insensitive
*/
int Stricmp(s1, s2)
  char *s1, *s2;
{
  int diff;


  while (!(diff = Toupper(*s1) - Toupper(*s2)) && *s1)
    s1++, s2++;

  return diff;
}



main(argc,argv)
  int argc;
  char *argv[];
{
  int fd;
  struct sockaddr_in addr;
  int addrlen;
  int port;
  FILE *fp_in, *fp_out;
  int lport, fport;
  char buffer[8192];
  char reply_type[81];
  char opsys_or_error[81];
  char identifier[1024];
  
  
    /* Assuming:
     *  stdin - read from calling program
     *  stdout - write to calling program
     *
     * Calling program has responsibility to create descriptors and
     * dup2() them between fork() and exec()
     */

  
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1)
    perror("socket");

  addr.sin_family = AF_INET;
  if(argc > 1)
    addr.sin_addr.s_addr = inet_addr(argv[1]);
  else
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  addr.sin_port = htons(...);
  addrlen = sizeof(addr);

  if (connect(fd, &addr, addrlen) == -1)
    perror("connect");

  addrlen = sizeof(addr);
  if (getsockname(fd, &addr, &addrlen) == -1)
    perror("getsockname");

}

void send_query()
{
  fp_in  = fdopen(fd, "r");
  fp_out = fdopen(fd, "w");
  if (!fp_in || !fp_out)
    perror("fdopen");

  fprintf(fp_out, "%d , %d\n", other_port, mud_port);
  fflush(fp_out);
}

void get_answer(struct descriptor *d)
{
  shutdown(fd, 1);

  argc = sscanf(d->, "%d , %d : %[^ \t\n\r:] : %[^ \t\n\r:] : %[^\n\r]",
		&lport, &fport, reply_type, opsys_or_error, identifier);
  if (argc < 3)
  {
    fprintf(stderr, "fscanf: too few arguments (%d)\n", argc);
    return;
  }
  if (Stricmp(reply_type, "ERROR") == 0)
  {
    printf("Ident error: error code: %s\n", opsys_or_error);
    exit(1);
  }
  else if (Stricmp(reply_type, "USERID") != 0)
  {
    printf("Ident error: illegal reply type: %s\n", reply_type);
    exit(1);
  }
  else
    printf("Ident returned: Opsys=%s, Identifier: %s\n",
	   opsys_or_error, identifier);

  fclose(fp_out);
  fclose(fp_in);

} /* for(;;) */
}
