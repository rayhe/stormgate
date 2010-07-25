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

/*$Id: magic4.c,v 1.36 2005/03/23 19:18:22 ahsile Exp $*/

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

int spell_aura_of_anti_magic( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED3( victim, AFF_AURA_ANTI_MAGIC ) )
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

    af.type	= sn;
    af.level	= level;
    af.duration	= -1;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_AURA_ANTI_MAGIC;
    affect_to_char3( victim, &af);
    victim->shields += 1;

    send_to_char(AT_PINK, "Your body is surrounded by an aura of anti-magic.\n\r", victim );
    act(AT_PINK, "$n's body is surrounded by an aura of anti-magic.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_holy_protection( int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED3( victim, AFF_HOLY_PROTECTION ) )
	return SKPELL_MISSED;

    af.type	= sn;
    af.level	= level;
    af.duration = number_fuzzy(level / 10);
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_HOLY_PROTECTION;
    affect_to_char3( victim, &af);

    send_to_char(AT_WHITE, "You feel protected by the Gods.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int spell_soul_bind( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA  *soulgem;

    if ( !IS_NPC(victim) || saves_spell( level, victim ) || IS_SET(victim->act, ACT_UNDEAD ) )
    {
	send_to_char(AT_BLUE, "You failed.\n\r", ch);
	return SKPELL_MISSED;
    }

    if(IS_SIMM(victim, IMM_CHARM))
      return SKPELL_MISSED;

    soulgem = create_object( get_obj_index( OBJ_VNUM_SOULGEM ), 0 );
    soulgem->ac_vnum = victim->pIndexData->vnum;
    soulgem->level = ch->level;
    soulgem->timer = ch->level / 4;
    soulgem->cost  = victim->level * 10;
    soulgem->ac_charge[0] = soulgem->ac_charge[1] = 1;
    obj_to_char( soulgem, ch);

    act(AT_BLUE, "You tear out $Ns soul, binding it to form a Soul Gem.", ch, NULL, victim, TO_CHAR);
    act(AT_BLUE, "$n tears out $Ns soul, binding it to form a Soul Gem.", ch, NULL, victim, TO_ROOM);
    act(AT_BLUE, "$N screams in agony as it slowly dissipates into nothingness!", ch, NULL, victim, TO_CHAR);
    act(AT_BLUE, "$N screams in agony as it slowly dissipates into nothingness!", ch, NULL, victim, TO_ROOM);
    act(AT_WHITE, "Your Soul is stolen by $n!", ch, NULL, victim, TO_VICT);

    if( IS_NPC( victim ) )
	extract_char(victim, TRUE );
    else
	extract_char( victim, FALSE );

    return SKPELL_NO_DAMAGE;
}

int spell_blood_gout( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int	       dam;

    if( saves_spell( level, victim ) )
	return SKPELL_MISSED;

    dam = dice( 4, level );
    if( ( ch->bp + dam ) > ( ch->max_bp ) )
        ch->bp = ch->max_bp;
    else
        ch->bp += dam;

   // damage( ch, victim, dam, sn );

    return dam;
}

int spell_bloodshield( int sn, int level, CHAR_DATA  *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED3( victim, AFF_BLOODSHIELD ) )
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

    af.type	= sn;
    af.level	= level;
    af.duration = -1;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_BLOODSHIELD;
    affect_to_char3( victim, &af );
    victim->shields += 1;

    send_to_char(AT_BLOOD, "Your body is surrounded by swirling blood.\n\r", victim );
    act(AT_BLOOD, "$n's body is surrounded by swirling blood.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_mother_natures_blessing( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
        return SKPELL_MISSED;
    af.type      = sn;
    af.level     = level;
    af.duration  = 40 + level;
    af.location  = APPLY_DAMROLL;
    af.modifier  = level / 5;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_HITROLL;
    af.modifier  = level / 5;
    affect_to_char( victim, &af );

    send_to_char(AT_BLUE, "You pray to the gods of Nature.\n\r", ch );
    send_to_char(AT_BLUE, "You feel the touch of Nature itself.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}
int spell_bark_skin( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = -level/2;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char(AT_GREY, "Your skin is covered in protective bark.\n\r", victim );
    act(AT_GREY, "$n's skin becomes covered with protective bark.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_bend_light( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        SkNum;
    int        yesno  = FALSE;

    SkNum=skill_lookup("blindfold");
    if (is_affected( victim, SkNum) || saves_spell( level, victim ) )
    {   
        affect_strip( victim, SkNum);
        send_to_char(AT_BLUE, "Your blindfold has been removed.\n\r", victim);
        send_to_char(AT_BLUE, "Your victim is no longer blindfolded.\n\r", ch);
        yesno = TRUE;
    }    
    SkNum=skill_lookup("true sight");
    if ( is_affected( victim, SkNum) || saves_spell( level, victim ) )
    {
	affect_strip( victim, SkNum);
	send_to_char(AT_BLUE, "Your sight becomes untrue.\n\r", victim );
        send_to_char(AT_BLUE, "Your victim's sight is no longer true.\n\r", ch);
	yesno = TRUE;
    }
   
    SkNum=skill_lookup("detect invis");
    if ( is_affected( victim, SkNum) || saves_spell( level, victim ) )
    {
	affect_strip( victim, SkNum);
	send_to_char(AT_BLUE, "You no longer see the invisible.\n\r", victim );
        send_to_char(AT_BLUE, "Your victim can no longer see the invisible.\n\r", ch);
	yesno = TRUE; 
    }
	 
    SkNum=skill_lookup("detect hidden");
    if ( is_affected( victim, SkNum) || saves_spell( level, victim ) )
    {
	affect_strip( victim, SkNum);
	send_to_char(AT_BLUE, "You no longer can detect the hidden.\n\r", victim );
        send_to_char(AT_BLUE, "Your victim can no longer detect the hidden.\n\r", ch);
	yesno = TRUE;
   }

    if (IS_AFFECTED3(victim,AFF_BEND_LIGHT) || saves_spell(level,victim))
   {
send_to_char(AT_BLUE, "You have failed in blinding your victim.\n\r", ch);
   return SKPELL_MISSED;
   }
    else
   {
    af.type      = sn;
    af.level	 = level;
    af.duration  = level / 25;
    af.location  = APPLY_HITROLL;
    af.modifier  = -(level/4);
    af.bitvector = AFF_BEND_LIGHT;
    affect_to_char3( victim, &af );

    af.location  = APPLY_DAMROLL;
    af.modifier  = -(level/4);
    affect_to_char3( victim, &af );

   send_to_char(AT_BLUE, "You have blinded your victim.\n\r", ch );
   send_to_char(AT_BLUE, "You have become blinded!\n\r", victim );
   }

   if ( ch != victim && yesno )
   {
        send_to_char(C_DEFAULT, "Darkness overcomes you as the light bends around you.\n\r", victim);
        send_to_char(AT_WHITE, "The illusion of darkness is successful!\n\r", ch );
        act(AT_GREY, "The illusion of darkness envelops $N !\n\r", victim, NULL, victim, TO_ROOM);
   }

  return SKPELL_NO_DAMAGE;

}

int spell_cloud_of_healing( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
                 
    if ( IS_AFFECTED3( victim, AFF_CLOUD_OF_HEALING) )
        return SKPELL_MISSED;
    
    af.type = sn;
    af.level = ch->level;
    af.duration = number_fuzzy(ch->level/4);
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_CLOUD_OF_HEALING;
    affect_to_char3(victim, &af);
 
    send_to_char(AT_BLUE, "A small cloud hovers over your head.\n\r",ch);
    act(AT_BLUE, "$n has a small cloud hovering over their head.", victim,
NULL, NULL, TO_ROOM);
    return SKPELL_NO_DAMAGE;
}

int spell_earthblast( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim       = (CHAR_DATA *) vo;
    int     count;
    int     chance;
    int     chance2;

    count = 1;
    chance = 100;		     
    if (!IS_NPC(victim ) )
    {
	while ((chance > 25) && (victim->hit > 0))
	{
	chance2 = number_range(2,10);
        act(AT_ORANGE, "The earth ERRUPTS under $n.\n\r",victim,NULL,
	NULL,TO_ROOM);	
        damage(ch, victim, ch->level*chance2, sn);
        count = count + 1;
	chance = number_range(1,100/count);
	}
    }

    if (IS_NPC(victim ) )
    {
        while ((chance > 15) && (victim->hit > 0))
        {
	chance2 = number_range(2,20);
        act(AT_ORANGE, "The earth ERRUPTS under $n.\n\r",victim,NULL,
        NULL,TO_ROOM);  
        damage(ch, victim, ch->level*chance2, sn);         
        count = count + 1;
        chance = number_range(1,100/count);
        }
    }

	return SKPELL_NO_DAMAGE;
}

int spell_tale_of_terror( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim       = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED3( victim, AFF_TALE_OF_TERROR) )
        return SKPELL_MISSED;

    af.type = sn;
    af.level = ch->level;
    af.duration = number_fuzzy(ch->level/33);
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_TALE_OF_TERROR;
    affect_to_char3(victim, &af);
    
    send_to_char(AT_BLUE, "The horror story instills fear into you.\n\r",
    victim);
    act(AT_BLUE, "$n looks a little frightened.", victim, NULL,NULL,
    TO_ROOM);
    return SKPELL_NO_DAMAGE;  
}

int spell_power_leak( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim       = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED3( victim, AFF_POWER_LEAK) )
        return SKPELL_MISSED;

    af.type = sn;
    af.level = ch->level;
    af.duration = number_fuzzy(ch->level/5);
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_POWER_LEAK;
    affect_to_char3(victim, &af);
    
    send_to_char(AT_CYAN, "You feel a tap on your resources.\n\r",victim);
    act(AT_CYAN, "$n face pales.", victim,NULL,NULL,TO_ROOM);
    return SKPELL_NO_DAMAGE;
}

void do_multiburst( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int mana;
    int sn = 0;
    int mana2;
    int sn2 = 0;
    int WAIT1;
    int WAIT2;
    int WAIT_FINAL;
    int mana_final;
    int intreq = 0;
    int intreq1 = 0;
    int intreq2 = 0;
    int intreq3 = 0;

    char arg1 [ MAX_INPUT_LENGTH ];
    char arg2 [ MAX_INPUT_LENGTH ];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( !IS_NPC( ch )
        && ((ch->level < skill_table[skill_lookup("multiburst")].skill_level[ch->class] )
        && (ch->level < skill_table[skill_lookup("multiburst")].skill_level[ch->multied]))))
    {
        send_to_char(C_DEFAULT,
            "You'd better leave the spellcasting to the spellcasters.\n\r", ch );
        return;
    }

    if ( IS_NPC( ch ) )
	return;

    if (arg2[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Multiburst requires 2 spells.\n\r", ch );
	return;
    }

    /* Design a checkalign(), set do_cast do_quickburst and this */

    if ( !IS_NPC( ch ) )
    if ( ( sn = skill_lookup( arg1 ) ) < 0
        || ((ch->level < skill_table[sn].skill_level[ch->class]) &&
           (ch->level < skill_table[sn].skill_level[ch->multied]) ))
    {
        send_to_char(AT_BLUE, "You can't do that.\n\r", ch );
        return;
    }

	if (!skill_table[sn].is_spell)
	{
		send_to_char(AT_WHITE, "Casting a skill? Try using it...\n\r", ch);
		return;
	}

    if ( !IS_NPC( ch ) )
    if ( ( sn2 = skill_lookup( arg2 ) ) < 0
        || ((ch->level < skill_table[sn2].skill_level[ch->class]) &&
           (ch->level < skill_table[sn2].skill_level[ch->multied]) ))
    {
        send_to_char(AT_BLUE, "You can't do that.\n\r", ch );
        return;
    }

	if (!skill_table[sn2].is_spell)
	{
		send_to_char(AT_WHITE, "Casting a skill? Try using it...", ch);
		return;
	}

    if ( !IS_NPC( ch ) )
    if ( ch->class == CLASS_BARBARIAN  || ch->multied == CLASS_BARBARIAN )
    {
        if ( skill_table[sn].skill_level[ch->class] > 60
        && skill_table[sn].skill_level[ch->multied] > 60 )
        {
            send_to_char(C_DEFAULT, "Barbarians can not cast magic of such high level.\n\r", ch);
            return;
        }
    }

    if ( !IS_NPC( ch ) )
    if ( ch->class == CLASS_BARBARIAN  || ch->multied == CLASS_BARBARIAN )
    {
        if ( skill_table[sn2].skill_level[ch->class] > 60
        && skill_table[sn2].skill_level[ch->multied] > 60 )
        {
            send_to_char(C_DEFAULT, "Barbarians can not cast magic of such high level.\n\r", ch );
            return;
        }
    }

    if ( !IS_NPC( ch ) )
    if ( ( ch->race == 10 && IS_EVIL( ch ) ) ||
         ( ch->race == 10 && IS_NEUTRAL( ch ) ) ||
         ( ch->race == 15 && IS_GOOD( ch ) ) ||
         ( ch->race == 15 && IS_NEUTRAL( ch ) ) ||
         ( ch->class == CLASS_CLERIC && IS_EVIL( ch ) ) ||
         ( ch->multied == CLASS_CLERIC && IS_EVIL( ch ) ) ||
         ( ch->class == CLASS_PALADIN && IS_EVIL( ch ) ) ||
         ( ch->multied == CLASS_PALADIN && IS_EVIL( ch ) ) ||
         ( ch->class == CLASS_ANTI_PALADIN && IS_GOOD( ch ) ) ||
         ( ch->multied == CLASS_ANTI_PALADIN && IS_GOOD( ch ) ) ||
         ( ch->class == CLASS_DARKPRIEST && IS_GOOD( ch ) ) ||
         ( ch->multied == CLASS_DARKPRIEST && IS_GOOD( ch ) )           )
    {
        send_to_char(AT_BLUE, "You can not cast when you are that alignment.\n\r", ch);
        return;
    }

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if( IS_AFFECTED2( ch, AFF_SLIT ) )
    {
	send_to_char(AT_BLOOD, "Your throat is slit, you can not cast.\n\r", ch );
	return;
    }

    if( IS_STUNNED( ch, STUN_MAGIC ) )
    {
	send_to_char(AT_LBLUE, "You're too stunned to cast spells.\n\r", ch );
	return;
    }

    if ( ch->position < skill_table[sn].minimum_position )
    {
        send_to_char(AT_BLUE, "You can't concentrate enough.\n\r", ch );
        return;
    }

    if ( ch->position < skill_table[sn2].minimum_position )
    {
        send_to_char(AT_BLUE, "You can't concentrate enough.\n\r", ch );
        return;
    }

    if ( skill_table[sn].target == TAR_GROUP_OFFENSIVE ||
	 skill_table[sn].target == TAR_GROUP_DEFENSIVE ||
	 skill_table[sn].target == TAR_GROUP_IGNORE ||
	 skill_table[sn].target == TAR_GROUP_OBJ ||
	 skill_table[sn].target == TAR_GROUP_ALL )
      {
	send_to_char(AT_BLUE, "You can't multiburst group spells!\n\r", ch);
	return;
      }
    if ( skill_table[sn2].target == TAR_GROUP_OFFENSIVE ||
	 skill_table[sn2].target == TAR_GROUP_DEFENSIVE ||
	 skill_table[sn2].target == TAR_GROUP_IGNORE ||
	 skill_table[sn2].target == TAR_GROUP_OBJ ||
	 skill_table[sn2].target == TAR_GROUP_ALL )
      {
	send_to_char(AT_BLUE, "You can't multiburst group spells!\n\r", ch);
	return;
      }



    mana = 0;
    if ( !IS_NPC( ch ) && ch->level >= skill_table[sn].skill_level[ch->multied] )
    {
        mana = MANA_COST_MULTI( ch, sn );
        intreq1 = ( skill_table[sn].skill_level[ch->multied] / 2 ) - 5;
        if( intreq1 >= 40 )
        {
            intreq1 = 40;
        }
    }
    if ( !IS_NPC( ch ) && ch->level >= skill_table[sn].skill_level[ch->class] )
    {
        mana = MANA_COST( ch, sn );
        intreq2 = ( skill_table[sn].skill_level[ch->class] / 2 ) - 5;
        if( intreq2 >= 40 )
        {
            intreq2 = 40;
	}
    }
    intreq = UMIN( intreq1, intreq2 );
    mana2 = 0;
    if ( !IS_NPC( ch ) && ch->level >= skill_table[sn2].skill_level[ch->multied] )
    {
        mana2= MANA_COST_MULTI( ch, sn2 );
        intreq1 = ( skill_table[sn2].skill_level[ch->multied] / 2 ) - 5;
        if( intreq1 >= 40 )
        {
            intreq1 = 40;
        }	
    }
    if ( !IS_NPC( ch ) && ch->level >= skill_table[sn2].skill_level[ch->class] )
    {
        mana2 = MANA_COST( ch, sn2 );
        intreq2 = ( skill_table[sn2].skill_level[ch->class] / 2 ) - 5;
        if( intreq2 >= 40 )
        {
            intreq2 = 40;
	}
    }
    intreq3 = UMIN( intreq1, intreq2 );
    intreq  = UMAX( intreq,  intreq3 ); 

    mana_final = ( mana + mana2 ) * 3 / 4;

    if ( ch->class == CLASS_VAMPIRE || ch->class == CLASS_ANTI_PALADIN )
    {
       mana_final /= 4;
    }

    if(ch->class != CLASS_CLERIC && ch->class != CLASS_DARKPRIEST )
    {
        if( get_curr_int( ch ) < intreq )
        {
            send_to_char(AT_BLUE, "You do not have the required intelligence.\n\r", ch );
            return;
        }
    }

    if(ch->class == CLASS_CLERIC || ch->class == CLASS_DARKPRIEST)
    {
        if( get_curr_wis( ch ) < intreq )
        {
            send_to_char(AT_BLUE, "You do not have the required wisdom.\n\r", ch );
            return;
        }
    } 

    victim = ch->fighting;

	update_skpell( ch, skill_lookup("multiburst"), 0 );

    /* Make sure the spell is offensive, and NOT area-affecting */

    /* Make sure the other spell is too */

    WAIT1 = ( skill_table[sn].beats );
    WAIT2 = ( skill_table[sn2].beats );
    WAIT_FINAL = ( WAIT1 + WAIT2 ) * 3 / 4;
    WAIT_STATE( ch, WAIT_FINAL);
    mana = mana_final;
   
    if ( ( ch->class != CLASS_VAMPIRE ) && ( ch->class != CLASS_ANTI_PALADIN ) && ( ch->mana < mana ) )
    {
        send_to_char(AT_BLUE, "You don't have enough mana.\n\r", ch );
        return;
    }
    else
    if ( ( ch->bp < mana ) && (( ch->class == CLASS_VAMPIRE )||( ch->class == CLASS_ANTI_PALADIN)))
    {
        send_to_char(AT_RED, "You are to starved to cast, you must feed.\n\r", ch );
        return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
    {
	send_to_char(AT_BLUE, "You failed.\n\r", ch );
	return;
    }

    if ( number_percent( ) > ( ch->pcdata->learned[sn] / 10 ) || ( number_percent( ) > ch->pcdata->learned[skill_lookup("multiburst")] / 10 ) )
    {
        send_to_char(AT_BLUE, "You lost your concentration.\n\r", ch );
        if (( ch->class != CLASS_VAMPIRE )&&( ch->class  != CLASS_ANTI_PALADIN))
           ch->mana -= mana / 2;
        else
           ch->bp -= mana / 2; 
	if( ch->pcdata->learned[sn] <= 750 )
	   update_skpell( ch, sn, 0 );
    }
    else
    {
		int dmg  = 0;
		int dmg2 = 0;

        if (( ch->class != CLASS_VAMPIRE )&&(ch->class != CLASS_ANTI_PALADIN))
           ch->mana -= mana;
        else
           ch->bp -= mana;
        if ( ( IS_AFFECTED2( ch, AFF_CONFUSED ) )
            && number_percent( ) < 10 )
        {
           act(AT_YELLOW, "$n looks around confused at what's going on.", ch, NULL, NULL, TO_ROOM );
           send_to_char( AT_YELLOW, "You become confused and botch the spell.\n\r", ch );
           return;
        }

                if (ch->pcdata->learned[skill_lookup("psionic casting")])
                        update_skpell(ch, skill_lookup("psionic casting"), 0);

		send_to_char( AT_BLUE, "You release a burst of energy!\n\r", ch );
		act( AT_BLUE, "$n releases a burst of energy.", ch, NULL, NULL, TO_ROOM);

		if  (skill_table[sn].target== TAR_CHAR_SELF)
		{
			dmg = (*skill_table[sn].spell_fun) (sn, URANGE( 1, ch->level, LEVEL_DEMIGOD), ch, ch); 
			if (dmg > SKPELL_NO_DAMAGE)
			{
				damage( ch, ch, dmg, sn );
			}
		}
		else 
		{
			dmg = (*skill_table[sn].spell_fun) ( sn, URANGE( 1, ch->level, LEVEL_DEMIGOD ), ch, victim ); 
			if (dmg > SKPELL_NO_DAMAGE)
			{
				damage( ch, victim, dmg, sn );
			}
			
		}

		if (dmg >= SKPELL_NO_DAMAGE)
			update_skpell( ch, sn, 0 );

		if  (skill_table[sn2].target== TAR_CHAR_SELF)
		{
			dmg2 = (*skill_table[sn2].spell_fun) (sn2, URANGE( 1, ch->level, LEVEL_DEMIGOD), ch, ch); 
			if (dmg2 > SKPELL_NO_DAMAGE)
			{
				damage( ch, victim, dmg2, sn2 );
			}
		}
		else 
		{
    
			dmg2 = (*skill_table[sn2].spell_fun) ( sn2, URANGE( 1, ch->level, LEVEL_DEMIGOD ), ch, victim ); 
			if (dmg2 > SKPELL_NO_DAMAGE)
			{
				damage( ch, victim, dmg2, sn2 );
			}
			
		}
		if (dmg2 >= SKPELL_NO_DAMAGE)
			update_skpell( ch, sn2, 0 );

    }

}

int spell_unholy_sword_spell( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
	return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( (int)(level * 1.5) );
    af.modifier	 = number_fuzzy( (int)(level * 5) );
    af.location	 = APPLY_HIT;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char( AT_RED, "Your unholy weapon glows with a black light.\n\r", ch );
    act(AT_RED, "$n's unholy weapon glows with a black light.", ch, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_holy_sword_spell( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
	return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( (int)(level * 1.5) );
    af.modifier  = number_fuzzy( (int)(level * 5) );
    af.location	 = APPLY_HIT;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char( AT_YELLOW, "Your holy weapon glows with a bright light.\n\r", ch );
    act(AT_RED, "$n's holy weapon glows with a bright light.", ch, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_unholysword( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
     
    if ( obj->item_type != ITEM_WEAPON
        || IS_OBJ_STAT( obj, ITEM_MAGIC )
        || obj->affected )   
    {
        send_to_char(AT_BLUE, "That item cannot be consecrated.\n\r", ch );
        return SKPELL_MISSED;
    }
    
        paf             = new_affect();
    
    paf->type           = sn;
    paf->duration       = -1;
    paf->location       = APPLY_HITROLL;
    paf->modifier       = 16 + (level >= 18) + (level >= 25) + (level >= 40) + (level >= 60) +(level >= 90);
    paf->bitvector      = 0;
    paf->next           = obj->affected;
    obj->affected       = paf;
    
        paf             = new_affect();
    
    paf->type           = sn;
    paf->duration       = -1;
    paf->location       = APPLY_DAMROLL;
    paf->modifier       = 16 + (level >= 18) + (level >= 25) + (level >= 45) + (level >= 65) +(level >= 90);;
    paf->bitvector      = 0;
    paf->next           = obj->affected;
    obj->affected       = paf;
    
        paf             = new_affect();
    paf->type           = sn;
    paf->duration       = -1;
    paf->location       = APPLY_HIT;
    paf->modifier       = 90 + ( (level >=18) * 10) + ( (level >=25) * 10) + ( (level >= 45) * 10) + ( (level >= 65) * 10) + ( ( level >= 85) * 10 ) ;;
    paf->bitvector      = 0;
    paf->next           = obj->affected;
    obj->affected       = paf;

    obj->ac_type 	= 5;
    obj->ac_spell	= skill_table[skill_lookup("unholy enchantment")].name;
    obj->ac_charge[0]	= 1;
    obj->ac_charge[1]	= 1;

    if ( IS_GOOD( ch ) )
    {
        SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL);
        act(AT_YELLOW, "$p glows.",   ch, obj, NULL, TO_CHAR );
    }   
    else if ( IS_EVIL( ch ) )
    {
        SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
        act(AT_RED, "$p glows",    ch, obj, NULL, TO_CHAR );
    }
    else
    {
        SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
        SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
        act(AT_WHITE, "$p glows.", ch, obj, NULL, TO_CHAR );
    }

    if( ch->multied == CLASS_MAGE || ch->class == CLASS_MAGE ) { SET_BIT( obj->extra_flags3, ITEM_PRO_MAGE ); }
    if( ch->multied == CLASS_CLERIC || ch->class == CLASS_CLERIC ) { SET_BIT( obj->extra_flags3, ITEM_PRO_CLERIC ); }
    if( ch->multied == CLASS_THIEF || ch->class == CLASS_THIEF ) { SET_BIT( obj->extra_flags3, ITEM_PRO_THIEF ); }
    if( ch->multied == CLASS_WARRIOR || ch->class == CLASS_WARRIOR ) { SET_BIT( obj->extra_flags3, ITEM_PRO_WARRIOR ); }
    if( ch->multied == CLASS_PSIONICIST || ch->class == CLASS_PSIONICIST ) { SET_BIT( obj->extra_flags3, ITEM_PRO_PSI ); }
    if( ch->multied == CLASS_DRUID || ch->class == CLASS_DRUID ) { SET_BIT( obj->extra_flags3, ITEM_PRO_DRUID ); }
    if( ch->multied == CLASS_RANGER || ch->class == CLASS_RANGER ) { SET_BIT( obj->extra_flags3, ITEM_PRO_RANGER ); }
    if( ch->multied == CLASS_PALADIN || ch->class == CLASS_PALADIN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_PALADIN ); }
    if( ch->multied == CLASS_BARD || ch->class == CLASS_BARD ) { SET_BIT( obj->extra_flags3, ITEM_PRO_BARD ); }
    if( ch->multied == CLASS_VAMPIRE || ch->class == CLASS_VAMPIRE ) { SET_BIT( obj->extra_flags3, ITEM_PRO_VAMP ); }
    if( ch->multied == CLASS_WEREWOLF || ch->class == CLASS_WEREWOLF ) { SET_BIT( obj->extra_flags3, ITEM_PRO_WEREWOLF ); }
    if( ch->multied == CLASS_ANTI_PALADIN || ch->class == CLASS_ANTI_PALADIN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_ANTIPAL ); }
    if( ch->multied == CLASS_ASSASSIN || ch->class == CLASS_ASSASSIN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_ASSASSIN ); }
    if( ch->multied == CLASS_MONK || ch->class == CLASS_MONK ) { SET_BIT( obj->extra_flags3, ITEM_PRO_MONK ); }
    if( ch->multied == CLASS_BARBARIAN || ch->class == CLASS_BARBARIAN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_BARBARIAN ); }
    if( ch->multied == CLASS_ILLUSIONIST || ch->class == CLASS_ILLUSIONIST ) { SET_BIT( obj->extra_flags3, ITEM_PRO_ILLUSIONIST ); }
    if( ch->multied == CLASS_NECROMANCER || ch->class == CLASS_NECROMANCER ) { SET_BIT( obj->extra_flags3, ITEM_PRO_NECROMANCER ); }
    if( ch->multied == CLASS_DEMONOLOGIST || ch->class == CLASS_DEMONOLOGIST ) { SET_BIT( obj->extra_flags3, ITEM_PRO_DEMONOLOGIST ); }
    if( ch->multied == CLASS_SHAMAN || ch->class == CLASS_SHAMAN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_SHAMAN ); }
    if( ch->multied == CLASS_DARKPRIEST || ch->class == CLASS_DARKPRIEST ) { SET_BIT( obj->extra_flags3, ITEM_PRO_DARKPRIEST ); }

    send_to_char(AT_BLUE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

int spell_daggers_of_pain( int sn, int level, CHAR_DATA *ch, void *vo )
{
     CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
          0,   0,   0,   0,  30,         35,  40,  45,  50,  55,
         60,  65,  70,  75,  80,         82,  84,  86,  88,  90,
         92,  94,  96,  98, 100,        102, 104, 106, 108, 110,
        112, 114, 116, 118, 120,        122, 124, 126, 128, 130,
        132, 134, 136, 138, 140,        142, 144, 146, 148, 150,
        152, 154, 156, 158, 160,        162, 164, 166, 168, 170,
        172, 174, 176, 178, 180,        182, 184, 186, 188, 190,
        192, 194, 196, 198, 200,        202, 204, 206, 208, 210,
        215, 220, 225, 230, 235,        240, 245, 250, 255, 260
    };
    int        dam;
      
    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam      = number_range( dam_each[level], dam_each[level] * 7 );

    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_spectral_armor( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) )
        return SKPELL_MISSED;
    af.type      = sn;  
    af.level     = level;
    af.duration  = level / 2 + 30;
    af.location  = APPLY_AC;
    af.modifier  = -50;
    af.bitvector = 0;
    affect_to_char( victim, &af );
     
    if ( ch != victim )
        send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "You feel spectral forces protecting you.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int spell_wisp_of_protection( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) )
        return SKPELL_MISSED;

    af.type      = sn;
    af.level     = level;
    af.duration  = 8 + level;
    af.location  = APPLY_AC;
    af.modifier  = ( level / 2 ) * -1;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char(AT_BLUE, "You are surrounded by a wisp of protection.\n\r", victim );
    act(AT_BLUE, "$n is surrounded by a wisp of protection.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_breathe_water( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED3( victim, AFF_GILLS ) )
        return SKPELL_MISSED;

    af.type      = sn;
    af.duration  = 2 * level / 3 ;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.level	 = ch->level;
    af.bitvector = AFF_GILLS;
    affect_to_char3( victim, &af );

    send_to_char( AT_BLUE, "You can now breathe underwater.\n\r", victim );
    act( AT_BLUE, "$n can now breathe underwater.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_mark( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;
     
    if ( obj->item_type != ITEM_RUNE
        || IS_OBJ_STAT( obj, ITEM_MAGIC ) )
    {
        send_to_char(AT_BLUE, "That item cannot be marked.\n\r", ch );
        return SKPELL_MISSED;
    }

    if ( !IS_SET( ch->in_room->area->area_flags, AREA_PRESENT ) )
    {
	send_to_char(C_DEFAULT, "Sorry, you can not recall while in the future or past.\n\r", ch );
	return SKPELL_MISSED;
    }

    if( IS_SET( ch->in_room->room_flags, ROOM_NO_ASTRAL_IN )
        || IS_SET( ch->in_room->room_flags, ROOM_NO_ASTRAL_OUT )
        || IS_SET( ch->in_room->area->area_flags, AREA_PROTOTYPE ) )
    {
	send_to_char(C_DEFAULT, "Sorry, you can not mark here.\n\r", ch );
	return SKPELL_MISSED;
    }

    obj->value[0] = ch->in_room->vnum;
    SET_BIT( obj->extra_flags, ITEM_MAGIC);

    send_to_char(AT_BLUE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}


int spell_rune_recall( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA       *victim;
    ROOM_INDEX_DATA *location;
    char             buf [ MAX_STRING_LENGTH ];
    int              place;
    char             name[ MAX_STRING_LENGTH ];
    OBJ_DATA    *obj = (OBJ_DATA *) vo;

    if ( obj->item_type != ITEM_RUNE )
    {
	send_to_char( C_DEFAULT, "That is not a rune.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !IS_OBJ_STAT( obj, ITEM_MAGIC ) )	
    {
	send_to_char( C_DEFAULT, "That rune is not marked.\n\r", ch );
	return SKPELL_MISSED;
    }

    place = obj->value[0];

    if ( !( location = get_room_index( place ) ) )

    {
	send_to_char(C_DEFAULT, "You are completely lost.\n\r", ch );
 	return SKPELL_MISSED;
    }

    act(C_DEFAULT, "$n prays for transportation!", ch, NULL, NULL, TO_ROOM );

    if ( ch->in_room == location )
	return SKPELL_MISSED;

    if ( IS_AFFECTED( ch, AFF_CURSE )
	|| IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) )
    {
	act(C_DEFAULT, "The Gods have forsaken you.", ch, NULL, name, TO_CHAR );
	return SKPELL_MISSED;
    }

    if ( IS_SET(location->room_flags, ROOM_SAFE) && ch->pkill && (ch->combat_timer > 0))
    {
        send_to_char(AT_RED, "Your blood is too hot to recall there!", ch);
        return SKPELL_MISSED;
    }

    if ( ( victim = ch->fighting ) )
    {
	int lose;

	if ( number_bits( 1 ) == 0 )
	{
	    WAIT_STATE( ch, 4 );
	    lose = ( ch->desc ) ? 50 : 100;
	    gain_exp( ch, 0 - lose );
	    sprintf( buf, "You failed!  You lose %d exps.\n\r", lose );
	    send_to_char(C_DEFAULT, buf, ch );
	    return SKPELL_MISSED;
	}

	lose = ( ch->desc ) ? 100 : 200;
	gain_exp( ch, 0 - lose );
	sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
	send_to_char(C_DEFAULT, buf, ch );
	stop_fighting( ch, TRUE );
    }

    act(C_DEFAULT, "$n disappears.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act(C_DEFAULT, "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return SKPELL_NO_DAMAGE;
}

int spell_nagaroms_curse( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED3( victim, AFF_NAGAROMS_CURSE ) )
        return SKPELL_MISSED;

    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_NAGAROMS_CURSE;
    affect_to_char3( victim, &af );

    send_to_char(AT_BLUE, "You feel the wrath of Nagarom!\n\r", victim );
    act(AT_BLUE, "$n's suffers from Nagaroms Curse.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_prayer( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED3( victim, AFF_PRAYER ) )
        return SKPELL_MISSED;

    af.duration = ch->level / 3;
    af.level = ch->level;
    af.type = sn;
    af.bitvector = AFF_PRAYER;
    af.location = APPLY_HIT;
    af.modifier = number_range( ch->level, ch->level * 5 );
    affect_to_char3(ch, &af);

    af.location = APPLY_MANA;
    af.modifier = number_range( ch->level / 2, ch->level );
    if (( ch->class == 9 )||( ch->class == 11 ))
    {
      af.location = APPLY_BP;
      af.modifier /= 4;
    }
    affect_to_char3( ch, &af );

    af.location = APPLY_INT;
    af.modifier = ((ch->level - 1) / 50) + 1;
    affect_to_char3( ch, &af );

    af.location = APPLY_DEX;
    af.modifier = ((ch->level - 1) / 50) + 1;
    affect_to_char3( ch, &af );

    send_to_char( AT_BLUE, "The Gods place their blessings upon you.\n\r", ch);
  send_to_char( AT_GREY, "You fall to the ground, unconscious.\n\r", ch );
  act( AT_GREY, "$n falls to the ground, unconscious.", ch, NULL, NULL, TO_ROOM );
  STUN_CHAR( ch, 4, STUN_COMMAND );
  return SKPELL_NO_DAMAGE;
}

int spell_lullaby( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_SLEEP )
        || level < victim->level
        || ( saves_spell(level + IS_SRES(victim, RES_MAGIC) ? -5 : 0, victim)
        && !(get_trust( ch ) > 100) ) )
    {
        send_to_char(AT_BLUE, "You failed.\n\r", ch );
        return SKPELL_MISSED;
    }

    if(IS_SIMM(victim, IMM_MAGIC))
      return SKPELL_MISSED;

    af.type      = sn;
    af.level     = level;
    af.duration  = 4 + level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE( victim ) )
    {
        send_to_char(AT_BLUE, "You feel very sleepy ..... zzzzzz.\n\r", victim );
        if ( victim->position == POS_FIGHTING )
           stop_fighting( victim, TRUE );
        do_sleep( victim, "" );
    }

    return SKPELL_NO_DAMAGE;
}

int spell_randomshield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED3( victim, AFF_RANDOMSHIELD ) )
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
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_RANDOMSHIELD;
    affect_to_char3( victim, &af );
    victim->shields += 1;

    send_to_char(AT_GREY, "You bring forth a plethora of random illusions.\n\r", victim);
    act(AT_GREY, "$n's body is surrounded by a plethora of random illusions.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_sonic_blast( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
          0,   0,   0,   0,  30,         35,  40,  45,  50,  55,
         60,  65,  70,  75,  80,         82,  84,  86,  88,  90,
         92,  94,  96,  98, 100,        102, 104, 106, 108, 110,
        112, 114, 116, 118, 120,        122, 124, 126, 128, 130,
        132, 134, 136, 138, 140,        142, 144, 146, 148, 150,
        152, 154, 156, 158, 160,        162, 164, 166, 168, 170,
        172, 174, 176, 178, 180,        182, 184, 186, 188, 190,
        192, 194, 196, 198, 200,        202, 204, 206, 208, 210,
        215, 220, 225, 230, 235,        240, 245, 250, 255, 260
    };
    int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam      = number_range( dam_each[level], dam_each[level] * 6 );

    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_sonic_boom( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
          0,   0,   0,   0,  30,         35,  40,  45,  50,  55,
         60,  65,  70,  75,  80,         82,  84,  86,  88,  90,
         92,  94,  96,  98, 100,        102, 104, 106, 108, 110,
        112, 114, 116, 118, 120,        122, 124, 126, 128, 130,
        132, 134, 136, 138, 140,        142, 144, 146, 148, 150,
        152, 154, 156, 158, 160,        162, 164, 166, 168, 170,
        172, 174, 176, 178, 180,        182, 184, 186, 188, 190,
        192, 194, 196, 198, 200,        202, 204, 206, 208, 210,
        215, 220, 225, 230, 235,        240, 245, 250, 255, 260
    };
    int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam      = number_range( dam_each[level] * 3, dam_each[level] * 10 );

    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_hellfire( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
          0,   0,   0,   0,  60,         70,  80,  90, 100, 110,
        120, 130, 140, 150, 160,        164, 168, 172, 176, 180,
        184, 188, 192, 196, 200,        204, 208, 212, 216, 220,
        224, 228, 232, 236, 240,        244, 248, 252, 256, 260,
        264, 268, 272, 276, 280,        284, 288, 292, 296, 300,
        305, 310, 315, 320, 325,        330, 335, 340, 345, 350,
        355, 360, 365, 370, 375,        380, 385, 390, 395, 400,
        405, 410, 415, 420, 425,        430, 435, 440, 445, 450,
        460, 470, 480, 490, 500,        510, 520, 530, 540, 550
    };
                 int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam      = number_range( dam_each[level] / 2, dam_each[level] * 5 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_resurrection( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if( IS_NPC( victim ) )
    {
        send_to_char(C_DEFAULT, "You can not resurrect a non-player character.\n\r", ch );
        return SKPELL_MISSED;
    }
    if( IS_NPC( ch ) || (
        ( IS_EVIL( ch ) && ( IS_EVIL( victim ) || IS_NEUTRAL( victim ) ) ) ||
        ( IS_GOOD( ch ) && ( IS_GOOD( victim ) || IS_NEUTRAL( victim ) ) ) ) )
    {
        if( victim->position != POS_GHOST )
        {
            send_to_char( C_DEFAULT, "That character is not dead!\n\r", ch );
            return SKPELL_MISSED;
        }

        victim->position = POS_STANDING;
        update_pos( victim );
        send_to_char(AT_GREY, "You leave the ethereal plane and become whole once again.\n\r", victim);
        act(AT_GREY, "$n's leaves the etheral plane and becomes whole once again.", victim, NULL, NULL, TO_ROOM );
        return SKPELL_NO_DAMAGE;
    }
    else
    {
        send_to_char(C_DEFAULT, "You can not resurrect someone of that alignment.\n\r", ch );
        return SKPELL_MISSED;
    }
}

int spell_earthshield( int sn, int level, CHAR_DATA  *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED4( victim, AFF_EARTHSHIELD ) )
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
    af.location = APPLY_DAM_WIND;
    if( ( ( ch->level / 2 ) -10 ) <= 0 )
    {
	af.modifier = 0;
    }
    else 
    {
	af.modifier = (ch->level/2)-10;
    }
    af.bitvector = AFF_EARTHSHIELD;
    affect_to_char4( victim, &af );
    victim->shields += 1;
    
    send_to_char(AT_ORANGE, "Your body is surrounded by an earthen shield.\n\r", victim );
    act(AT_ORANGE, "$n's body is surrounded by an earthen shield.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_leaf_shield( int sn, int level, CHAR_DATA  *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED4( victim, AFF_LEAF_SHIELD ) )
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
    af.location = APPLY_DAM_EARTH; 
    if( ( ( ch->level / 2 ) -20 ) <= 0 )
    {
        af.modifier = 0;
    }
    else
    {
        af.modifier = (ch->level/2)-20;
    }
    af.bitvector = AFF_LEAF_SHIELD;
    affect_to_char4( victim, &af );
    victim->shields += 1;
    
    send_to_char(AT_DGREEN, "Your body is surrounded by swirling leaves.\n\r", victim );
    act(AT_DGREEN, "$n's body is surrounded by swirling leaves.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_leaf_strike( int sn, int level, CHAR_DATA *ch, void *vo )
{

    CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
          0,   0,   0,   0,  30,         35,  40,  45,  50,  55,
         60,  65,  70,  75,  80,         82,  84,  86,  88,  90,
         92,  94,  96,  98, 100,        102, 104, 106, 108, 110,
        112, 114, 116, 118, 120,        122, 124, 126, 128, 130,
        132, 134, 136, 138, 140,        142, 144, 146, 148, 150,
        152, 154, 156, 158, 160,        162, 164, 166, 168, 170,
        172, 174, 176, 178, 180,        182, 184, 186, 188, 190,
        192, 194, 196, 198, 200,        202, 204, 206, 208, 210,
        215, 220, 225, 230, 235,        240, 245, 250, 255, 260
    };
                 int        dam;
 
    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam      = number_range( dam_each[level] / 2, dam_each[level] * 7 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_shards_of_glass( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
          0,   0,   0,   0,  60,         70,  80,  90, 100, 110,
        120, 130, 140, 150, 160,        164, 168, 172, 176, 180,
        184, 188, 192, 196, 200,        204, 208, 212, 216, 220,
        224, 228, 232, 236, 240,        244, 248, 252, 256, 260,
        264, 268, 272, 276, 280,        284, 288, 292, 296, 300,
        305, 310, 315, 320, 325,        330, 335, 340, 345, 350,
        355, 360, 365, 370, 375,        380, 385, 390, 395, 400,
        405, 410, 415, 420, 425,        430, 435, 440, 445, 450,
        460, 470, 480, 490, 500,        510, 520, 530, 540, 550
    };
                 int        dam;
  
    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam      = number_range( dam_each[level], dam_each[level] * 5 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_circle_of_love( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    AFFECT_DATA af;

    for ( vch = char_list; vch; vch = vch->next )
    {
        if ( vch->deleted || !vch->in_room )
            continue;
        if ( vch->in_room == ch->in_room && vch->position != POS_GHOST )
        {
            if ( !is_affected(vch, sn))
            {
		af.type       = sn;
		af.level      = level;
		af.duration   = level /3;
		af.location   = APPLY_HITROLL;
		af.modifier   = number_fuzzy( level / 4 );
		af.bitvector  = 0;
		affect_to_char( vch, &af );
     
		af.location  = APPLY_DAMROLL;
		af.modifier  = number_fuzzy( level / 3 );
		affect_to_char( vch, &af );
  
		af.location  = APPLY_SAVING_SPELL;
		af.modifier   = ( number_fuzzy( level / 2 ) * -1 );
		affect_to_char( vch, &af );

                send_to_char(AT_RED, "Your body has been energized by the Circle of Love.\n\r", vch);
             }
            continue;
        }
   
    }

    return SKPELL_NO_DAMAGE;
}

int spell_luck_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED4( victim, AFF_LUCK_SHIELD ) )
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
    
    if ( ! IS_SHIELDABLE( victim ) )
        return SKPELL_MISSED;
    
    af.type      = sn; 
    af.level     = level;
    af.duration  = 5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_LUCK_SHIELD;
    affect_to_char4( victim, &af );
    victim->shields += 1;
 
    send_to_char(AT_BLUE, "Your soul is infused with the luck of the gods.\n\r", victim );
    act(AT_BLUE, "$n's souls is infused with the luck of the gods.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_tongues( int sn, int level, CHAR_DATA *ch, void *vo )
{   
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if ( IS_AFFECTED4( victim, AFF_TONGUES ) )
        return SKPELL_MISSED;
    
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level ) + 10;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_TONGUES;
    affect_to_char4( victim, &af );
 
    send_to_char(AT_BLUE, "You feel enchanted with the knowledge of language.\n\r", victim );
    act(AT_BLUE, "$n has been enchanted with the knowledge of language.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_liquid_skin( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
        
    if ( IS_AFFECTED4( victim, AFF_LIQUID_SKIN ) )
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
    af.location  = APPLY_DAM_FIRE;
    af.modifier  = 75;
    af.bitvector = AFF_LIQUID_SKIN;
    affect_to_char4( victim, &af );
    victim->shields += 1;
 
    send_to_char(AT_DBLUE, "Your skin suddenly turns liquid.\n\r", victim );
    act(AT_DBLUE, "$n's skin suddenly turns liquid.", victim, NULL,NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_spiritual_hammer( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA           *blade;
    int                duration;
        
    if(ch->summon_timer > 0)
      {
      send_to_char(AT_BLUE,
       "You cast the spell, but nothing appears.\n\r", ch);
      return SKPELL_MISSED;
      }
    duration = level/5;
    blade = create_object( get_obj_index( OBJ_VNUM_SPIRITUAL_HAMMER ), ch->level );
    blade->timer = duration;
    act(AT_BLUE, "A spiritual hammer appears in the sky and descends into your hands.", ch, NULL, NULL, TO_CHAR );
    act(AT_BLUE, "A spiritual hammer appears in the sky and descends into $n's hands.", ch, NULL, NULL, TO_ROOM );
    obj_to_char( blade, ch );
  if(ch->mana < level * 2 && ((ch->class != CLASS_VAMPIRE) || (ch->class != CLASS_ANTI_PALADIN )))
  {
    act(AT_RED,
     "You don't have enough mana to call the hammer!", ch, NULL, NULL, TO_CHAR);
    return SKPELL_MISSED;
  }
  if(ch->bp < level / 2 && ((ch->class == CLASS_VAMPIRE) || (ch->class == CLASS_ANTI_PALADIN)))
  {
    act(AT_BLOOD,
     "You don't have enough blood to call the hammer!", ch, NULL, NULL, TO_CHAR);
    return SKPELL_MISSED;
  }
  if((ch->class != CLASS_VAMPIRE) || (ch->class != CLASS_ANTI_PALADIN))
    ch->mana -= level * 4;
  else
    ch->bp -= level * 2;
    ch->summon_timer = 50;
    return SKPELL_NO_DAMAGE;
}   

int spell_angelic_aura( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_AFFECTED4( victim, AFF_ANGELIC_AURA ) )
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
    af.location  = APPLY_DAM_UNHOLY;  
    af.modifier  = 75;
    af.bitvector = AFF_ANGELIC_AURA;
    affect_to_char4( victim, &af );
    victim->shields += 1;
 
    send_to_char(AT_YELLOW, "Your body sprouts a pair of ethereal wings.\n\r", victim );
    act(AT_YELLOW, "$n's body sprouts a pair of ethereal wings.", victim, NULL,NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_waterspike( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
          0,   0,   0,   0,  30,         35,  40,  45,  50,  55,
         60,  65,  70,  75,  80,         82,  84,  86,  88,  90,
         92,  94,  96,  98, 100,        102, 104, 106, 108, 110,
        112, 114, 116, 118, 120,        122, 124, 126, 128, 130,
        132, 134, 136, 138, 140,        142, 144, 146, 148, 150,
        152, 154, 156, 158, 160,        162, 164, 166, 168, 170,
        172, 174, 176, 178, 180,        182, 184, 186, 188, 190,
        192, 194, 196, 198, 200,        202, 204, 206, 208, 210,
        215, 220, 225, 230, 235,        240, 245, 250, 255, 260
    };
                 int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam      = number_range( dam_each[level] / 2, dam_each[level] * 8 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}
