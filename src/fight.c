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

/*$Id: fight.c,v 1.121 2005/04/10 16:29:00 tyrion Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

extern char* target_name;

/*
 * Local functions.
 */
int     damclass_adjust      args( ( CHAR_DATA *victim, int dam, int sn) );
void	damclass_shield_weaken args( ( CHAR_DATA *vic, int dam, int sn) );
bool	check_dodge	     args( ( CHAR_DATA *ch, CHAR_DATA *victim, int attackCount ) );
bool	check_enhanced_dodge args( ( CHAR_DATA *ch, CHAR_DATA *victim, int attackCount ) );
void	check_killer	     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_parry	     args( ( CHAR_DATA *ch, CHAR_DATA *victim, int attackCount ) );
bool	check_enhanced_parry args( ( CHAR_DATA *ch, CHAR_DATA *victim, int attackCount ) );
bool	check_shield_block   args( ( CHAR_DATA *ch, CHAR_DATA *victim, int attackCount ) );
void	dam_message	     args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				    int dt ) );
void	death_cry	     args( ( CHAR_DATA *ch ) );
void	group_gain	     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	     args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
int     damreduce 	     args( ( CHAR_DATA *ch,  CHAR_DATA *victim, int dam, int dt ) );

bool	is_safe		     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    is_wielding_poisoned args( ( CHAR_DATA *ch ) );
bool    is_wielding_flaming  args( ( CHAR_DATA *ch ) );
bool    is_wielding_chaos    args( ( CHAR_DATA *ch ) );
bool    is_wielding_icy      args( ( CHAR_DATA *ch ) );
bool    is_wielding_sparking args( ( CHAR_DATA *ch ) );
bool    is_wielding_dispelling args( ( CHAR_DATA *ch ) );
void	make_corpse	     args( ( CHAR_DATA *ch ) );
bool	check_hit		 args( ( CHAR_DATA *ch, CHAR_DATA *victim, int attackCount, bool dual ) );
void	one_hit		     args( ( CHAR_DATA *ch, CHAR_DATA *victim,
				    int dt ) );
void	one_dual	     args( ( CHAR_DATA *ch, CHAR_DATA *victim,
				    int dt ) );
void	raw_kill	     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	set_fighting	     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm		     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    item_damage             args( ( CHAR_DATA *ch, int dam ) );
void    use_magical_item     args( ( CHAR_DATA *ch ) );
void    miss_message	     args( ( CHAR_DATA* ch, CHAR_DATA* victim, bool dual ) );
bool	can_use_attack	     args( ( int attack_num, CHAR_DATA* ch ) );
bool	can_dual			 args( ( CHAR_DATA* ch ) );


int     dam_class_value; /* global damclass value. ewwy */

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Slightly less efficient than Merc 2.2.  Takes 10% of
 *  total CPU time.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *victim;
    CHAR_DATA *rch;
    bool       mobfighting;
    int stun;

    for ( ch = char_list; ch; ch = ch->next )
    {
	if ( !ch->in_room || ch->deleted )
	    continue;

	for (stun = 0; stun < STUN_MAX; stun++)
	{
	  if ( IS_STUNNED( ch, stun ) )
	    ch->stunned[stun]--;
	}

	if ( ( victim = ch->fighting ) )
	{
	    if ( IS_AWAKE( ch ) && ch->in_room == victim->in_room )
	        multi_hit( ch, victim, TYPE_UNDEFINED );
	    else
	        stop_fighting( ch, FALSE );
	    continue;
	}


	if ( IS_AFFECTED( ch, AFF_BLIND )
	    || ( IS_NPC( ch ) && ch->pIndexData->pShop ) )
	    continue;

	/* Ok. So ch is not fighting anyone.
	 * Is there a fight going on?
	 */

	mobfighting = FALSE;

	for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
	{
	    if ( rch->deleted
		|| !IS_AWAKE( rch )
		|| !( victim = rch->fighting ) )
	        continue;

	    if ( !IS_NPC( ch )
		&& ( !IS_NPC( rch ) || IS_AFFECTED( rch, AFF_CHARM ) )
		&& is_same_group( ch, rch )
		&& IS_NPC( victim ) )
		break;

	    if ( IS_NPC( ch )
		&& IS_NPC( rch )
		&& !IS_NPC( victim ) )
	    {
		mobfighting = TRUE;
		break;
	    }
	}

	if ( !victim || !rch )
	    continue;

	/*
	 * Now that someone is fighting, consider fighting another pc
	 * or not at all.
	 */
	if ( mobfighting )
	{
	    CHAR_DATA *vch;
	    int        number;

	    number = 0;
	    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
	    {
		if ( can_see( ch, vch )
		    && ( vch->level > 5 )
		    && is_same_group( vch, victim )
		    && number_range( 0, number ) == 0 )
		{
		    victim = vch;
		    number++;
		}
	    }

	    if ( ( rch->pIndexData != ch->pIndexData && number_bits( 3 ) != 0 )
		|| ( IS_GOOD( ch ) && IS_GOOD( victim ) )
		|| abs( victim->level - ch->level ) > 3 )
	        continue;
	}
        mprog_hitprcnt_trigger( ch, victim );
        mprog_fight_trigger( ch, victim );
	multi_hit( ch, victim, TYPE_UNDEFINED );

    }

    return;
}

/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    bool berserk;
    bool hit;
    bool dual_atk;
    int chance;
    int chance2;
    int chance3;
    int attack = 0;


    if (!IS_NPC(ch) && ch->pcdata->craft_timer)
   	destroy_craft(ch, FALSE);

    if ( IS_NPC( ch ) )
    {
      mprog_hitprcnt_trigger( ch, victim );
      mprog_fight_trigger( ch, victim );
    }


    if ( ( IS_AFFECTED2( ch, AFF_CONFUSED ) )
       && number_percent ( ) < 30 )
    {
      act(AT_YELLOW, "$n looks around confused at what's going on.", ch, NULL, NULL, TO_ROOM );
      send_to_char( AT_YELLOW, "You stand confused.\n\r", ch );
      return;
    }

    dual_atk = FALSE;
    hit = FALSE;

    /* FIRST ATTACK */
    one_hit( ch, victim, dt );

    if ( ch->fighting != victim || dt == skill_lookup("backstab") )
	return;
    if ( ch->fighting != victim || dt == skill_lookup("assassinate") )
	return;
    if ( ch->fighting != victim || dt == skill_lookup("double backstab") )
	return;
    if ( ch->fighting != victim || dt == skill_lookup("leap of torfi") )
  	return;
    if ( ch->fighting != victim || dt == skill_lookup("triple backstab") )
	return;


    chance = number_percent( );
    chance2 = number_percent( ) / 2;
    chance3 = number_percent( );
    berserk = FALSE;

    if(IS_AFFECTED2(ch, AFF_BERSERK) || IS_AFFECTED3(ch, AFF_BLOODTHIRSTY) )
    {
	berserk = TRUE;
    }

	for (attack = 2; attack <= MAX_ATTACKS; attack++)
	{
		hit       = FALSE;
		dual_atk  = FALSE;

		if (victim->position == POS_DEAD || victim->deleted)
			return;

		if ( can_use_attack( attack, ch ) )
		{
			if ( ( check_hit(ch, victim, attack, FALSE) ) || berserk )
			{
				hit  = TRUE;
				one_hit( ch, victim, dt );
				if ( ch->fighting != victim )
				{
					return;
				}
			}
			else if ( can_dual( ch ) && (dual_atk  = TRUE) && check_hit(ch, victim, attack, TRUE) )
			{
				hit  = TRUE;
				one_dual( ch, victim, dt );
				if ( ch->fighting != victim )
				{
					return;
				}
			}

			if (!hit)
				miss_message( ch, victim, dual_atk  );
		    
		}
	}

    hit  = FALSE;
    dual_atk  = FALSE;

    /* HASTE ATTACK */
    if( is_affected( ch, skill_lookup("quickness") ) || IS_AFFECTED( ch, AFF_HASTE ) )
    {
	if (victim->position == POS_DEAD || victim->deleted )
		return;

        if ( ( check_hit(ch, victim, 1, FALSE) ) || berserk )
        {
			hit = TRUE;
            one_hit( ch, victim, dt );
            if ( ch->fighting != victim )
            {
                return;
            }
        }
        else if (  can_dual( ch ) && (dual_atk  = TRUE) && check_hit(ch, victim, 1, TRUE) )
        {
			hit = TRUE;
            one_dual( ch, victim, dt );
            if ( ch->fighting != victim )
            {
                return;
			}
		}

		if (!hit)
			miss_message(ch, victim, dual_atk );
    }

    hit  = FALSE;
    dual_atk  = FALSE; 


    if (victim->position == POS_DEAD || victim->deleted )
	return;

    /* WILDERNESS MASTERY ATTACK */
    if( can_use_skspell( ch, skill_lookup("wilderness mastery") ) )
    {
	if( !IS_NPC( ch ) )
        {
		if( ( number_percent( ) < ( ch->pcdata->learned[skill_lookup("wilderness mastery")] / 10 ) ) && IS_OUTSIDE( ch ) && IS_IN_NATURE( ch ) )
		{
			update_skpell( ch, skill_lookup("wilderness mastery"), 0 );
			if ( ( check_hit(ch, victim, 1, FALSE) ) || berserk )
			{
				hit = TRUE;
				one_hit( ch, victim, dt );
				if ( ch->fighting != victim )
				{
					return;
				}
			}
			else if (  can_dual( ch ) && (dual_atk  = TRUE) && check_hit(ch, victim, 1, TRUE) )
			{
				hit = TRUE;
				one_dual( ch, victim, dt );
				if ( ch->fighting != victim )
				{
					return;
				}
			
			}

			if (!hit)
				miss_message( ch, victim, dual_atk  );
	    	}
	}
    }


    return;
}

/*
	Logic to determine if a character hits his victim and deals damage to him.
		ch is the character trying to hit
		victim is the character being hit
		attackCount is which attack this is a check for, first, second third, etc.
		dual is whether or not this is a check for your main attack, or the dualed.
*/
bool check_hit( CHAR_DATA *ch, CHAR_DATA *victim, int attackCount, bool dual ) {
	/* now defined in merc.h
	int MAX_ATTACKS = 8;
	*/
	int toHit = 0;
	int chDex;
	int victimDex;
	char buf[1000];
	OBJ_DATA* wield= NULL;
	OBJ_DATA* ammo = NULL;

	if (!dual)
		wield = get_eq_char( ch, WEAR_WIELD );
	else
		wield = get_eq_char( ch, WEAR_WIELD_2 );

	if ( !IS_NPC(ch) && wield && wield->value[3] == flag_value( weapon_flags, "shot" ) )
	{
		if (wield->value[0] == flag_value( ammo_flags, "arrow" ) )
			ammo = find_arrows( ch );
		else if (wield->value[0] == flag_value( ammo_flags, "bullet" ) )
			ammo = find_bullets( ch );
		else if (wield->value[0] == flag_value( ammo_flags, "bolt" ) )
			ammo = find_bolts( ch );
		else
			ammo = NULL;

		if (ammo)
		{
		/* I don't think this needs to be here, it is causing you to lose double ammo during combat rounds - Tyrion */
		/* Leave it here. Remove it from one_hit. This will take care of 99% of ammo checks - Ahsile */
			if (!--ammo->value[0])
			{
				sprintf( buf, "%s has just run out of shots!\n\r", ammo->short_descr );
				send_to_char( AT_RED, buf, ch );
				extract_obj( ammo );
			}
		}
	}


	chDex = get_curr_dex(ch);
	victimDex = get_curr_dex(victim);

	toHit = GET_HITROLL(ch);

	toHit += (chDex - victimDex) * 20;

	if (!IS_NPC(ch)) {
		if ( can_use_skspell(ch, skill_lookup("enhanced hit")) ) {
			toHit += ( ch->pcdata->learned[skill_lookup("enhanced hit")] / 10 );
			update_skpell( ch, skill_lookup("enhanced hit"), 0 );
		}
		if ( can_use_skspell(ch, skill_lookup("enhanced hit two")) ) {
			toHit += ( ch->pcdata->learned[skill_lookup("enhanced hit two")] / 10 );
			update_skpell( ch, skill_lookup("enhanced hit two"), 0 );
		}
	}
	else {
		toHit += 100;
	};


	toHit += number_range((MAX_ATTACKS - 2)*50, (MAX_ATTACKS)*50);

	toHit = toHit*(16-attackCount)/16;

	/* Do all the dodge/parry/shield block checks here. */
	sprintf(buf, "%s's hitroll value for attack num %d is %d", ch->name, attackCount, toHit);
/*if(!IS_NPC(ch))	bug(buf, 0);*/

	/* If this is the dual attack, but they don't have enhanced dual, half the chance to hit */
	if (dual)
	{
		if (!can_use_skspell(ch, skill_lookup("enhanced dual"))) 
		{
			toHit /= 2;
		}
		else
		{
			update_skpell( ch, skill_lookup("enhanced dual"), 0 );
		}
	}

	if ( number_percent() > toHit/10 ) {
		/*
			sprintf(buf, "Attack #: %d", attackCount);
			bug(buf, 0);
		    if ( IS_SET( ch->act, PLR_COMBAT ) )
				act(AT_GREEN, "You attack $N, but miss.", ch, NULL, victim, TO_CHAR    );
			if ( IS_SET( victim->act, PLR_COMBAT ) )
				act(AT_GREEN, "$n attacks you, but misses.",  ch, NULL, victim, TO_VICT    );
		*/
		return FALSE;
	}

	if (check_parry( ch, victim, attackCount ) )
		return FALSE;
	if (check_shield_block( ch, victim, attackCount ))
		return FALSE;
	if (check_dodge( ch, victim, attackCount ))
		return FALSE;
	if (check_enhanced_parry( ch, victim, attackCount ) )
		return FALSE;
	if (check_enhanced_dodge( ch, victim, attackCount ) )
		return FALSE;

	return TRUE;
}




