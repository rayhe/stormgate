/***************************************************************************
 *  File: olc_act.c                                                        *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

/*$Id: olc_act.c,v 1.32 2005/04/11 03:25:01 tyrion Exp $*/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>	/* OLC 1.1b */
#include "merc.h"
#include "olc.h"

/*
 * External functions.
 */
char *mprog_type_to_name   args( ( int type ) );
HELP_DATA *get_help        args( ( char *argument ) );
extern void clan_sort      args ( ( CLAN_DATA *pClan ) );
extern void religion_sort  args ( ( RELIGION_DATA *pReligion ) );


struct olc_help_type
{
    char *command;
    const void *structure;
    char *desc;
};



bool show_version( CHAR_DATA *ch, char *argument )
{
    send_to_char(C_DEFAULT, VERSION, ch );
    send_to_char(C_DEFAULT, "\n\r", ch );
    send_to_char(C_DEFAULT, AUTHOR, ch );
    send_to_char(C_DEFAULT, "\n\r", ch );
    send_to_char(C_DEFAULT, DATE, ch );
    send_to_char(C_DEFAULT, "\n\r", ch );
    send_to_char(C_DEFAULT, CREDITS, ch );
    send_to_char(C_DEFAULT, "\n\r", ch );

    return FALSE;
}    

/*
 * This table contains help commands and a brief description of each.
 * ------------------------------------------------------------------
 */
const struct olc_help_type help_table[] =
{
    {	"area",		area_flags,	 "Area attributes."		},
    {	"room",		room_flags,	 "Room attributes."		},
    {	"sector",	sector_flags,	 "Sector types, terrain."	},
    {	"exit",		exit_flags,	 "Exit types."			},
    {	"type",		type_flags,	 "Types of objects."		},
    {	"extra",	extra_flags,	 "Object attributes."		},
    {	"extra2",	extra_flags2,	 "Object attributes."		},
    {   "extra3",       extra_flags3,    "Object attributes."           },
    {   "extra4",       extra_flags4,    "Object attributes."           },
    {	"wear",		wear_flags,	 "Where to wear object."	},
    {	"spec",		spec_table,	 "Available special programs." 	},
    {   "game",         game_table,      "Available gambling programs." },
    {	"sex",		sex_flags,	 "Sexes."			},
    {	"act",		act_flags,	 "Mobile attributes."		},
    {	"affect",	affect_flags,	 "Mobile affects."		},
    {	"affect2",	affect2_flags,	 "Mobile affects2."		},
    {   "affect3",	affect3_flags,	 "Mobile affects3."		},
    {   "affect4",	affect4_flags,	 "Mobile affects4."		},
    {	"wear-loc",	wear_loc_flags,	 "Where mobile wears object."	},
    {   "applies",      apply_flags,     "Applies you can put on objs." },
    {	"spells",	skill_table,	 "Names of current spells." 	},
    {	"weapon",	weapon_flags,	 "Type of weapon." 		},
    {	"container",	container_flags, "Container status."		},
    {	"liquid",	liquid_flags,	 "Types of liquids."		},
    {   "immune",       immune_flags,    "Types of immunities."         },
    {   "mprogs",       mprog_types,     "Types of MobProgs."           },
    {   "oprogs",       oprog_types,     "Types of ObjProgs."           },
    {   "rprogs",       rprog_types,     "Types of RoomProgs."          },
    {   "eprogs",       eprog_types,     "Types of ExitProgs."          },
    {   "damclass",	damage_flags,	 "Damage Classes"		},
    {	"",		0,		 ""				}
};



/*****************************************************************************
 Name:		show_flag_cmds
 Purpose:	Displays settable flags and stats.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds( CHAR_DATA *ch, const struct flag_type *flag_table )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  flag;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (flag = 0; *flag_table[flag].name; flag++)
    {
	if ( flag_table[flag].settable )
	{
	    sprintf( buf, "%-19.18s", flag_table[flag].name );
	    strcat( buf1, buf );
	    if ( ++col % 4 == 0 )
		strcat( buf1, "\n\r" );
	}
    }
 
    if ( col % 4 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char(C_DEFAULT, buf1, ch );
    return;
}



/*****************************************************************************
 Name:		show_skill_cmds
 Purpose:	Displays all skill functions.
 		Does remove those damn immortal commands from the list.
 		Could be improved by:
 		(1) Adding a check for a particular class.
 		(2) Adding a check for a level range.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds( CHAR_DATA *ch, int tar )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH*2 ];
    int  sn;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (sn = 0; skill_table[sn].name[0] != '\0'; sn++)
    {
	if ( !skill_table[sn].name )
	    break;

	if ( !str_cmp( skill_table[sn].name, "reserved" )
	  || skill_table[sn].spell_fun == spell_null )
	    continue;

	if ( tar == -1 || skill_table[sn].target == tar )
	{
	    sprintf( buf, "%-19.18s", skill_table[sn].name );
	    strcat( buf1, buf );
	    if ( ++col % 4 == 0 )
		strcat( buf1, "\n\r" );
	}
    }
 
    if ( col % 4 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char(C_DEFAULT, buf1, ch );
    return;
}



/*****************************************************************************
 Name:		show_spec_cmds
 Purpose:	Displays settable special functions.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_spec_cmds( CHAR_DATA *ch )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  spec;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    send_to_char(C_DEFAULT, "Preceed special functions with 'spec_'\n\r\n\r", ch );
    for (spec = 0; *spec_table[spec].spec_fun != NULL; spec++)
    {
	sprintf( buf, "%-19.18s", &spec_table[spec].spec_name[5] );
	strcat( buf1, buf );
	if ( ++col % 4 == 0 )
	    strcat( buf1, "\n\r" );
    }
 
    if ( col % 4 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char(C_DEFAULT, buf1, ch );
    return;
}


void show_game_cmds( CHAR_DATA *ch )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  game;
    int  col;

    buf1[0] = '\0';
    col = 0;
    send_to_char(AT_WHITE, "Preceed game functions with 'game_'\n\r\n\r", ch );
    for (game = 0; *game_table[game].game_fun != NULL; game++)
    {
        sprintf( buf, "%-19.18s", &game_table[game].game_name[5] );
        strcat( buf1, buf );
        if ( ++col % 4 == 0 )
            strcat( buf1, "\n\r" );
    }

    if ( col % 4 != 0 )
        strcat( buf1, "\n\r" );

    send_to_char(AT_WHITE, buf1, ch );
    return;
}


/*****************************************************************************
 Name:		show_help
 Purpose:	Displays help for many tables used in OLC.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_help( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char spell[MAX_INPUT_LENGTH];
    int cnt;

    argument = one_argument( argument, arg );
    one_argument( argument, spell );

    /*
     * Display syntax.
     */
    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  ? [command]\n\r\n\r", ch );
	send_to_char(C_DEFAULT, "[command]  [description]\n\r", ch );
	for (cnt = 0; help_table[cnt].command[0] != '\0'; cnt++)
	{
	    sprintf( buf, "%-10.10s -%s\n\r",
	        capitalize( help_table[cnt].command ),
		help_table[cnt].desc );
	    send_to_char(C_DEFAULT, buf, ch );
	}
	return FALSE;
    }

    /*
     * Find the command, show changeable data.
     * ---------------------------------------
     */
    for (cnt = 0; *help_table[cnt].command; cnt++)
    {
        if (  arg[0] == help_table[cnt].command[0]
          && !str_prefix( arg, help_table[cnt].command ) )
	{
	    if ( help_table[cnt].structure == spec_table )
	    {
		show_spec_cmds( ch );
		return FALSE;
	    }
            else if ( help_table[cnt].structure == game_table )
            {
                show_game_cmds( ch );
                return FALSE;
            }
	    else
	    if ( help_table[cnt].structure == skill_table )
	    {

		if ( spell[0] == '\0' )
		{
		    send_to_char(C_DEFAULT, "Syntax:  ? spells "
		        "[ignore/attack/defend/self/object/all]\n\r", ch );
		    return FALSE;
		}

		if ( !str_prefix( spell, "all" ) )
		    show_skill_cmds( ch, -1 );
		else if ( !str_prefix( spell, "ignore" ) )
		    show_skill_cmds( ch, TAR_IGNORE );
		else if ( !str_prefix( spell, "attack" ) )
		    show_skill_cmds( ch, TAR_CHAR_OFFENSIVE );
		else if ( !str_prefix( spell, "defend" ) )
		    show_skill_cmds( ch, TAR_CHAR_DEFENSIVE );
		else if ( !str_prefix( spell, "self" ) )
		    show_skill_cmds( ch, TAR_CHAR_SELF );
		else if ( !str_prefix( spell, "object" ) )
		    show_skill_cmds( ch, TAR_OBJ_INV );
		else
		    send_to_char(C_DEFAULT, "Syntax:  ? spell "
		        "[ignore/attack/defend/self/object/all]\n\r", ch );
		    
		return FALSE;
	    }
	    else
	    {
		show_flag_cmds( ch, help_table[cnt].structure );
		return FALSE;
	    }
	}
    }

    show_help( ch, "" );
    return FALSE;
}



bool redit_mlist( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA	*pMobIndex;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH*25 ];
    char		buf1 [ MAX_STRING_LENGTH*25 ];
    char		arg  [ MAX_INPUT_LENGTH    ];
    bool fAll, found;
    int vnum;
    int  col = 0;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  mlist <all/name>\n\r", ch );
	return FALSE;
    }

    pArea = ch->in_room->area;
    buf1[0] = '\0';
    fAll    = !str_cmp( arg, "all" );
    found   = FALSE;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) )
	{
	    if ( fAll || is_name( arg, pMobIndex->player_name ) )
	    {
		found = TRUE;
		sprintf( buf, "&b[&C%5d&b]&w %-17.16s",
		    pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
		strcat( buf1, buf );
		if ( ++col % 3 == 0 )
		    strcat( buf1, "\n\r" );
	    }
	}
    }

    if ( !found )
    {
	send_to_char(C_DEFAULT, "Mobile(s) not found in this area.\n\r", ch);
	return FALSE;
    }

    if ( col % 3 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char(C_DEFAULT, buf1, ch );
    return FALSE;
}



bool redit_olist( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA	*pObjIndex;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH*25 ];
    char		buf1 [ MAX_STRING_LENGTH*25 ];
    char		arg  [ MAX_INPUT_LENGTH    ];
    bool fAll, found;
    int vnum;
    int  col = 0;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  olist <all/name/item_type>\n\r", ch );
	return FALSE;
    }

    pArea = ch->in_room->area;
    buf1[0] = '\0';
    fAll    = !str_cmp( arg, "all" );
    found   = FALSE;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) )
	{
	    if ( fAll || is_name( arg, pObjIndex->name )
	    || flag_value( type_flags, arg ) == pObjIndex->item_type )
	    {
		found = TRUE;
		sprintf( buf, "&b[&C%5d&b]&w %-17.16s",
		    pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
		strcat( buf1, buf );
		if ( ++col % 3 == 0 )
		    strcat( buf1, "\n\r" );
	    }
	}
    }

    if ( !found )
    {
	send_to_char(C_DEFAULT, "Object(s) not found in this area.\n\r", ch);
	return FALSE;
    }

    if ( col % 3 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char(C_DEFAULT, buf1, ch );
    return FALSE;
}

bool redit_rlist( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA	*pRoomIndex;
    AREA_DATA		*pArea;
    char		buf  [ MAX_STRING_LENGTH*25 ];
    char		buf1 [ MAX_STRING_LENGTH*25 ];
    bool found;
    int vnum;
    int  col = 0;

    pArea = ch->in_room->area;
    buf1[0] = '\0';
    found   = FALSE;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if ( ( pRoomIndex = get_room_index( vnum ) ) )
	{
		found = TRUE;
		sprintf( buf, "&b[&C%5d&b]&w %-17.16s",
		    pRoomIndex->vnum, capitalize( pRoomIndex->name ) );
		strcat( buf1, buf );
		if ( ++col % 3 == 0 )
		    strcat( buf1, "\n\r" );
	}
    }

    if ( !found )
    {
	send_to_char(C_DEFAULT, "Room(s) not found in this area.\n\r", ch);
	return FALSE;
    }

    if ( col % 3 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char(C_DEFAULT, buf1, ch );
    return FALSE;
}



bool redit_mshow( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  mshow <vnum>\n\r", ch );
	return FALSE;
    }

    if ( is_number( argument ) )
    {
	value = atoi( argument );
	if ( !( pMob = get_mob_index( value ) ))
	{
	    send_to_char(C_DEFAULT, "REdit:  That mobile does not exist.\n\r", ch );
	    return FALSE;
	}

	ch->desc->pEdit = (void *)pMob;
    }
 
    medit_show( ch, argument );
    ch->desc->pEdit = (void *)ch->in_room;
    return FALSE; 
}



bool redit_oshow( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  oshow <vnum>\n\r", ch );
	return FALSE;
    }

    if ( is_number( argument ) )
    {
	value = atoi( argument );
	if ( !( pObj = get_obj_index( value ) ))
	{
	    send_to_char(C_DEFAULT, "REdit:  That object does not exist.\n\r", ch );
	    return FALSE;
	}

	ch->desc->pEdit = (void *)pObj;
    }
 
    oedit_show( ch, argument );
    ch->desc->pEdit = (void *)ch->in_room;
    return FALSE; 
}



/*****************************************************************************
 Name:		check_range( lower vnum, upper vnum )
 Purpose:	Ensures the range spans only one area.
 Called by:	aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range( int lower, int upper )
{
    AREA_DATA *pArea;
    int cnt = 0;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
	/*
	 * lower < area < upper
	 */
	if ( ( lower <= pArea->lvnum && upper >= pArea->lvnum )
	||   ( upper >= pArea->uvnum && lower <= pArea->uvnum ) )
	    cnt++;

	if ( cnt > 1 )
	    return FALSE;
    }
    return TRUE;
}



AREA_DATA *get_vnum_area( int vnum )
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        if ( vnum >= pArea->lvnum
          && vnum <= pArea->uvnum )
            return pArea;
    }

    return 0;
}



/*
 * Area Editor Functions.
 */
bool aedit_show( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char buf  [MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    sprintf( buf, "&b[&CName&b]:&w     [%5d] %s\n\r", pArea->vnum, pArea->name );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CRecall&b]:&w   [%5d] %s\n\r", pArea->recall,
	get_room_index( pArea->recall )
	? get_room_index( pArea->recall )->name : "none" );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CNoise&b]:&w    %s\n\r", pArea->reset_sound ? pArea->reset_sound :
	                              "(default)" );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CSoundfile&b]:&w%s\n\r", pArea->actual_sound ?
                                      pArea->actual_sound : "(none)" );
    send_to_char( C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CMusicfile&b]:&w%s\n\r", pArea->musicfile ?
                                      pArea->musicfile : "(none)" );
    send_to_char( C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CFile&b]:&w     %s\n\r", pArea->filename );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CVnums&b]:&w    [%d-%d]\n\r", pArea->lvnum, pArea->uvnum );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CAge&b]:&w      [%d]\n\r", pArea->age );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CLlevel&b]:&w   [%d]\n\r", pArea->llevel );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CUlevel&b]:&w   [%d]\n\r", pArea->ulevel );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CPlayers&b]:&z  [%d]\n\r", pArea->nplayer );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CSecurity&b]:&w [%d]\n\r", pArea->security );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CBuilders&b]:&w [%s]\n\r", pArea->builders );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CCreator&b]:&w  [%s]\n\r", pArea->creator );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CFlags&b]:&w    [%s]\n\r", flag_string( area_flags, pArea->area_flags ) );
    send_to_char(C_DEFAULT, buf, ch );

    return FALSE;
}


bool aedit_reset( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    reset_area( pArea );
    send_to_char(C_DEFAULT, "Area reset.\n\r", ch );

    return FALSE;
}



bool aedit_create( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char buff[MAX_STRING_LENGTH] = "";

    if ( top_area >= INT_MAX )	/* OLC 1.1b */
    {
	send_to_char(C_DEFAULT, "We're out of vnums for new areas.\n\r", ch );
	return FALSE;
    }

    sprintf(buff, "proto%d.are", top_area);

    pArea               =   new_area();
    pArea->name         =   str_dup( "New Area" ); /* ahsile */
    pArea->builders     =   str_dup( ch->name   ); /* ahsile */
    pArea->creator 	=   str_dup( ch->name   ); /* ahsile */
    pArea->reset_sound  =   str_dup( "(Default)");
    pArea->musicfile    =   str_dup( "(None)"   );
    pArea->actual_sound =   str_dup( "(None)"   );
    pArea->filename     =   str_dup( buff       );
    pArea->vnum		=   top_area;              /* ahsile */
    pArea->version      =   100;
    pArea->next         =   area_last->next; /* Eversor: We need this. */
    area_last->next     =   pArea;
    area_last		=   pArea;	/* Thanks, Walker. */
    ch->desc->pEdit     =   (void *)pArea;
    pArea->recall       =   17809;
#ifdef SQL_SYSTEM
	pArea->AreaID		= 0;
#endif
    SET_BIT( pArea->area_flags, AREA_ADDED );
    SET_BIT( pArea->area_flags, AREA_PROTOTYPE );
    send_to_char(C_DEFAULT, "Area Created.\n\r", ch );
    return TRUE;	/* OLC 1.1b */
}


bool aedit_creator( CHAR_DATA* ch, char* argument)
{
	AREA_DATA* pArea;

	EDIT_AREA(ch, pArea);

	if (argument[0] == '\0')
	{
		send_to_char(C_DEFAULT, "Syntax:   creator [name]\n\r", ch);
		return FALSE;
	}

	free_string( pArea->creator );
	pArea->creator = str_dup(argument);

	send_to_char(C_DEFAULT, "Creator set.\n\r", ch);
	return TRUE;
}

bool aedit_name( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:   name [$name]\n\r", ch );
	return FALSE;
    }

    free_string( pArea->name );
    pArea->name = str_dup( argument );

    send_to_char(C_DEFAULT, "Name set.\n\r", ch );
    return TRUE;
}



bool aedit_file( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char file[MAX_STRING_LENGTH];
    int i, length;

    EDIT_AREA(ch, pArea);

    one_argument( argument, file );	/* Forces Lowercase */

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  filename [$file]\n\r", ch );
	return FALSE;
    }

    /*
     * Simple Syntax Check.
     */
    length = strlen( argument );
    if ( length > 8 )
    {
	send_to_char(C_DEFAULT, "No more than eight characters allowed.\n\r", ch );
	return FALSE;
    }

    /*
     * Allow only letters and numbers.
     */
    for ( i = 0; i < length; i++ )
    {
	if ( !isalnum( file[i] ) )
	{
	    send_to_char(C_DEFAULT, "Only letters and numbers are valid.\n\r", ch );
	    return FALSE;
	}
    }    

    free_string( pArea->filename );
    strcat( file, ".are" );
    pArea->filename = str_dup( file );

    send_to_char(C_DEFAULT, "Filename set.\n\r", ch );
    return TRUE;
}



