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

/*$Id: magic3.c,v 1.32 2005/04/10 16:29:00 tyrion Exp $*/

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

char *target_name;
extern void    set_fighting    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );


int spell_summon_shadow(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_WHITE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }
  /* the following block disables the summon_shadow spell */
  /* temp patch until the mob vnum exists -- REK */
  /* Enabled.
  if(ch->summon_timer <= 0)
  {
    send_to_char(AT_WHITE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }
  */
  mob = create_mobile(get_mob_index(MOB_VNUM_SHADOW));
  mob->level = URANGE(51, level, 100) - 20;
  mob->max_hit = mob->level * 20 + dice(10,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_GREY, "You summon $N from the shadow plane.", ch, NULL, mob, TO_CHAR);
  if(ch->bp < level/2 )
  {
    act(AT_RED,
     "You don't have enough blood to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  ch->bp -= level /2;
  act(AT_GREY, "$n calls forth $N from the shadow plane.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_WHITE, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_WHITE, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}


int spell_summon_trent(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_WHITE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_TRENT));
  mob->level = URANGE(51, level, 100) - 10;
  mob->max_hit = mob->level * 40 + dice(20,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_ORANGE, "You summon $N from the plane of nature.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 )
  {
    act(AT_WHITE,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  ch->mana -= level * 2;
  act(AT_ORANGE, "$n calls forth $N from the plane of nature.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_WHITE, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_WHITE, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_summon_beast(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;
  char        buf[MAX_STRING_LENGTH];
  char       *beast;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_WHITE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }
  /* the following block disables the summon_beast spell */
  /* temp patch until the mob vnum exists -- REK */
  /* Enabled.
  if(ch->summon_timer <= 0)
  {
    send_to_char(AT_WHITE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }
  */
  switch (number_bits( 4 ) )
  {
    case 0: beast = "horse"; break;
    case 1: beast = "cow"; break;
    case 2: beast = "bear"; break;
    case 3: beast = "lion"; break;
    case 4: beast = "bobcat"; break;
    case 5: beast = "mongoose"; break;
    case 6: beast = "rattle snake"; break;
    case 7: beast = "monkey"; break;
    default: beast = "tigeress"; break;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_BEAST));
  sprintf(buf, mob->short_descr, beast);
  free_string( mob->short_descr );
  mob->short_descr = str_dup(buf);
  sprintf(buf, mob->long_descr, beast, ch->name);
  free_string( mob->long_descr );
  mob->long_descr = str_dup(buf);
  mob->level = URANGE(51, level, 100) - 20;
  mob->max_hit = mob->level * 20 + dice(10,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_GREEN, "You call $N from the forests.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 )
  {
    act(AT_WHITE,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  ch->mana -= level * 2;
  act(AT_GREEN, "$n calls forth $N from the forests.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_WHITE, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_WHITE, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

void perm_spell(CHAR_DATA *victim, int sn)
{
  AFFECT_DATA *af;

  if(is_affected(victim, sn))
  {
    for(af = victim->affected; af != NULL; af = af->next)
    {
      if(af->type == sn)
      {
        af->duration = -1;
		return;
      }
    }
	for(af = victim->affected2; af != NULL; af = af->next)
    {
      if(af->type == sn)
      {
        af->duration = -1;
		return;
      }
    }
	for(af = victim->affected3; af != NULL; af = af->next)
    {
      if(af->type == sn)
      {
        af->duration = -1;
		return;
      }
    }
	for(af = victim->affected4; af != NULL; af = af->next)
    {
      if(af->type == sn)
      {
        af->duration = -1;
		return;
      }
    }
	for(af = victim->affected_powers; af != NULL; af = af->next)
    {
      if(af->type == sn)
      {
        af->duration = -1;
		return;
      }
    }
	for(af = victim->affected_weaknesses; af != NULL; af = af->next)
    {
      if(af->type == sn)
      {
        af->duration = -1;
		return;
      }
    }

  }
  return;
}

int spell_duration(CHAR_DATA *victim, int sn)
{
  AFFECT_DATA *af;

  if(is_affected(victim, sn))
  {
    for(af = victim->affected; af != NULL; af = af->next)
    {
      if(af->type == sn)
      {
        return af->duration;
      }
    }
    for(af = victim->affected2; af != NULL; af = af->next)
    {
      if(af->type == sn)
      {
        return af->duration;
      }
    }    for(af = victim->affected3; af != NULL; af = af->next)
    {
      if(af->type == sn)
      {
        return af->duration;
      }
    }    for(af = victim->affected4; af != NULL; af = af->next)
    {
      if(af->type == sn)
      {
        return af->duration;
      }
    }
  }
  return -2;
}
/* RT save for dispels */
/* modified for envy -XOR */
bool saves_dispel(int dis_level, int spell_level, int duration)
{
  int save;

  if(duration == -1)
    spell_level += 5;/* very hard to dispel permanent effects */
  save = 50 + (spell_level - dis_level) * 5;
  save = URANGE( 5, save, 95 );
  return number_percent() < save;
}

/* co-routine for dispel magic and cancellation */
bool check_dispel(int dis_level, CHAR_DATA *victim, int sn)
{
  AFFECT_DATA *af;

  if (is_affected(victim, sn))
  {
    for(af = victim->affected; af != NULL; af = af->next)
    {
      if(af->type == sn)
      {
        if(!saves_dispel(dis_level,af->level,af->duration))
        {
          affect_strip(victim,sn);
          if(skill_table[sn].msg_off)
          {
            send_to_char(C_DEFAULT, skill_table[sn].msg_off, victim );
            send_to_char(C_DEFAULT, "\n\r", victim );
          }
          if ( af->type == AFF_FLYING )
            check_nofloor( victim );
          return TRUE;
	}
	else
          af->level--;
      }
    }
  }
  return FALSE;
}
/* first of functions to try to dispel a bit off each of the
   affected loc'tns -- .._aff1, _aff2, _aff3 _aff4 are the indentical
   functions cept they work on different bit fields.--Manaux*/


void check_dispel_aff( CHAR_DATA *victim, bool * found, int level, const char * spell, long vector )
{
  int sn;
  sn = skill_lookup(spell);
  if (!skill_table[sn].dispel_bit)
    return;
  if(IS_AFFECTED(victim,vector)
   && !saves_spell(level, victim)
   && !is_affected(victim,sn) )
  {
    *found = TRUE;
    REMOVE_BIT(victim->affected_by,vector);
    if(skill_table[sn].msg_off)
    {
      act(C_DEFAULT, skill_table[sn].msg_off,
            victim, NULL, NULL, TO_CHAR);
      if(skill_table[sn].msg_off_room)
      {
        act(C_DEFAULT, skill_table[sn].msg_off_room,
            victim, NULL, NULL, TO_ROOM);
      }
    }
    if ( vector == AFF_FLYING )
     check_nofloor( victim );
  }
}
void check_dispel_aff2( CHAR_DATA *victim, bool * found, int level, const char * spell, long vector )
{
  int sn;
  sn = skill_lookup(spell);
  if(!skill_table[sn].dispel_bit)
    return;
  if(IS_AFFECTED2(victim,vector)
   && !saves_spell(level, victim)
   && !is_affected(victim,sn) )
  {
    *found = TRUE;
    REMOVE_BIT(victim->affected_by2,vector);
    if(skill_table[sn].msg_off)
    {
      act(C_DEFAULT, skill_table[sn].msg_off,
            victim, NULL, NULL, TO_CHAR);
      if(skill_table[sn].msg_off_room)
      {
        act(C_DEFAULT, skill_table[sn].msg_off_room,
            victim, NULL, NULL, TO_ROOM);
      }
    }
  }
}

void check_dispel_aff3( CHAR_DATA *victim, bool * found, int level, const char * spell, long vector )
{
  int sn;
  sn = skill_lookup(spell);
if (!skill_table[sn].dispel_bit)
  return;
  if(IS_AFFECTED3(victim,vector)
   && !saves_spell(level, victim)
   && !is_affected(victim,sn) )
  {
    *found = TRUE;
    REMOVE_BIT(victim->affected_by3,vector);
    if(skill_table[sn].msg_off)
    {
      act(C_DEFAULT, skill_table[sn].msg_off,
            victim, NULL, NULL, TO_CHAR);
      if(skill_table[sn].msg_off_room)
      {
        act(C_DEFAULT, skill_table[sn].msg_off_room,
            victim, NULL, NULL, TO_ROOM);
      }
    }
  }
}

void check_dispel_aff4( CHAR_DATA *victim, bool * found, int level, const char * spell, long vector )
{
  int sn;
  sn = skill_lookup(spell);
  if (!skill_table[sn].dispel_bit)
    return;
   if(IS_AFFECTED4(victim,vector)
      && !saves_spell(level, victim)
      && !is_affected(victim,sn) )
   {
    *found = TRUE;
    REMOVE_BIT(victim->affected_by4,vector);
    if(skill_table[sn].msg_off)
    {
      act(C_DEFAULT, skill_table[sn].msg_off,
            victim, NULL, NULL, TO_CHAR);
      if(skill_table[sn].msg_off_room)
      {
        act(C_DEFAULT, skill_table[sn].msg_off_room,
            victim, NULL, NULL, TO_ROOM);
      }
    }
  }

}




bool dispel_flag_only_spells(int level, CHAR_DATA *vo);
bool dispel_flag_only_spells( int level,  CHAR_DATA * victim )
{
bool found;
found = FALSE;
/*check_dispel_aff ( victim, &found, level, "detect evil", AFF_DETECT_EVIL);*/
check_dispel_aff ( victim, &found, level, "invis", AFF_INVISIBLE);
check_dispel_aff ( victim, &found, level, "detect invis", AFF_DETECT_INVIS);
check_dispel_aff ( victim, &found, level, "detect hidden", AFF_DETECT_HIDDEN);
check_dispel_aff ( victim, &found, level, "sanctuary", AFF_SANCTUARY);
check_dispel_aff ( victim, &found, level, "protection evil", AFF_PROTECT);
check_dispel_aff ( victim, &found, level, "hide", AFF_HIDE);
check_dispel_aff ( victim, &found, level, "fly", AFF_FLYING);
check_dispel_aff ( victim, &found, level, "fireshield", AFF_FIRESHIELD);
check_dispel_aff ( victim, &found, level, "shockshield", AFF_SHOCKSHIELD);
check_dispel_aff ( victim, &found, level, "iceshield", AFF_ICESHIELD);
check_dispel_aff ( victim, &found, level, "chaos field", AFF_CHAOS);
check_dispel_aff2( victim, &found, level, "unholy strength", AFF_UNHOLY_STRENGTH);
check_dispel_aff2( victim, &found, level, "blade barrier", AFF_BLADE);
check_dispel_aff2( victim, &found, level, "protection good", AFF_PROTECTION_GOOD);
check_dispel_aff2( victim, &found, level, "phase shift", AFF_PHASED);
check_dispel_aff2( victim, &found, level, "golden armor", AFF_GOLDEN_ARMOR);
check_dispel_aff2( victim, &found, level, "ghost shield", AFF_GHOST_SHIELD);
check_dispel_aff2( victim, &found, level, "mist", AFF_MIST);
check_dispel_aff2( victim, &found, level, "shadow image", AFF_SHADOW_IMAGE);
check_dispel_aff2( victim, &found, level, "improved invis", AFF_IMPROVED_INVIS);
check_dispel_aff3( victim, &found, level, "circle of fire", AFF_COFIRE);
check_dispel_aff3( victim, &found, level, "satanic inferno", AFF_SATANIC_INFERNO);
check_dispel_aff3( victim, &found, level, "bloodshield", AFF_BLOODSHIELD);
check_dispel_aff3( victim, &found, level, "randomshield", AFF_RANDOMSHIELD);
check_dispel_aff3( victim, &found, level, "acidshield", AFF_ACIDSHIELD);
check_dispel_aff3( victim, &found, level, "demonshield", AFF_DEMONSHIELD);
check_dispel_aff4( victim, &found, level, "golden sanctuary",AFF_GOLDEN_SANCTUARY);
check_dispel_aff4( victim, &found, level, "biofeedback",AFF_BIOFEEDBACK);
check_dispel_aff4( victim, &found, level, "earthshield", AFF_EARTHSHIELD);
check_dispel_aff4( victim, &found, level, "leaf shield", AFF_LEAF_SHIELD);
check_dispel_aff4( victim, &found, level, "luck shield", AFF_LUCK_SHIELD);
check_dispel_aff4( victim, &found, level, "tongues", AFF_TONGUES);
check_dispel_aff4( victim, &found, level, "liquid skin", AFF_LIQUID_SKIN);
check_dispel_aff4( victim, &found, level, "force of nature", AFF_FORCE_OF_NATURE);
check_dispel_aff4( victim, &found, level, "essence of gaia", AFF_ESSENCE_OF_GAIA);
return found;
}

/* modified by XOR */
/* Rom2 modified for enhanced use */

int spell_dispel_magic ( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;        /*flag to see if we have dispeled something yet*/
  AFFECT_DATA *paf;          /*pointer to affect to count through,
                               this will allows us to count through only the
                               affects that the character has --Manaux */

  if(saves_spell(level, victim))
  {
    send_to_char(C_DEFAULT, "You feel a brief tingling sensation.\n\r",victim);
    send_to_char(AT_BLUE, "You failed.\n\r", ch);
    return SKPELL_MISSED;
  }


  if ( IS_AFFECTED3( victim, AFF_HOLY_PROTECTION ) )
  {
    if( number_percent() < ch->level - 29 )
    {
    send_to_char(AT_WHITE, "Your holy protection withstands.\n\r", victim );
    send_to_char(AT_WHITE, "You failed to penetrate the holy protection.\n\r", ch);
      return SKPELL_MISSED;
    }
    send_to_char(AT_WHITE, "Your holy protection has failed.\n\r", victim );
    send_to_char(AT_WHITE, "You have breached the holy protection.\n\r", ch );
    REMOVE_BIT(victim->affected_by3, AFF_HOLY_PROTECTION);
    return SKPELL_MISSED;

  }



  for ( paf = victim->affected; paf; paf = paf->next )
  {
    if ( paf->deleted )
      continue;
    if ( skill_table[ (int) paf->type].dispel_bit == TRUE )
      {
      if(saves_spell(level, victim))
        continue;
        if(!saves_dispel(ch->level,paf->level,paf->duration))
          {
            affect_strip(victim,(int) paf->type);
            if(skill_table[paf->type].msg_off)
            {
              send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, victim );
              send_to_char(C_DEFAULT, "\n\r", victim );
            }
	    if(skill_table[paf->type].shield_bit)
	    {
		victim->shields -= 1;
	    }
	    found = TRUE;

            act(C_DEFAULT, skill_table[paf->type].msg_off_room,
	                         victim,NULL,NULL,TO_NOTVICT);

            if ( paf->type == AFF_FLYING )
              check_nofloor( victim );
          }
        }
  }

  for ( paf = victim->affected2; paf; paf = paf->next )
  {
    if ( paf->deleted )
      continue;
    if ( skill_table[ (int) paf->type].dispel_bit == TRUE )
      {
      if(saves_spell(level, victim) )
        continue;

        if(!saves_dispel(ch->level,paf->level,paf->duration))
          {
            affect_strip(victim,(int) paf->type);
            if(skill_table[paf->type].msg_off)
            {
              send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, victim );
              send_to_char(C_DEFAULT, "\n\r", victim );
            }
	    if(skill_table[paf->type].shield_bit)
	    {
		victim->shields -= 1;
	    }
	    found = TRUE;
            act(C_DEFAULT, skill_table[paf->type].msg_off_room,
	                         victim,NULL,NULL,TO_NOTVICT);

            if ( paf->type == AFF_FLYING )
              check_nofloor( victim );
          }
        }
      }

  for ( paf = victim->affected3; paf; paf = paf->next )
  {
    if ( paf->deleted )
      continue;
    if ( skill_table[ (int) paf->type].dispel_bit == TRUE )
      {
      if(saves_spell(level, victim))
        continue;


        if(!saves_dispel(ch->level,paf->level,paf->duration))
          {
            affect_strip(victim,(int) paf->type);
            if(skill_table[paf->type].msg_off)
            {
              send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, victim );
              send_to_char(C_DEFAULT, "\n\r", victim );
            }
	    if(skill_table[paf->type].shield_bit)
	    {
		victim->shields -= 1;
	    }
	    found = TRUE;

            act(C_DEFAULT, skill_table[paf->type].msg_off_room,
	                         victim,NULL,NULL,TO_NOTVICT);

            if ( paf->type == AFF_FLYING )
              check_nofloor( victim );
          }
        }
      }

  for ( paf = victim->affected4; paf; paf = paf->next )
    {
      if ( paf->deleted )
        continue;
      if ( skill_table[ (int) paf->type].dispel_bit == TRUE )
      {
      if(saves_spell(level, victim))
        continue;

        if(!saves_dispel(ch->level,paf->level,paf->duration))
          {
            affect_strip(victim,(int) paf->type);
            if(skill_table[paf->type].msg_off)
            {
              send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, victim );
              send_to_char(C_DEFAULT, "\n\r", victim );
            }
	    if(skill_table[paf->type].shield_bit)
	    {
		victim->shields -= 1;
    	    }
	    found = TRUE;

            act(C_DEFAULT, skill_table[paf->type].msg_off_room,
	                         victim,NULL,NULL,TO_NOTVICT);

            if (paf->type == AFF_FLYING )
              check_nofloor( victim );
          }
       }
    }
  found = found || dispel_flag_only_spells(level, victim);
  if (found == FALSE)
    {
      send_to_char(AT_BLUE, "You Failed.\n\r", ch);
    }
  else
    {
      send_to_char(AT_BLUE, "You have succeeded!\n\r", ch);
      act(AT_BLUE, "Some of $N's magic is removed!", ch, NULL, victim, TO_NOTVICT);
    }

   return SKPELL_NO_DAMAGE;
 }


int spell_cancellation(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;
  AFFECT_DATA *paf ;
  level += 2;

  if( sn == skill_lookup("cancellation") )
  {
    if((!IS_NPC(ch) && IS_NPC(victim)
     && !(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim))
     || (IS_NPC(ch) && !IS_NPC(victim))
     || !is_same_group(ch, victim))
    {
      send_to_char(C_DEFAULT, "You failed, try dispel magic.\n\r",ch);
      return SKPELL_MISSED;
    }
  }

  if( ch->fighting || victim->fighting )
  {
    send_to_char(C_DEFAULT, "You failed.\n\r", ch );
    return SKPELL_MISSED;
  }

/*Ok, here's the new improved cancellation routine --Manaux */
 for ( paf = victim->affected; paf; paf = paf->next )
  {
    if ( paf->deleted )
       continue;
    if ( skill_table[ (int) paf->type].cancel_bit == TRUE )
      {
      if(saves_spell(level - 20, victim))  /* -20 so it's easier to make */
        continue;                          /* saves vs yourself */
                                /* This will make having a really good
                                    savings not neccessarily a good thing */
        affect_strip(victim,(int) paf->type);
        if(skill_table[paf->type].msg_off)
        {
          send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, victim );
          send_to_char(C_DEFAULT, "\n\r", victim );
        }
        if(skill_table[paf->type].shield_bit)
        {
	    victim->shields -= 1;
	}
        found = TRUE;

        act(C_DEFAULT, skill_table[paf->type].msg_off_room,
	                         victim,NULL,NULL,TO_NOTVICT);

        if ( paf->type == AFF_FLYING )
          check_nofloor( victim );
      }

  }
 for ( paf = victim->affected2; paf; paf = paf->next )
  {
    if ( paf->deleted )
      continue;
    if ( skill_table[ (int) paf->type].cancel_bit == TRUE )
      {
      if(saves_spell(level - 20, victim))  /* -20 so it's easier to make */
        continue;                          /* saves vs yourself */
                                /* This will make having a really good
                                    savings not neccessarily a good thing */
        affect_strip(victim,(int) paf->type);
        if(skill_table[paf->type].msg_off)
        {
          send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, victim );
          send_to_char(C_DEFAULT, "\n\r", victim );
        }
	if(skill_table[paf->type].shield_bit)
	{
	    victim->shields -= 1;
	}
        found = TRUE;

        act(C_DEFAULT, skill_table[paf->type].msg_off_room,
	                         victim,NULL,NULL,TO_NOTVICT);

        if ( paf->type == AFF_FLYING )
          check_nofloor( victim );

    }
  }
 for ( paf = victim->affected3; paf; paf = paf->next )
  {
    if ( paf->deleted )
      continue;
    if ( skill_table[ (int) paf->type].cancel_bit == TRUE )
      {
      if(saves_spell(level - 20, victim))  /* -20 so it's easier to make */
        continue;                          /* saves vs yourself */
                                /* This will make having a really good
                                    savings not neccessarily a good thing */
        affect_strip(victim,(int) paf->type);
        if(skill_table[paf->type].msg_off)
        {
          send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, victim );
          send_to_char(C_DEFAULT, "\n\r", victim );
        }
	if(skill_table[paf->type].shield_bit)
	{
	    victim->shields -= 1;
	}
        found = TRUE;

        act(C_DEFAULT, skill_table[paf->type].msg_off_room,
	                         victim,NULL,NULL,TO_NOTVICT);

        if ( paf->type == AFF_FLYING )
          check_nofloor( victim );
      }

  }
 for ( paf = victim->affected4; paf; paf = paf->next )
  {
    if ( paf->deleted )
      continue;
    if ( skill_table[ (int) paf->type].cancel_bit == TRUE )
      {
      if(saves_spell(level - 20, victim))  /* -20 so it's easier to make */
        continue;                          /* saves vs yourself */
                                /* This will make having a really good
                                    savings not neccessarily a good thing */
        affect_strip(victim,(int) paf->type);
        if(skill_table[paf->type].msg_off)
        {
          send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, victim );
          send_to_char(C_DEFAULT, "\n\r", victim );
        }
	if(skill_table[paf->type].shield_bit)
	{
	    victim->shields -= 1;
	}
        found = TRUE;

        act(C_DEFAULT, skill_table[paf->type].msg_off_room,
	                         victim,NULL,NULL,TO_NOTVICT);

        if ( paf->type == AFF_FLYING )
          check_nofloor( victim );
      }

  }

  if(found)
    send_to_char(C_DEFAULT, "Ok.\n\r",ch);
  else
    send_to_char(C_DEFAULT, "Spell failed.\n\r",ch);
  return SKPELL_NO_DAMAGE;
}

