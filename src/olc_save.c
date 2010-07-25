/***************************************************************************
 *  File: olc_save.c                                                       *
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

/*$Id: olc_save.c,v 1.20 2005/04/11 03:25:02 tyrion Exp $*/

 /* OLC_SAVE.C
 * This takes care of saving all the .are information.
 * Notes:
 * -If a good syntax checker is used for setting vnum ranges of areas
 *  then it would become possible to just cycle through vnums instead
 *  of using the iHash stuff and checking that the room or reset or
 *  mob etc is part of that area.
 */

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

#ifdef SQL_SYSTEM
void Sleep(int);
#endif

/*****************************************************************************
 Name:		fix_string
 Purpose:	Returns a string without \r and ~.
 ****************************************************************************/
char *fix_string( const char *str )
{
    static char strfix[MAX_STRING_LENGTH];
    int i;
    int o;

    if ( str == NULL )
        return '\0';

    for ( o = i = 0; str[i+o] != '\0'; i++ )
    {
        if (str[i+o] == '\r' || str[i+o] == '~')
            o++;
        strfix[i] = str[i+o];
    }
    strfix[i] = '\0';
    return strfix;
}

/* This function exists for returning the name of a mobprog type when the
 * value of the type is given
 * TRI
 */

extern char *mprog_type_to_name( int type );

#ifndef SQL_SYSTEM

/*
{
   switch( type )
   {
    case IN_FILE_PROG:
      return "in_file_prog";
    case ACT_PROG:
      return "act_prog";
    case SPEECH_PROG:
      return "speech_prog";
    case RAND_PROG:
      return "rand_prog";
    case FIGHT_PROG:
      return "fight_prog";
    case HITPRCNT_PROG:
      return "hitprcnt_prog";
    case DEATH_PROG:
      return "death_prog";
    case ENTRY_PROG:
      return "entry_prog";
    case GREET_PROG:
      return "greet_prog";
    case ALL_GREET_PROG:
      return "all_greet_prog";
    case GIVE_PROG:
      return "give_prog";
    case BRIBE_PROG:
      return "bribe_prog";
    default:
      return "error_prog";
  }
  return "error_prog";
}
*/

/* TRI
 * This proc is the one that is originally called, it handles going from
 * one mob to the next until there are no more mobs with mobprogs from
 * the current pArea to have their mobprogs saved
 */
/*
 * Heh.. totally redid it so that it <should> work.. :)
 * Sorry TRI!.. :)
 * -- Altrag
 */
void save_mobprogs( FILE *fp, MOB_INDEX_DATA *pMob )
{
#ifdef SQL_SYSTEM
	sql_save_mprogs(pMob->mobprogs, pMob->vnum);
#else
  MPROG_DATA *mprog;

  if ( !pMob->mobprogs )
    return;
  
  for ( mprog = pMob->mobprogs; mprog != NULL; mprog = mprog->next )
  {
    if ( !str_cmp( mprog_type_to_name( mprog->type ), "error_prog" ) )
      continue;
    fprintf( fp, ">%s %s~\n", mprog_type_to_name( mprog->type ),
	    mprog->arglist );
    fprintf( fp, "%s~\n", fix_string( mprog->comlist ) );
  }
  fprintf( fp, "|\n" );
#endif
}

void save_traps( FILE *fp, TRAP_DATA *pFirst,
		 const struct flag_type *flags )
{
#ifdef SQL_SYSTEM
  bug("Save Traps: Sql System doesn't have enough info. Please traceback this call!", 0);
  return;
#else
  TRAP_DATA *pTrap;

  if ( !pFirst )
    return;

  for ( pTrap = pFirst; pTrap; pTrap = pTrap->next_here )
  {
    if ( !str_cmp( flag_string( flags, pTrap->type ), "error_prog" ) ||
	 !str_cmp( flag_string( flags, pTrap->type ), "none" ) )
      continue;
    fprintf( fp, ">%s %s~\n", flag_string( flags, pTrap->type ),
	     pTrap->arglist );
    fprintf( fp, "%d\n", pTrap->disarmable );
    fprintf( fp, "%s~\n", fix_string( pTrap->comlist ) );
  }
  fprintf( fp, "|\n" );
#endif
}

/*****************************************************************************
 Name:		save_area_list
 Purpose:	Saves the listing of files to be loaded at startup.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area_list()
{
    FILE *fp;
    AREA_DATA *pArea;

    if ( ( fp = fopen( ( "areaTS.lst" ), "w" ) ) == NULL )
    {
	bug( "Save_area_list: fopen", 0 );
	perror( "area.lst" );
    }
    else
    {
	/*
	 * Add any help files that need to be loaded at
	 * startup to this section.
	 */
/*
 *	- Kill this. mstrhelp.are == area 0 now
 *		- Ahsile
 *
 *
 *	fprintf( fp, "mstrhelp.are\n" );
 *
 */
	for( pArea = area_first; pArea; pArea = pArea->next )
	{
	    fprintf( fp, "%s\n", pArea->filename );
	}

	fprintf( fp, "$\n" );
	fclose( fp );
    }
    return;
}



