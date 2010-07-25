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

/*$Id: fight2.c,v 1.29 2005/03/29 16:03:41 ahsile Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

extern void disarm        args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
extern void check_killer  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

extern char* target_name;

int skill_punch( int sn, int level, CHAR_DATA *ch, void *vo )
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

    WAIT_STATE( ch, skill_table[sn].beats/2  );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 ) )
    {
	dam = number_range(ch->level, ch->level * 5); //damage( ch, victim, number_range( ch->level, ch->level*5 ), sn );
    }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, sn );

    return dam;
}

int skill_feed( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam = 0;

    if ( ( ch->class != 9 )&&( ch->class != 11 )&&(ch->multied != 9 )&&( ch->multied != 11 ) )
       {
         send_to_char( AT_WHITE, "You may not feed on the living.\n\r", ch );
         return SKPELL_MISSED;
       }

 
    if ( !ch->fighting )
    {
	send_to_char(AT_WHITE, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !check_blind( ch ) )
        return SKPELL_MISSED;

    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 ) )
       {
	  int        amnt;

	amnt = ( number_range( 10, 25 )+ ch->level / 2 );
	if (( ch->class == CLASS_VAMPIRE || ch->multied == CLASS_VAMPIRE) &&
	    (time_info.hour < 6 || time_info.hour >= 19))
		amnt = amnt*2;
	if ( ( ch->bp + amnt ) > ch->max_bp )
	   ch->bp = ch->max_bp;
	else
	   ch->bp += amnt;
		dam = number_range( ch->level / 10, ch->level * 2); //damage( ch, victim, number_range( ch->level / 10, ch->level * 2 ), gsn_feed );
       }
    else
		dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, gsn_feed );

    return dam;
}



int skill_disarm( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
    int        percent;


    if ( IS_NPC( ch ) )
    {
	return SKPELL_MISSED;
    }

    if ( ( !get_eq_char( ch, WEAR_WIELD ) ) && ( !get_eq_char( ch, WEAR_WIELD_2 ) && ch->race != 13 ) )
    {
	send_to_char(C_DEFAULT, "You must wield a weapon to disarm.\n\r", ch );
	return SKPELL_MISSED;
    }

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

    if ( victim->fighting != ch && ch->fighting != victim )
    {
	act(C_DEFAULT, "$E is not fighting you!", ch, NULL, victim, TO_CHAR );
	return SKPELL_MISSED;
    }

    if ( !( obj = get_eq_char( victim, WEAR_WIELD ) ) )
    {
	if ( !( obj = get_eq_char( victim, WEAR_WIELD_2 ) ) )
	{
	  send_to_char(C_DEFAULT, "Your opponent is not wielding a weapon.\n\r", ch );
	  return SKPELL_MISSED;
        }
    }

    if ( !IS_NPC( victim ) &&  victim->race == 0 && victim->class == 7 )
       {
	 send_to_char( C_DEFAULT, "You failed.\n\r", ch );
	 return SKPELL_MISSED;
       }

    if ( number_percent( ) < ( victim->antidisarm / 3 ) )
       {
         send_to_char( C_DEFAULT, "You failed.\n\r", ch );
         return SKPELL_MISSED;
       }

    WAIT_STATE( ch, skill_table[sn].beats );
    percent = number_percent( ) + victim->level - ch->level;


    if ( ( IS_NPC( ch ) && percent < 20 ) || percent < ( ch->pcdata->learned[sn] / 10 )  * 2 / 3 )
    {
	disarm( ch, victim );
    }
    else
	send_to_char(C_DEFAULT, "You failed.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    rch = get_char( ch );

    if ( !authorized( rch, "slay" ) )
        return;

    send_to_char(C_DEFAULT, "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *rch;
    char       arg [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "slay" ) )
        return;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Slay whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	return;
    }

    if ( ( !IS_NPC( victim ) && victim->level >= ch->level && victim != ch ) ||
	    (IS_NPC( ch ) && !IS_NPC( victim )) )
    {
	send_to_char(C_DEFAULT, "You failed.\n\r", ch );
	return;
    }

    act(C_DEFAULT, "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
    act(C_DEFAULT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
    act(C_DEFAULT, "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    sprintf( log_buf, "%s slays %s at %d.\n\r", ch->name, victim->name,
             victim->in_room->vnum );
    log_string( log_buf, CHANNEL_LOG, ch->level - 1 );
    sprintf( log_buf, "%s unleashes the wrath of the gods, and %s has been slain!", ch->name, victim->name );
    log_string( log_buf, CHANNEL_INFO, -1 );
    raw_kill( ch, victim );
    return;
}

/* This code is for PC's who polymorph into dragons.
 * Yeah I know this is specialized code, but this is fun.  :)
 * Breathe on friend and enemy alike.
 * -Kahn
 */

void pc_breathe( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *victim_next;
    int        sn;

    send_to_char(AT_RED, "You feel the urge to burp!\n\r", ch );
    act(AT_RED, "$n belches!", ch, NULL, NULL, TO_ROOM );
    for ( victim = ch->in_room->people; victim; victim = victim_next )
    {
        victim_next = victim->next_in_room;
        if ( victim->deleted )
            continue;

        if ( victim == ch )
            continue;

        sn = skill_lookup( "fire breath" );
        (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim );
    }

    return;
}

/* This code is for PC's who polymorph into harpies.
 * Yeah I know this is specialized code, but this is fun.  :)
 * Scream into the ears of enemy and friend alike.
 * -Kahn
 */

void pc_screech( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *victim_next;
    int        sn;

    send_to_char(AT_WHITE, "You feel the urge to scream!\n\r", ch );
    interpret( ch, "scream" );
    for ( victim = ch->in_room->people; victim; victim = victim_next )
    {
        victim_next = victim->next_in_room;
        if ( victim->deleted )
            continue;

        if ( victim == ch )
            continue;

        act(AT_BLOOD, "Your ears pop from $n's scream.  Ouch!", ch, NULL, victim,
            TO_VICT );
        sn = skill_lookup( "agitation" );
        (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim );
    }

    return;
}
void pc_spit( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *victim_next;
    int        sn;

    send_to_char(AT_GREEN, "You feel the urge to spit!\n\r", ch );
    act(AT_GREEN, "$n spews vitriol!", ch, NULL, NULL, TO_ROOM );
    for ( victim = ch->in_room->people; victim; victim = victim_next )
    {
        victim_next = victim->next_in_room;
        if ( victim->deleted )
            continue;

        if ( victim == ch )
            continue;

        act(AT_BLOOD, "You are splattered with $n's vitriol.  Ouch!", ch, NULL, victim,
            TO_VICT );
        sn = skill_lookup( "poison" );
        (*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim );

        damage( ch, victim, number_range( 1, ch->level ), skill_lookup("poison weapon") );
    }

    return;
}