/*
 * Turn undead and mental block by Altrag
 */

int spell_turn_undead( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int chance;

  if ( !IS_NPC(victim) || !IS_SET(victim->act, ACT_UNDEAD))
  {
    send_to_char(C_DEFAULT, "Spell failed.\n\r", ch );
    return SKPELL_MISSED;
  }

  chance = (level * (10 + IS_GOOD(ch) ? 15 : IS_EVIL(ch) ? 0 : 10) );
  chance /= victim->level;
  if (number_percent( ) < chance && !saves_spell( level, victim ))
  {
    act(AT_WHITE,"$n has turned $N!",ch,NULL,victim,TO_ROOM);
    act(AT_WHITE,"You have turned $N!",ch,NULL,victim,TO_CHAR);
    raw_kill(ch,victim);
    return SKPELL_NO_DAMAGE;
  }

  send_to_char(C_DEFAULT,"Spell failed.\n\r",ch);
  return SKPELL_MISSED;
}

int spell_mental_block( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected(victim,sn) )
    return SKPELL_MISSED;

  af.type = sn;
  af.level = level;
  af.duration = number_range( level / 4, level / 2 );
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_NOASTRAL;

  affect_to_char( victim, &af );

  send_to_char( AT_BLUE, "Your mind feels free of instrusion.\n\r",victim);
  if ( ch != victim )
    send_to_char(AT_BLUE, "Ok.\n\r",ch);

  return SKPELL_NO_DAMAGE;
}
/* END */
int spell_protection_good(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED2(ch, AFF_PROTECTION_GOOD) )
    return SKPELL_MISSED;

  af.type = sn;
  af.level = level;
  af.duration = 24;
  af.location = APPLY_DAM_HOLY;
  af.modifier = 20;
  af.bitvector = AFF_PROTECTION_GOOD;
  affect_to_char2( victim, &af );

  if ( ch != victim )
    send_to_char( AT_BLUE, "Ok.\n\r",ch);
  send_to_char(AT_BLUE, "You feel protected.\n\r",victim);
  return SKPELL_NO_DAMAGE;
}

