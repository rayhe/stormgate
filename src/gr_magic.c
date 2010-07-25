/*$Id: gr_magic.c,v 1.14 2005/03/17 02:41:09 tyrion Exp $*/

/*****************************************************************************
 * Interface for group spell casting.                                        *
 * -- Altrag                                                                 *
 *****************************************************************************/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"

/*
 * Local functions
 */
void add_gspell  args ( ( CHAR_DATA *ch, int sn, int level, void *vo ) );
int gslot_lookup  args ( ( int sn ) );
bool is_same_gspell  args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );


/*
 * NonLocal functions manually declared
 */
void say_spell(CHAR_DATA * ch, int sn);
int slot_lookup(int slot);

bool is_same_gspell( CHAR_DATA *ch, CHAR_DATA *victim )
{
  if ( !is_same_group( ch, victim ) )
    return FALSE;
  if ( !ch->gspell || !victim->gspell )
    return FALSE;
  if ( ch->gspell->sn != victim->gspell->sn )
    return FALSE;
  if ( ch->gspell->victim != victim->gspell->victim )
    return FALSE;
  return TRUE;
}

void set_gspell( CHAR_DATA *ch, GSPELL_DATA *gsp )
{
  GSPELL_DATA *gsp_new;

  gsp_new = alloc_mem( sizeof(*gsp_new) );

  *gsp_new = *gsp;
  ch->gspell = gsp_new;
}

void end_gspell( CHAR_DATA *ch )
{
  if ( !ch->gspell )
  {
    bug( "end_gspell: no gspell", 0 );
    return;
  }
  free_mem( ch->gspell, sizeof(*ch->gspell) );
  ch->gspell = NULL;
}

/*
 * Implementation stuff
 */
void check_gcast( CHAR_DATA *ch )
{
  CHAR_DATA *gch;
  int looper;
  int casters[MAX_CLASS];
  int sn;
  int level = 0;
  int total = 0;

  if ( !ch->gspell || ch->gspell->timer <= 0 )  {
    return;
  }

  sn = ch->gspell->sn;

  for ( looper = 0; looper < MAX_CLASS; looper++ )
    casters[looper] = 0;

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
    if ( is_same_gspell( ch, gch ) )
    {
      if (gch->class != gch->multied)
		casters[gch->multied]++;
      casters[gch->class]++;
      total++;
      level = (level / 2) + gch->gspell->level / 2;
    }
  }

  for ( looper = 0; looper < MAX_CLASS; looper++ )
    if ( casters[looper] < gskill_table[gslot_lookup(sn)].casters[looper] )
    {
      send_to_char(AT_BLUE, "You begin casting a group spell.\n\r", ch );
	  say_spell(ch, slot_lookup(sn));
      return;
    }

  sn = slot_lookup(sn);
  say_spell(ch, slot_lookup(sn));
  act(AT_BLUE, "$n unleashes a group spell!", ch, NULL, NULL, TO_ROOM );
  (* skill_table[sn].spell_fun) ( sn, level, ch->leader ? ch->leader : ch,
				  ch->gspell->victim );
  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
    if ( is_same_gspell( ch, gch ) && ch != gch )
      end_gspell( gch );
  }
  switch (skill_table[sn].target)
  {
  case TAR_GROUP_OFFENSIVE:
    {
      CHAR_DATA *victim = (CHAR_DATA *)ch->gspell->victim;

      rprog_cast_sn_trigger( ch->in_room, ch, sn, victim );
      if ( !victim->fighting )
	{
	  multi_hit( victim, ch->leader ? ch->leader : ch, TYPE_UNDEFINED );
	}
	  /* Ok, now we're starting a fight, do a multi_hit on either the
	     caster, or the caster's leader, and make all non-fighting
	     group members do a multi_hit() on the victim
	     --Manaux
	     */
      for (gch = ch->in_room->people; gch; gch = gch->next_in_room)
	{
	  if (gch->deleted)
	    continue;
	  if ((gch->leader == (ch->leader ? ch->leader : ch )) 
	      && (!gch->fighting))
	    {
	      multi_hit(gch, victim, TYPE_UNDEFINED);
	    }
	}
    }
  
    break;
  case TAR_GROUP_DEFENSIVE:
    rprog_cast_sn_trigger( ch->in_room, ch, sn, ch->gspell->victim );
    break;
  case TAR_GROUP_ALL:
    rprog_cast_sn_trigger( ch->in_room, ch, sn, ch );
    break;
  case TAR_GROUP_IGNORE:
    rprog_cast_sn_trigger( ch->in_room, ch, sn,
			  (ch->gspell->victim ? ch->gspell->victim : ch) );
    break;
  case TAR_GROUP_OBJ:
    if ( ch->gspell->victim )
    {
      oprog_cast_sn_trigger( ch->gspell->victim, ch, sn, ch->gspell->victim );
      rprog_cast_sn_trigger( ch->in_room, ch, sn, ch->gspell->victim );
    }
    break;
  }
  end_gspell( ch );
  return;
}

