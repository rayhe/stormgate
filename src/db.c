/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  Envy Diku Mud improvements copyright (C) 1994 by Michael Quan, David   *
 *  Love, Guilherme 'Willie' Arnold, and Mitchell Tse.                     *
 *                                                                         *
 *  In order to use any part of this Envy Diku Mud, you must comply with   *
 *  the original Diku license in 'license.doc', the Merc license in        *
 *  'license.txt', as well as the Envy license in 'license.nvy'.           *
 *  In particular, you may not remove either of these copyright notices.   *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*$Id: db.c,v 1.57 2005/04/14 14:21:50 ahsile Exp $*/

#ifdef RUN_AS_WIN32SERVICE
#include <windows.h>
#endif

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

#if !defined( macintosh )
extern  int     _filbuf	        args( (FILE *) );
#endif

#if !defined( ultrix )
#include <memory.h>
#endif

#ifdef SQL_SYSTEM
#include "/usr/local/mysql/include/mysql.h"

extern int boot_time;
extern int query_count;

/* Helper SQL Functions */
bool  sql_system_init(char* DBName, char* User, char* Pass, char* Host);
MYSQL_RES* do_query( char* query );
MYSQL_ROW get_row( MYSQL_RES* res);
int rowcount( MYSQL_RES* res );
void cleanup_query( MYSQL_RES* res );

#endif

/*
 * Globals.
 */
HELP_DATA *		help_first;
HELP_DATA *		help_last;

SHOP_DATA *		shop_first;
SHOP_DATA *		shop_last;

CHAR_DATA *		char_free;
EXTRA_DESCR_DATA *	extra_descr_free;
NOTE_DATA *		note_free;
OBJ_DATA *		obj_free;
PC_DATA *		pcdata_free;

CORRUPT_AREA_LIST *	corrupt;

char                    bug_buf                 [ MAX_INPUT_LENGTH*2 ];
CHAR_DATA *		char_list;
ROOM_INDEX_DATA*	timed_room_list;
char *			help_greeting_one;
char *			help_greeting_two;
char *			help_greeting_three;
char * 			help_greeting_four;
char * 			help_greeting_five;
char	                log_buf                 [ MAX_INPUT_LENGTH*2 ];
KILL_DATA	        kill_table              [ MAX_LEVEL          ];
NOTE_DATA *		note_list;
OBJ_DATA *		object_list;
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;
DEAD_OBJ_LIST *        dead_object_list;
bool                    DeadObjPrntOnly;


char *                  down_time;
char *                  warning1;
char *                  warning2;
int                     stype;
int                     port;

/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash	        [ MAX_KEY_HASH       ];
OBJ_INDEX_DATA *	obj_index_hash	        [ MAX_KEY_HASH       ];
ROOM_INDEX_DATA *	room_index_hash         [ MAX_KEY_HASH       ];

AREA_DATA *		area_first;
AREA_DATA *		area_last;
CLAN_DATA *             clan_first;
RELIGION_DATA *	religion_first;

char *			string_space;
char *			top_string;
char			str_empty	        [ 1                  ];

int			top_ed;
int			top_vnum_room;	/* OLC */
int			top_room;
int			top_area;
int			top_mob_index;
int			top_vnum_mob;	/* OLC */
int			top_vnum_obj;   /* OLC */
int 		 	mprog_name_to_type	args ( ( char* name ) );
MPROG_DATA *		mprog_file_read 	args ( ( char* f, MPROG_DATA* mprg, 
						MOB_INDEX_DATA *pMobIndex ) );
void			load_mobprogs           args ( ( FILE* fp ) );
void   			mprog_read_programs     args ( ( FILE* fp,
					        MOB_INDEX_DATA *pMobIndex ) );
void                    load_traps              args ( ( FILE* fp,
							OBJ_INDEX_DATA *pObj,
							ROOM_INDEX_DATA *pRoom,
							EXIT_DATA *pExit ) );
char *			wind_str		args ( ( int str ) );
void                    area_sort               args ( ( AREA_DATA *pArea ) );
void                    clan_sort               args ( ( CLAN_DATA *pClan ) );
void				religion_sort		args ( ( RELIGION_DATA *pReligion ) );

/*
 * Memory management.
 * Increase MAX_STRING from 1500000 if you have too.
 * Tune the others only if you understand what you're doing.
 */


#if defined( machintosh )
#define			MAX_PERM_BLOCK  131072
#define			MAX_MEM_LIST    11

void *			rgFreeList              [ MAX_MEM_LIST       ];
const int		rgSizeList              [ MAX_MEM_LIST       ]  =
{
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768-64
};
#else
#define			MAX_PERM_BLOCK  131072
#define			MAX_MEM_LIST    12

void *			rgFreeList              [ MAX_MEM_LIST       ];
const int		rgSizeList              [ MAX_MEM_LIST       ]  =
{
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768, 65536
};
#endif


int			nAllocString;
int			sAllocString;
int			nAllocPerm;
int			sAllocPerm;



/*
 * Semi-locals.
 */
FILE *			fpArea;
char			strArea                 [ MAX_INPUT_LENGTH   ];
bool 			quest = FALSE;
int			qmin = 0;
int			qmax = 100;
extern  int MAX_STRING;
void    init_string_space       args( ( void ) );
char *  daPrompt;

/*
 * Local booting procedures.
 */
void	init_mm		args( ( void ) );

void	load_area       args( ( FILE *fp ) );
void	load_helps      args( ( FILE *fp ) );
void    load_recall     args( ( FILE *fp ) );
void	load_mobiles    args( ( FILE *fp ) );
void	load_objects    args( ( FILE *fp ) );
void	load_resets     args( ( FILE *fp ) );
void	load_rooms      args( ( FILE *fp ) );
void	load_shops      args( ( FILE *fp ) );
void	load_specials   args( ( FILE *fp ) );
void    load_games      args( ( FILE *fp ) );
void	load_notes      args( ( void ) );
void    load_clans      args( ( void ) );
void	load_religions	args( ( void ) );
void    load_down_time  args( ( void ) );
void	load_permanent_objects args( ( void ) );
void	fix_exits       args( ( void ) );
void    load_dead_obj_info args( (void ) );
void	reset_area      args( ( AREA_DATA * pArea ) );
extern void    userl_load      args( ( void ) );



/*
 * Non-Envy Loading procedures.
 * Put any new loading function in this section.
 */
void	new_load_area	args( ( FILE *fp ) );	/* OLC */
void	new_load_rooms	args( ( FILE *fp ) );	/* OLC 1.1b */

void	load_banlist ( void );			/* 06/17/96 REK */

/*
 * Big mama top level function.
 */



void boot_db( void )
{
char buf[MAX_STRING_LENGTH];
int debug = 0;

	pool_first = NULL;
	pool_last  = NULL;
   
	corrupt = NULL;

    /*
     * Init some data space stuff.
     */
    {
        init_string_space();
    }

    /*
     * Init random number generator.
     */
    {
	init_mm( );
    }

    /*
     * Set time and weather.
     */
    {
/*
	long lhour, lday, lmonth;

	lhour		= ( current_time - 650336715 )
			   / ( PULSE_TICK / PULSE_PER_SECOND );
	time_info.hour  = lhour  % 24;
	lday		= lhour  / 24;
	time_info.day	= lday   % 35;
	lmonth		= lday   / 35;
	time_info.month	= lmonth % 17;
	time_info.year	= lmonth / 17;
	time_info.total = 0;
*/

	time_info.hour = 0;
	time_info.day  = 0;
	time_info.year = ((((( current_time - 650336715 )
                           / ( PULSE_TICK / PULSE_PER_SECOND )) % 24) % 30) %17);


	     if ( time_info.hour <  5 ) weather_info.sunlight = SUN_DARK;
	else if ( time_info.hour <  6 ) weather_info.sunlight = SUN_RISE;
	else if ( time_info.hour < 19 ) weather_info.sunlight = SUN_LIGHT;
	else if ( time_info.hour < 20 ) weather_info.sunlight = SUN_SET;
	else                            weather_info.sunlight = SUN_DARK;

	weather_info.change	= 0;
	weather_info.mmhg	= 960;
	if ( time_info.month >= 7 && time_info.month <=12 )
	    weather_info.mmhg += number_range( 1, 50 );
	else
	    weather_info.mmhg += number_range( 1, 80 );

	     if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
	else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
	else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
	else                                  weather_info.sky = SKY_CLOUDLESS;

    }

    /*
     * Assign gsn's for skills which have them.
     */
    {
	int sn;

	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( skill_table[sn].pgsn )
		*skill_table[sn].pgsn = sn;
	}
    }

#ifdef SQL_SYSTEM

	sql_system_init(DBNAME, USER, PASS, HOST);
	log_string("Loading areas", CHANNEL_NONE, -1);
	sql_load_areas();
	log_string("Loading help data", CHANNEL_NONE, -1);
	sql_load_helps();
	log_string("Loading resets", CHANNEL_NONE, -1);
	sql_load_resets();
	log_string("Loading shops", CHANNEL_NONE, -1);
	sql_load_shops();
	log_string("Loading social table", CHANNEL_NONE, -1);
	sql_load_socials();
	log_string("Loading clan table", CHANNEL_NONE, -1);
	sql_load_clans();
	log_string("Loading religion table", CHANNEL_NONE, -1);
	sql_load_religions();
	log_string("Loading ban list", CHANNEL_NONE, -1);
	sql_load_bans();
	log_string("Loading disabled list", CHANNEL_NONE, -1);
	sql_load_disabled();
	
	fix_exits();
	area_update();

	userl_load();	// need to do later
	load_notes();	// need to do later

	load_down_time(); // just do it


	create_fastinv_tables();

	sprintf(buf, "Booted SQLDB in %d seconds", (int)(time(NULL)-boot_time));
	log_string(buf, CHANNEL_NONE, -1);
	sprintf(buf, "Executed %d queries on server %s", query_count, HOST);
	log_string(buf, CHANNEL_NONE, -1);
	sprintf(buf, "Queries/second: %d", (int)(query_count/(time(NULL)-boot_time)));
	log_string(buf, CHANNEL_NONE, -1);

#else
	/*
     * Read in all the socials.
     */
     {

        log_string ("Loading socials", CHANNEL_NONE, -1);
		load_social_table ( );
     }
    /*
     * Read in all the area files.
     */
    {
	FILE *fpList;

	if ( !( fpList = fopen( ( AREA_LIST), "r" ) ) )
	{
	    perror( AREA_LIST );
	    exit( 1 );
	}

	for ( ; ; )
	{
	    strcpy( strArea, fread_word( fpList ) );
if(debug)
{
sprintf(buf, "loading area %s", strArea);
bug(buf, 0);
}
	    if ( strArea[0] == '$' )
		break;

	    if ( strArea[0] == '-' )
	    {
		fpArea = stdin;
	    }
	    else
	    {
		if ( !( fpArea = fopen( strArea, "r" ) ) )
		{
		    perror( strArea );
		    exit( 1 );
		}
	    }

	    for ( ; ; )
	    {
		char *word;

		if ( fread_letter( fpArea ) != '#' )
		{
		    bug( "Boot_db: # not found.", 0 );
		    exit( 1 );
		}

		word = fread_word( fpArea );

		     if ( word[0] == '$'               )
                    break;
		else if ( !str_cmp( word, "AREA"     ) )
		{
			if (debug)
				bug("Loading area section.",0);
			load_area    ( fpArea );
		}
		else if ( !str_cmp( word, "HELPS"    ) )
		{
			if (debug)
				bug("Loading help section.",0);
		    load_helps   ( fpArea );
		}
		else if ( !str_cmp( word, "RECALL"   ) )
		    load_recall  ( fpArea );
		else if ( !str_cmp( word, "MOBILES"  ) )
	    	     load_mobiles ( fpArea );
	        else if ( !str_cmp( word, "MOBPROGS" ) )
	            load_mobprogs( fpArea );
		else if ( !str_cmp( word, "OBJECTS"  ) )
		    load_objects ( fpArea );
		else if ( !str_cmp( word, "RESETS"   ) )
		    load_resets  ( fpArea );
		else if ( !str_cmp( word, "ROOMS"    ) )
		    load_rooms   ( fpArea );
		else if ( !str_cmp( word, "SHOPS"    ) )
		    load_shops   ( fpArea );
		else if ( !str_cmp( word, "SPECIALS" ) )
		    load_specials( fpArea );
                else if ( !str_cmp( word, "GAMES"    ) )
                    load_games ( fpArea );
		else if ( !str_cmp( word, "AREADATA" ) )	/* OLC */
		    new_load_area( fpArea );
		else if ( !str_cmp( word, "ROOMDATA" ) )	/* OLC 1.1b */
		    new_load_rooms( fpArea );
		else
		{
		    bug( "Boot_db: bad section name.", 0 );
		    exit( 1 );
		}
	    }

	    /* We're finished reading the area. Set its version
	     * equal to the current version, and mark it changed
	     * so it saves.
	     *    - Ahsile
	     */ 
    	    if (area_last && area_last->version < CURRENT_AREA_VERSION)
    	    {
		char buf[MAX_STRING_LENGTH];
		sprintf(buf, "Updating %s from version %d to %d", area_last->name, area_last->version, CURRENT_AREA_VERSION);
		log_string( buf, CHANNEL_NONE, -1);
		area_last->version = CURRENT_AREA_VERSION;
        	SET_BIT(area_last->area_flags, AREA_CHANGED);
    	    }


	{
	  char buf[MAX_STRING_LENGTH];
	  sprintf(buf, "loading %s...", strArea );
	  log_string( buf, CHANNEL_NONE , -1 );
	}

	    if ( fpArea != stdin )
	    {
		fclose( fpArea );
	    } else
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf(buf, "Couldn't close %s...", strArea);
		log_string( buf, CHANNEL_NONE, -1);
	    }
	    fpArea = NULL;
	}
	fclose( fpList );
    }

    /*
     * Fix up exits.
     * Declare db booting over.
     * Reset all areas once.
     * Load up the notes file.
     * load the banlist.
     */
     {
       fix_exits( );
       load_clans( );
	 load_religions( );
       area_update( );
         userl_load( );
       load_notes( );
       log_string ("Loading disabled commands...", CHANNEL_NONE, -1);
       load_disabled( );
       load_banlist( );
       load_dead_obj_info();
       load_down_time( );
       load_permanent_objects( );

}
	 #endif