/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    OBJ_DATA *dual;
    char      buf [ MAX_STRING_LENGTH ];
    int       dam;
    int	      val = 0;
    int	      ammo_bonus = 0;
    int       divisor= 1; /* this is to cut firearm dmg down if no ammo is present */

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
		return;

    if ( IS_STUNNED( ch, STUN_NON_MAGIC ) ||
	 IS_STUNNED( ch, STUN_TOTAL ) )
      return;

    /*
     * Figure out the type of damage message.
     */
    wield = get_eq_char( ch, WEAR_WIELD );
    dual  = get_eq_char( ch, WEAR_WIELD_2  );
    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
    }

    /*
     * Hit.
     * Calc damage.
     */
         if ( IS_AFFECTED( victim, AFF_FIRESHIELD ) )
           {
             if ( number_percent( ) < 50 )
	     {
			damage( victim, ch, spell_fireball ( skill_lookup("fireball"), 25, victim, ch ), skill_lookup("fireball"));
	     }
           }
         if ( IS_AFFECTED4( victim, AFF_LEAF_SHIELD ) )
           {
             if ( number_percent( ) < 50 )
             {
                 damage( victim, ch, spell_leaf_strike ( skill_lookup("leaf strike"), 25, victim, ch ), skill_lookup("leaf strike"));
             }
           }

         if ( IS_AFFECTED3( victim, AFF_DEMONSHIELD ) )
           {
             if ( number_percent( ) < 30 )
	     {
		 damage( victim, ch, spell_demonfire ( skill_lookup("demonfire"), 25, victim, ch ), skill_lookup("demonfire"));
	     }
 }
         if ( IS_AFFECTED3( victim, AFF_ACIDSHIELD ) )
           {
             if ( number_percent( ) < 75 )
		 damage( victim, ch, spell_acid_blast ( skill_lookup("acid blast"), 25, victim, ch ), skill_lookup("acid blast"));

           }

	 if ( IS_AFFECTED3( victim, AFF_AURA_ANTI_MAGIC ) )
	   {
	     if ( number_percent ( ) < 25 )
	     {
		 spell_dispel_magic (skill_lookup("dispel magic"), 80, victim, ch );
	     }
	   }
         if ( IS_AFFECTED3( victim, AFF_COFIRE ) )
           {
             if ( number_percent( ) < 50 )
	     {
		 damage( victim, ch, spell_fireball ( skill_lookup("fireball"), 25, victim, ch ), skill_lookup("fireball"));
	     }
           }
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
	if ( IS_AFFECTED( victim, AFF_ICESHIELD ) )
           {
             if ( number_percent( ) < 40 ) 
	     {
		 damage( victim, ch, spell_iceball ( skill_lookup("iceball"), 40, victim, ch ), skill_lookup("iceball"));
	     }
	   }
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
	if ( IS_AFFECTED3(victim, AFF_BLOODSHIELD ) )
	   {
	     if ( number_percent( ) < 40 )
	     {
		   damage( victim, ch, spell_blood_gout( skill_lookup("blood gout"), 40, victim, ch ), skill_lookup("blood gout"));
	     }
	   }
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
	if ( IS_AFFECTED2(victim, AFF_UNHOLY_STRENGTH ) )
	   {
	     if ( number_percent( ) < 40 )
	     {
		   damage( victim, ch, spell_blood_gout( skill_lookup("blood gout"), 40, victim, ch ), skill_lookup("blood gout"));
	     }
	   }
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
	if ( IS_AFFECTED( victim, AFF_SHOCKSHIELD ) )
           {
             if ( number_percent( ) < 50 )
	     {
		 damage( victim, ch, spell_lightning_bolt ( skill_lookup("lightning bolt"), 35, victim, ch ), skill_lookup("lightning bolt"));
	     }
	   }
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
	if ( IS_AFFECTED4( victim, AFF_ESSENCE_OF_GAIA ) )
	   {
	     if ( ( number_percent( ) < 40 ) )
	     {
		 damage( victim, ch, spell_lightning_bolt( skill_lookup("lightning bolt"), 35, victim, ch ), skill_lookup("lightning bolt") );
	     }
	   }
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
	if ( IS_AFFECTED( victim, AFF_CHAOS ) )
           {
             if ( number_percent( ) < 40 )
	     {
		 damage( victim, ch, spell_energy_drain ( skill_lookup("energy drain"), 40, victim, ch ), skill_lookup("energy drain"));
	     }
           }
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
	if ( IS_AFFECTED( victim, AFF_INERTIAL ) )
           {
             if ( number_percent( ) < 35 )
	     {
	       damage( victim, ch, spell_psionic_blast ( skill_lookup("psionic blast"), 25, victim, ch ), skill_lookup("psionic blast"));
	     }
           }
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
	if ( IS_AFFECTED2( victim, AFF_GHOST_SHIELD ) )
	   {
	     if ( number_percent( ) < 50 )
	       damage( victim, ch, spell_harm ( skill_lookup("harm"), 35, victim, ch ), skill_lookup("harm"));
	   }
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
        return;
        if ( IS_AFFECTED3( victim, AFF_SATANIC_INFERNO ) )
           {
             if ( number_percent( ) < 40 )
	     {
               damage( victim, ch, spell_unholy_fires ( skill_lookup("unholy fires"), 35, victim, ch ), skill_lookup("unholy fires"));
	     }
           }
 if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
	if ( IS_AFFECTED2( victim, AFF_SHADOW_IMAGE ) )
	   {
	     if ( number_percent( ) < 20 )
	       damage( victim, ch, spell_energy_drain ( skill_lookup("energy drain"), 35, victim, ch ),skill_lookup("energy drain"));
	   }
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
	if ( IS_AFFECTED2( victim, AFF_BLADE ) )
           {
             if ( number_percent( ) < 40 )
                damage( victim, ch, spell_daggers_of_pain ( skill_lookup("daggers of pain"), 25, victim, ch ),skill_lookup("daggers of pain"));
           }
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
        return;
        if ( IS_AFFECTED4( victim, AFF_ANGELIC_AURA ) )
           {
             if ( number_percent( ) < 40 )
                damage( victim, ch, spell_holy_fires ( skill_lookup("holy fires"), 40, victim, ch ),skill_lookup("holy fires"));
           }  
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
        return;
        if ( IS_AFFECTED4( victim, AFF_LIQUID_SKIN ) )
           {
             if ( number_percent( ) < 35 )
               damage( victim, ch, spell_waterspike ( skill_lookup("waterspike"), 30, victim, ch ),skill_lookup("waterspike"));
           }

  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
	if ( IS_AFFECTED4( victim, AFF_ETHEREAL_WOLF ) )
	   {
	     if ( number_percent( ) < 40 )
		spell_ethereal_wolf_bite ( skill_lookup("ethereal wolf bite"), 35, victim, ch );
	   }

  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
	if ( IS_AFFECTED4( victim, AFF_ETHEREAL_SNAKE ) )
	   {
	     if ( number_percent( ) < 40 )
		spell_ethereal_snake_strike ( skill_lookup("ethereal snake strike"), 35, victim, ch );
	   }

  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
        return;
        if ( IS_AFFECTED3( victim, AFF_RANDOMSHIELD ) )
           {
	     int choice = number_range(1, 7);
             if ( number_percent( ) < 40 )
	     switch( choice )
	     {
		default:
		    damage( victim, ch, spell_fireball( skill_lookup("fireball"), 25, victim, ch ),skill_lookup("fireball"));
		    break;
		case 1:
		    damage( victim, ch, spell_fireball( skill_lookup("fireball"), 25, victim, ch ),skill_lookup("fireball"));
		    break;
		case 2:
		    damage( victim, ch, spell_energy_drain ( skill_lookup("energy drain"), 25, victim, ch ),skill_lookup("energy drain"));
		    break;
		case 3:
		    damage( victim, ch, spell_harm ( skill_lookup("harm"), 25, victim, ch ),skill_lookup("harm"));
		    break;
		case 4:
		    damage( victim, ch, spell_unholy_fires ( skill_lookup("unholy fires"), 25, victim, ch ),skill_lookup("unholy fires"));
		    break;
		case 5:
		    damage( victim, ch, spell_lightning_bolt ( skill_lookup("lightning bolt"), 25, victim, ch ),skill_lookup("lightning bolt"));
		    break;
		case 6:
		    damage( victim, ch, spell_daggers_of_pain ( skill_lookup("daggers of pain"), 50, victim, ch ),skill_lookup("daggers of pain"));
		    break;
                case 7:
                    damage( victim, ch, spell_waterspike ( skill_lookup("waterspike"), 25, victim, ch ),skill_lookup("waterspike"));
                    break;
	     }
           }
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
        if ( IS_AFFECTED2( victim, AFF_IMAGE ) && victim->clan == 3 )
	    {
	      AFFECT_DATA *iaf;

	      for ( iaf = victim->affected2; iaf; iaf = iaf->next )
	      {
		if ( iaf->deleted )
		  continue;

		if ( iaf->type == skill_lookup("image") )
		{
		  if ( number_percent( ) < (iaf->modifier * 7) )
		    spell_curse(skill_lookup("curse"),
					(iaf->modifier * 4), victim, ch );
		  break;
		}
	      }
	    }
  if ( !victim || victim->position == POS_DEAD || ch->in_room != victim->in_room )
        return;

/* bow stuff here */

	if (!IS_NPC(ch) && wield &&  wield->value[3] == flag_value( weapon_flags, "shot" ) )
	{
		OBJ_DATA* ammo = NULL;
		if ( IS_SET(wield->extra_flags2, ITEM_MAGIC_SHOT) )
			ammo = (OBJ_DATA*) 1; // fake out
		else if (wield->value[0] ==  flag_value( ammo_flags, "arrow"))
			ammo = 	find_arrows( ch );
		else if (wield->value[0] ==  flag_value( ammo_flags, "bullet"))
			ammo = 	find_bullets( ch );
		else if (wield->value[0] == flag_value( ammo_flags, "bolt"))
			ammo = 	find_bolts( ch );
		else
			ammo = NULL;

		if (!ammo)
		{
			dt = TYPE_HIT + flag_value( weapon_flags, "crush" ); /* force bludg damage */
			divisor 	= 2;  /* half damage if you have no ammo */
			ammo_bonus 	= 0; /* no ammo bonus */
		} else
		{
			if ((int) ammo == 1)
			{
				ammo_bonus = number_range( ch->level, ch->level / 2 );
				val = DAMCLASS_MAGIC;
			} else
			{
				val = ammo->value[1];
				ammo_bonus = number_range( ammo->value[2], ammo->value[3] );	
			}
			divisor    = 1; /* make sure damage = 100% */
			
		}
	} else
	{
		divisor = 1;
		ammo_bonus = 0;
	}


/* end bow stuff */


    if ( IS_NPC( ch ) )
    {
	dam = number_range( ch->level / 3, ch->level * 3 / 2 );
	if ( wield )
	    dam += dam / 3;
    }
    else
    {
	if ( wield )
	{
	    dam  = number_range( wield->value[1], wield->value[2] );
	    dam += ammo_bonus;
	    dam  = (int) (dam / divisor); /* only necessary for firearms */
	}
	else
	{
	    if ( ch->class == CLASS_MONK || ch->multied == CLASS_MONK || ch->class == CLASS_WEREWOLF || ch->multied == CLASS_WEREWOLF )
	    {
		dam = number_range( 1, ( ch->level / 20 ) + 4 ) * race_table[ ch->race ].size;
	    }
	    else
	    {
		dam = number_range( 1, 4 ) * race_table[ ch->race ].size;
	    }
	}
	if ( wield && dam > 1000 && !IS_IMMORTAL(ch) )
	{
	    sprintf( buf, "One_hit dam range > 1000 from %d to %d",
		    wield->value[1], wield->value[2] );
	    bug( buf, 0 );
	    if ( wield->name )
	      bug( wield->name, 0 );
	}
    }

    /*
     * Bonuses.
     */
    dam += GET_DAMROLL( ch );

    if ( ( ch->class == CLASS_MONK || ch->multied == CLASS_MONK ) && !wield && !dual )
        dam += ch->level/2;
    if ( ( ch->class == CLASS_WEREWOLF || ch->multied == CLASS_WEREWOLF ) && !wield && !dual )
	dam += ch->level/2 + 10;
    /* VAMP Mods */
    if (ch->class == CLASS_VAMPIRE || ch->multied == CLASS_VAMPIRE)
    {
	if (time_info.hour < 6 || time_info.hour >= 19)
	{
	    dam += dam / 3;
	}
	else
	{
	    dam -= dam / 3;
	}
    }

    if ( wield && IS_SET( wield->extra_flags, ITEM_POISONED ) )
        dam += dam / 16;
    if ( wield && IS_SET( wield->extra_flags, ITEM_FLAME ) )
        dam += dam / 16;
     if ( wield && IS_SET( wield->extra_flags, ITEM_CHAOS ) )
        dam += dam / 16;
    if ( wield && IS_SET( wield->extra_flags, ITEM_ICY   ) )
        dam += dam / 16;
    if ( wield && IS_SET( wield->extra_flags2, ITEM_SPARKING ) )
        dam += dam / 16;
/*     if ( wield && IS_SET( wield->extra_flags, ITEM_ACID ) )
        dam += dam / 16;
*/
  if ( !IS_NPC( ch ) && ch->pcdata->learned[skill_lookup("enhanced damage")] > 0 )
    {
	dam += dam / 2 * ch->pcdata->learned[skill_lookup("enhanced damage")] / 1000;
	update_skpell( ch, skill_lookup("enhanced damage"), 0 );
    }
    if ( !IS_NPC( ch ) && ch->pcdata->learned[skill_lookup("enhanced damage two")] > 0 )
    {
        dam += dam / 6 * ch->pcdata->learned[skill_lookup("enhanced damage two")] / 1000;
	update_skpell( ch, skill_lookup("enhanced damage two"), 0 );
    }
    if ( !IS_NPC( ch ) && ch->pcdata->learned[skill_lookup("enhanced damage three")] > 0 )
    {
        dam += dam / 6 * ch->pcdata->learned[skill_lookup("enhanced damage three")] / 1600;
	update_skpell( ch, skill_lookup("enhanced damage three"), 0 );
    }
    if ( !IS_AWAKE( victim ) )
	dam *= 2;
    if ( dt == skill_lookup( "backstab" ) )
    {
	dam *= 2 + UMIN( ( ch->level / 8) , 4 );
    }
    if ( dt == skill_lookup( "assassinate" ) )
    {
	dam = (int)(dam * (1.65 * (2 + UMIN( ( ch->level / 8) , 4 ) ) ));
    }
    if ( dt == skill_lookup( "leap of torfi" ) )
    {
	dam *= 2 + UMIN( ( ch->level / 8) , 4 );
        if (!str_cmp(ch->name,"Torfi"))
	{
	   dam = (int)(dam * 1.1);
	}
    }
    if ( dt == skill_lookup( "double backstab" ) )
    {
	dam *= 2 + UMIN( ( ch->level / 8) , 4 );
    }

    if ( dt == skill_lookup( "triple backstab" ) )
    {
	dam *= 2 + UMIN( ( ch->level / 8) , 4 );
    }

    if ( dam <= 0 )
	dam = 1;

    if (!IS_NPC(victim))
    	dam = damreduce( ch, victim, dam, dt );    

    dam_class_value = val;
    damage( ch, victim, dam, dt );
    dam_class_value = 0;

    tail_chain( );
    return;
}

void one_dual( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    OBJ_DATA *dual;
    char      buf [MAX_STRING_LENGTH ];
    int       dam;
    int	      val = 0;
    int	      divisor = 1;
    int	      ammo_bonus = 0;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if (!get_eq_char ( ch, WEAR_WIELD_2 ) && ( IS_SET( race_table[ch->race].race_abilities,
			 RACE_WEAPON_WIELD ) ) )
       return;
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    if ( IS_STUNNED( ch, STUN_NON_MAGIC ) ||
	 IS_STUNNED( ch, STUN_TOTAL ) )
      return;

    /*
     * Figure out the type of damage message.
     */
    wield = get_eq_char( ch, WEAR_WIELD_2 );
    dual = get_eq_char( ch, WEAR_WIELD );
    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
    }


/* bow stuff here */

	if ( !IS_NPC( ch) && wield && wield->value[3] == flag_value( weapon_flags, "shot" ) )
	{
		OBJ_DATA* ammo = NULL;
		if (wield->value[0] ==  flag_value( ammo_flags, "arrow"))
			ammo = 	find_arrows( ch );
		else if (wield->value[0] ==  flag_value( ammo_flags, "bullet"))
			ammo = 	find_bullets( ch );
		else if (wield->value[0] == flag_value( ammo_flags, "bolt"))
			ammo = 	find_bolts( ch );
		else
			ammo = NULL;	
	
		if (!ammo)
		{
			dt = TYPE_HIT + flag_value( weapon_flags, "crush" ); /* force bludg damage */
			divisor 	= 2;  /* half damage if you have no ammo */
			ammo_bonus 	= 0; /* no ammo bonus */
		} else
		{
			val 	   = ammo->value[1];
			divisor    = 1; /* make sure damage = 100% */
			ammo_bonus = number_range( ammo->value[2], ammo->value[3] );			
			if (!--ammo->value[0])
			{
				sprintf(buf, "You have just run out of %s!\n\r", ammo->short_descr);
				send_to_char(AT_RED, buf, ch );
				extract_obj( ammo );
			}
		}
	} else
	{
		divisor = 1;
		ammo_bonus = 0;
	}


/* end bow stuff */


    /*
     * Hit.
     * Calc damage.
     */

    if ( IS_NPC( ch ) )
    {
	dam = number_range( ch->level / 3, ch->level * 3 / 2 );
	if ( wield )
	    dam += dam / 3;
    }
    else
    {
        if ( wield )
        {
            dam  = number_range( wield->value[1], wield->value[2] );
	    dam += ammo_bonus;
	    dam  = (int) (dam / divisor);
        }
        else
        {
	    if ( ch->class == CLASS_MONK || ch->multied == CLASS_MONK || ch->class == CLASS_WEREWOLF || ch->multied == CLASS_WEREWOLF )
	    {
		dam = number_range( 1, (ch->level / 20 ) + 4 ) * race_table[ ch->race ].size;
	    }
	    else
	    {
		dam = number_range( 1, 4 ) * race_table[ ch->race ].size;
	    }
	}
	if ( wield && dam > 1000 && !IS_IMMORTAL(ch) )
	{
	    sprintf( buf, "One_hit dam range > 1000 from %d to %d",
		    wield->value[1], wield->value[2] );
	    bug( buf, 0 );
	    if( wield->name )
	      bug( wield->name, 0 );
	}
    }

    /*
     * Bonuses.
     */
    dam += GET_DAMROLL( ch );

    if ( ch->race == 13 )
	dam += ch->level;
    if ( ( ch->class == CLASS_MONK || ch->multied == CLASS_MONK ) && !wield && !dual)
        dam += ch->level/2;
    if ( ( ch->class == CLASS_WEREWOLF || ch->multied == CLASS_WEREWOLF ) && !wield && !dual )
	dam += ch->level/2 + 10;
    /* VAMP mods */
    if ( ch->class == CLASS_VAMPIRE || ch->multied == CLASS_VAMPIRE)
    {
	if (time_info.hour < 6 || time_info.hour >= 19)
	{
	    dam += dam / 3;
	}
	else
	{
	    dam -= dam / 3;
	}
    }

    if ( wield && IS_SET( wield->extra_flags, ITEM_POISONED ) )
        dam += dam / 16;
    if ( wield && IS_SET( wield->extra_flags, ITEM_FLAME ) )
        dam += dam / 16;
    if ( wield && IS_SET( wield->extra_flags, ITEM_CHAOS ) )
        dam += dam / 16;
    if ( wield && IS_SET( wield->extra_flags, ITEM_ICY   ) )
        dam += dam / 16;
    if ( wield && IS_SET( wield->extra_flags2, ITEM_SPARKING ) )
        dam += dam / 16;
    if ( !IS_NPC( ch ) && ch->pcdata->learned[skill_lookup("enhanced damage")] > 0 )
    {
	dam += dam / 2 * ch->pcdata->learned[skill_lookup("enhanced damage")] / 1000;
	update_skpell( ch, skill_lookup("enhanced damage"), 0 );
    }
    if ( !IS_NPC( ch ) && ch->pcdata->learned[skill_lookup("enhanced damage two")] > 0 )
    {
        dam += dam / 6 * ch->pcdata->learned[skill_lookup("enhanced damage two")] / 1000;
	update_skpell( ch, skill_lookup("enhanced damage two"), 0 );
    }
    if ( !IS_NPC( ch ) && ch->pcdata->learned[skill_lookup("enhanced damage three")] > 0 )
    {
        dam += dam / 6 * ch->pcdata->learned[skill_lookup("enhanced damage three")] / 1000;
	update_skpell( ch, skill_lookup("enhanced damage three"), 0 );
    }
    if ( !IS_AWAKE( victim ) )
	dam *= 2;
    if ( dt == skill_lookup("backstab") )
    {
	dam *= 2 + UMIN( ( ch->level / 8) , 4 );
    }
    if ( dt == skill_lookup("assassinate") )
    {
	dam = (int)(dam * (1.5 * (2 + UMIN( ( ch->level / 8) , 4 ) ) ));
    }
    if ( dt == skill_lookup("leap of torfi") )
    {
	dam = (int)(dam * (1.5 * (2 + UMIN( ( ch->level / 8) , 4 ) ) ));
    }
    if ( dt == skill_lookup("double backstab") )
    {
	dam *= 2 + UMIN( ( ch->level / 8) , 4 );
    }
    if ( dt == skill_lookup("triple backstab") )
    {
	dam *= 2 + UMIN( ( ch->level / 8) , 4 );
    }

    if ( dam <= 0 )
	dam = 1;

    if (!IS_NPC(victim))
       dam = damreduce( ch, victim, dam, dt );

    dam_class_value = val;
    damage( ch, victim, dam, dt );
    dam_class_value = 0;
    tail_chain( );
    update_skpell( ch, skill_lookup("dual"), 0 );

    return;
}

/* removes an object and put it right back on,
   we need this so that when a player get pkilled
   the spells on objs don't disappear. */
void remove_requip(  CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if ( !( obj = get_eq_char( ch, iWear ) ) )
        return;

    unequip_char( ch, obj );
    equip_char( ch, obj, iWear );
}


/*
 * Inflict damage from a hit.
 */
void damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{

    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    int iWear;

    if ( victim->position == POS_DEAD )
	return;

    /* Combat Timer for PVP, and immunity for self damaging attacks
	IE: Poison, Meteor Swarm - Ahsile */
    if(!IS_NPC(ch) && !IS_NPC(victim) && (victim!=ch) && (dt != skill_lookup("incinerate") 
	&& dt != skill_lookup("deadly poison") && dt != skill_lookup("tortured soul") 
	&& dt != skill_lookup("pestilence") && dt != skill_lookup("breathe water") 
	&& dt != skill_lookup("poison") ) )
    {
        ch->combat_timer = 90;
        victim->combat_timer = 90;
    }

    /*
     * Lets stick in the enhanced spell damage from spellcraft.
     */
    /* FIXME  needs to check to make sure the damage is magical */
    if (!IS_NPC(ch) && (number_percent() < (ch->pcdata->learned[skill_lookup("improved concentration")]/10)))
    {
		if (skill_table[dt].is_spell) /* FIXED - Ahsile */
		{
			update_skpell( ch, skill_lookup("improved concentration"), 0 );
	       dam = number_range(dam , (int) (dam * 1.25));
		}
	}
    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 6000 )
    {
        char buf [ MAX_STRING_LENGTH ];

//        if ( ( dt != 91 && dt != 268 && dt != 269 && dt != 92 && dt != 229 && dt != 347 && dt != 402 && dt != 304 && dt != 353 && dt != 37 ) && ch->level <= LEVEL_DEMIGOD )
	/*  37 - Energy Dragin */
	/* 174 - Ultrablast */
	/* 269 - Assassinate */
	/* 445 - Death Strike */
        if ( ( dt != 37 && dt != 174 && dt != 445 && dt != 269 ) && ch->level <= LEVEL_DEMIGOD )
        {
            if ( IS_NPC( ch ) && ch->desc && ch->desc->original )
	    sprintf( buf,
		    "Damage: %d from %s by %s: > 6000 points with %d dt!",
		    dam, ch->name, ch->desc->original->name, dt );
            else
	    sprintf( buf,
		    "Damage: %d from %s: > 6000 points with %d dt!",
		    dam, ch->name, dt );

            bug( buf, 0 );
        }
    }

    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */

        if ( !IS_NPC(ch) && !IS_NPC(victim) && !is_pkillable( ch, victim)  )
        {
            return;
        }
	if ( is_safe( ch, victim ) )
	    return;

	check_killer( ch, victim );

	if ( victim->position > POS_STUNNED )
	{
	    if ( !victim->fighting )
		set_fighting( victim, ch );
	    victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( !ch->fighting )
	    {
		ch->initiated = TRUE;
		set_fighting( ch, victim );
	    }
	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if (   IS_NPC( ch )
		&& IS_NPC( victim )
		&& IS_AFFECTED( victim, AFF_CHARM )
		&& victim->master
		&& victim->master->in_room == ch->in_room
		&& number_bits( 3 ) == 0 )
	    {
		stop_fighting( ch, FALSE );
		set_fighting( ch, victim->master );
		return;
	    }
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );

	/*
	 * Inviso attacks ... not.
	 */
	if ( IS_AFFECTED( ch, AFF_INVISIBLE ) )
	{
	    affect_strip( ch, skill_lookup("invis")      );
	    affect_strip( ch, skill_lookup("mass invis") );
	    REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	    act(AT_GREY, "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
	}
	if (IS_AFFECTED2( ch, AFF_PHASED ) )
	{
	    affect_strip( ch, skill_lookup("phase shift") );
	    REMOVE_BIT( ch->affected_by2, AFF_PHASED );
	    act(AT_GREY, "$n returns from an alternate plane.", ch, NULL, NULL, TO_ROOM );
	}

	/* if you or target has shadow plane, and there is combat, this here gets rid of it */

        if ( IS_AFFECTED2( ch, AFF_SHADOW_PLANE ) )
        {
	    affect_strip( ch, skill_lookup("shadow plane") );
	    REMOVE_BIT( ch->affected_by2, AFF_SHADOW_PLANE );
	    act(AT_BLOOD, "$n returns to reality!", ch, NULL, NULL, TO_ROOM );
        }
        if ( IS_AFFECTED4( ch, AFF_BURROW ) )
        {
	    affect_strip( ch, skill_lookup("tomba di vemon")     );
	    REMOVE_BIT( ch->affected_by4, AFF_BURROW );
	    act(AT_BLOOD, "$n is expelled from the earth!", ch, NULL, NULL, TO_ROOM );
        }

        if ( IS_AFFECTED2( victim, AFF_SHADOW_PLANE ) )
	{
	    affect_strip (victim, skill_lookup("shadow plane") );
	    REMOVE_BIT( victim->affected_by2, AFF_SHADOW_PLANE );
	    act(AT_BLOOD, "$n returns to reality!", victim, NULL, NULL, TO_ROOM );
	}
        if ( IS_AFFECTED4( victim, AFF_BURROW ) )
	{
	    affect_strip (victim, skill_lookup("tomba di vemon") );
	    REMOVE_BIT( victim->affected_by4, AFF_BURROW );
	    act(AT_BLOOD, "$n is expelled from the earth!", victim, NULL, NULL, TO_ROOM );
	}


	/*
	 * Proficiency modifiers
	 */
	 if (dt > TYPE_HIT && !IS_NPC(ch) )
	 {
		 int prof = dt - TYPE_HIT;
		 int sn = 0;

		 if ( ( 
				   flag_value(weapon_flags, "hit")		== prof 
				|| flag_value(weapon_flags, "claw")		== prof
				|| flag_value(weapon_flags, "blast")	== prof
				|| flag_value(weapon_flags, "whip")		== prof
				|| flag_value(weapon_flags, "grep")		== prof
				|| flag_value(weapon_flags, "bite")		== prof
				|| flag_value(weapon_flags, "suction")	== prof
			  )
			 && ch->pcdata->learned[(sn = skill_lookup("proficiency hit"))] )
		 {

			dam += (int) ( dam * (ch->pcdata->learned[sn] / 10000.0f) );
			 update_skpell( ch, sn, 0 );

		 } else if ( (
						   flag_value(weapon_flags, "stab")		== prof
						|| flag_value(weapon_flags, "pierce")	== prof
					 )
					 && ch->pcdata->learned[(sn = skill_lookup("proficiency pierce"))] )
		{

			dam += (int) ( dam * (ch->pcdata->learned[sn] / 10000.0f) );
			update_skpell( ch, sn, 0 );

		}  else if ( (
						   flag_value(weapon_flags, "slash")	== prof
					 )
					 && ch->pcdata->learned[(sn = skill_lookup("proficiency slash"))] )
		{

			dam += (int) ( dam * (ch->pcdata->learned[sn] / 10000.0f) );
			update_skpell( ch, sn, 0 );

		}  else if ( (
						   flag_value(weapon_flags, "slice")	== prof
					 )
					 && ch->pcdata->learned[(sn = skill_lookup("proficiency slice"))] )
		{

			dam += (int) ( dam * (ch->pcdata->learned[sn] / 10000.0f) );
			update_skpell( ch, sn, 0 );

		}  else if ( (
							flag_value(weapon_flags, "crush")	== prof
						 || flag_value(weapon_flags, "pound")	== prof
					 )
					 && ch->pcdata->learned[(sn = skill_lookup("proficiency crush"))] )
		{

			dam += (int) ( dam * (ch->pcdata->learned[sn] / 10000.0f) );
			update_skpell( ch, sn, 0 );

		}  else if ( (
							flag_value(weapon_flags, "chop")	== prof
					 )
					 && ch->pcdata->learned[(sn = skill_lookup("proficiency chop"))] )
		{

			dam += (int) ( dam * (ch->pcdata->learned[sn] / 10000.0f) );
			update_skpell( ch, sn, 0 );

		}
	 }

	/*
	 * Damage modifiers.
	 */

	if ( !IS_NPC(victim) && !IS_NPC(ch))
	   {
           dam -= dam / 2;
           ch->pkill_timer = 0;  /* added to insure that if a player is damaging another
                                 player, his pk timer is set to 0 */
         }
        if ( ch->race == 0 && ch->class == 7)
	    dam = (int)(dam * 0.8);

        if ( IS_AFFECTED( victim, AFF_SANCTUARY ) || IS_AFFECTED4( victim, AFF_BIOFEEDBACK ) || IS_AFFECTED4( victim, AFF_ESSENCE_OF_GAIA ) || IS_SET( race_table[ victim->race ].race_abilities, RACE_SANCT ) )
            dam  = (int)(dam / 1.5);
	if ( IS_AFFECTED4( victim, AFF_EARTHSHIELD ) )
	    dam = (int)(dam / 1.3);
	if ( IS_AFFECTED2( victim, AFF_THICK_SKIN ) )
	    dam = (int)(dam / 1.15);
	if ( IS_AFFECTED2( victim, AFF_GOLDEN_ARMOR ) )
	    dam = (int)(dam / 1.35);
        if ( IS_AFFECTED4( victim, AFF_GOLDEN_SANCTUARY ) )
	    dam = (int)(dam / 1.5);
	if ( IS_AFFECTED2( victim, AFF_SHADOW_IMAGE ) )
	    dam = (int)(dam / 1.25);
	if ( IS_AFFECTED2( victim, AFF_GHOST_SHIELD ) )
            dam = (int)(dam / 1.05);
	if ( IS_AFFECTED3( victim, AFF_SATANIC_INFERNO ) )
            dam = (int)(dam / 1.05);
	if ( IS_AFFECTED2( victim, AFF_DANCING ) )
	    dam = (int)(dam * 1.25);
	if ( IS_AFFECTED( victim, AFF_FIRESHIELD ) )
	    dam = (int)(dam / 1.05);
	if ( IS_AFFECTED4( victim, AFF_LEAF_SHIELD ) )
	    dam = (int)(dam / 1.05);
	if ( IS_AFFECTED3( victim, AFF_COFIRE ) )
	    dam = (int)(dam / 1.05);
	if ( IS_AFFECTED3( victim, AFF_AURA_ANTI_MAGIC ) )
	    dam = (int)(dam / 1.05);
	if ( IS_AFFECTED( victim, AFF_ICESHIELD ) )
	    dam = (int)(dam / 1.05);
	if ( IS_AFFECTED2( victim, AFF_MIST) )
	    dam = (int)(dam / 1.25);
        if ( IS_AFFECTED( victim, AFF_CHAOS ) )
	    dam = (int)(dam / 1.05);
	if ( IS_AFFECTED( victim, AFF_SHOCKSHIELD ) )
	    dam = (int)(dam / 1.15);
	if ( IS_AFFECTED3( victim, AFF_BLOODSHIELD ) )
	    dam = (int)(dam / 1.15);
	if ( IS_AFFECTED2( victim, AFF_UNHOLY_STRENGTH ) )
	    dam = (int)(dam / 1.12);
	if ( IS_AFFECTED( victim, AFF_INERTIAL ) )
	    dam = (int)(dam / 1.05);
	if ( IS_AFFECTED3( victim, AFF_DEMONSHIELD ) )
	    dam = (int)(dam / 1.10);
        if ( IS_AFFECTED3( victim, AFF_ACIDSHIELD ) )
            dam = (int)(dam / 1.15);
	if ( IS_AFFECTED4( victim, AFF_LIQUID_SKIN ) )
	    dam = (int)(dam / 1.15);
	if ( IS_AFFECTED4( victim, AFF_ANGELIC_AURA ) )
	    dam = (int)(dam / 1.10);
        if ( IS_AFFECTED4( victim, AFF_FORCE_OF_NATURE ) )
	    dam = (int)(dam / 1.15 );

        if ( IS_SET( victim->act, PLR_UNDEAD ) )
            dam = (int)(dam / 1.05);
        if ( (   IS_AFFECTED( victim, AFF_PROTECT      )
              || IS_SET( race_table[ victim->race ].race_abilities,
                        RACE_PROTECTION )              )
            && IS_EVIL( ch )                           )
            dam  = (int)(dam / 1.1);
	if ( IS_AFFECTED2( victim, AFF_PROTECTION_GOOD ) )
	    dam = (int)(dam / 1.1);
/* Damclass Stuff --Manaux */
	dam = damclass_adjust(victim, dam, dt);

	if ( dam < 0 )
	    dam = 0;

	/*
	 * Check for disarm, trip, parry, and dodge.
	 */
	if ( dt >= TYPE_HIT || dt == skill_lookup("kick") || dt == skill_lookup("feed") || dt == skill_lookup("punch") )
	{
	    int leveldiff = ch->level - victim->level;

	    if ( IS_NPC( ch ) && number_percent( )
		< ( leveldiff < -5 ? ch->level / 2 : UMAX( 10, leveldiff ) )
		&& dam == 0 && number_bits(4) == 0)
	        disarm( ch, victim );
            if ( IS_NPC( ch )
                && IS_SET( race_table[ ch->race ].race_abilities,
                          RACE_WEAPON_WIELD )
                && number_percent() < UMIN( 25, UMAX( 10, ch->level ) )
                && !IS_NPC( victim ) )
                use_magical_item( ch );
	}

    }
    if ( ( !IS_NPC(ch) ) && ( !IS_NPC(victim) ) )
       dam -= dam/4;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */

/* tell the victim how much he got hurt */
     if ( dt != TYPE_UNDEFINED )
        dam_message( ch, victim, dam, dt );
     if ( dam > 25 && number_range( 0, 100 ) <= 15 )
        item_damage(victim, dam);
     victim->hit -= dam;

     if ( !IS_NPC( victim )
	&& victim->level >= LEVEL_IMMORTAL
	&& victim->hit < 1 )
	victim->hit = 1;

     if ( IS_AFFECTED4( victim, AFF_IMMORTAL ) && ( victim->hit < 1 ) )
     {
         victim->hit = 1;
     }

