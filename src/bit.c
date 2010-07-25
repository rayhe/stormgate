/***************************************************************************
 *  File: bit.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was written by Jason Dinkel and inspired by Russ Taylor,     *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 The code below uses a table lookup system that is based on suggestions
 from Russ Taylor.  There are many routines in handler.c that would benefit
 with the use of tables.  You may consider simplifying your code base by
 implementing a system like below with such functions. -Jason Dinkel
*/

/*$Id: bit.c,v 1.47 2005/04/10 16:29:00 tyrion Exp $*/


#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



struct flag_stat_type
{
    const struct flag_type *structure;
    bool stat;
};



/*****************************************************************************
 Name:		flag_stat_table
 Purpose:	This table catagorizes the tables following the lookup
 		functions below into stats and flags.  Flags can be toggled
 		but stats can only be assigned.  Update this table when a
 		new set of flags is installed.
 ****************************************************************************/
const struct flag_stat_type flag_stat_table[] =
{
    {	area_flags,		FALSE	},
    {   sex_flags,		TRUE	},
    {   exit_flags,		FALSE	},
    {   door_resets,		TRUE	},
    {   room_flags,		FALSE	},
    {   sector_flags,		TRUE	},
    {   type_flags,		TRUE	},
    {   extra_flags,		FALSE	},
    {   extra_flags2,		FALSE	},
    {	extra_flags3,		FALSE	},
    {	extra_flags4,		FALSE	},
    {   wear_flags,		FALSE	},
    {   act_flags,		FALSE	},
    {   affect_flags,		FALSE	},
    {   affect2_flags,          FALSE   },
    {   affect3_flags,          FALSE   },
    {   affect4_flags,          FALSE   },
    {   affect_powers_flags,	FALSE   },
    {	affect_weaknesses_flags,FALSE	},
    {   apply_flags,		TRUE	},
	{	quality_flags,		TRUE	},
    {   wear_loc_flags,		TRUE	},
    {   wear_loc_strings,	TRUE	},
    {   weapon_flags,		TRUE	},
    {   ammo_flags,		TRUE	},
    {   damage_flags,		TRUE    },
    {   container_flags,	FALSE	},
    {   liquid_flags,		TRUE	},
    {   immune_flags,           FALSE   },
    {   mprog_types,            TRUE    },
    {   oprog_types,            FALSE   },
    {   rprog_types,            FALSE   },
    {   eprog_types,            FALSE   },

    {   0,			0	}
};
    


/*****************************************************************************
 Name:		is_stat( table )
 Purpose:	Returns TRUE if the table is a stat table and FALSE if flag.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
bool is_stat( const struct flag_type *flag_table )
{
    int flag;

    for (flag = 0; flag_stat_table[flag].structure; flag++)
    {
	if ( flag_stat_table[flag].structure == flag_table
	  && flag_stat_table[flag].stat )
	    return TRUE;
    }
    return FALSE;
}



/*
 * This function is Russ Taylor's creation.  Thanks Russ!
 * All code copyright (C) Russ Taylor, permission to use and/or distribute
 * has NOT been granted.  Use only in this OLC package has been granted.
 */
/*****************************************************************************
 Name:		flag_lookup( flag, table )
 Purpose:	Returns the value of a single, settable flag from the table.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
int flag_lookup(const char *name, const struct flag_type *flag_table)
{
    int flag;
 
    for (flag = 0; *flag_table[flag].name; flag++)	/* OLC 1.1b */
    {
        if ( !str_cmp( name, flag_table[flag].name )
          && flag_table[flag].settable )
            return flag_table[flag].bit;
    }
 
    return NO_FLAG;
}



/*****************************************************************************
 Name:		flag_value( table, flag )
 Purpose:	Returns the value of the flags entered.  Multi-flags accepted.
 Called by:	olc.c and olc_act.c.
 ****************************************************************************/
int flag_value( const struct flag_type *flag_table, char *argument)
{
    char word[MAX_INPUT_LENGTH];
    int  bit;
    int  marked = 0;
    bool found = FALSE;

    if ( is_stat( flag_table ) )
    {
	one_argument( argument, word );

	if ( ( bit = flag_lookup( word, flag_table ) ) != NO_FLAG )
	    return bit;
	else
	    return NO_FLAG;
    }

    /*
     * Accept multiple flags.stare bow

     */
    for (; ;)
    {
        argument = one_argument( argument, word );

        if ( word[0] == '\0' )
	    break;

        if ( ( bit = flag_lookup( word, flag_table ) ) != NO_FLAG )
        {
            SET_BIT( marked, bit );
	    SET_BIT2( marked, bit );
            found = TRUE;
        }
    }

    if ( found )
	return marked;
    else
	return NO_FLAG;
}



/*****************************************************************************
 Name:		flag_string( table, flags/stat )
 Purpose:	Returns string with name(s) of the flags or stat entered.
 Called by:	act_olc.c, olc.c, and olc_save.c.
 ****************************************************************************/
char *flag_string( const struct flag_type *flag_table, int bits )
{
    static char buf[512];
    int  flag;

    buf[0] = '\0';

    for (flag = 0; *flag_table[flag].name; flag++)	/* OLC 1.1b */
    {
	if ( !is_stat( flag_table ) && IS_SET(bits, flag_table[flag].bit) )
	{
	    strcat( buf, " " );
	    strcat( buf, flag_table[flag].name );
	}
	else
	if ( flag_table[flag].bit == bits )
	{
	    strcat( buf, " " );
	    strcat( buf, flag_table[flag].name );
	    break;
	}
    }
    return (buf[0] != '\0') ? buf+1 : "none";
}



const struct flag_type area_flags[] =
{
    {	"none",			AREA_NONE,		FALSE	},
    {	"changed",		AREA_CHANGED,		FALSE	},
    {	"added",		AREA_ADDED,		FALSE	},
    {	"loading",		AREA_LOADING,		FALSE	},
    {	"verbose",		AREA_VERBOSE,		FALSE	},
    {   "prototype",            AREA_PROTOTYPE,         FALSE   },
    {   "noquest",              AREA_NOQUEST,           FALSE   },
    {   "future",               AREA_FUTURE,            FALSE   },
    {   "past",                 AREA_PAST,              FALSE   },
    {   "present",              AREA_PRESENT,           FALSE   },
    {   "nosave",               AREA_NO_SAVE,           FALSE   },
    {   "random",             	AREA_RANDOM,            FALSE  	},
    {   "corrupt",		AREA_CORRUPT,		FALSE  	},

    {	"",			0,			0	}
};



const struct flag_type sex_flags[] =
{
    {	"male",			SEX_MALE,		TRUE	},
    {	"female",		SEX_FEMALE,		TRUE	},
    {	"neutral",		SEX_NEUTRAL,		TRUE	},
    {	"",			0,			0	}
};



const struct flag_type exit_flags[] =
{
    {   "door",	      EX_ISDOOR,		TRUE  },
    {	"closed",	      EX_CLOSED,		TRUE	},
    {	"locked",	      EX_LOCKED,		TRUE	},
    {	"bashed",	      EX_BASHED,		FALSE	},
    {	"bashproof",      EX_BASHPROOF,	TRUE	},
    {	"pickproof",      EX_PICKPROOF,	TRUE	},
    {	"passproof",      EX_PASSPROOF,	TRUE	},
    { "random",	      EX_RANDOM,        TRUE  },
    { "magiclock",      EX_MAGICLOCK,     TRUE  },
    { "hidden",         EX_HIDDEN,        TRUE  },
    {	"",			0,			0	}
};



const struct flag_type door_resets[] =
{
    {	"open and unlocked",	0,		TRUE	},
    {	"closed and unlocked",	1,		TRUE	},
    {	"closed and locked",	2,		TRUE	},
    {	"",			0,		0	}
};