MOBtrigger = TRUE;
return;
}

/* Snarf a MOBprogram section from the area file.
 */
void load_mobprogs( FILE *fp )
{
  MOB_INDEX_DATA *iMob;
  MPROG_DATA     *original;
  MPROG_DATA     *working;
  char            letter;
  int             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_mobprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp ); 
      return;
    case '*':
      fread_to_eol( fp ); 
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iMob = get_mob_index( value ) ) == NULL )
      {
	bug( "Load_mobprogs: vnum %d doesnt exist", value );
	exit( 1 );
      }
    
      /* Go to the end of the prog command list if other commands
         exist */

      if ( ( original = iMob->mobprogs ) )
	for ( ; original->next != NULL; original = original->next );

      working = new_mprog();
      if ( original )
	original->next = working;
      else
	iMob->mobprogs = working;
      working       = mprog_file_read( fread_word( fp ), working, iMob );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

} 

int mprog_name_to_type ( char *name )
{
   if ( !str_cmp( name, "in_file_prog"   ) )	return IN_FILE_PROG;
   if ( !str_cmp( name, "act_prog"       ) )    return ACT_PROG;
   if ( !str_cmp( name, "speech_prog"    ) )	return SPEECH_PROG;
   if ( !str_cmp( name, "rand_prog"      ) ) 	return RAND_PROG;
   if ( !str_cmp( name, "fight_prog"     ) )	return FIGHT_PROG;
   if ( !str_cmp( name, "hitprcnt_prog"  ) )	return HITPRCNT_PROG;
   if ( !str_cmp( name, "death_prog"     ) )	return DEATH_PROG;
   if ( !str_cmp( name, "entry_prog"     ) )	return ENTRY_PROG;
   if ( !str_cmp( name, "greet_prog"     ) )	return GREET_PROG;
   if ( !str_cmp( name, "all_greet_prog" ) )	return ALL_GREET_PROG;
   if ( !str_cmp( name, "give_prog"      ) ) 	return GIVE_PROG;
   if ( !str_cmp( name, "bribe_prog"     ) )	return BRIBE_PROG;

   return( ERROR_PROG );
}

/* This routine reads in scripts of MOBprograms from a file */

MPROG_DATA* mprog_file_read( char *f, MPROG_DATA *mprg,
			    MOB_INDEX_DATA *pMobIndex )
{

  char        MOBProgfile[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg2;
  FILE       *progfile;
  char        letter;
  bool        done = FALSE;

/*  sprintf( MOBProgfile, "%s%s", MOB_DIR, f );
*/
  progfile = fopen( MOBProgfile, "r" );
  if ( !progfile )
  {
     bug( "Mob: %d couldnt open mobprog file", pMobIndex->vnum );
     exit( 1 );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty mobprog file.", 0 );
       exit( 1 );
     break;
    default:
       bug( "in mobprog file syntax error.", 0 );
       exit( 1 );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
        bug( "mobprog file type error", 0 );
        exit( 1 );
      break;
     case IN_FILE_PROG:
        bug( "mprog file contains a call to file.", 0 );
        exit( 1 );
      break;
     default:
        pMobIndex->progtypes = pMobIndex->progtypes | mprg2->type;
        mprg2->arglist       = fread_string( progfile );
        mprg2->comlist       = fread_string( progfile );
        switch ( letter = fread_letter( progfile ) )
        {
          case '>':
             mprg2->next = new_mprog();
             mprg2       = mprg2->next;
             mprg2->next = NULL;
           break;
          case '|':
             done = TRUE;
           break;
          default:
             bug( "in mobprog file syntax error.", 0 );
             exit( 1 );
           break;
        }
      break;
    }
  }
  fclose( progfile );
  return mprg2;
}
/* This procedure is responsible for reading any in_file MOBprograms.
 */

void mprog_read_programs( FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
  MPROG_DATA *mprg;
  char        letter;
  bool        done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Mprog_read_programs: vnum %d MOBPROG char", pMobIndex->vnum );
      exit( 1 );
  }
  pMobIndex->mobprogs = new_mprog();
  mprg = pMobIndex->mobprogs;
  mprg->status = 0;
  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
        bug( "Load_mobiles: vnum %d MOBPROG type.", pMobIndex->vnum );
        exit( 1 );
      break;
     case IN_FILE_PROG:
        mprg = mprog_file_read( fread_string( fp ), mprg,pMobIndex );
        fread_to_eol( fp );
        switch ( letter = fread_letter( fp ) )
        {
          case '>':
             mprg->next = new_mprog();
             mprg       = mprg->next;
             mprg->next = NULL;
           break;
          case '|':
             mprg->next = NULL;
             fread_to_eol( fp );
             done = TRUE;
           break;
          default:
             bug( "Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->vnum );
             exit( 1 );
           break;
        }
      break;
     default:
        pMobIndex->progtypes = pMobIndex->progtypes | mprg->type;
        mprg->arglist        = fread_string( fp );
        fread_to_eol( fp );
        mprg->comlist        = fread_string( fp );
        fread_to_eol( fp );
        switch ( letter = fread_letter( fp ) )
        {
          case '>':
             mprg->next = new_mprog();
             mprg       = mprg->next;
             mprg->next = NULL;
           break;
          case '|':
             mprg->next = NULL;
             fread_to_eol( fp );
             done = TRUE;
           break;
          default:
             bug( "Load_mobiles: vnum %d bad MOBPROG.", pMobIndex->vnum );
             exit( 1 );
           break;
        }
      break;
    }
  }

  return;

}

void load_traps( FILE *fp, OBJ_INDEX_DATA *pObj, ROOM_INDEX_DATA *pRoom,
	         EXIT_DATA *pExit )
{
  TRAP_DATA *pTrap;
  TRAP_DATA *pFirst;
  int *traptypes;
  char letter;
  bool done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
    bug("Load_traps:  No Traps", 0);
    exit(1);
  }

  if ( !trap_list )
  {
    trap_list = new_trap();
    pFirst = trap_list;
  }
  else
  {
    for( pFirst = trap_list; pFirst->next; pFirst = pFirst->next );
    pFirst->next = new_trap();
    pFirst = pFirst->next;
  }
  pFirst->next = NULL;
  pFirst->next_here = NULL;
    
  if ( pObj )
  {
    pObj->traps = pFirst;
    traptypes = &pObj->traptypes;
  }
  else if ( pRoom )
  {
    pRoom->traps = pFirst;
    traptypes = &pRoom->traptypes;
  }
  else if ( pExit )
  {
    pExit->traps = pFirst;
    traptypes = &pExit->traptypes;
  }
  else
  {
    bug("Load_traps:  Nothing to load to!", 0);
    exit( 1 );
  }
  pTrap = pFirst;

  while( !done )
  {
    pTrap->type = flag_value( (pObj ? oprog_types : (pRoom ? rprog_types :
			       eprog_types)), fread_word( fp ) );
    switch( pTrap->type )
    {
    //case (pObj ? OBJ_TRAP_ERROR : (pRoom ? ROOM_TRAP_ERROR : EXIT_TRAP_ERROR)):
	case 0:
    case NO_FLAG:
      bug( "Load_traps: No flag found.", 0 );
      exit( 1 );
    default:
      pTrap->on_obj = pObj;
      pTrap->in_room = pRoom;
      pTrap->on_exit = pExit;
      *traptypes |= pTrap->type;
      pTrap->arglist = fread_string( fp );
      fread_to_eol( fp );
      pTrap->disarmable = fread_number( fp );
      fread_to_eol( fp );
      pTrap->comlist = fread_string( fp );
      fread_to_eol( fp );
      switch ( letter = fread_letter( fp ) )
      {
      case '>':
	pTrap->next = new_trap();
	pTrap->next_here = pTrap->next;
	pTrap = pTrap->next;
	pTrap->next = NULL;
	pTrap->next_here = NULL;
	break;
      case '|':
	pTrap->next = NULL;
	pTrap->next_here = NULL;
	fread_to_eol( fp );
	done = TRUE;
	break;
      default:
	bug( "Load_traps:  bad TRAP", 0);
	break;
      }
      break;
    }
  }
  return;
}


/*
 * Snarf an 'area' header line.
 */
void load_area( FILE *fp )
{
    AREA_DATA *pArea;

    pArea		= new_area();
    pArea->reset_first	= NULL;					 
    pArea->reset_last	= NULL;					 
    pArea->name		= fread_string( fp );
    pArea->recall       = ROOM_VNUM_TEMPLE;
    pArea->area_flags   = AREA_LOADING;		/* OLC */
    pArea->security     = 1;			/* OLC */
    pArea->builders     = str_dup( "None" );	/* OLC */
    pArea->lvnum        = 0;			/* OLC */
    pArea->uvnum        = 0;			/* OLC */
    pArea->vnum		= top_area;		/* OLC */
    pArea->filename	= str_dup( strArea );	/* OLC */
    pArea->age		= 15;
    pArea->nplayer	= 0;

    area_sort(pArea);

    top_area++;
    return;
}



/*
 * OLC
 * Use these macros to load any new area formats that you choose to
 * support on your MUD.  See the new_load_area format below for
 * a short example.
 */
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                \
		if ( !str_cmp( word, literal ) )    \
                {                                   \
                    field  = value;                 \
                    fMatch = TRUE;                  \
                    break;                          \
		}

#define SKEY( string, field )                       \
                if ( !str_cmp( word, string ) )     \
                {                                   \
                    free_string( field );           \
                    field = fread_string( fp );     \
                    fMatch = TRUE;                  \
                    break;                          \
		}



/* OLC
 * Snarf an 'area' header line.   Check this format.  MUCH better.  Add fields
 * too.
 *
 * #AREAFILE
 * Name   { All } Locke    Newbie School~
 * Repop  A teacher pops in the room and says, 'Repop coming!'~
 * Recall 3001
 * End
 */
void new_load_area( FILE *fp )
{
    AREA_DATA *pArea;
    char      *word;
    bool      fMatch;

    pArea               = new_area( );
    pArea->age          = 15;
    pArea->nplayer      = 0;
    pArea->filename     = str_dup( strArea );
    pArea->vnum         = top_area;
    pArea->name         = str_dup( "New Area" );
    pArea->builders     = str_dup( "" );
    pArea->reset_sound  = NULL;
    pArea->security     = 1;
    pArea->lvnum        = 0;
    pArea->uvnum        = 0;
    pArea->area_flags   = 0;
    pArea->version	= 100; /* 1.00 default */
    pArea->creator	= str_dup( "None" );
    pArea->llevel	= 0;
    pArea->ulevel	= 0;
    pArea->recall       = ROOM_VNUM_TEMPLE;
    pArea->actual_sound = NULL;
    pArea->musicfile    = NULL;
	pArea->status       = 1;    /* for Area Theme Quest status */	
    for ( ; ; )
    {
       word   = feof( fp ) ? "End" : fread_word( fp );
       fMatch = FALSE;

       switch ( UPPER(word[0]) )
       {
           case 'N':
            SKEY( "Name", pArea->name );
            break;
           case 'S':
             KEY( "Security", pArea->security, fread_number( fp ) );
	    SKEY( "Sounds", pArea->reset_sound );
	    SKEY( "SoundsA", pArea->actual_sound );
	KEY( "Status", pArea->status, fread_number(fp) );

            break;
           case 'V':
            KEY( "Version", pArea->version, fread_number( fp ) );
            if ( !str_cmp( word, "VNUMs" ) )
            {
                pArea->lvnum = fread_number( fp );
                pArea->uvnum = fread_number( fp );
            }
            break;
           case 'F':
            KEY( "Flags", pArea->area_flags, fread_number( fp ) );
            break;
           case 'M':
            SKEY( "Music", pArea->musicfile );
            break;
           case 'E':
             if ( !str_cmp( word, "End" ) )
             {
                 fMatch = TRUE;
		 area_sort(pArea);
                 top_area++;
                 return;
            }
            break;
	   case 'L':
		if (pArea->version >= 107)
		{
			KEY( "Llevel", pArea->llevel, fread_number( fp ) );
		}
		else
		{
			/* nothing, use default */
		}
	    break;
	   case 'U':
		if (pArea->version >= 107) 
		{
			KEY("Ulevel", pArea->ulevel, fread_number(fp) );
		}
		else
		{
			/* nothing, use default */
		}
	    break;
           case 'B':
            SKEY( "Builders", pArea->builders );
            break;
           case 'R':
             KEY( "Recall", pArea->recall, fread_number( fp ) );
            break;
	   case 'C':
		if (pArea->version >= 102) /* 1.02 */
		{
			SKEY( "Creator", pArea->creator);
		} else
		{
			/* nothing, use the deafaul above */
		}
/*
 *
 *         - This is how version is done 
 *	   - Ahsile
 *
	   case '?':		
    		if (pArea->version == 100) // 1.00
    		{
		        pArea->newfield = someDefaultValue;
    		} else
		{
			KEY("NewKey", pArea->newfield, fread_number( fp ) );
		}
*/
	}
    }
}



