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

/*$Id: merc.h,v 1.140 2005/04/11 03:25:02 tyrion Exp $*/

// Windows port by Ahsile - mcarpent@zenwerx.com
//#define RUN_AS_WIN32SERVICE /* Only Uncomment for Windows */

#include "colors.h"   /* Include the ansi color routines. */
#include "config.h"   /* Configuration / options */

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined( TRADITIONAL )
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	int	 fun( )
#define DECLARE_SKILL_FUN( fun )    int  fun( )
#define DECLARE_GAME_FUN( fun )		void fun( )
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#define DECLARE_SKILL_FUN( fun )    SKILL_FUN fun
#define DECLARE_GAME_FUN( fun )		GAME_FUN  fun
#endif

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined( FALSE )
#define FALSE	 0
#endif

#if	!defined( TRUE )
#define TRUE	 1
#endif

#if	defined( _AIX )
#if	!defined( const )
#define const
#endif
typedef int				bool;
#define unix
#else
typedef unsigned char			bool;
#endif
#include <stdlib.h>

void ___exit( int arg );
#ifdef RUN_AS_WIN32SERVICE								// Ahsile
FILE* __stderr;											// Ahsile
#define bcopy(p0,p1,len)	memcpy((p0),(p1),(len))		// Ahsile
#define bzero(p0,len)		memset((p0), 0x0, (len))	// Ahsile
#endif

#define alloc_mem malloc

/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct  new_clan_data           CLAN_DATA;
typedef struct  new_religion_data	RELIGION_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct  userl_data              USERL_DATA;
typedef struct  gskill_data             GSPELL_DATA;
typedef struct	char_data		CHAR_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	kill_data		KILL_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct  sac_data		SAC_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	weather_data		WEATHER_DATA;
typedef struct  disabled_data           DISABLED_DATA;
typedef struct  mob_prog_data           MPROG_DATA;
typedef struct  mob_prog_act_list       MPROG_ACT_LIST;
typedef struct	guild_data		GUILD_DATA;	/* XOR */
typedef struct  alias_data              ALIAS_DATA;     /* Altrag */
typedef struct  phobia_data             PHOBIA_DATA;    /* Altrag */
typedef struct  trap_data               TRAP_DATA;      /* Altrag */
typedef struct  dead_obj_data		DEAD_OBJ_DATA;  /* Manaux */
typedef struct 	dead_obj_list		DEAD_OBJ_LIST;  /* Manaux */
typedef struct  pools			POOLS;		/* Ahsile */
typedef struct  corrupt_area_list	CORRUPT_AREA_LIST; /* Ahsile */
typedef struct  social_type			SOCIAL_DATA; /* Ahsile - Just for ease */
/*
 * Function types.
 */
typedef	void DO_FUN                     args( ( CHAR_DATA *ch,
					       char *argument ) );
typedef bool SPEC_FUN                  args( ( CHAR_DATA *ch ) );
typedef int SPELL_FUN                  args( ( int sn, int level,
					       CHAR_DATA *ch, void *vo ) );
typedef int SKILL_FUN                  args( ( int sn, int level,
					       CHAR_DATA *ch, void *vo ) );
typedef void GAME_FUN    	        args( ( CHAR_DATA *ch,
                                	       CHAR_DATA *croupier,
                                	       int     amount,
                                   	       int     cheat,
                                               char *argument ) );

/*
 * String and memory management parameters.
 */
#define	MAX_KEY_HASH		 1024
#define MAX_STRING_LENGTH	 4096
#define MAX_INPUT_LENGTH	 400
#define OFFENDING_STRING	 "__mud_offending_string__"

/*
 more memory stuff... needed for cleanup due to sloppy tracking
   - Ahsile
*/
struct pools
{
	void*  addr;
	POOLS* next;
};

POOLS* pool_first;
POOLS* pool_last;

/*
 * DEBUG CODE FOR MEMORY
 */

#ifdef MEM_DEBUG

char    callee_name[MAX_STRING_LENGTH];

#define free_string( arg ) \
	strcpy(callee_name, __FUNCTION__); \
        f_string( ( arg ) );

#endif

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_CHUNKS                 180           /* Used in ssm.c */
#define MAX_SKILL		   485
#define MAX_GSPELL                 8
#define MAX_CLASS		   21
#define MAX_RACE                   27
#define MAX_CLAN                   21 /*max 20 clans + 1 for clan 0*/
#define MAX_RELIGION		   8  /* max 7 + 1 for 0 */
#define MAX_LEVEL		   116 /* 116 is not used */
#define MAX_LANGUAGE               27
#define MAX_EXPERIENCE		   500000 /* max exp for a mortal */
#define MAX_ARMOR		   750    /* max effective AC */
#define MAX_GROUP		   8      /* Maximum group members */
#define MAX_ATTACKS		   8
#define PKILL_RANGE		   8
#define MAX_POISON_LEVEL	   100	  /* max poison level */
#define NUMBER_OF_DAYS_TO_RESET    30 /* Number of MUD days, not real days */
#define NUMBER_OF_DAYS_TO_REBOOT   360 /* Number of MUD days, not real days */
#define STUN_MAX                   5
#define L_IMP			   MAX_LEVEL	     /* 116 UNUSED */
#define L_TLD                     ( L_IMP - 1 )	     /* 115 */
#define L_KPR			  ( L_TLD - 1 )	     /* 114 */
#define L_CON                     ( L_KPR - 1 )      /* 113 */
#define L_OVD			  ( L_CON - 1 )	     /* 112 */
#define L_SEN			  ( L_OVD - 1 )	     /* 111 */
#define L_ARC			  ( L_SEN - 1 )      /* 110 */
#define L_DIR		          ( L_ARC - 1 )      /* 109 */
#define L_IMM		          ( L_DIR - 1 )      /* 108 */
#define L_DEI                     ( L_IMM - 1 )      /* 107 */
#define L_APP                     ( L_DEI - 1 )      /* 106 */
#define LEVEL_IMMORTAL		  ( L_DEI - 1 )
#define LEVEL_DEMIGOD		  ( L_APP - 1 ) /* max mortal */
#define LEVEL_CHAMP		  ( LEVEL_DEMIGOD - 1 )
#define LEVEL_HERO3		  ( LEVEL_CHAMP - 1)
#define LEVEL_HERO2		  ( LEVEL_HERO3 - 1)
#define LEVEL_HERO1		  ( LEVEL_HERO2 - 1)
#define LEVEL_HERO		  ( LEVEL_HERO1 - 1)

#define PULSE_PER_SECOND	    4
#define PULSE_VIOLENCE		  (  3 * PULSE_PER_SECOND )
#define PULSE_MOBILE		  (  4 * PULSE_PER_SECOND )
#define PULSE_TICK		  ( 30 * PULSE_PER_SECOND )
#define PULSE_AREA		  ( 60 * PULSE_PER_SECOND )
#define PULSE_QUEST		  ( 60 * PULSE_PER_SECOND )


/* Save the database - OLC 1.1b */
#define PULSE_DB_DUMP		  (1800* PULSE_PER_SECOND ) /* 30 minutes  */

/*
 * Due to a slow machine, we coded the SQL system to use memory tables
 * and only dump everything on a timed basis. Unfortunately we never
 * made a full conversion to a db system by only pulling data on demand.
 * Data is all stored in the db, and loaded at startup just like a
 * normal mud, but rather than loading from files we load from the
 * database.
 * 	- Ahsile - mcarpent@zenwerx.com
 */
#ifdef SQL_SYSTEM
#define PULSE_DIRTY (300 * PULSE_PER_SECOND ) /* Save "dirty" characters */
#endif

/*
 * Language stuff
 */

#define COMMON          0
#define HUMAN           1
#define DWARVISH        2
#define ELVISH          3
#define GNOMISH         4
#define DRAGON          5
#define DEMON           6
#define OGRE            7
#define DROW            8
#define ELDER           9
#define PIXIE           10
#define HOBBIT          11
#define MINOTAUR	12
#define LIZARD          13
#define HALFLING        14
#define FELINE          15
#define CANINE          16
#define ANGEL		17
#define ORCISH		18
#define MAGICK		19
#define SHADOW_SPEAK	20
#define SPIRITSPEAK	21
#define ENLIGHTENED	22
#define SATANIC		23
#define ANIMALSPEAK	24
#define BRETONNIAN	25
#define GARGISH		26


#define CLASS_MAGE       0
#define CLASS_CLERIC     1
#define CLASS_THIEF      2
#define CLASS_WARRIOR    3
#define CLASS_PSIONICIST 4
#define CLASS_DRUID      5
#define CLASS_RANGER     6
#define CLASS_PALADIN    7
#define CLASS_BARD       8
#define CLASS_VAMPIRE    9
#define CLASS_WEREWOLF   10
#define CLASS_ANTI_PALADIN 11
#define CLASS_ASSASSIN   12
#define CLASS_MONK       13
#define CLASS_BARBARIAN  14
#define CLASS_ILLUSIONIST 15
#define CLASS_NECROMANCER 16
#define CLASS_DEMONOLOGIST 17
#define CLASS_SHAMAN 18
#define CLASS_DARKPRIEST 19
#define CLASS_EMPTY2     20

#define MAX_SAC_TYPES   4

#define SAC_HP 		0
#define SAC_GOLD	1
#define SAC_MOVE        2
#define SAC_MANA       	3

/*
 *  Religious quest paramters - Ahsiles
 */

#define    ALIGN_EVIL           -1
#define    ALIGN_NEUTRAL        0
#define    ALIGN_GOOD           1

/* Relgion area vnum lower AND upper */
#define  REL_VNUM_LOWER         24601
#define  REL_VNUM_UPPER         24900

/*
 * SQL Version by Ahsile - mcarpent@zenwerx.com
 */
#ifdef SQL_SYSTEM

#define USER   "dbuser"
#define HOST   "localhost"
#define PASS   "dbuser"
#define DBNAME "mud"

#endif

/*
 *  End Religions quest paramaters
 *  More to be found in rel_quest.c
 */


/*
 *  User list structure
 */
struct  userl_data
{
    USERL_DATA *  next;
    char *        name;
    int           level;
    char *        user;
    char *        host;
    char *        lastlogin;
    char *        desc;
    int           class;
    int           multi;
};


/*
 * Site ban structure.
 */
struct	ban_data
{
    BAN_DATA *	next;
    char *	name;
    char *      user;
};

/*
 * Drunk struct
 */
struct struckdrunk
{
        int     min_drunk_level;
        int     number_of_rep;
        char    *replacement[11];
};

/*
 * Dead Object vnum Range struct, for use in conjunction with player
 * updater to cleanly remove objects from players --Manaux
 */

struct dead_obj_data
{
	int low_vnum;
	int high_vnum;
	DEAD_OBJ_DATA * left;
	DEAD_OBJ_DATA * right;
};

struct dead_obj_list
{
   /*hold the player update version that they are updating from.   */

	int update;

	DEAD_OBJ_DATA * head;
	DEAD_OBJ_LIST * next;
};

/* Corrupt Area Struct - Ahsile*/
struct corrupt_area_list
{
	int	   	   vnum;
	AREA_DATA* 	   area;
	CORRUPT_AREA_LIST* next;
};



/*
 * Disable struct
 */

struct disabled_data
{
        DISABLED_DATA           *next;          /* pointer to the next one */
        struct cmd_type const   *command;       /* pointer to the command struct */
        char                    *disabled_by;   /* name of disabler */
        int                      dislevel;      /* level of disabler */
        int                      uptolevel;     /* level of execution allowed */
};


bool    MOBtrigger;

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           1
#define SPEECH_PROG        2
#define RAND_PROG          4
#define FIGHT_PROG         8
#define DEATH_PROG        16
#define HITPRCNT_PROG     32
#define ENTRY_PROG        64
#define GREET_PROG       128
#define ALL_GREET_PROG   256
#define GIVE_PROG        512
#define BRIBE_PROG      1024

/*
 * Time and weather stuff.
 */
#define SUN_DARK		    0
#define SUN_RISE		    1
#define SUN_LIGHT		    2
#define SUN_SET			    3

#define SKY_CLOUDLESS		    0
#define SKY_CLOUDY		    1
#define SKY_RAINING		    2
#define SKY_LIGHTNING		    3

struct	time_info_data
{
    int		hour;
    int		day;
    int		month;
    int		year;
    int		total;
};

struct	weather_data
{
    int		mmhg;
    int		change;
    int		sky;
    int		sunlight;
};



/*
 * Connected state for a channel.
 */
#define CON_PLAYING			0
#define CON_GET_NAME			1
#define CON_GET_OLD_PASSWORD		2
#define CON_CONFIRM_NEW_NAME		3
#define CON_GET_NEW_PASSWORD		4
#define CON_CONFIRM_NEW_PASSWORD	5
#define CON_DISPLAY_RACE		6
#define CON_GET_NEW_RACE	        7
#define CON_CONFIRM_RACE                8
#define CON_GET_NEW_SEX			9
#define CON_DISPLAY_CLASS	       10
#define CON_GET_NEW_CLASS	       11
#define CON_CONFIRM_CLASS              12
#define CON_READ_MOTD		       13
#define CON_GET_PKILL	       	       14
#define CON_CONFIRM_PKILL  	       15
#define CON_DISPLAY_MULTICLASS	       16
#define CON_GET_MULTICLASS	       17
#define CON_CONFIRM_MULTICLASS	       18
#define CON_CHOICE_MULTICLASS	       20
#define CON_DISPLAY_RELIGION	       21
#define CON_CHOSE_RELIGION	       22
#define CON_CONFIRM_RELIGION	       23
#define CON_DISPLAY_ATTRIBUTES	       24
#define CON_CHOSE_ATTRIBUTES	       25
#define CON_CONFIRM_ATTRIBUTES	       26

#define CON_GET_ANSI			105
#define CON_AUTHORIZE_NAME		100
#define CON_AUTHORIZE_NAME1		101
#define CON_AUTHORIZE_NAME2		102
#define CON_AUTHORIZE_NAME3		103
#define CON_AUTHORIZE_LOGOUT		104

#define CON_CHATTING                    200

/*
 * Maximum number of damage modifier types for res/vuln
 */
#define DAMCLASS_MAX 21
/*
 * Values for type in const2.c
 */
#define TYPE_SPELL 1
#define TYPE_SKILL 2
#define TYPE_NONE  0
/* TYPE_NONE is for non-usable skills, like  parry, enhanced hit, etc*/

/*
 * Race structures
 */
struct  race_type
{
    char *              race_name;
    char *              race_full;
    int                 race_lang;
    int                 race_abilities;
    int                 size;
    int                 str_mod;
    int                 int_mod;
    int                 wis_mod;
    int                 dex_mod;
    int                 con_mod;
    int                 not_class;
    char *              dmg_message;
    char *              hate;
    char *              default_weap;
};

/* Race ability bits */
#define RACE_NO_ABILITIES             0
#define RACE_PC_AVAIL                 1
#define RACE_WATERBREATH              2
#define RACE_FLY                      4
#define RACE_SWIM                     8
#define RACE_WATERWALK               16
#define RACE_PASSDOOR                32
#define RACE_INFRAVISION             64
#define RACE_DETECT_ALIGN           128
#define RACE_DETECT_INVIS           256
#define RACE_DETECT_HIDDEN          512
#define RACE_PROTECTION            1024
#define RACE_SANCT                 2048
#define RACE_WEAPON_WIELD          4096
#define RACE_DETECT_HIDDEN_EXIT	   8192

/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    char *		 host;
    char *              user;
    unsigned int		descriptor;
    char                abuf [ 100 ];
    int                 atimes;
    int			auth_fd;
    int			auth_state;
    int	                auth_inc;
    int		        connected;
    bool		fcommand;
    char		inbuf		[ MAX_INPUT_LENGTH*4 ];
    char		incomm		[ MAX_INPUT_LENGTH   ];
    char		inlast		[ MAX_INPUT_LENGTH   ];
    int			repeat;
    char *              showstr_head;
    char *              showstr_point;
    char *		outbuf;
    int			outsize;
    int			outtop;
    void *              pEdit;		/* OLC */
    void *              inEdit;         /* Altrag, for nested editors */
    char **             pString;	/* OLC */
    int			editor;		/* OLC */
    int                 editin;         /* Altrag, again for nesting */
    bool		ansi;
};



/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    int 	        tohit;
    int         	todam;
    int                 carry;
    int         	wield;
};

struct	int_app_type
{
    int         	learn;
    int			bonus;
};

struct	wis_app_type
{
    int         	practice;
};

struct	dex_app_type
{
    int         	defensive;
    int			bonus;
};

struct	con_app_type
{
    int         	hitp;
    int         	shock;
};



/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define TO_COMBAT           4



/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA * 	next;
    AREA_DATA *		area;		/* OLC 1.1b */
    int 	        level;
    char *      	keyword;
    char *      	text;
};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    int 	keeper;			/* Vnum of shop keeper mob	*/
    int 	buy_type [ MAX_TRADE ];	/* Item types shop will buy	*/
    int 	profit_buy;		/* Cost multiplier for buying	*/
    int 	profit_sell;		/* Cost multiplier for selling	*/
    int 	open_hour;		/* First opening hour		*/
    int 	close_hour;		/* First closing hour		*/
};



/*
 * Per-class stuff.
 */
struct	class_type
{
    char 	who_name	[ 4 ];	/* Three-letter name for 'who'	*/
    char        who_long        [ 15 ]; /* Long name of Class           */
    int 	attr_prime;		/* Prime attribute		*/
    int 	weapon;			/* First weapon			*/
    int 	guild;			/* Vnum of guild room		*/
    int 	skill_adept;		/* Maximum skill level		*/
    int 	thac0_00;		/* Thac0 for level  0		*/
    int 	thac0_97;		/* Thac0 for level 47		*/
    int  	hp_min;			/* Min hp gained on leveling	*/
    int	        hp_max;			/* Max hp gained on leveling	*/
    bool	fMana;			/* Class gains mana on level	*/
    int         no_shields;		/* Number of Shields Allowed	*/
};

struct  clan_type
{
    char        clan_name      [ 15 ];
    char        clan_deity     [ 15 ];
    char        lev1           [ 20 ];
    char        lev2           [ 20 ];
    char        lev3           [ 20 ];
    char        lev4           [ 20 ];
    char        lev5           [ 20 ];
    int         cloc;
    int         clvnum1;
    int         clvnum2;
    int         clvnum3;
    int         sac_type;
};

/*
 * Data structure for notes.
 */
struct	note_data
{
    NOTE_DATA *	next;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    char *	text;
    time_t      date_stamp;
    bool        protected;
    int         on_board;
};



/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    int 		type;
    int			level;
    int 		duration;
    int 		location;
    int 		modifier;
    int			bitvector;
    char		count;
    bool                deleted;
};



/*
 * A kill structure (indexed by level).
 */
struct	kill_data
{
    int                 number;
    int                 killed;
};


/* RT ASCII conversions -- used so we can have letters in this file */

#define A		  	1
#define B			2
#define C			4
#define D			8
#define E			16
#define F			32
#define G			64
#define H			128

#define I			256
#define J			512
#define K		        1024
#define L		 	2048
#define M			4096
#define N		 	8192
#define O			16384
#define P			32768

#define Q			65536
#define R			131072
#define S			262144
#define T			524288
#define U			1048576
#define V			2097152
#define W			4194304
#define X			8388608

#define Y			16777216
#define Z			33554432
#define aa			67108864 	/* doubled due to conflicts */
#define bb			134217728
#define cc			268435456
#define dd			536870912
#define ee			1073741824

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_CITYGUARD	   127
#define MOB_VNUM_DEMON1 	   4
#define MOB_VNUM_DEMON2            4
#define MOB_VNUM_DOGGY             25036
/*#define MOB_VNUM_ULT               3160
#define MOB_VNUM_SECRETARY         3142
#define MOB_VNUM_MIDGAARD_MAYOR    3143
*/

#define MOB_VNUM_AIR_ELEMENTAL      50
#define MOB_VNUM_EARTH_ELEMENTAL    51
#define MOB_VNUM_WATER_ELEMENTAL    52
#define MOB_VNUM_FIRE_ELEMENTAL     53
#define MOB_VNUM_DUST_ELEMENTAL     54
#define MOB_VNUM_ICE_ELEMENTAL      55
#define MOB_VNUM_DRAGON             56

#define MOB_VNUM_GREATER_DEMON      13
#define MOB_VNUM_WOLF               14
#define MOB_VNUM_HAWK               15
#define MOB_VNUM_TIGER              16
#define MOB_VNUM_UNDEAD	         11905
/* XOR */
#define MOB_VNUM_DEMON		   5
#define MOB_VNUM_INSECTS	   6
#define MOB_VNUM_WOLFS		   8
#define MOB_VNUM_SUPERMOB	   7

/*ELVIS*/
#define MOB_VNUM_ANGEL             9
#define MOB_VNUM_SHADOW            10
#define MOB_VNUM_BEAST             11
#define MOB_VNUM_TRENT             12

