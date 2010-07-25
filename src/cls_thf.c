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

/*$Id: cls_thf.c,v 1.5 2005/03/15 04:22:58 tyrion Exp $*/

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

int spell_thieves_cant( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;

    if ( IS_AFFECTED2( ch, AFF_SHADOW_PLANE ))
    {
      send_to_char(AT_WHITE, "You must leave the shadow realm.\n\r", ch);
      return SKPELL_MISSED;
    }
 
    af.duration  = ch->level/4;
    af.level = ch->level;
    af.location  = APPLY_DEX;
    af.modifier = ch->level/25;
    af.bitvector = AFF_THIEVESCANT;
    affect_to_char4(ch , &af);
    send_to_char(AT_ORANGE, "The knowledge of the thieves flows through you!\n\r", ch);
    return SKPELL_NO_DAMAGE;
}

int skill_double_backstab( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
    
    one_argument( target_name, arg );
        
    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Double Backstab whom?\n\r", ch );
	return SKPELL_MISSED;
    }
    
    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	return SKPELL_MISSED;
    }
  
    if ( victim == ch )   
    {
	send_to_char(C_DEFAULT, "How can you sneak up on yourself?\n\r", ch );
	return SKPELL_MISSED;
    } 
   
    if ( !( obj = get_eq_char( ch, WEAR_WIELD ) ) || obj->value[3] != 11 )
    {
	send_to_char(C_DEFAULT, "You need to wield a piercing weapon.\n\r", ch );
	return SKPELL_MISSED;
    }
    
    if ( victim->fighting )
    {
	send_to_char(C_DEFAULT, "You can't double backstab a fighting person.\n\r", ch );
	return SKPELL_MISSED;
    }
      
    if ( victim->hit < (victim->max_hit - 200) )
    {
	act(C_DEFAULT, "$N is hurt and suspicious ... you can't sneak up.", ch, NULL, victim, TO_CHAR );
	return SKPELL_MISSED;
    }
     
    if ( !is_pkillable( ch, victim ) )
    {
	return SKPELL_MISSED;
    }
     
    if ( IS_AFFECTED2( ch, AFF_SHADOW_PLANE ))
    {
	send_to_char(AT_WHITE, "You must leave the shadow realm.\n\r", ch);
	return SKPELL_MISSED;
    }

    if ( IS_AFFECTED4( ch, AFF_BURROW ))
    {
	send_to_char(AT_WHITE, "You must wait until the earth heals you.\n\r", ch);
	return SKPELL_MISSED;
    }
    
    if ( IS_AFFECTED2( victim, AFF_SHADOW_PLANE ))
    {
	send_to_char(AT_WHITE, "You can not attack someone who is in the shadow plane.\n\r", ch);
	return SKPELL_MISSED;
    }

    if ( IS_AFFECTED4( victim, AFF_BURROW ))
    {
	send_to_char(AT_WHITE, "You can not attack someone who is burrowed.\n\r", ch);
	return SKPELL_MISSED;
    }
 
    if ( IS_AFFECTED( victim, AFF_PEACE ) )
    {
	send_to_char(AT_WHITE, "A wave of peace overcomes you.\n\r", ch);
	return SKPELL_MISSED;
    }
  
    if (!IS_NPC(victim))
	ch->pkill_timer = 0;
  
    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[sn].beats );
    if ( !IS_AWAKE( victim ) || IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 ) )
    {
	multi_hit( ch, victim, sn );
	multi_hit( ch, victim, sn );
    }
    else
    {
	return SKPELL_ZERO_DAMAGE;
    }
   
    return SKPELL_NO_DAMAGE;
}

int skill_triple_backstab( int sn, int level, CHAR_DATA *ch, void *vo )
{

    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
    
    one_argument( target_name, arg );
        
    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Triple Backstab whom?\n\r", ch );
	return SKPELL_MISSED;
    }
    
    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	return SKPELL_MISSED;
    }
  
    if ( victim == ch )   
    {
	send_to_char(C_DEFAULT, "How can you sneak up on yourself?\n\r", ch );
	return SKPELL_MISSED;
    } 
   
    if ( !( obj = get_eq_char( ch, WEAR_WIELD ) ) || obj->value[3] != 11 )
    {
	send_to_char(C_DEFAULT, "You need to wield a piercing weapon.\n\r", ch );
	return SKPELL_MISSED;
    }
    
    if ( victim->fighting )
    {
	send_to_char(C_DEFAULT, "You can't triple backstab a fighting person.\n\r", ch );
	return SKPELL_MISSED;
    }
      
    if ( victim->hit < (victim->max_hit - 200) )
    {
	act(C_DEFAULT, "$N is hurt and suspicious ... you can't sneak up.", ch, NULL, victim, TO_CHAR );
	return SKPELL_MISSED;
    }
     
    if ( !is_pkillable( ch, victim ) )
    {
	return SKPELL_MISSED;
    }
     
    if ( IS_AFFECTED2( ch, AFF_SHADOW_PLANE ))
    {
	send_to_char(AT_WHITE, "You must leave the shadow realm.\n\r", ch);
	return SKPELL_MISSED;
    }

    if ( IS_AFFECTED4( ch, AFF_BURROW ))
    {
	send_to_char(AT_WHITE, "You must wait until the earth heals you.\n\r", ch);
	return SKPELL_MISSED;
    }
    
    if ( IS_AFFECTED2( victim, AFF_SHADOW_PLANE ))
    {
	send_to_char(AT_WHITE, "You can not attack someone who is in the shadow plane.\n\r", ch);
	return SKPELL_MISSED;
    }

    if ( IS_AFFECTED4( victim, AFF_BURROW ))
    {
	send_to_char(AT_WHITE, "You can not attack someone who is burrowed.\n\r", ch);
	return SKPELL_MISSED;
    }
 
    if ( IS_AFFECTED( victim, AFF_PEACE ) )
    {
	send_to_char(AT_WHITE, "A wave of peace overcomes you.\n\r", ch);
	return SKPELL_MISSED;
    }
  
    if (!IS_NPC(victim))
	ch->pkill_timer = 0;
  
    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[sn].beats );
    if ( !IS_AWAKE( victim ) || IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 ) )
    {
	multi_hit( ch, victim, sn );
	multi_hit( ch, victim, sn );
	multi_hit( ch, victim, sn );
    }
    else
    {
	return SKPELL_ZERO_DAMAGE;
    }
   
    return SKPELL_NO_DAMAGE;
}

int skill_unwavering_reflexes( int sn, int level, CHAR_DATA *ch, void *vo )
{
// dodge web/solidify/paralyze
	return SKPELL_NO_DAMAGE;
} 

