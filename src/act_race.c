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
/*comment*/
/*$Id: act_race.c,v 1.8 2005/02/03 15:32:58 ahsile Exp $*/
#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

void do_breathe_fire( CHAR_DATA *ch, char *argument )
{

   CHAR_DATA *victim;
   char	arg [ MAX_INPUT_LENGTH ];

    if ( IS_NPC( ch ) )
    {
	return;
    }

    if ( ch->race != 13 )
    {
	send_to_char(AT_BLUE, "You are not a dragon.\n\r", ch );
	return;
    }

    if ( ch->level < 30 )
    {
	send_to_char(AT_BLUE, "You must be level 30 to use your breath attack.\n\r", ch);
	return;
    }

    one_argument( argument, arg) ;
    
    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Breathe fire on whom?\n\r", ch);
	return;
    }

    if ( !(victim = get_char_room( ch, arg ) ) )
    {
	send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char(C_DEFAULT, "Breathe on yourself?\n\r", ch );
	return;
    }

    if ( !is_pkillable( ch, victim ) ) {
	return;
    }

    if ( IS_AFFECTED2( ch, AFF_SHADOW_PLANE ) )
    {
	send_to_char(AT_WHITE, "You must leave the shadow realm.\n\r", ch);
	return;
    }
    if ( IS_AFFECTED4( ch, AFF_BURROW ) )
    {
	send_to_char(AT_WHITE, "You must wait until the earth releases you!\n\r", ch);
	return;
    }

    if ( IS_AFFECTED2( victim, AFF_SHADOW_PLANE ))
    {
	send_to_char(AT_WHITE, "You can not attack someone who is in the shadow plane.\n\r", ch);
	return;
    }
    if ( IS_AFFECTED4( victim, AFF_BURROW ) )
    {
	send_to_char(AT_WHITE, "You cannot attack someone who is burrowed!\n\r", ch);
	return;
    }

    if ( IS_AFFECTED( victim, AFF_PEACE ) )
    {
	send_to_char(AT_WHITE, "A wave of peace overcomes you.\n\r", ch);
	return;
    }

    if (!IS_NPC(victim))
	    ch->pkill_timer = 0;

    WAIT_STATE( ch, skill_table[skill_lookup("breathe fire")].beats );


    damage( ch, victim, number_range( ch->level * 3, ch->level * 9 ), skill_lookup("breathe fire") );
    return;
}

void do_race_fly( CHAR_DATA *ch, char *argument ) /* void do_fly */
{
   AFFECT_DATA af;
   char	arg [ MAX_INPUT_LENGTH ];

    if ( IS_NPC( ch ) )
    {
	return;
    }

    if ( ch->race != 13 && ch->race != 10 && ch->race != 15 && ch->race != 16
      && ch->race != 20 && ch->race != 21 && ch->race != 22 && ch->race != 23 
      && ch->race != 24 && ch->race != 25 ) // Extra race checks - ahsile 
    {
	send_to_char(AT_BLUE, "You can not fly.\n\r", ch );
	return;
    }

    if ( ch->level < 5 )
    {
	send_to_char(AT_BLUE, "You must be level 5 to fly.\n\r", ch);
	return;
    }

    one_argument( argument, arg) ;
    

    if ( IS_AFFECTED( ch, AFF_FLYING ) )
    {
	send_to_char(AT_WHITE, "You are already flying.\n\r", ch);
	return;
    }

    WAIT_STATE( ch, skill_table[skill_lookup("race fly")].beats );

    af.type	= skill_lookup("race fly");
    af.duration = ch->level;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( ch, &af );

    send_to_char(AT_BLUE, "Your feet rise off the ground.\n\r", ch);
    act(AT_BLUE, "$n's feet rise off the ground.", ch, NULL, NULL, TO_ROOM );

    return;
}