/* BILL GATES */
#define GUILD_NORMAL		0
#define GUILD_PKILL		(A)
#define GUILD_CHAOS		(B)

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		      1		/* Auto set for mobs	*/
#define ACT_SENTINEL		      2		/* Stays in one room	*/
#define ACT_SCAVENGER		      4		/* Picks up objects	*/
#define ACT_IS_HEALER                 8         /* Healers by MANIAC    */
#define ACT_AGGRESSIVE		     32		/* Attacks PC's		*/
#define ACT_STAY_AREA		     64		/* Won't leave area	*/
#define ACT_WIMPY		    128		/* Flees when hurt	*/
#define ACT_PET			    256		/* Auto set for pets	*/
#define ACT_TRAIN		    512		/* Can train PC's	*/
#define ACT_PRACTICE		   1024		/* Can practice PC's	*/
#define ACT_GAMBLE                 2048         /* Runs a gambling game */
#define ACT_PROTOTYPE              4096         /* Prototype flag       */
#define ACT_UNDEAD                 8192         /* Can be turned        */
#define ACT_TRACK                 16384         /* Track players        */
#define ACT_MOVED		  32768		/* Dont ever set!	*/
#define ACT_TEACHER               65536         /* Teacher of languages */
#define ACT_BANKER               131072         /* Is a banker          */
#define ACT_CLASSMASTER		 262144		/* Is classmaster	*/
#define ACT_NO_EXP		 524288		/* Does not give xp	*/
#define ACT_VEHICLE		1048576		/* Is a vehicle		*/
#define ACT_IS_CLAN_HEALER	2097152		/* Is a clan healer     */
#define ACT_NO_CORPSE		4194304		/* Mob leaves no corpse */
#define ACT_IS_DEITY		8388608		/* Is a Religious Deity */
#define ACT_RELBOSS	       16777216		/* Religious level boss */

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND		      1
#define AFF_INVISIBLE		      2
#define AFF_DETECT_EVIL		      4
#define AFF_DETECT_INVIS	      8
#define AFF_DETECT_MAGIC	     16
#define AFF_DETECT_HIDDEN	     32
#define AFF_HASTE		     64
#define AFF_SANCTUARY		    128
#define AFF_FAERIE_FIRE		    256
#define AFF_INFRARED		    512
#define AFF_CURSE		   1024
#define AFF_FLAMING                2048
#define AFF_POISON		   4096
#define AFF_PROTECT		   8192
#define AFF_INERTIAL    	  16384		/* Used	*/
#define AFF_SNEAK		  32768
#define AFF_HIDE		  65536
#define AFF_SLEEP		 131072
#define AFF_CHARM		 262144
#define AFF_FLYING		 524288
#define AFF_PASS_DOOR		1048576
#define AFF_STUN                2097152
#define AFF_SUMMONED            4194304
#define AFF_MUTE                8388608
#define AFF_PEACE              16777216
#define AFF_FIRESHIELD         33554432
#define AFF_SHOCKSHIELD        67108864
#define AFF_ICESHIELD         134217728
#define AFF_CHAOS             268435456
#define AFF_SCRY              536870912
#define AFF_ANTI_FLEE        1073741824

#define AFF_POLYMORPH		1
#define CODER			2
#define AFF_NOASTRAL            4
#define AFF_DOOMSHIELD          8
#define AFF_TRUESIGHT          16
#define AFF_BLADE              32
#define AFF_DETECT_GOOD        64
#define AFF_PROTECTION_GOOD    128
#define AFF_BERSERK            256
#define AFF_UNHOLY_STRENGTH    512
#define AFF_CONFUSED          1024
#define AFF_FUMBLE            2048
#define AFF_DANCING           4096
#define AFF_IMAGE             8192
#define AFF_PHASED           16384
#define AFF_GOLDEN_ARMOR     32768
#define AFF_GHOST_SHIELD     65536
#define AFF_CURSE_NATURE    131072
#define AFF_MIST	    262144
#define AFF_SHADOW_IMAGE    524288
#define AFF_WEAPONMASTER   1048576
#define AFF_IMPROVED_INVIS 2097152
#define AFF_BLINDFOLD      4194304
#define AFF_SLIT           8388608
#define AFF_THICK_SKIN           Y
#define AFF_MALIGNIFY            Z
#define AFF_CLOAKING            aa
#define AFF_SHADOW_PLANE        bb
#define AFF_WOLFED              cc
#define AFF_HOLD                dd
#define AFF_CHANGE_SEX          ee

#define AFF_WATERWALK		 1
#define AFF_GILLS		 2
#define AFF_VAMP_BITE		 4
#define AFF_GHOUL		 8
#define AFF_CHALLENGER          16
#define AFF_CHALLENGED          32
#define AFF_RAGE	        64
#define AFF_BLOODTHIRSTY        128
#define AFF_COFIRE		256
#define AFF_TORTURE		512
#define AFF_AGE			1024
#define AFF_SATANIC_INFERNO     2048
#define AFF_PESTILENCE		4096
#define AFF_AURA_ANTI_MAGIC	8192
#define AFF_HOLY_PROTECTION	16384
#define AFF_IMPROVED_HIDE	32768
#define AFF_STEALTH		65536
#define AFF_BLOODSHIELD		131072
#define AFF_BEND_LIGHT          262144
#define AFF_CLOUD_OF_HEALING    524288
#define AFF_TALE_OF_TERROR      1048576
#define AFF_POWER_LEAK          2097152
#define AFF_MANA_SHIELD		4194304
#define AFF_WAR_CHANT		8388608
#define AFF_NAGAROMS_CURSE       Y
#define AFF_PRAYER	         Z
#define AFF_DEADLY_POISON       aa
#define AFF_RANDOMSHIELD        bb
#define AFF_PRIMALSCREAM        cc
#define AFF_DEMONSHIELD	        dd
#define AFF_ACIDSHIELD          ee

#define AFF_IMMORTAL	         1
#define AFF_NO_SUMMON	  	 2
#define AFF_GOLDEN_SANCTUARY     4
#define AFF_MOUNTABLE		 8
#define AFF_BIOFEEDBACK         16
#define AFF_EARTHSHIELD		32
#define AFF_LEAF_SHIELD		64
#define AFF_LUCK_SHIELD	       128
#define AFF_TONGUES	       256
#define AFF_LIQUID_SKIN        512
#define AFF_ANGELIC_AURA      1024
#define AFF_ETHEREAL_WOLF     2048
#define AFF_DECEPTION         4096
#define AFF_ETHEREAL_SNAKE    8192
#define AFF_BURROW	     16384
#define AFF_THIEVESCANT      32768
#define AFF_NEWBIE_SLAYER    65536
#define AFF_FORCE_OF_NATURE 131072
#define AFF_ESSENCE_OF_GAIA 262144

#define RECALL_RELIGION_DEFAULT 25000
#define RECALL_RELIGION_ONE	25000
#define RECALL_RELIGION_TWO	25000
#define RECALL_RELIGION_THREE	25000
#define RECALL_RELIGION_FOUR	25000
#define RECALL_RELIGION_FIVE	25000
#define RECALL_RELIGION_SIX	25000
#define RECALL_RELIGION_SEVEN	25000

/* Bits for religions - powers and weaknesses */

/* damage classes */
#define DAM_NONE                0
#define DAM_BASH                1
#define DAM_PIERCE              2
#define DAM_SLASH               3
#define DAM_FIRE                4
#define DAM_COLD                5
#define DAM_LIGHTNING           6
#define DAM_ACID                7
#define DAM_POISON              8
#define DAM_NEGATIVE            9
#define DAM_HOLY                10
#define DAM_ENERGY              11
#define DAM_MENTAL              12
#define DAM_DISEASE             13
#define DAM_DROWNING            14
#define DAM_LIGHT		15
#define DAM_OTHER               16
#define DAM_HARM		17
#define DAM_CHARM		18
#define DAM_SOUND		19

/* return values for check_imm */
#define IS_NORMAL		0
#define IS_IMMUNE		1
#define IS_RESISTANT		2
#define IS_VULNERABLE		3

/* values for dispel_bit */

#define DISPEL_NO		0
#define DISPEL_YES		1

/* Is it a shield */

#define SHIELD_NO               0
#define SHIELD_YES              1

/* values for cancel_bit */

#define CANCEL_NO		0
#define CANCEL_YES		1

/*  values for skill bit */
#define SKPELL_SPELL		1
#define SKPELL_SKILL		0

/* values for spell_damage_type */

#define DAMCLASS_NULL		0
#define DAMCLASS_ACID		1
#define DAMCLASS_HOLY		2
#define DAMCLASS_MAGIC		4
#define DAMCLASS_FIRE		8
#define DAMCLASS_ENERGY		16
#define DAMCLASS_WIND		32
#define DAMCLASS_WATER		64
#define DAMCLASS_ILLUSION	128
#define DAMCLASS_DISPEL		256
#define DAMCLASS_EARTH		512
#define DAMCLASS_PSYCHIC	1024
#define DAMCLASS_POISON		2048
#define DAMCLASS_BREATH		4096
#define DAMCLASS_SUMMON		8192
#define DAMCLASS_PHYSICAL	16384
#define DAMCLASS_EXPLOSIVE	32768
#define DAMCLASS_SONG		65536
#define DAMCLASS_NAGAROM	131072
#define DAMCLASS_UNHOLY		262144
#define DAMCLASS_CLAN		524288

/* IMM bits for mobs */
#define IMM_SUMMON              (A)
#define IMM_CHARM               (B)
#define IMM_MAGIC               (C)
#define IMM_WEAPON              (D)
#define IMM_BASH                (E)
#define IMM_PIERCE              (F)
#define IMM_SLASH               (G)
#define IMM_FIRE                (H)
#define IMM_COLD                (I)
#define IMM_LIGHTNING           (J)
#define IMM_ACID                (K)
#define IMM_POISON              (L)
#define IMM_NEGATIVE            (M)
#define IMM_HOLY                (N)
#define IMM_ENERGY              (O)
#define IMM_MENTAL              (P)
#define IMM_DISEASE             (Q)
#define IMM_DROWNING            (R)
#define IMM_LIGHT		(S)
#define IMM_SOUND		(T)
#define IMM_WOOD                (X)
#define IMM_SILVER              (Y)
#define IMM_IRON                (Z)

/* RES bits for mobs */
#define RES_SUMMON		(A)
#define RES_CHARM		(B)
#define RES_MAGIC               (C)
#define RES_WEAPON              (D)
#define RES_BASH                (E)
#define RES_PIERCE              (F)
#define RES_SLASH               (G)
#define RES_FIRE                (H)
#define RES_COLD                (I)
#define RES_LIGHTNING           (J)
#define RES_ACID                (K)
#define RES_POISON              (L)
#define RES_NEGATIVE            (M)
#define RES_HOLY                (N)
#define RES_ENERGY              (O)
#define RES_MENTAL              (P)
#define RES_DISEASE             (Q)
#define RES_DROWNING            (R)
#define RES_LIGHT		(S)
#define RES_SOUND		(T)
#define RES_WOOD                (X)
#define RES_SILVER              (Y)
#define RES_IRON                (Z)

/* VULN bits for mobs */
#define VULN_SUMMON		(A)
#define VULN_CHARM		(B)
#define VULN_MAGIC              (C)
#define VULN_WEAPON             (D)
#define VULN_BASH               (E)
#define VULN_PIERCE             (F)
#define VULN_SLASH              (G)
#define VULN_FIRE               (H)
#define VULN_COLD               (I)
#define VULN_LIGHTNING          (J)
#define VULN_ACID               (K)
#define VULN_POISON             (L)
#define VULN_NEGATIVE           (M)
#define VULN_HOLY               (N)
#define VULN_ENERGY             (O)
#define VULN_MENTAL             (P)
#define VULN_DISEASE            (Q)
#define VULN_DROWNING           (R)
#define VULN_LIGHT		(S)
#define VULN_SOUND		(T)
#define VULN_WOOD               (X)
#define VULN_SILVER             (Y)
#define VULN_IRON		(Z)


/* weapon class */
#define WEAPON_EXOTIC		0
#define WEAPON_SWORD		1
#define WEAPON_DAGGER		2
#define WEAPON_SPEAR		3
#define WEAPON_MACE		4
#define WEAPON_AXE		5
#define WEAPON_FLAIL		6
#define WEAPON_WHIP		7
#define WEAPON_POLEARM		8

/* weapon types */
#define WEAPON_FLAMING		(A)
#define WEAPON_FROST		(B)
#define WEAPON_VAMPIRIC		(C)
#define WEAPON_SHARP		(D)
#define WEAPON_VORPAL		(E)
#define WEAPON_TWO_HANDED	(F)
#define WEAPON_SHOCKING		(G)
#define WEAPON_POISON		(H)
#define WEAPON_DISPEL		(I)

/* gate flags */
#define GATE_NORMAL_EXIT	(A)
#define GATE_NOCURSE		(B)
#define GATE_GOWITH		(C)
#define GATE_BUGGY		(D)
#define GATE_RANDOM		(E)


/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000


/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL		      0
#define SEX_MALE		      1
#define SEX_FEMALE		      2

/*
 * Crafting defines.
 * Whether or not spells can be added to
 * player made items.
 */
#define CRAFT_NONE 			0
#define CRAFT_POTION			1
#define CRAFT_SCROLL			2
#define CRAFT_ARMOR			4
#define CRAFT_WEAPON			8

/*
 * Tanning Defines
 * Used for skinning/etc
 */
#define TAN_NONE			  0
#define TAN_NPC				  1
#define TAN_PC				  2 /* pk? imm? */

#define SKIN_DESTROYED			0
#define SKIN_HORRIBLE			1
#define SKIN_LOUSY			2
#define SKIN_PASSABLE			3
#define SKIN_UNEXCEPTIONAL		4
#define SKIN_DECENT			5
#define SKIN_GOOD			6
#define SKIN_EXCELLENT			7
#define SKIN_SUPERIOR			8
#define SKIN_PERFECT			9


/* Ammo defines */
#define AMMO_NONE			0
#define AMMO_ARROW			1
#define AMMO_BOLT			2
#define AMMO_BULLET			3

/* Shooting defines */
#define SHOOT_UNKNOWN			0
#define SHOOT_DOOR			1
#define SHOOT_DIST			2
#define SHOOT_FOUND			3
#define SHOOT_HERE			4

/*
 * Skill defines.
 * Possibly for calculating level based damages for lower level skills
 */
/*
#define SKILL_ACTUAL_DAMAGE  > 0
*/
#define	SKPELL_ZERO_DAMAGE	   0
#define	SKPELL_NO_DAMAGE	  -1
#define SKPELL_MISSED		  -2
#define SKPELL_BOTCHED		  -3

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE	      2
#define OBJ_VNUM_MONEY_SOME	      3
#define OBJ_VNUM_BLADE_DOOM	      5
#define OBJ_VNUM_SPIRITUAL_HAMMER     9

#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_HEAD	     12
#define OBJ_VNUM_TORN_HEART	     13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_FINAL_TURD	     16
#define OBJ_VNUM_PORTAL              17
#define OBJ_VNUM_DOLL                18
#define OBJ_VNUM_BERRY               19

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING		     22
#define OBJ_VNUM_VINE_PORTAL         23
#define OBJ_VNUM_FORAGE_FOOD         24
#define OBJ_VNUM_SPRING_DIVINING     25
#define OBJ_VNUM_CORPSE_VEHICLE      26
#define OBJ_VNUM_TO_FORGE_A          27
#define OBJ_VNUM_TO_FORGE_W          28
#define OBJ_VNUM_SOULGEM	     40
#define OBJ_VNUM_VORTEX		     41
#define OBJ_VNUM_VORTEX_NULL	     42
#define OBJ_VNUM_RUNE		     44

#define OBJ_VNUM_LEATHER	     62
#define OBJ_VNUM_TANNED		     63
#define OBJ_VNUM_NEEDLE		     64
#define OBJ_VNUM_PARCHMENT        25050
#define OBJ_VNUM_QUILL            25051
#define OBJ_VNUM_FLASK            25052
#define OBJ_VNUM_CAULDRON         25053
#define OBJ_VNUM_MFIRE            25054
#define OBJ_VNUM_MINK             25055
#define OBJ_VNUM_PESTLE		  25070

#define OBJ_VNUM_SCHOOL_MACE	   148
#define OBJ_VNUM_SCHOOL_DAGGER	   138
#define OBJ_VNUM_SCHOOL_SWORD	   147
#define OBJ_VNUM_SCHOOL_VEST	   136
#define OBJ_VNUM_SCHOOL_SHIELD	   146
#define OBJ_VNUM_SCHOOL_BANNER     144
#define OBJ_VNUM_SCHOOL_CLUB       145
/* 15001 - 15007 */

/*#define OBJ_VNUM_BLACK_POWDER      8903
#define OBJ_VNUM_FLAMEBLADE        8920
*/

/* Was 60 and 61 */
#define OBJ_VNUM_DIAMOND_RING   60    /* Wedding rings for Marriage code */
#define OBJ_VNUM_WEDDING_BAND   61    /* Canth (canth@xs4all.nl) of Mythran */

/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		      1
#define ITEM_SCROLL		      2
#define ITEM_WAND		      3
#define ITEM_STAFF		      4
#define ITEM_WEAPON		      5
#define ITEM_TREASURE		      8
#define ITEM_ARMOR		      9
#define ITEM_POTION		     10
#define ITEM_NOTEBOARD               11
#define ITEM_FURNITURE		     12
#define ITEM_TRASH		     13
#define ITEM_CONTAINER		     15
#define ITEM_DRINK_CON		     17
#define ITEM_KEY		     18
#define ITEM_FOOD		     19
#define ITEM_MONEY		     20
#define ITEM_BOAT		     22
#define ITEM_CORPSE_NPC		     23
#define ITEM_CORPSE_PC		     24
#define ITEM_FOUNTAIN		     25
#define ITEM_PILL		     26
#define ITEM_LENSE                   27
#define ITEM_BLOOD                   28
#define ITEM_PORTAL                  29
#define ITEM_VODOO		     30
#define ITEM_BERRY                   31
#define ITEM_GUN		     32
#define ITEM_RUNE		     33
#define ITEM_WRECK		     34
#define ITEM_IMPLANTED		     35
#define ITEM_SKIN		     36
#define ITEM_ARROW		     37
#define ITEM_BOLT		     38
#define ITEM_BULLET		     39
#define ITEM_BOOK		     40
#define ITEM_NEEDLE		     41
#define ITEM_QUILL		     42
#define ITEM_HAMMER		     43
#define ITEM_PESTLE		     44
#define ITEM_TIMBER			 45
#define ITEM_ORE			 46

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		      1		/*	1	*/
#define ITEM_HUM		      2
#define ITEM_DARK		      4
#define ITEM_LOCK		      8
#define ITEM_EVIL		     16
#define ITEM_INVIS		     32
#define ITEM_MAGIC		     64
#define ITEM_NODROP		    128
#define ITEM_BLESS		    256
#define ITEM_ANTI_GOOD		    512		/*	10	*/
#define ITEM_ANTI_EVIL		   1024
#define ITEM_ANTI_NEUTRAL	   2048
#define ITEM_NOREMOVE		   4096
#define ITEM_INVENTORY		   8192
#define ITEM_POISONED             16384
#define ITEM_ANTI_MAGE            32768
#define ITEM_ANTI_CLERIC          65536
#define ITEM_ANTI_THIEF          131072
#define ITEM_ANTI_WARRIOR        262144
#define ITEM_ANTI_PSI            524288		/*	20	*/
#define ITEM_ANTI_DRUID         1048576
#define ITEM_ANTI_RANGER        2097152
#define ITEM_ANTI_PALADIN       4194304
#define ITEM_ANTI_BARD          8388608
#define ITEM_ANTI_VAMP         16777216
#define ITEM_FLAME             33554432
#define ITEM_CHAOS             67108864
#define ITEM_NO_LOCATE		(cc)
#define ITEM_NO_DAMAGE          (dd)
#define ITEM_PATCHED            (ee)
#define ITEM_ICY                (bb)
#define ITEM_ACID             134217728

#define ITEM_ANTI_BARBARIAN           1
#define	ITEM_ANTI_ANTIPAL             2
#define	ITEM_ANTI_MONK	              4
#define	ITEM_ANTI_ASSASSIN   	      8
#define	ITEM_ANTI_WEREWOLF   	     16
#define	ITEM_ANTI_ILLUSIONIST        32
#define ITEM_HOLY                    64
#define ITEM_NOPURGE                128          /* Can't be purged */
#define ITEM_HIDDEN                 256          /* Item has been hidden */
#define ITEM_ANTI_NECROMANCER       512
#define ITEM_ANTI_DEMONOLOGIST     1024
#define ITEM_ANTI_SHAMAN           2048
#define ITEM_ANTI_DARKPRIEST       4096
#define ITEM_SPARKING              8192		/* next must exceed this */
#define ITEM_DISPEL               16384		/* next must exceed this */
#define ITEM_TWO_HANDED			  32768
#define ITEM_CLAN				  65536
#define ITEM_NO_STEAL			 131072
#define ITEM_NO_SAC				 262144
#define ITEM_REBOOT_ONLY		 524288
#define ITEM_PERMANENT			1048576
#define ITEM_LEGEND 			2097152
#define ITEM_QUEST				4194304
#define ITEM_MAGIC_SHOT			8388608
#define ITEM_CRAFTED			16777216

