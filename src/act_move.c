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

/*$Id: act_move.c,v 1.42 2005/03/24 19:09:36 ahsile Exp $*/
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

char *  const   dir_noun        [ ]             =
{
    "the north", "the east", "the south", "the west", "above", "below"
};

char *	const	dir_name	[ ]		=
{
    "north", "east", "south", "west", "up", "down"
};

const	int	rev_dir		[ ]		=
{
    2, 3, 0, 1, 5, 4
};

const	int	movement_loss	[ SECT_MAX ]	=
{
    1, 2, 2, 3, 4, 5, 4, 1, 6, 10, 6
};



/*
 * Local functions.
 */
OBJ_DATA *has_key	args( ( CHAR_DATA *ch, int key ) );


int find_door( CHAR_DATA *ch, char *arg );

void move_char( CHAR_DATA *ch, int door )
{
    CHAR_DATA       *fch;
    CHAR_DATA       *fch_next;
    EXIT_DATA       *pexit;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    char xxxtemp [ MAX_STRING_LENGTH ];



    if ( IS_AFFECTED(ch, AFF_ANTI_FLEE) )
     {
       send_to_char(AT_WHITE, "You cannot move.\n\r", ch);
       return;
     }
    if ( door < 0 || door > 5 )
    {
	bug( "Do_move: bad door %d.", door );
	return;
    }

    in_room = ch->in_room;
    
    if ( !IS_NPC( ch ) )
    {
      int drunk = 0;
      int nd = 0;
      drunk = ch->pcdata->condition[COND_DRUNK];

      if ( number_percent() < drunk )
      {
        for ( nd = door; nd == door; nd = number_door() );
        door = nd;
        send_to_char( AT_BLUE, "You're too drunk to think clearly! You wander"
        " off in the wrong direction.\n\r", ch );
      }
    }
    
    if ( !( pexit = in_room->exit[door] ) || !( to_room = pexit->to_room ) )
    {
	send_to_char(AT_GREY, "Alas, you cannot go that way.\n\r", ch );
	return;
    }

    if ( IS_SET( pexit->exit_info, EX_HIDDEN )
        && !IS_SET( race_table[ch->race].race_abilities, RACE_DETECT_HIDDEN_EXIT)
        && !IS_SET( ch->act, PLR_HOLYLIGHT) )
    {
      send_to_char(AT_GREY, "Alas, you cannot go that way.\n\r", ch );
	return;
    } 
    else
    {
      if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
        if ( !IS_AFFECTED( ch, AFF_PASS_DOOR )
            && !IS_SET( race_table[ ch->race ].race_abilities, RACE_PASSDOOR )
	    && ( ch->position != POS_GHOST ) )
        {
	    act(AT_GREY, "The &W$d&w is closed.",
		ch, NULL, pexit->keyword, TO_CHAR );
	    return;
        }
	  if ( IS_SET( pexit->exit_info, EX_PASSPROOF ) && ch->position != POS_GHOST )
        {
	    act(AT_GREY, "You are unable to pass through the &W$d&w.",
		ch, NULL, pexit->keyword, TO_CHAR );
	    return;
	  }
      }
    }

    if ( IS_AFFECTED( ch, AFF_CHARM )
	&& ch->master
	&& in_room == ch->master->in_room )
    {
	send_to_char(AT_GREY, "What?  And leave your beloved master?\n\r", ch );
	return;
    }

    if ( room_is_private( to_room ) )
    {
	send_to_char(AT_GREY, "That room is private right now.\n\r", ch );
	return;
    }

    if ( (ch->pkill) && (ch->combat_timer>0) && (IS_SET( to_room->room_flags, ROOM_SAFE ) ) && ch->level >= 30 ) 
    {
        send_to_char(AT_YELLOW, "A strange force bars your entrance. Your blood runs too hot for this room!\n\r", ch);
        return;
    }

    if ( !IS_NPC( ch ) )
    {
	int iClass;
	int move;

	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    if ( iClass != ch->class
		&& to_room->vnum == class_table[iClass].guild )
	    {
		send_to_char(AT_GREY, "You aren't allowed in there.\n\r", ch );
		return;
	    }
	}

	if (   in_room->sector_type == SECT_AIR
	    || to_room->sector_type == SECT_AIR )
	{
            if ( !IS_AFFECTED( ch, AFF_FLYING )
                && !IS_SET( race_table[ ch->race ].race_abilities, RACE_FLY ) 
	       && ( ch->race != 3 )
	       && ( ch->race != 6 )
	       && ( ch->position != POS_GHOST ) ) 
	    {
		send_to_char(AT_GREY, "You can't fly.\n\r", ch );
		return;
	    }
	}

	if (   in_room->sector_type == SECT_WATER_NOSWIM
	    || to_room->sector_type == SECT_WATER_NOSWIM )
	{
	    OBJ_DATA *obj;
	    bool      found;

	    /*
	     * Look for a boat.
	     */
	    found = FALSE;

	    /*
	     * Suggestion for flying above water by Sludge
	     */
            if ( IS_AFFECTED( ch, AFF_FLYING )
                || IS_SET( race_table[ ch->race ].race_abilities, RACE_FLY )
	        || IS_SET( race_table[ ch->race ].race_abilities, RACE_WATERWALK )
                || IS_SET( race_table[ ch->race ].race_abilities, RACE_SWIM ) 
		|| ( ch->position == POS_GHOST ) )
	        found = TRUE;

	    for ( obj = ch->carrying; obj; obj = obj->next_content )
	    {
		if ( obj->item_type == ITEM_BOAT )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
	    {
		send_to_char(AT_GREY, "You need a boat to go there.\n\r", ch );
		return;
	    }
	}
      


	move = movement_loss[UMIN( SECT_MAX-1, in_room->sector_type )]
	     + movement_loss[UMIN( SECT_MAX-1, to_room->sector_type )]
	     ;

	if( IS_AFFECTED( ch, AFF_FLYING ) )
	{
	    move /= 2;
 	}

	if( ch->mounted > 0 )
	{
	    move /= 2;
	}

	if ( ch->move < move && ch->position != POS_GHOST)
	{
	    send_to_char(AT_GREY, "You are too exhausted.\n\r", ch );
	    return;
	}

	if( ch->race == 3 || ch->race == 8 )
	{
	    move /= 2;
	}

	if( in_room->sector_type == SECT_FOREST && is_affected( ch, skill_lookup("forestwalk") ) )
	{
	    move = 0;
	}
	if( ( in_room->sector_type == SECT_HILLS || in_room->sector_type == SECT_MOUNTAIN ) && is_affected( ch, skill_lookup("mountainwalk") ) )
	{
	    move = 0;
	}
	if( ( in_room->sector_type == SECT_FIELD || in_room->sector_type == SECT_DESERT ) && is_affected( ch, skill_lookup("plainswalk") ) )
	{
	    move = 0;
	}
	if( in_room->sector_type == SECT_BADLAND && is_affected( ch, skill_lookup("swampwalk") ) )
	{
	    move = 0;
	}

	WAIT_STATE( ch, 1 );
	if ( ch->position != POS_GHOST )
	{
	   ch->move -= move;
	}
    }

    if ( !IS_AFFECTED( ch, AFF_SNEAK )
	&& ( IS_NPC( ch ) || !IS_SET( ch->act, PLR_WIZINVIS ) ) 
	&& ( ch->race != 4 ) 
	&& ( ch->position != POS_GHOST )
	&& ( ch->mounted == 0 ) )
	  {
	   if ( ( !IS_AFFECTED( ch, AFF_FLYING ) )
	       && ( ch->race != 3 )
	       && ( ch->race != 6 ) )
	       act(AT_GREY, "&B$n&w leaves $T.", ch, NULL, dir_name[door], TO_ROOM );
           else
               act(AT_GREY, "&B$n&w flies $T.", ch, NULL, dir_name[door], TO_ROOM );
          }

   if ( !IS_AFFECTED( ch, AFF_SNEAK )
        && ( IS_NPC( ch ) || !IS_SET( ch->act, PLR_WIZINVIS ) )
        && ( ch->race != 4 )
        && ( ch->position != POS_GHOST )
	&& ( ch->mounted > 0 ) )
          {
           if ( ( !IS_AFFECTED( ch, AFF_FLYING ) )
               && ( ch->race != 3 )
               && ( ch->race != 6 ) )
	   {
		sprintf( xxxtemp, "&B$n&w, mounted on %s, leaves $T.", ch->mountshort );
	        act(AT_GREY, xxxtemp, ch, NULL, dir_name[door], TO_ROOM );
	   }
           else
	   {
		sprintf( xxxtemp, "&B$n&w, mounted on %s, flies $T.", ch->mountshort );
	        act(AT_GREY, xxxtemp, ch, NULL, dir_name[door], TO_ROOM );
	   }
          }

    if ( !IS_AFFECTED( ch, AFF_SNEAK )
        && ( IS_NPC( ch ) || !IS_SET( ch->act, PLR_WIZINVIS ) )
        && ( ch->race != 4 )
        && ( ch->position == POS_GHOST ) )
    {
	act(AT_GREY, "&CThe Ghost of $n&w floats away $T.", ch, NULL, dir_name[door], TO_ROOM );
    }

    eprog_enter_trigger( pexit, ch->in_room, ch );
    if ( ch->in_room != to_room )
    {
      char_from_room( ch );
      char_to_room( ch, to_room );
    }
    if ( !IS_AFFECTED( ch, AFF_SNEAK )
	&& ( IS_NPC( ch ) || !IS_SET( ch->act, PLR_WIZINVIS ) )
	&& ( ch->race != 4 )
	&& ( ch->position != POS_GHOST )
	&& ( ch->mounted == 0 ) )
          act(AT_GREY, "&B$n&w arrives from $T.", ch, NULL,
	      dir_noun[rev_dir[door]], TO_ROOM );

    if ( !IS_AFFECTED( ch, AFF_SNEAK )
        && ( IS_NPC( ch ) || !IS_SET( ch->act, PLR_WIZINVIS ) )
        && ( ch->race != 4 )
        && ( ch->position != POS_GHOST )  
        && ( ch->mounted > 0 ) )
    {
	sprintf( xxxtemp, "&B$n&w arrives from $T, mounted on %s.", ch->mountshort );
        act(AT_GREY, xxxtemp, ch, NULL, dir_noun[rev_dir[door]], TO_ROOM );
    }

    if ( !IS_AFFECTED( ch, AFF_SNEAK )
        && ( IS_NPC( ch ) || !IS_SET( ch->act, PLR_WIZINVIS ) )
        && ( ch->race != 4 )
        && ( ch->position == POS_GHOST ) )
          act(AT_GREY, "&CThe Ghost of $n&w floats in from $T.", ch, NULL,
              dir_noun[rev_dir[door]], TO_ROOM );

    do_look( ch, "auto" );

    if ( to_room->exit[rev_dir[door]] &&
	 to_room->exit[rev_dir[door]]->to_room == in_room )
      eprog_exit_trigger( to_room->exit[rev_dir[door]], ch->in_room, ch );
    else
      rprog_enter_trigger( ch->in_room, ch );

    /*special handling incase two people are following each other*/
	/*Make sure the room we're going to isn't the room we're in */
    /*-- Manaux */
    if (ch->in_room  != in_room)
    {
        for ( fch = in_room->people; fch; fch = fch_next )
        {
            fch_next = fch->next_in_room;
    
            if ( fch->deleted )
    	    continue;
         	 
    		if ( fch->master == ch && fch->position == POS_STANDING )
    
    		{
    		    act(AT_GREY, "You follow $N.", fch, NULL, ch, TO_CHAR );
		    if (!IS_NPC(ch) && ch->pcdata->craft_timer)
			destroy_craft(ch, FALSE);
    		    move_char( fch, door );
    	 	}
        }
	}
        
    if ( !IS_IMMORTAL( ch ) && !(ch->position == POS_GHOST)
             && ( to_room->sector_type == SECT_UNDERWATER
                 && !IS_AFFECTED3( ch, AFF_GILLS )
                 && !IS_SET( race_table[ ch->race ].race_abilities,
                            RACE_WATERBREATH ) ) )
    {
        send_to_char( AT_BLUE, "You can't breathe!\n\r", ch );
        act( AT_BLUE, "$n sputters and chokes!", ch, NULL, NULL, TO_ROOM );
        damage( ch, ch, 6, TYPE_UNDEFINED );
    }

    if ( IS_SET( to_room->room_flags, ROOM_NOFLOOR ) &&
     ( ch->position != POS_GHOST ) &&
     !IS_AFFECTED( ch, AFF_FLYING ) && ( ( pexit = to_room->exit[5] ) != NULL )
       && ( ( to_room = pexit->to_room ) != NULL ) )
    {
      act( AT_WHITE, "$n falls through the air to the room below.", ch,
       NULL, NULL, TO_ROOM );
   act( AT_RED, "You fall down through where you thought the ground was!",
       ch, NULL, NULL, TO_CHAR );
      move_char( ch, 5 );
      act( AT_WHITE, "$n falls down from above.", ch, NULL, NULL, TO_ROOM );
      damage( ch, ch, 5, TYPE_UNDEFINED );
    }

    if ( !IS_NPC( ch ) )
    {
      if ( !IS_SET( ch->act, PLR_WIZINVIS ) )
      {
	mprog_greet_trigger( ch );
	return;
      }
      else return;
    }

    if (IS_SET(ch->in_room->room_flags,ROOM_PKILL) && ch->pkill_timer > 0)
    {
       ch->pkill_timer = 0;
    }

    mprog_entry_trigger( ch );
    mprog_greet_trigger( ch );
    return;
}