int spell_detect_good(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED2( victim, AFF_DETECT_GOOD ) )
    return SKPELL_MISSED;

  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_DETECT_GOOD;
  affect_to_char2(ch, &af);

  if ( ch != victim)
    send_to_char(AT_BLUE, "Ok.\n\r",ch);
  send_to_char(AT_BLUE, "Your eyes tingle.\n\r",victim);
  return SKPELL_NO_DAMAGE;
}

int spell_holy_strength(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if(victim->position == POS_FIGHTING || is_affected(victim, sn))
    return SKPELL_MISSED;
  af.type       = sn;
  af.level	= level;
  af.duration   = 6 + level;
  af.location   = APPLY_HITROLL;
  af.modifier   = level / 4;
  af.bitvector  = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_DAMROLL;
  af.modifier  = level / 4;
  affect_to_char( victim, &af );

  af.location  = APPLY_STR;
  af.modifier  = level / 50;
  affect_to_char( victim, &af );

  if(ch != victim)
    send_to_char(AT_BLUE, "Ok.\n\r", ch );
  send_to_char(AT_BLUE, "The strength of the gods fills you.\n\r", victim);
  return SKPELL_NO_DAMAGE;
}

int spell_curse_of_nature(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int loss;

  if(IS_AFFECTED2( victim, AFF_CURSE_NATURE) || saves_spell(level, victim))
  {
	send_to_char(AT_RED, "You have failed.\n\r", ch );
	return SKPELL_MISSED;
  }

  loss = number_fuzzy ( ch->level * 2 / 3 );

  af.type       = sn;
  af.level	= level;
  af.duration   = level / 6;
  af.location   = APPLY_HITROLL;
  af.modifier   = 0 - loss;
  af.bitvector  = AFF_CURSE_NATURE;
  affect_to_char2( victim, &af );

  af.location  = APPLY_DAMROLL;
  af.modifier  = 0 - loss;
  af.bitvector = AFF_CURSE;
  affect_to_char2( victim, &af );

  af.location  = APPLY_STR;
  af.modifier  = 0 - level / 30;
  affect_to_char2( victim, &af );

  if(ch != victim)
    send_to_char(AT_GREEN, "Ok.\n\r", ch );
    send_to_char(AT_GREEN, "The wrath of nature wrecks you.\n\r", victim);
  return SKPELL_NO_DAMAGE;
}

int spell_enchanted_song(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( ch == victim )
  {
    act( AT_BLUE, "$n sings an enchanting song.", ch, NULL, NULL, TO_ROOM );
    send_to_char(AT_BLUE, "You sing a song.\n\r", ch );
  }

  if ( victim->position == POS_STUNNED || IS_STUNNED( ch, STUN_TO_STUN ) )
    return SKPELL_MISSED;

  act( AT_BLUE, "Your song pacifies $N.", ch, NULL, victim, TO_CHAR );
  act( AT_BLUE, "$n's song pacifies $N.", ch, NULL, victim, TO_NOTVICT );
  act( AT_BLUE, "$n's song slows your reactions.", ch, NULL, victim, TO_VICT );

  STUN_CHAR( ch, 5, STUN_TO_STUN );
  STUN_CHAR( victim, 1, STUN_TOTAL );
  victim->position = POS_STUNNED;

  return SKPELL_NO_DAMAGE;
}

/* RT haste spell */

