/***************************************************************************
 *  File: olc.c                                                            *
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

/*$Id: olc.c,v 1.19 2005/04/11 03:25:02 tyrion Exp $*/


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
#include "merc.h"
#include "olc.h"

extern void save_area( AREA_DATA *pArea );
/*
 * Local functions.
 */
AREA_DATA  *get_area_data	args( ( int vnum ) );
MPROG_DATA *get_mprog_data      args( ( MOB_INDEX_DATA *pMob, int vnum ) );

#ifdef DISABLE_EDITS_SAVES

void send_disabled_message(CHAR_DATA* ch, char* argument)
{
	char command[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	strcpy(arg, argument);
	argument = one_argument( argument, command );
    if ( command[0] == '\0' ) /* let them see aeditor at least */
    {
		aedit_show( ch, argument );
		return;
    } else if (!str_cmp(command, "done"))
	{
		edit_done(ch);
		return;
	}
	send_to_char(AT_WHITE, "Area editing has been disabled on this port.\n\r", ch);
	send_to_char(AT_WHITE, "Please use the building port.\n\r", ch);
	send_to_char(AT_WHITE, "Your commands are being sent to the default interpreter.\n\r", ch);
	interpret(ch, arg);
	return;
}

#endif
/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor( DESCRIPTOR_DATA *d )
{
    switch ( d->editor )
    {
    case ED_AREA:
	aedit( d->character, d->incomm );
	break;
    case ED_ROOM:
	redit( d->character, d->incomm );
	break;
    case ED_OBJECT:
	oedit( d->character, d->incomm );
	break;
    case ED_MOBILE:
	medit( d->character, d->incomm );
	break;
    case ED_MPROG:
	mpedit( d->character, d->incomm );
	break;
    case ED_CLAN:
        cedit( d->character, d->incomm );
        break;
    case ED_RELIGION:
	reledit( d->character, d->incomm );
	break;
    case ED_HELP:
	hedit( d->character, d->incomm );	/* XOR */
	break;
    case ED_OPROG:
    case ED_RPROG:
    case ED_EPROG:
        tedit( d->character, d->incomm );
	break;
    case ED_FORGE:
        forge_obj( d->character, d->incomm );
	break;
    default:
	return FALSE;
    }
    return TRUE;
}



char *olc_ed_name( CHAR_DATA *ch )
{
    static char buf[10];
    
    buf[0] = '\0';
    switch (ch->desc->editor)
    {
    case ED_AREA:
	sprintf( buf, "AEdit" );
	break;
    case ED_ROOM:
	sprintf( buf, "REdit" );
	break;
    case ED_OBJECT:
	sprintf( buf, "OEdit" );
	break;
    case ED_MOBILE:
	sprintf( buf, "MEdit" );
	break;
    case ED_MPROG:
	sprintf( buf, "MPEdit" );
	break;
    case ED_CLAN:
        sprintf( buf, "CEdit" );
        break;
    case ED_RELIGION:
	sprintf( buf, "RELEdit" );
	break;
    case ED_HELP:
	sprintf( buf, "CEdit" );
	break;
    case ED_OPROG:
	sprintf( buf, "OPEdit" );
	break;
    case ED_RPROG:
	sprintf( buf, "RPEdit" );
	break;
    case ED_EPROG:
	sprintf( buf, "EPEdit" );
	break;
    case ED_FORGE:
        sprintf( buf, "Forge_Obj" );
	break;
    default:
	sprintf( buf, " " );
	break;
    }
    return buf;
}



char *olc_ed_vnum( CHAR_DATA *ch )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    OBJ_INDEX_DATA *pObj;
    MOB_INDEX_DATA *pMob;
    CLAN_DATA      *pClan;
    RELIGION_DATA  *pReligion;
    static char buf[10];
	
    buf[0] = '\0';
    switch ( ch->desc->editor )
    {
    case ED_AREA:
	pArea = (AREA_DATA *)ch->desc->pEdit;
	sprintf( buf, "%d", pArea ? pArea->vnum : 0 );
	break;
    case ED_ROOM:
	pRoom = ch->in_room;
	sprintf( buf, "%d", pRoom ? pRoom->vnum : 0 );
	break;
    case ED_OBJECT:
	pObj = (OBJ_INDEX_DATA *)ch->desc->pEdit;
	sprintf( buf, "%d", pObj ? pObj->vnum : 0 );
	break;
    case ED_MOBILE:
	pMob = (MOB_INDEX_DATA *)ch->desc->pEdit;
	sprintf( buf, "%d", pMob ? pMob->vnum : 0 );
	break;
    case ED_MPROG:
	pMob = (MOB_INDEX_DATA *)ch->desc->inEdit;
	sprintf( buf, "%d", pMob ? pMob->vnum : 0 );
	break;
    case ED_CLAN:
        pClan = (CLAN_DATA *)ch->desc->pEdit;
        sprintf( buf, "%d", pClan ? pClan->vnum : 0 );
        break;
    case ED_RELIGION:
	pReligion = (RELIGION_DATA *)ch->desc->pEdit;
	sprintf( buf, "%d", pReligion ? pReligion->vnum : 0 );
	break;
    case ED_OPROG:
	pObj = (OBJ_INDEX_DATA *)ch->desc->inEdit;
	sprintf( buf, "%d", pObj ? pObj->vnum : 0 );
	break;
    case ED_RPROG:
	pRoom = ch->in_room;
	sprintf( buf, "%d", pRoom ? pRoom->vnum : 0 );
	break;
    case ED_EPROG:
      {
	/* Woah.. this is a long way around nothing.. :) */
	int dir;
	EXIT_DATA *pExit;

	pRoom = ch->in_room;
	for ( dir = 0; dir < 6; dir++ )
	  if ( (pExit = pRoom->exit[dir]) )
	  {
	    TRAP_DATA *pTrap;

	    for ( pTrap = pExit->traps; pTrap; pTrap = pTrap->next_here )
	      if ( pTrap == (TRAP_DATA *)ch->desc->pEdit)
		break;
	    if ( pTrap )
	      break;
	  }
	if ( dir < 6 )
	  sprintf( buf, capitalize( dir_name[dir] ) );
	else
	  sprintf( buf, "Unknown" );
	break;
      }
    default:
	sprintf( buf, " " );
	break;
    }

    return buf;
}



/*****************************************************************************
 Name:		show_olc_cmds
 Purpose:	Format up the commands from given table.
 Called by:	show_commands(olc_act.c).
 ****************************************************************************/
void show_olc_cmds( CHAR_DATA *ch, const struct olc_cmd_type *olc_table )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  cmd;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (cmd = 0; olc_table[cmd].name[0] != '\0'; cmd++)
    {
	sprintf( buf, "%-15.15s", olc_table[cmd].name );
	strcat( buf1, buf );
	if ( ++col % 5 == 0 )
	    strcat( buf1, "\n\r" );
    }
 
    if ( col % 5 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char(C_DEFAULT, buf1, ch );
    return;
}



/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_commands( CHAR_DATA *ch, char *argument )
{
    switch (ch->desc->editor)
    {
	case ED_AREA:
	    show_olc_cmds( ch, aedit_table );
	    break;
	case ED_ROOM:
	    show_olc_cmds( ch, redit_table );
	    break;
	case ED_OBJECT:
	    show_olc_cmds( ch, oedit_table );
	    break;
	case ED_MOBILE:
	    show_olc_cmds( ch, medit_table );
	    break;
	case ED_MPROG:
	    show_olc_cmds( ch, mpedit_table );
	    break;
        case ED_CLAN:
            show_olc_cmds( ch, cedit_table );
            break;    
	case ED_RELIGION:
	    show_olc_cmds( ch, reledit_table );
	    break;
        case ED_HELP:
	    show_olc_cmds( ch, hedit_table );
	    break;
	case ED_OPROG:
	case ED_RPROG:
	case ED_EPROG:
	    show_olc_cmds( ch, tedit_table );
	    break;
    case ED_FORGE:
      show_olc_cmds(ch, forge_table );
    }

    return FALSE;
}



/*****************************************************************************
 *                           Interpreter Tables.                             *
 *****************************************************************************/