/*
 * Sets vnum range for area using OLC protection features.
 */
void assign_area_vnum( int vnum )
{
    if ( area_last->lvnum == 0 || area_last->uvnum == 0 )
	area_last->lvnum = area_last->uvnum = vnum;
    if ( vnum != URANGE( area_last->lvnum, vnum, area_last->uvnum ) )
    {
	if ( vnum < area_last->lvnum )
	    area_last->lvnum = vnum;
	else
	    area_last->uvnum = vnum;
    }
    return;
}



/*
 * Snarf a help section.
 */
void load_helps( FILE *fp )
{
    HELP_DATA *pHelp;

    for ( ; ; )
    {
	pHelp		= new_help();
	pHelp->area	= area_last ? area_last : NULL;		/* OLC 1.1b */
	pHelp->level	= fread_number( fp );
	pHelp->keyword	= fread_string( fp );
	if ( pHelp->keyword[0] == '$' )
	    break;
	pHelp->text	= fread_string( fp );
	
	if ( !str_cmp( pHelp->keyword, "greeting1" ) )
	    help_greeting_one = pHelp->text;
        if ( !str_cmp( pHelp->keyword, "greeting2" ) )
	    help_greeting_two = pHelp->text;
        if ( !str_cmp( pHelp->keyword, "greeting3" ) )
	    help_greeting_three = pHelp->text;
	if ( !str_cmp( pHelp->keyword, "greeting4" ) )
	    help_greeting_four = pHelp->text;
	if ( !str_cmp( pHelp->keyword, "greeting5" ) )
	    help_greeting_five = pHelp->text;

	if ( !help_first )
	    help_first = pHelp;
	if (  help_last  )
	    help_last->next = pHelp;

	help_last	= pHelp;
	pHelp->next	= NULL;
	top_help++;
    }

    return;
}



/*
 * Snarf a recall point.
 */
void load_recall( FILE *fp )
{
    AREA_DATA *pArea;
    char       buf [ MAX_STRING_LENGTH ];

    pArea         = area_last;
    pArea->recall = fread_number( fp );

    if ( pArea->recall < 1 )
    {
        sprintf( buf, "Load_recall:  %s invalid recall point", pArea->name );
	bug( buf, 0 );
	pArea->recall = ROOM_VNUM_TEMPLE;
    }

    return;

}



/*
 * Snarf a mob section.
 */
void load_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex = NULL;
    char letter;
    int num;

    if ( !area_last )	/* OLC */
    {
	bug( "Load_mobiles: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	int  vnum;
	int  iHash;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_mobiles: # not found.", 0 );
	    exit( 1 );
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	if ( get_mob_index( vnum ) )
	{
	    bug( "Load_mobiles: vnum %d duplicated.", vnum );
	    exit( 1 );
	}

	pMobIndex			= new_mob_index();
	pMobIndex->vnum			= vnum;
	pMobIndex->area			= area_last;		/* OLC */
	pMobIndex->player_name		= fread_string( fp );
	pMobIndex->short_descr		= fread_string( fp );
	pMobIndex->long_descr		= fread_string( fp );
	pMobIndex->description		= fread_string( fp );

	pMobIndex->long_descr[0]	= UPPER( pMobIndex->long_descr[0]  );
	pMobIndex->description[0]	= UPPER( pMobIndex->description[0] );

	pMobIndex->act			= fread_number( fp ) | ACT_IS_NPC;
	pMobIndex->affected_by		= fread_number( fp );
	pMobIndex->affected_by2         = fread_number( fp );
	pMobIndex->affected_by3         = fread_number( fp );
	pMobIndex->affected_by4         = fread_number( fp );
	pMobIndex->pShop		= NULL;
	pMobIndex->alignment		= fread_number( fp );
	letter				= fread_letter( fp );
	pMobIndex->level		= fread_number( fp );

	if (area_last->version >= 103 )
	{
		pMobIndex->size = fread_number( fp );
	} else
	{
		pMobIndex->size = 1;
	}

	/*
	 * The unused stuff is for imps who want to use the old-style
	 * stats-in-files method.
	 */
	pMobIndex->hitroll              = fread_number( fp );
	pMobIndex->damroll		= fread_number( fp );
	pMobIndex->ac			= fread_number( fp );
	pMobIndex->hitnodice            = fread_number( fp );   /* Unused */
	/* 'd'		*/                fread_letter( fp );   /* Unused */
	pMobIndex->hitsizedice          = fread_number( fp );   /* Unused */
	/* '+'		*/                fread_letter( fp );   /* Unused */
	pMobIndex->hitplus		= fread_number( fp );   /* Unused */
	pMobIndex->damnodice            = fread_number( fp );   /* Unused */
	/* 'd'		*/                fread_letter( fp );   /* Unused */
	pMobIndex->damsizedice          = fread_number( fp );   /* Unused */
	/* '+'		*/                fread_letter( fp );   /* Unused */
	pMobIndex->damplus              = fread_number( fp );   /* Unused */
	pMobIndex->gold                 = fread_number( fp );   /* Unused */
	pMobIndex->speaking		= fread_number( fp );   /* Tyrion */
	/* xp           */                fread_number( fp );   /* Unused */
	/* position	*/                fread_number( fp );   /* Unused */
	/* start pos	*/                fread_number( fp );   /* Unused */
	pMobIndex->sex			= fread_number( fp );
	pMobIndex->imm_flags            = fread_number( fp );
	pMobIndex->res_flags            = fread_number( fp );
	pMobIndex->vul_flags            = fread_number( fp );

	if ( letter != 'S' )
	{
	    bug( "Load_mobiles: vnum %d non-S.", vnum );
	    exit( 1 );
	}
        letter = fread_letter( fp );
        if ( letter == '>' )
        {
          ungetc( letter, fp );
          mprog_read_programs( fp, pMobIndex );
        }
        else ungetc( letter,fp );

/* XORPHOX */
/* This stuff is already in there Xor.. *POKE*
 * Grumble.. now we hafta leave it here or it'll cause errors
 * Sheesh.. -- Altrag
 */
	for ( ; ; ) /* only way so far */
	{
	    letter = fread_letter( fp );

	    if ( letter == 'F' )
	    {
              num = fread_number(fp);
              switch(num)
              {
                case 0: /* affected_by2 */
                  pMobIndex->affected_by2	= fread_number(fp);
                break;
                case 1: /* imm_flags */
                  pMobIndex->imm_flags		= fread_number(fp);
                break;
                case 2: /* res_flags */
                  pMobIndex->res_flags		= fread_number(fp);
                break;
                case 3: /* vul_flags */
                  pMobIndex->vul_flags		= fread_number(fp);
                break;
                case 4: /* affected_by3 */
                  pMobIndex->affected_by3	= fread_number(fp);
                break;
                case 5: /* affected_by4 */
                  pMobIndex->affected_by4	= fread_number(fp);
                break;
              }
	    }
	    else
	    {
		ungetc(letter, fp);
		break;
	    }
	}
/* END */

	iHash			= vnum % MAX_KEY_HASH;
	pMobIndex->next		= mob_index_hash[iHash];
	mob_index_hash[iHash]	= pMobIndex;
	top_mob_index++;
	top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  /* OLC */
	assign_area_vnum( vnum );				   /* OLC */
	kill_table[URANGE( 0, pMobIndex->level, MAX_LEVEL-1 )].number++;

	if (area_last && area_last->version <= 104)
    	{
		if ( pMobIndex->gold > (pMobIndex->level * 10) )
			pMobIndex->gold = pMobIndex->level * 10;
    	}

	if (area_last && area_last->version <= 105)
	{
		if (!pMobIndex->damroll)
			pMobIndex->damroll = pMobIndex->level * 2; 
		if (!pMobIndex->hitroll)
			pMobIndex->hitroll = pMobIndex->level * 2; 
	}


    }

    return;
}



/*
 * Snarf an obj section.
 */
void load_objects( FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;

    if ( !area_last )	/* OLC */
    {
	bug( "Load_objects: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
        char *value [ 4 ];
	char  letter;
	int   vnum;
	int   iHash;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_objects: # not found.", 0 );
	    exit( 1 );
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	if ( get_obj_index( vnum ) )
	{
	    bug( "Load_objects: vnum %d duplicated.", vnum );
	    exit( 1 );
	}

	pObjIndex			= new_obj_index();
	pObjIndex->vnum			= vnum;
        pObjIndex->area			= area_last;		/* OLC */
	pObjIndex->name			= fread_string( fp );
	pObjIndex->short_descr		= fread_string( fp );
	pObjIndex->description		= fread_string( fp );
	/* Action description */	  fread_string( fp );

	pObjIndex->short_descr[0]	= LOWER( pObjIndex->short_descr[0] );
	pObjIndex->description[0]	= UPPER( pObjIndex->description[0] );

	pObjIndex->item_type		= fread_number( fp );
	pObjIndex->extra_flags		= fread_number( fp );
	pObjIndex->extra_flags2		= fread_number( fp );
        pObjIndex->extra_flags3         = fread_number( fp );
        pObjIndex->extra_flags4         = fread_number( fp );
	pObjIndex->wear_flags		= fread_number( fp );
        pObjIndex->level                = fread_number( fp );

	if (area_last->version >= 104)
	{
		pObjIndex->durability_max = fread_number( fp );
		pObjIndex->durability_cur = fread_number( fp );
	} else
	{
		pObjIndex->durability_max = 100;
		pObjIndex->durability_cur = 100;
	}

	value[0]		        = fread_string( fp );
	value[1]		        = fread_string( fp );
	value[2]		        = fread_string( fp );
	value[3]		        = fread_string( fp );
	pObjIndex->weight		= fread_number( fp );
	pObjIndex->cost			= fread_number( fp );
	/* Cost per day */		  fread_number( fp );   /* Unused */
	pObjIndex->ac_type              = fread_number( fp );
	pObjIndex->ac_vnum              = fread_number( fp );
	pObjIndex->ac_spell             = fread_string( fp );
	pObjIndex->ac_charge[0]         = fread_number( fp );
	pObjIndex->ac_charge[1]         = fread_number( fp ); 
	pObjIndex->join                 = fread_number( fp );
	pObjIndex->sep_one              = fread_number( fp );
	pObjIndex->sep_two              = fread_number( fp );
	/*
	if (area_last->version == 100) // 1.00
	{
		pObjIndex->new_field = str_dup( "Beefy Default Value" );
	} else
	{
		pObjIndex->new_field = fread_string( fp );
	}
	*/


	/*
	 * Check here for the redundancy of invisible light sources - Kahn
	 */
	if ( pObjIndex->item_type == ITEM_LIGHT
	    && IS_SET( pObjIndex->extra_flags, ITEM_INVIS ) )
	{
	    bug( "Vnum %d : light source with ITEM_INVIS set", vnum );
	    REMOVE_BIT( pObjIndex->extra_flags, ITEM_INVIS );
	}

	if ( pObjIndex->item_type == ITEM_POTION )
	    SET_BIT( pObjIndex->extra_flags, ITEM_NO_LOCATE );

	for ( ; ; )
	{
	    char letter;

	    letter = fread_letter( fp );

	    if ( letter == 'A' )
	    {
		AFFECT_DATA *paf;

		paf			= new_affect();
		paf->type		= -1;
		paf->level		= pObjIndex->level;
		paf->duration		= -1;
		paf->location		= fread_number( fp );
		paf->modifier		= fread_number( fp );
		paf->bitvector		= 0;
		paf->next		= pObjIndex->affected;
		pObjIndex->affected	= paf;
		top_affect++;
	    }

	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		ed			= new_extra_descr();
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= pObjIndex->extra_descr;
		pObjIndex->extra_descr	= ed;
		top_ed++;
	    }

	    else
	    {
		ungetc( letter, fp );
		break;
	    }
	}

	/*
	 * Translate character strings *value[] into integers:  sn's for
	 * items with spells, or straight conversion for other items.
	 * - Thelonius
	 */
	switch ( pObjIndex->item_type )
	{
	default:
	    pObjIndex->value[0] = atoi( value[0] );
	    pObjIndex->value[1] = atoi( value[1] );
	    pObjIndex->value[2] = atoi( value[2] );
	    pObjIndex->value[3] = atoi( value[3] );
	    break;

	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
	    pObjIndex->value[0] = atoi( value[0] );
	    pObjIndex->value[1] = skill_lookup( value[1] );
	    pObjIndex->value[2] = skill_lookup( value[2] );
	    pObjIndex->value[3] = skill_lookup( value[3] );
	    break;

	case ITEM_PORTAL:
	    pObjIndex->value[0] = atoi( value[0] );
	    break;
	    
	case ITEM_STAFF:
	case ITEM_LENSE:
        case ITEM_GUN:
	case ITEM_IMPLANTED:
	case ITEM_WAND:
	    pObjIndex->value[0] = atoi( value[0] );
	    pObjIndex->value[1] = atoi( value[1] );
	    pObjIndex->value[2] = atoi( value[2] );
	    pObjIndex->value[3] = skill_lookup( value[3] );
	    break;
	}

	if ( (letter = fread_letter( fp )) == '>' )
	{
	  ungetc( letter, fp );
	  load_traps( fp, pObjIndex, NULL, NULL );
	}
	else
	  ungetc( letter, fp );

	iHash			= vnum % MAX_KEY_HASH;
	pObjIndex->next		= obj_index_hash[iHash];
	obj_index_hash[iHash]	= pObjIndex;
	top_obj_index++;
	top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;  /* OLC */
	assign_area_vnum( vnum );				   /* OLC */
    }

    return;
}