bool check_race_special( CHAR_DATA *ch )
{
    if ( ch->race == race_lookup( "Dragon" ) )
    {
        if ( number_percent( ) < ch->level )
        {
            pc_breathe( ch );
            return TRUE;
        }
    }

    if ( ch->race == race_lookup( "Harpy" ) )
    {
        if ( number_percent( ) < ch->level )
        {
            pc_screech( ch );
            return TRUE;
        }
    }

    if ( ch->race == race_lookup( "Arachnid" )
        || ch->race == race_lookup( "Snake" ) )
    {
        if ( number_percent( ) < ch->level )
        {
            pc_spit( ch );
            return TRUE;
        }
    }

    return FALSE;
}


void use_magical_item( CHAR_DATA *ch )
 {
    OBJ_DATA *obj;
    OBJ_DATA *cobj     = NULL;
    int       number   = 0;
    char      buf[ MAX_INPUT_LENGTH ];

    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
        if ( (   obj->item_type == ITEM_SCROLL
              || obj->item_type == ITEM_WAND
              || obj->item_type == ITEM_STAFF
	      || obj->item_type == ITEM_GUN
              || obj->item_type == ITEM_PILL )
            && number_range( 0, number ) == 0 )
        {
            cobj = obj;
            number++;
        }
    }

    if ( !cobj )
        return;

    switch( cobj->item_type )
    {
        case ITEM_SCROLL: do_recite( ch, "scroll" );
                          break;
        case ITEM_WAND:   if ( cobj->wear_loc == WEAR_HOLD )
                              do_zap( ch, "" );
                          break;
	case ITEM_GUN: if ( cobj->wear_loc == WEAR_FIREARM )
			      do_fire( ch, "" );
			  break;
        case ITEM_STAFF:  if ( cobj->wear_loc == WEAR_HOLD )
                              do_brandish( ch, "" );
                          break;
        case ITEM_POTION: do_quaff( ch, "potion" );
                          break;
        case ITEM_PILL:   sprintf( buf, "%s", cobj->name );
                          do_eat( ch, buf );
                          break;
    }
    return;

}

int per_type( CHAR_DATA *ch, OBJ_DATA *Obj )
{
  switch ( Obj->item_type )
  {
    case ITEM_WEAPON:
      return number_range( 5, ch->level + 5 );
    case ITEM_STAFF:
      return number_range( 3, ch->level + 3 );
    case ITEM_GUN:
      return number_range( 2, ch->level + 2 );
    case ITEM_WAND:
      return number_range( 2, ch->level + 2 );
    default:
      return number_range( 1, ch->level );
  }
  return number_range( 1, ch->level );
}

void do_throw( CHAR_DATA *ch, char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *to_room;
  ROOM_INDEX_DATA *in_room;
  OBJ_DATA *Obj;
  EXIT_DATA *pexit;
  int dir = 0;
  int dist = 0;
  int MAX_DIST = 2;
  extern char *dir_noun [];

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  if ( arg1[0] == '\0' )
  {
    send_to_char( C_DEFAULT, "Throw what item?\n\r", ch );
    return;
  }

  if ( ( Obj = get_obj_wear( ch, arg1 ) ) == NULL )
  {
    send_to_char( C_DEFAULT,
		 "You are not wearing, wielding, or holding that item.\n\r",
		 ch );
    return;
  }

  if ( Obj->wear_loc != WEAR_WIELD && Obj->wear_loc != WEAR_WIELD_2 &&
       Obj->wear_loc != WEAR_HOLD )
  {
    send_to_char( C_DEFAULT,
		 "You are not wielding or holding that item.\n\r", ch );
    return;
  }

  if ( IS_SET( Obj->extra_flags, ITEM_NOREMOVE ) || IS_SET( Obj->extra_flags,ITEM_NODROP ) /*|| IS_SET( Obj->extra_flags2,ITEM_QUEST) */)
  {
    send_to_char( C_DEFAULT, "You can't let go of it!\n\r", ch );
    return;
  }

  in_room = ch->in_room;
  to_room = ch->in_room;

  if ( ( victim = ch->fighting ) == NULL )
  {
    if ( arg2[0] == '\0' )
    {
      send_to_char( C_DEFAULT, "Throw it at who?\n\r", ch );
      return;
    }

    if ( arg3[0] == '\0' )
    {
      if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
      {
	send_to_char( C_DEFAULT, "They aren't here.\n\r", ch );
	return;
      }
    }
    else
    {
      if ( get_curr_str( ch ) >= 20 )
      {
        MAX_DIST = 3;
        if ( get_curr_str( ch ) == 25 )
          MAX_DIST = 4;
      }

      for ( dir = 0; dir < 6; dir++ )
	if ( arg2[0] == dir_name[dir][0] && !str_prefix( arg2,
							 dir_name[dir] ) )
	  break;

      if ( dir == 6 )
      {
	send_to_char( C_DEFAULT, "Throw in which direction?\n\r", ch );
	return;
      }

      if ( ( pexit = to_room->exit[dir] ) == NULL ||
	   ( to_room = pexit->to_room ) == NULL )
      {
	send_to_char( C_DEFAULT, "You cannot throw in that direction.\n\r",
		     ch );
	return;
      }

      if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
	send_to_char( C_DEFAULT, "You cannot throw through a door.\n\r", ch );
	return;
      }

      for ( dist = 1; dist <= MAX_DIST; dist++ )
      {
	char_from_room( ch );
	char_to_room( ch, to_room );
	if ( ( victim = get_char_room( ch, arg3 ) ) != NULL )
	  break;

	if ( ( pexit = to_room->exit[dir] ) == NULL ||
	     ( to_room = pexit->to_room ) == NULL ||
	       IS_SET( pexit->exit_info, EX_CLOSED ) )
	{
	  sprintf( buf, "A $p flys in from $T and hits the %s wall.",
		   dir_name[dir] );
	  act( AT_WHITE, buf, ch, Obj, dir_noun[rev_dir[dir]], TO_ROOM );
	  sprintf( buf, "You throw your $p %d room%s $T, where it hits a wall.",
		   dist, dist > 1 ? "s" : "" );
	  act( AT_WHITE, buf, ch, Obj, dir_name[dir], TO_CHAR );
	  char_from_room( ch );
	  char_to_room( ch, in_room );
	  oprog_throw_trigger( Obj, ch );
	  unequip_char( ch, Obj );
	  obj_from_char( Obj );
	  obj_to_room( Obj, to_room );
	  return;
	}
      }

      if ( victim == NULL )
      {
	act( AT_WHITE,
	    "A $p flies in from $T and falls harmlessly to the ground.",
	    ch, Obj, dir_noun[rev_dir[dir]], TO_ROOM );
	sprintf( buf,
		"Your $p falls harmlessly to the ground %d room%s $T of here.",
		dist, dist > 1 ? "s" : "" );
	act( AT_WHITE, buf, ch, Obj, dir_name[dir], TO_CHAR );
	char_from_room( ch );
	char_to_room( ch, in_room );
	oprog_throw_trigger( Obj, ch );
	unequip_char( ch, Obj );
	obj_from_char( Obj );
	obj_to_room( Obj, to_room );
	return;
      }
    }
    if ( dist > 0 )
    {
      char_from_room( ch );
      char_to_room( ch, in_room );
      act( AT_WHITE, "A $p flys in from $T and hits $n!", victim, Obj,
	  dir_noun[rev_dir[dir]], TO_NOTVICT );
      act( AT_WHITE, "A $p flys in from $T and hits you!", victim, Obj,
	  dir_noun[rev_dir[dir]], TO_CHAR );
      sprintf( buf, "Your $p flew %d rooms %s and hit $N!", dist,
	      dir_name[dir] );
      act( AT_WHITE, buf, ch, Obj, victim, TO_CHAR );
      oprog_throw_trigger( Obj, ch );
      unequip_char( ch, Obj );
      obj_from_char( Obj );
      obj_to_room( Obj, to_room );
      damage( ch, victim, per_type( ch, Obj ), skill_lookup("throw") );
      if ( IS_NPC( victim ) )
      {
         if ( victim->level > 3 )
             victim->hunting = ch;
      }
      return;
    }
  }
  unequip_char( ch, Obj );
  obj_from_char( Obj );
  obj_to_room( Obj, to_room );
  act( AT_WHITE, "$n threw a $p at $N!", ch, Obj, victim, TO_ROOM );
  act( AT_WHITE, "You throw your $p at $N.", ch, Obj, victim, TO_CHAR );
  oprog_throw_trigger( Obj, ch );
  damage( ch, victim, per_type( ch, Obj ), skill_lookup("throw") );
  multi_hit( victim, ch, TYPE_UNDEFINED );
  return;
}

