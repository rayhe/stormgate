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

/*$Id: cls_asn.c,v 1.9 2005/03/17 02:41:08 tyrion Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"

extern void check_killer  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
extern char* target_name;

int skill_death_strike( int sn, int level, CHAR_DATA *ch, void *vo )
{
    int dmg = 0;
    CHAR_DATA *victim;
   
    if ( ch->fighting )
    {
	victim = ch->fighting;

	/* While fighting attack */
	if ( ( victim->hit * 100 / victim->max_hit <= 15 ) && ( number_percent( ) <= 15 + get_curr_dex( ch ) - get_curr_dex( victim ) ) )
	{
	    //damage( ch, victim, number_range( ch->level*100, ch->level*120 ), gsn_death_strike );
		dmg = number_range(ch->level * 100, ch->level * 120 );
	    WAIT_STATE( ch, skill_table[sn].beats );
	    WAIT_STATE( ch, skill_table[sn].beats );
	}
	else
	{
	    //damage( ch, victim, number_range( ch->level*10, ch->level*12 ), gsn_death_strike );
		dmg = number_range(ch->level * 10, ch->level*12);
	    WAIT_STATE( ch, skill_table[sn].beats );
	}
	return dmg;
    }

    if ( target_name[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Deathstrike whom?\n\r", ch );
	return SKPELL_BOTCHED;
    }

    if ( !( victim = get_char_room( ch, target_name ) ) )
    {
	send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	return SKPELL_BOTCHED;
    }

    if ( victim == ch )
    {
	send_to_char(C_DEFAULT, "You can't do that to yourself.\n\r", ch );
	return SKPELL_BOTCHED;
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

    if( IS_NPC( ch ) )
    {
	send_to_char(AT_WHITE, "You can not perform that skill.\n\r", ch );
	return SKPELL_MISSED;
    }

    if (!IS_NPC(victim))
		ch->pkill_timer = 0;

    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[sn].beats );
    
    if ( ( victim->hit * 100 / victim->max_hit <= 15 ) && ( number_percent( ) <= 15 + get_curr_dex( ch ) - get_curr_dex( victim ) ) )
    {
	/* Death Strike, Double Wait State */
	//damage( ch, victim, number_range( ch->level*200, ch->level*240 ), gsn_death_strike );
	dmg = number_range(ch->level * 200, ch->level*240);
	WAIT_STATE( ch, skill_table[sn].beats );
    }
    else
    {
	/* Damage attack */
	//damage( ch, victim, number_range( ch->level*10, ch->level*12 ), gsn_death_strike );
	dmg = number_range(ch->level * 10, ch->level*12);
    }

    return dmg;

}

int spell_deception_of_aura( int sn, int level, CHAR_DATA *ch, void *vo )
{
    if ( is_affected(ch,sn) )
    {
        affect_strip( ch, sn );
        REMOVE_BIT(ch->affected_by4, AFF_DECEPTION);
        send_to_char(AT_RED, "You drop your mental defenses.\n\r", ch);
        return SKPELL_NO_DAMAGE;
    }
   
    if (number_percent() <= ((number_fuzzy(ch->pcdata->learned[sn])/20) + get_curr_int( ch )))
    {
		AFFECT_DATA af;
		int level = ch->level;

    	af.type      = sn;
    	af.level     = level;
        if (!str_cmp(ch->name,"Aura"))
        {
           af.duration = -1;
    	   af.location  = APPLY_DEX;	
     	   af.modifier = 6;
        } else {
    	   af.duration  = level/4;
    	   af.location  = APPLY_NONE;	
  	   af.modifier = 0;
        }
  	af.bitvector = AFF_DECEPTION;
	affect_to_char4(ch , &af);
        send_to_char(AT_WHITE,"You succeed in imagining that you are somewhere else.\n\r",ch);
    } else
    {
        send_to_char(AT_WHITE,"You are unable to partition your mind!\n\r",ch);
    }

    WAIT_STATE( ch, skill_table[sn].beats);
    return SKPELL_NO_DAMAGE;
}