/*
 * Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c
 */
void link_reset( ROOM_INDEX_DATA *pR, RESET_DATA *pReset )
{
    RESET_DATA *pr;

    if ( !pR )
       return;

    pr = pR->reset_last;

    if ( !pr )
    {
        pR->reset_first = pReset;
        pR->reset_last  = pReset;
    }
    else
    {
        pR->reset_last->next = pReset;
        pR->reset_last       = pReset;
        pR->reset_last->next = NULL;
    }

    top_reset++;
    return;
}



/*
 * Snarf a reset section.	Changed for OLC.
 */
void load_resets( FILE *fp )
{
    RESET_DATA	*pReset;
    int 	iLastRoom = 0;
    int 	iLastObj  = 0;

    if ( !area_last )
    {
	bug( "Load_resets: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	EXIT_DATA       *pexit;
	ROOM_INDEX_DATA *pRoomIndex;
	char             letter;

	if ( ( letter = fread_letter( fp ) ) == 'S' )
	    break;

	if ( letter == '*' )
	{
	    fread_to_eol( fp );
	    continue;
	}

	pReset		= new_reset();
	pReset->command	= letter;
	pReset->status  = fread_number( fp ); /* for area theme status */
	pReset->arg1	= fread_number( fp );
	pReset->arg2	= fread_number( fp );
	pReset->arg3	= ( letter == 'G' || letter == 'R' )
			    ? 0 : fread_number( fp );
			  fread_to_eol( fp );

	/*
	 * Validate parameters.
	 * We're calling the index functions for the side effect.
	 */
	switch ( letter )
	{
	default:
	    bug( "Load_resets: bad command '%c'.", letter );
	    exit( 1 );
	    break;

	case 'M':
	    get_mob_index  ( pReset->arg1 );
	    if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) )
	    {
		link_reset( pRoomIndex, pReset );
		iLastRoom = pReset->arg3;
	    }
	    break;

	case 'O':
	    get_obj_index  ( pReset->arg1 );
	    if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) )
	    {
		link_reset( pRoomIndex, pReset );
		iLastObj = pReset->arg3;
	    }
	    break;

	case 'P':
	    get_obj_index  ( pReset->arg1 );
	    if ( ( pRoomIndex = get_room_index ( iLastObj ) ) )
	    {
		link_reset( pRoomIndex, pReset );
	    }
	    break;

	case 'G':
	case 'E':
	    get_obj_index  ( pReset->arg1 );
	    if ( ( pRoomIndex = get_room_index ( iLastRoom ) ) )
	    {
		link_reset( pRoomIndex, pReset );
		iLastObj = iLastRoom;
	    }
	    break;

	case 'D':
	    pRoomIndex = get_room_index( pReset->arg1 );

	    if (   pReset->arg2 < 0
		|| pReset->arg2 > 5
		|| !pRoomIndex
		|| !( pexit = pRoomIndex->exit[pReset->arg2] )
		|| !IS_SET( pexit->rs_flags, EX_ISDOOR ) )
	    {
		bug( "Load_resets: 'D': exit %d not door.", pReset->arg2 );
		exit( 1 );
	    }

	    switch ( pReset->arg3 )	/* OLC 1.1b */
	    {
		default:
		    bug( "Load_resets: 'D': bad 'locks': %d." , pReset->arg3);
		case 0:
		    break;
		case 1: SET_BIT( pexit->rs_flags, EX_CLOSED );
		    break;
		case 2: SET_BIT( pexit->rs_flags, EX_CLOSED | EX_LOCKED );
		    break;
	    }
	    break;

	case 'R':
	    if ( pReset->arg2 < 0 || pReset->arg2 > 6 )	/* Last Door. */
	    {
		bug( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
		exit( 1 );
	    }

	    if ( ( pRoomIndex = get_room_index( pReset->arg1 ) ) )
		link_reset( pRoomIndex, pReset );

	    break;
	}
    }

    return;
}


/*
 * Snarf a room section.
 */
void load_rooms( FILE *fp )
{
    ROOM_INDEX_DATA *pRoomIndex;

    if ( !area_last )
    {
	bug( "Load_rooms: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	char letter;
	int  vnum;
	int  door;
	int  iHash;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_rooms: # not found.", 0 );
	    exit( 1 );
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	if ( get_room_index( vnum ) )
	{
	    bug( "Load_rooms: vnum %d duplicated.", vnum );
	    exit( 1 );
	}

	pRoomIndex			= new_room_index();
	pRoomIndex->people		= NULL;
	pRoomIndex->contents		= NULL;
	pRoomIndex->extra_descr		= NULL;
	pRoomIndex->area		= area_last;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->name		= fread_string( fp );
        pRoomIndex->soundfile           = fread_string( fp );
        pRoomIndex->musicfile           = fread_string( fp );
	pRoomIndex->description		= fread_string( fp );
	/* Area number */		  fread_number( fp );   /* Unused */
	pRoomIndex->room_flags		= fread_number( fp );
	pRoomIndex->sector_type		= fread_number( fp );
	pRoomIndex->light		= 0;
	for ( door = 0; door <= 5; door++ )
	    pRoomIndex->exit[door] = NULL;

	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'S' || letter == 's' )
	    {
		if ( letter == 's' )
		    bug( "Load_rooms: vnum %d has lowercase 's'", vnum );
		break;
	    }

	    if ( letter == 'D' )
	    {
		EXIT_DATA *pexit;
		int        locks;
		char       tLetter;

		door = fread_number( fp );
		if ( door < 0 || door > 5 )
		{
		    bug( "Fread_rooms: vnum %d has bad door number.", vnum );
		    exit( 1 );
		}

		pexit			= new_exit();
		pexit->description	= fread_string( fp );
		pexit->keyword		= fread_string( fp );
		pexit->exit_info	= 0;
		pexit->rs_flags		= 0;			/* OLC */
		locks			= fread_number( fp );
		pexit->key		= fread_number( fp );
		pexit->vnum		= fread_number( fp );

		switch ( locks )	/* OLC exit_info to rs_flags. */
		{
		case 1: pexit->rs_flags  = EX_ISDOOR;                    break;
		case 2: pexit->rs_flags  = EX_ISDOOR | EX_PICKPROOF;     break;
		case 3: pexit->rs_flags  = EX_ISDOOR | EX_BASHPROOF;     break;
		case 4: pexit->rs_flags  = EX_ISDOOR | EX_PICKPROOF
		                         | EX_BASHPROOF;                 break;
		case 5: pexit->rs_flags  = EX_ISDOOR | EX_PASSPROOF;     break;
		case 6: pexit->rs_flags  = EX_ISDOOR | EX_PICKPROOF
		                         | EX_PASSPROOF;                 break;
		case 7: pexit->rs_flags  = EX_ISDOOR | EX_BASHPROOF
		                         | EX_PASSPROOF;                 break;
		case 8: pexit->rs_flags  = EX_ISDOOR | EX_PICKPROOF
		                         | EX_BASHPROOF | EX_PASSPROOF;  break;
		}
		if ( (tLetter = fread_letter( fp )) == '>' )
		{
		  ungetc( tLetter, fp );
		  load_traps( fp, NULL, NULL, pexit );
		}
		else
		  ungetc( tLetter, fp );
		pRoomIndex->exit[door]   = pexit;
		top_exit++;
	    }
	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		ed			= new_extra_descr();
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= pRoomIndex->extra_descr;
		pRoomIndex->extra_descr	= ed;
		top_ed++;
	    }
	    else
	    {
		bug( "Load_rooms: vnum %d has flag not 'DES'.", vnum );
		exit( 1 );
	    }
	}

	if ( (letter = fread_letter( fp )) == '>' )
	{
	  ungetc( letter, fp );
	  load_traps( fp, NULL, pRoomIndex, NULL );
	}
	else
	  ungetc( letter, fp );

	iHash			= vnum % MAX_KEY_HASH;
	pRoomIndex->next	= room_index_hash[iHash];
	room_index_hash[iHash]	= pRoomIndex;
	top_room++;
	top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
	assign_area_vnum( vnum );				     /* OLC */

    }

    return;
}



/*****************************************************************************
 Name:		new_load_rooms
 Purpose:	Loads rooms without the anoying case sequence.
 ****************************************************************************/
/* OLC 1.1b */
void new_load_rooms( FILE *fp )
{
    ROOM_INDEX_DATA *pRoomIndex;

    if ( !area_last )
    {
	bug( "Load_rooms: no #AREA seen yet.", 0 );
	exit( 1 );
    }

    for ( ; ; )
    {
	char letter;
	int  vnum;
	int  door;
	int  iHash;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_rooms: # not found.", 0 );
	    exit( 1 );
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	if ( get_room_index( vnum ) )
	{
	    bug( "Load_rooms: vnum %d duplicated.", vnum );
	    exit( 1 );
	}

	pRoomIndex			= new_room_index();
	pRoomIndex->people		= NULL;
	pRoomIndex->contents		= NULL;
	pRoomIndex->extra_descr		= NULL;
	pRoomIndex->area		= area_last;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->name		= fread_string( fp );
        pRoomIndex->soundfile           = fread_string( fp );
        pRoomIndex->musicfile           = fread_string( fp );
	pRoomIndex->description		= fread_string( fp );
	/* Area number */		  fread_number( fp );   /* Unused */
	pRoomIndex->room_flags		= fread_number( fp );
	pRoomIndex->sector_type		= fread_number( fp );

/*
	if (area_last->version == 100 ) // 1.00
	{
		pRoomIndex->new_field = someDefaultValue;	
	} else
	{
		pRoomIndex->new_field = fread_number( fp );
	}
*/
	pRoomIndex->light		= 0;
	pRoomIndex->rd                  = 0;

	for ( door = 0; door <= 5; door++ )
	    pRoomIndex->exit[door] = NULL;

	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'R' )
	    {
	      char *word;

	      ungetc( letter, fp );
	      word = fread_word( fp );
	      if ( !str_cmp( word, "Rd" ) )
	      {
		if ( pRoomIndex->rd != 0 )
		  bug( "New_load_rooms: rd already assigned for room #%d; updating.", pRoomIndex->vnum );
		pRoomIndex->rd = fread_number( fp );
	      }
	      else
	      {
		bug( "New_Load_rooms: vnum %d has flag not 'DES'.", vnum );
		exit( 1 );
	      }
	      continue;
	    }

	    if ( letter == 'S' || letter == 's' )
	    {
		if ( letter == 's' )
		    bug( "New_Load_rooms: vnum %d has lowercase 's'", vnum );
		break;
	    }

	    if ( letter == 'D' )
	    {
		EXIT_DATA *pexit;
		int        locks;
		char       tLetter;

		door = fread_number( fp );
		if ( door < 0 || door > 5 )
		{
		    bug( "Fread_rooms: vnum %d has bad door number.", vnum );
		    exit( 1 );
		}

		pexit			= new_exit();
		pexit->description	= fread_string( fp );
		pexit->keyword		= fread_string( fp );
		locks			= fread_number( fp );
		pexit->exit_info	= locks;
		pexit->rs_flags		= locks;
		pexit->key		= fread_number( fp );
		pexit->vnum		= fread_number( fp );

		if ( (tLetter = fread_letter( fp )) == '>' )
		{
		  ungetc( tLetter, fp );
		  load_traps( fp, NULL, NULL, pexit );
		}
		else
		  ungetc( tLetter, fp );

		pRoomIndex->exit[door]		= pexit;
		top_exit++;
	    }
	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		ed			= new_extra_descr();
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= pRoomIndex->extra_descr;
		pRoomIndex->extra_descr	= ed;
		top_ed++;
	    }
	    else
	    {
		bug( "New_Load_rooms: vnum %d has flag not 'DES'.", vnum );
		exit( 1 );
	    }
	}

	if ( (letter = fread_letter( fp )) == '>' )
	{
	  ungetc( letter, fp );
	  load_traps( fp, NULL, pRoomIndex, NULL );
	}
	else
	  ungetc( letter, fp );

	iHash			= vnum % MAX_KEY_HASH;
	pRoomIndex->next	= room_index_hash[iHash];
	room_index_hash[iHash]	= pRoomIndex;
	top_room++;
	top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room;
	assign_area_vnum( vnum );
    }

    return;
}



/*
 * Snarf a shop section.
 */
void load_shops( FILE *fp )
{
    SHOP_DATA *pShop;

    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	int iTrade;

	pShop			= new_shop();
	pShop->keeper		= fread_number( fp );
	if ( pShop->keeper == 0 )
	    break;
	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	    pShop->buy_type[iTrade] = fread_number( fp );
	pShop->profit_buy	= fread_number( fp );
	pShop->profit_sell	= fread_number( fp );
	pShop->open_hour	= fread_number( fp );
	pShop->close_hour	= fread_number( fp );
				  fread_to_eol( fp );
	pMobIndex		= get_mob_index( pShop->keeper );
	pMobIndex->pShop	= pShop;

	if ( !shop_first )
	    shop_first = pShop;
	if (  shop_last  )
	    shop_last->next = pShop;

	shop_last	= pShop;
	pShop->next	= NULL;
	top_shop++;
    }

    return;
}