OBJ_DATA* find_arrows( CHAR_DATA* ch)
{
	return find_ammo( ch, ITEM_ARROW );
}

OBJ_DATA* find_bolts( CHAR_DATA* ch)
{
	return find_ammo( ch, ITEM_BOLT );
}

OBJ_DATA* find_bullets( CHAR_DATA* ch)
{
	return find_ammo( ch, ITEM_BULLET );
}

OBJ_DATA* find_ammo( CHAR_DATA* ch, int type)
{
	OBJ_DATA* obj;

	for (obj = ch->carrying; obj; obj = obj->next_content )
	{
		if (obj->item_type == type && !obj->deleted && obj->level <= ch->level)
		{
			return obj;
		}
	}

	return NULL;
}

bool find_missle_target( int sn, CHAR_DATA *ch, char *argument, CHAR_DATA** victim, ROOM_INDEX_DATA** to_room, int* distance )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf [MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *in_room;
  EXIT_DATA *pexit;
  int dir = 0;
  int dist = 0;
  int MAX_DIST = 2;
  char dir_buf[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  in_room = ch->in_room;
  *to_room = ch->in_room;

  if ( ( *victim = ch->fighting ) == NULL )
  {
    if ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) )
    {
      send_to_char( AT_BLUE, "You failed.\n\r", ch );
      return SHOOT_UNKNOWN;
    }
    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
      send_to_char( AT_BLUE, "You are in a safe room!\n\r", ch );
      return SHOOT_UNKNOWN;
    }


    if ( arg1[0] == '\0' )
    {
      send_to_char( C_DEFAULT, "Shoot it at who?\n\r", ch );
      *distance = 0;
      *victim = NULL;
      return SHOOT_UNKNOWN;
    }

    if ( arg2[0] == '\0' )
    {
      if ( ( *victim = get_char_room( ch, arg1 ) ) == NULL )
      {
	send_to_char( C_DEFAULT, "They aren't here.\n\r", ch );
        *distance = 0;
	*victim = NULL;
	return SHOOT_UNKNOWN;
      }
    }
    else
    {
      if ( get_curr_str( ch ) >= 20 )
      {
        MAX_DIST = 3;
        if ( get_curr_str( ch ) >= 35 )
          MAX_DIST = 4;
	if ( get_curr_str( ch ) == 50 )
	  MAX_DIST = 5;
      }

      for ( dir = 0; dir < 6; dir++ )
      {
	if ( arg1[0] == dir_name[dir][0] && !str_prefix( arg1, dir_name[dir] ) )
	{
	  break;
	}
      }

      if ( dir == 6 )
      {
	send_to_char( C_DEFAULT, "Shoot in which direction?\n\r", ch );
	*distance = 0;
	return SHOOT_UNKNOWN;
      }

 		switch (dir)
		{
			case DIR_NORTH:
				strcpy(dir_buf, "south");
				break;
			case DIR_SOUTH:
				strcpy(dir_buf, "north");
				break;
			case DIR_EAST:
				strcpy(dir_buf, "west");
				break;
			case DIR_WEST:
				strcpy(dir_buf, "east");
				break;
			case DIR_UP:
				strcpy(dir_buf, "down");
				break;
			case DIR_DOWN:
				strcpy(dir_buf, "up");
				break;
		}


      if ( ( pexit = (*to_room)->exit[dir] ) == NULL ||
	   ( *to_room = pexit->to_room ) == NULL )
      {
	send_to_char( C_DEFAULT, "You cannot shoot in that direction.\n\r", ch );
	*distance = 0;
	*to_room = NULL;
	*victim = NULL;
	return SHOOT_UNKNOWN;
      }

      if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
	send_to_char( C_DEFAULT, "You cannot shoot through a door.\n\r", ch );
	*distance = 0;
	*victim = NULL;
	*to_room = NULL;
	return SHOOT_UNKNOWN;
      }

      for ( dist = 1; dist <= MAX_DIST; dist++ )
      {
	char_from_room( ch );
	char_to_room( ch, *to_room );
	if ( ( *victim = get_char_room( ch, arg2 ) ) != NULL )
	  break;

	 if ( IS_SET( (*to_room)->room_flags, ROOM_NO_OFFENSIVE ) || IS_SET( (*to_room)->room_flags, ROOM_SAFE ) )
         {
		char_from_room( ch );
		char_to_room( ch, in_room);
		*distance = dist - 1;		
		sprintf(buf, "A strange force diverts your %s to the ground %d rooms away!!\n\r", skill_table[sn].name, dist);
		send_to_char(AT_GREEN, buf, ch );
		sprintf(buf, "$n's %s flies in from the %s and is forced to the ground!\n\r", skill_table[sn].name, dir_buf );
		act( AT_GREEN, buf, ch, NULL, NULL, TO_ROOM );
                return SHOOT_UNKNOWN;
         }

	if ( ( pexit = (*to_room)->exit[dir] ) == NULL ||
	     ( *to_room = pexit->to_room ) == NULL ||
	       IS_SET( pexit->exit_info, EX_CLOSED ) )
	{
	  char_from_room( ch );
	  char_to_room( ch, in_room );

	  *distance = dist - 1;
	  *victim    = NULL;
	  return SHOOT_DOOR;
	}

	if (dist != MAX_DIST)
	{
		sprintf(buf, "$n's %s flies through the room from the %s!\n\r", skill_table[sn].name, dir_buf );
		act( AT_GREEN, buf, ch, NULL, NULL, TO_ROOM );
	} else
	{
		sprintf(buf, "$n's %s flies in from the %s and lands on the ground!\n\r", skill_table[sn].name, dir_buf );
		act( AT_GREEN, buf, ch, NULL, NULL, TO_ROOM );
	}
      }

      if ( *victim == NULL )
      {
		*to_room = ch->in_room;
		char_from_room( ch );
		char_to_room( ch, in_room );
		*distance = dist - 1;
		*victim    = NULL; 
		return SHOOT_DIST;
      }
    }

    if ( dist > 0 )
    {

      /* Same message no matter what. At that distance, would you really know? */
      if( !is_pkillable(ch, *victim ) || IS_AFFECTED( *victim, AFF_PEACE) 
	  || IS_AFFECTED2( *victim, AFF_SHADOW_PLANE) || IS_AFFECTED4( *victim, AFF_BURROW ) )
      {
	sprintf(buf, "Your %s hits a protective barrier around $N %d rooms away!\n\r", skill_table[sn].name, dist);
	act( AT_GREEN, buf, ch, NULL, *victim, TO_CHAR );
	sprintf( buf, "$n's %s flies in from the %s at YOU but disappears in a flash of blue light!\n\r", skill_table[sn].name, dir_buf);
	act( AT_GREEN, buf, ch, NULL, *victim, TO_VICT );
	sprintf( buf, "$n's %s flies in from the %s at $N but disappears in a flash of blue light!\n\r", skill_table[sn].name, dir_buf);
	act( AT_GREEN, buf, ch, NULL, *victim, TO_NOTVICT );
	
        char_from_room( ch );
        char_to_room( ch, in_room );
        *distance = dist - 1;
        return SHOOT_UNKNOWN;
      }

      if ( IS_NPC( *victim ) && IS_SET( (*victim)->act, ACT_SENTINEL ) )
      {
	 sprintf(buf, "$N guards against your %s!\n\r", skill_table[sn].name );
	 act( AT_GREEN, buf, ch, NULL, *victim, TO_CHAR );
	 sprintf(buf, "$n's %s comes in from the %s but is blocked by $N!\n\r", skill_table[sn].name, dir_buf);
	 act( AT_GREEN, buf, ch, NULL, *victim, TO_ROOM );
		
	 char_from_room( ch );
	 char_to_room( ch, in_room ); 
	 return SHOOT_UNKNOWN;
      }


      if ( IS_NPC( *victim ) )
      {
         if ( (*victim)->level > 3 )
             (*victim)->hunting = ch;
      }

      char_from_room( ch );
      char_to_room( ch, in_room );
      *distance = dist;
      return SHOOT_FOUND;
    }

  }

      if( !is_pkillable(ch, *victim ) ) 
      {
	send_to_char( AT_WHITE, "You cannot.\n\r", ch );
	return SHOOT_UNKNOWN;
      }

      if ( IS_AFFECTED(*victim, AFF_PEACE) )
      {
        send_to_char(AT_WHITE, "A wave of peace overcomes you.\n\r", ch);
	return SHOOT_UNKNOWN;
      }

      if ( IS_AFFECTED2( *victim, AFF_SHADOW_PLANE) )
      {
        send_to_char(AT_WHITE, "You can not attack someone who is in the shadow realm.\n\r", ch);
	return SHOOT_UNKNOWN;
      }

      if ( IS_AFFECTED4( *victim, AFF_BURROW) )
      {
        send_to_char(AT_WHITE, "You can not attack someone who is burrowed.\n\r", ch);
	return SHOOT_UNKNOWN;
      }

  *distance = 0;
  return SHOOT_HERE;
}

