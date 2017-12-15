/**********************************************************************
* Weather.h -- Definitions for new zone-based weather routines. Made  
* 12/23/91 by Smandoggi/Dbra/abradfor. This is for DikuMUD, and whatever 
* legal things that apply to it, apply here as well, so there.  
*********************************************************************/

#ifndef WEATHER_H
#define WEATHER_H

/* Season patterns */
#define ONE_SEASON 1
#define TWO_SEASONS_EQUAL 2
#define TWO_SEASONS_FIRST_LONG 3
#define TWO_SEASONS_SECOND_LONG 4
#define THREE_SEASONS_EQUAL 5
#define FOUR_SEASONS_EQUAL 6
#define FOUR_SEASONS_EVEN_LONG 7
#define FOUR_SEASONS_ODD_LONG 8

#define MAX_SEASONS 4

/* Seasonal Wind characteristics */
#define SEASON_CALM 1
#define SEASON_BREEZY 2
#define SEASON_UNSETTLED 3
#define SEASON_WINDY 4
#define SEASON_CHINOOK 5
#define SEASON_VIOLENT 6
#define SEASON_HURRICANE 7

/* Seasonal Precipitation characteristics */
#define SEASON_NO_PRECIP_EVER 1
#define SEASON_ARID 2
#define SEASON_DRY 3
#define SEASON_LOW_PRECIP 4
#define SEASON_AVG_PRECIP 5
#define SEASON_HIGH_PRECIP 6
#define SEASON_STORMY 7
#define SEASON_TORRENT 8
#define SEASON_CONSTANT_PRECIP 9  /* Interesting if it's cold enough to snow */

/* Seasonal Temperature characteristics */
#define SEASON_FROSTBITE 1     /* Need to keep warm...*/
#define SEASON_NIPPY 2
#define SEASON_FREEZING 3
#define SEASON_COLD 4
#define SEASON_COOL 5
#define SEASON_MILD 6
#define SEASON_WARM 7
#define SEASON_HOT 8
#define SEASON_BLUSTERY 9
#define SEASON_HEATSTROKE 10   /* Definite HP loss for these two */
#define SEASON_BOILING 11

/* Flags */
#define NO_MOON_EVER 1
#define NO_SUN_EVER 2
#define NON_CONTROLLABLE 4
#define AFFECTS_INDOORS 8

struct climate {
    int season_pattern;
    int season_wind[MAX_SEASONS];
    int season_wind_dir[MAX_SEASONS];
    int season_wind_variance[MAX_SEASONS];
    int season_precip[MAX_SEASONS];
    int season_temp[MAX_SEASONS];
    int flags;
    signed int energy_add;
};


/* Weather flags */
#define MOON_VISIBLE 1
#define SUN_VISIBLE 2
#define WEATHER_CONTROLLED 4        /* Idea for expansion */

struct weather_data {
    signed int temp; /* In Celsius... So what if I'm a yankee, I */
		  /* still prefer the metric system */
    signed int humidity;
    signed int precip_rate;
    int  windspeed;
    int wind_dir;
    int pressure;               /* Kept from previous system */
    int ambient_light;         /* Interaction between sun, moon, */
		  /* clouds, etc. Local lights ignored */
    int free_energy;
    int flags;
    int precip_depth; /* Snowpack, flood level */
    int pressure_change,precip_change;
};

#endif /* !defined(WEATHER_H) */