const struct flag_type room_flags[] =
{
    {	"dark",			ROOM_DARK,		TRUE	},
    {   "no_shadow",         ROOM_NO_SHADOW,           TRUE    },
    {	"no_mob",		ROOM_NO_MOB,		TRUE	},
    {	"indoors",		ROOM_INDOORS,		TRUE	},
    {	"private",		ROOM_PRIVATE,		TRUE	},
    {	"safe",			ROOM_SAFE,		TRUE	},
    {	"solitary",		ROOM_SOLITARY,		TRUE	},
    {	"pet_shop",		ROOM_PET_SHOP,		TRUE	},
    {	"no_recall",		ROOM_NO_RECALL,		TRUE	},
    {	"cone_of_silence",	ROOM_CONE_OF_SILENCE,	TRUE	},
    {   "no_in",                ROOM_NO_ASTRAL_IN,      TRUE    },
    {   "no_out",               ROOM_NO_ASTRAL_OUT,     TRUE    },
    {   "tele_area",            ROOM_TELEPORT_AREA,     TRUE    },
    {   "tele_world",           ROOM_TELEPORT_WORLD,    TRUE    },
    {   "no_magic",             ROOM_NO_MAGIC,          TRUE    },
    {   "no_offensive",         ROOM_NO_OFFENSIVE,      TRUE    },
    {   "no_flee",              ROOM_NO_FLEE,           TRUE    },
    {   "silent",               ROOM_SILENT,            TRUE    },
    {   "bank",		        ROOM_BANK,              TRUE    },
    {   "nofloor",              ROOM_NOFLOOR,           TRUE    },
    {   "smithy",               ROOM_SMITHY,            TRUE    },
    {   "noscry",            ROOM_NOSCRY,             TRUE    },
    {   "damage",               ROOM_DAMAGE,            TRUE    },
    {   "pkill",                ROOM_PKILL,             TRUE    },
    {   "tannery",		ROOM_TANNERY,		TRUE    },
    {   "alchemist",		ROOM_ALCHEMIST,		TRUE    },
    {   "library",		ROOM_LIBRARY,		TRUE    },
    {   "forge",		ROOM_FORGE,		TRUE    },
    {   "bowyer",		ROOM_BOWYER,		TRUE    },
    {	"",			0,			0	}
};

const struct flag_type timed_room_flags[] =
{
    { 	"deforeseted",	ROOM_TIMED_DEFORESTED,	TRUE	},
    { 	"deforeseted",	ROOM_TIMED_MINED,	TRUE	},
    { 	"camp",		ROOM_TIMED_CAMP,		TRUE	},
	{	"",			0,			0	}
};

const struct flag_type sector_flags[] =
{
    {	"inside",	SECT_INSIDE,		TRUE	},
    {	"city",		SECT_CITY,		TRUE	},
    {	"field",	SECT_FIELD,		TRUE	},
    {	"forest",	SECT_FOREST,		TRUE	},
    {	"hills",	SECT_HILLS,		TRUE	},
    {	"mountain",	SECT_MOUNTAIN,		TRUE	},
    {	"swim",		SECT_WATER_SWIM,	TRUE	},
    {	"noswim",	SECT_WATER_NOSWIM,	TRUE	},
    {	"underwater",	SECT_UNDERWATER,	TRUE	},
    {	"air",		SECT_AIR,		TRUE	},
    {	"desert",	SECT_DESERT,		TRUE	},
    {   "badland",      SECT_BADLAND,           TRUE    },
    {	"",		0,			0	}
};



const struct flag_type type_flags[] =
{
    {	"light",		ITEM_LIGHT,		TRUE	},
    {	"scroll",		ITEM_SCROLL,		TRUE	},
    {	"wand",			ITEM_WAND,		TRUE	},
    {	"staff",		ITEM_STAFF,		TRUE	},
    {	"weapon",		ITEM_WEAPON,		TRUE	},
    {	"treasure",		ITEM_TREASURE,		TRUE	},
    {	"armor",		ITEM_ARMOR,		TRUE	},
    {	"potion",		ITEM_POTION,		TRUE	},
    {   "noteboard",            ITEM_NOTEBOARD,         TRUE    },
    {	"furniture",		ITEM_FURNITURE,		TRUE	},
    {	"trash",		ITEM_TRASH,		TRUE	},
    {	"container",		ITEM_CONTAINER,		TRUE	},
    {	"drink-container",	ITEM_DRINK_CON,		TRUE	},
    {   "blood",                ITEM_BLOOD,             TRUE    },
    {	"key",			ITEM_KEY,		TRUE	},
    {	"food",			ITEM_FOOD,		TRUE	},
    {	"money",		ITEM_MONEY,		TRUE	},
    {	"boat",			ITEM_BOAT,		TRUE	},
    {	"npc_corpse",		ITEM_CORPSE_NPC,	TRUE	},
    {	"pc_corpse",		ITEM_CORPSE_PC,		FALSE	},
    {	"fountain",		ITEM_FOUNTAIN,		TRUE	},
    {	"pill",			ITEM_PILL,		TRUE	},
    {   "contact",              ITEM_LENSE,             TRUE    },
    {   "portal",               ITEM_PORTAL,            TRUE    },
    {   "doll",                 ITEM_VODOO,             TRUE    },
    {   "berry",                ITEM_BERRY,             TRUE    },
    {   "firearm", 		ITEM_GUN,		TRUE    },
    {   "implant",		ITEM_IMPLANTED,		TRUE    },
    {   "rune",			ITEM_RUNE,		TRUE    },
    {   "skin",			ITEM_SKIN,		TRUE    },
    {	"bullet",		ITEM_BULLET,		TRUE	},
    {	"arrow",		ITEM_ARROW,		TRUE	},
    {	"bolt",			ITEM_BOLT,		TRUE	},
    {	"book",			ITEM_BOOK,		TRUE	},
    {	"needle",		ITEM_NEEDLE,		TRUE	},
    {	"quill",		ITEM_QUILL,		TRUE	},
    {	"forge_hammer",		ITEM_HAMMER,		TRUE	},
    {	"pestle_mortar",	ITEM_PESTLE,		TRUE	},
    {	"",			0,			0	}
};


const struct flag_type extra_flags[] =
{
    {	"glow",			ITEM_GLOW,		TRUE	},
    {	"hum",			ITEM_HUM,		TRUE	},
    {	"dark",			ITEM_DARK,		TRUE	},
    {	"lock",			ITEM_LOCK,		TRUE	},
    {	"evil",			ITEM_EVIL,		TRUE	},
    {	"invis",		ITEM_INVIS,		TRUE	},
    {	"magic",		ITEM_MAGIC,		TRUE	},
    {	"nodrop",		ITEM_NODROP,		TRUE	},
    {	"bless",		ITEM_BLESS,		TRUE	},
    {	"anti_good",		ITEM_ANTI_GOOD,		TRUE	},
    {	"anti_evil",		ITEM_ANTI_EVIL,		TRUE	},
    {	"anti_neutral",		ITEM_ANTI_NEUTRAL,	TRUE	},
    {   "anti_mage",            ITEM_ANTI_MAGE,         TRUE    },
    {   "anti_paladin",         ITEM_ANTI_PALADIN,      TRUE    },
    {   "anti_cleric",          ITEM_ANTI_CLERIC,       TRUE    },
    {   "anti_thief",           ITEM_ANTI_THIEF,        TRUE    },
    {   "anti_warrior",         ITEM_ANTI_WARRIOR,      TRUE    },
    {   "anti_psi",             ITEM_ANTI_PSI,          TRUE    },
    {   "anti_druid",           ITEM_ANTI_DRUID,        TRUE    },
    {   "anti_ranger",          ITEM_ANTI_RANGER,       TRUE    },
    {   "anti_bard",            ITEM_ANTI_BARD,         TRUE    },
    {   "anti_vamp",            ITEM_ANTI_VAMP,         TRUE    },
    {	"noremove",		ITEM_NOREMOVE,		TRUE	},
    {	"inventory",		ITEM_INVENTORY,		TRUE	},
    {	"poisoned",		ITEM_POISONED,		TRUE	},
    {   "nolocate",             ITEM_NO_LOCATE,	        TRUE    },
    {   "nodamage",             ITEM_NO_DAMAGE,         TRUE    },
    {   "frosty",               ITEM_ICY,               TRUE    },
    {   "flame",                ITEM_FLAME,             TRUE    },
    {   "chaos",                ITEM_CHAOS,             TRUE    },
    {   "patched",              ITEM_PATCHED,           TRUE    },
 /*   {   "acidic",              ITEM_ACID,             TRUE    },  */
    {	"",			0,			0	}
};

