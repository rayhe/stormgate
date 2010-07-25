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
 *  around, comes around.
 *
 ***************************************************************************/

/*$Id: save.c,v 1.26 2005/03/23 01:36:17 tyrion Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

#if !defined( macintosh )
extern	int	_filbuf		args( (FILE *) );
#endif

#if defined( ultrix ) || defined( sequent )
void    system          args( ( char *string ) );
#endif

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[ MAX_NEST ];

// external func
void ch_stripbadinv 	args( ( CHAR_DATA* ch ) );

#ifdef SQL_SYSTEM

#endif

/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
			       FILE *fp, int iNest, bool storage ) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp, bool storage ) );
void   fread_pet        args( ( CHAR_DATA *ch, FILE *fp ) );
void    save_pet        args( ( CHAR_DATA *ch, FILE *fp, CHAR_DATA *pet ) );
void    add_alias  args( ( CHAR_DATA *ch, ALIAS_DATA *pAl, char *old,
				char *new ) );
void    fwrite_alias    args( ( CHAR_DATA *ch, FILE *fp ) );
void    fread_alias     args( ( CHAR_DATA *ch, FILE *fp ) );

bool    replace_missing_skills args( ( CHAR_DATA* ch, char* missing, int value ) );

char    NULL_STRING[MAX_STRING_LENGTH];
/* Courtesy of Yaz of 4th Realm */
char *initial( const char *str )
{
    static char strint [ MAX_STRING_LENGTH ];

    strint[0] = LOWER( str[ 0 ] );
    return strint;

}
/* Check to see if a player exists */
bool _stat( char *name )
{
  bool found;
  char strsave[256];
  CHAR_DATA *ch = 0;
  FILE *fp;

  ch->name = str_dup( name );

  sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( ch->name ), "/",
       capitalize( name ) );

  found = ( fp = fopen( strsave, "r" ) ) != NULL;
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
  return found;
}

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch, bool leftgame )
{
#ifdef SQL_SYSTEM
	sql_save_dirty_char(ch, leftgame);
#else
    FILE *fp;
    CHAR_DATA *pet;
    char  buf     [ MAX_STRING_LENGTH ];
    char  strsave [ MAX_INPUT_LENGTH  ];
    /*
    RELIGION_DATA *pReligion;
    */

    if ( IS_NPC( ch ) )
	return;
    if (ch->level < 2)
      return;
   if ( ch->desc && ch->desc->connected != CON_PLAYING)
       return;
    if ( ch->desc && ch->desc->original )
	ch = ch->desc->original;

    ch->save_time = current_time;
    fclose( fpReserve );

    /* player files parsed directories by Yaz 4th Realm */
#if !defined( macintosh ) && !defined( MSDOS )
		sprintf( strsave, "%s%s%s%s", PLAYER_DIR,
				initial( ch->name ), "/", capitalize( ch->name ) );
#else
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
#endif
	fp = fopen( strsave, "w" );
    if ( !fp )
    {
        sprintf( buf, "Save_char_obj: fopen %s: ", ch->name );
		bug( buf, 0 );
		perror( strsave );
    }
    else
    {
      fwrite_char( ch, fp );
      if ( ch->carrying )
	fwrite_obj( ch, ch->carrying, fp, 0, FALSE );
      if ( !IS_NPC( ch ) && ch->pcdata->storage )
	fwrite_obj( ch, ch->pcdata->storage, fp, 0, TRUE );
      for ( pet = ch->in_room->people; pet; pet = pet->next_in_room )
      {
	if (IS_NPC( pet ) )
	  if ( IS_SET( pet->act, ACT_PET ) && ( pet->master == ch ) )
	  {
/*	    save_pet( ch, fp, pet ); */
	    break;
	  }
      }
      tail_chain();
      fprintf( fp, "#END\n" );
    }
    fclose( fp );
#if !defined( macintosh ) && !defined( MSDOS )
	if( leftgame )
    {
        sprintf( buf, "gzip -1fq %s", strsave );

		system( buf );
    }
#endif
    fpReserve = fopen( NULL_FILE, "r" );
#endif // SQL_SYSTEM
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int          sn;

    fprintf( fp, "#%s\n", IS_NPC( ch ) ? "MOB" : "PLAYER"	);

    fprintf( fp, "Nm           %s~\n",	ch->name		);
    if ( ch->pcdata->lname )
      fprintf( fp, "Lnm          %s~\n",   ch->pcdata->lname       );
    fprintf( fp, "ShtDsc       %s~\n",	ch->short_descr		);
    fprintf( fp, "LngDsc       %s~\n",	ch->long_descr		);
    fprintf( fp, "Dscr         %s~\n",	ch->description		);
    fprintf( fp, "Prmpt        %s~\n",	ch->prompt		);
    fprintf( fp, "Sx           %d\n",	ch->sex			);
    fprintf( fp, "Cla          %d\n",	ch->class		);
    fprintf( fp, "Mlt          %d\n",	ch->multied		);
    fprintf( fp, "Race         %s~\n",   race_table[ ch->race ].race_full
);
    fprintf( fp, "Clan         %d\n",    ch->clan                );
    fprintf( fp, "Updated      %d\n",	ch->updated		);
    fprintf( fp, "Clvl         %d\n",    ch->clev                );
    fprintf( fp, "Ctmr         %d\n",    ch->ctimer              );
    fprintf( fp, "Stun         %d %d %d %d %d\n",
	     ch->stunned[0], ch->stunned[1], ch->stunned[2],
	     ch->stunned[3], ch->stunned[4] );
    fprintf( fp, "ResAcid      %d\n",    ch->damage_mods[0]	);
    fprintf( fp, "ResHoly      %d\n",    ch->damage_mods[1]	);
    fprintf( fp, "ResMagic     %d\n",    ch->damage_mods[2]	);
    fprintf( fp, "ResFire      %d\n",    ch->damage_mods[3]	);
    fprintf( fp, "ResEnergy    %d\n",    ch->damage_mods[4]	);
    fprintf( fp, "ResWind      %d\n",    ch->damage_mods[5]	);
    fprintf( fp, "ResWater     %d\n",    ch->damage_mods[6]	);
    fprintf( fp, "ResIllusion  %d\n",    ch->damage_mods[7]	);
    fprintf( fp, "ResDispel    %d\n",    ch->damage_mods[8]	);
    fprintf( fp, "ResEarth     %d\n",    ch->damage_mods[9]	);
    fprintf( fp, "ResPsychic   %d\n",    ch->damage_mods[10]	);
    fprintf( fp, "ResPoison    %d\n",    ch->damage_mods[11]	);
    fprintf( fp, "ResBreath    %d\n",    ch->damage_mods[12]	);
    fprintf( fp, "ResSUmmon    %d\n",    ch->damage_mods[13]	);
    fprintf( fp, "ResPhysical  %d\n",    ch->damage_mods[14]	);
    fprintf( fp, "ResExplosive %d\n",    ch->damage_mods[15]	);
    fprintf( fp, "ResSong      %d\n",    ch->damage_mods[16]	);
    fprintf( fp, "ResNagarom   %d\n",    ch->damage_mods[17]	);
    fprintf( fp, "ResUnholy    %d\n",    ch->damage_mods[18]	);
    fprintf( fp, "ResClan      %d\n",    ch->damage_mods[19]	);
    fprintf( fp, "WizLev       %d\n",    ch->wizinvis            );
    fprintf( fp, "Lvl          %d\n",	ch->level		);
    fprintf( fp, "Pkill        %d\n",	ch->pkill		);
    fprintf( fp, "PoisonLevel  %d\n",	ch->poison_level	);
    fprintf( fp, "Antidisarm   %d\n",    ch->antidisarm          );
    fprintf( fp, "Mounted      %d\n",	ch->mounted		);
    fprintf( fp, "Mountcharmed %d\n",	ch->mountcharmed	);
    fprintf( fp, "Mountshort   %s~\n",   ch->mountshort		);
    fprintf( fp, "Trst         %d\n",	ch->trust		);
    fprintf( fp, "Security     %d\n",    ch->pcdata->security	);  /* OLC */
    fprintf( fp, "Wizbt        %d\n",	ch->wizbit		);
    fprintf( fp, "ArenaWins    %d\n",	ch->pcdata->awins	);
    fprintf( fp, "ArenaLoses   %d\n",	ch->pcdata->alosses	);
    fprintf( fp, "MobKills     %d\n",   ch->pcdata->mobkills    );
    fprintf( fp, "Wizbt        %d\n",	ch->wizbit		);
    fprintf( fp, "Playd        %d\n",
	ch->played + (int) ( current_time - ch->logon )		);
    fprintf( fp, "Note         %ld\n",   ch->last_note           );
    fprintf( fp, "Room         %d\n",
		( IS_SET(ch->in_room->area->area_flags, AREA_NO_SAVE) ?
		(ch->in_room->area->recall) : (
	    (  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
	     && ch->was_in_room )
	    ? ch->was_in_room->vnum
	    : ch->in_room->vnum)) );

    fprintf( fp, "HpMnMvBp     %d %d %d %d %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->bp, ch->max_bp );
    fprintf( fp, "Gold         %d\n",	ch->gold		);
    fprintf( fp, "GRank        %d\n",	ch->guild_rank		);
    fprintf( fp, "Guild        %s~\n",
     (ch->guild != NULL) ? ch->guild->name : "\0"		);
    fprintf( fp, "RNumber      %d\n",	ch->religion		);
    fprintf( fp, "Recall       %d\n",    ch->pcdata->recall         );
    fprintf( fp, "Rtimer       %d\n",	ch->rtimer		   );
    fprintf( fp, "Exp          %d\n",	ch->exp			);
    fprintf( fp, "Act          %d\n",    ch->act			);
    fprintf( fp, "Act2         %d\n",    ch->act2 		);
    fprintf( fp, "AffdBy       %ld\n",	ch->affected_by		);
    fprintf( fp, "AffdBy2      %ld\n",    ch->affected_by2        );
    fprintf( fp, "AffdBy3      %ld\n",    ch->affected_by3        );
    fprintf( fp, "AffdBy4      %ld\n",    ch->affected_by4        );
    fprintf( fp, "AffdByp      %ld\n",	 ch->affected_by_powers	 );
    fprintf( fp, "AffdByw      %ld\n",	 ch->affected_by_weaknesses );
    fprintf( fp, "ImmBits      %ld\n",    ch->imm_flags		);
    fprintf( fp, "ResBits      %ld\n",	ch->res_flags		);
    fprintf( fp, "VulBits      %ld\n",	ch->vul_flags		);
    fprintf( fp, "Shields      %d\n",	ch->shields		);
    /* Bug fix from Alander */
    fprintf( fp, "Pos         %d\n",
	    ch->position == POS_FIGHTING ? POS_STANDING : ch->position );

    fprintf( fp, "Prac         %d\n",	ch->practice		);
    fprintf( fp, "SavThr       %d\n",	ch->saving_throw	);
    fprintf( fp, "Align        %d\n",	ch->alignment		);
    fprintf( fp, "Hit          %d\n",	ch->hitroll		);
    fprintf( fp, "Dam          %d\n",	ch->damroll		);
    fprintf( fp, "Armr         %d\n",	ch->armor		);
    fprintf( fp, "Wimp         %d\n",	ch->wimpy		);
    fprintf( fp, "Deaf         %d\n",	ch->deaf		);
    fprintf( fp, "Corpses      %d\n",    ch->pcdata->corpses	);
    fprintf( fp, "QuestPnts    %d\n",	ch->questpoints		);
    fprintf( fp, "QuestNext    %d\n",	ch->nextquest		);
    fprintf( fp, "QuestCount   %d\n",	ch->countdown           );
    fprintf( fp, "QuestObj     %d\n",	ch->questobj		);
    fprintf( fp, "QuestMob     %d\n",	ch->questmob		);
    if (ch->questgiver)
    fprintf( fp, "QuestGiver   %d\n",    ch->questgiver->pIndexData->vnum );
    fprintf( fp, "RQuestPnts   %d\n",    ch->rquestpoints        );
    fprintf( fp, "RQuestNext   %d\n",    ch->rnextquest          );

    if ( IS_NPC( ch ) )
    {
	fprintf( fp, "Vnum         %d\n",	ch->pIndexData->vnum	);
    }
    else
    {
	fprintf( fp, "Paswd        %s~\n",	ch->pcdata->pwd		);
        fprintf( fp, "Speak        %d\n",        ch->speaking    );
        fprintf( fp, "Learn        %d\n",        ch->pcdata->learn      );
	fprintf( fp, "Bmfin        %s~\n",	ch->pcdata->bamfin	);
	fprintf( fp, "Bmfout       %s~\n",	ch->pcdata->bamfout	);
	fprintf( fp, "Bmfsin       %s~\n",	ch->pcdata->bamfsin	);
	fprintf( fp, "Bmfsout      %s~\n",	ch->pcdata->bamfsout	);
	fprintf( fp, "Bank         %d\n",        ch->pcdata->bankaccount);
	fprintf( fp, "BankShares   %d\n", 	ch->pcdata->shares	);
        fprintf( fp, "Immskll      %s~\n",       ch->pcdata->immskll    );
	fprintf( fp, "Ttle         %s~\n",	ch->pcdata->title	);
        fprintf( fp, "WhoTxt       %s~\n",       ch->pcdata->who_text   );
	fprintf( fp, "AtrPrm       %d %d %d %d %d\n",
		ch->pcdata->perm_str,
		ch->pcdata->perm_int,
		ch->pcdata->perm_wis,
		ch->pcdata->perm_dex,
		ch->pcdata->perm_con );

	fprintf( fp, "AtrMd        %d %d %d %d %d\n",
		ch->pcdata->mod_str,
		ch->pcdata->mod_int,
		ch->pcdata->mod_wis,
		ch->pcdata->mod_dex,
		ch->pcdata->mod_con );

	fprintf( fp, "Cond         %d %d %d\n",
		ch->pcdata->condition[0],
		ch->pcdata->condition[1],
		ch->pcdata->condition[2] );

	fprintf( fp, "Pglen        %d\n",   ch->pcdata->pagelen     );

        for ( sn = 0; sn < MAX_LANGUAGE; sn++)
        {
            fprintf( fp, "Lang        %d '%s'\n",
                ch->language[sn], lang_table[sn].name );
        }

	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( skill_table[sn].name && ch->pcdata->learned[sn] > 0 )
	    {
		fprintf( fp, "Skll        %d '%s'\n",
		    ch->pcdata->learned[sn], skill_table[sn].name );
	    }
	}
    }

    for ( paf = ch->affected; paf; paf = paf->next )
    {
        if ( paf->deleted )
	    continue;

	fprintf( fp, "Aff       %3d %3d %3d %3d %10d\n",
		skill_table[paf->type].slot,
		paf->duration,
		paf->modifier,
		paf->location,
		paf->bitvector );
    }
    for ( paf = ch->affected2; paf; paf = paf->next )
    {
        if ( paf->deleted )
	    continue;

	fprintf( fp, "Aff2       %3d %3d %3d %3d %10d\n",
		skill_table[paf->type].slot,
		paf->duration,
		paf->modifier,
		paf->location,
		paf->bitvector );
    }
    for ( paf = ch->affected3; paf; paf = paf->next )
    {
        if ( paf->deleted )
	    continue;

	fprintf( fp, "Aff3       %3d %3d %3d %3d %10d\n",
		skill_table[paf->type].slot,
		paf->duration,
		paf->modifier,
		paf->location,
		paf->bitvector );
    }
    for ( paf = ch->affected4; paf; paf = paf->next )
    {
        if ( paf->deleted )
	    continue;

	fprintf( fp, "Aff4       %3d %3d %3d %3d %10d\n",
		skill_table[paf->type].slot,
		paf->duration,
		paf->modifier,
		paf->location,
		paf->bitvector );
    }
    for ( paf = ch->affected_powers; paf; paf = paf->next )
    {
        if ( paf->deleted )
            continue;

        fprintf( fp, "Affp       %3d %3d %3d %3d %10d\n",
                skill_table[paf->type].slot,
                paf->duration,
                paf->modifier,
                paf->location,
                paf->bitvector );
    }
    for ( paf = ch->affected_weaknesses; paf; paf = paf->next )
    {
        if ( paf->deleted )
            continue;

        fprintf( fp, "Affw       %3d %3d %3d %3d %10d\n",
                skill_table[paf->type].slot,
                paf->duration,
                paf->modifier,
                paf->location,
                paf->bitvector );
    }
    if ( ch->pcdata && ch->pcdata->alias_list )
    {
      ALIAS_DATA *pAl;

      for ( pAl = ch->pcdata->alias_list; pAl; pAl = pAl->next )
	fprintf( fp, "Alias      %s~ %s~\n",
		pAl->old, pAl->_new );
    }

    fprintf( fp, "End\n\n" );
    return;
}