#define ITEM_PRO_MAGE				  1
#define ITEM_PRO_CLERIC				  2
#define ITEM_PRO_THIEF				  4
#define ITEM_PRO_WARRIOR              8
#define ITEM_PRO_PSI                 16
#define ITEM_PRO_DRUID               32
#define ITEM_PRO_RANGER              64
#define ITEM_PRO_PALADIN            128
#define ITEM_PRO_BARD               256
#define ITEM_PRO_VAMP               512
#define ITEM_PRO_WEREWOLF          1024
#define ITEM_PRO_ANTIPAL           2048
#define ITEM_PRO_ASSASSIN          4096
#define ITEM_PRO_MONK              8192
#define ITEM_PRO_BARBARIAN        16384
#define ITEM_PRO_ILLUSIONIST      32768
#define ITEM_PRO_NECROMANCER      65536
#define ITEM_PRO_DEMONOLOGIST    131072
#define ITEM_PRO_SHAMAN          262144
#define ITEM_PRO_DARKPRIEST      524288
#define ITEM_PRO_HUMAN          1048576
#define ITEM_PRO_ELF            2097152
#define ITEM_PRO_HALFELF        4194304
#define ITEM_PRO_ORC            8388608
#define ITEM_PRO_DROW          16777216
#define ITEM_PRO_DWARF         33554432
#define ITEM_PRO_HALFDWARF     67108864
#define ITEM_PRO_HOBBIT       134217728
#define ITEM_PRO_GIANT        268435456
#define ITEM_PRO_OGRE         536870912

#define ITEM_PRO_ANGEL                1
#define ITEM_PRO_MINOTAUR             2
#define ITEM_PRO_FELINE               4
#define ITEM_PRO_GARGOYLE             8
#define ITEM_PRO_CANINE              16
#define ITEM_PRO_DEMON               32
#define ITEM_PRO_PIXIE               64
#define ITEM_PRO_ELDER              128
#define ITEM_PRO_LIZARDMAN          256
#define ITEM_PRO_GNOME              512
#define ITEM_ANTI_HUMAN            1024
#define ITEM_ANTI_ELF              2048
#define ITEM_ANTI_HALFELF          4096
#define ITEM_ANTI_ORC              8192
#define ITEM_ANTI_DROW            16384
#define ITEM_ANTI_DWARF           32768
#define ITEM_ANTI_HALFDWARF       65536
#define ITEM_ANTI_HOBBIT         131072
#define ITEM_ANTI_GIANT          262144
#define ITEM_ANTI_OGRE           524288
#define ITEM_ANTI_ANGEL         1048576
#define ITEM_ANTI_MINOTAUR      2097152
#define ITEM_ANTI_FELINE        4194304
#define ITEM_ANTI_GARGOYLE      8388608
#define ITEM_ANTI_CANINE       16777216
#define ITEM_ANTI_DEMON        33554432
#define ITEM_ANTI_PIXIE        67108864
#define ITEM_ANTI_ELDER       134217728
#define ITEM_ANTI_LIZARDMAN   268435456
#define ITEM_ANTI_GNOME       536870912

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		      1
#define ITEM_WEAR_FINGER	      2
#define ITEM_WEAR_NECK		      4
#define ITEM_WEAR_BODY		      8
#define ITEM_WEAR_HEAD		     16
#define ITEM_WEAR_LEGS		     32
#define ITEM_WEAR_FEET		     64
#define ITEM_WEAR_HANDS		    128
#define ITEM_WEAR_ARMS		    256
#define ITEM_WEAR_SHIELD	    512
#define ITEM_WEAR_ABOUT		   1024
#define ITEM_WEAR_WAIST		   2048
#define ITEM_WEAR_WRIST		   4096
#define ITEM_WIELD		   8192
#define ITEM_HOLD		  16384
#define ITEM_WEAR_ORBIT           32768
#define ITEM_WEAR_FACE            65536
#define ITEM_WEAR_CONTACT        131072
#define ITEM_PROTOTYPE           262144
#define ITEM_WEAR_EARS           524288
#define ITEM_WEAR_ANKLE         1048576
#define ITEM_WEAR_FIREARM       2097152
#define ITEM_WEAR_IMPLANTED     4194304


/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE		      0
#define APPLY_STR		      1
#define APPLY_DEX		      2
#define APPLY_INT		      3
#define APPLY_WIS		      4
#define APPLY_CON		      5
#define APPLY_SEX		      6
#define APPLY_CLASS		      7
#define APPLY_LEVEL		      8
#define APPLY_AGE		      9
#define APPLY_HEIGHT		     10
#define APPLY_WEIGHT		     11
#define APPLY_MANA		     12
#define APPLY_HIT		     13
#define APPLY_MOVE		     14
#define APPLY_GOLD		     15
#define APPLY_EXP		     16
#define APPLY_AC		     17
#define APPLY_HITROLL		     18
#define APPLY_DAMROLL		     19
#define APPLY_SAVING_PARA	     20
#define APPLY_SAVING_ROD	     21
#define APPLY_SAVING_PETRI	     22
#define APPLY_SAVING_BREATH	     23
#define APPLY_SAVING_SPELL	     24
#define APPLY_BP                     25
#define APPLY_ANTI_DIS               26
#define APPLY_RACE                   27

/* Manaux */
#define APPLY_DAM_NULL			28
#define APPLY_DAM_ACID			29
#define APPLY_DAM_HOLY			30
#define APPLY_DAM_MAGIC			31
#define APPLY_DAM_FIRE			32
#define APPLY_DAM_ENERGY		33
#define APPLY_DAM_WIND			34
#define APPLY_DAM_ILLUSION		35
#define APPLY_DAM_DISPEL		36
#define APPLY_DAM_EARTH			37
#define APPLY_DAM_PSYCHIC		38
#define APPLY_DAM_POISON		39
#define APPLY_DAM_BREATH		40
#define APPLY_DAM_SUMMON		41
#define APPLY_DAM_PHYSICAL		42
#define APPLY_DAM_EXPLOSIVE		43
#define APPLY_DAM_SONG			44
#define APPLY_DAM_NAGAROM		45
#define APPLY_DAM_UNHOLY		46
#define APPLY_DAM_CLAN			47
#define APPLY_DAM_WATER			48

/* X */
#define PERM_SPELL_BEGIN	100
#define APPLY_INVISIBLE		(PERM_SPELL_BEGIN + 0)
#define APPLY_BLIND		(PERM_SPELL_BEGIN + 1)
#define APPLY_DETECT_EVIL	(PERM_SPELL_BEGIN + 2)
#define APPLY_DETECT_INVIS	(PERM_SPELL_BEGIN + 3)
#define APPLY_DETECT_MAGIC	(PERM_SPELL_BEGIN + 4)
#define APPLY_DETECT_HIDDEN	(PERM_SPELL_BEGIN + 5)
#define APPLY_HASTE		(PERM_SPELL_BEGIN + 6)
#define APPLY_SANCTUARY		(PERM_SPELL_BEGIN + 7)
#define APPLY_FAERIE_FIRE	(PERM_SPELL_BEGIN + 8)
#define APPLY_INFRARED		(PERM_SPELL_BEGIN + 9)
#define APPLY_CURSE		(PERM_SPELL_BEGIN + 10)
#define APPLY_FLAMING		(PERM_SPELL_BEGIN + 11)
#define APPLY_POISON		(PERM_SPELL_BEGIN + 12)
#define APPLY_PROTECT		(PERM_SPELL_BEGIN + 13)
#define APPLY_INERTIAL		(PERM_SPELL_BEGIN + 14)
#define APPLY_SNEAK		(PERM_SPELL_BEGIN + 15)
#define APPLY_HIDE		(PERM_SPELL_BEGIN + 16)
#define APPLY_SLEEP		(PERM_SPELL_BEGIN + 17)
#define APPLY_CHARM		(PERM_SPELL_BEGIN + 18)
#define APPLY_FLYING		(PERM_SPELL_BEGIN + 19)
#define APPLY_PASS_DOOR		(PERM_SPELL_BEGIN + 20)
#define APPLY_STUN		(PERM_SPELL_BEGIN + 21)
#define APPLY_SUMMONED		(PERM_SPELL_BEGIN + 22)
#define APPLY_MUTE		(PERM_SPELL_BEGIN + 23)
#define APPLY_PEACE		(PERM_SPELL_BEGIN + 24)
#define APPLY_FIRESHIELD        (PERM_SPELL_BEGIN + 25)
#define APPLY_SHOCKSHIELD       (PERM_SPELL_BEGIN + 26)
#define APPLY_ICESHIELD         (PERM_SPELL_BEGIN + 27)
#define APPLY_CHAOS             (PERM_SPELL_BEGIN + 28)
#define APPLY_SCRY              (PERM_SPELL_BEGIN + 29)

#define APPLY_ANTI_FLEE		(PERM_SPELL_BEGIN + 30)
#define APPLY_POLYMORPH		(PERM_SPELL_BEGIN + 31)
#define APPLY_NOASTRAL		(PERM_SPELL_BEGIN + 32)
#define APPLY_DOOMSHIELD	(PERM_SPELL_BEGIN + 33)
#define APPLY_TRUESIGHT		(PERM_SPELL_BEGIN + 34)
#define APPLY_BLADE		(PERM_SPELL_BEGIN + 35)
#define APPLY_DETECT_GOOD	(PERM_SPELL_BEGIN + 36)
#define APPLY_PROTECTION_GOOD	(PERM_SPELL_BEGIN + 37)
#define APPLY_BERSERK		(PERM_SPELL_BEGIN + 38)
#define APPLY_UNHOLY_STRENGTH	(PERM_SPELL_BEGIN + 39)
#define APPLY_CONFUSED		(PERM_SPELL_BEGIN + 40)
#define APPLY_FUMBLE		(PERM_SPELL_BEGIN + 41)
#define APPLY_DANCING		(PERM_SPELL_BEGIN + 42)
#define APPLY_IMAGE		(PERM_SEPLL_BEGIN + 43)
#define APPLY_PHASED		(PERM_SPELL_BEGIN + 44)
#define APPLY_GOLDEN_ARMOR	(PERM_SPELL_BEGIN + 45)
#define APPLY_GHOST_SHIELD	(PERM_SPELL_BEGIN + 46)
#define APPLY_CURSE_NATURE	(PERM_SPELL_BEGIN + 47)
#define APPLY_MIST	        (PERM_SPELL_BEGIN + 48)
#define APPLY_SHADOW_IMAGE  	(PERM_SPELL_BEGIN + 49)
#define APPLY_WEAPONMASTER	(PERM_SPELL_BEGIN + 50)
#define APPLY_IMPROVED_INVIS  	(PERM_SPELL_BEGIN + 51)
#define APPLY_BLINDFOLD		(PERM_SPELL_BEGIN + 52)
#define APPLY_SLIT		(PERM_SPELL_BEGIN + 53)
#define APPLY_THICK_SKIN	(PERM_SPELL_BEGIN + 54)
#define APPLY_MALIGNIFY		(PERM_SPELL_BEGIN + 55)
#define APPLY_CLOAKING		(PERM_SPELL_BEGIN + 56)
#define APPLY_SHADOW_PLANE	(PERM_SPELL_BEGIN + 57)
#define APPLY_WOLFED		(PERM_SPELL_BEGIN + 58)
#define APPLY_HOLD		(PERM_SPELL_BEGIN + 59)
#define APPLY_CHANGE_SEX	(PERM_SPELL_BEGIN + 60)

#define APPLY_WATERWALK		(PERM_SPELL_BEGIN + 61)
#define APPLY_GILLS		(PERM_SPELL_BEGIN + 62)
#define APPLY_VAM_BITE		(PERM_SPELL_BEGIN + 63)
#define APPLY_GHOUL		(PERM_SPELL_BEGIN + 64)
#define APPLY_CHALLENGER	(PERM_SPELL_BEGIN + 65)
#define APPLY_CHALLENGED	(PERM_SELLL_BEGIN + 66)
#define APPLY_RAGE		(PERM_SPELL_BEGIN + 67)
#define APPLY_BLOODTHIRSTY	(PERM_SPELL_BEGIN + 68)
#define APPLY_COFIRE            (PERM_SPELL_BEGIN + 69)
#define APPLY_TORTURE		(PERM_SPELL_BEGIN + 70)
#define APPLY_AGE_SPELL		(PERM_SPELL_BEGIN + 71)
#define APPLY_SATANIC_INFERNO   (PERM_SPELL_BEGIN + 73)
#define APPLY_PESTILENCE	(PERM_SPELL_BEGIN + 74)
#define APPLY_AURA_ANTI_MAGIC	(PERM_SPELL_BEGIN + 75)
#define APPLY_HOLY_PROTECTION	(PERM_SPELL_BEGIN + 76)
#define APPLY_IMPROVED_HIDE	(PERM_SPELL_BEGIN + 77)
#define APPLY_STEALTH		(PERM_SPELL_BEGIN + 78)
#define APPLY_BLOODSHIELD	(PERM_SPELL_BEGIN + 79)
#define APPLY_BEND_LIGHT	(PERM_SPELL_BEGIN + 80)
#define APPLY_CLOUD_OF_HEALING	(PERM_SPELL_BEGIN + 81)
#define APPLY_TALE_OF_TERROR	(PERM_SPELL_BEGIN + 82)
#define APPLY_POWER_LEAK	(PERM_SPELL_BEGIN + 83)
#define APPLY_MANA_SHIELD	(PERM_SPELL_BEGIN + 84)
#define APPLY_NAGAROMS_CURSE	(PERM_SPELL_BEGIN + 85)
#define APPLY_PRAYER		(PERM_SPELL_BEGIN + 86)
#define APPLY_RANDOMSHIELD	(PERM_SPELL_BEGIN + 87)
#define APPLY_GOLDEN_SANCTUARY  (PERM_SPELL_BEGIN + 88)
#define APPLY_BIOFEEDBACK	(PERM_SPELL_BEGIN + 89)
#define APPLY_EARTHSHIELD	(PERM_SPELL_BEGIN + 90)
#define APPLY_LEAF_SHIELD	(PERM_SPELL_BEGIN + 91)
#define APPLY_LUCK_SHIELD	(PERM_SPELL_BEGIN + 92)
#define APPLY_TONGUES		(PERM_SPELL_BEGIN + 93)
#define APPLY_LIQUID_SKIN	(PERM_SPELL_BEGIN + 94)
#define APPLY_ANGELIC_AURA	(PERM_SPELL_BEGIN + 95)
#define APPLY_ETHEREAL_WOLF	(PERM_SPELL_BEGIN + 96)
#define APPLY_ETHEREAL_SNAKE	(PERM_SPELL_BEGIN + 97)
#define APPLY_FORCE_OF_NATURE	(PERM_SPELL_BEGIN + 98)
#define APPLY_FORESTWALK	(PERM_SPELL_BEGIN + 99)
#define APPLY_MOUNTAINWALK	(PERM_SPELL_BEGIN + 100)
#define APPLY_PLAINSWALK	(PERM_SPELL_BEGIN + 101)
#define APPLY_SWAMPWALK		(PERM_SPELL_BEGIN + 102)
#define APPLY_ESSENCE_OF_GAIA	(PERM_SPELL_BEGIN + 103)

#define APPLY_BLESS             (PERM_SPELL_BEGIN + 201)
#define APPLY_GIANT_STRENGTH    (PERM_SPELL_BEGIN + 202)
#define APPLY_COMBAT_MIND       (PERM_SPELL_BEGIN + 203)
#define APPLY_HEIGHTEN_SENSES   (PERM_SPELL_BEGIN + 204)
#define APPLY_QUICKNESS		(PERM_SPELL_BEGIN + 205)
#define APPLY_OCCULUTUS_VISUM   (PERM_SPELL_BEGIN + 206)
#define APPLY_TITAN_STRENGTH	(PERM_SPELL_BEGIN + 207)

#define PERM_SPELL_END			(PERM_SPELL_BEGIN + 207 )
/* END */

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8



/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_CHAT		   3054
#define ROOM_VNUM_TEMPLE	   25000
#define ROOM_VNUM_ALTAR		   25000
#define ROOM_VNUM_ALOSER	   25000
#define ROOM_VNUM_AWINNER	   25000
#define ROOM_VNUM_SCHOOL	   101
#define ROOM_VNUM_GRAVEYARD_A      25000
#define ROOM_VNUM_PURGATORY_A      25000
#define ROOM_VNUM_OUTSIDEMETH	   17809

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		      1
#define ROOM_NO_SHADOW		      2
#define ROOM_NO_MOB		      4
#define ROOM_INDOORS		      8
#define ROOM_TANNERY	       	     16
#define ROOM_ALCHEMIST	       	     32
#define ROOM_LIBRARY	       	     64
#define ROOM_FORGE	       	    128
#define ROOM_BOWYER	       	    256
#define ROOM_PRIVATE		    512
#define ROOM_SAFE		   1024
#define ROOM_SOLITARY		   2048
#define ROOM_PET_SHOP		   4096
#define ROOM_NO_RECALL		   8192
#define ROOM_CONE_OF_SILENCE      16384 /* delete */
#define ROOM_NO_MAGIC             32768
#define ROOM_NO_PKILL             65536
#define ROOM_NO_ASTRAL_IN        131072
#define ROOM_NO_ASTRAL_OUT       262144
#define ROOM_TELEPORT_AREA       524288
#define ROOM_TELEPORT_WORLD     1048576
#define ROOM_NO_OFFENSIVE       2097152
#define ROOM_NO_FLEE            4194304
#define ROOM_SILENT             8388608
#define ROOM_BANK		16777216
#define ROOM_NOFLOOR            33554432
#define ROOM_SMITHY             67108864
#define ROOM_NOSCRY		134217728
#define ROOM_DAMAGE             268435456
#define ROOM_PKILL              536870912
#define ROOM_MARK              1073741824

#define ROOM_TIMED_DEFORESTED	      1
#define ROOM_TIMED_MINED	      2
#define ROOM_TIMED_CAMP		      4


/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5



/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		      1
#define EX_CLOSED		      2
#define EX_LOCKED		      4
#define EX_BASHED                     8
#define EX_BASHPROOF                 16
#define EX_PICKPROOF		     32
#define EX_PASSPROOF                 64
#define EX_RANDOM                   128
#define EX_MAGICLOCK		    256
#define EX_HIDDEN		    512

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		      0
#define SECT_CITY		      1
#define SECT_FIELD		      2
#define SECT_FOREST		      3
#define SECT_HILLS		      4
#define SECT_MOUNTAIN		      5
#define SECT_WATER_SWIM		      6
#define SECT_WATER_NOSWIM	      7
#define SECT_UNDERWATER  	      8
#define SECT_AIR		      9
#define SECT_DESERT		     10
#define SECT_BADLAND                 11
#define SECT_MAX		     12



/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		     -1
#define WEAR_LIGHT		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK_1		      3
#define WEAR_NECK_2		      4
#define WEAR_BODY		      5
#define WEAR_HEAD		      6
#define WEAR_IN_EYES		      7
#define WEAR_ON_FACE		      8
#define WEAR_ORBIT                    9
#define WEAR_ORBIT_2                 10
#define WEAR_LEGS		     11
#define WEAR_FEET		     12
#define WEAR_HANDS		     13
#define WEAR_ARMS		     14
#define WEAR_SHIELD		     15
#define WEAR_ABOUT		     16
#define WEAR_WAIST		     17
#define WEAR_WRIST_L		     18
#define WEAR_WRIST_R		     19
#define WEAR_WIELD		     20
#define WEAR_WIELD_2		     21
#define WEAR_HOLD		     22
#define WEAR_EARS                    24
#define WEAR_ANKLE_L                 25
#define WEAR_ANKLE_R                 26
#define WEAR_FIREARM		     23
#define WEAR_IMPLANTED1		     27
#define WEAR_IMPLANTED2		     28
#define WEAR_IMPLANTED3		     29
#define MAX_WEAR		     30



/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK		      0
#define COND_FULL		      1
#define COND_THIRST		      2



/*
 * Positions.
 */
#define POS_DEAD		      0
#define POS_MORTAL		      1
#define POS_INCAP		      2
#define POS_STUNNED		      3
#define POS_SLEEPING		      4
#define POS_RESTING		      5
#define POS_GHOST		      6
#define POS_FIGHTING		      7
#define POS_STANDING		      8
#define POS_MEDITATING               10


/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		      1		/* Don't EVER set.	*/
#define PLR_BOUGHT_PET		      2
#define PLR_AFK			      4
#define PLR_AUTOEXIT		      8
#define PLR_AUTOLOOT		     16
#define PLR_AUTOSAC                  32
#define PLR_BLANK		     64
#define PLR_BRIEF		    128
#define PLR_MUSIC		    256
#define PLR_COMBINE		    512
#define PLR_PROMPT		   1024
#define PLR_TELNET_GA		   2048
#define PLR_HOLYLIGHT		   4096
#define PLR_WIZINVIS		   8192
#define PLR_QUESTOR		  16384
#define	PLR_SILENCE		  32768
#define PLR_NO_EMOTE		  65536
#define PLR_GHOST		 131072
#define PLR_NO_TELL		 262144
#define PLR_LOG			 524288
#define PLR_DENY		1048576
#define PLR_FREEZE		2097152
#define PLR_THIEF		4194304
#define PLR_KILLER		8388608
#define PLR_ANSI               16777216
#define PLR_AUTOGOLD           33554432
#define PLR_KEEPTITLE          67108864
#define PLR_UNDEAD            134217728
#define PLR_SOUND             268435456
#define PLR_COMBAT            536870912
#define PLR_CSKILL           1073741824

#define PLR_RELQUEST		      1

/*
 * Obsolete bits.
 */
#if 0
#define PLR_AUCTION		      4	/* Obsolete	*/
#endif


#define STUN_TOTAL            0   /* Commands and combat halted. Normal stun */
#define STUN_COMMAND          1   /* Commands halted. Combat goes through */
#define STUN_MAGIC            2   /* Can't cast spells */
#define STUN_NON_MAGIC        3   /* No weapon attacks */
#define STUN_TO_STUN          4   /* Requested. Stop continuous stunning */


/*
 * Channel bits.  Had to skip 1, 2 and 4 for some reason.
 */
