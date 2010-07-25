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

/*$Id: cls_mag.c,v 1.3 2005/03/10 14:54:58 tyrion Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"

int spell_mana_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int gain;
 
    if ( IS_AFFECTED3( victim, AFF_MANA_SHIELD ) )
	return SKPELL_MISSED;
                         
    if(victim->class == CLASS_VAMPIRE || victim->class == CLASS_ANTI_PALADIN )
    {
	send_to_char(AT_WHITE, "You do not use mana and this spell is useless to you.\n\r", victim );
	return SKPELL_MISSED;
    }
    
    gain = victim->max_mana-150;
    if( gain > victim->level * 40 )
    {
	gain = victim->level * 40;
    }                    

    if ( ch->level >= skill_table[sn].skill_level[ch->class ] )
    {
	if( ch->mana + MANA_COST( ch, sn ) < gain * 1.5 + 1 )
	{
	    send_to_char(AT_WHITE, "You do not have enough mana to form the shield.\n\r", victim );
	    return SKPELL_MISSED;
	}
    }   
    if ( ch->level >= skill_table[sn].skill_level[ch->multied ] )
    {
	if( ch->mana + MANA_COST_MULTI( ch, sn ) < gain * 1.5 + 1 )
	{
	    send_to_char(AT_WHITE, "You do not have enough mana to form the shield.\n\r", victim );
	    return SKPELL_MISSED;
        }
    }

    af.type      = sn;
    af.level     = level;   
    af.duration  = number_fuzzy( level / 2 );
    af.location  = APPLY_HIT;
    af.modifier  = gain;
    af.bitvector = AFF_MANA_SHIELD;
    affect_to_char3( victim, &af );

    af.location  = APPLY_MANA;
    af.modifier  = (int) (gain * -1.5);
    affect_to_char( victim, &af );
    
    victim->mana -= (int) (gain * 1.5);
    send_to_char(AT_WHITE, "A pearly white ball appears above your head.\n\r", victim );
    act(AT_WHITE, "A pearly white ball appears above $n's head.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}
