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

/*$Id: cls_vam.c,v 1.4 2006/05/31 18:07:14 ahsile Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"

int spell_tomba_di_vemon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;

    if ( !IS_NPC( ch )
        && ( ( ch->level < skill_table[sn].skill_level[ch->class] )
        && (ch->level < skill_table[sn].skill_level[ch->multied])))
    {
        send_to_char(AT_GREY,
            "You know nothing Burrowing.\n\r", ch );
        return SKPELL_BOTCHED;
    }

    if ( ch->fighting )
    {
        send_to_char(C_DEFAULT, "Not while in combat.\n\r", ch );
        return SKPELL_BOTCHED;
    }
    if ( IS_SET(ch->in_room->room_flags, ROOM_INDOORS) )
    {
        send_to_char(C_DEFAULT, "Not inside!\n\r", ch);
        return SKPELL_BOTCHED;
    }
    if (ch->hit >= ch->max_hit)
    {
	send_to_char(C_DEFAULT, "You are not injured!\n\r", ch);
        return SKPELL_BOTCHED;
    }
    if (ch->combat_timer > 0)
    {
	send_to_char(C_DEFAULT, "You must calm down before the earth will accept you!\n\r",ch);
	return SKPELL_BOTCHED;
    }
    
    af.type = sn;
    af.duration  = -1;
    af.level = ch->level;
    af.location  = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_BURROW;
    affect_to_char4(ch , &af);
    send_to_char(AT_ORANGE, "You quickly burrow underground!\n\r", ch);
    act(AT_ORANGE, "$n quickly burrows underground!", ch, NULL, NULL, TO_ROOM );

    return SKPELL_NO_DAMAGE;
}