#define CHANNEL_NONE                  0
#define	CHANNEL_AUCTION		8388608
#define	CHANNEL_CHAT	       16777216
#define	CHANNEL_IMMTALK	              8
#define	CHANNEL_MUSIC		     16
#define	CHANNEL_QUESTION	     32
#define	CHANNEL_SHOUT		     64
#define	CHANNEL_YELL		    128
#define CHANNEL_CLAN                256
#define CHANNEL_CLASS               512
#define CHANNEL_CHAMPION           1024
#define CHANNEL_DEMIGOD	 	 262144
#define CHANNEL_MUTTER		 524289
#define CHANNEL_TIMELORD       1048576
#define CHANNEL_GRATZ		2097152
#define CHANNEL_OOC             4194304
#define CHANNEL_LOG		   2048
#define CHANNEL_BUILD		   4096
#define CHANNEL_GOD                8192
#define CHANNEL_GUARDIAN          16384
#define CHANNEL_GUILD		  32768
#define CHANNEL_CODER             65536
#define CHANNEL_INFO		 131072
#define CHANNEL_COMLOG         33554432
/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
    MOB_INDEX_DATA *	next;
    SPEC_FUN *		spec_fun;
    GAME_FUN *          game_fun;
    SHOP_DATA *		pShop;
    AREA_DATA *		area;			/* OLC */
    MPROG_DATA *        mobprogs;
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    int			shields;
    int 		vnum;
    int			progtypes;
    int 		count;
    int 		killed;
    int 		sex;
    int			class;
    int 		level;
    int			act;
    int			act2;
    int			size;
    long		affected_by;
    long		affected_by2;
    long		affected_by3;
    long		affected_by4;
    long		affected_by_powers;
    long		affected_by_weaknesses;
    long		imm_flags;	/* XOR */
    long		res_flags;
    long		vul_flags;	/* XOR */
    int 		alignment;
    int 		hitroll;
    int			damroll;
    int 		ac;			/* Unused */
    int 		hitnodice;		/* Unused */
    int 		hitsizedice;		/* Unused */
    int 		hitplus;		/* Unused */
    int 		damnodice;		/* Unused */
    int 		damsizedice;		/* Unused */
    int 		damplus;		/* Unused */
    int			gold;			/* Unused */
    int			speaking;		/* Tyrion */
/*    int			race; */
};

/*
 * -- Altrag
 */
struct  gskill_data
{
    int                 sn;
    void *              victim;
    int                 level;
    int                 timer;
};

/*
 * One character (PC or NPC).
 */
struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *		master;
    CHAR_DATA *		leader;
    CHAR_DATA *		fighting;
    CHAR_DATA *         hunting;
    CHAR_DATA *		reply;
    CHAR_DATA *         questgiver;             /* Vassago, quest */
    CHAR_DATA *         gladiator;          	/* ARENA player wagered on */
    SPEC_FUN *		spec_fun;
    GAME_FUN *          game_fun;               /* Maniac, dice games */
    MOB_INDEX_DATA *	pIndexData;
    DESCRIPTOR_DATA *	desc;
    AFFECT_DATA *	affected;
    AFFECT_DATA *	affected2;
    AFFECT_DATA *	affected3;
    AFFECT_DATA *	affected4;
    AFFECT_DATA *	affected_powers;	/* Religion powers, Tyrion */
    AFFECT_DATA *	affected_weaknesses;	/* Religion weaknesses, Tyrion */
    NOTE_DATA *		pnote;
    OBJ_DATA *		carrying;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	was_in_room;
    PC_DATA *		pcdata;
    MPROG_ACT_LIST *    mpact;
    GSPELL_DATA *       gspell;
    PHOBIA_DATA *       phobia;
    char *		name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    char *              prompt;
    char *		nullstring;
    int			updated;
    int 		sex;
    int 		class;
    int 		multied;
    int                 mpactnum;
    int 		race;
    int                 clan;
    int                 religion;
    int                 clev;
    int                 ctimer;
    int                 wizinvis;
    int 		level;
    int                 antidisarm;
    int  		trust;
    bool                wizbit;
    int			played;
    time_t		logon;
    time_t		save_time;
    time_t              last_note;
    int 		timer;
    int 		wait;
    int 		hit;
    int 		max_hit;
    int 		mana;
    int 		max_mana;
    int 		move;
    int 		max_move;
    int                 bp;
    int                 max_bp;
    int 		gold;
    int 		exp;
    int 		shields;
    int                 questpoints;            /* Vassago */
    int                 nextquest;              /* Vassago */
    int                 countdown;              /* Vassago */
    int                 questobj;               /* Vassago */
    int                 questmob;               /* Vassago */
    int			rquestpoints;		/* Tyrion */
    int			rnextquest;		/* Tyrion */
    int			rcountdown;		/* Tyrion */
    int			rquestobj[MAX_LEVEL];	/* Tyrion/Ahsile */
    int			rquestmob[MAX_LEVEL];	/* Tyrion/Ahsile */
    int			relquest_level;		/* ahsile */
    int			act;
    int		        act2;			/* ahsile */
    long		affected_by;
    long		affected_by2;
    long		affected_by3;
    long		affected_by4;
    long		affected_by_powers;
    long		affected_by_weaknesses;
    long		imm_flags;
    long		res_flags;
    long		vul_flags;
    int 		position;
    int 		practice;
    int 		carry_weight;
    int 		carry_number;
    int 		saving_throw;
    int 		alignment;
    int 		hitroll;
    int 		damroll;
    int 		armor;
    int 		wimpy;
    int 		deaf;
    int                 language        [ MAX_LANGUAGE ];
    int                 speaking;                           /* Maniac */
    int			size;
    bool                deleted;
    int			combat_timer;	/* XOR */
    int			summon_timer;	/* XOR */
    time_t              pkill_timer;
    bool		pkill;
    bool		initiated;
    int			guild_rank;
    const GUILD_DATA *	guild;
    int			rtimer;			/* All Religion data by Tyrion */
    int                 stunned[STUN_MAX];
    int			damage_mods[DAMCLASS_MAX]; /* Manaux */
    int			mounted;
    int			mountcharmed;
    char * 		mountshort;
    bool		fixed_error;		/* Ahsile */
    int                 poison_level;		/* Tyrion */
#ifdef SQL_SYSTEM
	int			CharID;					/* Ahsile */
	bool		playing;
	bool		new;
	CHAR_DATA*  next_dirty;
	ROOM_INDEX_DATA* last_room;
#endif
};


struct guild_data
{
  char *	name;
  char *	deity;
  int		color;
  int		type;
};

struct  mob_prog_act_list
{
    MPROG_ACT_LIST * next;
    char *           buf;
    CHAR_DATA *      ch;
    OBJ_DATA *       obj;
    void *           vo;
};

struct  mob_prog_data
{
    MPROG_DATA *next;
    int         type;
    char *      arglist;
    char *      comlist;
	int			status; /* states this prog is active in, 0 => all states*/
};

/*
 * Data which only PC's have.
 */
struct	pc_data
{
    PC_DATA *		next;
    ALIAS_DATA *        alias_list;
    char *		pwd;
    char *		bamfin;
    char *		bamfout;
    char *		bamfsin;
    char *		bamfsout;
    char *		immskll;
    char *		title;
    char *              prompt;
    char *              lname;
//    char *		religion;	/* The long version of the name */
//    char *		religion_name;	/* The 3 character short */
    int 		recall;		/* Recall room */
    char *              who_text;       /* Text in who listing - Canth */
    char *		spouse;		/* Mythran, modified by Tyrion */
    int 		perm_str;
    int 		perm_int;
    int 		perm_wis;
    int 		perm_dex;
    int 		perm_con;
    int 		mod_str;
    int 		mod_int;
    int 		mod_wis;
    int 		mod_dex;
    int 		mod_con;
    int 		condition	[ 3 ];
    int                 pagelen;
    int 		learned		[ MAX_SKILL ];
    bool                switched;
    int 		security;	/* OLC - Builder security */
    int                 bankaccount;
    OBJ_DATA          * storage;
    int                 storcount;
    int                 learn;                                  /* Maniac */
    int                 shares;                                 /* Maniac */
    int                 plr_wager;  /* ARENA amount wagered */
    int                 awins;      /* ARENA number of wins */
    int                 alosses;    /* ARENA number of losses */
    int			mobkills;
    bool                confirm_delete;                         /* Maniac */
    int		        corpses;
    bool		clean;  /*Player is clean. -- Manaux */
    char * 		plan;
    char * 		email;

    int			spell1;   		/* spells for scribing/brewing/forging */
    int			spell2;
    int			spell3;
    int			craft_timer; 		/* time to wait while crafting */
    int 		craft_type;		/* scroll, potion, etc */
    OBJ_DATA*		craft_target;		/* the target of the craft */

};

struct  alias_data
{
    ALIAS_DATA *next;
    char       *old;
    char       *_new;
};

#define PHOBIA_FIRE      0
#define PHOBIA_LIGHTNING 1
#define PHOBIA_WATER     2
#define PHOBIA_MAGIC     3
#define PHOBIA_NONMAGIC  4
#define PHOBIA_PSI       5
#define PHOBIA_DRAGON    6
#define PHOBIA_MAX       7

struct  phobia_data
{
    PHOBIA_DATA *next;
    int          type;
    int          duration;
    int          strength;
};

/*
 * heh.. were discussing obj/room progs.. and then these triggers started
 * looking a helluva lot like em.. :).. so what the hell..? :).. main
 * difference between this struct and the mobprog stuff is that this is
 * implemented as traps.  ie.. the disarmable stuff..
 * also, these are a little more global.. :)
 * -- Altrag
 */

/*
 * The object triggers.. quite a few of em.. :)
 */
#define OBJ_TRAP_ERROR           0  /* error! */
#define OBJ_TRAP_GET             A  /* obj is picked up */
#define OBJ_TRAP_DROP            B  /* obj is dropped */
#define OBJ_TRAP_PUT             C  /* obj is put into something */
#define OBJ_TRAP_WEAR            D  /* obj is worn */
#define OBJ_TRAP_LOOK            E  /* obj is looked at/examined */
#define OBJ_TRAP_LOOK_IN         F  /* obj is looked inside (containers) */
#define OBJ_TRAP_INVOKE          G  /* obj is invoked */
#define OBJ_TRAP_USE             H  /* obj is used (recited, zapped, ect) */
#define OBJ_TRAP_CAST            I  /* spell is cast on obj - percent */
#define OBJ_TRAP_CAST_SN         J  /* spell is cast on obj - by slot */
#define OBJ_TRAP_JOIN            K  /* obj is joined with another */
#define OBJ_TRAP_SEPARATE        L  /* obj is separated into two */
#define OBJ_TRAP_BUY             M  /* obj is bought from store */
#define OBJ_TRAP_SELL            N  /* obj is sold to store */
#define OBJ_TRAP_STORE           O  /* obj is stored in storage boxes */
#define OBJ_TRAP_RETRIEVE        P  /* obj is retrieved from storage */
#define OBJ_TRAP_OPEN            Q  /* obj is opened (containers) */
#define OBJ_TRAP_CLOSE           R  /* obj is closed (containers) */
#define OBJ_TRAP_LOCK            S  /* obj is locked (containers) */
#define OBJ_TRAP_UNLOCK          T  /* obj is unlocked (containers) */
#define OBJ_TRAP_PICK            U  /* obj is picked (containers) */
#define OBJ_TRAP_RANDOM          V  /* random trigger */
#define OBJ_TRAP_THROW           W  /* obj is thrown */
#define OBJ_TRAP_GET_FROM        X  /* to allow secondary obj's in get */
#define OBJ_TRAP_GIVE            Y  /* give an obj away */
#define OBJ_TRAP_FILL            Z  /* obj is filled (drink_cons) */

/*
 * Note that entry/exit/pass are only called if the equivilant exit
 * trap for the exit the person went through failed.
 * Pass is only called if the respective enter or exit trap failed.
 */
#define ROOM_TRAP_ERROR          0  /* error! */
#define ROOM_TRAP_ENTER          A  /* someone enters the room */
#define ROOM_TRAP_EXIT           B  /* someone leaves the room */
#define ROOM_TRAP_PASS           C  /* someone enters or leaves */
#define ROOM_TRAP_CAST           D  /* a spell was cast in room - percent */
#define ROOM_TRAP_CAST_SN        E  /* a spell was cast in room - by slot */
#define ROOM_TRAP_SLEEP          F  /* someone sleeps in the room */
#define ROOM_TRAP_WAKE           G  /* someone wakes up in the room */
#define ROOM_TRAP_REST           H  /* someone rests in the room */
#define ROOM_TRAP_DEATH          I  /* someone dies in the room */
#define ROOM_TRAP_TIME           J  /* depends on the time of day */
#define ROOM_TRAP_RANDOM         K  /* random trigger */

/*
 * enter/exit/pass rules are the same as those for room traps.
 * note that look trap is only called if scry trap fails.
 */
#define EXIT_TRAP_ERROR          0  /* error! */
#define EXIT_TRAP_ENTER          A  /* someone enters through the exit */
#define EXIT_TRAP_EXIT           B  /* someone leaves through the exit */
#define EXIT_TRAP_PASS           C  /* someone enters/leaves through exit */
#define EXIT_TRAP_LOOK           D  /* someone looks through exit */
#define EXIT_TRAP_SCRY           E  /* someone scrys through the exit */
#define EXIT_TRAP_OPEN           F  /* someone opens the exit (door) */
#define EXIT_TRAP_CLOSE          G  /* someone closes the exit (door) */
#define EXIT_TRAP_LOCK           H  /* someone locks the exit (door) */
#define EXIT_TRAP_UNLOCK         I  /* someone unlocks the exit (door) */
#define EXIT_TRAP_PICK           J  /* someone picks the exit (locked door) */

struct trap_data
{
    TRAP_DATA *next;
    TRAP_DATA *next_here;
    OBJ_INDEX_DATA *on_obj;
    ROOM_INDEX_DATA *in_room;
    EXIT_DATA *on_exit;
    int type;
    char *arglist;
    char *comlist;
    bool disarmable;
    bool disarmed;
    int disarm_dur;
};


/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		16

struct	liq_type
{
    char               *liq_name;
    char               *liq_color;
    int                 liq_affect [ 3 ];
};



/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	   /* Next in list                     */
    char             *keyword;     /* Keyword in look/examine          */
    char             *description; /* What to see                      */
    bool              deleted;
};



/*
 * Prototype for an object.
 */
struct	obj_index_data
{
    OBJ_INDEX_DATA *	next;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    AREA_DATA *		area;			/* OLC */
    TRAP_DATA *         traps;
    int                 traptypes;
    char *		name;
    char *		short_descr;
    char *		description;
    int 		vnum;
    int 		item_type;
    int			durability_max;
    int			durability_cur;
    int 		extra_flags;
    int 		extra_flags2;
    int			extra_flags3;
    int			extra_flags4;
    int 		wear_flags;
    int 		count;
    int 		weight;
    int			cost;			/* Unused */
    int                 level;
    int			value	[ 4 ];
    int                 ac_type;
    int                 ac_vnum;
    int			pad	[ 4 ];          /* Unused */
    char *              ac_spell;
    int                 ac_charge [ 2 ];
    int                 join;
    int                 sep_one;
    int                 sep_two;
};



/*
 * One object.
 */
struct	obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		next_content;
    OBJ_DATA *		contains;
    OBJ_DATA *		in_obj;
    CHAR_DATA *		carried_by;
    CHAR_DATA *         stored_by;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    char *		name;
    char *		short_descr;
    char *		description;
    int 		item_type;
    int 		extra_flags;
    int 		extra_flags2;
    int			extra_flags3;
    int			extra_flags4;
    int 		wear_flags;
    int 		wear_loc;
    int			durability_max;
    int			durability_cur;
    int 		weight;
    int			cost;
    int 		level;
    int 		timer;
    int			value	[ 4 ];
    int                 ac_type;
    int                 ac_vnum;
    char *              ac_spell;
    int                 ac_charge [ 2 ];
    bool                deleted;
};

struct sac_data
{
   char  god[ MAX_INPUT_LENGTH ];
   int   reward_total;
};

/*
 * Exit data.
 */
struct	exit_data
{
    ROOM_INDEX_DATA *	to_room;
    EXIT_DATA *		next;		/* OLC */
    TRAP_DATA *         traps;
    int                 traptypes;
    int			rs_flags;	/* OLC */
    int 		vnum;
    int 		exit_info;
    int 		key;
    char *		keyword;
    char *		description;
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
    RESET_DATA *	next;
    char		command;
    int 		arg1;
    int 		arg2;
    int 		arg3;
	int			status; /* status for Area theme quests */
};



/*
 * Area definition.
 */
struct	area_data
{
    AREA_DATA *		next;
    RESET_DATA *        reset_first;
    RESET_DATA *        reset_last;
    char *		name;
    int                 recall;
    int 		age;
    int 		nplayer;
    char *		filename;	/* OLC */
    char *		builders;	/* OLC - Listing of builders */
    int			security;	/* OLC - Value 0-infinity  */
    int			lvnum;		/* OLC - Lower vnum */
    int			uvnum;		/* OLC - Upper vnum */
    int			vnum;		/* OLC - Area vnum  */
    int			area_flags;	/* OLC */
    char *              reset_sound;    /* Altrag */
    int			windstr;
    int			winddir;
    int			llevel;		/* Tyrion */
    int			ulevel;		/* Tyrion */
    char *		actual_sound; 	/* Tyrion */
    char *		musicfile;
    int			status;    /* Status for Area theme quests */
    int 		version; /* area file version */
    char *		creator;
#ifdef SQL_SYSTEM
	int			AreaID;
#endif
};

struct  new_clan_data
{
    CLAN_DATA *         next;
    char *              name;
    char *              deity;
    char *              description;
    char *              leader;
    char *              first;
    char *              second;
    char *              champ;
    bool                isleader;
    bool                isfirst;
    bool                issecond;
    bool                ischamp;
    int                 vnum;
    int                 recall;
    int                 pkills;
    int                 mkills;
    int                 members;
    int                 pdeaths;
    int                 mdeaths;
    int                 obj_vnum_1;
    int                 obj_vnum_2;
    int                 obj_vnum_3;
    bool                pkill;
};

struct  new_religion_data
{
    RELIGION_DATA *	next;
    char * 		name;
    char * 		deity;
    char * 		description;
    char *		shortdesc;
    int			vnum;
    int			recall;
    int			start;
    int			pkills;
    int			mkills;
    int			pdeaths;
    int			mdeaths;
    int			members;
};

/*
 * Room type.
 */
struct	room_index_data
{
    ROOM_INDEX_DATA *	next;
    ROOM_INDEX_DATA *   next_timed_room;  /* next room with temporary flags */
    CHAR_DATA *		people;
    OBJ_DATA *		contents;
    EXTRA_DESCR_DATA *	extra_descr;
    AREA_DATA *		area;
    EXIT_DATA *		exit	[ 6 ];
    RESET_DATA *	reset_first;	/* OLC */
    RESET_DATA *	reset_last;	/* OLC */
    TRAP_DATA *         traps;
    int                 traptypes;
    char *		name;
    char *		description;
    char *		soundfile;
    char *		musicfile;
    int 		vnum;
    int 		room_flags;
    int			timed_room_flags;
    int			flag_timer;
    int 		light;
    int 		sector_type;
    int                 rd;    /* TRI ( Room damage ) */

};



/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED             -1
#define TYPE_HIT                 1000



/*
 *  Target types.
 */
#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4
#define TAR_GROUP_OFFENSIVE         5
#define TAR_GROUP_DEFENSIVE         6
#define TAR_GROUP_ALL               7
#define TAR_GROUP_OBJ               8
#define TAR_GROUP_IGNORE            9

/* Language type */
struct  lang_type
{
        char    *name;                  /* Language name */
};


/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
    char *	name;			   /* Name of skill		 */
    int 	skill_level [ MAX_CLASS ]; /* Level needed by class	 */
    SPELL_FUN *	spell_fun;		   /* Spell pointer (for spells) */
    int 	target;			   /* Legal targets		 */
    int 	minimum_position;	   /* Position for caster / user */
    int *	pgsn;			   /* Pointer to associated gsn	 */
    int 	min_mana;		   /* Minimum mana used		 */
    int 	beats;			   /* Waiting time after use	 */
    char *	noun_damage;		   /* Damage message		 */
    char *	msg_off;		   /* Wear off message		 */
    int         slot;                      /* For object loading         */
    int		dispel_bit;		   /* For dispelling crap	 */
    int		cancel_bit;		   /* For cancelling crap	 */
    int		damclass;		   /* For damage types.		 */
    char *	msg_off_room;		   /* Dispel message             */
    int		shield_bit;
    char 	type;			   /* skill, spell type      */
    int		str_req;
    int		wis_req;
    int		int_req;
    int		dex_req;
    int		con_req;
    int		is_spell;		   /* True if spell, False if skill */
    int		craftable;		   /* bitvector */
    char *	msg_fail;		   /* Message if skpell fails	  */
};

/*
 * -- Altrag
 */
struct gskill_type
{
    int         wait;                      /* Wait for casters in ticks  */
    int         slot;                      /* Matching skill_table sn    */
    int         casters[MAX_CLASS];        /* Casters needed by class    */
};

/*
 * Global dead object list   --Manaux
 */
extern DEAD_OBJ_LIST *dead_object_list;
extern bool   DeadObjPrntOnly;


/*
 * These are skill_lookup return values for common skills and spells.
 */