/*****************************************************************************
 Name:		save_mobiles
 Purpose:	Save #MOBILES secion of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_mobiles( FILE *fp, AREA_DATA *pArea )
{
#ifdef SQL_SYSTEM
	sql_save_mobiles(pArea);
#else
    int vnum;
    MOB_INDEX_DATA *pMobIndex;

    fprintf( fp, "#MOBILES\n" );
    for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if( ( pMobIndex = get_mob_index(vnum) ) )
	{
	    if ( pMobIndex->area == pArea )
            {
                fprintf( fp, "#%d\n",		pMobIndex->vnum );
                fprintf( fp, "%s~\n",		pMobIndex->player_name );
                fprintf( fp, "%s~\n",		pMobIndex->short_descr );
                fprintf( fp, "%s~\n",		fix_string( pMobIndex->long_descr ) );
                fprintf( fp, "%s~\n",		fix_string( pMobIndex->description ) );
                fprintf( fp, "%d ",		pMobIndex->act );
                fprintf( fp, "%ld ",		pMobIndex->affected_by );
		fprintf( fp, "%ld ",		pMobIndex->affected_by2 );
		fprintf( fp, "%ld ",		pMobIndex->affected_by3 );
		fprintf( fp, "%ld ",		pMobIndex->affected_by4 );
                fprintf( fp, "%d S\n",		pMobIndex->alignment );
                fprintf( fp, "%d ",		pMobIndex->level );
		fprintf( fp, "%d ",		pMobIndex->size  );                
		fprintf( fp, "%d ",		pMobIndex->hitroll );
		fprintf( fp, "%d ",		pMobIndex->damroll );
                fprintf( fp, "%d ",		pMobIndex->ac );
                fprintf( fp, "%dd%d+%d ",	pMobIndex->hitnodice, 
                                                pMobIndex->hitsizedice, 
                                                pMobIndex->hitplus );
                fprintf( fp, "%dd%d+%d\n",	pMobIndex->damnodice, 
                                                pMobIndex->damsizedice, 
                                                pMobIndex->damplus );
                fprintf( fp, "%d ",		pMobIndex->gold );
		fprintf( fp, "%d ",		pMobIndex->speaking );
		fprintf( fp, "0\n0 0 " );
                fprintf( fp, "%d\n",  pMobIndex->sex );
		fprintf( fp, "%ld %ld %ld\n",  pMobIndex->imm_flags,
			                    pMobIndex->res_flags,
			                    pMobIndex->vul_flags );
		save_mobprogs( fp, pMobIndex );
            }
        }
    }
    fprintf( fp, "#0\n\n\n\n" );
#endif
    return;
}

/*****************************************************************************
 Name:		save_objects
 Purpose:	Save #OBJECTS section of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_objects( FILE *fp, AREA_DATA *pArea )
{
#ifdef SQL_SYSTEM
	sql_save_objects(pArea);
#else
    int vnum;
    OBJ_INDEX_DATA *pObjIndex;
    AFFECT_DATA *pAf;
    EXTRA_DESCR_DATA *pEd;

    fprintf( fp, "#OBJECTS\n" );
    for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if( ( pObjIndex = get_obj_index(vnum) ) )
	{
	    if ( pObjIndex->area == pArea )
            {
                fprintf( fp, "#%d\n",    pObjIndex->vnum );
                fprintf( fp, "%s~\n",    pObjIndex->name );
                fprintf( fp, "%s~\n",    pObjIndex->short_descr );
                fprintf( fp, "%s~\n",    fix_string( pObjIndex->description ) );
                fprintf( fp, "~\n" );
                fprintf( fp, "%d ",      pObjIndex->item_type );
                fprintf( fp, "%d ",      pObjIndex->extra_flags );
                fprintf( fp, "%d ",      pObjIndex->extra_flags2 );
                fprintf( fp, "%d ",      pObjIndex->extra_flags3 );
                fprintf( fp, "%d ",      pObjIndex->extra_flags4 );
                fprintf( fp, "%d ",     pObjIndex->wear_flags);
		fprintf( fp, "%d\n", pObjIndex->level );
		fprintf( fp, "%d %d\n", pObjIndex->durability_max, pObjIndex->durability_cur);
                switch ( pObjIndex->item_type )
                {
                default:
                fprintf( fp, "%d~ %d~ %d~ %d~\n",
			pObjIndex->value[0],
			pObjIndex->value[1],
			pObjIndex->value[2],
			pObjIndex->value[3] );
                    break;

                case ITEM_PILL:
                case ITEM_POTION:
                case ITEM_SCROLL:
                fprintf( fp, "%d~ %s~ %s~ %s~\n",
			pObjIndex->value[0],
			pObjIndex->value[1] != -1 ?
			      skill_table[pObjIndex->value[1]].name
			    : " ",
			pObjIndex->value[2] != -1 ?
			      skill_table[pObjIndex->value[2]].name
			    : " ",
			pObjIndex->value[3] != -1 ?
			      skill_table[pObjIndex->value[3]].name
			    : " " );
                    break;

                case ITEM_STAFF:
                case ITEM_LENSE:
		case ITEM_IMPLANTED:
		case ITEM_GUN:
                case ITEM_WAND:
                fprintf( fp, "%d~ %d~ %d~ %s~\n",
			pObjIndex->value[0],
			pObjIndex->value[1],
			pObjIndex->value[2],
			pObjIndex->value[3] != -1 ?
			      skill_table[pObjIndex->value[3]].name
			    : " " );
                    break;
                }
                fprintf( fp, "%d ", pObjIndex->weight );
                fprintf( fp, "%d 0\n", pObjIndex->cost );
                fprintf( fp, "%d ", pObjIndex->ac_type );
                fprintf( fp, "%d\n", pObjIndex->ac_vnum );
                fprintf( fp, "%s~\n", pObjIndex->ac_spell );
                fprintf( fp, "%d ", pObjIndex->ac_charge[0] );
                fprintf( fp, "%d\n", pObjIndex->ac_charge[1] );
		fprintf( fp, "%d %d %d\n", pObjIndex->join,
			 pObjIndex->sep_one, pObjIndex->sep_two );
                for( pAf = pObjIndex->affected; pAf; pAf = pAf->next )
                {
		if( pAf->location != 17 )
		{
                fprintf( fp, "A\n%d %d\n",  pAf->location, pAf->modifier );
		}
                }

                for( pEd = pObjIndex->extra_descr; pEd; pEd = pEd->next )
                {
                fprintf( fp, "E\n%s~\n%s~\n", pEd->keyword,
                                              fix_string( pEd->description ) );
                }
		save_traps( fp, pObjIndex->traps, oprog_types );
            }
        }
    }
    fprintf( fp, "#0\n\n\n\n" );
#endif
    return;
}



/* OLC 1.1b */
/*****************************************************************************
 Name:		save_rooms
 Purpose:	Save #ROOMDATA section of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_rooms( FILE *fp, AREA_DATA *pArea )
{
#ifdef SQL_SYSTEM
	sql_save_rooms(pArea);
#else
    ROOM_INDEX_DATA *pRoomIndex;
    EXTRA_DESCR_DATA *pEd;
    EXIT_DATA *pExit;
    int vnum;
    int door;

    fprintf( fp, "#ROOMDATA\n" );
    for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if( ( pRoomIndex = get_room_index(vnum) ) )
	{
	    if ( pRoomIndex->area == pArea )
	    {
		fprintf( fp, "#%d\n", pRoomIndex->vnum );
		fprintf( fp, "%s~\n", pRoomIndex->name );
		fprintf( fp, "%s~\n", pRoomIndex->soundfile );
		fprintf( fp, "%s~\n", pRoomIndex->musicfile );
		fprintf( fp, "%s~\n", fix_string( pRoomIndex->description ) );
		fprintf( fp, "0 " );
		fprintf( fp, "%d ", pRoomIndex->room_flags );
		fprintf( fp, "%d\n", pRoomIndex->sector_type );
		fprintf( fp, "Rd     %d\n", pRoomIndex->rd );

		for ( pEd = pRoomIndex->extra_descr; pEd; pEd = pEd->next )
                {
		    fprintf( fp, "E\n%s~\n%s~\n", pEd->keyword,
			fix_string( pEd->description ) );
                }

		for( door = 0; door < MAX_DIR; door++ )
		{
		    if ( ( pExit = pRoomIndex->exit[door] ) )
		    {
			fprintf( fp, "D%d\n", door );
			fprintf( fp, "%s~\n",
			    fix_string( pExit->description ) );
			fprintf( fp, "%s~\n", pExit->keyword );
			fprintf( fp, "%d %d %d\n", pExit->rs_flags,
			    pExit->key,
			    pExit->to_room ? pExit->to_room->vnum : 0 );
			save_traps( fp, pExit->traps, eprog_types );
		    }
		}
		fprintf( fp, "S\n" );
		save_traps( fp, pRoomIndex->traps, rprog_types );
	    }
	}
    }
    fprintf( fp, "#0\n\n\n\n" );
#endif
    return;
}


/* OLC 1.1b */
/*****************************************************************************
 Name:		save_specials
 Purpose:	Save #SPECIALS section of area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_specials( FILE *fp, AREA_DATA *pArea )
{
    int vnum;
    MOB_INDEX_DATA *pMobIndex;
    
    fprintf( fp, "#SPECIALS\n" );

    for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if( ( pMobIndex = get_mob_index(vnum) ) )
	{
	    if ( pMobIndex->area == pArea && pMobIndex->spec_fun )
            {
                fprintf( fp, "M %d %s\n", pMobIndex->vnum,
                                          spec_string( pMobIndex->spec_fun ) );
            }
        }
    }

    fprintf( fp, "S\n\n\n\n" );
    return;
}


/* OLC 1.1b */
/*****************************************************************************
 Name:		vsave_specials
 Purpose:	Save #SPECIALS section of area file.
 		New formating thanks to Rac.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void vsave_specials( FILE *fp, AREA_DATA *pArea )
{
    int vnum;
    MOB_INDEX_DATA *pMobIndex;
    
    fprintf( fp, "#SPECIALS\n" );

    for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if( ( pMobIndex = get_mob_index(vnum) ) )
	{
	    if ( pMobIndex->area == pArea && pMobIndex->spec_fun )
            {
                fprintf( fp, "M %d %s \t; %s\n", pMobIndex->vnum,
                                                 spec_string( pMobIndex->spec_fun ),
                                                 pMobIndex->short_descr );
            }
        }
    }

    fprintf( fp, "S\n\n\n\n" );
    return;
}


/* OLC 1.1b */
/*****************************************************************************
 Name:          save_games
 Purpose:       Save #GAMES section of area file.
 Called by:     save_area(olc_save.c).
 Written by:    Maniac
 ****************************************************************************/