int gslot_lookup( int sn )
{
  int count;

  for ( count = 0; count < MAX_GSPELL; count++ )
    if ( gskill_table[count].slot == sn )
      return count;

  bug( "Gslot_lookup: sn not found #%d", sn );
  return 0;
}

void add_gspell( CHAR_DATA *ch, int sn, int level, void *vo )
{
  CHAR_DATA *gch;
  GSPELL_DATA gsp;

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
  {
    if ( gch == ch || !gch->gspell || gch->gspell->timer <= 0 )
      continue;
    if ( is_same_group( ch, gch ) && gch->gspell->sn == sn )
    {
      switch ( skill_table[sn].target )
      {
      case TAR_GROUP_DEFENSIVE:
      case TAR_GROUP_OFFENSIVE:
      case TAR_GROUP_OBJ:
      case TAR_GROUP_IGNORE:
	if ( gch->gspell->victim != vo )
	  continue;
	break;
      }
      break;
    }
  }
  sn = skill_table[sn].slot;
  gsp.sn = sn;
  gsp.victim = vo;
  gsp.level = level;
  if ( !gch || !gch->gspell )
    gsp.timer = gskill_table[gslot_lookup(sn)].wait;
  else
    gsp.timer = gch->gspell->timer;
  set_gspell( ch, &gsp );
  check_gcast( ch );
  return;
}