const struct flag_type extra_flags2[] =
{
    {   "anti_barbarian",       ITEM_ANTI_BARBARIAN,    TRUE    },
    {   "anti_antipal",         ITEM_ANTI_ANTIPAL,      TRUE    },
    {   "anti_monk",            ITEM_ANTI_MONK,         TRUE    },
    {   "anti_assassin",        ITEM_ANTI_ASSASSIN,     TRUE    },
    {   "anti_illusionist",     ITEM_ANTI_ILLUSIONIST,  TRUE    },
    {   "anti_werewolf",        ITEM_ANTI_WEREWOLF,     TRUE    },
    {   "holy",			ITEM_HOLY,		TRUE	},
    {	"nopurge",		ITEM_NOPURGE,		TRUE	},
    {   "hidden",               ITEM_HIDDEN,            TRUE    },
    {   "anti_necromancer",     ITEM_ANTI_NECROMANCER,  TRUE    },
    {   "anti_demonologist",    ITEM_ANTI_DEMONOLOGIST, TRUE    },
    {   "anti_shaman",          ITEM_ANTI_SHAMAN,       TRUE    },
    {   "anti_darkpriest",      ITEM_ANTI_DARKPRIEST,   TRUE    },
    {   "sparking",	   	ITEM_SPARKING,		TRUE	},
    {   "dispel",		ITEM_DISPEL,		TRUE	},
    {   "two_handed",		ITEM_TWO_HANDED,	TRUE	},
    {	"clan",			ITEM_CLAN,		TRUE	},
    {	"no_steal",		ITEM_NO_STEAL,		TRUE	},
    {	"no_sac",		ITEM_NO_SAC,		TRUE	},
    {	"reboot_only",		ITEM_REBOOT_ONLY,	TRUE	},
    {	"permanent",		ITEM_PERMANENT,		TRUE    },
    {	"legend_only",		ITEM_LEGEND,		TRUE	},
	{	"quest_reward",		ITEM_QUEST,		TRUE	},
	{   "magic_shot",		ITEM_MAGIC_SHOT, TRUE   },
	{   "crafted",		ITEM_CRAFTED, TRUE   },

    {	"",			 0,			0	}
};

const struct flag_type extra_flags3[] =
{
    {   "pro_mage",             ITEM_PRO_MAGE,          TRUE    },
    {   "pro_cleric",           ITEM_PRO_CLERIC,        TRUE    },
    {   "pro_thief",            ITEM_PRO_THIEF,         TRUE    },
    {   "pro_warrior",          ITEM_PRO_WARRIOR,       TRUE    },
    {   "pro_psi",              ITEM_PRO_PSI,           TRUE    },
    {   "pro_druid",            ITEM_PRO_DRUID,         TRUE    },
    {   "pro_ranger",           ITEM_PRO_RANGER,        TRUE    },
    {   "pro_paladin",          ITEM_PRO_PALADIN,       TRUE    },
    {   "pro_bard",             ITEM_PRO_BARD,          TRUE    },
    {   "pro_vamp",             ITEM_PRO_VAMP,          TRUE    },
    {   "pro_werewolf",         ITEM_PRO_WEREWOLF,      TRUE    },
    {   "pro_antipal",          ITEM_PRO_ANTIPAL,       TRUE    },
    {   "pro_assassin",         ITEM_PRO_ASSASSIN,      TRUE    },
    {   "pro_monk",             ITEM_PRO_MONK,          TRUE    },
    {   "pro_barbarian",        ITEM_PRO_BARBARIAN,     TRUE    },
    {   "pro_illusionist",      ITEM_PRO_ILLUSIONIST,   TRUE    },
    {   "pro_necromancer",      ITEM_PRO_NECROMANCER,   TRUE    },
    {   "pro_demonologist",     ITEM_PRO_DEMONOLOGIST,  TRUE    },
    {   "pro_shaman"     ,      ITEM_PRO_SHAMAN,        TRUE    },
    {   "pro_darkpriest",       ITEM_PRO_DARKPRIEST,    TRUE    },
    {   "pro_human",            ITEM_PRO_HUMAN,         TRUE    },
    {   "pro_elf",              ITEM_PRO_ELF,           TRUE    },
    {   "pro_halfelf",          ITEM_PRO_HALFELF,       TRUE    },
    {   "pro_orc",              ITEM_PRO_ORC,           TRUE    },
    {   "pro_drow",             ITEM_PRO_DROW,          TRUE    },
    {   "pro_dwarf",            ITEM_PRO_DWARF,         TRUE    },
    {   "pro_halfdwarf",        ITEM_PRO_HALFDWARF,     TRUE    },
    {   "pro_hobbit",           ITEM_PRO_HOBBIT,        TRUE    },
    {   "pro_giant",            ITEM_PRO_GIANT,         TRUE    },
    {   "pro_ogre",             ITEM_PRO_OGRE,          TRUE    },
    {   "",                      0,                     0       }
};

const struct flag_type extra_flags4[] =
{
    {   "pro_angel",            ITEM_PRO_ANGEL,         TRUE    },
    {   "pro_minotaur",         ITEM_PRO_MINOTAUR,      TRUE    },
    {   "pro_feline",           ITEM_PRO_FELINE,        TRUE    },
    {   "pro_gargoyle",         ITEM_PRO_GARGOYLE,      TRUE    },
    {   "pro_canine",           ITEM_PRO_CANINE,        TRUE    },
    {   "pro_demon",            ITEM_PRO_DEMON,         TRUE    },
    {   "pro_pixie",            ITEM_PRO_PIXIE,         TRUE    },
    {   "pro_elder",            ITEM_PRO_ELDER,         TRUE    },
    {   "pro_lizardman",        ITEM_PRO_LIZARDMAN,     TRUE    },
    {   "pro_nome",             ITEM_PRO_GNOME,         TRUE    },
    {   "anti_human",           ITEM_ANTI_HUMAN,        TRUE    },
    {   "anti_elf",             ITEM_ANTI_ELF,          TRUE    },
    {   "anti_halfelf",         ITEM_ANTI_HALFELF,      TRUE    },
    {   "anti_orc",             ITEM_ANTI_ORC,          TRUE    },
    {   "anti_drow",            ITEM_ANTI_DROW,         TRUE    },
    {   "anti_dwarf",           ITEM_ANTI_DWARF,        TRUE    },
    {   "anti_halfdwarf",       ITEM_ANTI_HALFDWARF,    TRUE    },
    {   "anti_hobbit",          ITEM_ANTI_HOBBIT,       TRUE    },
    {   "anti_giant",           ITEM_ANTI_GIANT,        TRUE    },
    {   "anti_ogre",            ITEM_ANTI_OGRE,         TRUE    },
    {   "anti_angel",           ITEM_ANTI_ANGEL,        TRUE    },
    {   "anti_minotaur",        ITEM_ANTI_MINOTAUR,     TRUE    },
    {   "anti_feline",          ITEM_ANTI_FELINE,       TRUE    },
    {   "anti_gargoyle",        ITEM_ANTI_GARGOYLE,     TRUE    },
    {   "anti_canine",          ITEM_ANTI_CANINE,       TRUE    },
    {   "anti_demon",           ITEM_ANTI_DEMON,        TRUE    },
    {   "anti_pixie",           ITEM_ANTI_PIXIE,        TRUE    },
    {   "anti_elder",           ITEM_ANTI_ELDER,        TRUE    },
    {   "anti_lizardman",       ITEM_ANTI_LIZARDMAN,    TRUE    },
    {   "anti_gnome",           ITEM_ANTI_GNOME,        TRUE    },
    {   "",                      0,                     0       }
};

