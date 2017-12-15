/* Make a header to this sometime... */

/* Introduction: This is what the mechanisms of cron look like * * * * * * * *
*
*           Scripts                      Crons
*          -----------               ---------------------------------
*           |   |   |                      |
*          Sc1  |   |         +--------- Cron1 ---> Vars, Actors, Stack
* Quantums q1  Sc2  |         |            |
*          q2  q1  Sc3        |  +------ Cron2 ---> ...
*          q3  q2  q1<--------+  |         |
*          q4  q3  q2<------- | -+ +---- Cron3
*          q5<-q4--q3-------- | ---+       |
*          q6  q5  q4         +--------- Cron4
*
* Scripts and quantums are static - a cron script could be compared with
* the script of a play. The diku world can be considered a theatre (perhaps
* a multiplex seven-screen, er, stage one) with many dramas, comedies, and
* such going on at any time (these are crons). A play can be played by
* different actors, and perhaps simultaneously on different stages (at the
* same line (quantum) of the script or not).
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef CRON_H
#define CRON_H

#define CRON_FILE "world.cron"

/* Boolean comparatorrs */
#define BOOL_EQUAL           0
#define BOOL_NOT_EQUAL       1
#define BOOL_GREATER         2
#define BOOL_LESS            3
#define BOOL_EQUAL_GREATER   4
#define BOOL_EQUAL_LESS      5
#define BOOL_TRUE            6
#define BOOL_FALSE           7

/* Expressions */

#define EXP_CONSTANT 0
#define EXP_OPERATOR 1
#define EXP_FUNCTION 2
#define EXP_VARIABLE 3

#define OP_PLUS      0
#define OP_MINUS     1
#define OP_MULT      2
#define OP_DIVIDE    3
#define OP_MODULUS   4
#define OP_XXXX      5

#define FUNC_ROOM_OF 1
#define FUNC_ZONE_OF 2
#define FUNC_XXX     3

struct exp {
    int type;
    int data;
    char *name;
    struct exp *left;
    struct exp *right;
};

/* Quantum flags */
#define QF_WAIT_REGARDLESS 1    /* If no success, keep trying */

/* Quantum commands */
#define QC_NOP                0
#define QC_VAR                1
#define QC_LOG                2
#define QC_ACTOR_COMMAND      3
#define QC_SET_DOOR           4
#define QC_LOAD               5 /* Do I really want these, or LOAD_MOB/OBJ? */
#define QC_ECHO               6
#define QC_MESSAGE            7
#define QC_SPEED              8
#define QC_RESTART            9
#define QC_RUN               10
#define QC_SET               11
#define QC_IF                12
#define QC_ENDIF             13
#define QC_WHILE             14
#define QC_ENDWHILE          15
#define QC_WAIT              16

#define LOAD_MOB              0
#define LOAD_OBJ              1

#define ECHO_ROOM             0
#define ECHO_ZONE             1

#define SPEED_FAST            0
#define SPEED_MEDIUM          1
#define SPEED_SLOW            2

struct cron_quantum {
    short int flags;
    int cmd;
    int data;
    struct exp *exp1,*exp2;
    char *name;
    char *arg;
char *debug;
	
    struct cron_quantum *next;
};

struct script {
    char *name;
    int number;
    struct cron_quantum *first,*last;
    struct script *next_script;
};

/* cron_var holds the current value of a cron variable */

#define VAR_ACTOR  1
#define VAR_OBJ    2
#define VAR_NUM    3
/* VAR_STRING?? */

struct cron_var {
    char type;
    char num;
    char *name;
    long int value;
    struct cron_var *next;
};

/* stack_unit holds information about the state of the process/interpreter */

#define STACK_CALL       0
#define STACK_IF_LOOP    1
#define STACK_WHILE_LOOP 2

struct stack_unit {
    char type;
    struct cron_quantum *ref;
    struct stack_unit *next;
};

/* cron is basically the "working unit" for each instance of "happenings" */

/* cron modes */ 
#define CRON_WAIT      0
#define CRON_GO        1
#define CRON_SUSPENDED 5
#define CRON_WACKY     10  /* if something weird happens */

/* Crons will be doubly linked for speed */
struct cron {
    struct cron *Succ,*Pred;
    short int mode;
    signed char pri;           /* not sure if this is useful */
    char timer;
    struct cron_var *vars;
    struct stack_unit *stack;
    struct script *script;
    struct cron_quantum *executing;
    struct cron *subsuming;    /* When we were called from another cron */
};                                 /*     ^^^^ does this make sense ^^^^    */

struct cron_list {
    struct cron *Head;
    struct cron *Tail;
    struct cron *TailPred;
};

/* Keep track of everything there is */

#define CRON_MEDIUM_TICKS 5
#define CRON_SLOW_TICKS  15
struct cronbase {
    struct cron_list fast_crons,
	    medium_crons,
	    slow_crons;
    struct script *scripts;
};

#endif /* !defined(CRON_H) */