bool aedit_age( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char age[MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    one_argument( argument, age );

    if ( !is_number( age ) || age[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  age [#age]\n\r", ch );
	return FALSE;
    }

    pArea->age = atoi( age );

    send_to_char(C_DEFAULT, "Age set.\n\r", ch );
    return TRUE;
}

bool aedit_sounds( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:   noise [$reset_noise]\n\r", ch );
	return FALSE;
    }

    free_string( pArea->reset_sound );
    pArea->reset_sound = str_dup( argument );

    send_to_char(C_DEFAULT, "Reset noise set.\n\r", ch );
    return TRUE;
}

bool aedit_actual_sounds( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )
    {
        send_to_char(C_DEFAULT, "Syntax:   soundfile [$wav_filename]\n\r", ch );
        return FALSE;
    }

    free_string( pArea->actual_sound );
    pArea->actual_sound = str_dup( argument );

    send_to_char(C_DEFAULT, "Actual sounds filename set.\n\r", ch );
    return TRUE;
}

bool aedit_musicfile( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    EDIT_AREA(ch, pArea);

    if ( argument[0] == '\0' )
    {
        send_to_char(C_DEFAULT, "Syntax:   musicfile [string]\n\r", ch );
        return FALSE;
    }

    free_string( pArea->musicfile );
    pArea->musicfile = str_dup( argument );

    send_to_char(C_DEFAULT, "Background Music set.\n\r", ch );
    return TRUE;
}

bool aedit_prototype( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    EDIT_AREA( ch, pArea );

    if ( ( get_trust( ch ) >= 109 ) || ( IS_SET( ch->affected_by2, CODER ) ) 
        || ( !str_cmp( ch->guild->name, "EDEN" ) ) )
    {
        pArea->area_flags ^= AREA_PROTOTYPE;
        send_to_char(C_DEFAULT, "Ok.\n\r", ch );
        return TRUE;
    }
     else
    {
        send_to_char(C_DEFAULT, "You are &B*&wtoo&B*&w low of trust to do this.\n\r", ch );
        return FALSE;
    }
return TRUE;   
}   

bool aedit_noquest( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    
    EDIT_AREA( ch, pArea );
    
    if ( ( get_trust( ch ) >= 109 ) || ( IS_SET( ch->affected_by2, CODER ) ) 
        || ( !str_cmp( ch->guild->name, "EDEN" ) ) )
    {
        pArea->area_flags ^= AREA_NOQUEST;
        send_to_char(C_DEFAULT, "Ok.\n\r", ch );
        return TRUE;
    }
     else
    {
        send_to_char(C_DEFAULT, "You are &B*&wtoo&B*&w low of trust to do this.\n\r", ch );
        return FALSE;
    }
return TRUE;   
}

bool aedit_random( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    
    EDIT_AREA( ch, pArea );
    
    if ( ( get_trust( ch ) >= 109 ) || ( IS_SET( ch->affected_by2, CODER ) ) 
        || ( !str_cmp( ch->guild->name, "EDEN" ) ) )
    {
        pArea->area_flags ^= AREA_RANDOM;
        send_to_char(C_DEFAULT, "Ok.\n\r", ch );
        return TRUE;
    }
     else
    {
        send_to_char(C_DEFAULT, "You are &B*&wtoo&B*&w low of trust to do this.\n\r", ch );
        return FALSE;
    }
return TRUE;   
}

bool aedit_nosave( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    
    EDIT_AREA( ch, pArea );
    
    if ( ( get_trust( ch ) >= 109 ) || ( IS_SET( ch->affected_by2, CODER ) ) 
        || ( !str_cmp( ch->guild->name, "EDEN" ) ) )
    {
        pArea->area_flags ^= AREA_NO_SAVE;
        send_to_char(C_DEFAULT, "Ok.\n\r", ch );
        return TRUE;
    }
     else
    {
        send_to_char(C_DEFAULT, "You are &B*&wtoo&B*&w low of trust to do this.\n\r", ch );
        return FALSE;
    }
return TRUE;   
}   

bool aedit_future( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    
    EDIT_AREA( ch, pArea );
    
    if ( ( get_trust( ch ) >= 109 ) || ( IS_SET( ch->affected_by2, CODER ) ) 
        || ( !str_cmp( ch->guild->name, "EDEN" ) ) )
    {
        pArea->area_flags ^= AREA_FUTURE;
        send_to_char(C_DEFAULT, "Ok.\n\r", ch );
        return TRUE;
    }
     else
    {
        send_to_char(C_DEFAULT, "You are &B*&wtoo&B*&w low of trust to do this.\n\r", ch );
        return FALSE;
    }
return TRUE;   
}   

bool aedit_past( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    
    EDIT_AREA( ch, pArea );
    
    if ( ( get_trust( ch ) >= 109 ) || ( IS_SET( ch->affected_by2, CODER ) ) 
        || ( !str_cmp( ch->guild->name, "EDEN" ) ) )
    {
        pArea->area_flags ^= AREA_PAST;
        send_to_char(C_DEFAULT, "Ok.\n\r", ch );
        return TRUE;
    }
     else
    {
        send_to_char(C_DEFAULT, "You are &B*&wtoo&B*&w low of trust to do this.\n\r", ch );
        return FALSE;
    }
return TRUE;   
}   

bool aedit_present( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    
    EDIT_AREA( ch, pArea );
    
    if ( ( get_trust( ch ) >= 109 ) || ( IS_SET( ch->affected_by2, CODER ) ) 
        || ( !str_cmp( ch->guild->name, "EDEN" ) ) )
    {
        pArea->area_flags ^= AREA_PRESENT;
        send_to_char(C_DEFAULT, "Ok.\n\r", ch );
        return TRUE;
    }
     else
    {
        send_to_char(C_DEFAULT, "You are &B*&wtoo&B*&w low of trust to do this.\n\r", ch );
        return FALSE;
    }
return TRUE;   
}   


bool aedit_recall( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char room[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, room );

    if ( !is_number( argument ) || argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  recall [#rvnum]\n\r", ch );
	return FALSE;
    }

    value = atoi( room );

    if ( !get_room_index( value ) )
    {
	send_to_char(C_DEFAULT, "AEdit:  Room vnum does not exist.\n\r", ch );
	return FALSE;
    }

    pArea->recall = value;

    send_to_char(C_DEFAULT, "Recall set.\n\r", ch );
    return TRUE;
}



bool aedit_security( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char sec[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int  value;

    EDIT_AREA(ch, pArea);

    one_argument( argument, sec );

    if ( !is_number( sec ) || sec[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  security [#level]\n\r", ch );
	return FALSE;
    }

    value = atoi( sec );

    if ( value > ch->pcdata->security || value < 0 )
    {
	if ( ch->pcdata->security != 0 )
	{
	    sprintf( buf, "Security is 0-%d.\n\r", ch->pcdata->security );
	    send_to_char(C_DEFAULT, buf, ch );
	}
	else
	    send_to_char(C_DEFAULT, "Security is 0 only.\n\r", ch );
	return FALSE;
    }

    pArea->security = value;

    send_to_char(C_DEFAULT, "Security set.\n\r", ch );
    return TRUE;
}



bool aedit_builder( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char name[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    EDIT_AREA(ch, pArea);

    one_argument( argument, name );

    if ( name[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  builder [$name]  -toggles builder\n\r", ch );
	send_to_char(C_DEFAULT, "Syntax:  builder All      -allows everyone\n\r", ch );
	return FALSE;
    }

    name[0] = UPPER( name[0] );

    if ( strstr( pArea->builders, name ) != '\0' )
    {
	pArea->builders = string_replace( pArea->builders, name, "\0" );
	pArea->builders = string_unpad( pArea->builders );

	if ( pArea->builders[0] == '\0' )
	{
	    free_string( pArea->builders );
	    pArea->builders = str_dup( "None" );
	}
	send_to_char(C_DEFAULT, "Builder removed.\n\r", ch );
	return TRUE;
    }
    else
    {
	buf[0] = '\0';
	if ( strstr( pArea->builders, "None" ) != '\0' )
	{
	    pArea->builders = string_replace( pArea->builders, "None", "\0" );
	    pArea->builders = string_unpad( pArea->builders );
	}

	if (pArea->builders[0] != '\0' )
	{
	    strcat( buf, pArea->builders );
	    strcat( buf, " " );
	}
	strcat( buf, name );
	free_string( pArea->builders );
	pArea->builders = string_proper( str_dup( buf ) );

	send_to_char(C_DEFAULT, "Builder added.\n\r", ch );
	return TRUE;
    }

    return FALSE;
}



bool aedit_vnum( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    argument = one_argument( argument, lower );
    one_argument( argument, upper );

    if ( !is_number( lower ) || lower[0] == '\0'
    || !is_number( upper ) || upper[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  vnum [#lower] [#upper]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = atoi( upper ) ) )
    {
	send_to_char(C_DEFAULT, "AEdit:  Upper must be larger then lower.\n\r", ch );
	return FALSE;
    }
    
    /* OLC 1.1b */
    if ( ilower <= 0 || ilower >= INT_MAX || iupper <= 0 || iupper >= INT_MAX )
    {
	char output[MAX_STRING_LENGTH];

	sprintf( output, "AEdit: vnum must be between 0 and %d.\n\r", INT_MAX );
	send_to_char(C_DEFAULT, output, ch );
	return FALSE;
    }

    if ( !check_range( ilower, iupper ) )
    {
	send_to_char(C_DEFAULT, "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( ilower )
    && get_vnum_area( ilower ) != pArea )
    {
	send_to_char(C_DEFAULT, "AEdit:  Lower vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->lvnum = ilower;
    send_to_char(C_DEFAULT, "Lower vnum set.\n\r", ch );

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
	send_to_char(C_DEFAULT, "AEdit:  Upper vnum already assigned.\n\r", ch );
	return TRUE;	/* The lower value has been set. */
    }

    pArea->uvnum = iupper;
    send_to_char(C_DEFAULT, "Upper vnum set.\n\r", ch );

    return TRUE;
}



bool aedit_lvnum( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char lower[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, lower );

    if ( !is_number( lower ) || lower[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  lvnum [#lower]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = pArea->uvnum ) )
    {
	send_to_char(C_DEFAULT, "AEdit:  Value must be less than the uvnum.\n\r", ch );
	return FALSE;
    }
    
    /* OLC 1.1b */
    if ( ilower <= 0 || ilower >= INT_MAX || iupper <= 0 || iupper >= INT_MAX )
    {
	char output[MAX_STRING_LENGTH];

	sprintf( output, "AEdit: vnum must be between 0 and %d.\n\r", INT_MAX );
	send_to_char(C_DEFAULT, output, ch );
	return FALSE;
    }

    if ( !check_range( ilower, iupper ) )
    {
	send_to_char(C_DEFAULT, "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( ilower )
    && get_vnum_area( ilower ) != pArea )
    {
	send_to_char(C_DEFAULT, "AEdit:  Lower vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->lvnum = ilower;
    send_to_char(C_DEFAULT, "Lower vnum set.\n\r", ch );
    return TRUE;
}



bool aedit_uvnum( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char upper[MAX_STRING_LENGTH];
    int  ilower;
    int  iupper;

    EDIT_AREA(ch, pArea);

    one_argument( argument, upper );

    if ( !is_number( upper ) || upper[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  uvnum [#upper]\n\r", ch );
	return FALSE;
    }

    if ( ( ilower = pArea->lvnum ) > ( iupper = atoi( upper ) ) )
    {
	send_to_char(C_DEFAULT, "AEdit:  Upper must be larger then lower.\n\r", ch );
	return FALSE;
    }
    
    /* OLC 1.1b */
    if ( ilower <= 0 || ilower >= INT_MAX || iupper <= 0 || iupper >= INT_MAX )
    {
	char output[MAX_STRING_LENGTH];

	sprintf( output, "AEdit: vnum must be between 0 and %d.\n\r", INT_MAX );
	send_to_char(C_DEFAULT, output, ch );
	return FALSE;
    }

    if ( !check_range( ilower, iupper ) )
    {
	send_to_char(C_DEFAULT, "AEdit:  Range must include only this area.\n\r", ch );
	return FALSE;
    }

    if ( get_vnum_area( iupper )
    && get_vnum_area( iupper ) != pArea )
    {
	send_to_char(C_DEFAULT, "AEdit:  Upper vnum already assigned.\n\r", ch );
	return FALSE;
    }

    pArea->uvnum = iupper;
    send_to_char(C_DEFAULT, "Upper vnum set.\n\r", ch );

    return TRUE;
}



/*
 * Room Editor Functions.
 */
bool redit_show( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA	*pRoom;
    char		buf  [MAX_STRING_LENGTH];
    char		buf1 [2*MAX_STRING_LENGTH];
    OBJ_DATA		*obj;
    CHAR_DATA		*rch;
    int			door;
    bool		fcnt;
    
    EDIT_ROOM(ch, pRoom);

    buf1[0] = '\0';
    
    sprintf( buf, "&b[&CDescription&b]:&w\n\r%s", pRoom->description );
    strcat( buf1, buf );

    sprintf( buf, "&b[&CName&b]:       &w[%s]\n\r&b[&CArea&b]:       &z[%5d] %s\n\r",
	    pRoom->name, pRoom->area->vnum, pRoom->area->name );
    strcat( buf1, buf );

    sprintf( buf, "&b[&CVnum&b]:       &z[%5d]\n\r&b[&CSector&b]:     &w[%s]\n\r",
	    pRoom->vnum, flag_string( sector_flags, pRoom->sector_type ) );
    strcat( buf1, buf );

    sprintf( buf, "&b[&CSoundfile&b]:  &w[%s]\n\r",
	    pRoom->soundfile );
    strcat( buf1, buf );

    sprintf( buf, "&b[&CMusicfile&b]:  &w[%s]\n\r",
       pRoom->musicfile );
    strcat( buf1, buf );

    sprintf( buf, "&b[&CRoom flags&b]: &w[%s]\n\r",
	    flag_string( room_flags, pRoom->room_flags ) );
    strcat( buf1, buf );

    if ( pRoom->rd > 0 )
    {
      sprintf( buf, "&b[&CRoom damage amount&b]: &w[%d]\n\r", pRoom->rd );
      strcat( buf1, buf );
    }

    if ( pRoom->extra_descr )
    {
	EXTRA_DESCR_DATA *ed;

	strcat( buf1, "&b[&CDesc Kwds&b]:  [&w" );
	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    strcat( buf1, ed->keyword );
	    if ( ed->next )
		strcat( buf1, " " );
	}
	strcat( buf1, "&b]\n\r" );
    }

    strcat( buf1, "&b[&CCharacters&b]: &w[" );
    fcnt = FALSE;
    for ( rch = pRoom->people; rch; rch = rch->next_in_room )
    {
	one_argument( rch->name, buf );
	strcat( buf1, buf );
	strcat( buf1, " " );
	fcnt = TRUE;
    }

    if ( fcnt )
    {
	int end;

	end = strlen(buf1) - 1;
	buf1[end] = ']';
	strcat( buf1, "\n\r" );
    }
    else
	strcat( buf1, "none]\n\r" );

    strcat( buf1, "&b[&CObjects&b]:    &w[" );
    fcnt = FALSE;
    for ( obj = pRoom->contents; obj; obj = obj->next_content )
    {
	one_argument( obj->name, buf );
	strcat( buf1, buf );
	strcat( buf1, " " );
	fcnt = TRUE;
    }

    if ( fcnt )
    {
	int end;

	end = strlen(buf1) - 1;
	buf1[end] = ']';
	strcat( buf1, "\n\r" );
    }
    else
	strcat( buf1, "none]\n\r" );

    for ( door = 0; door < MAX_DIR; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = pRoom->exit[door] ) )
	{
	    char word[MAX_INPUT_LENGTH];
	    char reset_state[MAX_STRING_LENGTH];
	    char *state;
	    int i, length;

	    sprintf( buf, "&w-%-5s &Cto &w[%5d] &CKey&b: &w[%5d]",
		capitalize(dir_name[door]),
		pexit->to_room ? pexit->to_room->vnum : 0,
		pexit->key );
	    strcat( buf1, buf );

	    /*
	     * Format up the exit info.
	     * Capitalize all flags that are not part of the reset info.
	     */
	    strcpy( reset_state, flag_string( exit_flags, pexit->rs_flags ) );
	    state = flag_string( exit_flags, pexit->exit_info );
	    strcat( buf1, " &CExit flags&b: &w[" );
	    for (; ;)
	    {
		state = one_argument( state, word );

		if ( word[0] == '\0' )
		{
		    int end;

		    end = strlen(buf1) - 1;
		    buf1[end] = ']';
		    strcat( buf1, "\n\r" );
		    break;
		}

		if ( str_infix( word, reset_state ) )
		{
		    length = strlen(word);
		    for (i = 0; i < length; i++)
			word[i] = toupper(word[i]);
		}
		strcat( buf1, word );
		strcat( buf1, " " );
	    }

	    if ( pexit->keyword && pexit->keyword[0] != '\0' )
	    {
		sprintf( buf, "&CKwds&b: &w[%s]\n\r", pexit->keyword );
		strcat( buf1, buf );
	    }
	    if ( pexit->description && pexit->description[0] != '\0' )
	    {
		sprintf( buf, "&w%s", pexit->description );
		strcat( buf1, buf );
	    }
	}
    }

    send_to_char(C_DEFAULT, buf1, ch );
    return FALSE;
}

bool redit_rdamage( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *pRoom;
  char arg[MAX_STRING_LENGTH];
  int damage;
  
  EDIT_ROOM( ch, pRoom );
  
  if ( argument[0] == '\0' )
  {
    send_to_char( AT_PURPLE, "Syntax: rdamage [ammount]\n\r", ch );
    return FALSE;
  }
  
  argument = one_argument( argument, arg );
  if ( !is_number( arg ) )
  {
    send_to_char( AT_WHITE, "Ammount must be the number of hp the room is to damage the CH per update.\n\r", ch );
    return FALSE;
  }
  
  damage = atoi( arg );
  pRoom->rd = damage;
  send_to_char( AT_WHITE, "Ok.\n\r", ch );
  return TRUE;
}


/* OLC 1.1b */
/*****************************************************************************
 Name:		change_exit
 Purpose:	Command interpreter for changing exits.
 Called by:	redit_<dir>.  This is a local function.
 ****************************************************************************/
bool change_exit( CHAR_DATA *ch, char *argument, int door )
{
    ROOM_INDEX_DATA *pRoom;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char total_arg[MAX_STRING_LENGTH];
    int  rev;
    int  value = 0;

    EDIT_ROOM(ch, pRoom);

    /* Often used data. */
    rev = rev_dir[door];
    
    if ( argument[0] == '\0' )
    {
	do_help( ch, "EXIT" );
	return FALSE;
    }

    /*
     * Now parse the arguments.
     */
    strcpy( total_arg, argument );
    argument = one_argument( argument, command );
    one_argument( argument, arg );

    if ( !str_cmp( command, "delete" ) )
    {	
	if ( !pRoom->exit[door] )
	{
	    send_to_char(C_DEFAULT, "REdit:  Exit does not exist.\n\r", ch );
	    return FALSE;
	}
	if (  pRoom->exit[door]->to_room == 0 )
	{
	   send_to_char( C_DEFAULT, "REdit:  Bad Room number 0.. don't delete it :)", ch );   
	   return FALSE;
	}

	/*
	 * Remove To Room Exit.
	 */
	if ( pRoom->exit[door]->to_room->exit[rev] )
	{
	    free_exit( pRoom->exit[door]->to_room->exit[rev] );
	    pRoom->exit[door]->to_room->exit[rev] = NULL;
	}

	/*
	 * Remove this exit.
	 */
	free_exit( pRoom->exit[door] );
	pRoom->exit[door] = NULL;

	send_to_char(C_DEFAULT, "Exit unlinked.\n\r", ch );
	return TRUE;
    }

    /*
     * Create a two-way exit.
     */
    if ( !str_cmp( command, "link" ) )
    {
	EXIT_DATA	*pExit;
	ROOM_INDEX_DATA	*pLinkRoom;

	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char(C_DEFAULT, "Syntax:  [direction] link [vnum]\n\r", ch );
	    return FALSE;
	}

	if ( !( pLinkRoom = get_room_index( atoi(arg) ) ) )
	{
	    send_to_char(C_DEFAULT, "REdit:  Non-existant room.\n\r", ch );
	    return FALSE;
	}

	if ( !IS_BUILDER( ch, pLinkRoom->area ) )
	{
	    send_to_char(C_DEFAULT, "REdit:  Cannot link to that area.\n\r", ch );
	    return FALSE;
	}

	if ( pLinkRoom->exit[rev] )
	{
	    send_to_char(C_DEFAULT, "REdit:  Remote side's exit exists.\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )		/* No exit.		*/
	    pRoom->exit[door] = new_exit();

	pRoom->exit[door]->to_room = pLinkRoom;	/* Assign data.		*/
	pRoom->exit[door]->vnum = value;

	pExit			= new_exit();	/* No remote exit.	*/

	pExit->to_room		= ch->in_room;	/* Assign data.		*/
	pExit->vnum		= ch->in_room->vnum;

	pLinkRoom->exit[rev]	= pExit;	/* Link exit to room.	*/

	send_to_char(C_DEFAULT, "Two-way link established.\n\r", ch );
	return TRUE;
    }

    /*
     * Create room and make two-way exit.
     */
    if ( !str_cmp( command, "dig" ) )
    {
	char buf[MAX_INPUT_LENGTH];
	
	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char(C_DEFAULT, "Syntax: [direction] dig <vnum>\n\r", ch );
	    return FALSE;
	}
	
	redit_create( ch, arg );		/* Create the room.	*/
	sprintf( buf, "link %s", arg );
	change_exit( ch, buf, door);		/* Create the exits.	*/
	return TRUE;
    }

    /*
     * Create one-way exit.
     */
    if ( !str_cmp( command, "room" ) )
    {
	ROOM_INDEX_DATA *pLinkRoom;

	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char(C_DEFAULT, "Syntax:  [direction] room [vnum]\n\r", ch );
	    return FALSE;
	}

	if ( !( pLinkRoom = get_room_index( atoi( arg ) ) ) )
	{
	    send_to_char(C_DEFAULT, "REdit:  Non-existant room.\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	    pRoom->exit[door] = new_exit();

	pRoom->exit[door]->to_room = pLinkRoom;
	pRoom->exit[door]->vnum = value;

	send_to_char(C_DEFAULT, "One-way link established.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "remove" ) )
    {
	if ( arg[0] == '\0' )
	{
	    send_to_char(C_DEFAULT, "Syntax:  [direction] remove [key/name/desc]\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	{
	    send_to_char(C_DEFAULT, "REdit:  Exit does not exist.\n\r", ch );
	    return FALSE;
	}

	if ( !str_cmp( argument, "key" ) )
	{
	    pRoom->exit[door]->key = 0;
            send_to_char(C_DEFAULT, "Exit key removed.\n\r", ch );                        
            return TRUE;
	}

	if ( !str_cmp( argument, "name" ) )
	{
	    free_string( pRoom->exit[door]->keyword );
	    pRoom->exit[door]->keyword = &str_empty[0];
            send_to_char(C_DEFAULT, "Exit name removed.\n\r", ch );                        
            return TRUE;
	}

	if ( argument[0] == 'd' && !str_prefix( argument, "description" ) )
	{
	    free_string( pRoom->exit[door]->description );
	    pRoom->exit[door]->description = &str_empty[0];
            send_to_char(C_DEFAULT, "Exit description removed.\n\r", ch );                        
            return TRUE;
	}

	send_to_char(C_DEFAULT, "Syntax:  [direction] remove [key/name/desc]\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( command, "key" ) )
    {
	OBJ_INDEX_DATA *pObjIndex;

	if ( arg[0] == '\0' || !is_number( arg ) )
	{
	    send_to_char(C_DEFAULT, "Syntax:  [direction] key [vnum]\n\r", ch );
	    return FALSE;
	}

	if ( !( pObjIndex = get_obj_index( atoi( arg ) ) ) )
	{
	    send_to_char(C_DEFAULT, "REdit:  Item does not exist.\n\r", ch );
	    return FALSE;
	}

	if ( pObjIndex->item_type != ITEM_KEY )
	{
	    send_to_char(C_DEFAULT, "REdit:  Item is not a key.\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	    pRoom->exit[door] = new_exit();

	pRoom->exit[door]->key = pObjIndex->vnum;

	send_to_char(C_DEFAULT, "Exit key set.\n\r", ch );
	return TRUE;
    }

    if ( !str_cmp( command, "name" ) )
    {
	if ( arg[0] == '\0' )
	{
	    send_to_char(C_DEFAULT, "Syntax:  [direction] name [string]\n\r", ch );
	    return FALSE;
	}

	if ( !pRoom->exit[door] )
	    pRoom->exit[door] = new_exit();

	free_string( pRoom->exit[door]->keyword );
	pRoom->exit[door]->keyword = str_dup( argument );

	send_to_char(C_DEFAULT, "Exit name set.\n\r", ch );
	return TRUE;
    }

    if ( command[0] == 'd' && !str_prefix( command, "description" ) )
    {
	if ( arg[0] == '\0' )
	{
	    if ( !pRoom->exit[door] )
	        pRoom->exit[door] = new_exit();

	    string_append( ch, &pRoom->exit[door]->description );
	    return TRUE;
	}

	send_to_char(C_DEFAULT, "Syntax:  [direction] desc\n\r", ch );
	return FALSE;
    }

    /*
     * Set the exit flags, needs full argument.
     * ----------------------------------------
     */
    if ( ( value = flag_value( exit_flags, total_arg ) ) != NO_FLAG )
    {
	ROOM_INDEX_DATA *pToRoom;

	/*
	 * Create an exit if none exists.
	 */
	if ( !pRoom->exit[door] )
	    pRoom->exit[door] = new_exit();

	/*
	 * Set door bits for this room.
	 */
	TOGGLE_BIT(pRoom->exit[door]->rs_flags, value);
	pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;

	/*
	 * Set door bits of connected room.
	 * Skip one-way exits and non-existant rooms.
	 */
	if ( ( pToRoom = pRoom->exit[door]->to_room ) && pToRoom->exit[rev] )
	{
	    TOGGLE_BIT(pToRoom->exit[rev]->rs_flags, value);
	    pToRoom->exit[rev]->exit_info =  pToRoom->exit[rev]->rs_flags;
	}

	send_to_char(C_DEFAULT, "Exit flag toggled.\n\r", ch );
	return TRUE;
    }

    return FALSE;
}



bool redit_north( CHAR_DATA *ch, char *argument )
{
    if ( change_exit( ch, argument, DIR_NORTH ) )
	return TRUE;

    return FALSE;
}



bool redit_south( CHAR_DATA *ch, char *argument )
{
    if ( change_exit( ch, argument, DIR_SOUTH ) )
	return TRUE;

    return FALSE;
}



bool redit_east( CHAR_DATA *ch, char *argument )
{
    if ( change_exit( ch, argument, DIR_EAST ) )
	return TRUE;

    return FALSE;
}



bool redit_west( CHAR_DATA *ch, char *argument )
{
    if ( change_exit( ch, argument, DIR_WEST ) )
	return TRUE;

    return FALSE;
}



bool redit_up( CHAR_DATA *ch, char *argument )
{
    if ( change_exit( ch, argument, DIR_UP ) )
	return TRUE;

    return FALSE;
}



bool redit_down( CHAR_DATA *ch, char *argument )
{
    if ( change_exit( ch, argument, DIR_DOWN ) )
	return TRUE;

    return FALSE;
}


/* OLC 1.1b */
bool redit_move( CHAR_DATA *ch, char *argument )
{
    interpret( ch, argument );
    return FALSE;
}



bool redit_ed( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, command );
    one_argument( argument, keyword );

    if ( command[0] == '\0' || keyword[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  ed add [keyword]\n\r", ch );
	send_to_char(C_DEFAULT, "         ed edit [keyword]\n\r", ch );
	send_to_char(C_DEFAULT, "         ed delete [keyword]\n\r", ch );
	send_to_char(C_DEFAULT, "         ed format [keyword]\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( command, "add" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char(C_DEFAULT, "Syntax:  ed add [keyword]\n\r", ch );
	    return FALSE;
	}

	ed			=   new_extra_descr();
	ed->keyword		=   str_dup( keyword );
	ed->description		=   str_dup( "" );
	ed->next		=   pRoom->extra_descr;
	pRoom->extra_descr	=   ed;

	string_append( ch, &ed->description );

	return TRUE;
    }


    if ( !str_cmp( command, "edit" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char(C_DEFAULT, "Syntax:  ed edit [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	}

	if ( !ed )
	{
	    send_to_char(C_DEFAULT, "REdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	string_append( ch, &ed->description );

	return TRUE;
    }


    if ( !str_cmp( command, "delete" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )
	{
	    send_to_char(C_DEFAULT, "Syntax:  ed delete [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
	    send_to_char(C_DEFAULT, "REdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	if ( !ped )
	    pRoom->extra_descr = ed->next;
	else
	    ped->next = ed->next;

	free_extra_descr( ed );

	send_to_char(C_DEFAULT, "Extra description deleted.\n\r", ch );
	return TRUE;
    }


    if ( !str_cmp( command, "format" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char(C_DEFAULT, "Syntax:  ed format [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pRoom->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	}

	if ( !ed )
	{
	    send_to_char(C_DEFAULT, "REdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	/* OLC 1.1b */
	if ( strlen(ed->description) >= (MAX_STRING_LENGTH - 4) )
	{
	    send_to_char(C_DEFAULT, "String too long to be formatted.\n\r", ch );
	    return FALSE;
	}

	ed->description = format_string( ed->description );

	send_to_char(C_DEFAULT, "Extra description formatted.\n\r", ch );
	return TRUE;
    }

    redit_ed( ch, "" );
    return FALSE;
}



bool redit_create( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int value;
    int iHash;
    
    EDIT_ROOM(ch, pRoom);

    value = atoi( argument );

    /* OLC 1.1b */
    if ( argument[0] == '\0' || value <= 0 || value >= INT_MAX )
    {
	char output[MAX_STRING_LENGTH];

	sprintf( output, "Syntax:  create [0 < vnum < %d]\n\r", INT_MAX );
	send_to_char(C_DEFAULT, output, ch );
	return FALSE;
    }

    if ( argument[0] == '\0' || value <= 0 )
    {
	send_to_char(C_DEFAULT, "Syntax:  create [vnum > 0]\n\r", ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );
    if ( !pArea )
    {
	send_to_char(C_DEFAULT, "REdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char(C_DEFAULT, "REdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    if ( get_room_index( value ) )
    {
	send_to_char(C_DEFAULT, "REdit:  Room vnum already exists.\n\r", ch );
	return FALSE;
    }

    pRoom			= new_room_index();
    pRoom->area			= pArea;
    pRoom->vnum			= value;

    if ( value > top_vnum_room )
        top_vnum_room = value;

    iHash			= value % MAX_KEY_HASH;
    pRoom->next			= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoom;
    ch->desc->pEdit		= (void *)pRoom;

    send_to_char(C_DEFAULT, "Room created.\n\r", ch );
    return TRUE;
}



bool redit_name( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  name [name]\n\r", ch );
	return FALSE;
    }

    free_string( pRoom->name );
    pRoom->name = str_dup( argument );

    send_to_char(C_DEFAULT, "Name set.\n\r", ch );
    return TRUE;
}

bool redit_soundfile( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
        send_to_char(C_DEFAULT, "Syntax:  soundfile [$wav_filename]\n\r", ch );
        return FALSE;
    }

    free_string( pRoom->soundfile );
    pRoom->soundfile = str_dup( argument );

    send_to_char(C_DEFAULT, "Soundfile set.\n\r", ch );
    return TRUE;
}

bool redit_musicfile( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
        send_to_char(C_DEFAULT, "Syntax:  musicfile [string]\n\r", ch );
        return FALSE;
    }

    free_string( pRoom->musicfile );
    pRoom->musicfile = str_dup( argument );

    send_to_char(C_DEFAULT, "Override Music set.\n\r", ch );
    return TRUE;
}

bool redit_desc( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    if ( argument[0] == '\0' )
    {
	string_append( ch, &pRoom->description );
	return TRUE;
    }

    send_to_char(C_DEFAULT, "Syntax:  desc\n\r", ch );
    return FALSE;
}




bool redit_format( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch, pRoom);

    /* OLC 1.1b */
    if ( strlen(pRoom->description) >= (MAX_STRING_LENGTH - 4) )
    {
	send_to_char(C_DEFAULT, "String too long to be formatted.\n\r", ch );
	return FALSE;
    }

    pRoom->description = format_string( pRoom->description );

    send_to_char(C_DEFAULT, "String formatted.\n\r", ch );
    return TRUE;
}



bool redit_mreset( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA	*pRoom;
    MOB_INDEX_DATA	*pMobIndex;
    CHAR_DATA		*newmob;
    char		arg [ MAX_INPUT_LENGTH ];
    RESET_DATA		*pReset;
    char		output [ MAX_STRING_LENGTH ];
	char		arg2 [ MAX_INPUT_LENGTH ];
    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2);
    if ( arg[0] == '\0' || !is_number( arg ) )
    {
	send_to_char (C_DEFAULT, "Syntax:  mreset <vnum> <max #> <status>\n\r", ch );
	send_to_char (C_DEFAULT, " <status> for Area Theme Quests is optional\n\rdefaults to mobile being reset in all statuses\n\r", ch);
	return FALSE;
    }

    if ( !( pMobIndex = get_mob_index( atoi( arg ) ) ) )
    {
	send_to_char(C_DEFAULT, "REdit: No mobile has that vnum.\n\r", ch );
	return FALSE;
    }

    if ( ( ( pMobIndex->area != pRoom->area ) && get_trust( ch ) < 109 ) &&
       !IS_CODER( ch ) )
    {
	send_to_char(C_DEFAULT, "REdit: No such mobile in this area.\n\r", ch );
	return FALSE;
    }

    /*
     * Create the mobile reset.
     */
    pReset = new_reset();
    pReset->command	= 'M';
    pReset->arg1	= pMobIndex->vnum;
    pReset->arg2	= is_number( arg2 ) ? atoi( arg2 ) : MAX_MOB;
	pReset->status  = is_number(argument) ? atoi( argument) : 0;
    pReset->arg3	= pRoom->vnum;
    add_reset( pRoom, pReset, 0/* Last slot*/ );

    /*
     * Create the mobile.
     */
    newmob = create_mobile( pMobIndex );
    char_to_room( newmob, pRoom );

    sprintf( output, "%s (%d) has been loaded and added to resets.\n\r"
	"There will be a maximum of %d loaded to this room.\n\r",
	capitalize( pMobIndex->short_descr ),
	pMobIndex->vnum,
	pReset->arg2 );
    send_to_char(C_DEFAULT, output, ch );
    act(C_DEFAULT, "$n has created $N!", ch, NULL, newmob, TO_ROOM );
    return TRUE;
}

struct wear_type
{
    int	wear_loc;
    int	wear_bit;
};



const struct wear_type wear_table[] =
{
    {	WEAR_NONE,	ITEM_TAKE		},
    {	WEAR_LIGHT,	ITEM_LIGHT		},
    {	WEAR_FINGER_L,	ITEM_WEAR_FINGER	},
    {	WEAR_FINGER_R,	ITEM_WEAR_FINGER	},
    {	WEAR_NECK_1,	ITEM_WEAR_NECK		},
    {	WEAR_NECK_2,	ITEM_WEAR_NECK		},
    {	WEAR_BODY,	ITEM_WEAR_BODY		},
    {	WEAR_HEAD,	ITEM_WEAR_HEAD		},
    {   WEAR_IN_EYES,   ITEM_WEAR_CONTACT       },
    {   WEAR_ORBIT,     ITEM_WEAR_ORBIT         },
    {   WEAR_ORBIT_2,   ITEM_WEAR_ORBIT         },
    {   WEAR_ON_FACE,   ITEM_WEAR_FACE          },
    {	WEAR_LEGS,	ITEM_WEAR_LEGS		},
    {	WEAR_FEET,	ITEM_WEAR_FEET		},
    {	WEAR_HANDS,	ITEM_WEAR_HANDS		},
    {	WEAR_ARMS,	ITEM_WEAR_ARMS		},
    {	WEAR_SHIELD,	ITEM_WEAR_SHIELD	},
    {	WEAR_ABOUT,	ITEM_WEAR_ABOUT		},
    {	WEAR_WAIST,	ITEM_WEAR_WAIST		},
    {	WEAR_WRIST_L,	ITEM_WEAR_WRIST		},
    {	WEAR_WRIST_R,	ITEM_WEAR_WRIST		},
    {	WEAR_WIELD,	ITEM_WIELD		},
    {   WEAR_WIELD_2,   ITEM_WIELD              },
    {	WEAR_HOLD,	ITEM_HOLD		},
    {   WEAR_FIREARM,	ITEM_WEAR_FIREARM,	},
    {   WEAR_ANKLE_L,   ITEM_WEAR_ANKLE         },
    {   WEAR_ANKLE_R,   ITEM_WEAR_ANKLE         },
    {   WEAR_EARS,      ITEM_WEAR_EARS          },
    {   WEAR_IMPLANTED1,ITEM_WEAR_IMPLANTED	},
    {   WEAR_IMPLANTED2,ITEM_WEAR_IMPLANTED	},
    {   WEAR_IMPLANTED3,ITEM_WEAR_IMPLANTED	},
    {	NO_FLAG,	NO_FLAG			}
};



/*****************************************************************************
 Name:		wear_loc
 Purpose:	Returns the location of the bit that matches the count.
 		1 = first match, 2 = second match etc.
 Called by:	oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc(int bits, int count)
{
    int flag;
 
    for (flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++)
    {
        if ( IS_SET(bits, wear_table[flag].wear_bit) && --count < 1)
            return wear_table[flag].wear_loc;
    }
 
    return NO_FLAG;
}



/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit(int loc)
{
    int flag;
 
    for (flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++)
    {
        if ( loc == wear_table[flag].wear_loc )
            return wear_table[flag].wear_bit;
    }
 
    return 0;
}



bool redit_oreset( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA	*pRoom;
    OBJ_INDEX_DATA	*pObjIndex;
    OBJ_DATA		*newobj;
    OBJ_DATA		*to_obj;
    CHAR_DATA		*to_mob;
    char		arg1 [ MAX_INPUT_LENGTH ];
    char		arg2 [ MAX_INPUT_LENGTH ];
    int			olevel = 0;

    RESET_DATA		*pReset;
    char		output [ MAX_STRING_LENGTH ];

    EDIT_ROOM(ch, pRoom);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
	send_to_char (C_DEFAULT, "Syntax:  oreset <vnum> <args>\n\r", ch );
	send_to_char (C_DEFAULT, "        -no_args               = into room\n\r", ch );
	send_to_char (C_DEFAULT, "        -<obj_name>            = into obj\n\r", ch );
	send_to_char (C_DEFAULT, "        -<mob_name> <wear_loc> = into mob\n\r", ch );
	return FALSE;
    }

    if ( !( pObjIndex = get_obj_index( atoi( arg1 ) ) ) )
    {
	send_to_char(C_DEFAULT, "REdit: No object has that vnum.\n\r", ch );
	return FALSE;
    }

    if ( ( ( pObjIndex->area != pRoom->area ) && get_trust( ch ) < 109 ) &&
      !IS_CODER( ch ) )
    {
	send_to_char(C_DEFAULT, "REdit: No such object in this area.\n\r", ch );
	return FALSE;
    }

    /*
     * Load into room.
     */
    if ( arg2[0] == '\0' )
    {
	pReset		= new_reset();
	pReset->command	= 'O';
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= 0;
	pReset->arg3	= pRoom->vnum;
	add_reset( pRoom, pReset, 0/* Last slot*/ );

	newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
	obj_to_room( newobj, pRoom );

	sprintf( output, "%s (%d) has been loaded and added to resets.\n\r",
	    capitalize( pObjIndex->short_descr ),
	    pObjIndex->vnum );
	send_to_char(C_DEFAULT, output, ch );
    }
    else
    /*
     * Load into object's inventory.
     */
    if ( argument[0] == '\0'
    && ( ( to_obj = get_obj_list( ch, arg2, pRoom->contents ) ) != NULL ) )
    {
	pReset		= new_reset();
	pReset->command	= 'P';
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= 0;
	pReset->arg3	= to_obj->pIndexData->vnum;
	add_reset( pRoom, pReset, 0/* Last slot*/ );

	newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
	newobj->cost = 0;
	obj_to_obj( newobj, to_obj );

	sprintf( output, "%s (%d) has been loaded into "
	    "%s (%d) and added to resets.\n\r",
	    capitalize( newobj->short_descr ),
	    newobj->pIndexData->vnum,
	    to_obj->short_descr,
	    to_obj->pIndexData->vnum );
	send_to_char(C_DEFAULT, output, ch );
    }
    else
    /*
     * Load into mobile's inventory.
     */
    if ( ( to_mob = get_char_room( ch, arg2 ) ) != NULL )
    {
	int	wear_loc;

	/* Find specific reset to load AFTER */
	RESET_DATA *pMob;
	int     reset_loc = 1;
	int     mob_num;
	int     counter = 0;

	mob_num = number_argument( arg2, arg2 );
	for ( pMob = ch->in_room->reset_first; pMob; pMob = pMob->next )
	{
	  ++reset_loc;
	  if ( pMob->arg1 == to_mob->pIndexData->vnum && ++counter == mob_num )
	    break;
	}
	if ( !pMob )
	{
	  send_to_char(C_DEFAULT, "Mobile not reset in this room.\n\r",ch);
	  return FALSE;
	}
	/* Load after all other worn/held items, but before next reset
	 * of any other type. */
	for ( pMob = pMob->next; pMob; pMob = pMob->next )
	{
	  ++reset_loc;
	  if ( pMob->command != 'G' && pMob->command != 'E' )
	    break;
	}
	if ( !pMob )
	  reset_loc = 0;

	/*
	 * Make sure the location on mobile is valid.
	 */
	if ( (wear_loc = flag_value( wear_loc_flags, argument )) == NO_FLAG )
	{
	    send_to_char(C_DEFAULT, "REdit: Invalid wear_loc.  '? wear-loc'\n\r", ch );
	    return FALSE;
	}

	/*
	 * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
	 */
	if ( !IS_SET( pObjIndex->wear_flags, wear_bit(wear_loc) ) )
	{
	    sprintf( output,
	        "%s (%d) has wear flags: [%s]\n\r",
	        capitalize( pObjIndex->short_descr ),
	        pObjIndex->vnum,
		flag_string( wear_flags, pObjIndex->wear_flags ) );
	    send_to_char(C_DEFAULT, output, ch );
	    return FALSE;
	}

	/*
	 * Can't load into same position.
	 */
	if ( ( get_eq_char( to_mob, wear_loc ) ) && ( wear_loc != -1 ) )
	{
	    send_to_char(C_DEFAULT, "REdit:  Object already equipped.\n\r", ch );
	    return FALSE;
	}

	pReset		= new_reset();
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= wear_loc;
	if ( pReset->arg2 == WEAR_NONE )
	    pReset->command = 'G';
	else
	    pReset->command = 'E';
	pReset->arg3	= wear_loc;

	add_reset( pRoom, pReset, reset_loc );

	olevel  = URANGE( 0, to_mob->level - 2, LEVEL_DEMIGOD );
        newobj = create_object( pObjIndex, number_fuzzy( olevel ) );

	if ( to_mob->pIndexData->pShop )	/* Shop-keeper? */
	{
	    switch ( pObjIndex->item_type )
	    {
	    default:		olevel = 0;				break;
	    case ITEM_PILL:	olevel = number_range(  0, 10 );	break;
	    case ITEM_POTION:	olevel = number_range(  0, 10 );	break;
	    case ITEM_IMPLANTED: olevel= number_range( 10, 20 );	break;
	    case ITEM_SCROLL:	olevel = number_range(  5, 15 );	break;
	    case ITEM_GUN:      olevel = number_range( 10, 20 );	break;
	    case ITEM_WAND:	olevel = number_range( 10, 20 );	break;
            case ITEM_LENSE:    olevel = number_range( 10, 20 );        break;
	    case ITEM_STAFF:	olevel = number_range( 15, 25 );	break;
	    case ITEM_ARMOR:	olevel = number_range(  5, 15 );	break;
	    case ITEM_WEAPON:	if ( pReset->command == 'G' )
	    			    olevel = number_range( 5, 15 );
				else
				    olevel = number_fuzzy( olevel );
		break;
	    }

	    newobj = create_object( pObjIndex, olevel );
	    if ( pReset->arg2 == WEAR_NONE )
		SET_BIT( newobj->extra_flags, ITEM_INVENTORY );
	}
	else
	    newobj = create_object( pObjIndex, number_fuzzy( olevel ) );

	obj_to_char( newobj, to_mob );
	if ( pReset->command == 'E' )
	    equip_char( to_mob, newobj, pReset->arg3 );

	sprintf( output, "%s (%d) has been loaded "
	    "%s of %s (%d) and added to resets.\n\r",
	    capitalize( pObjIndex->short_descr ),
	    pObjIndex->vnum,
	    flag_string( wear_loc_strings, pReset->arg3 ),
	    to_mob->short_descr,
	    to_mob->pIndexData->vnum );
	send_to_char(C_DEFAULT, output, ch );
    }
    else	/* Display Syntax */
    {
	send_to_char(C_DEFAULT, "REdit:  That mobile isn't here.\n\r", ch );
	return FALSE;
    }

    act(C_DEFAULT, "$n has created $p!", ch, newobj, NULL, TO_ROOM );
    return TRUE;
}

/*
 * Randomize Exits.
 * Added by Altrag.
 */
bool redit_rreset( CHAR_DATA *ch, char *argument )
{
	static const char * dir_name[6] =
	{ "North\0", "East\0", "South\0", "West\0", "Up\0", "Down\0" };

	char arg[MAX_STRING_LENGTH];
	char output[MAX_STRING_LENGTH];
	RESET_DATA *pReset;
	ROOM_INDEX_DATA *pRoom;
	int direc;
	
	EDIT_ROOM(ch, pRoom);

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char( C_DEFAULT, "Syntax: rreset <last-door>\n\r", ch );
		return FALSE;
	}
	
	if ( is_number( arg ) )
		direc = atoi(arg);
	else
	{
		for ( direc = 0; direc < 6; direc++ )
			if ( UPPER(arg[0]) == dir_name[direc][0] ) break;
	}

	if ( direc < 0 || direc > 5 )
	{
		send_to_char( C_DEFAULT, "That is not a direction.\n\r", ch );
		return FALSE;
	}
	
	pReset = new_reset();
	pReset->command   = 'R';
	pReset->arg1      = pRoom->vnum;
	pReset->arg2      = direc;
	add_reset( pRoom, pReset, 0/* Last slot*/ );

	sprintf( output, "Exits North (0) to %s (%d) randomized.\n\r",
			 dir_name[direc], direc );
	send_to_char( C_DEFAULT, output, ch );
	return TRUE;
}

/*
 * Object Editor Functions.
 */
void show_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *obj )
{
    char buf[MAX_STRING_LENGTH];

    switch( obj->item_type )
    {
	default:	/* No values. */
	    break;
            
	case ITEM_LIGHT:
            if ( obj->value[2] == -1 )
		sprintf( buf, "&b[&Cv2&b] &CLight&b:&w  Infinite[-1]\n\r" );
            else
		sprintf( buf, "&b[&Cv2&b] &CLight&b:&w  [%d]\n\r", obj->value[2] );
	    send_to_char(C_DEFAULT, buf, ch );
	    break;

	case ITEM_PORTAL:
	    sprintf( buf, "&b[&Cv0&b] &CDestination&b:&w         [%d vnum]\n\r", obj->value[0] );
            send_to_char( AT_BLUE, buf, ch );
            break;

        case ITEM_RUNE:
	    sprintf( buf, "&b[&Cv0&b] &CDestination&b:&w         [%d vnum]\n\r", obj->value[0] );
	    send_to_char( AT_BLUE, buf, ch );
	    break;

	case ITEM_WAND:
	case ITEM_LENSE:
	case ITEM_GUN:
	case ITEM_IMPLANTED:
	case ITEM_STAFF:
            if (obj->value[1] == -1 )
                sprintf( buf,
                    "&b[&Cv0&b] &CLevel&b:&w          [%d]\n\r"
                    "&b[&Cv1&b] &CCharges&b:&w        [Infinite(-1)]\n\r"
                    "&b[&Cv3&b] &CSpell&b:&w          %s\n\r",
                    obj->value[0],
		    obj->value[3] != -1 ? skill_table[obj->value[3]].name
		                    : "none" );
            else        
                sprintf( buf,
        	    "&b[&Cv0&b] &CLevel&b:&w          [%d]\n\r"
        	    "&b[&Cv1&b] &CCharges Total&b:&w  [%d]\n\r"
        	    "&b[&Cv2&b] &CCharges Left&b:&w   [%d]\n\r" 
        	    "&b[&Cv3&b] &CSpell&b:&w          %s\n\r",
        	    obj->value[0],
        	    obj->value[1],
        	    obj->value[2],
        	    obj->value[3] != -1 ? skill_table[obj->value[3]].name
		                    : "none" );
	    send_to_char(AT_BLUE, buf, ch );
	    break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
            sprintf( buf,
		"&b[&Cv0&b] &CLevel&b:&w  [%d]\n\r"
		"&b[&Cv1&b] &CSpell&b:&w  %s\n\r"
		"&b[&Cv2&b] &CSpell&b:&w  %s\n\r"
		"&b[&Cv3&b] &CSpell&b:&w  %s\n\r",
		obj->value[0],
		obj->value[1] != -1 ? skill_table[obj->value[1]].name
		                    : "none",
		obj->value[2] != -1 ? skill_table[obj->value[2]].name
                                    : "none",
		obj->value[3] != -1 ? skill_table[obj->value[3]].name
		                    : "none" );
	    send_to_char(C_DEFAULT, buf, ch );
	    break;

	case ITEM_BOOK:
	    sprintf( buf,
		"&b[&Cv0&b] &CGain&b:&w   [%d]\n\r"
		"&b[&Cv1&b] &CSpell&b:&w  %s\n\r"
		"&b[&Cv2&b] &CSpell&b:&w  %s\n\r"
		"&b[&Cv3&b] &CSpell&b:&w  %s\n\r",
		obj->value[0],
		obj->value[1] != -1 ? skill_table[obj->value[1]].name
		                    : "none",
		obj->value[2] != -1 ? skill_table[obj->value[2]].name
                                    : "none",
		obj->value[3] != -1 ? skill_table[obj->value[3]].name
		                    : "none" );
	    send_to_char(C_DEFAULT, buf, ch );
	    break;

	case ITEM_WEAPON:
		{
			char weap_buf[MAX_INPUT_LENGTH];
			char ammo_buf[MAX_INPUT_LENGTH];
            sprintf( buf,
        	"&b[&Cv1&b] &CDamage minimum&b:&w [%d]\n\r"
        	"&b[&Cv2&b] &CDamage maximum&b:&w [%d]\n\r"
		"&b[&Cv3&b] &CType&b:&w           %s\n\r"
		"&b[&Cv4&b] &CAmmo&b:&w           %s\n\r",
		obj->value[1],
		obj->value[2],
		strcpy(weap_buf, flag_string( weapon_flags, obj->value[3] ) ),
		strcpy(ammo_buf, flag_string( ammo_flags  , obj->value[0] ) ) );
		}
	    send_to_char(C_DEFAULT, buf, ch );
	    break;

	case ITEM_ARMOR:
	    sprintf( buf,
		"&b[&Cv0&b] &CArmor class&b: &w[%d]\n\r", obj->value[0] );
	    send_to_char(C_DEFAULT, buf, ch );
	    break;

	case ITEM_NOTEBOARD:
	    sprintf( buf,
		"&b[&Cv0&b] &CDecoder item&b:&w        [%d] %s\n\r"
		"&b[&Cv1&b] &CMinimum read level&b:&w  [%d]\n\r"
		"&b[&Cv2&b] &CMinimum write level&b:&w [%d]\n\r",
		obj->value[0], 
		get_obj_index(obj->value[0])
		    ? get_obj_index(obj->value[0])->short_descr
		    : "none",
		obj->value[1],
		obj->value[2] );
	    send_to_char(C_DEFAULT, buf, ch );
	    break;

	case ITEM_CONTAINER:
	    sprintf( buf,
		"&b[&Cv0&b] &CWeight&b:&w [%d kg]\n\r"
		"&b[&Cv1&b] &CFlags&b:&w  [%s]\n\r"
		"&b[&Cv2&b] &CKey&b:&w    %s [%d]\n\r",
		obj->value[0],
		flag_string( container_flags, obj->value[1] ),
                get_obj_index(obj->value[2])
                    ? get_obj_index(obj->value[2])->short_descr
                    : "none", obj->value[2]);
	    send_to_char(C_DEFAULT, buf, ch );
	    break;

	case ITEM_ARROW:
	case ITEM_BULLET:
	case ITEM_BOLT:
	    sprintf( buf,
		"&b[&Cv0&b] &CStack&b:&w    [%d shots]\n\r"
		"&b[&Cv1&b] &CDamClass&b:&w [%s]\n\r"
		"&b[&Cv2&b] &CMin Dmg&b:&w  [%d]\n\r"
		"&b[&Cv3&b] &CMax Dmg&b:&w  [%d]\n\r",
		obj->value[0],
		flag_string( damage_flags, obj->value[1] ),
		obj->value[2],
		obj->value[3] );
	    send_to_char(C_DEFAULT, buf, ch );
	    break;
	case ITEM_DRINK_CON:
	    sprintf( buf,
	        "&b[&Cv0&b] &CLiquid Total&b:&w [%d]\n\r"
	        "&b[&Cv1&b] &CLiquid Left&b:&w  [%d]\n\r"
	        "&b[&Cv2&b] &CLiquid&b:&w       %s\n\r"
	        "&b[&Cv3&b] &CPoisoned&b:&w     %s\n\r",
	        obj->value[0],
	        obj->value[1],
	        flag_string( liquid_flags, obj->value[2] ),
	        obj->value[3] != 0 ? "Yes" : "No" );
	    send_to_char(C_DEFAULT, buf, ch );
	    break;

	case ITEM_BLOOD:
	    sprintf( buf,
	        "&b[&Cv0&b] &CBlood Total&b:&w [%d]\n\r"
	        "&b[v&C1&b] &CBlood Left&b:&w  [%d]\n\r", 
	        obj->value[0],
	        obj->value[1] );
	    send_to_char(C_DEFAULT, buf, ch );
	    break;

	case ITEM_FOOD:
	    sprintf( buf,
		"&b[&Cv0&b] &CFood hours&b:&w [%d]\n\r"
		"&b[&Cv3&b] &CPoisoned&b:&w   %s\n\r",
		obj->value[0],
		obj->value[3] != 0 ? "Yes" : "No" );
	    send_to_char(C_DEFAULT, buf, ch );
	    break;

	case ITEM_MONEY:
            sprintf( buf, "&b[&Cv0&b] &CGold&b:&w   [%d]\n\r", obj->value[0] );
	    send_to_char(C_DEFAULT, buf, ch );
	    break;
    }

    return;
}



bool set_ac_type ( CHAR_DATA *ch, char *argument )
{    

    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(AT_WHITE, "Syntax:  ac_type [type #]\n\r", ch );
	send_to_char(AT_WHITE, "         type #1 = oload item.\n\r", ch );
	send_to_char(AT_WHITE, "              #2 = mload mob.\n\r", ch );
	send_to_char(AT_WHITE, "              #3 = transfer character.\n\r", ch );
	send_to_char(AT_WHITE, "              #4 = item morph.\n\r", ch );
	send_to_char(AT_WHITE, "              #5 = item cast spell.\n\r", ch );
	return FALSE;
    }

    pObj->ac_type = atoi( argument );

    send_to_char(C_DEFAULT, "Invoke type set.\n\r", ch);
    return TRUE;

}

bool set_ac_vnum ( CHAR_DATA *ch, char *argument )
{    

    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(AT_WHITE, "Syntax:  ac_vnum [ # ]\n\r", ch );
	return FALSE;
    }

    pObj->ac_vnum = atoi( argument );

    send_to_char(C_DEFAULT, "Invoke vnum set.\n\r", ch);
    return TRUE;

}

bool set_ac_v1 ( CHAR_DATA *ch, char *argument )
{    

    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(AT_WHITE, "Syntax:  ac_v1 [ current charges ]\n\r", ch );
	return FALSE;
    }

    pObj->ac_charge[0] = atoi( argument );

    send_to_char(C_DEFAULT, "Current charge set.\n\r", ch);
    return TRUE;

}

bool set_ac_v2 ( CHAR_DATA *ch, char *argument )
{    

    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(AT_WHITE, "Syntax:  ac_v2 [ max charges (-1 unlimited) ]\n\r", ch );
	return FALSE;
    }

    pObj->ac_charge[1] = atoi( argument );

    send_to_char(C_DEFAULT, "Max Charge set.\n\r", ch);
    return TRUE;

}


bool set_ac_setspell ( CHAR_DATA *ch, char *argument )
{    

    OBJ_INDEX_DATA *pObj;
    int            spn;
    
    EDIT_OBJ(ch, pObj);

    spn = skill_lookup( argument );
    
    if ( ( argument[0] == '\0' ) || ( spn == -1 ) )
    {
	send_to_char(AT_WHITE, "Syntax:  ac_setspell [ valid spell name ]\n\r", ch );
	return FALSE;
    }
    
    pObj->ac_spell = skill_table[spn].name;

    send_to_char(C_DEFAULT, "Spell set.\n\r", ch);
    return TRUE;

}

bool set_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, int value_num, char *argument)
{
    switch( pObj->item_type )
    {
        default:
            break;
            
        case ITEM_PORTAL:
            switch ( value_num )
            {
            default: return FALSE;
            case 0:
               send_to_char(C_DEFAULT, "DESTINATION SET.\n\r\n\r", ch );
               pObj->value[0] = atoi( argument );
               break;
            }
            break; 

        case ITEM_RUNE:
            switch ( value_num )
            {
            default: return FALSE;
            case 0:
               send_to_char(C_DEFAULT, "DESTINATION SET.\n\r\n\r", ch );
               pObj->value[0] = atoi( argument );
               break;
            }
            break;
              
        case ITEM_LIGHT:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_LIGHT" );
	            return FALSE;
	        case 2:
	            send_to_char(C_DEFAULT, "HOURS OF LIGHT SET.\n\r\n\r", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	    }
            break;

        case ITEM_WAND:
	case ITEM_GUN:
	case ITEM_IMPLANTED:
        case ITEM_LENSE:
        case ITEM_STAFF:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_STAFF_WAND" );
	            return FALSE;
	        case 0:
	            send_to_char(C_DEFAULT, "SPELL LEVEL SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char(C_DEFAULT, "TOTAL NUMBER OF CHARGES SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char(C_DEFAULT, "CURRENT NUMBER OF CHARGES SET.\n\r\n\r", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char(C_DEFAULT, "SPELL TYPE SET.\n\r", ch );
	            pObj->value[3] = skill_lookup( argument );
	            break;
	    }
            break;

        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_SCROLL_POTION_PILL" );
	            return FALSE;
	        case 0:
	            send_to_char(C_DEFAULT, "SPELL LEVEL SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char(C_DEFAULT, "SPELL TYPE 1 SET.\n\r\n\r", ch );
	            pObj->value[1] = skill_lookup( argument );
	            break;
	        case 2:
	            send_to_char(C_DEFAULT, "SPELL TYPE 2 SET.\n\r\n\r", ch );
	            pObj->value[2] = skill_lookup( argument );
	            break;
	        case 3:
	            send_to_char(C_DEFAULT, "SPELL TYPE 3 SET.\n\r\n\r", ch );
	            pObj->value[3] = skill_lookup( argument );
	            break;
	    }
            break;

        case ITEM_BOOK:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_BOOK" );
	            return FALSE;
	        case 0:
	            send_to_char(C_DEFAULT, "SKPELL GAIN SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char(C_DEFAULT, "SPELL TYPE 1 SET.\n\r\n\r", ch );
	            pObj->value[1] = skill_lookup( argument );
	            break;
	        case 2:
	            send_to_char(C_DEFAULT, "SPELL TYPE 2 SET.\n\r\n\r", ch );
	            pObj->value[2] = skill_lookup( argument );
	            break;
	        case 3:
	            send_to_char(C_DEFAULT, "SPELL TYPE 3 SET.\n\r\n\r", ch );
	            pObj->value[3] = skill_lookup( argument );
	            break;
	    }
            break;

	case ITEM_BULLET:
	case ITEM_BOLT:
	case ITEM_ARROW:
	    switch ( value_num )
	    {
		default:
		    do_help( ch, "ITEM_AMMUNITION" );
		    return FALSE;
		case 0:
		    send_to_char( C_DEFAULT, "STACK SIZE SET.\n\r", ch );
		    pObj->value[0] = atoi( argument );
		    break;
		case 1:
		    send_to_char( C_DEFAULT, "DAMCLASS SET.\n\r", ch );
		    pObj->value[1] = flag_value( damage_flags, argument );
		    break;
		case 2:
		    send_to_char( C_DEFAULT, "MINIMUM DAMAGE SET.\n\r", ch );
		    pObj->value[2] = atoi( argument );
		    break;
		case 3:
		    send_to_char( C_DEFAULT, "MAXIMUM DAMAGE SET.\n\r", ch );
		    pObj->value[3] = atoi( argument );
		    break;
	    }
	    break;
        case ITEM_WEAPON:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_WEAPON" );
	            return FALSE;
	        case 1:
	            send_to_char(C_DEFAULT, "MINIMUM DAMAGE SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char(C_DEFAULT, "MAXIMUM DAMAGE SET.\n\r\n\r", ch );
	            pObj->value[2] = atoi( argument );
	            break;
	        case 3:
	            send_to_char(C_DEFAULT, "WEAPON TYPE SET.\n\r\n\r", ch );
	            pObj->value[3] = flag_value( weapon_flags, argument );
		    if (pObj->value[3] != flag_value( weapon_flags, "shot" ))
			pObj->value[0] = 0;
	            break;
		case 4:
		    if (pObj->value[3] != flag_value( weapon_flags, "shot" ) )
			send_to_char(C_DEFAULT, "Only weapons that shoot need ammo!\n\r\n\r", ch);
 		    else
		    {
		    	send_to_char(C_DEFAULT, "AMMO TYPE SET.\n\r\n\r", ch);
			pObj->value[0] = flag_value( ammo_flags, argument );
		    }
		    break;
	    }
            break;

	case ITEM_ARMOR:
	    switch( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_ARMOR" );
		    return FALSE;
	        case 0:
		    send_to_char(C_DEFAULT, "ARMOR CLASS SET.\n\r\n\r", ch );
		    pObj->value[0] = atoi( argument );
		    break;
	    }
	    break;

	case ITEM_NOTEBOARD:
	    switch ( value_num )
	    {
	        default:
	            do_help( ch, "ITEM_NOTEBOARD" );
		    return FALSE;
		case 0:
		    send_to_char(C_DEFAULT, "DECODER VNUM SET.\n\r\n\r", ch );
		    pObj->value[0] = atoi( argument );
		    break;
		case 1:
		    if ( atoi(argument) > get_trust(ch) )
		    {
		      send_to_char(C_DEFAULT, "Limited by your trust.\n\r",ch);
		      return FALSE;
		    }
		    send_to_char(C_DEFAULT, "MINIMUM READ LEVEL SET.\n\r\n\r",ch);
		    pObj->value[1] = atoi( argument );
		    break;
		case 2:
		    if ( atoi(argument) > get_trust(ch) )
		    {
		      send_to_char(C_DEFAULT, "Limited by your trust.\n\r",ch);
		      return FALSE;
		    }
		    send_to_char(C_DEFAULT, "MINIMUM WRITE LEVEL SET.\n\r\n\r",ch);
		    pObj->value[2] = atoi( argument );
		    break;
	    }
	    break;

        case ITEM_CONTAINER:
	    switch ( value_num )
	    {
		int value;
		
	        default:
		    do_help( ch, "ITEM_CONTAINER" );
	            return FALSE;
		case 0:
	            send_to_char(C_DEFAULT, "WEIGHT CAPACITY SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
		case 1:
	            if ( ( value = flag_value( container_flags, argument ) )
	              != NO_FLAG )
	        	TOGGLE_BIT(pObj->value[1], value);
		    else
		    {
			do_help ( ch, "ITEM_CONTAINER" );
			return FALSE;
		    }
	            send_to_char(C_DEFAULT, "CONTAINER TYPE SET.\n\r\n\r", ch );
	            break;
		case 2:
		    if ( atoi(argument) != 0 )
		    {
			if ( !get_obj_index( atoi( argument ) ) )
			{
			    send_to_char(C_DEFAULT, "THERE IS NO SUCH ITEM.\n\r\n\r", ch );
			    return FALSE;
			}

			if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
			{
			    send_to_char(C_DEFAULT, "THAT ITEM IS NOT A KEY.\n\r\n\r", ch );
			    return FALSE;
			}
		    }
		    send_to_char(C_DEFAULT, "CONTAINER KEY SET.\n\r\n\r", ch );
		    pObj->value[2] = atoi( argument );
		    break;
	    }
	    break;

	case ITEM_DRINK_CON:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_DRINK" );
	            return FALSE;
	        case 0:
	            send_to_char(C_DEFAULT, "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char(C_DEFAULT, "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	        case 2:
	            send_to_char(C_DEFAULT, "LIQUID TYPE SET.\n\r\n\r", ch );
	            pObj->value[2] = flag_value( liquid_flags, argument );
	            break;
	        case 3:
	            send_to_char(C_DEFAULT, "POISON VALUE TOGGLED.\n\r\n\r", ch );
	            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	    }
            break;

	case ITEM_BLOOD:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_DRINK" );
	            return FALSE;
	        case 0:
	            send_to_char(C_DEFAULT, "MAXIMUM AMOUT OF BLOOD SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 1:
	            send_to_char(C_DEFAULT, "CURRENT AMOUNT OF BLOOD SET.\n\r\n\r", ch );
	            pObj->value[1] = atoi( argument );
	            break;
	    }
            break;

	case ITEM_FOOD:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_FOOD" );
	            return FALSE;
	        case 0:
	            send_to_char(C_DEFAULT, "HOURS OF FOOD SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	        case 3:
	            send_to_char(C_DEFAULT, "POISON VALUE TOGGLED.\n\r\n\r", ch );
	            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
	            break;
	    }
            break;

	case ITEM_MONEY:
	    switch ( value_num )
	    {
	        default:
		    do_help( ch, "ITEM_MONEY" );
	            return FALSE;
	        case 0:
	            send_to_char(C_DEFAULT, "GOLD AMOUNT SET.\n\r\n\r", ch );
	            pObj->value[0] = atoi( argument );
	            break;
	    }
            break;
    }

    show_obj_values( ch, pObj );

    return TRUE;
}



bool oedit_show( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    int cnt;

    EDIT_OBJ(ch, pObj);

    sprintf( buf, "&b[&CName&b]:&w           [%s]\n\r&b[&CArea&b]:&z           [%5d] %s\n\r",
	pObj->name,
	!pObj->area ? -1        : pObj->area->vnum,
	!pObj->area ? "No Area" : pObj->area->name );
    send_to_char(C_DEFAULT, buf, ch );


    sprintf( buf, "&b[&CVnum&b]:&z           [%5d]\n\r&b[&CType&b]:&w           [%s]\n\r",
	pObj->vnum,
	flag_string( type_flags, pObj->item_type ) );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CWear flags&b]:&w     [%s]\n\r",
	flag_string( wear_flags, pObj->wear_flags ) );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CExtra flags&b]:&w    [%s]\n\r",
	flag_string( extra_flags, pObj->extra_flags ) );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CExtra2 flags&b]:&w   [%s]\n\r",
	flag_string( extra_flags2, pObj->extra_flags2 ) );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CExtra3 flags&b]:&w   [%s]\n\r",
        flag_string( extra_flags3, pObj->extra_flags3 ) );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CExtra4 flags&b]:&w   [%s]\n\r",
        flag_string( extra_flags4, pObj->extra_flags4 ) );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CLevel&b]:&w          [%d]\n\r", pObj->level );
    send_to_char(C_DEFAULT, buf, ch);

    sprintf( buf, "&b[&CDurability_Cur&b]:&w [%d]\n\r", pObj->durability_cur );
    send_to_char(C_DEFAULT, buf, ch);

    sprintf( buf, "&b[&CDurability_Max&b]:&w [%d]\n\r", pObj->durability_max );
    send_to_char(C_DEFAULT, buf, ch ); 

    sprintf( buf, "&b[&CWeight&b]:&w         [%d]\n\r&b[&CCost&b]:&w           [%d]\n\r",
	pObj->weight, pObj->cost );
    send_to_char(C_DEFAULT, buf, ch );


    if ( pObj->join )
    {
      sprintf( buf, "&b[&CJoins to create&b]:&w         [%d]\n\r", pObj->join );
      send_to_char( C_DEFAULT, buf, ch );
    }

    if ( pObj->sep_one )
    {
      sprintf( buf, "&b[&CFirst seperated vnum&b]:&w    [%d]\n\r", pObj->sep_one );
      send_to_char( C_DEFAULT, buf, ch );
    }

    if ( pObj->sep_two )
    {
      sprintf( buf, "&b[&CSecond seperated vnum&b]:&w   [%d]\n\r", pObj->sep_two );
      send_to_char( C_DEFAULT, buf, ch );
    }

    sprintf( buf, "&b[&CInvoke Type&b]:&w [%d]   &b[&CInvoke Vnum&b]:&w [%d]\n\r", 
             pObj->ac_type, pObj->ac_vnum );
    send_to_char(C_DEFAULT, buf, ch );

    if ( pObj->ac_charge[1] == -1 )
        sprintf( buf, "&wInvoke is permanent.\n\r" );
    else
        sprintf( buf, "&b[&CInvoke charges&b]:&w [%d/%d]\n\r",
                 pObj->ac_charge[0], pObj->ac_charge[1] );
    send_to_char( C_DEFAULT, buf, ch );

    if ( ( pObj->ac_type == 5 ) && ( pObj->ac_spell != '\0' ) )
        sprintf( buf, "&b[&CInvoke Spell&b]:&w     [%s]\n\r", pObj->ac_spell );
    else sprintf( buf,"&b[&CInvoke Spell&b]:&w     [!NONE!]\n\r" );
    send_to_char( C_DEFAULT, buf, ch );

    if ( pObj->extra_descr )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char(C_DEFAULT, "&b[&CEx desc kwd&b]:&w ", ch );

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    send_to_char(C_DEFAULT, "[", ch );
	    send_to_char(C_DEFAULT, ed->keyword, ch );
	    send_to_char(C_DEFAULT, "]", ch );
	}

	send_to_char(C_DEFAULT, "\n\r", ch );
    }

    sprintf( buf, "&b[&CShort desc&b]:&w %s\n\r&b[&CLong desc&b]:&w\n\r      %s\n\r",
	pObj->short_descr, pObj->description );
    send_to_char(C_DEFAULT, buf, ch );

    for ( cnt = 0, paf = pObj->affected; paf; paf = paf->next )
    {
	if ( cnt == 0 )
	{
	    send_to_char(C_DEFAULT, "&b[&CNumber&b] [&CModifier&b] [&CAffects&b]\n\r", ch );
	    send_to_char(C_DEFAULT, " ------   --------   ------- \n\r", ch );
	}
	sprintf( buf, "[ %4d ]  %-8d   %s\n\r", cnt,
	    paf->modifier,
	    flag_string( apply_flags, paf->location ) );
	send_to_char(C_DEFAULT, buf, ch );
	cnt++;
    }

    show_obj_values( ch, pObj );

    return FALSE;
}


/*
 * Need to issue warning if flag isn't valid.
 */
bool oedit_addaffect( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, loc );
    one_argument( argument, mod );

    if ( loc[0] == '\0' || mod[0] == '\0' || !is_number( mod ) )
    {
	send_to_char(C_DEFAULT, "Syntax:  addaffect [location] [#mod]\n\r", ch );
	return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   flag_value( apply_flags, loc );
    pAf->modifier   =   atoi( mod );
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char(C_DEFAULT, "Affect added.\n\r", ch);
    return TRUE;
}



/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
bool oedit_delaffect( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    AFFECT_DATA *pAf_next;
    char affect[MAX_STRING_LENGTH];
    int  value;
    int  cnt = 0;

    EDIT_OBJ(ch, pObj);

    one_argument( argument, affect );

    if ( !is_number( affect ) || affect[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  delaffect [#affect]\n\r", ch );
	return FALSE;
    }

    value = atoi( affect );

    if ( value < 0 )
    {
	send_to_char(C_DEFAULT, "Only non-negative affect-numbers allowed.\n\r", ch );
	return FALSE;
    }

    if ( !( pAf = pObj->affected ) )
    {
	send_to_char(C_DEFAULT, "OEdit:  Non-existant affect.\n\r", ch );
	return FALSE;
    }

    if( value == 0 )	/* First case: Remove first affect */
    {
	pAf = pObj->affected;
	pObj->affected = pAf->next;
	free_affect( pAf );
    }
    else		/* Affect to remove is not the first */
    {
	while ( ( pAf_next = pAf->next ) && ( ++cnt < value ) )
	     pAf = pAf_next;

	if( pAf_next )		/* See if it's the next affect */
	{
	    pAf->next = pAf_next->next;
	    free_affect( pAf_next );
	}
	else                                 /* Doesn't exist */
	{
	     send_to_char(C_DEFAULT, "No such affect.\n\r", ch );
	     return FALSE;
	}
    }

    send_to_char(C_DEFAULT, "Affect removed.\n\r", ch);
    return TRUE;
}



bool oedit_name( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  name [string]\n\r", ch );
	return FALSE;
    }

    free_string( pObj->name );
    pObj->name = str_dup( argument );

    send_to_char(C_DEFAULT, "Name set.\n\r", ch);
    return TRUE;
}



bool oedit_short( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  short [string]\n\r", ch );
	return FALSE;
    }

    free_string( pObj->short_descr );
    pObj->short_descr = str_dup( argument );
/*    pObj->short_descr[0] = LOWER( pObj->short_descr[0] );*/

    send_to_char(C_DEFAULT, "Short description set.\n\r", ch);
    return TRUE;
}



bool oedit_long( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  long [string]\n\r", ch );
	return FALSE;
    }
        
    free_string( pObj->description );
    pObj->description = str_dup( argument );
    pObj->description[0] = UPPER( pObj->description[0] );

    send_to_char(C_DEFAULT, "Long description set.\n\r", ch);
    return TRUE;
}



bool set_value( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, char *argument, int value )
{
    if ( argument[0] == '\0' )
    {
	set_obj_values( ch, pObj, -1, '\0' );
	return FALSE;
    }

    if ( set_obj_values( ch, pObj, value, argument ) )
	return TRUE;

    return FALSE;
}

/* oedit_join is for setting the vnum of which an object can be joined to */
bool oedit_join( CHAR_DATA *ch, char *argument )
{
   OBJ_INDEX_DATA *pObj;
   char arg[MAX_STRING_LENGTH];
   int value = 0;
   
   EDIT_OBJ( ch, pObj );
   
   argument = one_argument( argument, arg );
   
   if ( arg[0] == '\0' || !is_number( arg ) )
   {
      send_to_char( AT_WHITE, " &pSyntax: ojoin [vnum]\n\r", ch );
      return FALSE;
   }
   
   value = atoi( arg );
   
   if ( value < 0 || value > 33000 )
   {
      send_to_char(AT_WHITE, "Invalid vnum.\n\r", ch );
      return FALSE;
   }
   
   pObj->join = value; 
   send_to_char( AT_WHITE, "Ok.\n\r", ch );
   return TRUE;  
}
   
/* oedit_sepone is for setting the first vnum which an object can seperate
 * into.
 */
bool oedit_sepone( CHAR_DATA *ch, char *argument )
{
  OBJ_INDEX_DATA *pObj;
  char arg[MAX_STRING_LENGTH];
  int value = 0;

  EDIT_OBJ( ch, pObj );
  
  argument = one_argument( argument, arg );
  
  if ( arg[0] == '\0' || !is_number( arg ) )
  {
     send_to_char( AT_WHITE, " &pSyntax:  osepone [vnum]\n\r", ch );
     return FALSE;
  }
  
  value = atoi( arg );
  if ( value < 0 || value > 33000 )
  {
    send_to_char( AT_WHITE, "Invalid vnum.\n\r", ch );
    return FALSE;
  }
  pObj->sep_one = value;
  send_to_char( AT_WHITE, "Ok.\n\r", ch );
  return TRUE;
}

/* oedit_septwo is for setting the second vnum which an object splits into */
bool oedit_septwo( CHAR_DATA *ch, char *argument )
{
  OBJ_INDEX_DATA *pObj;
  char arg[MAX_STRING_LENGTH];
  int value = 0;
  
  EDIT_OBJ( ch, pObj );
  
  argument = one_argument( argument, arg );
  
  if ( arg[0] == '\0' || !is_number( arg ) )
  {
     send_to_char( AT_WHITE, " &pSyntax: oseptwo [vnum]\n\r", ch );
     return FALSE;
  }
  
  value = atoi( arg );
  if ( value < 0 || value > 33000 )
  {
    send_to_char( AT_WHITE, "Invalid vnum.\n\r", ch );
    return FALSE;
  }
  pObj->sep_two = value;
  send_to_char( AT_WHITE, "Ok.\n\r", ch );
  return TRUE;
}
/*****************************************************************************
 Name:		oedit_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below.
 ****************************************************************************/
bool oedit_values( CHAR_DATA *ch, char *argument, int value )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( set_value( ch, pObj, argument, value ) )
        return TRUE;

    return FALSE;
}


bool oedit_value0( CHAR_DATA *ch, char *argument )
{
    if ( oedit_values( ch, argument, 0 ) )
        return TRUE;

    return FALSE;
}



bool oedit_value1( CHAR_DATA *ch, char *argument )
{
    if ( oedit_values( ch, argument, 1 ) )
        return TRUE;

    return FALSE;
}



bool oedit_value2( CHAR_DATA *ch, char *argument )
{
    if ( oedit_values( ch, argument, 2 ) )
        return TRUE;

    return FALSE;
}



bool oedit_value3( CHAR_DATA *ch, char *argument )
{
    if ( oedit_values( ch, argument, 3 ) )
        return TRUE;

    return FALSE;
}

bool oedit_value4( CHAR_DATA *ch, char *argument )
{
    if ( oedit_values( ch, argument, 4 ) )
        return TRUE;

    return FALSE;
}

bool oedit_weight( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(C_DEFAULT, "Syntax:  weight [number]\n\r", ch );
	return FALSE;
    }

    pObj->weight = atoi( argument );

    send_to_char(C_DEFAULT, "Weight set.\n\r", ch);
    return TRUE;
}



bool oedit_cost( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(C_DEFAULT, "Syntax:  cost [number]\n\r", ch );
	return FALSE;
    }

    pObj->cost = atoi( argument );

    send_to_char(C_DEFAULT, "Cost set.\n\r", ch);
    return TRUE;
}

bool oedit_level( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(C_DEFAULT, "Syntax:  level [number]\n\r", ch );
	return FALSE;
    }

    pObj->level = atoi( argument );

    send_to_char(C_DEFAULT, "Level set.\n\r", ch);
    return TRUE;
}

bool oedit_durability_cur( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(C_DEFAULT, "Syntax:  durability_cur [number]\n\r", ch );
	return FALSE;
    }

    pObj->durability_cur = atoi( argument );

    send_to_char(C_DEFAULT, "Current Durability set.\n\r", ch);
    return TRUE;
}

bool oedit_durability_max( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(C_DEFAULT, "Syntax:  durability_max [number]\n\r", ch );
	return FALSE;
    }

    pObj->durability_max = atoi( argument );

    send_to_char(C_DEFAULT, "Max Durability set.\n\r", ch);
    return TRUE;
}

bool oedit_create( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    int  value;
    int  iHash;

    value = atoi( argument );

    /* OLC 1.1b */
    if ( argument[0] == '\0' || value <= 0 || value >= INT_MAX )
    {
	char output[MAX_STRING_LENGTH];

	sprintf( output, "Syntax:  create [0 < vnum < %d]\n\r", INT_MAX );
	send_to_char(C_DEFAULT, output, ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );
    if ( !pArea )
    {
	send_to_char(C_DEFAULT, "OEdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char(C_DEFAULT, "OEdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    if ( get_obj_index( value ) )
    {
	send_to_char(C_DEFAULT, "OEdit:  Object vnum already exists.\n\r", ch );
	return FALSE;
    }
        
    pObj			= new_obj_index();
    pObj->vnum			= value;
    pObj->area			= pArea;
    pObj->ac_spell              = "reserved";
    pObj->durability_cur	= 100;
    pObj->durability_max	= 100;
    if ( value > top_vnum_obj )
	top_vnum_obj = value;

    iHash			= value % MAX_KEY_HASH;
    pObj->next			= obj_index_hash[iHash];
    obj_index_hash[iHash]	= pObj;
    ch->desc->pEdit		= (void *)pObj;

    send_to_char(C_DEFAULT, "Object Created.\n\r", ch );
    return TRUE;
}



bool oedit_ed( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, command );
    one_argument( argument, keyword );

    if ( command[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  ed add [keyword]\n\r", ch );
	send_to_char(C_DEFAULT, "         ed delete [keyword]\n\r", ch );
	send_to_char(C_DEFAULT, "         ed edit [keyword]\n\r", ch );
	send_to_char(C_DEFAULT, "         ed format [keyword]\n\r", ch );
	return FALSE;
    }

    if ( !str_cmp( command, "add" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char(C_DEFAULT, "Syntax:  ed add [keyword]\n\r", ch );
	    return FALSE;
	}

	ed                  =   new_extra_descr();
	ed->keyword         =   str_dup( keyword );
	ed->next            =   pObj->extra_descr;
	pObj->extra_descr   =   ed;

	string_append( ch, &ed->description );

	return TRUE;
    }

    if ( !str_cmp( command, "edit" ) )
    {
	if ( keyword[0] == '\0' )
	{
	    send_to_char(C_DEFAULT, "Syntax:  ed edit [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	}

	if ( !ed )
	{
	    send_to_char(C_DEFAULT, "OEdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	string_append( ch, &ed->description );

	return TRUE;
    }

    if ( !str_cmp( command, "delete" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )
	{
	    send_to_char(C_DEFAULT, "Syntax:  ed delete [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
	    send_to_char(C_DEFAULT, "OEdit:  Extra description keyword not found.\n\r", ch );
	    return FALSE;
	}

	if ( !ped )
	    pObj->extra_descr = ed->next;
	else
	    ped->next = ed->next;

	free_extra_descr( ed );

	send_to_char(C_DEFAULT, "Extra description deleted.\n\r", ch );
	return TRUE;
    }


    if ( !str_cmp( command, "format" ) )
    {
	EXTRA_DESCR_DATA *ped = NULL;

	if ( keyword[0] == '\0' )
	{
	    send_to_char(C_DEFAULT, "Syntax:  ed format [keyword]\n\r", ch );
	    return FALSE;
	}

	for ( ed = pObj->extra_descr; ed; ed = ed->next )
	{
	    if ( is_name( keyword, ed->keyword ) )
		break;
	    ped = ed;
	}

	if ( !ed )
	{
                send_to_char(C_DEFAULT, "OEdit:  Extra description keyword not found.\n\r", ch );
                return FALSE;
	}

	/* OLC 1.1b */
	if ( strlen(ed->description) >= (MAX_STRING_LENGTH - 4) )
	{
	    send_to_char(C_DEFAULT, "String too long to be formatted.\n\r", ch );
	    return FALSE;
	}

	ed->description = format_string( ed->description );

	send_to_char(C_DEFAULT, "Extra description formatted.\n\r", ch );
	return TRUE;
    }

    oedit_ed( ch, "" );
    return FALSE;
}


/*
 * Clan Editor Functions.
 */
bool cedit_show( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;
    char buf[MAX_STRING_LENGTH];

    EDIT_CLAN(ch, pClan);

    sprintf( buf, "Name:        [%s]\n\rDeity:       [%s]\n\r",
	pClan->name, pClan->deity );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "Clan:        [%3d]\n\r",
	pClan->vnum );
    send_to_char(C_DEFAULT, buf, ch );
    sprintf( buf, "Recall:      [%5d] %s\n\r", pClan->recall,
	get_room_index( pClan->recall )
	? get_room_index( pClan->recall )->name : "none" );
    send_to_char(C_DEFAULT, buf, ch );
    sprintf( buf, "Pkill:       [%3s]\n\r", pClan->pkill ? "YES" : "NO" );
    send_to_char(C_DEFAULT, buf, ch );
    sprintf( buf,
	"Members:     [%3d]\n\r",
	pClan->members );
    send_to_char(C_DEFAULT, buf, ch );
    sprintf( buf, "Rank 4:      [%10s]\n\rRank 3:      [%10s]\n\r", pClan->champ, pClan->leader );
    send_to_char( C_DEFAULT, buf, ch );
    sprintf( buf, "Rank 2:      [%10s]\n\rRank 1:      [%10s]\n\r", pClan->first, pClan->second );
    send_to_char( C_DEFAULT, buf, ch );  
    sprintf( buf,
        "Object:      [%5d], [%5d], [%5d]\n\r",
        pClan->obj_vnum_1, pClan->obj_vnum_2, pClan->obj_vnum_3  );
    send_to_char(C_DEFAULT, buf, ch);
    sprintf( buf, "Description:\n\r%s\n\r", pClan->description );
    send_to_char(C_DEFAULT, buf, ch );
    
    return FALSE;
}



bool cedit_create( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;
    int  value;

    value = atoi( argument );

    /* OLC 1.1b */
    if ( argument[0] == '\0' || value <= 0 || value >= MAX_CLAN )
    {
	char output[MAX_STRING_LENGTH];

	sprintf( output, "Syntax:  cedit create [1 < vnum < %d]\n\r",
		 MAX_CLAN );
	send_to_char(C_DEFAULT, output, ch );
	return FALSE;
    }


    if ( get_clan_index( value ) )
    {
	send_to_char(C_DEFAULT, "CEdit:  Clan vnum already exists.\n\r", ch );
	return FALSE;
    }

    pClan			= new_clan();
    pClan->vnum			= value;
/*    iHash			= value % MAX_KEY_HASH;
    pClan->next			= clan_index_hdata;
    clan_index_hash[iHash]	= pClan;*/
/*    if ( !clan_first )
      clan_first = pClan;
    if ( clan_last )
      clan_last->next = pClan;
    clan_last->next             = pClan;
    clan_last                   = pClan;*/
    clan_sort(pClan);
    ch->desc->pEdit		= (void *)pClan;

    send_to_char(C_DEFAULT, "Clan Created.\n\r", ch );
    return TRUE;
}

bool cedit_members( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;

    EDIT_CLAN(ch, pClan);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(C_DEFAULT, "Syntax:  members [number]\n\r", ch );
	return FALSE;
    }

    pClan->members = atoi( argument );

    send_to_char(C_DEFAULT, "Members set.\n\r", ch);
    return TRUE;
}


bool cedit_name( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;

    EDIT_CLAN(ch, pClan);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  name [string]\n\r", ch );
	return FALSE;
    }

    free_string( pClan->name );
    pClan->name = str_dup( argument );

    send_to_char(C_DEFAULT, "Name set.\n\r", ch);
    return TRUE;
}

bool cedit_deity( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;

    EDIT_CLAN(ch, pClan);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  deity [string]\n\r", ch );
	return FALSE;
    }

    free_string( pClan->deity );
    pClan->deity = str_dup( argument );

    send_to_char(C_DEFAULT, "Deity set.\n\r", ch);
    return TRUE;
}

bool cedit_pkill( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;

    EDIT_CLAN(ch, pClan);

    pClan->pkill = !pClan->pkill;
    
    if (pClan->pkill)
        send_to_char(C_DEFAULT, "Clan switched to Pkill.\n\r", ch);
    else
        send_to_char(C_DEFAULT, "Clan switched to Peace.\n\r", ch);
    
    return TRUE;
}

bool cedit_object( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;
    char number[MAX_INPUT_LENGTH];
    char onum[MAX_INPUT_LENGTH];
    int  value;
    int  vnum;

    argument = one_argument( argument, number );
    argument = one_argument( argument, onum );

    EDIT_CLAN(ch, pClan);

    if ( number[0] == '\0' )
    {
     send_to_char( C_DEFAULT, "Syntax: object <1/2/3> <vnum>\n\r", ch );
     return FALSE;
    }
    
    value = atoi( number );
    vnum  = atoi( onum );
    
    if ( ( value < 1 ) || ( value > 3 ) )
       return FALSE;
       
    switch ( value )
    {
     case 1:
        pClan->obj_vnum_1 = vnum;
        break;
     case 2:
        pClan->obj_vnum_2 = vnum;
        break;
     case 3:
        pClan->obj_vnum_3 = vnum;
        break;
     default:
        break;
    }
    
    send_to_char( C_DEFAULT, "Object vnum set.\n\r", ch );
    return TRUE;
}

bool cedit_power( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;
    char number[MAX_INPUT_LENGTH];
    char onum[MAX_INPUT_LENGTH];
    int  value;

    argument = one_argument( argument, number );
    argument = one_argument( argument, onum );

    EDIT_CLAN(ch, pClan);

    if ( number[0] == '\0' )
    {
     send_to_char( C_DEFAULT, "Syntax: power <1/2/3/4> <name>\n\r", ch );
     return FALSE;
    }
    
    value = atoi( number );
    
    if ( ( value < 1 ) || ( value > 4 ) )
       return FALSE;
       
    switch ( value )
    {
     case 1:
        pClan->second = str_dup( onum );
        break;
     case 2:
        pClan->first = str_dup( onum );
        break;
     case 3:
        pClan->leader = str_dup( onum);
        break;
     case 4:
        pClan->champ = str_dup( onum );
        break;
     default:
        break;
    }
    
    send_to_char( C_DEFAULT, "Power seat set.\n\r", ch );
    return TRUE;
}

bool cedit_clist( CHAR_DATA *ch, char *argument )
{
  return TRUE;
}

bool cedit_desc( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;

    EDIT_CLAN(ch, pClan);

    if ( argument[0] == '\0' )
    {
	string_append( ch, &pClan->description );
	return TRUE;
    }

    send_to_char(C_DEFAULT, "Syntax:  desc    - line edit\n\r", ch );
    return FALSE;
}

bool cedit_recall( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;
    char room[MAX_STRING_LENGTH];
    int  value;

    EDIT_CLAN(ch, pClan);

    one_argument( argument, room );

    if ( !is_number( argument ) || argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  recall [#rvnum]\n\r", ch );
	return FALSE;
    }

    value = atoi( room );

    if ( !get_room_index( value ) )
    {
	send_to_char(C_DEFAULT, "CEdit:  Room vnum does not exist.\n\r", ch );
	return FALSE;
    }

    pClan->recall = value;

    send_to_char(C_DEFAULT, "Recall set.\n\r", ch );
    return TRUE;
}
  
/*
 * Mobile Editor Functions.
 */
bool medit_show( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    char buf[MAX_STRING_LENGTH];

    EDIT_MOB(ch, pMob);

    sprintf( buf, "&b[&CName&b]:&w         [%s]\n\r&b[&CArea&b]:&z         [%5d] %s\n\r",
	pMob->player_name,
	!pMob->area ? -1        : pMob->area->vnum,
	!pMob->area ? "No Area" : pMob->area->name );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CAct&b]:&w          [%s]\n\r",
	flag_string( act_flags, pMob->act ) );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CVnum&b]:&w         [%5d]\n\r&b[&CSex&b]:&w          [%s]\n\r",
	pMob->vnum,
	pMob->sex == SEX_MALE    ? "male"   :
	pMob->sex == SEX_FEMALE  ? "female" : "neutral" );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf,
	"&b[&CSize&b]:&w         [%2d]\n\r",
	pMob->size );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf,
	"&b[&CLevel&b]:&w        [%2d]\n\r&b[&CAlignment&b]:&w    [%d]\n\r",
	pMob->level,       pMob->alignment );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf,
        "&b[&CHit Points&b]:&w   [%5d]\n\r&b[&CGold&b]:&w         [%d]\n\r",
        pMob->hitnodice, pMob->gold );
    send_to_char(C_DEFAULT, buf, ch);

    sprintf( buf,
        "&b[&CHitroll&b]:&w      [%d]\n\r&b[&CDamroll&b]:&w      [%d]\n\r",
	pMob->hitroll, pMob->damroll );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, 
	"&b[&CLanguage&b]:&w     [%s]\n\r",
	lang_table[pMob->speaking].name );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CAffected by&b]:&w  [%s]\n\r",
	flag_string( affect_flags, pMob->affected_by ) );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CAffected by2&b]:&w [%s]\n\r",
	flag_string( affect2_flags, pMob->affected_by2 ) );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CAffected by3&b]:&w [%s]\n\r",
	flag_string( affect3_flags, pMob->affected_by3 ) );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CAffected by4&b]:&w [%s]\n\r",
	flag_string( affect4_flags, pMob->affected_by4 ) );
    send_to_char(C_DEFAULT, buf, ch );

    if ( pMob->spec_fun )
    {
	sprintf( buf, "&b[&CSpec fun&b]:&w     [%s]\n\r",  spec_string( pMob->spec_fun ) );
	send_to_char(C_DEFAULT, buf, ch );
    }

    if ( pMob->game_fun )
    {
        sprintf( buf, "&b[&CGame fun&b]:&w     [%s]\n\r",  game_string( pMob->game_fun ) );
        send_to_char(AT_WHITE, buf, ch );
    }

    sprintf( buf, "&b[&CImmunities&b]:&w  [%s]\n\r",
	flag_string( immune_flags, pMob->imm_flags ) );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CShort descr&b]:&w %s\n\r&b[&CLong descr&b]:&w\n\r     %s",
	pMob->short_descr,
	pMob->long_descr );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "&b[&CDescription&b]:&w\n\r%s", pMob->description );
    send_to_char(C_DEFAULT, buf, ch );

    if ( pMob->pShop )
    {
	SHOP_DATA *pShop;
	int iTrade;

	pShop = pMob->pShop;

	sprintf( buf,
	  "&CShop data for&z [%5d]:\n\r"
	  "  &CMarkup for purchaser&b:&w %d%%\n\r"
	  "  &CMarkdown for seller&b:&w  %d%%\n\r",
	    pShop->keeper, pShop->profit_buy, pShop->profit_sell );
	send_to_char(C_DEFAULT, buf, ch );
	sprintf( buf, "  &CHours&b:&w %d &bto&w %d.\n\r",
	    pShop->open_hour, pShop->close_hour );
	send_to_char(C_DEFAULT, buf, ch );

	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	{
	    if ( pShop->buy_type[iTrade] != 0 )
	    {
		if ( iTrade == 0 ) {
		    send_to_char(C_DEFAULT, "  &b[&CNumber&b] [&CTrades Type&b]\n\r", ch );
		    send_to_char(C_DEFAULT, "  -------- -------------\n\r", ch );
		}
		sprintf( buf, "&w  [ %4d ]  %s\n\r", iTrade,
		    flag_string( type_flags, pShop->buy_type[iTrade] ) );
		send_to_char(C_DEFAULT, buf, ch );
	    }
	}
    }

    return FALSE;
}