void do_north( CHAR_DATA *ch, char *argument )
{
    if(IS_AFFECTED4(ch, AFF_BURROW))
    {
        send_to_char(C_DEFAULT, "You can not move while burrowed!\n\r",ch);
	return;
    }
    if( ch->position == POS_FIGHTING )
    {
        send_to_char(C_DEFAULT, "You can not move while fighting!\n\r", ch );
        return;
    }
    move_char( ch, DIR_NORTH );
    return;
}



void do_east( CHAR_DATA *ch, char *argument )
{
    if(IS_AFFECTED4(ch, AFF_BURROW))
    {
        send_to_char(C_DEFAULT, "You can not move while burrowed!\n\r",ch);
	return;
    }
    if( ch->position == POS_FIGHTING )
    {
        send_to_char(C_DEFAULT, "You can not move while fighting!\n\r", ch );
        return;
    }
    move_char( ch, DIR_EAST );
    return;
}



void do_south( CHAR_DATA *ch, char *argument )
{
    if(IS_AFFECTED4(ch, AFF_BURROW))
    {
        send_to_char(C_DEFAULT, "You can not move while burrowed!\n\r",ch);
	return;
    }
    if( ch->position == POS_FIGHTING )
    {
        send_to_char(C_DEFAULT, "You can not move while fighting!\n\r", ch );
        return;
    }
    move_char( ch, DIR_SOUTH );
    return;
}



void do_west( CHAR_DATA *ch, char *argument )
{
    if(IS_AFFECTED4(ch, AFF_BURROW))
    {
        send_to_char(C_DEFAULT, "You can not move while burrowed!\n\r",ch);
	return;
    }
    if( ch->position == POS_FIGHTING )
    {
        send_to_char(C_DEFAULT, "You can not move while fighting!\n\r", ch );
        return;
    }
    move_char( ch, DIR_WEST );
    return;
}



void do_up( CHAR_DATA *ch, char *argument )
{
    if(IS_AFFECTED4(ch, AFF_BURROW))
    {
        send_to_char(C_DEFAULT, "You can not move while burrowed!\n\r",ch);
	return;
    }

    if( ch->position == POS_FIGHTING )
    {
        send_to_char(C_DEFAULT, "You can not move while fighting!\n\r", ch );
        return;
    }
    move_char( ch, DIR_UP );
    return;
}



void do_down( CHAR_DATA *ch, char *argument )
{
    if(IS_AFFECTED4(ch, AFF_BURROW))
    {
        send_to_char(C_DEFAULT, "You can not move while burrowed!\n\r",ch);
	return;
    }
    if( ch->position == POS_FIGHTING )
    {
        send_to_char(C_DEFAULT, "You can not move while fighting!\n\r", ch );
        return;
    }
    move_char( ch, DIR_DOWN );
    return;
}



int find_door( CHAR_DATA *ch, char *arg )
{
    EXIT_DATA *pexit;
    int        door;

	 if ( !str_prefix( arg, "north" ) ) door = 0;
    else if ( !str_prefix( arg, "east"  ) ) door = 1;
    else if ( !str_prefix( arg, "south" ) ) door = 2;
    else if ( !str_prefix( arg, "west"  ) ) door = 3;
    else if ( !str_prefix( arg, "up"    ) ) door = 4;
    else if ( !str_prefix( arg, "down"  ) ) door = 5;
    else
    {
	for ( door = 0; door <= 5; door++ )
	{
	    if ( ( pexit = ch->in_room->exit[door] )
		&& IS_SET( pexit->exit_info, EX_ISDOOR )
		&& pexit->keyword
		&& is_name( arg, pexit->keyword ) 
            && (!IS_SET( pexit->exit_info, EX_HIDDEN )
                || IS_SET(race_table[ch->race].race_abilities, RACE_DETECT_HIDDEN_EXIT) ) )
		return door;
	}
	act(AT_GREY, "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !( pexit = ch->in_room->exit[door] ) )
    {
	act(AT_GREY, "I see no door $T here.", ch, NULL, arg, TO_CHAR );
	return -1;
    }

    if ( !IS_SET( pexit->exit_info, EX_ISDOOR ) )
    {
	send_to_char(AT_GREY, "You can't do that.\n\r", ch );
	return -1;
    }

    return door;
}



void do_open( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];
    int       door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Open what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) )
    {
	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char(C_DEFAULT, "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET( obj->value[1], CONT_CLOSED )    )
	    { send_to_char(C_DEFAULT, "It's already open.\n\r",      ch ); return; }
	if ( !IS_SET( obj->value[1], CONT_CLOSEABLE ) )
	    { send_to_char(C_DEFAULT, "You can't do that.\n\r",      ch ); return; }
	if (  IS_SET( obj->value[1], CONT_LOCKED )    )
	    { send_to_char(C_DEFAULT, "It's locked.\n\r",            ch ); return; }

	REMOVE_BIT( obj->value[1], CONT_CLOSED );
	send_to_char(C_DEFAULT, "Ok.\n\r", ch );
	act(C_DEFAULT, "$n opens $p.", ch, obj, NULL, TO_ROOM );
	oprog_open_trigger( obj, ch );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'open door' */
	EXIT_DATA       *pexit;
	EXIT_DATA       *pexit_rev;
	ROOM_INDEX_DATA *to_room;

	pexit = ch->in_room->exit[door];
      if ( !IS_SET( pexit->exit_info, EX_HIDDEN ) 
          || IS_SET(race_table[ch->race].race_abilities, RACE_DETECT_HIDDEN_EXIT)
          || IS_SET(ch->act, PLR_HOLYLIGHT))
      {
        if ( !IS_SET( pexit->exit_info, EX_CLOSED )  )
  	    { send_to_char(C_DEFAULT, "It's already open.\n\r",     ch ); return; }
	  if (  IS_SET( pexit->exit_info, EX_LOCKED )  )
	    { send_to_char(C_DEFAULT, "It's locked.\n\r",           ch ); return; }

	  REMOVE_BIT( pexit->exit_info, EX_CLOSED );
	  act(C_DEFAULT, "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	  send_to_char(C_DEFAULT, "Ok.\n\r", ch );
	  eprog_open_trigger( pexit, ch->in_room, ch );

	  /* open the other side */
	  if (   ( to_room   = pexit->to_room               )
	     && ( pexit_rev = to_room->exit[rev_dir[door]] )
	     && pexit_rev->to_room == ch->in_room )
	  {
	    CHAR_DATA *rch;

	    REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch; rch = rch->next_in_room )
	    {
		if ( rch->deleted )
		    continue;
		act(C_DEFAULT, "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	    }
	  }
      }
      else
      { 
        act(AT_GREY, "I see no door $T here.", ch, NULL, arg, TO_CHAR );
      }
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];
    int       door;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Close what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) )
    {
	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char(C_DEFAULT, "That's not a container.\n\r", ch ); return; }
	if (  IS_SET( obj->value[1], CONT_CLOSED )    )
	    { send_to_char(C_DEFAULT, "It's already closed.\n\r",    ch ); return; }
	if ( !IS_SET( obj->value[1], CONT_CLOSEABLE ) )
	    { send_to_char(C_DEFAULT, "You can't do that.\n\r",      ch ); return; }

	SET_BIT( obj->value[1], CONT_CLOSED );
	send_to_char(C_DEFAULT, "Ok.\n\r", ch );
	act(C_DEFAULT, "$n closes $p.", ch, obj, NULL, TO_ROOM );
	oprog_close_trigger( obj, ch );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'close door' */
	EXIT_DATA       *pexit;
	EXIT_DATA       *pexit_rev;
	ROOM_INDEX_DATA *to_room;

	pexit	= ch->in_room->exit[door];
      if ( !IS_SET( pexit->exit_info, EX_HIDDEN)
         || IS_SET( race_table[ch->race].race_abilities, RACE_DETECT_HIDDEN_EXIT) 
         || IS_SET( ch->act, PLR_HOLYLIGHT ))
      {
        if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
        {
	    send_to_char(C_DEFAULT, "It's already closed.\n\r",    ch );
	    return;
	  }

	  if ( IS_SET( pexit->exit_info, EX_BASHED ) )
	  {
	    act(C_DEFAULT, "The $d has been bashed open and cannot be closed.",
		ch, NULL, pexit->keyword, TO_CHAR );
	    return;
	  }

	  SET_BIT( pexit->exit_info, EX_CLOSED );
	  act(C_DEFAULT, "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	  send_to_char(C_DEFAULT, "Ok.\n\r", ch );
	  eprog_close_trigger( pexit, ch->in_room, ch );

	/* close the other side */
	  if (   ( to_room   = pexit->to_room               )
	      && ( pexit_rev = to_room->exit[rev_dir[door]] )
	      && pexit_rev->to_room == ch->in_room )
	  {
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = to_room->people; rch; rch = rch->next_in_room )
	    {
		if ( rch->deleted )
		    continue;
		act(C_DEFAULT, "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	    }
	  }
      }
      else
      {
        act(AT_GREY, "I see no door $T here.", ch, NULL, arg, TO_CHAR );
      }
    }
    return;
}


OBJ_DATA *has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key )
	    return obj;
    }

    return NULL;
}