/*
 * Removed. See note in db.c - Ahsile
 */

/*
 * Race abilities gsn's.
 */

/*
 * Utility macros.
 */
#define UMIN( a, b )		( ( a ) < ( b ) ? ( a ) : ( b ) )
#define UMAX( a, b )		( ( a ) > ( b ) ? ( a ) : ( b ) )
#define URANGE( a, b, c )	( ( b ) < ( a ) ? ( a )                       \
				                : ( ( b ) > ( c ) ? ( c )     \
						                  : ( b ) ) )
#define LOWER( c )		( ( c ) >= 'A' && ( c ) <= 'Z'                \
				                ? ( c ) + 'a' - 'A' : ( c ) )
#define UPPER( c )		( ( c ) >= 'a' && ( c ) <= 'z'                \
				                ? ( c ) + 'A' - 'a' : ( c ) )
#define IS_SET( flag, bit )	( ( flag ) &   ( bit ) )
#define SET_BIT( var, bit )	( ( var )  |=  ( bit ) )
#define SET_BIT2( var, bit )	( ( var )  |=  ( bit ) )
#define REMOVE_BIT( var, bit )	( ( var )  &= ~( bit ) )
#define IS_QUESTOR(ch)          ( IS_SET((ch)->act, PLR_QUESTOR))
#define MOB_GAME_CHEAT(mob)     ( mob->pIndexData->ac )

/*
 * Character macros.
 */
#define IS_CODER( ch )          ( IS_SET( ( ch )->affected_by2, CODER ) )
#define IS_NPC( ch )		( IS_SET( ( ch )->act, ACT_IS_NPC ) )
#define IS_IMMORTAL( ch )	( get_trust( ch ) >= LEVEL_IMMORTAL )
#define IS_HERO3( ch )		( get_trust( ch ) >= LEVEL_HERO3    )
#define IS_HERO2( ch )		( get_trust( ch ) >= LEVEL_HERO2    )
#define IS_HERO( ch )		( get_trust( ch ) >= LEVEL_HERO     )
#define IS_CHAMP( ch )		( get_trust( ch ) >= LEVEL_CHAMP    )
#define IS_DEMIGOD( ch )	( get_trust( ch ) >= LEVEL_DEMIGOD  )
#define IS_LEGEND( ch )		( (IS_DEMIGOD( ch ) && ( ch->exp >= MAX_EXPERIENCE )) || (IS_IMMORTAL( ch )))
#define IS_AFFECTED( ch, sn )	( IS_SET( ( ch )->affected_by, ( sn ) ) )
#define IS_AFFECTED2(ch, sn)	( IS_SET( ( ch )->affected_by2, (sn)))
#define IS_AFFECTED3(ch, sn)	( IS_SET( ( ch )->affected_by3, (sn)))
#define IS_AFFECTED4(ch, sn)	( IS_SET( ( ch )->affected_by4, (sn)))
#define IS_AFFECTED_POWERS(ch, sn)	( IS_SET( ( ch )->affected_by_powers, (sn)))
#define IS_AFFECTED_WEAKNESSES(ch, sn)	( IS_SET( ( ch )->affected_by_weaknesses, (sn)))
#define IS_SIMM(ch, sn)		(IS_SET((ch)->imm_flags, (sn)))
#define IS_SRES(ch, sn)		(IS_SET((ch)->res_flags, (sn)))
#define IS_SVUL(ch, sn)		(IS_SET((ch)->vul_flags, (sn)))

#define IS_GOOD( ch )		( ch->alignment >=  350 )
#define IS_EVIL( ch )		( ch->alignment <= -350 )
#define IS_NEUTRAL( ch )	( !IS_GOOD( ch ) && !IS_EVIL( ch ) )

#define IS_AWAKE( ch )		( ch->position > POS_SLEEPING )
#define GET_AC( ch )		( ( ch )->armor				     \
				    + ( IS_AWAKE( ch )			     \
				    ? dex_app[get_curr_dex( ch )].defensive  \
				    : 0 ) )
#define GET_HITROLL( ch )      	( ( ch )->hitroll                            \
				 + str_app[get_curr_str( ch )].tohit )
#define GET_DAMROLL( ch )      	( ( ch )->damroll / 2                        \
				 + str_app[get_curr_str( ch )].todam )

/* damroll is cut in half to reduce damage dealt by physical attacks, cuz its getting rediculous
    All occurences of the DISPLAY of your damroll is doubled, to appear like its normal.         */

#define GET_CLASS( ch )		( ( ch )->class )

#define IS_OUTSIDE( ch )       	( !IS_SET(				     \
				    ( ch )->in_room->room_flags,       	     \
				    ROOM_INDOORS ) )

#define IS_FOREST( ch ) 	( ch->in_room->sector_type == SECT_FOREST )
#define IS_MOUNTAIN( ch ) 	( ch->in_room->sector_type == SECT_MOUNTAIN )
#define IS_IN_NATURE( ch )	( ch->in_room->sector_type == SECT_FIELD || ch->in_room->sector_type == SECT_FOREST || ch->in_room->sector_type == SECT_MOUNTAIN || ch->in_room->sector_type == SECT_HILLS || ch->in_room->sector_type == SECT_BADLAND || ch->in_room->sector_type == SECT_DESERT )

#define WAIT_STATE( ch, pulse ) ( ( ch )->wait = UMAX( ( ch )->wait,         \
						      ( pulse ) ) )


#define STUN_CHAR( ch, pulse, type ) ( (ch)->stunned[(type)] =               \
				       UMAX( (ch)->stunned[(type)],          \
					     (pulse) ) )

#define IS_STUNNED( ch, type ) ( (ch)->stunned[(type)] > 0 )

#define MANA_COST( ch, sn )     ( IS_NPC( ch ) ? 0 : UMAX (                  \
				skill_table[sn].min_mana,                    \
				100 / UMAX(1, ( 2 + ch->level -              \
				skill_table[sn].skill_level[ch->class] ) ) ) )

#define MANA_COST_MULTI( ch, sn ) ( IS_NPC(ch) ? 0 : UMAX (                  \
                                skill_table[sn].min_mana,                    \
                                100 / UMAX(1, ( 2 + ch->level -                      \
                                skill_table[sn].skill_level[ch->multied] ) ) ) )

#define IS_SWITCHED( ch )       ( ch->pcdata->switched )

/* Useful Macro I often need for debugging.  Walker */
#define CHAR_NAME( ch )         ( IS_NPC(ch)                                 \
                                        ? ( ch )->short_descr                \
                                        :       ( ch )->name )


/*
 * Object macros.
 */
#define CAN_WEAR( obj, part )	( IS_SET( ( obj)->wear_flags,  ( part ) ) )
#define IS_OBJ_STAT( obj, stat )( IS_SET( ( obj)->extra_flags, ( stat ) ) )
#define IS_OBJ_STAT2( obj, stat )( IS_SET( ( obj)->extra_flags2, ( stat ) ) )
#define IS_OBJ_STAT3( obj, stat )( IS_SET( ( obj)->extra_flags3, ( stat ) ) )
#define IS_OBJ_STAT4( obj, stat )( IS_SET( ( obj)->extra_flags4, ( stat ) ) )


/*
 * Description macros.
 */
#define PERS( ch, looker )	( can_see( looker, ( ch ) ) ?		     \
				( IS_NPC( ch ) ? ( ch )->short_descr	     \
				               : ( ch )->name ) : "someone" )



/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
    int 		position;
    int 		level;
    int 		log;
};



/*
 * Structure for a social in the socials table.
 */
struct  social_type
{
    char          * name;
    char          * char_no_arg;
    char          * others_no_arg;
    char          * char_found;
    char          * others_found;
    char          * vict_found;
    char          * char_auto;
    char          * others_auto;
    char	  * sound;
};

/*
 * Global constants.
 */
extern	const	struct	str_app_type	str_app		[ 51 ];
extern	const	struct	int_app_type	int_app		[ 51 ];
extern	const	struct	wis_app_type	wis_app		[ 51 ];
extern	const	struct	dex_app_type	dex_app		[ 51 ];
extern	const	struct	con_app_type	con_app		[ 51 ];

extern	const	struct	class_type	class_table	[ MAX_CLASS   ];
extern	const	struct	race_type	race_table	[ MAX_RACE    ];
extern  const   struct  clan_type       clan_table      [ ];
extern  const   struct  guild_data      guild_table     [ ];
extern  struct	cmd_type	cmd_table	[ ];
extern	const	struct	liq_type	liq_table	[ LIQ_MAX     ];
extern	const	struct	skill_type	skill_table	[ ];
extern  const   struct  gskill_type     gskill_table    [ MAX_GSPELL  ];
/*extern	const	struct	social_type	social_table	[ ];*/
extern	char *	const			title_table	[ MAX_CLASS   ]
							[ MAX_LEVEL+1 ]
							[ 2 ];
extern  const   struct  lang_type       lang_table      [ MAX_LANGUAGE  ];
extern          SOCIAL_DATA    *social_table;

/* For variable XP table.  Walker */
extern  const   int     xp_table        [ MAX_LEVEL + 1][ MAX_CLASS ];

extern  int exp_table[]; /* Tyrion */

/*
 * Global variables.
 */
extern		HELP_DATA	  *	help_first;
extern		HELP_DATA	  *	help_last;
extern		HELP_DATA	  *	help_free;

extern		SHOP_DATA	  *	shop_first;

extern		BAN_DATA	  *	ban_list;
extern          USERL_DATA        *     user_list;  /* finger data/mailing */
extern		CHAR_DATA	  *	char_list;
extern		ROOM_INDEX_DATA	  *	timed_room_list;
extern		DESCRIPTOR_DATA   *	descriptor_list;
extern		NOTE_DATA	  *	note_list;
extern		OBJ_DATA	  *	object_list;
extern          TRAP_DATA         *     trap_list;  /* Altech trap stuff */

extern		AFFECT_DATA	  *	affect_free;
extern		BAN_DATA	  *	ban_free;
extern		CHAR_DATA	  *	char_free;
extern		DESCRIPTOR_DATA	  *	descriptor_free;
extern		EXTRA_DESCR_DATA  *	extra_descr_free;
extern		NOTE_DATA	  *	note_free;
extern		OBJ_DATA	  *	obj_free;
extern		PC_DATA		  *	pcdata_free;

extern		char			bug_buf		[ ];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		KILL_DATA		kill_table	[ ];
extern		char			log_buf		[ ];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;
extern          char              *     down_time;
extern          char	          *     warning1;
extern          char		  *     warning2;
extern          int                     stype;
extern          int			prtv;
/*extern          bool 			quest;
extern          int			qmin;
extern		int			qmax;*/
extern          int                     port;
extern          int                     share_value;
extern          int                     pulse_db_dump;
extern		bool			relquest;  /* ahsile */
extern		CORRUPT_AREA_LIST *	corrupt;

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
/*DECLARE_DO_FUN( do_qecho   );
DECLARE_DO_FUN( do_qset         );
DECLARE_DO_FUN( do_questflag    );*/
DECLARE_DO_FUN( do_mpasound     );
DECLARE_DO_FUN( do_mpat         );
DECLARE_DO_FUN( do_mpastatus	);
DECLARE_DO_FUN( do_mpecho       );
DECLARE_DO_FUN( do_mpechoaround );
DECLARE_DO_FUN( do_mpechoat     );
DECLARE_DO_FUN( do_mpforce      );
DECLARE_DO_FUN( do_mpainxp       );
DECLARE_DO_FUN( do_mpainqp       );
DECLARE_DO_FUN( do_mpjunk       );
DECLARE_DO_FUN( do_mpkill       );
DECLARE_DO_FUN( do_mpmload      );
DECLARE_DO_FUN( do_mpoload      );
DECLARE_DO_FUN( do_mppurge      );
DECLARE_DO_FUN( do_mpstat       );
DECLARE_DO_FUN( do_mpcommands   );
DECLARE_DO_FUN( do_mpteleport   );
DECLARE_DO_FUN( do_mptransfer   );
DECLARE_DO_FUN( do_mpgoto   	);
DECLARE_DO_FUN( do_mpgainxp   	);
DECLARE_DO_FUN( do_mpgainqp   	);
DECLARE_DO_FUN( do_mpdisable  	);
DECLARE_DO_FUN( do_mpenable   	);
DECLARE_DO_FUN( do_account      );
DECLARE_DO_FUN(	do_advance	);
DECLARE_DO_FUN( do_makelegend   );
DECLARE_DO_FUN( change_area_status );
DECLARE_DO_FUN( do_afk          );
DECLARE_DO_FUN( do_affectedby   );
DECLARE_DO_FUN(	do_allow	);
DECLARE_DO_FUN(	do_answer	);
DECLARE_DO_FUN(	do_areas	);
DECLARE_DO_FUN( do_astrip       );
DECLARE_DO_FUN(	do_at		);
DECLARE_DO_FUN(	do_auction	);
DECLARE_DO_FUN( do_auto         );
DECLARE_DO_FUN( do_autoexit     );
DECLARE_DO_FUN( do_autogold     );
DECLARE_DO_FUN( do_autoloot     );
DECLARE_DO_FUN( do_autosac      );
DECLARE_DO_FUN(	do_bamfin	);
DECLARE_DO_FUN(	do_bamfout	);
DECLARE_DO_FUN( do_bamfsin	);	/* Tyrion */
DECLARE_DO_FUN( do_bamfsout	);	/* Tyrion */
DECLARE_DO_FUN(	do_ban		);
DECLARE_DO_FUN( do_bet          );              /* by Thelonius */
DECLARE_DO_FUN( do_bestow       );
DECLARE_DO_FUN( do_bid          );
DECLARE_DO_FUN( do_blank        );
DECLARE_DO_FUN( do_bodybag      );
DECLARE_DO_FUN(	do_brandish	);
DECLARE_DO_FUN( do_brief        );
DECLARE_DO_FUN(	do_bug		);
DECLARE_DO_FUN(	do_buy		);
DECLARE_DO_FUN(	do_cast		);
DECLARE_DO_FUN(	do_use		);
DECLARE_DO_FUN(	do_quickburst	);	/* Tyrion */
DECLARE_DO_FUN( do_multiburst	);	/* Tyrion */
DECLARE_DO_FUN(	do_channels	);
DECLARE_DO_FUN(	do_chat		);
DECLARE_DO_FUN( do_clan         );
DECLARE_DO_FUN( do_cinfo        );
DECLARE_DO_FUN( do_religioninfo );	/* Tyrion */
DECLARE_DO_FUN( do_clans        );
DECLARE_DO_FUN( do_religions	);	/* Tyrion */
DECLARE_DO_FUN( do_relquest	);	/* Ahsile */
DECLARE_DO_FUN( do_crusade	);	/* Ahsile */
DECLARE_DO_FUN( do_map		);	/* Ahsile */
DECLARE_DO_FUN( do_class        );
DECLARE_DO_FUN(	do_close	);
DECLARE_DO_FUN( do_combine      );
DECLARE_DO_FUN(	do_commands	);
DECLARE_DO_FUN(	do_compare	);
DECLARE_DO_FUN( do_conference   );
DECLARE_DO_FUN(	do_config	);
DECLARE_DO_FUN(	do_consider	);
DECLARE_DO_FUN( do_countcommands);
DECLARE_DO_FUN(	do_credits	);
DECLARE_DO_FUN(	do_deny		);
DECLARE_DO_FUN( do_deposit      );
DECLARE_DO_FUN(	do_description	);
DECLARE_DO_FUN(	do_disconnect	);
DECLARE_DO_FUN(	do_down		);
DECLARE_DO_FUN(	do_drink	);
DECLARE_DO_FUN(	do_drop		);
DECLARE_DO_FUN( do_dual         );
DECLARE_DO_FUN(	do_east		);
DECLARE_DO_FUN(	do_eat		);
DECLARE_DO_FUN(	do_echo		);
DECLARE_DO_FUN( do_pecho        );	/* Darkone */
DECLARE_DO_FUN( do_rename       );      /* rename players */
DECLARE_DO_FUN( do_wizpwd       );      /* Edit players' passwords */
DECLARE_DO_FUN( do_smite        );	/* Darkone */
DECLARE_DO_FUN( do_disable      );	/* Darkone */
DECLARE_DO_FUN( do_sedit        );	/* Darkone */
DECLARE_DO_FUN( do_for          );	/* Darkone */
DECLARE_DO_FUN( do_bank         );      /* Mud economy                  */
DECLARE_DO_FUN( do_update       );      /* Update game functions        */
DECLARE_DO_FUN( do_dog          );	/* Darkone */
DECLARE_DO_FUN( do_iscore       );	/* Darkone */
/*DECLARE_DO_FUN( do_wager        );*/
//DECLARE_DO_FUN( do_accept       );
//DECLARE_DO_FUN( do_arena        );
//DECLARE_DO_FUN( do_decline      );
//DECLARE_DO_FUN( do_challenge    );
DECLARE_DO_FUN(	do_emote	);
DECLARE_DO_FUN( do_enter        );
DECLARE_DO_FUN(	do_equipment	);
DECLARE_DO_FUN(	do_examine	);
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN(	do_fill		);
DECLARE_DO_FUN( do_dostat       );      /* Manaux */
DECLARE_DO_FUN( do_finger       );
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN( do_forge        );
DECLARE_DO_FUN(	do_follow	);
DECLARE_DO_FUN(	do_force	);
DECLARE_DO_FUN(	do_freeze	);
DECLARE_DO_FUN( do_fset         );
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN(	do_give		);
DECLARE_DO_FUN(	do_seize	);	/* Darkone */
DECLARE_DO_FUN(	do_goto		);
DECLARE_DO_FUN(	do_group	);
DECLARE_DO_FUN(	do_gtell	);
DECLARE_DO_FUN( do_guard        );
DECLARE_DO_FUN( do_timelord     );	/* Tyrion */
DECLARE_DO_FUN( do_ooc		);	/* Tyrion */
DECLARE_DO_FUN( do_info		);	/* Darkone */
DECLARE_DO_FUN(	do_help		);
DECLARE_DO_FUN( do_hero         );
DECLARE_DO_FUN( do_hlist        );
DECLARE_DO_FUN(	do_holylight	);
DECLARE_DO_FUN(	do_idea		);
DECLARE_DO_FUN(	do_immtalk	);
DECLARE_DO_FUN( do_imtlset      );	/* Darkone */
DECLARE_DO_FUN( do_induct       );
DECLARE_DO_FUN(	do_inventory	);
DECLARE_DO_FUN( do_invoke       );
DECLARE_DO_FUN( do_join         );
DECLARE_DO_FUN( do_timequake	);	/* Tyrion, real cool Time function */
DECLARE_DO_FUN(	do_kill		);
DECLARE_DO_FUN(	do_list		);
DECLARE_DO_FUN(	do_lock		);
DECLARE_DO_FUN(	do_log		);
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN(	do_memory	);
DECLARE_DO_FUN(	do_mfind	);
DECLARE_DO_FUN(	do_mload	);
DECLARE_DO_FUN( do_multi        );      /* Multi-class command. Walker. */
DECLARE_DO_FUN(	do_mset		);
DECLARE_DO_FUN(	do_set		);
DECLARE_DO_FUN(	do_mstat	);
DECLARE_DO_FUN(	do_stat		);
DECLARE_DO_FUN(	do_mwhere	);
DECLARE_DO_FUN(	do_murde	);
DECLARE_DO_FUN(	do_murder	);
DECLARE_DO_FUN(	do_music	);
DECLARE_DO_FUN( do_newcorpse	);	/* Tyrion */
DECLARE_DO_FUN(	do_newlock	);
DECLARE_DO_FUN(	do_noemote	);
DECLARE_DO_FUN(	do_north	);
DECLARE_DO_FUN(	do_note		);
DECLARE_DO_FUN( do_notestat     );
DECLARE_DO_FUN(	do_notell	);
DECLARE_DO_FUN(	do_numlock	);
DECLARE_DO_FUN(	do_ofind	);
DECLARE_DO_FUN( do_olist	);
DECLARE_DO_FUN(	do_oload	);
DECLARE_DO_FUN(	do_open		);
DECLARE_DO_FUN(	do_order	);
DECLARE_DO_FUN(	do_oset		);
DECLARE_DO_FUN(	do_ostat	);
DECLARE_DO_FUN( do_outcast      );
DECLARE_DO_FUN(	do_owhere	);
DECLARE_DO_FUN( do_pagelen      );
DECLARE_DO_FUN(	do_pardon	);
DECLARE_DO_FUN(	do_password	);
DECLARE_DO_FUN( do_patch        );
DECLARE_DO_FUN(	do_peace	);
DECLARE_DO_FUN( do_pload        );
DECLARE_DO_FUN(	do_pose		);
DECLARE_DO_FUN(	do_practice	);
DECLARE_DO_FUN( do_pray         );
DECLARE_DO_FUN( do_prompt       );
DECLARE_DO_FUN(	do_purge	);
DECLARE_DO_FUN(	do_put		);
DECLARE_DO_FUN( do_pwhere       );
DECLARE_DO_FUN(	do_quaff	);
DECLARE_DO_FUN( do_quest        );
DECLARE_DO_FUN(	do_question	);
DECLARE_DO_FUN(	do_qui		);
DECLARE_DO_FUN(	do_quit		);
DECLARE_DO_FUN(	do_reboo	);
DECLARE_DO_FUN(	do_reboot	);
DECLARE_DO_FUN(	do_recall	);
DECLARE_DO_FUN(	do_noob_recall	);
DECLARE_DO_FUN(	do_recho	);
DECLARE_DO_FUN(	do_recite	);
DECLARE_DO_FUN(	do_remove	);
DECLARE_DO_FUN(	do_rent		);
DECLARE_DO_FUN( do_repair       );
DECLARE_DO_FUN(	do_reply	);
DECLARE_DO_FUN(	do_report	);
DECLARE_DO_FUN(	do_rest		);
DECLARE_DO_FUN( do_restrict     );
DECLARE_DO_FUN(	do_restore	);
DECLARE_DO_FUN( do_retrieve     );
DECLARE_DO_FUN(	do_return	);
DECLARE_DO_FUN(	do_rset		);
DECLARE_DO_FUN(	do_rstat	);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_save		);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN(	do_score	);	/* Tyrion */
DECLARE_DO_FUN( do_pcscore	);	/* Tyrion */
DECLARE_DO_FUN(	do_sell		);
DECLARE_DO_FUN( do_separate     );
DECLARE_DO_FUN( do_setlev       );
DECLARE_DO_FUN(	do_shout	);
DECLARE_DO_FUN(	do_shutdow	);
DECLARE_DO_FUN(	do_shutdown	);
DECLARE_DO_FUN(	do_silence	);
DECLARE_DO_FUN(	do_sla		);
DECLARE_DO_FUN(	do_slay		);
DECLARE_DO_FUN(	do_sleep	);
DECLARE_DO_FUN( do_slist        );
DECLARE_DO_FUN(	do_slookup	);
DECLARE_DO_FUN(	do_snoop	);
DECLARE_DO_FUN(	do_socials	);
DECLARE_DO_FUN(	do_south	);
DECLARE_DO_FUN( do_spells       );
DECLARE_DO_FUN( do_spells2	);	/* Tyrion */
DECLARE_DO_FUN( do_skills	);	/* Ahsile */
DECLARE_DO_FUN(	do_split	);
DECLARE_DO_FUN(	do_sset		);
DECLARE_DO_FUN( do_sstat        );
DECLARE_DO_FUN( do_string       );
DECLARE_DO_FUN(	do_sstime	);
DECLARE_DO_FUN(	do_stand	);
DECLARE_DO_FUN( do_store        );
DECLARE_DO_FUN( do_study	);
DECLARE_DO_FUN(	do_switch	);
DECLARE_DO_FUN( do_control_switch);	/* Tyrion, for the CONTROL spells */
DECLARE_DO_FUN(	do_tell		);
DECLARE_DO_FUN( do_throw        );
DECLARE_DO_FUN(	do_time		);
DECLARE_DO_FUN(	do_title	);
DECLARE_DO_FUN(	do_train	);
DECLARE_DO_FUN(	do_transfer	);
DECLARE_DO_FUN(	do_trust	);
DECLARE_DO_FUN(	do_typo		);
DECLARE_DO_FUN(	do_unlock	);
DECLARE_DO_FUN(	do_up		);
DECLARE_DO_FUN(	do_users	);
DECLARE_DO_FUN(	do_value	);
DECLARE_DO_FUN(	do_visible	);
DECLARE_DO_FUN(	do_reality	);	/* Darkone */
DECLARE_DO_FUN( do_vused        );
DECLARE_DO_FUN(	do_wake		);
DECLARE_DO_FUN(	do_wear		);
DECLARE_DO_FUN(	do_weather	);
DECLARE_DO_FUN(	do_west		);
DECLARE_DO_FUN(	do_where	);
DECLARE_DO_FUN(	do_who		);
DECLARE_DO_FUN(	do_wimpy	);
DECLARE_DO_FUN( do_withdraw     );
DECLARE_DO_FUN(	do_wizhelp	);
DECLARE_DO_FUN( do_wizify       );
DECLARE_DO_FUN( do_wizlist      );
DECLARE_DO_FUN(	do_wizlock	);
DECLARE_DO_FUN( do_worth        );
DECLARE_DO_FUN( do_wrlist       );
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN(	do_zap		);
DECLARE_DO_FUN( do_fire		);	/* Tyrion */
DECLARE_DO_FUN( do_stare        );
/* XOR */
DECLARE_DO_FUN( do_vnum		);
DECLARE_DO_FUN( do_load		);
DECLARE_DO_FUN( do_push		);
DECLARE_DO_FUN( do_drag		);
DECLARE_DO_FUN( do_guild	);
DECLARE_DO_FUN( do_unguild	);
DECLARE_DO_FUN( do_setrank	);
DECLARE_DO_FUN( do_gdt		);
DECLARE_DO_FUN( do_authorize	);
DECLARE_DO_FUN( do_hide_obj     );      /* Hiding objects */
DECLARE_DO_FUN( do_search       );      /* Search for hidden items */
DECLARE_DO_FUN( do_lose         );      /* lose your followers */
DECLARE_DO_FUN( do_delet        );      /* delete pfile securety */
DECLARE_DO_FUN( do_delete       );      /* delete playerfile */
DECLARE_DO_FUN( do_demigod	);	/* Tyrion */
DECLARE_DO_FUN( do_mutter	);	/* Tyrion */
DECLARE_DO_FUN( do_gratz	);	/* Tyrion */
DECLARE_DO_FUN( do_clanview	);	/* Tyrion */
DECLARE_DO_FUN( do_clan_heal	);	/* Tyrion */
DECLARE_DO_FUN( do_mount	);	/* Tyrion */
DECLARE_DO_FUN( do_dismount	);	/* Tyrion */
DECLARE_DO_FUN( do_devote	);	/* Tyrion, for religion purposes */
DECLARE_DO_FUN( do_resetxp	);	/* Ahsile */
DECLARE_DO_FUN( do_realmemory	);	/* Ahsile */
DECLARE_DO_FUN( do_top		);	/* Ahsile */
DECLARE_DO_FUN( do_showcorrupt 	);	/* Ahsile */

