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

/*$Id: act_info.c,v 1.55 2005/04/10 19:57:31 tyrion Exp $*/
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

char *	const	where_name	[] =
{
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on body>      ",
    "<worn on head>      ",
    "<worn in eyes>      ",
    "<worn on face>      ",
    "<orbiting>          ",
    "<orbiting>          ",
    "<worn on legs>      ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<wielded>           ",
    "<dual wielded>      ",
    "<held>              ",
    "<holstered>         ",
    "<worn on ears>      ",
    "<worn around ankle> ",
    "<worn around ankle> ",
    "<implanted>         ",
    "<implanted>         ",
    "<implanted>         "
};


/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				       bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				       bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
void    do_scry_exits           args( ( CHAR_DATA *ch, ROOM_INDEX_DATA  *scryer ) );



char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf [ MAX_STRING_LENGTH ];

    buf[0] = '\0';
    if ( IS_OBJ_STAT( obj, ITEM_INVIS)     )   strcat( buf, "(Invis) "     );
    if ( ( IS_AFFECTED( ch, AFF_DETECT_EVIL  )
          || ( IS_SET( race_table[ ch->race ].race_abilities,
                      RACE_DETECT_ALIGN )
              && IS_GOOD( ch ) ) )
        && IS_OBJ_STAT( obj, ITEM_EVIL )   )   strcat( buf, "&r(Red Aura)&w ");
    if ( IS_AFFECTED( ch, AFF_DETECT_MAGIC )
	&& IS_OBJ_STAT( obj, ITEM_MAGIC )  )   strcat( buf, "&Y(Magical)&w " );
    if ( IS_OBJ_STAT( obj, ITEM_GLOW )     )   strcat( buf, "&W(Glowing)&B " );
    if ( IS_OBJ_STAT( obj, ITEM_HUM )      )   strcat( buf, "(Humming) " );
    if ( IS_OBJ_STAT( obj, ITEM_POISONED ) )   strcat( buf, "&G(Poisoned)&w " );
    if ( IS_OBJ_STAT( obj, ITEM_FLAME )    )   strcat( buf, "&r(Burning)&w " );
    if ( IS_OBJ_STAT( obj, ITEM_CHAOS )    )   strcat( buf, "&Y(Chaotic)&w " );
    if ( IS_OBJ_STAT( obj, ITEM_ICY   )    )   strcat( buf, "&B(Frosty)&w " );
 /*   if ( IS_OBJ_STAT( obj, ITEM_ACID )    )   strcat( buf, "&G(Acidic)&w " );  */
    if ( IS_OBJ_STAT2( obj, ITEM_HIDDEN) )     strcat( buf, "&w(Hidden)&w " );
    if ( IS_OBJ_STAT2( obj, ITEM_SPARKING)   )   strcat( buf, "&b(Sparking)&w " ); 
    if ( IS_OBJ_STAT2( obj, ITEM_DISPEL)   )   strcat( buf, "&b(Dispelling)&w " ); 
    if ( fShort )
    {
	if ( obj->short_descr )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description )
	    strcat( buf, obj->description );
    }

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    OBJ_DATA  *obj;
    char       buf [ MAX_STRING_LENGTH ];
    char     **prgpstrShow;
    char      *pstrShow;
    int       *prgnShow;
    int       *prgnType;
    int        nShow;
    int        iShow;
    int        count;
    bool       fCombine;

    if ( !ch->desc )
	return;

    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj; obj = obj->next_content )
    {
	if ( obj->deleted )
	    continue;
	count++;
    }

    prgpstrShow	= alloc_mem( count * sizeof( char * ) );
    prgnShow    = alloc_mem( count * sizeof( int )    );
    prgnType    = alloc_mem( count * sizeof( int )    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_NPC( ch ) || IS_SET( ch->act, PLR_COMBINE ) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnType    [nShow] = obj->item_type;
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if ( IS_NPC( ch ) || IS_SET( ch->act, PLR_COMBINE ) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		switch( prgnType[iShow] )
		{
		default:
		     sprintf( buf, "(%2d) ", prgnShow[iShow] );
		     send_to_char(AT_GREEN, buf, ch );
		     break;
		case ITEM_LIGHT:
		     sprintf( buf, "(%2d) ", prgnShow[iShow] );
		     send_to_char(AT_WHITE, buf, ch );
		     break;
		case ITEM_FOOD:
		case ITEM_BERRY:
		case ITEM_PILL:
		     sprintf( buf, "(%2d) ", prgnShow[iShow] );
		     send_to_char(AT_ORANGE, buf, ch );
		     break;
		case ITEM_FOUNTAIN:
		case ITEM_DRINK_CON:
		case ITEM_POTION:
		     sprintf( buf, "(%2d) ", prgnShow[iShow] );
		     send_to_char(AT_BLUE, buf, ch );
		     break;
		case ITEM_MONEY:
		     sprintf( buf, "(%2d) ", prgnShow[iShow] );
		     send_to_char(AT_YELLOW, buf, ch );
		     break;
		case ITEM_BLOOD:     
		     sprintf( buf, "(%2d) ", prgnShow[iShow] );
		     send_to_char(AT_RED, buf, ch );
		     break;
		}   
	    }
	    else
	    {
		send_to_char(C_DEFAULT, "     ", ch );
	    }
	}
		switch( prgnType[iShow] )
                {
		default:
	             send_to_char(AT_GREEN, prgpstrShow[iShow], ch );
	             send_to_char(C_DEFAULT, "\n\r", ch );
		     break;
		case ITEM_LIGHT:
	             send_to_char(AT_WHITE, prgpstrShow[iShow], ch );
	             send_to_char(C_DEFAULT, "\n\r", ch );
		     break;
		case ITEM_FOOD:
		case ITEM_PILL:
	             send_to_char(AT_ORANGE, prgpstrShow[iShow], ch );
	             send_to_char(C_DEFAULT, "\n\r", ch );
		     break;
		case ITEM_FOUNTAIN:
		case ITEM_DRINK_CON:
		case ITEM_POTION:
	             send_to_char(AT_BLUE, prgpstrShow[iShow], ch );
	             send_to_char(C_DEFAULT, "\n\r", ch );
		     break;
		case ITEM_MONEY:
	             send_to_char(AT_YELLOW, prgpstrShow[iShow], ch );
	             send_to_char(C_DEFAULT, "\n\r", ch );
		     break;
		case ITEM_BLOOD:     
	             send_to_char(AT_RED, prgpstrShow[iShow], ch );
	             send_to_char(C_DEFAULT, "\n\r", ch );
		     break;
		}   
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC( ch ) || IS_SET( ch->act, PLR_COMBINE ) )
	    send_to_char(C_DEFAULT, "     ", ch );
	send_to_char(AT_DGREEN, "Nothing.\n\r", ch );
    }

    /*
     * Clean up.
     */
    free_mem( prgpstrShow, count * sizeof( char * ) );
    free_mem( prgnShow,    count * sizeof( int )    );
    free_mem( prgnType,    count * sizeof( int )    );
    return;
}



void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf [ MAX_STRING_LENGTH ];
    char buf2 [MAX_STRING_LENGTH ];

    buf[0] = '\0';
    buf2[0] = '\0';
    if (!victim->desc && !IS_NPC(victim))
        strcat( buf, "(Link-dead) " );
    if ( !IS_NPC( victim ) )

    if ( IS_NPC(victim) && ch->questmob > 0 && victim->pIndexData->vnum == ch->questmob )
                                                strcat( buf, "[TARGET] "     );
    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_WIZINVIS )
        && (get_trust( victim ) < get_trust( ch ) ) )
                                                strcat( buf, "(Wizinvis) "   );

    if ( victim->desc && victim->desc->editor != 0 && get_trust(ch) > 100 )
      strcat( buf, "&R<&BEDITING&R>&P " );
    if ( IS_AFFECTED( victim, AFF_INVISIBLE )   )
                                                strcat( buf, "(Invis) "      );
    if ( IS_AFFECTED2( victim, AFF_IMPROVED_INVIS )   )
                                                strcat( buf, "(Improved Invis) "      );
    if ( IS_AFFECTED( victim, AFF_HIDE )        )
                                                strcat( buf, "(Hide) "       );
    if ( IS_AFFECTED( victim, AFF_CHARM ) && ( !IS_SET( victim->act, PLR_UNDEAD ) ) )
                                                strcat( buf, "(Charmed) "    );
    if ( IS_AFFECTED( victim, AFF_PEACE )       )
                                                strcat( buf, "(Peaceful) "    );
    if ( IS_AFFECTED2( victim, AFF_SHADOW_PLANE )       )
                                                strcat( buf, "(Shadow Plane) "    );
    if ( IS_AFFECTED4( victim, AFF_BURROW )       )
                                                strcat( buf, "(Burrowed) "    );
    if ( IS_AFFECTED( victim, AFF_PASS_DOOR )
        || ( IS_SET( race_table[ victim->race ].race_abilities, RACE_PASSDOOR )
            && ( !str_cmp( race_table[ victim->race ].race_full, "undead" )
                || !str_cmp( race_table[ victim->race ].race_full, "vampire" ) ) ) )
                                                strcat( buf, "(Translucent) ");
    if ( IS_AFFECTED( victim, AFF_FAERIE_FIRE ) )
                                                strcat( buf, "(Pink Aura) "  );
    if ( IS_EVIL( victim )
        && ( IS_AFFECTED( ch, AFF_DETECT_EVIL )
            || ( IS_SET( race_table[ ch->race ].race_abilities,
                        RACE_DETECT_ALIGN )
                && IS_GOOD( ch ) ) ) )
                                                strcat( buf, "(Red Aura) "   );
    if ( IS_GOOD( victim ) && IS_AFFECTED2( ch, AFF_DETECT_GOOD ) )
                                                strcat( buf, "(Blue Aura) "  );
    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_KILLER )  )
						strcat( buf, "(KILLER) "     );
    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_THIEF  )  )
						strcat( buf, "(THIEF) "      );
    if ( IS_NPC(victim ) && IS_SET( victim->act, PLR_UNDEAD ) )
                                                strcat( buf, "(Undead) "     );
    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_AFK )  )
                                                strcat( buf, "(AFK) "        );
    if ( !IS_NPC( victim ) && IS_SET( victim->act2, PLR_RELQUEST )  )
                                                strcat( buf, "(CRUSADE)"        );

    if ( victim->position == POS_GHOST )
        strcat( buf, "&CThe Ghost of " );

