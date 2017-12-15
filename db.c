/**************************************************************************
*  file: db.c , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars, booting world, resetting etc.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/




#include CONFIG

#if HAVE_STRINGS_H

#include <strings.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#include <ctype.h>
#include <time.h>
#include <errno.h>

#include "structs.h"
#include "mob.h"
#include "utils.h"
#include "db.h"
#include "org.h"
#include "bio.h"
#include "comm.h"
#include "player.h"
#include "skills.h"
#include "time.h"
#include "proto.h"


/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

struct room_data *world;              /* dyn alloc'ed array of rooms     */
int top_of_world = 0;                 /* ref to the top element of world */
struct obj_data  *object_list = 0;    /* the global linked list of obj's */
struct char_data *character_list = 0; /* global l-list of chars          */
struct ban_t *ban_list=0;	      /* list of banned site--sigh 	 */
int log_all=0;			      /* If 1, log all commands to syslog*/
struct zone_data *zone_table;         /* table of reset data             */
int top_of_zone_table = 0;
int top_of_bio_table = 0;

char credits[MAX_STRING_LENGTH];      /* the Credits List                */
char news[MAX_STRING_LENGTH];	        /* the news                        */
char motd[MAX_STRING_LENGTH];         /* the messages of today           */
char help[MAX_STRING_LENGTH];         /* the main help page              */
char info[MAX_STRING_LENGTH];         /* the info text                   */


FILE *mob_f,                          /* file containing mob prototypes  */
  *obj_f,                          /* obj prototypes                  */
  *help_fl;                        /* file for help texts (HELP <kwd>)*/

struct mob_index_data *mob_index;     /* index table for mobile file     */
struct bio_index_data *bio_index;     /* index table for bio file        */
struct obj_index_data *obj_index;     /* index table for object file     */
struct help_index_element *help_index = 0;

int top_of_mobt = 0;                  /* top of mobile index table       */
int top_of_objt = 0;                  /* top of object index table       */
int top_of_helpt;                     /* top of help index table         */

struct time_info_data time_info;	/* the infomation about the time   */

int dont_save_ids=TRUE; /* We won't save until we're done loading */




/* local procedures */
void setup_dir(FILE *fl, int room, int dir);
void setup_current(FILE *fl, int room, int dir);
void boot_world(void);
void boot_zones(void);
void boot_id(void);
void boot_bio(void);
struct mob_index_data *generate_mob_indices(FILE *fl, int *top);
struct obj_index_data *generate_obj_indices(FILE *fl, int *top);
int is_empty(int zone_nr);
void reset_zone(int zone);
int file_to_string(char *name, char *buf);
void renum_world(void);
void renum_zone_table(void);
void reset_time(void);
void weather_setup(void);
void update_id(void);

/* For the identification system */
#define ID_MOB 0
#define ID_PLAYER 1
#define ID_OBJ 2

int next_id(int kind);
#define nextmob() next_id(ID_MOB)
#define nextplayer() next_id(ID_PLAYER)
#define nextobj() next_id(ID_OBJ)

/* external refs */
void boot_cron(void);
void boot_orgs(void);
void boot_plot(void);
void boot_syllables(void);
extern struct descriptor_data *descriptor_list;
void load_messages(void);
void initialize_boards(void);
void weather_and_time ( int mode );
void log(char *str);
int dice(int number, int size);
int number(int from, int to);
void boot_social_messages(void);
struct help_index_element *build_help_index(FILE *fl, int *num);
void calc_light_zone(struct zone_data *zone);
char get_season(struct zone_data *zone);
extern int make_admin;
extern char log_buf[];
extern struct bio_info bio_attrs[];
extern size_t obj_info_size[];
char *strdup();


/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

#if !HAVE_MEMSET
void memset(char *s, char c, int n)
{
 int i;

 for (i = 0; i < n; i++) {
   s[i] = c;
 } /* for */
} 
#endif

int count_char(char c,FILE *fl)
{
    char buf[100];
    int count=0;

    rewind(fl);
    while(fgets(buf,99,fl)) {
        if(*buf==c)
            count++;
    }
    rewind(fl);
    return(count);
}

/* body of the booting system */
void boot_db(void)
{
    int i;
    extern int no_specials;

    log("Boot db -- BEGIN.");

    log("Resetting the game time:");
    reset_time();

    log("Reading newsfile, credits, help-page, info and motd.");
    file_to_string(NEWS_FILE, news);
    file_to_string(CREDITS_FILE, credits);
    file_to_string(MOTD_FILE, motd);
    file_to_string(HELP_PAGE_FILE, help);
    file_to_string(INFO_FILE, info);

    log("Opening help file.");
    if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
	log("   Could not open help file.");
    else 
	help_index = build_help_index(help_fl, &top_of_helpt);

    log("Loading cron files.");
    boot_cron();

    log("Loading online readers.");
    boot_readers();

    log("Loading org files.");
    boot_orgs();

    log("Loading zone table.");
    boot_zones();

    log("Loading rooms.");
    boot_world();
    log("Renumbering rooms.");
    renum_world();

    log("Opening mobile, object files.");
    if (!(mob_f = fopen(MOB_FILE, "r")))
    {
	perror("boot");
	exit(0);
    }

    if (!(obj_f = fopen(OBJ_FILE, "r")))
    {
	perror("boot");
	exit(0);
    }
    log("Generating index tables for mobile and object files.");
    mob_index = generate_mob_indices(mob_f, &top_of_mobt);
    obj_index = generate_obj_indices(obj_f, &top_of_objt);

    log("Booting bios");
    boot_bio();

    log("Loading current identification limits.");
    boot_id();
	    
    log("Renumbering zone table.");
    renum_zone_table();

    log("Loading fight messages.");
    load_messages();

    log("Loading social messages.");
    boot_social_messages();

    log("Initializing message boards.");
    initialize_boards();

    log("Assigning function pointers:");
    if (!no_specials)
    {
	log("   Mobiles.");
	assign_mobiles();
	log("   Objects.");
	assign_objects();
	log("   Room.");
	assign_rooms();
    }

    log("Loading world objects");
    load_world_obj();

    log("Setting up weather");
    weather_setup();

    for (i = 0; i <= top_of_zone_table; i++)
    {
	fprintf(stderr, "Boot-time reset of %s, rooms %d-%d (%d-%d).\n",
	    zone_table[i].name,
	    (i ? (zone_table[i - 1].top + 1) : 0),
	    zone_table[i].top,
	    zone_table[i].real_bottom,
	    zone_table[i].real_top);
	reset_zone(i);
    }

    log("Booting syllables");
    boot_syllables();

    log("Booting plot system");
    boot_plot();

    reset_q.head = reset_q.tail = 0;

    dont_save_ids = FALSE;
    update_id();

    log("Boot db -- DONE.");
{void temp_cron_hack(); temp_cron_hack(); }
}


/* reset the time in the game from file */
void reset_time(void)
{
    char buf[MAX_STRING_LENGTH];

    long beginning_of_time = 650336715;

    struct time_info_data mud_time_passed(time_t t2, time_t t1);

    FILE *f1;
    long current_time;
    long last_time;
    long diff_time;

    time_info = mud_time_passed(time(0), beginning_of_time);

    if (!(f1 = fopen(TIME_FILE, "r")))
    {
	perror("reset time");
	exit(0);
    }

    fscanf(f1, "#\n");

    fscanf(f1, "%d\n", &last_time);
    fscanf(f1, "%hd\n", &time_info.hours);
    fscanf(f1, "%hd\n", &time_info.day);
    fscanf(f1, "%hd\n", &time_info.month);
    fscanf(f1, "%hd\n", &time_info.year);

    fclose(f1);

    current_time = time(0);
    diff_time = current_time - last_time;

    sprintf(buf,"   Time since last shutdown: %d.", diff_time);
    log(buf);

/* not sure of the purpose of this... 
    diff_hours = diff_time/SECS_PER_MUD_HOUR;
    diff_time = diff_time % SEC_PR_HOUR;
    
    sprintf(buf,"   Real time lack : %d sec.", diff_time);
    log(buf);

    for(;diff_hours > 0; diff_hours--) 
	weather_and_time(0);
*/


    sprintf(buf,"   Current Gametime: %dH %dD %dM %dY.",
	time_info.hours, time_info.day,
	time_info.month, time_info.year);
    log(buf);
}