void do_lock( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    OBJ_DATA *key;
    char      arg [ MAX_INPUT_LENGTH ];
    int       door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Lock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) )
    {
	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char(C_DEFAULT, "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET( obj->value[1], CONT_CLOSED ) )
	    { send_to_char(C_DEFAULT, "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char(C_DEFAULT, "It can't be locked.\n\r",     ch ); return; }
	if ( !(key = has_key( ch, obj->value[2] )) )
	    { send_to_char(C_DEFAULT, "You lack the key.\n\r",       ch ); return; }
	if (  IS_SET( obj->value[1], CONT_LOCKED ) )
	    { send_to_char(C_DEFAULT, "It's already locked.\n\r",    ch ); return; }

	SET_BIT( obj->value[1], CONT_LOCKED );
	send_to_char(C_DEFAULT, "*Click*\n\r", ch );
	act(C_DEFAULT, "$n locks $p.", ch, obj, NULL, TO_ROOM );
	oprog_lock_trigger( obj, ch, key );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'lock door' */
	EXIT_DATA       *pexit;
	EXIT_DATA       *pexit_rev;
	ROOM_INDEX_DATA *to_room;

	pexit	= ch->in_room->exit[door];
      if ( !IS_SET( pexit->exit_info, EX_HIDDEN )
          || IS_SET( race_table[ch->race].race_abilities, RACE_DETECT_HIDDEN_EXIT)
          || IS_SET(ch->act, PLR_HOLYLIGHT))
	{
        if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )
	     { send_to_char(C_DEFAULT, "It's not closed.\n\r",        ch ); return; }
	  if ( pexit->key < 0 )
	     { send_to_char(C_DEFAULT, "It can't be locked.\n\r",     ch ); return; }
	  if ( !(key = has_key( ch, pexit->key )) )
	     { send_to_char(C_DEFAULT, "You lack the key.\n\r",       ch ); return; }
	  if (  IS_SET( pexit->exit_info, EX_LOCKED ) )
	     { send_to_char(C_DEFAULT, "It's already locked.\n\r",    ch ); return; }

	  SET_BIT( pexit->exit_info, EX_LOCKED );
	  send_to_char(C_DEFAULT, "*Click*\n\r", ch );
	  act(C_DEFAULT, "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	  eprog_lock_trigger( pexit, ch->in_room, ch, key );

	/* lock the other side */
	  if (   ( to_room   = pexit->to_room               )
	      && ( pexit_rev = to_room->exit[rev_dir[door]] )
	      && pexit_rev->to_room == ch->in_room )
	  {
	     SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	  }
      }
      else
      {
        act(AT_GREY, "I see no door $T here.", ch, NULL, arg, TO_CHAR );
      }
    }
    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    OBJ_DATA *key;
    char      arg [ MAX_INPUT_LENGTH ];
    int       door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Unlock what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) )
    {
	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char(C_DEFAULT, "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET( obj->value[1], CONT_CLOSED ) )
	    { send_to_char(C_DEFAULT, "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char(C_DEFAULT, "It can't be unlocked.\n\r",   ch ); return; }
	if ( !(key = has_key( ch, obj->value[2] )) )
	    { send_to_char(C_DEFAULT, "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET( obj->value[1], CONT_LOCKED ) )
	    { send_to_char(C_DEFAULT, "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT( obj->value[1], CONT_LOCKED );
	send_to_char(C_DEFAULT, "*Click*\n\r", ch );
	act(C_DEFAULT, "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
	oprog_unlock_trigger( obj, ch, key );
	return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )
    {
	/* 'unlock door' */
	EXIT_DATA       *pexit;
	EXIT_DATA       *pexit_rev;
	ROOM_INDEX_DATA *to_room;

	pexit = ch->in_room->exit[door];
      if (!IS_SET( pexit->exit_info, EX_HIDDEN )
          || IS_SET( race_table[ch->race].race_abilities, RACE_DETECT_HIDDEN_EXIT)
          || IS_SET(ch->act, PLR_HOLYLIGHT) )
      {
	  if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )
	     { send_to_char(C_DEFAULT, "It's not closed.\n\r",        ch ); return; }
	  if ( pexit->key < 0 )
	     { send_to_char(C_DEFAULT, "It can't be unlocked.\n\r",   ch ); return; }
	  if ( !(key = has_key( ch, pexit->key )) )
	     { send_to_char(C_DEFAULT, "You lack the key.\n\r",       ch ); return; }
	  if ( !IS_SET( pexit->exit_info, EX_LOCKED ) )
	     { send_to_char(C_DEFAULT, "It's already unlocked.\n\r",  ch ); return; }

	  REMOVE_BIT( pexit->exit_info, EX_LOCKED );
	  send_to_char(C_DEFAULT, "*Click*\n\r", ch );
	  act(C_DEFAULT, "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	  eprog_unlock_trigger( pexit, ch->in_room, ch, key );

	  /* unlock the other side */
	  if (   ( to_room   = pexit->to_room               )
	      && ( pexit_rev = to_room->exit[rev_dir[door]] )
	      && pexit_rev->to_room == ch->in_room )
	  {
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	  }
      }
      else
      {
        act(AT_GREY, "I see no door $T here.", ch, NULL, arg, TO_CHAR );
      }
    }
    return;
}



int skill_pick( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA  *obj;
    CHAR_DATA *gch;
    char       arg [ MAX_INPUT_LENGTH ];
    int        door;

    /* look for guards */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
        if ( gch->deleted )
	    continue;
	if ( IS_NPC( gch ) && IS_AWAKE( gch ) && ch->level + 5 < gch->level )
	{
	    act(C_DEFAULT, "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR );
	    return SKPELL_BOTCHED;
	}
    }

    if ( ( obj = get_obj_here( ch, target_name ) ) )
    {
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char(C_DEFAULT, "That's not a container.\n\r", ch ); return SKPELL_MISSED; }
	if ( !IS_SET( obj->value[1], CONT_CLOSED )    )
	    { send_to_char(C_DEFAULT, "It's not closed.\n\r",        ch ); return SKPELL_MISSED; }
	if ( obj->value[2] < 0 )
	    { send_to_char(C_DEFAULT, "It can't be unlocked.\n\r",   ch ); return SKPELL_MISSED; }
	if ( !IS_SET( obj->value[1], CONT_LOCKED )    )
	    { send_to_char(C_DEFAULT, "It's already unlocked.\n\r",  ch ); return SKPELL_MISSED; }
	if (  IS_SET( obj->value[1], CONT_PICKPROOF ) )
	    { send_to_char(C_DEFAULT, "You failed.\n\r",             ch ); return SKPELL_MISSED; }

	REMOVE_BIT( obj->value[1], CONT_LOCKED );
	send_to_char(C_DEFAULT, "*Click*\n\r", ch );
	act(C_DEFAULT, "$n picks $p.", ch, obj, NULL, TO_ROOM );
	oprog_pick_trigger( obj, ch );
	return SKPELL_NO_DAMAGE;
    }

    if ( ( door = find_door( ch, target_name ) ) >= 0 )
    {
	/* 'pick door' */
	EXIT_DATA       *pexit;
	EXIT_DATA       *pexit_rev;
	ROOM_INDEX_DATA *to_room;

	pexit = ch->in_room->exit[door];
      if (!IS_SET( pexit->exit_info, EX_HIDDEN)
          || IS_SET( race_table[ch->race].race_abilities, RACE_DETECT_HIDDEN_EXIT)
          || IS_SET( ch->act, PLR_HOLYLIGHT) )
      {
	  if ( !IS_SET( pexit->exit_info, EX_CLOSED )    )
	     { send_to_char(C_DEFAULT, "It's not closed.\n\r",        ch ); return SKPELL_MISSED; }
	  if ( pexit->key < 0 )
	     { send_to_char(C_DEFAULT, "It can't be picked.\n\r",     ch ); return SKPELL_MISSED; }
	  if ( !IS_SET( pexit->exit_info, EX_LOCKED )    )
	     { send_to_char(C_DEFAULT, "It's already unlocked.\n\r",  ch ); return SKPELL_MISSED; }
	  if (  IS_SET( pexit->exit_info, EX_PICKPROOF ) )
	     { send_to_char(C_DEFAULT, "You failed.\n\r",             ch ); return SKPELL_MISSED; }

	  REMOVE_BIT( pexit->exit_info, EX_LOCKED );
	  send_to_char(C_DEFAULT, "*Click*\n\r", ch );
	  act(C_DEFAULT, "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	  eprog_pick_trigger( pexit, ch->in_room, ch );

	  /* pick the other side */
	  if (   ( to_room   = pexit->to_room               )
	      && ( pexit_rev = to_room->exit[rev_dir[door]] )
	      && pexit_rev->to_room == ch->in_room )
	  {
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	  }
      }
      else
      {
        act(AT_GREY, "I see no door $T here.", ch, NULL, arg, TO_CHAR );
		return SKPELL_BOTCHED;
      }
    }
    return SKPELL_NO_DAMAGE;
}




void do_stand( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED( ch, AFF_SLEEP ) )
	    { send_to_char(AT_CYAN, "You can't wake up!\n\r", ch ); return; }

	send_to_char(AT_CYAN, "You wake and stand up.\n\r", ch );
	act(AT_CYAN, "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	rprog_wake_trigger( ch->in_room, ch );
	break;

    case POS_RESTING:
	send_to_char(AT_CYAN, "You stand up.\n\r", ch );
	act(AT_CYAN, "$n stands up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	rprog_wake_trigger( ch->in_room, ch );
	break;

    case POS_GHOST:
        send_to_char(AT_CYAN, "You are a ghost, and can't do that!\n\r", ch );
        break;

    case POS_FIGHTING:
	send_to_char(AT_CYAN, "You are already fighting!\n\r",  ch );
	break;

    case POS_STANDING:
	send_to_char(AT_CYAN, "You are already standing.\n\r",  ch );
	break;
    }

    return;
}



void do_rest( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char(AT_CYAN, "You wake up and start resting.\n\r", ch );
	act(AT_CYAN, "$n wakes up and rests.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_RESTING;
	rprog_rest_trigger( ch->in_room, ch );
	break;

    case POS_RESTING:
	send_to_char(AT_CYAN, "You are already resting.\n\r",   ch );
	break;

    case POS_FIGHTING:
	send_to_char(AT_CYAN, "Not while you're fighting!\n\r", ch );
	break;

    case POS_GHOST:
	send_to_char(AT_CYAN, "You are a ghost, and can't do that!\n\r", ch );
	break;

    case POS_STANDING:
	send_to_char(AT_CYAN, "You rest.\n\r", ch );
	act(AT_CYAN, "$n rests.", ch, NULL, NULL, TO_ROOM );
	rprog_rest_trigger( ch->in_room, ch );
	ch->position = POS_RESTING;
	break;
    }

    return;
}



void do_sleep( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	send_to_char(AT_CYAN, "You are already sleeping.\n\r",  ch );
	break;

    case POS_RESTING:
    case POS_STANDING: 
	send_to_char(AT_CYAN, "You sleep.\n\r", ch );
	act(AT_CYAN, "$n sleeps.", ch, NULL, NULL, TO_ROOM );
	rprog_sleep_trigger( ch->in_room, ch );
	ch->position = POS_SLEEPING;
	break;

    case POS_GHOST:
        send_to_char(AT_CYAN, "You are a ghost, and can't do that!\n\r", ch );
        break; 

    case POS_FIGHTING:
	send_to_char(AT_CYAN, "Not while you're fighting!\n\r", ch );
	break;
    }

    return;
}



void do_wake( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( ch->position == POS_GHOST )
    {
        send_to_char(AT_CYAN, "You are a ghost, and can't do that!\n\r", ch );
    }

    if ( !IS_AWAKE( ch ) )
	{ send_to_char(AT_CYAN, "You are asleep yourself!\n\r",       ch ); return; }

    if ( !( victim = get_char_room( ch, arg ) ) )
	{ send_to_char(AT_CYAN, "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE( victim ) )
	{ act(AT_CYAN, "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED( victim, AFF_SLEEP ) )
	{ act(AT_CYAN, "You can't wake $M!",   ch, NULL, victim, TO_CHAR ); return; }

    victim->position = POS_STANDING;
    act(AT_CYAN, "You wake $M.",  ch, NULL, victim, TO_CHAR );
    act(AT_CYAN, "$n wakes you.", ch, NULL, victim, TO_VICT );
    rprog_wake_trigger( victim->in_room, victim );
    return;
}



int do_sneak( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;

    send_to_char(AT_LBLUE, "You attempt to move silently.\n\r", ch );
    affect_strip( ch, sn );

	af.type      = sn;
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );
   
    return SKPELL_NO_DAMAGE;
}



int skill_hide( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;

    if ( target_name[0] != '\0' )
    {
        do_hide_obj(ch, target_name);
        return SKPELL_NO_DAMAGE;
    }

	if (is_affected(ch, sn))
	{
		send_to_char(AT_BLUE, "You are already hiding the best you can!\n\r", ch);
		return SKPELL_BOTCHED;
	}

	af.type      = sn;
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_HIDE;
	affect_to_char( ch, &af );

    return SKPELL_NO_DAMAGE;
}



/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, skill_lookup("invis")			);
    affect_strip ( ch, skill_lookup("improved invis")	);
    affect_strip ( ch, skill_lookup("mass invis")		);
    affect_strip ( ch, skill_lookup("sneak")			);
    affect_strip ( ch, skill_lookup("shadow")           );
    affect_strip ( ch, skill_lookup("hide")             );
    affect_strip ( ch, skill_lookup("cloaking")         );
    affect_strip ( ch, skill_lookup("improved hide")	);
    affect_strip ( ch, skill_lookup("stealth")			);

    REMOVE_BIT   ( ch->affected_by2, AFF_IMPROVED_INVIS	);
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE	);
    REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE		);
    REMOVE_BIT   ( ch->affected_by2, AFF_CLOAKING	);
    REMOVE_BIT   ( ch->affected_by3, AFF_IMPROVED_HIDE  );
    REMOVE_BIT   ( ch->affected_by3, AFF_STEALTH	);
    send_to_char(AT_WHITE, "Ok.\n\r", ch );
    return;
}

void do_reality( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, skill_lookup("shadow plane")	);
    REMOVE_BIT   ( ch->affected_by2, AFF_SHADOW_PLANE	);
    act(AT_BLOOD, "$n returns to reality!", ch, NULL, NULL, TO_ROOM );
    send_to_char(AT_BLOOD, "You return to reality once again.\n\r", ch );
    return;
}

void do_noob_recall( CHAR_DATA *ch, char* argument )
{
	if (ch->level > 10)
	{
		send_to_char(AT_WHITE, "You are too high level to use the recall command.\n\r", ch);
		return;
	}
	if (ch->fighting)
	{
		send_to_char(AT_WHITE, "You cannot recall while fighting!\n\r", ch);
		return;
	}
	do_recall(  ch, argument );
	return;
}

void do_recall( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *victim;
    CHAR_DATA       *pet;
    ROOM_INDEX_DATA *location;
    char             buf [ MAX_STRING_LENGTH ];
    int              place;
    char             name[ MAX_STRING_LENGTH ];
    CLAN_DATA       *pClan;
    char	     arg [ MAX_INPUT_LENGTH ];
            
 /*   OBJ_DATA	    *obj; */

    argument = one_argument( argument, arg );
    place = ch->in_room->area->recall;  /* stuck this here to 
                                          get rid of compiler warnings */
    if ( arg[0] == '\0' )
    {
	if (!(pClan=get_clan_index(ch->clan)))
	{
	    ch->clan = 0;
	    pClan=get_clan_index(ch->clan);
	}

	sprintf( name, "%s", pClan->deity );
	
        if ( ( ch->clan != 0 ) && ( ch->combat_timer < 1 ))
	{
            place = pClan->recall;
	}
        else
	{
	    if( ch->in_room->area->recall > 1 )
	    {
		place = ch->in_room->area->recall;
	    }
	    else
	    {
		place = get_religion_index(ch->religion)->recall;
	    }
	}
    }

    if ( !( location = get_room_index( place ) ) )
    {
	send_to_char(C_DEFAULT, "You are completely lost.\n\r", ch );
 	return;
    }

    act(C_DEFAULT, "$n prays for transportation!", ch, NULL, NULL, TO_ROOM );

    if ( ch->in_room == location )
	return;

    if ( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
	|| IS_AFFECTED( ch, AFF_CURSE ) )
    {
	act(C_DEFAULT, "$T has forsaken you.", ch, NULL, name, TO_CHAR );
	return;
    }

    if ( !IS_SET( ch->in_room->area->area_flags, AREA_PRESENT ) )
    {
	send_to_char(C_DEFAULT, "Sorry, you can not recall while in the future or past.\n\r", ch );
	return;
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
	    return;
	}

	lose = ( ch->desc ) ? 100 : 200;
	gain_exp( ch, 0 - lose );
	sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
	send_to_char(C_DEFAULT, buf, ch );
	stop_fighting( ch, TRUE );
    }

    for ( pet = ch->in_room->people; pet; pet = pet->next_in_room )
    {
      if ( IS_NPC( pet ) )
        if ( IS_SET( pet->act, ACT_PET ) && ( pet->master == ch ) )
        {
          if ( pet->fighting )
            stop_fighting( pet, TRUE );
          break;
        }
    }

    if (ch->pkill && (ch->combat_timer > 0) && IS_SET(location->room_flags, ROOM_SAFE))
    {
       send_to_char(AT_RED, "The gods frown at your bloodthirst, and do not hear your prayer!", ch);
       return;
    }

    act(C_DEFAULT, "$n disappears.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act(C_DEFAULT, "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    if ( pet )
    {
      act( C_DEFAULT, "$n disappears.", pet, NULL, NULL, TO_ROOM );
      char_from_room( pet );
      char_to_room( pet, location );
      act(C_DEFAULT, "$n appears in the room.", pet, NULL, NULL, TO_ROOM );
    }

    return;
}



void do_train( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    char      *pOutput;
    char       buf [ MAX_STRING_LENGTH ];
    bool       ok = FALSE;
    int       *pAbility;
    int        cost;
    int        bone_flag = 0; /*Added for training of hp ma mv - Don't like this, partially rewritten by Tyrion, adds bp */
    int        trainvaluestr;
    int        trainvalueint;
    int        trainvaluewis;
    int        trainvaluedex;
    int        trainvaluecon;
    int		maxHitPoints;

    trainvaluestr = 35;
    trainvalueint = 35;
    trainvaluewis = 35;
    trainvaluedex = 35;
    trainvaluecon = 35;

    if ( class_table[ch->class].attr_prime == APPLY_STR )
    {
        trainvaluestr = 40;
    }
    if ( class_table[ch->class].attr_prime == APPLY_INT )
    {
        trainvalueint = 40;  
    }
    if ( class_table[ch->class].attr_prime == APPLY_WIS )
    {
        trainvaluewis = 40;
    }
    if ( class_table[ch->class].attr_prime == APPLY_DEX )  
    {
        trainvaluedex = 40;
    }
    if ( class_table[ch->class].attr_prime == APPLY_CON )
    {
        trainvaluecon = 40;
    }


    if ( IS_NPC( ch ) )
	return;

    /*
     * Check for trainer.
     */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
	if ( IS_NPC( mob ) && IS_SET( mob->act, ACT_TRAIN ) )
	    break;
    }

    if ( !mob )
    {
	send_to_char(AT_WHITE, "You can't do that here.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "You have %d practice sessions.\n\r", ch->practice );
	send_to_char(AT_CYAN, buf, ch );
	argument = "foo";
    }

    cost = 5;

    if ( !str_cmp( argument, "str" ) )
    {
	if ( class_table[ch->class].attr_prime == APPLY_STR )
	    cost    = 3;
	pAbility    = &ch->pcdata->perm_str;
	pOutput     = "strength";
    }

    else if ( !str_cmp( argument, "int" ) )
    {
	if ( class_table[ch->class].attr_prime == APPLY_INT )
	    cost    = 3;
	pAbility    = &ch->pcdata->perm_int;
	pOutput     = "intelligence";
    }

    else if ( !str_cmp( argument, "wis" ) )
    {
	if ( class_table[ch->class].attr_prime == APPLY_WIS )
	    cost    = 3;
	pAbility    = &ch->pcdata->perm_wis;
	pOutput     = "wisdom";
    }

    else if ( !str_cmp( argument, "dex" ) )
    {
	if ( class_table[ch->class].attr_prime == APPLY_DEX )
	    cost    = 3;
	pAbility    = &ch->pcdata->perm_dex;
	pOutput     = "dexterity";
    }

    else if ( !str_cmp( argument, "con" ) )
    {
	if ( class_table[ch->class].attr_prime == APPLY_CON )
	    cost    = 3;
	pAbility    = &ch->pcdata->perm_con;
	pOutput     = "constitution";
    }

    /* ---------------- By Bonecrusher ------------------- */

    else if ( !str_cmp( argument, "hp" ) )
    {
 	    cost    = 1;
	bone_flag   = 1;
        pAbility    = &ch->max_hit;
        pOutput     = "hit points";
    }
	    
    else if ( !str_cmp( argument, "mana" ) )
    {
 	    cost    = 1;
	bone_flag   = 2;
        pAbility    = &ch->max_mana;
        pOutput     = "mana points";
    }

    else if ( !str_cmp( argument, "move" ) )
    {
 	    cost    = 1;
	bone_flag   = 2;
        pAbility    = &ch->max_move;
        pOutput     = "move points";
    }

    else if ( !str_cmp( argument, "blood" ) )
    {
	   cost     = 1;
	bone_flag   = 1;
	pAbility    = &ch->max_bp;
	pOutput     = "blood points";
    }

    /* --------------------------------------------*/

    else
    {
        strcpy( buf, "You can train:" );
        if ( ch->pcdata->perm_str < trainvaluestr + race_table[ ch->race ].str_mod )
            strcat( buf, " str" );
        if ( ch->pcdata->perm_int < trainvalueint + race_table[ ch->race ].int_mod )
            strcat( buf, " int" );
        if ( ch->pcdata->perm_wis < trainvaluewis + race_table[ ch->race ].wis_mod )
            strcat( buf, " wis" );
        if ( ch->pcdata->perm_dex < trainvaluedex + race_table[ ch->race ].dex_mod )
            strcat( buf, " dex" );
        if ( ch->pcdata->perm_con < trainvaluecon + race_table[ ch->race ].con_mod )
            strcat( buf, " con" );

	strcat( buf, " hp mana move blood" );

	if ( buf[strlen( buf )-1] != ':' )
	{
	    strcat( buf, ".\n\r" );
	    send_to_char(AT_CYAN, buf, ch );
	}

	return;
    }

    if ( bone_flag == 0 )
      {
        if ( !str_cmp( argument, "str" ) )
        {
            if ( *pAbility < trainvaluestr + race_table[ ch->race ].str_mod )
              {
                ok = TRUE;
              }
        }
        else if ( !str_cmp( argument, "int" ) )
        {
            if ( *pAbility < trainvalueint + race_table[ ch->race ].int_mod )
              {
                ok = TRUE;
              }
        }
        else if ( !str_cmp( argument, "wis" ) )
        {
            if ( *pAbility < trainvaluewis + race_table[ ch->race ].wis_mod )
              {
                ok = TRUE;
              }
        }
        else if ( !str_cmp( argument, "dex" ) )
        {
            if ( *pAbility < trainvaluedex + race_table[ ch->race ].dex_mod )
              {
                ok = TRUE;
              }
        }
        else if ( !str_cmp( argument, "con" ) )
        {
            if ( *pAbility < trainvaluecon + race_table[ ch->race ].con_mod )
              {
                ok = TRUE;
              }
        }

        if ( !ok )
        {
            act(AT_CYAN, "Your $T is already at maximum.",
                ch, NULL, pOutput, TO_CHAR );
            return;
        }
    }


    if ( cost > ch->practice )
    {
	send_to_char(AT_CYAN, "You don't have enough practices.\n\r", ch );
	return;
    }

    if ( ch->level == LEVEL_DEMIGOD && ch->exp >= MAX_EXPERIENCE )
    {
	maxHitPoints = 55000;
    }
    else
    {
	maxHitPoints = 35000;
    }

    if( pOutput == "hit points" && ch->max_hit >= maxHitPoints )
    {
	send_to_char( C_DEFAULT, "You are already at your maximum hitpoints.\n\r", ch );
	return;
    }

    ch->practice        	-= cost;

    if ( bone_flag == 0 )
        *pAbility		+= 1;
    else if ( bone_flag == 1 )
        *pAbility               += dice( 1, 2 );
    else
        *pAbility               += dice( 1, 5 );

    if ( bone_flag == 0 )
    {
        act(AT_CYAN, "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
        act(AT_CYAN, "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
	return;
    }

    act(AT_CYAN, "Your $T increase!", ch, NULL, pOutput, TO_CHAR );
    act(AT_CYAN, "$n's $T increase!", ch, NULL, pOutput, TO_ROOM );

    return;
}

int skill_chameleon ( int sn, int level, CHAR_DATA *ch, void *vo )
{

    send_to_char(AT_DGREY, "You attempt to blend in with your surroundings.\n\r", ch);

    if ( IS_AFFECTED( ch, AFF_HIDE ) )
        REMOVE_BIT( ch->affected_by, AFF_HIDE );

    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 ) )
        SET_BIT( ch->affected_by, AFF_HIDE );

    return SKPELL_NO_DAMAGE;
}

int skill_occulutus ( int sn, int level, CHAR_DATA *ch, void *vo )
{

    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
        send_to_char(AT_BLOOD, "Your eyes already see beyond.\n\r", ch);
	return SKPELL_MISSED;

   
	af.type      = sn;
	af.duration  = 10;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_INVIS;
	affect_to_char( ch, &af );
        af.bitvector = AFF_DETECT_HIDDEN;
	affect_to_char( ch, &af );
	
	send_to_char(AT_BLOOD, "Your eyes now see beyond the grave.\n\r", ch );

    return SKPELL_NO_DAMAGE;
}

int skill_heighten ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
        return SKPELL_MISSED;

	af.type      = sn;
	af.duration  = (level * 2) / 4;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_INVIS;
	affect_to_char( ch, &af );

	af.bitvector = AFF_DETECT_HIDDEN;
	affect_to_char( ch, &af );
	
	af.bitvector = AFF_INFRARED;
	affect_to_char( ch, &af );
	
	send_to_char(AT_BLUE, "Your senses are heightened.\n\r", ch );
    
    return SKPELL_NO_DAMAGE;

}

int skill_quickness ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
        return SKPELL_MISSED;


	af.type      = sn;
	af.duration  = 24;
	af.modifier  = 1 + (ch->level >= 18) + (ch->level >= 25) + (ch->level >= 32);
	af.location  = APPLY_DEX;
	af.bitvector = AFF_HASTE;
	affect_to_char( ch, &af );

	send_to_char(AT_BLUE, "You begin to move quickly.\n\r", ch );
    
    return SKPELL_NO_DAMAGE;

}

int skill_shadow ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;

    send_to_char(AT_DGREY, "You attempt to move in the shadows.\n\r", ch );
    affect_strip( ch, sn );

	af.type      = sn;
	af.duration  = ch->level;
	af.modifier  = APPLY_NONE;
	af.location  = 0;
	af.bitvector = AFF_SNEAK;
	affect_to_char( ch, &af );

	return SKPELL_NO_DAMAGE;

}

/*
 * Bash code by Thelonius for EnvyMud (originally bash_door)
 * Damage modified using Morpheus's code
 */
void do_enter ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    OBJ_DATA *obj;
    char      arg1 [ MAX_INPUT_LENGTH ];
    int       destination;
    ROOM_INDEX_DATA *location;
    argument = one_argument( argument, arg1 );

    if( ch->position == POS_FIGHTING )
    {
	send_to_char(C_DEFAULT, "You can not enter something while fighting!\n\r", ch );
	return;
    }

    if ( arg1[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Enter what?\n\r", ch );
	return;
    }
    if ( !( obj = get_obj_list( ch, arg1, ch->in_room->contents ) ) )
      {
       act(AT_WHITE, "You cannot enter that", ch, NULL, NULL, TO_CHAR );
       return;
      }
    if ( obj->item_type != ITEM_PORTAL )
      {
       send_to_char(AT_WHITE, "There is nothing to enter here.\n\r", ch );
       return;
      }
    in_room = ch->in_room;
    destination = obj->value[0];
/*
    if (destination >= 2500 && destination <= 2599)
        destination = 8;
*/
    if ( !( location = get_room_index( destination ) ) )
    {
	act(AT_BLUE, "You try to enter $p but can't.", ch, obj, NULL, TO_CHAR );
	return;
    }

    if ( IS_SET(location->room_flags, ROOM_SAFE) && ch->pkill && (ch->combat_timer > 0))
    {
        send_to_char(AT_RED, "Your blood is too hot to enter that portal!", ch);
        return;
    }

    if ( IS_SET(ch->act2, PLR_RELQUEST) && destination != ROOM_VNUM_TEMPLE )
    {
	DESCRIPTOR_DATA* d;
	if (ch->leader || ch->master)
	{
		send_to_char(AT_WHITE, "Only your group leader can advance the quest!\n\r", ch);
		return;
	}
	if (!(ch->rquestmob[ch->relquest_level] == 0 && ch->rquestobj[ch->relquest_level] == 0))
	{
		send_to_char(AT_WHITE, "You haven't finished the objective for this level yet!\n\r", ch);
		return;
	}
	ch->relquest_level++;
	for (d=descriptor_list; d; d = d->next)
        {
        	if ( d->connected != CON_PLAYING )
                        continue;
                if (is_same_group(d->character, ch))
                {
			if (ch == d->character)
				continue;

			char_from_room(d->character);
			char_to_room(d->character, ch->in_room);
			send_to_char(AT_WHITE, "A strange feeling comes over you as your advance in your quest!\n\r", d->character);
			d->character->relquest_level++;
                }
        }
	
    }

    act( AT_BLUE, "You enter the $p.",ch,obj,NULL, TO_CHAR );
    act(AT_BLUE, "$n enters the $p and is gone.", ch, obj, NULL, TO_ROOM );
    rprog_exit_trigger( ch->in_room, ch );
    char_from_room( ch );
    char_to_room( ch, location );
    send_to_char(AT_BLUE, "You arrive at your new destination.\n\r", ch );
    act(AT_BLUE, "$n enters the area from the $p.", ch, obj, NULL, TO_ROOM);
    if( ch->mounted > 0 )
    {
	act(AT_BLUE, "$n is mounted on $T.", ch, NULL, ch->mountshort, TO_ROOM );
    }
    do_look( ch, "auto" );
    for ( fch = in_room->people; fch; fch = fch_next )
    {
        fch_next = fch->next_in_room;

        if ( fch->deleted )
	    continue;
      
	if ( fch->master == ch && fch->position == POS_STANDING )
	{
	    act(AT_WHITE, "You follow $N through $p.", fch, obj, ch, TO_CHAR );
	    if (IS_NPC(fch) || !IS_SET(fch->act2, PLR_RELQUEST))
		    do_enter( fch, arg1 );
	    else
	    {
		REMOVE_BIT(fch->act2, PLR_RELQUEST);
		do_enter( fch, arg1 );
		SET_BIT(fch->act2, PLR_RELQUEST);  
	    }
	}
    }
    rprog_enter_trigger( ch->in_room, ch );

    if (IS_SET(ch->act2, PLR_RELQUEST))
    {
	CHAR_DATA* gch;
	for (gch = ch->in_room->people; gch; gch = gch->next_in_room)
	{
		if (IS_NPC(gch)) continue;
 
		interpret(gch, "relquest info");
	}
    }
    return;
}

int skill_bash ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *gch;
    int        door;
    int		dam;


    if ( target_name[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Bash what?\n\r", ch );
	return SKPELL_BOTCHED;
    }

    if ( ch->fighting )
    {
	send_to_char(C_DEFAULT, "You can't break off your fight.\n\r", ch );
	return SKPELL_BOTCHED;
    }

    if ( ( door = find_door( ch, target_name ) ) >= 0 )
    {
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA       *pexit;
	EXIT_DATA       *pexit_rev;
	int              chance;

	pexit = ch->in_room->exit[door];
	if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )
	{
	    send_to_char(C_DEFAULT, "Calm down.  It is already open.\n\r", ch );
	    return SKPELL_MISSED;
	}

	WAIT_STATE( ch, skill_table[sn].beats );

	chance = ( ch->pcdata->learned[sn] / 10 );
	if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
	    chance /= 2;

	if ( IS_SET( pexit->exit_info, EX_BASHPROOF ) )
	{
	    act(C_DEFAULT, "WHAAAAM!!!  You bash against the $d, but it doesn't budge.",
		ch, NULL, pexit->keyword, TO_CHAR );
	    act(C_DEFAULT, "WHAAAAM!!!  $n bashes against the $d, but it holds strong.",
		ch, NULL, pexit->keyword, TO_ROOM );
	    dam = number_range( get_curr_str(ch) * 10, get_curr_str(ch) * 20);
	    dam = UMAX(dam, ch->hit / 3);
	    damage( ch, ch, dam, sn );
	    
		return SKPELL_NO_DAMAGE;
	}

	if ( ( get_curr_str( ch ) >= 40 )
	    && number_percent( ) < ( chance/2 + ( get_curr_str( ch ) - 20 ) ) )
	{
	    /* Success */

	    REMOVE_BIT( pexit->exit_info, EX_CLOSED );
	    if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
	        REMOVE_BIT( pexit->exit_info, EX_LOCKED );
	    
	    SET_BIT( pexit->exit_info, EX_BASHED );

	    act(C_DEFAULT, "Crash!  You bashed open the $d!", ch, NULL, pexit->keyword, TO_CHAR );
	    act(C_DEFAULT, "$n bashes open the $d!",          ch, NULL, pexit->keyword, TO_ROOM );

	    damage( ch, ch, number_range( 100, 1000 ), sn );

	    /* Bash through the other side */
	    if (   ( to_room   = pexit->to_room               )
		&& ( pexit_rev = to_room->exit[rev_dir[door]] )
		&& pexit_rev->to_room == ch->in_room        )
	    {
		CHAR_DATA *rch;

		REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
		if ( IS_SET( pexit_rev->exit_info, EX_LOCKED ) )
		    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );

		SET_BIT( pexit_rev->exit_info, EX_BASHED );

		for ( rch = to_room->people; rch; rch = rch->next_in_room )
		{
		    if ( rch->deleted )
		        continue;
		    act(C_DEFAULT, "The $d crashes open!",
			rch, NULL, pexit_rev->keyword, TO_CHAR );
		}

	    }
	}
	else
	{
	    /* Failure */
	    
	    act(C_DEFAULT, "OW!  You bash against the $d, but it doesn't budge.",
		ch, NULL, pexit->keyword, TO_CHAR );
	    act(C_DEFAULT, "$n bashes against the $d, but it holds strong.",
		ch, NULL, pexit->keyword, TO_ROOM );
	    damage( ch, ch, ( ch->max_hit / 10 ), sn );
	}
    }

    /*
     * Check for "guards"... anyone bashing a door is considered as
     * a potential aggressor, and there's a 25% chance that mobs
     * will do unto before being done unto.
     */
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
        if ( gch->deleted )
	    continue;
	if ( IS_AWAKE( gch )
	    && !gch->fighting
	    && ( IS_NPC( gch ) && !IS_AFFECTED( gch, AFF_CHARM ) )
	    && ( ch->level - gch->level <= 4 )
	    && number_bits( 2 ) == 0 )
	    multi_hit( gch, ch, TYPE_UNDEFINED );
    }

    return SKPELL_NO_DAMAGE;

}

