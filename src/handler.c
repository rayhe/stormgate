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

/*$Id: handler.c,v 1.60 2005/04/10 16:29:00 tyrion Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



AFFECT_DATA *		affect_free;

/*
 * Local functions.
 */
int 	spell_duration(CHAR_DATA *victim, int sn);
void    perm_spell(CHAR_DATA *victim, int sn);
void    affect_modify   args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
void    affect_modify2  args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
void    affect_modify3  args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
void    affect_modify4  args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
bool    is_colcode      args( ( char code ) );


/*
 * External functions
 */
int obj_invcount    args( ( OBJ_DATA* obj, bool one_item ) );
int ch_invcount     args( ( CHAR_DATA* ch ) );
int ch_weightcount  args( ( CHAR_DATA* ch ) );
 
/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch )
{
    if ( ch->desc && ch->desc->original )
	ch = ch->desc->original;

    if ( IS_CODER( ch ) && ch->trust < 100000 )
      return 100000;

    if ( ch->trust != 0 )
	return ch->trust;

    if ( IS_NPC( ch ) && ch->level >= LEVEL_DEMIGOD )
	return LEVEL_DEMIGOD - 1;
    else
	return ch->level;
}



/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch )
{
    return 17 + ( ch->played + (int) ( current_time - ch->logon ) ) / 14400;

    /* 14400 assumes 30 second hours, 24 hours a day, 20 day - Kahn */
}



/*
 * Retrieve character's current strength.
 */
int get_curr_str( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].str_mod;
    value = 13 + mod;

    if ( IS_NPC( ch ) )
	 return URANGE( 3, ch->level / 3 + 3, 50 ); 

    if ( class_table[ch->class].attr_prime == APPLY_STR )
	max = 50;
    else
	max = 50;

    return URANGE( 3, ch->pcdata->perm_str + ch->pcdata->mod_str, max );
}



/*
 * Retrieve character's current intelligence.
 */
int get_curr_int( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].int_mod;
    value = 13 + mod;

    if ( IS_NPC( ch ) )
	 return URANGE( 3, ch->level / 3 + 3, 50 ); 

    if ( class_table[ch->class].attr_prime == APPLY_INT )
	max = 50;
    else
	max = 50;

    return URANGE( 3, ch->pcdata->perm_int + ch->pcdata->mod_int, max );
}



/*
 * Retrieve character's current wisdom.
 */
int get_curr_wis( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].wis_mod;
    value = 13 + mod;

    if ( IS_NPC( ch ) )
	 return URANGE( 3, ch->level / 3 + 3, 50 ); 

    if ( class_table[ch->class].attr_prime == APPLY_WIS )
	max = 50;
    else
	max = 50;

    return URANGE( 3, ch->pcdata->perm_wis + ch->pcdata->mod_wis, max );
}



/*
 * Retrieve character's current dexterity.
 */
int get_curr_dex( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].dex_mod;
    value = 13 + mod;

    if ( IS_NPC( ch ) )
	return URANGE( 3, ch->level / 3 + 3, 50 ); 

    if ( class_table[ch->class].attr_prime == APPLY_DEX )
	max = 50;
    else
	max = 50;

    return URANGE( 3, ch->pcdata->perm_dex + ch->pcdata->mod_dex, max );
}



/*
 * Retrieve character's current constitution.
 */
int get_curr_con( CHAR_DATA *ch )
{
    int max;
    int mod;
    int value;

    mod   = race_table[ch->race].con_mod;
    value = 13 + mod;

    if ( IS_NPC( ch ) )
	 return URANGE( 3, ch->level / 3 + 3, 50 ); 

    if ( class_table[ch->class].attr_prime == APPLY_CON )
	max = 50;
    else
	max = 50;

    return URANGE( 3, ch->pcdata->perm_con + ch->pcdata->mod_con, max );
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch )
{
    int carry = 0;

    if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
	return 1000;

    if ( IS_NPC( ch ) && IS_SET( ch->act, ACT_PET ) )
	return 0;
    
    carry = MAX_WEAR + 2 * get_curr_dex( ch ) + get_curr_str( ch ) * 4 + ( ch->level / 2 );
    if (ch->level >= LEVEL_DEMIGOD)
       return (carry + 50);
  
    return carry;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    int carry = 0;
    if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
	return 1000000;

    if ( IS_NPC( ch ) && IS_SET( ch->act, ACT_PET ) )
	return 0;
   
    carry = str_app[get_curr_str( ch )].carry;

    if (ch->level >= LEVEL_DEMIGOD)
       return (carry + 150);
    else if (ch->level >= (LEVEL_CHAMP))
       return (carry + 100);
    else if (ch->level >= (LEVEL_HERO3))
       return (carry + 50);
    else if (ch->level >= (LEVEL_HERO2))
       return (carry + 25);
    else if (ch->level >= (LEVEL_HERO1))
       return (carry + 12);
    else if (ch->level >= LEVEL_HERO)
       return (carry + 5);

    return carry;
}



/*
 * See if a string is one of the names of an object.
 * New is_name sent in by Alander.
 * Even newer is_name by Altrag.. :)
 * 'dagger silver' will now pick up '1.dagger silver', or
 * '1.silver dagger', instead of '1.dagger' regardless.
 * -- Altrag
 */


bool is_name( const char *str, char *namelist )
{
    char name [ MAX_INPUT_LENGTH ];
    char arg [ MAX_INPUT_LENGTH ];
    char *brk;

    brk = str_dup(str);

    brk = one_argument(brk, arg );

    if ( arg[0] == '\0' )
      return FALSE;

    for ( ; ; )
    {
        namelist = one_argument( namelist, name );
        if ( name[0] == '\0' )
            return FALSE;
        if ( !str_prefix( str, name ) )
            return TRUE;
    }
}



/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    OBJ_DATA *wield2;
    char      buf1 [ MAX_STRING_LENGTH ];
    AFFECT_DATA *paf2;
    int count = 0;
    int       mod;

/* XORPHOX */
    AFFECT_DATA af;
    int sn;
    int psn = -1;
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';
/* END */
/* MANAUX, for reference couting */
/* END */
    mod = paf->modifier;
/* Ok, we're adding reference counting to affects */

    for (paf2 = ch->affected; paf2; paf2 = paf2->next)
    {    
	if (paf2->deleted)
	continue;
    	if (paf2->bitvector == paf->bitvector)
	if (fAdd || paf!=paf2)
    	count++;
    }
/* now count holds the total number of affects that modify the bit... */
/* if we're adding the affect, then, we just make each 
affect->count = count
   provided that the bitvectors are equal.... */

    if ( fAdd )
    {
	SET_BIT   ( ch->affected_by, paf->bitvector );
	for (paf2 = paf; paf2; paf2=paf2->next)
	{
	if (paf2->deleted)
	continue;
	if (paf2->bitvector == paf->bitvector)
		paf2->count = count;
	}
    }
    else
    {
	if (count == 0)
	REMOVE_BIT( ch->affected_by, paf->bitvector );
	for (paf2 = ch->affected; paf2; paf2=paf2->next)
	{
	if (paf2->deleted)
		continue;
	if (paf2->bitvector == paf->bitvector)
		paf2->count = count;
	}
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
        sprintf( buf1, "Affect_modify: unknown location %d on %s.",
		paf->location, ch->name );
	bug ( buf1, 0 );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_str += mod;                         break;
    case APPLY_DEX:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_dex += mod;                         break;
    case APPLY_INT:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_int += mod;                         break;
    case APPLY_WIS:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_wis += mod;                         break;
    case APPLY_CON:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_con += mod;                         break;
    case APPLY_SEX:           ch->sex                   += mod; break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_AGE:						break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MANA:          ch->max_mana              += mod; break;
    case APPLY_BP:            ch->max_bp                += mod; break;
    case APPLY_ANTI_DIS:      ch->antidisarm            += mod; break;
    case APPLY_HIT:           ch->max_hit               += mod; break;
    case APPLY_MOVE:          ch->max_move              += mod; break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:            ch->armor                 += mod; break;
    case APPLY_HITROLL:       ch->hitroll               += mod; break;
    case APPLY_DAMROLL:       ch->damroll               += mod; break;
    case APPLY_SAVING_PARA:   ch->saving_throw          += mod; break;
    case APPLY_SAVING_ROD:    ch->saving_throw          += mod; break;
    case APPLY_SAVING_PETRI:  ch->saving_throw          += mod; break;
    case APPLY_SAVING_BREATH: ch->saving_throw          += mod; break;
    case APPLY_SAVING_SPELL:  ch->saving_throw          += mod; break;
    case APPLY_DAM_ACID:      ch->damage_mods[0] 	+= mod; break;
    case APPLY_DAM_HOLY:      ch->damage_mods[1] 	+= mod; break;
    case APPLY_DAM_MAGIC:      ch->damage_mods[2] 	+= mod; break;
    case APPLY_DAM_FIRE:      ch->damage_mods[3] 	+= mod; break;
    case APPLY_DAM_ENERGY:      ch->damage_mods[4] 	+= mod; break;
    case APPLY_DAM_WIND:     ch->damage_mods[5] 	+= mod; break;
    case APPLY_DAM_WATER:      ch->damage_mods[6] 	+= mod; break;
    case APPLY_DAM_ILLUSION:      ch->damage_mods[7] 	+= mod; break;
    case APPLY_DAM_DISPEL:      ch->damage_mods[8] 	+= mod; break;
    case APPLY_DAM_EARTH:      ch->damage_mods[9] 	+= mod; break;
    case APPLY_DAM_PSYCHIC:      ch->damage_mods[10] 	+= mod; break;
    case APPLY_DAM_POISON:      ch->damage_mods[11] 	+= mod; break;
    case APPLY_DAM_BREATH:      ch->damage_mods[12] 	+= mod; break;
    case APPLY_DAM_SUMMON:      ch->damage_mods[13] 	+= mod; break;
    case APPLY_DAM_PHYSICAL:      ch->damage_mods[14] 	+= mod; break;
    case APPLY_DAM_EXPLOSIVE:      ch->damage_mods[15] 	+= mod; break;
    case APPLY_DAM_SONG:      ch->damage_mods[16] 	+= mod; break;
    case APPLY_DAM_NAGAROM:      ch->damage_mods[17] 	+= mod; break;
    case APPLY_DAM_UNHOLY:      ch->damage_mods[18] 	+= mod; break;
    case APPLY_DAM_CLAN:      ch->damage_mods[19] 	+= mod; break;
/* XORPHOX perm spells */

/*
 * Sorted alphabetically. Leave anything that can fit on 1 line in alpha.
 * Anything else should be put at the bottom.
 *			- Ahsile
 */
	  case APPLY_AGE_SPELL:			psn = skill_lookup("age"); strcpy(buf, "The years seem to peel off $n's face."); break;
      case APPLY_ANGELIC_AURA:		psn = skill_lookup("angelic aura"); strcpy(buf, "The ethereal wings of $n dissipate."); break;		
      case APPLY_AURA_ANTI_MAGIC:	psn = skill_lookup("aura of anti-magic"); strcpy(buf, "The aura of anti-magic slowly vanishes."); break;			
	  case APPLY_BEND_LIGHT:		psn = skill_lookup("bend light"); break;
      case APPLY_BIOFEEDBACK:		psn = skill_lookup("biofeedback"); strcpy(buf, "$n is no longer surrounded by an electromagnetic aura."); break;		
	  case APPLY_BLADE:				psn = skill_lookup("blade barrier"); strcpy(buf, "The blades around $n's body disappear."); break;
      case APPLY_BLESS:				psn = skill_lookup("bless"); break;
	  case APPLY_BLIND:				psn = skill_lookup("blind"); break;
      case APPLY_BLOODSHIELD:		psn = skill_lookup("bloodshield"); strcpy( buf, "The blood around $n's body evaporates."); break;		
      case APPLY_CHANGE_SEX:		psn = skill_lookup("change sex"); strcpy(buf, "$n's body changes... a lot!"); break;
	  case APPLY_CHAOS:				psn = skill_lookup("chaos field"); strcpy(buf, "The chaos around $n fades away."); break;
      case APPLY_CLOAKING:			psn = skill_lookup("cloaking"); strcpy(buf, "$n stops hiding their items."); break;
	  case APPLY_CLOUD_OF_HEALING:	psn = skill_lookup("cloud of healing"); strcpy(buf, "The cloud around $n evaporates."); break;
	  case APPLY_COFIRE:			psn = skill_lookup("circle of fire"); strcpy(buf, "The ring of fire slowly burns out."); break;	
      case APPLY_COMBAT_MIND:		psn = skill_lookup("combat mind" ); break;		
	  case APPLY_CONFUSED:			psn = skill_lookup("confusion"); strcpy(buf, "$n looks more aware of their surroundings."); break;
	  case APPLY_CURSE:				psn = skill_lookup("curse"); break;
	  case APPLY_CURSE_NATURE:		psn = skill_lookup("curse of nature"); break;
	  case APPLY_DANCING:			psn = skill_lookup("dancing lights"); strcpy(buf, "The lights around $n's head disappear."); break;
      case APPLY_DETECT_EVIL:		psn = skill_lookup("detect evil"); break;		
      case APPLY_DETECT_HIDDEN:		psn = skill_lookup("detect hidden"); break;		
      case APPLY_DETECT_INVIS:		psn = skill_lookup("detect invis"); break;		
      case APPLY_DETECT_MAGIC:		psn = skill_lookup("detect magic"); break;		
      case APPLY_EARTHSHIELD:		psn = skill_lookup("earthshield"); strcpy(buf, "The earthen shield about $n's body dissipates."); break;		
      case APPLY_ESSENCE_OF_GAIA:	psn = skill_lookup("essence of gaia"); strcpy(buf, "The essence fused within $n slowly dissipates."); break;			
      case APPLY_ETHEREAL_SNAKE:	psn = skill_lookup("ethereal snake"); strcpy(buf, "The ethereal snake guarding $n fades out of existance."); break;			
      case APPLY_ETHEREAL_WOLF:		psn = skill_lookup("ethereal wolf"); strcpy(buf, "The ethereal wolf guarding $n fades out of existance.");break;		
      case APPLY_FAERIE_FIRE:		psn = skill_lookup("faerie fire"); strcpy(buf, "$n's outline fades."); break;		
      case APPLY_FIRESHIELD:		psn = skill_lookup("fireshield");  strcpy(buf, "The flames about $n's body burn out."); break;		
      case APPLY_FLAMING:			psn = skill_lookup("incinerate");  strcpy(buf, "The fires about $n are extinguised."); break;
	  case APPLY_FLYING:			psn = skill_lookup("fly"); strcpy(buf, "$n floats slowly to the ground."); break;	
      case APPLY_FORCE_OF_NATURE:	psn = skill_lookup("force of nature"); strcpy(buf, "The force of nature leaves $n's soul."); break;			
      case APPLY_FORESTWALK:		psn = skill_lookup("forestwalk"); strcpy(buf, "The power of the forest leaves $n."); break;		
	  case APPLY_FUMBLE:			psn = skill_lookup("fumble"); strcpy(buf, "$n gains control of their hands."); break;
      case APPLY_GHOST_SHIELD:		psn = skill_lookup("ghost shield"); strcpy(buf, "The ghost about $n's body disperse."); break;		
      case APPLY_GIANT_STRENGTH:	psn = skill_lookup("giant strength"); strcpy(buf, "$n no longer looks so mighty."); break;			
/* 
     case APPLY_GOLDEN_ARMOR:		psn = skill_lookup("golden armor"); strcpy(buf, "The golden glow of $n's armor fades."); break;		
      case APPLY_GOLDEN_SANCTUARY:	psn = skill_lookup("golden sanctuary"); strcpy(buf, "The golden glow of $n's sanctuary fades."); break;			
*/
      case APPLY_HASTE:				psn = skill_lookup("haste"); strcpy(buf, "$n slows down."); break;
      case APPLY_HOLY_PROTECTION:	psn = skill_lookup("holy protection");  strcpy(buf, "The protection of the Gods wears off."); break;			
      case APPLY_ICESHIELD:			psn = skill_lookup("iceshield"); strcpy(buf, "The icy crust about $n's body melts to a puddle."); break;	
      case APPLY_IMPROVED_HIDE:		psn = skill_lookup("improved hide"); strcpy(buf, "The improved hide has worn off."); break;		
      case APPLY_IMPROVED_INVIS:	psn = skill_lookup("improved invis"); strcpy(buf, "$n slowly fades into existence."); break;			
	  case APPLY_INERTIAL:			psn = skill_lookup("inertial barrier"); break;
      case APPLY_INFRARED:			psn = skill_lookup("infravision"); break;	
      case APPLY_INVISIBLE:			psn = skill_lookup("invis"); strcpy(buf, "$n slowly fades into existence."); break;	
      case APPLY_LEAF_SHIELD:		psn = skill_lookup("leaf shield"); strcpy(buf, "The swirling leaves about $n's body disperse."); break;		
      case APPLY_LIQUID_SKIN:		psn = skill_lookup("liquid skin"); strcpy(buf, "The skin of $n no longer looks liquid."); break;		
      case APPLY_LUCK_SHIELD:		psn = skill_lookup("luck shield"); strcpy(buf, "The luck of the gods leaves $n's soul."); break; 		
	  case APPLY_MALIGNIFY:			psn = skill_lookup("malifnify"); break;
      case APPLY_MANA_SHIELD:		psn = skill_lookup("mana shield"); strcpy(buf, "The mana shield above $n's head dispapears."); break;		
      case APPLY_MIST:				psn = skill_lookup("mist"); strcpy(buf, "The mist about $n's body slowly fades."); break;
      case APPLY_MOUNTAINWALK:		psn = skill_lookup("mountainwalk"); strcpy(buf, "The power of the mountains and hills leaves $n."); break;		
      case APPLY_NAGAROMS_CURSE:	psn = skill_lookup("nagaroms curse"); break;			
      case APPLY_PASS_DOOR:			psn = skill_lookup("pass door"); break;	
	  case APPLY_PEACE:				psn = skill_lookup("aura of peace"); strcpy(buf, "$n seems less peaceful."); break;
	  case APPLY_PESTILENCE:		psn = skill_lookup("pestilence"); strcpy(buf, "The pestilence leaves $n."); break;
      case APPLY_PLAINSWALK:		psn = skill_lookup("plainswalk"); strcpy(buf, "The power of the plains and deserts leaves $n."); break;		
	  case APPLY_POWER_LEAK:		psn = skill_lookup("power leak"); strcpy(buf, "$n looks less drained."); break;
      case APPLY_PRAYER:			psn = skill_lookup("prayer"); break;	
      case APPLY_PROTECT:			psn = skill_lookup("protection evil"); break;	
	  case APPLY_PROTECTION_GOOD:	psn = skill_lookup("protection good"); break;
      case APPLY_RANDOMSHIELD:		psn = skill_lookup("randomshield"); strcpy(buf, "The plethora of illusions about $n's body disperse."); break;		
      case APPLY_SANCTUARY:         psn = skill_lookup("sanctuary"); strcpy(buf, "The white aura around $n's body vanishes.");break;				
      case APPLY_UNHOLY_STRENGTH:         psn = skill_lookup("unholy strength"); strcpy(buf, "$n's body is no longer fused with the strength of the unholy.");break;				
      case APPLY_SATANIC_INFERNO:	psn = skill_lookup("satanic inferno"); strcpy(buf, "The Inferno about $n's body subsides."); break;			
      case APPLY_SCRY:				psn = skill_lookup("scry"); break;
      case APPLY_SHADOW_IMAGE:		psn = skill_lookup("shadow image"); strcpy(buf, "$n no longer appears to be in more than one location."); break;		
      case APPLY_SHOCKSHIELD:		psn = skill_lookup("shockshield"); strcpy(buf, "The electricity about $n's body flee's into the ground."); break;		
	  case APPLY_SLIT:				psn = skill_lookup("slit"); strcpy(buf, "The slit in $n's neck heals."); break;
      case APPLY_STEALTH:			psn = skill_lookup("stealth"); strcpy(buf, "Your stealth mode has worn off."); break;	
      case APPLY_SWAMPWALK:			psn = skill_lookup("swampwalk"); strcpy(buf, "The power of the swamps leaves $n."); break;	
	  case APPLY_TALE_OF_TERROR:	psn = skill_lookup("tale of terror"); strcpy(buf, "$n looks less afraid!"); break;
      case APPLY_TITAN_STRENGTH:	psn = skill_lookup("titan strength"); strcpy(buf, "$n no longer looks titanically strong."); break;			
      case APPLY_TONGUES:			psn = skill_lookup("tongues"); strcpy(buf, "The magical knowledge of language leaves $n."); break;
	  case APPLY_TORTURE:			psn = skill_lookup("tortured soul"); strcpy(buf, "$n looks less tortured."); break;
	  case APPLY_TRUESIGHT:			psn = skill_lookup("true sight"); break;

	  case APPLY_HEIGHTEN_SENSES:	
		sn = skill_lookup("heighten senses");
        if(fAdd)
        {
          affect_strip(ch, sn);
          af.type     = sn;
          
          af.duration  = mod;
          af.location  = APPLY_NONE;
          af.modifier  = 0;
          af.bitvector = AFF_DETECT_INVIS;
          affect_to_char(ch, &af);
          af.bitvector = AFF_DETECT_HIDDEN;
          affect_to_char(ch, &af);
          af.bitvector = AFF_INFRARED;
          affect_to_char(ch, &af);
          send_to_char(AT_BLUE, "Your senses are heightened.\n\r", ch );
        }
        else if ((IS_AFFECTED(ch, AFF_DETECT_INVIS)) && (IS_AFFECTED(ch, AFF_DETECT_HIDDEN) )
          && (IS_AFFECTED(ch, AFF_INFRARED ) ) )
        {
          affect_strip( ch, sn );
          send_to_char(AT_BLUE, skill_table[sn].msg_off, ch);
          send_to_char(AT_BLUE, "\n\r", ch);
        }
        break;
	case APPLY_OCCULUTUS_VISUM:
        sn = skill_lookup("occulutus visum");
        if(fAdd)
        {
          affect_strip(ch, sn);
          af.type     = sn;
          
          af.duration  = mod;
          af.location  = APPLY_NONE;
          af.modifier  = 0;
          af.bitvector = AFF_DETECT_INVIS;
          affect_to_char(ch, &af);
          af.bitvector = AFF_DETECT_HIDDEN;
          affect_to_char(ch, &af);
          send_to_char(AT_BLOOD, "Your eyes now see beyond the grave.\n\r", ch );
        }
        else if ((IS_AFFECTED(ch, AFF_DETECT_INVIS)) && (IS_AFFECTED(ch, AFF_DETECT_HIDDEN) ) )
        {
          affect_strip( ch, sn );
          send_to_char(AT_BLOOD, skill_table[sn].msg_off, ch);
          send_to_char(AT_BLOOD, "\n\r", ch);
        }
        break;   
    case APPLY_SNEAK:
        sn = skill_lookup("sneak");
        if(fAdd)
        {
          affect_strip(ch, sn);
          af.type      = sn;
          
          af.duration  = mod;
          af.location  = APPLY_NONE;
          af.modifier  = 0;
          af.bitvector = AFF_SNEAK;
          affect_to_char(ch, &af);
          send_to_char(AT_BLUE, "You move silently.\n\r", ch);
        }
        else if(IS_AFFECTED(ch, AFF_SNEAK))
        {
          affect_strip(ch, sn);
          send_to_char(AT_BLUE, skill_table[sn].msg_off, ch);
          send_to_char(AT_BLUE, "\n\r", ch);
        }
      break;
      case APPLY_QUICKNESS:
        sn = skill_lookup("quickness");
        if(fAdd)
        {
          affect_strip(ch, sn);
          af.type      = sn;
          
          af.duration  = mod;
          af.location  = APPLY_DEX;
	  af.modifier  = 1 + (ch->level >= 18) + (ch->level >= 25) + (ch->level >= 32);
          af.bitvector = AFF_HASTE;
          affect_to_char(ch, &af);
          send_to_char(AT_BLUE, "Your body moves quickly.\n\r", ch);
        }
        else if(IS_AFFECTED(ch, AFF_HASTE))
        {
          affect_strip(ch, sn);
          send_to_char(AT_BLUE, skill_table[sn].msg_off, ch);
          send_to_char(AT_BLUE, "\n\r", ch);
        }
      break;
      case APPLY_HIDE:
        if(fAdd)
        {
          if(IS_AFFECTED(ch, AFF_HIDE))
            REMOVE_BIT(ch->affected_by, AFF_HIDE);
          SET_BIT(ch->affected_by, AFF_HIDE);
          act(AT_BLUE, "$n blends into the shadows.\n\r", ch, NULL, NULL, TO_ROOM);
          send_to_char(AT_BLUE, "You blend into the shadows.\n\r", ch);
        }
        else if(IS_AFFECTED(ch, AFF_HIDE))
        {
          REMOVE_BIT(ch->affected_by, AFF_HIDE);
          send_to_char(AT_BLUE, "You fade out of the shadows.\n\r", ch);
        }
      break;

/* END */
    }

    if(psn != -1)
    {
      if(fAdd && !is_affected(ch, psn))
      {
        obj_cast_spell(psn, paf->level, ch, ch, NULL);
        perm_spell(ch, psn);
      }
      else if(!fAdd && spell_duration(ch, psn) == -1)
      {
			if (skill_table[psn].shield_bit)
			{
				obj_cast_spell(psn, paf->level, ch, ch, NULL);
			} else
			{
			    affect_strip(ch, psn);
				if(buf[0] != '\0')
					act(AT_BLUE, buf, ch, NULL, NULL, TO_ROOM);
				if(skill_table[psn].msg_off)
				{
					send_to_char(AT_BLUE, skill_table[psn].msg_off, ch);
					send_to_char(AT_BLUE, "\n\r", ch);
				}
			}
	  }
    }

    if ( IS_NPC( ch ) )
        return;

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( ( wield = get_eq_char( ch, WEAR_WIELD ) )
	&& get_obj_weight( wield ) > str_app[get_curr_str( ch )].wield )
    {
        if ( ( wield2 = get_eq_char( ch, WEAR_WIELD_2 ) ) )
        {
            if ( ( ( get_obj_weight( wield ) + get_obj_weight( wield2 ) )
                  > str_app[get_curr_str( ch )].wield )
                || !IS_SET( race_table[ ch->race ].race_abilities,
                           RACE_WEAPON_WIELD ) )
            {
                static int depth;

                if ( depth == 0 )
                {
                    depth++;
                    act(AT_RED, "You drop $p.", ch, wield2, NULL, TO_CHAR );
                    act(AT_RED, "$n drops $p.", ch, wield2, NULL, TO_ROOM );
                    obj_from_char( wield2 );
                    obj_to_room( wield2, ch->in_room );
                    depth--;
                }

            }
        }
        else
        if ( ( get_obj_weight( wield ) > str_app[get_curr_str( ch )].wield )
            || !IS_SET( race_table[ ch->race ].race_abilities,
                       RACE_WEAPON_WIELD ) )
        {
            static int depth;

            if ( depth == 0 )
            {
                depth++;
                act(AT_RED, "You drop $p.", ch, wield, NULL, TO_CHAR );
                act(AT_RED, "$n drops $p.", ch, wield, NULL, TO_ROOM );
                obj_from_char( wield );
                obj_to_room( wield, ch->in_room );
                depth--;
            }

        }
    }
    else if ( ( wield2 = get_eq_char( ch, WEAR_WIELD_2 ) )
             && ( get_obj_weight( wield2 ) > str_app[get_curr_str( ch )].wield
                 || !IS_SET( race_table[ ch->race ].race_abilities,
                            RACE_WEAPON_WIELD ) ) )
    {
        static int depth;

        if ( depth == 0 )
        {
            depth++;
            act(AT_RED, "You drop $p.", ch, wield2, NULL, TO_CHAR );
            act(AT_RED, "$n drops $p.", ch, wield2, NULL, TO_ROOM );
            obj_from_char( wield2 );
            obj_to_room( wield2, ch->in_room );
            depth--;
        }

    }

    return;
}