/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest,
		 bool storage )
{
    AFFECT_DATA      *paf;
    EXTRA_DESCR_DATA *ed;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content )
	fwrite_obj( ch, obj->next_content, fp, iNest, storage );

    /*
     * Castrate storage characters.
     */
/* Changed it so that objects are now saved, even if higher level. */
/*
    if ( ( ch->level < obj->level && ch->class == ch->multied )
	|| obj->item_type == ITEM_KEY
	|| obj->deleted )
	return;
*/
    if ( obj->item_type == ITEM_KEY || obj->deleted )
	return;

    if ( obj->item_type == ITEM_VODOO || obj->deleted )
        return;

    if ( storage )
      fprintf( fp, "#STORAGE\n" );
    else
      fprintf( fp, "#OBJECT\n" );
    fprintf( fp, "Nest         %d\n",	iNest			     );
    fprintf( fp, "Name         %s~\n",	obj->name		     );
    fprintf( fp, "ShortDescr   %s~\n",	obj->short_descr	     );
    fprintf( fp, "Description  %s~\n",	obj->description	     );
    fprintf( fp, "Vnum         %d\n",	obj->pIndexData->vnum	     );
    fprintf( fp, "DurabilityMax	%d\n",	obj->durability_max	     );
    fprintf( fp, "DurabilityCur	%d\n",	obj->durability_cur	     );
    fprintf( fp, "ExtraFlags   %d\n",	obj->extra_flags	     );
    fprintf( fp, "ExtraFlags2  %d\n",	obj->extra_flags2	     );
    fprintf( fp, "ExtraFlags3  %d\n",   obj->extra_flags3            );
    fprintf( fp, "ExtraFlags4  %d\n",   obj->extra_flags4            );
    fprintf( fp, "WearFlags    %d\n",	obj->wear_flags		     );
    fprintf( fp, "WearLoc      %d\n",	obj->wear_loc		     );
    fprintf( fp, "ItemType     %d\n",	obj->item_type		     );
    fprintf( fp, "Weight       %d\n",	obj->weight		     );
    fprintf( fp, "Level        %d\n",	obj->level		     );
    fprintf( fp, "Timer        %d\n",	obj->timer		     );
    fprintf( fp, "Cost         %d\n",	obj->cost		     );
    fprintf( fp, "Values       %d %d %d %d\n",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3]   );
    fprintf( fp, "Activates    %d %d %d %d\n",
        obj->ac_type, obj->ac_vnum, obj->ac_charge[0], obj->ac_charge[1] );
    fprintf( fp, "AcSpell      %s~\n",  obj->ac_spell                );
    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
    case ITEM_IMPLANTED:
	if ( obj->value[1] > 0 )
	{
	    fprintf( fp, "Spell 1      '%s'\n",
		skill_table[obj->value[1]].name );
	}

	if ( obj->value[2] > 0 )
	{
	    fprintf( fp, "Spell 2      '%s'\n",
		skill_table[obj->value[2]].name );
	}

	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n",
		skill_table[obj->value[3]].name );
	}

	break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_LENSE:
    case ITEM_GUN:
    case ITEM_WAND:
	if ( obj->value[3] > 0 )
	{
	    fprintf( fp, "Spell 3      '%s'\n",
		skill_table[obj->value[3]].name );
	}

	break;
	default:
	break;
    }

    for ( paf = obj->affected; paf; paf = paf->next )
    {
	fprintf( fp, "Affect       %d %d %d %d %d\n",
		skill_table[paf->type].slot,
		paf->duration,
		paf->modifier,
		paf->location,
		paf->bitvector );
    }

    for ( ed = obj->extra_descr; ed; ed = ed->next )
    {
	fprintf( fp, "ExtraDescr   %s~ %s~\n",
		ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains )
	fwrite_obj( ch, obj->contains, fp, iNest + 1, storage );

    tail_chain();
    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
#ifdef SQL_SYSTEM
		bool found;
		CHAR_DATA* ch = sql_load_char( name );

		if (ch)
		{
			ch->desc = d;
			d->character = ch;
			return TRUE;
		} else
		{
			ch = new_char();
			clear_char(ch);
			ch->desc = d;
			ch->new = TRUE;
			d->character = ch;
			return FALSE;
		}
#else
         FILE      *fp;
         static    PC_DATA    pcdata_zero;
         CHAR_DATA *ch;
         char buf [ MAX_STRING_LENGTH ];

#if !defined( MSDOS )
#endif
	   char       strsave [ MAX_INPUT_LENGTH ];
	   bool       found;

	memset(&pcdata_zero, 0, sizeof(PC_DATA));
	ch				= new_char();
    clear_char( ch );

	ch->pcdata			= new_pc();
    *ch->pcdata				= pcdata_zero;

    d->character			= ch;
    ch->desc				= d;

    ch->name				= str_dup( name );
    ch->prompt                          = str_dup( "<%hhp %mm %vmv> " );
    ch->last_note                       = 0;
    ch->fixed_error 			= FALSE;
    ch->act				= PLR_BLANK
					| PLR_COMBINE
					| PLR_PROMPT
					| PLR_ANSI
					| PLR_COMBAT;
    ch->act2				= 0;
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->bamfsin			= str_dup( "" );
    ch->pcdata->bamfsout		= str_dup( "" );
    ch->pcdata->immskll                 = str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
    ch->pcdata->who_text                = str_dup( "" );
    ch->pcdata->awins			= 0;
    ch->pcdata->alosses			= 0;
    if(!IS_NPC( ch ) )
    {
	ch->pcdata->perm_str		= 0;
	ch->pcdata->perm_int		= 0;
	ch->pcdata->perm_wis		= 0;
	ch->pcdata->perm_dex		= 0;
	ch->pcdata->perm_con		= 0;
    }
    if(IS_NPC( ch ) )
    {
	ch->pcdata->perm_str		= 13;
	ch->pcdata->perm_int		= 13;
	ch->pcdata->perm_wis		= 13;
	ch->pcdata->perm_dex		= 13;
	ch->pcdata->perm_con		= 13;
    }
    ch->pcdata->condition[COND_THIRST]	= 48;
    ch->pcdata->condition[COND_FULL]	= 48;
    ch->pcdata->pagelen                 = 20;
    ch->pcdata->security		= 0;	/* OLC */

    ch->pcdata->switched                = FALSE;
    ch->combat_timer			= 0;	/* XOR */
    ch->summon_timer			= 0;	/* XOR */
    ch->imm_flags			= 0;	/* XOR */
    ch->res_flags			= 0;	/* XOR */
    ch->vul_flags			= 0;	/* XOR */
    ch->guild				= NULL;	/* XOR */
    ch->pcdata->bankaccount		= 0;	/* TRI */
    ch->pcdata->shares			= 0;
    ch->pcdata->alias_list		= NULL; /* TRI */
    ch->pcdata->corpses			= 0;	/* Tyrion */
    ch->poison_level			= 0;	/* Tyrion */

    found = FALSE;
    fclose( fpReserve );

    /* parsed player file directories by Yaz of 4th Realm */
    /* decompress if .gz file exists - Thx Alander */

#if !defined( macintosh ) && !defined( MSDOS )
			 sprintf( strsave, "%s%s%s%s.gz", PLAYER_DIR, initial( ch->name ),
            "/", capitalize( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) )
    {

        fclose( fp );
        sprintf( buf, "gzip -dfq %s", strsave );
        system( buf );
    }
#endif

#if !defined( macintosh ) && !defined( MSDOS )
    sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( ch->name ),
          "/", capitalize( name ) );