/* Smandoggi's new weather setup */
void weather_setup()
{
    int zon,s;

    /* default conditions for season values */
    char winds[6] = {2,12,30,40,50,80};
    char precip[9] = {0,0,0,1,3,8,15,23,45};
    char humid[9] = {4,10,20,30,40,50,60,75,100};
    char temps[11] = {-20,-10,0,2,7,17,23,29,37,55,95};

    for(zon=0;zon<=top_of_zone_table;zon++) {
	/* get the season */
	s=get_season(&zone_table[zon]);

	/* These are pretty standard start values */
	zone_table[zon].conditions.pressure=980;
	zone_table[zon].conditions.free_energy=10000;
	zone_table[zon].conditions.precip_depth=0;

	/* These use the default conditions above */
	zone_table[zon].conditions.windspeed=
	    winds[(int)zone_table[zon].climate.season_wind[s]];

	zone_table[zon].conditions.wind_dir=
	    zone_table[zon].climate.season_wind_dir[s];

	zone_table[zon].conditions.precip_rate=
	    precip[(int)zone_table[zon].climate.season_precip[s]];

	zone_table[zon].conditions.temp=
	    temps[(int)zone_table[zon].climate.season_temp[s]];

	zone_table[zon].conditions.humidity=
	    humid[(int)zone_table[zon].climate.season_precip[s]];
    
	/* Set ambient light */
	calc_light_zone(&zone_table[zon]);
    }
}



/* update the time file */
void update_time(void)
{
    FILE *f1;
    extern struct time_info_data time_info;
    long current_time;


    if (!(f1 = fopen(TIME_FILE, "w")))
    {
	perror("update time");
	exit(0);
    }

    current_time = time(0);
    log("Time update.");

    fprintf(f1, "#\n");

    fprintf(f1, "%d\n", current_time);
    fprintf(f1, "%d\n", time_info.hours);
    fprintf(f1, "%d\n", time_info.day);
    fprintf(f1, "%d\n", time_info.month);
    fprintf(f1, "%d\n", time_info.year);

    fclose(f1);
}

/* generate index table for monster file */
struct mob_index_data *generate_mob_indices(FILE *fl, int *top)
{
    int i = 0;
    struct mob_index_data *index;
    char buf[82];

    CREATE(index,struct mob_index_data,count_char('#',fl));
 
    for (;;)
    {
	if (fgets(buf, 81, fl))
	{
	    if (*buf == '#')
	    {
		sscanf(buf, "#%d", &index[i].virtual);
		index[i].pos = ftell(fl);
		index[i].number = 0;
		index[i].func = 0;
                /* location and frequency information to go here */
                index[i].name = fread_string(fl);
		i++;
	    }
	    else 
		if (*buf == '$')	/* EOF */
		    break;
	}
	else
	{
	    perror("generate mob indices");
	    exit(0);
	}
    }
    *top = i - 2;
    return(index);
}

/* generate index table for object file */
struct obj_index_data *generate_obj_indices(FILE *fl, int *top)
{
    int i = 0;
    struct obj_index_data *index;
    char buf[82];

    CREATE(index,struct obj_index_data,count_char('#',fl));

    for (;;)
    {
	if (fgets(buf, 81, fl))
	{
	    if (*buf == '#')
	    {
		sscanf(buf, "#%d", &index[i].virtual);
		index[i].pos = ftell(fl);
		index[i].number = 0;
		index[i].func = 0;
                index[i].name = fread_string(fl);
		i++;
	    }
	    else 
		if (*buf == '$')	/* EOF */
		    break;
	}
	else
	{
	    perror("generate obj indices");
	    exit(0);
	}
    }
    *top = i - 2;
    return(index);
}




/* load the rooms */
void boot_world(void)
{
    FILE *fl;
    int room_nr = 0, zone = 0, virtual_nr, flag, tmp;
    char *temp, chk[50];
    struct extra_descr_data *new_descr;

    world = 0;
    character_list = 0;
    object_list = 0;
    
    if (!(fl = fopen(WORLD_FILE, "r")))
    {
	perror("fopen");
	log("boot_world: could not open world file.");
	exit(0);
    }

    CREATE(world,struct room_data,count_char('#',fl));

    do
    {
	fscanf(fl, " #%d\n", &virtual_nr);

	temp = fread_string(fl);
	if((flag = (*temp != '$')))	/* a new record to be read */
	{
            for(tmp=0;tmp<MAX_RDESC;tmp++)
                world[room_nr].description[tmp] = NULL;

	    world[room_nr].number = virtual_nr;
	    world[room_nr].name = temp;
	    world[room_nr].description[0] = fread_string(fl);

	    if (top_of_zone_table >= 0)
	    {
		fscanf(fl, " %*d ");

		/* OBS: Assumes ordering of input rooms */

		if (world[room_nr].number <= (zone ? zone_table[zone-1].top : -1))
		{
		    fprintf(stderr, "Room nr %d is below zone %d.\n",
			room_nr, zone);
		    exit(0);
		}
		while (world[room_nr].number > zone_table[zone].top)
		    if(++zone > top_of_zone_table)
		    {
			fprintf(stderr, "Room %d is outside of any zone.\n",
			    virtual_nr);
			exit(0);
		    }
		world[room_nr].zone = zone;

		/* Update zone range of real rooms */
		if(zone_table[zone].real_bottom==-1)
		    zone_table[zone].real_bottom=room_nr;
		zone_table[zone].real_top=room_nr;
	    }
	    fscanf(fl, " %d ", &tmp);
	    world[room_nr].room_flags = tmp;
	    fscanf(fl, " %d ", &tmp);
	    world[room_nr].sector_type = tmp;

	    world[room_nr].funct = 0;
	    world[room_nr].contents = 0;
	    world[room_nr].people = 0;
	    world[room_nr].light = 0; /* Zero light sources */

	    world[room_nr].ex_description = 0;

	    /* Default values */
	    world[room_nr].chance_fall = 0;
	    world[room_nr].mana_alignment = -1;
	    world[room_nr].mana = 0;
	    world[room_nr].trap = TRAP_NONE;

	    for (;;)
	    {
		fscanf(fl, " %s \n", chk);

		if (*chk == 'C')  /* current field */
		    setup_current(fl,room_nr,atoi(chk + 1));
		else if (*chk == 'D')  /* direction field */
		    setup_dir(fl, room_nr, atoi(chk + 1));
		else if (*chk == 'E')  /* extra description field */
		{
		    CREATE(new_descr, struct extra_descr_data, 1);
		    new_descr->keyword = fread_string(fl);
		    new_descr->description = fread_string(fl);
		    new_descr->next = world[room_nr].ex_description;
		    world[room_nr].ex_description = new_descr;
		} else if (*chk == 'R') {
		    fscanf(fl," %d ",&tmp);
		    fscanf(fl," %d ",&tmp);
		} else if(*chk == 'F') {
		    fscanf(fl," %d ",&tmp);
		    world[room_nr].chance_fall = tmp;
		} else if(*chk == 'M') {
		    fscanf(fl," %d ",&tmp);
		    world[room_nr].mana_alignment = tmp;
		    fscanf(fl," %d ",&tmp);
		    world[room_nr].mana = tmp;
		} else if(*chk == 'S') /* end of current room */
		    break;
	    }
			
	    room_nr++;
 		}
    }
    while (flag);

    free(temp);	/* cleanup the area containing the terminal $  */

    fclose(fl);
    top_of_world = --room_nr;
}