bool medit_create( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int  value;
    int  iHash;

    value = atoi( argument );

    /* OLC 1.1b */
    if ( argument[0] == '\0' || value <= 0 || value >= INT_MAX )
    {
	char output[MAX_STRING_LENGTH];

	sprintf( output, "Syntax:  create [0 < vnum < %d]\n\r", INT_MAX );
	send_to_char(C_DEFAULT, output, ch );
	return FALSE;
    }

    pArea = get_vnum_area( value );

    if ( !pArea )
    {
	send_to_char(C_DEFAULT, "MEdit:  That vnum is not assigned an area.\n\r", ch );
	return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char(C_DEFAULT, "MEdit:  Vnum in an area you cannot build in.\n\r", ch );
	return FALSE;
    }

    if ( get_mob_index( value ) )
    {
	send_to_char(C_DEFAULT, "MEdit:  Mobile vnum already exists.\n\r", ch );
	return FALSE;
    }

    pMob			= new_mob_index();
    pMob->vnum			= value;
    pMob->area			= pArea;
        
    if ( value > top_vnum_mob )
	top_vnum_mob = value;        

    pMob->act			= ACT_IS_NPC;
    iHash			= value % MAX_KEY_HASH;
    pMob->next			= mob_index_hash[iHash];
    mob_index_hash[iHash]	= pMob;
    ch->desc->pEdit		= (void *)pMob;

    send_to_char(C_DEFAULT, "Mobile Created.\n\r", ch );
    return TRUE;
}