const struct flag_type wear_flags[] =
{
    {	"take",			ITEM_TAKE,		TRUE	},
    {	"finger",		ITEM_WEAR_FINGER,	TRUE	},
    {	"neck",			ITEM_WEAR_NECK,		TRUE	},
    {	"body",			ITEM_WEAR_BODY,		TRUE	},
    {	"head",			ITEM_WEAR_HEAD,		TRUE	},
    {	"legs",			ITEM_WEAR_LEGS,		TRUE	},
    {	"feet",			ITEM_WEAR_FEET,		TRUE	},
    {	"hands",		ITEM_WEAR_HANDS,	TRUE	},
    {	"arms",			ITEM_WEAR_ARMS,		TRUE	},
    {	"shield",		ITEM_WEAR_SHIELD,	TRUE	},
    {	"about",		ITEM_WEAR_ABOUT,	TRUE	},
    {	"waist",		ITEM_WEAR_WAIST,	TRUE	},
    {	"wrist",		ITEM_WEAR_WRIST,	TRUE	},
    {	"wield",		ITEM_WIELD,		TRUE	},
    {	"hold",			ITEM_HOLD,		TRUE	},
    {   "lense",                ITEM_WEAR_CONTACT,      TRUE    },
    {   "orbit",                ITEM_WEAR_ORBIT,        TRUE    },
    {   "mask",                 ITEM_WEAR_FACE,         TRUE    },
    {   "ears",                 ITEM_WEAR_EARS,         TRUE    },
    {   "ankle",                ITEM_WEAR_ANKLE,        TRUE    },
    {   "holster",		ITEM_WEAR_FIREARM,	TRUE,   },
    {   "implanted",		ITEM_WEAR_IMPLANTED,	TRUE,   },
    {	"",			0,			0	}
};



const struct flag_type act_flags[] =
{
    {	"npc",			ACT_IS_NPC,		FALSE	},
    {	"sentinel",		ACT_SENTINEL,		TRUE	},
    {	"scavenger",		ACT_SCAVENGER,		TRUE	},
    {	"aggressive",		ACT_AGGRESSIVE,		TRUE	},
    {	"stay_area",		ACT_STAY_AREA,		TRUE	},
    {	"wimpy",		ACT_WIMPY,		TRUE	},
    {	"pet",			ACT_PET,		TRUE	},
    {	"train",		ACT_TRAIN,		TRUE	},
    {	"practice",		ACT_PRACTICE,		TRUE	},
    {   "undead",               ACT_UNDEAD,             TRUE    },
    {   "track",                ACT_TRACK,              TRUE    },
    {   "banker",               ACT_BANKER,             TRUE    },
    {   "teacher",              ACT_TEACHER,            TRUE    },
    {	"gamble",		ACT_GAMBLE,		TRUE	},
    {   "healer",               ACT_IS_HEALER,          TRUE    },
    {   "classmaster",		ACT_CLASSMASTER,	TRUE    },
    {   "no_exp",		ACT_NO_EXP,		TRUE    },
    {   "clanhealer",		ACT_IS_CLAN_HEALER,	TRUE	},
    {	"vehicle",		ACT_VEHICLE,		TRUE	},
    {	"no_corpse",		ACT_NO_CORPSE,		TRUE	},
    {   "deity",		ACT_IS_DEITY,		TRUE    },
    {   "boss",			ACT_RELBOSS,		TRUE    },
    {	"",			0,			0	}
};



const struct flag_type affect_flags[] =
{
    {	"blind",		AFF_BLIND,		TRUE	},
    {	"invisible",		AFF_INVISIBLE,		TRUE	},
    {	"detect_evil",		AFF_DETECT_EVIL,	TRUE	},
    {	"detect_invis",		AFF_DETECT_INVIS,	TRUE	},
    {	"detect_magic",		AFF_DETECT_MAGIC,	TRUE	},
    {	"detect_hidden",	AFF_DETECT_HIDDEN,	TRUE	},
    {	"haste",		AFF_HASTE,		TRUE	},
    {	"sanctuary",		AFF_SANCTUARY,		TRUE	},
    {   "fireshield",           AFF_FIRESHIELD,         TRUE    },
    {   "shockshield",          AFF_SHOCKSHIELD,        TRUE    },
    {   "iceshield",            AFF_ICESHIELD,          TRUE    },
    {   "chaos_field",          AFF_CHAOS,              TRUE    },
    {	"faerie_fire",		AFF_FAERIE_FIRE,	TRUE	},
    {	"infrared",		AFF_INFRARED,		TRUE	},
    {	"curse",		AFF_CURSE,		TRUE	},
    {	"flaming",		AFF_FLAMING,		FALSE	},
    {	"poison",		AFF_POISON,		TRUE	},
    {	"protect",		AFF_PROTECT,		TRUE	},
    {	"vibrating",		AFF_INERTIAL,		TRUE	},
    {	"sneak",		AFF_SNEAK,		TRUE	},
    {	"hide",			AFF_HIDE,		TRUE	},
    {	"sleep",		AFF_SLEEP,		TRUE	},
    {	"charm",		AFF_CHARM,		TRUE	},
    {	"flying",		AFF_FLYING,		TRUE	},
    {	"pass_door",		AFF_PASS_DOOR,		TRUE	},
    {	"stunned",		AFF_STUN,		TRUE	},
    {	"summoned",		AFF_SUMMONED,		FALSE	},
    {	"mute",			AFF_MUTE,		TRUE	},
    {	"peace",		AFF_PEACE,		TRUE	},
    {   "webbed",		AFF_ANTI_FLEE,		TRUE    },
    {	"",			0,			0	}
};

const struct flag_type affect2_flags [] =
{
    {   "mental_block",         AFF_NOASTRAL,           TRUE    },
    {	"improved_invis",	AFF_IMPROVED_INVIS,	TRUE	},
    {   "true_sight",           AFF_TRUESIGHT,          TRUE    },
    {   "golden_armor",		AFF_GOLDEN_ARMOR,	TRUE	},
    {   "mist",        		AFF_MIST,	        TRUE    },
    {   "ghost_shield",        	AFF_GHOST_SHIELD,	TRUE    },
    {   "shadow_image",		AFF_SHADOW_IMAGE,	TRUE	},
    {   "blade_barrier",        AFF_BLADE,              TRUE    },
    {   "detect_good",          AFF_DETECT_GOOD,        TRUE    },
    {   "protect_good",         AFF_PROTECTION_GOOD,    TRUE    },
    {   "berserk",              AFF_BERSERK,            FALSE   },
    {   "weaponmaster",    	AFF_WEAPONMASTER,	FALSE	},
    {   "blindfold",    	AFF_BLINDFOLD, 		TRUE	},
    {   "cloaking",     	AFF_CLOAKING, 		TRUE	},
    {   "thick_skin",   	AFF_THICK_SKIN, 	TRUE	},
    {   "slit",		    	AFF_SLIT, 		TRUE	},
    {   "malignify",	    	AFF_MALIGNIFY, 		TRUE	},
    {   "phased",		AFF_PHASED,		FALSE   },
    {   "curse_of_nature",	AFF_CURSE_NATURE,	FALSE   },
    {   "shadow_plane",		AFF_SHADOW_PLANE, 	FALSE	},
    {   "metamorph",     	AFF_WOLFED, 		FALSE	},
    {   "confused",		AFF_CONFUSED,		TRUE    },
    {   "fumble",		AFF_FUMBLE,		FALSE   },
    {   "dancing_lights", 	AFF_DANCING,		FALSE   },
    {	"unholy_strength",	AFF_UNHOLY_STRENGTH,	TRUE	},
    {   "", 			0,			0	}
};