/* read current data */
void setup_current(FILE *fl, int room, int dir)
{
/*
    int tmp;

    CREATE(world[room].current[dir], 
	struct current_data, 1);

    world[room].current[dir]->msg_char = fread_string(fl);
    world[room].current[dir]->msg_to_room = fread_string(fl);
    world[room].current[dir]->msg_from_room = fread_string(fl);
    world[room].current[dir]->msg_item_to_room = fread_string(fl);
    world[room].current[dir]->msg_item_from_room = fread_string(fl);

    fscanf(fl, " %d ", &tmp);
    world[room].current[dir]->strength = tmp;
    fscanf(fl, " %d ", &tmp);
    world[room].current[dir]->chance = tmp;
    fscanf(fl, " %d ", &tmp);
    world[room].current[dir]->flags = tmp;
*/
}


/* read direction data */
void setup_dir(FILE *fl, int room, int dir)
{
    int tmp;
    struct obj_data *exit_obj;
    struct obj_info_exit *exit_info;

    exit_obj = new_object();
    exit_obj->obj_flags.extra_flags = ITEM_IGNORE;
    exit_info=(struct obj_info_exit *)new_obj_info(ITEM_EXIT,exit_obj);

    exit_info->dir = dir;
    exit_info->general_description = fread_string(fl);
    exit_obj->name = fread_string(fl);

    fscanf(fl, " %d ", &tmp);
    if (tmp == 1)
	exit_info->exit_info = EX_ISDOOR;
    else if (tmp == 2)
	exit_info->exit_info = EX_ISDOOR | EX_PICKPROOF;
    else
	exit_info->exit_info = 0;

    fscanf(fl, " %d ", &tmp);
    exit_info->key = tmp;

    fscanf(fl, " %d ", &tmp);
    exit_info->to_room = tmp;

    obj_to_room(exit_obj,room);
}




void renum_world(void)
{
    register int room,res;
    struct obj_data *o;
    struct obj_info_exit *exit;

    for (room = 0; room <= top_of_world; room++)
        for (o=world[room].contents;o;o=o->next_content)
            if ((exit=(struct obj_info_exit *)get_obj_info(o,ITEM_EXIT))) {
                if(exit->to_room !=-1) {
                    res = exit->to_room = real_room(exit->to_room);
                    if(res==-1)
                        fprintf(stderr,"from room %d dir %d\n",
                                    world[room].number,exit->dir);
                }
            }
}


void renum_zone_table(void)
{
    int zone, comm;

    for (zone = 0; zone <= top_of_zone_table; zone++)
	for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++)
	    switch(zone_table[zone].cmd[comm].command)
	    {
		case 'M':
		    zone_table[zone].cmd[comm].arg1 =
			real_mobile(zone_table[zone].cmd[comm].arg1);
		    zone_table[zone].cmd[comm].arg3 = 
			real_room(zone_table[zone].cmd[comm].arg3);
		break;
		case 'O':
		    zone_table[zone].cmd[comm].arg1 = 
			real_object(zone_table[zone].cmd[comm].arg1);
		    if (zone_table[zone].cmd[comm].arg3 != NOWHERE)
			zone_table[zone].cmd[comm].arg3 =
			real_room(zone_table[zone].cmd[comm].arg3);
		break;
		case 'G':
		    zone_table[zone].cmd[comm].arg1 =
			real_object(zone_table[zone].cmd[comm].arg1);
		break;
		case 'E':
		    zone_table[zone].cmd[comm].arg1 =
			real_object(zone_table[zone].cmd[comm].arg1);
		break;
		case 'P':
		    zone_table[zone].cmd[comm].arg1 =
			real_object(zone_table[zone].cmd[comm].arg1);
		    zone_table[zone].cmd[comm].arg3 =
			real_object(zone_table[zone].cmd[comm].arg3);
		break;					
		case 'D':
		    zone_table[zone].cmd[comm].arg1 =
			real_room(zone_table[zone].cmd[comm].arg1);
		break;
	    }
}


/* load the zone table and command tables */
void boot_zones(void)
{
    FILE *fl;
    int zon = 0, cmd_no = 0, expand, tmp, i;
    char *check, buf[81];

    if (!(fl = fopen(ZONE_FILE, "r")))
    {
	perror("boot_zones");
	exit(0);
    }

    CREATE(zone_table,struct zone_data,count_char('#',fl));

    for (;;)
    {
	fscanf(fl, " #%d\n",&tmp);
	check = fread_string(fl);

	if (*check == '$')
	    break;		/* end of file */


	zone_table[zon].number = tmp;
	zone_table[zon].name = check;
        zone_table[zon].author = fread_string(fl);
	fscanf(fl, " %d ", &zone_table[zon].top);
	fscanf(fl, " %d ", &zone_table[zon].lifespan);
	fscanf(fl, " %d ", &zone_table[zon].reset_mode);
	fscanf(fl, " %d ", &zone_table[zon].flags);

        zone_table[zon].desc_mode = RDESC_NORMAL; /* Start in default mode */

	/* get weather info for the zone */
	fscanf(fl, " %d ", &zone_table[zon].climate.season_pattern);
	fscanf(fl, " %d ", &zone_table[zon].climate.flags);
	fscanf(fl, " %d ", &zone_table[zon].climate.energy_add);

	for(i=0;i<MAX_SEASONS;i++)
	    fscanf(fl," %d ",&zone_table[zon].climate.season_wind[i]);
	for(i=0;i<MAX_SEASONS;i++)
	    fscanf(fl," %d ",&zone_table[zon].climate.season_wind_dir[i]);
	for(i=0;i<MAX_SEASONS;i++)
	    fscanf(fl," %d ",&zone_table[zon].climate.season_wind_variance[i]);
	for(i=0;i<MAX_SEASONS;i++)
	    fscanf(fl," %d ",&zone_table[zon].climate.season_precip[i]);
	for(i=0;i<MAX_SEASONS;i++)
	    fscanf(fl," %d ",&zone_table[zon].climate.season_temp[i]);

	/* Set beginning values for zone real range */
	zone_table[zon].real_bottom=zone_table[zon].real_top=-1;

	/* read the command table */

	cmd_no = 0;

	for (expand = 1;;)
	{
	    if (expand)
		if (!cmd_no)
		    CREATE(zone_table[zon].cmd, struct reset_com, 1);
		else
		    if (!(zone_table[zon].cmd =
		     (struct reset_com *) realloc(zone_table[zon].cmd, 
		     (cmd_no + 1) * sizeof(struct reset_com))))
		    {
			perror("reset command load");
			exit(0);
		    }

	    expand = 1;

	    fscanf(fl, " "); /* skip blanks */
	    fscanf(fl, "%c", 
		&zone_table[zon].cmd[cmd_no].command);

	    if (zone_table[zon].cmd[cmd_no].command == 'S')
		break;

	    if (zone_table[zon].cmd[cmd_no].command == '*')
	    {
		expand = 0;
		fgets(buf, 80, fl); /* skip command */
		continue;
	    }

	    fscanf(fl, " %d %d %d", 
		&tmp,
		&zone_table[zon].cmd[cmd_no].arg1,
		&zone_table[zon].cmd[cmd_no].arg2);

	    zone_table[zon].cmd[cmd_no].if_flag = tmp;

	    if (zone_table[zon].cmd[cmd_no].command == 'M' ||
		zone_table[zon].cmd[cmd_no].command == 'O' ||
		zone_table[zon].cmd[cmd_no].command == 'E' ||
		zone_table[zon].cmd[cmd_no].command == 'P' ||
		zone_table[zon].cmd[cmd_no].command == 'D')
		fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);

	    fgets(buf, 80, fl);	/* read comment */

	    cmd_no++;
	}
	zon++;
    }
    top_of_zone_table = --zon;
    free(check);
    fclose(fl);
}



