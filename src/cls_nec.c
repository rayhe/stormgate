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

/*$Id: cls_nec.c,v 1.5 2005/02/22 23:55:15 ahsile Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"

int spell_niraks_curse_of_the_damned( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;   

    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
        return SKPELL_MISSED;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy(level / 5);
    af.location  = APPLY_DAMROLL;
    af.modifier  = level / 3 * -1;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_HITROLL;
    af.modifier  = level / 3 * -1;
    affect_to_char( victim, &af );
 
    af.location  = APPLY_STR;
    af.modifier  = level / 10 * -1;
    affect_to_char( victim, &af );

    af.location  = APPLY_DEX;
    af.modifier  = level / 10 * -1;
    affect_to_char( victim, &af );
  
    if ( ch != victim )
	send_to_char(AT_BLUE, "You call forth Nirak's Curse of the Damned.\n\r", ch );
    send_to_char(AT_BLUE, "Nirak's Curse of the Damned infects you.\n\r", victim );

    return SKPELL_NO_DAMAGE;
}
