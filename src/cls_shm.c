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

/*$Id: cls_shm.c,v 1.17 2005/04/04 13:49:13 ahsile Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"

extern char* target_name;

int skill_ethereal_wolf( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;
    char arg [ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
   
/*
    if ( !IS_NPC( ch )
        && ( ( ch->level < skill_table[sn].skill_level[ch->class] )
        && (ch->level < skill_table[sn].skill_level[ch->multied])))
    {
        send_to_char(C_DEFAULT, "You can not perform that skill.\n\r", ch );
        return SKPELL_MISSED;
    }
*/

    // modified check so if NPCs have spell on already
    // they can use bite/howl/etc - Ahsile
    if ( IS_NPC(ch) || number_percent( ) > ( ch->pcdata->learned[sn] / 10 )  )
    {
	if (IS_NPC(ch))
	{
		if (!IS_AFFECTED4(ch, AFF_ETHEREAL_WOLF))
		{	
			send_to_char( C_DEFAULT, "You failed.\n\r", ch );
			return SKPELL_MISSED;
		}
	} else
	{
			send_to_char( C_DEFAULT, "You failed.\n\r", ch );
			return SKPELL_MISSED;
	}
    }
       
    if ( !ch->fighting )
    {
	if ( IS_AFFECTED4( ch, AFF_ETHEREAL_WOLF ) )
	{
	    send_to_char( AT_DGREY, "You replace your ethereal wolf with a new one.\n\r", ch );
	    return SKPELL_NO_DAMAGE;
	}

	/* TO DO: Mana costs */
   
	af.type = sn;
	af.level = ch->level;
	af.duration = ch->level;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_ETHEREAL_WOLF;
	affect_to_char4(ch, &af);
	act( AT_DGREY, "$n summons an ethereal wolf!", ch, NULL, NULL, TO_ROOM );
	send_to_char( AT_DGREY, "You have summoned an ethereal wolf to assist you.\n\r", ch );

	WAIT_STATE( ch, skill_table[sn].beats );

	return SKPELL_NO_DAMAGE;
    }

    /* Wolf commands will go here */

    victim = ch->fighting;

    if (target_name[0] == '\0')
    {
	send_to_char( AT_DGREY, "You get the attention of your wolf.\n\r", ch );
	act( AT_DGREY, "$n gets the attention of $m wolf.", ch, NULL, NULL, TO_ROOM );
	return SKPELL_NO_DAMAGE;
    }

    if ( !str_cmp( target_name, "bite" ) )
    {
	send_to_char( AT_DGREY, "You order your wolf to bite!\n\r", ch );
	act( AT_DGREY, "$n orders $m ethereal wolf to bite!", ch, NULL, NULL, TO_ROOM );
	WAIT_STATE( ch, skill_table[skill_lookup("ethereal wolf bite")].beats );
	return spell_ethereal_wolf_bite ( skill_lookup("ethereal wolf bite"), ch->level, ch, victim );
	
    }

    if ( !str_cmp( arg, "claw" ) )
    {
	send_to_char( AT_DGREY, "You order your wolf to claw!\n\r", ch );
	act( AT_DGREY, "$n orders $m ethereal wolf to claw!", ch, NULL, NULL, TO_ROOM );
	WAIT_STATE( ch, skill_table[skill_lookup("ethereal wolf claw")].beats );
	return spell_ethereal_wolf_claw ( skill_lookup("ethereal wolf claw"), ch->level, ch, victim );
    }

    if ( !str_cmp( arg, "howl" ) )
    {
	send_to_char( AT_DGREY, "You order your wolf to howl!\n\r", ch );
	act( AT_DGREY, "$n orders $m ethereal wolf to howl!", ch, NULL, NULL, TO_ROOM );
	spell_ethereal_wolf_howl ( skill_lookup("ethereal wolf howl"), ch->level, ch, victim );
    
	WAIT_STATE( ch, skill_table[skill_lookup("ethereal wolf howl")].beats / 2);
	send_to_char( AT_DGREY, "You are terrified!\n\r", victim );
	if( !saves_spell( ch->level, victim ) )
	{
	    WAIT_STATE( victim, skill_table[skill_lookup("ethereal wolf howl")].beats );
	}
    
	return SKPELL_NO_DAMAGE;
    }

    send_to_char ( AT_DGREY, "You may only order your wolf to BITE, CLAW or HOWL.\n\r", ch );

    return SKPELL_BOTCHED;

}