/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*********************************************************************** */

void boot_bio()
{
    FILE *fl;
    char buf[MAX_INPUT_LENGTH],ch,atname[MAX_INPUT_LENGTH],
        atval1[MAX_INPUT_LENGTH], atval2[MAX_INPUT_LENGTH];
    struct bio_attr *new_attr;
    int tmp,attrib;

    if(!(fl=fopen(BIO_FILE,"r"))) {
        log("Cannot open bio file!");
        exit(1);
    }

    CREATE(bio_index,struct bio_index_data, count_char('#',fl));

    top_of_bio_table=0;

    for(;;) {
	fscanf(fl, "#%d\n", &tmp);

        if(feof(fl))
            break;

	bio_index[top_of_bio_table].virtual=tmp;

        /* What we keep track of internally */
        bio_index[top_of_bio_table].count=0;

        /* location */
        fgets(buf,80,fl);
        tmp=0;
        bio_index[top_of_bio_table].loc = 0;

        while(buf[tmp] && buf[tmp]!='\n') {
            switch(buf[tmp]) {
                case 'C':
                    bio_index[top_of_bio_table].loc |= TER_CITY;
                    break;
                case 'V':
                    bio_index[top_of_bio_table].loc |= TER_VILLAGE;
                    break;
                case 'c':
                    bio_index[top_of_bio_table].loc |= TER_CAVES;
                    break;
                case 'D':
                    bio_index[top_of_bio_table].loc |= TER_DUNGEON;
                    break;
                case 'F':
                    bio_index[top_of_bio_table].loc |= TER_FOREST;
                    break;
                case 'P':
                    bio_index[top_of_bio_table].loc |= TER_PLAIN;
                    break;
                case 'H':
                    bio_index[top_of_bio_table].loc |= TER_HILLS;
                    break;
                case 'M':
                    bio_index[top_of_bio_table].loc |= TER_MOUNTAIN;
                    break;
                case 'L':
                    bio_index[top_of_bio_table].loc |= TER_LAKE;
                    break;
                case 'm':
                    bio_index[top_of_bio_table].loc |= TER_MARSH;
                    break;
                case 'S':
                    bio_index[top_of_bio_table].loc |= TER_SWAMP;
                    break;
                case 'R':
                    bio_index[top_of_bio_table].loc |= TER_RIVER;
                    break;
                case 'h':
                    bio_index[top_of_bio_table].loc |= TER_SHALLOW;
                    break;
                case 'd':
                    bio_index[top_of_bio_table].loc |= TER_DEEPWATER;
                    break;
                case 'A':
                    bio_index[top_of_bio_table].loc |= TER_ASTRAL;
                    break;
                case 'E':
                    bio_index[top_of_bio_table].loc |= TER_ETHER;
                    break;
                case 'U':
                    bio_index[top_of_bio_table].loc |= TER_UNDERWORLD;
                    break;
                case 'O':
                    bio_index[top_of_bio_table].loc |= TER_OTHERPLANE;
                    break;
                case '-':
                    bio_index[top_of_bio_table].loc = 0;
                    break;
                default:
                    fprintf(stderr,"BUG: Bad bio location %c.\n",buf[tmp]);
                    break;
            }
            tmp++;
        }

        /* frequency */
        fscanf(fl,"%c\n",&ch);

        switch(ch) {
            case 'C':
                bio_index[top_of_bio_table].freq = FREQ_COMMON;
                break;
            case 'U':
                bio_index[top_of_bio_table].freq = FREQ_UNCOMMON;
                break;
            case 'R':
                bio_index[top_of_bio_table].freq = FREQ_RARE;
                break;
            case 'V':
                bio_index[top_of_bio_table].freq = FREQ_VERYRARE;
                break;
            case 'S':
                bio_index[top_of_bio_table].freq = FREQ_SPECIAL;
                break;
            case '1':
                bio_index[top_of_bio_table].freq = FREQ_UNIQUE;
                break;
            case '-':
                bio_index[top_of_bio_table].freq = FREQ_NEVER;
                break;
            default:
                fprintf(stderr,"BUG: Bad bio frequency %c.\n",ch);
                bio_index[top_of_bio_table].freq = FREQ_NEVER;
                break;
        }

        /* inherits */
        fgets(buf,80,fl);
        sscanf(buf,"%d",&tmp);
        bio_index[top_of_bio_table].inherits = find_bio(tmp);
 
        bio_index[top_of_bio_table].name = fread_string(fl);

        /* descriptions */
        bio_index[top_of_bio_table].sdesc = fread_string(fl);
        bio_index[top_of_bio_table].ldesc = fread_string(fl);

        /* Loop on reading attributes */
        for(;;) {
            fgets(buf,80,fl);

            if(buf[0]=='S')
                break;

            sscanf(buf,"%s %s %s",atname, atval1,atval2);

            attrib=search_block_offset(atname,(char **)bio_attrs,1,sizeof(struct bio_info),0);

            if(attrib==-1) {
                sprintf(buf,"BUG: Bad bio attribute %s in %s.",atname,
                        bio_index[top_of_bio_table].name);
                log(buf);
            } else {
                /* Make attribute and add it */
                CREATE(new_attr,struct bio_attr,1);

                new_attr->next = bio_index[top_of_bio_table].attribs;
                bio_index[top_of_bio_table].attribs = new_attr;

/***                new_attr->value = mung_somehow(atval);****/
                new_attr->type = attrib;

                new_attr->value1 = atoi(atval1);
                if(new_attr->value1<bio_attrs[attrib].low ||
                        new_attr->value1>bio_attrs[attrib].high) {
                    sprintf(log_buf,"BUG: Bio %s value %s out of range [%d,%d]",
                        atname,atval1,bio_attrs[attrib].low,bio_attrs[attrib].high);
                    log(log_buf);
                }

                if(bio_attrs[attrib].flags & BI_RANGE) {
                    new_attr->value2 = atoi(atval2);
                    if(new_attr->value2<bio_attrs[attrib].low ||
                            new_attr->value2>bio_attrs[attrib].high) {
                        sprintf(log_buf,"Bio %s value %s out of range [%d,%d]",
                            atname,atval2,bio_attrs[attrib].low,bio_attrs[attrib].high);
                        log(log_buf);
                    }
                } else if(bio_attrs[attrib].flags & BI_DICE) {
                    new_attr->value2 = atoi(atval2);
                    if(new_attr->value1<bio_attrs[attrib].low ||
                            new_attr->value1 * new_attr->value2 >
                            bio_attrs[attrib].high) {
                        sprintf(log_buf,
                            "Bio dice %s value %sd%s out of range [%d,%d]",
                            atname,atval1,atval2,bio_attrs[attrib].low,
                            bio_attrs[attrib].high);
                        log(log_buf);
                    }
                }
            }
        }

        top_of_bio_table++;
    }

    fclose(fl);
}

int find_bio(int number)
{
    int top,bot,mid;

    top=top_of_bio_table-1;
    bot=0;

    if(top==-1) /* This *is* quite possible */
        return -1;

    if(top==0)  /* This too... */
        return (bio_index[0].virtual==number) ? 0 : -1;

    while(top >= bot) {
        mid=(top+bot)/2;
        if(bio_index[mid].virtual==number)
            return mid;
        else if(bio_index[mid].virtual<number)
            bot=mid+1;
        else
            top=mid-1;
    }
    return -1;
}