const struct olc_cmd_type aedit_table[] =
{
    {   "age",		aedit_age		},
    {   "builders",	aedit_builder		},
    {   "commands",	show_commands		},
    {   "create",	aedit_create		},
    {   "filename",	aedit_file		},
    {   "name",		aedit_name		},
    {   "recall",	aedit_recall		},
    {	"reset",	aedit_reset		},
    {   "security",	aedit_security		},
    {	"show",		aedit_show		},
    {   "vnum",		aedit_vnum		},
    {   "lvnum",	aedit_lvnum		},
    {   "uvnum",	aedit_uvnum		},
    {   "noise",        aedit_sounds            },
    {   "soundfile",	aedit_actual_sounds	},
    {   "musicfile",	aedit_musicfile		},
    {   "prototype",    aedit_prototype         },
    {   "noquest",      aedit_noquest           },
    {   "random",       aedit_random	      	},
    {   "future",       aedit_future            },
    {   "past",         aedit_past              },
    {   "present",      aedit_present           },
    {   "nosave",       aedit_nosave            },
    {   "creator",	aedit_creator		}, 
    {   "?",		show_help		},
    {   "version",	show_version		},
    {	"",		0,			}
};

const struct olc_cmd_type cedit_table[]=
{
    {   "commands",     show_commands           },
    {   "create",       cedit_create            },
    {   "clist",        cedit_clist             },
    {   "deity",        cedit_deity             },
    {   "members",      cedit_members           },
    {   "name",         cedit_name              },
    {   "object",       cedit_object            },
    {   "pkill",        cedit_pkill             },
    {   "recall",        cedit_recall           },
    {   "description",  cedit_desc              },
    {   "power",        cedit_power             },
    {   "",             0,                      }
};

const struct olc_cmd_type reledit_table[] =
{
    {   "commands",     show_commands           },
    {   "create",       reledit_create          },
    {   "deity",        reledit_deity           },
    {   "recall",       reledit_recall          },
    {   "description",  reledit_desc            },
    {	"start",	reledit_start		},
//    {   "short",        reledit_short           },
    {   "",             0,                      }
};

const struct olc_cmd_type redit_table[] =
{
    {   "commands",	show_commands		},
    {   "create",	redit_create		},
    {   "desc",		redit_desc		},
    {   "ed",		redit_ed		},
    {   "format",	redit_format		},
    {   "name",		redit_name		},
    {	"show",		redit_show		},
    {   "soundfile",	redit_soundfile		},
    {   "musicfile",	redit_musicfile		},

    {   "north",	redit_north		},
    {   "south",	redit_south		},
    {   "east",		redit_east		},
    {   "west",		redit_west		},
    {   "up",		redit_up		},
    {   "down",		redit_down		},
    {   "walk",		redit_move		},

    {	"mreset",	redit_mreset		},
    {	"oreset",	redit_oreset		},
    {   "rreset",	redit_rreset		},
    {	"mlist",	redit_mlist		},
    {	"olist",	redit_olist		},
    {   "rlist",        redit_rlist             },
    {	"mshow",	redit_mshow		},
    {	"oshow",	redit_oshow		},
    {   "rdamage",      redit_rdamage           },
    {   "rpedit",       redit_rpedit            },
    {   "rplist",       redit_rplist            },
    {   "rpremove",     redit_rpremove          },
    {   "epedit",       redit_epedit            },
    {   "eplist",       redit_eplist            },
    {   "epremove",     redit_epremove          },

    {   "?",		show_help		},
    {   "version",	show_version		},

    {	"",		0,			}
};



const struct olc_cmd_type oedit_table[] =
{
    {   "addaffect",	oedit_addaffect		},
    {   "commands",	show_commands		},
    {   "cost",		oedit_cost		},
    {   "level",        oedit_level             },
    {   "ac_type",      set_ac_type             },
    {   "ac_vnum",      set_ac_vnum             },
    {   "ac_v1",        set_ac_v1               },
    {   "ac_v2",        set_ac_v2               },
    {   "ac_setspell",  set_ac_setspell         },
    {   "create",	oedit_create		},
    {   "delaffect",	oedit_delaffect		},
    {   "ed",		oedit_ed		},
    {   "long",		oedit_long		},
    {   "name",		oedit_name		},
    {   "short",	oedit_short		},
    {	"show",		oedit_show		},
    {   "v0",		oedit_value0		},
    {   "v1",		oedit_value1		},
    {   "v2",		oedit_value2		},
    {   "v3",		oedit_value3		},
    {   "v4",		oedit_value4		},
    {   "weight",	oedit_weight		},
    {   "ojoin",        oedit_join		},
    {   "osepone",      oedit_sepone		},
    {   "oseptwo",      oedit_septwo		},
    {   "opedit",       oedit_opedit            },
    {   "oplist",       oedit_oplist            },
    {   "opremove",     oedit_opremove          },
    {   "durability_cur",oedit_durability_cur	},
    {	"durability_max",oedit_durability_max	},
    {	"dur_cur",	oedit_durability_cur	},
    {	"dur_max",	oedit_durability_max	},
    {   "?",		show_help		},
    {   "version",	show_version		},

    {	"",		0,			}
};



const struct olc_cmd_type medit_table[] =
{
    {   "alignment",	medit_align		},
    {   "commands",	show_commands		},
    {   "create",	medit_create		},
    {   "desc",		medit_desc		},
    {   "level",	medit_level		},
    {   "hp",           medit_hitpoint          },
    {   "gold",         medit_gold              },
    {   "long",		medit_long		},
    {   "name",		medit_name		},
    {   "shop",		medit_shop		},
    {   "class",	medit_class	        },
    {   "short",	medit_short		},
    {	"show",		medit_show		},
    {   "spec",		medit_spec		},
    {   "game",         medit_game              },
    {   "immune",	medit_immune		},	/* XOR */
    {   "mpedit",       medit_mpedit            },      /* Altrag */
    {   "mplist",       medit_mplist            },      /* Altrag */
    {   "mpremove",     medit_mpremove          },      /* Altrag */
    {   "language",	medit_language		},	/* Tyrion */
    {   "hitroll",	medit_hitroll		},	/* Tyrion */
    {	"damroll",	medit_damroll		},	/* Tyrion */
    {	"size",		medit_size		},	/* Tyrion */

    {   "?",		show_help		},
    {   "version",	show_version		},

    {	"",		0,			}
};


const struct olc_cmd_type mpedit_table[] =
{
    {   "arglist",      mpedit_arglist,         },
    {   "comlist",      mpedit_comlist,         },
	{	"state",		mpedit_state,			},
    {   "?",            show_help,              },
    {   "version",      show_version,           },
    {   "commands",     show_commands,          },

    {   "",             0,                      }
};

const struct olc_cmd_type tedit_table[] =
{
    {   "arglist",     tedit_arglist,           },
    {   "comlist",     tedit_comlist,           },

    {   "?",           show_help,               },
    {   "version",     show_version,            },
    {   "commands",    show_commands,           },

    {   "",            0,                       }
};

const struct olc_cmd_type hedit_table[] =
{
    {   "commands",	show_commands		},
    {   "delet",	hedit_delet		},
    {   "delete",	hedit_delete		},
    {   "keyword",	hedit_name		},
    {   "name",		hedit_name		},
    {   "level",	hedit_level		},
    {   "show",		hedit_show		},
    {   "desc",		hedit_desc		},
    {   "text",		hedit_desc		},

    {   "?",            show_help               },
    {   "version",      show_version            },

    {   "",		0,			}
};
const struct olc_cmd_type forge_table[] =
{
  {  "keywords",    forge_name },
  {  "short",   forge_short },
  {  "long",    forge_long },
  {  "1",       forge_addaffect },
  {  "2",       forge_addaffect },
  {  "3",       forge_addaffect },
  {  "4",       forge_addaffect },
  {  "5",       forge_addaffect },
  {  "6",       forge_addaffect },
  {  "commands",       show_commands },
  {  "", 0 },
};


/*****************************************************************************
 *                          End Interpreter Tables.                          *
 *****************************************************************************/



/*****************************************************************************
 Name:		get_area_data
 Purpose:	Returns pointer to area with given vnum.
 Called by:	do_aedit(olc.c).
 ****************************************************************************/
AREA_DATA *get_area_data( int vnum )
{
    AREA_DATA *pArea;

    for (pArea = area_first; pArea; pArea = pArea->next )
    {
        if (pArea->vnum == vnum)
            return pArea;
    }

    return 0;
}

/*
 * Get data for a MobProg -- Altrag
 */
MPROG_DATA *get_mprog_data( MOB_INDEX_DATA *pMob, int vnum )
{
  int value = 0;
  MPROG_DATA *pMProg;

  for ( pMProg = pMob->mobprogs; pMProg; pMProg = pMProg->next, value++ )
  {
    if ( value == vnum )
      return pMProg;
  }
  return NULL;
}