void save_games( FILE *fp, AREA_DATA *pArea )
{
    int vnum;
    MOB_INDEX_DATA *pMobIndex;
    fprintf( fp, "#GAMES\n" );

    for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
        if( ( pMobIndex = get_mob_index(vnum) ) )
        {
            if ( pMobIndex->area == pArea && pMobIndex->game_fun )
            {
                fprintf( fp, "M %d %s~ %d 100 %d\n", pMobIndex->vnum,
                    game_string( pMobIndex->game_fun ),
                    pMobIndex->gold, pMobIndex->ac );
            }
        }
    }

    fprintf( fp, "S\n\n\n\n" );
    return;
}

/* OLC 1.1b */
/*****************************************************************************
 Name:          vsave_games
 Purpose:       Save #GAMES section of area file.
                New formating thanks to Rac.
 Called by:     save_area(olc_save.c).
 ****************************************************************************/
void vsave_games( FILE *fp, AREA_DATA *pArea )
{
    int vnum;
    MOB_INDEX_DATA *pMobIndex;

    fprintf( fp, "#GAMES\n" );

    for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
        if( ( pMobIndex = get_mob_index(vnum) ) )
        {
            if ( pMobIndex->area == pArea && pMobIndex->game_fun )
            {
                fprintf( fp, "M %d %s~ 5000 100 0\t; %s\n", pMobIndex->vnum,
                                                 game_string( pMobIndex->game_fun ),
                                                 pMobIndex->short_descr );
            }
        }
    }

    fprintf( fp, "S\n\n\n\n" );
    return;
}