/*
void do_track( CHAR_DATA *ch, char *argument )
{
   ROOM_INDEX_DATA *room[30];
   ROOM_INDEX_DATA *to_room;
   ROOM_INDEX_DATA *in_room;
   EXIT_DATA *pexit[30];
   char arg[MAX_STRING_LENGTH];
   int vnums[30];
   int dist[30];
   int sdir[30];
   int dir[6];
   int nsdir;

   nsdir = 1;

   argument = one_argument( argument, arg );

   if ( is_number( arg ) || arg[0] == \0' )
   {
      send_to_char( AT_WHITE, "Track what?\n\r", ch );
      return;
   }

   in_room = ch->in_room;
   to_room = ch->in_room;

   for ( dir = 0; dir != -1; dir++ )
   {
      if ( !( pexit[nsdir] = ch->in_room->exit[dir] )
        || ( !( to_room = pexit[nsdir]->to_room ) ) )
      {
        if ( dir == 6 )
          dir = -1;
        continue;

        char_from_room( ch );
        char_to_room( ch, to_room );

        if ( get_char_room( ch, arg ) )
           break;


      }

   }

   if ( dir != -1 )
   {
     sprintf( log_buf, "You sense the trail of %s to the %s.\n\r",
          arg, dir_name[dir] );
     send_to_char( AT_WHITE, log_buf, ch );
     char_from_room( ch );
     char_to_room( ch, in_room );
   }

   if ( dir == -1 )
   {
     sprintf( log_buf, "You can't sense any %s from here.\n\r", arg );
     send_to_char( AT_WHITE, log_buf, ch );
     return;
   }

   return;
}
*/

int skill_drain_life( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim;
  int	     dam = 0;


  if( !(ch->fighting) )
  {
    send_to_char(C_DEFAULT, "You are not fighting!\n\r", ch);
    return SKPELL_MISSED;
  }

  if(target_name[0] != '\0')
  {
    if(!(victim = get_char_room(ch, target_name)))
    {
      send_to_char(C_DEFAULT, "They aren't here.\n\r", ch);
      return SKPELL_MISSED;
    }
  }
  else
  {
    if(!(victim = ch->fighting))
    {
      send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch);
      return SKPELL_MISSED;
    }
  }

  if(is_safe(ch, victim))
    return SKPELL_MISSED;

  WAIT_STATE(ch, skill_table[sn].beats);
  /* 3xlevel + 1dlevel*/
  if(IS_NPC(ch) || number_percent() < ( ch->pcdata->learned[sn] / 10 ) )
  {
    dam = ch->level + number_range(ch->level * 2, ch->level * 4);
    //damage(ch, victim, dam, gsn_drain_life);
    ch->hit = UMIN(ch->hit + dam/2, ch->max_hit);
  }
  else
  {
    send_to_char(C_DEFAULT, "You failed.", ch);
    dam = SKPELL_ZERO_DAMAGE; //damage(ch, victim, 0, gsn_drain_life);
  }

  return dam;
}