TRAP_DATA *get_trap_data( void *vo, int vnum, int type )
{
  int value = 0;
  OBJ_INDEX_DATA *obj = (OBJ_INDEX_DATA *)vo;
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *)vo;
  EXIT_DATA *pExit = (EXIT_DATA *)vo;
  TRAP_DATA *pTrap;

  switch(type)
  {
  case ED_OPROG:
    for (pTrap = obj->traps; pTrap; pTrap = pTrap->next_here, value++)
      if ( value == vnum )
	return pTrap;
    return NULL;
  case ED_RPROG:
    for (pTrap = room->traps; pTrap; pTrap = pTrap->next_here, value++)
      if ( value == vnum )
	return pTrap;
    return NULL;
  case ED_EPROG:
    for (pTrap = pExit->traps; pTrap; pTrap = pTrap->next_here, value++)
      if ( value == vnum )
	return pTrap;
    return NULL;
  default:
    bug( "Get_trap_index: Invalid type %d", type );
    return NULL;
  }
  return NULL;
}

/*****************************************************************************
 Name:		edit_done
 Purpose:	Resets builder information on completion.
 Called by:	aedit, redit, oedit, medit(olc.c), mpedit(Altrag)
 ****************************************************************************/
bool edit_done( CHAR_DATA *ch )
{
/*
 * Well, since i have the inEdit for mpedit, why not make it usable for
 * all nested editors..?
 * -- Altrag
 */
    if ( ch->desc->editin || ch->desc->inEdit )
    {
      ch->desc->pEdit = ch->desc->inEdit;
      ch->desc->inEdit = NULL;
      ch->desc->editor = ch->desc->editin;
      ch->desc->editin = 0;
      return FALSE;
    }
    if (ch->desc->editor == ED_FORGE)
      forge_pay_cost(ch);
    ch->desc->pEdit = NULL;
    ch->desc->editor = 0;
    return FALSE;
}



/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/