bool medit_class( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    
    EDIT_MOB(ch, pMob );
    
    if ( argument[0] == '\0' )
    {   
        send_to_char(C_DEFAULT, "Syntax: class [mobs class]\n\r", ch );
        return FALSE;
    }
    
    if ( ( atoi(argument) < 0 ) || ( atoi(argument) > MAX_CLASS ) )
    {
      pMob->class = (atoi(argument));
      return TRUE;   
    }
    
    send_to_char(C_DEFAULT, "Incorrect class number.\n\r", ch );
    return FALSE;
}


bool medit_spec( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  spec [special function]\n\r", ch );
	return FALSE;
    }


    if ( !str_cmp( argument, "none" ) )
    {
        pMob->spec_fun = NULL;

        send_to_char(C_DEFAULT, "Spec removed.\n\r", ch);
        return TRUE;
    }

    if ( spec_lookup( argument ) )
    {
	pMob->spec_fun = spec_lookup( argument );
	send_to_char(C_DEFAULT, "Spec set.\n\r", ch);
	return TRUE;
    }

    send_to_char(C_DEFAULT, "MEdit: No such special function.\n\r", ch );
    return FALSE;
}

bool medit_game( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
        send_to_char(AT_WHITE, "Syntax:  game [game function]\n\r", ch );
        return FALSE;
    }


    if ( !str_cmp( argument, "none" ) )
    {
        pMob->game_fun = NULL;

        send_to_char(AT_WHITE, "Game removed.\n\r", ch);
        return TRUE;
    }

    if ( game_lookup( argument ) )
    {
        pMob->game_fun = game_lookup( argument );
        send_to_char(AT_WHITE, "Game set.\n\r", ch);
        return TRUE;
    }

    send_to_char(AT_WHITE, "MEdit: No such game function.\n\r", ch );
    return FALSE;
}