/* Snare skill by Binky for EnvyMud */
int skill_snare( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA   *victim;
    AFFECT_DATA  af;

    return SKPELL_MISSED;

    /*
     *  First, this checks for case of no second argument (valid only
     *  while fighting already).  Later, if an argument is given, it
     *  checks validity of argument.  Unsuccessful snares flow through
     *  and receive messages at the end of the function.
     */

    if ( target_name[0] == '\0' )
    {
        if ( !( victim = ch->fighting ) )
        {
            send_to_char(AT_WHITE, "Ensnare whom?\n\r", ch );
            return SKPELL_BOTCHED;
        }
        /* No argument, but already fighting: valid use of snare */
        WAIT_STATE( ch, skill_table[sn].beats );

        /* Only appropriately skilled PCs and uncharmed mobs */
        if ( ( IS_NPC( ch ) && !IS_AFFECTED( ch, AFF_CHARM ) )
             || ( !IS_NPC( ch )
             && number_percent( ) < ( ch->pcdata->learned[sn] / 10 ) ) )
        {
            affect_strip( victim, sn );

            af.type      = sn;
            af.duration  = 1 + ( ( ch->level ) / 8 );
            af.location  = APPLY_NONE;
            af.modifier  = 0;
            af.bitvector = AFF_ANTI_FLEE;

            affect_to_char( victim, &af );

            act(AT_WHITE, "You have ensnared $M!", ch, NULL, victim, TO_CHAR    );
            act(AT_WHITE, "$n has ensnared you!",  ch, NULL, victim, TO_VICT    );
            act(AT_WHITE, "$n has ensnared $N.",   ch, NULL, victim, TO_NOTVICT );
        }
        else
        {
            act(AT_WHITE, "You failed to ensnare $M.  Uh oh!",
                ch, NULL, victim, TO_CHAR    );
            act(AT_WHITE, "$n tried to ensnare you!  Get $m!",
                ch, NULL, victim, TO_VICT    );
            act(AT_WHITE, "$n attempted to ensnare $N, but failed!",
                ch, NULL, victim, TO_NOTVICT );
        }
    }
    else                                /* argument supplied */
    {
        if ( !( victim = get_char_room( ch, target_name ) ) )
        {
            send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
            return SKPELL_BOTCHED;
        }

        if ( victim != ch->fighting ) /* TRUE if not fighting, or fighting  */
        {                             /* if person other than victim        */
            if ( ch->fighting )       /* TRUE if fighting other than vict.  */
            {
                send_to_char(AT_WHITE,
                    "Take care of the person you are fighting first!\n\r",
                             ch );
                return SKPELL_BOTCHED;
            }
            WAIT_STATE( ch, skill_table[sn].beats );

            /* here, arg supplied, ch not fighting */
            /* only appropriately skilled PCs and uncharmed mobs */
            if ( ( IS_NPC( ch ) && !IS_AFFECTED( ch, AFF_CHARM ) )
                || ( !IS_NPC( ch )
                && number_percent( ) < ( ch->pcdata->learned[sn] / 10 ) ) )
            {
                affect_strip( victim, sn );

                af.type      = sn;
                af.duration  = 3 + ( (ch->level ) / 8 );
                af.location  = APPLY_NONE;
                af.modifier  = 0;
                af.bitvector = AFF_ANTI_FLEE;

                affect_to_char( victim, &af );

                act(AT_WHITE, "You have ensnared $M!", ch, NULL, victim, TO_CHAR    );
                act(AT_WHITE, "$n has ensnared you!",  ch, NULL, victim, TO_VICT    );
                act(AT_WHITE, "$n has ensnared $N.",   ch, NULL, victim, TO_NOTVICT );
            }
            else
            {
                act(AT_WHITE, "You failed to ensnare $M.  Uh oh!",
                    ch, NULL, victim, TO_CHAR    );
                act(AT_WHITE, "$n tried to ensnare you!  Get $m!",
                    ch, NULL, victim, TO_VICT    );
                act(AT_WHITE, "$n attempted to ensnare $N, but failed!",
                    ch, NULL, victim, TO_NOTVICT );
            }
            if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
            {
                /* go for the one who wanted to fight :) */
                multi_hit( victim, ch->master, TYPE_UNDEFINED );
            }
            else /* we are already fighting the intended victim */
            {
                multi_hit( victim, ch, TYPE_UNDEFINED );
            }
        }
        else
        {
            WAIT_STATE( ch, skill_table[sn].beats );

            /* charmed mobs not allowed to do this */
            if ( ( IS_NPC( ch ) && !IS_AFFECTED( ch, AFF_CHARM ) )
                || ( !IS_NPC( ch )
                && number_percent( ) < ( ch->pcdata->learned[sn] / 10 ) ) )
            {
                affect_strip( victim, sn );

                af.type      = sn;
                af.duration  = 1 + ( ( ch->level ) / 8 );
                af.location  = APPLY_NONE;
                af.modifier  = 0;
                af.bitvector = AFF_ANTI_FLEE;

                affect_to_char( victim, &af );

                act(AT_WHITE, "You have ensnared $M!", ch, NULL, victim, TO_CHAR    );
                act(AT_WHITE, "$n has ensnared you!",  ch, NULL, victim, TO_VICT    );
                act(AT_WHITE, "$n has ensnared $N.",   ch, NULL, victim, TO_NOTVICT );
            }
            else
            {
                act(AT_WHITE, "You failed to ensnare $M.  Uh oh!",
                    ch, NULL, victim, TO_CHAR    );
                act(AT_WHITE, "$n tried to ensnare you!  Get $m!",
                    ch, NULL, victim, TO_VICT    );
                act(AT_WHITE, "$n attempted to ensnare $N, but failed!",
                    ch, NULL, victim, TO_NOTVICT );
            }
        }
    }

    return SKPELL_NO_DAMAGE;
}