void affect_modify2( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    char      buf [ MAX_STRING_LENGTH ];
    int       mod;
    AFFECT_DATA *paf2;
    int	count = 0;

    mod = paf->modifier;
	for (paf2 = ch->affected2; paf2; paf2=paf2->next)
	{
	if (paf2->deleted)
		continue;
	if (paf2->bitvector == paf->bitvector)
	if (fAdd || paf2 != paf)
		count++;
	}

    if ( fAdd )
    {
	SET_BIT   ( ch->affected_by2, paf->bitvector );
	for (paf2 = ch->affected2; paf2; paf2=paf2->next)
	{
		if (paf2->bitvector == paf->bitvector)
		paf2->count = count;
	}
    }
    else
    {
	if (count == 0)
	REMOVE_BIT( ch->affected_by2, paf->bitvector );
	mod = 0 - mod;
	for (paf2= ch->affected2; paf2; paf2=paf2->next)
	{
		if (paf2->bitvector == paf->bitvector)
			paf2->count = count;
	}
    }

    switch ( paf->location )
    {
    default:
        sprintf( buf, "Affect_modify: unknown location %d on %s.",
		paf->location, ch->name );
	bug ( buf, 0 );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_str += mod;                         break;
    case APPLY_DEX:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_dex += mod;                         break;
    case APPLY_INT:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_int += mod;                         break;
    case APPLY_WIS:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_wis += mod;                         break;
    case APPLY_CON:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_con += mod;                         break;
    case APPLY_SEX:           ch->sex                   += mod; break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_AGE:						break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MANA:          ch->max_mana              += mod; break;
    case APPLY_BP:            ch->max_bp                += mod; break;
    case APPLY_ANTI_DIS:      ch->antidisarm            += mod; break;
    case APPLY_HIT:           ch->max_hit               += mod; break;
    case APPLY_MOVE:          ch->max_move              += mod; break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:            ch->armor                 += mod; break;
    case APPLY_HITROLL:       ch->hitroll               += mod; break;
    case APPLY_DAMROLL:       ch->damroll               += mod; break;
    case APPLY_SAVING_PARA:   ch->saving_throw          += mod; break;
    case APPLY_SAVING_ROD:    ch->saving_throw          += mod; break;
    case APPLY_SAVING_PETRI:  ch->saving_throw          += mod; break;
    case APPLY_SAVING_BREATH: ch->saving_throw          += mod; break;
    case APPLY_SAVING_SPELL:  ch->saving_throw          += mod; break;
    case APPLY_DAM_ACID:      ch->damage_mods[0] 	+= mod; break;
    case APPLY_DAM_HOLY:      ch->damage_mods[1] 	+= mod; break;
    case APPLY_DAM_MAGIC:      ch->damage_mods[2] 	+= mod; break;
    case APPLY_DAM_FIRE:      ch->damage_mods[3] 	+= mod; break;
    case APPLY_DAM_ENERGY:      ch->damage_mods[4] 	+= mod; break;
    case APPLY_DAM_WIND:     ch->damage_mods[5] 	+= mod; break;
    case APPLY_DAM_WATER:      ch->damage_mods[6] 	+= mod; break;
    case APPLY_DAM_ILLUSION:      ch->damage_mods[7] 	+= mod; break;
    case APPLY_DAM_DISPEL:      ch->damage_mods[8] 	+= mod; break;
    case APPLY_DAM_EARTH:      ch->damage_mods[9] 	+= mod; break;
    case APPLY_DAM_PSYCHIC:      ch->damage_mods[10] 	+= mod; break;
    case APPLY_DAM_POISON:      ch->damage_mods[11] 	+= mod; break;
    case APPLY_DAM_BREATH:      ch->damage_mods[12] 	+= mod; break;
    case APPLY_DAM_SUMMON:      ch->damage_mods[13] 	+= mod; break;
    case APPLY_DAM_PHYSICAL:      ch->damage_mods[14] 	+= mod; break;
    case APPLY_DAM_EXPLOSIVE:      ch->damage_mods[15] 	+= mod; break;
    case APPLY_DAM_SONG:      ch->damage_mods[16] 	+= mod; break;
    case APPLY_DAM_NAGAROM:      ch->damage_mods[17] 	+= mod; break;
    case APPLY_DAM_UNHOLY:      ch->damage_mods[18] 	+= mod; break;
    case APPLY_DAM_CLAN:      ch->damage_mods[19] 	+= mod; break;
 
   }

    if ( IS_NPC( ch ) )
        return;

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( ( wield = get_eq_char( ch, WEAR_WIELD ) )
	&& get_obj_weight( wield ) > str_app[get_curr_str( ch )].wield )
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act(AT_GREY, "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act(AT_GREY, "$n drops $p.", ch, wield, NULL, TO_ROOM );
	    obj_from_char( wield );
	    obj_to_room( wield, ch->in_room );
	    depth--;
	}
    }

    return;
}

void affect_modify3( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    char      buf [ MAX_STRING_LENGTH ];
    int       mod;

    mod = paf->modifier;

    if ( fAdd )
    {
	SET_BIT   ( ch->affected_by3, paf->bitvector );
    }
    else
    {
	REMOVE_BIT( ch->affected_by3, paf->bitvector );
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
        sprintf( buf, "Affect_modify: unknown location %d on %s.",
		paf->location, ch->name );
	bug ( buf, 0 );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_str += mod;                         break;
    case APPLY_DEX:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_dex += mod;                         break;
    case APPLY_INT:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_int += mod;                         break;
    case APPLY_WIS:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_wis += mod;                         break;
    case APPLY_CON:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_con += mod;                         break;
    case APPLY_SEX:           ch->sex                   += mod; break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_AGE:						break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MANA:          ch->max_mana              += mod; break;
    case APPLY_BP:            ch->max_bp                += mod; break;
    case APPLY_ANTI_DIS:      ch->antidisarm            += mod; break;
    case APPLY_HIT:           ch->max_hit               += mod; break;
    case APPLY_MOVE:          ch->max_move              += mod; break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:            ch->armor                 += mod; break;
    case APPLY_HITROLL:       ch->hitroll               += mod; break;
    case APPLY_DAMROLL:       ch->damroll               += mod; break;
    case APPLY_SAVING_PARA:   ch->saving_throw          += mod; break;
    case APPLY_SAVING_ROD:    ch->saving_throw          += mod; break;
    case APPLY_SAVING_PETRI:  ch->saving_throw          += mod; break;
    case APPLY_SAVING_BREATH: ch->saving_throw          += mod; break;
    case APPLY_SAVING_SPELL:  ch->saving_throw          += mod; break;
    case APPLY_DAM_ACID:      ch->damage_mods[0] 	+= mod; break;
    case APPLY_DAM_HOLY:      ch->damage_mods[1] 	+= mod; break;
    case APPLY_DAM_MAGIC:      ch->damage_mods[2] 	+= mod; break;
    case APPLY_DAM_FIRE:      ch->damage_mods[3] 	+= mod; break;
    case APPLY_DAM_ENERGY:      ch->damage_mods[4] 	+= mod; break;
    case APPLY_DAM_WIND:     ch->damage_mods[5] 	+= mod; break;
    case APPLY_DAM_WATER:      ch->damage_mods[6] 	+= mod; break;
    case APPLY_DAM_ILLUSION:      ch->damage_mods[7] 	+= mod; break;
    case APPLY_DAM_DISPEL:      ch->damage_mods[8] 	+= mod; break;
    case APPLY_DAM_EARTH:      ch->damage_mods[9] 	+= mod; break;
    case APPLY_DAM_PSYCHIC:      ch->damage_mods[10] 	+= mod; break;
    case APPLY_DAM_POISON:      ch->damage_mods[11] 	+= mod; break;
    case APPLY_DAM_BREATH:      ch->damage_mods[12] 	+= mod; break;
    case APPLY_DAM_SUMMON:      ch->damage_mods[13] 	+= mod; break;
    case APPLY_DAM_PHYSICAL:      ch->damage_mods[14] 	+= mod; break;
    case APPLY_DAM_EXPLOSIVE:      ch->damage_mods[15] 	+= mod; break;
    case APPLY_DAM_SONG:      ch->damage_mods[16] 	+= mod; break;
    case APPLY_DAM_NAGAROM:      ch->damage_mods[17] 	+= mod; break;
    case APPLY_DAM_UNHOLY:      ch->damage_mods[18] 	+= mod; break;
    case APPLY_DAM_CLAN:      ch->damage_mods[19] 	+= mod; break;

    }

    if ( IS_NPC( ch ) )
        return;

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( ( wield = get_eq_char( ch, WEAR_WIELD ) )
	&& get_obj_weight( wield ) > str_app[get_curr_str( ch )].wield )
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act(AT_GREY, "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act(AT_GREY, "$n drops $p.", ch, wield, NULL, TO_ROOM );
	    obj_from_char( wield );
	    obj_to_room( wield, ch->in_room );
	    depth--;
	}
    }

    return;
}