/*
 * Snarf spec proc declarations.
 */
void load_specials( FILE *fp )
{
    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	char letter;

	switch ( letter = fread_letter( fp ) )
	{
	default:
	    bug( "Load_specials: letter '%c' not *MS.", letter );
	    exit( 1 );

	case 'S':
	    return;

	case '*':
	    break;

	case 'M':
	    pMobIndex           = get_mob_index ( fread_number ( fp ) );
	    pMobIndex->spec_fun = spec_lookup   ( fread_word   ( fp ) );
	    if ( pMobIndex->spec_fun == 0 )
	    {
		bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
		exit( 1 );
	    }
	    break;
	}

	fread_to_eol( fp );
    }
}


/*
 * Snarf games proc declarations.
 */
void load_games( FILE *fp )
{
    for ( ; ; )
    {
        MOB_INDEX_DATA *pMobIndex;
        char            letter;

        switch ( letter = fread_letter( fp ) )
        {
        default:
            bug( "Load_games: letter '%c' not *MS.", letter );
            exit( 1 );

        case 'S':
            return;

        case '*':
            break;

        case 'M':
            pMobIndex           = get_mob_index ( fread_number ( fp ) );
            pMobIndex->game_fun = game_lookup   ( fread_string ( fp ) );
            pMobIndex->gold     =       fread_number ( fp );
            /* Read max wait unused */  fread_number ( fp );
            pMobIndex->ac       =       fread_number ( fp );
            /* I use ac here, because it is unused... since not many
                mobs in the world will have game funs, it's best not to
                add a new field for this, this saves you 4 bytes of
                memory for the cheat and 4 for the gold value per mob,
                with 6000 mobs, this saves you 48k memory !! Maniac */
            if ( pMobIndex->game_fun == 0 )
            {
                bug( "Load_games: 'M': vnum %d.", pMobIndex->vnum );
                exit( 1 );
            }
            break;
        }

        fread_to_eol( fp );
    }
}


void load_clans( void )
{
    FILE      *fp;
    CLAN_DATA *pClanIndex;
    char letter;

    if ( !( fp = fopen( CLAN_FILE, "r" ) ) )
	return;
    for ( ; ; )
    {
	int  vnum;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_clans: # not found.", 0 );
	    exit( 1 );
	}

	vnum				= fread_number( fp );
	if ( vnum == 999 )
	    break;

	if ( get_clan_index( vnum ) )
	{
	    bug( "Load_clans: vnum %d duplicated.", vnum );
	    exit( 1 );
	}

	pClanIndex			= new_clan();
	pClanIndex->vnum		= vnum;
	pClanIndex->name		= fread_string( fp );
	pClanIndex->deity               = fread_string( fp );
	pClanIndex->description         = fread_string( fp );
	pClanIndex->champ               = fread_string( fp );
	pClanIndex->leader              = fread_string( fp );
	pClanIndex->first               = fread_string( fp );
	pClanIndex->second              = fread_string( fp );
	pClanIndex->ischamp             = fread_number( fp );
	pClanIndex->isleader            = fread_number( fp );
	pClanIndex->isfirst             = fread_number( fp );
	pClanIndex->issecond            = fread_number( fp );
	pClanIndex->recall		= fread_number( fp );
	pClanIndex->pkills		= fread_number( fp );
	pClanIndex->mkills		= fread_number( fp );
	pClanIndex->members		= fread_number( fp );
	pClanIndex->pdeaths		= fread_number( fp );
	pClanIndex->mdeaths		= fread_number( fp );
	pClanIndex->obj_vnum_1		= fread_number( fp );
	pClanIndex->obj_vnum_2		= fread_number( fp );
	pClanIndex->obj_vnum_3		= fread_number( fp );
        pClanIndex->pkill               = fread_number( fp );

	clan_sort(pClanIndex);
	top_clan++;
    }
    fclose ( fp );   
 
    return;
}

/*
 * Snarf notes file.
 */
void load_notes( void )
{
    FILE      *fp;
    NOTE_DATA *pnotelast;

    if ( !( fp = fopen( NOTE_FILE, "r" ) ) )
	return;

    pnotelast = NULL;
    for ( ; ; )
    {
	NOTE_DATA *pnote;
	char       letter;

	do
	{
	    letter = getc( fp );
	    if ( feof(fp) )
	    {
		fclose( fp );
		return;
	    }
	}
	while ( isspace( letter ) );
	ungetc( letter, fp );

	pnote		  = new_note();

	if ( str_cmp( fread_word( fp ), "sender" ) )
	    break;
	pnote->sender     = fread_string( fp );

	if ( str_cmp( fread_word( fp ), "date" ) )
	    break;
	pnote->date       = fread_string( fp );

	if ( str_cmp( fread_word( fp ), "stamp" ) )
	    break;
	pnote->date_stamp = fread_number( fp );

	if ( str_cmp( fread_word( fp ), "to" ) )
	    break;
	pnote->to_list    = fread_string( fp );

	if ( str_cmp( fread_word( fp ), "subject" ) )
	    break;
	pnote->subject    = fread_string( fp );

	pnote->protected = FALSE;
	pnote->on_board = 0;
	{
	  char letter;

	  letter = fread_letter( fp );
	  ungetc( letter, fp );
	  if ( letter == 'P' && !str_cmp( fread_word( fp ), "protect" ) )
	    pnote->protected  = fread_number( fp );
	  letter = fread_letter( fp );
	  ungetc( letter, fp );
	  if ( letter == 'B' && !str_cmp( fread_word( fp ), "board" ) )
	    pnote->on_board   = fread_number( fp );
	}

	if ( str_cmp( fread_word( fp ), "text" ) )
	    break;
	pnote->text       = fread_string( fp );

	if ( !note_list )
	    note_list           = pnote;
	else
	    pnotelast->next     = pnote;

	pnotelast               = pnote;
    }

    strcpy( strArea, NOTE_FILE );
    fpArea = fp;
    bug( "Load_notes: bad key word.", 0 );
    exit( 1 );
    return;
}


void load_down_time( void )
{
    FILE *fp;

    down_time = str_dup ( "*" );
    warning1  = str_dup ( "*" );
    warning2  = str_dup ( "*" );
    stype     = 1;

    if ( !( fp = fopen( DOWN_TIME_FILE, "r" ) ) )
        return;

    for ( ; ; )
    {
        char *word;
	char  letter;

	do
	{
	    letter = getc( fp );
	    if ( feof( fp ) )
	    {
		fclose( fp );
		return;
	    }
	}
	while ( isspace( letter ) );
	ungetc( letter, fp );
	
	word = fread_word( fp );

	if ( !str_cmp( word, "DOWNTIME" ) )
	{
	    free_string( down_time );
	    down_time = fread_string( fp );
	}
	if ( !str_cmp( word, "WARNINGA" ) )
	{
	    free_string( warning1 );
	    warning1 = fread_string( fp );
	}
	if ( !str_cmp( word, "WARNINGB" ) )
	{
	    free_string( warning2 );
	    warning2 = fread_string( fp );
	}
    }
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad or suspicious reverse exits.
 */
void fix_exits( void )
{
		 EXIT_DATA       *pexit;
		 EXIT_DATA       *pexit_rev;
		 ROOM_INDEX_DATA *pRoomIndex;
		 ROOM_INDEX_DATA *to_room;
    extern const int              rev_dir [ ];
		 int              iHash;
		 int              door;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = room_index_hash[iHash];
	      pRoomIndex;
	      pRoomIndex  = pRoomIndex->next )
	{
	    bool fexit;

	    fexit = FALSE;
	    for ( door = 0; door <= 5; door++ )
	    {
		if ( ( pexit = pRoomIndex->exit[door] ) )
		{
		    fexit = TRUE;
		    if ( pexit->vnum <= 0 )
			pexit->to_room = NULL;
		    else
			pexit->to_room = get_room_index( pexit->vnum );
		}
	    }

	    if ( !fexit )
		SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
	}
    }

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = room_index_hash[iHash];
	      pRoomIndex;
	      pRoomIndex  = pRoomIndex->next )
	{
	    for ( door = 0; door <= 5; door++ )
	    {
		if (   ( pexit     = pRoomIndex->exit[door]       )
		    && ( to_room   = pexit->to_room               )
		    && ( pexit_rev = to_room->exit[rev_dir[door]] )
		    &&   pexit_rev->to_room != pRoomIndex )
		{
/* commented out... too many
		    sprintf( buf, "Fix_exits: %d:%d -> %d:%d -> %d.",
			    pRoomIndex->vnum, door,
			    to_room->vnum,    rev_dir[door],
			    ( !pexit_rev->to_room ) ? 0
			    :  pexit_rev->to_room->vnum );
		    bug( buf, 0 );
*/
		}
	    }
	}
    }

    return;
}



/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
    AREA_DATA *pArea;
    char	sound [MAX_STRING_LENGTH];

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
	CHAR_DATA *pch;

	if ( ++pArea->age < 3 )
	    continue;

	/*
	 * Check for PC's.
	 */
	if ( pArea->nplayer > 0 && pArea->age == 15 - 1 )
	{
	    for ( pch = char_list; pch; pch = pch->next )
	    {
		if ( !IS_NPC( pch )
		    && IS_AWAKE( pch )
		    && pch->in_room
		    && pch->in_room->area == pArea )
		{
		    if ( pArea->reset_sound != NULL )
		      send_to_char(AT_GREEN, pArea->reset_sound, pch );
		    else
		      send_to_char(AT_GREEN, "You shiver as a cold breeze blows through the room.",
		      pch );
		    send_to_char(AT_GREEN, " ", pch );
		    if(!IS_NPC( pch ) && IS_SET( pch->act, PLR_SOUND ) && pArea->actual_sound != NULL )
		    {
			sprintf(sound, "!!SOUND(%s)", pArea->actual_sound );
			send_to_char(AT_GREEN, sound, pch );
		    }
		    send_to_char( AT_GREEN, "\n\r", pch );
		}
	    }
	}

	/*
	 * Check age and reset.
	 * Note: Mud School resets every 3 minutes (not 15).
	 */
	if ( pArea->nplayer == 0 || pArea->age >= 15 )
	{
	    ROOM_INDEX_DATA *pRoomIndex;

	    reset_area( pArea );
	    pArea->age = number_range( 0, 3 );
	    pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
	    if ( pRoomIndex && pArea == pRoomIndex->area )
		pArea->age = 15 - 3;
	}
    }

    return;
}



/* OLC
 * Reset one room.  Called by reset_area and olc.
 */