#else
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
#endif

    if ( ( fp = fopen( strsave, "r" ) ) )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	for ( ; ; )
	{
	    char  letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	         if ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
	    else if ( !str_cmp( word, "OBJECT" ) ) fread_obj ( ch, fp, FALSE );
	    else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
	    else if ( !str_cmp( word, "STORAGE" ) ) fread_obj ( ch, fp, TRUE );
	    else if ( !str_cmp( word, "END"    ) ) break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		break;
	    }
	}

	ch_stripbadinv( ch );

	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
#endif

    return found;
}



/*
 * Read in a char.
 */

#if defined( KEY )
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_char( CHAR_DATA *ch, FILE *fp )
{
    char *word;
    char *race;
    char  buf [ MAX_STRING_LENGTH ];
    bool  fMatch;
    int   temp;

    ch->religion = 0;
    ch->pcdata->recall = RECALL_RELIGION_DEFAULT;
    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER( word[0] ) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Act",		ch->act,		fread_number( fp ) );
	    KEY( "Act2",	ch->act2,		fread_number( fp ) );
	    KEY( "AffdBy",	ch->affected_by,	fread_number( fp ) );
	    KEY( "AffdBy2",     ch->affected_by2,       fread_number( fp ) );
	    KEY( "AffdBy3",     ch->affected_by3,       fread_number( fp ) );
	    KEY( "AffdBy4",     ch->affected_by4,       fread_number( fp ) );
	    KEY( "AffdByp",	ch->affected_by_powers, fread_number( fp ) );
	    KEY( "AffdByw",	ch->affected_by_weaknesses, fread_number( fp ) );
	    KEY( "Align",	ch->alignment,		fread_number( fp ) );
	    KEY( "Antidisarm",  ch->antidisarm,         fread_number( fp ) );
	    KEY( "ArenaWins",   ch->pcdata->awins,      fread_number( fp ) );
	    KEY( "ArenaLoses",  ch->pcdata->alosses,    fread_number( fp ) );
	    KEY( "Armr",	ch->armor,		fread_number( fp ) );

            if ( IS_SET( ch->act2, PLR_RELQUEST) )
	    {
		REMOVE_BIT( ch->act2, PLR_RELQUEST);
		ch->rnextquest = number_range(10, 30);
		ch->rcountdown = 0;
	    }

	    if ( !str_cmp( word, "Aff" ) )
	    {
		AFFECT_DATA *paf;

		    paf		= new_affect();

		paf->type	= fread_number( fp );
		paf->type       = slot_lookup(paf->type);
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->deleted    = FALSE;
		paf->next	= ch->affected;
		ch->affected	= paf;
		if ( !is_sn(paf->type) || paf->type == 0 )
		  paf->deleted = TRUE;
		fMatch = TRUE;
		break;
	    }
	    if ( !str_cmp( word, "Aff2" ) )
	    {
		AFFECT_DATA *paf;

		    paf		= new_affect();

		paf->type	= fread_number( fp );
		paf->type       = slot_lookup(paf->type);
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->deleted    = FALSE;
		paf->next	= ch->affected2;
		ch->affected2	= paf;
		if ( !is_sn(paf->type) || paf->type == 0 )
		  paf->deleted = TRUE;
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Aff3" ) )
	    {
		AFFECT_DATA *paf;

		    paf		= new_affect();

		paf->type	= fread_number( fp );
		paf->type       = slot_lookup(paf->type);
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->deleted    = FALSE;
		paf->next	= ch->affected3;
		ch->affected3	= paf;
		if ( !is_sn(paf->type) || paf->type == 0 )
		  paf->deleted = TRUE;
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Aff4" ) )
	    {
		AFFECT_DATA *paf;

		    paf		= new_affect();

		paf->type	= fread_number( fp );
		paf->type       = slot_lookup(paf->type);
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->deleted    = FALSE;
		paf->next	= ch->affected4;
		ch->affected4	= paf;
		if ( !is_sn(paf->type) || paf->type == 0 )
		  paf->deleted = TRUE;
		fMatch = TRUE;
		break;
	    }
            if ( !str_cmp( word, "Affp" ) )
            {
                AFFECT_DATA *paf;

                    paf         = new_affect();

                paf->type       = fread_number( fp );
                paf->type       = slot_lookup(paf->type);
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->deleted    = FALSE;
                paf->next       = ch->affected_powers;
                ch->affected_powers   = paf;
                if ( !is_sn(paf->type) || paf->type == 0 )
                  paf->deleted = TRUE;
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "Affw" ) )
            {
                AFFECT_DATA *paf;

                    paf         = new_affect();

                paf->type       = fread_number( fp );
                paf->type       = slot_lookup(paf->type);
                paf->duration   = fread_number( fp );
                paf->modifier   = fread_number( fp );
                paf->location   = fread_number( fp );
                paf->bitvector  = fread_number( fp );
                paf->deleted    = FALSE;
                paf->next       = ch->affected_weaknesses;
                ch->affected_weaknesses   = paf;
                if ( !is_sn(paf->type) || paf->type == 0 )
                  paf->deleted = TRUE;
                fMatch = TRUE;
                break;
            }

	    if ( !str_cmp( word, "Alias" ) )
	    {
		ALIAS_DATA *pAl;

		fMatch = TRUE;
		if ( !ch->pcdata )
		{
		  bug("Fread_char: Alias without pcdata",0);
		  fread_string(fp);
		  fread_string(fp);
		  break;
		}
		pAl = new_alias();
		pAl->old = fread_string(fp);
		pAl->_new = fread_string(fp);
		pAl->next = ch->pcdata->alias_list;
		ch->pcdata->alias_list = pAl;
		break;
	    }

	    if ( !str_cmp( word, "AtrMd"  ) )
	    {
		ch->pcdata->mod_str  = fread_number( fp );
		ch->pcdata->mod_int  = fread_number( fp );
		ch->pcdata->mod_wis  = fread_number( fp );
		ch->pcdata->mod_dex  = fread_number( fp );
		ch->pcdata->mod_con  = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AtrPrm" ) )
	    {
		ch->pcdata->perm_str = fread_number( fp );
		ch->pcdata->perm_int = fread_number( fp );
		ch->pcdata->perm_wis = fread_number( fp );
		ch->pcdata->perm_dex = fread_number( fp );
		ch->pcdata->perm_con = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'B':
	    KEY( "Bmfin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEY( "Bmfout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    KEY( "Bmfsin",	ch->pcdata->bamfsin,	fread_string( fp ) );
	    KEY( "Bmfsout",	ch->pcdata->bamfsout,	fread_string( fp ) );
	    KEY( "Bank",        ch->pcdata->bankaccount, fread_number( fp ) );
	    KEY( "BankShares",	ch->pcdata->shares,	fread_number( fp ) );
	    break;

	case 'C':
	    KEY( "Clan",        ch->clan,               fread_number( fp ) );
	    KEY( "Clvl",        ch->clev,               fread_number( fp ) );
	    KEY( "Ctmr",        ch->ctimer,             fread_number( fp ) );
	    KEY( "Cla", 	ch->class,		fread_number( fp ) );
	    KEY( "Corpses",	ch->pcdata->corpses,	fread_number( fp ) );

	    if ( !str_cmp( word, "Cond" ) )
	    {
		ch->pcdata->condition[0] = fread_number( fp );
		ch->pcdata->condition[1] = fread_number( fp );
		ch->pcdata->condition[2] = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'D':
	    KEY( "Dam", 	ch->damroll,		fread_number( fp ) );
 	    KEY( "Deaf",	ch->deaf,		fread_number( fp ) );
	    KEY( "Dscr",	ch->description,	fread_string( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
	        /* Coders have "insane" trusts for a reason ELVIS *
		 * -- Altrag                                      */
	        if ( ch->trust > L_CON && ch->level < L_IMP && !IS_CODER(ch) )
		  ch->trust = 0;
		return;
	    }
	    KEY( "Exp",		ch->exp,		fread_number( fp ) );
	    KEY( "Email",	ch->pcdata->email,	fread_string( fp ) );
	    break;

	case 'G':
	    KEY( "Gold",	ch->gold,		fread_number( fp ) );
/* XOR */
	    KEY( "GRank",	ch->guild_rank,		fread_number( fp ) );
	    if(!str_cmp(word, "Guild"))
            {
              int i;
              char *guild;
              guild = fread_string(fp);
              fMatch = TRUE;
              for(i = 0;guild_table[i].name[0] != '\0';i++)
              {
                if(!strcmp(guild_table[i].name, guild))
                {
                  ch->guild = &(guild_table[i]);
                  break;
                }
              }
            }
	    break;

	case 'H':
	    KEY( "Hit", 	ch->hitroll,		fread_number( fp ) );

	    if ( !str_cmp( word, "HpMnMvBp" ) )
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		ch->move	= fread_number( fp );
		ch->max_move	= fread_number( fp );
		ch->bp          = fread_number( fp );
		ch->max_bp      = fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    break;
        case 'I':
            KEY ( "Immskll",    ch->pcdata->immskll,    fread_string( fp ) );
	    KEY( "ImmBits",	ch->imm_flags,		fread_number( fp ) );
            break;
	case 'L':
	    KEY( "Lnm",         ch->pcdata->lname,       fread_string( fp ) );
	    KEY( "Lvl", 	ch->level,		fread_number( fp ) );
            KEY( "Learn",       ch->pcdata->learn,      fread_number( fp ) );

            if ( !str_cmp( word, "Lang" ) )
            {
                int ln;
                int value;

                value = fread_number( fp );
                ln    = lang_lookup( fread_word( fp ) );
                if ( ln < 0 )
                    bug( "Fread_char: unknown language.", 0 );
                else
                    ch->language[ln] = value;
                fMatch = TRUE;
            }


	    if ( !str_cmp( word, "LngDsc" ) )
	    {
	      fread_to_eol( fp );
	      fMatch = TRUE;
	      break;
	    }
	    break;

        case 'M':
	    KEY( "Mlt", 	ch->multied,		fread_number( fp ) );
	    KEY( "Mounted",	ch->mounted,		fread_number( fp ) );
	    KEY( "Mountcharmed",ch->mountcharmed,	fread_number( fp ) );
	    KEY( "Mountshort",	ch->mountshort,		fread_string( fp ) );
            KEY( "MobKills",	ch->pcdata->mobkills,	fread_number( fp ) );

	case 'N':
	    if ( !str_cmp( word, "Nm" ) )
	    {
		/*
		 * Name already set externally.
		 */
		fread_to_eol( fp );
		fMatch = TRUE;
		break;
	    }
	    KEY( "Note",        ch->last_note,          fread_number( fp ) );
	    break;

	case 'P':
	    KEY( "Pglen",       ch->pcdata->pagelen,    fread_number( fp ) );
	    KEY( "Paswd",	ch->pcdata->pwd,	fread_string( fp ) );
	    KEY( "Playd",	ch->played,		fread_number( fp ) );
	    KEY( "PoisonLevel",	ch->poison_level,	fread_number( fp ) );
	    KEY( "Pos", 	ch->position,		fread_number( fp ) );
	    KEY( "Prac",	ch->practice,		fread_number( fp ) );
   	    KEY( "Pkill",	ch->pkill,		fread_number( fp ) );
	    KEY( "Prmpt",	ch->prompt,		fread_string( fp ) );
	    KEY( "Plan",	ch->pcdata->plan,	fread_string( fp ) );
	    break;

        case 'Q':
            KEY( "QuestPnts",   ch->questpoints,        fread_number( fp ) );
            KEY( "QuestNext",   ch->nextquest,          fread_number( fp ) );
	    KEY( "QuestCount",	ch->countdown,		fread_number( fp ) );
	    KEY( "QuestObj",	ch->questobj,		fread_number( fp ) );
	    KEY( "QuestMob", 	ch->questmob,		fread_number( fp ) );
	    KEY( "QuestGiver",  ch->questgiver,         get_char_world( ch, get_mob_index( fread_number( fp) )->player_name));
            break;


	case 'R':
	    KEY( "Rce",         ch->race,		fread_number( fp ) );
            KEY( "Recall",      ch->pcdata->recall,     fread_number( fp ) );
	    KEY( "Religion",	ch->nullstring,		fread_string( fp ) );
	    KEY( "ReligionN",	ch->nullstring,		fread_string( fp ) );
            KEY( "ResBits",     ch->res_flags,          fread_number( fp ) );
           KEY( "RNumber",     ch->religion,		fread_number( fp ) );
    KEY( "Rtimer",	ch->rtimer,		fread_number( fp ) );
    KEY( "RQuestPnts",	ch->rquestpoints,	fread_number( fp ) );
    KEY( "RQuestNext",	ch->rnextquest,		fread_number( fp ) );
    KEY( "ResAcid",	ch->damage_mods[0], 	fread_number( fp ));
    KEY( "ResHoly",	ch->damage_mods[1],	fread_number(fp));
    KEY( "ResMagic",	ch->damage_mods[2],	fread_number(fp));
    KEY( "ResFire",	ch->damage_mods[3],	fread_number(fp));
    KEY( "ResEnergy",	ch->damage_mods[4],	fread_number(fp));
    KEY( "ResWind",	ch->damage_mods[5],	fread_number(fp));
    KEY( "ResWater",	ch->damage_mods[6],	fread_number(fp));
    KEY( "ResIllusion",	ch->damage_mods[7],	fread_number(fp));
    KEY( "ResDispel",	ch->damage_mods[8],	fread_number(fp));
    KEY( "ResEarth",	ch->damage_mods[9],	fread_number(fp));
    KEY( "ResPsychic",	ch->damage_mods[10],	fread_number(fp));
    KEY( "ResPoison",	ch->damage_mods[11],	fread_number(fp));
    KEY( "ResBreath",	ch->damage_mods[12],	fread_number(fp));
    KEY( "ResSummon",	ch->damage_mods[13],	fread_number(fp));
    KEY( "ResPhysical",	ch->damage_mods[14],	fread_number(fp));
    KEY( "ResExplosive",	ch->damage_mods[15],fread_number(fp ));
    KEY( "ResSong",	ch->damage_mods[16],	fread_number(fp));
    KEY( "ResNagarom",	ch->damage_mods[17],	fread_number(fp));
    KEY( "ResUnholy",	ch->damage_mods[18],	fread_number(fp));
    KEY( "ResClan",	ch->damage_mods[19],	fread_number(fp));
    KEY( "RQuestCount", temp, 			fread_number(fp));
    KEY( "RQuestObj",   temp,			fread_number(fp));
    KEY( "RQuestMob",	temp,			fread_number(fp));

            if ( !str_cmp( word, "Race" ) )
            {
                race     = fread_string( fp );
                ch->race = race_lookup( race );
                fMatch   = TRUE;
                break;
            }

	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( !ch->in_room )
		    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
                if ( ch->in_room->vnum >= REL_VNUM_LOWER &&
		     ch->in_room->vnum <= REL_VNUM_UPPER )
		{
		    REMOVE_BIT(ch->act2, PLR_RELQUEST); // Just in case
		    ch->rnextquest = number_range(15, 30);
		    ch->in_room = get_room_index( ROOM_VNUM_TEMPLE );
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'S':
	    KEY( "SavThr",	ch->saving_throw,	fread_number( fp ) );
	    KEY( "Sx",		ch->sex,		fread_number( fp ) );
	    if ( !str_cmp( word, "Stun" ) )
	    {
	      ch->stunned[0] = fread_number( fp );
	      ch->stunned[1] = fread_number( fp );
	      ch->stunned[2] = fread_number( fp );
	      ch->stunned[3] = fread_number( fp );
	      ch->stunned[4] = fread_number( fp );
	      fMatch = TRUE;
	      break;
	    }
	    if ( !str_cmp( word, "ShtDsc" ) )
	    {
	      fread_to_eol( fp );
	      fMatch = TRUE;
	      break;
	    }
	    KEY( "Security",    ch->pcdata->security,	fread_number( fp ) );	/* OLC */
	    KEY( "Shields",	ch->shields,	fread_number( fp ) );
            KEY( "Speak",       ch->speaking,   fread_number( fp ) );

	    if ( !str_cmp( word, "Skll" ) )
	    {
		int sn;
		int value;

		value = fread_number( fp );
		sn    = skill_lookup( fread_word( fp ) );
		if ( sn < 0 )
		{
			if (!replace_missing_skills(ch, word, value))
				log_string("Fread_char: unknown skill.", CHANNEL_BUILD, L_APP);
		}
		else
		    ch->pcdata->learned[sn] = value;
		fMatch = TRUE;
	    }

	    break;

	case 'T':
	    KEY( "Trst",	ch->trust,		fread_number( fp ) );
            /* keeps people from having obsene trusts --- ELVIS */
	    /* The insane trusts have a purpose -- Altrag */

	    if ( !str_cmp( word, "Ttle" ) )
	    {
		ch->pcdata->title = fread_string( fp );
		if (   isalpha( ch->pcdata->title[0] )
		    || isdigit( ch->pcdata->title[0] ) )
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    free_string( ch->pcdata->title );
		    ch->pcdata->title = str_dup( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'U':
	    ch->updated = 0;
	    KEY( "Updated",	ch->updated,	fread_number( fp ) );

	    break;

	case 'V':
	    KEY( "VulBits",	ch->vul_flags,	fread_number( fp ) );

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "Wimp",	ch->wimpy,		fread_number( fp ) );
            KEY( "WhoTxt",      ch->pcdata->who_text,   fread_string( fp ) );
	    KEY( "Wizbt",	ch->wizbit,		fread_number( fp ) );
	    KEY( "WizLev",      ch->wizinvis,           fread_number( fp ) );
	    break;
	}

	/* Make sure old chars have this field - Kahn */
	if ( !ch->pcdata->pagelen )
	    ch->pcdata->pagelen = 20;
	if ( !ch->prompt || ch->prompt == '\0' )
	    ch->prompt = str_dup ( "<%hhp %mm %vmv> " );

	/* Make sure old chars do not have pagelen > 60 - Kahn */
	if ( ch->pcdata->pagelen > 60 )
	    ch->pcdata->pagelen = 60;

	if ( !fMatch )
	{
	    sprintf(buf, "Fread_char: no match :: %s", word);
	    bug( buf, 0 );
	    fread_to_eol( fp );
	}
    }
}



void fread_obj( CHAR_DATA *ch, FILE *fp, bool storage )
{
    static OBJ_DATA  obj_zero;
           OBJ_DATA *obj;
           char     *word;
           int       iNest;
           bool      fMatch;
           bool      fNest;
           bool      fVnum;

	obj		= new_obj();

    *obj		= obj_zero;
    obj->durability_max = 100;
    obj->durability_cur = 100;
    obj->name		= str_dup( "" );
    obj->short_descr	= str_dup( "" );
    obj->description	= str_dup( "" );
    obj->deleted        = FALSE;

    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER( word[0] ) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
            if ( !str_cmp( word, "Activates") )
	    {
		obj->ac_type	  = fread_number( fp );
		obj->ac_vnum	  = fread_number( fp );
		obj->ac_charge[0] = fread_number( fp );
		obj->ac_charge[1] = fread_number( fp );
		fMatch		= TRUE;
		break;
	    }

            if ( !str_cmp( word, "AcSpell" ) )
               {
  		char* tmp	  = fread_string( fp );

		if (!str_cmp(tmp, str_empty))
			tmp = str_dup( "reserved" );

                obj->ac_spell     = tmp;
                fMatch            = TRUE;
                break;
               }
	    if ( !str_cmp( word, "Affect" ) )
	    {
		AFFECT_DATA *paf;

		    paf		= new_affect();

		paf->type	= fread_number( fp );
		paf->type       = slot_lookup(paf->type);
		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_number( fp );
		paf->next	= obj->affected;
		obj->affected	= paf;
		fMatch		= TRUE;
		break;
	    }
	    break;

	case 'C':
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    KEY( "DurabilityMax", obj->durability_max, 	fread_number( fp ) );
	    KEY( "DurabilityCur", obj->durability_cur, 	fread_number( fp ) );
	    break;

	case 'E':
	    KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
	    KEY( "ExtraFlags2",	obj->extra_flags2,	fread_number( fp ) );
	    KEY( "ExtraFlags3", obj->extra_flags3,      fread_number( fp ) );
	    KEY( "ExtraFlags4", obj->extra_flags4,      fread_number( fp ) );
	    if ( !str_cmp( word, "ExtraDescr" ) )
	    {
		EXTRA_DESCR_DATA *ed;

		    ed			= new_extra_descr();

		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		ed->next		= obj->extra_descr;
		obj->extra_descr	= ed;
		fMatch = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !fNest || !fVnum )
		{
		    bug( "Fread_obj: incomplete object.", 0 );
		    free_obj( obj);
		    return;
		}
		else
		{
		    obj->next	= object_list;
		    object_list	= obj;
		    obj->pIndexData->count++;
		    if ( iNest == 0 || !rgObjNest[iNest] )
		    {
		        if ( storage && !IS_NPC( ch ) )
			  obj_to_storage( obj, ch );
			else
			  obj_to_char( obj, ch );
		    }
		    else
                    {
			obj_to_obj( obj, rgObjNest[iNest-1] );
                        if (storage) { ch->pcdata->storcount++; } /* Keep accurate count of storage - Ahsile */
                    }
		    if ( obj->item_type == ITEM_POTION )
		      SET_BIT( obj->extra_flags, ITEM_NO_LOCATE );
/*
		    if ( IS_SET(obj->extra_flags2, ITEM_DISPEL ) )
		      REMOVE_BIT(obj->extra_flags2, ITEM_DISPEL );
*/
		    return;
		}
	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );
	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		}
		else
		{
		    rgObjNest[iNest] = obj;
		    fNest = TRUE;
		}
		fMatch = TRUE;
	    }
	    break;

	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 3 )
		{
		    bug( "Fread_obj: bad iValue %d.", iValue );
		}
		else if ( sn < 0 )
		{
		    bug( "Fread_obj: unknown skill.", 0 );
		}
		else
		{
		    obj->value[iValue] = sn;
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) )
	    {
		obj->value[0]	= fread_number( fp );
		obj->value[1]	= fread_number( fp );
		obj->value[2]	= fread_number( fp );
		obj->value[3]	= fread_number( fp );
		fMatch		= TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )	/* OLC */
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( !( obj->pIndexData = get_obj_index( vnum ) ) )
			obj->pIndexData = get_obj_index( OBJ_VNUM_DUMMY );

		fVnum = TRUE;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    bug( obj->name, 0 );
	    bug( "Fread_obj: no match.", 0 );
	    fread_to_eol( fp );
	}
    }
}