void group_cast( int sn, int level, CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim = NULL;
  CHAR_DATA * rch = NULL;
  OBJ_DATA *obj = NULL;
  int found = FALSE;
  int mana;

  if ( IS_NPC(ch) )
    return;

  if ( ch->gspell && ch->gspell->timer > 0 )
    {
      send_to_char(AT_BLUE,"You already have a group spell in progress.\n\r",ch);
      return;
    }
  /* check to make sure that ch is part of a group.
     --Manaux
     */
  if ( (!ch->leader) || (ch->leader == ch) )
    for (rch = ch->in_room->people; rch; rch = rch->next_in_room )
      {
	if (rch->deleted || rch == ch)
	  continue;
	if (rch->leader == ch )
	  {
	    found = TRUE;
	    break;
	  }
      }
  else
    found = TRUE;
  if (!found)
    {
      send_to_char(AT_BLUE, "You don't have a group here!\n\r", ch);
      return;
    }

  mana = MANA_COST(ch,sn);
  if (( ch->class == 9 )||( ch->class == 11))
    mana /= 4;

  if ((( ch->class == 9 )||( ch->class == 11 )) && ( ch->bp < mana))
    {
      send_to_char(AT_RED, "You don't have enough blood to cast that!\n\r", ch);
      return;
    }
  else if ((ch->class != 9 && ch->class != 11) && (ch->mana < mana))
    {
      send_to_char(AT_BLUE, "You don't have enough mana!\n\r", ch);
      return;
    }
  WAIT_STATE(ch, skill_table[sn].beats);

  if ( number_percent( ) > ( ch->pcdata->learned[sn] / 10 )  )
    {
      send_to_char(AT_BLUE, "You lost your concentration.\n\r",ch);
      if (( ch->class != 9 )&&(ch->class != 11))
	ch->mana -= mana / 2;
      else
	ch->bp -= mana / 2;
      if( ch->pcdata->learned[sn] <= 750 )
	update_skpell( ch, sn, 0 );
      return;
    }

  if (( ch->class != 9 )&&( ch->class != 11))
    ch->mana -= mana;
  else
    ch->bp -= mana;

  switch ( skill_table[sn].target )
    {
    default:
      bug( "group_cast: non-group target on sn #%d.", sn );
      return;
    case TAR_GROUP_DEFENSIVE:
      if ( argument[0] == '\0' )
	{
	  victim = ch;
	  break;
	}
      if ( !( victim = get_char_room( ch, argument ) ) )
	{
	  send_to_char( AT_BLUE, "They aren't here.\n\r", ch );
	  return;
	}
      add_gspell( ch, sn, level, (void *) victim );
      break;
    case TAR_GROUP_OFFENSIVE:
      if ( argument[0] == '\0' )
	{
	  if ( ch->fighting )
	    victim = ch->fighting;
	  else
	    {
	      send_to_char(AT_BLUE, "Cast the spell on whom?\n\r",ch);
	      return;
	    }
	}
      else
	{
	  if ( !( victim = get_char_room( ch, argument ) ) )
	    {
	      send_to_char( AT_BLUE, "They aren't here.\n\r", ch );
	      return;
	    }
	}
      if ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) )
	{
	  send_to_char( AT_BLUE, "You failed.\n\r", ch );
	  return;
	}

      if(!is_pkillable(ch, victim ) ) {
	send_to_char(AT_WHITE, "You failed.\n\r", ch);
	return;
      }

      if (!IS_NPC(victim))
	ch->pkill_timer = 0;
	
      if ( IS_AFFECTED(victim, AFF_PEACE) )
	{
	  send_to_char(AT_WHITE, "A wave of peace overcomes you.\n\r", ch);
	  return;
	}
      if ( IS_AFFECTED2( ch, AFF_SHADOW_PLANE) )
	{
	  send_to_char(AT_WHITE, "You must exit the shadow realm.\n\r", ch);
	  return;
	}
      if ( IS_AFFECTED4( ch, AFF_BURROW) )
	{
	  send_to_char(AT_WHITE, "You must wait until the earth heals you!\n\r", ch);
	  return;
	}
      if ( IS_AFFECTED2( victim, AFF_SHADOW_PLANE) )
	{
	  send_to_char(AT_WHITE, "You can not attack someone who is in the shadow realm.\n\r", ch);
	  return;
	}
      if ( IS_AFFECTED4( victim, AFF_BURROW) )
	{
	  send_to_char(AT_WHITE, "You can not attack someone who is burrowed.\n\r", ch);
	  return;
	}
      if ( IS_AFFECTED( ch, AFF_PEACE) )
	{
	  affect_strip( ch, skill_lookup("aura of peace") );
	  REMOVE_BIT( ch->affected_by, AFF_PEACE );
	}

      if (is_safe(ch, victim ) )
	{
	  send_to_char( AT_BLUE, "You failed.\n\r",ch);
	  return;
	}

      add_gspell( ch, sn, level, (void *) victim );
      break;
    case TAR_GROUP_ALL:
      add_gspell( ch, sn, level, NULL );
      break;
    case TAR_GROUP_OBJ:
      if ( !( obj = get_obj_list( ch, argument, ch->in_room->contents ) ) )
	{
	  send_to_char( AT_WHITE, "You don't see that.\n\r", ch );
	  return;
	}
      add_gspell( ch, sn, level, (void *) obj );
      break;
    case TAR_GROUP_IGNORE:
      add_gspell( ch, sn, level, (void *) argument );
      break;
    }

  return;
}

int gspell_flamesphere( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice(1000, level/5);
  if ( saves_spell( level, victim ) )
    dam /= 2;
  damage( ch, victim, dam, sn );
  return SKPELL_NO_DAMAGE;
}

/* Smite evil is very similar to wrath of god, except that the target
 * must be evil, and, the damage isn't doubled against pc's 
 * (otherwise, pk would be very very short) As is, against a evil
 * with an average amount of shields, it'll do around 4k damage with champs
 * -- Manaux   (note, smite good is almost identical)
 */
int gspell_smite_evil( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  if (!IS_EVIL( victim ) || IS_EVIL(ch) )
    {
      act(AT_WHITE, "$n's burst of holy energy backfires.", ch, 0, 0, TO_ROOM);
      act(AT_WHITE, "Your burst of holy energy backfires.", ch, 0, 0, TO_CHAR);
      damage(ch, ch, number_range(500, 2000), sn);
      update_pos(ch);
      return SKPELL_NO_DAMAGE;
    }
  dam = number_fuzzy( number_fuzzy(level) * number_fuzzy(level ) );
  damage( ch, victim, dam, sn );

  return SKPELL_NO_DAMAGE;
}
int gspell_smite_good( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  if ((!IS_GOOD( victim )) || (IS_GOOD(ch)) )
    {
      act(AT_RED, "$n's burst of unholy energy backfires.", ch, 0, 0, TO_ROOM);
      act(AT_RED, "Your burst of unholy energy backfires.", ch, 0, 0, TO_CHAR);
      damage(ch, ch, number_range(500, 2000), sn);
      update_pos(ch);
      return SKPELL_NO_DAMAGE;
    }
  dam = number_fuzzy( number_fuzzy(level) * number_fuzzy(level ) );
  damage( ch, victim, dam, sn );

  return SKPELL_NO_DAMAGE;
}