void reset_room( ROOM_INDEX_DATA *pRoom )
{
    RESET_DATA	*pReset;
    CHAR_DATA	*pMob;
    OBJ_DATA	*pObj;
    CHAR_DATA	*LastMob = NULL;
    OBJ_DATA	*LastObj = NULL;
    int iExit;
    int level = 0;
    bool last;

    if ( !pRoom )
	return;

    pMob	= NULL;
    last	= FALSE;
    
    for ( iExit = 0;  iExit < MAX_DIR;  iExit++ )
    {
	EXIT_DATA *pExit;
	if ( ( pExit = pRoom->exit[iExit] )
	  && !IS_SET( pExit->exit_info, EX_BASHED ) )	/* Skip Bashed. */
	{
	    pExit->exit_info = pExit->rs_flags;
	    if ( ( pExit->to_room != NULL )
	      && ( ( pExit = pExit->to_room->exit[rev_dir[iExit]] ) ) )
	    {
		/* nail the other side */
		pExit->exit_info = pExit->rs_flags;
	    }
	}
    }

    for ( pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next )
    {
	MOB_INDEX_DATA	*pMobIndex;
	OBJ_INDEX_DATA	*pObjIndex;
	OBJ_INDEX_DATA	*pObjToIndex;
	ROOM_INDEX_DATA	*pRoomIndex;
	if (pReset->status && (!(pReset->status & pRoom->area->status)))
		continue;
	switch ( pReset->command )
	{
	default:
		sprintf(log_buf, "Reset_room: bad command %c in room %d.", 
		pReset->command, pRoom->vnum );
		bug( log_buf, 0 ); 
		break;

	case 'M':
	    if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )
	    {
		sprintf(log_buf,  "Reset_room: 'M': bad vnum %d in room %d.", 
		pReset->arg1, pRoom->vnum );
		bug( log_buf, 0 ); 
		continue;
	    }

	    /*
	     * Some hard coding.
	     */
	    if ( ( pMobIndex->spec_fun == spec_lookup( "spec_cast_ghost" ) &&
	         ( weather_info.sunlight != SUN_DARK ) &&
		  !room_is_dark( pRoom ) ) ) continue;

	    if ( pMobIndex->count >= pReset->arg2 )
	    {
		last = FALSE;
		break;
	    }

	    pMob = create_mobile( pMobIndex );

	    /*
	     * Some more hard coding.
	     */
	    if ( room_is_dark( pRoom ) )
		SET_BIT(pMob->affected_by, AFF_INFRARED);

	    /*
	     * Pet shop mobiles get ACT_PET set.
	     */
	    {
		ROOM_INDEX_DATA *pRoomIndexPrev;

		pRoomIndexPrev = get_room_index( pRoom->vnum - 1 );
		if ( pRoomIndexPrev
		    && IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP ) )
		    SET_BIT( pMob->act, ACT_PET);
	    }

	    char_to_room( pMob, pRoom );

	    LastMob = pMob;
	    level  = URANGE( 0, pMob->level - 2, LEVEL_DEMIGOD );
	    last = TRUE;
	    break;

	case 'O':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
	    {
		sprintf( log_buf, "Reset_room: 'O': bad vnum %d in room %d.", 
		pReset->arg1, pRoom->vnum );
		bug( log_buf, 0 ); 
		continue;
	    }

	    if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
	    {
		sprintf( log_buf, "Reset_room: 'O': bad vnum %d in room %d.", 
		pReset->arg3, pRoom->vnum );
		bug( log_buf, 0 ); 
		continue;
	    }

	    if ( pRoom->area->nplayer > 0
	      || count_obj_list( pObjIndex, pRoom->contents ) > 0 )
		break;

	    pObj = create_object( pObjIndex, level );
	    obj_to_room( pObj, pRoom );
	    break;

	case 'P':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
	    {
		sprintf( log_buf, "Reset_room: 'P': bad vnum %d in room %d.", 
		pReset->arg1, pRoom->vnum );
		bug( log_buf, 0 ); 
		continue;
	    }

	    if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
	    {
		sprintf( log_buf, "Reset_room: 'P': bad vnum %d in room %d.", 
		pReset->arg3, pRoom->vnum );		
		bug( log_buf, 0 ); 
		continue;
	    }

	    if ( pRoom->area->nplayer > 0
	      || !( LastObj = get_obj_type( pObjToIndex ) )
	      || count_obj_list( pObjIndex, LastObj->contains ) > 0 )
		break;

	    pObj = create_object( pObjIndex, level);
	    obj_to_obj( pObj, LastObj );

	    /*
	     * Ensure that the container gets reset.	OLC 1.1b
	     */
	    if ( LastObj->item_type == ITEM_CONTAINER )
	    {
		LastObj->value[1] = LastObj->pIndexData->value[1];
	    }
	    else
	    {
	    	    /* THIS SPACE INTENTIONALLY LEFT BLANK */
	    }
	    break;

	case 'G':
	case 'E':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
	    {
		sprintf( log_buf, "Reset_room: 'E' or 'G': bad vnum %d in room %d.", 
		pReset->arg1, pRoom->vnum );
		bug( log_buf, 0 );
		continue; 
	    }

	    if ( !last )
		break;

	    if ( !LastMob )
	    {
		sprintf( log_buf, "Reset_room: 'E' or 'G': null mob for vnum %d in room %d.",
		    pReset->arg1, pRoom->vnum );
		    bug( log_buf, 0 ); 
		last = FALSE;
		break;
	    }

	    if ( LastMob->pIndexData->pShop )	/* Shop-keeper? */
	    {
		int olevel;

		switch ( pObjIndex->item_type )
		{
		default:                olevel = 0;                      break;
		case ITEM_PILL:         olevel = number_range(  0, 10 ); break;
		case ITEM_POTION:	olevel = number_range(  0, 10 ); break;
		case ITEM_SCROLL:	olevel = number_range(  5, 15 ); break;
		case ITEM_WAND:		olevel = number_range( 10, 20 ); break;
		case ITEM_GUN:		olevel = number_range( 10, 20 ); break;
		case ITEM_IMPLANTED:	olevel = number_range( 10, 20 ); break;
		case ITEM_LENSE:        olevel = number_range( 10, 20 ); break;
		case ITEM_STAFF:	olevel = number_range( 15, 25 ); break;
		case ITEM_ARMOR:	olevel = number_range(  5, 15 ); break;
		case ITEM_WEAPON:	if ( pReset->command == 'G' )
		                            olevel = number_range( 5, 15 );
		                        else
					    olevel = number_fuzzy( level );
		  break;
		}

		pObj = create_object( pObjIndex, olevel );
		if ( pReset->command == 'G' )
		    SET_BIT( pObj->extra_flags, ITEM_INVENTORY );
	    }
	    else
	    {
  		pObj = create_object( pObjIndex, level);
	    }
            obj_to_char( pObj, LastMob );
	    if ( pReset->command == 'E' )
		equip_char( LastMob, pObj, pReset->arg3 );
	    last = TRUE;
	    break;

	case 'D':
	    break;

	case 'R':
/* OLC 1.1b */
	    if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
	    {
		sprintf( log_buf, "Reset_room: 'R': bad vnum %d in room %d.", 
		pReset->arg1, pRoom->vnum );
		bug( log_buf, 0 ); 
		continue;
	    }

		if ( pRoomIndex->area->builders[0] != '\0' )
			continue;

	    {
		EXIT_DATA *pExit;
		int d0;
		int d1;

		for ( d0 = 0; d0 < pReset->arg2; d0++ )
		{
		    d1                   = number_range( d0, pReset->arg2 );
		    pExit                = pRoomIndex->exit[d0];
		    pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
		    pRoomIndex->exit[d1] = pExit;
		}
	    }
	    break;
	}
    }

    return;
}



/* OLC
 * Reset one area.
 */
void reset_area( AREA_DATA *pArea )
{
    ROOM_INDEX_DATA *pRoom;
    int  vnum;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if ( ( pRoom = get_room_index(vnum) ) )
	    reset_room(pRoom);
    }

    return;
}



/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex )
{
    CHAR_DATA *mob;

    if ( !pMobIndex )
    {
	bug( "Create_mobile: NULL pMobIndex.", 0 );
	exit( 1 );
    }

	mob		= new_char();

    clear_char( mob );
    mob->pIndexData     = pMobIndex;

    mob->name		= str_dup( pMobIndex->player_name );	/* OLC */
    mob->short_descr	= str_dup( pMobIndex->short_descr );	/* OLC */
    mob->long_descr	= str_dup( pMobIndex->long_descr );	/* OLC */
    mob->description	= str_dup( pMobIndex->description );	/* OLC */
    mob->spec_fun	= pMobIndex->spec_fun;
    mob->prompt         = str_dup( "<%hhp %mm %vmv> " );
    mob->game_fun       = pMobIndex->game_fun;

    /* If mobile has a game fun, use the money supplied by the pay-roll
        argument in the area file, see games.c and area.txt (under GAMES)
        i use this to save about 50 of memory (8 bytes per mob) */
        /* Maniac */
    if (mob->pIndexData->game_fun)
        mob->gold       = mob->pIndexData->gold;

    mob->level		= pMobIndex->level;
    mob->act		= pMobIndex->act | ACT_IS_NPC;
    mob->affected_by	= pMobIndex->affected_by;
    mob->affected_by2   = pMobIndex->affected_by2;
    mob->affected_by3   = pMobIndex->affected_by3;
    mob->affected_by4   = pMobIndex->affected_by4;
    mob->imm_flags	= pMobIndex->imm_flags;
    mob->res_flags      = pMobIndex->res_flags;
    mob->vul_flags      = pMobIndex->vul_flags;

    mob->alignment	= pMobIndex->alignment;
    mob->sex		= pMobIndex->sex;
    mob->size		= pMobIndex->size;

    mob->armor		= interpolate( mob->level, 100, -100 );
    mob->gold           = pMobIndex->gold;
    mob->max_hit	= pMobIndex->hitnodice + 1;
    mob->hit		= mob->max_hit;
    mob->combat_timer	= 0;			/* XOR */
    mob->summon_timer	= -1;			/* XOR */
    mob->speaking	= pMobIndex->speaking;	/* Tyrion, OLC */
    mob->hitroll	= pMobIndex->hitroll;
    mob->damroll	= pMobIndex->damroll;

    /*
     * Insert in list.
     */
    mob->next		= char_list;
    char_list		= mob;
    pMobIndex->count++;
    return mob;
}

bool find_corruption(char* aString)
{
	register int length = strlen(aString);
	register int i = 0;
	for (i = 0; i < length; i++)
	{
		register char c = aString[i];
	 	if ((c < 32 || c > 126) && (c != 9) && (c != 10) && (c != 13))
                {
                        return TRUE;
                }
	}
	return FALSE;
}


/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level )
{
    static OBJ_DATA  obj_zero;
           OBJ_DATA *obj;

    static int depth = 0;

    if (depth>3)
       return NULL; 	// this will cause a crash
			// but avoid an infinite loop
    depth++;

    if ( !pObjIndex )
    {
	bug( "Create_object: NULL pObjIndex.", 0 );
	bug( "Replacing with dummy object.", 0 );
	pObjIndex = get_obj_index( 1 );
	/*
	exit( 1 );
	*/
    }

    obj			= new_obj();
    *obj		= obj_zero;
    obj->pIndexData	= pObjIndex;
    obj->in_room	= NULL;
    obj->level		= pObjIndex->level;
    obj->wear_loc	= -1;

    obj->name		= str_dup( pObjIndex->name );		/* OLC */
    obj->short_descr	= str_dup( pObjIndex->short_descr );	/* OLC */
    obj->description	= str_dup( pObjIndex->description );	/* OLC */
    obj->item_type	= pObjIndex->item_type;
    obj->extra_flags	= pObjIndex->extra_flags;
    obj->extra_flags2	= pObjIndex->extra_flags2;
    obj->extra_flags3   = pObjIndex->extra_flags3;
    obj->extra_flags4   = pObjIndex->extra_flags4;
    obj->durability_max = pObjIndex->durability_max;
    obj->durability_cur = pObjIndex->durability_cur;
    obj->wear_flags	= pObjIndex->wear_flags;
    obj->value[0]	= pObjIndex->value[0];
    obj->value[1]	= pObjIndex->value[1];
    obj->value[2]	= pObjIndex->value[2];
    obj->value[3]	= pObjIndex->value[3];
    obj->weight		= pObjIndex->weight;
    obj->cost		= pObjIndex->cost;
    obj->ac_type        = pObjIndex->ac_type;
    obj->ac_vnum        = pObjIndex->ac_vnum;
    obj->ac_spell       = str_dup( pObjIndex->ac_spell );
    obj->ac_charge[0]   = pObjIndex->ac_charge[0];
    obj->ac_charge[1]   = pObjIndex->ac_charge[1];
    obj->deleted        = FALSE;

    /*
     * Mess with object properties.
     */
    switch ( obj->item_type )
    {
    default:
	bug( "Read_object: vnum %d bad type.", pObjIndex->vnum );
	break;

    case ITEM_LIGHT:
    case ITEM_TREASURE:
    case ITEM_FURNITURE:
    case ITEM_TRASH:
    case ITEM_CONTAINER:
    case ITEM_DRINK_CON:
    case ITEM_KEY:
    case ITEM_FOOD:
    case ITEM_BOAT:
    case ITEM_CORPSE_PC:
    case ITEM_FOUNTAIN:
    case ITEM_BLOOD:
    case ITEM_VODOO:
    case ITEM_BERRY:
    case ITEM_NOTEBOARD:
    case ITEM_SKIN:
    case ITEM_HAMMER:
    case ITEM_QUILL:
    case ITEM_NEEDLE:
    case ITEM_PESTLE:
	break;

	case ITEM_ARROW:
	case ITEM_BULLET:
	case ITEM_BOLT:
	obj->value[0]	= obj->value[0];

    case ITEM_RUNE:
	obj->value[0]	= obj->value[0];
	break;

    case ITEM_SCROLL:
	obj->value[0]   = obj->value[0];
	break;

    case ITEM_BOOK:
	obj->value[0]	= obj->value[0];
	break;

    case ITEM_WAND:
    case ITEM_IMPLANTED:
    case ITEM_LENSE:
    case ITEM_GUN:
    case ITEM_STAFF:
	obj->value[0]   = obj->value[0];
	obj->value[1]	= obj->value[1];
	obj->value[2]	= obj->value[1];
	obj->value[3]   = obj->value[3];
	break;
    case ITEM_CORPSE_NPC:
    case ITEM_PORTAL:
        obj->value[0]   = obj->value[0];
        break;
        
    case ITEM_WEAPON:
	obj->value[1]   = number_fuzzy( number_fuzzy( 1 * level / 4 + 2 ) );
	obj->value[2]	= number_fuzzy( number_fuzzy( 3 * level / 4 + 6 ) );
	if ( IS_SET ( obj->extra_flags2, ITEM_TWO_HANDED ) )
	{
	    obj->value[1] = obj->value[1] * 23 / 7;
	    obj->value[2] = obj->value[1] * 23 / 7;
	}
	if ( obj->value[3] == WEAPON_POLEARM)
	{
	    obj->value[1] = obj->value[1] * 5 / 3;
	    obj->value[2] = obj->value[2] * 5 / 3;
	}

	obj->value[1] = number_fuzzy( obj->value[1] );
	obj->value[2] = UMAX ( obj->value[1] + 1, number_fuzzy( obj->value[2] ) );
	break;

    case ITEM_ARMOR:
	obj->value[0]   = number_fuzzy( level / 4 + 2 );
	break;

    case ITEM_POTION:
    case ITEM_PILL:
	obj->value[0]   = obj->value[0];
	break;

    case ITEM_MONEY:
	obj->value[0]   = obj->cost;
	break;
    }

    obj->next		= object_list;
    object_list		= obj;
    pObjIndex->count++;

    // search for corruption in the objects
    if (find_corruption(obj->name) || find_corruption(obj->description) || find_corruption(obj->short_descr) || find_corruption(obj->ac_spell))
    {
	OBJ_DATA* co;
	CORRUPT_AREA_LIST* cal;
	bool found = FALSE;

    	bug("Corruption in object vnum %d reboot is necessary!", obj->pIndexData->vnum);
	SET_BIT(obj->pIndexData->area->area_flags, AREA_CORRUPT);
	
	for (cal=corrupt; cal; cal = cal->next)
	{
		if (cal->vnum==obj->pIndexData->vnum)
		{
			found = TRUE;
			break;
		}
	}
	if (!found)
	{
		cal = new_corruptl();
		cal->vnum = obj->pIndexData->vnum;
		cal->area = obj->pIndexData->area;
		cal->next = corrupt;
		corrupt = cal;
	}
	extract_obj(obj);
	// we're going to take a guess and assume object 1 isn't corrupt
	co = create_object( get_obj_index( 1 ), 1);
	depth--;
	return co;
    }

    depth--;
    return obj;
}