void affect_modify4( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    char      buf [ MAX_STRING_LENGTH ];
    int       mod;

    mod = paf->modifier;

    if ( fAdd )
    {
	SET_BIT   ( ch->affected_by4, paf->bitvector );
    }
    else
    {
	REMOVE_BIT( ch->affected_by4, paf->bitvector );
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
        sprintf( buf, "Affect_modify: unknown location %d on %s.",
		paf->location, ch->name );
	bug ( buf, 0 );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_str += mod;                         break;
    case APPLY_DEX:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_dex += mod;                         break;
    case APPLY_INT:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_int += mod;                         break;
    case APPLY_WIS:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_wis += mod;                         break;
    case APPLY_CON:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_con += mod;                         break;
    case APPLY_SEX:           ch->sex                   += mod; break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_AGE:						break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MANA:          ch->max_mana              += mod; break;
    case APPLY_BP:            ch->max_bp                += mod; break;
    case APPLY_ANTI_DIS:      ch->antidisarm            += mod; break;
    case APPLY_HIT:           ch->max_hit               += mod; break;
    case APPLY_MOVE:          ch->max_move              += mod; break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:            ch->armor                 += mod; break;
    case APPLY_HITROLL:       ch->hitroll               += mod; break;
    case APPLY_DAMROLL:       ch->damroll               += mod; break;
    case APPLY_SAVING_PARA:   ch->saving_throw          += mod; break;
    case APPLY_SAVING_ROD:    ch->saving_throw          += mod; break;
    case APPLY_SAVING_PETRI:  ch->saving_throw          += mod; break;
    case APPLY_SAVING_BREATH: ch->saving_throw          += mod; break;
    case APPLY_SAVING_SPELL:  ch->saving_throw          += mod; break;
    case APPLY_DAM_ACID:      ch->damage_mods[0] 	+= mod; break;
    case APPLY_DAM_HOLY:      ch->damage_mods[1] 	+= mod; break;
    case APPLY_DAM_MAGIC:      ch->damage_mods[2] 	+= mod; break;
    case APPLY_DAM_FIRE:      ch->damage_mods[3] 	+= mod; break;
    case APPLY_DAM_ENERGY:      ch->damage_mods[4] 	+= mod; break;
    case APPLY_DAM_WIND:     ch->damage_mods[5] 	+= mod; break;
    case APPLY_DAM_WATER:      ch->damage_mods[6] 	+= mod; break;
    case APPLY_DAM_ILLUSION:      ch->damage_mods[7] 	+= mod; break;
    case APPLY_DAM_DISPEL:      ch->damage_mods[8] 	+= mod; break;
    case APPLY_DAM_EARTH:      ch->damage_mods[9] 	+= mod; break;
    case APPLY_DAM_PSYCHIC:      ch->damage_mods[10] 	+= mod; break;
    case APPLY_DAM_POISON:      ch->damage_mods[11] 	+= mod; break;
    case APPLY_DAM_BREATH:      ch->damage_mods[12] 	+= mod; break;
    case APPLY_DAM_SUMMON:      ch->damage_mods[13] 	+= mod; break;
    case APPLY_DAM_PHYSICAL:      ch->damage_mods[14] 	+= mod; break;
    case APPLY_DAM_EXPLOSIVE:      ch->damage_mods[15] 	+= mod; break;
    case APPLY_DAM_SONG:      ch->damage_mods[16] 	+= mod; break;
    case APPLY_DAM_NAGAROM:      ch->damage_mods[17] 	+= mod; break;
    case APPLY_DAM_UNHOLY:      ch->damage_mods[18] 	+= mod; break;
    case APPLY_DAM_CLAN:      ch->damage_mods[19] 	+= mod; break;

    }

    if ( IS_NPC( ch ) )
        return;

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( ( wield = get_eq_char( ch, WEAR_WIELD ) )
	&& get_obj_weight( wield ) > str_app[get_curr_str( ch )].wield )
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act(AT_GREY, "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act(AT_GREY, "$n drops $p.", ch, wield, NULL, TO_ROOM );
	    obj_from_char( wield );
	    obj_to_room( wield, ch->in_room );
	    depth--;
	}
    }

    return;
}

void affect_modify_powers( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    char      buf [ MAX_STRING_LENGTH ];
    int       mod;

    mod = paf->modifier;

    if ( fAdd )
    {
        SET_BIT   ( ch->affected_by_powers, paf->bitvector );
    }
    else
    {
        REMOVE_BIT( ch->affected_by_powers, paf->bitvector );
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
        sprintf( buf, "Affect_modify_powers: unknown location %d on %s.",
		paf->location, ch->name );
	bug ( buf, 0 );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_str += mod;                         break;
    case APPLY_DEX:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_dex += mod;                         break;
    case APPLY_INT:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_int += mod;                         break;
    case APPLY_WIS:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_wis += mod;                         break;
    case APPLY_CON:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_con += mod;                         break;
    case APPLY_SEX:           ch->sex                   += mod; break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_AGE:						break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MANA:          ch->max_mana              += mod; break;
    case APPLY_BP:            ch->max_bp                += mod; break;
    case APPLY_ANTI_DIS:      ch->antidisarm            += mod; break;
    case APPLY_HIT:           ch->max_hit               += mod; break;
    case APPLY_MOVE:          ch->max_move              += mod; break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:            ch->armor                 += mod; break;
    case APPLY_HITROLL:       ch->hitroll               += mod; break;
    case APPLY_DAMROLL:       ch->damroll               += mod; break;
    case APPLY_SAVING_PARA:   ch->saving_throw          += mod; break;
    case APPLY_SAVING_ROD:    ch->saving_throw          += mod; break;
    case APPLY_SAVING_PETRI:  ch->saving_throw          += mod; break;
    case APPLY_SAVING_BREATH: ch->saving_throw          += mod; break;
    case APPLY_SAVING_SPELL:  ch->saving_throw          += mod; break;
    }

    if ( IS_NPC( ch ) )
        return;

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( ( wield = get_eq_char( ch, WEAR_WIELD ) )
	&& get_obj_weight( wield ) > str_app[get_curr_str( ch )].wield )
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act(AT_GREY, "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act(AT_GREY, "$n drops $p.", ch, wield, NULL, TO_ROOM );
	    obj_from_char( wield );
	    obj_to_room( wield, ch->in_room );
	    depth--;
	}
    }

    return;
}


void affect_modify_weaknesses( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield;
    char      buf [ MAX_STRING_LENGTH ];
    int       mod;

    mod = paf->modifier;

    if ( fAdd )
    {
        SET_BIT   ( ch->affected_by_weaknesses, paf->bitvector );
    }
    else
    {
        REMOVE_BIT( ch->affected_by_weaknesses, paf->bitvector );
	mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
        sprintf( buf, "Affect_modify: unknown location %d on %s.",
		paf->location, ch->name );
	bug ( buf, 0 );
	return;

    case APPLY_NONE:						break;
    case APPLY_STR:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_str += mod;                         break;
    case APPLY_DEX:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_dex += mod;                         break;
    case APPLY_INT:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_int += mod;                         break;
    case APPLY_WIS:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_wis += mod;                         break;
    case APPLY_CON:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->mod_con += mod;                         break;
    case APPLY_SEX:           ch->sex                   += mod; break;
    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_AGE:						break;
    case APPLY_HEIGHT:						break;
    case APPLY_WEIGHT:						break;
    case APPLY_MANA:          ch->max_mana              += mod; break;
    case APPLY_BP:            ch->max_bp                += mod; break;
    case APPLY_ANTI_DIS:      ch->antidisarm            += mod; break;
    case APPLY_HIT:           ch->max_hit               += mod; break;
    case APPLY_MOVE:          ch->max_move              += mod; break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;
    case APPLY_AC:            ch->armor                 += mod; break;
    case APPLY_HITROLL:       ch->hitroll               += mod; break;
    case APPLY_DAMROLL:       ch->damroll               += mod; break;
    case APPLY_SAVING_PARA:   ch->saving_throw          += mod; break;
    case APPLY_SAVING_ROD:    ch->saving_throw          += mod; break;
    case APPLY_SAVING_PETRI:  ch->saving_throw          += mod; break;
    case APPLY_SAVING_BREATH: ch->saving_throw          += mod; break;
    case APPLY_SAVING_SPELL:  ch->saving_throw          += mod; break;
    }

    if ( IS_NPC( ch ) )
        return;

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( ( wield = get_eq_char( ch, WEAR_WIELD ) )
	&& get_obj_weight( wield ) > str_app[get_curr_str( ch )].wield )
    {
	static int depth;

	if ( depth == 0 )
	{
	    depth++;
	    act(AT_GREY, "You drop $p.", ch, wield, NULL, TO_CHAR );
	    act(AT_GREY, "$n drops $p.", ch, wield, NULL, TO_ROOM );
	    obj_from_char( wield );
	    obj_to_room( wield, ch->in_room );
	    depth--;
	}
    }

    return;
}


/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

	paf_new		= new_affect();

    *paf_new		= *paf;
    paf_new->deleted    = FALSE;
    paf_new->next	= ch->affected;
    ch->affected	= paf_new;
    paf_new->count 	= 0;
    affect_modify( ch, paf_new, TRUE );
    return;
}

void affect_to_char2( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

	paf_new		= new_affect();

    *paf_new		= *paf;
    paf_new->deleted    = FALSE;
    paf_new->next	= ch->affected2;
    ch->affected2	= paf_new;

    affect_modify2( ch, paf_new, TRUE );
    return;
}


void affect_to_char3( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

	paf_new		= new_affect();

    *paf_new		= *paf;
    paf_new->deleted    = FALSE;
    paf_new->next	= ch->affected3;
    ch->affected3	= paf_new;

    affect_modify3( ch, paf_new, TRUE );
    return;
}

void affect_to_char4( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

	paf_new		= new_affect();

    *paf_new		= *paf;
    paf_new->deleted    = FALSE;
    paf_new->next	= ch->affected4;
    ch->affected4	= paf_new;

    affect_modify4( ch, paf_new, TRUE );
    return;
}

void affect_to_char_powers( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;
    
        paf_new         = new_affect();
     
    *paf_new            = *paf;
    paf_new->deleted    = FALSE;
    paf_new->next       = ch->affected_powers;
    ch->affected_powers = paf_new;
        
    affect_modify_powers( ch, paf_new, TRUE );
    return;
}    

void affect_to_char_weaknesses( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;
    
        paf_new         = new_affect();
     
    *paf_new            = *paf;
    paf_new->deleted    = FALSE;
    paf_new->next       = ch->affected_weaknesses;
    ch->affected_weaknesses = paf_new;
        
    affect_modify_weaknesses( ch, paf_new, TRUE );
    return;
}    


/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    if ( !ch->affected )
    {
      bug( "Affect_remove: no affect.", 0 );
	    return;
    }

    if ( paf->deleted ) return;
    affect_modify( ch, paf, FALSE );
    paf->deleted = TRUE;

    return;
}

void affect_remove2( CHAR_DATA *ch, AFFECT_DATA *paf )
{
  if ( !ch->affected2 )
  {
    bug( "Affect_remove2: no affect.", 0 );
    return;
  }

  if ( paf->deleted ) return;
  affect_modify2( ch, paf, FALSE );
  paf->deleted = TRUE;

  return;
}

void affect_remove3( CHAR_DATA *ch, AFFECT_DATA *paf )
{
  if ( !ch->affected3 )
  {
    bug( "Affect_remove3: no affect.", 0 );
    return;
  }

  if ( paf->deleted ) return;
  affect_modify3( ch, paf, FALSE );
  paf->deleted = TRUE;

  return;
}

void affect_remove4( CHAR_DATA *ch, AFFECT_DATA *paf )
{
  if ( !ch->affected4 )
  {
    bug( "Affect_remove4: no affect.", 0 );
    return;
  }

  if ( paf->deleted ) return;
  affect_modify4( ch, paf, FALSE );
  paf->deleted = TRUE;

  return;
}

void affect_remove_powers( CHAR_DATA *ch, AFFECT_DATA *paf )
{
  if ( !ch->affected_powers )
  {
    bug( "Affect_remove_powers: no affect.", 0 );
    return;
  }  

  if ( paf->deleted ) return;
  affect_modify_powers( ch, paf, FALSE );
  paf->deleted = TRUE;

  return;
}

void affect_remove_weaknesses( CHAR_DATA *ch, AFFECT_DATA *paf )
{
  if ( !ch->affected_weaknesses )
  {
    bug( "Affect_remove_weaknesses: no affect.", 0 );
    return;
  }  

  if ( paf->deleted ) return;
  affect_modify_weaknesses( ch, paf, FALSE );
  paf->deleted = TRUE;

  return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf; paf = paf->next )
    {
        if ( paf->deleted )
	    continue;
	if ( paf->type == sn )
	    affect_remove( ch, paf );
    }

    for ( paf = ch->affected2; paf; paf = paf->next )
    {
      if ( paf->deleted )
	continue;
      if ( paf->type == sn )
	affect_remove2( ch, paf );
    }

    for ( paf = ch->affected3; paf; paf = paf->next )
    {
      if ( paf->deleted )
	continue;
      if ( paf->type == sn )
	affect_remove3( ch, paf );
    }

    for ( paf = ch->affected4; paf; paf = paf->next )
    {
      if ( paf->deleted )
	continue;
      if ( paf->type == sn )
	affect_remove4( ch, paf );
    }

    for ( paf = ch->affected_powers; paf; paf = paf->next )
    {
      if ( paf->deleted )
        continue;
      if ( paf->type == sn )
        affect_remove_powers( ch, paf );
    }

    for ( paf = ch->affected_weaknesses; paf; paf = paf->next )
    {
      if ( paf->deleted )
        continue;
      if ( paf->type == sn )
        affect_remove_weaknesses( ch, paf );
    }

    return;
}



/*
 * Return true if a char is affected by a spell.
 */
bool is_affected( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf; paf = paf->next )
    {
        if ( paf->deleted )
	    continue;
	if ( paf->type == sn )
	    return TRUE;
    }
    for ( paf = ch->affected2; paf; paf = paf->next )
    {
      if ( paf->deleted )
	continue;
      if ( paf->type == sn )
	return TRUE;
    }
    for ( paf = ch->affected3; paf; paf = paf->next )
    {
      if ( paf->deleted )
        continue;
      if ( paf->type == sn )
        return TRUE;
    }
    for ( paf = ch->affected4; paf; paf = paf->next )
    {
      if ( paf->deleted )
        continue;
      if ( paf->type == sn )
        return TRUE;
    }
    for ( paf = ch->affected_powers; paf; paf = paf->next )
    {
      if ( paf->deleted )
        continue;
      if ( paf->type == sn )
        return TRUE;
    }
    for ( paf = ch->affected_weaknesses; paf; paf = paf->next )
    {
      if ( paf->deleted )
        continue;
      if ( paf->type == sn )
        return TRUE;
    }

    return FALSE;
}