DECLARE_DO_FUN( do_marry	);
DECLARE_DO_FUN( do_divorce	);
DECLARE_DO_FUN( do_rings	);

DECLARE_DO_FUN( do_remortalize	);	/* Tyrion */

DECLARE_DO_FUN( do_null );
DECLARE_DO_FUN( do_null_dis ); /* Hack for disabled commands - Ahsile */

/* Racial DO_FUN declarations */

DECLARE_DO_FUN( do_breathe_fire ); 	/* Tyrion */
DECLARE_DO_FUN( do_race_fly 	);	/* Tyrion */

/* Additions by The Maniac for Languages */

DECLARE_DO_FUN( do_common       );
DECLARE_DO_FUN( do_human        );
DECLARE_DO_FUN( do_dwarvish     );
DECLARE_DO_FUN( do_elvish       );
DECLARE_DO_FUN( do_gnomish      );
DECLARE_DO_FUN( do_dragon       );
DECLARE_DO_FUN( do_demon        );
DECLARE_DO_FUN( do_angel	);
DECLARE_DO_FUN( do_ogre         );
DECLARE_DO_FUN( do_drow         );
DECLARE_DO_FUN( do_elder        );
DECLARE_DO_FUN( do_pixie        );
DECLARE_DO_FUN( do_hobbit       );
DECLARE_DO_FUN( do_lizard       );
DECLARE_DO_FUN( do_minotaur     );
DECLARE_DO_FUN( do_halfling     );
DECLARE_DO_FUN( do_canine       );
DECLARE_DO_FUN( do_feline       );
DECLARE_DO_FUN( do_orcish	);
DECLARE_DO_FUN( do_magick	);
DECLARE_DO_FUN( do_shadow_talk	);
DECLARE_DO_FUN( do_spiritspeak 	);
DECLARE_DO_FUN( do_enlightened	);
DECLARE_DO_FUN( do_satanic	);
DECLARE_DO_FUN( do_animalspeak	);
DECLARE_DO_FUN( do_bretonnian	);
DECLARE_DO_FUN( do_lstat        );      /* language stat */
DECLARE_DO_FUN( do_lset         );      /* Language set */
DECLARE_DO_FUN( do_learn        );      /* learn a language */
DECLARE_DO_FUN( do_speak        );      /* Select a language to speak */
DECLARE_DO_FUN( do_gargoyle	);
DECLARE_DO_FUN( do_transmute    );
DECLARE_DO_FUN( do_utopian_healing);	/* Tyrion */
DECLARE_DO_FUN( do_image        );
DECLARE_DO_FUN( do_doomshield   );
DECLARE_DO_FUN( do_smash        );
DECLARE_DO_FUN( do_farsight     );
DECLARE_DO_FUN( do_palm         );   /* Darkone */
DECLARE_DO_FUN(	do_steal	);	/* Darkone */
DECLARE_DO_FUN(	do_invis	);
DECLARE_DO_FUN( do_heal         );              /* by Maniac    */

/*
 * All learnable skills are operated with the "use" command
 * - Ahsile
 */
DECLARE_SKILL_FUN( skill_assassinate			);
DECLARE_SKILL_FUN( skill_backstab				);
DECLARE_SKILL_FUN( skill_bash				);
DECLARE_SKILL_FUN( skill_berserk				);
DECLARE_SKILL_FUN( skill_cause_panic			); /* Darkone */
DECLARE_SKILL_FUN( skill_bloodthirsty			); /* Darkone */
DECLARE_SKILL_FUN( skill_chameleon			);
DECLARE_SKILL_FUN( skill_circle				);
DECLARE_SKILL_FUN( skill_dim_mak				); /* Darkone */
DECLARE_SKILL_FUN( skill_gravebind			); /* Tyrion */
DECLARE_SKILL_FUN( skill_embalm				); /* Tyrion */
DECLARE_SKILL_FUN( skill_lore					); /* Darkone */
DECLARE_SKILL_FUN( skill_snare				); /* Darkone */
DECLARE_SKILL_FUN( skill_untangle			); /* Darkone */
DECLARE_SKILL_FUN( skill_ethereal_wolf			); /* Tyrion */
DECLARE_SKILL_FUN( skill_ethereal_snake		); /* Tyrion */
DECLARE_SKILL_FUN( skill_study				); /* Studie spells from scrolls */
DECLARE_SKILL_FUN( skill_double_backstab		); /* Darkone */
DECLARE_SKILL_FUN( skill_triple_backstab		); /* Ahsile */
DECLARE_SKILL_FUN( skill_weaponmaster			); /* Darkone */
DECLARE_SKILL_FUN( skill_rage					); /* Darkone */
DECLARE_SKILL_FUN( skill_blindfold				); /* Darkone */
DECLARE_SKILL_FUN( skill_cloaking				); /* Darkone */
DECLARE_SKILL_FUN( skill_thick_skin			); /* Darkone */
DECLARE_SKILL_FUN( skill_paralyse				); /* Darkone */
DECLARE_SKILL_FUN( skill_slit					); /* Darkone */
DECLARE_SKILL_FUN( skill_devour				); /* Tyrion */
DECLARE_SKILL_FUN( skill_forage				); /* Tyrion */
DECLARE_SKILL_FUN( skill_divining				); /* Tyrion */
DECLARE_SKILL_FUN( skill_primalscream			); /* Tyrion */
DECLARE_SKILL_FUN( skill_axe_kick				); /* Tyrion */
DECLARE_SKILL_FUN( skill_death_strike			); /* Tyrion */
DECLARE_SKILL_FUN( skill_improved_hide		); /* Tyrion */
DECLARE_SKILL_FUN( skill_stealth				); /* Tyrion */
DECLARE_SKILL_FUN( skill_disguise			);
DECLARE_SKILL_FUN( skill_drain_life			);
DECLARE_SKILL_FUN( skill_gouge					);
DECLARE_SKILL_FUN( skill_dirt_kick				); /* Aglovale */
DECLARE_SKILL_FUN( skill_break_weapon			); /* Darkone */
DECLARE_SKILL_FUN( skill_rake					); /* Darkone */
DECLARE_SKILL_FUN( skill_brew				);
DECLARE_SKILL_FUN( skill_inscription				);
DECLARE_SKILL_FUN( skill_metamorph				); /* Darkone */
DECLARE_SKILL_FUN( skill_voodo				); /* Rewrote by Tyrion */
DECLARE_SKILL_FUN( skill_war_chant				);
DECLARE_SKILL_FUN( skill_stun					);
DECLARE_SKILL_FUN( skill_trip					); /* Darkone */
DECLARE_SKILL_FUN( skill_strangle				); /* Darkone */
DECLARE_SKILL_FUN( skill_soulstrike			);
DECLARE_SKILL_FUN( skill_sneak				);
DECLARE_SKILL_FUN( skill_shadow				);
DECLARE_SKILL_FUN( skill_shadow_walk		);
DECLARE_SKILL_FUN( skill_rescue				);
DECLARE_SKILL_FUN( skill_punch					);
DECLARE_SKILL_FUN( skill_poison_weapon		);
DECLARE_SKILL_FUN( skill_depoison_weapon	); /* Darkone */
DECLARE_SKILL_FUN( skill_pick				);
DECLARE_SKILL_FUN( skill_mental_drain			);
DECLARE_SKILL_FUN( skill_kick					);
DECLARE_SKILL_FUN( skill_back_kick				); /* Tyrion */
DECLARE_SKILL_FUN( skill_slam					); /* Darkone */
DECLARE_SKILL_FUN( skill_shriek				); /* Aglovale */
DECLARE_SKILL_FUN( skill_chop					); /* Aglovale */
DECLARE_SKILL_FUN( skill_head_butt				); /* Darkone */
DECLARE_SKILL_FUN( skill_claw					); /* Darkone */
DECLARE_SKILL_FUN( skill_bite					); /* Darkone */
DECLARE_SKILL_FUN( skill_irongrip			);
DECLARE_SKILL_FUN( skill_heighten			);
DECLARE_SKILL_FUN( skill_occulutus			); /* Aglovale  */
DECLARE_SKILL_FUN( skill_home_travel		);
DECLARE_SKILL_FUN( skill_quickness			); /* Darkone */
DECLARE_SKILL_FUN( skill_unwavering_reflexes   ); /* Ahsile */
DECLARE_SKILL_FUN( skill_hide				);
DECLARE_SKILL_FUN( skill_hunt				); /* by Maniac    */
DECLARE_SKILL_FUN( skill_retreat				); /* Darkone */
DECLARE_SKILL_FUN( skill_dim_mak				); /* Darkone */
DECLARE_SKILL_FUN( skill_feed					);
DECLARE_SKILL_FUN( skill_gorge				);
DECLARE_SKILL_FUN( skill_disarm	);
DECLARE_SKILL_FUN( skill_scan         );	/* Darkone */
DECLARE_SKILL_FUN( skill_skin );
DECLARE_SKILL_FUN( skill_tan  );
DECLARE_SKILL_FUN( skill_forestry  );
DECLARE_SKILL_FUN( skill_fletching  );
DECLARE_SKILL_FUN( skill_mining  );
DECLARE_SKILL_FUN( skill_forging  );
DECLARE_SKILL_FUN( skill_track  );
DECLARE_SKILL_FUN( skill_lightning_arrow );
/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(      spell_iceball           );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_cone_of_frost     );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_solidify          );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_cloud_of_cold     );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_winters_chill     );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_summon_ice_elemental );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_tomb_rot          );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_summon_water_elemental );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_summon_earth_elemental );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_summon_air_elemental );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_summon_dust_elemental );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_summon_dragon     );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_icequake          );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_circle_of_fire    );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_summon_fire_elemental );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_daemonic_might    );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_age               );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_blood_omen        );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_daemonic_possession );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_tortured_soul     );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_flash_burn        );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_summon_greater_demon );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_demonfire         );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_acid_blast	);
DECLARE_SPELL_FUN(      spell_animate           );
DECLARE_SPELL_FUN(	spell_armor		);
DECLARE_SPELL_FUN(      spell_astral            );
DECLARE_SPELL_FUN(      spell_aura              );
DECLARE_SPELL_FUN(      spell_shadow_plane      );	/* Darkone */
DECLARE_SPELL_FUN(	spell_bless		);
DECLARE_SPELL_FUN(	spell_blindness		);
DECLARE_SPELL_FUN(      spell_blood_bath        );
DECLARE_SPELL_FUN(	spell_burning_hands	);
DECLARE_SPELL_FUN(	spell_call_lightning	);
DECLARE_SPELL_FUN(	spell_cause_critical	);
DECLARE_SPELL_FUN(	spell_cause_light	);
DECLARE_SPELL_FUN(	spell_cause_serious	);
DECLARE_SPELL_FUN(	spell_change_sex	);
DECLARE_SPELL_FUN(	spell_charm_person	);
DECLARE_SPELL_FUN(      spell_fireshield        );
DECLARE_SPELL_FUN(      spell_demonshield        );
DECLARE_SPELL_FUN(      spell_acid_shield        );       /* Aglovale */
DECLARE_SPELL_FUN(	spell_chill_touch	);
DECLARE_SPELL_FUN(	spell_colour_spray	);
DECLARE_SPELL_FUN(	spell_continual_light	);
DECLARE_SPELL_FUN(	spell_control_weather	);
DECLARE_SPELL_FUN(	spell_create_food	);
DECLARE_SPELL_FUN(	spell_create_spring	);
DECLARE_SPELL_FUN(	spell_create_water	);
DECLARE_SPELL_FUN(	spell_cure_blindness	);
DECLARE_SPELL_FUN(	spell_cure_critical	);
DECLARE_SPELL_FUN(	spell_cure_light	);
DECLARE_SPELL_FUN(	spell_cure_poison	);
DECLARE_SPELL_FUN(	spell_cure_serious	);
DECLARE_SPELL_FUN(	spell_curse		);
DECLARE_SPELL_FUN(	spell_detect_evil	);
DECLARE_SPELL_FUN(	spell_detect_hidden	);
DECLARE_SPELL_FUN(	spell_detect_invis	);
DECLARE_SPELL_FUN(	spell_detect_magic	);
DECLARE_SPELL_FUN(	spell_detect_poison	);
DECLARE_SPELL_FUN(	spell_dispel_evil	);
DECLARE_SPELL_FUN(	spell_dispel_magic	);
DECLARE_SPELL_FUN(	spell_earthquake	);
DECLARE_SPELL_FUN(	spell_enchant_weapon	);
DECLARE_SPELL_FUN(	spell_energy_drain	);
DECLARE_SPELL_FUN(	spell_faerie_fire	);
DECLARE_SPELL_FUN(	spell_faerie_fog	);
DECLARE_SPELL_FUN(	spell_fireball		);
DECLARE_SPELL_FUN(	spell_flamestrike	);
DECLARE_SPELL_FUN(	spell_fly		);
DECLARE_SPELL_FUN(	spell_gate		);
DECLARE_SPELL_FUN(	spell_general_purpose	);
DECLARE_SPELL_FUN(	spell_giant_strength	);
DECLARE_SPELL_FUN(	spell_titan_strength	);
DECLARE_SPELL_FUN(	spell_haste		);
DECLARE_SPELL_FUN(      spell_swiftness         );
DECLARE_SPELL_FUN(      spell_goodberry         );
DECLARE_SPELL_FUN(	spell_harm		);
DECLARE_SPELL_FUN(	spell_heal		);
DECLARE_SPELL_FUN(	spell_high_explosive	);
DECLARE_SPELL_FUN(	spell_iceshield		);
DECLARE_SPELL_FUN(      spell_icestorm          );
DECLARE_SPELL_FUN(	spell_identify		);
DECLARE_SPELL_FUN(      spell_incinerate        );
DECLARE_SPELL_FUN(      spell_inertial          );
DECLARE_SPELL_FUN(	spell_infravision	);
DECLARE_SPELL_FUN(	spell_invis		);
DECLARE_SPELL_FUN(	spell_improved_invis	);	/* Darkone */
DECLARE_SPELL_FUN(	spell_know_alignment	);
DECLARE_SPELL_FUN(	spell_lightning_bolt	);
DECLARE_SPELL_FUN(	spell_locate_object	);
DECLARE_SPELL_FUN(	spell_magic_missile	);
DECLARE_SPELL_FUN(      spell_mana              );
DECLARE_SPELL_FUN(	spell_mass_invis	);
DECLARE_SPELL_FUN(      spell_mental_block      );
DECLARE_SPELL_FUN(	spell_pass_door		);
DECLARE_SPELL_FUN(      spell_permenancy        );
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_pestilence	);	/* Tyrion */
DECLARE_SPELL_FUN(      spell_blade_doom        );	/* Darkone */
DECLARE_SPELL_FUN(      spell_portal            );
DECLARE_SPELL_FUN(      spell_vine_portal       );	/* Darkone */
DECLARE_SPELL_FUN(	spell_protection	);
DECLARE_SPELL_FUN(	spell_refresh		);
DECLARE_SPELL_FUN(	spell_remove_curse	);
DECLARE_SPELL_FUN(	spell_remove_invis	);	/* Darkone */
DECLARE_SPELL_FUN(	spell_sanctuary		);
DECLARE_SPELL_FUN(      spell_scry              );
DECLARE_SPELL_FUN(	spell_shocking_grasp	);
DECLARE_SPELL_FUN(      spell_shockshield       );
DECLARE_SPELL_FUN(	spell_shield		);
DECLARE_SPELL_FUN(	spell_sleep		);
DECLARE_SPELL_FUN(      spell_spell_bind        );
DECLARE_SPELL_FUN(	spell_stone_skin	);
DECLARE_SPELL_FUN(	spell_summon		);
DECLARE_SPELL_FUN(	spell_teleport		);
DECLARE_SPELL_FUN(      spell_turn_undead       );
DECLARE_SPELL_FUN(	spell_ventriloquate	);
DECLARE_SPELL_FUN(	spell_weaken		);
DECLARE_SPELL_FUN(	spell_word_of_recall	);
DECLARE_SPELL_FUN(	spell_acid_breath	);
DECLARE_SPELL_FUN(	spell_fire_breath	);
DECLARE_SPELL_FUN(	spell_frost_breath	);
DECLARE_SPELL_FUN(	spell_gas_breath	);
DECLARE_SPELL_FUN(	spell_lightning_breath	);