/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA *ch )
{
    static CHAR_DATA ch_zero;

    *ch				= ch_zero;
    ch->name			= &str_empty[0];
    ch->short_descr		= &str_empty[0];
    ch->long_descr		= &str_empty[0];
    ch->description		= &str_empty[0];
    ch->prompt                  = &str_empty[0];
    ch->last_note               = 0;
    ch->logon			= current_time;
    ch->armor			= 100;
    ch->position		= POS_STANDING;
    ch->level                   = 0;
    ch->practice		= 21;
    ch->hit			= 20;
    ch->max_hit			= 20;
    ch->mana			= 100;
    ch->max_mana		= 100;
    ch->bp                      = 20;
    ch->max_bp                  = 20;
    ch->move			= 100;
    ch->max_move		= 100;
    ch->leader                  = NULL;
    ch->master                  = NULL;
    ch->deleted                 = FALSE;
    return;
}



/*
 * Free a character.
 * Modified to use new free functions in memory.c
 * -- Manaux
 */
void free_ch( CHAR_DATA *ch )
{
    OBJ_DATA    *obj;
    OBJ_DATA    *obj_next;
    AFFECT_DATA *paf;

    for ( obj = ch->carrying; obj; obj = obj_next )
    {
        obj_next = obj->next_content;
        if ( obj->deleted )
	    continue;
	extract_obj( obj );
    }
/*bug("obj",0);*/
    for ( paf = ch->affected; paf; paf = paf->next )
    {
        if ( paf->deleted )
	    continue;
	affect_remove( ch, paf );
    }
/*bug("string",0);*/
    if ( ch->pcdata )
    {
        ALIAS_DATA *pAl;
	ALIAS_DATA *pAl_next;

	for ( obj = ch->pcdata->storage; obj; obj = obj_next )
	{
	  obj_next = obj->next_content;
	  if ( obj->deleted )
	    continue;
	  extract_obj( obj );
	}
/*bug("pc_obj",0);*/
	for ( pAl = ch->pcdata->alias_list; pAl; pAl = pAl_next )
	{
	  pAl_next = pAl->next;
	  free_alias(pAl);
	}
	free_pc(ch->pcdata);
    }

    if ( ch->pnote )
    {
	    free_note(ch->pnote);
    }
/*bug("pnote",0);*/
    if ( ch->gspell )
      end_gspell( ch );
/*bug("gspell",0);*/
    free_char(ch);
/*bug("end",0);*/
    return;
}



/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed; ed = ed->next )
    {
	if ( is_name( name, ed->keyword ) )
	    return ed->description;
    }
    return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
    MOB_INDEX_DATA *pMobIndex;

    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
	  pMobIndex;
	  pMobIndex  = pMobIndex->next )
    {
	if ( pMobIndex->vnum == vnum )
	    return pMobIndex;
    }

    return NULL;
}



/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
	  pObjIndex;
	  pObjIndex  = pObjIndex->next )
    {
	if ( pObjIndex->vnum == vnum )
	    return pObjIndex;
    }

    return NULL;
}



/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( int vnum )
{
    ROOM_INDEX_DATA *pRoomIndex;

    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
	  pRoomIndex;
	  pRoomIndex  = pRoomIndex->next )
    {
	if ( pRoomIndex->vnum == vnum )
	    return pRoomIndex;
    }

    return NULL;
}

CLAN_DATA *get_clan_index( int vnum )
{
    CLAN_DATA *pClanIndex;

    for ( pClanIndex  = clan_first;
	  pClanIndex;
	  pClanIndex  = pClanIndex->next )
    {
	if ( pClanIndex->vnum == vnum )
	    return pClanIndex;
    }

    return NULL;
}



/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    do
    {
	c = getc( fp );
    }
    while ( isspace( c ) );

    return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    char c;
    int  number;
    bool sign;

    do
    {
	c = getc( fp );
    }
    while ( isspace( c ) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit( c ) )
    {
	bug( "Fread_number: bad format.", 0 );
	bug( "   If bad object, check for missing '~' in value[] fields.", 0 );
	exit( 1 );
    }

    while ( isdigit(c) )
    {
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}



/*
 * Read and allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed:
 *   each string prepended with hash pointer to prev string,
 *   hash code is simply the string length.
 * This function takes 40% to 50% of boot-up time.
 */

/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

    do
    {
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char  word [ MAX_INPUT_LENGTH ];
           char *pword;
           char  cEnd;

    do
    {
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace( *pword ) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    bug( "Fread_word: word too long.", 0 );
    exit( 1 );
    return NULL;
}

void *new_mem( int sMem )
{
  void *pMem;
  static bool Killit;
  static bool Already;

  if (!( pMem = calloc(1, sMem) ) ||
      (((sAllocPerm * 10) / (1024*1024)) > 95 && !Killit))
  {
    if ( !pMem )
    {
      /* Not even enough mem to shutdown decently */
      exit( 1 );
    }
    if ( !Already )
    {
      bug( "New_mem: Out of memory.", 0 );
      bug( "Mem used: %d", sAllocPerm);
    }
    Already = TRUE;
    if ( pMem && !Already )
    {
      char time_buf[MAX_STRING_LENGTH];
      char down_buf[MAX_STRING_LENGTH];
      time_t ntime;
      extern bool sreset;

      Killit = TRUE;
      ntime = current_time + 1;         /*First warning is 1 second from now*/
      strcpy(time_buf, ctime(&ntime));
      strcpy(down_buf, time_buf + 11);
      down_buf[8] = '\0';
      free_string(warning1);
      warning1 = str_dup(down_buf);
      ntime = ntime + 60;               /*Second warning at +61 seconds*/
      strcpy(time_buf, ctime(&ntime));
      strcpy(down_buf, time_buf + 11);
      down_buf[8] = '\0';
      free_string(warning2);
      warning2 = str_dup(down_buf);
      ntime = ntime + 60;               /*Reboot at +121 seconds*/
      strcpy(time_buf, ctime(&ntime));
      strcpy(down_buf, time_buf + 11);
      down_buf[8] = '\0';
      free_string(down_time);
      down_time = str_dup(down_buf);
      stype = 0;                       /*Reboot not shutdown*/
      sreset = FALSE;                  /*SSTime not settable now*/
    }
    else
      exit( 1 );                       /*No mem for anything. Straight reboot*/
  }

  sAllocPerm += sMem;
  nAllocPerm++;
  return pMem;
}

void dispose( void *pMem, int sMem )
{
  if ( !pMem )
  {
    bug( "Dispose: pMem is null.", 0 );
    return;
  }

  free( pMem );
  pMem = NULL;

  sAllocPerm -= sMem;
  nAllocPerm--;
  return;
}


/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
/*
void *alloc_mem( int sMem )
{
    
    return new_mem( sMem );

}
*/


/*
 * Free some memory.
 * Recycle it back onto the free list for blocks of that size.
 */
void free_mem( void *pMem, int sMem )
{
    int iList;

    dispose( pMem, sMem );
    return;

    for ( iList = 0; iList < MAX_MEM_LIST; iList++ )
    {
	if ( sMem <= rgSizeList[iList] )
	    break;
    }

    if ( iList == MAX_MEM_LIST )
    {
	bug( "Free_mem: size %d too large.", sMem );
	exit( 1 );
    }

    * ( (void **) pMem ) = rgFreeList[iList];
    rgFreeList[iList]  = pMem;

    return;
}



/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
void *alloc_perm( int sMem )
{
           void *pMem;
    static char *pMemPerm;
    static int   iMemPerm;

    return new_mem( sMem );

    while ( sMem % sizeof( long ) != 0 )
	sMem++;
    if ( sMem > MAX_PERM_BLOCK )
    {
	bug( "Alloc_perm: %d too large.", sMem );
	exit( 1 );
    }

    if ( !pMemPerm || iMemPerm + sMem > MAX_PERM_BLOCK )
    {
	iMemPerm = 0;
	if ( !( pMemPerm = calloc( 1, MAX_PERM_BLOCK ) ) )
	{
	    perror( "Alloc_perm" );
	    exit( 1 );
	}
    }

    pMem        = pMemPerm + iMemPerm;
    iMemPerm   += sMem;
    nAllocPerm += 1;
    sAllocPerm += sMem;
    return pMem;
}



/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */

/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */

void do_areas( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    char        buf  [ MAX_STRING_LENGTH   ];
    char        buf1 [ MAX_STRING_LENGTH*2 ];
    int         iArea = 0;
    int         iAreaHalf;

    buf1[0] = '\0'; 
    for ( pArea1 = area_first; pArea1; pArea1 = pArea1->next )
      if ( !IS_SET( pArea1->area_flags, AREA_PROTOTYPE ) )
	iArea++;

    iAreaHalf = ( iArea + 1 ) / 2;
    pArea1    = area_first;
    pArea2    = area_first;
   
    for ( iArea = 0; pArea2; pArea2 = pArea2->next )
    {
      if ( !IS_SET( pArea2->area_flags, AREA_PROTOTYPE ))
	iArea++;
      if ( iArea >= iAreaHalf + 1 )
	break;
    }

    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
    {
      if (iArea)
      {
      for ( pArea1 = pArea1->next; pArea1; pArea1 = pArea1->next )
	if ( !IS_SET( pArea1->area_flags, AREA_PROTOTYPE ) && pArea1->vnum )
	  break;
      if ( pArea2 )
	for ( pArea2 = pArea2->next; pArea2; pArea2 = pArea2->next )
	  if ( !IS_SET( pArea2->area_flags, AREA_PROTOTYPE ) && pArea2->vnum )
	    break;
      } else
      {
      for ( ; pArea1; pArea1 = pArea1->next )
	if ( !IS_SET( pArea1->area_flags, AREA_PROTOTYPE ) && pArea1->vnum )
	  break;
      if ( pArea2 )
	for ( ; pArea2; pArea2 = pArea2->next )
	  if ( !IS_SET( pArea2->area_flags, AREA_PROTOTYPE ) && pArea2->vnum )
	    break;
      }

      sprintf( buf, "%-28s %-10s %-28s %-10s\n\r",
	      pArea1->name, pArea1->creator, ( pArea2 ) ? pArea2->name : "", ( pArea2 ) ? pArea2->creator : "" );

      strcat( buf1, buf );

    }

    send_to_char(C_DEFAULT, buf1, ch );
    return;
}




/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
    case 0:  number -= 1; break;
    case 3:  number += 1; break;
    }

    return UMAX( 1, number );
}



/*
 * Generate a random number.
 */
int number_range( int from, int to )
{
    int power;
    int number;

    if ( ( to = to - from + 1 ) <= 1 )
	return from;

    for ( power = 2; power < to; power <<= 1 )
	;

    while ( ( number = number_mm( ) & ( power - 1 ) ) >= to )
	;

    return from + number;
}



/*
 * Generate a percentile roll.
 */
int number_percent( void )
{
    int percent;

    while ( ( percent = number_mm( ) & ( 128-1 ) ) > 99 )
	;

    return 1 + percent;
}



/*
 * Generate a random door.
 */
int number_door( void )
{
    int door;

    while ( ( door = number_mm( ) & ( 8-1 ) ) > 5 )
	;

    return door;
}



int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}



/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static	int	rgiState[2+55];

void init_mm( )
{
    int *piState;
    int  iState;

    piState	= &rgiState[2];

    piState[-2]	= 55 - 55;
    piState[-1]	= 55 - 24;

    piState[0]	= ( (int) current_time ) & ( ( 1 << 30 ) - 1 );
    piState[1]	= 1;
    for ( iState = 2; iState < 55; iState++ )
    {
	piState[iState] = ( piState[iState-1] + piState[iState-2] )
			& ( ( 1 << 30 ) - 1 );
    }
    return;
}



int number_mm( void )
{
    int *piState;
    int  iState1;
    int  iState2;
    int  iRand;

    piState		= &rgiState[2];
    iState1	 	= piState[-2];
    iState2	 	= piState[-1];
    iRand	 	= ( piState[iState1] + piState[iState2] )
			& ( ( 1 << 30 ) - 1 );
    piState[iState1]	= iRand;
    if ( ++iState1 == 55 )
	iState1 = 0;
    if ( ++iState2 == 55 )
	iState2 = 0;
    piState[-2]		= iState1;
    piState[-1]		= iState2;
    return iRand >> 6;
}



/*
 * Roll some dice.
 */
int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
    case 0: return 0;
    case 1: return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
	sum += number_range( 1, size );

    return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_47 )
{
    return value_00 + level * ( value_47 - value_00 ) / 47;
}