int spell_haste( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE))
    {
	if (victim == ch)
	  send_to_char(C_DEFAULT, "You can't move any faster!\n\r",ch);
 	else
	  act(C_DEFAULT, "$N is already moving as fast as they can.",
	      ch,NULL,victim,TO_CHAR);
        return SKPELL_MISSED;
    }
    af.type      = sn;
    af.level     = level;
    if (victim == ch)
      af.duration  = level/2;
    else
      af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    send_to_char(C_DEFAULT,
     "You feel yourself moving more quickly.\n\r", victim );
    act(C_DEFAULT, "$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
    if ( ch != victim )
        send_to_char(C_DEFAULT, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

int spell_swiftness( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE))
    {
        if (victim == ch)
          send_to_char(C_DEFAULT, "You can't move any faster!\n\r",ch);
        else
          act(C_DEFAULT, "$N is already moving as fast as $e can.",
              ch,NULL,victim,TO_CHAR);
        return SKPELL_MISSED;
    }
    af.type      = sn;
    af.level     = level;
    if (victim == ch)
      af.duration  = level/2;
  else
      af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    send_to_char(C_DEFAULT,
     "You feel yourself moving more quickly.\n\r", victim );
    act(C_DEFAULT, "$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
    if ( ch != victim )
        send_to_char(C_DEFAULT, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

int spell_plague(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int hr, dr;
  if(is_affected(victim, sn)|| saves_spell(level, victim))
  {
    send_to_char(AT_RED, "You have failed.\n\r", ch );
    return SKPELL_MISSED;
  }
	hr = GET_HITROLL(victim);
	dr = GET_DAMROLL(victim);
  af.type       = sn;
  af.level      = level;
  af.duration   = level / 20;
  af.location   = APPLY_HITROLL;
  af.modifier   = 0 - hr / 4;
  af.bitvector  = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_DAMROLL;
  af.modifier  = 0 - dr / 4;
  affect_to_char( victim, &af );

  af.location  = APPLY_STR;
  af.modifier  = 0 - level / 15;
  affect_to_char( victim, &af );

  af.location  = APPLY_HIT;
  af.modifier  = 0 - level * 4;
  affect_to_char( victim, &af );

  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_CURSE;
  affect_to_char( victim, &af );

  if(ch != victim)
    send_to_char(AT_GREEN, "Ok.\n\r", ch );
  send_to_char(AT_GREEN, "You feel the wrath of the Plague.\n\r", victim);
  return SKPELL_NO_DAMAGE;
}

int spell_unholy_curse( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	int hr, dr;

    if ( saves_spell( level + (ch->race == 15 ? 20 : 0), victim )
)
    {
	send_to_char(AT_BLUE, "You have failed.\n\r", ch );
	return SKPELL_MISSED;
    }

    if (is_affected( victim, sn ) )
    {
	send_to_char(AT_BLUE, "You have failed.\n\r", ch );
	return SKPELL_MISSED;
    }

	hr = GET_HITROLL(victim);
	dr = 2 * GET_DAMROLL(victim);
    af.type      = sn;
    af.level	 = level;
    af.duration  = level / 20;
    af.location  = APPLY_HITROLL;
    af.modifier  = 0 - hr  / 3;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );

    af.location  = APPLY_DAMROLL;
    af.modifier  = 0 - dr  / 3;
    affect_to_char( victim, &af );

    af.location  = APPLY_AC;
    af.modifier  = level * 2 ;
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    act(AT_WHITE, "$N has been cursed!", ch, NULL, victim, TO_CHAR    );
    send_to_char(AT_WHITE, "You feel the wrath of an unholy curse!\n\r", victim );
    act(AT_WHITE, "$N has been cursed!", ch, NULL, victim, TO_NOTVICT );
    return SKPELL_NO_DAMAGE;
}

int spell_unholy_prayer(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if(victim->position == POS_FIGHTING || is_affected(victim, sn))
    return SKPELL_MISSED;

  af.type       = sn;
  af.level	= level;
  af.duration   = 6 + level;
  af.location   = APPLY_HITROLL;
  af.modifier   = level / 5;
  af.bitvector  = 0;
  affect_to_char( victim, &af );

  af.location  = APPLY_DAMROLL;
  af.modifier  = level / 3;
  affect_to_char( victim, &af );

  af.location  = APPLY_STR;
  af.modifier  = level / 30;
  affect_to_char( victim, &af );

  af.location  = APPLY_HIT;
  af.modifier  = level * 5;
  affect_to_char( victim, &af );

  if(ch != victim)
    send_to_char(AT_BLUE, "Ok.\n\r", ch );

  send_to_char(AT_BLUE, "The strength of demons fill you.\n\r", victim);
  return SKPELL_NO_DAMAGE;
}

int spell_unholy_wrath( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
	  0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,  45,	 50,  55,  60,  70,  80,
	 90,  95, 105, 110, 120,	123, 126, 129, 132, 135,
	138, 141, 144, 147, 150,	153, 156, 159, 162, 165,
	168, 171, 174, 177, 180,	183, 186, 189, 192, 195,
	198, 201, 204, 207, 210,	213, 216, 219, 222, 225,
	228, 231, 234, 237, 240,	243, 246, 249, 252, 255,
	258, 261, 264, 267, 270,	273, 276, 279, 282, 285,
	288, 291, 294, 297, 300,	303, 306, 309, 312, 315,
	318, 321, 324, 327, 330,	335, 340, 345, 350, 350
    };
		 int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 5 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_chi_blast( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
	  0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,  27,	 32,  37,  42,  46,  50,
	 54,  58,  63,  67,  72,	 75,  76,  77,  79,  81,
	 83,  85,  87,  89,  90,	 92,  94,  96,  98, 100,
	102, 104, 106, 108, 110,	112, 114, 116, 118, 110,
	122, 124, 126, 128, 130,	132, 134, 136, 138, 140,
	142, 144, 146, 148, 150,	152, 154, 156, 158, 160,
	162, 164, 166, 168, 170,	172, 174, 176, 178, 180,
	182, 184, 186, 188, 190,	192, 194, 196, 198, 200,
	205, 210, 215, 220, 225,	230, 235, 240, 245, 250
    };
		 int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 7 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}


int spell_chi_storm( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
	  0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,  36,	 42,  48,  54,  60,  66,
	 72,  78,  84,  90,  96,	102, 104, 106, 107, 108,
	110, 113, 115, 117, 120,	122, 124, 127, 129, 132,
	134, 137, 139, 142, 144,	147, 149, 152, 154, 157,
	159, 162, 164, 167, 169,	172, 174, 177, 179, 182,
	184, 187, 189, 192, 194,	196, 198, 200, 202, 204,
	206, 208, 210, 212, 214,	216, 218, 220, 222, 224,
	226, 228, 230, 231, 232,	233, 234, 235, 236, 237,
	238, 239, 240, 242, 244,	246, 248, 250, 255, 260
    };
		 int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 7 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}


int spell_sunburst( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
	  0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,  60,	 70,  80,  90, 100, 110,
	120, 130, 140, 150, 160,	164, 168, 172, 176, 180,
	184, 188, 192, 196, 200,	204, 208, 212, 216, 220,
	224, 228, 232, 236, 240,	244, 248, 252, 256, 260,
	264, 268, 272, 276, 280,	284, 288, 292, 296, 300,
	305, 310, 315, 320, 325,	330, 335, 340, 345, 350,
	355, 360, 365, 370, 375,	380, 385, 390, 395, 400,
	405, 410, 415, 420, 425,	430, 435, 440, 445, 450,
	460, 470, 480, 490, 500,	510, 520, 530, 540, 550
    };
		 int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( (int) (dam_each[level] * 1.5), dam_each[level] * 7 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
//    damage( ch, victim, dam, sn );
    return dam;
}

int spell_chi_wave( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
   AFFECT_DATA af;

    send_to_char(AT_RED, "You send your soul into the world!\n\r", ch );
    act(AT_RED, "$n is surrounded by Chi energy!", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch; vch = vch->next )
    {
        if ( vch->deleted || !vch->in_room )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && ( IS_NPC( ch ) ? !IS_NPC( vch )
			                     :  IS_NPC( vch ) ) )
		damage( ch, vch, level + dice( level, 12 ), sn );
            if ( vch != ch )
            {
                af.type      = sn;
                af.level     = level;
                af.duration  = level / 10;
                af.location  = APPLY_NONE;
                af.modifier  = 0;
                af.bitvector = AFF_CURSE;
                affect_join( vch, &af );
	        send_to_char(AT_RED, "You feel the wrath of the Chi Wave!\n\r", vch);
	     }
	    continue;
	}

    }

    return SKPELL_NO_DAMAGE;
}