/*
 * Add or enhance an affect.
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;
    bool         found;

    found = FALSE;
    for ( paf_old = ch->affected; paf_old; paf_old = paf_old->next )
    {
        if ( paf_old->deleted )
	    continue;
	if ( paf_old->type == paf->type )
	{
	    paf->duration += paf_old->duration;
	    paf->modifier += paf_old->modifier;
	    affect_remove( ch, paf_old );
	    break;
	}
    }

    affect_to_char( ch, paf );
    return;
}

void affect_join2( CHAR_DATA *ch, AFFECT_DATA *paf )
{
  AFFECT_DATA *paf_old;
  bool         found;

  found = FALSE;
  for ( paf_old = ch->affected2; paf_old; paf_old = paf_old->next )
  {
    if ( paf_old->deleted )
      continue;
    if ( paf_old->type == paf->type )
    {
      paf->duration += paf_old->duration;
      paf->modifier += paf_old->modifier;
      affect_remove2( ch, paf_old );
      break;
    }
  }

  affect_to_char2( ch, paf );
  return;
}

/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( !ch->in_room )
    {
	bug( "Char_from_room: NULL.", 0 );
	return;
    }

    if ( !IS_NPC( ch ) && ch->in_room->area )
	--ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) )
	&& obj->item_type == ITEM_LIGHT
	&& obj->value[2] != 0
	&& ch->in_room->light > 0 )
	--ch->in_room->light;

    if ( ch == ch->in_room->people )
    {
	ch->in_room->people = ch->next_in_room;
    }
    else
    {
	CHAR_DATA *prev;

	for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
	{
	    if ( prev->next_in_room == ch )
	    {
		prev->next_in_room = ch->next_in_room;
		break;
	    }
	}

	if ( !prev )
	    bug( "Char_from_room: ch not found.", 0 );
    }

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    return;
}



/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;

    if ( !pRoomIndex )
    {
	bug( "Char_to_room: NULL.", 0 );
	return;
    }

    ch->in_room		= pRoomIndex;
    ch->next_in_room	= pRoomIndex->people;
    pRoomIndex->people	= ch;

    if ( !IS_NPC( ch ) && ch->in_room->area )
	++ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) )
	&& obj->item_type == ITEM_LIGHT
	&& obj->value[2] != 0 )
	++ch->in_room->light;

    return;
}



/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && ( obj->item_type == ITEM_BULLET || obj->item_type == ITEM_ARROW || obj->item_type == ITEM_BOLT ) )
    {
	OBJ_DATA* search;
	for (search = ch->carrying; search; search = search->next_content )
	{
		if (obj->deleted) { continue; }
		if ( obj->item_type == search->item_type && search->level == obj->level && search->value[1] == obj->value[1] )
		{
			char buf[MAX_STRING_LENGTH];
			int stack_tot = obj->value[0] + search->value[0];
	
			/* average damage values, they could be different */
			search->value[2] = (int) ( ( obj->value[2] * ( (double) obj->value[0] / stack_tot ) )
					   + ( search->value[2] * ( (double) search->value[0] / stack_tot ) ) );
			search->value[3] = (int) ( ( obj->value[3] * ( (double) obj->value[0] / stack_tot ) )
			 		   + ( search->value[3] * ( (double) search->value[0] / stack_tot ) ) );

			sprintf( buf, "You add a stack of %d %s to a stack of %d %s.\n\r",
					obj->value[0], obj->short_descr,
					search->value[0], search->short_descr );
			send_to_char( C_DEFAULT, buf, ch );

			search->value[0] += obj->value[0];
			obj->deleted = TRUE;
			return;
		}
	}
    }
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    obj->stored_by       = NULL;
    ch->carry_number     = ch_invcount( ch );
    ch->carry_weight     = ch_weightcount( ch );
}

/*
 * -- Altrag
 */
void obj_to_storage( OBJ_DATA *obj, CHAR_DATA *ch )
{
    if ( IS_NPC( ch ) )
    {
      bug( "obj_to_storage: NPC storage from %d", ch->pIndexData->vnum );
      obj_to_char( obj, ch );
      return;
    }
    obj->next_content    = ch->pcdata->storage;
    ch->pcdata->storage  = obj;
    obj->carried_by      = NULL;
    obj->in_room         = NULL;
    obj->in_obj          = NULL;
    obj->stored_by       = ch;
    ch->pcdata->storcount += get_obj_number( obj );
}

/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;

    if ( !( ch = obj->carried_by ) )
    {
	bug( "Obj_from_char: null ch.", 0 );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
	unequip_char( ch, obj );

    if ( ch->carrying == obj )
    {
	ch->carrying = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = ch->carrying; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( !prev )
	    bug( "Obj_from_char: obj not in list.", 0 );
    }

    obj->carried_by      = NULL;
    obj->next_content	 = NULL;
    ch->carry_number	 = ch_invcount( ch );
    ch->carry_weight	 = ch_weightcount( ch );
    return;
}

/*
 * -- Altrag
 */
void obj_from_storage( OBJ_DATA *obj )
{
  CHAR_DATA *ch;

  if ( !( ch = obj->stored_by ) )
  {
    bug( "obj_from_storage: NULL ch.", 0 );
    return;
  }

  if ( IS_NPC( ch ) )
  {
    bug( "obj_from_storage: NPC storage by %d.", ch->pIndexData->vnum );
    return;
  }

  if ( ch->pcdata->storage == obj )
  {
    ch->pcdata->storage = obj->next_content;
  }
  else
  {
    OBJ_DATA *prev;

    for ( prev = ch->pcdata->storage; prev; prev = prev->next_content )
    {
      if ( prev->next_content == obj )
      {
	prev->next_content = obj->next_content;
	break;
      }
    }

    if ( !prev )
      bug( "Obj_from_storage: obj not in list.", 0 );
  }

  obj->stored_by = NULL;
  obj->next_content = NULL;
  ch->pcdata->storcount-= get_obj_number( obj );
  return;
}

/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear )
{
    if ( obj->item_type != ITEM_ARMOR )
	return 0;

    switch ( iWear )
    {
    case WEAR_BODY:     return 3 * obj->value[0];
    case WEAR_HEAD:	return 2 * obj->value[0];
    case WEAR_LEGS:	return 2 * obj->value[0];
    case WEAR_FEET:	return     obj->value[0];
    case WEAR_HANDS:	return     obj->value[0];
    case WEAR_ARMS:	return     obj->value[0];
    case WEAR_SHIELD:	return     obj->value[0];
    case WEAR_FINGER_L:	return     obj->value[0];
    case WEAR_FINGER_R: return     obj->value[0];
    case WEAR_NECK_1:	return     obj->value[0];
    case WEAR_NECK_2:	return     obj->value[0];
    case WEAR_ABOUT:	return 2 * obj->value[0];
    case WEAR_WAIST:	return     obj->value[0];
    case WEAR_WRIST_L:	return     obj->value[0];
    case WEAR_WRIST_R:	return     obj->value[0];
    case WEAR_HOLD:	return     obj->value[0];
    case WEAR_ON_FACE:  return     obj->value[0];
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
        if ( obj->deleted )
	    continue;
	if ( obj->wear_loc == iWear )
	    return obj;
    }

    return NULL;
}



/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    AFFECT_DATA *paf;
    char         buf [ MAX_STRING_LENGTH ];

    if ( get_eq_char( ch, iWear ) )
    {
        sprintf( buf, "Equip_char: %s (%s) already equipped at %d.",
		ch->name, ch->short_descr, iWear );
	bug( buf, 0 );
	return;
    }

    if ( !IS_NPC( ch ) ) {
    if (   ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL   ) && IS_EVIL   ( ch ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_GOOD   ) && IS_GOOD   ( ch ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL( ch ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_MAGE   ) && ( ch->class == 0 || ch->multied == 0 ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_CLERIC ) && ( ch->class == 1 || ch->multied == 1 ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_THIEF  ) && ( ch->class == 2 || ch->multied == 2 ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_WARRIOR) && ( ch->class == 3 || ch->multied == 3 ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_PSI    ) && ( ch->class == 4 || ch->multied == 4 ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_DRUID  ) && ( ch->class == 5 || ch->multied == 5 ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_RANGER ) && ( ch->class == 6 || ch->multied == 6 ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_PALADIN) && ( ch->class == 7 || ch->multied == 7 ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_BARD   ) && ( ch->class == 8 || ch->multied == 8 ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_VAMP   ) && ( ch->class == 9 || ch->multied == 9 ) )
        || ( IS_OBJ_STAT2( obj, ITEM_ANTI_WEREWOLF ) && ( ch->class == 10 || ch->multied == 10 ) )
        || ( IS_OBJ_STAT2( obj, ITEM_ANTI_ANTIPAL  ) && ( ch->class == 11 || ch->multied == 11 ) )
        || ( IS_OBJ_STAT2( obj, ITEM_ANTI_ASSASSIN ) && ( ch->class == 12 || ch->multied == 12 ) )
        || ( IS_OBJ_STAT2( obj, ITEM_ANTI_MONK     ) && ( ch->class == 13 || ch->multied == 13 ) )
        || ( IS_OBJ_STAT2( obj, ITEM_ANTI_BARBARIAN) && ( ch->class == 14 || ch->multied == 14 ) )
        || ( IS_OBJ_STAT2( obj, ITEM_ANTI_ILLUSIONIST) && ( ch->class == 15 || ch->multied == 15 ) )
	|| ( IS_OBJ_STAT2( obj, ITEM_ANTI_NECROMANCER) && ( ch->class == 16 || ch->multied == 16 ) )
	|| ( IS_OBJ_STAT2( obj, ITEM_ANTI_DEMONOLOGIST) && ( ch->class == 17 || ch->multied == 17 ) )
	|| ( IS_OBJ_STAT2( obj, ITEM_ANTI_SHAMAN) && ( ch->class == CLASS_SHAMAN || ch->multied == CLASS_SHAMAN ) )
	|| ( IS_OBJ_STAT2( obj, ITEM_ANTI_DARKPRIEST) && ( ch->class == 19 || ch->multied == 19 ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_MAGE     ) && ( ch->class != CLASS_MAGE && ch->multied != CLASS_MAGE ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_CLERIC   ) && ( ch->class != CLASS_CLERIC && ch->multied != CLASS_CLERIC ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_THIEF    ) && ( ch->class != CLASS_THIEF && ch->multied != CLASS_THIEF ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_WARRIOR  ) && ( ch->class != CLASS_WARRIOR && ch->multied != CLASS_WARRIOR ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_PSI      ) && ( ch->class != CLASS_PSIONICIST && ch->multied != CLASS_PSIONICIST ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_DRUID    ) && ( ch->class != CLASS_DRUID && ch->multied != CLASS_DRUID ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_RANGER   ) && ( ch->class != CLASS_RANGER && ch->multied != CLASS_RANGER ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_PALADIN  ) && ( ch->class != CLASS_PALADIN && ch->multied != CLASS_PALADIN ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_BARD     ) && ( ch->class != CLASS_BARD && ch->multied != CLASS_BARD ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_VAMP     ) && ( ch->class != CLASS_VAMPIRE && ch->multied != CLASS_VAMPIRE) ) 
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_WEREWOLF ) && ( ch->class != CLASS_WEREWOLF && ch->multied != CLASS_WEREWOLF ) ) 
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_ANTIPAL  ) && ( ch->class != CLASS_ANTI_PALADIN && ch->multied != CLASS_ANTI_PALADIN ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_ASSASSIN ) && ( ch->class != CLASS_ASSASSIN && ch->multied != CLASS_ASSASSIN ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_MONK     ) && ( ch->class != CLASS_MONK && ch->multied != CLASS_MONK ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_BARBARIAN) && ( ch->class != CLASS_BARBARIAN && ch->multied != CLASS_BARBARIAN ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_ILLUSIONIST) && ( ch->class != CLASS_ILLUSIONIST && ch->multied != CLASS_ILLUSIONIST ) ) 
      || ( IS_OBJ_STAT3( obj, ITEM_PRO_NECROMANCER ) && ( ch->class != CLASS_NECROMANCER && ch->multied != CLASS_NECROMANCER ) )
      || ( IS_OBJ_STAT3( obj, ITEM_PRO_DEMONOLOGIST ) && ( ch->class != CLASS_DEMONOLOGIST && ch->multied != CLASS_DEMONOLOGIST ) )
	|| ( IS_OBJ_STAT3( obj, ITEM_PRO_SHAMAN ) && ( ch->class != CLASS_SHAMAN && ch->multied != CLASS_SHAMAN ) )
      || ( IS_OBJ_STAT3( obj, ITEM_PRO_DARKPRIEST ) && ( ch->class != CLASS_DARKPRIEST && ch->multied != CLASS_DARKPRIEST ) )
      || ( IS_OBJ_STAT3( obj, ITEM_PRO_HUMAN )    && ( ch->race != 0 ) )
      || ( IS_OBJ_STAT3( obj, ITEM_PRO_ELF )      && ( ch->race != 1 ) )
      || ( IS_OBJ_STAT3( obj, ITEM_PRO_HALFELF )  && ( ch->race != 2 ) )
      || ( IS_OBJ_STAT3( obj, ITEM_PRO_ORC )      && ( ch->race != 3 ) )
      || ( IS_OBJ_STAT3( obj, ITEM_PRO_DROW )     && ( ch->race != 4 ) )
      || ( IS_OBJ_STAT3( obj, ITEM_PRO_DWARF )    && ( ch->race != 5 ) )
      || ( IS_OBJ_STAT3( obj, ITEM_PRO_HALFDWARF )&& ( ch->race != 6 ) )
      || ( IS_OBJ_STAT3( obj, ITEM_PRO_HOBBIT )   && ( ch->race != 7 ) )
      || ( IS_OBJ_STAT3( obj, ITEM_PRO_GIANT )    && ( ch->race != 8 ) )
      || ( IS_OBJ_STAT3( obj, ITEM_PRO_OGRE )     && ( ch->race != 9 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_PRO_ANGEL )    && ( ch->race != 10 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_PRO_MINOTAUR ) && ( ch->race != 11 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_PRO_FELINE )   && ( ch->race != 12 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_PRO_GARGOYLE ) && ( ch->race != 20 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_PRO_CANINE )   && ( ch->race != 14 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_PRO_DEMON )    && ( ch->race != 15 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_PRO_PIXIE )    && ( ch->race != 16 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_PRO_ELDER )    && ( ch->race != 17 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_PRO_LIZARDMAN )&& ( ch->race != 18 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_PRO_GNOME )    && ( ch->race != 19 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_HUMAN )    && ( ch->race == 0 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_ELF )      && ( ch->race == 1 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_HALFELF )  && ( ch->race == 2 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_ORC )      && ( ch->race == 3 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_DROW )     && ( ch->race == 4 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_DWARF )    && ( ch->race == 5 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_HALFDWARF )&& ( ch->race == 6 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_HOBBIT )   && ( ch->race == 7 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_GIANT )    && ( ch->race == 8 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_OGRE )     && ( ch->race == 9 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_ANGEL )    && ( ch->race == 10 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_MINOTAUR ) && ( ch->race == 11 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_FELINE )   && ( ch->race == 12 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_GARGOYLE ) && ( ch->race == 20 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_CANINE )   && ( ch->race == 14 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_DEMON )    && ( ch->race == 15 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_PIXIE )    && ( ch->race == 16 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_ELDER )    && ( ch->race == 17 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_LIZARDMAN )&& ( ch->race == 18 ) )
      || ( IS_OBJ_STAT4( obj, ITEM_ANTI_GNOME )    && ( ch->race == 19 ) ) )
    {
	/*
	 * Thanks to Morgenes for the bug fix here!
	 */
	act(AT_BLUE, "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	return;
    } else if ((IS_OBJ_STAT2( obj, ITEM_LEGEND )) && (!IS_LEGEND( ch )))
    {
	act(AT_BLUE, "You somehow don't feel powerful enough for $p and set it on the ground...", ch, obj, NULL, TO_CHAR);
        act(AT_BLUE, "$n attempts to wear $p, thinks better of it and sets it on the ground.",  ch, obj, NULL, TO_ROOM );
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        return;
    }
    }

    ch->armor      	-= apply_ac( obj, iWear );
    obj->wear_loc	 = iWear;

    for ( paf = obj->pIndexData->affected; paf; paf = paf->next )
	affect_modify( ch, paf, TRUE );
    for ( paf = obj->affected; paf; paf = paf->next )
	affect_modify( ch, paf, TRUE );

    if ( obj->item_type == ITEM_LIGHT
	&& obj->value[2] != 0
	&& ch->in_room )
	++ch->in_room->light;
    ch->carry_number -= get_obj_number( obj );
    oprog_wear_trigger( obj, ch );
    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf;
    char         buf [ MAX_STRING_LENGTH ];

    if ( obj->wear_loc == WEAR_NONE )
    {
        sprintf( buf, "Unequip_char: %s already unequipped with %d.",
		ch->name, obj->pIndexData->vnum );
	bug( buf, 0 );
	return;
    }

    ch->armor		+= apply_ac( obj, obj->wear_loc );
    obj->wear_loc	 = -1;

    for ( paf = obj->pIndexData->affected; paf; paf = paf->next )
	affect_modify( ch, paf, FALSE );
    for ( paf = obj->affected; paf; paf = paf->next )
	affect_modify( ch, paf, FALSE );

    if ( obj->item_type == ITEM_LIGHT
	&& obj->value[2] != 0
	&& ch->in_room
	&& ch->in_room->light > 0 )
	--ch->in_room->light;
    ch->carry_number += get_obj_number( obj );
    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int       nMatch;

    nMatch = 0;
    for ( obj = list; obj; obj = obj->next_content )
    {
        if ( obj->deleted )
	    continue;
	if ( obj->pIndexData == pObjIndex )
	    nMatch++;
    }

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *in_room;

    if ( !( in_room = obj->in_room ) )
    {
	bug( "obj_from_room: NULL.", 0 );
	return;
    }

    if ( obj == in_room->contents )
    {
	in_room->contents = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = in_room->contents; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( !prev )
	{
	    bug( "Obj_from_room: obj not found.", 0 );
	    return;
	}
    }

    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex )
{
    if ( pRoomIndex->vnum > 32000 )
       pRoomIndex->vnum = 1;
    obj->next_content		= pRoomIndex->contents;
    pRoomIndex->contents	= obj;
    obj->in_room		= pRoomIndex;
    obj->carried_by		= NULL;
    obj->in_obj			= NULL;
    obj->stored_by              = NULL;
    return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    if ( obj_to->deleted )
    {
	bug( "Obj_to_obj:  Obj_to already deleted", 0 );
        return;
    }

    obj->next_content		= obj_to->contains;
    obj_to->contains		= obj;
    obj->in_obj			= obj_to;
    obj->in_room		= NULL;
    obj->carried_by		= NULL;
    obj->stored_by              = NULL;

    return;
}