/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
    {
	if ( *str == '~' )
	    *str = '-';
    }

    return;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( !astr )
    {
	bug( "Str_cmp: null astr.", 0 );
	return TRUE;
    }

    if ( !bstr )
    {
	bug( "Str_cmp: null bstr.", 0 );
	return TRUE;
    }

    for ( ; *astr || *bstr; astr++, bstr++ )
    {
	if ( LOWER( *astr ) != LOWER( *bstr ) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( !astr )
    {
	bug( "Str_prefix: null astr.", 0 );
	return TRUE;
    }

    if ( !bstr )
    {
	bug( "Str_prefix: null bstr.", 0 );
	return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
	if ( LOWER( *astr ) != LOWER( *bstr ) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
    char c0;
    int  sstr1;
    int  sstr2;
    int  ichar;

    if ( ( c0 = LOWER( astr[0] ) ) == '\0' )
	return FALSE;

    sstr1 = strlen( astr );
    sstr2 = strlen( bstr );

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    {
	if ( c0 == LOWER( bstr[ichar] ) && !str_prefix( astr, bstr + ichar ) )
	    return FALSE;
    }

    return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

    sstr1 = strlen( astr );
    sstr2 = strlen( bstr );
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
	return FALSE;
    else
	return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
    static char strcap [ MAX_STRING_LENGTH ];
           int  i;

    for ( i = 0; str[i] != '\0'; i++ )
	strcap[i] = LOWER( str[i] );
    strcap[i] = '\0';
    strcap[0] = UPPER( strcap[0] );
    return strcap;
}



/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, char *file, char *str )
{
    FILE *fp;
    if ( IS_NPC( ch ) || str[0] == '\0' )
	{
	//return;
	}

    fclose( fpReserve );
    if ( !( fp = fopen( file, "a" ) ) )
    {
	perror( file );
	send_to_char(C_DEFAULT, "Could not open the file!\n\r", ch );
    }
    else
    {
	fprintf( fp, "[%5d] %s: %s\n",
		ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



/*
 * Reports a bug.
 */
void bug( const char *str, int param )
{
    FILE *fp;
    char  buf [ MAX_STRING_LENGTH ];

    if ( fpArea )
    {
	int iLine;
	int iChar;

	if ( fpArea == stdin )
	{
	    iLine = 0;
	}
	else
	{
	    iChar = ftell( fpArea );
	    fseek( fpArea, 0, 0 );
	    for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
	    {
		while ( getc( fpArea ) != '\n' )
		    ;
	    }
	    fseek( fpArea, iChar, 0 );
	}

	sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
	log_string( buf, CHANNEL_BUILD, -1 );

	if ( ( fp = fopen( "shutdown.txt", "a" ) ) )
	{
	    fprintf( fp, "[*****] %s\n", buf );
	    fclose( fp );
	}
    }

    strcpy( buf, "[*****] BUG: " );
    sprintf( buf + strlen( buf ), str, param );
    log_string( buf, CHANNEL_BUILD , -1 );

    fclose( fpReserve );
    if ( ( fp = fopen( BUG_FILE, "a" ) ) )
    {
	fprintf( fp, "%s\n", buf );
	fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );

    return;
}

/*
 * Send logs to imms.
 * Added by Altrag.
 */
void logch( char *l_str, int l_type, int lvl )
{
	DESCRIPTOR_DATA *d;
	int level;
	char log_str[MAX_STRING_LENGTH];
	
	switch ( l_type )
	{
	default:
		strcpy( log_str, "Unknown: " );
		level = L_DIR;
		if ( lvl > level )
		  level = lvl;
		break;
	case 1:
		strcpy( log_str, "Coder: " );
		level = 100000;
		break;
	case CHANNEL_LOG:
		strcpy( log_str, "Log: " );
		level = L_APP;
		if ( lvl > level )
		  level = lvl;
		break;
	case CHANNEL_INFO:
		strcpy( log_str, "&b[&CINFO&b]:&B " );
		level = 1;
		if ( lvl > level )
		  level = lvl;
		break;
	case CHANNEL_BUILD:
		strcpy( log_str, "Build: " );
		level = L_APP;
		if ( lvl > level )
		  level = lvl;
		break;
	case CHANNEL_COMLOG:
		strcpy( log_str, "&p**COMLOG&z:&P ");
		level = L_APP;
		if ( lvl > level )
		  level = lvl;
		break;
	case CHANNEL_GOD:
		strcpy( log_str, "God: " );
		level = L_DIR;
		if ( lvl > level )
		  level = lvl;
		break;
	}
	strcat( log_str, l_str );
	
	for ( d = descriptor_list; d; d = d->next )
	{
		if ( d->connected != CON_PLAYING || IS_SET( d->character->deaf, l_type )
			 || get_trust( d->character ) < level )
			continue;
		send_to_char( AT_PURPLE, log_str, d->character );
		/*
		 * \n\r could have been added earlier,
		 * but need to send a C_DEFAULT line anywayz
		 * Altrag.
		 */
		send_to_char( C_DEFAULT, "\n\r", d->character );
	}
	return;
}

/*
 * Writes a string to the log.
 */
void log_string( char *str, int l_type, int level )
{
    char *strtime;

    strtime                    = ctime( &current_time );
    strtime[strlen( strtime )-1] = '\0';
    if (l_type != CHANNEL_COMLOG )
    {
#ifdef RUN_AS_WIN32SERVICE
	fprintf( __stderr, "%s :: %s\n", strtime, str );
#else
	fprintf( stderr, "%s :: %s\n", strtime, str );
#endif
    }
	/*
	 * The Actual Implementation of the Log Channels.
	 * Added by Altrag.
	 */
    if ( l_type != CHANNEL_NONE )
    	logch( str, l_type, level );
    return;
}

char *wind_str( int str )
{
  int temp = 6;
  
  if ( str> 200 )
   temp = 1;
  else if ( str > 150 )
   temp = 2;
  else if ( str > 100 )
   temp = 3;
  else if ( str > 50 )
   temp = 4;
  else if ( str > 0 )
   temp = 5;
      
  switch( temp )
  {
    case 1: return "impossibly strongly";
    case 2: return "very strongly";
    case 3: return "strongly";
    case 4: return "moderately strongly";
    case 5: return "lightly";
    default: return "wierdly";
  }
  return "";
}


void wind_update( AREA_DATA *pArea )
{
  return;
}  

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}

void area_sort( AREA_DATA *pArea )
{
  AREA_DATA *fArea;

  if ( !pArea )
  {
    bug( "area_sort: NULL pArea", 0); /* MAJOR probs if you ever see this.. */
    return;
  }

  area_last = pArea;

  if ( !area_first )
  {
    area_first = pArea;
    return;
  }

  for ( fArea = area_first; fArea; fArea = fArea->next )
  {
    if ( pArea->lvnum == fArea->lvnum ||
       ( pArea->lvnum > fArea->lvnum &&
       (!fArea->next || pArea->lvnum < fArea->next->lvnum) ) )
    {
      pArea->next = fArea->next;
      fArea->next = pArea;
      return;
    }
  }
  pArea->next = area_first;
  area_first = pArea;
  return;
}

void clan_sort( CLAN_DATA *pClan )
{
  CLAN_DATA *fClan;

  if ( !clan_first )
  {
    clan_first = pClan;
    return;
  }
  for ( fClan = clan_first; fClan; fClan = fClan->next )
  {
    if ( pClan->vnum == fClan->vnum ||
       ( pClan->vnum > fClan->vnum &&
       (!fClan->next || pClan->vnum < fClan->next->vnum) ) )
    {
      pClan->next = fClan->next;
      fClan->next = pClan;
      return;
    }
  }
  pClan->next = clan_first;
  clan_first = pClan;
  return;
}

void load_banlist ( void )
{
  FILE 		*fp;
  char		buf [ MAX_INPUT_LENGTH ];
  char		*p;
  char		*uname;
  BAN_DATA	*pban;
  
  if ( ( fp = fopen ( "banlist.txt", "r" ) ) == NULL )
    return;
    
  while ( fgets ( buf, sizeof ( buf ), fp) != NULL )
  {
    if ( ( p = strchr ( buf, '\n' ) ) != NULL )
      *p = '\0';
    if ( ( uname = strchr ( buf, ' ' ) ) != NULL )
      *(uname++) = '\0';
    
    if ( *buf )
    {
      pban = new_ban();
      pban->name = str_dup ( buf );
      if ( uname != NULL )
        pban->user = str_dup ( uname );
      pban->next = ban_list;
      ban_list = pban;
    }
  }
  
  fclose ( fp );
  
} /* load_banlist() */

/* prototype for below function... */
void insert_into_dead_obj_tree(DEAD_OBJ_DATA * node, DEAD_OBJ_DATA * tree);

/*load up dead object info */

void load_dead_obj_info()
{
  FILE 		*fp;
  int         count, i;
  int         update;
  int            low;
  int            high;
  char           buf[512];
  DEAD_OBJ_LIST * list_node;
  DEAD_OBJ_DATA * tree_node; 
 
  if ( ( fp = fopen ( "deadobj.txt", "r" ) ) == NULL )
 {
  perror("Error! can't find deadobj.txt");
    return;
}
  count = fread_number(fp);
  dead_object_list = 0;
  sprintf(buf, "found %d dead obj vnum ranges", count);
  perror(buf);
 

  for (i = 0 ; i < count; i++)
    {
      update = fread_number(fp);
      low = fread_number(fp);
      high = fread_number(fp);
      sprintf(buf, "Adding to update %d, range %d - %d\n\r ", update, low,
high);
      perror(buf);
	if (!dead_object_list)
	{
		dead_object_list = alloc_perm( sizeof( DEAD_OBJ_LIST ) );
		dead_object_list->update = update;
		dead_object_list->head = 0;
		dead_object_list->next = 0;
	}
      for ( list_node = dead_object_list; list_node; list_node = 
	      list_node->next )
	{
	  if (list_node->update == update)
	    {
	      tree_node = alloc_perm( sizeof( *tree_node) );
	      tree_node->high_vnum = high;
	      tree_node->low_vnum = low;
	      tree_node->left = 0;
	      tree_node->right = 0;
	      if (list_node->head)
		insert_into_dead_obj_tree(tree_node, list_node->head);
	      else
		list_node->head = tree_node;
           sprintf(buf, "added to an existing list item successfully\n\r"
);
             perror(buf);
             break;
	    }
	}
      /* if we didn't find the correct tree, then make a new one */
      if (!list_node)
	{
	  list_node = alloc_perm( sizeof( *list_node) );
	  list_node->update = update;
	  list_node->next = dead_object_list;
	  dead_object_list = list_node;
	}
		
    }
  if (!dead_object_list)
    perror("Error! No dead object list after load_dead_object_inf");
  fclose ( fp );
    
}

void insert_into_dead_obj_tree(DEAD_OBJ_DATA * node, DEAD_OBJ_DATA * tree)
{
  if (tree->low_vnum < node->high_vnum)
    {
    if (tree->left)
      {
       insert_into_dead_obj_tree(node, tree->left);
       return;
      }
    else 
      {
	tree->left = node;
	return;
      }
    }
  else if (tree->high_vnum < node->low_vnum )
    {
      if (tree->right)
	{
	  insert_into_dead_obj_tree(node, tree->right);
          return;
	}
      else
	{
	  tree->right = node;
	  return;
	}
    }
  else
    log_string("BAD Dead object data, vnums overlapping", CHANNEL_NONE, -1);
  return;
}

void load_religions( void )
{
    FILE      *fp;
    RELIGION_DATA *pReligionIndex;
    char letter;

    if ( !( fp = fopen( RELIGION_FILE, "r" ) ) )
	return;
    for ( ; ; )
    {
	int  vnum;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_religions: # not found.", 0 );
	    exit( 1 );
	}

	vnum				= fread_number( fp );
	if ( vnum == 999 )
	    break;

	if ( get_religion_index( vnum ) )
	{
	    bug( "Load_religions: vnum %d duplicated.", vnum );
	    exit( 1 );
	}

	pReligionIndex			= new_religion();
	pReligionIndex->vnum		= vnum;
	pReligionIndex->name		= fread_string( fp );
	pReligionIndex->shortdesc	= fread_string( fp );
	pReligionIndex->deity               = fread_string( fp );
	pReligionIndex->description         = fread_string( fp );
	pReligionIndex->recall		= fread_number( fp );
	pReligionIndex->start		= fread_number( fp );
	pReligionIndex->pkills		= fread_number( fp );
	pReligionIndex->mkills		= fread_number( fp );
	pReligionIndex->pdeaths		= fread_number( fp );
	pReligionIndex->mdeaths		= fread_number( fp );
	pReligionIndex->members		= fread_number( fp );

	religion_sort(pReligionIndex);
    }
    fclose ( fp );   
 
    /* Set Religion Quest Flag */
    relquest = FALSE;

    return;
}

RELIGION_DATA *get_religion_index( int vnum )
{
    RELIGION_DATA *pReligionIndex;

    for ( pReligionIndex  = religion_first;
	  pReligionIndex;
	  pReligionIndex  = pReligionIndex->next )
    {
	if ( pReligionIndex->vnum == vnum )
	    return pReligionIndex;
    }

    return NULL;
}

void religion_sort( RELIGION_DATA *pReligion )
{
  RELIGION_DATA *fReligion;

  if ( !religion_first )
  {
    religion_first = pReligion;
    return;
  }
  for ( fReligion = religion_first; fReligion; fReligion = fReligion->next )
  {
    if ( pReligion->vnum == fReligion->vnum ||
       ( pReligion->vnum > fReligion->vnum &&
       (!fReligion->next || pReligion->vnum < fReligion->next->vnum) ) )
    {
      pReligion->next = fReligion->next;
      fReligion->next = pReligion;
      return;
    }
  }
  pReligion->next = religion_first;
  religion_first = pReligion;
  return;
}

void load_permanent_objects( void )
{
   return;
}
