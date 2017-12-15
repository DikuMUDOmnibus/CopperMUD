

#ifndef PLAYER_H
#define PLAYER_H

/* Constructs for player preferences, etc. */

/* Flags */
#define PLR_MSGECHO	1
#define PLR_BRIEF	2
#define PLR_COMPACT	4
#define PLR_GUEST	8
#define PLR_DEAD	16
#define PLR_STATUS      32
#define PLR_SECURE      64

#define DISCON_HUNGER   0
#define DISCON_THIRST   1
#define DISCON_ATTACK   2
#define DISCON_NOISE    3

#define MAX_DISCON_CMDS 5

struct player_prefs {

    int flags;

    /* How do we behave when we disconnect? */
    char discon[MAX_DISCON_CMDS][MAX_INPUT_LENGTH];

    char pwd[12];  /* encrypted password */
    char user[12]; /* what is username on normal login machine */
    /* ip or name of host??? */
};

#endif /* !defined(PLAYER_H) */