int skill_mental_drain( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim;
  int	     dam;
  int	     drain;
  int        dmana;
  int	     dmove;

  if(target_name[0] != '\0')
  {
    if(!(victim = get_char_room(ch, target_name)))
    {
      send_to_char(C_DEFAULT, "They aren't here.\n\r", ch);
      return SKPELL_MISSED;
    }
  }
  else
  {
    if(!(victim = ch->fighting))
    {
      send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch);
      return SKPELL_MISSED;
    }
  }

  if(is_safe(ch, victim))
    return SKPELL_MISSED;

  WAIT_STATE(ch, skill_table[sn].beats);
  /* 3xlevel + 1d100*/
  if(IS_NPC(ch) || number_percent() < ( ch->pcdata->learned[sn] / 10 ) )
  {
    dam = number_range(ch->level / 2, ch->level);
    dmove = dmana = drain = ch->level + number_range(ch->level / 2, ch->level);
    //damage(ch, victim, dam, gsn_mental_drain);
    if ( !IS_NPC(victim) )
    { /* New drain amounts by Manaux */
      dmana = UMIN(victim->mana - victim->mana/3 , 500 );
      dmove = drain = UMAX(victim->move - drain, 0 );
      victim->mana -= dmana;
      victim->mana = URANGE(0, victim->mana , victim->max_mana);
      victim->move -= dmove;
    }
    ch->mana = UMIN(ch->mana + dmana / 2, ch->max_mana );
    ch->move = UMIN(ch->move + dmove / 2, ch->max_move );
  }
  else
  {
	victim->mana = UMAX(victim->mana - victim->mana/2 -40, 0);
    send_to_char(C_DEFAULT, "Ok.", ch);
    dam = SKPELL_ZERO_DAMAGE; //damage(ch, victim, 0, gsn_mental_drain);
  }

  return dam;
}

int skill_stun( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim;

  if ( !(victim = ch->fighting) )
  {
    send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r",ch);
    return SKPELL_MISSED;
  }

  if ( victim->position == POS_STUNNED || IS_STUNNED( ch, STUN_TO_STUN ) )
    return SKPELL_MISSED;

  if ( ( IS_NPC(ch) || number_percent() < ( ch->pcdata->learned[sn] / 10 ) ) &&
      number_percent() < (ch->level * 75) / victim->level )
  {
    STUN_CHAR( ch, 6, STUN_TO_STUN );
    STUN_CHAR( victim, 1, STUN_TOTAL );
    victim->position = POS_STUNNED;
    act( AT_WHITE, "You stun $N!", ch, NULL, victim, TO_CHAR );
    act( AT_WHITE, "$n stuns $N!", ch, NULL, victim, TO_NOTVICT );
    act( AT_WHITE, "$n stuns you!", ch, NULL, victim, TO_VICT );
    return SKPELL_NO_DAMAGE;
  }

  send_to_char(C_DEFAULT, "You failed.\n\r", ch );
  return SKPELL_MISSED;
}

int skill_trip( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim;
  AFFECT_DATA *paf;

  if ( !(victim = ch->fighting) )
  {
    send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r",ch);
    return SKPELL_MISSED;
  }

  for ( paf = victim->affected; paf; paf = paf->next ) {
      if ( paf->deleted )
          continue;
      if ( paf->bitvector == AFF_FLYING ) {
	send_to_char(AT_WHITE, "You can't, your opponent is flying.\n\r", ch);
        return SKPELL_MISSED;
      }
  }

  if ( victim->position == POS_STUNNED || IS_STUNNED( ch, STUN_TO_STUN ) )
    return SKPELL_MISSED;

  if ( ( IS_NPC(ch) || number_percent() < ( ch->pcdata->learned[sn] / 10 ) ) &&
      number_percent() < (ch->level * 75) / victim->level )
  {
    STUN_CHAR(ch, 4, STUN_TO_STUN);
    STUN_CHAR(victim, 1, STUN_TOTAL);
    victim->position = POS_STUNNED;
    act(AT_WHITE, "You trip $N!", ch, NULL, victim, TO_CHAR);
    act(AT_WHITE, "$n trips $N!", ch, NULL, victim, TO_NOTVICT);
    act(AT_WHITE, "$n trips you and you go down!", ch, NULL, victim, TO_VICT);
    return SKPELL_NO_DAMAGE;
  }
  send_to_char(C_DEFAULT, "You failed.\n\r", ch );
  return SKPELL_MISSED;
}

int skill_strangle( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim;

  if ( !(victim = ch->fighting) )
  {
    send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r",ch);
    return SKPELL_MISSED;
  }

  if ( victim->position == POS_STUNNED || IS_STUNNED( ch, STUN_TO_STUN ) )
    return SKPELL_MISSED;

  if ( ( IS_NPC(ch) || number_percent() < ( ch->pcdata->learned[sn] / 10 ) ) &&
      number_percent() < (ch->level * 75) / victim->level )
  {
    STUN_CHAR( ch, 4, STUN_TO_STUN );
    STUN_CHAR( victim, 1, STUN_TOTAL );
    victim->position = POS_STUNNED;
    act( AT_WHITE, "You strangle $N!", ch, NULL, victim, TO_CHAR );
    act( AT_WHITE, "$n strangles $N!", ch, NULL, victim, TO_NOTVICT );
    act( AT_WHITE, "$n strangles you!", ch, NULL, victim, TO_VICT );
    WAIT_STATE( ch, skill_table[sn].beats );
    return SKPELL_NO_DAMAGE;
  }

  send_to_char(C_DEFAULT, "You failed.\n\r", ch );
  return SKPELL_MISSED;
}

int skill_berserk( int sn, int level, CHAR_DATA *ch, void *vo )
{
  AFFECT_DATA af;

  if ( !ch->fighting )
    return SKPELL_BOTCHED;

  if ( IS_AFFECTED2(ch, AFF_BERSERK) )
    return SKPELL_MISSED;

  af.type = sn;
  af.level = ch->level;
  af.duration = ch->level / 20;
  af.bitvector = AFF_BERSERK;

  af.location = APPLY_AC;
  af.modifier = ch->level * 3;
  affect_to_char2(ch, &af);

  send_to_char(AT_WHITE, "You suddenly go berserk.\n\r",ch);
  act(AT_WHITE, "$n suddenly goes berserk!", ch, NULL, NULL, TO_ROOM );
  return SKPELL_NO_DAMAGE;
}

int skill_bloodthirsty( int sn, int level, CHAR_DATA *ch, void *vo )
{
  AFFECT_DATA af;

  if ( !ch->fighting )
    return SKPELL_BOTCHED;

  if ( IS_AFFECTED3(ch, AFF_BLOODTHIRSTY) )
    return SKPELL_MISSED;

  af.type = sn;
  af.level = ch->level;
  af.duration = ch->level / 20;
  af.bitvector = AFF_BLOODTHIRSTY;

  af.location = APPLY_AC;
  af.modifier = ch->level * 7 / 2;
  affect_to_char3(ch, &af);

  send_to_char(AT_BLOOD, "You become bloodthirsty.\n\r",ch);
  act(AT_BLOOD, "$n becomes bloodthirsty!", ch, NULL, NULL, TO_ROOM );
  return SKPELL_NO_DAMAGE;
}