/* OLC 1.1b */
/*****************************************************************************
 Name:		save_resets
 Purpose:	Saves the #RESETS section of an area file.
                New format thanks to Rac.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_resets( FILE *fp, AREA_DATA *pArea )
{
#ifdef SQL_SYSTEM
	sql_save_resets( pArea );
#else
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pLastMob = NULL;
    OBJ_INDEX_DATA *pLastObj;
    ROOM_INDEX_DATA *pRoomIndex;
    char buf[MAX_STRING_LENGTH];
    int vnum;

    fprintf( fp, "#RESETS\n" );

    for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if( ( pRoomIndex = get_room_index(vnum) ) )
	{
	    if ( pRoomIndex->area == pArea )
	    {
    for ( pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next )
    {
	switch ( pReset->command )
	{
	default:
	    bug( "Save_resets: bad command %c.", pReset->command );
	    break;

	case 'M':
            pLastMob = get_mob_index( pReset->arg1 );
	    fprintf( fp, "M %d %d %d %d\n", 
			pReset->status,
	        pReset->arg1,
                pReset->arg2,
                pReset->arg3 );
            break;

	case 'O':
            pLastObj = get_obj_index( pReset->arg1 );
	    fprintf( fp, "O %d %d 0 %d\n", 
			pReset->status,
	        pReset->arg1,
                pReset->arg3 );
            break;

	case 'P':
            pLastObj = get_obj_index( pReset->arg1 );
	    fprintf( fp, "P %d %d 0 %d\n", 
			pReset->status,
	        pReset->arg1,
                pReset->arg3  );
            break;

	case 'G':
	    fprintf( fp, "G %d %d 0\n", pReset->status, pReset->arg1 );
            if ( !pLastMob )
            {
                sprintf( buf,
                    "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                bug( buf, 0 );
            }
            break;

	case 'E':
	    fprintf( fp, "E %d %d 0 %d\n",
			pReset->status,
	        pReset->arg1,
                pReset->arg3 );
            if ( !pLastMob )
            {
                sprintf( buf,
                    "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                bug( buf, 0 );
            }
            break;

	case 'D':
            break;

	case 'R':
	    fprintf( fp, "R %d %d %d\n", 
			pReset->status,
	        pReset->arg1,
                pReset->arg2 );
            break;
        }	/* End switch */
    }	/* End for pReset */
	    }	/* End if correct area */
	}	/* End if pRoomIndex */
    }	/* End for vnum */
    fprintf( fp, "S\n\n\n\n" );
#endif
    return;
}