DECLARE_SPELL_FUN(	spell_summon_swarm	);	/* XOR */
DECLARE_SPELL_FUN(	spell_summon_pack	);	/* XOR */
DECLARE_SPELL_FUN(	spell_summon_demon	);	/* XOR */
DECLARE_SPELL_FUN(	spell_cancellation	);	/* XOR */
DECLARE_SPELL_FUN(	spell_detect_good	);	/* XOR */
DECLARE_SPELL_FUN(	spell_protection_good	);	/* XOR */
DECLARE_SPELL_FUN(	spell_enchanted_song	);	/* XOR */
DECLARE_SPELL_FUN(	spell_holy_strength	);	/* XOR */
DECLARE_SPELL_FUN(	spell_curse_of_nature	);	/* XOR */
DECLARE_SPELL_FUN(      spell_holysword         );      /* ELVIS */
DECLARE_SPELL_FUN(      spell_summon_angel      );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_holy_fires        );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_truesight         );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_bladebarrier      );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_flame_blade       );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_chaos_blade       );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_frost_blade       );     /* ELVIS */
/* DECLARE_SPELL_FUN(      spell_acid_blade        );      Aglovale */
DECLARE_SPELL_FUN(      spell_web               );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_entangle          );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_darkbless         );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_confusion         );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_bio_acceleration  );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_mind_probe        );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_chain_lightning   );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_meteor_swarm      );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_psychic_quake     );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_fumble            );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_dancing_lights    );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_summon_shadow     );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_summon_beast      );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_summon_trent      );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_shatter           );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_molecular_unbind  );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_phase_shift       );     /* ELVIS */
DECLARE_SPELL_FUN(      spell_malignify         ); 	/* Tyrion */
DECLARE_SPELL_FUN(      spell_healing_hands     );     /* -- Altrag */
DECLARE_SPELL_FUN(      spell_plague            );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_unholy_curse      );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_unholy_wrath      );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_unholy_prayer     );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_golden_armor	);	/* Darkone */
DECLARE_SPELL_FUN(      spell_divining          );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_ghost_shield      );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_mist	        );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_chi_blast         );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_chi_storm         );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_chi_healing       );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_chi_wave          );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_shockwave         );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_sunburst          );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_phantom_form      );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_spark             );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_scrye             );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_visions           );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_shadow_image      );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_control_dragon    );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_control_hawk      );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_control_wolf      );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_control_tiger     );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_control_trent     );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_summon_hawk       );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_summon_wolf       );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_summon_tiger      );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_unholy_fires      );	/* Sethric */
DECLARE_SPELL_FUN(	spell_stigeon_mists     );	/* Sethric */
DECLARE_SPELL_FUN(      spell_satanic_caress    );	/* Sethric */
DECLARE_SPELL_FUN(      spell_control_undead    );	/* Sethric */
DECLARE_SPELL_FUN(      spell_satanic_inferno   );      /* Sethric */
DECLARE_SPELL_FUN(	spell_spark_blade	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_dispel_blade	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_armour_piercing_bullet ); /* Tyrion */
DECLARE_SPELL_FUN(	spell_emp_bullet	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_energy_pulse_bullet );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_laser_beam_bullet );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_particle_beam_bullet );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_flame_thrower_bullet );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_stun_gun_bullet	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_mortar_bullet	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_nails_bullet	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_nuclear_bullet	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_normal_bullet	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_white_light_bullet );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_freeze_bullet	);	/* Tyrion */
DECLARE_SPELL_FUN(      spell_musket_bullet     );      /* Tyrion */
DECLARE_SPELL_FUN(      spell_cannon_bullet     );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_crossbow_bullet   );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_dart_bullet	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_pie_bullet	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_arrow_bullet	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_wrath_of_god      );      /* Tyrion */
DECLARE_SPELL_FUN(	spell_aura_of_anti_magic );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_holy_protection   );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_soul_bind		);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_blood_gout	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_bloodshield	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_mother_natures_blessing );/* Tyrion */
DECLARE_SPELL_FUN(	spell_bark_skin		);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_thunder_strike	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_friend_of_nature  );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_bend_light        );	/* Keirkan */
DECLARE_SPELL_FUN(      spell_create_illusion   );	/* Keirkan */
DECLARE_SPELL_FUN(      spell_cloud_of_healing  );      /* Keirkan */
DECLARE_SPELL_FUN(      spell_earthblast        );      /* Keirkan */
DECLARE_SPELL_FUN(      spell_tale_of_terror    );      /* Keirkan */
DECLARE_SPELL_FUN(	spell_mana_shield	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_holy_sword_spell  );      /* Tyrion, used in spell_holysword */
DECLARE_SPELL_FUN(	spell_unholysword	);      /* Cuz it sucked the first time */
DECLARE_SPELL_FUN(	spell_unholy_sword_spell );	/* Tyrion, used in spell_unholysword */
DECLARE_SPELL_FUN(	spell_daggers_of_pain	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_spectral_armor	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_wisp_of_protection);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_vortex		);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_nagaroms_curse	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_prayer		);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_lullaby		);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_golden_sanctuary  );	/* Tyrion */
/*
 * Psi spell_functions, in magic.c.
 */
DECLARE_SPELL_FUN(      spell_adrenaline_control);
DECLARE_SPELL_FUN(      spell_agitation         );
DECLARE_SPELL_FUN(      spell_aura_sight        );
DECLARE_SPELL_FUN(      spell_awe               );
DECLARE_SPELL_FUN(      spell_ballistic_attack  );
DECLARE_SPELL_FUN(      spell_biofeedback       );
DECLARE_SPELL_FUN(      spell_cell_adjustment   );
DECLARE_SPELL_FUN(      spell_chaosfield        );
DECLARE_SPELL_FUN(      spell_combat_mind       );
DECLARE_SPELL_FUN(      spell_complete_healing  );
DECLARE_SPELL_FUN(      spell_control_flames    );
DECLARE_SPELL_FUN(      spell_create_sound      );
DECLARE_SPELL_FUN(      spell_death_field       );
DECLARE_SPELL_FUN(      spell_detonate          );
DECLARE_SPELL_FUN(      spell_disintegrate      );
DECLARE_SPELL_FUN(      spell_displacement      );
DECLARE_SPELL_FUN(      spell_disrupt           );
DECLARE_SPELL_FUN(      spell_domination        );
DECLARE_SPELL_FUN(      spell_ectoplasmic_form  );
DECLARE_SPELL_FUN(      spell_ego_whip          );
DECLARE_SPELL_FUN(      spell_energy_containment);
DECLARE_SPELL_FUN(      spell_enhance_armor     );
DECLARE_SPELL_FUN(      spell_enhanced_strength );
DECLARE_SPELL_FUN(      spell_flesh_armor       );
DECLARE_SPELL_FUN(      spell_inertial_barrier  );
DECLARE_SPELL_FUN(      spell_inflict_pain      );
DECLARE_SPELL_FUN(      spell_intellect_fortress);
DECLARE_SPELL_FUN(      spell_lend_health       );
DECLARE_SPELL_FUN(      spell_levitation        );
DECLARE_SPELL_FUN(      spell_spectral_wings    );
DECLARE_SPELL_FUN(      spell_mental_barrier    );
DECLARE_SPELL_FUN(      spell_mind_thrust       );
DECLARE_SPELL_FUN(      spell_project_force     );
DECLARE_SPELL_FUN(      spell_psionic_blast     );
DECLARE_SPELL_FUN(      spell_psychic_crush     );
DECLARE_SPELL_FUN(      spell_psychic_drain     );
DECLARE_SPELL_FUN(      spell_psychic_healing   );
DECLARE_SPELL_FUN(      spell_share_strength    );
DECLARE_SPELL_FUN(      spell_thought_shield    );
DECLARE_SPELL_FUN(      spell_ultrablast        );
DECLARE_SPELL_FUN(     gspell_flamesphere       );
DECLARE_SPELL_FUN(     gspell_mass_shield       );
DECLARE_SPELL_FUN(     gspell_smite_good       );
DECLARE_SPELL_FUN(     gspell_smite_evil       );
DECLARE_SPELL_FUN(     gspell_deadly_poison       );
DECLARE_SPELL_FUN(     gspell_volcanic_blast    );
DECLARE_SPELL_FUN(      spell_power_leak        );	/* Tyrion and Keirkan */
DECLARE_SPELL_FUN(	spell_breathe_water	);
DECLARE_SPELL_FUN(       spell_shadow_bolt      );      /* Tyrion */
DECLARE_SPELL_FUN(       spell_shadow_storm     );      /* Tyrion */
DECLARE_SPELL_FUN(	spell_mark		);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_rune_recall	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_randomshield	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_sonic_blast	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_sonic_boom	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_hellfire		);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_resurrection	);	/* Tyrion */
DECLARE_SPELL_FUN(     gspell_timequake		);      /* Tyrion */
DECLARE_SPELL_FUN(	spell_earthshield	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_leaf_shield	);	/* Tyrion */
DECLARE_SPELL_FUN(      spell_leaf_strike	);	/* Tyrion */
DECLARE_SPELL_FUN(     gspell_restoration	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_shards_of_glass	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_circle_of_love	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_luck_shield	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_tongues		);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_liquid_skin	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_spiritual_hammer	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_angelic_aura	);	/* Tyrion */
DECLARE_SPELL_FUN(      spell_waterspike        );      /* Ahsile */
DECLARE_SPELL_FUN(	spell_ethereal_wolf_bite );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_ethereal_wolf_claw );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_ethereal_wolf_howl );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_deception_of_aura  );  	/* Ahsile */
DECLARE_SPELL_FUN(	spell_ethereal_snake_bite );	/* Tyrion */
DECLARE_SPELL_FUN(      spell_leap  		);  	/* Ahsile */
DECLARE_SPELL_FUN(      spell_leap_of_torfi  	);  	/* Ahsile */
DECLARE_SPELL_FUN(      spell_tomba_di_vemon  	);  	/* Ahsile */
DECLARE_SPELL_FUN(	spell_niraks_curse_of_the_damned);	/* Tyrion */
DECLARE_SPELL_FUN(      spell_thieves_cant    	);	/* Ahsile */
DECLARE_SPELL_FUN(	spell_ethereal_snake_strike );  /* Tyrion */
DECLARE_SPELL_FUN(	spell_ethereal_snake_devour );	/* Tyrion */
DECLARE_SPELL_FUN(	spell_force_of_nature	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_heavy_fog		);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_forestwalk	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_mountainwalk	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_plainswalk	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_swampwalk		);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_tranquility	); 	/* Tyrion */
DECLARE_SPELL_FUN(	spell_lightning_storm	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_refreshing_rain	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_storm_seeker	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_essence_of_gaia	);	/* Tyrion */
DECLARE_SPELL_FUN(	spell_transmutation	);
DECLARE_SPELL_FUN(	spell_unholy_strength	);	/* Tyrion */
/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined( _AIX )
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined( apollo )
int	atoi		args( ( const char *string ) );
void *	calloc		args( ( unsigned nelem, size_t size ) );
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined( hpux )
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined( linux )
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined( macintosh )
#define NOCRYPT
#if	defined( unix )
#undef	unix
#endif
#endif

#if	defined( MIPS_OS )
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined( MSDOS )
#define NOCRYPT
#if	defined( unix )
#undef	unix
#endif
#endif

#if defined( RUN_AS_WIN32SERVICE )
#include "crypt.h"
#endif

//#include "mmgr.h"

#if	defined( NeXT )
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined( sequent )
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined( sun )
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf		args( ( FILE *stream, const char *format, ... ) );
size_t	fread	args( ( void *ptr, size_t size, size_t n, FILE *stream ) );
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined( ultrix )
char *	crypt		args( ( const char *key, const char *salt ) );
#endif



/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined( NOCRYPT )
#define crypt( s1, s2 )	( s1 )
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined( macintosh )
#define PLAYER_DIR	""		/* Player files			*/
#define NULL_FILE	"proto.are"	/* To reserve one stream	*/
#endif

#if defined(RUN_AS_WIN32SERVICE)
#define PLAYER_DIR "../player/"
#define NULL_FILE "nul"
#endif

#if defined( MSDOS )
#define PLAYER_DIR	""		/* Player files                 */
#define NULL_FILE	"nul"		/* To reserve one stream	*/
#endif

#if defined( unix )
#define PLAYER_DIR	"../player/"	/* Player files			*/
#define NULL_FILE	"/dev/null"	/* To reserve one stream	*/
#endif

#if defined( linux )
#define PLAYER_DIR	"../player/"	/* Player files			*/
#define NULL_FILE	"/dev/null"	/* To reserve one stream	*/
#endif

#if defined( linux )
#define MOB_DIR		"../MOBProgs/"  	/* MOBProg files	*/
#define NULL_FILE       "/dev/null"
#endif

#define AREA_LIST	"areaTS.lst"	/* List of areas		*/

#define BUG_FILE	"bugs.txt"      /* For 'bug' and bug( )		*/
#define IDEA_FILE	"ideas.txt"	/* For 'idea'			*/
#define TYPO_FILE	"typos.txt"     /* For 'typo'			*/
#define NOTE_FILE	"notes.txt"	/* For 'notes'			*/
#define CLAN_FILE       "clan.dat"      /* For 'clans'                  */
#define SHUTDOWN_FILE	"shutdown.txt"	/* For 'shutdown'		*/
#define DOWN_TIME_FILE  "time.txt"      /* For automatic shutdown       */
#define USERLIST_FILE   "users.txt"     /* Userlist -- using identd TRI */
#define DISABLED_FILE   "disabled.txt"  /* For disabled commands        */
#define BANK_FILE       "BANK.TXT"      /* For Bank Info                */
#define MULTI_FILE      "MULTI.TXT"     /* For Multi Info               */
#define RELIGION_FILE	"religion.dat"	/* For 'religions'		*/

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define CID     CLAN_DATA
#define SF	SPEC_FUN
#define GF      GAME_FUN
#define RLID	RELIGION_DATA

/* act_comm.c */
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch, char *name ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
bool	is_note_to	args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );
void    talk_channel_info       args( ( CHAR_DATA *ch, char *argument,
                                     int channel, const char *verb ) );
void	broadcast_channel  args( ( CHAR_DATA *ch, char *argument,
                                int channel, const char *sound ) );
void    broadcast_room  args( ( CHAR_DATA *ch, char *argument,
				const char *sound ) );
#ifdef SQL_SYSTEM
void	unlink_char		args( (CHAR_DATA* ch) );
void	link_char		args( (CHAR_DATA* ch) );
#endif


/* act_info.c */
void	set_title	args( ( CHAR_DATA *ch, char *title ) );
bool	check_blind	args( ( CHAR_DATA *ch ) );

/* act_move.c */
void	move_char	args( ( CHAR_DATA *ch, int door ) );

/* act_obj.c */

/* act_wiz.c */
ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );

/* crafting.c - callbacks */
void 	destroy_craft	args( ( CHAR_DATA* ch, bool failed ) );
void 	finish_craft	args( ( CHAR_DATA* ch ) );
void    strip_timed_room_flags args( ( ROOM_INDEX_DATA* room ) );
void	set_timed_room_flags   args( ( ROOM_INDEX_DATA* room, int flag, int timer ) );

/* comm.c */
void	close_socket	 args( ( DESCRIPTOR_DATA *dclose ) );
void	write_to_buffer	 args( ( DESCRIPTOR_DATA *d, const char *txt,
				int length ) );
void    send_to_all_char args( ( const char *text ) );
void    send_to_al       args( ( int clr, int level, char *text ) );
/* send to above level---^   TRI */
void	send_to_char	 args( ( int AType, const char *txt, CHAR_DATA *ch ) );
void    set_char_color   args( ( int AType, CHAR_DATA *ch ) );
void    show_string      args( ( DESCRIPTOR_DATA *d, char *input ) );
void	act	         args( ( int AType, const char *format, CHAR_DATA *ch,
				const void *arg1, const void *arg2,
				int type ) );
bool    is_pkillable    args( ( CHAR_DATA *ch, CHAR_DATA *victim ));
bool	IS_SHIELDABLE	args( ( CHAR_DATA *ch) );

/* act_multi.c */
void    SendMultiMessage  args( ( CHAR_DATA* ch ) );