struct bio_attr *find_bio_attr(int bio_type,int attr)
{
    int i;
    struct bio_attr *ba;

    if((i = find_bio(bio_type)) == -1) {
        sprintf(log_buf,"BUG: Can't find bio %d in find_bio_attr",bio_type);
        log(log_buf);
        return NULL;
    }

    while(i!=-1) {
        for(ba = bio_index[i].attribs;ba;ba = ba->next)
            if(ba->type == attr)
                return ba;

        i = bio_index[i].inherits; /* inherits is real, not virtual */
    }

    log("BUG: Hit root bio node without finding attr.");
    return NULL;
}

int get_bio_attr(struct char_data *ch, int attr)
{
    int ret=NULL;
    struct bio_attr *ba;

    switch(attr) {
        case BIO_STR            :
            ret = GET_STR(ch);
            break;
        case BIO_INT            :
            ret = GET_INT(ch);
            break;
        case BIO_WIS            :
            ret = GET_WIS(ch);
            break;
        case BIO_DEX            :
            ret = GET_DEX(ch);
            break;
        case BIO_CON            :
            ret = GET_CON(ch);
            break;
        case BIO_CHR            :
            ret = GET_CHR(ch);
            break;
        case BIO_HITPOINTS      :
            ret = GET_HIT(ch);
            break;
        case BIO_MANA           :
            ret = GET_MANA(ch);
            break;
        case BIO_MOVES          :
            ret = GET_MOVE(ch);
            break;
        case BIO_HITDICE        :
        case BIO_LIMBS          :
        case BIO_LIMBS_PROPEL   :
        case BIO_LIMBS_GRASP    :
        case BIO_TAILS          :
        case BIO_SIGHTED        :
        case BIO_HEARING        :
        case BIO_SMELLING       :
        case BIO_ODOR_TYPE      :
        case BIO_ODOR_STRENGTH  :
        case BIO_SIZE           :
        case BIO_RESERVED1      :
        case BIO_RESERVED2      :
        case BIO_RESERVED3      :
        case BIO_RESERVED4      :
        case BIO_RESERVED5      :
        case BIO_POS_SLEEP      :
        case BIO_POS_REST       :
        case BIO_POS_SIT        :
        case BIO_POS_SWIM       :
        case BIO_POS_STAND      :
        case BIO_POS_LEVITATE   :
        case BIO_POS_FLY        :
        case BIO_MOBILE         :
        case BIO_GESTATION      :
        case BIO_LITTER_SIZE    :
        case BIO_SEXES          :
        case BIO_SEX_RATIO      :
        case BIO_DIET           :
        case BIO_PREF_TEMP      :
        case BIO_PREF_HUMIDITY  :
        case BIO_TEMP_TOLERATE  :
        case BIO_HUMID_TOLERATE :
        case BIO_DAILY_CALS     :
        default: /* This should *ALWAYS* be a value, not a range!! */
            ba=find_bio_attr(ch->physical->species,attr);
            if(ba)
                ret = ba->value1;
    }
    return ret;
}

/* to probably take over as read_mobile... */

/* There needs to be routine(s) to call this which does
 * mob- or player- specific data, struct creation
 */

struct char_data *read_bio(int nr,int type)
{
    struct char_data *ch;
    int i;

    if (type == VIRTUAL) {
	if ((nr = find_bio(nr)) < 0){
	    return(0);
	}
    }

    
    ch = bare_char();

    init_char(ch);

    ch->desc = 0;

    ch->in_room = NOWHERE;

    ch->physical->species = bio_index[nr].virtual;

    ch->player.name = strdup(bio_index[nr].name);
    ch->player.short_descr = strdup(bio_index[nr].sdesc);
    ch->player.long_descr = strdup(bio_index[nr].ldesc);

    /* Loop on the various attributes, assigning them */
    for(i=0;*bio_attrs[i].name!='\n';i++)
        if(bio_attrs[i].flags & BI_INTRINSIC)
            roll_bio_attr(ch,i);

    /* Handle those bios which have no hitdice */
    if(GET_HIT(ch)<1)
        roll_bio_attr(ch,BIO_HITPOINTS);

    /* NEED TO MAKE ch->physical FIRST!!! */
    ch->tmpabilities = ch->physical->abilities;

    CREATE(ch->mobinfo,struct char_mob_data,1);

    return(ch);
}

struct char_data *load_bio_to(int nr,int type,int where)
{
    struct char_data *ch;

    if(!(ch = read_bio(nr,type)))
        return NULL;

    SET_BIT(ch->specials.act,ACT_ISNPC);
    ch->id=nextmob();

    /* insert in list */

    ch->next = character_list;
    character_list = ch;

    char_to_room(ch,where,0);

    return ch;
}

void roll_bio_attr(struct char_data *ch,int attr)
{
    int val;
    struct bio_attr *ba;
    struct char_phys_data *phys;

    phys=ch->physical;

    if(!(ba = find_bio_attr(phys->species,attr))) {
        sprintf(log_buf,"Cannot roll bio attr '%s'.",bio_attrs[attr].name);
        log(log_buf);
        return; /* What can we do? */
    }

    if(bio_attrs[attr].flags & BI_RANGE)
        val = number(ba->value1,ba->value2);
    else if(bio_attrs[attr].flags & BI_DICE)
        val = dice(ba->value1,ba->value2);
    else
        val = ba->value1;

    switch(attr) {
        case BIO_STR:
            phys->abilities.str = val;
            break;
        case BIO_INT:
            phys->abilities.intel = val;
            break;
        case BIO_WIS:
            phys->abilities.wis = val;
            break;
        case BIO_DEX:
            phys->abilities.dex = val;
            break;
        case BIO_CON:
            phys->abilities.con = val;
            break;
        case BIO_CHR:
            phys->abilities.chr = val;
            break;
        case BIO_HITPOINTS:
            phys->max_hit = val;
            break;
        case BIO_HITDICE:
            phys->max_hit = dice(val,8);
            break;
        case BIO_MANA:
            phys->max_mana = val;
            break;
        case BIO_MOVES:
            phys->max_move = val;
            break;
        default:
            sprintf(log_buf,"Cannot set bio attr %d.",attr);
            log(log_buf);
            break;
    }

    if(val < bio_attrs[attr].low) {
    } else if(val > bio_attrs[attr].high) {
    }
}


/* read a mobile from MOB_FILE */
struct char_data *read_mobile(int nr, int type)
{
    int i;
    long tmp, tmp2, tmp3;
    struct char_data *mob;
    char letter;

    i = nr;
    if (type == VIRTUAL) {
	if ((nr = real_mobile(nr)) < 0){
	    return(0);
	}
    }

    fseek(mob_f, mob_index[nr].pos, 0);

    mob = bare_char();

    /***** String data *** */
	
    mob->player.name = fread_string(mob_f);
    mob->player.short_descr = fread_string(mob_f);
    mob->player.long_descr = fread_string(mob_f);
    mob->player.description = fread_string(mob_f);
    mob->player.title = 0;

    /* *** Numeric data *** */

    fscanf(mob_f, "%d ", &tmp);
    mob->specials.act = tmp & (ACT_NICE_THIEF|ACT_NOSAVE);
    SET_BIT(mob->specials.act, ACT_ISNPC);

    fscanf(mob_f, " %d ", &tmp);

    fscanf(mob_f, " %d ", &tmp);
    mob->specials.alignment = tmp;

    fscanf(mob_f, " %c \n", &letter);