if ( victim->position == POS_STANDING && victim->long_descr[0] != '\0' )
    {
	strcat( buf, victim->long_descr );
	if ( !IS_NPC( victim ) )
	  if ( victim->pcdata->lname )
	    {
	      strcat( buf, " " );
	      strcat( buf, victim->pcdata->lname );
	    }
	send_to_char(AT_PINK, buf, ch );
    }
    else
    {
      strcat( buf, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
        {
          strcat( buf, " " );
          strcat( buf, victim->pcdata->lname );
        }
      if ( !IS_NPC( victim ) && !IS_SET( ch->act, PLR_BRIEF ) )
	strcat( buf, victim->pcdata->title );

      switch ( victim->position )
      {
	case POS_DEAD:     strcat( buf, " is DEAD!!"              ); break;
	case POS_MORTAL:   strcat( buf, " is mortally wounded."   ); break;
	case POS_INCAP:    strcat( buf, " is incapacitated."      ); break;
	case POS_STUNNED:  strcat( buf, " is lying here stunned." ); break;
	case POS_SLEEPING: strcat( buf, " is sleeping here."      ); break;
	case POS_RESTING:  strcat( buf, " is resting here."       ); break;
	case POS_STANDING: strcat( buf, " is here."               ); break;
	case POS_GHOST:    strcat( buf, " is floating here."	  ); break;
	case POS_FIGHTING:
	  strcat( buf, " is here, fighting " );
	  if ( !victim->fighting )
	    strcat( buf, "thin air??" );
	  else if ( victim->fighting == ch )
	    strcat( buf, "YOU!" );
	  else if ( victim->in_room == victim->fighting->in_room )
	  {
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, "." );
	  }
	  else
	    strcat( buf, "someone who left??" );
	  break;
      }

      strcat( buf, "\n\r" );
      buf[0] = UPPER( buf[0] );
      send_to_char(AT_PINK, buf, ch );
    }
    if ( victim->mounted > 0 )
    {
     strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )  
        if ( victim->pcdata->lname )
          {   
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s is mounted on " );
      strcat( buf2, victim->mountshort );
      strcat( buf2, ".\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_GREEN, buf2, ch );
      buf2[0] = '\0'; 
    }

    if ( IS_AFFECTED( victim, AFF_SANCTUARY )
        || IS_SET( race_table[ victim->race ].race_abilities, RACE_SANCT ) )
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body is veiled in a glowing white mist.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_WHITE, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED3( victim, AFF_DEMONSHIELD ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body is surrounded by Demons.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_PURPLE, buf2, ch );
      buf2[0] = '\0';
	}
     if ( IS_AFFECTED3( victim, AFF_ACIDSHIELD ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body sizzles and bubbles with acid.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_GREEN, buf2, ch );
      buf2[0] = '\0';
     } 
    if ( IS_AFFECTED2( victim, AFF_GOLDEN_ARMOR ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body is surrounded by a golden aura.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_YELLOW, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED4( victim, AFF_GOLDEN_SANCTUARY ) )
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
	if ( victim->pcdata->lname )
	  {
	    strcat( buf2, " " );
	    strcat( buf2, victim->pcdata->lname );
	  }
      strcat( buf2, "'s body is surrounded by a golden sanctuary.\n\r" );
      buf2[4] = UPPER( buf2[4]);
      send_to_char( AT_YELLOW, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED2( victim, AFF_GHOST_SHIELD ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body is surrounded by swirling ghosts.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_GREY, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED3( victim, AFF_SATANIC_INFERNO ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
       strcat( buf2, "'s body is ingulfed by satanic fires.\n\r");
       buf2[4] = UPPER( buf2[4] );
       send_to_char ( AT_RED, buf2, ch );
       buf2[0] = '\0';    
    }
    if ( IS_AFFECTED2( victim, AFF_MIST ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body is shrouded in a heavy mist.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_GREY, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED3( victim, AFF_MANA_SHIELD ) )
    {
      strcat( buf2, "    ");
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
	if ( victim->pcdata->lname )
	{
	  strcat( buf2, " " );
	  strcat( buf2, victim->pcdata->lname );
	}
      strcat( buf2, "'s body has a pearly white ball floating over their head.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char( AT_WHITE, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED( victim, AFF_FIRESHIELD ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname)
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body is burning with unfelt heat.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_RED, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED3( victim, AFF_COFIRE ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname)
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, " is surrounded by a ring of fire.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_RED, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED3(victim, AFF_AURA_ANTI_MAGIC ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
	if ( victim->pcdata->lname )
          {
	    strcat( buf2, " " );
	    strcat( buf2, victim->pcdata->lname );
	  }
      strcat( buf2, "'s body is surrounded by an anti-magic aura.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char (AT_PINK, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED( victim, AFF_SHOCKSHIELD ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body is sparking with electricity.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_BLUE, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED( victim, AFF_ICESHIELD ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body is covered in frost and ice.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_LBLUE, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED( victim, AFF_CHAOS ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body shimmers randomly with raw chaos.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_YELLOW, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED( victim, AFF_INERTIAL ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body is vibrating rapidly.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( C_DEFAULT, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED2( victim, AFF_DOOMSHIELD ) )
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, " is surrounded by an aura of insanity.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char( AT_PURPLE, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED3( victim, AFF_BLOODSHIELD ) )
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
	if ( victim->pcdata->lname )
	  {
	    strcat( buf2, " " );
	    strcat( buf2, victim->pcdata->lname );
	  }
      strcat( buf2, " is surrounded by blood.\n\r" );
      buf2[4] = UPPER(buf2[4]);
      send_to_char(AT_BLOOD, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED2( victim, AFF_IMAGE ) )
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
	if ( victim->pcdata->lname )
	  {
	    strcat( buf2, " " );
	    strcat( buf2, victim->pcdata->lname );
	  }
      strcat( buf2, " is surrounded by images of the Talisman.\n\r" );
      buf2[4] = UPPER(buf2[4]);
      send_to_char(AT_CYAN, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED2( victim, AFF_BLADE ) )
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, " is surrounded by thousands of spinning blades.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char( AT_GREY, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED2( victim, AFF_SHADOW_IMAGE ) )
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, " appears to be in more than one place.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_GREY, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED3( victim, AFF_RANDOMSHIELD ) )
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, " is surrounded by random illusions.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_GREY, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED2( victim, AFF_DANCING ) )
    {
      strcat( buf2, "&w    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, "&w " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, " &.is s&.urrounde&.d by &.thou&.sand&.s of &.danci&.ng l&.igh&.ts&w.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char( AT_GREY, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED4( victim, AFF_BIOFEEDBACK ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body is surrounded by an electromagnetic aura.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_BLUE, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED4( victim, AFF_EARTHSHIELD ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body is surrounded by an earthen shield.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_ORANGE, buf2, ch );
      buf2[0] = '\0';
    }   
    if ( IS_AFFECTED4( victim, AFF_LEAF_SHIELD ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body is surrounded by swirling leaves.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_DGREEN, buf2, ch );
      buf2[0] = '\0';
    }   
    if ( IS_AFFECTED4( victim, AFF_LUCK_SHIELD ))
    { 
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s soul is infused with the luck of the gods.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_DGREEN, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED4( victim, AFF_LIQUID_SKIN ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s skin appears liquid.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_DBLUE, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED4( victim, AFF_FORCE_OF_NATURE ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s soul has embraced a force of nature.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_DGREEN, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED4( victim, AFF_ESSENCE_OF_GAIA ) )
    {
	strcat( buf2, "    ");
	strcat( buf2, PERS( victim, ch ) );
	if( !IS_NPC( victim ) )
	  if ( victim->pcdata->lname )
	    {
	      strcat( buf2, " " );
	      strcat( buf2, victim->pcdata->lname );
	    }
	strcat( buf2, "'s body is pulsing with the power of Gaia.\n\r" );
	buf2[4] = UPPER( buf2[4] );
	send_to_char( AT_GREEN, buf2, ch );
	buf2[0] = '\0';
    }
    if ( IS_AFFECTED2( victim, AFF_UNHOLY_STRENGTH ) )
    {
	strcat( buf2, "    ");
	strcat( buf2, PERS( victim, ch ) );
	if( !IS_NPC( victim ) )
	  if ( victim->pcdata->lname )
	    {
	      strcat( buf2, " " );
	      strcat( buf2, victim->pcdata->lname );
	    }
	strcat( buf2, "'s body is fused with the strength of the unholy.\n\r" );
	buf2[4] = UPPER( buf2[4] );
	send_to_char( AT_DGREY, buf2, ch );
	buf2[0] = '\0';
    }
    if ( IS_AFFECTED4( victim, AFF_ANGELIC_AURA ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname )
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, "'s body has sprouted a pair of ethereal wings.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_YELLOW, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED4( victim, AFF_ETHEREAL_WOLF ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname)
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, " is being guarded by an ethereal wolf.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_DGREY, buf2, ch );
      buf2[0] = '\0';
    }
    if ( IS_AFFECTED4( victim, AFF_ETHEREAL_SNAKE ))
    {
      strcat( buf2, "    " );
      strcat( buf2, PERS( victim, ch ) );
      if ( !IS_NPC( victim ) )
        if ( victim->pcdata->lname)
          {
            strcat( buf2, " " );
            strcat( buf2, victim->pcdata->lname );
          }
      strcat( buf2, " is being guarded by an ethereal snake.\n\r" );
      buf2[4] = UPPER( buf2[4] );
      send_to_char ( AT_DGREY, buf2, ch );
      buf2[0] = '\0';
    }
    return;
}

void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    char      buf [ MAX_STRING_LENGTH ];
    int       iWear;
    int       percent;
    bool      found;

    if ( can_see( victim, ch ) )
    {
	act(AT_GREY, "$n looks at you.", ch, NULL, victim, TO_VICT    );
	act(AT_GREY, "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
    }

    if ( victim->description[0] != '\0' )
    {
	send_to_char(AT_GREEN, victim->description, ch );
    }
    else
    {
	act(AT_GREY, "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS( victim, ch ) );

         if ( percent >= 100 ) strcat( buf, " is in perfect health.\n\r"  );
    else if ( percent >=  90 ) strcat( buf, " is slightly scratched.\n\r" );
    else if ( percent >=  80 ) strcat( buf, " has a few bruises.\n\r"     );
    else if ( percent >=  70 ) strcat( buf, " has some cuts.\n\r"         );
    else if ( percent >=  60 ) strcat( buf, " has several wounds.\n\r"    );
    else if ( percent >=  50 ) strcat( buf, " has many nasty wounds.\n\r" );
    else if ( percent >=  40 ) strcat( buf, " is bleeding freely.\n\r"    );
    else if ( percent >=  30 ) strcat( buf, " is covered in blood.\n\r"   );
    else if ( percent >=  20 ) strcat( buf, " is leaking guts.\n\r"       );
    else if ( percent >=  10 ) strcat( buf, " is almost dead.\n\r"        );
    else                       strcat( buf, " is DYING.\n\r"              );

    buf[0] = UPPER( buf[0] );

         if ( percent >= 100 ) send_to_char( AT_WHITE, buf, ch );
    else if ( percent >=  90 ) send_to_char( AT_WHITE, buf, ch );
    else if ( percent >=  80 ) send_to_char( AT_WHITE, buf, ch );
    else if ( percent >=  70 ) send_to_char( AT_BLUE,  buf, ch );
    else if ( percent >=  60 ) send_to_char( AT_BLUE,  buf, ch );
    else if ( percent >=  50 ) send_to_char( AT_BLUE,  buf, ch );
    else if ( percent >=  40 ) send_to_char( AT_RED,   buf, ch );
    else if ( percent >=  30 ) send_to_char( AT_RED,   buf, ch );
    else if ( percent >=  20 ) send_to_char( AT_RED,   buf, ch );
    else if ( percent >=  10 ) send_to_char( AT_BLOOD, buf, ch );
    else                       send_to_char( (AT_BLOOD+AT_BLINK),  buf, ch );

    found = FALSE;
    if ((IS_AFFECTED2( victim, AFF_CLOAKING)) && (ch->level < 105))
    {
    send_to_char(AT_RED, "\n\rThe equipment is cloaked, you can not see it.\n\r", ch );
    return;
    }
    else
    {
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) )
	    && can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char(AT_GREY, "\n\r", ch );
		act(AT_WHITE, "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char(AT_BLUE, where_name[iWear], ch );
	    send_to_char(AT_CYAN, format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char(AT_CYAN, "\n\r", ch );
	} else if ( iWear == 20 ) {
	    if ( !found )
            {
                send_to_char(AT_GREY, "\n\r", ch );
                act(AT_WHITE, "$N is using:", ch, NULL, victim, TO_CHAR );
                found = TRUE;
            }
	    send_to_char(AT_BLUE, where_name[iWear], ch );
	    send_to_char(C_DEFAULT, race_table[victim->race].default_weap, ch );
	    send_to_char(C_DEFAULT, "\n\r", ch );
        }
    }
    }

    if ( victim != ch
	&& !IS_NPC( ch ) 
	&& ( number_percent( ) < (ch->pcdata->learned[skill_lookup("peek")]/10)  || ( ch->race == 4 ) ) )
    {
	send_to_char(AT_WHITE, "\n\rYou peek at the inventory:\n\r", ch );
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
	update_skpell( ch, skill_lookup("peek"), 0 );
    }

    return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch; rch = rch->next_in_room )
    {
        if ( rch->deleted || rch == ch )
	    continue;
	
	if ( !(rch->desc) 
         && !IS_NPC(rch)
         && get_trust( ch ) < L_APP )
	    continue;

	if ( !IS_NPC( rch )
	    && IS_SET( rch->act, PLR_WIZINVIS )
	    && get_trust( ch ) < rch->wizinvis )
	    continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
        else if ( room_is_dark( ch->in_room )
                 && ( IS_AFFECTED( rch, AFF_INFRARED )
                     || IS_SET( race_table[ rch->race ].race_abilities,
                               RACE_INFRAVISION ) ) )
	{
	    send_to_char(AT_RED, "You see glowing red eyes watching YOU!\n\r", ch );
	}
    }

    return;
} 



bool check_blind( CHAR_DATA *ch )
{
    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
	return TRUE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) ||IS_AFFECTED3(ch, AFF_BEND_LIGHT ))
    {
	send_to_char(AT_WHITE, "You can't see a thing!\n\r", ch );
	return FALSE;
    }

    return TRUE;
}



void do_look( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    char       buf  [ MAX_STRING_LENGTH ];
    char       sound  [ MAX_STRING_LENGTH ];
    char       music [ MAX_STRING_LENGTH ];
    char       arg1 [ MAX_INPUT_LENGTH  ];
    char       arg2 [ MAX_INPUT_LENGTH  ];
    char      *pdesc;
    ROOM_INDEX_DATA *portroom;
    int        door;
    extern OBJ_DATA *auc_obj;
    extern int auc_cost;
    int        namecolor;
    int        desccolor;

    if ( !IS_NPC( ch ) && !ch->desc )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char(AT_CYAN, "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char(AT_CYAN, "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC( ch )
	&& !IS_SET( ch->act, PLR_HOLYLIGHT )
	&& room_is_dark( ch->in_room )
        && ( ch->race != 1 )
        && ( ch->race != 2 )
        && ( ch->race != 5 )
        && ( ch->race != 9 )
	&& ( ch->position != POS_GHOST ) )

    {
	send_to_char(AT_DGREY, "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->people, ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if( ch->position != POS_GHOST )
    {
        namecolor = AT_RED;
        desccolor = AT_GREY;
    }
    else
    {
        namecolor = AT_WHITE;
        desccolor = AT_LBLUE;
    }

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	/* 'look' or 'look auto' */
	send_to_char(namecolor, ch->in_room->name, ch );
	if(!IS_NPC( ch ) && IS_SET( ch->act, PLR_SOUND ) )
	{
	if( strlen( ch->in_room->soundfile ) >= 5 )
	{
	    send_to_char(namecolor, "  ", ch );
	    sprintf( sound, "!!SOUND(%s)", ch->in_room->soundfile );
	    send_to_char(namecolor, sound, ch );
	}
	}
   if(!IS_NPC( ch ) && IS_SET( ch->act, PLR_MUSIC ) )
   {
   send_to_char(namecolor, "  ", ch );
   if( ch->in_room->area->musicfile != NULL && strlen( ch->in_room->musicfile) <= 4 )
   {
       sprintf( music, "!!MUSIC(%s)", ch->in_room->area->musicfile );
       send_to_char(namecolor, music, ch );
   }
   if( strlen( ch->in_room->musicfile ) >= 5 )
   {
       sprintf( music, "  !!MUSIC(%s)", ch->in_room->musicfile );
       send_to_char(namecolor, music, ch );
   }
   }
	send_to_char(namecolor, "\n\r", ch );

	if ( arg1[0] == '\0'
	    || ( !IS_NPC( ch ) && !IS_SET( ch->act, PLR_BRIEF ) ) )
	    send_to_char(desccolor, ch->in_room->description, ch );

	if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOEXIT ) )
	    do_exits( ch, "auto" );

	if ( IS_NPC( ch ) )
	    do_exits( ch, "auto" );

	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
	show_char_to_char( ch->in_room->people,   ch );
	return;
    }

    if ( !str_prefix( arg1, "in" ) )
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char(AT_DGREEN, "Look in what?\n\r", ch );
	    return;
	}

	if ( !( obj = get_obj_here( ch, arg2 ) ) )
	{
	    if ( !str_prefix( arg2, "auction" ) )
	    {
	      int objcount = 1;
	      char buf[MAX_INPUT_LENGTH];

	      if ( !auc_obj )
	      {
		send_to_char(C_DEFAULT, "There is no object being auctioned.\n\r",ch);
		return;
	      }
	      obj_to_char( auc_obj, ch );
	      for ( obj = ch->carrying; obj; obj = obj->next )
	      {
		if ( obj == auc_obj )
		  break;
		objcount++;
	      }
	      sprintf(buf, "in %d.%s", objcount, auc_obj->name );
	      do_look(ch, buf );
	      obj_from_char(auc_obj);
	      return;
	    }
	      
	    send_to_char(AT_DGREEN, "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char(AT_DGREEN, "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char(AT_BLUE, "It is empty.\n\r", ch );
		oprog_look_in_trigger( obj, ch );
		break;
	    }

	    sprintf( buf, "It's %s full of a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about"     : "more than",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char(AT_BLUE, buf, ch );
	    oprog_look_in_trigger( obj, ch );
	    break;

	
	case ITEM_CONTAINER:
	case ITEM_WRECK:
	    if ( IS_SET( obj->value[1], CONT_CLOSED ))
	    {
		send_to_char(AT_GREEN, "It is closed.\n\r", ch );
		break;
	    }
      case ITEM_CORPSE_PC:
	case ITEM_CORPSE_NPC:

	    act(AT_WHITE, "$p contains:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    oprog_look_in_trigger( obj, ch );
	    break;
	case ITEM_PORTAL:
	    if ( !( portroom = get_room_index( obj->value[0] ) ) )
	     { 
	      act(AT_WHITE, "You cannot see anything through $p", ch, obj, NULL, TO_CHAR );
	      break;
	     }
	act(AT_GREEN, "You look into the $p and see...", ch, obj, NULL, TO_CHAR );
	act(AT_GREEN, "$n looks into the $p.", ch, obj, NULL, TO_ROOM);
	send_to_char(AT_RED, portroom->name, ch );
	send_to_char(AT_RED, "\n\r", ch );

	if ( arg1[0] == '\0'
	    || ( !IS_NPC( ch ) && !IS_SET( ch->act, PLR_BRIEF ) ) )
	    send_to_char(AT_GREY, portroom->description, ch );
	do_scry_exits( ch, portroom );
	show_list_to_char( portroom->contents, ch, FALSE, FALSE );
	show_char_to_char( portroom->people,   ch );
	    oprog_look_in_trigger( obj, ch );
	break;
	     
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) )
    {
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if (( can_see_obj( ch, obj ) ) )
	{
	    pdesc = get_extra_descr( arg1, obj->extra_descr );
	    if ( pdesc )
	    {
		send_to_char(AT_BLUE, pdesc, ch );
		oprog_look_trigger( obj, ch );
		return;
	    }

	    pdesc = get_extra_descr( arg1, obj->pIndexData->extra_descr );
	    if ( pdesc )
	    {
		send_to_char(AT_BLUE, pdesc, ch );
		oprog_look_trigger( obj, ch );
		return;
	    }
	}

	if ( is_name( arg1, obj->name ) )
	{
	    send_to_char(AT_GREEN, obj->description, ch );
	    send_to_char(AT_GREEN, "\n\r", ch );
	    oprog_look_trigger( obj, ch );
	    return;
	}
    }

    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg1, obj->extra_descr );
	    if ( pdesc )
	    {
		send_to_char(AT_GREEN, pdesc, ch );
		oprog_look_trigger( obj, ch );
		return;
	    }

	    pdesc = get_extra_descr( arg1, obj->pIndexData->extra_descr );
	    if ( pdesc )
	    {
		send_to_char(AT_GREEN, pdesc, ch );
		oprog_look_trigger( obj, ch );
		return;
	    }
	}

	if ( is_name( arg1, obj->name ) )
	{
	    send_to_char(AT_GREEN, obj->description, ch );
	    send_to_char(AT_GREEN, "\n\r", ch );
	    oprog_look_trigger( obj, ch );
	    return;
	}
    }

    pdesc = get_extra_descr( arg1, ch->in_room->extra_descr );
    if ( pdesc )
    {
	send_to_char(AT_WHITE, pdesc, ch );
	return;
    }

         if ( !str_prefix( arg1, "north" ) ) door = 0;
    else if ( !str_prefix( arg1, "east"  ) ) door = 1;
    else if ( !str_prefix( arg1, "south" ) ) door = 2;
    else if ( !str_prefix( arg1, "west"  ) ) door = 3;
    else if ( !str_prefix( arg1, "up"    ) ) door = 4;
    else if ( !str_prefix( arg1, "down"  ) ) door = 5;
    else
    {
        if ( !str_prefix( arg1, "auction" ) )
	{
	  char buf[MAX_STRING_LENGTH];

	  if ( !auc_obj )
	  {
	    send_to_char( C_DEFAULT, "There is no object being auctioned.\n\r",ch);
	    return;
	  }
	  sprintf( buf, "Object: %s\n\r", auc_obj->short_descr );
	  send_to_char( AT_WHITE, buf, ch );
	  sprintf( buf, "Type: %s   Level: %d\n\r",
		   item_type_name( auc_obj ), auc_obj->level );
	  send_to_char( AT_WHITE, buf, ch );
	  sprintf( buf, "Value: %d   Price: %d\n\r", auc_obj->cost, auc_cost );
	  send_to_char( AT_WHITE, buf, ch );
	  return;
	}
	send_to_char(AT_GREY, "You do not see that here.\n\r", ch );
	return;
    }

    /* 'look direction' */
    if ( !( pexit = ch->in_room->exit[door] ) || !pexit->to_room )
    {
	send_to_char(AT_GREY, "Nothing special there.\n\r", ch );
	return;
    }

    if ( pexit->description && pexit->description[0] != '\0' )
	send_to_char(AT_GREY, pexit->description, ch );
    else
	send_to_char(AT_GREY, "Nothing special there.\n\r", ch );
    if ( ( IS_AFFECTED( ch, AFF_SCRY ) ) )
    {
        ROOM_INDEX_DATA *rid;
	if ( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) 
	   || IS_SET( pexit->to_room->room_flags, ROOM_NOSCRY ) )
	  {
	   send_to_char( AT_BLUE, "You failed.\n\r", ch );
	   return;
	  }
	act( AT_BLUE, "You scry to the $T.", ch, NULL, dir_name[door], TO_CHAR );
	rid = ch->in_room;
	ch->in_room = pexit->to_room;
	do_look( ch, "" );
	ch->in_room = rid;
	eprog_scry_trigger( pexit, ch->in_room, ch );
	return;
    }
    if (   pexit->keyword
	&& pexit->keyword[0] != '\0'
	&& pexit->keyword[0] != ' ' )
    {
      if ( IS_SET( pexit->exit_info, EX_BASHED ) )
	act(AT_GREY, "The $d has been bashed from its hinges.",
	    ch, NULL, pexit->keyword, TO_CHAR );
      else if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
	act(AT_GREY, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
      else if ( IS_SET( pexit->exit_info, EX_ISDOOR ) )
	act(AT_GREY, "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
    }
    eprog_look_trigger( pexit, ch->in_room, ch );
    return;
}



void do_examine( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      buf [ MAX_STRING_LENGTH ];
    char      arg [ MAX_INPUT_LENGTH  ];
    char      msg [ MAX_INPUT_LENGTH  ];
    int brk;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_BLUE, "Examine what?\n\r", ch );
	return;
    }

    do_look( ch, arg );

    if ( ( obj = get_obj_here( ch, arg ) ) )
    {
	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_WEAPON:
	case ITEM_ARMOR:
	    if ( obj->pIndexData->cost > 0 )
	      brk = (obj->cost * 100) / obj->pIndexData->cost;
	    else
	      brk = 101;
	         if ( brk ==  0 ) strcpy( msg, "is utterly destroyed!" );
	    else if ( brk <= 10 ) strcpy( msg, "is almost useless." );
	    else if ( brk <= 20 ) strcpy( msg, "should be replaced soon." );
	    else if ( brk <= 30 ) strcpy( msg, "is in pretty bad shape." );
	    else if ( brk <= 40 ) strcpy( msg, "has seen better days." );
	    else if ( brk <= 50 ) strcpy( msg, "could use some repairs." );
	    else if ( brk <= 60 ) strcpy( msg, "is in average condition." );
	    else if ( brk <= 70 ) strcpy( msg, "has the odd dent." );
	    else if ( brk <= 80 ) strcpy( msg, "needs a bit of polishing." );
	    else if ( brk <= 90 ) strcpy( msg, "looks almost new." );
	    else if ( brk <=100 ) strcpy( msg, "is in perfect condition." );
	    else                  strcpy( msg, "looks almost indestructable!");
	    act(AT_WHITE,"Looking closer, you see that $p $T",ch,obj,msg,
		TO_CHAR);
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	case ITEM_PORTAL:
	    send_to_char(AT_WHITE, "When you look inside, you see:\n\r", ch );
	    sprintf( buf, "in %s", arg );
	    do_look( ch, buf );
	}
    }
    
    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_scry_exits( CHAR_DATA *ch, ROOM_INDEX_DATA  *scryer )
{
           EXIT_DATA       *pexit;
    extern char *    const  dir_name [ ];
           char             buf      [ MAX_STRING_LENGTH ];
           int              door;
           bool             found;
           bool             fAuto;
    
    fAuto = TRUE;
    strcpy( buf, "&R[Exits:&Y" );
    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = scryer->exit[door] )
	    && pexit->to_room
	    && !IS_SET( pexit->exit_info, EX_CLOSED ) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
		strcat( buf, dir_name[door] );
	    }
        }
    }
    if ( !found )
	strcat( buf, "None" );

    if ( fAuto )
	strcat( buf, "&R]\n\r" );

    send_to_char(AT_WHITE, buf, ch );
    return;
}