/* Area Interpreter, called by do_aedit. */
void aedit( CHAR_DATA *ch, char *argument )
{
#ifdef DISABLE_EDITS_SAVES
	send_disabled_message(ch, argument);
#else
	char command[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
    AREA_DATA *pArea;
    int  cmd;
    int  value;

    EDIT_AREA(ch, pArea);
    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( !IS_BUILDER( ch, pArea ) )
	send_to_char(C_DEFAULT, "AEdit:  Insufficient security to modify area.\n\r", ch );

    if ( command[0] == '\0' )
    {
	aedit_show( ch, argument );
	return;
    }

    if ( !str_cmp(command, "done") )
    {
	edit_done( ch );
	return;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	interpret( ch, arg );
	return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; *aedit_table[cmd].name; cmd++ )
    {
	if ( !str_prefix( command, aedit_table[cmd].name ) )
	{
	    if ( (*aedit_table[cmd].olc_fun) ( ch, argument ) )
		SET_BIT( pArea->area_flags, AREA_CHANGED );
	    return;
	}
    }

    /* Take care of flags. */
    if ( ( value = flag_value( area_flags, arg ) ) != NO_FLAG )
    {
	if (value==AREA_CORRUPT)
	{
	    send_to_char(C_DEFAULT, "Can't flag an area corrupt!\n\r", ch);
	    return;
	}

	TOGGLE_BIT(pArea->area_flags, value);

	send_to_char(C_DEFAULT, "Flag toggled.\n\r", ch );
	return;
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
#endif
    return;
}



/* Room Interpreter, called by do_redit. */
void redit( CHAR_DATA *ch, char *argument )
{
	#ifdef DISABLE_EDITS_SAVES
	send_disabled_message(ch, argument);
	#else
    ROOM_INDEX_DATA *pRoom;
    AREA_DATA *pArea;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int  cmd;
    int  value;

    EDIT_ROOM(ch, pRoom);
    pArea = pRoom->area;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( !IS_BUILDER( ch, pArea ) )
        send_to_char(C_DEFAULT, "REdit:  Insufficient security to modify room.\n\r", ch );

    if ( command[0] == '\0' )
    {
	redit_show( ch, argument );
	return;
    }

    if ( !str_cmp(command, "done") )
    {
	edit_done( ch );
	return;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
        interpret( ch, arg );
        return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; *redit_table[cmd].name; cmd++ )
    {
	if ( !str_prefix( command, redit_table[cmd].name ) )
	{
	    if ( (*redit_table[cmd].olc_fun) ( ch, argument ) )
		SET_BIT( pArea->area_flags, AREA_CHANGED );
	    return;
	}
    }

    /* Take care of flags. */
    if ( ( value = flag_value( room_flags, arg ) ) != NO_FLAG )
    {
        TOGGLE_BIT(pRoom->room_flags, value);

        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char(C_DEFAULT, "Room flag toggled.\n\r", ch );
        return;
    }

    if ( ( value = flag_value( sector_flags, arg ) ) != NO_FLAG )
    {
        pRoom->sector_type  = value;

        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char(C_DEFAULT, "Sector type set.\n\r", ch );
        return;
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
	#endif
    return;
}

void cedit( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int  cmd;

    EDIT_CLAN(ch, pClan);

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( !str_cmp( "done", command ) )
    {
        edit_done( ch );
        return;
    }
    
    if ( command[0] == '\0' )
    {
	cedit_show( ch, argument );
	return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; *cedit_table[cmd].name; cmd++ )
    {
	if ( !str_prefix( command, cedit_table[cmd].name ) )
	{
	    (*cedit_table[cmd].olc_fun) ( ch, argument );
	    return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}



/* Object Interpreter, called by do_oedit. */
void oedit( CHAR_DATA *ch, char *argument )
{
	#ifdef DISABLE_EDITS_SAVES
	send_disabled_message(ch, argument);
	#else
    AREA_DATA *pArea;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int  cmd;
    int  value;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_OBJ(ch, pObj);
    pArea = pObj->area;

    if ( !IS_BUILDER( ch, pArea ) )
	send_to_char(C_DEFAULT, "OEdit: Insufficient security to modify area.\n\r", ch );

    if ( command[0] == '\0' )
    {
	oedit_show( ch, argument );
	return;
    }

    if ( !str_cmp(command, "done") )
    {
	edit_done( ch );
	return;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	interpret( ch, arg );
	return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; *oedit_table[cmd].name; cmd++ )
    {
	if ( !str_prefix( command, oedit_table[cmd].name ) )
	{
	    if ( (*oedit_table[cmd].olc_fun) ( ch, argument ) )
		SET_BIT( pArea->area_flags, AREA_CHANGED );
	    return;
	}
    }

    /* Take care of flags. */
    if ( ( value = flag_value( type_flags, arg ) ) != NO_FLAG )
    {
        pObj->item_type = value;

        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char(C_DEFAULT, "Type set.\n\r", ch);

        /*
         * Clear the values.
         */
        pObj->value[0] = 0;
        pObj->value[1] = 0;
        pObj->value[2] = 0;
        pObj->value[3] = 0;

        return;
    }

    if ( ( value = flag_value( extra_flags, arg ) ) != NO_FLAG )
    {
        TOGGLE_BIT(pObj->extra_flags, value);

        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char(C_DEFAULT, "Extra flag toggled.\n\r", ch);
        return;
    }

    if ( ( value = flag_value( extra_flags2, arg ) ) != NO_FLAG )
    {
        TOGGLE_BIT(pObj->extra_flags2, value);

        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char(C_DEFAULT, "Extra flag toggled.\n\r", ch);
        return;
    }

    if ( ( value = flag_value( extra_flags3, arg ) ) != NO_FLAG )
    {
        TOGGLE_BIT(pObj->extra_flags3, value);

        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char(C_DEFAULT, "Extra flag toggled.\n\r", ch);
        return;
    }

    if ( ( value = flag_value( extra_flags4, arg ) ) != NO_FLAG )
    {
        TOGGLE_BIT(pObj->extra_flags4, value);

        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char(C_DEFAULT, "Extra flag toggled.\n\r", ch);
        return;
    }

    if ( ( value = flag_value( wear_flags, arg ) ) != NO_FLAG )
    {
        TOGGLE_BIT(pObj->wear_flags, value);

        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char(C_DEFAULT, "Wear flag toggled.\n\r", ch);
        return;
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
	#endif
    return;
}



/* Mobile Interpreter, called by do_medit. */
void medit( CHAR_DATA *ch, char *argument )
{
	#ifdef DISABLE_EDITS_SAVES
	send_disabled_message(ch, argument);
	#else
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int  cmd;
    int  value;


    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_MOB(ch, pMob);
    pArea = pMob->area;

    if ( !IS_BUILDER( ch, pArea ) )
	send_to_char(C_DEFAULT, "MEdit: Insufficient security to modify area.\n\r", ch );

    if ( command[0] == '\0' )
    {
        medit_show( ch, argument );
        return;
    }

    if ( !str_cmp(command, "done") )
    {
	edit_done( ch );
	return;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
	interpret( ch, arg );
	return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; *medit_table[cmd].name; cmd++ )
    {
	if ( !str_prefix( command, medit_table[cmd].name ) )
	{
	    if ( (*medit_table[cmd].olc_fun) ( ch, argument ) )
		SET_BIT( pArea->area_flags, AREA_CHANGED );
	    return;
	}
    }

    /* Take care of flags. */
    if ( ( value = flag_value( sex_flags, arg ) ) != NO_FLAG )
    {
        pMob->sex = value;

        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char(C_DEFAULT, "Sex set.\n\r", ch);
        return;
    }


    if ( ( value = flag_value( act_flags, arg ) ) != NO_FLAG )
    {
        TOGGLE_BIT(pMob->act, value);

        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char(C_DEFAULT, "Act flag toggled.\n\r", ch);
        return;
    }


    if ( ( value = flag_value( affect_flags, arg ) ) != NO_FLAG )
    {
        TOGGLE_BIT(pMob->affected_by, value);

        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char(C_DEFAULT, "Affect flag toggled.\n\r", ch);
        return;
    }

    if ( ( value = flag_value( affect2_flags, arg ) ) != NO_FLAG )
    {
        TOGGLE_BIT(pMob->affected_by2, value);

	SET_BIT( pArea->area_flags, AREA_CHANGED );
	send_to_char(C_DEFAULT, "Affect flag toggled.\n\r", ch);
	return;
    }

    if ( ( value = flag_value( affect3_flags, arg ) ) != NO_FLAG )
    {
        TOGGLE_BIT(pMob->affected_by3, value);

	SET_BIT( pArea->area_flags, AREA_CHANGED );
	send_to_char(C_DEFAULT, "Affect flag toggled.\n\r", ch);
	return;
    }

    if ( ( value = flag_value( affect4_flags, arg ) ) != NO_FLAG )
    {
        TOGGLE_BIT(pMob->affected_by4, value);

	SET_BIT( pArea->area_flags, AREA_CHANGED );
	send_to_char(C_DEFAULT, "Affect flag toggled.\n\r", ch);
	return;
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
	#endif
    return;
}


/*
 * Editor for MobPrograms.
 * -- Altrag
 */
void mpedit( CHAR_DATA *ch, char *argument )
{
#ifdef DISABLE_EDITS_SAVES
   send_disabled_message(ch, argument);
#else
  AREA_DATA *pArea;
  MPROG_DATA *pMProg;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int  cmd;
  int  value;

  smash_tilde( argument );
  strcpy( arg, argument );
  argument = one_argument( argument, command );

  EDIT_MPROG(ch, pMProg);
  {
    MOB_INDEX_DATA *pMob;

    pMob = (MOB_INDEX_DATA *)ch->desc->inEdit;
    pArea = pMob->area;
  }

  if ( command[0] == '\0' )
  {
    mpedit_show( ch, "" );
    return;
  }

  if ( !str_cmp(command, "done") )
  {
    edit_done( ch );
    return;
  }  

  if ( !IS_BUILDER( ch, pArea ) )
  {
    interpret( ch, arg );
    return;
  }

  /* Search Table and Dispatch Command. */
  for ( cmd = 0; *mpedit_table[cmd].name; cmd++ )
  {
    if ( !str_prefix( command, mpedit_table[cmd].name ) )
    {
      if ( (*mpedit_table[cmd].olc_fun) ( ch, argument ) )
	SET_BIT( pArea->area_flags, AREA_CHANGED );
      return;
    }
  }
  
  if ( ( value = flag_value( mprog_types, arg ) ) != NO_FLAG )
  {
    MOB_INDEX_DATA *pMob;
    MPROG_DATA *prog;

    pMProg->type = value;
    pMob = (MOB_INDEX_DATA *)ch->desc->inEdit;
    /*
     * Full list search again in case theres more than one of the type
     * -- Altrag
     */
    pMob->progtypes = 0;
    for ( prog = pMob->mobprogs; prog; prog = prog->next )
      SET_BIT( pMob->progtypes, prog->type );
    SET_BIT( pArea->area_flags, AREA_CHANGED );
    send_to_char(C_DEFAULT, "MobProg type set.\n\r", ch);
    return;
  }
  
  /* Default to Standard Interpreter. */
  interpret( ch, arg );
  #endif
  return;
}
/*
 * Very klutzy global trap editor.. Main cause is that exits are handled
 * differently from the other items with traps.
 * -- Altrag
 */
void tedit( CHAR_DATA *ch, char *argument )
{
  #ifdef DISABLE_EDITS_SAVES
  send_disabled_message(ch, argument);
  #else
  AREA_DATA *pArea;
  TRAP_DATA *pTrap;
  EXIT_DATA *pExit = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int cmd;
  int value;

  switch (ch->desc->editor)
  {
  case ED_OPROG:
    pArea = ((OBJ_INDEX_DATA *)ch->desc->inEdit)->area;
    break;
  case ED_RPROG:
    {
      TRAP_DATA *trap;

      for ( trap = ch->in_room->traps; trap; trap = trap->next_here )
	if ( trap == (TRAP_DATA *)ch->desc->pEdit )
	  break;
      if ( !trap )
      {
	send_to_char(C_DEFAULT, "Room changed.  Returning to REditor.\n\r",ch);
	edit_done( ch );
/*	redit( ch, arg );*/
	return;
      }
    }
    pArea = ch->in_room->area;
    break;
  case ED_EPROG:
    {
      int dir;

      for ( dir = 0; dir < 6; dir++ )
	if ( (pExit = ch->in_room->exit[dir]) )
	{
	  TRAP_DATA *trap = (TRAP_DATA *)ch->desc->pEdit;

/*	  for ( trap = pExit->traps; trap; trap = trap->next_here )
	    if ( trap == (TRAP_DATA *)ch->desc->pEdit )
	      break;
	  if ( trap )
	    break;*/
	  if ( trap->on_exit == pExit )
	    break;
	}
      if ( dir == 6 )
      {
	send_to_char(C_DEFAULT, "Room changed.  Returning to REditor.\n\r",ch);
	edit_done( ch );
/*	redit(ch, arg);*/
	return;
      }
      pArea = ch->in_room->area;
      break;
    }
  default:
    bug("Tedit: Invalid editor type %d", ch->desc->editor);
    return;
  }

  smash_tilde(argument);
  strcpy(arg, argument);
  argument = one_argument(argument, command);

  EDIT_TRAP(ch, pTrap);

  if ( command[0] == '\0' )
  {
    tedit_show( ch, "" );
    return;
  }

  if ( !str_cmp(command, "done") )
  {
    edit_done( ch );
    return;
  }

  if ( !IS_BUILDER( ch, pArea ) )
  {
    interpret( ch, arg );
    return;
  }

  /* Search table and dispatch command. */
  for ( cmd = 0; *tedit_table[cmd].name; cmd++ )
  {
    if ( !str_prefix( command, tedit_table[cmd].name ) )
    {
      if ( (*tedit_table[cmd].olc_fun) ( ch, argument ) )
	SET_BIT( pArea->area_flags, AREA_CHANGED );
      return;
    }
  }

  switch (ch->desc->editor)
  {
    TRAP_DATA *trap;
    
  case ED_OPROG:
    if ( ( value = flag_value( oprog_types, arg ) ) != NO_FLAG )
    {
      OBJ_INDEX_DATA *obj = (OBJ_INDEX_DATA *)ch->desc->inEdit;
      
      pTrap->type = value;
      obj->traptypes = 0;

      for ( trap = obj->traps; trap; trap = trap->next_here )
	SET_BIT( obj->traptypes, trap->type );
      SET_BIT( pArea->area_flags, AREA_CHANGED );
      send_to_char(C_DEFAULT, "ObjProg type set.\n\r", ch );
      return;
    }
    break;
  case ED_RPROG:
    if ( ( value = flag_value( rprog_types, arg ) ) != NO_FLAG )
    {
      ROOM_INDEX_DATA *room = ch->in_room;

      pTrap->type = value;
      room->traptypes = 0;

      for ( trap = room->traps; trap; trap = trap->next_here )
	SET_BIT( room->traptypes, trap->type );
      SET_BIT( pArea->area_flags, AREA_CHANGED );
      send_to_char( C_DEFAULT, "RoomProg type set.\n\r",ch);
      return;
    }
    break;
  case ED_EPROG:
    if ( !pExit )
    {
      send_to_char(C_DEFAULT, "Exit not found.  Returning to REditor.\n\r",ch);
      bug("Tedit: NULL pExit in %d", ch->in_room->vnum);
      edit_done( ch );
      redit( ch, arg );
      return;
    }

    if ( ( value = flag_value( eprog_types, arg ) ) != NO_FLAG )
    {
      TRAP_DATA *trap;

      pTrap->type = value;
      pExit->traptypes = 0;

      for ( trap = pExit->traps; trap; trap = trap->next_here )
	SET_BIT( pExit->traptypes, trap->type );
      SET_BIT( pArea->area_flags, AREA_CHANGED );
      send_to_char( C_DEFAULT, "ExitProg type set.\n\r",ch);
      return;
    }
    break;
  default:
    bug("Tedit: Invalid editor %d", ch->desc->editor);
    break;
  }

  /* Default to Standard Interpreter. */
  interpret( ch, arg );
  #endif
  return;
}



void do_aedit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );
    pArea = ch->in_room->area;

    if ( command[0] == 'r' && !str_prefix( command, "reset" ) )
    {
	if ( ch->desc->editor == ED_AREA )
	    reset_area( (AREA_DATA *)ch->desc->pEdit );
	else
	    reset_area( pArea );
	send_to_char(C_DEFAULT, "Area reset.\n\r", ch );
	return;
    }

    if ( command[0] == 'c' && !str_prefix( command, "create" ) )
    {
	#ifdef DISABLE_EDITS_SAVES
	send_to_char(AT_WHITE, "Area creation/editing is disabled on this port.\n\r", ch);
	return;
	#else
	if ( aedit_create( ch, argument ) )
	{
	    ch->desc->editor = ED_AREA;
	    pArea = (AREA_DATA *)ch->desc->pEdit;
	    SET_BIT( pArea->area_flags, AREA_CHANGED );
	    aedit_show( ch, "" );
	}
	return;
	#endif
    }

    if ( is_number( command ) )
    {
	if ( !( pArea = get_area_data( atoi(command) ) ) )
	{
	    send_to_char(C_DEFAULT, "No such area vnum exists.\n\r", ch );
	    return;
	}
    }
	

    /*
     * Builder defaults to editing current area.
     */
    ch->desc->pEdit = (void *)pArea;
    ch->desc->editor = ED_AREA;
    aedit_show( ch, "" );
    return;
}



/* Entry point for editing room_index_data. */
void do_redit( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );
    pRoom = ch->in_room;

    if ( command[0] == 'r' && !str_prefix( command, "reset" ) )
    {
	reset_room( pRoom );
	send_to_char(C_DEFAULT, "Room reset.\n\r", ch );
	return;
    }

    if ( command[0] == 'c' && !str_prefix( command, "create" ) )
    {
	#ifdef DISABLE_EDITS_SAVES
	send_to_char(AT_WHITE, "Room creation/editing is disabled on this port.\n\r", ch);
	return;
	#else
	if ( redit_create( ch, argument ) )
	{
	    char_from_room( ch );
	    char_to_room( ch, ch->desc->pEdit );
	    SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
    }
	#endif
    }


    /*
     * Builder defaults to editing current room.
     */
    ch->desc->editor = ED_ROOM;
    redit_show( ch, "" );
    return;
}



/* Entry point for editing obj_index_data. */
void do_oedit( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj = NULL;
    AREA_DATA *pArea = NULL;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );

    if ( is_number( command ) )
    {
	if ( !( pObj = get_obj_index( atoi( command ) ) ) )
	{
	    send_to_char(C_DEFAULT, "OEdit:  That vnum does not exist.\n\r", ch );
	    return;
	}

	ch->desc->pEdit = (void *)pObj;
	ch->desc->editor = ED_OBJECT;
	oedit_show( ch, "" );
	return;
    }


    if ( command[0] == 'c' && !str_prefix( command, "create" ) )
    {
		#ifdef DISABLE_EDITS_SAVES
		send_to_char(AT_WHITE, "Object creation/editing is disabled on this port.\n\r", ch);
		return;
		#else
        if ( oedit_create( ch, argument ) )
	{
	    pArea = get_vnum_area( atoi( argument ) );
	    SET_BIT( pArea->area_flags, AREA_CHANGED );
	    ch->desc->editor = ED_OBJECT;
	    oedit_show( ch, "" );
	}
	return;
	#endif
    }


    send_to_char(C_DEFAULT, "OEdit:  There is no default object to edit.\n\r", ch );
    return;
}



/* Entry point for editing mob_index_data. */
void do_medit( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob = NULL;
    AREA_DATA *pArea = NULL;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );

    if ( is_number( command ) )
    {
	if ( !( pMob = get_mob_index( atoi( command ) ) ))
	{
	    send_to_char(C_DEFAULT, "MEdit:  That vnum does not exist.\n\r", ch );
	    return;
	}

	ch->desc->pEdit = (void *)pMob;
	ch->desc->editor = ED_MOBILE;
	medit_show( ch, "" );
	return;
    }

	#ifdef DISABLE_EDITS_SAVES
		send_to_char(AT_WHITE, "Mobile creation/editing is disabled on this port.\n\r", ch);
		return;
	#else

    if ( command[0] == 'c' && !str_prefix( command, "create" ) )
    {
	if ( medit_create( ch, argument ) )
	{
	    pArea = get_vnum_area( atoi( argument ) );
	    SET_BIT( pArea->area_flags, AREA_CHANGED );
	    ch->desc->editor = ED_MOBILE;
	    medit_show( ch, "" );
	}
	return;
    }

	#endif

    send_to_char(C_DEFAULT, "MEdit:  There is no default mobile to edit.\n\r", ch );
    return;
}

void do_cedit( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *pClan;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );

    if ( is_number( command ) )
    {
	if ( !( pClan = get_clan_index( atoi( command ) ) ))
	{
	    send_to_char(C_DEFAULT, "CEdit:  That clan does not exist.\n\r", ch );
	    return;
	}

	ch->desc->pEdit = (void *)pClan;
	ch->desc->editor = ED_CLAN;
	cedit_show( ch, "" );
	return;
    }

    if ( command[0] == 'c' && !str_prefix( command, "create" ) )
    {
	if ( cedit_create( ch, argument ) )
	{
	    ch->desc->editor = ED_CLAN;
	    cedit_show( ch, "" );
	}
	return;
    }

    send_to_char(C_DEFAULT, "CEdit:  There is no default clan to edit.\n\r", ch );
    return;
}

/*
 * The last part needed in this file.. :)
 * -- Altrag
 */
bool medit_mpedit( CHAR_DATA *ch, char *argument )
{
  MPROG_DATA *pMProg;
  char command[MAX_INPUT_LENGTH];

  argument = one_argument( argument, command );

  if ( is_number( command ) )
  {
    if ( !( pMProg = get_mprog_data( (MOB_INDEX_DATA *)ch->desc->pEdit, 
				    atoi( command ) ) ) )
    {
      send_to_char( C_DEFAULT, "MPEdit:  Mobile has no such MobProg.\n\r", ch );
      return FALSE;
    }
    ch->desc->editin = ch->desc->editor;
    ch->desc->editor = ED_MPROG;
    ch->desc->inEdit = ch->desc->pEdit;
    ch->desc->pEdit  = (void *)pMProg;
    mpedit_show( ch, "" );
    return TRUE;
  }

  if ( command[0] == 'c' && !str_prefix( command, "create" ) )
  {
    if ( mpedit_create( ch, argument ) )
    {
      ch->desc->editin = ch->desc->editor;
      ch->desc->editor = ED_MPROG;

      mpedit_show( ch, "" );
      return TRUE;
    }
    return FALSE;
  }

  send_to_char( C_DEFAULT, "MPEdit:  There is no default MobProg to edit.\n\r", ch );
  return FALSE;
}

bool oedit_opedit( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;
  char command[MAX_INPUT_LENGTH];

  argument = one_argument( argument, command );

  if ( is_number( command ) )
  {
    if ( !( pTrap = get_trap_data( ch->desc->pEdit, atoi(command),
				   ED_OPROG ) ) )
    {
      send_to_char( C_DEFAULT, "OPEdit:  Object has no such ObjProg.\n\r", ch );
      return FALSE;
    }
    ch->desc->editin = ch->desc->editor;
    ch->desc->editor = ED_OPROG;
    ch->desc->inEdit = ch->desc->pEdit;
    ch->desc->pEdit  = (void *)pTrap;
    tedit_show( ch, "" );
    return TRUE;
  }

  if ( command[0] == 'c' && !str_prefix( command, "create" ) )
  {
    ch->desc->editin = ch->desc->editor;
    ch->desc->editor = ED_OPROG;
    if ( tedit_create( ch, argument ) )
    {
      tedit_show( ch, "" );
      return TRUE;
    }
    ch->desc->editor = ch->desc->editin;
    ch->desc->editin = 0;
    return FALSE;
  }

  send_to_char( C_DEFAULT, "OPEdit:  There is no default ObjProg to edit.\n\r", ch );
  return FALSE;
}

bool redit_rpedit( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;
  char command[MAX_INPUT_LENGTH];

  argument = one_argument( argument, command );

  if ( is_number( command ) )
  {
    if ( !( pTrap = get_trap_data( ch->in_room, atoi(command), ED_RPROG ) ) )
    {
      send_to_char(C_DEFAULT, "RPEdit:  Room has no such RoomProg.\n\r",ch);
      return FALSE;
    }
    ch->desc->editin = ch->desc->editor;
    ch->desc->editor = ED_RPROG;
    ch->desc->pEdit  = (void *)pTrap;
    tedit_show( ch, argument );
    return TRUE;
  }

  if ( command[0] == 'c' && !str_prefix( command, "create" ) )
  {
    ch->desc->editin = ch->desc->editor;
    ch->desc->editor = ED_RPROG;
    if ( tedit_create( ch, argument ) )
    {
      tedit_show( ch, argument );
      return TRUE;
    }
    ch->desc->editor = ch->desc->editin;
    ch->desc->editin = 0;
    return FALSE;
  }

  send_to_char(C_DEFAULT, "RPEdit:  There is no default RoomProg to edit.\n\r",ch);
  return FALSE;
}

bool redit_epedit( CHAR_DATA *ch, char *argument )
{
  TRAP_DATA *pTrap;
  char command[MAX_INPUT_LENGTH];
  int dir;

  argument = one_argument( argument, command );
  for ( dir = 0; dir < 6; dir++ )
    if ( !str_prefix( command, dir_name[dir] ) && ch->in_room->exit[dir] )
      break;
  if ( dir == 6 )
  {
    send_to_char(C_DEFAULT, "EPEdit:  Room has no such exit.\n\r", ch);
    return FALSE;
  }

  argument = one_argument( argument, command );

  if ( is_number( command ) )
  {
    if ( !( pTrap = get_trap_data( ch->in_room->exit[dir], atoi(command),
				   ED_EPROG ) ) )
    {
      send_to_char(C_DEFAULT, "EPEdit:  Exit has no such ExitProg.\n\r", ch);
      return FALSE;
    }
    ch->desc->editin = ch->desc->editor;
    ch->desc->editor = ED_EPROG;
    ch->desc->pEdit  = pTrap;
    tedit_show( ch, "" );
    return TRUE;
  }

  if ( command[0] == 'c' && !str_prefix( command, "create" ) )
  {
    char buf[MAX_INPUT_LENGTH];

    ch->desc->editin = ch->desc->editor;
    ch->desc->editor = ED_EPROG;
    strcpy( buf, dir_name[dir] );
    if ( tedit_create( ch, buf ) )
    {
      tedit_show( ch, "" );
      return TRUE;
    }
    ch->desc->editor = ch->desc->editin;
    ch->desc->editin = 0;
    return FALSE;
  }

  send_to_char(C_DEFAULT, "EPEdit:  There is no default ExitProg to edit.\n\r",ch);
  return FALSE;
}

void display_resets( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA	*pRoom;
    RESET_DATA		*pReset;
    MOB_INDEX_DATA	*pMob = NULL;
    char 		buf   [ MAX_STRING_LENGTH ];
    char 		final [ MAX_STRING_LENGTH ];
    int 		iReset = 0;

    EDIT_ROOM(ch, pRoom);
    final[0]  = '\0';
    
    send_to_char ( C_DEFAULT, 
  " No.  Loads    Descrip     Location     Vnum    Max  Description Status"
  "\n\r"
  "==== ======== ========== ============= ======= ===== =========== ======"
  "\n\r", ch );

    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
    {
	OBJ_INDEX_DATA  *pObj;
	MOB_INDEX_DATA  *pMobIndex;
	OBJ_INDEX_DATA  *pObjIndex;
	OBJ_INDEX_DATA  *pObjToIndex;
	ROOM_INDEX_DATA *pRoomIndex;

	final[0] = '\0';
	sprintf( final, "&R[&w%2d&R]&w ", ++iReset );

	switch ( pReset->command )
	{
	default:
	    sprintf( buf, "Bad reset command: %c.", pReset->command );
	    strcat( final, buf );
	    break;

	case 'M':
	    if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )
	    {
                sprintf( buf, "Load Mobile - Bad Mob %d\n\r", pReset->arg1 );
                strcat( final, buf );
                continue;
	    }

	    if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
	    {
                sprintf( buf, "Load Mobile - Bad Room %d\n\r", pReset->arg3 );
                strcat( final, buf );
                continue;
	    }

            pMob = pMobIndex;
            sprintf( buf, "&BM&R[&B%5d&R]&B %-10.10s in room       R[%5d] [%3d] %-15.15s&w[%d]\n\r",
                       pReset->arg1, pMob->short_descr, pReset->arg3,
                       pReset->arg2, pRoomIndex->name, pReset->status );
            strcat( final, buf );

	    /*
	     * Check for pet shop.
	     * -------------------
	     */
	    {
		ROOM_INDEX_DATA *pRoomIndexPrev;

		pRoomIndexPrev = get_room_index( pRoomIndex->vnum - 1 );
		if ( pRoomIndexPrev
		    && IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP ) )
                    final[5] = 'P';
	    }

	    break;

	case 'O':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
	    {
                sprintf( buf, "Load Object - Bad Object %d\n\r",
		    pReset->arg1 );
                strcat( final, buf );
                continue;
	    }

            pObj       = pObjIndex;

	    if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
	    {
                sprintf( buf, "Load Object - Bad Room %d\n\r", pReset->arg3 );
                strcat( final, buf );
                continue;
	    }

            sprintf( buf, "&WO&R[&W%5d&R]&W %-13.13s in room             "
                          "R[%5d]       %-15.15s&w\n\r",
                          pReset->arg1, pObj->short_descr,
                          pReset->arg3, pRoomIndex->name );
            strcat( final, buf );

	    break;

	case 'P':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
	    {
                sprintf( buf, "Put Object - Bad Object %d\n\r",
                    pReset->arg1 );
                strcat( final, buf );
                continue;
	    }

            pObj       = pObjIndex;

	    if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
	    {
                sprintf( buf, "Put Object - Bad To Object %d\n\r",
                    pReset->arg3 );
                strcat( final, buf );
                continue;
	    }

	    sprintf( buf,
		"O[%5d] %-13.13s inside              O[%5d]       %-15.15s\n\r",
		pReset->arg1,
		pObj->short_descr,
		pReset->arg3,
		pObjToIndex->short_descr );
            strcat( final, buf );

	    break;

	case 'G':
	case 'E':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
	    {
                sprintf( buf, "Give/Equip Object - Bad Object %d\n\r",
                    pReset->arg1 );
                strcat( final, buf );
                continue;
	    }

            pObj       = pObjIndex;

	    if ( !pMob )
	    {
                sprintf( buf, "Give/Equip Object - No Previous Mobile\n\r" );
                strcat( final, buf );
                break;
	    }

	    if ( pMob->pShop )
	    {
	    sprintf( buf,
		"O[%5d] %-13.13s in the inventory of S[%5d]       %-15.15s\n\r",
		pReset->arg1,
		pObj->short_descr,                           
		pMob->vnum,
		pMob->short_descr  );
	    }
	    else
	    sprintf( buf,
		"O[%5d] %-13.13s %-19.19s M[%5d]       %-15.15s\n\r",
		pReset->arg1,
		pObj->short_descr,
		(pReset->command == 'G') ?
		    flag_string( wear_loc_strings, WEAR_NONE )
		  : flag_string( wear_loc_strings, pReset->arg3 ),
		  pMob->vnum,
		  pMob->short_descr );
	    strcat( final, buf );

	    break;

	/*
	 * Doors are set in rs_flags don't need to be displayed.
	 * If you want to display them then uncomment the new_reset
	 * line in the case 'D' in load_resets in db.c and here.
	 *
	case 'D':
	    pRoomIndex = get_room_index( pReset->arg1 );
	    sprintf( buf, "R[%5d] %s door of %-19.19s reset to %s\n\r",
		pReset->arg1,
		capitalize( dir_name[ pReset->arg2 ] ),
		pRoomIndex->name,
		flag_string( door_resets, pReset->arg3 ) );
	    strcat( final, buf );

	    break;
	 *
	 * End Doors Comment.
	 */
	case 'R':
	    if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
	    {
		sprintf( buf, "Randomize Exits - Bad Room %d\n\r",
		    pReset->arg1 );
		strcat( final, buf );
		continue;
	    }

	    sprintf( buf, "R[%5d] Exits (0) to (%d) are randomized in %s\n\r",
			pReset->arg1, pReset->arg2, pRoomIndex->name );
	    strcat( final, buf );

	    break;
	}
	send_to_char(C_DEFAULT, final, ch );
    }

    return;
}



