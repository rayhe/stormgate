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

/*$Id: cls_ant.c,v 1.1 2005/04/10 16:29:00 tyrion Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"

int spell_unholy_strength( int sn, int level, CHAR_DATA  *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_UNHOLY_STRENGTH ) )
    {
        affect_strip(victim, sn);
        if(skill_table[sn].msg_off)
        {
            send_to_char(C_DEFAULT, skill_table[sn].msg_off, victim );
            send_to_char(C_DEFAULT, "\n\r", victim );
        }
        act(C_DEFAULT, skill_table[sn].msg_off_room, victim, NULL, NULL, TO_NOTVICT);
        victim->shields -= 1;
        return SKPELL_NO_DAMAGE;
    }
    
    if ( !IS_SHIELDABLE( victim ) )
        return SKPELL_MISSED;
    
    af.type     = sn;
    af.level    = level;
    af.duration = -1;
    af.location = APPLY_STR;
    af.modifier = number_fuzzy( level / 12 );
    af.bitvector = AFF_UNHOLY_STRENGTH;
    affect_to_char2( victim, &af );
    victim->shields += 1;
    
    send_to_char(AT_DGREY, "Your body is fused with the strength of the unholy.\n\r", victim );
    act(AT_DGREY, "$n's body is fused with the strength of the unholy.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}