const struct flag_type affect3_flags [] =
{
    {   "rage",  	  	AFF_RAGE,		FALSE	},
    {   "gills",		AFF_GILLS,		TRUE    },
    {   "bloodthirsty",         AFF_BLOODTHIRSTY,       FALSE   },
    {   "circle_of_fire",  	AFF_COFIRE,		TRUE	},
    {   "torture",  	  	AFF_TORTURE,		FALSE	},
    {   "age",  	  	AFF_AGE,		FALSE	},
    {   "aura_of_anti_magic",	AFF_AURA_ANTI_MAGIC,    TRUE    },
    {   "holy_protection", 	AFF_HOLY_PROTECTION, 	TRUE    },
    {   "improved_hide", 	AFF_IMPROVED_HIDE,	TRUE    },
    {	"stealth",		AFF_STEALTH,		FALSE   },
    {   "bloodshield",		AFF_BLOODSHIELD,	TRUE    },
    {   "pestilence",		AFF_PESTILENCE,		TRUE    },
    {   "bend_light",		AFF_BEND_LIGHT,		FALSE   },
    {   "cloud_of_healing",	AFF_CLOUD_OF_HEALING,	TRUE    },
    {   "satanic_inferno",      AFF_SATANIC_INFERNO,	TRUE	},
    {   "mana_shield", 		AFF_MANA_SHIELD, 	FALSE   },
    {   "war_chant",		AFF_WAR_CHANT,		FALSE   },
    {   "nagaroms_curse",	AFF_NAGAROMS_CURSE,	FALSE	},
    {	"prayer",		AFF_PRAYER,		FALSE	},
    {	"randomshield",		AFF_RANDOMSHIELD,	TRUE	},
    {	"primalscream",		AFF_PRIMALSCREAM,	FALSE	},
    {   "acidshield",           AFF_ACIDSHIELD,         TRUE    },
    {	"demonshield",		AFF_DEMONSHIELD,	TRUE	},
    {   "",                     0,                      0       }
};

const struct flag_type affect4_flags [] =
{
    {   "immortal",		AFF_IMMORTAL,		TRUE	},
    {	"no_summon",		AFF_NO_SUMMON,		TRUE    },
    {   "golden_sanctuary", 	AFF_GOLDEN_SANCTUARY,   TRUE    },
    {   "mountable",		AFF_MOUNTABLE,		TRUE	},
    {   "biofeedback", 		AFF_BIOFEEDBACK,	TRUE	},
    {	"earthshield",		AFF_EARTHSHIELD,	TRUE	},
    {	"leaf_shield",		AFF_LEAF_SHIELD,	TRUE	},
    {   "luck_shield",		AFF_LUCK_SHIELD,	TRUE	},
    {	"tongues",		AFF_TONGUES,		TRUE	},
    {	"liquid_skin",		AFF_LIQUID_SKIN,	TRUE	},
    {   "angelic_aura",		AFF_ANGELIC_AURA,	TRUE	},
    {	"ethereal_wolf",	AFF_ETHEREAL_WOLF,	TRUE	},
    {	"deception",		AFF_DECEPTION,		TRUE	},
    {	"ethereal_snake",	AFF_ETHEREAL_SNAKE,	TRUE	},
    {   "tomba_di_vemon",       AFF_BURROW,          	TRUE    },
    {   "thieves_cant",         AFF_THIEVESCANT,        TRUE    },
    {   "newbie_slayer",        AFF_NEWBIE_SLAYER,      TRUE    },
    {	"force_of_nature",	AFF_FORCE_OF_NATURE,	TRUE	},
    {   "essence_of_gaia",	AFF_ESSENCE_OF_GAIA,	TRUE	},
    {   "",                     0,                      0       }
};

const struct flag_type affect_powers_flags [] =
{
    {	"",			0,			0	}
};

const struct flag_type affect_weaknesses_flags [] =
{
    {	"",			0,			0	}
};


/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
    {	"none",			APPLY_NONE,		TRUE	},
    {	"strength",		APPLY_STR,		TRUE	},
    {	"dexterity",		APPLY_DEX,		TRUE	},
    {	"intelligence",		APPLY_INT,		TRUE	},
    {	"wisdom",		APPLY_WIS,		TRUE	},
    {	"constitution",		APPLY_CON,		TRUE	},
    {	"sex",			APPLY_SEX,		TRUE	},
    {	"class",		APPLY_CLASS,		TRUE	},
    {	"level",		APPLY_LEVEL,		TRUE	},
    {	"age",			APPLY_AGE,		TRUE	},
    {	"height",		APPLY_HEIGHT,		TRUE	},
    {	"weight",		APPLY_WEIGHT,		TRUE	},
    {	"mana",			APPLY_MANA,		TRUE	},
    {   "blood",                APPLY_BP,               TRUE    },
    {   "anti_disarm",          APPLY_ANTI_DIS,         TRUE    },
    {	"hp",			APPLY_HIT,		TRUE	},
    {	"move",			APPLY_MOVE,		TRUE	},
    {	"gold",			APPLY_GOLD,		TRUE	},
    {	"experience",		APPLY_EXP,		TRUE	},
    {	"ac",			APPLY_AC,		TRUE	},
    {	"hitroll",		APPLY_HITROLL,		TRUE	},
    {	"damroll",		APPLY_DAMROLL,		TRUE	},
    {	"saving_para",		APPLY_SAVING_PARA,	TRUE	},
    {	"saving_rod",		APPLY_SAVING_ROD,	TRUE	},
    {	"saving_petri",		APPLY_SAVING_PETRI,	TRUE	},
    {	"saving_breath",	APPLY_SAVING_BREATH,	TRUE	},
    {	"saving_spell",		APPLY_SAVING_SPELL,	TRUE	},
    {   "resist_acid",		APPLY_DAM_ACID,		TRUE	},
    {   "resist_holy",		APPLY_DAM_HOLY,		TRUE	},
    {   "resist_magic",		APPLY_DAM_MAGIC,	TRUE	},
    {   "resist_fire",		APPLY_DAM_FIRE,		TRUE	},
    {   "resist_energy",	APPLY_DAM_ENERGY,	TRUE	},
    {   "resist_wind",		APPLY_DAM_WIND,		TRUE	},
    {   "resist_water",		APPLY_DAM_WATER,	TRUE	},
    {   "resist_illusion",	APPLY_DAM_ILLUSION,	TRUE	},
    {   "resist_dispel",	APPLY_DAM_DISPEL,	TRUE	},
    {   "resist_earth",		APPLY_DAM_EARTH,	TRUE	},
    {   "resist_psychic",	APPLY_DAM_PSYCHIC,	TRUE	},
    {   "resist_poison",	APPLY_DAM_POISON,	TRUE	},
    {   "resist_breath",	APPLY_DAM_BREATH,	TRUE	},
    {   "resist_summon",	APPLY_DAM_SUMMON,	TRUE	},
    {   "resist_physical",	APPLY_DAM_PHYSICAL,	TRUE	},
    {   "resist_explosive",	APPLY_DAM_EXPLOSIVE,	TRUE	},
    {   "resist_song",		APPLY_DAM_SONG,		TRUE	},
    {   "resist_nagarom",	APPLY_DAM_NAGAROM,	TRUE	},
    {   "resist_unholy",	APPLY_DAM_UNHOLY,	TRUE	},
    {   "resist_clan",		APPLY_DAM_CLAN,		TRUE	},