int spell_chi_healing( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = (int)(victim->max_hit * 0.1);
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "The feeling of Chi fills your body.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}


int spell_phantom_form( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 5 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );

    send_to_char(AT_GREY, "You take on a phantom form.\n\r", victim );
    act(AT_GREY, "$n takes on a phantom form.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}


int spell_spark( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim      = (CHAR_DATA *) vo;
    static const int       dam_each [ ] =
    {
         0,
         3,  3,  4,  4,  5,      6,  6,  6,  6,  6,
         7,  7,  7,  7,  7,      8,  8,  8,  8,  8,
         9,  9,  9,  9,  9,     10, 10, 10, 10, 10,
        11, 11, 11, 11, 11,     12, 12, 12, 12, 12,
        13, 13, 13, 13, 13,     14, 14, 14, 14, 14,
        15, 15, 15, 15, 15,     16, 16, 16, 16, 16,
        18, 18, 18, 18, 18,     20, 20, 20, 20, 20,
        21, 21, 21, 21, 21,     22, 22, 22, 22, 22,
        24, 24, 24, 24, 24,     26, 26, 26, 26, 26,
        28, 28, 28, 28, 28,     30, 31, 32, 33, 40
    };
                 int       dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam      = number_range( dam_each[level] / 2, dam_each[level] * 5 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}


int spell_ghost_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_GHOST_SHIELD ) )
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
    af.level	 = level;
    af.duration  = -1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_GHOST_SHIELD;
    affect_to_char2( victim, &af );
    victim->shields += 1;

    send_to_char(AT_GREY, "You conjure hundreds of ghosts to surround you.\n\r", victim );
    act(AT_GREY, "$n's body is protected by hundreds of ghosts.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}
/*
int spell_flaming_fists( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_FLAMING_FISTS ) )
	return SKPELL_MISSED;

    af.type	= sn;
    af.level	= level;
    af.duration = number_fuzzy( level / 5 );
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_FLAMING_FISTS;
    affect_to_char2( victim, &af );

    send_to_char(AT_RED, "You set your fists on fire!\n\r", victim );
    act(AT_RED, "$n's hands burst into flames!", victim, NULL, NULL, TO_ROOM );
    return SKPELL_MISSED;
}
*/
int spell_mist( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_MIST ) )
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
    af.level	 = level;
    af.duration  = -1;
    af.location  = APPLY_AC;
    af.modifier  = ch->level * -1;
    af.bitvector = AFF_MIST;
    affect_to_char2( victim, &af );
    victim->shields += 1;

    send_to_char(AT_GREY, "You create a mist about yourself.\n\r", victim );
    act(AT_GREY, "$n's body is shrouded by a heavy mist.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_scrye( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
   ROOM_INDEX_DATA *blah;

    if ( !( victim = get_char_world( ch, target_name ) )
        || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
        || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  ) )
    {
        send_to_char(AT_BLUE, "You failed.\n\r", ch );
        return SKPELL_MISSED;
    }

    if (IS_NPC(victim) && victim->pIndexData->vnum == 7)
    {
	send_to_char(AT_BLUE, "You failed.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PRESENT)
        && (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
        || IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
        send_to_char(AT_BLUE, "You cannot!!!!!!!\n\r", ch );
        return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_FUTURE)
        && (IS_SET( victim->in_room->area->area_flags, AREA_PRESENT )
        || IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
        send_to_char(AT_BLUE, "You cannot!!!!!!!\n\r", ch );
        return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PAST)
        && (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
        || IS_SET( victim->in_room->area->area_flags, AREA_PRESENT ) ) )
    {
        send_to_char(AT_BLUE, "You cannot!!!!!!!\n\r", ch );
        return SKPELL_MISSED;
    }

    if ( IS_AFFECTED4( victim, AFF_DECEPTION ) && (ch!=victim) 
       && ( number_percent( ) <= ( number_fuzzy( victim->pcdata->learned[skill_lookup("deception of aura")])/2 ) + get_curr_int( victim ) ) )
    {
        int vnum = 0;
        ROOM_INDEX_DATA* room	 	= NULL;
        ROOM_INDEX_DATA* victim_room	= NULL;

        while (!room)
        {
           vnum = number_range(1, 31000);
	   room = get_room_index( vnum );

	   if (!room) { continue; }

	   if (IS_SET(room->area->area_flags, AREA_PRESENT) && !IS_SET(ch->in_room->area->area_flags, AREA_PRESENT))
	   {
		room = NULL;
		continue;
	   }
	   if (IS_SET(room->area->area_flags, AREA_FUTURE) && !IS_SET(ch->in_room->area->area_flags, AREA_FUTURE))
	   { 
		room = NULL;
		continue;
	   }
	   if (IS_SET(room->area->area_flags, AREA_PAST) && !IS_SET(ch->in_room->area->area_flags, AREA_PAST))
	   {
		room = NULL;
		continue;
	   }
	}
        victim_room = victim->in_room;
        char_from_room( victim );
        char_to_room  ( victim, room );
      
   	blah = ch->in_room;
        char_from_room( ch );
        char_to_room  ( ch, room );

        do_look(ch, "auto");         
	
        char_from_room( victim );
        char_from_room( ch     );
        char_to_room  ( victim, victim_room);
        char_to_room  ( ch, blah );
 
        return SKPELL_NO_DAMAGE;
    }    

    blah = ch->in_room;
    if ( ch != victim )
    {
     char_from_room( ch );
     char_to_room( ch, victim->in_room );
    }
    do_look( ch, "auto" );
    if (ch != victim )
    {
      char_from_room( ch );
      char_to_room( ch, blah );
     }
    return SKPELL_NO_DAMAGE;
}

int spell_visions( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
   ROOM_INDEX_DATA *blah;

    if ( !( victim = get_char_world( ch, target_name ) ) )
    {
        send_to_char(AT_BLUE, "You failed.\n\r", ch );
        return SKPELL_MISSED;
    }

    // Supermob sanity check - Ahsile
    if ( IS_NPC(victim) && victim->pIndexData->vnum == 7)
    {
	send_to_char(AT_BLUE, "You failed.\n\r", ch);
	return SKPELL_MISSED;
    }

/* Changed to work through time, to make it different than Scrye */
/*
    if ( IS_SET(ch->in_room->area->area_flags, AREA_PRESENT)
        && (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
        || IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
        send_to_char(AT_BLUE, "You cannot!!!!!!!\n\r", ch );
        return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_FUTURE)
        && (IS_SET( victim->in_room->area->area_flags, AREA_PRESENT )
        || IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
        send_to_char(AT_BLUE, "You cannot!!!!!!!\n\r", ch );
        return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PAST)
        && (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
        || IS_SET( victim->in_room->area->area_flags, AREA_PRESENT ) ) )
    {
        send_to_char(AT_BLUE, "You cannot!!!!!!!\n\r", ch );
        return SKPELL_MISSED;
    }
*/

    if ( IS_AFFECTED4( victim, AFF_DECEPTION ) && (ch!=victim) 
       && ( number_percent( ) <= ( number_fuzzy( victim->pcdata->learned[skill_lookup("deception of aura")])/2 ) + get_curr_int( victim ) ) )
    {
        int vnum = 0;
        ROOM_INDEX_DATA* room	 	= NULL;
        ROOM_INDEX_DATA* victim_room	= NULL;

        while (!room)
        {
           vnum = number_range(1, 31000);
	   room = get_room_index( vnum );
	}
          
        victim_room = victim->in_room;
        char_from_room( victim );
        char_to_room  ( victim, room );
      
   	blah = ch->in_room;
        char_from_room( ch );
        char_to_room  ( ch, room );

        do_look(ch, "auto");         
	
        char_from_room( victim );
        char_from_room( ch     );
        char_to_room  ( victim, victim_room);
        char_to_room  ( ch, blah );
 
        return SKPELL_NO_DAMAGE;
    }    

    blah = ch->in_room;
    if ( ch != victim )
    {
     char_from_room( ch );
     char_to_room( ch, victim->in_room );
    }
    do_look( ch, "auto" );
    if (ch != victim )
    {
      char_from_room( ch );
      char_to_room( ch, blah );
     }
    return SKPELL_NO_DAMAGE;
}

int spell_shadow_image( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_SHADOW_IMAGE ) )
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
    af.level	 = level;
    af.duration  = -1;
    af.location  = APPLY_AC;
    af.modifier  = (ch->level / 4) * -1;
    af.bitvector = AFF_SHADOW_IMAGE;
    affect_to_char2( victim, &af );
    victim->shields += 1;

    send_to_char(AT_BLUE, "You appear to be in more than one location..\n\r", victim );
    act(AT_BLUE, "$n's body appears to be in more than one location.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_malignify(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if(saves_spell(level, victim))
  {
    send_to_char(AT_RED, "You have failed.\n\r", ch );
    return SKPELL_MISSED;
  }

  af.type       = sn;
  af.level	= level;
  af.duration   = level / 25;
  af.location   = APPLY_DEX;
  af.modifier   = -1;
  af.bitvector  = AFF_MALIGNIFY;
  affect_to_char2( victim, &af );

  af.location  = APPLY_STR;
  af.modifier  = -1;
  affect_to_char2( victim, &af );

  af.location  = APPLY_CON;
  af.modifier  = -1;
  affect_to_char2( victim, &af );

  af.location  = APPLY_INT;
  af.modifier  = -1;
  affect_to_char2( victim, &af );

  if(ch != victim)
    send_to_char(AT_BLOOD, "You malignify them.\n\r", ch );
  send_to_char(AT_BLOOD, "You have been hit with malignification.\n\r", victim);
  return SKPELL_NO_DAMAGE;
}


int spell_age(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED3( victim, AFF_AGE ) || saves_spell( level, victim ) )
  {
	send_to_char(AT_RED, "You have failed.\n\r", ch );
	return SKPELL_MISSED;
  }

  af.type       = sn;
  af.level	= level;
  af.duration   = number_fuzzy( level );
  af.location   = APPLY_DEX;
  af.modifier   = -2;
  af.bitvector  = AFF_AGE;
  affect_to_char3( victim, &af );

  af.location  = APPLY_STR;
  af.modifier  = -4;
  affect_to_char( victim, &af );

  af.location  = APPLY_WIS;
  af.modifier  = 3;
  affect_to_char( victim, &af );

  af.location  = APPLY_CON;
  af.modifier  = -1;
  affect_to_char( victim, &af );

  af.location  = APPLY_INT;
  af.modifier  = 3;
  affect_to_char( victim, &af );

  if(ch != victim)
    send_to_char(AT_BLOOD, "You age them.\n\r", ch );
  send_to_char(AT_BLOOD, "You appear to be older.\n\r", victim);
  return SKPELL_NO_DAMAGE;
}