int skill_ethereal_snake( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;
    CHAR_DATA *victim;
   
/*
    if ( !IS_NPC( ch )
        && ( ( ch->level < skill_table[sn].skill_level[ch->class] )
        && (ch->level < skill_table[sn].skill_level[ch->multied])))
    {
        send_to_char(C_DEFAULT, "You can not perform that skill.\n\r", ch );
        return SKPELL_MISSED;
    }
*/

    // modified check so if NPCs have spell on already
    // they can use bite/devour/etc - Ahsile
    if ( IS_NPC(ch) || number_percent( ) > ( ch->pcdata->learned[sn] / 10 ) )
    {
	if (IS_NPC(ch))
	{
		if (!IS_AFFECTED4(ch, AFF_ETHEREAL_SNAKE))
		{	
			send_to_char( C_DEFAULT, "You failed.\n\r", ch );
			return SKPELL_MISSED;
		}
	} else
	{
			send_to_char( C_DEFAULT, "You failed.\n\r", ch );
			return SKPELL_MISSED;
	}
    }
   
    if ( !ch->fighting )
    {
	if ( IS_AFFECTED4( ch, AFF_ETHEREAL_SNAKE ) )
	{
	    send_to_char( AT_DGREY, "You replace your ethereal snake with a new one.\n\r", ch );
	    return SKPELL_NO_DAMAGE;
	}

	/* TO DO: Mana costs */
   
	af.type = sn;
	af.level = ch->level;
	af.duration = ch->level;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = AFF_ETHEREAL_SNAKE;
	affect_to_char4(ch, &af);
	act( AT_DGREY, "$n summons an ethereal snake!", ch, NULL, NULL, TO_ROOM );
	send_to_char( AT_DGREY, "You have summoned an ethereal snake to assist you.\n\r", ch );

	WAIT_STATE( ch, skill_table[sn].beats );

	return SKPELL_NO_DAMAGE;
    }

    /* Snake commands will go here */

    victim = ch->fighting;

    if (target_name[0] == '\0')
    {
	send_to_char( AT_DGREY, "You get the attention of your snake.\n\r", ch );
	act( AT_DGREY, "$n gets the attention of $m snake.", ch, NULL, NULL, TO_ROOM );
	return SKPELL_NO_DAMAGE;
    }

    if ( !str_cmp( target_name, "bite" ) )
    {
	send_to_char( AT_DGREY, "You order your snake to bite!\n\r", ch );
	act( AT_DGREY, "$n orders $m ethereal snake to bite!", ch, NULL, NULL, TO_ROOM );
	WAIT_STATE( ch, skill_table[skill_lookup("ethereal snake bite")].beats );
	
	return spell_ethereal_snake_bite ( skill_lookup("ethereal snake bite"), ch->level, ch, victim );
    }

    if ( !str_cmp( target_name, "strike" ) )
    {
	send_to_char( AT_DGREY, "You order your snake to strike!\n\r", ch );
	act( AT_DGREY, "$n orders $m ethereal snake to strike!", ch, NULL, NULL, TO_ROOM );
	WAIT_STATE( ch, skill_table[skill_lookup("ethereal snake strike")].beats );

	return spell_ethereal_snake_strike ( skill_lookup("ethereal snake strike"), ch->level, ch, victim );

    }

    if ( !str_cmp( target_name, "devour" ) )
    {
	send_to_char( AT_DGREY, "You order your snake to devour!\n\r", ch );
	act( AT_DGREY, "$n orders $m ethereal snake to devour its target whole!", ch, NULL, NULL, TO_ROOM );
	WAIT_STATE( ch, skill_table[skill_lookup("ethereal snake devour")].beats);
	
	return spell_ethereal_snake_devour ( skill_lookup("ethereal snake devour"), ch->level, ch, victim );

    }

    send_to_char ( AT_DGREY, "You may only order your snake to BITE, STRIKE or DEVOUR.\n\r", ch );

    return SKPELL_BOTCHED;

}