void do_exits( CHAR_DATA *ch, char *argument )
{
           EXIT_DATA       *pexit;
    extern char *    const  dir_name [ ];
           char             buf      [ MAX_STRING_LENGTH ];
           int              door;
           bool             found;
           bool             fAuto;


    buf[0] = '\0';
    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    if( ch->position == POS_GHOST )
    {
        strcpy( buf, fAuto ? "&W[Exits:&C" : "Obvious exits:\n\r" );
    }
    else
    {
        strcpy( buf, fAuto ? "&R[Exits:&Y" : "Obvious exits:\n\r" );
    }

    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] )
	    && pexit->to_room
	    && !IS_SET( pexit->exit_info, EX_CLOSED )
          && !IS_SET( pexit->exit_info, EX_HIDDEN ) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		strcat( buf, " " );
		strcat( buf, dir_name[door] );
	    }
	    else
	    {
		sprintf( buf + strlen( buf ), "%-5s - %s\n\r",
		    capitalize( dir_name[door] ),
		    room_is_dark( pexit->to_room )
			?  "Too dark to tell"
			: pexit->to_room->name);
	    }
	}
      if ( ( pexit = ch->in_room->exit[door] )
            && pexit->to_room
            && IS_SET( pexit->exit_info, EX_HIDDEN )
            && IS_SET( pexit->exit_info, EX_CLOSED )
            && (IS_SET( race_table[ch->race].race_abilities, RACE_DETECT_HIDDEN_EXIT)
                || IS_SET( ch->act, PLR_HOLYLIGHT) ) )
      {
	    found = TRUE;
	    if ( fAuto )
	    {

            strcat( buf, " &g(&z" );
            strcat( buf, dir_name[door] );
            if( ch->position == POS_GHOST )
            {
                strcat( buf, "&g)&W" );
            }
            else
            {
                strcat( buf, "&g)&Y" );
            }


       }
	    else
	    {
		sprintf( buf + strlen( buf ), "(%-5s) - %s\n\r",
		    capitalize( dir_name[door] ),
		    room_is_dark( pexit->to_room )
			?  "Too dark to tell"
			: pexit->to_room->name);
          }
      }
      if ( ( pexit = ch->in_room->exit[door] )
            && pexit->to_room
            && IS_SET( pexit->exit_info, EX_CLOSED )
            && !IS_SET( pexit->exit_info, EX_HIDDEN ))
      {
	    found = TRUE;
	    if ( fAuto )
	    {
            if( ch->position == POS_GHOST )
            {
                strcat( buf, " &w[&W" );
                strcat( buf, dir_name[door] );
                strcat( buf, "&w]&W" );

            }
            else
            {
                strcat( buf, " &w[&Y" );
                strcat( buf, dir_name[door] );
                strcat( buf, "&w]&Y" );
            }

          }
	    else
	    {
		sprintf( buf + strlen( buf ), "[%-5s] - %s\n\r",
		    capitalize( dir_name[door] ),
		    room_is_dark( pexit->to_room )
			?  "Too dark to tell"
			: pexit->to_room->name);
          }

      }
      if ( ( pexit = ch->in_room->exit[door] )
            && pexit->to_room
            && IS_SET( pexit->exit_info, EX_HIDDEN )
            && !IS_SET( pexit->exit_info, EX_CLOSED )
            && (IS_SET( race_table[ch->race].race_abilities, RACE_DETECT_HIDDEN_EXIT)
                || IS_SET( ch->act, PLR_HOLYLIGHT) ) )
      {
	    found = TRUE;
	    if ( fAuto )
	    {
            if( ch->position == POS_GHOST )
            {
                strcat( buf, " &g(&W" );
                strcat( buf, dir_name[door] );
                strcat( buf, "&g)&W" );

            }
            else
            {
                strcat( buf, " &g(&Y" );
                strcat( buf, dir_name[door] );
                strcat( buf, "&g)&Y" );
            }
          }
	    else
	    {
		sprintf( buf + strlen( buf ), "(%-5s) - %s\n\r",
		    capitalize( dir_name[door] ),
		    room_is_dark( pexit->to_room )
			?  "Too dark to tell"
			: pexit->to_room->name);
          }
      }

    }

    if ( !found )
	strcat( buf, fAuto ? " none" : "None.\n\r" );

    if ( fAuto )
    {
    if( ch->position == POS_GHOST )
    {
        strcat( buf, "&W]\n\r" );
    }
    else
    {
        strcat( buf, "&R]\n\r" );
    }
    }

    send_to_char(AT_WHITE, buf, ch );
    return;
}

void do_affectedby( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf;
    char         buf  [ MAX_STRING_LENGTH ];
    char         buf1 [ MAX_STRING_LENGTH ];

    buf1[0] = '\0';

  if ( IS_NPC( ch ) )
    return;

    if ( ( !ch->affected ) && ( !ch->affected2 ) && ( !ch->affected3 ) && ( !ch->affected4 ) && ( !ch->affected_powers ) && ( !ch->affected_weaknesses ) )
      { send_to_char( AT_CYAN, "You are not affected by anything.\n\r", ch); } 

    else
    {
    send_to_char( AT_CYAN, "You are affected by:\n\r", ch );

    if ( ch->affected )
    {

	for ( paf = ch->affected; paf; paf = paf->next )
	{
	    if ( paf->deleted )
	        continue;

	    sprintf( buf, "&RSpell: &Y'%s'", skill_table[paf->type].name );
            send_to_char( AT_WHITE, buf, ch );
	    if ( ch->level >= 20 )
	    {
		sprintf( buf,
			"&R modifies %s by %d for &G%d &Rhours",
			affect_loc_name( paf->location ),
			paf->modifier,
			paf->duration );
		send_to_char(AT_WHITE, buf, ch );
	    }

	    send_to_char( AT_WHITE, ".\n\r", ch );
	}
    }

    if ( ch->affected2 )
    {
	for ( paf = ch->affected2; paf; paf = paf->next )
	{
	    if ( paf->deleted )
	        continue;

	    sprintf( buf, "&RSpell:&Y '%s'", skill_table[paf->type].name );
            send_to_char( AT_WHITE, buf, ch );
	    if ( ch->level >= 20 )
	    {
		sprintf( buf,
			" &Rmodifies %s by %d for &G%d &Rhours",
			affect_loc_name( paf->location ),
			paf->modifier,
			paf->duration );
		send_to_char(AT_WHITE, buf, ch );
	    }
	    send_to_char( AT_WHITE, ".\n\r", ch );
	}
    }

    if ( ch->affected3 )
    {
	for ( paf = ch->affected3; paf; paf = paf->next )
	{
	    if ( paf->deleted )
	        continue;

	    sprintf( buf, "&RSpell: &Y'%s'", skill_table[paf->type].name );
            send_to_char( AT_WHITE, buf, ch );
	    if ( ch->level >= 20 )
	    {
		sprintf( buf,
			" &Rmodifies %s by %d for &G%d &Rhours",
			affect_loc_name( paf->location ),
			paf->modifier,
			paf->duration );
		send_to_char(AT_WHITE, buf, ch );
	    }

	    send_to_char( AT_WHITE, ".\n\r", ch );
	}
    }

    if ( ch->affected4 )
    {
	for ( paf = ch->affected4; paf; paf = paf->next )
	{
	    if ( paf->deleted )
	        continue;

	    sprintf( buf, "&RSpell: &Y'%s'", skill_table[paf->type].name );
            send_to_char( AT_WHITE, buf, ch );
	    if ( ch->level >= 20 )
	    {
		sprintf( buf,
			" &Rmodifies %s by %d for &G%d &Rhours",
			affect_loc_name( paf->location ),
			paf->modifier,
			paf->duration );
		send_to_char(AT_WHITE, buf, ch );
	    }

	    send_to_char( AT_WHITE, ".\n\r", ch );
	}
    }

    if ( ch->affected_powers )
    {
        for ( paf = ch->affected_powers; paf; paf = paf->next )
        {
            if ( paf->deleted )
                continue;
                        
            sprintf( buf, "Spell: '%s'", skill_table[paf->type].name );
            send_to_char( AT_WHITE, buf, ch );
            if ( ch->level >= 20 )
            {
                sprintf( buf,
                        " modifies %s by %d for %d hours",
                        affect_loc_name( paf->location ),
                        paf->modifier,
                        paf->duration );
                send_to_char(AT_WHITE, buf, ch );
            }
     
            send_to_char( AT_WHITE, ".\n\r", ch );
        }
    }

    if ( ch->affected_weaknesses )
    {
        for ( paf = ch->affected_weaknesses; paf; paf = paf->next )
        {
            if ( paf->deleted )
                continue;
                        
            sprintf( buf, "Spell: '%s'", skill_table[paf->type].name );
            send_to_char( AT_WHITE, buf, ch );
            if ( ch->level >= 20 )
            {
                sprintf( buf,
                        " modifies %s by %d for %d hours",
                        affect_loc_name( paf->location ),
                        paf->modifier,
                        paf->duration );
                send_to_char(AT_WHITE, buf, ch );
            }
     
            send_to_char( AT_WHITE, ".\n\r", ch );
        }
    }

    }

    return;
}


char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

/*
char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};
*/

char *	const	month_name	[] =
{
    "the Winter Wolf", "the Frost Giant",
    "the Grand Struggle", "Nature", "Futility", "the Dragon",
    "the Sun", "the Battle", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};



void do_time( CHAR_DATA *ch, char *argument )
{
           char  buf           [ MAX_STRING_LENGTH ];
    extern char  str_boot_time[];
           char *suf;
           int   day;

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf( buf,
	    "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r",
	    ( time_info.hour % 12 == 0 ) ? 12 : time_info.hour % 12,
	    time_info.hour >= 12 ? "pm" : "am",
	    day_name[day % 7],
	    day, suf,
	    month_name[time_info.month] );
    send_to_char(AT_YELLOW, buf, ch );
    sprintf( buf,
	    "The mud was born %d months, %d days ago.\n\r", time_info.month, (time_info.total % NUMBER_OF_DAYS_TO_RESET) );
    send_to_char(AT_YELLOW, buf, ch );
    sprintf( buf,
	    "The mud awoke at %s\rThe system time is %s\r",
	    str_boot_time,
	    (char *) ctime( &current_time ) );
    send_to_char(AT_RED, buf, ch );
    return;
}



void do_weather( CHAR_DATA *ch, char *argument )
{
           char         buf     [ MAX_STRING_LENGTH ];
    static char * const sky_look[ 4 ] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE( ch ) )
    {
	send_to_char(AT_BLUE, "You can't see the weather indoors.\n\r", ch );
	return;
    }

    sprintf( buf, "The sky is %s and %s.\n\r",
	    sky_look[weather_info.sky],
	    weather_info.change >= 0
	    ? "a warm southerly breeze blows"
	    : "a cold northern gust blows" );
    send_to_char(AT_BLUE, buf, ch );
    return;
}



void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;

    if ( argument[0] == '\0' )
	argument = "summary";

    for ( pHelp = help_first; pHelp; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch ) )
	    continue;

	if ( is_name( argument, pHelp->keyword ) )
	{
	    if ( pHelp->level >= 0 && str_cmp( argument, "imotd" ) 
	       && str_prefix( "advm_", argument ) 
	       && str_prefix( "demm_", argument ) )
	    {
		send_to_char(AT_GREY, pHelp->keyword, ch );
		send_to_char(AT_GREY, "\n\r", ch );
	    }

	    /*
	     * Strip leading '.' to allow initial blanks.
	     */
	    if ( pHelp->text[0] == '.' )
		send_to_char(AT_GREY, pHelp->text+1, ch );
	    else
		send_to_char(AT_GREY, pHelp->text  , ch );
	    return;
	}
    }

    send_to_char(AT_WHITE, "There is no help available.\n\r", ch );
    return;
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    send_to_char(AT_WHITE, "You are carrying:\n\r", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int       iWear;

    send_to_char(AT_WHITE, "You are using:\n\r", ch );
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( !( obj = get_eq_char( ch, iWear ) ) ) {
    	    if ( iWear == 20 ) {
               send_to_char(AT_BLUE, where_name[20], ch);
               send_to_char(C_DEFAULT, race_table[ch->race].default_weap, ch);
	       send_to_char(C_DEFAULT, "\n\r", ch);
            }
	    continue;
        }

	send_to_char(AT_BLUE, where_name[iWear], ch );
	if ( can_see_obj( ch, obj ) )
	{
	    send_to_char(AT_CYAN, format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char(AT_CYAN, "\n\r", ch );
	}
	else
	{
	    send_to_char(AT_RED, "something.\n\r", ch );
	}
    }

    return;
}



