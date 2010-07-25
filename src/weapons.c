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

/*$Id: weapons.c,v 1.4 2005/02/22 23:55:19 ahsile Exp $*/

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

int spell_normal_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 100, 500 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_armour_piercing_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 100, 500 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    if ( IS_AFFECTED( victim, AFF_SANCTUARY ) )
       dam += dam/2;
    if ( IS_AFFECTED2( victim, AFF_GOLDEN_ARMOR ) )
       dam += dam/2;
    if ( IS_AFFECTED4( victim, AFF_GOLDEN_SANCTUARY ) )
       dam += dam/2;
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_emp_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 200, 500 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_energy_pulse_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 250, 550 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_laser_beam_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 400, 600 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_particle_beam_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 600, 1000 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_flame_thrower_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA  *obj_lose;
    OBJ_DATA  *obj_next;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam;
    int        hpch;

    if ( number_percent( ) < 2 * level && !saves_spell( level, victim ) )
    {
	for ( obj_lose = victim->carrying; obj_lose; obj_lose = obj_next )
	{
	    char *msg;

	    obj_next = obj_lose->next_content;
	    if ( obj_lose->deleted )
	        continue;
	    if ( IS_SET( obj_lose->extra_flags, ITEM_NO_DAMAGE ) )
	      continue;
	    if ( number_bits( 2 ) != 0 )
		continue;

	    switch ( obj_lose->item_type )
	    {
	    default:             continue;
	    case ITEM_CONTAINER: msg = "$p ignites and burns!";   break;
	    case ITEM_POTION:    msg = "$p bubbles and boils!";   break;
	    case ITEM_SCROLL:    msg = "$p crackles and burns!";  break;
	    case ITEM_STAFF:     msg = "$p smokes and chars!";    break;
	    case ITEM_WAND:      msg = "$p sparks and sputters!"; break;
	    case ITEM_LENSE:     msg = "$p shrivels and dries!";  break;
	    case ITEM_FOOD:      msg = "$p blackens and crisps!"; break;
	    case ITEM_GUN:	 msg = "$p melts and drips!";	  break;
	    case ITEM_PILL:      msg = "$p melts and drips!";     break;
	    }

	    act(AT_GREEN, msg, victim, obj_lose, NULL, TO_CHAR );
	    extract_obj( obj_lose );
	}
    }

    hpch = UMAX( 10, ch->hit );
    dam  = number_range( hpch / 8 + 1, hpch / 4 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_stun_gun_bullet(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *)vo;
/*  int		dam;*/

  if ( ch == victim )
  {
  	send_to_char(C_DEFAULT, "On yourself?  Are you stupid?\n\r", ch);
	return SKPELL_BOTCHED;
  }
  act( AT_BLUE, "Your stun stuns $N.", ch, NULL, victim, TO_CHAR );
  act( AT_BLUE, "$n stun stuns $N.", ch, NULL, victim, TO_NOTVICT );
  act( AT_BLUE, "$n's stun has stunned you.", ch, NULL, victim, TO_VICT );
  STUN_CHAR( victim, UMIN( level / 50, 1 * PULSE_VIOLENCE ), STUN_TOTAL );
  return SKPELL_NO_DAMAGE;
}

int spell_mortar_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 450, 750 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_nails_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
//    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 100, 300 );
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_nuclear_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 1800, 3200 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_freeze_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    char        buf[MAX_STRING_LENGTH];
    
    if ( IS_AFFECTED( victim, AFF_ANTI_FLEE ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 40 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ANTI_FLEE;
    affect_to_char( victim, &af );
    
    sprintf( buf, "%s fires and your legs are frozen!\n\r", ch->name );
    send_to_char(AT_LBLUE, buf, victim );
    act(AT_LBLUE, "$n has been immobilized by a beam of ice.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_white_light_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_BLIND ) || saves_spell( level, victim ) || IS_AFFECTED2( victim, AFF_BLINDFOLD ) )
    {
	send_to_char(AT_BLUE, "You have failed.\n\r", ch );
	return SKPELL_MISSED;
    }

    af.type      = sn;
    af.level	 = level;
    af.duration  = level / 20;
    af.location  = APPLY_HITROLL;
    af.modifier  = -50;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );

    act(AT_RED, "$N is blinded by the bright light!", ch, NULL, victim, TO_CHAR    );
    send_to_char(AT_RED, "You are blinded by a bright light!\n\r", victim );
    act(AT_RED, "$N is blinded by a bright light!", ch, NULL, victim, TO_NOTVICT );
    return SKPELL_NO_DAMAGE;
}

int spell_musket_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 50, 550 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_cannon_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 500, 900 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_crossbow_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 250, 600 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_dart_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 100, 300 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_pie_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
  //  CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 5, 25 );
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_arrow_bullet( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 250, 500 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}