int spell_ethereal_wolf_bite( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam;
        
    dam = number_range(  ch->level * 3, ch->level * 7 );
    if ( saves_spell( level, victim ) )
    {
        dam = number_range( ch->level * 3 / 2, ch->level * 7 / 2 );
    }
    damage( ch, victim, dam, sn );

    return SKPELL_NO_DAMAGE;
}

int spell_ethereal_wolf_claw( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam;
    
    dam = number_range(  ch->level * 4, ch->level * 6 );
    if ( saves_spell( level, victim ) )
    {
        dam = number_range( ch->level * 4 / 2, ch->level * 7 / 6 );
    }
    damage( ch, victim, dam, sn );
    
    return SKPELL_NO_DAMAGE;
}

int spell_ethereal_wolf_howl( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || is_affected( ch, sn ) )
    {
        return SKPELL_MISSED;
    }

    af.type      = sn;
    af.level     = level;
    af.duration  = level / 5;
    af.location  = APPLY_DAMROLL;
    af.modifier  = ( level / 5 ) * -1;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_HITROLL;
    af.modifier  = ( level / 5 ) * -1;
    affect_to_char( victim, &af );

    af.location = APPLY_DAMROLL;
    af.modifier = level / 5;
    affect_to_char( ch, &af );

    af.location = APPLY_HITROLL;
    af.modifier	 = level / 5;
    affect_to_char( ch, &af );

    return SKPELL_NO_DAMAGE;
}

int spell_ethereal_snake_strike( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam;
        
    dam = number_range(  ch->level * 5, ch->level * 7 );
    if ( saves_spell( level, victim ) )
    {
        dam = number_range( ch->level * 5 / 2, ch->level * 7 / 2 );
    }
    damage( ch, victim, dam, sn );

    return SKPELL_NO_DAMAGE;
}

int spell_ethereal_snake_bite(int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam;

    if ( !saves_spell( level, victim ) )
    {
	dam = number_range( ch->level * 5, ch->level * 6 );
    }
    else
    {
	dam = number_range( ch->level * 7, ch->level * 9 );

	spell_poison( skill_lookup("poison"), ch->level, ch, victim );
    }

    damage( ch, victim, dam, sn );

    return SKPELL_NO_DAMAGE;
}

int spell_ethereal_snake_devour(int sn, int level, CHAR_DATA *ch, void *vo )
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA  *obj_lose;
    OBJ_DATA  *obj_next;
        
    if ( !IS_NPC(victim) )
    {
      send_to_char(AT_BLUE, "You failed.\n\r", ch);
      return SKPELL_MISSED;
    }
        
    if ( number_percent( ) < level && !saves_spell( level, victim ) )
      for ( obj_lose = victim->carrying; obj_lose; obj_lose = obj_next )
      {
          obj_next = obj_lose->next_content;
          if ( obj_lose->deleted )
              continue;
   
          if ( number_bits( 2 ) != 0 )
              continue;
     
          act(AT_WHITE, "$p has been devoured by an ethereal snake!",      victim, obj_lose, NULL, TO_CHAR );
          act(AT_WHITE, "$n's $p has been devoured by an ethereal snake!", victim, obj_lose, NULL, TO_ROOM );
          extract_obj( obj_lose ) ;
      }
        
    if ( !saves_spell( level, victim ) )

    /*
     * Devour char, do not generate a corpse, do not
     * give experience for kill.  Extract_char will take care   
     * of items carried/wielded by victim.
     */
    {
        act(AT_WHITE, "Your ethereal snake has DEVOURED $N!",         ch, NULL, victim, TO_CHAR );
        act(AT_WHITE, "You have been DEVOURED by $n's ethereal snake!", ch, NULL, victim, TO_VICT );
        act(AT_WHITE, "$n's ethereal snake DEDVOURS $N!",       ch, NULL, victim, TO_ROOM );
    
        if ( IS_NPC( victim ) )
	{
            extract_char( victim, TRUE );
	}
        else
	{
            extract_char( victim, FALSE );
	}
    }     
    return SKPELL_NO_DAMAGE;
}