bool medit_align( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(C_DEFAULT, "Syntax:  alignment [number]\n\r", ch );
	return FALSE;
    }

    pMob->alignment = atoi( argument );

    send_to_char(C_DEFAULT, "Alignment set.\n\r", ch);
    return TRUE;
}



bool medit_level( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(C_DEFAULT, "Syntax:  level [number]\n\r", ch );
	return FALSE;
    }

    pMob->level = atoi( argument );
    pMob->hitroll = atoi( argument ) * 2;
    pMob->damroll = atoi( argument ) * 2;
    pMob->gold = atoi(argument ) * 10;

    send_to_char(C_DEFAULT, "Level set.  Hitroll, Damroll, Gold generated.\n\r", ch);
    return TRUE;
}

bool medit_hitroll( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char(C_DEFAULT, "Syntax:  hitroll [number]\n\r", ch );
        return FALSE;
    }

    pMob->hitroll = atoi( argument );

    send_to_char(C_DEFAULT, "Hitroll set.\n\r", ch);
    return TRUE;
}

bool medit_damroll( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char(C_DEFAULT, "Syntax:  damroll [number]\n\r", ch );
        return FALSE;
    }

    pMob->damroll = atoi( argument );

    send_to_char(C_DEFAULT, "Damroll set.\n\r", ch);
    return TRUE;
}