int skill_soulstrike( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim = ch->fighting;
  int dam;

  if ( !victim )
  {
    send_to_char( AT_WHITE, "You aren't fighting anyone!\n\r", ch );
    return SKPELL_MISSED;
  }

  if ( IS_NEUTRAL(ch) )
  {
    send_to_char(AT_RED, "Nothing happened.\n\r", ch );
    return SKPELL_MISSED;
  }

  dam = number_range( ch->level * 3, (ch->level * 20) );
  if ( ch->hit < dam )
  {
    send_to_char(AT_WHITE, "You do not have the strength.\n\r", ch );
    return SKPELL_MISSED;
  }
  
  if ( !IS_NPC( ch ) ) {
  	if (number_percent() > ( ch->pcdata->learned[sn] / 10 ) ) {
     	    send_to_char(AT_WHITE, "You fail to focus your souls energies.\n\r", ch);
     	    return SKPELL_MISSED;
  	}
  }

  damage(ch, ch, dam/5, sn);

  WAIT_STATE( ch, skill_table[sn].beats );

  if ( IS_EVIL(ch) )
  {
    send_to_char(AT_RED, "Your soul recoils!\n\r", ch);
    damage(ch, ch, dam/2, sn);
    return SKPELL_NO_DAMAGE;
  }

  act( AT_BLUE, "Your soul strikes deep into $N.", ch, NULL, victim, TO_CHAR );
  act( AT_BLUE, "$n's soul strikes deep into you.", ch, NULL, victim, TO_VICT );
  act( AT_BLUE, "$n's soul strikes deep into $N.", ch, NULL, victim, TO_NOTVICT );
  if ( IS_EVIL( victim ) )
  {
    dam *=2; //damage(ch, victim, dam*2, gsn_soulstrike);
  }
  else
  {
    dam = dam; //damage(ch, victim, dam, gsn_soulstrike);
  }
  return dam;
}

int skill_weaponmaster( int sn, int level, CHAR_DATA *ch, void *vo )
{
  AFFECT_DATA af;

  if ( ch->fighting )
    return SKPELL_MISSED;

  if IS_AFFECTED2( ch, AFF_WEAPONMASTER )
   return SKPELL_MISSED;

  af.type = sn;
  af.level = ch->level;
  af.duration = ch->level / 5;
  af.bitvector = AFF_WEAPONMASTER;

  af.location = APPLY_DAMROLL;
  af.modifier = ch->level * 3 / 4;
  affect_to_char2(ch, &af);

  af.location = APPLY_HITROLL;
  af.modifier = ch->level * 3 / 4;
  affect_to_char2(ch, &af);

  send_to_char(AT_WHITE, "You study your weapon carefully.\n\r",ch);
  act(AT_WHITE, "$n studies $s weapon.", ch, NULL, NULL, TO_ROOM );
  return SKPELL_NO_DAMAGE;
}

int skill_rage( int sn, int level, CHAR_DATA *ch, void *vo )
{
  AFFECT_DATA af;

  if ( ch->fighting )
    return SKPELL_MISSED;

  if IS_AFFECTED3( ch, AFF_RAGE )
   return SKPELL_MISSED;

  af.type = sn;
  af.level = ch->level;
  af.duration = ch->level / 5;
  af.bitvector = AFF_RAGE;

  af.location = APPLY_DAMROLL;
  af.modifier = ch->level * 3 / 4;
  affect_to_char3(ch, &af);

  af.location = APPLY_HITROLL;
  af.modifier = ch->level / 2;
  affect_to_char3(ch, &af);

  send_to_char(AT_BLOOD, "You enrage yourself.\n\r",ch);
  act(AT_BLOOD, "$n enrages $mself.", ch, NULL, NULL, TO_ROOM );
  return SKPELL_NO_DAMAGE;
}

int skill_blindfold( int sn, int level, CHAR_DATA *ch, void *vo )
{
  AFFECT_DATA af;

  if IS_AFFECTED2( ch, AFF_BLINDFOLD)
    return SKPELL_MISSED;

  if ( ch->fighting )
    return SKPELL_MISSED;

  af.type = sn;
  af.level = ch->level;
  af.duration = ch->level / 4;
  af.bitvector = AFF_BLINDFOLD;

  af.location = APPLY_NONE;
  af.modifier = 0;
  affect_to_char2(ch, &af);

  send_to_char(AT_WHITE, "You cover your eyes and rely on your senses.\n\r",ch);
  act(AT_WHITE, "$n covers $s eyes.", ch, NULL, NULL, TO_ROOM );
  return SKPELL_MISSED;
}

int skill_slit( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
	int dam = 0;

    if ( !check_blind( ch ) )
        return SKPELL_MISSED;

    one_argument( target_name, arg );

    victim = ch->fighting;

    if ( arg[0] != '\0' )
        if ( !( victim = get_char_room( ch, target_name ) ) )
	{
	    send_to_char(C_DEFAULT, "They aren't here.\n\r", ch );
	    return SKPELL_MISSED;
	}

    if ( !ch->fighting && victim != ch)
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }


    WAIT_STATE( ch, skill_table[sn].beats );

    if( !IS_NPC( victim ) )
    {
	if( victim->pcdata->learned[skill_lookup("psionic casting")] )
        {
	    send_to_char(C_DEFAULT, "They can not be slit.\n\r", ch );
	    return SKPELL_MISSED;
	}
    }

    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  + 10 )
    {
	dam = number_range(ch->level * 2, ch->level * 3); //damage( ch, victim, number_range( ch->level*2, ch->level*3 ), sn );

        if IS_AFFECTED2(victim, AFF_SLIT)
          return dam;
	else
	{
	if ( number_percent( ) < 50 )
        {
	AFFECT_DATA af;

	af.type      = sn;
	af.level     = ch->level;
	if (!IS_NPC (victim))
	af.duration  = ch->level / 100;
	else
	af.duration  = ch->level / 25;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SLIT;
        affect_to_char2(victim, &af);

        send_to_char(AT_BLOOD, "You run your weapon quickly across your opponents neck, blood sprays everywhere.\n\r",ch);
        send_to_char(AT_BLOOD, "Your neck has been slit, blood gushes out.\n\r", victim );
        act(AT_BLOOD, "$n opens up their opponents neck.", ch, NULL, NULL, TO_ROOM );
        return dam;
        }
        }
    update_pos( victim );
    }
    else
	dam = SKPELL_ZERO_DAMAGE; //damage( ch, victim, 0, gsn_slit );

    return dam;
}