void do_compare( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    char     *msg;
    char      arg1 [ MAX_INPUT_LENGTH ];
    char      arg2 [ MAX_INPUT_LENGTH ];
    int       value1;
    int       value2;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Compare what to what?\n\r", ch );
	return;
    }

    if ( !( obj1 = get_obj_carry( ch, arg1 ) ) )
    {
	send_to_char(C_DEFAULT, "You do not have that item.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	for ( obj2 = ch->carrying; obj2; obj2 = obj2->next_content )
	{
	    if ( obj2->wear_loc != WEAR_NONE
		&& can_see_obj( ch, obj2 )
		&& obj1->item_type == obj2->item_type
		&& ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if ( !obj2 )
	{
	    send_to_char(C_DEFAULT, "You aren't wearing anything comparable.\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( !( obj2 = get_obj_carry( ch, arg2 ) ) )
	{
	    send_to_char(C_DEFAULT, "You do not have that item.\n\r", ch );
	    return;
	}
    }
	    
    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0];
	    value2 = obj2->value[0];
	    break;

	case ITEM_WEAPON:
	    value1 = obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( !msg )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act(C_DEFAULT, msg, ch, obj1, obj2, TO_CHAR );
    return;
}



void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "diku" );
    return;
}



void do_where( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *victim;
    DESCRIPTOR_DATA *d;
    char             buf [ MAX_STRING_LENGTH ];
    char             arg [ MAX_INPUT_LENGTH  ];
    bool             found;

    if ( !check_blind( ch ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_PURPLE, "Players near you:\n\r", ch );
	found = FALSE;
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
		&& ( victim = d->character )
		&& !IS_NPC( victim )
		&& victim->in_room
		&& victim->in_room->area == ch->in_room->area
		&& can_see( ch, victim )
		&& (!IS_AFFECTED3( victim, AFF_IMPROVED_HIDE ) ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
			victim->name, victim->in_room->name );
		send_to_char(AT_PINK, buf, ch );
	    }
	}
	if ( !found )
	    send_to_char(AT_PINK, "None\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = char_list; victim; victim = victim->next )
	{
	    if ( !victim->in_room
		|| IS_AFFECTED( victim, AFF_HIDE ) 
		|| IS_AFFECTED( victim, AFF_SNEAK )
		|| ( victim->race == 4 ) )
	        continue;

	    if ( victim->in_room->area == ch->in_room->area
		&& can_see( ch, victim )
		&& is_name( arg, victim->name ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
			PERS( victim, ch ), victim->in_room->name );
		send_to_char(AT_PINK, buf, ch );
		break;
	    }
	}
	if ( !found )
	    act(AT_PINK, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}




void do_consider( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char      *msg                      = '\0';
    char      *buf                      = '\0';
    char       arg [ MAX_INPUT_LENGTH ];
    int        diff;
    int        hpdiff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Consider killing whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char(C_DEFAULT, "They're not here.\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim ) )
    {
	send_to_char(C_DEFAULT, "The gods do not accept this type of sacrifice.\n\r",
		     ch );
	return;
    }

    diff = victim->level - ch->level;
         if ( diff <= -50 ) msg = "$N almost died from your mere gaze!";
    else if ( diff <= -25 ) msg = "$N is a complete wimp."; 
    else if ( diff <= -15 ) msg = "You can kill $N naked and weaponless.";
    else if ( diff <=  -5 ) msg = "$N is no match for you.";
    else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
    else if ( diff <=   1 ) msg = "The perfect match!";
    else if ( diff <=   4 ) msg = "$N says 'Do you feel lucky, punk?'.";
    else if ( diff <=   9 ) msg = "$N laughs at you mercilessly.";
    else if ( diff <=  12 ) msg = "Oh boy, this is gonna be tough.";
    else if ( diff <=  25 ) msg = "You got to be kidding!";
    else                    msg = "&RDont try it, you WILL die!"; 
    act(C_DEFAULT, msg, ch, NULL, victim, TO_CHAR );

    /* additions by king@tinuviel.cs.wcu.edu */
    hpdiff = ( ch->hit - victim->hit );

    if ( ( ( diff >= 0) && ( hpdiff <= 0 ) )
	|| ( ( diff <= 0 ) && ( hpdiff >= 0 ) ) )
    {
        send_to_char(C_DEFAULT, "Also,", ch );
    }
    else
    {
        send_to_char(C_DEFAULT, "However,", ch );
    }
    
    if ( hpdiff >= 2501 )
        buf = " $E is of very fragile constitution.";
    if ( hpdiff <= 2500 )
        buf = " you are currently much healthier than $E.";
    if ( hpdiff <= 500 )
        buf = " you are currently healthier than $E.";
    if ( hpdiff <= 200 ) 
        buf = " you are currently slightly healthier than $E.";
    if ( hpdiff <= 50 )
        buf = " you are a teensy bit healthier than $E.";
    if ( hpdiff <= 0 )
        buf = " $E is a teensy bit healthier than you.";
    if ( hpdiff <= -50 )
        buf = " $E is slightly healthier than you.";
    if ( hpdiff <= -200 )
        buf = " $E is healthier than you.";
    if ( hpdiff <= -500 )
        buf = " $E is much healthier than you.";
    if ( hpdiff <= -2500 )
        buf = " $E ridicules your hitpoints.";
    if ( hpdiff <= -10000 ) 
        buf = " $E is built like a TANK!.";
             
    act(C_DEFAULT, buf, ch, NULL, victim, TO_CHAR );
    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf [ MAX_STRING_LENGTH ];

    if ( IS_NPC( ch ) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    buf[0] = '\0';
    
    if ( !str_cmp( "none", title ) )
    {
       free_string( ch->pcdata->title );
       ch->pcdata->title = str_dup( " " );
       return;
     }

    if ( isalpha( title[0] ) || isdigit( title[0] ) )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Change your title to what?\n\r", ch );
	return;
    }

    if ( strlen( argument ) > 50 )
	argument[50] = '\0';

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char(C_DEFAULT, "Ok.\n\r", ch );
}



void do_description( CHAR_DATA *ch, char *argument )
{
    if(!IS_NPC( ch ) )
    {
	string_append( ch, &ch->description );
    }
    return;
}



void do_report( CHAR_DATA *ch, char *argument )
{
    char buf [ MAX_INPUT_LENGTH ];

    if (( ch->class != 9 )&&( ch->class != 11))
        sprintf( buf,
	       "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\n\r",
	       ch->hit,  ch->max_hit,
	       ch->mana, ch->max_mana,
	       ch->move, ch->max_move,
	       ch->exp );
    else
        sprintf( buf,
	       "You report: %d/%d hp %d/%d bp %d/%d mv %d xp.\n\r",
	       ch->hit,  ch->max_hit,
	       ch->bp, ch->max_bp,
	       ch->move, ch->max_move,
	       ch->exp );
	       
    send_to_char(AT_RED, buf, ch );

    if (( ch->class != 9 )&&( ch->class != 11))
         sprintf( buf,
	            "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.",
        	    ch->hit,  ch->max_hit,
        	    ch->mana, ch->max_mana,
         	    ch->move, ch->max_move,
        	    ch->exp );
    else
         sprintf( buf,
	            "$n reports: %d/%d hp %d/%d bp %d/%d mv %d xp.",
        	    ch->hit,  ch->max_hit,
        	    ch->bp,   ch->max_bp,
         	    ch->move, ch->max_move,
        	    ch->exp );

    act(AT_RED, buf, ch, NULL, NULL, TO_ROOM );

    return;
}

void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH   ];
    char buf1 [ MAX_STRING_LENGTH*10 ];
    int  sn;
    int  lvl = L_APP;

    if ( IS_NPC( ch ) )
	return;

    buf1[0] = '\0';

    if ( ch->level < 3 )
    {
	send_to_char(C_DEFAULT,
	    "You must be third level to practice.  Go train instead!\n\r",
	    ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	CHAR_DATA *mob;
	int        col;

	for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
	{
	    if ( mob->deleted )
	        continue;
	    if ( IS_NPC( mob ) && IS_SET( mob->act, ACT_PRACTICE ) )
	        break;
	}

	col    = 0;
	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( !skill_table[sn].name )
		break;
	    if ( ( ch->level < skill_table[sn].skill_level[ch->class] )
	       && (ch->level < skill_table[sn].skill_level[ch->multied] ) )
		continue;
	    if ( ( mob ) || ( ch->pcdata->learned[sn] > 0 ) )
	    {
		sprintf( buf, "%26s %5.1f%%  ", skill_table[sn].name, (UMIN(1000, ch->pcdata->learned[sn]) / 10.0f ) );
		strcat( buf1, buf );
		if ( ++col % 2 == 0 )
		    strcat( buf1, "\n\r" );
	    }
	}

	if ( col % 2 != 0 )
	    strcat( buf1, "\n\r" );

	sprintf( buf, "You have %d practice sessions left.\n\r",
		ch->practice );
	strcat( buf1, buf );
	send_to_char(C_DEFAULT, buf1, ch );
    }
    else
    {
	CHAR_DATA *mob;
	int        adept;

	if ( !IS_AWAKE( ch ) )
	{
	    send_to_char(C_DEFAULT, "In your dreams, or what?\n\r", ch );
	    return;
	}

	for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
	{
	    if ( mob->deleted )
	        continue;
	    if ( IS_NPC( mob ) && IS_SET( mob->act, ACT_PRACTICE ) )
		break;
	}

	if ( !mob )
	{
	    send_to_char(C_DEFAULT, "You can't do that here.\n\r", ch );
	    return;
	}

	if ( ch->practice <= 0 )
	{
	    send_to_char(C_DEFAULT, "You have no practice sessions left.\n\r", ch );
	    return;
	}

	if ( ( sn = skill_lookup( argument ) ) < 0
	    || ( (  ch->level < skill_table[sn].skill_level[ch->class] )
	    &&( ch->level < skill_table[sn].skill_level[ch->multied]) ) )
	{
	    send_to_char(C_DEFAULT, "You can't practice that.\n\r", ch );
	    return;
	}
	adept = 0;

	if (skill_table[sn].skill_level[ch->class] < lvl)
                lvl = skill_table[sn].skill_level[ch->class];

        if (skill_table[sn].skill_level[ch->multied] < lvl)
                lvl = skill_table[sn].skill_level[ch->multied];


	{
		int max = 0;
		double inverse = 10.0f / (double) lvl;
		max   = (int) (
				  ( inverse * 1000.0f 				) 
				+ ( int_app[get_curr_int(ch)].learn		) 
				+ ( class_table[ch->class].skill_adept		)
			      );
		adept = (int) UMIN( class_table[ch->class].skill_adept * 10, max );
	}

/*
	if(ch->multied != ch->class )
	{
	    adept = IS_NPC( ch ) ? 1000 : class_table[ch->multied].skill_adept * 10;
    	}

	
	if(ch->level >= LEVEL_CHAMP )
	{
	    adept = 1000;
	}

*/

	if ( ch->pcdata->learned[sn] >= adept )
	{
	    sprintf( buf, "You are already an adept of %s.\n\r",
		skill_table[sn].name );
	    send_to_char(C_DEFAULT, buf, ch );
	}
	else
	{
	    ch->practice--;
	    ch->pcdata->learned[sn] += (int_app[get_curr_int( ch )].learn * ((wis_app[get_curr_wis( ch )].practice + 1)/2)); 
	    if ( ch->pcdata->learned[sn] < adept )
	    {
		act(C_DEFAULT, "You practice $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act(C_DEFAULT, "$n practices $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	    else
	    {
		ch->pcdata->learned[sn] = adept;
		act(C_DEFAULT, "You are now an adept of $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act(C_DEFAULT, "$n is now an adept of $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	}
    }
    return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char buf [ MAX_STRING_LENGTH ];
    char arg [ MAX_INPUT_LENGTH  ];
    int  wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char(C_DEFAULT, "Your courage exceeds your wisdom.\n\r", ch );
	return;
    }

    if ( wimpy > ch->max_hit )
    {
	send_to_char(C_DEFAULT, "Such cowardice ill becomes you.\n\r", ch );
	return;
    }

    ch->wimpy	= wimpy;
    sprintf( buf, "Wimpy set to %d hit points.\n\r", wimpy );
    send_to_char(C_DEFAULT, buf, ch );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char *pArg;
    char *pwdnew;
    char *p;
    char  arg1 [ MAX_INPUT_LENGTH ];
    char  arg2 [ MAX_INPUT_LENGTH ];
    char  cEnd;

    if ( IS_NPC( ch ) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace( *argument ) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace( *argument ) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1 , ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char(C_DEFAULT, "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen( arg2 ) < 5 )
    {
	send_to_char(C_DEFAULT,
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(C_DEFAULT,
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch, FALSE );
    send_to_char(C_DEFAULT, "Ok.\n\r", ch );
    return;
}



void do_socials( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH * 2 ];
    char buf1 [ MAX_STRING_LENGTH * 2 ];
    int  iSocial;
    int  col;

    buf1[0] = '\0';
    col = 0;
    for ( iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++ )
    {
	sprintf( buf, "%-12s", social_table[iSocial].name );
	strcat( buf1, buf );
	if ( ++col % 6 == 0 )
	    strcat( buf1, "\n\r" );
    }
 
    if ( col % 6 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char(C_DEFAULT, buf1, ch );
    return;
}



/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH * 2];
    int  cmd;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if (   cmd_table[cmd].level <  LEVEL_HERO
	    && cmd_table[cmd].level <= get_trust( ch ) )
	{
	  if ( str_prefix( "mp", cmd_table[cmd].name ) )
	  {
	    sprintf( buf, "%-16s", cmd_table[cmd].name );
	    strcat( buf1, buf );
	    if ( ++col % 5 == 0 )
		strcat( buf1, "\n\r" );
	  }
	}
    }
 
    if ( col % 5 != 0 )
	strcat( buf1, "\n\r" );

    send_to_char(C_DEFAULT, buf1, ch );
    return;
}



void do_channels( CHAR_DATA *ch, char *argument )
{
    char arg [ MAX_INPUT_LENGTH  ];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_SILENCE ) )
	{
	    send_to_char(AT_PURPLE, "You are silenced.\n\r", ch );
	    return;
	}

	send_to_char(AT_PURPLE, "Channels:", ch );

	send_to_char(AT_PINK, !IS_SET( ch->deaf, CHANNEL_AUCTION  )
		     ? " +AUCTION"
		     : " -auction",
		     ch );

	send_to_char(AT_PINK, !IS_SET( ch->deaf, CHANNEL_CHAT     )
		     ? " +CHAT"
		     : " -chat",
		     ch );

	if(IS_HERO(ch)) /* XOR */
	{
	  send_to_char(AT_PINK, !IS_SET(ch->deaf, CHANNEL_CHAMPION)
	   ? " +CHAMPION"
	   : " -champion", ch);
	}
        if(IS_DEMIGOD(ch)) /* Tyrion */
	{
	  send_to_char(AT_PINK, !IS_SET(ch->deaf, CHANNEL_DEMIGOD)
	   ? " +DEMIGOD"
	   : " -demigod", ch);
	}
	if ( IS_IMMORTAL( ch ) )
	{
	    send_to_char(AT_PINK, !IS_SET( ch->deaf, CHANNEL_IMMTALK )
			 ? " +IMMTALK"
			 : " -immtalk",
			 ch );
	}

	send_to_char(AT_PINK, !IS_SET( ch->deaf, CHANNEL_MUSIC    )
		     ? " +MUSIC"
		     : " -music",
		     ch );

	send_to_char(AT_PINK, !IS_SET( ch->deaf, CHANNEL_INFO     )
		     ? " +INFO"
		     : " -info",
		     ch );

	send_to_char(AT_PINK, !IS_SET( ch->deaf, CHANNEL_QUESTION )
		     ? " +QUESTION"
		     : " -question",
		     ch );

	send_to_char(AT_PINK, !IS_SET( ch->deaf, CHANNEL_SHOUT    )
		     ? " +SHOUT"
		     : " -shout",
		     ch );

	send_to_char(AT_PINK, !IS_SET( ch->deaf, CHANNEL_MUTTER   )
		     ? " +MUTTER"
		     : " -mutter",
		     ch );

        send_to_char(AT_PINK, !IS_SET( ch->deaf, CHANNEL_OOC    )
		     ? " +OOC"
		     : " -ooc",
		     ch );

        send_to_char(AT_PINK, !IS_SET( ch->deaf, CHANNEL_GRATZ    )
		    ? " +GRATZ"
		    : " -gratz",
		    ch );

	send_to_char(AT_PINK, !IS_SET( ch->deaf, CHANNEL_YELL     )
		     ? " +YELL"
		     : " -yell",
		     ch );


	/*
	 * Log Channel Display.
	 * Added by Altrag.
	 */
	if ( get_trust( ch ) >= L_APP )
	{
		send_to_char(AT_PURPLE, !IS_SET( ch->deaf, CHANNEL_LOG )
				 ? " +LOG"
				 : " -log",
				 ch );
	}
	
	if ( get_trust( ch ) >= L_APP )
	{
		send_to_char(AT_PURPLE, !IS_SET( ch->deaf, CHANNEL_BUILD )
				 ? " +BUILD"
				 : " -build",
				 ch );
	}
	
	if ( get_trust( ch ) >= L_DIR )
	{
		send_to_char(AT_PURPLE, !IS_SET( ch->deaf, CHANNEL_GOD )
				 ? " +GOD"
				 : " -god",
				 ch );
	}
	
	if ( get_trust( ch ) >= L_DIR )
	{
	       send_to_char(AT_PURPLE, !IS_SET( ch->deaf, CHANNEL_GUARDIAN )
	       			 ? " +GUARD"
	       			 : " -guard",
	       			 ch );
	}

	if ( get_trust( ch ) >= L_CON )
	{
	       send_to_char(AT_PURPLE, !IS_SET( ch->deaf, CHANNEL_TIMELORD )
				 ? " +TIMELORD"
				 : " -timelord",
				 ch );
 	}

	if ( get_trust( ch ) >= L_CON )
	{
	       send_to_char(AT_PURPLE, !IS_SET( ch->deaf, CHANNEL_COMLOG )
				? " +COMLOG"
				: " -comlog",
				ch );
	}
		
	send_to_char(AT_PINK, ".\n\r", ch );
    }
    else
    {
	int  bit;
	bool fClear;

	     if ( arg[0] == '+' ) fClear = TRUE;
	else if ( arg[0] == '-' ) fClear = FALSE;
	else
	{
	    send_to_char(AT_PURPLE, "Channels -channel or +channel?\n\r", ch );
	    return;
	}

	     if ( !str_cmp( arg+1, "auction"  ) ) bit = CHANNEL_AUCTION;
        else if ( !str_cmp( arg+1, "chat"     ) ) bit = CHANNEL_CHAT;
        else if ( !str_cmp( arg+1, "info"     ) ) bit = CHANNEL_INFO;
	else if ( !str_cmp( arg+1, "champion" ) ) bit = CHANNEL_CHAMPION;
	else if ( !str_cmp( arg+1, "demigod"  ) ) bit = CHANNEL_DEMIGOD;
	else if ( !str_cmp( arg+1, "immtalk"  ) ) bit = CHANNEL_IMMTALK;
	else if ( !str_cmp( arg+1, "music"    ) ) bit = CHANNEL_MUSIC;
	else if ( !str_cmp( arg+1, "question" ) ) bit = CHANNEL_QUESTION;
	else if ( !str_cmp( arg+1, "shout"    ) ) bit = CHANNEL_SHOUT;
	else if ( !str_cmp( arg+1, "yell"     ) ) bit = CHANNEL_YELL;
	else if ( !str_cmp( arg+1, "mutter"   ) ) bit = CHANNEL_MUTTER;
	else if ( !str_cmp( arg+1, "gratz"    ) ) bit = CHANNEL_GRATZ;
	else if ( !str_cmp( arg+1, "log"      ) ) bit = CHANNEL_LOG;
	else if ( !str_cmp( arg+1, "build"    ) ) bit = CHANNEL_BUILD;
	else if ( !str_cmp( arg+1, "god"      ) ) bit = CHANNEL_GOD;
	else if ( !str_cmp( arg+1, "guard"    ) ) bit = CHANNEL_GUARDIAN;
        else if ( !str_cmp( arg+1, "timelord" ) ) bit = CHANNEL_TIMELORD;
	else if ( !str_cmp( arg+1, "comlog"   ) ) bit = CHANNEL_COMLOG;
        else if ( !str_cmp( arg+1, "ooc"      ) ) bit = CHANNEL_OOC;
	else if ( !str_cmp( arg+1, "all"      ) ) 
	 bit = ( CHANNEL_AUCTION + CHANNEL_CHAT + CHANNEL_CHAMPION 
		+ CHANNEL_DEMIGOD + CHANNEL_IMMTALK + CHANNEL_MUSIC
		+ CHANNEL_INFO + CHANNEL_QUESTION + CHANNEL_SHOUT
		+ CHANNEL_MUTTER + CHANNEL_OOC + CHANNEL_GRATZ
	        + CHANNEL_YELL + CHANNEL_LOG + CHANNEL_BUILD 
		+ CHANNEL_GOD + CHANNEL_GUARDIAN + CHANNEL_TIMELORD );
	else if ( !str_cmp( arg+1, "mort"     ) )
	 bit = ( CHANNEL_AUCTION + CHANNEL_CHAT + CHANNEL_CHAMPION
		+ CHANNEL_DEMIGOD + CHANNEL_MUSIC
		+ CHANNEL_MUTTER + CHANNEL_QUESTION + CHANNEL_SHOUT
		+ CHANNEL_GRATZ + CHANNEL_OOC );
	else
	{
	    send_to_char(AT_PURPLE, "Set or clear which channel?\n\r", ch );
	    return;
	}

	if ( fClear )
	    REMOVE_BIT ( ch->deaf, bit );
	else
	    SET_BIT    ( ch->deaf, bit );

	send_to_char(AT_PURPLE, "Ok.\n\r", ch );
    }

    return;
}

void do_study( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *book;
	char	arg [ MAX_INPUT_LENGTH ];

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( AT_WHITE, "Study what?\r\n", ch );
		return;
	}

	if( !( book = get_obj_carry( ch, arg ) ) )
	{
		send_to_char( AT_WHITE, "You don't have that book.\n\r", ch );
		return;
	}

	if ( book->item_type != ITEM_BOOK )
	{  
		send_to_char( AT_WHITE, "You can only study books.\n\r", ch );
		return;
	}

	if( ch->level < book->level )
	{
		send_to_char(AT_WHITE,"You are not a high enough level to study this book.\n\r", ch );
		return;
	}

	if( ( ch->level < skill_table[book->value[1]].skill_level[ch->class] ) &&
	   ( ch->level < skill_table[book->value[1]].skill_level[ch->multied] ) &&
	   ( ch->level < skill_table[book->value[2]].skill_level[ch->class] ) &&
	   ( ch->level < skill_table[book->value[2]].skill_level[ch->multied] ) &&
	   ( ch->level < skill_table[book->value[3]].skill_level[ch->class] ) &&
	   ( ch->level < skill_table[book->value[3]].skill_level[ch->multied] ) )
	{
		send_to_char(AT_WHITE,"This book contains magic that is too powerful for you.\n\r", ch );
		return;
	}

	if( ( ch->pcdata->learned[book->value[1]] >= 1000 ) &&
	   ( ch->pcdata->learned[book->value[2]] >= 1000 ) &&
	   ( ch->pcdata->learned[book->value[3]] >= 1000 ) )
	{ 
		send_to_char( AT_WHITE, "This book can not teach you anything.\n\r", ch );
		return;
	}

	act( AT_WHITE, "$n studies $p.", ch, book, NULL, TO_ROOM);
	act( AT_WHITE, "You study $p.", ch, book, NULL, TO_CHAR);

	if( ( ch->level >= skill_table[book->value[1]].skill_level[ch->class] ) ||
	   ( ch->level >= skill_table[book->value[1]].skill_level[ch->multied] ) )
	{
		update_skpell( ch, book->value[1], book->value[0] );
	}
	if( ( ch->level >= skill_table[book->value[2]].skill_level[ch->class] ) ||
	   ( ch->level >= skill_table[book->value[2]].skill_level[ch->multied] ) )
	{
		update_skpell( ch, book->value[2], book->value[0] );
	}
	if( ( ch->level >= skill_table[book->value[3]].skill_level[ch->class] ) ||
	   ( ch->level >= skill_table[book->value[3]].skill_level[ch->multied] ) )
	{
		update_skpell( ch, book->value[3], book->value[0] );
	}
	extract_obj( book );

	return;
}

void do_score( CHAR_DATA *ch, char *argument )
{
    char         buf  [ MAX_STRING_LENGTH ];
    char	 arg  [ MAX_INPUT_LENGTH] ;

    argument = one_argument( argument, arg );
   
    if (IS_NPC(ch))
    {
      return;
      do_mstat(ch, "self");
      return;
    }

    if( arg[0] == '\0' )
    {
        do_pcscore( ch, "page1" );
        return;
    }
    if ( !str_cmp( arg, "one" ) || !strcmp( arg, "1" ) )
    {
	do_pcscore( ch, "page1" );
	return;
    }

    if ( !str_cmp( arg, "two" ) || !strcmp( arg, "2" ) )
    {
	do_pcscore( ch, "page2" );
	return;
    }

    if ( !str_cmp( arg, "three" ) || !strcmp( arg, "3" ) )
    {
	do_pcscore( ch, "page3" );
	return;
    }


    if (!IS_NPC( ch ) )
    {
	do_pcscore( ch, "self" );
	return;
    }

    sprintf( buf,
	    "Name : %s%s.\n\r",
	    ch->name,
	    IS_NPC( ch ) ? "" : ch->pcdata->title );
    send_to_char( AT_CYAN, buf, ch );
    sprintf( buf,
	    "Race : %s\n\r",
		race_table[ch->race].race_full );
    send_to_char( AT_CYAN, buf, ch );
    sprintf( buf, "Class: %s\n\r",
		class_table[ch->class].who_long );
    send_to_char( AT_CYAN, buf, ch );
    if(ch->multied != ch->class )
    {
	sprintf( buf, "Multi: %s\n\r",
	    class_table[ch->multied].who_long );
	send_to_char( AT_CYAN, buf, ch );
    }
    if(ch->multied == ch->class )
    {
        send_to_char( AT_CYAN, "Multi: NONE\n\r", ch );
    }
    sprintf( buf, 
		"Level: %d\n\r",
	    ch->level );
    send_to_char(AT_CYAN, buf, ch );
    sprintf( buf,
            "Age  : %d years old (%d hours).\n\r",
	    get_age( ch ),
	    (get_age( ch ) - 17) * 4 );
    send_to_char( AT_CYAN, buf, ch );
    if ( ch->clan )
    {
        CLAN_DATA *clan;
        
	clan = get_clan_index( ch->clan );
	sprintf( buf, "Clan : %s\n\r", clan->name );
	send_to_char(AT_CYAN, buf, ch );
	if ( ch->ctimer )
	{
	    sprintf( buf, "Timer: %d.\n\r", ch->ctimer );
	    send_to_char( AT_CYAN, buf, ch );
	}
    }

    if(ch->guild != NULL)
    {
      sprintf(buf, "Guild: %s", ch->guild->name);
      switch(ch->guild_rank)
      {
	    case 0: sprintf(buf+strlen(buf), " ");		break;
	    case 1: sprintf(buf+strlen(buf), "  Rank: %s ", ch->sex == SEX_FEMALE ?
			    "Lady" : "Lord" );	break;
	    case 2: sprintf(buf+strlen(buf), " High %s ",
			    ch->sex == SEX_FEMALE ? "Lady" : "Lord" );	break;
	    case 3: sprintf(buf+strlen(buf), "  Rank: Over%s ",
			    ch->sex == SEX_FEMALE ? "lady" : "lord" );	break;
          default: sprintf(buf+strlen(buf), "Bug ");		break;
      }
      send_to_char(ch->guild->color, buf, ch);
      send_to_char(AT_CYAN, "\n\r", ch );
    }

    if ( get_trust( ch ) != ch->level )
    {
	sprintf( buf, "Trust: %d.\n\r",
		get_trust( ch ) );
        send_to_char( AT_CYAN, buf, ch );
    }

    sprintf( buf,
	"&pStr  : &P%d&p(&P%d&p)     &OHit: &Y%d&O/&Y%d HP\n\r",
	IS_NPC(ch) ? 13: ch->pcdata->perm_str, IS_NPC(ch) ? 13: get_curr_str( ch ),
	ch->hit, ch->max_hit );
    send_to_char( AT_CYAN, buf, ch );

    if (( ch->class != 9 )&&( ch->class != 11))
    {   
	sprintf( buf,
	"&pInt  : &P%d&p(&P%d&p)    &cMana: &C%d&c/&C%d MA\n\r",
	IS_NPC(ch) ? 13: ch->pcdata->perm_int, IS_NPC(ch) ? 13: get_curr_int( ch ),
	ch->mana, ch->max_mana );
    }
    else
    {
	sprintf ( buf,
	"&pInt  : &P%d&p(&P%d&p)   &rBlood: &R%d&r/&R%d BP\n\r",
	IS_NPC(ch) ? 13: ch->pcdata->perm_int, IS_NPC(ch) ? 13: get_curr_int( ch ),
	ch->bp, ch->max_bp );
    }
    send_to_char( AT_CYAN, buf, ch );
    sprintf( buf,
	"&pWis  : &P%d&p(&P%d&p)    &gMove: &G%d&g/&G%d MV\n\r",
	IS_NPC(ch) ? 13: ch->pcdata->perm_wis, IS_NPC(ch) ? 13: get_curr_wis( ch ),
	ch->move, ch->max_move );
    send_to_char( AT_CYAN, buf, ch );
    sprintf( buf,
	"&pDex  : &P%d&p(&P%d&p)    &OGold: &Y%d Gold\n\r",
	IS_NPC(ch) ? 13: ch->pcdata->perm_dex, IS_NPC(ch) ? 13: get_curr_dex( ch ),
	ch->gold );
    send_to_char( AT_CYAN, buf, ch );
    sprintf( buf,
	"&pCon  : &P%d&p(&P%d&p)    &OBank: &Y%d Gold\n\r",
	IS_NPC(ch) ? 13: ch->pcdata->perm_con, IS_NPC(ch) ? 13: get_curr_con( ch ),
	ch->pcdata->bankaccount );
    send_to_char( AT_CYAN, buf, ch );

    if ( ch->religion )
    {
        RELIGION_DATA	*religion;

        religion = get_religion_index( ch->religion );
	sprintf(buf, "&RReligion      :&W %s\n\r", religion->shortdesc  );
 	send_to_char(AT_CYAN, buf, ch);
    }
    sprintf( buf, "&RGranted Powers: &W%s.\n\r", affect_bit_name_powers( ch->affected_by_powers ) );
    send_to_char( AT_CYAN, buf, ch );
    sprintf( buf, "&RWeaknesses    : &W%s.\n\r", affect_bit_name_weaknesses( ch->affected_by_weaknesses ) );
    send_to_char( AT_CYAN, buf, ch );
    
    send_to_char( AT_CYAN, "------------------------------------------------------------\n\r", ch );

    sprintf( buf,
	"&rAutoexit    : &R%s            &cLearns      : &C%d learns\n\r",
	( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOEXIT ) ) ? "yes" : "no ",
	ch->pcdata->learn );
    send_to_char(AT_CYAN, buf, ch );
    sprintf( buf,
	"&rAutoloot    : &R%s            &cQuest Points: &C%dqp\n\r",
	( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOLOOT ) ) ? "yes" : "no ",
	ch->questpoints );
    send_to_char(AT_CYAN, buf, ch );
    sprintf( buf,
	"&rAutosac     : &R%s            &cPractices   : &C%d practices\n\r",
	( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOSAC  ) ) ? "yes" : "no ",
	ch->practice );
    send_to_char(AT_CYAN, buf, ch );
    sprintf( buf,
	"&rAutogold    : &R%s            &cPage Pausing: &C%d lines\n\r",
	( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOGOLD ) ) ? "yes" : "no ",
    ch->pcdata->pagelen );
    send_to_char( AT_CYAN, buf, ch );    

    sprintf( buf,
	"&gCarrying    &G: %4d &gof&G %4d   &gWeight      : &G%7d&g/&G%7d kg\n\r",
	ch->carry_number, can_carry_n( ch ),
	ch->carry_weight, can_carry_w( ch ) );
    send_to_char( AT_CYAN, buf, ch );

    sprintf( buf, "&bExperience  : &B%6d exp     &bSpeaking    : &B%s\n\r",
	ch->exp, lang_table[ch->speaking].name );
    send_to_char( AT_CYAN, buf, ch );

    sprintf( buf, "&bWimpy       : &B%d HP\n\r", ch->wimpy );
    send_to_char( AT_CYAN, buf, ch );
    if(!IS_NPC(ch))
    {
      sprintf(buf,"&bPlayer Kills: &G%6d         &bLosses      : &R%6d\n\r",
      ch->pcdata->awins, ch->pcdata->alosses );
      send_to_char(AT_CYAN,buf,ch);
    }

    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK]   > 10 )
	send_to_char( AT_CYAN, "You are drunk.\n\r", ch );
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] ==  0
	&& ch->level >= LEVEL_IMMORTAL && ch->position != POS_GHOST )
	send_to_char( AT_CYAN, "You are thirsty.\n\r", ch );
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL]   ==  0
	&& ch->level >= LEVEL_IMMORTAL && ch->position != POS_GHOST )
	send_to_char( AT_CYAN, "You are hungry.\n\r", ch  );

    switch ( ch->position )
    {
    case POS_DEAD:     
	send_to_char( (AT_RED + AT_BLINK), "You are DEAD!!\n\r", ch ); break;
    case POS_MORTAL:
	send_to_char( AT_RED, "You are mortally wounded.\n\r", ch ); break;
    case POS_INCAP:
	send_to_char( AT_RED, "You are incapacitated.\n\r", ch ); break;
    case POS_STUNNED:
	send_to_char( AT_RED, "You are stunned.\n\r", ch ); break;
    case POS_SLEEPING:
	send_to_char( AT_CYAN, "You are sleeping.\n\r", ch ); break;
    case POS_RESTING:
	send_to_char( AT_CYAN, "You are resting.\n\r", ch ); break;
    case POS_STANDING:
	send_to_char( AT_CYAN, "You are standing.\n\r", ch ); break;
    case POS_GHOST:
	send_to_char( AT_CYAN, "You are a floating, incorporeal ghost.\n\r", ch ); break;
    case POS_FIGHTING:
	send_to_char( AT_BLOOD, "You are fighting.\n\r", ch ); break;
    }

    if ( ch->level >= 20 )
    {
	sprintf( buf, "&gAC: &G%d.  ", GET_AC( ch ) );
	send_to_char( AT_CYAN, buf, ch );
    }

    send_to_char( AT_GREEN, "You are ", ch );
         if ( GET_AC( ch ) >=  101 ) send_to_char( AT_GREEN, "WORSE than naked!\n\r", ch );
    else if ( GET_AC( ch ) >=   20 ) send_to_char( AT_GREEN, 
"naked.\n\r"           , ch );
    else if ( GET_AC( ch ) >=    0 ) send_to_char( AT_GREEN, "wearing "
"clothes.\n\r" , ch );
    else if ( GET_AC( ch ) >= - 50 ) send_to_char( AT_GREEN, "slightly "
"armored.\n\r", ch );
    else if ( GET_AC( ch ) >= -100 ) send_to_char( AT_GREEN, "somewhat "
"armored.\n\r", ch );
    else if ( GET_AC( ch ) >= -250 ) send_to_char( AT_GREEN, 
"armored.\n\r"         , ch );
    else if ( GET_AC( ch ) >= -500 ) send_to_char( AT_GREEN, "well "
"armored.\n\r"    , ch );
    else if ( GET_AC( ch ) >= -750 ) send_to_char( AT_GREEN, "strongly "
"armored.\n\r", ch );
    else if ( GET_AC( ch ) >= -1000 ) send_to_char( AT_GREEN, "heavily "
"armored.\n\r" , ch );
    else if ( GET_AC( ch ) >= -1200 ) send_to_char( AT_GREEN, "superbly "
"armored.\n\r", ch );
    else if ( GET_AC( ch ) >= -1400 ) send_to_char( AT_GREEN, "divinely "
"armored.\n\r", ch );
    else                           send_to_char( AT_GREEN, "invincible!\n\r", ch );

    if ( ch->level >= 12 )
    {
	sprintf( buf, "Hitroll: " );
	send_to_char(AT_BLOOD, buf, ch );
	sprintf( buf, "%d", GET_HITROLL( ch ) );
	send_to_char(AT_RED, buf, ch);
	sprintf( buf, "  Damroll: " );
	send_to_char( AT_BLOOD, buf, ch );
	sprintf( buf, "%d.\n\r", GET_DAMROLL( ch ) * 2 );
	send_to_char( AT_RED, buf, ch );
    }
    
    if ( ch->level >= 8 )
    {
	sprintf( buf, "Alignment: %d.  ", ch->alignment );
	send_to_char( AT_CYAN, buf, ch );
    }

    send_to_char( AT_CYAN, "You are ", ch );
         if ( ch->alignment >  900 ) send_to_char( AT_BLUE, "angelic.\n\r",ch );
    else if ( ch->alignment >  700 ) send_to_char( AT_BLUE, "saintly.\n\r",ch );
    else if ( ch->alignment >  350 ) send_to_char( AT_BLUE, "good.\n\r"   ,ch );
    else if ( ch->alignment >  100 ) send_to_char( AT_BLUE, "kind.\n\r"   ,ch );
    else if ( ch->alignment > -100 ) send_to_char( AT_YELLOW, "neutral.\n\r",ch );
    else if ( ch->alignment > -350 ) send_to_char( AT_RED, "mean.\n\r"    ,ch);
    else if ( ch->alignment > -700 ) send_to_char( AT_RED, "evil.\n\r"    ,ch);
    else if ( ch->alignment > -900 ) send_to_char( AT_RED, "demonic.\n\r" ,ch);
    else                             send_to_char( AT_RED, "satanic.\n\r" ,ch);
  
}