    if (letter == 'S') {
	/* New line of information */
	fscanf(mob_f, " %c ",&letter);
	/*switch(letter) {
	    case 'A':
		GET_CLASS(mob)=CLASS_ANIMAL;
		break;
	    case 'B':
		GET_CLASS(mob)=CLASS_BIRD;
		break;
	    case 'D':
		GET_CLASS(mob)=CLASS_DRAGON;
		break;
	    case 'G':
		GET_CLASS(mob)=CLASS_GIANT;
		break;
	    case 'H':
		GET_CLASS(mob)=CLASS_HUMANOID;
		break;
	    case 'I':
		GET_CLASS(mob)=CLASS_INSECT;
		break;
	    case 'O':
	    default:
		GET_CLASS(mob)=CLASS_OTHER;
		break;
	    case 'U':
		GET_CLASS(mob)=CLASS_UNDEAD;
		break;
	    case 'X':
		GET_CLASS(mob)=CLASS_DEMON;
		break;
	}*/

	fscanf(mob_f, " %d ",&tmp);
	GET_HOME(mob)=tmp;
	
	fscanf(mob_f, " %d \n",&tmp);
	/* don't do anything with this one yet */

	/* The new easy monsters */
	mob->physical->abilities.str   = 11;
	mob->physical->abilities.intel = 11; 
	mob->physical->abilities.wis   = 11;
	mob->physical->abilities.dex   = 11;
	mob->physical->abilities.con   = 11;

        mob->physical->hunger = 24;
        mob->physical->thirst = 24;
        mob->physical->exhaustion = 24;
        mob->physical->impairment = 0;

	fscanf(mob_f, " %d ", &tmp);
	/*GET_LEVEL(mob) = tmp;*/
	
	fscanf(mob_f, " %d ", &tmp);
	mob->points.hitroll = 20-tmp;
	
	fscanf(mob_f, " %d ", &tmp);
	mob->physical->armor = 10*tmp;

	fscanf(mob_f, " %dd%d+%d ", &tmp, &tmp2, &tmp3);
	mob->physical->max_hit = dice(tmp, tmp2)+tmp3;
	mob->physical->hit = mob->physical->max_hit;

	fscanf(mob_f, " %dd%d+%d \n", &tmp, &tmp2, &tmp3);
	mob->points.damroll = tmp3;
	mob->specials.damnodice = tmp;
	mob->specials.damsizedice = tmp2;

	mob->physical->mana = 10;
	mob->physical->max_mana = 10;

	mob->physical->move = 50;
	mob->physical->max_move = 50;

	fscanf(mob_f, " %d ", &tmp);
/*		mob->points.gold = tmp;*/

	fscanf(mob_f, " %d \n", &tmp);
	GET_EXP(mob) = tmp;

	fscanf(mob_f, " %d ", &tmp);
	mob->specials.position = tmp;

	fscanf(mob_f, " %d ", &tmp);

	fscanf(mob_f, " %d \n", &tmp);
	GET_SEX(mob) = tmp;

	mob->player.time.birth = time(0);
	mob->player.time.played	= 0;
	mob->player.time.logon  = time(0);
	GET_WEIGHT(mob) = 200;
	GET_HEIGHT(mob) = 198;

	for (i = 0; i < 5; i++)
	    mob->specials.apply_saving_throw[i] = MAX(20, 2);

    }

    mob->tmpabilities = mob->physical->abilities;

    mob->nr = nr;

    mob->desc = 0;

    mob->in_room = NOWHERE;

    CREATE(mob->mobinfo,struct char_mob_data,1);

    /* insert in list */

    mob->next = character_list;
    character_list = mob;

    mob_index[nr].number++;

    mob->id = nextmob();

    return(mob);
}

struct obj_data *new_object()
{
    struct obj_data *obj;

    CREATE(obj, struct obj_data, 1);
    clear_object(obj);

    obj->in_room = NOWHERE;
    obj->next_content = 0;
    obj->carried_by = 0;
    obj->in_obj = 0;
    obj->contains = 0;
    obj->info = NULL;
    obj->material = NULL;
    obj->affected = NULL;

    obj->next = object_list;
    object_list = obj;

    return(obj);
}

struct obj_info *new_obj_info(sh_int type,struct obj_data *obj)
{
    struct  obj_info *new;

/*    CREATE(new,obj_info_size[type],1); silly macro, trix are for kids */

    /* This is what the macro looks like in real life...huh? */
    do {
        if(!(new=(struct obj_info *)malloc(obj_info_size[type]))) {
            abort();
        }
    } while(0); /* I can understand the occasional while(1), but 0? */

    new->next = obj->info;
    obj->info = new;

    new->obj_type = type;
    return new;
}

/* read an object from OBJ_FILE */
struct obj_data *read_object(int nr, int type)
{
    struct obj_data *obj;
    int tmp, i;
    char chk[50];
    struct extra_descr_data *new_descr;

    i = nr;
    if (type == VIRTUAL) {
	if ((nr = real_object(nr)) < 0) {
	    sprintf(log_buf, "BUG: Object %d does not exist in database.",i);
	    log(log_buf);
	    return(0);
	}
    }

    fseek(obj_f, obj_index[nr].pos, 0);
    obj = new_object();

    /* *** string data *** */

    obj->name = fread_string(obj_f);
    obj->short_description = fread_string(obj_f);
    obj->description = fread_string(obj_f);
    obj->action_description = fread_string(obj_f);

    /* *** numeric data *** */

    fscanf(obj_f, " %d ", &tmp);
/**    obj->obj_flags.type_flag = tmp; **/
    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.extra_flags = tmp;
    fscanf(obj_f, " %d ", &tmp);
/**    obj->obj_flags.wear_flags = tmp; **/
    fscanf(obj_f, " %d ", &tmp);
/**    obj->obj_flags.value[0] = tmp; **/
    fscanf(obj_f, " %d ", &tmp);
/**    obj->obj_flags.value[1] = tmp; **/
    fscanf(obj_f, " %d ", &tmp);
/**    obj->obj_flags.value[2] = tmp; **/
    fscanf(obj_f, " %d ", &tmp);
/**    obj->obj_flags.value[3] = tmp; **/
    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.weight = tmp;
    fscanf(obj_f, " %d \n", &tmp);
    obj->obj_flags.cost = tmp;
    fscanf(obj_f, " %d \n", &tmp); /* was cost per day */

    /* *** extra descriptions *** */

    obj->ex_description = 0;

    while (fscanf(obj_f, " %s \n", chk), *chk == 'E')
    {
	CREATE(new_descr, struct extra_descr_data, 1);

	new_descr->keyword = fread_string(obj_f);
	new_descr->description = fread_string(obj_f);

	new_descr->next = obj->ex_description;
	obj->ex_description = new_descr;
    }
/*
    for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++)
    {
	fscanf(obj_f, " %d ", &tmp);
	obj->affected[i].location = tmp;
	fscanf(obj_f, " %d \n", &tmp);
	obj->affected[i].modifier = tmp;
	fscanf(obj_f, " %s \n", chk);
    }

    for (;(i < MAX_OBJ_AFFECT);i++)
    {
	obj->affected[i].location = APPLY_NONE;
	obj->affected[i].modifier = 0;
    }*/

    obj->item_number = nr;	
    obj_index[nr].number++;

    return (obj);  
}

struct obj_info *get_obj_info(struct obj_data *o,int type)
{
    struct obj_info *i;

    for(i=o->info;i;i=i->next)
        if(i->obj_type==type)
            return i;

    return NULL;
}



#define ZO_DEAD  999

