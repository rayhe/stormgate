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

/*$Id: cls_pal.c,v 1.3 2005/02/26 01:07:28 tyrion Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"

char* target_name;

/* Use this file for all paladin skills */

int spell_healing_hands( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    int divisor = 1;
    int heal = 0;
  
    if ( ch == victim ) 
    {
	int mana;
  
	/* Refund mana lost by casting.  Make it seem like a new target type. */
	mana = MANA_COST( ch, sn );
	if (( ch->class == 9 )||( ch->class == 11))
	mana /= 4; 
	ch->mana += mana;
	send_to_char(AT_BLUE, "You cannot cast this spell on yourself.\n\r", ch );
	return SKPELL_MISSED;
    }
    
    if ( victim->hit >= victim->max_hit )
    {
	act(AT_BLUE, "You heal $N.", ch, NULL, victim, TO_CHAR );
	return SKPELL_NO_DAMAGE;
    }
  
    if ( IS_NEUTRAL( ch ) )
      divisor = 2;
    if ( IS_EVIL( ch ) )
      divisor = 4;
 
    heal = (victim->max_hit - victim->hit) / divisor;
  
    victim->hit += heal;
     
    act(AT_BLUE, "You heal $N.", ch, NULL, victim, TO_CHAR);
    act(AT_BLUE, "$n heals you.", ch, NULL, victim, TO_VICT);
    act(AT_BLUE, "$n heals $N.", ch, NULL, victim, TO_NOTVICT);
    ch->pkill_timer = 0;
    return SKPELL_NO_DAMAGE;   
}

int spell_wrath_of_god( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim       = (CHAR_DATA *) vo;
    int dam;
 
    if (IS_EVIL(ch) )
    {
	send_to_char(AT_WHITE, "The Gods ignore your plea.\n\r", ch);
	return SKPELL_MISSED;
    }
    if (IS_NEUTRAL(ch) )
    {
	send_to_char(AT_WHITE, "Perhaps you should be good.\n\r", ch);
	return SKPELL_MISSED;
    }
 
    dam = number_fuzzy( number_fuzzy(ch->level) * number_fuzzy(ch->level ) );

    if ( ch->hit - dam < 1 )
    {
	send_to_char( AT_WHITE, "The gods ignore your plea.\n\r", ch );
	return SKPELL_MISSED;
    }

    if (IS_NPC(victim ) )
    {
	damage( ch, victim, dam, sn );
	damage( ch, ch, dam/2, sn );

	send_to_char( AT_WHITE, "The gods answer your plea!\n\r", ch );
	return SKPELL_NO_DAMAGE;
    }
    if (!IS_NPC(victim) )
    {
	damage( ch, victim, dam*2, sn );
	damage( ch, ch, dam/2, sn );
    
	send_to_char( AT_WHITE, "The gods answer your plea!\n\r", ch );
	return SKPELL_NO_DAMAGE;
    }
    return SKPELL_NO_DAMAGE;
}
