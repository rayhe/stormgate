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
 *  In order to use any part of this Envy Diku Msud, you must comply with  *
 *  the original Diku license in 'license.doc', the Merc license in        *
 *  'license.txt', as well as the Envy license in 'license.nvy'.           *
 *  In particular, you may not remove either of these copyright notices.   *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*$Id: cls_bar.c,v 1.6 2005/03/11 17:47:55 ahsile Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"

char* target_name;
extern void check_killer  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
extern void one_hit       args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );

int spell_leap( int sn, int level, CHAR_DATA *ch, void *vo )
{

    CHAR_DATA *victim = (CHAR_DATA*) vo;

    if ( !( victim = get_char_world( ch, target_name ) )
        || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
        || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  ) )
    {
        send_to_char(AT_BLUE, "You failed.\n\r", ch );
        return SKPELL_BOTCHED;
    }

    if ( ch->fighting )
    {
        send_to_char(C_DEFAULT, "Not while in combat.\n\r", ch );
        return SKPELL_BOTCHED;
    }
    if ( victim == NULL )
    {
      send_to_char(AT_GREY, "Leap to whom?\n\r", ch );
      return SKPELL_BOTCHED;
    }

    if ( victim->in_room->area != ch->in_room->area
        || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
        || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  )
	|| IS_SET( victim->in_room->room_flags, ROOM_INDOORS )
	|| IS_SET( ch->in_room->room_flags, ROOM_INDOORS )
        || IS_SET( victim->in_room->area->area_flags, AREA_PROTOTYPE )
        || (IS_AFFECTED4(victim, AFF_DECEPTION) && (number_percent() < (ch->level - 10))))
    {
        send_to_char(AT_GREY, "You are unable to Leap to that one.\n\r", ch );
        return SKPELL_BOTCHED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PRESENT)
        && (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
        || IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
	send_to_char(AT_GREY, "You are unable to Leap to that one.\n\r", ch );
        return SKPELL_BOTCHED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_FUTURE)
        && (IS_SET( victim->in_room->area->area_flags, AREA_PRESENT )
        || IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
	send_to_char(AT_GREY, "Your are unable to Leap to that one.\n\r", ch );
        return SKPELL_BOTCHED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PAST)
        && (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
        || IS_SET( victim->in_room->area->area_flags, AREA_PRESENT ) ) )
    {
	send_to_char(AT_GREY, "Your are unable to Leap to that one.\n\r", ch );
        return SKPELL_BOTCHED;
    }

    act(AT_RED, "$n bends $s muscular legs and jumps into the clouds above!", ch, NULL, NULL, TO_ROOM );
    if ( ch != victim )
    {
     char_from_room( ch );
     char_to_room( ch, victim->in_room );
    }
    act(AT_RED, "$n comes hurtling from the sky at lands with a large THUD!", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    
	return SKPELL_NO_DAMAGE;
}

int spell_leap_of_torfi( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA*) vo;

    if ( !( victim = get_char_world( ch, target_name ) )
        || IS_SET( victim->in_room->room_flags, ROOM_SAFE  )
        || IS_SET( ch->in_room->room_flags, ROOM_SAFE  ) )
    {
        send_to_char(AT_BLUE, "You failed.\n\r", ch );
        return SKPELL_BOTCHED;
    }

    spell_leap(sn, ch->level, ch, (void*) victim);

    if(ch->in_room != victim->in_room)
    {
		return SKPELL_BOTCHED;
    }


    if ( victim == ch )
    {
        send_to_char(C_DEFAULT, "How can you jump at yourself?\n\r", ch );
        return SKPELL_BOTCHED;
    }

    if ( victim->fighting )
    {
        send_to_char(C_DEFAULT, "You can't jump on someone who is fighting.\n\r", ch );
        return SKPELL_MISSED;
    }

    if ( victim->hit < (victim->max_hit/2) )
    {
        act(C_DEFAULT, "$N is hurt and suspicious ... you can't leap upon them.",
            ch, NULL, victim, TO_CHAR );
        return SKPELL_MISSED;
    }

    if ( !is_pkillable( ch, victim ) ) {
        return SKPELL_MISSED;
    }

    if ( IS_AFFECTED2( ch, AFF_SHADOW_PLANE ))
    {
      send_to_char(AT_WHITE, "You must leave the shadow realm.\n\r", ch);
      return SKPELL_BOTCHED;
    }
    if ( IS_AFFECTED4( ch, AFF_BURROW ))
    {
      send_to_char(AT_WHITE, "You must wait until the earth heals you.\n\r", ch);
      return SKPELL_BOTCHED;
    }

    if ( IS_AFFECTED2( victim, AFF_SHADOW_PLANE ))
    {
      send_to_char(AT_WHITE, "You can not attack someone who is in the shadow plane.\n\r", ch);
      return SKPELL_BOTCHED;
    }
    if ( IS_AFFECTED4( victim, AFF_BURROW ))
    {
      send_to_char(AT_WHITE, "You can not attack someone who is burrowed.\n\r", ch);
      return SKPELL_BOTCHED;
    }

    if ( IS_AFFECTED( victim, AFF_PEACE ) )
    {
      send_to_char(AT_WHITE, "A wave of peace overcomes you.\n\r", ch);
      return SKPELL_BOTCHED;
    }
    if ( !IS_NPC ( ch ) )
    {
        if (( number_percent( ) >= ((number_fuzzy(ch->pcdata->learned[sn])/20) + ((get_curr_dex( ch ) + get_curr_str( ch )/2)))) && str_cmp(ch->name,"Torfi"))
        {
	    /* Double Wait state for missing */
            send_to_char(AT_YELLOW, "You miss your target!.\n\r", ch );
	    act(AT_RED, "$n's weapon grazes your ear narrowly missing you, $e looks stunned!", ch, NULL, NULL, TO_ROOM );
	    WAIT_STATE( ch, skill_table[sn].beats );
            return SKPELL_MISSED;
        }
    }

    if (!IS_NPC(victim))
        ch->pkill_timer = 0;

    one_hit( ch, victim, sn );    

    return SKPELL_NO_DAMAGE;
}