/*
 * Spell affect bits are listed alphabetically.
 * Please keep them that way for easy look-up.
 * - Ahsile
 */

	{	"age",				APPLY_AGE_SPELL,		TRUE	},
	{	"angelic_aura",		APPLY_ANGELIC_AURA,		TRUE	},
	{	"aura_anti_magic",	APPLY_AURA_ANTI_MAGIC,	TRUE	},
	{	"bend_light",		APPLY_BEND_LIGHT,		TRUE	},
	{	"biofeedback",		APPLY_BIOFEEDBACK,		TRUE	},
	{	"blade_barrier",	APPLY_BLADE,	TRUE	},
	{	"bless",			APPLY_BLESS,			TRUE	},
	{	"blind",			APPLY_BLIND,			TRUE	},
	{	"bloodshield",		APPLY_BLOODSHIELD,		TRUE	},
	{	"change_sex",		APPLY_CHANGE_SEX,		TRUE	},
	{	"chaos_field",		APPLY_CHAOS,			TRUE	},
	{	"cloaking",			APPLY_CLOAKING,			TRUE	},
	{	"cloud_of_healing",	APPLY_CLOUD_OF_HEALING,	TRUE	},
	{	"cofire",			APPLY_COFIRE,			TRUE	},
	{	"combat_mind",		APPLY_COMBAT_MIND,      TRUE    },
	{	"confusion",		APPLY_CONFUSED,			TRUE	},
	{	"curse",			APPLY_CURSE,			TRUE	},
	{	"curse_of_nature",	APPLY_CURSE_NATURE,		TRUE	},
	{	"dancing_lights",	APPLY_DANCING,			TRUE	},
	{	"detect_evil",		APPLY_DETECT_EVIL,		TRUE	},	
	{	"detect_hide",	 	APPLY_DETECT_HIDDEN,	TRUE	},	
	{	"detect_invis",		APPLY_DETECT_INVIS,		TRUE	},	
	{	"detect_magic",		APPLY_DETECT_MAGIC,		TRUE	},
	{	"earthshield",		APPLY_EARTHSHIELD,		TRUE	},
	{	"essence_of_gaia",	APPLY_ESSENCE_OF_GAIA,	TRUE	},
	{	"ethereal_snake",	APPLY_ETHEREAL_SNAKE,	TRUE	},
	{	"ethereal_wolf",	APPLY_ETHEREAL_WOLF,	TRUE	},
	{	"faerie_fire",		APPLY_FAERIE_FIRE,		TRUE	},
	{	"fireshield",		APPLY_FIRESHIELD,		TRUE	},
	{	"fly",   			APPLY_FLYING,			TRUE	},
	{	"force_of_nature",	APPLY_FORCE_OF_NATURE,	TRUE	},
	{	"forestwalk",		APPLY_FORESTWALK,		TRUE	},	
	{	"fumble",			APPLY_FUMBLE,			TRUE	},
	{	"ghost_shield",		APPLY_GHOST_SHIELD,		TRUE	},
	{	"giant_str",		APPLY_GIANT_STRENGTH,	TRUE	},
/*
	{	"golden_armor",		APPLY_GOLDEN_ARMOR,		TRUE	},
	{	"golden_sanc",		APPLY_GOLDEN_SANCTUARY,	TRUE	},
*/
	{	"haste",			APPLY_HASTE,			TRUE	},
	{	"heighten_senses",	APPLY_HEIGHTEN_SENSES,  TRUE    },					
	{	"hide",	        	APPLY_HIDE,     		TRUE	},
	{	"improved_hide",	APPLY_IMPROVED_HIDE,	TRUE	},
	{	"improved_invis",	APPLY_IMPROVED_INVIS,	TRUE	},
	{	"intertial_barrier",APPLY_INERTIAL,		TRUE	},
	{	"incinerate",		APPLY_FLAMING,			TRUE	},
	{	"infrared",			APPLY_INFRARED,			TRUE	},
	{	"invis",			APPLY_INVISIBLE,		TRUE	},
	{	"lead_shield",		APPLY_LEAF_SHIELD,		TRUE	},
	{	"liquid_skin",		APPLY_LIQUID_SKIN,		TRUE	},
	{	"luck_shield",		APPLY_LUCK_SHIELD,		TRUE	},
	{	"malignify",		APPLY_MALIGNIFY,		TRUE	},
	{	"mist",				APPLY_MIST,				TRUE	},
	{	"mountainwalk",		APPLY_MOUNTAINWALK,		TRUE	},
	{	"nagaroms_curse",	APPLY_NAGAROMS_CURSE,	TRUE	},
	{	"occulutus_visum",	APPLY_OCCULUTUS_VISUM,  TRUE    }, 					
	{	"pass_door",		APPLY_PASS_DOOR,		TRUE	},
	{	"peace",			APPLY_PEACE,	TRUE	},
	{	"pestilence",		APPLY_PESTILENCE,		TRUE	},
	{	"plainswalk",		APPLY_PLAINSWALK,		TRUE	},
	{	"power_leak",		APPLY_POWER_LEAK,		TRUE	},
	{	"prayer",			APPLY_PRAYER,			TRUE	},
	{	"poison",			APPLY_POISON,			TRUE	},
	{	"protect_evil",		APPLY_PROTECT,			TRUE	},
	{	"protect_good",		APPLY_PROTECTION_GOOD,	TRUE	},
	{	"quickness",		APPLY_QUICKNESS,		TRUE	},
	{	"randomshield",		APPLY_RANDOMSHIELD,		TRUE	},
	{	"sanctuary",		APPLY_SANCTUARY,		TRUE	},
	{	"satanic_inferno",	APPLY_SATANIC_INFERNO,	TRUE	},
	{	"scry",	        	APPLY_SCRY,				TRUE	},
	{	"shadow_image",		APPLY_SHADOW_IMAGE,		TRUE	},
	{	"shockshield",		APPLY_SHOCKSHIELD,		TRUE	},
	{	"slit",				APPLY_SLIT,				TRUE	},
	{	"sneak",			APPLY_SNEAK,			TRUE	},
	{	"stealth",			APPLY_STEALTH,			TRUE	},
	{	"swampwalk",		APPLY_SWAMPWALK,		TRUE	},
	{	"tale_of_terror",	APPLY_TALE_OF_TERROR,	TRUE	},
	{	"titan_strength", 	APPLY_TITAN_STRENGTH,	TRUE	},		
	{	"tongues",			APPLY_TONGUES,			TRUE	},
	{	"tortured_soul",	APPLY_TORTURE,			TRUE	},
	{	"true_sight",		APPLY_TRUESIGHT,		TRUE	},
	{	"web",				APPLY_ANTI_FLEE,		TRUE	},
	{	"unholy_strength",	APPLY_UNHOLY_STRENGTH,	TRUE,	},
    {	"",			0,			0	}
};