/* db.c */
void	boot_db		args( ( void ) );
void	area_update	args( ( void ) );
CD *	create_mobile	args( ( MOB_INDEX_DATA *pMobIndex ) );
OD *	create_object	args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
void	free_char	args( ( CHAR_DATA *ch ) );
char *	get_extra_descr	args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *	get_mob_index	args( ( int vnum ) );
OID *	get_obj_index	args( ( int vnum ) );
RID *	get_room_index	args( ( int vnum ) );
CID *   get_clan_index  args( ( int vnum ) );
RLID *  get_religion_index args( ( int vnum)  );
char	fread_letter	args( ( FILE *fp ) );
int	fread_number	args( ( FILE *fp ) );
char *	fread_string	args( ( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
//void *	alloc_mem	args( ( int sMem ) );
void *	alloc_perm	args( ( int sMem ) );
void	free_mem	args( ( void *pMem, int sMem ) );
char *	str_dup		args( ( const char *str ) );
#ifdef MEM_DEBUG
void	f_string	args( ( char *pstr ) );
#else
void	free_string	args( ( char *pstr ) );
#endif
void    cleanup_strings( args( void  ) );
void    cleanup_disabled( args( void ) );
int	number_fuzzy	args( ( int number ) );
int	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
int	number_mm	args( ( void ) );
int	dice		args( ( int number, int size ) );
int	interpolate	args( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args( ( char *str ) );
bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *	capitalize	args( ( const char *str ) );
void	append_file	args( ( CHAR_DATA *ch, char *file, char *str ) );
void	bug		args( ( const char *str, int param ) );
void    logch           args( ( char *l_str, int l_type, int lvl ) );
void	log_string	args( ( char *str, int l_type, int level ) );
void	tail_chain	args( ( void ) );
void    load_disabled   args( ( void ) );
void    save_disabled   args( ( void ) );
bool 	find_corruption args( (char* aString) );

/* fight.c */
bool    can_use_skspell      args( ( CHAR_DATA *ch, int skspellNum ) );
void	violence_update	args( ( void ) );
void	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	damage		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt ) );
void	update_pos	args( ( CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
void	raw_kill	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
bool    is_safe         args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

/* fight2.c */
void	tag_newbie_slayer args( ( CHAR_DATA* ch ) );
bool   	find_missle_target args( ( int sn, CHAR_DATA *ch, char *argument, CHAR_DATA** victim, ROOM_INDEX_DATA** to_room, int* distance ) );
OBJ_DATA* find_arrows	args( ( CHAR_DATA* ch) );
OBJ_DATA* find_bolts	args( ( CHAR_DATA* ch) );
OBJ_DATA* find_bullets	args( ( CHAR_DATA* ch) );
OBJ_DATA* find_ammo		args( ( CHAR_DATA* ch, int type) );


/* handler.c */
int	get_trust	args( ( CHAR_DATA *ch ) );
int	get_age		args( ( CHAR_DATA *ch ) );
int	get_curr_str	args( ( CHAR_DATA *ch ) );
int	get_curr_int	args( ( CHAR_DATA *ch ) );
int	get_curr_wis	args( ( CHAR_DATA *ch ) );
int	get_curr_dex	args( ( CHAR_DATA *ch ) );
int	get_curr_con	args( ( CHAR_DATA *ch ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( const char *str, char *namelist ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_char2	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_char3	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_char4	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_to_char_powers 		args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_to_char_weaknesses	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_remove2  args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_remove3  args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_remove4  args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove_powers		args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove_weaknesses	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, int sn ) );
bool	is_affected	args( ( CHAR_DATA *ch, int sn ) );
bool	valid_aff_loc(int location);
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_join2    args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	add_poison	args( ( CHAR_DATA *ch, int amount ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    obj_to_storage  args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
void    obj_from_storage args(( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
void	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_world	args( ( CHAR_DATA *ch, char *argument ) );
CD *    get_char_area   args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			       OBJ_DATA *list ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument ) );
OD *    get_obj_storage args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_here	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	create_money	args( ( int amount ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	item_type_name	args( ( OBJ_DATA *obj ) );
char *	affect_loc_name	args( ( int location ) );
char *	affect_bit_name	args( ( int vector ) );
char * affect_bit_name2 args( ( int vector ) );
char * affect_bit_name3 args( ( int vector ) );
char * affect_bit_name4 args( ( int vector ) );
char * affect_bit_name_weaknesses args( ( int vector ) );
char * affect_bit_name_powers args ( ( int vector ) );
char *	extra_bit_name	args( ( int extra_flags ) );
char *	extra_bit_name2	args( ( int extra_flags2 ) );
char *  extra_bit_name3 args( ( int extra_flags3 ) );
char *  extra_bit_name4 args( ( int extra_flags4 ) );
char *  act_bit_name    args( ( int act ) );
char *  act_bit_name2    args( ( int act ) );
char *  imm_bit_name    args( ( int ) );	/* XOR */
CD   *  get_char        args( ( CHAR_DATA *ch ) );
bool    longstring      args( ( CHAR_DATA *ch, char *argument ) );
bool    authorized      args( ( CHAR_DATA *ch, char *skllnm ) );
void    end_of_game     args( ( void ) );
int     race_lookup     args( ( const char *race ) );
int     affect_lookup   args( ( const char *race ) );
int     advatoi         args( ( const char *s ) );
int     strlen_wo_col   args( ( char *argument ) );
char *  strip_color     args( ( char *argument ) );

/* interp.c */
void	interpret	args( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args( ( char *arg ) );
int	number_argument	args( ( char *argument, char *arg ) );
char *	one_argument	args( ( char *argument, char *arg_first ) );
bool    IS_SWITCHED     args( ( CHAR_DATA *ch ) );

/* magic.c */
int     slot_lookup     args( ( int slot ) );
bool    is_sn           args( ( int sn ) );
int	skill_lookup	args( ( const char *name ) );
bool	saves_spell	args( ( int level, CHAR_DATA *victim ) );
void	obj_cast_spell	args( ( int sn, int level, CHAR_DATA *ch,
			       CHAR_DATA *victim, OBJ_DATA *obj ) );
void    update_skpell   args( ( CHAR_DATA *ch, int sn, int override ) );
void    do_acspell      args( ( CHAR_DATA *ch, OBJ_DATA *pObj,
			        char *argument ) );

/* save.c */
bool    _stat            args( ( char *name ) );
void	save_char_obj	args( ( CHAR_DATA *ch, bool leftgame ) );
bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name ) );
void    corpse_back     args( ( CHAR_DATA *ch, OBJ_DATA *corpse ) );

/* sqlbc.c */
#ifdef SQL_SYSTEM

/* Data SQL Functions */
CHAR_DATA*		  sql_load_char( char* name );
OBJ_DATA*		  sql_load_items( int InsideOf, bool Storage, OBJ_DATA* inside, CHAR_DATA* ch);
ALIAS_DATA*		  sql_load_alias( int CharID );
AFFECT_DATA*	  sql_load_affects( int CharID, int AffNum );
TRAP_DATA*		  sql_load_traps( int vnum, int ProgType, int Exit );
MPROG_DATA*		  sql_load_mprog( int vnum );
EXTRA_DESCR_DATA* sql_load_ed(int vnum, char* table);
AFFECT_DATA*	  sql_load_objaff();

bool sql_load_exits(int vnum, EXIT_DATA* e[]);
bool sql_load_skills( int CharID, int table[]);
bool sql_load_lang( int CharId, int table[]);
bool sql_load_areas();
bool sql_load_rooms(AREA_DATA* pArea);
bool sql_load_disabled();
bool sql_load_bans();
bool sql_load_clans();
bool sql_load_helps();
bool sql_load_religions();
bool sql_load_resets();
bool sql_load_mobiles(AREA_DATA* pArea);
bool sql_load_objects();
bool sql_load_socials();
bool sql_load_shops();

bool remove_fastinv_tables();
bool create_fastinv_tables();

bool sql_save_char(CHAR_DATA* ch);
bool sql_save_inv(OBJ_DATA* obj, int CharID, int InsideOf, bool Storage, bool Dirty);
bool sql_save_dirty_list(bool Refresh);
bool sql_save_dirty_char(CHAR_DATA* ch, bool Quit);
void sql_save_aff(AFFECT_DATA* aff, int CharID, int Vector);
bool sql_save_area(AREA_DATA* pArea);
bool sql_save_rooms(AREA_DATA* pArea);
bool sql_save_mobiles(AREA_DATA* pArea);
bool sql_save_objects(AREA_DATA* pArea);
bool sql_save_mprogs(MPROG_DATA* mplist, int Vnum);
bool sql_save_traps(TRAP_DATA* traplist, int Vnum, int Type, int Exit);
bool sql_save_bans();
bool sql_save_disabled();
bool sql_save_clans();
bool sql_save_religions();
bool sql_save_helps();
bool sql_save_socials();
bool sql_save_resets();
bool sql_save_shops();


#endif

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );

/* games.c */
GF *    game_lookup     args( ( const char *name ) );

/* dead obj handling function */
bool clean_player_objects( CHAR_DATA *ch);



/* update.c */
void	advance_level	args( ( CHAR_DATA *ch ) );
void	gain_exp	args( ( CHAR_DATA *ch, int gain ) );
void	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void	update_handler	args( ( void ) );
void    bank_update     args( ( void ) );

/* mob_prog.c */
#ifdef DUNNO_STRSTR
char *  strstr                  args ( (const char *s1, const char *s2 ) );
#endif
void    mprog_wordlist_check    args ( ( char * arg, CHAR_DATA *mob,
                			CHAR_DATA* actor, OBJ_DATA* object,
					void* vo, int type ) );
void    mprog_percent_check     args ( ( CHAR_DATA *mob, CHAR_DATA* actor,
					OBJ_DATA* object, void* vo,
					int type ) );
void    mprog_act_trigger       args ( ( char* buf, CHAR_DATA* mob,
		                        CHAR_DATA* ch, OBJ_DATA* obj,
					void* vo ) );
void    mprog_bribe_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
		                        int amount ) );
void    mprog_entry_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_give_trigger      args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
                		        OBJ_DATA* obj ) );
void    mprog_greet_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_fight_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_hitprcnt_trigger  args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_death_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_random_trigger    args ( ( CHAR_DATA* mob ) );
void    mprog_speech_trigger    args ( ( char* txt, CHAR_DATA* mob ) );

/*
 * Lotsa triggers for ore_progs.. (ore_prog.c)
 * -- Altrag
 */
/*
 * Object triggers
 */
void    oprog_get_trigger       args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    oprog_get_from_trigger  args ( ( OBJ_DATA *obj, CHAR_DATA *ch,
					 OBJ_DATA *secondary ) );
void    oprog_give_trigger      args ( ( OBJ_DATA *obj, CHAR_DATA *ch,
					 CHAR_DATA *victim ) );
void    oprog_drop_trigger      args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    oprog_put_trigger       args ( ( OBJ_DATA *obj, CHAR_DATA *ch,
					 OBJ_DATA *secondary ) );
void    oprog_fill_trigger      args ( ( OBJ_DATA *obj, CHAR_DATA *ch,
					 OBJ_DATA *spring ) );
void    oprog_wear_trigger      args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    oprog_look_trigger      args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    oprog_look_in_trigger   args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    oprog_invoke_trigger    args ( ( OBJ_DATA *obj, CHAR_DATA *ch,
					 void *vo ) );
void    oprog_use_trigger       args ( ( OBJ_DATA *obj, CHAR_DATA *ch,
					 void *vo ) );
void    oprog_cast_trigger      args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    oprog_cast_sn_trigger   args ( ( OBJ_DATA *obj, CHAR_DATA *ch,
					 int sn, void *vo ) );
void    oprog_join_trigger      args ( ( OBJ_DATA *obj, CHAR_DATA *ch,
					 OBJ_DATA *secondary ) );
void    oprog_separate_trigger  args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    oprog_buy_trigger       args ( ( OBJ_DATA *obj, CHAR_DATA *ch,
					 CHAR_DATA *vendor ) );
void    oprog_sell_trigger      args ( ( OBJ_DATA *obj, CHAR_DATA *ch,
					 CHAR_DATA *vendor ) );
void    oprog_store_trigger     args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    oprog_retrieve_trigger  args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    oprog_open_trigger      args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    oprog_close_trigger     args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    oprog_lock_trigger      args ( ( OBJ_DATA *obj, CHAR_DATA *ch,
					 OBJ_DATA *key ) );
void    oprog_unlock_trigger    args ( ( OBJ_DATA *obj, CHAR_DATA *ch,
					 OBJ_DATA *key ) );
void    oprog_pick_trigger      args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void    oprog_random_trigger    args ( ( OBJ_DATA *obj ) );
void    oprog_throw_trigger     args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );

/*
 * Room triggers
 */
void    rprog_enter_trigger     args ( ( ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    rprog_exit_trigger      args ( ( ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    rprog_pass_trigger      args ( ( ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    rprog_cast_trigger      args ( ( ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    rprog_cast_sn_trigger   args ( ( ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch, int sn,
					 void *vo ) );
void    rprog_sleep_trigger     args ( ( ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    rprog_wake_trigger      args ( ( ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    rprog_rest_trigger      args ( ( ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    rprog_death_trigger     args ( ( ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    rprog_time_trigger      args ( ( ROOM_INDEX_DATA *room, int hour ) );
void    rprog_random_trigger    args ( ( ROOM_INDEX_DATA *room ) );

/*
 * Exit triggers
 */
void    eprog_enter_trigger     args ( ( EXIT_DATA *pExit,
					 ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    eprog_exit_trigger      args ( ( EXIT_DATA *pExit,
					 ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    eprog_pass_trigger      args ( ( EXIT_DATA *pExit,
					 ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch, bool fEnter ) );
void    eprog_look_trigger      args ( ( EXIT_DATA *pExit,
					 ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    eprog_scry_trigger      args ( ( EXIT_DATA *pExit,
					 ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    eprog_open_trigger      args ( ( EXIT_DATA *pExit,
					 ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    eprog_close_trigger     args ( ( EXIT_DATA *pExit,
					 ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );
void    eprog_lock_trigger      args ( ( EXIT_DATA *pExit,
					 ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch, OBJ_DATA *obj ) );
void    eprog_unlock_trigger    args ( ( EXIT_DATA *pExit,
					 ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch, OBJ_DATA *obj ) );
void    eprog_pick_trigger      args ( ( EXIT_DATA *pExit,
					 ROOM_INDEX_DATA *room,
					 CHAR_DATA *ch ) );

/*
 * gr_magic.c
 * -- Altrag
 */
void    check_gcast             args ( ( CHAR_DATA *ch ) );
void    group_cast              args ( ( int sn, int level, CHAR_DATA *ch,
					 char *argument ) );
void    set_gspell              args ( ( CHAR_DATA *ch, GSPELL_DATA *gsp ) );
void    end_gspell              args ( ( CHAR_DATA *ch ) );

/*
 * track.c
 */
/*void    hunt_victim             args ( ( CHAR_DATA *ch ) );
bool    can_go                  args ( ( CHAR_DATA *ch, int dir ) );*/

/*
 * chatmode.c
 */
/*void    start_chat_mode         args ( ( DESCRIPTOR_DATA *d ) );
void    chat_interp             args ( ( CHAR_DATA *ch, char *argument ) );*/

#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF


/*****************************************************************************
 *                                    OLC                                    *
 *****************************************************************************/

/*
 * This structure is used in special.c to lookup spec funcs and
 * also in olc_act.c to display listings of spec funcs.
 */
struct spec_type
{
    char *	spec_name;
    SPEC_FUN *	spec_fun;
};

struct game_type
{
    char *      game_name;
    GAME_FUN *  game_fun;
};


/*
 * This structure is used in bit.c to lookup flags and stats.
 */
struct flag_type
{
    char * name;
    int  bit;
    bool settable;
};



/*
 * Object defined in limbo.are
 * Used in save.c to load objects that don't exist.
 */
#define OBJ_VNUM_DUMMY	1



/*
 * Area flags.
 */
#define         AREA_NONE       0
#define         AREA_CHANGED    1	/* Area has been modified. */
#define         AREA_ADDED      2	/* Area has been added to. */
#define         AREA_LOADING    4	/* Used for counting in db.c */
#define		AREA_VERBOSE	8	/* Used for saving in save.c */
#define	 	AREA_PROTOTYPE 16       /* Prototype area(no mortals) */
#define	 	AREA_NOQUEST   32       /* Prototype area(no mortals) */
#define	 	AREA_FUTURE    64       /* Prototype area(no mortals) */
#define	 	AREA_PAST     128       /* Prototype area(no mortals) */
#define	 	AREA_PRESENT  256       /* Prototype area(no mortals) */
#define		AREA_NO_SAVE  512       /* Can't save characters here.
					  Manaux... */
#define		AREA_RANDOM   1024       /* Quest Area - No teleports */
#define		AREA_CORRUPT  2048     /* Catch corrupt areas. Make sure
						they don't save to disk
					- Ahsile			*/

#define MAX_DIR	6
#define NO_FLAG -99	/* Must not be used in flags or stats. */



/*
 * Interp.c
 */
DECLARE_DO_FUN( do_aedit        );	/* OLC 1.1b */
DECLARE_DO_FUN( do_redit        );	/* OLC 1.1b */
DECLARE_DO_FUN( do_oedit        );	/* OLC 1.1b */
DECLARE_DO_FUN( do_medit        );	/* OLC 1.1b */
DECLARE_DO_FUN( do_cedit        );      /* IchiCode 1.1b */
DECLARE_DO_FUN( do_hedit        );      /* XOR 3.14159265359r^2 */
DECLARE_DO_FUN( do_asave	);
DECLARE_DO_FUN( do_alist	);
DECLARE_DO_FUN( do_resets	);
DECLARE_DO_FUN( do_alias        );
DECLARE_DO_FUN( do_cleanstat    );
DECLARE_DO_FUN( do_reledit	);	/* Tyrion */


/*
 * Global Constants
 */
extern	char *	const	dir_name        [];
extern	const	int	rev_dir         [];
extern	const	struct	spec_type	spec_table	[];
extern  const   struct  game_type       game_table      [];



/*
 * Global variables
 */
extern          AREA_DATA *             area_first;
extern          AREA_DATA *             area_last;
extern          CLAN_DATA *             clan_first;
extern  	SHOP_DATA *             shop_last;
extern		RELIGION_DATA *		religion_first;

extern          int                     top_affect;
extern          int                     top_area;
extern          int                     top_ed;
extern          int                     top_exit;
extern          int                     top_help;
extern          int                     top_mob_index;
extern          int                     top_obj_index;
extern          int                     top_reset;
extern          int                     top_room;
extern          int                     top_shop;
extern          int                     top_clan;
extern		int			top_religion;

extern          int                     top_vnum_mob;
extern          int                     top_vnum_obj;
extern          int                     top_vnum_room;

extern          char                    str_empty       [1];

extern  MOB_INDEX_DATA *        mob_index_hash  [MAX_KEY_HASH];
extern  OBJ_INDEX_DATA *        obj_index_hash  [MAX_KEY_HASH];
extern  ROOM_INDEX_DATA *       room_index_hash [MAX_KEY_HASH];


/* db.c */
void	reset_area      args( ( AREA_DATA * pArea ) );
void	reset_room	args( ( ROOM_INDEX_DATA *pRoom ) );
void	free_ch		args( ( CHAR_DATA *ch ) );
/* string.c */
void	string_edit	args( ( CHAR_DATA *ch, char **pString ) );
void    string_append   args( ( CHAR_DATA *ch, char **pString ) );
char *	string_replace	args( ( char * orig, char * old, char * _new ) );
void    string_add      args( ( CHAR_DATA *ch, char *argument ) );
char *  format_string   args( ( char *oldstring /*, bool fSpace */ ) );
char *  first_arg       args( ( char *argument, char *arg_first, bool fCase ) );
char *	string_unpad	args( ( char * argument ) );
char *	string_proper	args( ( char * argument ) );
char *	all_capitalize	args( ( const char *str ) );	/* OLC 1.1b */
char *  string_delline  args( ( CHAR_DATA *ch, char *argument, char *old ) );
char * string_insline   args( ( CHAR_DATA *ch, char *argument, char *old ) );

/* olc.c */
bool	run_olc_editor	args( ( DESCRIPTOR_DATA *d ) );
char	*olc_ed_name	args( ( CHAR_DATA *ch ) );
char	*olc_ed_vnum	args( ( CHAR_DATA *ch ) );

/* magic.c */
bool pk_combat_check( CHAR_DATA * ch, CHAR_DATA * victim);

/* special.c */
char *	spec_string	args( ( SPEC_FUN *fun ) );	/* OLC */

/* bit.c */
extern const struct flag_type 	area_flags[];
extern const struct flag_type	sex_flags[];
extern const struct flag_type	exit_flags[];
extern const struct flag_type	door_resets[];
extern const struct flag_type	room_flags[];
extern const struct flag_type	timed_room_flags[];
extern const struct flag_type	sector_flags[];
extern const struct flag_type	type_flags[];
extern const struct flag_type	extra_flags[];
extern const struct flag_type	extra_flags2[];
extern const struct flag_type   extra_flags3[];
extern const struct flag_type   extra_flags4[];
extern const struct flag_type	wear_flags[];
extern const struct flag_type	act_flags[];
extern const struct flag_type	affect_flags[];
extern const struct flag_type   affect2_flags[];
extern const struct flag_type   affect3_flags[];
extern const struct flag_type   affect4_flags[];
extern const struct flag_type   affect_powers_flags[];
extern const struct flag_type	affect_weaknesses_flags[];
extern const struct flag_type	apply_flags[];
extern const struct flag_type	quality_flags[];
extern const struct flag_type	wear_loc_strings[];
extern const struct flag_type	wear_loc_flags[];
extern const struct flag_type	weapon_flags[];
extern const struct flag_type	damage_flags[];
extern const struct flag_type	ammo_flags[];
extern const struct flag_type	container_flags[];
extern const struct flag_type	liquid_flags[];
extern const struct flag_type   immune_flags[];
extern const struct flag_type   mprog_types[];
extern const struct flag_type   oprog_types[];
extern const struct flag_type   rprog_types[];
extern const struct flag_type   eprog_types[];

/* olc_act.c */
extern int flag_value       args ( ( const struct flag_type *flag_table,
				     char *argument ) );

extern void check_nofloor         args ( ( CHAR_DATA *ch ) );
extern char *flag_string     args ( ( const struct flag_type *flag_table,
 				      int bits ) );
extern void save_clans            args ( ( ) );
extern void save_religions	  args ( ( ) );
extern void wind_update           args ( ( AREA_DATA *pArea ) );
extern void send_to_area          args ( ( AREA_DATA *pArea, char *txt ) );

/* hunt.c */
void    hunt_victim     args( ( CHAR_DATA *ch ) );

/* language.c */
void    do_language     args( ( CHAR_DATA *ch, char *argument, int language) );
int     lang_lookup     args( ( const char *name ) );
char*	translation_garbled args( (char* speech, int miss_by, char* buf, int buflen ) );
/* drunk.c */
char    *makedrunk      args( (char *string ,CHAR_DATA *ch) );

/* social-edit.c */
void save_social_table  ( );
void load_social_table  ( );

/* games.c */
char *  game_string     args( ( GAME_FUN *fun ) );      /* OLC Maniac */


/* memory.c */
extern void free_affect		args ( ( AFFECT_DATA *ptr) );
extern void free_area		args ( ( AREA_DATA *ptr) );
extern void free_clan		args ( ( CLAN_DATA *ptr) );
extern void free_religion		args ( ( RELIGION_DATA *ptr) );
extern void free_ban	args ( ( BAN_DATA *ptr) );
extern void free_userl		args ( ( USERL_DATA *ptr) );
extern void free_char		args ( ( CHAR_DATA *ptr) );
extern void free_descriptor		args ( ( DESCRIPTOR_DATA *ptr) );
extern void free_exit		args ( ( EXIT_DATA *ptr) );
extern void free_extra_descr		args ( ( EXTRA_DESCR_DATA *ptr) );
extern void free_help		args ( ( HELP_DATA *ptr) );
extern void free_mob_index		args ( ( MOB_INDEX_DATA *ptr) );
extern void free_note		args ( ( NOTE_DATA *ptr) );
extern void free_obj		args ( ( OBJ_DATA *ptr) );
extern void free_obj_index		args ( ( OBJ_INDEX_DATA *ptr) );
extern void free_pc		args ( ( PC_DATA *ptr) );
extern void free_reset		args ( ( RESET_DATA *ptr) );
extern void free_room_index		args ( ( ROOM_INDEX_DATA *ptr) );
extern void free_shop		args ( ( SHOP_DATA *ptr) );
extern void free_disabled		args ( ( DISABLED_DATA *ptr) );
extern void free_mprog	args ( (MPROG_DATA  *ptr) );
extern void free_mprog_act		args ( ( MPROG_ACT_LIST *ptr) );
extern void free_alias		args ( ( ALIAS_DATA *ptr) );
extern void free_phobia		args ( ( PHOBIA_DATA *ptr) );
extern void free_trap		args ( ( TRAP_DATA *ptr) );
extern void free_corruptl	args ( ( CORRUPT_AREA_LIST* ptr) );
extern void free_allocated_mem args( ( void ) );

AFFECT_DATA* new_affect		();
AREA_DATA * new_area		();
CLAN_DATA * new_clan		();
RELIGION_DATA * new_religion	();
BAN_DATA * new_ban		();
USERL_DATA * new_userl		();
CHAR_DATA * new_char		();
DESCRIPTOR_DATA * new_descriptor();
EXIT_DATA * new_exit		();
EXTRA_DESCR_DATA * new_extra_descr();
HELP_DATA * new_help		();
MOB_INDEX_DATA * new_mob_index	();
NOTE_DATA * new_note	 	();
OBJ_DATA * new_obj		();
OBJ_INDEX_DATA * new_obj_index	();
PC_DATA * new_pc		();
RESET_DATA * new_reset		();
ROOM_INDEX_DATA * new_room_index();
SHOP_DATA * new_shop		();
DISABLED_DATA * new_disabled	();
MPROG_DATA * new_mprog		();
MPROG_ACT_LIST * new_mprog_act	();
ALIAS_DATA * new_alias		();
PHOBIA_DATA * new_phobia	();
TRAP_DATA * new_trap		();
CORRUPT_AREA_LIST* new_corruptl	();