/*****************************************************************************
 Name:		add_reset
 Purpose:	Inserts a new reset in the given index slot.
 Called by:	do_resets(olc.c).
 ****************************************************************************/
void add_reset( ROOM_INDEX_DATA *room, RESET_DATA *pReset, int index )
{
    RESET_DATA *reset;
    int iReset = 0;

    if ( !room->reset_first )
    {
	room->reset_first	= pReset;
	room->reset_last	= pReset;
	pReset->next		= NULL;
	return;
    }

    index--;

    if ( index == 0 )	/* First slot (1) selected. */
    {
	pReset->next = room->reset_first;
	room->reset_first = pReset;
	return;
    }

    /*
     * If negative slot( <= 0 selected) then this will find the last.
     */
    for ( reset = room->reset_first; reset->next; reset = reset->next )
    {
	if ( ++iReset == index )
	    break;
    }

    pReset->next	= reset->next;
    reset->next		= pReset;
    if ( !pReset->next )
	room->reset_last = pReset;
    return;
}



void do_resets( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    RESET_DATA *pReset = NULL;
    AREA_DATA *pArea = NULL;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
    argument = one_argument( argument, arg5 );

    /*
     * Display resets in current room.
     * -------------------------------
     */
    if ( arg1[0] == '\0' )
    {
      if ( ch->in_room->reset_first )
      {
	send_to_char(C_DEFAULT,
		     "Resets: M = mobile, R = room, O = object, "
		     "P = pet, S = shopkeeper\n\r", ch );
	display_resets( ch );
      }
      else
	send_to_char(C_DEFAULT, "No resets in this room.\n\r", ch );
      return;
    }

    if ( !IS_BUILDER( ch, ch->in_room->area ) )
    {
      send_to_char(C_DEFAULT, "Resets: Invalid security for editing this area.\n\r", ch );
      return;
    }

    pArea = ch->in_room->area;

    /*
     * Take index number and search for commands.
     * ------------------------------------------
     */
    if ( is_number( arg1 ) )
    {
      ROOM_INDEX_DATA *pRoom = ch->in_room;

      /*
       * Delete a reset.
       * ---------------
       */
      if ( !str_cmp( arg2, "delete" ) )
      {
	int insert_loc = atoi( arg1 );

	if ( !ch->in_room->reset_first )
	{
	  send_to_char(C_DEFAULT, "No resets in this room.\n\r", ch );
	  return;
	}

	if ( insert_loc-1 <= 0 )
	{
	  pReset = pRoom->reset_first;
	  pRoom->reset_first = pRoom->reset_first->next;
	  if ( !pRoom->reset_first )
	    pRoom->reset_last = NULL;	  

	  free_reset( pReset );
	  SET_BIT( pArea->area_flags, AREA_CHANGED );
	  send_to_char(C_DEFAULT, "Reset deleted.\n\r", ch );
	  return;
	}
	else
	{
	  int iReset = 0;
	  RESET_DATA *prev = NULL;

	  for ( pReset = pRoom->reset_first;
	        pReset;
	        pReset = pReset->next )
	  {
	    if ( ++iReset == insert_loc )
	      break;
	    prev = pReset;
	  }

	  if ( !pReset )
	  {
	    send_to_char(C_DEFAULT, "Reset not found.\n\r", ch );
	    return;
	  }

	  if ( prev )
	    prev->next = prev->next->next;
	  else
	    pRoom->reset_first = pRoom->reset_first->next;

	  for ( pRoom->reset_last = pRoom->reset_first;
	        pRoom->reset_last->next;
	        pRoom->reset_last = pRoom->reset_last->next );
	}

	free_reset( pReset );
        SET_BIT( pArea->area_flags, AREA_CHANGED );

	send_to_char(C_DEFAULT, "Reset deleted.\n\r", ch );
	return;
      }
/* Delete this garbage */
/*
      else if ( (!str_cmp( arg2, "mob" ) && is_number( arg3 ))
	     || (!str_cmp( arg2, "obj" ) && is_number( arg3 ))
	     || (!str_cmp( arg2, "ran" ) && is_number( arg3 )) )
*/
	/*
	 * Add a reset.
	 * ------------
	 */
/*
      {
*/
	/*
	 * Check for Mobile reset.
	 * -----------------------
	 */
/*
	if ( !str_cmp( arg2, "mob" ) )
        {
	  pReset = new_reset_data();
	  pReset->command = 'M';
	  pReset->arg1    = atoi( arg3 );
	  pReset->arg2    = is_number( arg4 ) ? atoi( arg4 ) : 1;
	  pReset->arg3    = ch->in_room->vnum;
	}
	else if ( !str_cmp( arg2, "obj" ) )
*/
	  /*
	   * Check for Object reset.
	   * -----------------------
	   */
/*
	{
	  pReset = new_reset_data();
	  pReset->arg1    = atoi( arg3 );
*/
	  /*
	   * Inside another object.
	   * ----------------------
	   */
/*
	  if ( !str_prefix( arg4, "inside" ) )
	  {
	    pReset->command = 'P';
	    pReset->arg2    = 0;
	    pReset->arg3    = is_number( arg5 ) ? atoi( arg5 ) : 1;
	  }
	  else if ( !str_cmp( arg4, "room" ) )
*/
	    /*
	     * Inside the room.
	     * ----------------
	     */
/*
	  {
	    pReset = new_reset_data();
	    pReset->command = 'O';
	    pReset->arg2     = 0;
	    pReset->arg3     = ch->in_room->vnum;
	  }
	  else
*/
	    /*
	     * Into a Mobile's inventory.
	     * --------------------------
	     */
/*
	  {
	    if ( flag_value( wear_loc_flags, arg4 ) == NO_FLAG )
	    {
	      send_to_char(C_DEFAULT, "Resets: '? wear-loc'\n\r", ch );
	      return;
	    }
	    pReset = new_reset_data();
	    pReset->arg3 = flag_value( wear_loc_flags, arg4 );
	    if ( pReset->arg2 == WEAR_NONE )
	      pReset->command = 'G';
	    else
	      pReset->command = 'E';
	  }
	}
	else if ( !str_cmp( arg2, "ran" ) )
*/
	  /*
	   * Random Exit Resets.
	   * Added By Altrag.
	   */
/*
	{
	  pReset = new_reset_data();
	  pReset->command = 'R';
	  pReset->arg1    = atoi( arg3 );
	  pReset->arg2    = ch->in_room->vnum;
        }
                                        
	add_reset( ch->in_room, pReset, atoi( arg1 ) );
	send_to_char(C_DEFAULT, "Reset added.\n\r", ch );
      }
      else
      {
	send_to_char(C_DEFAULT, "Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r", ch );
	send_to_char(C_DEFAULT, "        RESET <number> OBJ <vnum> in <vnum>\n\r", ch );
	send_to_char(C_DEFAULT, "        RESET <number> OBJ <vnum> room\n\r", ch );
	send_to_char(C_DEFAULT, "        RESET <number> MOB <vnum> [<max #>]\n\r", ch );
	send_to_char(C_DEFAULT, "        RESET <number> RAN <last-door>\n\r", ch );
*/
	send_to_char(C_DEFAULT, "Syntax: RESET <number> DELETE\n\r", ch );
/*
      }
*/
    }

    return;
}