const struct flag_type quality_flags[] =
{
	{ "destroyed",	SKIN_DESTROYED,		TRUE },
	{ "horrible",	SKIN_HORRIBLE,		TRUE },
	{ "lousy",		SKIN_LOUSY,			TRUE },
	{ "passable",	SKIN_PASSABLE,		TRUE },
	{ "unexceptional", SKIN_UNEXCEPTIONAL,	TRUE },
	{ "decent",		SKIN_DECENT,		TRUE },
	{ "good",		SKIN_GOOD,			TRUE },
	{ "excellent",	SKIN_EXCELLENT,		TRUE },
	{ "superior",	SKIN_SUPERIOR,		TRUE },
	{ "perfect",	SKIN_PERFECT,		TRUE },
	{ "", 0,	0	}
};
/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] =
{
    {	"in the inventory",	WEAR_NONE,	TRUE	},
    {	"as a light",		WEAR_LIGHT,	TRUE	},
    {	"on the left finger",	WEAR_FINGER_L,	TRUE	},
    {	"on the right finger",	WEAR_FINGER_R,	TRUE	},
    {	"around the neck (1)",	WEAR_NECK_1,	TRUE	},
    {	"around the neck (2)",	WEAR_NECK_2,	TRUE	},
    {	"on the body",		WEAR_BODY,	TRUE	},
    {	"over the head",	WEAR_HEAD,	TRUE	},
    {   "in the eyes",          WEAR_IN_EYES,   TRUE    },
    {   "on the face",          WEAR_ON_FACE,   TRUE    },
    {   "spinning around(1)",   WEAR_ORBIT,     TRUE    },
    {   "spinning around(2)",   WEAR_ORBIT_2,   TRUE    },
    {	"on the legs",		WEAR_LEGS,	TRUE	},
    {	"on the feet",		WEAR_FEET,	TRUE	},
    {	"on the hands",		WEAR_HANDS,	TRUE	},
    {	"on the arms",		WEAR_ARMS,	TRUE	},
    {	"as a shield",		WEAR_SHIELD,	TRUE	},
    {	"about the shoulders",	WEAR_ABOUT,	TRUE	},
    {	"around the waist",	WEAR_WAIST,	TRUE	},
    {	"on the left wrist",	WEAR_WRIST_L,	TRUE	},
    {	"on the right wrist",	WEAR_WRIST_R,	TRUE	},
    {	"wielded",		WEAR_WIELD,	TRUE	},
    {   "dual wielded",         WEAR_WIELD,     TRUE    },
    {   "holstered",		WEAR_FIREARM,   TRUE    },
    {	"held in the hands",	WEAR_HOLD,	TRUE	},
    {   "on the left ankle",    WEAR_ANKLE_L,   TRUE    },
    {   "on the right ankle",   WEAR_ANKLE_R,   TRUE    },
    {   "in the ears",          WEAR_EARS,      TRUE    },
    {   "implanted",		WEAR_IMPLANTED1,TRUE	},
    {   "implanted",		WEAR_IMPLANTED2,TRUE	},
    {   "implanted",		WEAR_IMPLANTED3,TRUE	},
    {	"",			0			}
};


/*
 * What is typed.
 * Neck2 should not be settable for loaded mobiles.
 */
const struct flag_type wear_loc_flags[] =
{
    {	"none",		WEAR_NONE,	TRUE	},
    {	"light",	WEAR_LIGHT,	TRUE	},
    {	"lfinger",	WEAR_FINGER_L,	TRUE	},
    {	"rfinger",	WEAR_FINGER_R,	TRUE	},
    {	"neck1",	WEAR_NECK_1,	TRUE	},
    {	"neck2",	WEAR_NECK_2,	TRUE	},
    {	"body",		WEAR_BODY,	TRUE	},
    {	"head",		WEAR_HEAD,	TRUE	},
    {   "lense",        WEAR_IN_EYES,   TRUE    },
    {   "mask",         WEAR_ON_FACE,   TRUE    },
    {   "orbit1",       WEAR_ORBIT,     TRUE    },
    {   "orbit2",       WEAR_ORBIT_2,   TRUE    },
    {	"legs",		WEAR_LEGS,	TRUE	},
    {	"feet",		WEAR_FEET,	TRUE	},
    {	"hands",	WEAR_HANDS,	TRUE	},
    {	"arms",		WEAR_ARMS,	TRUE	},
    {	"shield",	WEAR_SHIELD,	TRUE	},
    {	"about",	WEAR_ABOUT,	TRUE	},
    {	"waist",	WEAR_WAIST,	TRUE	},
    {	"lwrist",	WEAR_WRIST_L,	TRUE	},
    {	"rwrist",	WEAR_WRIST_R,	TRUE	},
    {	"wielded",	WEAR_WIELD,	TRUE	},
    {   "dual",         WEAR_WIELD_2,   TRUE    },
    {	"hold",		WEAR_HOLD,	TRUE	},
    {   "holster",      WEAR_FIREARM,   TRUE	},
    {   "lankle",       WEAR_ANKLE_L,   TRUE    },
    {   "rankle",       WEAR_ANKLE_R,   TRUE    },
    {   "ears",         WEAR_EARS,      TRUE    },
    {   "implanted1",	WEAR_IMPLANTED1,TRUE	},
    {   "implanted2",	WEAR_IMPLANTED2,TRUE	},
    {   "implanted3",   WEAR_IMPLANTED3,TRUE	},
    {	"",		0,		0	}
};



const struct flag_type weapon_flags[] =
{
    {	"hit",		0,	TRUE	},
    {	"slice",	1,	TRUE	},
    {	"stab",		2,	TRUE	},
    {	"slash",	3,	TRUE	},
    {	"whip",		4,	TRUE	},
    {	"claw",		5,	TRUE	},
    {	"blast",	6,	TRUE	},
    {	"pound",	7,	TRUE	},
    {	"crush",	8,	TRUE	},
    {	"grep",		9,	TRUE	},
    {	"bite",		10,	TRUE	},
    {	"pierce",	11,	TRUE	},
    {	"suction",	12,	TRUE	},
    {	"chop",		13,	TRUE	},
    {   "blast",	14,	TRUE	},
    {	"shot",		15, 	TRUE	},
    {	"",		0,	0	}
};

const struct flag_type ammo_flags[] =
{
    { 	"none",		AMMO_NONE,	TRUE	},
    { 	"arrow",	AMMO_ARROW,	TRUE	},
    { 	"bolt",		AMMO_BOLT,	TRUE	},
    { 	"bullet",	AMMO_BULLET,	TRUE	},
    {	"",		0,		0	}
};

const struct flag_type damage_flags[] =
{         
    { "none",		DAMCLASS_NULL,          TRUE    },           
    { "acid",           DAMCLASS_ACID,          TRUE    },           
    { "holy",           DAMCLASS_HOLY,          TRUE    },           
    { "magic",          DAMCLASS_MAGIC,         TRUE    },          
    { "fire",           DAMCLASS_FIRE,          TRUE    },           
    { "energy",         DAMCLASS_ENERGY,        TRUE    },         
    { "lightning",      DAMCLASS_WIND,          TRUE    },           
    { "water",          DAMCLASS_WATER,         TRUE    },          
    { "illusion",       DAMCLASS_ILLUSION,      TRUE    },       
    { "dispel",         DAMCLASS_DISPEL,        TRUE    },         
    { "earth",          DAMCLASS_EARTH,         TRUE    },          
    { "psychic",        DAMCLASS_PSYCHIC,       TRUE    },        
    { "poison",         DAMCLASS_POISON,        TRUE    },         
    { "breath",         DAMCLASS_BREATH,        TRUE	},         
    { "summon",         DAMCLASS_SUMMON,        TRUE	},         
    { "slash",          DAMCLASS_PHYSICAL,      TRUE    },       
    { "explosive",      DAMCLASS_EXPLOSIVE,     TRUE    },      
    { "song",           DAMCLASS_SONG,          TRUE	},           
    { "nagarom",        DAMCLASS_NAGAROM,       TRUE	},
    { "unholy",         DAMCLASS_UNHOLY,        TRUE    },
    { "clan",           DAMCLASS_CLAN,          TRUE	},
    { "",		0,			0 	}
};

const struct flag_type container_flags[] =
{
    {	"closeable",		1,		TRUE	},
    {	"pickproof",		2,		TRUE	},
    {	"closed",		4,		TRUE	},
    {	"locked",		8,		TRUE	},
    {	"",			0,		0	}
};