/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( !( obj_from = obj->in_obj ) )
    {
	bug( "Obj_from_obj: null obj_from.", 0 );
	return;
    }

    if ( obj == obj_from->contains )
    {
	obj_from->contains = obj->next_content;
    }
    else
    {
	OBJ_DATA *prev;

	for ( prev = obj_from->contains; prev; prev = prev->next_content )
	{
	    if ( prev->next_content == obj )
	    {
		prev->next_content = obj->next_content;
		break;
	    }
	}

	if ( !prev )
	{
	    bug( "Obj_from_obj: obj not found.", 0 );
	    return;
	}
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

    return;
}



/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj )
{
  OBJ_DATA *obj_content;
  OBJ_DATA *obj_next;
  extern bool      delete_obj;

  if ( obj->deleted )
  {
    bug( "Extract_obj:  Obj already deleted", 0 );
    return;
  }

  if ( obj->in_room    )
    obj_from_room( obj );
  else if ( obj->carried_by )
    obj_from_char( obj );
  else if ( obj->in_obj     )
    obj_from_obj( obj  );
  else if ( obj->stored_by )
    obj_from_storage( obj );

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
        obj_next = obj_content->next_content;
	if( obj_content->deleted )
	    continue;
	extract_obj( obj_content );
    }

    obj->deleted = TRUE;

    delete_obj   = TRUE;
    return;
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
           CHAR_DATA *wch;
           OBJ_DATA  *obj;
           OBJ_DATA  *obj_next;
    extern bool       delete_char;

#ifdef SQL_SYSTEM
    if ( !ch->in_room && !ch->last_room)
#else
	if ( !ch->in_room )
#endif
    {
	bug( "Extract_char: NULL.", 0 );
	return;
    }

    if ( fPull )
    {
	char* name;

	if ( IS_NPC ( ch ) )
	    name = ch->short_descr;
	else
	    name = ch->name;

	die_follower( ch, name );
    }

    stop_fighting( ch, TRUE );

    for ( obj = ch->carrying; obj; obj = obj_next )
    {
        obj_next = obj->next_content;
        if ( obj->deleted )
	    continue;
	extract_obj( obj );
    }
   
#ifdef SQL_SYSTEM
	if (ch->in_room)
#endif
    char_from_room( ch );

    if ( !fPull )
    {
        ROOM_INDEX_DATA *location;
	RELIGION_DATA* pRel = get_religion_index( ch->religion );

	if ( !( location = get_room_index( ROOM_VNUM_PURGATORY_A ) ) )
	  {
	    bug( "Purgatory A does not exist!", 0 );
	    char_to_room( ch, get_room_index( ROOM_VNUM_ALTAR ) );
	  }
	else
	{
	    if (pRel->recall != ROOM_VNUM_OUTSIDEMETH)
              char_to_room( ch, get_room_index( pRel->recall ) );
	    else
	      char_to_room( ch, location );
	}
	return;
    }

    if ( IS_NPC( ch ) )
	--ch->pIndexData->count;

    if ( ch->desc && ch->desc->original )
	do_return( ch, "" );

    for ( wch = char_list; wch; wch = wch->next )
    {
	if ( wch->reply == ch )
	    wch->reply = NULL;
	if ( wch->hunting == ch )
	    wch->hunting = NULL;
	if ( wch->questgiver == ch )
	{
	    wch->questgiver = NULL;
	    wch->countdown  = 0;
	    wch->nextquest  = 1;
	    wch->questmob   = 0;
	    wch->questobj  = 0;
	    send_to_char( AT_YELLOW, "Your quest giver has been slain. Resetting quest data. We apologize.\n\r", ch);
	}
    }

    ch->deleted = TRUE;

    if ( ch->desc )
	ch->desc->character = NULL;

    delete_char = TRUE;
    return;
}



/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    char       arg [ MAX_INPUT_LENGTH ];
    int        number;
    int        count;

    number = number_argument( argument, arg );
    count  = 0;
    if ( !str_cmp( arg, "self" ) )
		return ch;

	if (!ch->in_room)
		return NULL;

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
	    continue;
	if ( ++count == number )
	    return rch;
    }

    return NULL;
}


/*
 * Find a char in an area.
 * (from get_char_world)
 *
 * (by Mikko Kilpikoski 09-Jun-94)
 */
CHAR_DATA *get_char_area( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *ach;
  int number;
  int count;

  if ( ( ach = get_char_room( ch, argument ) ) != NULL )
    return ach;

  number = number_argument( argument, arg );
  count  = 0;
  for ( ach = char_list; ach != NULL ; ach = ach->next )
    {
      if ( ach->in_room->area != ch->in_room->area
          || !can_see( ch, ach ) || !is_name( arg, ach->name ) )
        continue;
      if ( ++count == number )
        return ach;
    }

  return NULL;
}


/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *wch;
    char       arg [ MAX_INPUT_LENGTH ];
    int        number;
    int        count;

    if ( ( wch = get_char_room( ch, argument ) ) )
	return wch;

    number = number_argument( argument, arg );
    count  = 0;
    for ( wch = char_list; wch ; wch = wch->next )
    {
	if ( !can_see( ch, wch ) || !is_name( arg, wch->name ) )
	    continue;
	if ( ++count == number )
	    return wch;
    }

    return NULL;
}



/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = object_list; obj; obj = obj->next )
    {
        if ( obj->deleted )
	    continue;

	if ( obj->pIndexData == pObjIndex )
	    return obj;
    }

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];
    int       number;
    int       count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];
    int       number;
    int       count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->wear_loc == WEAR_NONE
	    && can_see_obj( ch, obj )
	    && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}

OBJ_DATA *get_obj_storage( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];
  int number;
  int count;

  if ( IS_NPC( ch ) )
  {
    bug( "get_obj_storage: NPC storage from %d", ch->pIndexData->vnum );
    return NULL;
  }

  number = number_argument( argument, arg );
  count = 0;

  for ( obj = ch->pcdata->storage; obj; obj = obj->next_content )
  {
    if ( can_see_obj( ch, obj )
	 && is_name( arg, obj->name ) )
    {
      if ( ++count == number )
	return obj;
    }
  }
  return NULL;
}

/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];
    int       number;
    int       count;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->wear_loc != WEAR_NONE
	    && can_see_obj( ch, obj )
	    && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    obj = get_obj_list( ch, argument, ch->in_room->contents );
    if ( obj )
	return obj;

    if ( ( obj = get_obj_carry( ch, argument ) ) )
	return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) )
	return obj;

    return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];
    int       number;
    int       count;

    if ( ( obj = get_obj_here( ch, argument ) ) )
	return obj;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = object_list; obj; obj = obj->next )
    {
	if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) )
	{
	    if ( ++count == number )
		return obj;
	}
    }

    return NULL;
}



/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int amount )
{
    OBJ_DATA *obj;

    if ( amount <= 0 )
    {
	bug( "Create_money: zero or negative money %d.", amount );
	amount = 1;
    }

    if ( amount == 1 )
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_ONE  ), 0 );
    }
    else
    {
	obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
    }

    obj->value[0]		= amount;
    return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
/* This was not being used. Replaced by obj_invcount
   Although this function is left here to avoid breaking
   old code. - Ahsile

    int number;

    if (obj->carried_by == NULL)
       number = obj_invcount( obj, TRUE );
    else
       number = 1;
    
    return number; 
*/
//    bug("Obj count: %d", obj_invcount(obj, TRUE)); // Debug
    return obj_invcount(obj, TRUE);
}



/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;
    weight = obj->weight;
    for ( obj = obj->contains; obj; obj = obj->next_content )
    {
	if ( obj->deleted )
	    continue;
	weight += get_obj_weight( obj );
    }

    return weight;
}



/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;

    if (!pRoomIndex)
        return TRUE;

    if ( pRoomIndex->light > 0 )
	return FALSE;

    for ( obj = pRoomIndex->contents; obj; obj = obj->next_content )
    {
	if ( obj->deleted )
	    continue;
	if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	    return FALSE;
    }

    if ( IS_SET( pRoomIndex->room_flags, ROOM_DARK ) )
	return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
	|| pRoomIndex->sector_type == SECT_CITY )
	return FALSE;

    if ( weather_info.sunlight == SUN_SET
	|| weather_info.sunlight == SUN_DARK )
	return TRUE;

    return FALSE;
}



/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int        count;

    count = 0;
    for ( rch = pRoomIndex->people; rch; rch = rch->next_in_room )
    {
	if ( rch->deleted )
	    continue;

	count++;
    }

    if ( IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE  ) && count >= 2 )
	return TRUE;

    if ( IS_SET( pRoomIndex->room_flags, ROOM_SOLITARY ) && count >= 1 )
	return TRUE;

    return FALSE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( victim->deleted )
        return FALSE;

    if ( ch == victim )
	return TRUE;
    
    if ( !IS_NPC( victim )
	&& IS_SET( victim->act, PLR_WIZINVIS )
	&& get_trust( ch ) < victim->wizinvis )
	return FALSE;
   
    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
	return TRUE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) || (IS_AFFECTED3(ch,AFF_BEND_LIGHT)))
	return FALSE;

     if ( room_is_dark( ch->in_room ) 
      && !IS_AFFECTED( ch, AFF_INFRARED )
      && !IS_SET( race_table[ ch->race ].race_abilities, RACE_INFRAVISION )
      && !IS_AFFECTED2( ch, AFF_TRUESIGHT )
      && ( ch->race != 1 )
      && ( ch->race != 2 )
      && ( ch->race != 5 )
      && ( ch->race != 9 ) )
	return FALSE;

    if ( victim->position == POS_DEAD )
        return TRUE;

 if ( IS_AFFECTED2( victim, AFF_PHASED )
      && (!IS_AFFECTED2(ch, AFF_TRUESIGHT )
      || (IS_NPC(ch)
          && ch->level < 50) ))
      return FALSE; 

 if ( IS_AFFECTED2( victim, AFF_SHADOW_PLANE )
      && (!IS_AFFECTED2(ch, AFF_TRUESIGHT )
      || (IS_NPC(ch)
          && ch->level < 50) ))
      return FALSE; 

 if ( IS_AFFECTED4( victim, AFF_BURROW )
      && (!IS_AFFECTED2(ch, AFF_TRUESIGHT )
      || (IS_NPC(ch)
          && ch->level < 50) ))
      return FALSE; 

         
    if ( IS_AFFECTED( victim, AFF_INVISIBLE )
	&& !IS_AFFECTED( ch, AFF_DETECT_INVIS )
        && !IS_SET( race_table[ ch->race ].race_abilities, RACE_DETECT_INVIS )
	&& !IS_AFFECTED2( ch, AFF_TRUESIGHT )
	&& ( ch->race != 9 )
	&& ( ch->class != 2
	     && ch->level < 30 ) )
	return FALSE;

    if ( IS_AFFECTED2( victim, AFF_IMPROVED_INVIS )
	&& !IS_AFFECTED( ch, AFF_DETECT_INVIS )
	&& !IS_AFFECTED2( ch, AFF_TRUESIGHT ))
	return FALSE;

    if ( IS_AFFECTED( victim, AFF_HIDE )
	&& !IS_AFFECTED( ch, AFF_DETECT_HIDDEN )
        && !IS_SET( race_table[ ch->race ].race_abilities, RACE_DETECT_HIDDEN )
	&& !IS_AFFECTED2( ch, AFF_TRUESIGHT )
	&& !victim->fighting
	&& ( IS_NPC( ch ) ? !IS_NPC( victim ) : IS_NPC( victim ) )
	&& ( ch->race != 2 ) )
	return FALSE;

    return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( obj->deleted )
        return FALSE;

    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
	return TRUE;
    if ( !IS_NPC( ch ) && IS_AFFECTED2( ch, AFF_TRUESIGHT ) )
        return TRUE;
    if ( IS_AFFECTED( ch, AFF_BLIND ) || IS_AFFECTED3(ch,AFF_BEND_LIGHT))
	return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
	return TRUE;

    if ( room_is_dark( ch->in_room ) 
       && !IS_AFFECTED( ch, AFF_INFRARED )
        && !IS_SET( race_table[ ch->race ].race_abilities, RACE_INFRAVISION )
       && !IS_AFFECTED2( ch, AFF_TRUESIGHT )
       && ( ch->race != 1 )
       && ( ch->race != 2 )
       && ( ch->race != 5 )
       && ( ch->race != 9 ) )
	return FALSE;

    if ( IS_SET( obj->extra_flags, ITEM_INVIS )
	&& !IS_AFFECTED( ch, AFF_DETECT_INVIS )
        && !IS_SET( race_table[ ch->race ].race_abilities, RACE_DETECT_INVIS )
	&& !IS_AFFECTED2( ch, AFF_TRUESIGHT )
	&& ( ch->race != 9 ) )
	return FALSE;

    if ( IS_SET( obj->extra_flags2, ITEM_HIDDEN )
        && !IS_SET( race_table[ ch->race ].race_abilities, RACE_DETECT_HIDDEN )
        && !IS_AFFECTED( ch, AFF_DETECT_HIDDEN  ))
        return FALSE;


    return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_SET( obj->extra_flags, ITEM_NODROP ) /*&& !IS_SET( obj->extra_flags, ITEM_QUEST) */)
		return TRUE;

    if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
	return TRUE;

    return FALSE;
}



/*
 * Return ascii name of an item type.
 */
char *item_type_name( OBJ_DATA *obj )
{
    OBJ_DATA *in_obj;
    char      buf [ MAX_STRING_LENGTH ];

    switch ( obj->item_type )
    {
    case ITEM_LIGHT:		return "light";
    case ITEM_SCROLL:		return "scroll";
    case ITEM_WAND:		return "wand";
    case ITEM_STAFF:		return "staff";
    case ITEM_WEAPON:		return "weapon";
    case ITEM_TREASURE:		return "treasure";
    case ITEM_ARMOR:		return "armor";
    case ITEM_POTION:		return "potion";
    case ITEM_FURNITURE:	return "furniture";
    case ITEM_TRASH:		return "trash";
    case ITEM_CONTAINER:	return "container";
    case ITEM_DRINK_CON:	return "drink container";
    case ITEM_BLOOD:            return "blood";
    case ITEM_KEY:		return "key";
    case ITEM_FOOD:		return "food";
    case ITEM_MONEY:		return "money";
    case ITEM_BOAT:		return "boat";
    case ITEM_CORPSE_NPC:	return "npc corpse";
    case ITEM_CORPSE_PC:	return "pc corpse";
    case ITEM_FOUNTAIN:		return "fountain";
    case ITEM_PILL:		return "pill";
    case ITEM_LENSE:            return "contacts";
    case ITEM_PORTAL:           return "portal";
    case ITEM_VODOO:            return "voodo doll";
    case ITEM_BERRY:            return "goodberry";
    case ITEM_GUN:		return "firearm";
    case ITEM_IMPLANTED:	return "implant";
    case ITEM_RUNE:		return "rune";
    case ITEM_SKIN:		return "skin";
    case ITEM_NEEDLE:		return "needle";
    case ITEM_HAMMER:		return "forging hammer";
    case ITEM_QUILL:		return "quill";
    case ITEM_PESTLE:		return "pestle and mortar";
    case ITEM_BOLT:		return "bolt";
    case ITEM_ARROW:		return "arrow";
    case ITEM_BULLET:		return "bullet";
    case ITEM_BOOK:		return "book";
    }

    for ( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj )
      ;

    if ( in_obj->carried_by )
      sprintf( buf, "Item_type_name: unknown type %d from %s owned by %s.",
	      obj->item_type, obj->name, obj->carried_by->name );
    else
      sprintf( buf,
	      "Item_type_name: unknown type %d from %s owned by (unknown).",
	      obj->item_type, obj->name );

    bug( buf, 0 );
    return "(unknown)";
}