bool medit_size( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    int size;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char(C_DEFAULT, "Syntax:  size [1-10]\n\r", ch );
        return FALSE;
    }

    size = atoi( argument );

    if (size > 10 || size < 1)
    {
        send_to_char(C_DEFAULT, "Syntax:  size [1-10]\n\r", ch );
        return FALSE;
    }

    pMob->size = size;

    send_to_char(C_DEFAULT, "Size set.\n\r", ch);
    return TRUE;
}

bool medit_language( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    int language;

    EDIT_MOB(ch, pMob);

    language = lang_lookup( argument );

    if ( argument[0] == '\0' || language == -1 )
    {
        send_to_char(C_DEFAULT, "Syntax:  language [language]\n\r", ch );
        return FALSE;
    }

    pMob->speaking = language;

    send_to_char(C_DEFAULT, "Language set.\n\r", ch);
    return TRUE;
}

bool medit_gold( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(C_DEFAULT, "Syntax:  gold [amount]\n\r", ch );
	return FALSE;
    }

    pMob->gold = atoi( argument );

    send_to_char(C_DEFAULT, "Gold coins set.\n\r", ch);
    return TRUE;
}
bool medit_hitpoint( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
	send_to_char(C_DEFAULT, "Syntax:  hp [amount]\n\r", ch );
	return FALSE;
    }

    pMob->hitnodice = atoi( argument );

    send_to_char(C_DEFAULT, "Hit points set.\n\r", ch);
    return TRUE;
}


bool medit_desc( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	string_append( ch, &pMob->description );
	return TRUE;
    }

    send_to_char(C_DEFAULT, "Syntax:  desc    - line edit\n\r", ch );
    return FALSE;
}

bool medit_long( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  long [string]\n\r", ch );
	return FALSE;
    }

    free_string( pMob->long_descr );
    strcat( argument, "\n\r" );
    pMob->long_descr = str_dup( argument );
    pMob->long_descr[0] = UPPER( pMob->long_descr[0]  );

    send_to_char(C_DEFAULT, "Long description set.\n\r", ch);
    return TRUE;
}



bool medit_short( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  short [string]\n\r", ch );
	return FALSE;
    }

    free_string( pMob->short_descr );
    pMob->short_descr = str_dup( argument );

    send_to_char(C_DEFAULT, "Short description set.\n\r", ch);
    return TRUE;
}



bool medit_name( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch, pMob);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  name [string]\n\r", ch );
	return FALSE;
    }

    free_string( pMob->player_name );
    pMob->player_name = str_dup( argument );

    send_to_char(C_DEFAULT, "Name set.\n\r", ch);
    return TRUE;
}




bool medit_shop( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );
    argument = one_argument( argument, arg1 );

    EDIT_MOB(ch, pMob);

    if ( command[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  shop hours [#opening] [#closing]\n\r", ch );
	send_to_char(C_DEFAULT, "         shop profit [#buying%] [#selling%]\n\r", ch );
	send_to_char(C_DEFAULT, "         shop type [#0-4] [item type]\n\r", ch );
	send_to_char(C_DEFAULT, "         shop delete [#0-4]\n\r", ch );
	send_to_char(C_DEFAULT, "         shop remove\n\r", ch );
	return FALSE;
    }


    if ( !str_cmp( command, "hours" ) )
    {
	if ( arg1[0] == '\0' || !is_number( arg1 )
	|| argument[0] == '\0' || !is_number( argument ) )
	{
	    send_to_char(C_DEFAULT, "Syntax:  shop hours [#opening] [#closing]\n\r", ch );
	    return FALSE;
	}

	if ( !pMob->pShop )
	{
	    pMob->pShop         = new_shop();
	    pMob->pShop->keeper = pMob->vnum;
	    shop_last->next     = pMob->pShop;
	}

	pMob->pShop->open_hour = atoi( arg1 );
	pMob->pShop->close_hour = atoi( argument );

	send_to_char(C_DEFAULT, "Shop hours set.\n\r", ch);
	return TRUE;
    }


    if ( !str_cmp( command, "profit" ) )
    {
	if ( arg1[0] == '\0' || !is_number( arg1 )
	|| argument[0] == '\0' || !is_number( argument ) )
	{
	    send_to_char(C_DEFAULT, "Syntax:  shop profit [#buying%] [#selling%]\n\r", ch );
	    return FALSE;
	}

	if ( !pMob->pShop )
	{
	    pMob->pShop         = new_shop();
	    pMob->pShop->keeper = pMob->vnum;
	    shop_last->next     = pMob->pShop;
	}

	pMob->pShop->profit_buy     = atoi( arg1 );
	pMob->pShop->profit_sell    = atoi( argument );

	send_to_char(C_DEFAULT, "Shop profit set.\n\r", ch);
	return TRUE;
    }


    if ( !str_cmp( command, "type" ) )
    {
	char buf[MAX_INPUT_LENGTH];
	int value;

	if ( arg1[0] == '\0' || !is_number( arg1 )
	|| argument[0] == '\0' )
	{
	    send_to_char(C_DEFAULT, "Syntax:  shop type [#0-4] [item type]\n\r", ch );
	    return FALSE;
	}

	if ( atoi( arg1 ) >= MAX_TRADE )
	{
	    sprintf( buf, "REdit:  May buy %d items max.\n\r", MAX_TRADE );
	    send_to_char(C_DEFAULT, buf, ch );
	    return FALSE;
	}

	if ( ( value = flag_value( type_flags, argument ) ) == NO_FLAG )
	{
	    send_to_char(C_DEFAULT, "REdit:  That type of item is not known.\n\r", ch );
	    return FALSE;
	}

	if ( !pMob->pShop )
	{
	    pMob->pShop         = new_shop();
	    pMob->pShop->keeper = pMob->vnum;
	    shop_last->next     = pMob->pShop;
	}

	pMob->pShop->buy_type[atoi( arg1 )] = value;

	send_to_char(C_DEFAULT, "Shop type set.\n\r", ch);
	return TRUE;
    }


    if ( !str_cmp( command, "delete" ) )
    {
	if ( arg1[0] == '\0' || !is_number( arg1 ) )
	{
	    send_to_char(C_DEFAULT, "Syntax:  shop delete [#0-4]\n\r", ch );
	    return FALSE;
	}

	if ( !pMob->pShop )
	{
	    send_to_char(C_DEFAULT, "REdit:  Non-existant shop.\n\r", ch );
	    return FALSE;
	}

	if ( atoi( arg1 ) >= MAX_TRADE )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "REdit:  May buy %d items max.\n\r", MAX_TRADE );
	    send_to_char(C_DEFAULT, buf, ch);
	    return FALSE;
	}

	pMob->pShop->buy_type[atoi( arg1 )] = 0;
	send_to_char(C_DEFAULT, "Shop type deleted.\n\r", ch );
	return TRUE;
    }

    else if ( !str_cmp( command, "remove" ) )
    {
        SHOP_DATA *pShop;
	SHOP_DATA *pPrev;

	if ( !pMob->pShop )
	{
	    send_to_char(C_DEFAULT, "REdit:  No shop to remove.\n\r", ch );
	    return FALSE;
	}

	for ( pShop = shop_first, pPrev = NULL; pShop; pPrev = pShop,
	      pShop = pShop->next )
	{
	    if ( pShop == pMob->pShop )
	      break;
	}

	if ( pPrev == NULL )
	  shop_first = shop_first->next;
	else
	  pPrev->next = pShop->next;

	free_shop( pShop );
	pMob->pShop = NULL;
	send_to_char( C_DEFAULT, "Shop removed.\n\r", ch );
	return TRUE;
    }

    medit_shop( ch, "" );
    return FALSE;
}

bool medit_immune(CHAR_DATA *ch, char *argument)
{
  MOB_INDEX_DATA *pMob;
  int value;

  EDIT_MOB(ch, pMob);

  if ( ( value = flag_value( immune_flags, argument ) ) != NO_FLAG )
  {
    TOGGLE_BIT(pMob->imm_flags, value);
    send_to_char(C_DEFAULT, "Immune toggled.\n\r", ch);
    return TRUE;
  }
  send_to_char(C_DEFAULT, "Immune not found.\n\r", ch);
  return FALSE;
}


/*
 * MobProg editor functions.
 * -- Altrag
 */
bool medit_mplist( CHAR_DATA *ch, char *argument )
{
  MPROG_DATA *pMProg;
  MOB_INDEX_DATA *pMob;
  char buf[MAX_STRING_LENGTH];
  int value = 0;

  EDIT_MOB(ch, pMob);

  for ( pMProg = pMob->mobprogs; pMProg; pMProg = pMProg->next, value++ )
  {
    sprintf( buf, "[%2d] (%14s)  %s\n\r", value,
	    mprog_type_to_name( pMProg->type ), pMProg->arglist );
    send_to_char(C_DEFAULT, buf, ch );
  }
  return FALSE;
}

bool oedit_oplist( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;
  OBJ_INDEX_DATA *pObj;
  char buf[MAX_STRING_LENGTH];
  int value = 0;

  EDIT_OBJ( ch, pObj );

  for ( pTrap = pObj->traps; pTrap; pTrap = pTrap->next_here, value++ )
  {
    sprintf(buf, "[%2d] (%13s)  %s\n\r", value,
	    flag_string(oprog_types, pTrap->type), pTrap->arglist);
    send_to_char(C_DEFAULT, buf, ch);
  }
  return FALSE;
}