const struct flag_type liquid_flags[] =
{
    {	"water",		0,	TRUE	},
    {	"beer",			1,	TRUE	},
    {	"wine",			2,	TRUE	},
    {	"ale",			3,	TRUE	},
    {	"dark-ale",		4,	TRUE	},
    {	"whisky",		5,	TRUE	},
    {	"lemonade",		6,	TRUE	},
    {	"firebreather",		7,	TRUE	},
    {	"local-specialty",	8,	TRUE	},
    {	"slime-mold-juice",	9,	TRUE	},
    {	"milk",			10,	TRUE	},
    {	"tea",			11,	TRUE	},
    {	"coffee",		12,	TRUE	},
    {	"blood",		13,	TRUE	},
    {	"salt-water",		14,	TRUE	},
    {	"cola",			15,	TRUE	},
    {	"",			0,	0	}
};

const struct flag_type immune_flags[] =
{
    {   "summon",    IMM_SUMMON,    TRUE   },
    {   "charm",     IMM_CHARM,     TRUE   },
    {   "magic",     IMM_MAGIC,     TRUE   },
    {   "weapon",    IMM_WEAPON,    TRUE   },
    {   "bash",      IMM_BASH,      TRUE   },
    {   "pierce",    IMM_PIERCE,    TRUE   },
    {   "slash",     IMM_SLASH,     TRUE   },
    {   "fire",      IMM_FIRE,      TRUE   },
    {   "cold",      IMM_COLD,      TRUE   },
    {   "lightning", IMM_LIGHTNING, TRUE   },
    {   "acid",      IMM_ACID,      TRUE   },
    {   "poison",    IMM_POISON,    TRUE   },
    {   "negative",  IMM_NEGATIVE,  TRUE   },
    {   "holy",      IMM_HOLY,      TRUE   },
    {   "energy",    IMM_ENERGY,    TRUE   },
    {   "mental",    IMM_MENTAL,    TRUE   },
    {   "disease",   IMM_DISEASE,   TRUE   },
    {   "drowning",  IMM_DROWNING,  TRUE   },
    {   "light",     IMM_LIGHT,     TRUE   },
    {   "",          0,             0      }
};

const struct flag_type mprog_types[] =
{
    {   "error_prog",     ERROR_PROG,     FALSE  },
    {   "in_file_prog",   IN_FILE_PROG,   FALSE  },
    {   "act_prog",       ACT_PROG,       TRUE   },
    {   "speech_prog",    SPEECH_PROG,    TRUE   },
    {   "rand_prog",      RAND_PROG,      TRUE   },
    {   "fight_prog",     FIGHT_PROG,     TRUE   },
    {   "death_prog",     DEATH_PROG,     TRUE   },
    {   "hitprcnt_prog",  HITPRCNT_PROG,  TRUE   },
    {   "entry_prog",     ENTRY_PROG,     TRUE   },
    {   "greet_prog",     GREET_PROG,     TRUE   },
    {   "all_greet_prog", ALL_GREET_PROG, TRUE   },
    {   "give_prog",      GIVE_PROG,      TRUE   },
    {   "bribe_prog",     BRIBE_PROG,     TRUE   },
    {   "",               0,              0,     }
};

const struct flag_type oprog_types[] =
{
    {   "error_prog",     OBJ_TRAP_ERROR,     FALSE  },
    {   "get_prog",       OBJ_TRAP_GET,       TRUE   },
    {   "get_from_prog",  OBJ_TRAP_GET_FROM,  TRUE   },
    {   "drop_prog",      OBJ_TRAP_DROP,      TRUE   },
    {   "put_prog",       OBJ_TRAP_PUT,       TRUE   },
    {   "give_prog",      OBJ_TRAP_GIVE,      TRUE   },
    {   "fill_prog",      OBJ_TRAP_FILL,      TRUE   },
    {   "wear_prog",      OBJ_TRAP_WEAR,      TRUE   },
    {   "look_prog",      OBJ_TRAP_LOOK,      TRUE   },
    {   "look_in_prog",   OBJ_TRAP_LOOK_IN,   TRUE   },
    {   "invoke_prog",    OBJ_TRAP_INVOKE,    TRUE   },
    {   "use_prog",       OBJ_TRAP_USE,       TRUE   },
    {   "cast_prog",      OBJ_TRAP_CAST,      TRUE   },
    {   "cast_sn_prog",   OBJ_TRAP_CAST_SN,   TRUE   },
    {   "join_prog",      OBJ_TRAP_JOIN,      TRUE   },
    {   "separate_prog",  OBJ_TRAP_SEPARATE,  TRUE   },
    {   "buy_prog",       OBJ_TRAP_BUY,       TRUE   },
    {   "sell_prog",      OBJ_TRAP_SELL,      TRUE   },
    {   "store_prog",     OBJ_TRAP_STORE,     TRUE   },
    {   "retrieve_prog",  OBJ_TRAP_RETRIEVE,  TRUE   },
    {   "open_prog",      OBJ_TRAP_OPEN,      TRUE   },
    {   "close_prog",     OBJ_TRAP_CLOSE,     TRUE   },
    {   "lock_prog",      OBJ_TRAP_LOCK,      TRUE   },
    {   "unlock_prog",    OBJ_TRAP_UNLOCK,    TRUE   },
    {   "pick_prog",      OBJ_TRAP_PICK,      TRUE   },
    {   "throw_prog",     OBJ_TRAP_THROW,     TRUE   },
    {   "rand_prog",      OBJ_TRAP_RANDOM,    TRUE   },
    {   "",               0,                  0      }
};

const struct flag_type rprog_types[] =
{
    {   "error_prog",     ROOM_TRAP_ERROR,    FALSE  },
    {   "enter_prog",     ROOM_TRAP_ENTER,    TRUE   },
    {   "exit_prog",      ROOM_TRAP_EXIT,     TRUE   },
    {   "pass_prog",      ROOM_TRAP_PASS,     TRUE   },
    {   "cast_prog",      ROOM_TRAP_CAST,     TRUE   },
    {   "cast_sn_prog",   ROOM_TRAP_CAST_SN,  TRUE   },
    {   "sleep_prog",     ROOM_TRAP_SLEEP,    TRUE   },
    {   "wake_prog",      ROOM_TRAP_WAKE,     TRUE   },
    {   "rest_prog",      ROOM_TRAP_REST,     TRUE   },
    {   "death_prog",     ROOM_TRAP_DEATH,    TRUE   },
    {   "time_prog",      ROOM_TRAP_TIME,     TRUE   },
    {   "rand_prog",      ROOM_TRAP_RANDOM,   TRUE   },
    {   "",               0,                  0      }
};

const struct flag_type eprog_types[] =
{
    {   "error_prog",     EXIT_TRAP_ERROR,    FALSE  },
    {   "enter_prog",     EXIT_TRAP_ENTER,    TRUE   },
    {   "exit_prog",      EXIT_TRAP_EXIT,     TRUE   },
    {   "pass_prog",      EXIT_TRAP_PASS,     TRUE   },
    {   "look_prog",      EXIT_TRAP_LOOK,     TRUE   },
    {   "scry_prog",      EXIT_TRAP_SCRY,     TRUE   },
    {   "open_prog",      EXIT_TRAP_OPEN,     TRUE   },
    {   "close_prog",     EXIT_TRAP_CLOSE,    TRUE   },
    {   "lock_prog",      EXIT_TRAP_LOCK,     TRUE   },
    {   "unlock_prog",    EXIT_TRAP_UNLOCK,   TRUE   },
    {   "pick_prog",      EXIT_TRAP_PICK,     TRUE   },
    {   "",               0,                  0      }
};