/*
    if ( !IS_NPC( victim )
	&& victim->clev == 5
	&& victim->clan == 8
        && victim->hit < 1 )
        victim->hit = 1;
*/

    if ( dam > 0 && dt > TYPE_HIT
	&& is_wielding_poisoned( ch )
	&& !saves_spell( ch->level, victim ) )
    {
	AFFECT_DATA af;

	if( !IS_AFFECTED( victim, AFF_POISON ) )
	{
	    af.type      = skill_lookup("poison");
	    af.duration  = 1;
	    af.location  = APPLY_STR;
	    af.modifier  = -2;
	    af.bitvector = AFF_POISON;
	    affect_join( victim, &af );
	    add_poison( victim, 1 );
	}
    }
    if ( dam > 0 && dt > TYPE_HIT
       && is_wielding_flaming( ch )
       && number_percent( ) < 30 )
          damage( ch, victim, spell_fireball ( skill_lookup("fireball"), 20, ch, victim ), skill_lookup("fireball"));

    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
    if ( dam > 0 && dt > TYPE_HIT
       && is_wielding_icy( ch )
       && number_percent( ) < 30 )
          damage( ch, victim, spell_icestorm ( skill_lookup("iceball"), 20, ch, victim ), skill_lookup("iceball"));

    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;
    if ( dam > 0 && dt > TYPE_HIT && is_wielding_sparking( ch )
       && number_percent( ) < 30 )
        damage( ch, victim, spell_lightning_bolt ( skill_lookup("lightning bolt"), 20, ch, victim ), skill_lookup("lightning bolt"));

    if ( dam > 0 && dt > TYPE_HIT && is_wielding_dispelling( ch )
       && number_percent( ) < 30 )
        spell_dispel_magic ( skill_lookup("dispel magic"), 30, ch, victim );

    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    if ( dam > 0 && dt > TYPE_HIT
       && is_wielding_chaos( ch )
       && number_percent( ) < 30 )
          damage( ch, victim, spell_energy_drain ( skill_lookup("energy drain"), 45, ch, victim ), skill_lookup("energy drain"));

    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    if (  IS_SET( ch->act, PLR_UNDEAD )
       && !saves_spell(ch->level, victim ) )
    {
	AFFECT_DATA af;

	af.type      = skill_lookup("poison");
	af.duration  = 2;
	af.location  = APPLY_CON;
	af.modifier  = -1;
	af.bitvector = AFF_POISON;
	affect_join( victim, &af );
	add_poison( victim, 2 );
    }

    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	send_to_char(AT_RED,
	    "You are mortally wounded, and will die soon, if not aided.\n\r",
	    victim );
	act(AT_RED, "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	break;

    case POS_INCAP:
	send_to_char(AT_RED,
	    "You are incapacitated and will slowly die, if not aided.\n\r",
	    victim );
	act(AT_RED, "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	break;

    case POS_STUNNED:
	send_to_char(AT_WHITE,"You are stunned, but will probably recover.\n\r",
	    victim );
	act(AT_WHITE, "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	break;

    case POS_DEAD:
	send_to_char(AT_BLOOD, "You have been KILLED!!\n\r\n\r", victim );
	act(AT_BLOOD, "$n is DEAD!!", victim, NULL, NULL, TO_ROOM );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char(AT_RED, "That really did HURT!\n\r", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char(AT_RED, "You sure are BLEEDING!\n\r", victim );
	break;
    }

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {

	group_gain( ch, victim );

	stop_fighting( victim, FALSE );
	stop_fighting( ch, FALSE	 );

        if(((ch->guild != NULL) ? ch->guild->type & GUILD_CHAOS : 0)
            && ch->guild == victim->guild
            && victim->guild_rank > ch->guild_rank)
        {
            int temp;
            temp = ch->guild_rank;
            ch->guild_rank = victim->guild_rank;
            victim->guild_rank = temp;
        }
        if ( !IS_NPC(ch) && !IS_NPC(victim) && (ch->name != victim->name) )
        {
            CLAN_DATA  *pClan;
            CLAN_DATA  *Cland;
            RELIGION_DATA  *pReligion;
            RELIGION_DATA  *Religiond;
            pClan = get_clan_index(ch->clan);
            Cland = get_clan_index(victim->clan);
            if(ch->clan != victim->clan )
            {
                pClan->pkills++;
                Cland->pdeaths++;
            }
            pReligion = get_religion_index(ch->religion );
            Religiond = get_religion_index(victim->religion );
            if(ch->religion != victim->religion )
            {
                pReligion->pkills++;
                Religiond->pdeaths++;
            }
        }
        if ( ( !IS_NPC(ch) ) && ( IS_NPC(victim) ) )
        {
            CLAN_DATA    *pClan;
            pClan=get_clan_index(ch->clan);
            pClan->mkills++;
        }
	if ( ( !IS_NPC(ch) ) && ( IS_NPC(victim) ) )
        {
            RELIGION_DATA  *pReligion;
            pReligion=get_religion_index(ch->religion);
            pReligion->mkills++;
	}
        if ( ( IS_NPC(ch) ) && (!IS_NPC(victim)) )
        {
            CLAN_DATA   *pClan;
            pClan=get_clan_index(victim->clan);
            pClan->mdeaths++;
        }
	if ( ( IS_NPC(ch) ) && (!IS_NPC( victim ) ) )
	{
            RELIGION_DATA  *pReligion;
            pReligion=get_religion_index(victim->religion );
            pReligion->mdeaths++;
	}
        if ( !IS_NPC(ch) ) {
            if ( is_affected( ch, skill_lookup("berserk") ) )
            {
                affect_strip( ch, skill_lookup("berserk") );
                send_to_char(C_DEFAULT, skill_table[skill_lookup("berserk")].msg_off,ch);
                send_to_char(C_DEFAULT, "\n\r",ch);
            }
            if ( IS_AFFECTED2(ch, AFF_BERSERK) )
                REMOVE_BIT(ch->affected_by2, AFF_BERSERK);

            if ( is_affected( ch, skill_lookup("bloodthirsty") ) )
            {
                affect_strip( ch, skill_lookup("bloodthirsty") );
                send_to_char(C_DEFAULT, skill_table[skill_lookup("bloodthirsty")].msg_off,ch);
                send_to_char(C_DEFAULT, "\n\r",ch);
            }
            if ( IS_AFFECTED3(ch, AFF_BLOODTHIRSTY) )
                REMOVE_BIT(ch->affected_by3, AFF_BLOODTHIRSTY);
	}

	if ( !IS_NPC( victim ) )
	{
	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    if ( IS_NPC(ch))
	    {
                if ( ( victim->exp > 1000 * victim->level ) && ( !IS_SET(victim->act, PLR_GHOST ) ) )
                {
                    gain_exp( victim, ( 1000 * victim->level - victim->exp ) / 2 );
                }
	    }
	    sprintf( log_buf, "%s killed by %s at %d.", victim->name,
	        ch->name, victim->in_room->vnum );
	    log_string( log_buf, CHANNEL_LOG, -1 );
	    sprintf( log_buf, "%s was slaughtered by %s.", victim->name, ch->name);
	    if(IS_NPC ( ch ) )
	    {
		sprintf( log_buf, "%s was slaughtered by %s.", victim->name, ch->short_descr );
	    }
	    if(!IS_NPC( ch ) && !IS_NPC( victim ) )
	    {
		broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(death8.wav V=100 L=1 P=100 T=Pkill)");
	    }
	    else
	    {
		int tempvar = number_range( 1, 6 );
		switch( tempvar )
		{
		    default:
			broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(death6.wav V=100 L=1 P=50 T=Combat)");
			break;
		    case 1:
			broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(death1.wav V=100 L=1 P=50 T=Combat)");
			break;
		    case 2:
			broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(death2.wav V=100 L=1 P=50 T=Combat)");
			break;
		    case 3:
			broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(death3.wav V=100 L=1 P=50 T=Combat)");
			break;
		    case 4:
			broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(death4.wav V=100 L=1 P=50 T=Combat)");
			break;
		    case 5:
			broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(death5.wav V=100 L=1 P=50 T=Combat)");
			break;
		    case 6:
			broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(death7.wav V=100 L=1 P=50 T=Combat)");
			break;
		}
	    }
            stop_fighting(ch,TRUE);
            victim->hit = 1;
            victim->pkill_timer = time( NULL );
            if ( !IS_NPC(ch) && (ch->name != victim->name ) ) {
            	ch->pcdata->awins += 1;
                victim->pcdata->alosses += 1;
	    }
/* strip the guy */
            for ( paf = victim->affected; paf; paf = paf_next )
            {
                paf_next = paf->next;
                if( !paf->deleted )
                affect_remove( victim, paf );
            }
            for ( paf = victim->affected2; paf; paf = paf_next )
            {
                paf_next = paf->next;
                if( !paf->deleted )
                affect_remove2( victim, paf );
            }
            for ( paf = victim->affected3; paf; paf = paf_next )
            {
                paf_next = paf->next;
                if( !paf->deleted )
                affect_remove3( victim, paf );
            }
            for ( paf = victim->affected4; paf; paf = paf_next )
            {
                paf_next = paf->next;
                if( !paf->deleted && paf->type != skill_lookup("newbie slayer"))
		{
                	affect_remove4( victim, paf );
		}
            }
            for ( paf = victim->affected_powers; paf; paf = paf_next )
            {
                paf_next = paf->next;
                if( !paf->deleted )
                affect_remove_powers( victim, paf );
            }
            for ( paf = victim->affected_weaknesses; paf; paf = paf_next )
            {
                paf_next = paf->next;
                if( !paf->deleted )
                affect_remove_weaknesses( victim, paf );
            }
            update_pos( victim );
	    victim->shields = 0;
            save_clans();
            save_religions();

            if(!IS_NPC( ch ) && !IS_NPC( victim ) && (ch->name != victim->name ) )
            {
		RELIGION_DATA* pRel = get_religion_index(victim->religion);

                char_from_room(victim);

		if (pRel->recall != ROOM_VNUM_OUTSIDEMETH )
			char_to_room(victim, get_room_index(ROOM_VNUM_ALOSER) );
		else
	                char_to_room(victim,get_room_index(pRel->recall));

                for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
                {
                    remove_requip( victim, iWear );
                }

            return;
            }
        }

	raw_kill( ch, victim );
	if( IS_NPC( victim ) && !IS_NPC( ch ) )
	{
	    ch->pcdata->mobkills ++;
	}
	/* Ok, now we want to remove the deleted flag from the
	 * PC victim.
	 */
	if ( !IS_NPC( victim ) )
	    victim->deleted = FALSE;

	if ( !IS_NPC( ch ) && IS_NPC( victim ) )
	{
	    if ( IS_SET( ch->act, PLR_AUTOLOOT ) )
		do_get( ch, "all corpse" );
	    else
		do_look( ch, "in corpse" );

	    if ( IS_SET( ch->act, PLR_AUTOGOLD ) )
	        do_get( ch, "gold corpse" );
	    if ( IS_SET( ch->act, PLR_AUTOSAC  ) )
		do_sacrifice( ch, "corpse" );
	}

	return;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE( victim ) )
	stop_fighting(victim, FALSE);

    if ( victim == ch )
	return;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC( victim ) && !victim->desc )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return;
	}
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC( victim ) && dam >= 0 )
    {
	if ( ( IS_SET( victim->act, ACT_WIMPY ) && number_bits( 1 ) == 0
	      && victim->hit < victim->max_hit / 2 )
	    || ( IS_AFFECTED( victim, AFF_CHARM ) && victim->master
		&& victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    if ( ( !IS_NPC( victim )
	&& victim->hit > 0
	&& victim->hit <= victim->wimpy
	&& victim->wait == 0 )
        && !IS_AFFECTED3(victim, AFF_BLOODTHIRSTY)
        && !IS_AFFECTED2(victim, AFF_BERSERK) )
	do_flee( victim, "" );

    tail_chain( );
    return;
}

void item_damage( CHAR_DATA *ch, int dam )
{
	OBJ_DATA  *obj_lose;
	OBJ_DATA  *obj_next;

	for ( obj_lose = ch->carrying; obj_lose; obj_lose = obj_next )
	{
		char *msg;
		int damage = 0;

		obj_next = obj_lose->next_content;
		if( obj_lose->deleted )
		    continue;
		if( number_bits( 2 ) != 0 )
		    continue;
		if( obj_lose->wear_loc == WEAR_NONE )
		    continue;
		if( IS_SET( obj_lose->extra_flags, ITEM_NO_DAMAGE ) )
		    continue;
		damage =  obj_lose->durability_cur % 10;
		if( damage >= 5 )
		    damage = 5;
		if( damage <= 0 )
		    continue;
		if( obj_lose->weight <= 25 )
		{
			if( obj_lose->weight * 2 <= number_percent( ) )
			    continue;
		}
		else
		{
			if( number_percent( ) <= 50 )
			    continue;
		}

		switch ( obj_lose->item_type )
		{
		default:
		    msg = "Your $p gets ruined!";
		    extract_obj( obj_lose );
		    break;

		case ITEM_DRINK_CON:
		case ITEM_POTION:
		case ITEM_CONTAINER:
		case ITEM_LIGHT:
		    msg = "Your $p shatters!";
		    extract_obj( obj_lose );
		    break;

		case ITEM_WEAPON:
		case ITEM_ARMOR:
		    if( obj_lose->durability_cur > 0 )
		    {
			obj_lose->durability_cur -= damage;
		    }

		    if( obj_lose->durability_cur <= 0 )
		    {
			OBJ_DATA       *pObj;
			OBJ_INDEX_DATA *pObjIndex;
			char           *name;
			char           buf[MAX_STRING_LENGTH];

			pObjIndex = get_obj_index(4);
			pObj = create_object(pObjIndex, obj_lose->level);
			name = obj_lose->short_descr;
			sprintf(buf, pObj->description, name);
			free_string(pObj->description);
			pObj->description = str_dup(buf);
			pObj->weight = obj_lose->weight;
			pObj->timer = obj_lose->level;
			msg = "$p has been destroyed!";
			extract_obj( obj_lose );
			obj_to_room ( pObj, ch->in_room );
		    }
		    else
		    {
			msg = "$p has been damaged!";
		    }
		    break;
		}

		act(AT_YELLOW, msg, ch, obj_lose, NULL, TO_CHAR );
		return;
	}
	return;
}

bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim )
{

  if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
       IS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
    return TRUE;

  if( ch->position == POS_GHOST || victim->position == POS_GHOST )
    return TRUE;

  if ( !IS_NPC(ch) && !IS_NPC(victim) &&
       (IS_SET(ch->in_room->room_flags, ROOM_NO_PKILL) ||
	IS_SET(victim->in_room->room_flags, ROOM_NO_PKILL)) )
    return TRUE;

  if ( IS_SET( ch->in_room->room_flags, ROOM_PKILL ) &&
       IS_SET( victim->in_room->room_flags, ROOM_PKILL ) )
    return FALSE;

  if ( IS_NPC( victim ) )
    return FALSE;

  if ( IS_NPC( ch ) )
  {
    if ( IS_SET(ch->affected_by, AFF_CHARM) && ch->master )
    {
      CHAR_DATA *nch;

      for ( nch = ch->in_room->people; nch; nch = nch->next )
	if ( nch == ch->master )
	  break;

      if ( nch == NULL )
	return FALSE;
      else
	ch = nch; /* Check person who ordered mob for clan stuff.. */
    }
    else
      return FALSE;
  }

/* give pkill guilds ability to pkill */
/* Err.. we might not want pkill guilds attacking unguilded.. */
  if ( (ch->guild && (ch->guild->type & GUILD_PKILL)) &&
       (victim->guild && (victim->guild->type & GUILD_PKILL)) )
  {
    return FALSE;
  }

/* can murder self for testing =) */
  if ( ch->clan == victim->clan && ch != victim && (ch->clan != 0 && victim->clan != 0))
  {
    send_to_char(AT_WHITE, "You cannot murder your own clan member.\n\r",ch);
    return TRUE;
  }

  if ( IS_SET( victim->act, PLR_KILLER ) )
    return FALSE;


  return FALSE;
}



/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    /*
     * NPC's are fair game.
     */
    if ( IS_NPC( victim ) )
        return;

    /* In arena rooms player killing is legal. Doesn't give you any
     * experience gain or loss either. -- Maniac --
     */

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_SET( victim->act, PLR_KILLER )
	|| IS_SET( victim->act, PLR_THIEF  ) )
	return;

    /*
     * NPC's are cool of course
     * Hitting yourself is cool too (bleeding).
     * And current killers stay as they are.
     */
    if ( IS_NPC( ch )
	|| ch == victim
	|| IS_SET( ch->act, PLR_KILLER     ) )
	return;

    if ( ch->clan != 0 )
       return;
    return;
}

/*
 * Check to see if weapon is poisoned.
 */
bool is_wielding_poisoned( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) )
	&& IS_SET( obj->extra_flags, ITEM_POISONED ) )
        return TRUE;

    return FALSE;

}
bool is_wielding_flaming( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) )
	&& IS_SET( obj->extra_flags, ITEM_FLAME ) )
        return TRUE;

    return FALSE;

}
bool is_wielding_sparking( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) )
	&& IS_SET( obj->extra_flags2, ITEM_SPARKING ) )
	return TRUE;

    return FALSE;
}
bool is_wielding_acid( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) )
        && IS_SET( obj->extra_flags, ITEM_ACID ) )
        return TRUE;

    return FALSE;

}
bool is_wielding_dispelling( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) )
	&& IS_SET( obj->extra_flags2, ITEM_DISPEL ) )
	return TRUE;

    return FALSE;
}



bool is_wielding_icy( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) )
	&& IS_SET( obj->extra_flags, ITEM_ICY ) )
        return TRUE;

    return FALSE;

}
bool is_wielding_chaos( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) )
	&& IS_SET( obj->extra_flags, ITEM_CHAOS ) )
        return TRUE;

    return FALSE;

}


/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim, int attackCount )
{
    int chance;

    if ( !IS_AWAKE( victim ) )
	return FALSE;

    if ( IS_NPC( victim ) )
    {
	chance	= UMIN( 50, 2 * victim->level );
	if ( !get_eq_char( victim, WEAR_WIELD ) )
	    return FALSE;
    }
    else
    {
	if ( !get_eq_char( victim, WEAR_WIELD ) )
	    return FALSE;
	if (!can_use_skspell( victim, skill_lookup("parry")))
	{
	    return FALSE;
	}
	chance	= victim->pcdata->learned[skill_lookup("parry")] / 20;
    }

    /* STR mod for high str victim */
    chance += get_curr_str(victim) - get_curr_str(ch);

    /* Mod for what attack this is in a round */
    chance -= (attackCount - 1) * 4;

    if(IS_AFFECTED2(victim, AFF_BERSERK) || IS_AFFECTED3(victim, AFF_BLOODTHIRSTY) )
    {
	chance /= 2;
    }

    if ( ch->wait == 0 )
	chance /= 2;

    if ( victim->wait != 0 )
	chance /= 2;

    if ( !IS_NPC( victim ) )
    {
	if ( chance < victim->pcdata->learned[skill_lookup("parry")] / 50 )
	{
	    chance = victim->pcdata->learned[skill_lookup("parry")] / 50;
	}
    }

    if ( ( number_percent( ) >= chance ) || IS_AFFECTED2( ch, AFF_BERSERK ) || IS_AFFECTED3( ch, AFF_BLOODTHIRSTY ) )
	return FALSE;

    update_skpell( victim, skill_lookup("parry"), 0 );

    if ( IS_SET( ch->act, PLR_COMBAT ) )
      act(AT_GREEN, "$N parries your attack.", ch, NULL, victim, TO_CHAR    );
    if ( IS_SET( victim->act, PLR_COMBAT ) )
      act(AT_GREEN, "You parry $n's attack.",  ch, NULL, victim, TO_VICT    );
    return TRUE;
}

bool check_enhanced_parry( CHAR_DATA *ch, CHAR_DATA *victim, int attackCount )
{
    int chance;

    if ( !IS_AWAKE( victim ) )
	return FALSE;

    if ( IS_NPC( victim ) )
    {
	return FALSE;
    }
    else
    {
	if ( !get_eq_char( victim, WEAR_WIELD ) )
	    return FALSE;
	if (!can_use_skspell( victim, skill_lookup("enhanced parry")))
	{
	    return FALSE;
	}
	chance	= victim->pcdata->learned[skill_lookup("enhanced parry")] / 40;
    }

    /* STR mod for high str victim */
    chance += get_curr_str(victim) - get_curr_str(ch);

    /* Mod for what attack this is in a round */
    chance -= (attackCount - 1) * 4;

    if(IS_AFFECTED2(victim, AFF_BERSERK) || IS_AFFECTED3(victim, AFF_BLOODTHIRSTY) )
    {
	chance /= 2;
    }

    if ( ch->wait == 0 )
	chance /= 2;

    if ( victim->wait != 0 )
	chance /= 2;

    if ( !IS_NPC( victim ) )
    {
	if ( chance < victim->pcdata->learned[skill_lookup("enhanced parry")] / 50 )
	{
	    chance = victim->pcdata->learned[skill_lookup("enhanced parry")] / 50;
	}
    }

    if ( ( number_percent( ) >= chance ) || IS_AFFECTED2( ch, AFF_BERSERK ) || IS_AFFECTED3( ch, AFF_BLOODTHIRSTY ) )
	return FALSE;

    update_skpell( victim, skill_lookup("enhanced parry"), 0 );

    if ( IS_SET( ch->act, PLR_COMBAT ) )
      act(AT_GREEN, "$N parries your attack.", ch, NULL, victim, TO_CHAR    );
    if ( IS_SET( victim->act, PLR_COMBAT ) )
      act(AT_GREEN, "You parry $n's attack.",  ch, NULL, victim, TO_VICT    );
    return TRUE;
}

bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim, int attackCount )
{
    int chance;

    if ( !IS_AWAKE( victim ) )
	return FALSE;

    if ( !get_eq_char( victim, WEAR_SHIELD ) )
		return FALSE;

    if ( IS_NPC( victim ) )
    {
	chance	= UMIN( 50, 2 * victim->level );
    }
    else
    {
	if( victim->pcdata->learned[skill_lookup("shield block")] < 300 )
	{
	    chance = number_range( 1, 15 );
	}
	else
	{
	    chance = victim->pcdata->learned[skill_lookup("shield block")] / 20;
	}
    }

    if ( ch->wait == 0 )
    	chance /= 2;

    if ( victim->wait != 0 )
	chance /= 2;

    /* STR mod for high str victim */
    chance += get_curr_str(victim) - get_curr_str(ch);

    /* Mod for what attack this is in a round */
    chance -= (attackCount - 1) * 4;

    if(IS_AFFECTED2(victim, AFF_BERSERK) || IS_AFFECTED3(victim, AFF_BLOODTHIRSTY) )
    {
	chance /= 2;
    }

    if ( number_percent( ) >= chance + 10 )
	return FALSE;

    update_skpell( victim, skill_lookup("shield block"), 0 );

    if ( IS_SET( ch->act, PLR_COMBAT ) )
      act(AT_GREEN, "$N's shield blocks your attack.", ch, NULL, victim, TO_CHAR    );
    if ( IS_SET( victim->act, PLR_COMBAT ) )
      act(AT_GREEN, "Your shield blocks $n's attack.",  ch, NULL, victim, TO_VICT    );
    return TRUE;
}