int skill_thick_skin( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;

    if IS_AFFECTED2( ch, AFF_THICK_SKIN)
    {
        affect_strip( ch, sn );
  	REMOVE_BIT(ch->affected_by2, AFF_THICK_SKIN);
  	send_to_char(AT_RED, "You allow your skin to lose its rigidity.\n\r", ch);
        ch->shields--; 
        return SKPELL_NO_DAMAGE;
    }

    if( !IS_SHIELDABLE( ch ) )
	return SKPELL_MISSED;

    if ( !IS_NPC( ch )
        && ( ( ch->level < skill_table[sn].skill_level[ch->class] )
        && (ch->level < skill_table[sn].skill_level[ch->multied])))
    {
        send_to_char(C_DEFAULT, "You failed.\n\r",ch);
        return SKPELL_MISSED;
    }

    af.type = sn;
    af.level = ch->level;
    af.location = APPLY_DAM_PHYSICAL;
    af.modifier = ch->level / 5;
    af.duration = ch->level / 5;
    af.bitvector = AFF_THICK_SKIN;
    affect_to_char2(ch, &af);
    ch->shields += 1;

    send_to_char(AT_WHITE, "Your skin becomes thicker and more protective.\n\r",ch);
    act(AT_WHITE, "$n's skin thickens.", ch, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int skill_paralyse( int sn, int level, CHAR_DATA *ch, void *vo )
{
  AFFECT_DATA af;
  CHAR_DATA *victim = ch->fighting;
  int dam = 0;

  if (!ch->fighting )
    return SKPELL_MISSED;

  if IS_AFFECTED( ch, AFF_ANTI_FLEE)
    return SKPELL_MISSED;


    WAIT_STATE( ch, skill_table[sn].beats );

    dam = number_range( 600, ch->level * 10); //damage( ch, victim, number_range( 600, ch->level*10 ), gsn_paralyse );

    af.type = sn;
    af.level = ch->level;
    af.duration = ch->level / 25;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_ANTI_FLEE;
    affect_to_char(victim, &af);

    send_to_char(AT_BLOOD, "You paralyse your opponent!\n\r",ch);
    send_to_char(AT_BLOOD, "You have been paralysed!\n\r", victim );
    act(AT_BLOOD, "$n paralyses $N!", ch, NULL, victim, TO_NOTVICT );
    return dam;
}

int skill_cloaking( int sn, int level, CHAR_DATA *ch, void *vo )
{
  AFFECT_DATA af;

  if IS_AFFECTED2( ch, AFF_CLOAKING)
    return SKPELL_MISSED;

  if ( ch->fighting )
    return SKPELL_MISSED;

  af.type = sn;
  af.level = ch->level;
  af.duration = ch->level / 2;
  af.bitvector = AFF_CLOAKING;

  af.location = APPLY_NONE;
  af.modifier = 0;
  affect_to_char2(ch, &af);

  send_to_char(AT_WHITE, "You conceal your items from on lookers.\n\r",ch);
  return SKPELL_NO_DAMAGE;
}

int skill_break_weapon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    OBJ_DATA  *obj_lose;
    OBJ_DATA  *obj_next;
    char       arg [ MAX_INPUT_LENGTH ];

    if ( !ch->fighting )
    {
	send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !check_blind( ch ) )
        return SKPELL_MISSED;

    one_argument( target_name, arg );

    victim = ch->fighting;

    WAIT_STATE( ch, skill_table[sn].beats );
    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 )  )
    {
    if ( number_percent( ) < 20 )
        {
        for ( obj_lose = victim->carrying; obj_lose; obj_lose = obj_next )
        {
            char *msg;

            obj_next = obj_lose->next_content;
            if ( obj_lose->deleted )
                continue;
            if ( obj_lose->wear_loc != WEAR_WIELD )
                continue;
            if ( IS_SET( obj_lose->extra_flags, ITEM_NO_DAMAGE ) )
                continue;
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
            case ITEM_ARMOR:
            case ITEM_WEAPON:
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
		    send_to_char(AT_WHITE, "You have destroyed their weapon!\n\r", ch);
	            extract_obj( obj_lose );
                    obj_to_room ( pObj, victim->in_room );
                          break;

            act(AT_YELLOW, msg, victim, obj_lose, NULL, TO_CHAR );
            }
            }
            }
	    }
	  else
	    {
            act(AT_WHITE, "You failed!", ch, NULL, NULL, TO_CHAR    );
            }
    update_pos( victim );
    return SKPELL_NO_DAMAGE;
	}

	return SKPELL_MISSED;
}

int skill_metamorph( int sn, int level, CHAR_DATA *ch, void *vo )
{
  AFFECT_DATA af;

  if ( ch->fighting )
    return SKPELL_MISSED;

  if(IS_AFFECTED2( ch, AFF_WOLFED))
  {
  affect_strip( ch, skill_lookup("metamorph") );
  REMOVE_BIT(ch->affected_by2, AFF_WOLFED);
  send_to_char(AT_RED, "You metamorph yourself.\n\r", ch);
  return SKPELL_NO_DAMAGE;
  }

  if(!IS_AFFECTED2( ch, AFF_WOLFED))
  {
  af.type = sn;
  af.level = ch->level;
  af.duration = -1;
  af.bitvector = AFF_WOLFED;

  af.location = APPLY_DAMROLL;
  af.modifier = ch->level / 4;
  affect_to_char2(ch, &af);

  af.location = APPLY_HITROLL;
  af.modifier = ch->level / 5;
  affect_to_char2(ch, &af);

  af.location = APPLY_STR;
  af.modifier = 3;
  affect_to_char2(ch, &af);

  af.location = APPLY_CON;
  af.modifier = 2;
  affect_to_char2(ch, &af);

  af.location = APPLY_INT;
  af.modifier = -3;
  affect_to_char2(ch, &af);

  af.location = APPLY_WIS;
  af.modifier = -2;
  affect_to_char2(ch, &af);
  }

  send_to_char(AT_RED, "You metamorph yourself.\n\r",ch);
  act(AT_GREY, "$n metamorphs.", ch, NULL, NULL, TO_ROOM );
  return SKPELL_NO_DAMAGE;
}

int skill_cause_panic( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

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
    if ( IS_NPC( ch ) || number_percent( ) > ( ch->pcdata->learned[sn] / 10 )  )
        {
	send_to_char(AT_WHITE, "You scream, but your opponent does not flee!\n\r", ch);
	return SKPELL_MISSED;
        }
    else
	{
	do_flee( victim, "" );
        send_to_char(AT_BLOOD, "You cause your opponent to flee!\n\r",ch);
        send_to_char(AT_BLOOD, "You become frightened and run from battle!\n\r", victim );
        act(AT_BLOOD, "$n frightens off $s opponent.", ch, NULL, NULL, TO_ROOM );
        return SKPELL_NO_DAMAGE;
        }
    update_pos( victim );
    return SKPELL_NO_DAMAGE;
}