int gspell_deadly_poison( int sn, int level, CHAR_DATA * ch, void * vo)
{
  CHAR_DATA * victim = (CHAR_DATA * ) vo;
  AFFECT_DATA af;
  int dur = level / 20 + 1;
  af.type	 = sn;
  af.level	 = level;
  af.duration	 = dur;
  af.location  = APPLY_NONE;
  af.modifier  = 0;
  af.bitvector = AFF_DEADLY_POISON;
  affect_to_char3(victim, &af);
  act(AT_GREEN, "$n's body is ravaged by a deadly poison!", ch, NULL, victim, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

int gspell_volcanic_blast(int sn, int level, CHAR_DATA * ch, void * vo)
{
    CHAR_DATA *victim;
    int     count;
    int     chance;
    int     chance2;


    for (victim = ch->in_room->people; victim; victim=victim->next_in_room )
      {
	if (victim->deleted || is_same_group(victim, ch ))
	  continue;
	count = 1;
	chance = 300;		     


	if (!IS_NPC(victim ) )
	  {
	    while ((chance > 20) && (victim->hit > 0))
	      {
		chance2 = number_range(15,25);
		act(AT_RED, "The Volcanic Ash ERRUPTS under $n.\n\r" , victim, NULL,
		    NULL,TO_ROOM);	
		damage(ch, victim, ch->level*chance2 * 2, sn);
		count = count + 1;
		chance -= number_range(1,200);
	      }
	  }

	else if (IS_NPC(victim ) )
	  {
	    while ((chance > 15) && (victim->hit > 0))
	      {
		chance2 = number_range(15,25);
		act(AT_RED, "The Volcanic Ash ERRUPTS under&w $n.\n\r",victim, NULL, NULL,TO_ROOM);  
		damage(ch, victim, ch->level*chance2* 3, sn);         
		count = count + 1;
		chance -= number_range(1,200);
	      }
	  }
      }

	return SKPELL_NO_DAMAGE;
}   

int gspell_mass_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *vch;

  for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
  {
    if ( !is_same_group( ch, vch ) )
      continue;

    switch ( number_range( 1, 10 ) )
    {
    case 1:
      spell_fireshield( skill_lookup( "fireshield" ), level, vch, vch );
      break;
    case 2:
      spell_shockshield( skill_lookup( "shockshield" ), level, vch, vch );
      break;
    case 3:
      spell_iceshield( skill_lookup( "iceshield" ), level, vch, vch );
      break;
    case 4:
      spell_chaosfield( skill_lookup( "chaos field" ), level, vch, vch );
      break;
    case 5:
      spell_inertial( skill_lookup( "vibrate" ), level, vch, vch );
      break;
    case 6:
      spell_sanctuary( skill_lookup( "sanctuary" ), level, vch, vch );
      break;
    case 7:
      spell_golden_armor( skill_lookup( "golden armor" ), level, vch, vch );
      break;
    case 8:
      spell_ghost_shield( skill_lookup( "ghost shield" ), level, vch, vch );
      break;
    case 9:
      spell_mist( skill_lookup( "mist" ), level, vch, vch );
      break;
    case 10:
      spell_shadow_image( skill_lookup( "shadow image" ), level, vch, vch );
      break;
    }
  }
  return SKPELL_NO_DAMAGE;
}

int gspell_timequake( int sn, int level, CHAR_DATA *ch, void *vo )
{
    do_timequake( (CHAR_DATA *) vo, "" );
    return SKPELL_NO_DAMAGE;
}

int gspell_restoration( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *rch;

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	    if (rch->deleted)
		    continue;
	affect_strip(rch,skill_lookup("poison"));
	affect_strip(rch,skill_lookup("blindness"));
	affect_strip(rch,skill_lookup("curse"));
	affect_strip(rch,skill_lookup("sleep"));
        
	rch->hit    = rch->max_hit;
	rch->move   = rch->max_move;
	rch->poison_level = 0;
	update_pos( rch);
	act(AT_BLUE,"$n has restored you.",ch,NULL, rch,TO_VICT);
    }
    send_to_char(AT_BLUE,"You have restored yourself..\n\r",ch);
    return SKPELL_NO_DAMAGE;
}