/* OLC 1.1b */
/*****************************************************************************
 Name:		save_resets
 Purpose:	Saves the #RESETS section of an area file.
                New format thanks to Rac.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void vsave_resets( FILE *fp, AREA_DATA *pArea )
{
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pLastMob = NULL;
    OBJ_INDEX_DATA *pLastObj;
    ROOM_INDEX_DATA *pRoomIndex;
    char buf[MAX_STRING_LENGTH];
    int vnum;

    fprintf( fp, "#RESETS\n" );

    for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if( ( pRoomIndex = get_room_index(vnum) ) )
	{
	    if ( pRoomIndex->area == pArea )
	    {
    for ( pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next )
    {
	switch ( pReset->command )
	{
	default:
	    bug( "Save_resets: bad command %c.", pReset->command );
	    break;

	case 'M':
            pLastMob = get_mob_index( pReset->arg1 );
	    fprintf( fp, "M 0 %d %2d %-5d \t; %s to %s\n", 
	        pReset->arg1,
                pReset->arg2,
                pReset->arg3,
                pLastMob->short_descr,
                pRoomIndex->name );
            break;

	case 'O':
            pLastObj = get_obj_index( pReset->arg1 );
	    fprintf( fp, "O 0 %d  0 %-5d \t; %s to %s\n", 
	        pReset->arg1,
                pReset->arg3,
                capitalize(pLastObj->short_descr),
                pRoomIndex->name );
            break;

	case 'P':
            pLastObj = get_obj_index( pReset->arg1 );
	    fprintf( fp, "P 0 %d  0 %-5d \t; %s inside %s\n", 
	        pReset->arg1,
                pReset->arg3,
                capitalize(get_obj_index( pReset->arg1 )->short_descr),
                pLastObj ? pLastObj->short_descr : "!NO_OBJ!" );

            if ( !pLastObj )	/* Thanks Rac! */
            {
                sprintf( buf, "Save_resets: P with !NO_OBJ! in [%s]", pArea->filename );
                bug( buf, 0 );
            }

            break;

	case 'G':
            pLastObj = get_obj_index( pReset->arg1 );
	    fprintf( fp, "G 0 %d  0      \t;   %s\n",
	        pReset->arg1,
	        capitalize(pLastObj->short_descr) );

            if ( !pLastMob )
            {
                sprintf( buf, "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                bug( buf, 0 );
            }
            break;

	case 'E':
	    fprintf( fp, "E 0 %d  0 %-5d \t;   %s %s\n",
	        pReset->arg1,
                pReset->arg3,
                capitalize(get_obj_index( pReset->arg1 )->short_descr),
                flag_string( wear_loc_strings, pReset->arg3 ) );
            if ( !pLastMob )
            {
                sprintf( buf, "Save_resets: !NO_MOB! in [%s]", pArea->filename );
                bug( buf, 0 );
            }
            break;

	case 'D':
            break;

	case 'R':
	    fprintf( fp, "R 0 %d %2d      \t; Randomize %s\n", 
	        pReset->arg1,
                pReset->arg2,
                pRoomIndex->name );
            break;
        }	/* End switch */
    }	/* End for pReset */
	    }	/* End if correct area */
	}	/* End if pRoomIndex */
    }	/* End for vnum */
    fprintf( fp, "S\n\n\n\n" );
    return;
}