bool redit_rplist( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;
  ROOM_INDEX_DATA *pRoom;
  char buf[MAX_STRING_LENGTH];
  int value = 0;

  EDIT_ROOM(ch, pRoom);

  for ( pTrap = pRoom->traps; pTrap; pTrap = pTrap->next_here, value++ )
  {
    sprintf(buf, "[%2d] (%12s)  %s\n\r", value,
	    flag_string(rprog_types, pTrap->type), pTrap->arglist);
    send_to_char(C_DEFAULT, buf, ch );
  }
  return FALSE;
}

bool redit_eplist( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;
  ROOM_INDEX_DATA *pRoom;
  int dir;
  EXIT_DATA *pExit = NULL;
  char buf[MAX_STRING_LENGTH];
  int value = 0;

  EDIT_ROOM(ch, pRoom);

  for ( dir = 0; dir < 6; dir++ )
    if ( !str_prefix( argument, dir_name[dir] ) &&
	(pExit = pRoom->exit[dir]) )
      break;
  if ( dir == 6 )
  {
    send_to_char(C_DEFAULT, "Exit does not exist in this room.\n\r",ch);
    return FALSE;
  }
  for ( pTrap = pExit->traps; pTrap; pTrap = pTrap->next_here, value++ )
  {
    sprintf(buf, "[%2d] (%11s)  %s\n\r", value,
	    flag_string(eprog_types, pTrap->type), pTrap->arglist);
    send_to_char(C_DEFAULT, buf, ch);
  }
  return FALSE;
}

bool medit_mpremove( CHAR_DATA *ch, char *argument )
{
  MPROG_DATA *pMProg;
  MPROG_DATA *pMPrev;
  MOB_INDEX_DATA *pMob;
  int value = 0;
  int vnum;

  if ( !is_number( argument ) )
  {
    send_to_char( C_DEFAULT, "Syntax:  mpremove #\n\r", ch );
    return FALSE;
  }

  vnum = atoi( argument );

  EDIT_MOB(ch, pMob);

  for ( pMProg = pMob->mobprogs, pMPrev = NULL; value < vnum;
        pMPrev = pMProg, pMProg = pMProg->next, value++ )
  {
    if ( pMProg == NULL )
    {
      send_to_char( C_DEFAULT, "No such MobProg.\n\r", ch );
      return FALSE;
    }
  }

  if ( pMPrev == NULL )
    pMob->mobprogs = pMob->mobprogs->next;
  else
    pMPrev->next = pMProg->next;

  free_mprog( pMProg );

  send_to_char(C_DEFAULT, "Ok.\n\r", ch );
  return TRUE;
}

bool oedit_opremove( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;
  TRAP_DATA *pPrev;
  OBJ_INDEX_DATA *pObj;
  int value = 0;
  int vnum;

  if ( !is_number( argument ) )
  {
    send_to_char(C_DEFAULT, "Syntax:  opremove #\n\r", ch );
    return FALSE;
  }

  vnum = atoi( argument );

  EDIT_OBJ( ch, pObj );

  for ( pTrap = pObj->traps, pPrev = NULL; value < vnum;
        pPrev = pTrap, pTrap = pTrap->next_here, value++ )
  {
    if ( !pTrap )
    {
      send_to_char(C_DEFAULT, "No such ObjProg.\n\r", ch );
      return FALSE;
    }
  }

  if ( !pPrev )
    pObj->traps = pObj->traps->next_here;
  else
    pPrev->next_here = pTrap->next_here;

  if ( pTrap == trap_list )
    trap_list = pTrap->next;
  else
  {
    for ( pPrev = trap_list; pPrev; pPrev = pPrev->next )
      if ( pPrev->next == pTrap )
	break;
    if ( pPrev )
      pPrev->next = pTrap->next;
  }

  free_trap( pTrap );

  send_to_char(C_DEFAULT, "Ok.\n\r", ch);
  return TRUE;
}

bool redit_rpremove( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;
  TRAP_DATA *pPrev;
  ROOM_INDEX_DATA *pRoom;
  int value = 0;
  int vnum;

  if ( !is_number( argument ) )
  {
    send_to_char(C_DEFAULT, "Syntax:  rpremove #", ch);
    return FALSE;
  }

  vnum = atoi( argument );

  EDIT_ROOM(ch, pRoom);

  for ( pTrap = pRoom->traps, pPrev = NULL; value < vnum;
        pPrev = pTrap, pTrap = pTrap->next_here, value++ )
  {
    if ( !pTrap )
    {
      send_to_char(C_DEFAULT, "No such RoomProg.\n\r", ch);
      return FALSE;
    }
  }

  if ( !pPrev )
    pRoom->traps = pRoom->traps->next_here;
  else
    pPrev->next_here = pTrap->next_here;

  if ( pTrap == trap_list )
    trap_list = pTrap->next;
  else
  {
    for ( pPrev = trap_list; pPrev; pPrev = pPrev->next )
      if ( pPrev->next == pTrap )
	break;
    if ( pPrev )
      pPrev->next = pTrap->next;
  }
  free_trap( pTrap );
  send_to_char(C_DEFAULT, "Ok.\n\r",ch);
  return TRUE;
}

bool redit_epremove( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;
  TRAP_DATA *pPrev;
  ROOM_INDEX_DATA *pRoom;
  int dir;
  EXIT_DATA *pExit = NULL;
  char arg[MAX_INPUT_LENGTH];
  int value = 0;
  int vnum;

  argument = one_argument(argument, arg);
  if ( arg[0] == '\0' || !is_number( argument ) )
  {
    send_to_char(C_DEFAULT, "Syntax:  epremove <direction> #\n\r",ch);
    return FALSE;
  }

  vnum = atoi(argument);

  EDIT_ROOM(ch, pRoom);

  for ( dir = 0; dir < 6; dir++ )
    if ( !str_prefix(arg, dir_name[dir]) && (pExit = pRoom->exit[dir]) )
      break;
  if ( dir == 6 )
  {
    send_to_char(C_DEFAULT, "Exit does not exist in this room.\n\r", ch);
    return FALSE;
  }

  for ( pTrap = pExit->traps, pPrev = NULL; value < vnum;
        pPrev = pTrap, pTrap = pTrap->next_here, value++ )
  {
    if ( !pPrev )
    {
      send_to_char(C_DEFAULT, "No such ExitProg.\n\r", ch);
      return FALSE;
    }
  }

  if ( !pPrev )
    pExit->traps = pExit->traps->next_here;
  else
    pPrev->next_here = pTrap->next_here;

  if ( pTrap == trap_list )
    trap_list = pTrap->next;
  else
  {
    for ( pPrev = trap_list; pPrev; pPrev = pPrev->next )
      if ( pPrev->next == pTrap )
	break;
    if ( pPrev )
      pPrev->next = pTrap->next;
  }
  free_trap( pTrap );
  send_to_char(C_DEFAULT, "Ok.\n\r", ch);
  return TRUE;
}

bool mpedit_show( CHAR_DATA *ch, char *argument )
{
  MPROG_DATA *pMProg;
  MOB_INDEX_DATA *pMob;
  char buf[MAX_STRING_LENGTH];
	int states;
  EDIT_MPROG(ch, pMProg);
  pMob = (MOB_INDEX_DATA *)ch->desc->inEdit;

  sprintf(buf, "&b[&CMobile&b]:&z [%5d] %s\n\r", pMob->vnum, pMob->player_name );
  send_to_char( C_DEFAULT, buf, ch );

  sprintf(buf, "&b[&CMobProg type&b]:&w %s\n\r", mprog_type_to_name( pMProg->type ) );
  send_to_char(C_DEFAULT, buf, ch );

  sprintf(buf, "&b[&CArguments&b]:&w %s\n\r", pMProg->arglist );
  send_to_char(C_DEFAULT, buf, ch );
	
  sprintf(buf, "&b[&CCommands&b]:&w\n\r%s", pMProg->comlist );
	
  send_to_char(C_DEFAULT, buf, ch );
  send_to_char(C_DEFAULT, "&b[&CStates&b]:&w ", ch);
	for (states = 1; states < 32; states++)
	{
		if ((1<<(states-1)) & pMProg->status)
		{
		sprintf(buf, "%d ", states);
		send_to_char(C_DEFAULT, buf, ch);
		}
	}
send_to_char(C_DEFAULT, "\r\n", ch);
  return TRUE;
}

bool tedit_show( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;
  char buf[MAX_STRING_LENGTH];

  EDIT_TRAP( ch, pTrap );

  if ( pTrap->on_obj )
  {
    sprintf(buf, "Object: [%5d] %s\n\r", pTrap->on_obj->vnum,
	    pTrap->on_obj->short_descr);
    send_to_char(C_DEFAULT, buf, ch);
  }

  if ( pTrap->in_room )
  {
    sprintf(buf, "Room: [%5d] %s\n\r", pTrap->in_room->vnum,
	    pTrap->in_room->name);
    send_to_char(C_DEFAULT, buf, ch);
  }

  if ( pTrap->on_exit )
  {
    int dir;
    EXIT_DATA *pExit;
    TRAP_DATA *trap;

    for ( dir = 0; dir < 6; dir++ )
      if ( (pExit = ch->in_room->exit[dir]) )
      {
	for ( trap = pExit->traps; trap; trap = trap->next_here )
	  if ( trap == pTrap )
	    break;
	if ( trap )
	  break;
      }

    sprintf(buf, "Exit: [%5s] %s\n\r", (dir == 6 ? "none" : dir_name[dir]),
	   (dir == 6 ? "Not found in room" : pExit->description));
    send_to_char(C_DEFAULT, buf, ch);
  }

  switch(ch->desc->editor)
  {
  case ED_OPROG:
    sprintf(buf, "ObjProg type: %s\n\r", flag_string(oprog_types,pTrap->type));
    break;
  case ED_RPROG:
    sprintf(buf, "RoomProg type: %s\n\r",flag_string(rprog_types,pTrap->type));
    break;
  case ED_EPROG:
    sprintf(buf, "ExitProg type: %s\n\r",flag_string(eprog_types,pTrap->type));
    break;
  default:
    bug("Tedit_show: Invalid editor %d",ch->desc->editor);
    sprintf(buf, "Unknown TrapProg type\n\r");
    break;
  }
  send_to_char(C_DEFAULT, buf, ch);

/*  sprintf(buf, "Disarmable: %s\n\r", (pTrap->disarmable ? "Yes" : "No"));
  send_to_char(C_DEFAULT, buf, ch);*/

  sprintf(buf, "Arguments: %s\n\r", pTrap->arglist);
  send_to_char(C_DEFAULT, buf, ch);

  sprintf(buf, "Commands:\n\r%s", pTrap->comlist);
  send_to_char(C_DEFAULT, buf, ch);

  return TRUE;
}

bool mpedit_create( CHAR_DATA *ch, char *argument )
{
  MPROG_DATA *pMProg;
  MPROG_DATA *pMLast;
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  pMProg = new_mprog( );
  pMProg->type = ACT_PROG;
  if ( !pMob->mobprogs )
    pMob->mobprogs = pMProg;
  else
  {
    /* No purpose except to find end of list. -- Altrag */
    for ( pMLast = pMob->mobprogs; pMLast->next; pMLast = pMLast->next );
    pMLast->next = pMProg;
  }

  SET_BIT( pMob->progtypes, 1 );

  ch->desc->inEdit = (void *)ch->desc->pEdit;
  ch->desc->pEdit  = (void *)pMProg;

  send_to_char(C_DEFAULT, "MobProg created.\n\r", ch );
  return TRUE;
}

bool tedit_create( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;
  TRAP_DATA *pLast;
  TRAP_DATA **pFirst;
  OBJ_INDEX_DATA *pObj = NULL;
  ROOM_INDEX_DATA *pRoom = NULL;
  EXIT_DATA *pExit = NULL;

  switch(ch->desc->editor)
  {
    int dir;
    char arg[MAX_STRING_LENGTH];
  case ED_OPROG:
    pExit = NULL;
    EDIT_OBJ(ch, pObj);
    pFirst = &pObj->traps;
    SET_BIT(pObj->traptypes, 1);
    break;
  case ED_RPROG:
    pExit = NULL;
    EDIT_ROOM(ch, pRoom);
    pFirst = &pRoom->traps;
    SET_BIT(pRoom->traptypes, 1);
    break;
  case ED_EPROG:
    pExit = NULL;
    EDIT_ROOM(ch, pRoom);
    argument = one_argument(argument, arg);
    for ( dir = 0; dir < 6; dir++ )
      if ( !str_prefix(arg, dir_name[dir]) && (pExit = pRoom->exit[dir]) )
	break;
    if ( dir == 6 )
    {
      bug("Tedit_create: No exit",0);
      return FALSE;
    }
    pRoom = NULL;
    pFirst = &pExit->traps;
    SET_BIT(pExit->traptypes, 1);
    break;
  default:
    pExit = NULL;
    bug("Tedit_create: Invalid editor %d", ch->desc->editor);
    return FALSE;
  }

  pTrap = new_trap( );
  pTrap->on_obj = pObj;
  pTrap->in_room = pRoom;
  pTrap->on_exit = pExit;

  if ( !trap_list )
    trap_list = pTrap;
  else
  {
    for ( pLast = trap_list; pLast->next; pLast = pLast->next );
    pLast->next = pTrap;
  }


  if ( !*pFirst )
    *pFirst = pTrap;
  else
  {
    for ( pLast = *pFirst; pLast->next_here; pLast = pLast->next_here );
    pLast->next_here = pTrap;
  }

  if ( ch->desc->editor == ED_OPROG )
    ch->desc->inEdit = (void *)ch->desc->pEdit;
  ch->desc->pEdit = (void *)pTrap;

  switch(ch->desc->editor)
  {
  case ED_OPROG:
    send_to_char(C_DEFAULT, "ObjProg created\n\r",ch);
    break;
  case ED_RPROG:
    send_to_char(C_DEFAULT, "RoomProg created\n\r",ch);
    break;
  case ED_EPROG:
    send_to_char(C_DEFAULT, "ExitProg created\n\r",ch);
    break;
  }
  return TRUE;
}
/* Edit the states that the mprog is active in*/
bool mpedit_state( CHAR_DATA *ch, char *argument )
{
	MPROG_DATA *pMProg;
	int state;  /* state to be added/removed */
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	EDIT_MPROG(ch, pMProg);
	argument = one_argument( argument, arg1);
	argument = one_argument( argument, arg2);
	buf[0] = '\0';
	if (!strcmp(arg1, "show"))
	{
		if (pMProg->status == 0)
		{
			buf[0] = '\0';
			strcat(buf, "Available in all states.\n\r");
			send_to_char(C_DEFAULT, buf, ch);
			return TRUE;
		}
		for (state = 1; state < 32; state++)
		{
			if (pMProg->status & (1 << (state -1)))
			{
				sprintf(buf1, " %d", state);
				strcat(buf, buf1);
			}	
		}
		sprintf(buf1, "Active in the following states: %s\r\n", buf);
		send_to_char(C_DEFAULT, buf1, ch);
		return TRUE;
	}
	else if (!strcmp(arg1, "enable"))
	{
		/*Adding a state */
		state = atoi(arg2);
		if (state > 33 || state < 1)
		{
		send_to_char(C_DEFAULT, "Valid states are 1 <= state <= 32\n\r", ch);
			return FALSE;
		}
		pMProg->status |= (1 << (state -1));
		send_to_char(C_DEFAULT, "State added.\r\n", ch);
		return TRUE;
	}
	else if (!strcmp(arg1, "disable"))
	{
		/* Disabling a state */
		state= atoi(arg2);
		if (state > 33 || state < 1)
		{
		send_to_char(C_DEFAULT, "Valid states are 1 <= state <= 32\r\n", ch);
			return FALSE;
		}
		pMProg->status &= (~(1 << (state -1)));
		send_to_char(C_DEFAULT, "State Disabled.\r\n", ch);
		return TRUE;
	}
	else
	{
		send_to_char(C_DEFAULT, "Syntax: state <enable|disable> <number>\r\n", ch);
		return FALSE;
	}
}
bool mpedit_arglist( CHAR_DATA *ch, char *argument )
{
  MPROG_DATA *pMProg;
  int prc = 0;

  EDIT_MPROG(ch, pMProg);

  if ( argument[0] == '\0' )
  {
    send_to_char( C_DEFAULT, "Syntax:  arglist [string]\n\r", ch );
    return FALSE;
  }

  prc = ( is_number( argument ) ? atoi( argument ) : 0 );
  if ( pMProg->type == RAND_PROG && prc > 95 )
  {
    send_to_char( C_DEFAULT, "You can't set the percentage that high on a rand_prog.\n\r", ch );
    return FALSE;
  }
  free_string(pMProg->arglist);
  pMProg->arglist = str_dup(argument);

  send_to_char( C_DEFAULT, "Arglist set.\n\r", ch );
  return TRUE;
}

bool tedit_arglist( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;

  EDIT_TRAP(ch, pTrap);

  if ( argument[0] == '\0' )
  {
    send_to_char(C_DEFAULT, "Syntax:  arglist [string]\n\r",ch);
    return FALSE;
  }
  free_string(pTrap->arglist);
  pTrap->arglist = str_dup(argument);

  send_to_char(C_DEFAULT, "Arglist set.\n\r",ch);
  return TRUE;
}

bool mpedit_comlist( CHAR_DATA *ch, char *argument )
{
  MPROG_DATA *pMProg;

  EDIT_MPROG(ch, pMProg);

  if ( argument[0] == '\0' )
  {
    string_append( ch, &pMProg->comlist );
    return TRUE;
  }

  send_to_char( C_DEFAULT, "Syntax:  comlist    - line edit\n\r", ch );
  return FALSE;
}

bool tedit_comlist( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;

  EDIT_TRAP(ch, pTrap);

  if ( argument[0] == '\0' )
  {
    string_append( ch, &pTrap->comlist );
    return TRUE;
  }

  send_to_char( C_DEFAULT, "Syntax:  comlist    - line edit\n\r", ch );
  return FALSE;
}

bool tedit_disarmable( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;

  EDIT_TRAP(ch, pTrap);

  pTrap->disarmable = !pTrap->disarmable;

  if ( pTrap->disarmable )
    send_to_char(C_DEFAULT, "Trap is now disarmable.\n\r",ch);
  else
    send_to_char(C_DEFAULT, "Trap is no longer disarmable.\n\r",ch);
  return TRUE;
}

/* XOR */
bool hedit_show(CHAR_DATA *ch, char *argument)
{
  HELP_DATA *pHelp;
  char buf[MAX_STRING_LENGTH];

  EDIT_HELP(ch, pHelp);
  if(pHelp == NULL)
  {
    send_to_char(C_DEFAULT, "bug 1", ch);
    return FALSE;
  }
  sprintf(buf, "Keyword(s):    [%s]\n\r",
   pHelp->keyword ? pHelp->keyword : "none");
  send_to_char(C_DEFAULT, buf, ch);

  sprintf(buf, "Level:         [%d]\n\r", pHelp->level);
  send_to_char(C_DEFAULT, buf, ch);

  sprintf(buf, "Description:\n\r%s\n\r",
   pHelp->text ? pHelp->text : "none.");
  send_to_char(C_DEFAULT, buf, ch);
  return FALSE;
}

bool hedit_desc(CHAR_DATA *ch, char *argument)
{
  HELP_DATA *pHelp;
  EDIT_HELP(ch, pHelp);

  if(argument[0] == '\0')
  {
    string_append( ch, &pHelp->text);
    return TRUE;
  }
  send_to_char(C_DEFAULT, "Syntax:  desc    - line edit\n\r", ch);
  return FALSE;
}

bool hedit_level(CHAR_DATA *ch, char *argument)
{
  HELP_DATA *pHelp;
  EDIT_HELP(ch, pHelp);
  
  if(argument[0] == '\0' || !is_number(argument))
  {
    send_to_char(C_DEFAULT, "Syntax:  level [number]\n\r", ch );
    return FALSE;
  }

  pHelp->level = atoi(argument);

  send_to_char(C_DEFAULT, "Level set.\n\r", ch);
  return TRUE;
}

bool hedit_name(CHAR_DATA *ch, char *argument)
{
  HELP_DATA *pHelp;
  EDIT_HELP(ch, pHelp);

  if(argument[0] == '\0')
  {
    send_to_char(C_DEFAULT, "Syntax:  keyword [string]\n\r", ch);
    return FALSE;
  }

  if( get_help(argument) != NULL )
  {
    send_to_char(C_DEFAULT, "Keyword already taken.\n\r", ch);
    return FALSE;
  }

  free_string( pHelp->keyword);
  pHelp->keyword = str_dup(argument);

  send_to_char(C_DEFAULT, "Keyword(s) set.\n\r", ch);
  return TRUE;
}

bool hedit_delet(CHAR_DATA *ch, char *argument)
{
  send_to_char(C_DEFAULT, "If you want to delete, spell it out.\n\r", ch);
  return FALSE;
}

bool hedit_delete(CHAR_DATA *ch, char *argument)
{
  HELP_DATA *pHelp;
  HELP_DATA *pMark;
  EDIT_HELP(ch, pHelp);

  if(argument[0] != '\0')
  {
    send_to_char(C_DEFAULT, "Type delete by itself.\n\r", ch);
    return FALSE;
  }

  if(pHelp == help_first)
    help_first = pHelp->next;
  for(pMark = help_first;pMark;pMark = pMark->next)
  {
    if(pHelp == pMark->next)
    {
      pMark->next = pHelp->next;
/*      pHelp->next = help_free;
      help_free = pHelp;*/
      free_help( pHelp);
      ch->desc->pEdit = NULL;
      ch->desc->editor = 0;
      send_to_char(C_DEFAULT, "Deleted.\n\r", ch);
      return TRUE;
    }
  }
  return FALSE;
}

/* END */

/* religion olc */

