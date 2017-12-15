#ifndef PROTO_H
#define PROTO_H

/* File for prototypes of all functions */

/* standard includes to keep things simple... */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/* act.adm.c */
bool avail_to(struct char_data *ch);

/* act.move.c */
int leave_by_exit(struct char_data *ch,int cmd);
int can_enter_room(struct char_data *ch,int room, bool show_msg);
int can_go(struct char_data *ch,int dir);
int find_door(struct char_data *ch, char *type, char *dir);
int has_key(struct char_data *ch, int key);
struct obj_info_exit *get_exit(int room, int dir);

/* comm.c */
int run_the_game(int port);
#ifdef COMM_H
char *get_from_q(struct txt_q *queue);
void write_to_q(char *txt, struct txt_q *queue);
#endif
struct timeval timediff(struct timeval *a, struct timeval *b);
void flush_queues(struct descriptor_data *d);
int init_socket(int port);
int new_connection(int s);
int number_playing(void);
int new_descriptor(int s);
int process_output(struct descriptor_data *t);
int write_to_descriptor(int desc, char *txt);
int process_input(struct descriptor_data *t);
void close_sockets(int s);
void close_socket(struct descriptor_data *d);
void nonblock(int s);
void coma(int s);
bool silent(struct char_data *ch);
void send_to_char(char *messg, struct char_data *ch);
void send_to_all(char *messg);
void send_to_outdoor(char *messg);
void send_to_zone_outdoor(int zone,char *messg);
void send_to_except(char *messg, struct char_data *ch);
void send_to_room(char *messg, int room);
void send_to_room_except(char *messg, int room, struct char_data *ch);
void send_to_room_except_two(char *messg, int room, struct char_data *ch1, struct char_data *ch2);
void act(char *str, int hide_invisible, struct char_data *ch,
    struct obj_data *obj, void *vict_obj, int type);

/**** for act() ****/
#define TO_ROOM    0
#define TO_VICT    1
#define TO_NOTVICT 2
#define TO_CHAR    3


/* db.c */
/* void memset(char *s, char c, int n); in string.h?? */
void boot_db(void);
void reset_time(void);
void weather_setup(void);
void update_time(void);
void boot_world(void);
void allocate_room(int new_top);
void boot_zones(void);
struct char_data *read_mobile(int nr, int type);
struct obj_data *read_object(int nr, int type);
struct obj_data *new_object(void);
struct obj_info *new_obj_info(sh_int type,struct obj_data *obj);
void zone_update(void);
void reset_zone(int zone);
int is_empty(int zone_nr);
char *fread_string(FILE *fl);
void free_char(struct char_data *ch);
void free_obj(struct obj_data *obj);
int file_to_string(char *name, char *buf);
void reset_char(struct char_data *ch);
struct char_data *bare_char(void);
void clear_object(struct obj_data *obj);
void init_char(struct char_data *ch);
int real_room(int virtual);
int real_mobile(int virtual);
int real_object(int virtual);
struct char_data *load_bio_to(int nr,int type,int where);
struct char_data *read_bio(int nr,int type);
void roll_bio_attr(struct char_data *ch,int attr);
int find_bio(int number);
struct obj_info *get_obj_info(struct obj_data *o,int type);
int get_bio_attr(struct char_data *ch,int attr);
void start_player(struct char_data *ch);
int count_char(char c,FILE *fl);
struct bio_attr *find_bio_attr(int bio_type,int attr);

/* debug.c */
void loop_debug(void);
void hour_debug(void);
void cmdlog(char *str);

/* end.c */
void outrip(struct char_data *ch);

/* event.c */
void add_event_char(struct char_data *ch,int type,int data,
                    CHAR_ID from,CHAR_ID to);
void add_event_zone(int zon,int type,int data,CHAR_ID from, CHAR_ID to);
void add_event_room(int loc,int type,int data,CHAR_ID from, CHAR_ID to);
void add_event(int loc,int type,int data,CHAR_ID from,CHAR_ID to);
void free_event(struct event *e);

/* force.c */
int create_magic(struct generic *initiator,struct magic **m);
int bind_magic_to_object(struct magic *m,struct generic *target);
int attribute_to_magic(struct magic *m,int attribute,int value);
int query_attribute(struct magic *m,int attribute);
int power_over_object(struct magic *m,struct generic *target);
int expend_magic(struct magic *m,struct generic *target);
int magical_attack(struct magic *m,struct generic *target);
int remove_magic(struct magic *m);