/* OLC 1.1b */
/*****************************************************************************
 Name:		save_shops
 Purpose:	Saves the #SHOPS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_shops( FILE *fp, AREA_DATA *pArea )
{
    SHOP_DATA *pShopIndex;
    MOB_INDEX_DATA *pMobIndex;
    int iTrade;
    int vnum;
    
    fprintf( fp, "#SHOPS\n" );

    for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if( ( pMobIndex = get_mob_index(vnum) ) )
	{
	    if ( pMobIndex->area == pArea && pMobIndex->pShop )
            {
                pShopIndex = pMobIndex->pShop;

                fprintf( fp, "%d ", pShopIndex->keeper );
                for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
                {
                    if ( pShopIndex->buy_type[iTrade] != 0 )
                    {
                       fprintf( fp, "%d ", pShopIndex->buy_type[iTrade] );
                    }
                    else
                       fprintf( fp, "0 ");
                }
                fprintf( fp, "%d %d ", pShopIndex->profit_buy, pShopIndex->profit_sell );
                fprintf( fp, "%d %d\n", pShopIndex->open_hour, pShopIndex->close_hour );
            }
        }
    }

    fprintf( fp, "0\n\n\n\n" );
    return;
}



/* OLC 1.1b */
/*****************************************************************************
 Name:		vsave_shops
 Purpose:	Saves the #SHOPS section of an area file.
                New formating thanks to Rac.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void vsave_shops( FILE *fp, AREA_DATA *pArea )
{
    SHOP_DATA *pShopIndex;
    MOB_INDEX_DATA *pMobIndex;
    int iTrade;
    int vnum;
    
    fprintf( fp, "#SHOPS\n" );

    for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
	if( ( pMobIndex = get_mob_index(vnum) ) )
	{
	    if ( pMobIndex->area == pArea && pMobIndex->pShop )
            {
                pShopIndex = pMobIndex->pShop;

                fprintf( fp, "%d ", pShopIndex->keeper );
                for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
                {
                    if ( pShopIndex->buy_type[iTrade] != 0 )
                    {
                       fprintf( fp, "%2d ", pShopIndex->buy_type[iTrade] );
                    }
                    else
                       fprintf( fp, " 0 ");
                }
                fprintf( fp, "%3d %3d ", pShopIndex->profit_buy, pShopIndex->profit_sell );
                fprintf( fp, "%2d %2d ", pShopIndex->open_hour, pShopIndex->close_hour );
                fprintf( fp, "\t; %s\n", get_mob_index( pShopIndex->keeper )->short_descr );
            }
        }
    }

    fprintf( fp, "0\n\n\n\n" );
    return;
}

#endif

/*****************************************************************************
 Name:		save_helps
 Purpose:	Save #HELPS section of an area file.
 Written by:	Walker <nkrendel@evans.Denver.Colorado.EDU>
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_helps( FILE *fp, AREA_DATA *pArea )
{
    HELP_DATA *pHelp;
    bool found = FALSE;

    for( pHelp = help_first; pHelp; pHelp = pHelp->next )
    {
        if( pHelp->area && pHelp->area == pArea )
	{
	    if( !found )
	    {
		fprintf( fp, "#HELPS\n\n" );
		found = TRUE;
	    }
	    fprintf( fp, "%d %s~\n%s~\n", 
		pHelp->level,
		all_capitalize( pHelp->keyword ), 
		fix_string( pHelp->text ) );
	}
    }

    if( found )
        fprintf( fp, "\n0 $~\n\n" );

    return;
}



/*****************************************************************************
 Name:		save_area
 Purpose:	Save an area, note that this format is new.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area( AREA_DATA *pArea )
{
#ifdef SQL_SYSTEM
	sql_save_area( pArea );
#else
    FILE *fp;

    fclose( fpReserve );
    if ( !( fp = fopen( pArea->filename, "w" ) ) )
    {
	bug( "Open_area: fopen", 0 );
	perror( pArea->filename );
    }

    fprintf( fp, "#AREADATA\n" );
    fprintf( fp, "Name        %s~\n",        pArea->name );
    fprintf( fp, "Builders    %s~\n",        fix_string( pArea->builders ) );
    fprintf( fp, "VNUMs       %d %d\n",      pArea->lvnum, pArea->uvnum );
    fprintf( fp, "Security    %d\n",         pArea->security );
    fprintf( fp, "Recall      %d\n",         pArea->recall );
    fprintf( fp, "Flags       %d\n",         pArea->area_flags );
    fprintf( fp, "Version     %d\n",	     pArea->version );
    fprintf( fp, "Creator     %s~\n",	     pArea->creator );
    fprintf( fp, "Llevel      %d\n",         pArea->llevel );
    fprintf( fp, "Ulevel      %d\n",         pArea->ulevel );
    if ( pArea->reset_sound )
      fprintf( fp, "Sounds      %s~\n",      pArea->reset_sound );
    if ( pArea->actual_sound )
      fprintf( fp, "SoundsA     %s~\n",      pArea->actual_sound );
    if ( pArea->musicfile )
      fprintf( fp, "Music       %s~\n",      pArea->musicfile );
    fprintf( fp, "End\n\n\n\n" );

    save_helps( fp, pArea );				/* OLC 1.1b */
    save_mobiles( fp, pArea );
    save_objects( fp, pArea );
    save_rooms( fp, pArea );

    if ( IS_SET(pArea->area_flags, AREA_VERBOSE) )	/* OLC 1.1b */
    {
	vsave_specials( fp, pArea );
        vsave_games( fp, pArea );
	vsave_resets( fp, pArea );
	vsave_shops( fp, pArea );
    }
    else
    {
	save_specials( fp, pArea );
        save_games( fp, pArea );
	save_resets( fp, pArea );
	save_shops( fp, pArea );
    }

    fprintf( fp, "#$\n" );

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
#endif
    return;
}


/* OLC 1.1b */
/*****************************************************************************
 Name:		do_asave
 Purpose:	Entry point for saving area data.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_asave( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    AREA_DATA *pArea;
    int value;
#ifdef SQL_SYSTEM
	char buf[MAX_STRING_LENGTH];
	time_t start;
#endif

    if ( !ch )       /* Do an autosave */
	{
	#ifndef SQL_SYSTEM
	save_area_list();
	#else
	int acount = 0;
	start = time(NULL);
	sql_save_socials();
	sql_save_clans();
	sql_save_religions();
	sql_save_bans();
	sql_save_disabled();
	sql_save_shops();
	sql_save_helps();
	#endif

	for( pArea = area_first; pArea; pArea = pArea->next )
	{
	  if ( IS_SET( pArea->area_flags, AREA_CHANGED ) ) 
	    {
		if (IS_SET( pArea->area_flags, AREA_CORRUPT ))
		{
			// nothing, don't save it
		} else
		{
#ifdef SQL_SYSTEM
			acount++;
#endif
	         REMOVE_BIT( pArea->area_flags, AREA_CHANGED | AREA_ADDED );
			 save_area( pArea );
		}
	    }
	}
#ifdef SQL_SYSTEM
	sprintf(buf, "Autosave: Saved %d areas in %d seconds", acount, (int)(time(NULL)-start));
	log_string(buf, CHANNEL_LOG, -1);
#endif
	return;
    }

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
    send_to_char(C_DEFAULT, "Syntax:\n\r", ch );
    send_to_char(C_DEFAULT, "  asave <vnum>    - saves a particular area\n\r",	 ch );
#ifndef SQL_SYSTEM
    send_to_char(C_DEFAULT, "  asave list      - saves the area.lst file\n\r",	 ch );
