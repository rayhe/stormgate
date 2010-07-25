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

/*$Id: cls_rng.c,v 1.50 2005/03/23 14:47:01 ahsile Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "merc.h"
char* target_name;

int spell_force_of_nature( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED4( victim, AFF_FORCE_OF_NATURE ) )
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
        
    af.type      = sn;
    af.level     = level;
    af.duration  = -1;
    af.location  = APPLY_HITROLL;
    if( !IS_NPC( ch ) )
    {
	af.modifier = ( ( ch->level + ch->pcdata->learned[sn] / 10 ) ) * 2 / 3;
    }
    else
    {
	af.modifier = ( ch->level + 100 ) * 2 / 3;
    }
    af.bitvector = AFF_FORCE_OF_NATURE;
    affect_to_char4( victim, &af );

    af.location  = APPLY_DAMROLL;
    if( !IS_NPC( ch ) )
    {
	af.modifier = ( ch->level + ( ch->pcdata->learned[sn] / 10 )  ) * 2 / 3;
    }
    else
    {
	af.modifier = ( ch->level + 100 ) * 2 / 3;
    }
    affect_to_char4( victim, &af );

    af.location  = APPLY_STR;
    af.modifier  =  ch->level / 10;
    affect_to_char4( victim, &af );

    af.location  = APPLY_DEX;
    af.modifier  =  ch->level / 10;
    affect_to_char4( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    if( !IS_NPC( ch ) )
    {
	af.modifier = ( ch->level + ( ch->pcdata->learned[sn] / 10 )  ) / 3 * -1;
    }
    else
    {
	af.modifier = ( ch->level + 100 ) * 2 / 3 * -1;
    }
    affect_to_char4( victim, &af );

    victim->shields += 1;
    
    send_to_char(AT_GREEN, "Your soul is fused with a force of nature.\n\r", victim );
    act(AT_GREEN, "$n's soul is fused with a force of nature.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}   

int spell_forestwalk( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;   

    if ( victim->position == POS_FIGHTING )
	return SKPELL_BOTCHED;

    if ( is_affected( victim, sn ) )
    {
	affect_strip(victim, sn);
	if(skill_table[sn].msg_off)
	{
	    send_to_char(C_DEFAULT, skill_table[sn].msg_off, victim );
	    send_to_char(C_DEFAULT, "\n\r", victim );
	}
        act(C_DEFAULT, skill_table[sn].msg_off_room, victim, NULL, NULL, TO_NOTVICT);
        return SKPELL_NO_DAMAGE;
    }
    
    af.type      = sn;
    af.level     = level;
    af.duration  = -1;
    af.location  = APPLY_NONE;
    af.modifier  = level;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    if ( ch != victim )
        send_to_char(AT_DGREEN, "You call forth the power of the forests.\n\r", ch );
    send_to_char(AT_DGREEN, "The power of the forests rests upon you.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int spell_mountainwalk( int sn, int level, CHAR_DATA *ch, void *vo )         
{       
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING )
	return SKPELL_BOTCHED;

    if ( is_affected( victim, sn ) )
    {
	affect_strip(victim, sn);
	if(skill_table[sn].msg_off)
	{
	    send_to_char(C_DEFAULT, skill_table[sn].msg_off, victim );
	    send_to_char(C_DEFAULT, "\n\r", victim );
	}
        act(C_DEFAULT, skill_table[sn].msg_off_room, victim, NULL, NULL, TO_NOTVICT);
        return SKPELL_NO_DAMAGE;
    }
    
    af.type      = sn;
    af.level     = level;
    af.duration  = -1;        
    af.location  = APPLY_NONE;
    af.modifier  = level;    
    af.bitvector = 0;
    affect_to_char( victim, &af );
     
    if ( ch != victim )
        send_to_char(AT_ORANGE, "You call forth the power of the mountains and hills.\n\r", ch );                
    send_to_char(AT_ORANGE, "The power of the mountains and hills rests upon you.\n\r", victim );                    
    return SKPELL_NO_DAMAGE;
}

int spell_plainswalk( int sn, int level, CHAR_DATA *ch, void *vo )         
{       
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING )
	return SKPELL_BOTCHED;

    if ( is_affected( victim, sn ) )
    {
	affect_strip(victim, sn);
	if(skill_table[sn].msg_off)
	{
	    send_to_char(C_DEFAULT, skill_table[sn].msg_off, victim );
	    send_to_char(C_DEFAULT, "\n\r", victim );
	}
        act(C_DEFAULT, skill_table[sn].msg_off_room, victim, NULL, NULL, TO_NOTVICT);
        return SKPELL_NO_DAMAGE;
    }
    
    af.type      = sn;
    af.level     = level;
    af.duration  = -1;        
    af.location  = APPLY_NONE;
    af.modifier  = level;    
    af.bitvector = 0;
    affect_to_char( victim, &af );
     
    if ( ch != victim )
        send_to_char(AT_YELLOW, "You call forth the power of the plains and deserts.\n\r", ch );                
    send_to_char(AT_YELLOW, "The power of the plains and deserts rests upon you.\n\r", victim );                    
    return SKPELL_NO_DAMAGE;
}

int spell_swampwalk( int sn, int level, CHAR_DATA *ch, void *vo )         
{       
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING )
	return SKPELL_BOTCHED;

    if ( is_affected( victim, sn ) )
    {
	affect_strip(victim, sn);
	if(skill_table[sn].msg_off)
	{
	    send_to_char(C_DEFAULT, skill_table[sn].msg_off, victim );
	    send_to_char(C_DEFAULT, "\n\r", victim );
	}
        act(C_DEFAULT, skill_table[sn].msg_off_room, victim, NULL, NULL, TO_NOTVICT);
        return SKPELL_NO_DAMAGE;
    }
    
    af.type      = sn;
    af.level     = level;
    af.duration  = -1;        
    af.location  = APPLY_NONE;
    af.modifier  = level;    
    af.bitvector = 0;
    affect_to_char( victim, &af );
     
    if ( ch != victim )
        send_to_char(AT_PINK, "You call forth the power of the swamps.\n\r", ch );                
    send_to_char(AT_PINK, "The power of the swamps rests upon you.\n\r", victim );                    
    return SKPELL_NO_DAMAGE;
}

int spell_thunder_strike( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
          0,   0,   0,   0,  30,         35,  40,  45,  50,  55,
         60,  65,  70,  75,  80,         82,  85,  90,  95, 100,
        105, 110, 115, 120, 125,        130, 135, 140, 145, 150,
        155, 160, 165, 170, 175,        180, 185, 190, 195, 200,
        205, 210, 215, 220, 225,        230, 235, 240, 245, 250,
        252, 254, 256, 258, 260,        262, 264, 266, 268, 270,
        272, 274, 276, 278, 280,        282, 284, 286, 288, 290,
        292, 394, 396, 398, 300,        302, 304, 306, 308, 310,
        315, 320, 325, 330, 335,        340, 345, 350, 355, 360
    };
    int        dam;
     
    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam      = number_range( dam_each[level] / 2, dam_each[level] * 5 );
    if ( IS_OUTSIDE( ch ) )   
    {
		dam = (int) (dam * 1.5);
        if ( weather_info.sky < SKY_RAINING )
        {
            dam /= 2;
        }
        if ( weather_info.sky > SKY_RAINING )
        {
            send_to_char(AT_WHITE, "The bad weather increases the spells power!\n\r", ch );
            dam *= 2;
        }
    }
    
    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int  spell_lightning_storm( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    int        dam;

    send_to_char(AT_BLUE, "You call forth a thunderous storm!\n\r", ch );
    act(AT_BLUE, "$n calls forth a thunderous storm.", ch, NULL, NULL, TO_ROOM );
        
    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE))
    {
	send_to_char(AT_WHITE, "You are in a safe room!", ch);
	return SKPELL_MISSED;
    }
        
    for ( vch = char_list; vch; vch = vch->next )
    {
	if ( vch->deleted || !vch->in_room || vch == ch )
	    continue;

	dam = number_range( ch->level*2, ch->level * 7 );
	if ( vch->in_room == ch->in_room )
	{
	    if ( IS_OUTSIDE( ch ) )
	    {
		dam = (int) (dam * 1.5);
		if ( weather_info.sky < SKY_RAINING )
		{
		    dam /= 2;
		}
		if ( weather_info.sky > SKY_RAINING )
		{
		    send_to_char(AT_WHITE, "The bad weather increases the spells power!\n\r", ch );
		    dam *= 2;
		}
	    }
	    if ( saves_spell( level, vch ) )
		dam /= 2;

	    damage( ch, vch, dam, sn );
	    continue;
	}
        
	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char(AT_ORANGE, "The earth trembles and shivers.\n\r", vch );
    }
        
    return SKPELL_NO_DAMAGE;
}    

int spell_friend_of_nature( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED( victim, AFF_PEACE) )
        return SKPELL_MISSED;
        
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 5 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PEACE;
    affect_to_char( victim, &af );
    
    send_to_char(AT_BLUE, "You the protective forces of nature come over you.\n\r", victim );
    act(AT_BLUE, "$n looks like he is one with nature.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_refreshing_rain( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    
    send_to_char(AT_BLUE, "You enchant the heavens to rain a healing water.\n\r", ch );
    act(AT_ORANGE, "$n causes the heavens to rain a healing rain.", ch, NULL, NULL, TO_ROOM );
    
    for ( vch = char_list; vch; vch = vch->next )
    {
        if ( vch->deleted || !vch->in_room )
            continue;
        if ( vch->in_room == ch->in_room )
        {
	    if( IS_OUTSIDE( ch ) )
	    {
		spell_heal( skill_lookup("heal"), ch->level + 50, ch, vch );
	    }
	    else
	    {
		spell_heal( skill_lookup("heal"), ch->level / 2, ch, vch );
	    }
            continue;
        }
    }
  
    return SKPELL_NO_DAMAGE;
}

int spell_storm_seeker(int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int loss;

    if ( is_affected( victim, sn ))
    {
	send_to_char(AT_RED, "A curse has already been inflicted.\n\r", ch );
	return SKPELL_MISSED;
    }
        
    if( IS_OUTSIDE( ch ) )
    {
	loss = number_fuzzy( ch->level * 2 / 3 );
    }
    else
    {
	loss = number_fuzzy( ch->level * 2 / 4 );
    }
    
    af.type       = sn;
    af.level      = level;
    af.duration   = level / 6;
    af.location   = APPLY_SAVING_SPELL;
    af.modifier   = loss;
    af.bitvector  = AFF_CURSE;
    affect_to_char( victim, &af );
  
    af.location  = APPLY_DAM_WIND;
    af.modifier  = 0 - loss;
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );
     
    if(ch != victim)
	send_to_char(AT_GREEN, "Ok.\n\r", ch );

    send_to_char(AT_GREEN, "A lightning bolt strikes you square in the chest.\n\r", victim);
    spell_lightning_bolt( skill_lookup("lightning bolt"), ch->level, ch, victim );

    return SKPELL_NO_DAMAGE;
}

int spell_essence_of_gaia( int sn, int level, CHAR_DATA *ch, void *vo )
{   
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED4( victim, AFF_ESSENCE_OF_GAIA ) )
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
            
    if ( IS_AFFECTED2( victim, AFF_GOLDEN_ARMOR ) || IS_AFFECTED4( victim, AFF_GOLDEN_SANCTUARY ) || IS_AFFECTED4( victim, AFF_BIOFEEDBACK ) || IS_AFFECTED( victim, AFF_SANCTUARY ) )
        return SKPELL_MISSED;
            
    if ( !IS_SHIELDABLE( victim ) )
        return SKPELL_MISSED;
            
    af.type      = sn;
    af.level     = level;
    af.duration  = -1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ESSENCE_OF_GAIA;
    affect_to_char4( victim, &af );
    victim->shields += 1;
 
    send_to_char(AT_WHITE, "You are surrounded by Gaia's essence.\n\r", victim );
    act(AT_WHITE, "$n is surrounded by Gaia's essence.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int skill_lightning_arrow( int sn, int level, CHAR_DATA *ch, void *vo )
{
    ROOM_INDEX_DATA* in_room = NULL;
    ROOM_INDEX_DATA* to_room = NULL;
    OBJ_DATA*  bow;
    OBJ_DATA*  ammo;
    CHAR_DATA* victim	= NULL;
    int        distance = 0;
    int        target	= 0;
    int	       dam		= 0;
    char       buf[MAX_STRING_LENGTH];

    /* check conditions */
    if (!IS_NPC(ch) &&  number_percent() > ( ch->pcdata->learned[sn] / 10 ) )
    {
	send_to_char(C_DEFAULT, "You failed.\n\r", ch);
	return SKPELL_MISSED;
    }
 
    if ( ! (bow 	   = get_eq_char(ch, WEAR_WIELD)	) 
	|| (bow->value[3] != flag_value (weapon_flags, "shot") )
	|| (bow->value[0] != flag_value (ammo_flags, "arrow") 	) )
    {
	send_to_char(AT_WHITE, "You need a BOW and arrows to shoot!\n\r", ch);
	return SKPELL_MISSED;
    } else if (!(ammo = find_arrows( ch ) ) )
    {
	send_to_char(AT_WHITE, "You need a bow and ARROWS to shoot!\n\r", ch);
	return SKPELL_MISSED;
    }

    /* calculate damage */
    dam = 100;

    if ( (target = find_missle_target(sn, ch, (char*) target_name, &victim, &to_room, &distance) ) )
    {
	ammo->value[0]--;
	act( AT_BLUE, "$n draws $s weapon and fires a lightning arrow!", ch, NULL, NULL, TO_ROOM );

	if (!ammo->value[0])
		ammo->deleted = TRUE;

	if (target == SHOOT_HERE)
	{
		if (ch->fighting == victim)
			return dam;
		else
		{
			damage( ch, victim, dam, sn);
			multi_hit( victim, ch, TYPE_UNDEFINED  ); /* start a fight */
		}
	}

	in_room = ch->in_room;
	if (in_room != to_room)
	{
		char_from_room( ch );
		char_to_room( ch, to_room );
	}

	if ( target == SHOOT_DOOR )
	{
		sprintf(buf, "Your lightning arrow flies %d rooms away and strikes something solid.", distance);
		act( AT_BLUE, buf, ch, NULL, NULL, TO_CHAR );
		act( AT_BLUE, "A lightning arrow flies into the area and strikes something solid.", ch, NULL, NULL, TO_ROOM );
	}
	else if (target == SHOOT_DIST )
	{
		sprintf(buf, "Your lightning arrow flies %d rooms away and falls harmlessly to the ground.\n\r", distance);
		send_to_char(AT_BLUE, buf, ch);
	} 
	else if (target == SHOOT_FOUND )
	{
		act( AT_BLUE, "A lightning arrow flies into the area and hits $N!", ch, NULL, victim, TO_NOTVICT );
		sprintf(buf, "Your lightning arrow flies %d rooms away and hits $N!", distance);
		act( AT_BLUE, buf, ch, NULL, victim, TO_CHAR);
		act( AT_BLUE, "$n's lightning arrow flies into the area and hits YOU!" , ch, NULL, victim, TO_VICT ); 
		damage( ch, victim, (int) ( dam / (distance + 1) ), sn );
	}

	char_from_room( ch );
	char_to_room( ch, in_room );
    } 

    return SKPELL_NO_DAMAGE;
}