void do_devote( CHAR_DATA *ch, char *argument )
{
    char arg [ MAX_INPUT_LENGTH ];

    if ( IS_NPC( ch ) )
    {
	return;
    }

    one_argument( argument, arg );

    if( ch->religion != 0 )
    {
	send_to_char(C_DEFAULT, "You are already part of a religion.\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Devote to which Religion?\n\r\n\r", ch );
        send_to_char( C_DEFAULT, "1. The Order of Truth, Honour and Chivalry\n\r", ch );
        send_to_char( C_DEFAULT, "2. The Order of Law, Order and Justice\n\r", ch );
        send_to_char( C_DEFAULT, "3. The Order of Life and Nature\n\r", ch );
        send_to_char( C_DEFAULT, "4. The Order of Lies, Manipulation and Deceit\n\r", ch );
        send_to_char( C_DEFAULT, "5. The Order of Chaos, Strife and Injustice\n\r", ch );
        send_to_char( C_DEFAULT, "6. The Order of Death and Decay\n\r", ch );
        send_to_char( C_DEFAULT, "7. The Order of Time and Fate\n\r\r", ch); 
	return;
    }

    switch( argument[0] )
    {
    default:
	ch->religion = 0;
	send_to_char(AT_WHITE, "Syntax: Devote\n\r", ch );
	send_to_char(AT_WHITE, "Syntax: Devote <number>\n\r\n\r", ch);
	send_to_char(AT_WHITE, "Devote with no argument displays options.\n\r", ch);
    break;
    case '1':
	ch->religion = 1;
	send_to_char(C_DEFAULT, "Ok.\n\r", ch );
    break;
    case '2':
	ch->religion = 2;
	send_to_char(C_DEFAULT, "Ok.\n\r", ch );
    break;
    case '3':
	ch->religion = 3;
	send_to_char(C_DEFAULT, "Ok.\n\r", ch );
    break;
    case '4':
	ch->religion = 4;
	send_to_char(C_DEFAULT, "Ok.\n\r", ch );
    break;
    case '5':
	ch->religion = 5;
	send_to_char(C_DEFAULT, "Ok.\n\r", ch );
    break;
    case '6':
	ch->religion = 6;
	send_to_char(C_DEFAULT, "Ok.\n\r", ch );
    break;
    case '7':
	ch->religion = 7;
	send_to_char(C_DEFAULT, "Ok.\n\r", ch );
    break;
    }

    return;
}