#define MAX_ZONES_PER_PASS 3

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
    int i;
    struct reset_q_element *update_u, *temp;

    /* enqueue zones */

    for (i = 0; i <= top_of_zone_table; i++)
    {
	if (zone_table[i].age < zone_table[i].lifespan &&
	    zone_table[i].reset_mode)
	    (zone_table[i].age)++;
	else
	    if (zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode)
	    {
	    /* enqueue zone */

	    CREATE(update_u, struct reset_q_element, 1);

	    update_u->zone_to_reset = i;
	    update_u->next = 0;

	    if (!reset_q.head)
		reset_q.head = reset_q.tail = update_u;
	    else
	    {
		reset_q.tail->next = update_u;
		reset_q.tail = update_u;
	    }

	    zone_table[i].age = ZO_DEAD;
	    }
    }

    /* dequeue zones (if possible) and reset */

    for(i=0, update_u = reset_q.head; update_u; update_u = update_u->next) 
	if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
	    is_empty(update_u->zone_to_reset))
	{
	reset_zone(update_u->zone_to_reset);

	/* dequeue */

	if (update_u == reset_q.head)
	    reset_q.head = reset_q.head->next;
	else
	{
	    for (temp = reset_q.head; temp->next != update_u;
		temp = temp->next);

	    if (!update_u->next)
		reset_q.tail = temp;

	    temp->next = update_u->next;


	}

	free(update_u);
	/* Thanks to Brian aka Benedict of Mudde Pathetique for this */
	/* We used to just "break;" here. But with so many zones, */
	/* things started to bunch up. So now we try to dequeue more */
	/* than one. */
	if(i++ >= MAX_ZONES_PER_PASS) break;
	} 
}




#define ZCMD zone_table[zone].cmd[cmd_no]

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
    int cmd_no =0, last_cmd = 1;
    char buf[256];
    struct char_data *mob;
    struct obj_data *obj, *obj_to;
return;

    for (cmd_no = 0;;cmd_no++)
    {
	if (ZCMD.command == 'S')
	    break;

	if (last_cmd || !ZCMD.if_flag)
	    switch(ZCMD.command) {
	    case 'M': /* read a mobile */
		if (mob_index[ZCMD.arg1].number < ZCMD.arg2)
		{
		    mob = read_mobile(ZCMD.arg1, REAL);
                    if(!mob) {
                       log("BUG: mob not loaded!!");
		       last_cmd = 0;
                    } else {
                       char_to_room(mob, ZCMD.arg3,0);
		       last_cmd = 1;
                    }
		}
		else
		    last_cmd = 0;
	    break;

	    case 'O': /* read an object */
		if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
		if (ZCMD.arg3 >= 0)
		{
		    if (!get_obj_in_list_num(ZCMD.arg1,world[ZCMD.arg3].contents))
		    {
			obj = read_object(ZCMD.arg1, REAL);
			obj_to_room(obj, ZCMD.arg3);
			last_cmd = 1;
		    }
		    else
			last_cmd = 0;
		}
		else
		{
		    obj = read_object(ZCMD.arg1, REAL);
		    obj->in_room = NOWHERE;
		    last_cmd = 1;
		}
		else
		    last_cmd = 0;
	    break;

	    case 'P': /* object to object */
		if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
		{
		    if((obj_to = get_obj_num(ZCMD.arg3))) {
			obj=read_object(ZCMD.arg1,REAL);
			obj_to_obj(obj, obj_to);
			last_cmd=1;
		    } else
			last_cmd=0;
		} else
		    last_cmd=0;
	    break;

	    case 'G': /* obj_to_char */
		if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
		{		
		    obj = read_object(ZCMD.arg1, REAL);
		    obj_to_char(obj, mob);
		    last_cmd = 1;
		}
		else
		    last_cmd = 0;
	    break;

	    case 'E': /* object to equipment list */
		if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
		{		
		    obj = read_object(ZCMD.arg1, REAL);
		    equip_char(mob, obj, ZCMD.arg3);
		    last_cmd = 1;
		}
		else
		    last_cmd = 0;
	    break;

	    case 'T':
		world[ZCMD.arg1].trap=ZCMD.arg2;
		break;

	    default:
		sprintf(buf,"BUG: Undefd cmd in reset table; zone %d cmd %d.",
		    zone, cmd_no);
		log(buf);
		exit(0);
	    break;
	}
	else
	    last_cmd = 0;

    }

    zone_table[zone].age = 0;
}


/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
    struct descriptor_data *i;

    for (i = descriptor_list; i; i = i->next)
	if (!i->connected)
	    if (world[i->character->in_room].zone == zone_nr)
		return(0);

    return(1);
}



/************************************************************************
*  procs of a (more or less) general utility nature			*
********************************************************************** */


/* New version supplied by MERC */

/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl)
{
    char	buf[MAX_STRING_LENGTH];
    char *	pAlloc;
    char *	pBufLast;

    for ( pBufLast = buf; pBufLast < &buf[sizeof(buf)-2]; )
    {
	switch( *pBufLast = getc( fl ) )
	{
	default:
	    pBufLast++;
	    break;

	case EOF:
	    perror( "fread_string: EOF" );
	    exit( 0 );
	    break;

/** Let's see if I like it with this commented out..
	case '\n':
	    while ( pBufLast > buf && isspace(pBufLast[-1]) )
		pBufLast--;
	    *pBufLast++ = '\n';
	    *pBufLast++ = '\r';
	    break;
***/

	case '~':
	    getc( fl );
	    if ( pBufLast == buf )
		pAlloc	= "";
	    else
	    {
		*pBufLast++	= '\0';
		CREATE( pAlloc, char, pBufLast-buf );
		memcpy( pAlloc, buf, pBufLast-buf );
	    }
	    return pAlloc;
	}
    }

    perror( "fread_string: string too long" );
    exit( 0 );
    return( NULL );
}


/* release memory allocated for a char struct */
void free_char(struct char_data *ch)
{
    struct affected_type *af;

    free(GET_NAME(ch));

 	if (ch->player.title)
	free(ch->player.title);
    if (ch->player.short_descr)
	free(ch->player.short_descr);
    if (ch->player.long_descr)
	free(ch->player.long_descr);
    if(ch->player.description)
	free(ch->player.description);


    for (af = ch->affected; af; af = af->next) 
	affect_remove(ch, af);

    free(ch);
}







/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj)
{
    struct extra_descr_data *this, *next_one;

    free(obj->name);
    if(obj->description)
	free(obj->description);
    if(obj->short_description)
	free(obj->short_description);
    if(obj->action_description)
	free(obj->action_description);

    for( this = obj->ex_description ;
	(this != 0);this = next_one )
    {
	next_one = this->next;
	if(this->keyword)
	    free(this->keyword);
	if(this->description)
	    free(this->description);
	free(this);
    }

    free(obj);
}






/* read contents of a text file, and place in buf */
int file_to_string(char *name, char *buf)
{
    FILE *fl;
    char tmp[100];

    *buf = '\0';

    if (!(fl = fopen(name, "r")))
    {
	sprintf(log_buf,"BUG: file_to_string (%s): %s",name,strerror(errno));
        log(log_buf);
	*buf = '\0';
	return(-1);
    }

    do
    {
	fgets(tmp, 99, fl);

	if (!feof(fl))
	{
	    if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH)
	    {
		log("BUGL fl->strng - string too big (file_to_string)");
		*buf = '\0';
		return(-1);
	    }

	    strcat(buf, tmp);
	    *(buf + strlen(buf) + 1) = '\0';
	    *(buf + strlen(buf)) = '\r';
	}
    }
    while (!feof(fl));

    fclose(fl);

    return(0);
}




/* clear some of the the working variables of a char */
void reset_char(struct char_data *ch)
{
    ch->followers = 0;
    ch->master = 0;
    ch->in_room = NOWHERE;
    ch->next = 0;
    ch->next_fighting = 0;
    ch->next_in_room = 0;
    ch->specials.fighting = 0;
    ch->specials.position = POS_STAND;

    if (GET_HIT(ch) <= 0)
	GET_HIT(ch) = 1;
    if (GET_MOVE(ch) <= 0)
	GET_MOVE(ch) = 1;
    if (GET_MANA(ch) <= 0)
	GET_MANA(ch) = 1;
}