void save_pet( CHAR_DATA *ch, FILE *fp, CHAR_DATA *pet )
{
#ifndef SQL_SYSTEM
  fprintf( fp, "#PET\n" );
  fprintf( fp, "%d %d %d %d %ld %ld %ld %ld\n", pet->pIndexData->vnum, pet->hit, pet->max_hit,
     pet->act, pet->affected_by, pet->affected_by2, pet->affected_by3, pet->affected_by4 );
  fprintf( fp, "\n" );
#endif
  return;
}

void fread_pet( CHAR_DATA *ch, FILE *fp )
{
  MOB_INDEX_DATA *pMob;
  CHAR_DATA *pet;
  int vnum;

  vnum = fread_number( fp );
  if ( ( pMob = get_mob_index( vnum ) ) == NULL ||
       ( pet = create_mobile( pMob ) ) == NULL )
  {
      for ( vnum = 0; vnum < 5; vnum++ )
	fread_number( fp );
      return;
  }
  char_to_room( pet, ch->in_room );
  pet->master = ch;
  pet->hit = fread_number( fp );
  pet->max_hit = fread_number( fp );
  pet->act = fread_number( fp );
  pet->affected_by = fread_number( fp ) | AFF_CHARM;
  pet->affected_by2 = fread_number( fp );
  pet->affected_by3 = fread_number( fp );
  pet->affected_by4 = fread_number( fp );
  return;
}