#endif
    send_to_char(C_DEFAULT, "  asave area      - saves the area being edited\n\r", ch );
    send_to_char(C_DEFAULT, "  asave changed   - saves all changed zones\n\r",	 ch );
    send_to_char(C_DEFAULT, "  asave world     - saves the world! (db dump)\n\r",	 ch );
#ifndef SQL_SYSTEM
    send_to_char(C_DEFAULT, "  asave ^ verbose - saves in verbose mode\n\r", ch );
#endif
    send_to_char(C_DEFAULT, "\n\r", ch );
        return;
    }

    /* Snarf the value (which need not be numeric). */
    value = atoi( arg1 );

    /* Save the area of given vnum. */
    /* ---------------------------- */

    if ( !( pArea = get_area_data( value ) ) && is_number( arg1 ) )
    {
	send_to_char(C_DEFAULT, "That area does not exist.\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) )
    {
	if ( !IS_BUILDER( ch, pArea ) )
	{
	    send_to_char(C_DEFAULT, "You are not a builder for this area.\n\r", ch );
	    return;
	} else if (IS_SET(pArea->area_flags, AREA_CORRUPT))
	{
	    char buf[MAX_STRING_LENGTH];
	    sprintf(buf, "CORRUPTION: Area %s is flagged as corrupt and will not be saved. Please contact your administrator!\n\r", pArea->name);
	    send_to_char(C_DEFAULT, buf, ch);
	    return;
	}

	#ifndef SQL_SYSTEM
	save_area_list();
	if ( !str_cmp( "verbose", argument ) )
	    SET_BIT( pArea->area_flags, AREA_VERBOSE );
	save_area( pArea );
	REMOVE_BIT( pArea->area_flags, AREA_VERBOSE | AREA_CHANGED | AREA_ADDED );
	act( C_DEFAULT, "Area $t saved.", ch, arg1, NULL, TO_CHAR );
	return;
	#endif
    }

    /* Save the world, only authorized areas. */
    /* -------------------------------------- */

    if ( !str_cmp( "world", arg1 ) )
    {
	#ifndef SQL_SYSTEM
	save_area_list();
	#else
	start = time(NULL);

	sql_save_socials();
	sql_save_clans();
	sql_save_religions();
	sql_save_bans();
	sql_save_disabled();
	sql_save_shops();
	sql_save_helps();
	#endif
	for( pArea = area_first; pArea; pArea = pArea->next )
	{
	    /* Builder must be assigned this area. */
	    if ( !IS_BUILDER( ch, pArea ) )
		continue;	  

	    if ( !str_cmp( "verbose", argument ) )
		SET_BIT( pArea->area_flags, AREA_VERBOSE );
	
	    if (IS_SET(pArea->area_flags, AREA_CORRUPT))
	    {
		char buf[MAX_STRING_LENGTH];
	        sprintf(buf, "CORRUPTION: Area %s is flagged as corrupt and will not be saved. Please contact your administrator!\n\r", pArea->name);
	    	bug( buf, 0);
		continue;
	    }
    	    save_area( pArea );
	    REMOVE_BIT( pArea->area_flags, AREA_CHANGED | AREA_ADDED | AREA_VERBOSE );
	}
	send_to_char(C_DEFAULT, "You saved the world.\n\r", ch );
#ifdef SQL_SYSTEM
	sprintf(buf, "ASAVE WORLD: By %s. Took %d seconds.", ch->name, (int)(time(NULL)-start));
	log_string(buf, CHANNEL_LOG, -1);
	sprintf(buf, "It took %d seconds!\n\r", (int)(time(NULL)-start));
	send_to_char(C_DEFAULT, buf, ch);
#endif
	send_to_al(C_DEFAULT, 0, "World saved to Database (We apologize for the delay).\n\r");
	return;
    }

    if ( !str_cmp( "clans", arg1 ) )
    {
      save_clans( );
      return;
    }

    if ( !str_cmp( "religions", arg1 ) )
    {
	save_religions( );
	return;
    }
      
    /* Save changed areas, only authorized areas. */
    /* ------------------------------------------ */

    if ( !str_cmp( "changed", arg1 ) )
    {
	char buf[MAX_INPUT_LENGTH];
	#ifndef SQL_SYSTEM
	save_area_list();
	#endif

	send_to_char(C_DEFAULT, "Saved zones:\n\r", ch );
	sprintf( buf, "None.\n\r" );

	for( pArea = area_first; pArea; pArea = pArea->next )
	{
	    /* Builder must be assigned this area. */
	    if ( !IS_BUILDER( ch, pArea ) )
		continue;

	    /* Save changed areas. */
	    if ( IS_SET(pArea->area_flags, AREA_CHANGED)
	      || IS_SET(pArea->area_flags, AREA_ADDED) )
	    {
		if ( !str_cmp( "verbose", argument ) )
		    SET_BIT( pArea->area_flags, AREA_VERBOSE );
		if (IS_SET(pArea->area_flags, AREA_CORRUPT))
		{
		    char buf[MAX_STRING_LENGTH];
		    sprintf(buf, "CORRUPTION: Area %s is flagged as corrupt and will not be saved. Please contact your administrator!\n\r", pArea->name);
	   	    bug(buf,0);
		    continue;
		}
		save_area( pArea );
		REMOVE_BIT( pArea->area_flags, AREA_CHANGED | AREA_ADDED | AREA_VERBOSE );
		sprintf( buf, "%24s - '%s'\n\r", pArea->name, pArea->filename );
		send_to_char(C_DEFAULT, buf, ch );
	    }
        }
	if ( !str_cmp( buf, "None.\n\r" ) )
	    send_to_char(C_DEFAULT, buf, ch );
        return;
    }

#ifndef SQL_SYSTEM
    /* Save the area.lst file. */
    /* ----------------------- */
    if ( !str_cmp( arg1, "list" ) )
    {
	save_area_list();
	return;
    }
#endif

    /* Save area being edited, if authorized. */
    /* -------------------------------------- */
    if ( !str_cmp( arg1, "area" ) )
    {	
	/* Find the area to save. */
	switch (ch->desc->editor)
	{
	    case ED_AREA:
		pArea = (AREA_DATA *)ch->desc->pEdit;
		break;
	    case ED_ROOM:
		pArea = ch->in_room->area;
		break;
	    case ED_OBJECT:
		pArea = ( (OBJ_INDEX_DATA *)ch->desc->pEdit )->area;
		break;
	    case ED_MOBILE:
		pArea = ( (MOB_INDEX_DATA *)ch->desc->pEdit )->area;
		break;
	    default:
		pArea = ch->in_room->area;
		break;
	}

	if ( !IS_BUILDER( ch, pArea ) )
	{
	    send_to_char(C_DEFAULT, "You are not a builder for this area.\n\r", ch );
	    return;
	}

	#ifndef SQL_SYSTEM
	save_area_list();
	#endif
	if ( !str_cmp( "verbose", argument ) )
	    SET_BIT( pArea->area_flags, AREA_VERBOSE );
	save_area( pArea );
	REMOVE_BIT( pArea->area_flags, AREA_CHANGED | AREA_ADDED | AREA_VERBOSE );
	send_to_char(C_DEFAULT, "Area saved.\n\r", ch );
	return;
    }

    /* Show correct syntax. */
    /* -------------------- */
   do_asave( ch, "" );
    return;
}

void save_religions( )
{
#ifdef SQL_SYSTEM
	sql_save_religions();
#else
    FILE *fp;
    RELIGION_DATA *pReligion;

    if ( ( fp = fopen( "religion.dat", "w" ) ) == NULL )
    {
        bug( "Save_religion: fopen", 0 );
        perror( "area.lst" );
    }
    else
    {
        for( pReligion = religion_first; pReligion; pReligion = pReligion->next )
        {
            fprintf( fp, "#%d\n",	pReligion->vnum );
            fprintf( fp, "%s~\n",	pReligion->name );
	    fprintf( fp, "%s~\n", 	pReligion->shortdesc );
            fprintf( fp, "%s~\n",	pReligion->deity );
            fprintf( fp, "%s~\n",	pReligion->description );
            fprintf( fp, "%d\n",	pReligion->recall );
            fprintf( fp, "%d\n",	pReligion->start  );
       	    fprintf( fp, "%d %d ",	pReligion->pkills, pReligion->mkills );
            fprintf( fp, "%d %d ",	pReligion->pdeaths, pReligion->mdeaths );
	    fprintf( fp, "%d\n",	pReligion->members );
        }

        fprintf( fp, "#999\n\n" );
        fclose( fp );
    }
#endif
    return;
}

void save_clans( )
{
#ifdef SQL_SYSTEM
	sql_save_clans();
#else
    FILE *fp; 
    CLAN_DATA *pClan;

    if ( ( fp = fopen( "clan.dat", "w" ) ) == NULL )
    {
	bug( "Save_clans: fopen", 0 );
	perror( "area.lst" );
    }
    else
    {
	for( pClan = clan_first; pClan; pClan = pClan->next )
	{
	    fprintf( fp, "#%d\n",      pClan->vnum );
	    fprintf( fp, "%s~\n",      pClan->name );
	    fprintf( fp, "%s~\n",      pClan->deity );
	    fprintf( fp, "%s~\n",      pClan->description );
	    fprintf( fp, "%s~ %s~ %s~ %s~\n", 
	             pClan->champ, pClan->leader,
	             pClan->first, pClan->second );
	    fprintf( fp, "%d %d %d %d\n",
	             pClan->ischamp, pClan->isleader,
	             pClan->isfirst, pClan->issecond );
	    fprintf( fp, "%d\n",       pClan->recall );
	    fprintf( fp, "%d %d ",      pClan->pkills, pClan->mkills );
	    fprintf( fp, "%d\n",       pClan->members );
	    fprintf( fp, "%d %d\n",    pClan->pdeaths, pClan->mdeaths );
	    fprintf( fp, "%d %d %d\n", pClan->obj_vnum_1, pClan->obj_vnum_2, pClan->obj_vnum_3 );
	    fprintf( fp, "%d\n",     pClan->pkill );
	}

	fprintf( fp, "#999\n\n" );
	fclose( fp );
    }
#endif
    return;
}