bool reledit_show( CHAR_DATA *ch, char *argument )
{
    RELIGION_DATA *pReligion;
    char buf[MAX_STRING_LENGTH];

    EDIT_RELIGION(ch, pReligion);

    sprintf( buf, "Name:        [%s]\n\rShort:       [%s]\n\rDeity:       [%s]\n\r",
	pReligion->name, pReligion->shortdesc, pReligion->deity );
    send_to_char(C_DEFAULT, buf, ch );

    sprintf( buf, "Religion:    [%3d]\n\r",
	pReligion->vnum );
    send_to_char(C_DEFAULT, buf, ch );
    sprintf( buf, "Recall:      [%5d] %s\n\r", pReligion->recall,
	get_room_index( pReligion->recall )
	? get_room_index( pReligion->recall )->name : "none" );
    send_to_char(C_DEFAULT, buf, ch );
    sprintf( buf, "Start:       [%5d] %s\n\r", pReligion->start,
        get_room_index( pReligion->start )
        ? get_room_index( pReligion->start )->name : "none" );
    send_to_char(C_DEFAULT, buf, ch );
    sprintf( buf, "Description:\n\r%s\n\r", pReligion->description );
    send_to_char(C_DEFAULT, buf, ch );
    
    return FALSE;
}

bool reledit_create( CHAR_DATA *ch, char *argument )
{
    RELIGION_DATA *pReligion;
    int  value;

    value = atoi( argument );

    /* OLC 1.1b */
    if ( argument[0] == '\0' || value <= 0 || value >= MAX_RELIGION )
    {
	char output[MAX_STRING_LENGTH];

	sprintf( output, "Syntax:  reledit create [1 < vnum < %d]\n\r",
		 MAX_RELIGION );
	send_to_char(C_DEFAULT, output, ch );
	return FALSE;
    }


    if ( get_religion_index( value ) )
    {
	send_to_char(C_DEFAULT, "RELEdit:  Religion vnum already exists.\n\r", ch );
	return FALSE;
    }

    pReligion			= new_religion();
    pReligion->vnum			= value;
    religion_sort(pReligion);
    ch->desc->pEdit		= (void *)pReligion;

    send_to_char(C_DEFAULT, "Religion Created.\n\r", ch );
    return TRUE;
}

bool reledit_name( CHAR_DATA *ch, char *argument )
{
    RELIGION_DATA *pReligion;

    EDIT_RELIGION(ch, pReligion);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  name [string]\n\r", ch );
	return FALSE;
    }

    free_string( pReligion->name );
    pReligion->name = str_dup( argument );

    send_to_char(C_DEFAULT, "Name set.\n\r", ch);
    return TRUE;
}

bool reledit_deity( CHAR_DATA *ch, char *argument )
{
    RELIGION_DATA *pReligion;

    EDIT_RELIGION(ch, pReligion);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  deity [string]\n\r", ch );
	return FALSE;
    }

    free_string( pReligion->deity );
    pReligion->deity = str_dup( argument );

    send_to_char(C_DEFAULT, "Deity set.\n\r", ch);
    return TRUE;
}

bool reledit_clist( CHAR_DATA *ch, char *argument )
{
  return TRUE;
}

bool reledit_desc( CHAR_DATA *ch, char *argument )
{
    RELIGION_DATA *pReligion;

    EDIT_RELIGION(ch, pReligion);

    if ( argument[0] == '\0' )
    {
	string_append( ch, &pReligion->description );
	return TRUE;
    }

    send_to_char(C_DEFAULT, "Syntax:  desc    - line edit\n\r", ch );
    return FALSE;
}

bool reledit_recall( CHAR_DATA *ch, char *argument )
{
    RELIGION_DATA *pReligion;

    char room[MAX_STRING_LENGTH];
    int  value;

    EDIT_RELIGION(ch, pReligion );

    one_argument( argument, room );

    if ( !is_number( argument ) || argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  recall [#rvnum]\n\r", ch );
	return FALSE;
    }

    value = atoi( room );

    if ( !get_room_index( value ) )
    {
	send_to_char(C_DEFAULT, "RELEdit:  Room vnum does not exist.\n\r", ch );
	return FALSE;
    }

    pReligion->recall = value;

    send_to_char(C_DEFAULT, "Recall set.\n\r", ch );
    return TRUE;
}

bool reledit_start( CHAR_DATA *ch, char *argument )
{
    RELIGION_DATA *pReligion;
    
    char room[MAX_STRING_LENGTH];
    int  value;
    
    EDIT_RELIGION(ch, pReligion );
    
    one_argument( argument, room );

    if ( !is_number( argument ) || argument[0] == '\0' )
    {
        send_to_char(C_DEFAULT, "Syntax:  start [#rvnum]\n\r", ch );
        return FALSE;
    }
 
    value = atoi( room );
    
    if ( !get_room_index( value ) )
    {
        send_to_char(C_DEFAULT, "RELEdit:  Room vnum does not exist.\n\r", ch );
        return FALSE;
    }
        
    pReligion->start = value;
    
    send_to_char(C_DEFAULT, "Start Room set.\n\r", ch );
    return TRUE; 
}

bool forge_show(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA  *pObj;
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next = NULL;
  int cnt, max_stat, max_dam, max_hit, max_hp, max_ac, max_mana;
  int max_saves, max_saveb, max_ad, max_bp;
  EDIT_FORGE( ch, pObj );
  max_stat = ( pObj->level > 100 ) ? 3 : 2;
  if ( pObj->item_type == ITEM_WEAPON )
	max_dam = (int)(pObj->level / 2.5);
  else
	max_dam = ( IS_SET( pObj->wear_flags, ITEM_WEAR_BODY ) )
		? pObj->level / 3 : pObj->level / 8;
  max_hit = max_dam * 2 / 3;
  max_hp = max_mana = pObj->level;
  max_bp = max_mana / 3;
  max_saves = max_saveb = 0 - UMAX( 1, pObj->level / 7 );
  max_ad = (int)(pObj->level * 0.4);
  max_ac = 0 - ( pObj->level * 3 / 4);
  send_to_char( AT_GREY, "Forging an item; type &RDONE &wonly when finished.\n\r", ch );
  send_to_char( AT_DGREY, "Name and descriptions:\n\r", ch );
  sprintf( buf, "Keywords&w:    &z[&W%s&z]\n\r", pObj->name );
  send_to_char( AT_WHITE, buf, ch );
  sprintf( buf, "Short Desc&w:  &z[&W%s&z]\n\r", pObj->short_descr );
  send_to_char( AT_WHITE, buf, ch );
  sprintf( buf, "Long Desc&w:   &z[&W%s&z]\n\r", pObj->description );
  send_to_char( AT_WHITE, buf, ch );
  if ( pObj->item_type == ITEM_WEAPON )
   {
   sprintf( buf, "Weapon Type&w: &z[&W%s&z]\n\r",
  	    flag_string( weapon_flags, pObj->value[3] ) );
   send_to_char( AT_WHITE, buf, ch );
   }
  send_to_char( AT_DGREY, "Availble stats and affects:\n\r", ch );
  sprintf( buf, "[&WStat 1&w:       &R+%d&z] [&WDamroll&w:      &R+%d&z] [&WHitroll&w:       &R+%d&z]\n\r",
	   max_stat, max_dam, max_hit );
  send_to_char( AT_DGREY, buf, ch );
  sprintf( buf, "[&WHit Points&w: &R+%d&z] [&WMana&w:        &R+%d&z] [&WArmor Class&w:  &R%d&z]",
	   max_hp, max_mana, max_ac );
  send_to_char( AT_DGREY, buf, ch );
  sprintf( buf, "\n\r[&WBlood&w:       &R+%d&z] ", max_bp );
  send_to_char( AT_DGREY, buf, ch );
  if ( pObj->level >= 40 )
    {
    sprintf( buf,  "[&WSaving-Spell&w: &R%d&z] [&WSaving-Breath&w: &R%d&z]\n\r",
 	     max_saves, max_saveb );
    send_to_char( AT_DGREY, buf, ch );
    }
  if ( pObj->level >= 45 )
    {
    sprintf( buf, "[&WStat 2&w:        &R+%d&z]", max_stat );
    send_to_char( AT_DGREY, buf, ch );
    }
  if ( pObj->level >= 60 )
    {
    sprintf( buf, " [&WAnti-Disarm&w: &R+%d&z]", max_ad );
    send_to_char( AT_DGREY, buf, ch );
    }
  if ( pObj->level >= 101 )
   {
   sprintf( buf, " [&WStat 3&w:        &R+%d&z]", max_stat );
   send_to_char( AT_DGREY, buf, ch );
   }
  send_to_char( AT_GREY, "\n\r", ch );
  send_to_char( AT_DGREY, "Added stats and affects:\n\r", ch );
  send_to_char( AT_WHITE, "#&w- &z[&W  affect  &z] [&Wmodifier&z]\n\r", ch );
  for ( cnt = 1, paf = pObj->affected; cnt <= pObj->level / 10; cnt++, paf = paf_next )
	{
	if ( cnt == 7 )
	  break;
	if ( paf )
	  {
	  paf_next = paf->next;
	  sprintf( buf, "%d&w- &z[&W%10s&z] [&R%8d&z]\n\r", cnt,
		   flag_string( apply_flags, paf->location ),
		   paf->modifier );
	  }
	else
	  sprintf( buf, "%d&w- &z[      &Wnone&z] [       &R0&z]\n\r", cnt );
	send_to_char( AT_WHITE, buf, ch );
	}
#ifdef NEW_MONEY
  sprintf( buf, "Gold Cost&w:   &z[&R%d&z]\n\r"
	        "Silver Cost&w: &z[&R%d&z]\n\r"
		"Copper Cost&w: &z[&R%d&z]\n\r",
          pObj->cost.gold, pObj->cost.silver, pObj->cost.copper );
#else
  sprintf( buf, "Object Cost&w: &z[&R%d&z]\n\r", pObj->cost );
#endif
  send_to_char( AT_WHITE, buf, ch );
  return FALSE;
}
bool forge_addaffect( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];
    char buf[MAX_INPUT_LENGTH];
  int cnt, max_stat, max_dam, max_hit, max_hp, max_ac, max_mana;
  int max_saves, max_saveb, max_ad, stat_cnt, max_statn, max_bp;
  int cost = 0;
  int Mod = 0;
  bool legal = FALSE;
  EDIT_FORGE( ch, pObj );
  max_statn = 1 + ( pObj->level >= 45 ) + ( pObj->level >= 101 );
  max_stat = ( pObj->level > 100 ) ? 3 : 2;
  if ( pObj->item_type == ITEM_WEAPON )
	max_dam = (int)(pObj->level / 2.5);
  else
	max_dam = ( IS_SET( pObj->wear_flags, ITEM_WEAR_BODY ) )
		? pObj->level / 3 : pObj->level / 8;
  max_hit = max_dam * 2 / 3;
  max_hp = max_mana = pObj->level;
  max_bp = max_mana / 3;
  max_saves = max_saveb = 0 - UMAX( 1, pObj->level / 7 );
  max_ad = (int)(pObj->level * 0.4);
  max_ac = 0 - ( pObj->level * 3 / 4 );

  argument = one_argument( argument, loc );
  one_argument( argument, mod );

  if ( loc[0] == '\0' || mod[0] == '\0' || !is_number( mod ) )
    {
    send_to_char(C_DEFAULT, "Syntax: # [affect] [modifier]\n\r", ch );
    return FALSE;
    }
  if ( !str_prefix( loc, "strength" ) )
    {
    strcpy( loc, "strength" );
    if ( (Mod=atoi( mod )) > max_stat )
	{
	sprintf( buf, "You may not add more than %d to %s.\n\r",
		 max_stat, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 2500;
    legal = TRUE;
    }
  if ( !str_prefix( loc, "intelligence" ) )
    {
    strcpy( loc, "intelligence" );
    if ( (Mod=atoi( mod )) > max_stat )
	{
	sprintf( buf, "You may not add more than %d to %s.\n\r",
		 Mod, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 2500;
    legal = TRUE;
    }
  if ( !str_prefix( loc, "wisdom" ) )
    {
    strcpy( loc, "wisdom" );
    if ( (Mod=atoi( mod )) > max_stat )
	{
	sprintf( buf, "You may not add more than %d to %s.\n\r",
		 max_stat, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 2500;
    legal = TRUE;
    }
  if ( !str_prefix( loc, "dexterity" ) )
    {
    strcpy( loc, "dexterity" );
    if ( (Mod=atoi( mod )) > max_stat )
	{
	sprintf( buf, "You may not add more than %d to %s.\n\r",
		 max_stat, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 2500;
    legal = TRUE;
    }
  if ( !str_prefix( loc, "constitution" ) )
    {
    strcpy( loc, "constitution" );
    if ( (Mod=atoi( mod )) > max_stat )
	{
	sprintf( buf, "You may not add more than %d to %s.\n\r",
		 max_stat, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 2500;
    legal = TRUE;
    }
  if ( !str_prefix( loc, "damroll" ) )
    {
    strcpy( loc, "damroll" );
    if ( (Mod=atoi( mod )) > max_dam )
	{
	sprintf( buf, "You may not add more than %d %s.\n\r",
		 max_dam, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 5000;
    legal = TRUE;
    }
  if ( !str_prefix( loc, "hitroll" ) )
    {
    strcpy( loc, "hitroll" );
    if ( (Mod=atoi( mod )) > max_hit )
	{
	sprintf( buf, "You may not add more than %d %s.\n\r",
		 max_hit, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 5000;
    legal = TRUE;
    }
  if ( !str_prefix( loc, "hitpoints" ) || !str_cmp( loc, "hp" ) )
    {
    strcpy( loc, "hp" );
    if ( (Mod=atoi( mod )) > max_hp )
	{
	sprintf( buf, "You may not add more than %d %ss.\n\r",
		 max_hp, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 1000;
    legal = TRUE;
    }
  if ( !str_prefix( loc, "blood" ) || !str_cmp( loc, "bp" ) )
    {
    strcpy( loc, "blood" );
    if ( (Mod=atoi( mod )) > max_bp )
	{
	sprintf( buf, "You may not add more than %d %ss.\n\r",
		 max_bp, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 3000;
    legal = TRUE;
    }
  if ( !str_prefix( loc, "armorclass" ) || !str_cmp( loc, "ac" ) )
    {
    strcpy( loc, "ac" );
    if ( (Mod=atoi( mod )) < max_ac )
	{
	sprintf( buf, "You may not add more than %d %ss.\n\r",
		 max_ac, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 2000;
    legal = TRUE;
    }
  if ( !str_prefix( loc, "mana" ) )
    {
    strcpy( loc, "mana" );
    if ( (Mod=atoi( mod )) > max_mana )
	{
	sprintf( buf, "You may not add more than %d %s.\n\r",
		 max_mana, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 1000;
    legal = TRUE;
    }
  if ( !str_prefix( loc, "anti-disarm" ) )
    {
    strcpy( loc, "anti-disarm" );
    if ( (Mod=atoi( mod )) > max_ad )
	{
	sprintf( buf, "You may not add more than %d %s.\n\r",
		 max_ad, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 3000;
    legal = TRUE;
    }
  if ( !str_prefix( loc, "saving-spell" ) )
    {
    strcpy( loc, "saving-spell" );
    if ( (Mod=atoi( mod )) < max_saves )
	{
	sprintf( buf, "You may not add more than %d %s.\n\r",
		 max_saves, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 10000;
    legal = TRUE;
    }
  if ( !str_prefix( loc, "saving-breath" ) )
    {
    strcpy( loc, "saving-breath" );
    if ( (Mod=atoi( mod )) < max_saveb )
	{
	sprintf( buf, "You may not add more than %d %s.\n\r",
		 max_saveb, loc );
	send_to_char( AT_GREY, buf, ch );
	return FALSE;
	}
    cost = abs(Mod) * 10000;
    legal = TRUE;
    }
    if ( !legal )
	{
	sprintf( buf, "Unknown affect %s, please choose from the list.\n\r", loc );
	send_to_char(AT_GREY, buf, ch );
	return FALSE;
	}
    if ( cost < 0 ) {
      send_to_char( AT_GREY, "You may not put a negative affect on an item anymore.\n\r", ch );
      sprintf( buf, "%s trying to abuse forge bug.", ch->name );
      bug( buf, 0 );
      return FALSE;
    }
    cnt = 0;
    stat_cnt = 0;
    for ( pAf = pObj->affected; pAf; pAf = pAf->next )
	{
	if (pAf->deleted)
		continue;
	cnt++;
	if ( cnt >= pObj->level / 10 || cnt >= 6 )
	  {
	  send_to_char( AT_GREY, "You can no longer add anything to this item.\n\r", ch );
	  return FALSE;
	  }
	if ( pAf->location == flag_value( apply_flags, loc ) )
	  {
	  sprintf( buf, "You have already added %d %s to this item.\n\r",
		   pAf->modifier, loc );
	  send_to_char( AT_GREY, buf, ch );
	  return FALSE;
	  }
        if ( pAf->location == APPLY_STR
	|| pAf->location == APPLY_DEX
	|| pAf->location == APPLY_INT
	|| pAf->location == APPLY_WIS
	|| pAf->location == APPLY_CON )
	  stat_cnt++;
        if ( stat_cnt >= max_statn
	&& (   !str_cmp( loc, "strength" )
	    || !str_cmp( loc, "dexterity" )
	    || !str_cmp( loc, "intelligence" )
	    || !str_cmp( loc, "wisdom" )
	    || !str_cmp( loc, "constitution" ) ) )
	  {
	  send_to_char( AT_GREY, "You have already added the maximum number of stats possible for your experience.\n\r", ch );
	  return FALSE;
	  }
    }
    pAf             =   new_affect();
    pAf->location   =   flag_value( apply_flags, loc );
    pAf->modifier   =   Mod;
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;
    sprintf( buf, "Added %d %s for a cost of %d.\n\r", Mod, loc, cost );
    send_to_char(C_DEFAULT, buf, ch);
#ifdef NEW_MONEY
    pObj->cost.gold += cost;
#else
    pObj->cost += cost;
#endif
    return TRUE;
}
bool forge_type( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *pObj;
  char buf[ MAX_INPUT_LENGTH ];
  EDIT_FORGE(ch, pObj);
  if ( pObj->item_type != ITEM_WEAPON )
    {
    send_to_char( AT_GREY, "You are not forging a weapon.\n\r", ch );
    return FALSE;
    }
  if ( argument[0] == '\0' )
    {
    sprintf( buf, "Legal values:\n\r%s %s %s %s %s %s %s\n\r",
	   weapon_flags[0].name, weapon_flags[1].name, weapon_flags[2].name,
	   weapon_flags[3].name, weapon_flags[4].name, weapon_flags[5].name,
	   weapon_flags[6].name );
    send_to_char( AT_GREY, buf, ch );
    sprintf( buf, "%s %s %s %s %s %s %s\n\r",
	   weapon_flags[7].name, weapon_flags[8].name, weapon_flags[9].name,
	   weapon_flags[10].name, weapon_flags[11].name, weapon_flags[12].name,
	   weapon_flags[13].name );
    send_to_char( AT_GREY, buf, ch );
    return FALSE;
    }
    if ( !str_cmp(
	 flag_string( weapon_flags, flag_value( weapon_flags, argument ) ),
	 "none" ) )
	{
	forge_type( ch, "" );
	return FALSE;
	}
    else
	{
	pObj->value[3] = flag_value( weapon_flags, argument );
	send_to_char( AT_GREY, "Weapon type set.\n\r", ch );
	return TRUE;
	}
  return FALSE;
}
void forge_pay_cost( CHAR_DATA *ch )
{
  OBJ_DATA *obj;

  EDIT_FORGE(ch, obj);

#ifdef NEW_MONEY

  if ( ( (ch->money.gold*C_PER_G) + (ch->money.silver*S_PER_G) +
	 (ch->money.copper) ) < ( (obj->cost.gold*C_PER_G) + (obj->cost.silver*S_PER_G) +
	 (obj->cost.copper) ) )
  {
     send_to_char( AT_GREY, "You cannot afford to forge this object.\n\r", ch );
     extract_obj( obj );
     return;
  }
  obj_to_char( obj, ch );
  spend_money( &ch->money, &obj->cost );
#else
  if ( ch->gold < obj->cost )
	{
	send_to_char( AT_GREY, "You cannot afford to forge this object.\n\r", ch );
	extract_obj( obj );
	return;
	}
  obj_to_char( obj, ch );
  ch->gold -= obj->cost;
#endif
  return;
}
bool forge_name( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *pObj;

    EDIT_FORGE(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  name [string]\n\r", ch );
	return FALSE;
    }

    free_string( pObj->name );
    pObj->name = str_dup( argument );

    send_to_char(C_DEFAULT, "Name set.\n\r", ch);
    return TRUE;
}



bool forge_short( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *pObj;

    EDIT_FORGE(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  short [string]\n\r", ch );
	return FALSE;
    }

    free_string( pObj->short_descr );
    pObj->short_descr = str_dup( argument );
/*    pObj->short_descr[0] = LOWER( pObj->short_descr[0] );*/

    send_to_char(C_DEFAULT, "Short description set.\n\r", ch);
    return TRUE;
}



bool forge_long( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *pObj;

    EDIT_FORGE(ch, pObj);

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax:  long [string]\n\r", ch );
	return FALSE;
    }
        
    free_string( pObj->description );
    pObj->description = str_dup( argument );
    pObj->description[0] = UPPER( pObj->description[0] );

    send_to_char(C_DEFAULT, "Long description set.\n\r", ch);
    return TRUE;
}