void fread_alias( CHAR_DATA *ch, FILE *fp )
{
  ALIAS_DATA *iAl;

  iAl = ch->pcdata->alias_list;

  for ( ; ; )
  {
    char *word = NULL;
    char *word1 = NULL;
    char letter;

    letter = fread_letter( fp );
    bug("letter: %c", letter );

    switch( letter )
    {
      case '~' :
	fread_to_eol( fp );
	return;
      default:
	ungetc( letter, fp );
	word = fread_string( fp );
	word1 = fread_string( fp );
	add_alias( ch, iAl, word, word1 );
	fread_to_eol( fp );
	sprintf( log_buf, "%s %s", word, word1 );
	bug( log_buf, 0 );
	break;
    }
  }
  return;
}

void fwrite_alias( CHAR_DATA *ch, FILE *fp )
{
  ALIAS_DATA *pAl;

  fprintf(fp, "#ALIAS\n");
  for ( pAl = ch->pcdata->alias_list; pAl; pAl = pAl->next )
  {
    fprintf( fp, "%s~ %s~\n", pAl->old, pAl->_new );
    bug( "Writing alias...", 0 );
  }
  fprintf( fp, "~\n\n" );
  return;
}

/*
 * Assumes ch->pcdata->corpses & that it is initialized to 0.
 * This routine WILL NOT work as written without it.
 * checks to make sure corpse is not empty before
 * reading/writing the corpse file.
 */