/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
struct char_data *bare_char()
{
    struct char_data *ch;

    CREATE(ch,struct char_data,1);
    
    memset((char *)ch, (char)'\0', (int)sizeof(struct char_data));

    CREATE(ch->physical,struct char_phys_data,1);

    memset((char *)ch->physical,(char)'\0',(int)sizeof(struct char_phys_data));

    ch->in_room = NOWHERE;
    ch->specials.was_in_room = NOWHERE;
    ch->specials.position = POS_STAND;

    ch->specials.dispflags = NULL;
    ch->specials.spec[0]=0;
    ch->specials.spec[1]=0;
    ch->specials.spec[2]=0;
    ch->specials.spec[3]=0;
    ch->specials.ovl_timer=0;
    ch->specials.ovl_count=0;
    ch->specials.lights_carried=0;

    GET_AC(ch) = 100;
    if (affected_by_spell(ch, SKILL_ARMOR))
      GET_AC(ch) -= 20;
    if (affected_by_spell(ch, SKILL_BLINDNESS))
      GET_AC(ch) += 40;

    if (ch->physical->max_mana < 100) {
     ch->physical->max_mana = 100;
    } /* if */

    return ch;
}


void clear_object(struct obj_data *obj)
{
    memset((char *)obj, (char)'\0', (int)sizeof(struct obj_data));

    obj->item_number = -1;
    obj->in_room	  = NOWHERE;
}




/* initialize a new guest character */
void init_char(struct char_data *ch)
{
    int i;

    GET_EXP(ch) = 0;

    ch->player.short_descr = 0;
    ch->player.long_descr = 0;
    ch->player.description = 0;

    ch->player.time.birth = time(0);
    ch->player.time.played = 0;
    ch->player.time.logon = time(0);

    for (i = 0; i < MAX_TOUNGE; i++)
    ch->player.talks[i] = 0;

    ch->orgs = NULL;

    ch->skills = NULL;

    for (i = 0; i < 5; i++)
	ch->specials.apply_saving_throw[i] = 0;

    for (i = 0; i < 4; i++)
	ch->specials.spec[i] = 0;
    ch->specials.ovl_timer=0;
    ch->specials.ovl_count=0;

    ch->physical->hunger = 24;
    ch->physical->thirst = 24;
    ch->physical->exhaustion = 24;
    ch->physical->impairment = 0;


/*	SET_BIT(ch->specials.act,ACT_NOSAVE);*/
}

/* player-related stuff... */
void start_player(struct char_data *ch)
{
    int i;
    struct obj_data *obj;

    void advance_level(struct char_data *ch);

    REMOVE_BIT(ch->specials.act,ACT_NOSAVE);

    set_title(ch);
/*shouldn't need now   ch->points.max_hit  = 10; / * These are BASE numbers   */

    advance_level(ch);

    GET_HIT(ch) = hit_limit(ch);
    GET_MANA(ch) = mana_limit(ch);
    GET_MOVE(ch) = move_limit(ch);

    ch->player.time.played = 0;
    ch->player.time.logon = time(0);

    ch->id = nextplayer();

    CREATE(ch->prefs,struct player_prefs,1);
    ch->prefs->flags=PLR_SECURE; /* force secure until the player changes it */
    ch->prefs->pwd[0]='\0';

    for(i=0;i<MAX_DISCON_CMDS;i++)
        ch->prefs->discon[0][i]='\0';

    save_char(ch);

    /* Give the player some minimal stuff --Sman */
    obj=read_object(3010,VIRTUAL); /* A loaf of bread */
    obj_to_char(obj,ch);
    obj=read_object(3102,VIRTUAL); /* A cup of water */
    obj_to_char(obj,ch);
    obj=read_object(90,VIRTUAL); /* Jacket */
    equip_char(ch,obj,WEAR_BODY);
    obj=read_object(91,VIRTUAL); /* Tights */
    equip_char(ch,obj,WEAR_LEGS);
    obj=read_object(3021,VIRTUAL); /* Short sword */
    equip_char(ch,obj,HOLD_HAND1);

    send_to_char("Welcome. This is now your character in Copper DikuMud,\n\rYou can now earn XP, and lots more...\n\r\n\r", ch);

    send_to_char("A disembodied voice tells you 'Take these meager " \
	"items. I wish you\n\rgood fortune in this life. Please be " \
	"courteous to your fellow mudders'\n\r\n\rYou find a map " \
	"and other objects in your hands, and you are clothed\n\r" \
	"in simple garb.\n\r\n\r",ch);

/*	enc = create_challenge(..ch...,DIFF_FAIR);
    char_to_room(ch,enc->???);
    execute_challenge(enc...);*/
}


/* returns the real number of the room with given virtual number */
int real_room(int virtual)
{
    int bot, top, mid;

    bot = 0;
    top = top_of_world;

    /* perform binary search on world-table */
    for (;;)
    {
	mid = (bot + top) / 2;

	if ((world + mid)->number == virtual)
	    return(mid);
	if (bot >= top)
	{
	    fprintf(stderr, "Room %d does not exist in database\n", virtual);
	    return(-1);
	}
	if ((world + mid)->number > virtual)
	    top = mid - 1;
	else
	    bot = mid + 1;
    }
}






/* returns the real number of the monster with given virtual number */
int real_mobile(int virtual)
{
    int bot, top, mid;

    bot = 0;
    top = top_of_mobt;

    /* perform binary search on mob-table */
    for (;;)
    {
	mid = (bot + top) / 2;

	if ((mob_index + mid)->virtual == virtual)
	    return(mid);
	if (bot >= top)
	    return(-1);
	if ((mob_index + mid)->virtual > virtual)
	    top = mid - 1;
	else
	    bot = mid + 1;
    }
}






/* returns the real number of the object with given virtual number */
int real_object(int virtual)
{
    int bot, top, mid;

    bot = 0;
    top = top_of_objt;

    /* perform binary search on obj-table */
    for (;;)
    {
	mid = (bot + top) / 2;

	if ((obj_index + mid)->virtual == virtual)
	    return(mid);
	if (bot >= top)
	    return(-1);
	if ((obj_index + mid)->virtual > virtual)
	    top = mid - 1;
	else
	    bot = mid + 1;
    }
}

int last[3] = {0,0,0};

void boot_id()
{
    FILE *fl;
    char buf[82],directive[82];
    int num;

    if(!(fl=fopen(ID_FILE,"r"))) {
        fprintf(stderr,"Cannot open id file.\n");
        exit(1);
    }

    while(fgets(buf,81,fl)) {
        sscanf(buf,"%s %d",directive,&num);
        if(!strcmp(directive,"mob:"))
            last[ID_MOB]=num;
        else if(!strcmp(directive,"player:"))
            last[ID_PLAYER]=num;
        else if(!strcmp(directive,"obj:"))
            last[ID_OBJ]=num;
    }

    if(!last[ID_MOB] || !last[ID_PLAYER] || !last[ID_OBJ]) {
        fprintf(stderr,"Missing proper match in id file.\n");
        exit(1);
    }
    fclose(fl);
}

void update_id()
{
    FILE *fl;

    if(dont_save_ids) /* So we don't waste time during boot */
        return;

    if(!(fl=fopen(ID_FILE,"w"))) {
        fprintf(stderr,"Cannot open id file.\n");
        exit(1);
    }

    fprintf(fl,"mob: %d\nplayer: %d\nobj: %d\n",last[ID_MOB],last[ID_PLAYER],last[ID_OBJ]);

    fclose(fl);
}

int next_id(int kind)
{
    if(kind==ID_MOB)
        last[kind]--;
    else
        last[kind]++;
    update_id();
    return last[kind];
}