/* Untangle by Thelonius for EnvyMud */
int skill_untangle( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA   *victim;

    return SKPELL_MISSED;

    if ( target_name[0] == '\0' )
        victim = ch;
    else if ( !( victim = get_char_room( ch, target_name ) ) )
    {
            send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
            return SKPELL_BOTCHED;
    }

    if ( !IS_AFFECTED( victim, AFF_ANTI_FLEE ) )
        return SKPELL_MISSED;

    if ( ( IS_NPC( ch ) && !IS_AFFECTED( ch, AFF_CHARM ) )
        || ( !IS_NPC( ch )
            && number_percent( ) < ( ch->pcdata->learned[sn] / 10 ) ) )
    {
        affect_strip( victim, sn );

        if ( victim != ch )
        {
            act(AT_WHITE, "You untangle $N.",  ch, NULL, victim, TO_CHAR    );
            act(AT_WHITE, "$n untangles you.", ch, NULL, victim, TO_VICT    );
            act(AT_WHITE, "$n untangles $n.",  ch, NULL, victim, TO_NOTVICT );
        }
        else
        {
            send_to_char(AT_WHITE, "You untangle yourself.\n\r", ch );
            act(AT_WHITE, "$n untangles $mself.", ch, NULL, NULL, TO_ROOM );
        }

    }

	return SKPELL_NO_DAMAGE;
}