/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim, int attackCount )
{
    int chance;

    if ( !IS_AWAKE( victim ) )
	return FALSE;

    if ( IS_NPC( victim ) )
    {
	return FALSE;
    }
    else
    {
	if (!can_use_skspell( victim, skill_lookup("dodge")))
	{
	    return FALSE;
	}
	chance	= victim->pcdata->learned[skill_lookup("dodge")] / 20;
    }

    /* DEX mod for high str victim */
    chance += get_curr_dex(victim) - get_curr_dex(ch);

    /* Mod for what attack this is in a round */
    chance -= (attackCount - 1) * 4;

    if( can_use_skspell( ch, skill_lookup("wilderness lore") ) )
    {
        if( !IS_NPC( ch ) )
        {
	    if( ( number_percent( ) < ( ch->pcdata->learned[skill_lookup("wilderness master")] / 10 ) ) && IS_OUTSIDE( ch ) && IS_IN_NATURE( ch ) )
	    {
		chance += ch->pcdata->learned[skill_lookup("wilderness lore")] / 80;
		update_skpell( ch, skill_lookup("wilderness lore"), 0 );
	    }
	}
    }

    if(IS_AFFECTED2(victim, AFF_BERSERK) || IS_AFFECTED3(victim, AFF_BLOODTHIRSTY) )
    {
	chance /= 2;
    }

    if ( ch->wait == 0 )
	chance /= 2;

    if ( victim->wait != 0 )
	chance /= 2;

    if ( !IS_NPC( victim ) )
    {
	if ( chance < victim->pcdata->learned[skill_lookup("dodge")] / 50 )
	{
	    chance = victim->pcdata->learned[skill_lookup("dodge")] / 50;
	}
    }

    if ( ( number_percent( ) >= chance ) || IS_AFFECTED2( ch, AFF_BERSERK ) || IS_AFFECTED3( ch, AFF_BLOODTHIRSTY ) )
	return FALSE;

    update_skpell( victim, skill_lookup("dodge"), 0 );

    if ( IS_SET( ch->act, PLR_COMBAT ) )
      act(AT_GREEN, "$N dodges your attack.", ch, NULL, victim, TO_CHAR    );
    if ( IS_SET( victim->act, PLR_COMBAT ) )
      act(AT_GREEN, "You dodge $n's attack.", ch, NULL, victim, TO_VICT    );
    return TRUE;
}

bool check_enhanced_dodge( CHAR_DATA *ch, CHAR_DATA *victim, int attackCount )
{
    int chance;

    if ( !IS_AWAKE( victim ) )
	return FALSE;

    if ( IS_NPC( victim ) )
    {
	return FALSE;
    }
    else
    {
	if (!can_use_skspell( victim, skill_lookup("enhanced dodge")))
	{
	    return FALSE;
	}
	chance	= victim->pcdata->learned[skill_lookup("enhanced dodge")] / 40;
    }

    /* DEX mod for high str victim */
    chance += get_curr_dex(victim) - get_curr_dex(ch);

    /* Mod for what attack this is in a round */
    chance -= (attackCount - 1) * 4;

    if(IS_AFFECTED2(victim, AFF_BERSERK) || IS_AFFECTED3(victim, AFF_BLOODTHIRSTY) )
    {
	chance /= 2;
    }

    if ( ch->wait == 0 )
	chance /= 2;

    if ( victim->wait != 0 )
	chance /= 2;

    if ( !IS_NPC( victim ) )
    {
	if ( chance < victim->pcdata->learned[skill_lookup("enhanced dodge")] / 50 )
	{
	    chance = victim->pcdata->learned[skill_lookup("enhanced dodge")] / 50;
	}
    }

    if ( ( number_percent( ) >= chance ) || IS_AFFECTED2( ch, AFF_BERSERK ) || IS_AFFECTED3( ch, AFF_BLOODTHIRSTY ) )
	return FALSE;

    update_skpell( victim, skill_lookup("enhanced dodge"), 0 );

    if ( IS_SET( ch->act, PLR_COMBAT ) )
      act(AT_GREEN, "$N dodges your attack.", ch, NULL, victim, TO_CHAR    );
    if ( IS_SET( victim->act, PLR_COMBAT ) )
      act(AT_GREEN, "You dodge $n's attack.", ch, NULL, victim, TO_VICT    );
    return TRUE;
}

/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
    	if ( victim->position < POS_STUNNED )
	    victim->position = POS_STANDING;
		return;
    }

    if ( IS_NPC( victim ) || victim->hit <= -11 )
    {
	//raw_kill( victim, victim );
	victim->position = POS_DEAD;
	return;
    }

         if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{

    char buf [ MAX_STRING_LENGTH ];

    if ( ch->fighting )
    {
	bug( "Set_fighting: already fighting", 0 );
	sprintf( buf, "...%s attacking %s at %d",
		( IS_NPC( ch )     ? ch->short_descr     : ch->name     ),
		( IS_NPC( victim ) ? victim->short_descr : victim->name ),
		victim->in_room->vnum );
	bug( buf , 0 );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_SLEEP ) )
	affect_strip( ch, skill_lookup("sleep") );

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    for ( fch = char_list; fch; fch = fch->next )
    {
	if ( fch == ch || ( fBoth && fch->fighting == ch ) )
	{
	    fch->fighting	= NULL;
	    /* fch->hunting        = NULL; */
	    fch->position	= POS_STANDING;
	    fch->initiated	= FALSE;

	    if ( is_affected( fch, skill_lookup("berserk") ) )
	    {
	      affect_strip( fch, skill_lookup("berserk") );
	      send_to_char(C_DEFAULT, skill_table[skill_lookup("berserk")].msg_off,fch);
	      send_to_char(C_DEFAULT, "\n\r",fch);
	    }
	    if ( IS_AFFECTED2(fch, AFF_BERSERK) )
	      REMOVE_BIT(fch->affected_by2, AFF_BERSERK);
	    if ( is_affected( fch, skill_lookup("bloodthirsty") ) )
	    {
	      affect_strip( fch, skill_lookup("bloodthirsty") );
	      send_to_char(C_DEFAULT, skill_table[skill_lookup("bloodthirsty")].msg_off,fch);
	      send_to_char(C_DEFAULT, "\n\r",fch);
	    }
	    if ( IS_AFFECTED3(fch, AFF_BLOODTHIRSTY) )
	      REMOVE_BIT(fch->affected_by3, AFF_BLOODTHIRSTY);
	    update_pos( fch );
	}
    }

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch )
{
    OBJ_DATA        *corpse;
    OBJ_DATA        *obj;
    OBJ_DATA        *obj_next;
    char            *name;
    char             buf [ MAX_STRING_LENGTH ];

    if ( IS_NPC( ch ) )
    {
        /*
	 * This longwinded corpse creation routine comes about because
	 * we dont want anything created AFTER a corpse to be placed
	 * INSIDE a corpse.  This had caused crashes from obj_update()
	 * in extract_obj() when the updating list got shifted from
	 * object_list to obj_free.          --- Thelonius (Monk)
	 */

	if ( ch->gold > 0 )
	{
	    OBJ_DATA * coins;

	    coins         = create_money( ch->gold );
	    name	  = ch->short_descr;
	    if( IS_SET( ch->act, ACT_VEHICLE ) )
	    {
		corpse	  = create_object(
					  get_obj_index( OBJ_VNUM_CORPSE_VEHICLE ),
					  0 );
		corpse->value[0] = TAN_NONE;
		corpse->value[1] = 0;
	    }
	    else
	    {
		corpse	  = create_object(
					  get_obj_index( OBJ_VNUM_CORPSE_NPC ),
					  0 );
		
		corpse->value[0] = TAN_NPC;
		corpse->value[1] = ch->pIndexData->vnum;
	    }
	    corpse->timer = number_range( 2, 4 );
	    obj_to_obj( coins, corpse );
	    ch->gold = 0;
	}
	else
	{
	    name	  = ch->short_descr;
	    if( IS_SET( ch->act, ACT_VEHICLE ) )
	    {
		corpse	  = create_object(
					  get_obj_index( OBJ_VNUM_CORPSE_VEHICLE ),
					  0 );
		corpse->value[0] = TAN_NONE;
		corpse->value[1] = 0;
	    }
	    else
	    {
		corpse	  = create_object(
					  get_obj_index( OBJ_VNUM_CORPSE_NPC ),
					  0 );
		corpse->level    = ch->pIndexData->level;
		corpse->value[0] = TAN_NPC;
		corpse->value[1] = ch->pIndexData->vnum;

	    }
	    corpse->timer = number_range( 2, 4 );
	}
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(
					get_obj_index( OBJ_VNUM_CORPSE_PC ),
					0 );
	corpse->value[0] = TAN_PC;
	corpse->value[1] = ch->level;
	corpse->timer	= number_range( 25, 40 );
	if ( ( ch->gold > 0 ) && ( ch->level > 5 ) )
	{
	  OBJ_DATA * coins;
	  coins = create_money( ch->gold );
	  obj_to_obj( coins, corpse );
	  ch->gold = 0;
	}
    }

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj; obj = obj_next )
    {
	obj_next = obj->next_content;

        if ( obj->deleted )
	    continue;
	obj_from_char( obj );
	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    extract_obj( obj );
	else
	    obj_to_obj( obj, corpse );
    }

    /* quill */
    if (  ( ch->level <= 50 && number_percent() <= 1 && number_percent() < 10)
	||( ch->level >  50 && ch->level < 90        && number_percent() < 2 )
	||( ch->level >= 90 && number_percent() < 3 ) )
    {
	obj = create_object( get_obj_index( OBJ_VNUM_QUILL ) , 1);
	obj_to_obj(obj, corpse);
    }

    /* pestle */
    if (  ( ch->level <= 50 && number_percent() <= 1 && number_percent() < 10)
	||( ch->level >  50 && ch->level < 90        && number_percent() < 2 )
	||( ch->level >= 90 && number_percent() < 3 ) )
    {
	obj = create_object( get_obj_index( OBJ_VNUM_PESTLE ) , 1);
	obj_to_obj(obj, corpse);
    }

    /* needle */
    if (  ( ch->level <= 50 && number_percent() <= 1 && number_percent() < 10)
	||( ch->level >  50 && ch->level < 90        && number_percent() < 2 )
	||( ch->level >= 90 && number_percent() < 3 ) )
    {
	obj = create_object( get_obj_index( OBJ_VNUM_NEEDLE ) , 1);
	obj_to_obj(obj, corpse);
    }


    sprintf( buf, "NPC killed: %s -> %d", ch->name, ch->in_room->vnum );
    log_string( buf, CHANNEL_NONE, -1 );
    if ( ( IS_NPC( ch ) ) && ( !IS_SET( ch->act, PLR_UNDEAD ) ) )
       corpse->ac_vnum=ch->pIndexData->vnum;
    if( !IS_SET( ch->act, ACT_NO_CORPSE ) )
    {
	obj_to_room( corpse, ch->in_room );
    }
    else
    {
	extract_obj( corpse );
    }

    if( !IS_NPC( ch ) )
    {
	corpse_back(ch, corpse );
    }

    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char            *msg;
    int              vnum;
    int              door;

    vnum = 0;
    switch ( number_bits( 4 ) )
    {
    default: msg  = "You hear $n's death cry.";				break;
    case  0: msg  = "$n hits the ground ... DEAD.";			break;
    case  1: msg  = "$n splatters blood on your armor.";		break;
    case  2: msg  = "$n's innards fall to the ground with a wet splat.";     break;
    case  3: msg  = "$n's severed head plops on the ground.";
	     vnum = OBJ_VNUM_SEVERED_HEAD;				break;
    case  4: msg  = "$n's heart is torn from $s chest.";
	     vnum = OBJ_VNUM_TORN_HEART;				break;
    case  5: msg  = "$n's arm is sliced from $s dead body.";
	     vnum = OBJ_VNUM_SLICED_ARM;				break;
    case  6: msg  = "$n's leg is sliced from $s dead body.";
	     vnum = OBJ_VNUM_SLICED_LEG;				break;
    }

    act(AT_BLOOD, msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum != 0 )
    {
	OBJ_DATA *obj;
	char     *name;
	char      buf [ MAX_STRING_LENGTH ];

	name		= IS_NPC( ch ) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	obj_to_room( obj, ch->in_room );
    }

    vnum = OBJ_VNUM_FINAL_TURD;
    if ( vnum != 0 )
    {
	OBJ_DATA *obj;

	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 3, 5 );

	obj_to_room( obj, ch->in_room );
    }
    if ( IS_NPC( ch ) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] )
	    && pexit->to_room
	    && pexit->to_room != was_in_room )
	{
	    ch->in_room = pexit->to_room;
	    act(AT_BLOOD, msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    ROOM_INDEX_DATA *location;

	if (victim->fighting)
		stop_fighting( victim, TRUE );
    if ( IS_NPC( victim) )
    {
        mprog_death_trigger( victim, ch );
    }
    rprog_death_trigger( victim->in_room, victim );
    make_corpse( victim );

    for ( paf = victim->affected; paf; paf = paf_next )
        {
     	 paf_next = paf->next;
         affect_remove( victim, paf );
        }
    for ( paf = victim->affected2; paf; paf = paf_next )
      {
       paf_next = paf->next;
       affect_remove2( victim, paf );
      }
    for ( paf = victim->affected3; paf; paf = paf_next )
      {
       paf_next = paf->next;
       affect_remove3( victim, paf );
      }
    for ( paf = victim->affected4; paf; paf = paf_next )
      {
       paf_next = paf->next;
       if (paf->type != skill_lookup("newbie slayer"))
	       affect_remove4( victim, paf );
      }
    for ( paf = victim->affected_powers; paf; paf = paf_next )
      {
       paf_next = paf->next;
       affect_remove_powers( victim, paf );
      }
    for ( paf = victim->affected_weaknesses; paf; paf = paf_next )
      {
       paf_next = paf->next;
       affect_remove_weaknesses( victim, paf );
      }

    victim->affected_by	= 0;
    victim->affected_by2 &= CODER;
    victim->affected_by3 = 0;
    victim->affected_by4 = 0;
    victim->affected_by_powers = 0;
    victim->affected_by_weaknesses = 0;
    victim->shields = 0;
    victim->poison_level = 0;
    if(!IS_NPC(victim) )
    {
	victim->pcdata->mod_str = 0;
	victim->pcdata->mod_int = 0;
	victim->pcdata->mod_dex = 0;
	victim->pcdata->mod_wis = 0;
	victim->pcdata->mod_con = 0;
    }
    if ( IS_NPC( victim ) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE( 0, victim->level, MAX_LEVEL-1 )].killed++;
	extract_char( victim, TRUE );
	return;
    }

    location = ch->in_room;
    extract_char( victim, FALSE );
    if( IS_SET( victim->act, PLR_GHOST ) )
    {
	char_from_room ( victim );
	char_to_room( victim, location );
    }

    victim->armor        = 100;
    if(IS_SET(victim->act, PLR_GHOST ) )
    {
        victim->position = POS_GHOST;
	victim->speaking = SPIRITSPEAK;
    }
    else
    {
        victim->position     = POS_RESTING;
    }
/*
    victim->hit	         = UMAX( 1, victim->hit  );
    victim->mana         = UMAX( 1, victim->mana );
    victim->bp           = UMAX( 1, victim->bp   );
    victim->move         = UMAX( 1, victim->move );
*/
    victim->hit		 = 1;
    victim->mana	 = 1;
    victim->bp		 = 1;
    victim->move	 = 1;
    victim->hitroll      = 0;
    victim->damroll      = 0;
    victim->saving_throw = 0;
    victim->carry_weight = 0;
    victim->carry_number = 0;
    victim->mounted	 = 0;
    victim->mountshort   = "";
    victim->mountcharmed = 0;

    victim->damage_mods[0] = 0;
    victim->damage_mods[1] = 0;
    victim->damage_mods[2] = 0;
    victim->damage_mods[3] = 0;
    victim->damage_mods[4] = 0;
    victim->damage_mods[5] = 0;
    victim->damage_mods[6] = 0;
    victim->damage_mods[7] = 0;
    victim->damage_mods[8] = 0;
    victim->damage_mods[9] = 0;
    victim->damage_mods[10] = 0;
    victim->damage_mods[11] = 0;
    victim->damage_mods[12] = 0;
    victim->damage_mods[13] = 0;
    victim->damage_mods[14] = 0;
    victim->damage_mods[15] = 0;
    victim->damage_mods[16] = 0;
    victim->damage_mods[17] = 0;
    victim->damage_mods[18] = 0;
    victim->damage_mods[19] = 0;

    save_char_obj( victim, FALSE );
    return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    char       buf[ MAX_STRING_LENGTH ];
    int        members;
    int        xp;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( IS_NPC( ch ) || victim == ch )
	return;

   /*
    * Killing in an arena won't get you anywhere either...
    * Arena's are for fun not experience...
    *                                           -- The Maniac! --
    */
    /* if ( !IS_NPC ( victim ) )
        return; */

    /*
     * If you're fighting a PC, and your victim is peaceful
     * then no xp for you!
     */
    if ((!IS_NPC(victim)) && (!victim->pkill))
	return;

    members = 0;
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
	    members++;
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
    }

    lch = ( ch->leader ) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_same_group( gch, ch ) )
	    continue;

	// Newbie slayer code.
	// If victim is single class, and killer is multi. Tag with newbie slayer.
	if ( !IS_NPC(victim) && victim->class == victim->multied && gch->class != gch->multied && !victim->initiated)
	{
		tag_newbie_slayer( gch );
	}

	gch->initiated = FALSE;

        if (IS_SET(gch->act, PLR_QUESTOR)&&IS_NPC(victim))
        {
            if (gch->questmob == victim->pIndexData->vnum)
            {
                send_to_char(AT_WHITE, "You have almost completed your QUEST!\n\r",ch);
                send_to_char(AT_WHITE, "Return to the questmaster before your time runs out!\n\r",ch);
                ch->questmob = -1;
            }
        } else if(IS_SET(gch->act2, PLR_RELQUEST)&&IS_NPC(victim))
	{
	    if (gch->rquestmob[gch->relquest_level] == victim->pIndexData->vnum)
	    {
		DESCRIPTOR_DATA* d;
		for (d=descriptor_list; d; d = d->next)
                {
                	if ( d->connected != CON_PLAYING )
                        	continue;
                	if (is_same_group(d->character, gch))
                	{
				send_to_char(AT_WHITE, "This section of your religious quest is almost over!\n\r", d->character);
				d->character->rquestmob[gch->relquest_level] = -1;
                        }
                }

	    }
	}


	if ( gch->level - lch->level >= 6 )
	{
	    send_to_char(AT_BLUE, "You are too high level for this group.\n\r", gch );
	    continue;
	}

	if ( gch->level - lch->level <= -6 )
	{
	    send_to_char(AT_BLUE, "You are too low level for this group.\n\r",  gch );
	    continue;
	}

        // No xp for non pk's in pk fights
        if (!gch->pkill && !IS_NPC(victim))
        {
           continue;
        }

	if (((gch->level - victim->level) < -PKILL_RANGE) 
		&& (!IS_NPC(victim)))
        {
           send_to_char(AT_BLUE, "You must be within pkill level range for pkill xp.\n\r", gch);
	   continue;
        }

	if (((gch->level - victim->level) >= PKILL_RANGE) && (!IS_NPC(victim)))
        {
           send_to_char(AT_BLUE, "You must be within pkill level range for pkill xp.\n\r", gch);
	   continue;
        }

	if (members > 1)
        {
		// 150% xp because of group
		xp = (int) ((float) (xp_compute( gch, victim ) / members) * 1.5);

		// 10% bonus for leader
		if ( gch == lch )
      		{
      	   		xp = (int) ((float) xp * 1.1);
		}

		// 10% bonus for killer
		if ( ch == gch )
        	{
      	   		xp = (int) ((float) xp * 1.1);
		}

		// 10% bonus for mob's target
		if (gch == victim->fighting)
		{
      		   xp = (int) ((float) xp * 1.1);
		}
 	} else
        {
           xp = xp_compute(gch, victim);
        }

	if ( IS_AFFECTED4( gch, AFF_LUCK_SHIELD ) )
	{
	    if( gch->level < LEVEL_CHAMP )
	    {	
		xp += number_range( 50, 400 );
	    }
/*	    if( gch->level == LEVEL_CHAMP || gch->level == LEVEL_DEMIGOD ) */
	    if( gch->level == LEVEL_CHAMP )
 	    {
		xp += number_range( 50, 200 );
	    }
	    send_to_char(AT_YELLOW, "The luck of the gods grants you experience.\n\r", gch );
	}

	if (!gch->pkill || ( gch->level == LEVEL_DEMIGOD ) )
	{
        if ( xp > 450 )
          {
	    xp = 450;
          }

	if (gch->multied != gch->class )
	{
            xp /= 2;
	}

	if (gch->level == 1 || gch->level == 2 )
	{
	    xp *= 2;
	}
	if (gch->level == 3 || gch->level == 4 )
	{
	    xp *= 3 / 2;
	}

	}
        if ( !str_infix( race_table[lch->race].race_full,
                        race_table[gch->race].hate) && members > 1 )
          {
            send_to_char(AT_WHITE, "You lost a third of your exps due to grouping with scum.\n\r", ch );
            xp = (int)(xp * .66);
          }
        sprintf( buf, "%s -> gains %dxp", gch->name, xp);
        log_string( buf, CHANNEL_NONE, -1 );
	sprintf( buf, "You receive %d experience points.\n\r", xp );
	send_to_char(AT_WHITE, buf, gch );
	gain_exp( gch, xp );

	for ( obj = ch->carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->deleted )
	        continue;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

            if ( !IS_NPC( ch ) )
	    if (   ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL    )
		    && IS_EVIL   ( ch ) )
		|| ( IS_OBJ_STAT( obj, ITEM_ANTI_GOOD    )
		    && IS_GOOD   ( ch ) )
		|| ( IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL )
		    && IS_NEUTRAL( ch ) ) )
	    {
		act(AT_BLUE, "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
		act(AT_BLUE, "$n is zapped by $p.",   ch, obj, NULL, TO_ROOM );
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
	    }
	}
    }

    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;
    float     bonus;
    int xp;
    int align;
    int extra;
    int level;
    int number;

    bonus = 1.0;
    xp    = 150 - URANGE( -30, gch->level - victim->level, 5 ) * 30;
    align = gch->alignment - victim->alignment;

    if ( align >  500 )
    {
	gch->alignment  = UMIN( gch->alignment + ( align - 500 ) / 4,  1000 );
	xp = 5 * xp / 4;
    }
    else if ( align < -500 )
    {
	gch->alignment  = UMAX( gch->alignment + ( align + 500 ) / 4, -1000 );
	xp = 5 * xp / 4;
    }
    else
    {
	gch->alignment -= victim->alignment / 3;
	xp = 3 * xp / 4;
    }

    if ( IS_AFFECTED( victim, AFF_SANCTUARY )
        || IS_SET( race_table[ victim->race ].race_abilities, RACE_SANCT ) )
    {
        bonus += 1.0/2;
    }
    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) )
    {
        bonus += 1.0/4;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD_2 ) ) )
    {
        bonus +=(float) (1.0/5);
    }

    if ( !str_infix( race_table[victim->race].race_full,
                    race_table[gch->race].hate ) )
    {
        bonus += (float)(1.0/10);
    }
    if ( victim->race == gch->race )
    {
        bonus -= (float)(1.0/8);
    }
    if ( IS_NPC( victim ) )
    {
        if ( IS_SET( victim->act, ACT_AGGRESSIVE ) )
        {
            bonus += (float)(1.0/20);
        }

        if ( victim->pIndexData->pShop != 0 )
            bonus -= (float)(1.0/4);

        if ( victim->spec_fun != 0 )
        {
            if (   victim->spec_fun == spec_lookup( "spec_breath_any"       )
                || victim->spec_fun == spec_lookup( "spec_cast_psionicist"  )
                || victim->spec_fun == spec_lookup( "spec_cast_undead"      )
                || victim->spec_fun == spec_lookup( "spec_breath_gas"       )
                || victim->spec_fun == spec_lookup( "spec_cast_mage"        ) )
                bonus += (float)(1.0/3);

            if (   victim->spec_fun == spec_lookup( "spec_breath_fire"      )
                || victim->spec_fun == spec_lookup( "spec_breath_cold"      )
                || victim->spec_fun == spec_lookup( "spec_breath_acid"      )
                || victim->spec_fun == spec_lookup( "spec_breath_lightning" )
                || victim->spec_fun == spec_lookup( "spec_cast_cleric"      )
                || victim->spec_fun == spec_lookup( "spec_cast_judge"       )
                || victim->spec_fun == spec_lookup( "spec_cast_ghost"       ) )
                bonus += (float)(1.0/5);

            if (   victim->spec_fun == spec_lookup( "spec_poison"           )
                || victim->spec_fun == spec_lookup( "spec_thief"            ) )
                bonus += (float)(1.0/20);

            if ( victim->spec_fun == spec_lookup( "spec_cast_adept"         ) )
                bonus -= (float)(1.0/2);
        }
    }
    else
    {
            bonus *= 2;
    }
    xp = (int) (xp * bonus);



    /*
     * Adjust for popularity of target:
     *   -1/8 for each target over  'par' (down to - 50%)
     *   +1/8 for each target under 'par' (  up to + 25%)
     */
    level  = URANGE( 0, victim->level, 99 );
    number = UMAX( 1, kill_table[level].number );
    if(IS_NPC(victim))
      extra  = victim->pIndexData->killed - kill_table[level].killed / number;
    else
      extra = 0;
    xp    -= xp * URANGE( -2, extra, 4 ) / 8;

    xp     = number_range( xp * 3 / 4, xp * 5 / 4 );
    xp     = UMAX( 0, xp );

    if(IS_NPC(victim) && IS_SET(victim->act, ACT_NO_EXP ) )
    {
	xp = 0;
    }

    if( ( victim->level < LEVEL_HERO ) && gch->level >= LEVEL_CHAMP )
    {
	xp = 0;
    }
    return xp;

}