/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( int location )
{
    switch ( location )
    {
    case APPLY_NONE:		return "none";
    case APPLY_STR:		return "strength";
    case APPLY_DEX:		return "dexterity";
    case APPLY_INT:		return "intelligence";
    case APPLY_WIS:		return "wisdom";
    case APPLY_CON:		return "constitution";
    case APPLY_SEX:		return "sex";
    case APPLY_CLASS:		return "class";
    case APPLY_LEVEL:		return "level";
    case APPLY_AGE:		return "age";
    case APPLY_HEIGHT:          return "height";
    case APPLY_WEIGHT:          return "weight";
    case APPLY_MANA:		return "mana";
    case APPLY_BP:              return "blood";
    case APPLY_ANTI_DIS:        return "anti-disarm";
    case APPLY_HIT:		return "hp";
    case APPLY_MOVE:		return "moves";
    case APPLY_GOLD:		return "gold";
    case APPLY_EXP:		return "experience";
    case APPLY_AC:		return "armor class";
    case APPLY_HITROLL:		return "hit roll";
    case APPLY_DAMROLL:		return "damage roll";
    case APPLY_SAVING_PARA:	return "save vs paralysis";
    case APPLY_SAVING_ROD:	return "save vs rod";
    case APPLY_SAVING_PETRI:	return "save vs petrification";
    case APPLY_SAVING_BREATH:	return "save vs breath";
    case APPLY_SAVING_SPELL:	return "save vs spell";
    case APPLY_DAM_ACID	:	return "resist acid";
    case APPLY_DAM_HOLY	:	return "resist holy";
    case APPLY_DAM_MAGIC:		return "resist magic";
    case APPLY_DAM_FIRE:	return "resist fire";
    case APPLY_DAM_ENERGY:		return "resist energy";
    case APPLY_DAM_WIND :	return "resist wind";
    case APPLY_DAM_WATER:		return "resist water";
    case APPLY_DAM_ILLUSION	:	return "resist illusion";
    case APPLY_DAM_DISPEL	:	return "resist dispel";
    case APPLY_DAM_EARTH	:	return "resist earth";
    case APPLY_DAM_PSYCHIC	:	return "resist psychic";
    case APPLY_DAM_POISON	:	return "resist poison";
    case APPLY_DAM_BREATH	:	return "resist breath";
    case APPLY_DAM_SUMMON	:	return "resist summon";
    case APPLY_DAM_PHYSICAL	:	return "resist physical";
    case APPLY_DAM_EXPLOSIVE	:	return "resist explosive";
    case APPLY_DAM_SONG	:	return "resist song";
    case APPLY_DAM_NAGAROM:		return "resist nagarom";
    case APPLY_DAM_UNHOLY:	return "resist unholy";
    case APPLY_DAM_CLAN	:	return "resist clan";

/* X */

/*
 * The following is in alphabetic order.
 * Please leave it that way. It makes it much
 * easier to search for missing applies.
 *
 *  - Ahsile
 */
	  case APPLY_AGE_SPELL:		return "age";
      case APPLY_ANGELIC_AURA: return "angelic aura";	
	  case APPLY_ANTI_FLEE: return "web";
      case APPLY_AURA_ANTI_MAGIC: return "aura of anti-magic";		
	  case APPLY_BEND_LIGHT:	return "bend light";
      case APPLY_BIOFEEDBACK: return "biofeedback";
	  case APPLY_BLADE: return "blade barrier";
      case APPLY_BLESS:		return "bless";
	  case APPLY_BLIND:		return "blind";
      case APPLY_BLOODSHIELD: 	return "bloodshield";	
	  case APPLY_CHANGE_SEX:	return "change sex";
      case APPLY_CHAOS:         return "chaos field";		
	  case APPLY_CLOAKING:		return "cloaking";
	  case APPLY_CLOUD_OF_HEALING: return "cloud of healing";
      case APPLY_COFIRE:        return "circle of fire";		
      case APPLY_COMBAT_MIND:   return "combat mind";
	  case APPLY_CONFUSED:		return "confusion";
	  case APPLY_CURSE:			return "curse";
	  case APPLY_CURSE_NATURE:	return "curse of nature";
	  case APPLY_DANCING:		return "dancing lights";
      case APPLY_DETECT_EVIL:	return "detect evil";	
	  case APPLY_DETECT_GOOD:	return "detect good";
      case APPLY_DETECT_HIDDEN:	return "detect hidden";	
      case APPLY_DETECT_INVIS:	return "detect invis";	
      case APPLY_DETECT_MAGIC:	return "detect magic";	
      case APPLY_EARTHSHIELD: return "earthshield";		
      case APPLY_ESSENCE_OF_GAIA: return "essence of gaia";		
      case APPLY_ETHEREAL_SNAKE: return "ethereal snake";		
      case APPLY_ETHEREAL_WOLF: return "ethereal wolf";		
      case APPLY_FAERIE_FIRE:	return "faerie fire";	
      case APPLY_FIRESHIELD:    return "fireshield";
	  case APPLY_FLAMING:		return "incinerate";
      case APPLY_FLYING:	return "fly";	
      case APPLY_FORCE_OF_NATURE: return "force of nature";		
      case APPLY_FORESTWALK:	return "forestwalk";	
	  case APPLY_FUMBLE:		return "fumble";
      case APPLY_GHOST_SHIELD:  return "ghost shield";		
      case APPLY_GIANT_STRENGTH:return "giant strength";		
      case APPLY_GOLDEN_ARMOR:	return "golden armor";	
      case APPLY_GOLDEN_SANCTUARY: return "golden sanctuary";		
      case APPLY_HASTE:		return "haste";
      case APPLY_HEIGHTEN_SENSES:      return "heighten-senses";		
      case APPLY_HIDE:		return "hide";
      case APPLY_HOLY_PROTECTION: return "holy protection";		
      case APPLY_ICESHIELD:     return "iceshield";  		
      case APPLY_IMPROVED_HIDE: return "improved hide";		
      case APPLY_IMPROVED_INVIS:	return "improved invis";	
	  case APPLY_INERTIAL:		return "intertial barrier";
      case APPLY_INFRARED:	return "infravision";	
      case APPLY_INVISIBLE:	return "invisible";	
      case APPLY_LEAF_SHIELD: return "leaf shield";		
      case APPLY_LIQUID_SKIN: return "liquid skin";		
      case APPLY_LUCK_SHIELD: return "luck shield";		
	  case APPLY_MALIGNIFY:		return "malignify";
      case APPLY_MIST: 		return "mist";
      case APPLY_MOUNTAINWALK:	return "mountainwalk";	
      case APPLY_NAGAROMS_CURSE: return"nagaroms curse";		
      case APPLY_OCCULUTUS_VISUM: return "occulutus visum";		
      case APPLY_PASS_DOOR:	return "pass door";	
	  case APPLY_PEACE: return "aura of peace";
	  case APPLY_PESTILENCE:	return "pestilence";
      case APPLY_PLAINSWALK:	return "plainswalk";	
	  case APPLY_POWER_LEAK:	return "power leak";
      case APPLY_PRAYER:	return "prayer";	
      case APPLY_PROTECT:	return "protection evil";	
	  case APPLY_PROTECTION_GOOD: return "protection good";
      case APPLY_QUICKNESS:      return "quickness";		
      case APPLY_RANDOMSHIELD: return "randomshield";		
      case APPLY_SANCTUARY:	return "sanctuary";	
      case APPLY_SATANIC_INFERNO: return "satanic inferno";		
      case APPLY_SCRY:          return "scry";		
      case APPLY_SHADOW_IMAGE:  return "shadow image";		
      case APPLY_SHOCKSHIELD:   return "shockshield";		
	  case APPLY_SLIT:		return "slit";
      case APPLY_SNEAK:		return "sneak";
      case APPLY_STEALTH:	return "stealth";	
      case APPLY_SWAMPWALK:	return "swampwalk";	
	  case APPLY_TALE_OF_TERROR: return "tale of terror";
      case APPLY_TITAN_STRENGTH: return "titan strength";		
      case APPLY_TONGUES: return "tongues";
	  case APPLY_TORTURE: return "tortured soul";
	  case APPLY_TRUESIGHT: return "true sight";
	case APPLY_UNHOLY_STRENGTH: return "unholy strength";

/* END */
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}

bool valid_aff_loc(int location)
{
	switch (location)
	{
	  case APPLY_AGE_SPELL:		return TRUE;
      case APPLY_ANGELIC_AURA: return TRUE;	
	  case APPLY_ANTI_FLEE: return TRUE;
      case APPLY_AURA_ANTI_MAGIC: return TRUE;		
	  case APPLY_BEND_LIGHT:	return TRUE;
      case APPLY_BIOFEEDBACK: return TRUE;
	  case APPLY_BLADE: return TRUE;
      case APPLY_BLESS:		return TRUE;
	  case APPLY_BLIND:		return TRUE;
      case APPLY_BLOODSHIELD: 	return TRUE;	
	  case APPLY_CHANGE_SEX:	return TRUE;
      case APPLY_CHAOS:         return TRUE;		
	  case APPLY_CLOAKING:		return TRUE;
	  case APPLY_CLOUD_OF_HEALING: return TRUE;
      case APPLY_COFIRE:        return TRUE;		
      case APPLY_COMBAT_MIND:   return TRUE;
	  case APPLY_CONFUSED:		return TRUE;
	  case APPLY_CURSE:			return TRUE;
	  case APPLY_CURSE_NATURE:	return TRUE;
	  case APPLY_DANCING:		return TRUE;
      case APPLY_DETECT_EVIL:	return TRUE;	
	  case APPLY_DETECT_GOOD:	return TRUE;
      case APPLY_DETECT_HIDDEN:	return TRUE;	
      case APPLY_DETECT_INVIS:	return TRUE;	
      case APPLY_DETECT_MAGIC:	return TRUE;	
      case APPLY_EARTHSHIELD: return TRUE;		
      case APPLY_ESSENCE_OF_GAIA: return TRUE;		
      case APPLY_ETHEREAL_SNAKE: return TRUE;		
      case APPLY_ETHEREAL_WOLF: return TRUE;		
      case APPLY_FAERIE_FIRE:	return TRUE;	
      case APPLY_FIRESHIELD:    return TRUE;
	  case APPLY_FLAMING:		return TRUE;
      case APPLY_FLYING:	return TRUE;	
      case APPLY_FORCE_OF_NATURE: return TRUE;		
      case APPLY_FORESTWALK:	return TRUE;	
	  case APPLY_FUMBLE:		return TRUE;
      case APPLY_GHOST_SHIELD:  return TRUE;		
      case APPLY_GIANT_STRENGTH:return TRUE;		
      case APPLY_GOLDEN_ARMOR:	return TRUE;	
      case APPLY_GOLDEN_SANCTUARY: return TRUE;		
      case APPLY_HASTE:		return TRUE;
      case APPLY_HEIGHTEN_SENSES:      return TRUE;		
      case APPLY_HIDE:		return TRUE;
      case APPLY_HOLY_PROTECTION: return TRUE;		
      case APPLY_ICESHIELD:     return TRUE;  		
      case APPLY_IMPROVED_HIDE: return TRUE;		
      case APPLY_IMPROVED_INVIS:	return TRUE;	
	  case APPLY_INERTIAL:		return TRUE;
      case APPLY_INFRARED:	return TRUE;	
      case APPLY_INVISIBLE:	return TRUE;	
      case APPLY_LEAF_SHIELD: return TRUE;		
      case APPLY_LIQUID_SKIN: return TRUE;		
      case APPLY_LUCK_SHIELD: return TRUE;		
	  case APPLY_MALIGNIFY:		return TRUE;
      case APPLY_MIST: 		return TRUE;
      case APPLY_MOUNTAINWALK:	return TRUE;	
      case APPLY_NAGAROMS_CURSE: return TRUE;		
      case APPLY_OCCULUTUS_VISUM: return TRUE;		
      case APPLY_PASS_DOOR:	return TRUE;	
	  case APPLY_PEACE: return TRUE;
	  case APPLY_PESTILENCE:	return TRUE;
      case APPLY_PLAINSWALK:	return TRUE;	
	  case APPLY_POWER_LEAK:	return TRUE;
      case APPLY_PRAYER:	return TRUE;	
      case APPLY_PROTECT:	return TRUE;	
	  case APPLY_PROTECTION_GOOD: return TRUE;
      case APPLY_QUICKNESS:      return TRUE;		
      case APPLY_RANDOMSHIELD: return TRUE;		
      case APPLY_SANCTUARY:	return TRUE;	
      case APPLY_SATANIC_INFERNO: return TRUE;		
      case APPLY_SCRY:          return TRUE;		
      case APPLY_SHADOW_IMAGE:  return TRUE;		
      case APPLY_SHOCKSHIELD:   return TRUE;		
	  case APPLY_SLIT:		return TRUE;
      case APPLY_SNEAK:		return TRUE;
      case APPLY_SWAMPWALK:	return TRUE;	
	  case APPLY_TALE_OF_TERROR: return TRUE;
      case APPLY_TITAN_STRENGTH: return TRUE;		
      case APPLY_TONGUES: return TRUE;
	  case APPLY_TORTURE: return TRUE;
	  case APPLY_TRUESIGHT: return TRUE;
      case APPLY_UNHOLY_STRENGTH: return TRUE;

	}

	return FALSE;
}

/*
 * Return ascii name of an affect bit vector.
 */
char *affect_bit_name( int vector )
{
    static char buf [ 512 ];

    buf[0] = '\0';
    if ( vector & AFF_BLIND         ) strcat( buf, " blind"         );
    if ( vector & AFF_INVISIBLE     ) strcat( buf, " invisible"     );
    if ( vector & AFF_DETECT_EVIL   ) strcat( buf, " detect_evil"   );
    if ( vector & AFF_DETECT_INVIS  ) strcat( buf, " detect_invis"  );
    if ( vector & AFF_DETECT_MAGIC  ) strcat( buf, " detect_magic"  );
    if ( vector & AFF_DETECT_HIDDEN ) strcat( buf, " detect_hidden" );
    if ( vector & AFF_HASTE         ) strcat( buf, " haste"         );
    if ( vector & AFF_SANCTUARY     ) strcat( buf, " sanctuary"     );
    if ( vector & AFF_FIRESHIELD    ) strcat( buf, " fireshield"    );
    if ( vector & AFF_SHOCKSHIELD   ) strcat( buf, " shockshield"   );
    if ( vector & AFF_ICESHIELD     ) strcat( buf, " iceshield"     );
    if ( vector & AFF_CHAOS         ) strcat( buf, " chaos field"   );
    if ( vector & AFF_FAERIE_FIRE   ) strcat( buf, " faerie_fire"   );
    if ( vector & AFF_INFRARED      ) strcat( buf, " infrared"      );
    if ( vector & AFF_CURSE         ) strcat( buf, " curse"         );
    if ( vector & AFF_FLAMING       ) strcat( buf, " flaming"       );
    if ( vector & AFF_POISON        ) strcat( buf, " poison"        );
    if ( vector & AFF_PROTECT       ) strcat( buf, " protect"       );
    if ( vector & AFF_INERTIAL      ) strcat( buf, " vibrating"     );
    if ( vector & AFF_SLEEP         ) strcat( buf, " sleep"         );
    if ( vector & AFF_SNEAK         ) strcat( buf, " sneak"         );
    if ( vector & AFF_HIDE          ) strcat( buf, " hide"          );
    if ( vector & AFF_CHARM         ) strcat( buf, " charm"         );
    if ( vector & AFF_FLYING        ) strcat( buf, " flying"        );
    if ( vector & AFF_PASS_DOOR     ) strcat( buf, " pass_door"     );
    if ( vector & AFF_PEACE         ) strcat( buf, " peace"         );
    if ( vector & AFF_STUN          ) strcat( buf, " stunned"         );
    if ( vector & AFF_ANTI_FLEE     ) strcat( buf, " webbed"        );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *affect_bit_name2( int vector )
{
    static char buf [ 512 ];

    buf[0] = '\0';
    if ( vector & AFF_POLYMORPH     ) strcat( buf, " polymorph"     );
    if ( vector & AFF_NOASTRAL      ) strcat( buf, " noastral"      );
    if ( vector & AFF_DOOMSHIELD    ) strcat( buf, " doomshield"    );
    if ( vector & AFF_TRUESIGHT     ) strcat( buf, " truesight"     );
    if ( vector & AFF_BLADE         ) strcat( buf, " bladebarrier"  );
    if ( vector & AFF_DETECT_GOOD   ) strcat( buf, " detect_good"   );
    if (vector & AFF_PROTECTION_GOOD) strcat( buf, " protection_good");
    if ( vector & AFF_BERSERK       ) strcat( buf, " berserk"       );
    if ( vector & AFF_UNHOLY_STRENGTH) strcat( buf, " unholy_strength");
    if ( vector & AFF_CONFUSED      ) strcat( buf, " confused"      );
    if ( vector & AFF_FUMBLE        ) strcat( buf, " fumble"        );
    if ( vector & AFF_DANCING       ) strcat( buf, " dancing-lights");
    if ( vector & AFF_IMAGE         ) strcat( buf, " image"         );
    if ( vector & AFF_PHASED        ) strcat( buf, " phase_shift"   );
    if ( vector & AFF_GOLDEN_ARMOR  ) strcat( buf, " golden_armor"  );
    if ( vector & AFF_GHOST_SHIELD  ) strcat( buf, " ghost_shield"  );
    if ( vector & AFF_CURSE_NATURE  ) strcat( buf, " curse_nature"  );
    if ( vector & AFF_MIST          ) strcat( buf, " mist"          );
    if ( vector & AFF_SHADOW_IMAGE  ) strcat( buf, " shadow_image"  );
    if ( vector & AFF_WEAPONMASTER  ) strcat( buf, " weaponmaster"  );
    if ( vector & AFF_IMPROVED_INVIS) strcat( buf, " improved_invis");
    if ( vector & AFF_BLINDFOLD     ) strcat( buf, " blindfold"     );
    if ( vector & AFF_SLIT          ) strcat( buf, " slit"          );
    if ( vector & AFF_THICK_SKIN    ) strcat( buf, " thick_skin"    );
    if ( vector & AFF_MALIGNIFY     ) strcat( buf, " malignify"     );
    if ( vector & AFF_CLOAKING      ) strcat( buf, " cloaking"      );
    if ( vector & AFF_SHADOW_PLANE  ) strcat( buf, " shadow_plane"  );
    if ( vector & AFF_WOLFED        ) strcat( buf, " metamorph"     );
    if ( vector & AFF_HOLD          ) strcat( buf, " hold"          );
    if ( vector & AFF_CHANGE_SEX    ) strcat( buf, " change_sex"    );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *affect_bit_name3( int vector )
{
    static char buf [ 512 ];

    buf[0] = '\0';
    if ( vector & AFF_WATERWALK     ) strcat( buf, " waterwalk"     );
    if ( vector & AFF_GILLS         ) strcat( buf, " gills"         );
    if ( vector & AFF_VAMP_BITE     ) strcat( buf, " vampire_bite"  );
    if ( vector & AFF_GHOUL         ) strcat( buf, " ghoul"         );
    if ( vector & AFF_CHALLENGER    ) strcat( buf, " challenger"    );
    if ( vector & AFF_CHALLENGED    ) strcat( buf, " challenged"    );
    if ( vector & AFF_RAGE	    ) strcat( buf, " rage" 	    );
    if ( vector & AFF_BLOODTHIRSTY  ) strcat( buf, " bloodthirsty"  );
    if ( vector & AFF_COFIRE	    ) strcat( buf, " circle_of_fire");
    if ( vector & AFF_TORTURE	    ) strcat( buf, " torture" 	    );
    if ( vector & AFF_AGE	    ) strcat( buf, " age" 	    );
    if ( vector & AFF_SATANIC_INFERNO)strcat( buf, " satanic_inferno");
    if ( vector & AFF_PESTILENCE    ) strcat( buf, " pestilence"    );
    if ( vector & AFF_AURA_ANTI_MAGIC ) strcat( buf, " aura_of_anti_magic" );
    if ( vector & AFF_HOLY_PROTECTION ) strcat(buf," holy_protection" );
    if ( vector & AFF_IMPROVED_HIDE ) strcat( buf, " improved_hide" );
    if ( vector & AFF_STEALTH	    ) strcat( buf, " stealth"	    );
    if ( vector & AFF_BLOODSHIELD   ) strcat( buf, " bloodshield"   );
    if ( vector & AFF_BEND_LIGHT    ) strcat( buf, " bend_light"    );
    if ( vector & AFF_CLOUD_OF_HEALING )strcat(buf," cloud_of_healing");
    if ( vector & AFF_TALE_OF_TERROR) strcat( buf, " tale_of_terror");
    if ( vector & AFF_POWER_LEAK    ) strcat( buf, " power_leak"    );
    if ( vector & AFF_MANA_SHIELD   ) strcat( buf, " mana_shield"   );
    if ( vector & AFF_WAR_CHANT     ) strcat( buf, " war_chant"     );
    if ( vector & AFF_NAGAROMS_CURSE) strcat( buf, " nagaroms_curse");
    if ( vector & AFF_PRAYER	    ) strcat( buf, " prayer"	    );
    if ( vector & AFF_RANDOMSHIELD  ) strcat( buf, " randomshield"  );
    if ( vector & AFF_PRIMALSCREAM  ) strcat( buf, " primalscream"  );
    if ( vector & AFF_ACIDSHIELD    ) strcat( buf, " acidshield"    );
    if ( vector & AFF_DEMONSHIELD   ) strcat( buf, " demonshield"   );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *affect_bit_name4( int vector )
{
    static char buf [ 512 ];

    buf[0] = '\0';
    if ( vector & AFF_IMMORTAL      ) strcat( buf, " immortal"      );
    if ( vector & AFF_NO_SUMMON	    ) strcat( buf, " no_summon"     );
    if ( vector & AFF_GOLDEN_SANCTUARY ) strcat( buf, " golden_sanctuary" );
    if ( vector & AFF_MOUNTABLE	    ) strcat( buf, " mountable"	    );
    if ( vector & AFF_BIOFEEDBACK   ) strcat( buf, " biofeedback"   );
    if ( vector & AFF_EARTHSHIELD   ) strcat( buf, " earthshield"   );
    if ( vector & AFF_LEAF_SHIELD   ) strcat( buf, " leaf_shield"   );
    if ( vector & AFF_LUCK_SHIELD   ) strcat( buf, " luck_shield"   );
    if ( vector & AFF_TONGUES	    ) strcat( buf, " tongues"	    );
    if ( vector & AFF_LIQUID_SKIN   ) strcat( buf, " liquid_skin"   );
    if ( vector & AFF_ANGELIC_AURA  ) strcat( buf, " angelic_aura"  );
    if ( vector & AFF_ETHEREAL_WOLF ) strcat( buf, " ethereal_wolf" );
    if ( vector & AFF_DECEPTION     ) strcat( buf, " deception"     );
    if ( vector & AFF_ETHEREAL_SNAKE) strcat( buf, " ethereal_snake");
    if ( vector & AFF_BURROW	    ) strcat( buf, " tomba_di_vemon");
    if ( vector & AFF_THIEVESCANT   ) strcat( buf, " thieves_cant"  );
    if ( vector & AFF_NEWBIE_SLAYER ) strcat( buf, " newbie_slayer" );
    if ( vector & AFF_FORCE_OF_NATURE )strcat(buf, " force_of_nature" );
    if ( vector & AFF_ESSENCE_OF_GAIA )strcat(buf, " essence_of_gaia" );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *affect_bit_name_powers( int vector )
{
    static char buf [ 512 ];

    buf[0] = '\0';
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *affect_bit_name_weaknesses( int vector )
{   
    static char buf [ 512 ];
  
    buf[0] = '\0';
    return ( buf[0] != '\0' ) ? buf+1 : "none";
} 
  
/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name( int extra_flags )
{
    static char buf [ 512 ];

    buf[0] = '\0';
    if ( extra_flags & ITEM_PROTOTYPE    ) strcat( buf, " prototype"    );
    if ( extra_flags & ITEM_GLOW         ) strcat( buf, " glow"         );
    if ( extra_flags & ITEM_HUM          ) strcat( buf, " hum"          );
    if ( extra_flags & ITEM_DARK         ) strcat( buf, " dark"         );
    if ( extra_flags & ITEM_LOCK         ) strcat( buf, " lock"         );
    if ( extra_flags & ITEM_EVIL         ) strcat( buf, " evil"         );
    if ( extra_flags & ITEM_INVIS        ) strcat( buf, " invis"        );
    if ( extra_flags & ITEM_MAGIC        ) strcat( buf, " magic"        );
    if ( extra_flags & ITEM_NODROP       ) strcat( buf, " nodrop"       );
    if ( extra_flags & ITEM_BLESS        ) strcat( buf, " bless"        );
    if ( extra_flags & ITEM_ANTI_GOOD    ) strcat( buf, " anti_good"    );
    if ( extra_flags & ITEM_ANTI_EVIL    ) strcat( buf, " anti_evil"    );
    if ( extra_flags & ITEM_ANTI_NEUTRAL ) strcat( buf, " anti_neutral" );
    if ( extra_flags & ITEM_NOREMOVE     ) strcat( buf, " noremove"     );
    if ( extra_flags & ITEM_INVENTORY    ) strcat( buf, " inventory"    );
    if ( extra_flags & ITEM_POISONED     ) strcat( buf, " poisoned"     );
    if ( extra_flags & ITEM_ANTI_MAGE    ) strcat( buf, " anti_mage"    );
    if ( extra_flags & ITEM_ANTI_CLERIC  ) strcat( buf, " anti_cleric"  );
    if ( extra_flags & ITEM_ANTI_THIEF   ) strcat( buf, " anti_thief"   );
    if ( extra_flags & ITEM_ANTI_WARRIOR ) strcat( buf, " anti_warrior" );
    if ( extra_flags & ITEM_ANTI_PSI     ) strcat( buf, " anti_psi"     );
    if ( extra_flags & ITEM_ANTI_DRUID   ) strcat( buf, " anti_druid"   );
    if ( extra_flags & ITEM_ANTI_PALADIN ) strcat( buf, " anti_paladin" );
    if ( extra_flags & ITEM_ANTI_RANGER  ) strcat( buf, " anti_ranger"  );
    if ( extra_flags & ITEM_ANTI_BARD    ) strcat( buf, " anti_bard"    );
    if ( extra_flags & ITEM_ANTI_VAMP    ) strcat( buf, " anti_vampire" );
    if ( extra_flags & ITEM_FLAME        ) strcat( buf, " burning"      );
/*    if ( extra_flags & ITEM_ACID         ) strcat( buf, " acidic"      );  */
    if ( extra_flags & ITEM_CHAOS        ) strcat( buf, " chaotic"      );
    if ( extra_flags & ITEM_NO_DAMAGE    ) strcat( buf, " indestructable");
    if ( extra_flags & ITEM_ICY          ) strcat( buf, " frosty"       );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *extra_bit_name2( int extra_flags2 )
{
    static char buf [ 512 ];

    buf[0] = '\0';
    if ( extra_flags2 & ITEM_ANTI_BARBARIAN) strcat( buf, " anti_barbarian" );
    if ( extra_flags2 & ITEM_ANTI_MONK    ) strcat( buf, " anti_monk"    );
    if ( extra_flags2 & ITEM_ANTI_ANTIPAL ) strcat( buf, " anti_antipal" );
    if ( extra_flags2 & ITEM_ANTI_WEREWOLF) strcat( buf, " anti_werewolf" );
    if ( extra_flags2 & ITEM_ANTI_ASSASSIN) strcat( buf, " anti_assassin" );
    if ( extra_flags2 & ITEM_ANTI_ILLUSIONIST) strcat( buf, " anti_illusionist" );
    if ( extra_flags2 & ITEM_ANTI_NECROMANCER) strcat( buf, " anti_necromancer" );
    if ( extra_flags2 & ITEM_ANTI_DEMONOLOGIST) strcat( buf, " anti_demonologist" );
    if ( extra_flags2 & ITEM_ANTI_SHAMAN) strcat( buf, " anti_shaman" );
    if ( extra_flags2 & ITEM_ANTI_DARKPRIEST) strcat( buf, " anti_darkpriest" );
    if ( extra_flags2 & ITEM_HIDDEN) strcat( buf, " hidden" );
    if ( extra_flags2 & ITEM_SPARKING ) strcat( buf, " sparking" );
    if ( extra_flags2 & ITEM_DISPEL ) strcat( buf, " dispel" );
    if ( extra_flags2 & ITEM_TWO_HANDED) strcat( buf, " two_handed" );
    if ( extra_flags2 & ITEM_CLAN ) strcat( buf, " clan");
    if ( extra_flags2 & ITEM_NO_STEAL) strcat( buf, " no_steal");
    if ( extra_flags2 & ITEM_NO_SAC)   strcat( buf, " no_sac" );
    if ( extra_flags2 & ITEM_REBOOT_ONLY) strcat(buf," reboot_only" );
    if ( extra_flags2 & ITEM_PERMANENT ) strcat( buf, " permanent" );
    if ( extra_flags2 & ITEM_LEGEND ) strcat( buf, " legend_only" );
	if ( extra_flags2 & ITEM_QUEST ) strcat( buf, " quest_reward" );
	if ( extra_flags2 & ITEM_MAGIC_SHOT ) strcat( buf, " magic_shot" );
	if ( extra_flags2 & ITEM_CRAFTED ) strcat( buf, " crafted" );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *extra_bit_name3( int extra_flags3 )
{
    static char buf [ 512 ];

    buf[0] = '\0'; 
    if ( extra_flags3 & ITEM_PRO_MAGE        ) strcat( buf, " pro_mage" );
    if ( extra_flags3 & ITEM_PRO_CLERIC      ) strcat( buf, " pro_cleric" );
    if ( extra_flags3 & ITEM_PRO_THIEF       ) strcat( buf, " pro_thief" );
    if ( extra_flags3 & ITEM_PRO_WARRIOR     ) strcat( buf, " pro_warrior" );
    if ( extra_flags3 & ITEM_PRO_PSI         ) strcat( buf, " pro_psi" );
    if ( extra_flags3 & ITEM_PRO_DRUID       ) strcat( buf, " pro_druid" );
    if ( extra_flags3 & ITEM_PRO_RANGER      ) strcat( buf, " pro_ranger" );
    if ( extra_flags3 & ITEM_PRO_PALADIN     ) strcat( buf, " pro_paladin" );
    if ( extra_flags3 & ITEM_PRO_BARD        ) strcat( buf, " pro_bard" );
    if ( extra_flags3 & ITEM_PRO_VAMP        ) strcat( buf, " pro_vamp" );
    if ( extra_flags3 & ITEM_PRO_WEREWOLF    ) strcat( buf, " pro_werewolf" );
    if ( extra_flags3 & ITEM_PRO_ANTIPAL     ) strcat( buf, " pro_antipal" );
    if ( extra_flags3 & ITEM_PRO_ASSASSIN    ) strcat( buf, " pro_assassin" );
    if ( extra_flags3 & ITEM_PRO_MONK        ) strcat( buf, " pro_monk" );
    if ( extra_flags3 & ITEM_PRO_BARBARIAN   ) strcat( buf, " pro_barbarian" );
    if ( extra_flags3 & ITEM_PRO_ILLUSIONIST ) strcat( buf, " pro_illusionist" );
    if ( extra_flags3 & ITEM_PRO_NECROMANCER ) strcat( buf, " pro_necromancer" );
    if ( extra_flags3 & ITEM_PRO_DEMONOLOGIST ) strcat( buf, " pro_demonologist" );
    if ( extra_flags3 & ITEM_PRO_SHAMAN      ) strcat( buf, " pro_shaman" );
    if ( extra_flags3 & ITEM_PRO_DARKPRIEST  ) strcat( buf, " pro_darkpriest" );
    if ( extra_flags3 & ITEM_PRO_HUMAN       ) strcat( buf, " pro_human" );
    if ( extra_flags3 & ITEM_PRO_ELF         ) strcat( buf, " pro_elf" );
    if ( extra_flags3 & ITEM_PRO_HALFELF     ) strcat( buf, " pro_halfelf" );
    if ( extra_flags3 & ITEM_PRO_ORC         ) strcat( buf, " pro_orc" );
    if ( extra_flags3 & ITEM_PRO_DROW        ) strcat( buf, " pro_drow" );
    if ( extra_flags3 & ITEM_PRO_DWARF       ) strcat( buf, " pro_dwarf" );
    if ( extra_flags3 & ITEM_PRO_HALFDWARF   ) strcat( buf, " pro_halfdwarf" );
    if ( extra_flags3 & ITEM_PRO_HOBBIT      ) strcat( buf, " pro_hobbit" );
    if ( extra_flags3 & ITEM_PRO_GIANT       ) strcat( buf, " pro_giant" );
    if ( extra_flags3 & ITEM_PRO_OGRE        ) strcat( buf, " pro_ogre" );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *extra_bit_name4( int extra_flags4 )
{
    static char buf [ 512 ];

    buf[0] = '\0';
    if ( extra_flags4 & ITEM_PRO_ANGEL       ) strcat( buf, " pro_angel" );
    if ( extra_flags4 & ITEM_PRO_MINOTAUR    ) strcat( buf, " pro_minotaur" );
    if ( extra_flags4 & ITEM_PRO_FELINE      ) strcat( buf, " pro_feline" );
    if ( extra_flags4 & ITEM_PRO_GARGOYLE    ) strcat( buf, " pro_gargoyle" );
    if ( extra_flags4 & ITEM_PRO_CANINE      ) strcat( buf, " pro_canine" );
    if ( extra_flags4 & ITEM_PRO_DEMON       ) strcat( buf, " pro_demon" );
    if ( extra_flags4 & ITEM_PRO_PIXIE       ) strcat( buf, " pro_pixie" );
    if ( extra_flags4 & ITEM_PRO_ELDER       ) strcat( buf, " pro_elder" );
    if ( extra_flags4 & ITEM_PRO_LIZARDMAN   ) strcat( buf, " pro_lizardman" );
    if ( extra_flags4 & ITEM_PRO_GNOME       ) strcat( buf, " pro_gnome" );
    if ( extra_flags4 & ITEM_ANTI_HUMAN      ) strcat( buf, " anti_human" );
    if ( extra_flags4 & ITEM_ANTI_ELF        ) strcat( buf, " anti_elf" );
    if ( extra_flags4 & ITEM_ANTI_HALFELF    ) strcat( buf, " anti_halfelf" );
    if ( extra_flags4 & ITEM_ANTI_ORC        ) strcat( buf, " anti_orc" );
    if ( extra_flags4 & ITEM_ANTI_DROW       ) strcat( buf, " anti_drow" );
    if ( extra_flags4 & ITEM_ANTI_DWARF      ) strcat( buf, " anti_dwarf" );
    if ( extra_flags4 & ITEM_ANTI_HALFDWARF  ) strcat( buf, " anti_halfdwarf" );
    if ( extra_flags4 & ITEM_ANTI_HOBBIT     ) strcat( buf, " anti_hobbit" );
    if ( extra_flags4 & ITEM_ANTI_GIANT      ) strcat( buf, " anti_giant" );
    if ( extra_flags4 & ITEM_ANTI_OGRE       ) strcat( buf, " anti_ogre" );
    if ( extra_flags4 & ITEM_ANTI_ANGEL      ) strcat( buf, " anti_angel" );
    if ( extra_flags4 & ITEM_ANTI_MINOTAUR   ) strcat( buf, " anti_minotaur" );
    if ( extra_flags4 & ITEM_ANTI_FELINE     ) strcat( buf, " anti_feline" );
    if ( extra_flags4 & ITEM_ANTI_GARGOYLE   ) strcat( buf, " anti_gargoyle" );
    if ( extra_flags4 & ITEM_ANTI_CANINE     ) strcat( buf, " anti_canine" );
    if ( extra_flags4 & ITEM_ANTI_DEMON      ) strcat( buf, " anti_demon" );
    if ( extra_flags4 & ITEM_ANTI_PIXIE      ) strcat( buf, " anti_pixie" );
    if ( extra_flags4 & ITEM_ANTI_ELDER      ) strcat( buf, " anti_elder" );
    if ( extra_flags4 & ITEM_ANTI_LIZARDMAN  ) strcat( buf, " anti_lizardman" );
    if ( extra_flags4 & ITEM_ANTI_GNOME      ) strcat( buf, " anti_gnome" );
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *act_bit_name( int act )
{
    static char buf [ 512 ];
    
    buf[0] = '\0';
    if ( act & 1 ) 
    {
      strcat( buf, " npc" );
      if ( act & ACT_PROTOTYPE )      strcat( buf, " prototype" );
      if ( act & ACT_SENTINEL )       strcat( buf, " sentinel" );
      if ( act & ACT_SCAVENGER )      strcat( buf, " scavenger" );
      if ( act & ACT_AGGRESSIVE )     strcat( buf, " aggressive" );
      if ( act & ACT_STAY_AREA )      strcat( buf, " stayarea" );
      if ( act & ACT_PET )            strcat( buf, " pet" );
      if ( act & ACT_TRAIN )          strcat( buf, " trainer" );
      if ( act & ACT_PRACTICE )       strcat( buf, " practicer" );
      if ( act & ACT_TRACK )          strcat( buf, " track" );
      if ( act & ACT_TEACHER   ) strcat( buf, " teacher"        );
      if ( act & ACT_BANKER    ) strcat( buf, " banker"        );
      if ( act & ACT_GAMBLE )         strcat( buf, " gambler" );
      if ( act & ACT_IS_HEALER ) strcat( buf, " healer"         );
      if ( act & ACT_CLASSMASTER ) strcat(buf," classmaster"	);
      if ( act & ACT_NO_EXP )	      strcat( buf, " no_exp"    );
      if ( act & ACT_IS_CLAN_HEALER ) strcat (buf," clanhealer" );
      if ( act & ACT_IS_DEITY ) strcat (buf," deity" );
      if ( act & ACT_RELBOSS ) strcat (buf," boss" );
    }
    else
    {
      strcat( buf, " pc" );
      if ( act & PLR_BOUGHT_PET ) strcat( buf, " boughtpet" );
      if ( act & PLR_AUTOEXIT ) strcat( buf, " autoexit" );
      if ( act & PLR_AUTOLOOT ) strcat( buf, " autoloot" );
      if ( act & PLR_AUTOSAC ) strcat( buf, " autosac" );
      if ( act & PLR_BLANK ) strcat( buf, " blankline" );
      if ( act & PLR_BRIEF ) strcat( buf, " brief" );
      if ( act & PLR_COMBINE ) strcat( buf, " combine" );
      if ( act & PLR_PROMPT ) strcat( buf, " prompt" );
      if ( act & PLR_TELNET_GA ) strcat( buf, " telnetga" );
      if ( act & PLR_HOLYLIGHT ) strcat( buf, " holylight" );
      if ( act & PLR_WIZINVIS ) strcat( buf, " wizinvis" );
      if ( act & PLR_SILENCE ) strcat( buf, " silence" );
      if ( act & PLR_NO_EMOTE ) strcat( buf, " noemote" );
      if ( act & PLR_NO_TELL ) strcat( buf, " notell" );
      if ( act & PLR_LOG ) strcat( buf, " log" );
      if ( act & PLR_FREEZE ) strcat( buf, " freeze" );
      if ( act & PLR_THIEF ) strcat( buf, " thief" );
      if ( act & PLR_KILLER ) strcat( buf, " killer" );
      if ( act & PLR_ANSI ) strcat( buf, " ansi" );
      if ( act & PLR_AUTOGOLD ) strcat( buf, " autogold" );
      if ( act & PLR_KEEPTITLE ) strcat( buf, " keeptitle" );
      if ( act & PLR_SOUND ) strcat( buf, " sound" );
      if ( act & PLR_MUSIC ) strcat ( buf, " music" );
      if ( act & PLR_GHOST ) strcat ( buf, " ghost" );
    }
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

char *act_bit_name2( int act )
{
    static char buf [ 512 ];
    
    buf[0] = '\0';
    if ( act & PLR_RELQUEST ) strcat (buf, " crusader");
    return ( buf[0] != '\0' ) ? buf+1 : "none";
}
        
CHAR_DATA *get_char( CHAR_DATA *ch )
{
    if ( !ch->pcdata )
        return ch->desc->original;
    else
        return ch;
}

bool longstring( CHAR_DATA *ch, char *argument )
{
    if ( strlen( argument) > 60 )
    {
	send_to_char(C_DEFAULT, "No more than 60 characters in this field.\n\r", ch );
	return TRUE;
    }
    else
        return FALSE;
}

bool authorized( CHAR_DATA *ch, char *skllnm )
{
    char buf [ MAX_STRING_LENGTH ];

    if ( ( !IS_NPC( ch ) && str_infix( skllnm, ch->pcdata->immskll ) )
        ||  IS_NPC( ch ) )
    {
        sprintf( buf, "Sorry, you are not authorized to use %s.\n\r", skllnm );
        send_to_char(AT_WHITE, buf, ch );
        return FALSE;
    }
    else

    return TRUE;

}


void end_of_game( void )
{


    DESCRIPTOR_DATA *d;
    DESCRIPTOR_DATA *d_next;

    for ( d = descriptor_list; d; d = d_next )
    {
	d_next = d->next;
	if ( d->connected == CON_PLAYING )
	{
	    if ( d->character->position == POS_FIGHTING )
	      interpret( d->character, "save" );
	    else
	      interpret( d->character, "quit" );
	}
    }

	#ifdef SQL_SYSTEM
	sql_save_dirty_list(FALSE);
	#endif

    return;

}

int race_lookup( const char *race )
{
    int index;

    for ( index = 0; index < MAX_RACE; index++ )
        if ( !str_cmp( race, race_table[index].race_full ) )
            return index;

    return -1;

}

int affect_lookup( const char *affectname )
{
    int index;

    for ( index = 0; index < MAX_SKILL; index++ )
        if ( !str_cmp( affectname, skill_table[index].name ) )
            return index;

    return -1;

}

/* XOR */
char *imm_bit_name(int imm_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (imm_flags & IMM_SUMMON		) strcat(buf, " summon");
    if (imm_flags & IMM_CHARM		) strcat(buf, " charm");
    if (imm_flags & IMM_MAGIC		) strcat(buf, " magic");
    if (imm_flags & IMM_WEAPON		) strcat(buf, " weapon");
    if (imm_flags & IMM_BASH		) strcat(buf, " blunt");
    if (imm_flags & IMM_PIERCE		) strcat(buf, " piercing");
    if (imm_flags & IMM_SLASH		) strcat(buf, " slashing");
    if (imm_flags & IMM_FIRE		) strcat(buf, " fire");
    if (imm_flags & IMM_COLD		) strcat(buf, " cold");
    if (imm_flags & IMM_LIGHTNING	) strcat(buf, " lightning");
    if (imm_flags & IMM_ACID		) strcat(buf, " acid");
    if (imm_flags & IMM_POISON		) strcat(buf, " poison");
    if (imm_flags & IMM_NEGATIVE	) strcat(buf, " negative");
    if (imm_flags & IMM_HOLY		) strcat(buf, " holy");
    if (imm_flags & IMM_ENERGY		) strcat(buf, " energy");
    if (imm_flags & IMM_MENTAL		) strcat(buf, " mental");
    if (imm_flags & IMM_DISEASE		) strcat(buf, " disease");
    if (imm_flags & IMM_DROWNING	) strcat(buf, " drowning");
    if (imm_flags & IMM_LIGHT		) strcat(buf, " light");
    if (imm_flags & IMM_SOUND		) strcat(buf, " light");
    if (imm_flags & VULN_IRON		) strcat(buf, " iron");
    if (imm_flags & VULN_WOOD		) strcat(buf, " wood");
    if (imm_flags & VULN_SILVER		) strcat(buf, " silver");

    return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune(CHAR_DATA *ch, int dam_type)
{
    int immune, def;
    int bit;

    immune = -1;
    def = IS_NORMAL;

    if (dam_type == DAM_NONE)
	return immune;

    if (dam_type <= 3)
    {
	if (IS_SET(ch->imm_flags,IMM_WEAPON))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_WEAPON))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vul_flags,VULN_WEAPON))
	    def = IS_VULNERABLE;
    }
    else /* magical attack */
    {	
	if (IS_SET(ch->imm_flags,IMM_MAGIC))
	    def = IS_IMMUNE;
	else if (IS_SET(ch->res_flags,RES_MAGIC))
	    def = IS_RESISTANT;
	else if (IS_SET(ch->vul_flags,VULN_MAGIC))
	    def = IS_VULNERABLE;
    }

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    switch (dam_type)
    {
	case(DAM_BASH):		bit = IMM_BASH;		break;
	case(DAM_PIERCE):	bit = IMM_PIERCE;	break;
	case(DAM_SLASH):	bit = IMM_SLASH;	break;
	case(DAM_FIRE):		bit = IMM_FIRE;		break;
	case(DAM_COLD):		bit = IMM_COLD;		break;
	case(DAM_LIGHTNING):	bit = IMM_LIGHTNING;	break;
	case(DAM_ACID):		bit = IMM_ACID;		break;
	case(DAM_POISON):	bit = IMM_POISON;	break;
	case(DAM_NEGATIVE):	bit = IMM_NEGATIVE;	break;
	case(DAM_HOLY):		bit = IMM_HOLY;		break;
	case(DAM_ENERGY):	bit = IMM_ENERGY;	break;
	case(DAM_MENTAL):	bit = IMM_MENTAL;	break;
	case(DAM_DISEASE):	bit = IMM_DISEASE;	break;
	case(DAM_DROWNING):	bit = IMM_DROWNING;	break;
	case(DAM_LIGHT):	bit = IMM_LIGHT;	break;
	case(DAM_CHARM):	bit = IMM_CHARM;	break;
	case(DAM_SOUND):	bit = IMM_SOUND;	break;
	default:		return def;
    }

    if (IS_SET(ch->imm_flags,bit))
	immune = IS_IMMUNE;
    else if (IS_SET(ch->res_flags,bit) && immune != IS_IMMUNE)
	immune = IS_RESISTANT;
    else if (IS_SET(ch->vul_flags,bit))
    {
	if (immune == IS_IMMUNE)
	    immune = IS_RESISTANT;
	else if (immune == IS_RESISTANT)
	    immune = IS_NORMAL;
	else
	    immune = IS_VULNERABLE;
    }

    if (immune == -1)
	return def;
    else
      	return immune;
}
/* END */

int advatoi (const char *s)
/*
  14k42 = 14 * 1000 + 14 * 100 + 2 * 10 = 14420

  Of course, it only pays off to use that notation when you can skip many 0's.
  There is not much point in writing 66k666 instead of 66666, except maybe
  when you want to make sure that you get 66,666.

  More than 3 (in case of 'k') or 6 ('m') digits after 'k'/'m' are automatically
  disregarded. Example:

  14k1234 = 14,123

  If the number contains any other characters than digits, 'k' or 'm', the
  function returns 0. It also returns 0 if 'k' or 'm' appear more than
  once.

*/

{

/* the pointer to buffer stuff is not really necessary, but originally I
   modified the buffer, so I had to make a copy of it. What the hell, it
   works:) (read: it seems to work:)
*/

  char string[MAX_INPUT_LENGTH]; /* a buffer to hold a copy of the argument */
  char *stringptr = string; /* a pointer to the buffer so we can move around */
  char tempstring[2];       /* a small temp buffer to pass to atoi*/
  int number = 0;           /* number to be returned */
  int multiplier = 0;       /* multiplier used to get the extra digits right */


  strcpy (string,s);        /* working copy */

  while ( isdigit (*stringptr)) /* as long as the current character is a digit */
  {
      strncpy (tempstring,stringptr,1);           /* copy first digit */
      number = (number * 10) + atoi (tempstring); /* add to current number */
      stringptr++;                                /* advance */
  }

  switch (UPPER(*stringptr)) {
      case 'K'  : multiplier = 1000;    number *= multiplier; stringptr++; break;
      case 'M'  : multiplier = 1000000; number *= multiplier; stringptr++; break;
      case '\0' : break;
      default   : return 0; /* not k nor m nor NUL - return 0! */
  }

  while ( isdigit (*stringptr) && (multiplier > 1))
/* if any digits follow k/m, add those too */
  {
      strncpy (tempstring,stringptr,1);           /* copy first digit */
      multiplier = multiplier / 10;  /* the further we get to right, the less are the digit 'worth' */
      number = number + (atoi (tempstring) * multiplier);
      stringptr++;
  }

  if (*stringptr != '\0' && !isdigit(*stringptr)) /* a non-digit character was found, other than NUL */
    return 0; /* If a digit is found, it means the multiplier is 1 - i.e. extra
                 digits that just have to be ignore, liked 14k4443 -> 3 is ignored */


  return (number);
}

/* checks to see whether this object needs to be deleted from the
character.  It calls itself recursively in two different ways.
One way is for finding the vnums in the dead obj tree, the other,
is for checking all the items in containers.... excerise in 
recursion anyone?    --Manaux
*/

bool dead_obj_check( CHAR_DATA *ch, OBJ_DATA * obj, DEAD_OBJ_DATA * node, 
		     DEAD_OBJ_DATA * head)
{
  OBJ_DATA * sub_obj;
  char        buf[200];

  int vnum = obj->pIndexData->vnum;


  /* If we've got a container, we need to check all the stuff inside
     recursively,  --Manaux
  */
if (obj->contains && obj->item_type == ITEM_CONTAINER)
    {
      if (DeadObjPrntOnly)
      {
	sprintf(buf, "Inside of %s:\n\r", obj->short_descr);
	send_to_char(AT_GREEN, buf, ch);
      }

      for( sub_obj = obj->contains; sub_obj; sub_obj = sub_obj->next_content )
	{
	  if (sub_obj->deleted)
	    continue;
	  if (dead_obj_check(ch, sub_obj, head, head) )
	    {
	      if (DeadObjPrntOnly)
		{
		  sprintf(buf, "In %s: %s\n\r", obj->short_descr,
			   sub_obj->short_descr);
		  send_to_char(AT_WHITE, buf, ch);
		}
	      else
		extract_obj(sub_obj);
	    }
	}
    }
  if (obj->pIndexData->vnum == OBJ_VNUM_DUMMY)
    return TRUE;
 
  if (!node)
    return FALSE;
  if (vnum < node->low_vnum)
    return dead_obj_check(ch, obj, node->left, head);
  else if (vnum > node->high_vnum)
    return dead_obj_check(ch, obj, node->right, head);
  else 
    return TRUE;
}



bool clean_player_objects( CHAR_DATA * ch)
{
  DEAD_OBJ_LIST * list = dead_object_list;
  DEAD_OBJ_DATA * head = 0;
  OBJ_DATA * obj = ch->carrying;
  int 	update = ch->updated;
  char    buf[200];


      /* find the appropriate tree */
  for ( ; list; list = list->next )
    {
      if (list->update == update)
	{
	  head = list->head;
	  break;
	}
    }
  if (!head && !DeadObjPrntOnly)
    {
      sprintf(buf, "No matching entry for update version %d\n\r", update);
      bug( buf, 0 );
      return TRUE;
    } 
 
  /* now, go through entire inventory and storage recursively checking
     against the tree.  */
  if (DeadObjPrntOnly)
    {
      sprintf(buf, "The following items are from non-existant areas, and will\n\rbe deleted during the next player update:\n\r");
      send_to_char(AT_WHITE, buf, ch);
    }
  for ( ; obj; obj = obj->next_content )
    {
      if (obj->deleted)
	continue;
      if (dead_obj_check(ch, obj, head, head))
	{ 
	  if (DeadObjPrntOnly)
	    {
	      sprintf(buf, "%s : %s \n\r", obj->name, obj->short_descr);
	      send_to_char(AT_WHITE, buf, ch);
	    }
	  else  extract_obj(obj);
	} 
    }

  for (obj = ch->pcdata->storage ; obj; obj = obj->next_content)
    {
      if (obj->deleted)
	continue;
      if (dead_obj_check(ch, obj, head, head))
	{
	  if (DeadObjPrntOnly)
	    {
	      sprintf( buf, "Storage: %s : %s\n\r", obj->name, obj->short_descr);     
	      send_to_char(AT_WHITE, buf, ch);
	    }
	  else extract_obj(obj);
	}
    }
  return TRUE;
}

int strlen_wo_col( char *argument )
{
  char *str;
  bool found = FALSE;
  int colfound = 0;
  int ampfound = 0;
  int len;
  for ( str = argument; *str != '\0'; str++ )
    {
      if ( found && is_colcode( *str ) )
        {
	  colfound++;
	  found = FALSE;
        }
      if ( found && *str == '&' )
        ampfound++;
      if ( *str == '&' )
        found = found ? FALSE : TRUE;
      else
        found = FALSE;
    }
  len = strlen( argument );
  len = len - ampfound - ( colfound * 2 );
  return len;
}

bool is_colcode( char code )
{
  if ( code == 'r'
    || code == 'R'
    || code == 'b'
    || code == 'B'
    || code == 'g'
    || code == 'G'
    || code == 'w'
    || code == 'W'
    || code == 'p'
    || code == 'P'
    || code == 'Y'
    || code == 'O'
    || code == 'c'
    || code == 'C'
    || code == 'z'
    || code == '.' )
        return TRUE;
  return FALSE;
}

char *strip_color( char *argument )
{
  char *str;
  char new_str [ MAX_INPUT_LENGTH ];
  int i = 0;
  for ( str = argument; *str != '\0'; str++ )
    {
    if ( new_str[ i-1 ] == '&' && is_colcode( *str ) )
        {
        i--;
        continue;
        }
    if ( new_str[ i-1 ] == '&' && *str == '&' )
        continue;
    new_str[i] = *str;
    i++;
    }
  if ( new_str[i] != '\0' )
        new_str[i] = '\0';
  str = str_dup( (char *)new_str );
  return str;
}

void add_poison( CHAR_DATA *ch, int amount )
{

    ch->poison_level += amount;

    if( ch->poison_level > MAX_POISON_LEVEL )
    {
	ch->poison_level = MAX_POISON_LEVEL ;
    }

    return;
}