int spell_iceball( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
	  0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,  30,	 35,  40,  45,  50,  55,
	 60,  65,  70,  75,  80,	 82,  84,  86,  88,  90,
	 92,  94,  96,  98, 100,	102, 104, 106, 108, 110,
	112, 114, 116, 118, 120,	122, 124, 126, 128, 130,
	132, 134, 136, 138, 140,	142, 144, 146, 148, 150,
	152, 154, 156, 158, 160,	162, 164, 166, 168, 170,
	172, 174, 176, 178, 180,	182, 184, 186, 188, 190,
	192, 194, 196, 198, 200,	202, 204, 206, 208, 210,
	215, 220, 225, 230, 235,	240, 245, 250, 255, 260
    };
		 int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 7 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_solidify( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    char        buf[MAX_STRING_LENGTH];

    if ( is_affected( victim, sn ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 40 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ANTI_FLEE;
    affect_to_char( victim, &af );

    sprintf( buf, "%s lifts his hands your legs are frozen!\n\r", ch->name );
    send_to_char(AT_LBLUE, buf, victim );
    act(AT_LBLUE, "$n has been immobilized by a chunk of ice.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}


int spell_winters_chill( int sn, int level, CHAR_DATA *ch, void *vo )
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
    dam	     = number_range( dam_each[level], dam_each[level] * 8 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
//    damage( ch, victim, dam, sn );
    return dam;
}

int spell_summon_ice_elemental(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_RED,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_ICE_ELEMENTAL));
  mob->level = URANGE(51, level, 100) - 5;
  mob->max_hit = mob->level * 20 + dice(1,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_RED, "You summon $N from the arctic.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 && ((ch->class != 9)||(ch->class != 11)))
  {
    act(AT_RED,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if(ch->bp < level / 2 && ((ch->class == 9)||(ch->class == 11)))
  {
    act(AT_BLOOD,
     "You don't have enough blood to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if((ch->class != 9) || (ch->class != 11))
    ch->mana -= level * 2;
  else
    ch->bp -= level / 2;
  act(AT_RED, "$n summons $N from the arctic.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_RED, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_RED, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_summon_fire_elemental(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_RED,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_FIRE_ELEMENTAL));
  mob->level = URANGE(51, level, 100) - 5;
  mob->max_hit = mob->level * 50 + dice(1,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_RED, "You summon $N from a blazing inferno.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 && ((ch->class != 9)||(ch->class != 11)))
  {
    act(AT_RED,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if(ch->bp < level / 2 && ((ch->class == 9)||(ch->class == 11)))
  {
    act(AT_BLOOD,
     "You don't have enough blood to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if((ch->class != 9) || (ch->class != 11))
    ch->mana -= level * 2;
  else
    ch->bp -= level / 2;
  act(AT_RED, "$n summons $N from a blazing inferno.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_RED, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_RED, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_summon_greater_demon(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_RED,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_GREATER_DEMON));
  mob->level = URANGE(51, level, 100) - 5;
  mob->max_hit = mob->level * 90 + dice(1,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_RED, "You summon $N from the bowels of hell.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 && ((ch->class != 9)||(ch->class != 11)))
  {
    act(AT_RED,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if(ch->bp < level / 2 && ((ch->class == 9)||(ch->class == 11)))
  {
    act(AT_BLOOD,
     "You don't have enough blood to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if((ch->class != 9) || (ch->class != 11))
    ch->mana -= level * 2;
  else
    ch->bp -= level / 2;
  act(AT_RED, "$n summons $N from the bowels of hell.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_RED, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_RED, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}


int spell_cone_of_frost( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE))
	{
	send_to_char(AT_WHITE, "You are in a safe room!!!!", ch);
	return SKPELL_MISSED;
	}

    send_to_char(AT_LBLUE, "A huge cone of frost stretches forth from your hands!\n\r", ch );
    act(AT_LBLUE, "A huge cone of frost stretches forth from $n's hands.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch; vch = vch->next )
    {
        if ( vch->deleted || !vch->in_room )
	    continue;
	if ( vch->in_room == ch->in_room && vch != ch )
	{
		damage( ch, vch, level + dice( level/3, level ), sn );

	    continue;
	}

    }

    return SKPELL_NO_DAMAGE;
}

int spell_icequake( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;

    send_to_char(AT_LBLUE, "The earth trembles as the ground is rocked with ice!\n\r", ch );
    act(AT_LBLUE, "$n causes the earth to tremble as it is rocked with ice.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch; vch = vch->next )
    {
        if ( vch->deleted || !vch->in_room )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && ( IS_NPC( ch ) ? !IS_NPC( vch )
			                     :  IS_NPC( vch ) ) )
		damage( ch, vch, level + dice( 3, 9 ), sn );
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char(AT_LBLUE, "The earth trembles as it is rocked with ice.\n\r", vch );
    }

    return SKPELL_NO_DAMAGE;
}

int spell_cloud_of_cold( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
   AFFECT_DATA af;

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE))
	{
	send_to_char(AT_WHITE, "You are in a safe room!!!!", ch);
	return SKPELL_MISSED;
	}

    send_to_char(AT_LBLUE, "You conjure a giant cloud of cold.\n\r", ch );
    act(AT_LBLUE, "$n conjures a giant cloud of cold!", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch; vch = vch->next )
    {
        if ( vch->deleted || !vch->in_room )
	    continue;
	if ( vch->in_room == ch->in_room && vch != ch )
	{
		damage( ch, vch, level + dice( level/2, level/3 ), sn );
            if ( vch != ch )
            {
                af.type      = sn;
                af.level     = level;
                af.duration  = 5;
                af.location  = APPLY_NONE;
                af.modifier  = 0;
                af.bitvector = AFF_CURSE;
                affect_join( vch, &af );
	        send_to_char(AT_LBLUE, "Your eyes are affected by the cloud!\n\r", vch);
	     }
	    continue;
	}

    }

    return SKPELL_NO_DAMAGE;
}

int spell_tomb_rot( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn) || saves_spell( level, victim )
)
    {
	send_to_char(AT_RED, "You have failed.\n\r", ch );
	return SKPELL_MISSED;
    }

    af.type      = sn;
    af.level	 = level;
    af.duration  = level / 20;
    af.location  = APPLY_HITROLL;
    af.modifier  = -80;
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    spell_poison( skill_lookup("poison"), ch->level, ch, victim );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 50;
    affect_to_char( victim, &af );

    if ( ch != victim )
	send_to_char(AT_RED, "You have inflicted the tomb rot.\n\r", ch );
    send_to_char(AT_RED, "You have been afflicted by the tomb rot.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}


int spell_flash_burn( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || saves_spell( level, victim )
|| IS_AFFECTED2( victim, AFF_BLINDFOLD ) )
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

    act(AT_RED, "$N is blinded by the flash burn!", ch, NULL, victim, TO_CHAR    );
    send_to_char(AT_RED, "You are blinded by a flash burn!\n\r", victim );
    act(AT_RED, "$N is blinded by a flash burn!", ch, NULL, victim, TO_NOTVICT );
    return SKPELL_NO_DAMAGE;
}

int spell_daemonic_might( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( !IS_EVIL( ch ) )
    {
	send_to_char(AT_RED, "You are not evil enough to do that!\n\r", ch);
	return SKPELL_MISSED;
    }

    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
        return SKPELL_MISSED;
    af.type      = sn;
    af.level     = level;
    af.duration  = level - 40;
    af.location  = APPLY_DAMROLL;
    af.modifier  = level / 4;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_HITROLL;
    af.modifier  = level / 4;
    affect_to_char( victim, &af );

    af.location  = APPLY_STR;
    af.modifier  = 3;
    affect_to_char( victim, &af );

    if ( ch != victim )
        send_to_char(AT_RED, "You conjure the strength of demons.\n\r", ch );
    send_to_char(AT_RED, "You feel the touch of demons rest upon you.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int spell_blood_omen( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( !IS_EVIL( ch ) )
    {
	send_to_char(AT_RED, "You are not evil enough to do that!\n\r", ch);
	return SKPELL_MISSED;
    }

    if ( is_affected( victim, sn ) )
        return SKPELL_MISSED;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 10;
    af.location  = APPLY_DAMROLL;
    af.modifier  = 0 - level / 3 * 2;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_HITROLL;
    af.modifier  = 0 - level / 3 * 2;
    affect_to_char( victim, &af );

    af.location  = APPLY_STR;
    af.modifier  = -3;
    affect_to_char( victim, &af );

    if ( ch != victim )
        send_to_char(AT_RED, "You have placed a blood omen hex.\n\r", ch );
    send_to_char(AT_RED, "You are hexed by the powers of the blood omen.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int spell_daemonic_possession( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( !IS_EVIL( ch ) )
    {
	send_to_char(AT_RED, "You are not evil enough to do that!\n\r", ch);
	return SKPELL_MISSED;
    }

    if ( victim == ch )
    {
	send_to_char(C_DEFAULT, "Possess yourself?  You're weird.\n\r", ch );
	return SKPELL_MISSED;
    }
    if ( !IS_NPC( victim ) )
       return SKPELL_MISSED;

    if (   IS_AFFECTED( victim, AFF_CHARM )
	|| IS_AFFECTED( ch,     AFF_CHARM )
	|| level < victim->level
	|| saves_spell( level, victim ) )
        return SKPELL_MISSED;

    if(IS_SIMM(victim, IMM_CHARM))
      return SKPELL_MISSED;

    if ( victim->master )
        stop_follower( victim );
    add_follower( victim, ch );

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = number_fuzzy( level / 4 );
    af.location	 = APPLY_NONE;
    af.modifier	 = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );

    act(AT_RED, "The demons take control of $N and transfer the control to you!", ch, NULL, victim, TO_CHAR );
    act(AT_RED, "Demons have taken control of you.", ch, NULL, victim, TO_VICT );
    return SKPELL_NO_DAMAGE;
}


int spell_circle_of_fire( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED3( victim, AFF_COFIRE ) )
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
    af.level	 = level;
    af.duration  = -1;
    af.location  = APPLY_DAM_WATER;
    af.modifier  = 25;
    af.bitvector = AFF_COFIRE;
    affect_to_char3( victim, &af );
    victim->shields += 1;

    send_to_char(AT_RED, "Your body is surrounded by a circle of fire.\n\r", victim );
    act(AT_RED, "$n's body is surrounded by a circle of fire.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_tortured_soul( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( !IS_EVIL( ch ) )
    {
	send_to_char(AT_RED, "You are not evil enough to do that!\n\r", ch);
	return SKPELL_MISSED;
    }

    if ( IS_AFFECTED3( victim, AFF_TORTURE ) )
        return SKPELL_MISSED;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 10;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_TORTURE;
    affect_to_char3( victim, &af );

    if ( ch != victim )
        send_to_char(AT_RED, "You have tortured their soul.\n\r", ch );
    send_to_char(AT_RED, "Your soul has been damned by pure evil.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}


int spell_demonfire( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
	  0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,  60,	 70,  80,  90, 100, 110,
	120, 130, 140, 150, 160,	164, 168, 172, 176, 180,
	184, 188, 192, 196, 200,	204, 208, 212, 216, 220,
	224, 228, 232, 236, 244,	248, 252, 256, 260, 262,
	262, 264, 264, 266, 266,	268, 268, 270, 270, 275,
	275, 280, 280, 285, 285,	290, 290, 295, 295, 300,
	305, 310, 315, 320, 325,	330, 345, 340, 345, 350,
	355, 360, 365, 370, 375,	380, 385, 390, 395, 400,
	410, 420, 430, 440, 450,	460, 470, 480, 490, 500
    };
		 int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 5 );
    if ( IS_AFFECTED( victim, AFF_SANCTUARY ) )
	dam += (int)(dam*1.3);
    if ( IS_AFFECTED4( victim, AFF_GOLDEN_SANCTUARY ) )
        dam += (int)(dam*1.15);
    if ( saves_spell( level, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );
    return SKPELL_MISSED;
}

int spell_summon_air_elemental(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_RED,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_AIR_ELEMENTAL));
  mob->level = URANGE(51, level, 100) - 5;
  mob->max_hit = mob->level * 50 + dice(1,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_RED, "You summon $N from the skies.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 && ((ch->class != 9)||(ch->class != 11)))
  {
    act(AT_RED,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if(ch->bp < level / 2 && ((ch->class == 9)||(ch->class == 11)))
  {
    act(AT_BLOOD,
     "You don't have enough blood to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if((ch->class != 9) || (ch->class != 11))
    ch->mana -= level * 2;
  else
    ch->bp -= level / 2;
  act(AT_RED, "$n summons $N from the skies.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_RED, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_RED, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_summon_water_elemental(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_RED,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_WATER_ELEMENTAL));
  mob->level = URANGE(51, level, 100) - 5;
  mob->max_hit = mob->level * 50 + dice(1,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_RED, "You summon $N from the oceans.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 && ((ch->class != 9)||(ch->class != 11)))
  {
    act(AT_RED,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if(ch->bp < level / 2 && ((ch->class == 9)||(ch->class == 11)))
  {
    act(AT_BLOOD,
     "You don't have enough blood to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if((ch->class != 9) || (ch->class != 11))
    ch->mana -= level * 2;
  else
    ch->bp -= level / 2;
  act(AT_RED, "$n summons $N from the oceans.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_RED, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_RED, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_summon_earth_elemental(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_RED,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_EARTH_ELEMENTAL));
  mob->level = URANGE(51, level, 100) - 5;
  mob->max_hit = mob->level * 50 + dice(1,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_RED, "You summon $N from the planet core.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 && ((ch->class != 9)||(ch->class != 11)))
  {
    act(AT_RED,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if(ch->bp < level / 2 && ((ch->class == 9)||(ch->class == 11)))
  {
    act(AT_BLOOD,
     "You don't have enough blood to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if((ch->class != 9) || (ch->class != 11))
    ch->mana -= level * 2;
  else
    ch->bp -= level / 2;
  act(AT_RED, "$n summons $N from the planet core.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_RED, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_RED, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_summon_dust_elemental(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_RED,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_ICE_ELEMENTAL));
  mob->level = URANGE(51, level, 100) - 5;
  mob->max_hit = mob->level * 45 + dice(1,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_RED, "You summon $N from the dust around you.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 && ((ch->class != 9)||(ch->class != 11)))
  {
    act(AT_RED,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if(ch->bp < level / 2 && ((ch->class == 9)||(ch->class == 11)))
  {
    act(AT_BLOOD,
     "You don't have enough blood to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if((ch->class != 9) || (ch->class != 11))
    ch->mana -= level * 2;
  else
    ch->bp -= level / 2;
  act(AT_RED, "$n summons $N from the dust around you.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_RED, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_RED, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_summon_dragon(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_RED,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_DRAGON));
  mob->level = URANGE(51, level, 100) - 5;
  mob->max_hit = mob->level * 75 + dice(1,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_RED, "You summon $N from the draconic plane.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 && ((ch->class != 9)||(ch->class != 11)))
  {
    act(AT_RED,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if(ch->bp < level / 2 && ((ch->class == 9)||(ch->class == 11)))
  {
    act(AT_BLOOD,
     "You don't have enough blood to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if((ch->class != 9) || (ch->class != 11))
    ch->mana -= level * 2;
  else
    ch->bp -= level / 2;
  act(AT_RED, "$n summons $N from the draconic plane.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_RED, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_RED, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_shockwave(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  int		dam;

  if ( ch == victim )
  {
  	send_to_char(C_DEFAULT, "Cast shockwave on yourself?  Are you stupid?\n\r", ch);
	return SKPELL_MISSED;
  }
  if (!IS_NPC( victim ) )
  {
	dam = UMAX(  20, victim->hit - dice( 1,4 ) );
	dam = UMIN( 175, dam);
	//damage( ch, victim, dam, sn );
	return dam;
  }
  act( AT_BLUE, "Your shockwave stuns $N.", ch, NULL, victim, TO_CHAR );
  act( AT_BLUE, "$n's shockwave stuns $N.", ch, NULL, victim, TO_NOTVICT );
  send_to_char( AT_BLUE, "You are stunned by a shockwave.", victim );
  STUN_CHAR( victim, UMIN( level / 50, 1 * PULSE_VIOLENCE ), STUN_TOTAL );
  return SKPELL_NO_DAMAGE;
}

int spell_control_dragon(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;
  char buf [ MAX_STRING_LENGTH ];

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_RED,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_DRAGON));
  mob->level = URANGE(51, level, 100) - 5;
  mob->max_hit = mob->level * 75 + dice(1,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_RED, "You summon $N from the draconic plane.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 && ((ch->class != 9)||(ch->class != 11)))
  {
    act(AT_RED,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if(ch->bp < level / 2 && ((ch->class == 9)||(ch->class == 11)))
  {
    act(AT_BLOOD,
     "You don't have enough blood to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  if((ch->class != 9) || (ch->class != 11))
    ch->mana -= level * 2;
  else
    ch->bp -= level / 2;
  act(AT_RED, "$n summons $N from the draconic plane.", ch, NULL, mob, TO_ROOM);

  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

    sprintf( buf, "Control Dragon: (%s)", ch->name );
    log_string( buf, CHANNEL_GOD, -1 );

    do_control_switch( ch, mob->name );

    act(AT_WHITE, "$n takes control of the dragon!!", ch,NULL, NULL, TO_ROOM );
    send_to_char(AT_WHITE, "You take control of the dragon!", ch );
    send_to_char(AT_WHITE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;

  if(ch->position == POS_FIGHTING)
  {
    act(AT_RED, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_RED, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_control_trent(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;
  char buf [ MAX_STRING_LENGTH ];

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_WHITE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_TRENT));
  mob->level = URANGE(51, level, 100) - 10;
  mob->max_hit = mob->level * 40 + dice(20,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_ORANGE, "You summon $N from the plane of nature.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 )
  {
    act(AT_WHITE,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  ch->mana -= level * 2;
  act(AT_ORANGE, "$n calls forth $N from the plane of nature.", ch, NULL, mob, TO_ROOM);

  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

    sprintf( buf, "Control Trent: (%s)", ch->name );
    log_string( buf, CHANNEL_GOD, -1 );

    do_control_switch( ch, mob->name );

    act(AT_WHITE, "$n takes control of the Trent!!", ch,NULL, NULL, TO_ROOM );
    send_to_char(AT_WHITE, "You take control of the Trent!", ch );
    send_to_char(AT_WHITE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;

  if(ch->position == POS_FIGHTING)
  {
    act(AT_WHITE, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_WHITE, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_control_wolf(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;
  char buf [ MAX_STRING_LENGTH ];

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_WHITE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_WOLF));
  mob->level = URANGE(31, level, 60) - 10;
  mob->max_hit = mob->level * 20 + dice(20,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_ORANGE, "You summon $N from the plane of nature.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 )
  {
    act(AT_WHITE,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  ch->mana -= level * 2;
  act(AT_ORANGE, "$n calls forth $N from the plane of nature.", ch, NULL, mob, TO_ROOM);

  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

    sprintf( buf, "Control Wolf: (%s)", ch->name );
    log_string( buf, CHANNEL_GOD, -1 );

    do_control_switch( ch, mob->name );

    act(AT_WHITE, "$n takes control of the Wolf!!", ch,NULL, NULL, TO_ROOM );
    send_to_char(AT_WHITE, "You take control of the Wolf!", ch );
    send_to_char(AT_WHITE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;

  if(ch->position == POS_FIGHTING)
  {
    act(AT_WHITE, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_WHITE, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_summon_wolf(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_WHITE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_WOLF));
  mob->level = URANGE(31, level, 60) - 10;
  mob->max_hit = mob->level * 20 + dice(20,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_ORANGE, "You summon $N from the plane of nature.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 )
  {
    act(AT_WHITE,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  ch->mana -= level * 2;
  act(AT_ORANGE, "$n calls forth $N from the plane of nature.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_WHITE, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_WHITE, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_control_hawk(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;
  char buf [ MAX_STRING_LENGTH ];

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_WHITE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_HAWK));
  mob->level = URANGE(31, level, 60) - 10;
  mob->max_hit = mob->level * 30 + dice(20,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_ORANGE, "You summon $N from the plane of nature.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 )
  {
    act(AT_WHITE,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  ch->mana -= level * 2;
  act(AT_ORANGE, "$n calls forth $N from the plane of nature.", ch, NULL, mob, TO_ROOM);

  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

    sprintf( buf, "Control Hawk: (%s)", ch->name );
    log_string( buf, CHANNEL_GOD, -1 );

    do_control_switch( ch, mob->name );

    act(AT_WHITE, "$n takes control of the Hawk!!", ch,NULL, NULL, TO_ROOM );
    send_to_char(AT_WHITE, "You take control of the Hawk!", ch );
    send_to_char(AT_WHITE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;

  if(ch->position == POS_FIGHTING)
  {
    act(AT_WHITE, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_WHITE, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_summon_hawk(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_WHITE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_HAWK));
  mob->level = URANGE(31, level, 60) - 10;
  mob->max_hit = mob->level * 30 + dice(20,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_ORANGE, "You summon $N from the plane of nature.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 )
  {
    act(AT_WHITE,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  ch->mana -= level * 2;
  act(AT_ORANGE, "$n calls forth $N from the plane of nature.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_WHITE, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_WHITE, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_summon_tiger(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_WHITE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_TIGER));
  mob->level = URANGE(41, level, 70) - 10;
  mob->max_hit = mob->level * 30 + dice(20,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_ORANGE, "You summon $N from the plane of nature.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 )
  {
    act(AT_WHITE,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  ch->mana -= level * 2;
  act(AT_ORANGE, "$n calls forth $N from the plane of nature.", ch, NULL, mob, TO_ROOM);

  mob->master = ch;
  mob->leader = ch;
  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  if(ch->position == POS_FIGHTING)
  {
    act(AT_WHITE, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_WHITE, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_control_tiger(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;
  char buf [ MAX_STRING_LENGTH ];

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_WHITE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_TIGER));
  mob->level = URANGE(41, level, 70) - 10;
  mob->max_hit = mob->level * 30 + dice(20,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_ORANGE, "You summon $N from the plane of nature.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 )
  {
    act(AT_WHITE,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  ch->mana -= level * 2;
  act(AT_ORANGE, "$n calls forth $N from the plane of nature.", ch, NULL, mob, TO_ROOM);

  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.modifier  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

    sprintf( buf, "Control Tiger: (%s)", ch->name );
    log_string( buf, CHANNEL_GOD, -1 );

    do_control_switch( ch, mob->name );

    act(AT_WHITE, "$n takes control of the Tiger!!", ch,NULL, NULL, TO_ROOM );
    send_to_char(AT_WHITE, "You take control of the Tiger!", ch );
    send_to_char(AT_WHITE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;


  if(ch->position == POS_FIGHTING)
  {
    act(AT_WHITE, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_WHITE, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_divining( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA *spring;

	spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0);
	spring->timer = level;
	obj_to_room( spring, ch->in_room );

	act(AT_BLUE, "$p flows from a hole in the ground.", ch, spring, NULL, TO_CHAR);
	act(AT_BLUE, "$p flows from a hole in the ground.", ch, spring, NULL, TO_ROOM );
	return SKPELL_NO_DAMAGE;
}

int spell_unholy_fires( int sn, int level, CHAR_DATA *ch, void *vo )
{
		CHAR_DATA *victim	= (CHAR_DATA *) vo;
		int     dam;

    dam = dice( level, 20 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
//    damage( ch, victim, dam, sn );
    return dam;
}

int spell_stigeon_mists(int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE))
	{
	send_to_char(AT_WHITE, "You are in a safe room!!!!", ch);
	return SKPELL_MISSED;
	}

    send_to_char(AT_LBLUE, "You conjure a cloud of mist.\n\r", ch );
    act(AT_LBLUE, "$n congures a poisonous mist!", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch; vch = vch->next )
    {
	if ( vch->deleted || !vch->in_room )
	    continue;
	if ( vch->in_room == ch->in_room && vch != ch)
    	{
	    damage( ch, vch, level + dice( level/2, level/4 ), sn );
	    if ( vch != ch && !IS_AFFECTED4(vch,AFF_BURROW))
 	    {
		spell_poison( skill_lookup("poison"), ch->level, ch, vch );
	    }
	   continue;
	}
   }
   return SKPELL_NO_DAMAGE;
}

int spell_satanic_caress( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
        return SKPELL_MISSED;
    af.type      = sn;
    af.level     = level;
    af.duration  = 10 + level;
    af.location  = APPLY_DAMROLL;
    af.modifier  = level / 5;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_HITROLL;
    af.modifier  = level / 5;
    affect_to_char( victim, &af );

    af.location  = APPLY_HIT;
    af.modifier  = level * 2;
    affect_to_char( victim, &af );

    if ( ch != victim )
    {
        send_to_char(AT_RED, "You pray for Satan's blessing.\n\r", ch );
    }
    send_to_char(AT_RED, "Satan's blessing is bestowed upon you.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int spell_control_undead(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;
  char buf [ MAX_STRING_LENGTH ];

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_WHITE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_UNDEAD));
  mob->level = URANGE(31, level, 60) - 10;
  mob->max_hit = mob->level * 20 + dice(20,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_ORANGE, "You summon $N from the Abyss.", ch, NULL, mob, TO_CHAR);

  if(ch->mana < level * 2 )
  {
    act(AT_WHITE,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  ch->mana -= level * 2;
  act(AT_ORANGE, "$n calls forth $N from the Abyss.", ch, NULL, mob, TO_ROOM);

  af.type      = skill_lookup("charm person");
  af.level     = level;
  af.duration  = -1;
  af.location  = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

    sprintf( buf, "Control Undead: (%s)", ch->name );
    log_string( buf, CHANNEL_GOD, -1 );

    do_control_switch( ch, mob->name );

    act(AT_WHITE, "$n tikes control of the Undead!!", ch,NULL, NULL, TO_ROOM );
    send_to_char(AT_WHITE, "You take control of the Undead!", ch );
    send_to_char(AT_WHITE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;

  if(ch->position == POS_FIGHTING)
  {
    act(AT_WHITE, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_WHITE, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int spell_satanic_inferno( int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED3( victim, AFF_SATANIC_INFERNO ) )
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
    af.location  = APPLY_DAM_HOLY;
    af.modifier  = 30;
    af.bitvector = AFF_SATANIC_INFERNO;
    affect_to_char3( victim, &af );
    send_to_char(AT_RED, "You summon Satan's mighty inferno to protect you.\n\r", ch );
    act(AT_RED, "$n's body is ingulfed by a satanic inferno.", victim, NULL, NULL, TO_ROOM );
    victim->shields += 1;

      return SKPELL_NO_DAMAGE;
}

int spell_spark_blade( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA	*obj = (OBJ_DATA *) vo;

    if ( obj->item_type != ITEM_WEAPON
	|| IS_OBJ_STAT( obj, ITEM_MAGIC )
	|| IS_OBJ_STAT2( obj, ITEM_SPARKING )
	|| obj->affected )
    {
	send_to_char(AT_DBLUE, "That item cannot be enchanted.\n\r", ch );
	return SKPELL_MISSED;
    }
    SET_BIT( obj->extra_flags, ITEM_MAGIC);
    SET_BIT( obj->extra_flags2, ITEM_SPARKING);
    send_to_char(AT_RED, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

int spell_pestilence( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( saves_spell( level, victim ) )
	return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level/10;
    af.location  = APPLY_STR;
    af.modifier  = -4;
    af.bitvector = AFF_PESTILENCE;
    affect_to_char3( victim, &af );

    if( ch != victim )
	send_to_char(AT_GREEN, "Ok.\n\r", ch );
    send_to_char(AT_GREEN, "You have been infected.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int spell_shadow_bolt( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
  0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,  30,	 35,  40,  45,  50,  55,
	 60,  65,  70,  75,  80,	 82,  84,  86,  88,  90,
	 92,  94,  96,  98, 100,	102, 104, 106, 108, 110,
	112, 114, 116, 118, 120,	122, 124, 126, 128, 130,
	132, 134, 136, 138, 140,	142, 144, 146, 148, 150,
	152, 154, 156, 158, 160,	162, 164, 166, 168, 170,
	172, 174, 176, 178, 180,	182, 184, 186, 188, 190,
	192, 194, 196, 198, 200,	202, 204, 206, 208, 210,
	215, 220, 225, 230, 235,	240, 245, 250, 255, 260
    };

                 int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam      = number_range( dam_each[level] / 2, dam_each[level] * 6 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_shadow_storm( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
   AFFECT_DATA af;

    send_to_char(AT_RED, "You send your shadows into the world!\n\r", ch
);
    act(AT_RED, "$n is surrounded by shadows!", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch; vch = vch->next )
    {
        if ( vch->deleted || !vch->in_room )
            continue;
        if ( vch->in_room == ch->in_room )
        {
            if ( vch != ch && ( IS_NPC( ch ) ? !IS_NPC( vch )
                                             :  IS_NPC( vch ) ) )
                damage( ch, vch, level + dice( level, 12 ), sn );
            if ( vch != ch )
            {
                af.type      = sn;
                af.level     = level;
                af.duration  = level / 10;
                af.location  = APPLY_NONE;
                af.modifier  = 0;
                af.bitvector = AFF_CURSE;
                affect_join( vch, &af );
                send_to_char(AT_RED, "You feel the wrath of shadows!\n\r",
vch);
             }
            continue;
        }

    }
    return SKPELL_NO_DAMAGE;
}