void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    static char * const  attack_table [ ] =
    {
	"hit",
	"slice",  "stab",    "slash", "whip", "claw",
	"blast",  "pound",   "crush", "grep", "bite",
	"pierce", "suction", "chop", "blast", "shot"
    };
    static char * const  damclass_attack_table [ ] =
    {
	"%s hit",
	"%s slice",  "%s stab",    "%s slash", "%s whip", "%s claw",
	"%s blast",  "%s pound",   "%s crush", "%s grep", "%s bite",
	"%s pierce", "%s suction", "%s chop", "%s blast", "%s shot"
    };

    const  char         *vs;
    const  char         *vp;
    const  char         *attack;
           char          buf          [ MAX_STRING_LENGTH ];
           char          buf1         [ 256 ];
           char          buf2         [ 256 ];
           char          buf3         [ 256 ];
           char          buf4         [ 256 ];
           char          buf5         [ 256 ];
	   char		 damclassbuf  [ MAX_STRING_LENGTH ];
           char          punct;

	 if ( dam <=   0 ) { vs = "miss";	vp = "misses";		}
    else if ( dam <=   4 ) { vs = "scratch";	vp = "scratches";	}
    else if ( dam <=   8 ) { vs = "graze";	vp = "grazes";		}
    else if ( dam <=  12 ) { vs = "hit";	vp = "hits";		}
    else if ( dam <=  16 ) { vs = "injure";	vp = "injures";		}
    else if ( dam <=  20 ) { vs = "wound";	vp = "wounds";		}
    else if ( dam <=  24 ) { vs = "maul";       vp = "mauls";		}
    else if ( dam <=  28 ) { vs = "decimate";	vp = "decimates";	}
    else if ( dam <=  32 ) { vs = "devastate";	vp = "devastates";	}
    else if ( dam <=  36 ) { vs = "maim";	vp = "maims";		}
    else if ( dam <=  40 ) { vs = "MUTILATE";	vp = "MUTILATES";	}
    else if ( dam <=  44 ) { vs = "DISEMBOWEL";	vp = "DISEMBOWELS";	}
    else if ( dam <=  48 ) { vs = "EVISCERATE";	vp = "EVISCERATES";	}
    else if ( dam <=  52 ) { vs = "MASSACRE";	vp = "MASSACRES";	}
    else if ( dam <= 100 ) { vs = "*** DEMOLISH ***";
			     vp = "*** DEMOLISHES ***";			}
    else if ( dam <= 150 ) { vs = "*** DEVASTATE ***";
			     vp = "*** DEVASTATES ***";			}
    else if ( dam <= 250 ) { vs = "*** OBLITERATE ***";
			     vp = "*** OBLITERATES ***";		}
    else if ( dam <= 300 ) { vs = "=== OBLITERATE ===";
			     vp = "=== OBLITERATES ===";		}
    else if ( dam <= 500 ) { vs = "*** ANNIHILATE ***";
			     vp = "*** ANNIHILATES ***";		}
    else if ( dam <= 750 ) { vs = ">>> ANNIHILATE <<<";
			     vp = ">>> ANNIHILATES <<<";		}
    else if ( dam <= 1000) { vs = "<<< ERADICATE >>>";
			     vp = "<<< ERADICATES >>>";			}
    else if ( dam <= 2000) { vs = "do UNSPEAKABLE things to";
			     vp = "does UNSPEAKABLE things to";		}
    else if ( dam <= 6000) { vs = "open a can of WOOP ASS on";
			     vp = "opens a can of WOOP ASS on";         }
    else                   { vs = "deal GODLIKE DAMAGE to";
			     vp = "deals GODLIKE DAMAGE to";    	}

    punct   = ( dam <= 24 ) ? '.' : '!';

    if ( dt == TYPE_HIT )
    {
        if ( ch->race > MAX_RACE )
        {
            bug( "Dam_message:  %d invalid race", ch->race );
            ch->race = 0;
        }

        attack = race_table[ ch->race ].dmg_message;

        sprintf( buf1, "Your %s %s $N%c (&R%d&W pts)",	attack, vp, punct, dam );
        sprintf( buf2, "$n's %s %s you%c (&R%d&W pts)",	attack, vp, punct, dam );
        sprintf( buf3, "$n's %s %s $N%c (&R%d&W pts)",	attack, vp, punct, dam );
        sprintf( buf4, "You %s %s yourself%c (&R%d&W pts)",	attack, vp, punct, dam );
        sprintf( buf5, "$n's %s %s $m%c (&R%d&W pts)",	attack, vp, punct, dam );
    }
    else
    {
	if ( is_sn(dt) )
	    attack	= skill_table[dt].noun_damage;
	else if (   dt >= TYPE_HIT
		 && dt  < TYPE_HIT
		        + sizeof( attack_table )/sizeof( attack_table[0] ) )
	{
   	  if (!dam_class_value)
	    attack	= attack_table[dt - TYPE_HIT];
	  else
	  {
	    sprintf(damclassbuf, damclass_attack_table[dt - TYPE_HIT], flag_string( damage_flags, dam_class_value ) );
	    attack = damclassbuf; 	
	  }
	}
	else
	{
	    sprintf( buf, "Dam_message: bad dt %d caused by %s.", dt,
		    ch->name );
	    bug( buf, 0 );
	    dt      = TYPE_HIT;
	    attack  = attack_table[0];
	}

	if ( dt > TYPE_HIT && is_wielding_poisoned( ch ) )
	{
	    sprintf( buf1, "Your poisoned %s %s $N%c (&R%d&W pts)", attack, vp, punct, dam );
	    sprintf( buf2, "$n's poisoned %s %s you%c (&R%d&W pts)", attack, vp, punct, dam );
	    sprintf( buf3, "$n's poisoned %s %s $N%c (&R%d&W pts)", attack, vp, punct, dam );
	    sprintf( buf4, "Your poisoned %s %s you%c (&R%d&W pts)", attack, vp, punct, dam );
	    sprintf( buf5, "$n's poisoned %s %s $n%c (&R%d&W pts)", attack, vp, punct, dam );
	}
	else
	{
	    sprintf( buf1, "Your %s %s $N%c (&R%d&W pts)",  attack, vp, punct, dam );
	    sprintf( buf2, "$n's %s %s you%c (&R%d&W pts)", attack, vp, punct, dam );
	    sprintf( buf3, "$n's %s %s $N%c (&R%d&W pts)",  attack, vp, punct, dam );
	    sprintf( buf4, "Your %s %s you%c (&R%d&W pts)", attack, vp, punct, dam );
	    sprintf( buf5, "$n's %s %s $n%c (&R%d&W pts)",  attack, vp, punct, dam );
	}
    }

    if ( victim != ch )
    {
        if ( dam != 0 || IS_SET( ch->act, PLR_COMBAT ) )
	  act(AT_WHITE, buf1, ch, NULL, victim, TO_CHAR    );
	if ( dam != 0 || IS_SET( victim->act, PLR_COMBAT ) )
	  act(AT_WHITE, buf2, ch, NULL, victim, TO_VICT    );
	act(AT_GREY, buf3, ch, NULL, victim,
	    dam == 0 ? TO_COMBAT : TO_NOTVICT );
    }
    else
    {
        if ( dam != 0 || IS_SET( ch->act, PLR_COMBAT ) )
	  act(AT_WHITE, buf4, ch, NULL, victim, TO_CHAR    );
  	act(AT_GREY, buf5, ch, NULL, victim,
	    dam == 0 ? TO_COMBAT : TO_NOTVICT );
     }

    return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( race_table[ ch->race ].size - race_table[ victim->race ].size )
        < -2 )
        return;

    if ( !( obj = get_eq_char( victim, WEAR_WIELD ) ) )
	if ( !( obj = get_eq_char( victim, WEAR_WIELD_2 ) ) )
	   return;

    if ( !get_eq_char( ch, WEAR_WIELD ) && number_bits( 1 ) == 0 )
	if ( !get_eq_char( ch, WEAR_WIELD_2 ) && number_bits( 1 ) == 0 )
	   return;

    act(AT_YELLOW, "You disarm $N!",  ch, NULL, victim, TO_CHAR    );
    act(AT_YELLOW, "$n DISARMS you!", ch, NULL, victim, TO_VICT    );
    act(AT_GREY, "$n DISARMS $N!",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );
    if ( IS_NPC( victim ) )
	obj_to_char( obj, victim );
    else
	obj_to_char( obj, victim );
    return;
}