/* handler.c */
char *fname(char *namelist);
int isname(char *str, char *namelist);
void affect_modify(struct char_data *ch,byte loc, byte mod, long bitv, bool add);
void affect_total(struct char_data *ch);
void affect_to_char( struct char_data *ch, struct affected_type *af );
void affect_remove( struct char_data *ch, struct affected_type *af );
void affect_from_char( struct char_data *ch, byte skill);
bool affected_by_spell( struct char_data *ch, byte skill );
void affect_join( struct char_data *ch, struct affected_type *af,
	 bool avg_dur, bool avg_mod )
;
void char_from_room(struct char_data *ch);
void char_to_room(struct char_data *ch, int room,int dir);
void obj_to_char(struct obj_data *object, struct char_data *ch);
void obj_from_char(struct obj_data *object);
int apply_ac(struct char_data *ch, struct obj_data *obj);
void equip_char(struct char_data *ch, struct obj_data *obj, int pos);
struct obj_data *unequip_char(struct char_data *ch, struct obj_data *obj);
int get_number(char **name);
struct obj_data *get_equip_used(struct char_data *ch,int use);
struct obj_data *get_obj_in_list(char *name, struct obj_data *list);
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list);
struct obj_data *get_obj_num(int nr);
struct char_data *get_char_from_id(CHAR_ID id);
struct char_data *get_char_room(char *name, int room);
struct char_data *get_char_num(int nr);
void obj_to_room(struct obj_data *object, int room);
void obj_from_room(struct obj_data *object);
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to);
void obj_from_obj(struct obj_data *obj);
void extract_obj(struct obj_data *obj);
void extract_char_eq(struct char_data *ch);
void extract_char(struct char_data *ch);
void remove_char(struct char_data *ch);
struct char_data *get_char_room_vis(struct char_data *ch, char *name);
struct char_data *get_char_vis(struct char_data *ch, char *name);
struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name,
		struct obj_data *list) ;
struct obj_data *get_obj_vis(struct char_data *ch, char *name);
struct obj_data *create_money( int amount );
int generic_find(char *arg, int bitvector, struct char_data *ch, void **tar);
/*** for generic_find ***/

#define FIND_CHAR_ROOM     1
#define FIND_CHAR_WORLD    2
#define FIND_OBJ_INV       4
#define FIND_OBJ_ROOM      8
#define FIND_OBJ_WORLD    16
#define FIND_DOOR         32
#define FIND_ROOM         64

/* interp.c */
int search_block(char *arg, char **list, bool exact);
int search_block_offset(char *arg, char **list, bool exact,int size,int offset);
int old_search_block(char *argument,int begin,int length,char **list,int mode);
int command_interpreter(struct char_data *ch, char *argument);
void argument_interpreter(char *argument,char *first_arg,char *second_arg );
int is_number(char *str);
char *one_argument(char *argument, char *first_arg );
int fill_word(char *argument);
int is_abbrev(char *arg1, char *arg2);
void half_chop(char *string, char *arg1, char *arg2);
int special(struct char_data *ch, int cmd, char *arg);

/* pulse.c */
void char_pulse(int pulse);
void item_pulse(void);
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6);
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);
int mana_gain(struct char_data *ch);
int hit_gain(struct char_data *ch);
int move_gain(struct char_data *ch);
void advance_level(struct char_data *ch);
void set_title(struct char_data *ch);
void gain_exp(struct char_data *ch, int gain);
void gain_exp_regardless(struct char_data *ch, int gain);
void gain_condition(struct char_data *ch,int condition,int value);
void check_idling( void );
void point_update( void );

/* magic.c */
void teleport_to(struct char_data *ch,int to_room);

/* mobcontrol.c */

/* modify.c */
void night_watchman(void);
void page_string(struct descriptor_data *d,char *str,int keep_internal);
void show_string(struct descriptor_data *d,char *input);
void check_reboot(void);
int workhours(void);
int load(void);
char *nogames(void);

/* nanny.c */
void nanny(struct descriptor_data *d, char *arg);
void boot_readers(void);

/* org.c */
void boot_orgs(void);
struct org_type *get_org_by_name(char *name);
struct org_type *get_org_by_id(char id);
struct char_org_data *get_char_org(struct char_data *ch,char id);
struct char_skill_data *get_skill(struct char_data *ch,int type);
struct char_skill_data *new_skill(struct char_data *ch,int type);
void start_org(struct char_data *member,struct org_type *org);
int get_skill_by_name(char *name);