/*****************************************************************************
 Name:		do_alist
 Purpose:	Normal command to list areas and display area information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_alist( CHAR_DATA *ch, char *argument )
{
    char buf    [ MAX_STRING_LENGTH ];
    char result [ MAX_STRING_LENGTH*4 ];	/* May need tweaking. */
    AREA_DATA *pArea;
    char *prot = "";

    sprintf( result, "[%3s] [%-20s] [%-10s] (%-5s-%5s) [%-10s] %3s [%-10s]\n\r",
       "Num", "Area Name", "Creator","lvnum", "uvnum", "Filename", "Sec", "Builders" );

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
      if ( IS_SET( pArea->area_flags, AREA_PROTOTYPE ) )
      prot = "*";
      else
      prot = "";
	sprintf( buf, "[%3d] %s%-22.22s %-10s (%-5d-%5d) %-12.12s [%d] [%-10.10s]\n\r",
	     pArea->vnum,
	     prot,
	     pArea->name,
	     pArea->creator,
	     pArea->lvnum,
	     pArea->uvnum,
	     pArea->filename,
	     pArea->security,
	     pArea->builders );
	     strcat( result, buf );
    }

    send_to_char(C_DEFAULT, result, ch );
    return;
}

/* XOR */
void hedit(CHAR_DATA *ch, char *argument)
{
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int cmd;

  smash_tilde(argument);
  strcpy(arg, argument);
  argument = one_argument(argument, command);

  if(command[0] == '\0')
  {
    hedit_show(ch, argument);
    return;
  }

  if(!str_cmp(command, "done"))
  {
    HELP_DATA *pHelp;

    EDIT_HELP(ch, pHelp);
    save_area(pHelp->area);
    edit_done( ch );
    return;
  }

  for(cmd = 0;*hedit_table[cmd].name;cmd++)
  {
    if(!str_prefix(command, hedit_table[cmd].name))
    {
      (*hedit_table[cmd].olc_fun) (ch, argument);
      return;
    }
  }

  /* Default to Standard Interpreter. */
  interpret(ch, arg);
  return;
}