void do_kill( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
      send_to_char( AT_WHITE, "That person is not here.\n\r", ch );
      return;
    }

    if ( is_safe( ch, victim ) )
      {
       send_to_char( AT_WHITE, "You cannot.\n\r", ch );
       return;
      }

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Kill whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(victim, AFF_PEACE ))
    {
      send_to_char(AT_WHITE, "A wave of peace overcomes you.\n\r", ch);
      return;
    }
    if ( IS_AFFECTED2( ch, AFF_SHADOW_PLANE ))
    {
      send_to_char(AT_WHITE, "You must leave the shadow realm.\n\r", ch);
      return;
    }
    if ( IS_AFFECTED4( ch, AFF_BURROW ))
    {
      send_to_char(AT_WHITE, "You must wait until the earth heals you!\n\r", ch);
      return;
    }
    if ( IS_AFFECTED2( victim, AFF_SHADOW_PLANE ))
    {
      send_to_char(AT_WHITE, "You can not attack someone who is in the shadow plane.\n\r", ch);
      return;
    }
    if ( IS_AFFECTED4( victim, AFF_BURROW ))
    {
      send_to_char(AT_WHITE, "You can not attack someone who is burrowed.\n\r", ch);
      return;
    }
    if ( IS_AFFECTED( ch, AFF_PEACE ) )
    {
	    affect_strip( ch, skill_lookup("aura of peace" ));
	    REMOVE_BIT( ch->affected_by, AFF_PEACE );
    }
    if ( !IS_NPC( victim ) )
    {
	if (   !IS_SET( victim->act, PLR_KILLER )
	    && !IS_SET( victim->act, PLR_THIEF  ) )
	{
	    send_to_char(AT_WHITE, "You must MURDER a player.\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( IS_AFFECTED( victim, AFF_CHARM ) && victim->master )
	{
	    send_to_char(AT_WHITE, "You must MURDER a charmed creature.\n\r", ch );
	    return;
	}
    }

    if ( victim == ch )
    {
	send_to_char(AT_RED, "You hit yourself.  Stupid!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
	act(AT_BLUE, "$N is your beloved master!", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char(C_DEFAULT, "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, PULSE_VIOLENCE );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char(C_DEFAULT, "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char       buf [ MAX_STRING_LENGTH ];
    char       arg [ MAX_INPUT_LENGTH  ];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Murder whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch ){
        send_to_char(C_DEFAULT, "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !is_pkillable( ch, victim ) ) {
    	return;
    }

    if ( IS_AFFECTED(victim, AFF_PEACE ))
    {
      send_to_char(AT_WHITE, "A wave of peace overcomes you.\n\r", ch);
      return;
    }
    if ( IS_AFFECTED2( ch, AFF_SHADOW_PLANE ))
    {
      send_to_char(AT_WHITE, "You must leave the shadow realm.\n\r", ch);
      return;
    }
    if ( IS_AFFECTED4( ch, AFF_BURROW ))
    {
      send_to_char(AT_WHITE, "You must wait until the earth heals you!\n\r", ch);
      return;
    }
    if ( IS_AFFECTED2( victim, AFF_SHADOW_PLANE ))
    {
      send_to_char(AT_WHITE, "You can not attack someone who is in the shadow plane.\n\r", ch);
      return;
    }
    if ( IS_AFFECTED4( victim, AFF_BURROW ))
    {
      send_to_char(AT_WHITE, "You can not attack someone who is burrowed.\n\r", ch);
      return;
    }
    if ( IS_AFFECTED( ch, AFF_PEACE ))
    {
	    affect_strip( ch, skill_lookup("aura of peace") );
	    REMOVE_BIT( ch->affected_by, AFF_PEACE );
    }

    if (!IS_NPC(victim))
    	ch->pkill_timer = 0;

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
	act(C_DEFAULT, "$N is your beloved master!", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char(C_DEFAULT, "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, PULSE_VIOLENCE );
    sprintf( buf, "Help!  I am being attacked by %s!", ch->name );
    if( IS_NPC( ch ) )
    {
	sprintf( buf, "Help!  I am being attacked by %s!", ch->short_descr );
    }
    do_shout( victim, buf );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



int skill_backstab( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    one_argument( target_name, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Backstab whom?\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	return SKPELL_BOTCHED;
    }

    if ( victim == ch )
    {
	send_to_char(C_DEFAULT, "How can you sneak up on yourself?\n\r", ch );
	return SKPELL_BOTCHED;
    }

    if ( !( obj = get_eq_char( ch, WEAR_WIELD ) )
	|| obj->value[3] != 11 )
    {
	send_to_char(C_DEFAULT, "You need to wield a piercing weapon.\n\r", ch );
	return SKPELL_BOTCHED;
    }

    if ( victim->fighting )
    {
	send_to_char(C_DEFAULT, "You can't backstab a fighting person.\n\r", ch );
	return SKPELL_BOTCHED;
    }

    if ( victim->hit < (victim->max_hit-200) )
    {
	act(C_DEFAULT, "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return SKPELL_BOTCHED;
    }

    if ( !is_pkillable( ch, victim ) ) {
    	return SKPELL_MISSED;
    }

    if ( IS_AFFECTED2( ch, AFF_SHADOW_PLANE ))
    {
      send_to_char(AT_WHITE, "You must leave the shadow realm.\n\r", ch);
      return SKPELL_MISSED;
    }
    if ( IS_AFFECTED4( ch, AFF_BURROW ))
    {
      send_to_char(AT_WHITE, "You must wait until the earth heals you!\n\r", ch);
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
    if ( !IS_AWAKE( victim )
	|| IS_NPC( ch )
	|| number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
	multi_hit( ch, victim, sn );
    }
    else
		damage( ch, victim, 0, sn );

    return SKPELL_NO_DAMAGE; 
}

int skill_assassinate( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
    int	       chance;


    chance = IS_NPC( ch ) ? 0 : ( ch->pcdata->learned[skill_lookup("mind of fanoom")] / 10 ) ;
    if( chance > 0 && !IS_IMMORTAL( ch ) && !IS_NPC( ch ) )
    {
	chance = chance - ( 10 * ( LEVEL_DEMIGOD - ch->level ) );
    }

    one_argument( target_name, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Assassinate whom?\n\r", ch );
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

    if ( victim->fighting )
    {
	send_to_char(C_DEFAULT, "You can't assassinate a fighting person.\n\r", ch );
	return SKPELL_MISSED;
    }

    if( chance <= 0 )
    {
	if ( victim->hit < (victim->max_hit - 200) )
	{
	    act(C_DEFAULT, "$N is hurt and suspicious ... you can't sneak up.",
		ch, NULL, victim, TO_CHAR );
	    return SKPELL_MISSED;
        }
    }
    else
    {
	if ( ( victim->hit < victim->max_hit/2 || chance < number_percent( ) ) && ( victim->hit < victim->max_hit - 200 ) )
	{
	    act(C_DEFAULT, "$N is hurt and suspicious ... you can't sneak up.",
		ch, NULL, victim, TO_CHAR );
	    return SKPELL_MISSED;
	}
    }

    if ( !( obj = get_eq_char( ch, WEAR_WIELD ) )
	|| obj->value[3] != 11 )
    {
	send_to_char(C_DEFAULT, "You need to wield a piercing weapon.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !is_pkillable( ch, victim ) ) {
    	return SKPELL_MISSED;
    }

    if ( IS_AFFECTED2( ch, AFF_SHADOW_PLANE ))
    {
      send_to_char(AT_WHITE, "You must leave the shadow realm.\n\r", ch);
      return SKPELL_MISSED;
    }
    if ( IS_AFFECTED4( ch, AFF_BURROW ))
    {
      send_to_char(AT_WHITE, "You must wait until the earth heals you!\n\r", ch);
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
    if ( !IS_NPC ( ch ) )
    {
	if ( number_percent( ) > ( ch->pcdata->learned[sn] / 10 )  )
	{
	    send_to_char(C_DEFAULT, "You failed.\n\r", ch );
	    return SKPELL_MISSED;
	}
    }

    if (!IS_NPC(victim))
    	ch->pkill_timer = 0;

    WAIT_STATE( ch, skill_table[sn].beats );
    update_skpell( ch, skill_lookup("mind of fanoom"), 0 );
    multi_hit( ch, victim, sn );

    return SKPELL_NO_DAMAGE;
}



void do_flee( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *victim;
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    int              attempt;

    if ( IS_AFFECTED( ch, AFF_ANTI_FLEE ) )
    {
      send_to_char( AT_RED, "You cannot!\n\r", ch );
      return;
    }

    if ( !IS_NPC(ch) && (IS_AFFECTED3(ch, AFF_BLOODTHIRSTY) || IS_AFFECTED2(ch, AFF_BERSERK)) )
    {
      send_to_char( AT_RED, "You cannot!\n\r", ch );
      return;
    }

    if ( !( victim = ch->fighting ) )
    {
	if ( ch->position == POS_FIGHTING )
	    ch->position = POS_STANDING;
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return;
    }
    if ( IS_SET( ch->in_room->room_flags, ROOM_NO_FLEE ) )
      {
       send_to_char(C_DEFAULT, "You failed!  You lose 10 exps.\n\r", ch );
       gain_exp( ch, -10 );
       return;
      }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int        door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	    ||   !pexit->to_room
            ||   (IS_SET(pexit->to_room->room_flags, ROOM_SAFE) && ch->pkill)
	    ||   IS_SET( pexit->exit_info, EX_CLOSED )
            || (IS_SET( pexit->exit_info, EX_HIDDEN)
                &&
!IS_SET (race_table[ch->race].race_abilities,RACE_DETECT_HIDDEN_EXIT ) )
	    || ( IS_NPC( ch )
		&& ( IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB )
		    || ( IS_SET( ch->act, ACT_STAY_AREA )
			&& pexit->to_room->area != ch->in_room->area ) ) ) )
	    continue;

	move_char( ch, door );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act(C_DEFAULT, "$n has fled!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC( ch ) )
	{
	    send_to_char(C_DEFAULT, "You flee from combat!  You lose 25 exps.\n\r", ch );
	    gain_exp( ch, -25 );
	}

        if ( ch->fighting && IS_NPC( ch->fighting ) )
          if ( IS_SET( ch->fighting->act, ACT_TRACK ) )
            ch->fighting->hunting = ch;

	stop_fighting( ch, TRUE );
	return;
    }

    send_to_char(C_DEFAULT, "You failed!  You lose 10 exps.\n\r", ch );
    gain_exp( ch, -10 );
    return;
}

int skill_retreat( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA       *victim;
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    int              attempt;

    if (IS_AFFECTED( ch, AFF_ANTI_FLEE ) )
    {
      send_to_char( AT_RED, "You cannot!\n\r", ch );
      return SKPELL_BOTCHED;
    }

    if ( !( victim = ch->fighting ) )
    {
	if ( ch->position == POS_FIGHTING )
	    ch->position = POS_STANDING;
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }
    if ( IS_SET( ch->in_room->room_flags, ROOM_NO_FLEE ) )
      {
       send_to_char(C_DEFAULT, "You failed your retreat!\n\r", ch );
       return SKPELL_MISSED;
      }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int        door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	    ||   !pexit->to_room
            ||   (IS_SET(pexit->to_room->room_flags, ROOM_SAFE) && ch->pkill)
	    ||   IS_SET( pexit->exit_info, EX_CLOSED )
	    || ( IS_NPC( ch )
		&& ( IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB )
		    || ( IS_SET( ch->act, ACT_STAY_AREA )
			&& pexit->to_room->area != ch->in_room->area ) ) ) )
	    continue;

	move_char( ch, door );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act(C_DEFAULT, "$n has retreated!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC( ch ) )
	{
	    send_to_char(C_DEFAULT, "You retreated successfully!\n\r", ch );
	}

        if ( ch->fighting && IS_NPC( ch->fighting ) )
          if ( IS_SET( ch->fighting->act, ACT_TRACK ) )
            ch->fighting->hunting = ch;

	stop_fighting( ch, TRUE );
	return SKPELL_NO_DAMAGE;
    }

    send_to_char(C_DEFAULT, "You failed your retreat!\n\r", ch );
    return SKPELL_MISSED;
}

int skill_rescue( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    CHAR_DATA *fch;
    char       arg [ MAX_INPUT_LENGTH ];

    one_argument( target_name, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Rescue whom?\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( victim == ch )
    {
	send_to_char(C_DEFAULT, "What about fleeing instead?\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !IS_NPC( ch ) && IS_NPC( victim ) )
    {
	send_to_char(C_DEFAULT, "Doesn't need your help!\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( ch->fighting == victim )
    {
	send_to_char(C_DEFAULT, "Too late.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !( fch = victim->fighting ) )
    {
	send_to_char(C_DEFAULT, "That person is not fighting right now.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !is_same_group( ch, victim ) && (ch->clan != victim->clan))
    {
	send_to_char(C_DEFAULT, "Why would you want to?\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !check_blind ( ch ) )
        return SKPELL_MISSED;

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( !IS_NPC( ch ) && number_percent( ) > ( ch->pcdata->learned[sn] / 10 )  )
    {
	send_to_char(C_DEFAULT, "You fail the rescue.\n\r", ch );
	return SKPELL_MISSED;
    }

    act(C_DEFAULT, "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
    act(C_DEFAULT, "$n rescues you!", ch, NULL, victim, TO_VICT    );
    act(C_DEFAULT, "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );

    stop_fighting( fch, FALSE );

    set_fighting( fch, ch );

    return SKPELL_NO_DAMAGE;
}



int skill_gouge( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam = 0;

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !check_blind( ch ) )
        return SKPELL_MISSED;

    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
	dam = number_range(ch->level * 2, ch->level * 5);// damage( ch, victim, number_range( ch->level*2, ch->level*5 ), sn );

	if IS_AFFECTED(victim, AFF_BLIND)
	return dam;

	if IS_AFFECTED2(victim, AFF_BLINDFOLD)
	return dam;

        else
	if ( number_percent( ) < 50 )
        {
	AFFECT_DATA af;

	af.type      = skill_lookup("blindness");
	af.level     = ch->level;
	af.duration  = 5;
	af.location  = APPLY_HITROLL;
	af.modifier  = -50;
	af.bitvector = AFF_BLIND;
	affect_join( victim, &af );

        act(AT_WHITE, "$N is blinded!", ch, NULL, victim, TO_CHAR    );
        send_to_char(AT_WHITE, "You are blinded!\n\r", victim );
        act(AT_WHITE, "$N is blinded!", ch, NULL, victim, TO_NOTVICT );
        return dam;
        }

    update_pos( victim );
    }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, gsn_gouge );

    return dam;
}

int skill_rake( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam = 0;

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !check_blind( ch ) )
        return SKPELL_MISSED;

    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
		dam = number_range(ch->level * 2,ch->level * 5); //damage( ch, victim, number_range( ch->level*2, ch->level*5 ), sn );

	if IS_AFFECTED2(victim, AFF_BLINDFOLD)
		return dam;

	if IS_AFFECTED(victim, AFF_BLIND)
		return dam;

        else
	if ( number_percent( ) < 50 )
        {
	AFFECT_DATA af;

	af.type      = skill_lookup("blindness");
	af.level     = ch->level;
	af.duration  = 5;
	af.location  = APPLY_HITROLL;
	af.modifier  = -50;
	af.bitvector = AFF_BLIND;
	affect_join( victim, &af );

        act(AT_WHITE, "$N is blinded!", ch, NULL, victim, TO_CHAR    );
        send_to_char(AT_WHITE, "You are blinded!\n\r", victim );
        act(AT_WHITE, "$N is blinded!", ch, NULL, victim, TO_NOTVICT );
        return dam;
        }

    update_pos( victim );
    }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, gsn_rake );

    return dam;
}

int skill_back_kick( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam = 0;

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    one_argument( target_name, arg);

    victim = ch->fighting;

    if ( arg[0] != '\0' )
       if ( !(victim = get_char_room( ch, arg ) ) )
       {
	send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	return SKPELL_MISSED;
       }

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
		dam = number_range( ch->level*5, ch->level*13 );
	//damage( ch, victim, number_range( ch->level*5, ch->level*13 ), sn );
    }
    else
		dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, sn );

    return dam;
}


int skill_circle( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam = 0;

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !check_blind( ch ) )
        return SKPELL_MISSED;

    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    WAIT_STATE( ch, skill_table[sn].beats/2 );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
	 dam = number_range(ch->level*5, ch->level*14); //damage( ch, victim, number_range( ch->level*5, ch->level*14 ), gsn_circle );
        update_pos( victim );
    }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, gsn_circle );

    return dam;
}

int skill_dim_mak( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam;
      

    if ( IS_NPC( ch ) )
      if ( IS_SET( ch->affected_by, AFF_CHARM ) )
        return SKPELL_MISSED;

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !check_blind( ch ) )
        return SKPELL_MISSED;

    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < (ch->pcdata->learned[sn]/20) )
    {
		dam = number_range( (int)(victim->hit * .5), (int)(victim->hit * .6) ); //damage( ch, victim, number_range( (int)(victim->hit * .5), (int)(victim->hit * .6) ), gsn_dim_mak );
        update_pos( victim );
    }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, gsn_dim_mak );

    return dam;
}

int skill_kick( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam;

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !check_blind( ch ) )
        return SKPELL_MISSED;

    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
	dam = number_range(ch->level*3, ch->level*9); //damage( ch, victim, number_range( ch->level*3, ch->level*9 ), gsn_kick );
    }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, gsn_kick );

    return dam; 
}

int skill_slam( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam;

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !check_blind( ch ) )
        return SKPELL_MISSED;

    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
	dam = number_range(ch->level * 8, ch->level * 19); //damage( ch, victim, number_range( ch->level*8, ch->level*19 ), gsn_slam );
    }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, gsn_slam );

    return dam;
}

int skill_claw( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam;

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !check_blind( ch ) )
        return SKPELL_MISSED;

    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
	dam = number_range(ch->level * 5, ch->level * 7); //damage( ch, victim, number_range( ch->level*5, ch->level*7 ), gsn_claw );
    }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, gsn_claw );

    return dam;
}

int skill_bite( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam = 0;

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !check_blind( ch ) )
        return SKPELL_MISSED;

    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
	dam = number_range(ch->level * 7, ch->level * 10); //damage( ch, victim, number_range( ch->level*7, ch->level*10 ), sn );
    }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, sn );

    return dam;
}
int skill_shriek( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam = 0;
      

    if IS_AFFECTED2( ch, AFF_SLIT )
    {
	send_to_char(AT_BLOOD, "You gargle through your slit throat.\n\r", ch);
	return SKPELL_MISSED;
    }

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
		dam = number_range(ch->level * 5, ch->level * 10); //damage( ch, victim, number_range( ch->level*5, ch->level*10),gsn_shriek );

        send_to_char(AT_BLOOD, "Your deadly wail shatters the air!\n\r",ch);
        send_to_char(AT_BLOOD, "Your hearing is damaged!\n\r", victim );
        act(AT_BLOOD, "$n shatters the air with a deadly wail!", ch, NULL, NULL, TO_ROOM );
        return dam;
     }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, gsn_shriek );

    return dam;
}