/* XORPHOX push/drag */
void do_push(CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  EXIT_DATA *pexit;
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *from_room;
  int door;
  char buf1[256], buf2[256], buf3[256];

  argument = one_argument(argument, arg1);
  one_argument(argument, arg2);

  if(arg1[0] == '\0')
  {
    send_to_char(AT_BLUE, "Push who what where?\n\r", ch);
    return;
  }

  if((victim = get_char_room(ch, arg1)) == NULL)
  {
    send_to_char(AT_BLUE, "They aren't here.\n\r", ch);
    return;
  }

    if(IS_AFFECTED4(ch, AFF_BURROW))
    {
        send_to_char(C_DEFAULT, "You can not push someone while burrowed!\n\r",ch);
	return;
    }

    if(IS_AFFECTED4(victim, AFF_BURROW))
    {
        send_to_char(C_DEFAULT, "You can not push someone who is burrowed!\n\r",ch);
	return;
    }

  if (victim->pkill == TRUE && victim->pkill_timer > 0)
  {
   ch->pkill_timer = 0;
   send_to_char(AT_BLUE, "That character is under protection!\n\r", ch);
   return;
  }

  if(victim->level > LEVEL_DEMIGOD)
  {
    act(AT_BLUE, "$N ignores you.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if ( !str_cmp( arg2, "n" ) || !str_cmp( arg2, "north" ) ) door = 0;
  else if ( !str_cmp( arg2, "e" ) || !str_cmp( arg2, "east"  ) ) door = 1;
  else if ( !str_cmp( arg2, "s" ) || !str_cmp( arg2, "south" ) ) door = 2;
  else if ( !str_cmp( arg2, "w" ) || !str_cmp( arg2, "west"  ) ) door = 3;
  else if ( !str_cmp( arg2, "u" ) || !str_cmp( arg2, "up"    ) ) door = 4;
  else if ( !str_cmp( arg2, "d" ) || !str_cmp( arg2, "down"  ) ) door = 5;
  else door = dice(1,6) - 1;

  if(ch == victim)
  {
    send_to_char(AT_BLUE, "You attempt to push yourself, oook.\n\r", ch);
    return;
  }

  if(victim->position != POS_STANDING)
  {
    send_to_char(AT_BLUE, "Can't push someone who is not standing.\n\r", ch);
    return;
  }

  if (!IS_NPC(victim) && victim->pcdata->craft_timer)
	destroy_craft(victim, FALSE);

  pexit = ch->in_room->exit[door];
  if( (pexit == NULL || IS_SET(pexit->exit_info, EX_CLOSED))
     ||( (!IS_SET(race_table[ch->race].race_abilities, RACE_DETECT_HIDDEN_EXIT)
          || !IS_SET(race_table[victim->race].race_abilities, RACE_DETECT_HIDDEN_EXIT) )
        && IS_SET( pexit->exit_info, EX_HIDDEN ) ) )
  {
    act(AT_BLUE, "There is no exit, but you push $M around anyways.", ch, NULL, victim, TO_CHAR);
    act(AT_BLUE, "$n pushes $N against a wall.", ch, NULL, victim, TO_NOTVICT);
    act(AT_BLUE, "$n pushes you against a wall, ouch.", ch, NULL, victim, TO_VICT);
    ch->pkill_timer = 0;
    return;
  }

  if(room_is_private(pexit->to_room)) {
    act(AT_BLUE, "The room is private, $M bounces right back at you.", ch, NULL, victim, TO_CHAR);
    act(AT_BLUE, "$n pushes $N, but he bounces right back at $n.", ch, NULL, victim, TO_NOTVICT);
    act(AT_BLUE, "$n pushes you, but you bounce right back at $n.", ch, NULL, victim, TO_VICT);
    ch->pkill_timer = 0;    
    return;
  }

  sprintf(buf1, "You slam into $N, pushing $M %s.", dir_name[door]);
  sprintf(buf2, "$n slams into $N, pushing $M %s.", dir_name[door]);
  sprintf(buf3, "$n slams into you, pushing you %s.", dir_name[door]);
  act(AT_BLUE, buf2, ch, NULL, victim, TO_NOTVICT );
  act(AT_BLUE, buf1, ch, NULL, victim, TO_CHAR );
  act(AT_BLUE, buf3, ch, NULL, victim, TO_VICT );
  from_room = victim->in_room;
  eprog_enter_trigger( pexit, victim->in_room, victim );
  char_from_room(victim);
  char_to_room(victim, pexit->to_room);

  act(AT_BLUE, "$n comes flying into the room.", victim, NULL, NULL, TO_ROOM);
  if ( (pexit = pexit->to_room->exit[rev_dir[door]]) &&
       pexit->to_room == from_room )
    eprog_exit_trigger( pexit, victim->in_room, victim );
  else
    rprog_enter_trigger( victim->in_room, victim );
  mprog_greet_trigger( victim );
 mprog_entry_trigger( victim );    
   ch->pkill_timer = 0;  
   return;
}

void do_drag(CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  EXIT_DATA *pexit;
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *from_room;
  int door;
  char buf1[256], buf2[256], buf3[256];

  argument = one_argument(argument, arg1);
  one_argument(argument, arg2);

  if(arg1[0] == '\0')
  {
    send_to_char(AT_BLUE, "Drag who what where?\n\r", ch);
    return;
  }

  if((victim = get_char_room(ch, arg1)) == NULL)
  {
    send_to_char(AT_BLUE, "They aren't here.\n\r", ch);
    return;
  }

    if(IS_AFFECTED4(ch, AFF_BURROW))
    {
        send_to_char(C_DEFAULT, "You can not push someone while burrowed!\n\r",ch);
	return;
    }

    if(IS_AFFECTED4(victim, AFF_BURROW))
    {
        send_to_char(C_DEFAULT, "You can not push someone who is burrowed!\n\r",ch);
	return;
    }

  if (victim->pkill == TRUE && victim->pkill_timer > 0)
  {
    ch->pkill_timer = 0;
    send_to_char(AT_BLUE, "That character is under protection!\n\r", ch);
    return;
  }

  if(victim->level > LEVEL_DEMIGOD)
  {
    act(AT_BLUE, "$N ignores you.", ch, NULL, victim, TO_CHAR);
    return;
  }

  if ( !str_cmp( arg2, "n" ) || !str_cmp( arg2, "north" ) ) door = 0;
  else if ( !str_cmp( arg2, "e" ) || !str_cmp( arg2, "east"  ) ) door = 1;
  else if ( !str_cmp( arg2, "s" ) || !str_cmp( arg2, "south" ) ) door = 2;
  else if ( !str_cmp( arg2, "w" ) || !str_cmp( arg2, "west"  ) ) door = 3;
  else if ( !str_cmp( arg2, "u" ) || !str_cmp( arg2, "up"    ) ) door = 4;
  else if ( !str_cmp( arg2, "d" ) || !str_cmp( arg2, "down"  ) ) door = 5;
  else door = dice(1,6) - 1;

  if(ch == victim)
  {
    send_to_char(AT_BLUE, "You attempt to drag yourself, oook.\n\r", ch);
    return;
  }

  if(victim->position == POS_STANDING)
  {
    send_to_char(AT_BLUE, "Can't drag someone who is standing.\n\r", ch);
    return;
  }
  pexit = ch->in_room->exit[door];
  if(pexit == NULL || IS_SET(pexit->exit_info, EX_CLOSED)
     || (!IS_SET( race_table[ch->race].race_abilities, RACE_DETECT_HIDDEN_EXIT)
          && IS_SET( pexit->exit_info, EX_HIDDEN) ) )
  {
    act(AT_BLUE, "There is no exit, but you drag $M around anyways.", ch, NULL, victim, TO_CHAR);
    act(AT_BLUE, "$n drags $N around the room.", ch, NULL, victim, TO_NOTVICT);
    act(AT_BLUE, "$n drags you around the room.", ch, NULL, victim, TO_VICT);
    ch->pkill_timer = 0;
    return;
  }

  if(room_is_private(pexit->to_room)) {
    act(AT_BLUE, "The room is private, but you drag $M around anyways.", ch, NULL, victim, TO_CHAR);
    act(AT_BLUE, "$n drags $N around the room.", ch, NULL, victim, TO_NOTVICT);
    act(AT_BLUE, "$n drags you around the room.", ch, NULL, victim, TO_VICT);
    ch->pkill_timer = 0;      
    return;
  }

  sprintf(buf1, "You get ahold of $N, dragging $M %s.", dir_name[door]);
  sprintf(buf2, "$n gets ahold of $N, dragging $M %s.", dir_name[door]);
  sprintf(buf3, "$n gets ahold of you, dragging you %s.", dir_name[door]);
  act(AT_BLUE, buf2, ch, NULL, victim, TO_NOTVICT);
  act(AT_BLUE, buf1, ch, NULL, victim, TO_CHAR);
  act(AT_BLUE, buf3, ch, NULL, victim, TO_VICT);
  ch->pkill_timer = 0;

  from_room = ch->in_room;
  eprog_enter_trigger( pexit, ch->in_room, ch );
  eprog_enter_trigger( pexit, victim->in_room, victim );
  char_from_room(victim);
  char_to_room(victim, pexit->to_room);
  act(AT_BLUE, "$N arrives, dragging $n with $M.", victim, NULL, ch, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, victim->in_room);
  if ( (pexit = pexit->to_room->exit[rev_dir[door]]) &&
       pexit->to_room == from_room )
  {
    eprog_exit_trigger( pexit, ch->in_room, ch );
    eprog_exit_trigger( pexit, victim->in_room, victim );
  }
  else
  {
    rprog_enter_trigger( ch->in_room, ch );
    rprog_enter_trigger( victim->in_room, victim );
  }
 mprog_greet_trigger(ch);
 mprog_entry_trigger(ch);
 mprog_greet_trigger(victim);
 mprog_entry_trigger(victim);
  return;
}
/* END */

void check_nofloor( CHAR_DATA *ch )
{
  EXIT_DATA *pexit;
  ROOM_INDEX_DATA *to_room;
  
  if ( IS_SET( ch->in_room->room_flags, ROOM_NOFLOOR ) 
      && ( pexit = ch->in_room->exit[5] )
      && ( to_room = pexit->to_room ) )
  {
    act( AT_RED, "You fall through where the floor should have been.", ch,
        NULL, NULL, TO_CHAR );
    act( C_DEFAULT, "$n falls down to the room below.", ch, NULL, NULL,
        TO_ROOM );
    damage( ch, ch, 5, TYPE_UNDEFINED );
    move_char( ch, 5 );
  }
  return;
}
int skill_shadow_walk( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;

    if ( ch->fighting )
    {
	send_to_char(C_DEFAULT, "Not while in combat.\n\r", ch );
	return SKPELL_BOTCHED;
    }
    if ( target_name[0] == '\0' )
    {
      send_to_char(AT_GREY, "Shadow Walk to whom?\n\r", ch );
      return SKPELL_BOTCHED;
    }

    if ( !( victim = get_char_world( ch, target_name ) )
	|| victim->in_room->area != ch->in_room->area
	|| IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
	|| IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  )
        || IS_SET( victim->in_room->room_flags, ROOM_NO_SHADOW )
        || (IS_SET( victim->in_room->room_flags, ROOM_SAFE) && ch->pkill && (ch->combat_timer>0))
        || IS_SET( ch->in_room->room_flags, ROOM_NO_SHADOW )
        || IS_SET( victim->in_room->area->area_flags, AREA_PROTOTYPE )
	|| IS_AFFECTED( victim, AFF_NOASTRAL )
        || IS_AFFECTED4(victim, AFF_DECEPTION) )
    {
	send_to_char(AT_GREY, "The shadows offer no path to that one.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PRESENT)
        && (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
        || IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
	send_to_char(AT_GREY, "The shadows offer no path to that one.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_FUTURE)
        && (IS_SET( victim->in_room->area->area_flags, AREA_PRESENT )
        || IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
	send_to_char(AT_GREY, "The shadows offer no path to that one.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PAST)
        && (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
        || IS_SET( victim->in_room->area->area_flags, AREA_PRESENT ) ) )
    {
	send_to_char(AT_GREY, "The shadows offer no path to that one.\n\r", ch );
	return SKPELL_MISSED;
    }

    act(AT_GREY, "$n steps into the shadows and is gone.", ch, NULL, NULL, TO_ROOM );
    if ( ch != victim )
    {
     char_from_room( ch );
     char_to_room( ch, victim->in_room );
    }
    act(AT_GREY, "$n steps forth from the shadows.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return SKPELL_NO_DAMAGE;
}

int skill_improved_hide( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;

    send_to_char(AT_BLUE, "You virtually vanish.\n\r", ch );

    if ( IS_AFFECTED3( ch, AFF_IMPROVED_HIDE ) )
	affect_strip( ch, sn);

	af.type		= sn;
	af.duration	= ch->level / 2;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_IMPROVED_HIDE;
	affect_to_char3( ch, &af );

    return SKPELL_NO_DAMAGE;
}

int skill_stealth( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;

    send_to_char( AT_BLUE, "You vanish completely.\n\r", ch );

    if ( IS_AFFECTED3( ch, AFF_STEALTH ) )
		affect_strip(ch, sn);
    
	af.type		= sn;
	af.duration	= ch->level / 2;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_STEALTH;
	affect_to_char3( ch, &af );
    

    return SKPELL_NO_DAMAGE;
}

void do_timequake( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    ROOM_INDEX_DATA *pRoomIndex;

    for( vch = char_list; vch; vch = vch-> next )
    {
	if( vch->deleted )
	    continue;

	if( IS_NPC(vch) )
	    continue;

        if( vch->level < 30)
	    continue;

	if( vch->level > 30 && vch->level < 61 && number_percent( ) < 6 )
	    continue;

	if( vch->level > 60 && vch->level < 91 && number_percent( ) < 21 )
	    continue;

        if( vch->level > 90 && number_percent( ) < 51 )
	    continue;

        if( IS_SET( vch->in_room->room_flags, ROOM_SAFE) )
	    continue;

	for( ; ; )
        {
	    pRoomIndex = get_room_index( number_range( 70, 32767 ) );
	    if( pRoomIndex )
		if((!IS_SET( pRoomIndex->area->area_flags, AREA_PROTOTYPE )
		   && !IS_SET( pRoomIndex->area->area_flags, AREA_NOQUEST )
		   && !IS_SET( pRoomIndex->area->area_flags, AREA_RANDOM   )) 
		   || (IS_SET(pRoomIndex->room_flags,ROOM_SAFE) && ch->pkill & (ch->combat_timer>0)))
		break;
	} 

	act( AT_PURPLE, "The air starts to sparkle, and $n is sucked into a vortex!", vch, NULL, NULL, TO_ROOM );

	char_from_room( vch );
	char_to_room( vch, pRoomIndex );
	sprintf(log_buf, "%s was caught in the Timequake!", vch->name );
        log_string(log_buf, CHANNEL_INFO, -1 );
	act( AT_PURPLE, "The air opens before you, and $n materializes from a purple vortex!", vch, NULL, NULL, TO_ROOM );
	send_to_char(AT_PURPLE, "You are sucked into the Timequake!\n\r", vch );
	do_look( vch, "auto" );
    }

    return;
}

int skill_disguise( int sn, int level, CHAR_DATA *ch, void *vo )
{
 /*   CHAR_DATA *victim;
    char arg[MAX_STRING_LENGTH]; */

    return SKPELL_NO_DAMAGE;
}

int skill_home_travel ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    return SKPELL_NO_DAMAGE;
}


void do_mount( CHAR_DATA *ch, char *argument )
{

    CHAR_DATA 	*mob;
    char	arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( C_DEFAULT, "Mount what?\n\r", ch );
	return;
    }

    if( ch->mounted > 0 )
    {
	send_to_char( C_DEFAULT, "You are already mounted.\n\r", ch );
	return;
    }
    
    mob = get_char_room( ch, arg );

    if( !mob )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
    } 
    
    if( !IS_NPC( mob ) )
    {
	send_to_char( C_DEFAULT, "You can not mount other players.\n\r", ch );
	return;
    }

    if( IS_SET( mob->affected_by4, AFF_MOUNTABLE ) )
    {
	    /* Mount */
	if( mob->level > ch->level)
	{
	    send_to_char( C_DEFAULT, "You can not mount something that is higher level than you.\n\r", ch );
	    return;
        }
	ch->mounted = mob->pIndexData->vnum;
	ch->mountshort = mob->pIndexData->short_descr;
	if( IS_AFFECTED( mob, AFF_CHARM ) )
	{
	    ch->mountcharmed = ch->level;
	}
	else
	{
	    ch->mountcharmed = 0;
	}
	act(AT_GREEN, "You mount $N.", ch, NULL, mob, TO_CHAR );
	act(AT_GREEN, "$n mounts $N.", ch, NULL, mob, TO_ROOM );
	extract_char( mob, TRUE );
	return;
    }
    else
    {
	send_to_char( C_DEFAULT, "You can't mount that.\n\r", ch );
	return;
    }

    return;
}

void do_dismount( CHAR_DATA *ch, char *argument )
{
        
    CHAR_DATA   *mob;
    AFFECT_DATA af;
    char        arg [ MAX_INPUT_LENGTH ];
    
    one_argument( argument, arg );

    if ( IS_NPC( ch ) )
    {
	return;
    }

    if ( ch->mounted == 0 )
    {
	send_to_char( C_DEFAULT, "You are not mounted on anything.\n\r", ch );
	return;
    }

    mob = create_mobile( get_mob_index( ch->mounted ) );

    if( ch->mountcharmed > 0 )
    {
	mob->master = ch;
	mob->leader = ch;
	af.type      = skill_lookup("charm person");
	af.level     = ch->level;
	af.duration  = ch->mountcharmed;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = AFF_CHARM;
	affect_to_char(mob, &af);
    }
    char_to_room(mob, ch->in_room);
    ch->mounted = 0;
    act(AT_GREEN, "You dismount $N.", ch, NULL, mob, TO_CHAR );
    act(AT_GREEN, "$n dismounts $N.", ch, NULL, mob, TO_ROOM );

    return;
}