HELP_DATA *get_help(char *argument)
{
  HELP_DATA *pHelp;
  for(pHelp = help_first;pHelp;pHelp = pHelp->next)
  {
    if(is_name(argument, pHelp->keyword))
    {
      return pHelp;
    }
  }
  return NULL;
}

void do_hedit(CHAR_DATA *ch, char *argument)
{
  HELP_DATA *pHelp;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_STRING_LENGTH*2];

  strcpy( arg, argument );
  argument = one_argument(argument, command);

  if(command[0] == 'c' && !str_prefix(command, "create"))
  {
    if(get_help(argument) != NULL)
    {
      send_to_char(C_DEFAULT, "Help entry already exist.\n\r", ch);
      return;
    }
      pHelp = new_help();
    if(!help_first)
      help_first = pHelp;
    if(help_last)
      help_last->next = pHelp;

    pHelp->area = help_last->area;
    help_last	= pHelp;
    pHelp->next	= NULL;
    top_help++;

    pHelp->level   = 107;
    if(pHelp->keyword)
      free_string(pHelp->keyword);
    pHelp->keyword = str_dup(argument);
    if(pHelp->text)
      free_string(pHelp->text);
  }
  else
  {
    if((pHelp = get_help(arg)) == NULL)
    {
      send_to_char(C_DEFAULT, "Help entry not found.\n\r", ch);
      return;
    }
  }
  ch->desc->pEdit = (void *) pHelp;
  ch->desc->editor = ED_HELP;
  ch->desc->inEdit = NULL;
  ch->desc->editin = 0;
  hedit_show(ch, "");
  return;
}
/* END */