/*
 * New 'who' command originally by Alander of Rivers of Mud.
 * Modified a whole bunch and almost completely rewritten by Tyrion
 */
void do_who( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char             arg1[ MAX_STRING_LENGTH ];
    char             buf      [ MAX_STRING_LENGTH*3 ];
    char             buf2     [ MAX_STRING_LENGTH   ];
    char	     xtext    [ MAX_STRING_LENGTH   ];
    CLAN_DATA *      tClan;
    RELIGION_DATA *  tReligion;
    int              iClass;
    int		     iMulticlass;
    int              iLevelLower;
    int              iLevelUpper;
    int              nNumber;
    int              nMatch;
    bool             rgfClass [ MAX_CLASS ];
    bool	     rgfMulticlass [ MAX_CLASS ];
    bool             fClassRestrict;
    bool	     fMulticlassRestrict;
    bool 	     fClanRestrict;
    bool             fImmortalOnly;
    bool	     fRaceRestrict;
    bool	     fNameRestrict;
    bool	     fReligionRestrict;
    bool             lng = FALSE;
    int		     iRace;
    int	   	     iClan;
    int		     iReligion;
    bool	     rgfReligion [ MAX_RELIGION ];
    bool	     rgfClan [ MAX_CLAN ];
    bool	     rgfRace [ MAX_RACE ];
    int              num_of_imm = 0;
    int              noclass[MAX_CLASS];
    int              WC_ONE = AT_LBLUE;
    int              WC_TWO = AT_CYAN;
    int              WC_THREE = AT_BLUE;    
    int              scheme = 0;

    iLevelLower    = 0;
    iLevelUpper    = L_IMP; /*Used to be Max_level */
    fClassRestrict = FALSE;
    fMulticlassRestrict = FALSE;
    fImmortalOnly  = FALSE;
    fRaceRestrict  = FALSE;
    fNameRestrict  = FALSE;
    fClanRestrict  = FALSE;
    fReligionRestrict = FALSE;
    tReligion = NULL;
    tClan = NULL;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        noclass[iClass] = 0;
    for ( iClan = 0; iClan < MAX_CLAN; iClan++ )
        rgfClan[iClan] = FALSE;
    for ( iRace = 0; iRace < MAX_RACE; iRace++ )
        rgfRace[iRace] = FALSE;
    for ( iReligion = 0; iReligion < MAX_RELIGION; iReligion++ )
	  rgfReligion[iReligion] = FALSE;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        rgfClass[iClass] = FALSE;
    for ( iMulticlass = 0; iMulticlass < MAX_CLASS; iMulticlass++ )
        rgfMulticlass[iMulticlass] = FALSE;

    /* 
     * Set up colour scheme
     */

    scheme = number_range( 1, 11 );

    if( scheme == 1 )
    {
        WC_ONE = AT_LBLUE;	/* Class */
        WC_TWO = AT_CYAN;	/* Highlight */
        WC_THREE = AT_BLUE;	/* Person Text */
    }
    if( scheme == 2 )
    {
        WC_ONE = AT_RED;
        WC_TWO = AT_BLOOD;
        WC_THREE = AT_PURPLE;
    }
    if( scheme == 3 )
    {
        WC_ONE = AT_PINK;
        WC_TWO = AT_PURPLE;
        WC_THREE = AT_RED;
    }
    if( scheme == 4 )
    {
        WC_ONE = AT_GREEN;
        WC_TWO = AT_DGREEN;
        WC_THREE = AT_GREEN;
    }
    if( scheme == 5 )
    {
        WC_ONE = AT_ORANGE;
        WC_TWO = AT_YELLOW;
        WC_THREE = AT_YELLOW;
    }
    if( scheme == 6 )
    {
        WC_ONE = AT_DGREY;
        WC_TWO = AT_BLOOD;
        WC_THREE = AT_DGREY;
    }
    if( scheme == 7 )
    {
        WC_ONE = AT_DGREY;
        WC_TWO = AT_GREY;
        WC_THREE = AT_GREY;
    }
    if( scheme == 8 )
    {
        WC_ONE = AT_BLOOD;
        WC_TWO = AT_DGREY;
        WC_THREE = AT_DGREY;
    }
    if( scheme == 9 )
    {
	WC_ONE = AT_GREY;
        WC_TWO = AT_WHITE;
        WC_THREE = AT_YELLOW;
    }
    if( scheme == 10 )
    {
	WC_ONE = AT_BLUE;
	WC_TWO = AT_GREEN;
        WC_THREE = AT_PINK;
    }
    if( scheme == 11 )
    {
	WC_ONE = AT_WHITE;
	WC_TWO = AT_GREY;
	WC_THREE = AT_YELLOW;
    }

    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
        char arg [ MAX_STRING_LENGTH ];

        argument = one_argument( argument, arg );
        if ( arg[0] == '\0' )
            break;
        strcpy( arg1, arg );

        if ( is_number( arg ) )
        {
            switch ( ++nNumber )
            {
                case 1: iLevelLower = atoi( arg ); break;
                case 2: iLevelUpper = atoi( arg ); break;
                default:
                    send_to_char(WC_THREE, "Only two level numbers allowed.\n\r", ch );
                    return;
            }
        }
        else
        {
            int iClass;

            if ( strlen( arg ) < 3 )
            {
                send_to_char(WC_THREE, "Classes must be longer than that.\n\r", ch );
                return;
            }
	    
            if ( strlen( arg ) > 3 )
                lng = TRUE;

            /*
             * Look for classes to turn on.
             */
            arg[3]    = '\0';
            if ( !str_cmp( arg, "imm" ) )
            {
                fImmortalOnly = TRUE;
            }
            else
            {
                fClassRestrict = TRUE;
                for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
                {
                    if ( ( !str_cmp( arg, class_table[iClass].who_name ) && !lng )
                    || ( !str_cmp( arg1, class_table[iClass].who_long ) && lng ) )
                    {
                        rgfClass[iClass] = TRUE;
                        break;
                    }
                }

                if ( iClass == MAX_CLASS )
                {
                    fClassRestrict = FALSE;
                    fMulticlassRestrict = TRUE;
                    for (iMulticlass = 0; iMulticlass < MAX_CLASS; iMulticlass++ )
                    {
                        if ( ( !str_cmp( arg, class_table[iMulticlass].who_name ) && !lng )
                        || ( !str_cmp( arg1, class_table[iMulticlass].who_long ) && !lng ) )
                        {
                            rgfMulticlass[iMulticlass] = TRUE;
                            break;
                        }
                    }
                    if ( iMulticlass == MAX_CLASS || iMulticlass == ch->class )
                    {
                        fMulticlassRestrict = FALSE;
                        fRaceRestrict = TRUE;
                        for ( iRace = 0; iRace < MAX_RACE; iRace++ )
                        {
                            if ( ( !str_prefix( arg, race_table[iRace].race_name ) && !lng )
                            || ( !str_cmp( arg1, race_table[iRace].race_full ) ) )
                            {
                                rgfRace[iRace] = TRUE;
                                break;
                            }
                        }
                        if ( iRace == MAX_RACE )
                        {
                            fRaceRestrict = FALSE;
                            fClanRestrict = TRUE;
                            for ( iClan = 0; iClan < MAX_CLAN; iClan++ )
                            {
                                tClan = get_clan_index( iClan );
                                if ( !(tClan) )
                                    continue;

                                if ( !str_cmp( arg1, tClan->name ) )
                                {
                                    rgfClan[iClan] = TRUE;
                                    break;
                                }
                            }  
                            if ( iClan >= MAX_CLAN )
                            { 
                                fClanRestrict = FALSE;    
                                fReligionRestrict = TRUE;
                                for ( iReligion = 0; iReligion < MAX_RELIGION; iReligion++ )
                                {
                                    tReligion = get_religion_index( iReligion );
                                    if ( !(tReligion) )
                                        continue;

                                    if ( !str_cmp( arg1, tReligion->name ) )
                                    {
                                        rgfReligion[iReligion] = TRUE;
                                        break;
                                    }
                                }
                                if ( iReligion >= MAX_RELIGION )
                                {
                                    fReligionRestrict = FALSE;
                                    fNameRestrict = TRUE;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    rgfClan[0] = FALSE;
    rgfReligion[0] = FALSE;
    if ( fNameRestrict )
      send_to_char( C_DEFAULT, "\n\r", ch );  
    for ( d = descriptor_list; d; d = d->next )
    {
	CHAR_DATA       *wch;
	CLAN_DATA       *pClan;
	RELIGION_DATA	*pReligion;
	char      const *class;
        char      const *race;
	char	  const *multied;
        char             clan[MAX_STRING_LENGTH];
	wch   = ( d->original ) ? d->original : d->character;

	/*
	 * Check for match against restrictions.
	 * Don't use trust as that exposes trusted mortals.
	 */
	if ( d->connected != CON_PLAYING || !can_see( ch, wch) || ( IS_AFFECTED3( wch, AFF_STEALTH ) && !IS_IMMORTAL ( ch ) ) )
	    continue;

	if (   wch->level < iLevelLower
	    || wch->level > iLevelUpper
	    || ( fImmortalOnly  && wch->level <= LEVEL_DEMIGOD )
	    || ( fClassRestrict && !rgfClass[wch->class] && !rgfClass[wch->multied] )
            || ( fMulticlassRestrict && !rgfMulticlass[wch->multied] )
	    || ( fRaceRestrict  && !rgfRace[wch->race] )
	    || ( fReligionRestrict && !rgfReligion[wch->religion] )
	    || ( fNameRestrict  && str_prefix( arg1, wch->name ) ) 
	    || ( fClanRestrict  && !rgfClan[wch->clan] ) )
	    continue;

	nMatch++;

        if ( wch->level > LEVEL_DEMIGOD )
          num_of_imm++;
        
        noclass[wch->class]++;
        if( wch->class != wch->multied )
        {
	    noclass[wch->multied]++;
	}
        
	/*
	 * Figure out what to print for class.
	 */
	class = class_table[wch->class].who_name;
        multied = class_table[wch->multied].who_name;
        pReligion = get_religion_index(wch->religion);
	if ( wch->level >= 100 )
	    switch ( wch->level )
	      {
	      default: break;
	      case MAX_LEVEL:
	             if ( wch->sex == 2 )
	                class = "&W      TYRION      ";
	             else   
	                class = "&W      TYRION      ";  
	             break;
	      case L_TLD: class = "&W   TI&CM&BE&bLORD   "; break;
	      case L_KPR: class = "&W CHRO&CN&BO&bS LORD "; break;
	      case L_CON: class = "&W    C&CO&BU&bNCIL   "; break;
	      case L_OVD: class = "&W    M&CE&BN&bTAT    "; break;
	      case L_SEN: class = "&W  MAG&CI&BS&bTRATE  "; break;
	      case L_ARC: class = "&W   GU&CA&BR&bDIAN   "; break;
	      case L_DIR: class = "&W  JUD&CI&BC&bATOR   "; break;
	      case L_IMM: class = "&W   IM&CM&BO&bRTAL   "; break;
	      case L_DEI: class = "&W     D&CE&BI&bTY    "; break;
	      case L_APP: class = "&W   HO&CN&BO&bRARY   "; break;
      case LEVEL_DEMIGOD: class = "&W   DE&YM&wI&z-GOD   "; break;
	case LEVEL_CHAMP: class = "&W    A&YV&wA&zTAR    "; break;
	case LEVEL_HERO3: class = "&W   CH&YA&wM&zPION   "; break;
	case LEVEL_HERO2: class = "&W   CH&YA&wM&zPION   "; break;
	case LEVEL_HERO1: class = "&W   CH&YA&wM&zPION   "; break;
	case LEVEL_HERO:  class = "&W   CH&YA&wM&zPION   "; break;
	      }
	if( wch->level == LEVEL_DEMIGOD && wch->exp >= MAX_EXPERIENCE )
 	{
	    class = "&W    L&YE&wG&zEND    ";
	}

	/*
	 * Figure out what to print for race.
	 */
	race = race_table[wch->race].race_name;

        /* Clan Stuff */
        if (wch->clan != 0)
          {
            pClan = get_clan_index(wch->clan);
           if (pClan->pkill)
              switch ( wch->clev )
              {
               default:
                  sprintf( clan, "-<%s>-", pClan->name ); break;
               case 0:
                  sprintf( clan, "-<%s>-", pClan->name ); break;
               case 1:
                  sprintf( clan, "-<Champion of %s>-", pClan->name ); break;
               case 2:
                  sprintf( clan, "-<Centurion of %s>-", pClan->name ); break;
               case 3:
                  sprintf( clan, "-<Council of %s>-", pClan->name ); break;
               case 4:
                  sprintf( clan, "-<Leader of %s>-", pClan->name ); break;
               case 5:
                  sprintf( clan, "-<Ruler of %s>-", pClan->name ); break;
              }
           else
              switch ( wch->clev )
              {
               default:
                  sprintf( clan, "(%s)", pClan->name ); break;
               case 0:
                  sprintf( clan, "(%s)", pClan->name ); break;
               case 1:
                  sprintf( clan, "(Champion of %s)", pClan->name ); break;
               case 2:
                  sprintf( clan, "(Centurion of %s)", pClan->name ); break;
               case 3:
                  sprintf( clan, "(Council of %s)", pClan->name ); break;
               case 4:
                  sprintf( clan, "(Leader of %s)", pClan->name ); break;
               case 5:
                  sprintf( clan, "(Ruler of %s)", pClan->name ); break;
              }
 	}

 	/*
	 * Format it up.
	 */
        /*
         * Whotext by Canth, canth@xs4all.nl
	 * Modified by Tyrion
         */

	/* Xtext by Tyrion */

        sprintf( xtext, "X" );
        if( wch->mounted > 0 )
        {         
            sprintf( xtext, "M" );
        }
        else if( wch->position == POS_SLEEPING )
        { 
            sprintf( xtext, "Z" );
        }
        else if( wch->in_room->sector_type == SECT_UNDERWATER )
        {         
            sprintf( xtext, "W" );
        }         
        else if( IS_AFFECTED( wch, AFF_POISON ) )
        {
            sprintf( xtext, "P" );
        }
	else if( wch->position == POS_FIGHTING )
	{
	    sprintf( xtext, "F" );
	}
        else if( wch->position == POS_GHOST )
        {
            sprintf( xtext, "D" );
        }         
        else if (IS_SET( wch->act, PLR_SILENCE ) )
        {
            sprintf( xtext, "S" );
        }
        else if( wch->in_room->vnum == 6 )
        {
            sprintf( xtext, "H" );
        }
        else
        {
            sprintf( xtext, "G" );
        }

        sprintf( buf2, "%s%s%s%s",
            IS_SET( wch->act, PLR_KILLER        ) ? "&w(KILLER)&B " : "",
            IS_SET( wch->act, PLR_THIEF         ) ? "&w(THIEF)&B "  : "",
            IS_SET( wch->act, PLR_AFK           ) ? "&w(AFK)&B "    : "",
            IS_SET( wch->act2, PLR_RELQUEST           ) ? "&w(CRUSADE)&B "    : ""
            );
        /* No whotext */

        if ( !str_cmp ( wch->pcdata->who_text, "@" )
                || !str_cmp ( wch->pcdata->who_text, "" ) )
	{
	    if ( wch->level < 100 && wch->multied == wch->class)
            {
	        sprintf( buf + strlen( buf ), "&C[&W%2d %s %s     &W%s %s&C] %s",
	        wch->level, race, class, pReligion->name, xtext, buf2 );
            }
            if ( wch->level < 100 && wch->multied != wch->class )
            {
                sprintf( buf + strlen( buf ), "&C[&W%2d %s %s %s &W%s %s&C] %s",
	        wch->level, race, class, multied, pReligion->name, xtext, buf2 );
            }
	    if ( wch->level >= 100 )
	    {
		sprintf( buf + strlen( buf ), "&C[&W%s&W &W%s %s&C] %s",
		class, pReligion->name, xtext, buf2 );
	    }
	}
        else
        {
	    sprintf( buf + strlen( buf ), "&C[&W%14s &W%s %s&C] %s",
	    wch->pcdata->who_text, pReligion->name, xtext, buf2 );
        }
	send_to_char(AT_WHITE, buf, ch );

	if(IS_SET( wch->act, PLR_WIZINVIS ) && wch->level >= LEVEL_IMMORTAL )
        {
	    sprintf( buf, "&w(&zWiz %d&w)&B ", wch->wizinvis );
	    send_to_char(AT_WHITE, buf, ch );
	}

        if(wch->pkill) {
	   send_to_char( AT_RED, "<PK> ", ch );
        }

        if (wch->pkill_timer > 0) {
           send_to_char( AT_LBLUE, "(PROTECTED) ", ch );
        }

	if (IS_AFFECTED4( wch, AFF_NEWBIE_SLAYER))
	{
		send_to_char( AT_BLOOD, "[Newbie Slayer] ", ch);
	}

        if(wch->guild != NULL)
        {
          buf[0] = '\0';
          sprintf(buf+strlen(buf), "[%s", wch->guild->name);
          send_to_char(wch->guild->color, buf, ch);
          buf[0] = '\0';
	  switch(wch->guild_rank)
          {
	    case 0: sprintf(buf+strlen(buf), "] ");		break;
	    case 1: sprintf(buf+strlen(buf), " %s] ", wch->sex == SEX_FEMALE ?
			    "Lady" : "Lord" );	break;
	    case 2: sprintf(buf+strlen(buf), " High %s] ",
			    wch->sex == SEX_FEMALE ? "Lady" : "Lord" );	break;
	    case 3: sprintf(buf+strlen(buf), " Over%s] ",
			    wch->sex == SEX_FEMALE ? "lady" : "lord" );	break;
            default: sprintf(buf+strlen(buf), "Bug] ");		break;
          }
          send_to_char(wch->guild->color, buf, ch);
        }
	buf[0] = '\0';
	if ( wch->desc && wch->desc->editor != 0 )
	send_to_char( AT_WHITE, "&R<&CBuilding&R> ", ch );
	if ( wch->pcdata->lname )
	  sprintf( buf + strlen( buf ), "%s %s%s.",
	  wch->name, wch->pcdata->lname, wch->pcdata->title );
	else
	  sprintf( buf + strlen( buf ), "%s%s. ", 
	    wch->name, wch->pcdata->title );
        send_to_char( WC_THREE, buf, ch);
	buf[0] = '\0';
        if (wch->clan != 0)
          sprintf( buf + strlen( buf ), "%s\n\r", clan );
        else sprintf( buf, "\n\r" );
        if (wch->clan != 0)
          {
            pClan=get_clan_index(wch->clan);
            if (pClan->pkill)
              send_to_char( AT_RED, buf, ch );
	    else
	      send_to_char( AT_LBLUE, buf, ch );
	  }
	else
	  send_to_char(C_DEFAULT, buf, ch );
	buf[0] = '\0';
	if ( fNameRestrict && !str_cmp( arg1, wch->name ) )
	   break;
	}

  if ( nMatch > 0 )
  {
    if ( fNameRestrict )
      send_to_char( C_DEFAULT, "\n\r\n\r", ch );

      send_to_char( WC_ONE, "Mag", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[0] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Cle",ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[1] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Thi", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[2] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "War", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[3] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Psi", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[4] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Dru", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[5] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Rng", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[6] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Pal", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[7] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Brd", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[8] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Vam", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[9] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( C_DEFAULT, "\n\r", ch );

      send_to_char( WC_ONE, "Wlf", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[10] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Ant", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[11] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Asn", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[12] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Mon", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[13] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Bar", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[14] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Ill", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[15] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Nec", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[16] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Dmn", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[17] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Shm", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[18] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( WC_ONE, "Dkp", ch );
      send_to_char( WC_TWO, "[", ch );
      sprintf( buf, "%d", noclass[19] );
      send_to_char( WC_ONE, buf, ch );
      send_to_char( WC_TWO, "] ", ch );

      send_to_char( AT_RED, "Imms", ch );
      send_to_char( AT_BLOOD, "[", ch );
      sprintf( buf, "%d", num_of_imm );
      send_to_char( AT_RED, buf, ch );
      send_to_char( AT_BLOOD, "]\n\r", ch );

    sprintf( buf, "You see %d player%s in the game.\n\r",
	    nMatch, nMatch == 1 ? "" : "s" );
    send_to_char(WC_THREE, buf, ch );
  }
 else
  send_to_char( WC_THREE, "There is no class/race/clan/person by that name in the game.\n\r", ch );
  return;
}

void do_pcscore( CHAR_DATA *ch, char *argument )
{
    char         buf  [ MAX_STRING_LENGTH ];
    char         arg  [ MAX_INPUT_LENGTH ];

    argument = one_argument( argument, arg );

    if( arg[0] == '\0' || !strcmp( arg, "page1" ) )
    {
        /*TOP LAYER*/
        sprintf(buf, "+------------------------------------------------------------------------------\n\r" );
        send_to_char(AT_WHITE, buf, ch );
        sprintf(buf, "|Name:     " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf,"%s%s.\n\r", ch->name, IS_NPC( ch ) ? "" : ch->pcdata->title );
        send_to_char(AT_LBLUE, buf, ch );
        sprintf(buf, "|Race:     " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf(buf, "%s\n\r", race_table[ch->race].race_full );
        send_to_char(AT_LBLUE, buf, ch );
        sprintf(buf, "|Class:    " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf(buf, "%s\n\r", class_table[ch->class].who_long);
        send_to_char(AT_LBLUE, buf, ch );
        sprintf(buf, "|Multi:    " );
        send_to_char(AT_WHITE, buf, ch );
        if(ch->multied != ch->class )
        {
             sprintf(buf, "%s\n\r", class_table[ch->multied].who_long );
        }
        else
        {
             sprintf(buf, "None\n\r" );
        }
        send_to_char(AT_LBLUE, buf, ch );
        sprintf(buf, "|Level:    "  );
        send_to_char(AT_WHITE, buf, ch );
        sprintf(buf, "%d\n\r", ch->level );
        send_to_char(AT_LBLUE, buf, ch );
        sprintf(buf, "|Age:      " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf(buf, "%d years old, (%d hours)\n\r", get_age(ch), (get_age(ch)-17) * 4 );
        send_to_char(AT_LBLUE, buf, ch );
        if( ch->clan )
        {
            CLAN_DATA *clan;

            clan = get_clan_index( ch->clan );
            sprintf(buf, "|Clan:     " );
            send_to_char(AT_WHITE, buf, ch );
            sprintf(buf, "%s\n\r", clan->name );
            send_to_char(AT_LBLUE, buf, ch );
            if( ch->ctimer )
            {
                sprintf(buf, "|Timer:    " );
                send_to_char(AT_WHITE, buf, ch );
                 sprintf(buf, "%d\n\r", ch->ctimer );
                 send_to_char(AT_LBLUE, buf, ch );
            }
        }
        if(ch->guild != NULL )
        {
            sprintf(buf, "|Guild:    " );
            send_to_char(AT_WHITE, buf, ch );
            sprintf(buf, "%s", ch->guild->name);
            switch(ch->guild_rank)
            {
                 case 0: sprintf(buf+strlen(buf), " "); break;
                 case 1: sprintf(buf+strlen(buf), "  Rank: %s ", ch->sex == SEX_FEMALE ?
                    "Lady" : "Lord" ); break;
                 case 2: sprintf(buf+strlen(buf), " High %s ",
                    ch->sex == SEX_FEMALE ? "Lady" : "Lord" ); break;
                 case 3: sprintf(buf+strlen(buf), "  Rank: Over%s ",
                    ch->sex == SEX_FEMALE ? "lady" : "lord" ); break;
                 default: sprintf(buf+strlen(buf), "Bug "); break;
            }
            send_to_char(ch->guild->color, buf, ch);
            send_to_char(AT_WHITE, "\n\r", ch );
        }
        if( ch->religion )
        {
             RELIGION_DATA *religion;

             religion = get_religion_index( ch->religion );
             sprintf(buf, "|Religion: " );
             send_to_char(AT_WHITE, buf, ch );
             sprintf(buf, "%s\n\r", religion->shortdesc );
             send_to_char(AT_LBLUE, buf, ch );
        }
        sprintf(buf, "+---------------------+-----------------------+--------------------------------\n\r" );
        send_to_char(AT_WHITE, buf, ch );

        /*ATTRIBUTE ETC LAYER*/ 
        sprintf( buf, "|Strength:     " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%2d", IS_NPC(ch) ? 13: ch->pcdata->perm_str );
        send_to_char(AT_LBLUE, buf, ch );
        sprintf( buf, "(" );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%2d", IS_NPC(ch) ? 13: get_curr_str( ch ) );
        send_to_char(AT_LBLUE, buf, ch );
        sprintf( buf, ") | Gold:    " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%-10dgp", ch->gold );
        send_to_char(AT_YELLOW, buf, ch );
        sprintf( buf, " | Quest Points:      " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%10dqp\n\r", ch->questpoints );
        send_to_char(AT_LBLUE, buf, ch );

        sprintf( buf, "|Intelligence: " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%2d", IS_NPC(ch) ? 13: ch->pcdata->perm_int );
        send_to_char(AT_LBLUE, buf, ch );
        sprintf( buf, "(" );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%2d", IS_NPC(ch) ? 13: get_curr_int( ch ) );
        send_to_char(AT_LBLUE, buf, ch );
        sprintf( buf, ") | Bank:    " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%-10dgp", ch->pcdata->bankaccount );
        send_to_char(AT_YELLOW, buf, ch );
        sprintf( buf, " | Religion Points:   " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%10drp\n\r", ch->rquestpoints);
        send_to_char(AT_LBLUE, buf, ch );

        sprintf( buf, "|Wisdom:       " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%2d", IS_NPC(ch) ? 13: ch->pcdata->perm_wis );
        send_to_char(AT_LBLUE, buf, ch );
        sprintf( buf, "(" );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%2d", IS_NPC(ch) ? 13: get_curr_wis( ch ) );
        send_to_char(AT_LBLUE, buf, ch );
        sprintf( buf, ") | Shares:  " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%-10d", ch->pcdata->shares );
        send_to_char(AT_YELLOW, buf, ch );
        sprintf( buf, "   | Learns:            " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%10d\n\r", ch->pcdata->learn );
        send_to_char(AT_LBLUE, buf, ch );

        sprintf( buf, "|Dexterity:    " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%2d", IS_NPC(ch) ? 13: ch->pcdata->perm_dex );
        send_to_char(AT_LBLUE, buf, ch );
        sprintf( buf, "(" );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%2d", IS_NPC(ch) ? 13: get_curr_dex( ch ) );
        send_to_char(AT_LBLUE, buf, ch );
        sprintf( buf, ") | Storage: " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%3d&W/&Y%3d     ", ch->pcdata->storcount, (ch->level * 2) );
        send_to_char(AT_YELLOW, buf, ch);
        sprintf( buf, " | Experience Points: " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%10dxp\n\r", ch->exp );
        send_to_char(AT_LBLUE, buf, ch );
   
        sprintf( buf, "|Constitution: " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%2d", IS_NPC(ch) ? 13: ch->pcdata->perm_con );
        send_to_char(AT_LBLUE, buf, ch );
        sprintf( buf, "(" );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%2d", IS_NPC(ch) ? 13: get_curr_con( ch ) );
        send_to_char(AT_LBLUE, buf, ch );
        sprintf( buf, ") |                      " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, " | Practices:         " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%10d\n\r", ch->practice );
        send_to_char(AT_LBLUE, buf, ch );
    
        sprintf( buf, "+---------------------+--------------+--------+-------+------------------------\n\r" );
        send_to_char(AT_WHITE, buf, ch );

        /*BOTTOM LAYER*/
        sprintf( buf, "|Hit Points: " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%10d", ch->hit );
        send_to_char(AT_YELLOW, buf, ch );
        sprintf( buf, "/" );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%-10dhp", ch->max_hit );
        send_to_char(AT_YELLOW, buf, ch );
        sprintf( buf, " | AC:      " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%5d", GET_AC( ch ) );
        send_to_char(AT_CYAN, buf, ch );
        sprintf( buf, " | Player Kills:  " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%-8d\n\r", ch->pcdata->awins );
        send_to_char(AT_LBLUE, buf, ch );

        if( ( ch->class != 9 ) && ( ch->class != 11 ) )
        {
            sprintf( buf, "|Mana      : " );
            send_to_char(AT_WHITE, buf, ch );
            sprintf( buf, "%10d", ch->mana );
            send_to_char(AT_LBLUE, buf, ch );
            sprintf( buf, "/" );
            send_to_char(AT_WHITE, buf, ch );
            sprintf( buf, "%-10dma", ch->max_mana );
            send_to_char(AT_LBLUE, buf, ch );
        }
        else
        {
            sprintf( buf, "|Blood     : " );
            send_to_char(AT_WHITE, buf, ch );
            sprintf( buf, "%10d", ch->bp );
            send_to_char(AT_RED, buf, ch );
	    sprintf( buf, "/" );
	    send_to_char(AT_WHITE, buf, ch );
    	    sprintf( buf, "%-10dbp", ch->max_bp );
	    send_to_char(AT_RED, buf, ch );
	}
	sprintf( buf, " | Hitroll: " );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( buf, "%5d", GET_HITROLL( ch ) );
	send_to_char(AT_CYAN, buf, ch );
	sprintf( buf, " | Player Deaths: " );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( buf, "%-8d\n\r", ch->pcdata->alosses );
	send_to_char(AT_LBLUE, buf, ch );

	sprintf( buf, "|Movement:   " );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( buf, "%10d", ch->move );
	send_to_char(AT_GREEN, buf, ch );
	sprintf( buf, "/" );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( buf, "%-10dmv", ch->max_move );
	send_to_char(AT_GREEN, buf, ch );
	sprintf( buf, " | Damroll: " );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( buf, "%5d", GET_DAMROLL( ch ) * 2 );
	send_to_char(AT_CYAN, buf, ch );
	sprintf( buf, " | Monster Kills: " );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( buf, "%-8d\n\r", ch->pcdata->mobkills );
	send_to_char(AT_LBLUE, buf, ch );

	sprintf( buf, "|Carrying:     " );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( buf, "%5d", ch->carry_number );
	send_to_char(AT_GREEN, buf, ch );
	sprintf( buf, " of " );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( buf, "%4d items  ", can_carry_n( ch ) );
	send_to_char(AT_GREEN, buf, ch );
	sprintf( buf, " +----------------+------------------------\n\r" );
	send_to_char(AT_WHITE, buf, ch );

	sprintf( buf, "|Weight:     " );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( buf, "%7d", ch->carry_weight );
	send_to_char(AT_GREEN, buf, ch );
	sprintf( buf, " of " );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( buf, "%7d kg  ", can_carry_w( ch ) );
	send_to_char(AT_GREEN, buf, ch );
	sprintf( buf, " | Align: " );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( buf, "%5d.", ch->alignment );
	send_to_char(AT_CYAN, buf, ch );
	sprintf( buf, " You are " );
	send_to_char(AT_WHITE, buf, ch );

	if ( ch->alignment >  900 ) send_to_char( AT_BLUE, "angelic.\n\r",ch );
	else if ( ch->alignment >  700 ) send_to_char( AT_BLUE, "saintly.\n\r",ch );
	else if ( ch->alignment >  350 ) send_to_char( AT_BLUE, "good.\n\r"   ,ch );
	else if ( ch->alignment >  100 ) send_to_char( AT_BLUE, "kind.\n\r"   ,ch );
	else if ( ch->alignment > -100 ) send_to_char( AT_YELLOW, "neutral.\n\r",ch );
	else if ( ch->alignment > -350 ) send_to_char( AT_RED, "mean.\n\r"    ,ch);
	else if ( ch->alignment > -700 ) send_to_char( AT_RED, "evil.\n\r"    ,ch);
	else if ( ch->alignment > -900 ) send_to_char( AT_RED, "demonic.\n\r" ,ch);
	else                             send_to_char( AT_RED, "satanic.\n\r" ,ch);

	sprintf( buf, "+------------------------------------+-----------------------------------------\n\r" );
	send_to_char(AT_WHITE, buf, ch );

	return;
    }

    if( !strcmp( arg, "page2" ) )
    {
        /*TOP LAYER*/
        sprintf(buf, "+------------------------------------------------------------------------------\n\r" );
        send_to_char(AT_WHITE, buf, ch );
        sprintf(buf, "|Name:     " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf,"%s%s\n\r", ch->name, IS_NPC( ch ) ? "" : ch->pcdata->title );
        send_to_char(AT_LBLUE, buf, ch );
	if ( get_trust( ch ) > LEVEL_DEMIGOD )
        {
            sprintf( buf, "|Bamfin:   " );
	    send_to_char( AT_WHITE, buf, ch );
    	    sprintf( buf, "%s\n\r", ch->pcdata->bamfin );
	    send_to_char( AT_LBLUE, buf, ch );
	    sprintf( buf, "|Bamfout:  " );
            send_to_char( AT_WHITE, buf, ch );
            sprintf( buf, "%s\n\r", ch->pcdata->bamfout );
            send_to_char( AT_LBLUE, buf, ch );
	    sprintf( buf, "|Bamfsin:  " );
            send_to_char( AT_WHITE, buf, ch );
            sprintf( buf, "%s\n\r", ch->pcdata->bamfsin );
            send_to_char( AT_LBLUE, buf, ch );
	    sprintf( buf, "|Bamfsout: " );
            send_to_char( AT_WHITE, buf, ch );
            sprintf( buf, "%s\n\r", ch->pcdata->bamfsout );
            send_to_char( AT_LBLUE, buf, ch );
	    sprintf( buf, "|Wizinvis: " );
	    send_to_char( AT_WHITE, buf, ch );
	    if( IS_SET( ch->act, PLR_WIZINVIS ) )
	    {
		sprintf( buf, "ON " );
	    }
	    else
	    {
		sprintf( buf, "OFF" );
	    }
	    send_to_char( AT_LBLUE, buf, ch );
	    sprintf( buf, "  Level: " );
	    send_to_char( AT_WHITE, buf, ch );
            sprintf( buf, "%d", ch->wizinvis );
	    send_to_char( AT_LBLUE, buf, ch );
	    sprintf( buf, "  Trust: " );
	    send_to_char( AT_WHITE, buf, ch );
	    sprintf( buf, "%d", get_trust( ch ) );
	    send_to_char( AT_LBLUE, buf, ch );
	    sprintf( buf, "\n\r" );
	    send_to_char( AT_WHITE, buf, ch );
        }
        sprintf(buf, "+-----------------------------------+--------------+---------------------------\n\r");
        send_to_char(AT_WHITE, buf, ch );

	/* Section Two */
        sprintf( buf, "|Speaking: " );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( buf, "%-24s", lang_table[ch->speaking].name );
	send_to_char(AT_LBLUE, buf, ch );
	sprintf( buf, " | Thirsty: " );
	send_to_char(AT_WHITE, buf, ch );
        if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] ==  0
           && ch->level >= LEVEL_IMMORTAL && ch->position != POS_GHOST )
        {
            send_to_char( AT_RED, "Yes", ch );
        }
        else
	{
	    send_to_char( AT_GREEN, "No ", ch );
	}
	send_to_char(AT_WHITE, " | Poisoned: ", ch );
	if( IS_AFFECTED( ch, AFF_POISON ) )
	{
	    send_to_char( AT_RED, "Yes", ch );
	}
	else
	{
	    send_to_char( AT_GREEN, "No ", ch );
	}
        send_to_char(AT_WHITE, "\n\r", ch );
        sprintf( buf, "|Wimpy:    " );
        send_to_char(AT_WHITE, buf, ch );
        sprintf( buf, "%-9d", ch->wimpy );
        send_to_char(AT_LBLUE, buf, ch );   
        sprintf( buf, "                | Hungry:  " );
        send_to_char(AT_WHITE, buf, ch );
        if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] ==  0
           && ch->level >= LEVEL_IMMORTAL && ch->position != POS_GHOST )
        {
            send_to_char( AT_RED, "Yes", ch );
        }
        else
        {
            send_to_char( AT_GREEN, "No ", ch );
        }
        send_to_char(AT_WHITE, " | Level:    ", ch );
	sprintf( buf, "%5d", ch->poison_level);
        if( IS_AFFECTED( ch, AFF_POISON ) )
        {
            send_to_char( AT_RED, buf, ch );
        }
        else
        {
            send_to_char( AT_GREEN, buf, ch );
        }
        send_to_char( AT_WHITE, "\n\r", ch );
        send_to_char( AT_WHITE, "|Position: You are ", ch );
        switch ( ch->position )
        {
	    case POS_DEAD:
	        send_to_char( (AT_RED + AT_BLINK), "DEAD!!          ", ch ); break;
	    case POS_MORTAL:
	        send_to_char( AT_RED, "mortally wounded.", ch ); break;
	    case POS_INCAP:
	        send_to_char( AT_RED, "incapacitated.   ", ch ); break;
	    case POS_STUNNED:
	        send_to_char( AT_RED, "stunned.         ", ch ); break;
	    case POS_SLEEPING:
	        send_to_char( AT_CYAN, "sleeping.        ", ch ); break;
	    case POS_RESTING:
	        send_to_char( AT_CYAN, "resting.         ", ch ); break;
	    case POS_STANDING:
	        send_to_char( AT_CYAN, "standing.        ", ch ); break;
	    case POS_GHOST:
	        send_to_char( AT_CYAN, "floating ghost.  ", ch ); break;
	    case POS_FIGHTING:  
	        send_to_char( AT_BLOOD, "fighting.        ", ch ); break;
        }
	send_to_char( AT_WHITE,  "| Drunk:   ", ch );
	if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK]   > 10 )
        {
            send_to_char( AT_RED, "Yes", ch );
        }
        else
        {
            send_to_char( AT_GREEN, "No ", ch );
        }
	send_to_char( AT_WHITE, " |", ch );
        send_to_char( AT_WHITE, "\n\r", ch );
	send_to_char( AT_WHITE, "+-----------------------------------+--------------+---------------------------\n\r", ch );

	/* Middle 2 Sections */
	send_to_char( AT_WHITE, "|AC:       ", ch );
	sprintf( buf, "%-10d", GET_AC( ch ) );
	send_to_char( AT_LBLUE, buf, ch );   
        send_to_char( AT_WHITE, "  You are ", ch );
        if ( GET_AC( ch ) >=  101 )
	{
	    send_to_char( AT_GREEN, "WORSE than naked!\n\r", ch );
        }
        else if ( GET_AC( ch ) >=   20 )
 	{
	    send_to_char( AT_GREEN, "naked."           , ch );
	}
        else if ( GET_AC( ch ) >=    0 )
	{
	    send_to_char( AT_GREEN, "wearing clothes." , ch );
	}
        else if ( GET_AC( ch ) >= - 50 )
	{
	    send_to_char( AT_GREEN, "slightly armored.", ch );
	}
        else if ( GET_AC( ch ) >= -100 ) 
	{
	    send_to_char( AT_GREEN, "somewhat armored.", ch );
	}
        else if ( GET_AC( ch ) >= -250 ) 
	{
	    send_to_char( AT_GREEN, "armored."         , ch );
	}
        else if ( GET_AC( ch ) >= -500 ) 
	{
	    send_to_char( AT_GREEN, "well armored."    , ch );
	}
        else if ( GET_AC( ch ) >= -750 ) 
	{
	    send_to_char( AT_GREEN, "strongly armored.", ch );
	}
        else if ( GET_AC( ch ) >= -1000 ) 
	{
	    send_to_char( AT_GREEN, "heavily armored." , ch );
	}
        else if ( GET_AC( ch ) >= -1200 ) 
	{
	    send_to_char( AT_GREEN, "superbly armored.", ch );
	}
        else if ( GET_AC( ch ) >= -1400 ) 
	{
	    send_to_char( AT_GREEN, "divinely armored.", ch );
	}
        else
	{
	    send_to_char( AT_GREEN, "invincible!"      , ch );
	}
	send_to_char( AT_WHITE, "\n\r", ch );
	send_to_char( AT_WHITE, "+------------------------------------------------------------------------------\n\r", ch );
	send_to_char( AT_WHITE, "|Autoexit: ", ch );
	if( IS_SET( ch->act, PLR_AUTOEXIT ) )
	{
	    send_to_char( AT_GREEN, "Yes", ch );
        }
	else
	{
	    send_to_char( AT_RED, "No ", ch );
	}
        send_to_char( AT_WHITE, "         Autoloot: ", ch );
        if( IS_SET( ch->act, PLR_AUTOLOOT ) ) 
        { 
            send_to_char( AT_GREEN, "Yes", ch ); 
        } 
        else 
        { 
            send_to_char( AT_RED, "No ", ch ); 
        }
        send_to_char( AT_WHITE, "         Autosac: ", ch );
        if( IS_SET( ch->act, PLR_AUTOSAC ) ) 
        { 
            send_to_char( AT_GREEN, "Yes", ch ); 
        } 
        else 
        { 
            send_to_char( AT_RED, "No ", ch ); 
        }
        send_to_char( AT_WHITE, "         Autogold: ", ch );
        if( IS_SET( ch->act, PLR_AUTOGOLD ) ) 
        { 
            send_to_char( AT_GREEN, "Yes", ch ); 
        } 
        else 
        { 
            send_to_char( AT_RED, "No ", ch ); 
        }
	send_to_char( AT_WHITE, "\n\r", ch );

	send_to_char( AT_WHITE, "|ANSI:     ", ch );
        if( IS_SET( ch->act, PLR_ANSI ) )
        {   
            send_to_char( AT_GREEN, "Yes", ch );
        }
        else
        {
            send_to_char( AT_RED, "No ", ch );  
        }
        send_to_char( AT_WHITE, "         Combat:   ", ch );
        if( IS_SET( ch->act, PLR_COMBAT ) )
        {   
            send_to_char( AT_GREEN, "Brief", ch );
        }
        else
        {
            send_to_char( AT_RED, "Full ", ch );
        }        
        send_to_char( AT_WHITE, "       Sound:   ", ch );
        if( IS_SET( ch->act, PLR_SOUND ) )
        {  
            send_to_char( AT_GREEN, "Yes", ch );
        }   
        else
        {
            send_to_char( AT_RED, "No ", ch );
        }        
        send_to_char( AT_WHITE, "         Music:    ", ch );
        if( IS_SET( ch->act, PLR_MUSIC ) )
        {
            send_to_char( AT_GREEN, "Yes", ch );
        }
        else
        {
            send_to_char( AT_RED, "No ", ch );
        }
	send_to_char( AT_WHITE, "\n\r", ch );
	send_to_char( AT_WHITE, "+--------------------------------------+---------------------------------------\n\r", ch );

	/* Lower Part */
	if( ch->level >= 10 )
	{
	    send_to_char( AT_WHITE, "|Religious Powers:                     |\n\r", ch );
	    send_to_char( AT_WHITE, "| 10:", ch );
	    send_to_char( AT_LBLUE, "  ????????????????????? ????????  ", ch );
	    if( ch->level >= 20 )
	    {
		send_to_char( AT_WHITE, "| 20:", ch );
		send_to_char( AT_LBLUE, "  ???????????????????? ????????\n\r", ch );
 	        if( ch->level >= 30 )
	        {
		    send_to_char( AT_WHITE, "| 30:", ch );
		    send_to_char( AT_LBLUE, "  ???????????????????? ?????????  ", ch );
	            if( ch->level >= 40 )
		    {
			send_to_char( AT_WHITE, "| 40:", ch );
			send_to_char( AT_LBLUE, "  ???????????????????? ?????????\n\r", ch );
		        if( ch->level >= 50 )
		        {
			    send_to_char( AT_WHITE, "| 50:", ch );
			    send_to_char( AT_LBLUE, "  ???????????????????? ?????????  ", ch );
	        	    if( ch->level >= 60 )
			    {
				send_to_char( AT_WHITE, "| 60:", ch );
				send_to_char( AT_LBLUE, "  ???????????????????? ?????????\n\r", ch );
				if( ch->level >= 70 )
				{
				    send_to_char( AT_WHITE, "| 70:", ch );
				    send_to_char( AT_LBLUE, "  ???????????????????? ?????????  ", ch );
			   	    if( ch->level >= 80 )
				    {
					send_to_char( AT_WHITE, "| 80:", ch );
					send_to_char( AT_LBLUE, "  ???????????????????? ?????????\n\r", ch );
				        if( ch->level >= 90 )
				        {
					    send_to_char( AT_WHITE, "| 90:", ch );
					    send_to_char( AT_LBLUE, "  ???????????????????? ?????????  ", ch );
					    if( ch->level >= 100 )
					    {
						send_to_char( AT_WHITE, "|100:", ch );
						send_to_char( AT_LBLUE, "  ???????????????????? ?????????\n\r", ch );
						if( ch->level >= 105 )
						{
				        	    send_to_char( AT_WHITE, "+--------------------------------------+---------------------------------------\n\r", ch );
						    send_to_char( AT_WHITE, "|105:", ch );
						    send_to_char( AT_LBLUE, "  ???????????????????? ?????????\n\r", ch );
						    send_to_char( AT_WHITE, "+------------------------------------------------------------------------------\n\r", ch );
						}
						else
						{
						    send_to_char( AT_WHITE, "+--------------------------------------+---------------------------------------\n\r", ch );
						}
					    }
					    else
					    {
						send_to_char( AT_WHITE, "|", ch );
						send_to_char( AT_WHITE, "\n\r+--------------------------------------+---------------------------------------\n\r", ch );
					    }
					}
					else
					{
					    send_to_char( AT_WHITE, "+--------------------------------------+---------------------------------------\n\r", ch );
					}
				    }
				    else
				    {
					send_to_char( AT_WHITE, "|", ch );
					send_to_char( AT_WHITE, "\n\r+--------------------------------------+---------------------------------------\n\r", ch );
				    }
				}
				else
				{
				    send_to_char( AT_WHITE, "+--------------------------------------+---------------------------------------\n\r", ch);
				}
			    }
			    else
			    {
				send_to_char( AT_WHITE, "|", ch );
				send_to_char( AT_WHITE, "\n\r+--------------------------------------+---------------------------------------\n\r", ch);
			    }
			}
			else
			{
			    send_to_char( AT_WHITE, "+--------------------------------------+---------------------------------------\n\r", ch );
			}
		    }
		    else
		    {
			send_to_char( AT_WHITE, "|", ch );
			send_to_char( AT_WHITE, "\n\r+--------------------------------------+---------------------------------------\n\r", ch);
		    }
		}
		else
		{
		    send_to_char( AT_WHITE, "+--------------------------------------+---------------------------------------\n\r", ch );
		}
	    }
	    else
	    {
		send_to_char( AT_WHITE, "|", ch );
		send_to_char( AT_WHITE, "\n\r+--------------------------------------+---------------------------------------\n\r", ch );
	    }

	}
	return;
    }
    if( !strcmp( arg, "page3" ) )
    {
	/*TOP LAYER*/
	sprintf( buf, "+------------------------------------------------------------------------------\n\r" );
        send_to_char( AT_WHITE, buf, ch );
        sprintf(buf, "|Name:     " );
        send_to_char( AT_WHITE, buf, ch );
        sprintf( buf,"%s%s.\n\r|\n\r", ch->name, IS_NPC( ch ) ? "" : ch->pcdata->title );
	send_to_char( AT_WHITE, buf, ch );
	sprintf( buf, "| Save vs Spell: " );
	send_to_char( AT_WHITE, buf, ch );
	sprintf( buf, "%d", ch->saving_throw );
	send_to_char( AT_YELLOW, buf, ch );
        sprintf( buf, "\tAnti-Disarm: " );
        send_to_char( AT_WHITE, buf, ch);
        sprintf( buf, "%d\n\r",ch->antidisarm);
        send_to_char( AT_YELLOW, buf, ch);
	sprintf( buf, "+------------------------------------------------------------------------------\n\r" );
        send_to_char( AT_WHITE, buf, ch );
	sprintf( buf, "|\n\r|Resistances:\n\r" );
	send_to_char( AT_WHITE, buf, ch );
	sprintf( buf, "+------------------+------------------+-------------------+--------------------\n\r" );
	send_to_char( AT_WHITE, buf, ch );
	sprintf( buf, "|  Acid   [%3d]    | Holy    [%3d]    | Magic    [%3d]    | Fire      [%3d]\n\r", 
ch->damage_mods[0], ch->damage_mods[1], ch->damage_mods[2], ch->damage_mods[3] );
	send_to_char( AT_WHITE, buf, ch );
	sprintf( buf, "|  Energy [%3d]    | Wind    [%3d]    | Water    [%3d]    | Illusion  [%3d]\n\r", 
ch->damage_mods[4], ch->damage_mods[5], ch->damage_mods[6], ch->damage_mods[7] );   
	send_to_char( AT_WHITE, buf, ch );
	sprintf( buf, "|  Dispel [%3d]    | Earth   [%3d]    | Psychic  [%3d]    | Poison    [%3d]\n\r", 
ch->damage_mods[8], ch->damage_mods[9], ch->damage_mods[10], ch->damage_mods[11] );
	send_to_char( AT_WHITE, buf, ch );
	sprintf( buf, "|  Breath [%3d]    | Summon  [%3d]    | Physical [%3d]    | Explosive [%3d]\n\r", 
ch->damage_mods[12], ch->damage_mods[13], ch->damage_mods[14], ch->damage_mods[15] );
	send_to_char( AT_WHITE, buf, ch );
	sprintf( buf, "|  Song   [%3d]    | Nagarom [%3d]    | Unholy   [%3d]    | Clan      [%3d]\n\r", 
ch->damage_mods[16], ch->damage_mods[17], ch->damage_mods[18], ch->damage_mods[19] );
	send_to_char( AT_WHITE, buf, ch );
	sprintf( buf, "+------------------+------------------+-------------------+--------------------\n\r" );
	send_to_char( AT_WHITE, buf, ch );

	return;
    }

    return;
}