int skill_war_chant ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;

    if ( IS_AFFECTED3 ( ch, AFF_WAR_CHANT ) || IS_NPC ( ch ) )
    {
        return SKPELL_MISSED;
    }

    if ( number_percent ( ) > ( ch->pcdata->learned[sn] / 10 )  )
    {
        send_to_char ( AT_BLUE, "Nothing happens.\n\r", ch );
        return SKPELL_MISSED;
    }

    af.type      = sn;
    af.level     = ch->level;
    af.duration  = ch->level/5;
    af.bitvector = AFF_WAR_CHANT;
    af.location  = APPLY_HIT;
    af.modifier  = ch->level * 4;
    affect_to_char3 ( ch, &af );

    af.location  = APPLY_DAMROLL;
    af.modifier  = ch->level / 4;
    affect_to_char3 ( ch, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - ch->level / 5;
    affect_to_char3 ( ch, &af );

    send_to_char ( AT_BLUE, "You begin chanting your war cry.\n\r", ch );
    act ( AT_BLUE, "$n begins chanting $s war cry.\n\r", ch, NULL, NULL, TO_ROOM );
    return SKPELL_MISSED;
}

int skill_primalscream( int sn, int level, CHAR_DATA *ch, void *vo )
{
  AFFECT_DATA af;

  if ( ch->fighting )
    return SKPELL_MISSED;

  if IS_AFFECTED3( ch, AFF_PRIMALSCREAM )
   return SKPELL_MISSED;

  af.type = sn;
  af.level = ch->level;
  af.duration = ch->level / 3;
  af.bitvector = AFF_PRIMALSCREAM;

  af.location = APPLY_DAMROLL;
  af.modifier = ch->level / 3;
  affect_to_char3(ch, &af);

  af.location = APPLY_HITROLL;
  af.modifier = ch->level / 3;
  affect_to_char3(ch, &af);

  send_to_char(AT_BLOOD, "You scream at the top of your lungs!\n\r",ch);
  act(AT_BLOOD, "$n utters a gutteral, primal scream.", ch, NULL, NULL, TO_ROOM );
  return  SKPELL_NO_DAMAGE;
}

void tag_newbie_slayer( CHAR_DATA* ch )
{

        AFFECT_DATA af;
        int level = ch->level;
	int aff   = number_range(1, 26);

        af.type      = skill_lookup("newbie slayer");
        af.level     = level;
        af.duration  = level;
        af.bitvector = AFF_NEWBIE_SLAYER;

	switch (aff)
	{
	case APPLY_STR:
	        af.location  = APPLY_STR;
        	af.modifier = number_range(level/30, level/3) * -1;
		break;
	case APPLY_DEX:
	        af.location  = APPLY_DEX;
        	af.modifier = number_range(level/30, level/3) * -1;
		break;
	case APPLY_INT:
	        af.location  = APPLY_INT;
        	af.modifier = number_range(level/30, level/3) * -1;
		break;
	case APPLY_WIS:
	        af.location  = APPLY_WIS;
        	af.modifier = number_range(level/30, level/3) * -1;
		break;
	case APPLY_CON:
	        af.location  = APPLY_CON;
        	af.modifier = number_range(level/30, level/3) * -1;
		break;
	case APPLY_SEX:
	        af.location  = APPLY_DAMROLL;
        	af.modifier = number_range(level/2, level*2) * -1;
		break;
	case APPLY_CLASS:
	        af.location  = APPLY_HITROLL;
        	af.modifier = number_range(level/2, level*2) * -1;
		break;
	case APPLY_LEVEL:
	        af.location  = APPLY_DEX;
        	af.modifier = number_range(level/30, level/3) * -1;
		break;
	case APPLY_AGE:
	        af.location  = APPLY_INT;
        	af.modifier = number_range(level/30, level/3) * -1;
		break;
	case APPLY_HEIGHT:
	        af.location  = APPLY_CON;
        	af.modifier = number_range(level/30, level/3) * -1;
		break;
	case APPLY_WEIGHT:
	        af.location  = APPLY_WIS;
        	af.modifier = number_range(level/30, level/3) * -1;
		break;
	case APPLY_MANA:
		if (ch->class != 9)
		{
	        af.location  = APPLY_MANA;
        	af.modifier = number_range(level*5, level*10) * -1;
		} else
		{
		af.location = APPLY_BP;
		af.modifier = number_range(level*2, level *10) * -1;
		}
		break;
	case APPLY_HIT:
	        af.location  = APPLY_HIT;
        	af.modifier = number_range(level*10, level*50) * -1;
		break;
	case APPLY_MOVE:
	        af.location  = APPLY_MOVE;
        	af.modifier = number_range(level*5, level*10) * -1;
		break;
	case APPLY_GOLD:
	        af.location  = APPLY_STR;
        	af.modifier = number_range(level/30, level/3) * -1;
		break;
	case APPLY_EXP:
	        af.location  = APPLY_SAVING_SPELL;
        	af.modifier = number_range(level/2, level*2) * -1;
		break;
	case APPLY_AC:
	        af.location  = APPLY_AC;
        	af.modifier = number_range(level*2, level*5);
		break;
	case APPLY_HITROLL:
	        af.location  = APPLY_HITROLL;
        	af.modifier = number_range(level/2, level*2) * -1;
		break;
	case APPLY_DAMROLL:
	        af.location  = APPLY_DAMROLL;
        	af.modifier = number_range(level/2, level*2) * -1;
		break;
	case APPLY_SAVING_SPELL:
	        af.location  = APPLY_SAVING_SPELL;
        	af.modifier = number_range(level/2, level*2);
		break;
	case APPLY_SAVING_BREATH:
	        af.location  = APPLY_SAVING_BREATH;
        	af.modifier = number_range(level/2, level*2);
		break;
	case APPLY_SAVING_PETRI:
	        af.location  = APPLY_SAVING_BREATH;
        	af.modifier = number_range(level/2, level*2);
		break;
	case APPLY_SAVING_ROD:
	        af.location  = APPLY_SAVING_SPELL;
        	af.modifier = number_range(level/2, level*2);
		break;
	case APPLY_SAVING_PARA:
	        af.location  = APPLY_SAVING_SPELL;
        	af.modifier = number_range(level/2, level*2);
		break;
	case APPLY_BP:
		if (ch->class != 9)
		{
	        af.location  = APPLY_MANA;
        	af.modifier = number_range(level*5, level*10) * -1;
		} else
		{
	        af.location  = APPLY_BP;
        	af.modifier = number_range(level*2, level*5) * -1;
		}
		break;
	case APPLY_ANTI_DIS:
	        af.location  = APPLY_ANTI_DIS;
        	af.modifier = number_range(level/2, level*2) * -1;
		break;
	default:
		af.location = APPLY_STR;
        	af.modifier = number_range(level/30, level/3) * -1;
		break;
	}			

        affect_to_char4(ch , &af);

	send_to_char(AT_RED, "You have been tagged as a [Newbie Slayer]!\n\r", ch);

	return;
}