void reledit( CHAR_DATA *ch, char *argument )
{
    RELIGION_DATA *pReligion;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int  cmd;

    EDIT_RELIGION(ch, pReligion);

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( !str_cmp( "done", command ) )
    {
        edit_done( ch );
        return;
    }
    
    if ( command[0] == '\0' )
    {
	reledit_show( ch, argument );
	return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; *reledit_table[cmd].name; cmd++ )
    {
	if ( !str_prefix( command, reledit_table[cmd].name ) )
	{
	    (*reledit_table[cmd].olc_fun) ( ch, argument );
	    return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}

void do_reledit( CHAR_DATA *ch, char *argument )
{
    RELIGION_DATA *pReligion;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );

    if ( is_number( command ) )
    {
	if ( !( pReligion = get_religion_index( atoi( command ) ) ))
	{
	    send_to_char(C_DEFAULT, "RELEdit:  That religion does not exist.\n\r", ch );
	    return;
	}

	ch->desc->pEdit = (void *)pReligion;
	ch->desc->editor = ED_RELIGION;
	reledit_show( ch, "" );
	return;
    }

    if ( command[0] == 'c' && !str_prefix( command, "create" ) )
    {
	if ( cedit_create( ch, argument ) )
	{
	    ch->desc->editor = ED_RELIGION;
	    reledit_show( ch, "" );
	}
	return;
    }

    send_to_char(C_DEFAULT, "RELEdit:  There is no default religion to edit.\n\r", ch );
    return;
}
void forge_object( CHAR_DATA *ch, OBJ_DATA *to_forge )
{
  ch->desc->pEdit = (void *)to_forge;
  ch->desc->editor = ED_FORGE;
  forge_show( ch, "" );
  return;
}
void forge_obj(CHAR_DATA *ch, char *argument)
{
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int cmd;

  smash_tilde(argument);
  strcpy(arg, argument);
  argument = one_argument(argument, command);

  if(command[0] == '\0')
  {
    forge_show(ch, argument);
    return;
  }

  if(!str_cmp(command, "done"))
  {
    edit_done( ch );
    save_char_obj( ch, FALSE );
    return;
  }

  /* Call editor function */
  for(cmd = 0;*forge_table[cmd].name;cmd++)
  {
    if(!str_prefix(command, forge_table[cmd].name))
    {
      if ( is_number( command )
      && atoi( command ) > ch->level / 10 )
	{
	send_to_char( AT_GREY, "You cannot forge so much at your level.\n\r", ch );
	return;
	}
      (*forge_table[cmd].olc_fun) (ch, argument);
      return;
    }
  }

  /* Default to Standard Interpreter. */
  interpret(ch, arg);
  return;
}
void do_forge( CHAR_DATA *ch, char *argument )
{
  char arg[ MAX_INPUT_LENGTH ];
  char arg2[ MAX_INPUT_LENGTH ];
  char buf[ MAX_INPUT_LENGTH ];
  OBJ_DATA *obj ;
  int wear, lvl;
  wear = 0;
  if ( argument[0] == '\0' )
	{
	send_to_char( AT_WHITE, "Syntax: Forge <obj> <race> <lvl>\n\r", ch );
	send_to_char( AT_WHITE, "  obj = ring necklace armor helm\n\r", ch );
	send_to_char( AT_WHITE, "        mask leggings boots gauntlets\n\r", ch );
	send_to_char( AT_WHITE, "        gauntlets armplates shield\n\r", ch );
	send_to_char( AT_WHITE, "        belt bracer anklet weapon\n\r", ch );
        send_to_char( AT_WHITE, "  race= any valid race. HELP FORGE RACES\n\r", ch );
	send_to_char( AT_WHITE, "        to see race groupings.\n\r", ch );
	sprintf( buf, "  lvl = minimum 30, maximum %d.\n\r", ch->level );
	send_to_char( AT_WHITE, buf, ch );
        send_to_char( AT_WHITE, "  BASE cost to make item is: 10 gold * lvl\n\r", ch );
	return;
	}
   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );
   if ( !str_prefix( arg, "ring" ) )
	wear = ITEM_WEAR_FINGER;
   if ( !str_prefix( arg, "necklace" ) )
	wear = ITEM_WEAR_NECK;
   if ( !str_prefix( arg, "armor" ) )
	wear = ITEM_WEAR_BODY;
   if ( !str_prefix( arg, "helm" ) )
	wear = ITEM_WEAR_HEAD;
   if ( !str_prefix( arg, "mask" ) )
	wear = ITEM_WEAR_FACE;
   if ( !str_prefix( arg, "leggings" ) )
	wear = ITEM_WEAR_LEGS;
   if ( !str_prefix( arg, "boots" ) )
	wear = ITEM_WEAR_FEET;
   if ( !str_prefix( arg, "gauntlets" ) )
	wear = ITEM_WEAR_HANDS;
   if ( !str_prefix( arg, "armplates" ) )
	wear = ITEM_WEAR_ARMS;
   if ( !str_prefix( arg, "shield" ) )
	wear = ITEM_WEAR_SHIELD;
   if ( !str_prefix( arg, "belt" ) )
	wear = ITEM_WEAR_WAIST;
   if ( !str_prefix( arg, "bracer" ) )
	wear = ITEM_WEAR_WRIST;
   if ( !str_prefix( arg, "anklet" ) )
	wear = ITEM_WEAR_ANKLE;
   if ( !str_prefix( arg, "weapon" ) )
	wear = ITEM_WIELD;
   if ( is_number( argument ) )
	lvl = atoi( argument );	
   else
	lvl = 0;
   if ( wear && ( lvl < 30 || lvl > ch->level ) )
	{
	sprintf( buf, "Illegal level.  Valid levels are 30 to %d.\n\r", ch->level );
	send_to_char( AT_GREY, buf, ch );
	return;
	}
   if ( wear )
	{
	if ( ch->gold < lvl * 100 )
	  {
	  send_to_char( AT_GREY, "You do not have enough money to create the base item of this level.\n\r", ch );
	  return;
	  }
	else
	if ( wear == ITEM_WIELD )
	obj = create_object( get_obj_index( OBJ_VNUM_TO_FORGE_W ), lvl );
	else
	obj = create_object( get_obj_index( OBJ_VNUM_TO_FORGE_A ), lvl );
	obj->cost = lvl * 100;
	obj->weight = (int)(lvl * 0.15);
	obj->level = lvl;
	if ( obj->level >= 101 )
	  obj->extra_flags += ITEM_NO_DAMAGE;
	obj->wear_flags += wear;
	obj->affected = 0;
	forge_object( ch, obj );
	}
   else
	do_forge( ch, "" );
   return;
}
