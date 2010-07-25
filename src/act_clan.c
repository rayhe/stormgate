/*****************************************************************************
 * Clan routines for clan funtions.. I hope to move the ones created prior   *
 * to this file in here eventually as well.                                  *
 * -- Altrag Dalosein, Lord of the Dragons                                   *
 *****************************************************************************/
/* $Id: act_clan.c,v 1.6 2005/04/10 16:28:59 tyrion Exp $ */
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


/*
 * Illuminati bestow command, for deity only.
 */
void do_bestow( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  if ( ch->clan != -1 || ch->clev < 5 )
  {
    send_to_char(C_DEFAULT, "Huh?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );

  if ( !( victim = get_char_room( ch, arg ) ) || IS_NPC(victim) )
  {
    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
    return;
  }

  if ( victim->clan != ch->clan )
  {
    send_to_char(C_DEFAULT, "They aren't in your clan!\n\r", ch );
    return;
  }

  if ( !IS_SET( victim->act, PLR_CSKILL ) )
  {
    SET_BIT( victim->act, PLR_CSKILL );
    act(AT_PINK, "$N bestowed with the Transmute skill.", ch, NULL, victim,
	TO_CHAR );
    send_to_char( AT_PINK, "You have been given the Transmute skill.\n\r",
		  victim );
  }
  else
  {
    REMOVE_BIT( victim->act, PLR_CSKILL );
    act(AT_PINK, "Transmute denied from $N.", ch, NULL, victim, TO_CHAR );
    send_to_char( AT_PINK, "You have been denied the Transmute skill.\n\r",
		  victim );
  }
}


/*
 * Illuminati transmute skill, must be given to a member by the deity
 */
void do_transmute( CHAR_DATA *ch, char *argument )
{
	
	send_to_char(C_DEFAULT, "Huh?\n\r", ch );
    return;
/*
  OBJ_DATA *obj;
  int chance;

  if ( ch->clan != -1 || !IS_SET(ch->act, PLR_CSKILL)
      || IS_NPC(ch) )
  {
    send_to_char(C_DEFAULT, "Huh?\n\r", ch );
    return;
  }

  if ( !( obj = get_obj_carry( ch, argument ) ) )
  {
    send_to_char(C_DEFAULT, "You do not have that item.\n\r", ch );
    return;
  }

  if ( obj->level > 0 )
    chance = (ch->level * 75) / obj->level;
  else
    chance = 85;

  if ( number_percent( ) < chance )
  {
    obj->extra_flags |= ITEM_NO_DAMAGE;
    act(AT_PINK, "$p transmuted.", ch, obj, NULL, TO_CHAR );
    return;
  }

  act(AT_PINK, "You failed!  $p exploded in your face!", ch, obj, NULL,
      TO_CHAR);
  extract_obj(obj);
  return;
  */
}


/*
 * Vendetta Doomshield skill
 */
void do_doomshield( CHAR_DATA *ch, char *argument )
{

    send_to_char( C_DEFAULT, "Huh?\n\r", ch );
    return;

	/*
	AFFECT_DATA af;
  if ( IS_AFFECTED2( ch, AFF_DOOMSHIELD ) )
    return;

  af.type = gsn_doomshield;
  af.level = ch->level;
  af.duration = 5;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_DOOMSHIELD;
  affect_to_char2(ch, &af);
  act( AT_PURPLE, "$n is surrounded with an Aura of Insanity!", ch, NULL,
       NULL, TO_ROOM );
  send_to_char(AT_PURPLE, "You are surrounded with the an Aura of Insanity.\n\r",
	       ch );
  return;
  */
}

void do_image( CHAR_DATA *ch, char *argument )
{
	
  send_to_char(C_DEFAULT, "Huh?\n\r", ch );
  return;

  /*
  AFFECT_DATA af;

  if ( ch->clan != -1 )
  {
    send_to_char( C_DEFAULT, "Huh?\n\r",ch);
    return;
  }
  if ( ch->ctimer || ch->combat_timer )
  {
    send_to_char(AT_BLUE, "You can't right now.\n\r",ch);
    return;
  }

  if ( IS_AFFECTED2(ch, AFF_IMAGE) )
    return;

  af.location = APPLY_NONE;
  af.modifier = number_range( 1, 5 );
  af.duration = number_fuzzy(3);
  af.bitvector = AFF_IMAGE;
  af.type = gsn_image;
  af.level = ch->level;
  ch->ctimer = af.duration + 10;
  affect_to_char2( ch, &af );
  send_to_char(AT_CYAN, "You are surrounded by images of the Talisman.\n\r",
	       ch);
  act(AT_CYAN, "$n suddenly splits into many images.",ch,NULL,NULL,TO_ROOM);
  return;
  */
}

void do_utopian_healing( CHAR_DATA *ch, char *argument )
{
    send_to_char(C_DEFAULT, "Huh?\n\r", ch );
    return;

/*
	char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument( argument, arg );

  if ( !( victim = get_char_room( ch, arg ) ) || IS_NPC(victim) )
  {
    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
    return;
  }
  if ( ch->fighting || ch->ctimer )
  {
    send_to_char(C_DEFAULT, "You failed.\n\r",ch);
    return;
  }

  if ( ch == victim )
  {
    int mana;

    // Refund mana lost by casting.  Make it seem like a new target type.
    mana = 50;
    if (( ch->class == 9 )||( ch->class == 11))
      mana /= 4;
    ch->mana += mana;
    send_to_char(AT_BLUE, "You cannot cast this spell on yourself.\n\r", ch );
    return;
  }

  if ( victim->fighting != NULL )
  {
    send_to_char(AT_WHITE, "That person is fighting, you can't do that.\n\r", ch );
    return;
  }

  if ( victim->hit >= victim->max_hit )
  {
    act(AT_BLUE, "You heal $N.", ch, NULL, victim, TO_CHAR );
    return;
  }

  ch->ctimer = 5;

  victim->hit += (victim->max_hit - victim->hit);

  act(AT_BLUE, "You use your Utopian will to heal $N.", ch, NULL, victim, TO_CHAR);
  act(AT_BLUE, "$n uses $s Utopian will to heal you.", ch, NULL, victim, TO_VICT);
  act(AT_BLUE, "$n uses $s Utopian will to heal $N.", ch, NULL, victim, TO_NOTVICT);
  return;
*/
}
/* END */

void do_clanview( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *from_room;
    char	target_name[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if (IS_NPC( ch ) )
	return;

    if (ch->clan < 1 )
    {
	send_to_char( C_DEFAULT, "Huh?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_world( ch, argument ) ) || IS_NPC( victim ) )
    {
	send_to_char(C_DEFAULT, "No such person exists.\n\r", ch );
	return;
    }

    if (victim->clan != ch->clan)
    {
	send_to_char(AT_BLUE, "They aren't in your clan!\n\r", ch );
	return;
    }

    one_argument( argument, target_name );

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PRESENT)
	&& (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
	|| IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
	send_to_char(AT_BLUE, "You cannot!!!!!!!!\n\r", ch );
	return;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_FUTURE )
	&& (IS_SET( victim->in_room->area->area_flags, AREA_PRESENT )
	|| IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
	send_to_char(AT_BLUE, "You cannot!!!!!!!!\n\r", ch );
	return;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PAST )
	&& (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
	|| IS_SET( victim->in_room->area->area_flags, AREA_PRESENT ) ) )
    {
	send_to_char( AT_BLUE, "You cannot!!!!!!!!\n\r", ch );
	return;
    }

    from_room = ch->in_room;
    if ( ch != victim )
    {
	char_from_room( ch );
	char_to_room( ch, victim->in_room );
    }
    do_look( ch, "auto" );
    if( ch != victim )
    {
	char_from_room( ch );
	char_to_room( ch, from_room );
    }
    return;
}