int skill_dirt_kick( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam;

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You kick some dirt into the air.\n\r", ch );
	return SKPELL_MISSED;
    }


    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
	dam = number_range(ch->level * 2, ch->level * 5); //damage( ch, victim, number_range( ch->level*2, ch->level*5 ), gsn_dirt_kick );

	if IS_AFFECTED(victim, AFF_BLIND)
	return dam;

	if IS_AFFECTED2(victim, AFF_BLINDFOLD)
	return dam;

        else
	if ( number_percent( ) < 40 )
        {
	AFFECT_DATA af;

	af.type      = skill_lookup("blindness");
	af.level     = ch->level;
	af.duration  = 1;
	af.location  = APPLY_HITROLL;
	af.modifier  = -50;
	af.bitvector = AFF_BLIND;
	affect_join( victim, &af );

        act(AT_WHITE, "$N is blinded!", ch, NULL, victim, TO_CHAR    );
        send_to_char(AT_WHITE, "The dirt blinds you!!\n\r", victim );
        act(AT_WHITE, "$N is blinded!", ch, NULL, victim, TO_NOTVICT );
        return dam;
        }

    update_pos( victim );
    }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, sn );

    return dam;
}


int skill_chop( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam = 0;

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !check_blind( ch ) )
        return SKPELL_MISSED;

    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
	dam = number_range(ch->level * 3, (int)(ch->level * 6.15)); //damage( ch, victim, number_range( ch->level*3, (int)(ch->level*6.15) ), gsn_chop );
    }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, gsn_chop );

    return dam;
}

int skill_head_butt( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam = 0;

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !check_blind( ch ) )
        return SKPELL_MISSED;

    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
	 dam = number_range(ch->level * 3, ch->level * 8); //damage( ch, victim, number_range( ch->level*3, ch->level*8 ), gsn_head_butt );
    }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, gsn_head_butt );

    return dam;
}

int damclass_adjust(CHAR_DATA *victim, int dam, int sn)
{
    int 	mod = 0;
     
   if (sn < TYPE_HIT)

	switch (skill_table[sn].damclass)
	{
	case DAMCLASS_NULL:
	break;
	case DAMCLASS_ACID :
	    mod = victim->damage_mods[0];
	    break;
	case DAMCLASS_HOLY :
	    mod = victim->damage_mods[1];
	    break;
	case DAMCLASS_MAGIC :
	    mod = victim->damage_mods[2];
	    break;
	case DAMCLASS_FIRE :
	    mod = victim->damage_mods[3];
	    break;
	case DAMCLASS_ENERGY :
	    mod = victim->damage_mods[4];
	    break;
	case DAMCLASS_WIND :
	    mod = victim->damage_mods[5];
	    break;
	case DAMCLASS_WATER :
	    mod = victim->damage_mods[6];
	    break;
	case DAMCLASS_ILLUSION :
	    mod = victim->damage_mods[7];
	    break;
	case DAMCLASS_DISPEL :
	    mod = victim->damage_mods[8];
	    break;
	case DAMCLASS_EARTH :
	    mod = victim->damage_mods[9];
	    break;
	case DAMCLASS_PSYCHIC :
	    mod = victim->damage_mods[10];
	    break;
	case DAMCLASS_POISON :
	    mod = victim->damage_mods[11];
	    break;
	case DAMCLASS_BREATH :
	    mod = victim->damage_mods[12];
	    break;
	case DAMCLASS_SUMMON :
	    mod = victim->damage_mods[13];
	    break;
	case DAMCLASS_PHYSICAL :
	    mod = victim->damage_mods[14];
	    break;
	case DAMCLASS_EXPLOSIVE :
	    mod = victim->damage_mods[15];
	    break;
	case DAMCLASS_SONG :
	    mod = victim->damage_mods[16];
	    break;
	case DAMCLASS_NAGAROM :
	    mod = victim->damage_mods[17];
	    break;
	case DAMCLASS_UNHOLY :
	    mod = victim->damage_mods[18];
	    break;
	case DAMCLASS_CLAN :
	    mod = victim->damage_mods[19];
	    break;
	default:
		break;
	}
    if (sn >=TYPE_HIT || sn == TYPE_UNDEFINED)
    {
	if (!dam_class_value)
		mod = victim->damage_mods[14];
	else
		mod = victim->damage_mods[dam_class_value];
    }
	mod = URANGE( -75, mod, 75);
	damclass_shield_weaken(victim, dam, sn);

	dam = dam - (dam * mod) / 100;
	return dam;
}

bool can_use_skspell(CHAR_DATA *ch, int skspellNum)
{
	/*char buf[1000];
	sprintf(buf, "gsn: %d, skillName: %s, level: %d, multi: %d",
			skspellNum, skill_table[skspellNum].name,
			skill_table[skspellNum].skill_level[ch->class],
			skill_table[skspellNum].skill_level[ch->multied]);
	if( !IS_NPC(ch) ) {
		bug(buf, 0);
	}*/
	if ( !IS_NPC( ch )&&(( ch->level < skill_table[skspellNum].skill_level[ch->class] )
        && (ch->level < skill_table[skspellNum].skill_level[ch->multied]))) {
		return FALSE;
	} else if (IS_NPC( ch ))
	{
		int gsn   = skspellNum;

		if (ch->level < 5 && ( gsn == skill_lookup("second attack") || gsn == skill_lookup("second strike") ) )
			return FALSE;
		if (ch->level < 15 && ( gsn == skill_lookup("third attack") || gsn == skill_lookup("third strike") ) )
			return FALSE;
		if (ch->level < 25 && ( gsn == skill_lookup("fourth attack") || gsn == skill_lookup("fourth strike") ) )
			return FALSE;
		if (ch->level < 35 && ( gsn == skill_lookup("fifth attack") || gsn == skill_lookup("fifth strike") ) )
			return FALSE;
		if (ch->level < 40 && ( gsn == skill_lookup("sixth attack") || gsn == skill_lookup("sixth strike") ) )
			return FALSE;
		if (ch->level < 45 && ( gsn == skill_lookup("seventh attack") || gsn == skill_lookup("seventh strike") ) )
			return FALSE;
		if (ch->level < 50 && ( gsn == skill_lookup("eighth attack") || gsn == skill_lookup("eighth strike") ) )
			return FALSE;
	}


	return TRUE;
}

void damclass_shield_weaken(CHAR_DATA *victim, int dam, int dt)
{
	int adjustment = dam /200;
	int damclass = 0;
	bool found = FALSE;
	int index = 0;
	int apply_loc = APPLY_NONE;
	AFFECT_DATA * paf;
	if (dt >= TYPE_HIT)
		damclass = DAMCLASS_PHYSICAL;
	else
		damclass = skill_table[dt].damclass;

	switch (damclass)
	{
	case DAMCLASS_ACID:
		apply_loc = APPLY_DAM_ACID;
		index = 0;
		break;
	case DAMCLASS_HOLY:
		apply_loc = APPLY_DAM_HOLY;
		index = 1;
		break;
	case DAMCLASS_MAGIC:
		apply_loc = APPLY_DAM_MAGIC;
		index = 2;
		break;
	case DAMCLASS_FIRE:
		apply_loc = APPLY_DAM_FIRE;
		index = 3;
		break;
	case DAMCLASS_ENERGY:
		apply_loc = APPLY_DAM_ENERGY;
		index = 4;
		break;
	case DAMCLASS_WIND:
		apply_loc = APPLY_DAM_WIND;
		index = 5;
		break;
	case DAMCLASS_WATER:
		apply_loc = APPLY_DAM_WATER;
		index = 6;
		break;
	case DAMCLASS_ILLUSION:
		apply_loc = APPLY_DAM_ILLUSION;
		index = 7;
		break;
	case DAMCLASS_DISPEL:
		apply_loc = APPLY_DAM_DISPEL;
		index = 8;
		break;
	case DAMCLASS_EARTH:
		apply_loc = APPLY_DAM_EARTH;
		index = 9;
		break;
	case DAMCLASS_PSYCHIC:
		apply_loc = APPLY_DAM_PSYCHIC;
		index = 10;
		break;
	case DAMCLASS_POISON:
		apply_loc = APPLY_DAM_POISON;
		index = 11;
		break;
	case DAMCLASS_BREATH:
		apply_loc = APPLY_DAM_BREATH;
		index = 12;
		break;
	case DAMCLASS_SUMMON:
		apply_loc = APPLY_DAM_SUMMON;
		index = 13;
		break;
	case DAMCLASS_PHYSICAL:
		apply_loc = APPLY_DAM_PHYSICAL;
		index = 14;
		break;
	case DAMCLASS_EXPLOSIVE:
		apply_loc = APPLY_DAM_EXPLOSIVE;
		index = 15;
		break;
	case DAMCLASS_SONG:
		apply_loc = APPLY_DAM_SONG;
		index = 16;
		break;
	case DAMCLASS_NAGAROM:
		apply_loc = APPLY_DAM_NAGAROM;
		index = 17;
		break;
	case DAMCLASS_UNHOLY:
		apply_loc = APPLY_DAM_UNHOLY;
		index = 18;
		break;
	case DAMCLASS_CLAN:
		apply_loc = APPLY_DAM_CLAN;
		index = 19;
		break;
	default:
		return;
	}

	for (paf = victim->affected; paf; paf = paf->next)
	{
		if (paf->deleted)
			continue;
		if (found)
			break;
		/* Ok, first we find the right affect... */
		if ((paf->location == apply_loc) && (paf->modifier > 0) )
		{
			victim->damage_mods[index] -=UMIN( adjustment, paf->modifier);
			paf->modifier -= adjustment;
			paf->modifier = UMAX(0,paf->modifier);
			found = TRUE;
		}
	}
	for (paf = victim->affected2; paf; paf = paf->next)
	{
		if (paf->deleted)
			continue;
		if (found)
			break;
		/* Ok, first we find the right affect... */
		if ((paf->location == apply_loc) && paf->modifier > 0 )
		{
			victim->damage_mods[index] -= UMIN( adjustment, paf->modifier);
			paf->modifier -= adjustment;
			paf->modifier = UMAX(0, paf->modifier);
			found = TRUE;
		}
	}
	for (paf = victim->affected3; paf; paf = paf->next)
	{
		if (paf->deleted)
			continue;
		if (found)
			break;
		/* Ok, first we find the right affect... */
		if ((paf->location == apply_loc) && paf->modifier > 0 )
		{
			victim->damage_mods[index] -= UMIN( adjustment, paf->modifier);
			paf->modifier -= adjustment;
			paf->modifier = UMAX(0, paf->modifier);
			found = TRUE;
		}
	}
	for (paf = victim->affected4; paf; paf = paf->next)
	{
		if (paf->deleted)
			continue;
		if (found)
			break;
		/* Ok, first we find the right affect... */
		if ((paf->location == apply_loc) && paf->modifier > 0 )
		{
			victim->damage_mods[index] -= UMIN(adjustment, paf->modifier);
			paf->modifier -= adjustment;
			paf->modifier = UMAX(0, paf->modifier);
			found = TRUE;
		}
	}
	return;
}

int skill_axe_kick( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ]; 
    int dam = 0;
	
    if ( !ch->fighting )
    {   
        send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
        return SKPELL_MISSED;
    }
        
    if ( !check_blind( ch ) )
        return SKPELL_MISSED;
 
    one_argument( target_name, arg );
    
    victim = ch->fighting;
      
    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
        {
            send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
            return SKPELL_MISSED;
        }
        
    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
        dam = number_range(ch->level * 4, ch->level * 11); //damage( ch, victim, number_range( ch->level*4, ch->level*11 ), gsn_axe_kick );
    }
    else
        dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, gsn_axe_kick );
    
    return dam;
}
// AC Damage Reduction - Ahsile
int damreduce (CHAR_DATA* ch, CHAR_DATA* victim, int dam, int dt)
{
    float mod;
    //char buf [MAX_STRING_LENGTH];

    // No reduction for certain skills
    if ( dt == skill_lookup("backstab") || dt == skill_lookup("double backstab") || dt == skill_lookup("assassinate") || dt == skill_lookup("triple backstab") )
        return dam;

    // Hitroll (you got good aim!)
    mod = -(victim->armor) - (IS_NPC(ch) ? (float)(ch->hitroll/4) : (float)(ch->hitroll/2));
    
    // Max out at MAX_ARMOR
    mod = (mod > MAX_ARMOR) ? MAX_ARMOR : (mod < -MAX_ARMOR ? -MAX_ARMOR : mod);

    // Translate into a number between -0.5 to 0.5
    mod /= (MAX_ARMOR*2);

    //sprintf(buf, "Mod: %f Orig Dam: %d", mod, dam);
    //send_to_char(AT_WHITE, buf, victim);
    dam -= (int)(dam*mod);
    //sprintf(buf, " New Damage: %d\n\r", dam);
    //send_to_char(AT_WHITE, buf, victim);
    return dam;
}

/* rewrite insane wield checks - Ahsile */
bool can_use_attack( int attack_num, CHAR_DATA* ch)
{
	char buf_att[MAX_STRING_LENGTH];
	char buf_str[MAX_STRING_LENGTH];
	char buf_sht[MAX_STRING_LENGTH];

	char buf    [MAX_STRING_LENGTH];
	static char* att = "%s attack";
	static char* str = "%s strike";
	static char* sht = "%s shot";


	OBJ_DATA* wielded = get_eq_char( ch, WEAR_WIELD );
	OBJ_DATA* dualed  = get_eq_char( ch, WEAR_WIELD_2 );
	
	int value	  = flag_value( weapon_flags, "shot" );

	if ( attack_num > 2 && attack_num < 7 && IS_AFFECTED3(ch, AFF_TALE_OF_TERROR) )
		return FALSE;

	switch ( attack_num )
	{
		case 2:
			strcpy(buf, "second");
			break;
		case 3:
			strcpy(buf, "third");
			break;
		case 4:
			strcpy(buf, "fourth");
			break;
		case 5:
			strcpy(buf, "fifth");
			break;
		case 6:
			strcpy(buf, "sixth");
			break;
		case 7:
			strcpy(buf, "seventh");
			break;
		case 8:
			strcpy(buf, "eighth");
			break;
		default:
			return FALSE;
	}

	sprintf(buf_att, att, buf); /* make "second attack", etc */
	sprintf(buf_str, str, buf); /* make "second strike", etc */
	sprintf(buf_sht, sht, buf); /* make "second shot", etc   */

    if ( 
	   (
       	 	( can_use_skspell( ch, skill_lookup( buf_att ) )	) &&  
         	( wielded != NULL || dualed != NULL 			) &&
		( ( wielded ? wielded->value[3] != value : FALSE ) || ( dualed ? dualed->value[3] != value : FALSE   ) ) &&
 		( IS_NPC(ch) || ( !IS_NPC(ch) && number_percent( ) < ( ch->pcdata->learned[ skill_lookup( buf_att )] / 10 ) ) )
	   )
       ||  (
         	( can_use_skspell( ch, skill_lookup( buf_str ) ) 	) && 
		( wielded == NULL && dualed == NULL  			) &&
 		( IS_NPC(ch) || ( !IS_NPC(ch) && number_percent( ) < ( ch->pcdata->learned[ skill_lookup( buf_str )] / 10 ) ) )
	   )
       ||  (
         	( can_use_skspell( ch, skill_lookup( buf_sht ) ) 	) && 
		( wielded != NULL || dualed != NULL  			) &&
		( ( wielded ? wielded->value[3] == value : FALSE ) || ( dualed ? dualed->value[3] == value : FALSE   ) ) &&
 		( IS_NPC(ch) || ( !IS_NPC(ch) && number_percent( ) < ( ch->pcdata->learned[ skill_lookup( buf_sht )] / 10 )  ) )
	   )
       )
     {
		if (!wielded && !dualed)
			update_skpell(ch, skill_lookup( buf_str ), 0 );
		else
		{
			if (wielded && !dualed)
			{
				if (IS_SET( wielded->extra_flags2, ITEM_TWO_HANDED ) && wielded->value[3] != flag_value( weapon_flags, "shot" ) )
				{
					if ( IS_NPC(ch) || number_percent() < (ch->pcdata->learned[skill_lookup("two handed")]/10 ) )
					{
						update_skpell( ch, skill_lookup("two handed"), 0 );
					} else
					{
						return FALSE; // wielding a two handed weapon, and two handed check failed
					}

				}
			}

			if ((wielded && wielded->value[3] == value) || (dualed && dualed->value[3] == value ))
				update_skpell( ch, skill_lookup( buf_sht ), 0 );

			if ((wielded && wielded->value[3] != value) || (dualed && dualed->value[3] != value ))	
				update_skpell( ch, skill_lookup( buf_att ), 0 );
		}
		return TRUE;
     }
     
    return FALSE;
}

/* dual actually depends on your skill % now */
bool can_dual( CHAR_DATA* ch)
{

	int pct = number_percent();
	int sn  = skill_lookup("dual");

	/* no know dual? fail */
	if ( !can_use_skspell(ch, sn ) )
		return FALSE;

	/* two handed weapon? fail */
	if (get_eq_char(ch, WEAR_WIELD) != NULL && IS_SET(get_eq_char(ch, WEAR_WIELD)->extra_flags2, ITEM_TWO_HANDED))
		return FALSE;

	/* bare fisted? succeed if % check succeeds */
	if (get_eq_char(ch, WEAR_WIELD) == NULL && get_eq_char(ch,WEAR_WIELD_2) == NULL )
	{
		if (IS_NPC(ch) || pct < ( ch->pcdata->learned[sn] / 10 ) )
		{
			update_skpell( ch, sn, 0 );
			return TRUE;
		}
	} 
	/* are you actually dual wielding? if so and % check succeeds */
	else if (get_eq_char(ch, WEAR_WIELD) != NULL && get_eq_char(ch, WEAR_WIELD_2) != NULL )
	{
		if (IS_NPC(ch) || pct < ( ch->pcdata->learned[sn] / 10 ) )
		{
			update_skpell( ch, sn, 0 );
			return TRUE;
		}
	}


	/* otherwise, you failed the checks because:
		a) you failed a % check 
			or
		b) you are wielding one weapon (either hand)
	*/
	
	return FALSE;
			
}

void miss_message( CHAR_DATA* ch, CHAR_DATA* victim, bool dual )
{
    if ( IS_SET( ch->act, PLR_COMBAT ) )
    {
	if (!dual)
		act(AT_GREEN, "You attack $N, but miss.", ch, NULL, victim, TO_CHAR    );
	else
		act(AT_GREEN, "You attack $N and miss both primary and dual attacks.", ch, NULL, victim, TO_CHAR);
    }

    if ( IS_SET( victim->act, PLR_COMBAT ) )
    {
	act(AT_GREEN, "$n attacks you, but misses.",  ch, NULL, victim, TO_VICT    );
    }
}