/* phys.c */
void check_fall(struct char_data *ch);
bool circle_follow(struct char_data *ch, struct char_data *victim);
void stop_follower(struct char_data *ch);
void die_follower(struct char_data *ch);
void add_follower(struct char_data *ch, struct char_data *leader);
void off_light(struct obj_data *obj);
void on_light(struct obj_data *obj);
bool can_have_pos(struct char_data *ch,int pos);
int can_wear(struct obj_data *obj,int bit);

/* player.c */
int player_action(struct char_data *ch);

/* plot.c */
void plot_handler(void);
void gen_year_plot(void);
void gen_month_plot(void);
void boot_plot(void);

/* save.c */
struct char_data *load_char(char *name);
void save_char(struct char_data *ch);
void delete_char(struct char_data *ch);
void load_world_obj(void);
int save_world_obj(int save_obj_state);
/*** for save_world_obj() ***/
#define SO_START   1
#define SO_SAVE    2
#define SO_DONE    3
#define SO_SAVEALL 4

/* specproc.c */
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);

/* utility.c */
#if !HAVE_MAXMIN
int MIN(int a, int b);
int MAX(int a, int b);
#endif
int choose_bit(int mask);
int bitcard(int number);
char *rating(int number,int scale,int mode);
bool check_hide(struct char_data *hiding,struct char_data *seeing);
bool CAN_SEE(struct char_data *ch,struct char_data *vict);
bool CAN_SEE_OBJ(struct char_data *ch,struct obj_data *obj);
int number(int from, int to);
int dice(int number, int size);
int str_cmp(char *arg1, char *arg2);
int strn_cmp(char *arg1, char *arg2, int n);
void log(char *str);
void sprintbit(long vektor, char *names[], char *result);
void sprinttype(int type, char *names[], char *result);
struct time_info_data real_time_passed(time_t t2, time_t t1);
struct time_info_data mud_time_passed(time_t t2, time_t t1);
struct time_info_data age(struct char_data *ch);
int default_loc(int hometown);
int jail_hometown(int hometown);
void strToLower(char *s1);
char *stristr (char *s1, char *s2);

/* vio.c */
void hit(struct char_data *ch, struct char_data *victim, int type);
void damage(struct char_data *ch, struct char_data *victim,
      int dam, int attacktype);
void set_fighting(struct char_data *ch,struct char_data *vict);
void stop_fighting(struct char_data *ch);
int to_hit(struct char_data *ch,int vict_ac,int adj);
int to_hit_char(struct char_data *ch,struct char_data *victim,int adj);
void update_pos(struct char_data *victim);
void death_cry(struct char_data *ch);

/* weather.c */
void weather_and_time(int mode);
#ifdef DB_H
void calc_light_zone(struct zone_data *zone);
char get_season(struct zone_data *zone);
#endif

/* Borrowed from merc.h of Merc 2.0 */

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		( const char *key, const char *salt );
#endif

#if	defined(apollo)
int	atoi		( const char *string );
void *	calloc		( unsigned nelem, size_t size );
char *	crypt		( const char *key, const char *salt );
long	random		( void );
int	srandom		( int seed );
#endif

#if	defined(hpux)
char *	crypt		( const char *key, const char *salt );
#endif

#if	defined(linux)
char *	crypt		( const char *key, const char *salt );
#endif

#if	defined(macintosh)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(MIPS_OS)
char *	crypt		( const char *key, const char *salt );
#endif

#if	defined(MSDOS)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(NeXT)
char *	crypt		( const char *key, const char *salt );
long	random		( void );
void	srandom		( int seed );
#endif

#if	defined(sequent)
char *	crypt		( const char *key, const char *salt );
void	fclose		( FILE *stream );
int	fprintf		( FILE *stream, const char *format, ... );
int	fread		( void *ptr, int size, int n, FILE *stream );
int	fseek		( FILE *stream, long offset, int ptrname );
void	perror		( const char *s );
long	random		( void );
int	ungetc		( int c, FILE *stream );
#endif

#if	defined(sun)
char *	crypt		( const char *key, const char *salt );
void	fclose		( FILE *stream );
int	fprintf		( FILE *stream, const char *format, ... );
int	fread		( void *ptr, int size, int n, FILE *stream );
int	fseek		( FILE *stream, long offset, int ptrname );
void	perror		( const char *s );
long	random		( void );
int	ungetc		( int c, FILE *stream );
#endif

#if	defined(ultrix)
char *	crypt		( const char *key, const char *salt );
long	random		( void );
void	srandom		( int seed );
#endif



/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif

#endif /* !defined(PROTO_H) */