void corpse_back( CHAR_DATA *ch, OBJ_DATA *corpse )
{
    FILE      *fp;
    OBJ_DATA  *obj, *obj_next;
    OBJ_DATA  *obj_nest,  *objn_next;
    char       strsave[MAX_INPUT_LENGTH ];
    char       buf    [MAX_STRING_LENGTH];
    int        corpse_cont[1024];
    int        item_level [1024];
    int        c =1;
    int        checksum1 =0;
    int        checksum2 =0;

    /* Don't do anything if the corpse is empty */
    if (!corpse->contains)
        return;

    if ( IS_NPC( ch ) )
        return;

    /* Ok, it isn't empty determine the # of items in the corpse.
     * Store the items in the LAST number of the array to  write
     * it backwards.  Easiest way to do it.
     */
    for ( obj = corpse->contains; obj; obj = obj_next )
    {
        obj_next = obj->next_content;
        if ( obj->item_type == ITEM_POTION )
                continue;
        corpse_cont[c] = obj->pIndexData->vnum;
        item_level[c] = obj->level;
        checksum1 += corpse_cont[c];
        checksum2 +=  item_level[c];
        ++c;
        if ( obj->contains) /* get stuff in containers */
        {
           for ( obj_nest = obj->contains; obj_nest; obj_nest = objn_next )
           {
              objn_next = obj_nest->next_content;
              if ( obj_nest->item_type == ITEM_POTION )
                  continue;
              corpse_cont[c] = obj_nest->pIndexData->vnum;
              item_level[c] = obj_nest->level;
              checksum1 += corpse_cont[c];
              checksum2 +=  item_level[c];
              ++c;
           }
        }
    }
    /* Check the corpse for only one item. Assumes if true the the player
     * died trying to retrieve their corpse. Change it if you like.
     */
    if (c <= 2 )
         return;

    /* Add in the number of items and checksum for validation check */
    corpse_cont[0] = c -1;
    item_level[0] = c -1;
    corpse_cont[c+1] = checksum1;
    item_level[c+1] = checksum2;

    /* Ok now we have a corpse to save */

    fclose( fpReserve );

#if !defined( macintosh ) && !defined( MSDOS )
    sprintf( strsave, "%s%s%s%s.cps", PLAYER_DIR,
	    initial( ch->name ), "/", capitalize( ch->name ) );
#else
    sprintf( strsave, "%s%s.cps", PLAYER_DIR, capitalize( ch->name ) );
#endif

    if ( !( fp = fopen( strsave, "w" ) ) )
    {
        sprintf( buf, "Corpse back: fopen %s: ", ch->name );
        bug( buf, 0 );
        perror( strsave );
    }
    else
    {
        int i;
        for ( i = 0 ; i < c ; i++ )
        {
           fprintf( fp, "%d ", corpse_cont[i] );
           fprintf( fp, "%d ",  item_level[i] );
        }
        fprintf( fp, "%d ",  corpse_cont[i+1] );
        fprintf( fp, "%d\n",  item_level[i+1] );
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    ch->pcdata->corpses = 1;
    return;

    fclose( fpReserve );

    /* Okay, it isn't the first corpse, read the rest */
   /* NOTICE THIS NEXT PART IS COMMENTED OUT */
/*
    if ( !( fp = fopen( strsave, "r" ) ) )
    {
        sprintf( buf, "Corpse back: fopen %s: ", ch->name );
        bug( buf, 0 );
        perror( strsave );
    }
    else
    {
        for ( i=4 ; i > 0 ; i-- )
        {
             corpse_cont[i][0] = fread_number( fp );
             item_level[i][0]  = fread_number( fp );

            if ( corpse_cont[i][0] == 99 )
                break;

            for ( c = 1 ; c < corpse_cont[i][0] +2 ; c++ )
            {
                corpse_cont[i][c] = fread_number ( fp );
                item_level[i][c]  = fread_number ( fp );
            }
        }

    }
    fclose( fp );


    if ( !( fp = fopen( strsave, "w" ) ) )
    {
        sprintf( buf, "Corpse back: fopen %s: ", ch->name );
        bug( buf, 0 );
        perror( strsave );
    }
    else
    {
        for ( i=5 ; i > 0 ; i-- )
        {
            if ( corpse_cont[i][0] == 99 )
                break;

            fprintf( fp, "%d ", corpse_cont[i][0] );
            fprintf( fp, "%d ",  item_level[i][0] );
            checksum1 = 0;
            checksum2 = 0;

            for ( c = 1 ; c < corpse_cont[i][0] +1 ; c++ )
            {
                fprintf( fp, "%d " , corpse_cont[i][c]  );
                fprintf( fp, "%d " ,  item_level[i][c]  );
                checksum1 += corpse_cont[i][c];
                checksum2 +=  item_level[i][c];
            }
            fprintf( fp, "%d ", checksum1  );
            fprintf( fp, "%d\n", checksum2 );
        }
        fprintf( fp, "99 99" );

    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
*/
}

/*
 *  Replace missing skills with new ones.
 *
 *  If you remove a skill from the game, add it here
 *  so that we dont get tons of log messages. Even if you
 *  don't replace the skill with anything, just return TRUE
 *		- Ahsile
 */
bool replace_missing_skills( CHAR_DATA* ch, char* missing, int value )
{

	if ( !str_cmp(missing, "mana shield ") )
	{
		ch->pcdata->learned[skill_lookup("mana shield")] = value;
		return TRUE;
	}
	else if (!str_cmp( missing, "turn evil") )
	{
		return TRUE;
	}
	else if (!str_cmp( missing, "dispel evil") )
	{
		return TRUE;
	}
	else if (!str_cmp( missing, "scribe") )
	{
		ch->pcdata->learned[skill_lookup("inscription")] = value;
		return TRUE;
	}

	/*
	else if (!str_cmp( missing, "some skill") )
	{
		return TRUE; // Not replacing skill, but return TRUE to avoid log messages
	}

    */

	return FALSE;
}
