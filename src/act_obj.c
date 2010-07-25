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

/*$Id: act_obj.c,v 1.29 2005/03/26 01:44:02 tyrion Exp $*/

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
#include <stdlib.h>
#include "merc.h"

extern char* target_name;

/*
 * External functions.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, 
		        bool fShowNothing );


/*
 * Local functions.
 */
#define CD CHAR_DATA
void	get_obj		args( ( CHAR_DATA *ch, OBJ_DATA *obj,
			       OBJ_DATA *container ) );
void	get_obj2	args( ( CHAR_DATA *ch, OBJ_DATA *obj,
			       OBJ_DATA *container ) );
bool	remove_obj	args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj,
			       bool fReplace ) );
CD *	find_keeper	args( ( CHAR_DATA *ch ) );
int	get_cost	args( ( CHAR_DATA *keeper, OBJ_DATA *obj,
			       bool fBuy ) );
void    do_acoload      args( ( CHAR_DATA *ch, OBJ_DATA *obj, int vnum ) );
void    do_acmload      args( ( CHAR_DATA *ch, OBJ_DATA *obj, int vnum ) );
void    do_actrans      args( ( CHAR_DATA *ch, OBJ_DATA *obj, int vnum ) );
void    do_acmorph      args( ( CHAR_DATA *ch, OBJ_DATA *obj, int vnum ) );
void	check_permanent args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool PICKED_UP ) );
int     ch_invcount     args( ( CHAR_DATA *ch ) );
int     ch_weightcount  args( ( CHAR_DATA *ch ) );
void    ch_stripbadinv  args( ( CHAR_DATA *ch ) );
int     obj_invcount    args( ( OBJ_DATA *obj, bool one_item ) );
int     obj_weightcount args( ( OBJ_DATA *obj, bool one_item ) );
void	obj_stripbadinv args( ( OBJ_DATA *obj, CHAR_DATA* ch ) );
void    obj_sac         args( ( OBJ_DATA *obj, CHAR_DATA* ch, SAC_DATA* sac) );

#undef	CD




void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    if ( !CAN_WEAR( obj, ITEM_TAKE ) )
    {
	send_to_char(AT_WHITE, "You can't take that.\r\n", ch );
	return;
    }
    
    /* make sure the carry number is correct - Ahsile */
    ch->carry_number = ch_invcount( ch );
    ch->carry_weight = ch_weightcount ( ch );
    
    if ( container == NULL )
    {
       if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
       {
	   act(AT_WHITE, "$d: you can't carry that many items.",
	       ch, NULL, obj->name, TO_CHAR );
	   return;
       }
    } else
    {
       if ( container->carried_by == NULL )
       {
           if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch  ) )
           {
              act(AT_WHITE, "$d: you can't carry that many items.",
                  ch, NULL, obj->name, TO_CHAR );
              return;
           }
       }
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	act(AT_WHITE, "$d: you can't carry that much weight.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if ( container )
    {
	act(AT_GREEN, "You get $p from $P.", ch, obj, container, TO_CHAR );
	act(AT_GREEN, "$n gets $p from $P.", ch, obj, container, TO_ROOM );
	oprog_get_from_trigger( obj, ch, container );
	obj_from_obj( obj );
    }
    else
    {
	act(AT_GREEN, "You get $p.", ch, obj, container, TO_CHAR );
	act(AT_GREEN, "$n gets $p.", ch, obj, container, TO_ROOM );
	oprog_get_trigger( obj, ch );
	obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY )
    {
        char buf [ MAX_STRING_LENGTH ];
	int  amount;

	amount = obj->value[0];
	ch->gold += amount;

	if ( amount > 1 )
	{
	    sprintf( buf, "You counted %d coins.\r\n", amount );
	    send_to_char(AT_YELLOW, buf, ch );
	}

	extract_obj( obj );
    }
    else
    {
	obj_to_char( obj, ch );
    }

    ch->carry_number = ch_invcount( ch );
    ch->carry_weight = ch_weightcount( ch );
    return;
}

void get_obj2( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    if ( !CAN_WEAR( obj, ITEM_TAKE ) )
    {
	send_to_char(AT_WHITE, "You can't take that.\r\n", ch );
	return;
    }

    /* make sure the carry number is correct - Ahsile */
    ch->carry_number = ch_invcount( ch );
    ch->carry_weight = ch_weightcount( ch );
    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act(AT_WHITE, "$d: you can't carry that many items.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	act(AT_WHITE, "$d: you can't carry that much weight.",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if ( container )
    {
	act(AT_GREEN, "You palm $p from $P.", ch, obj, container, TO_CHAR );
	oprog_get_from_trigger( obj, ch, container );
	obj_from_obj( obj );
    }
    else
    {
	act(AT_GREEN, "You palm $p.", ch, obj, container, TO_CHAR );
	oprog_get_trigger( obj, ch );
	obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY )
    {
        char buf [ MAX_STRING_LENGTH ];
	int  amount;

	amount = obj->value[0];
	ch->gold += amount;

	if ( amount > 1 )
	{
	    sprintf( buf, "You counted %d coins.\r\n", amount );
	    send_to_char(AT_YELLOW, buf, ch );
	}

	extract_obj( obj );
    }
    else
    {
	obj_to_char( obj, ch );
    }

    return;
}

void do_get( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    OBJ_DATA *container;
    char      arg1 [ MAX_INPUT_LENGTH ];
    char      arg2 [ MAX_INPUT_LENGTH ];
    bool      found;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Get what?\r\n", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );
	    if ( !obj )
	    {
		act(AT_WHITE, "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

	    get_obj( ch, obj, NULL );
	    check_permanent( ch, obj, TRUE );
	}
	else
	{
	    /* 'get all' or 'get all.obj' */
            OBJ_DATA *obj_next;

	    found = FALSE;
	    for ( obj = ch->in_room->contents; obj; obj = obj_next )
	    {
	        obj_next = obj->next_content;

		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		    && can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    get_obj( ch, obj, NULL );
		    check_permanent( ch, obj, TRUE );
		}
	    }

	    if ( !found ) 
	    {
		if ( arg1[3] == '\0' )
		    send_to_char(AT_WHITE, "I see nothing here.\r\n", ch );
		else
		    act(AT_WHITE, "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    }
    else
    {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
	    send_to_char(AT_WHITE, "You can't do that.\r\n", ch );
	    return;
	}

	if ( !( container = get_obj_here( ch, arg2 ) ) )
	{
	    act(AT_WHITE, "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	switch ( container->item_type )
	{
	default:
	    send_to_char(AT_WHITE, "That's not a container.\r\n", ch );
	    return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	    break;
      case ITEM_WRECK:
        break;
	case ITEM_CORPSE_PC:
	    {
		CHAR_DATA *gch;
		char      *pd;
		char       name[ MAX_INPUT_LENGTH ];

		if ( IS_NPC( ch ) )
		{
		    send_to_char(AT_WHITE, "You can't do that.\r\n", ch );
		    return;
		}

		pd = container->short_descr;
		pd = one_argument( pd, name );
		pd = one_argument( pd, name );
		pd = one_argument( pd, name );

		if ( str_cmp( name, ch->name ) && !IS_IMMORTAL( ch ) )
		{
		    bool fGroup;

		    fGroup = FALSE;
		    for ( gch = char_list; gch; gch = gch->next )
		    {
			if ( !IS_NPC( gch )
			    && is_same_group( ch, gch )
			    && !str_cmp( name, gch->name ) )
			{
			    fGroup = TRUE;
			    break;
			}
		    }
                if ( ch->clan != 0 )
                   fGroup = TRUE;
                   
		    if ( !fGroup )
		    {
			send_to_char(AT_WHITE, "You can't do that.\r\n", ch );
			return;
		    }
		}
	    }
	}

	if ( IS_SET( container->value[1], CONT_CLOSED ) && (container->item_type != ITEM_CORPSE_NPC) && (container->item_type != ITEM_CORPSE_PC) )
	{
	    act(AT_GREEN, "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	    return;
	}

	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->contains );
	    if ( !obj )
	    {
		act(AT_GREEN, "I see nothing like that in the $T.",
		    ch, NULL, arg2, TO_CHAR );
		return;
	    }
	    get_obj( ch, obj, container );
	}
	else
	{
	    /* 'get all container' or 'get all.obj container' */
            OBJ_DATA *obj_next;

	    found = FALSE;
	    for ( obj = container->contains; obj; obj = obj_next )
	    {
                obj_next = obj->next_content;

		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		    && can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    get_obj( ch, obj, container );
		}
	    }

	    if ( !found )
	    {
		if ( arg1[3] == '\0' )
		    act(AT_GREEN, "I see nothing in the $T.",
			ch, NULL, arg2, TO_CHAR );
		else
		    act(AT_GREEN, "I see nothing like that in the $T.",
			ch, NULL, arg2, TO_CHAR );
	    }
	}
    }
    return;
}

void do_palm( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg1 [ MAX_INPUT_LENGTH ];
    char      arg2 [ MAX_INPUT_LENGTH ];
    bool      found;
	
	int sn    = skill_lookup("palm");

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( !IS_NPC( ch )
        && ( ( ch->level < skill_table[sn].skill_level[ch->class] )
        && (ch->level < skill_table[sn].skill_level[ch->multied])))
    {
        send_to_char(AT_DGREEN, "What do you think you are, a thief?\r\n", ch );
        return;
    }

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Palm what?\r\n", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->contents );
	    if ( !obj )
	    {
		act(AT_WHITE, "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

	    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 ) )
	        get_obj2( ch, obj, NULL );
	    else
		return;
	}
	else
	{
	    /* 'get all' or 'get all.obj' */
            OBJ_DATA *obj_next;

	    found = FALSE;
	    for ( obj = ch->in_room->contents; obj; obj = obj_next )
	    {
	        obj_next = obj->next_content;

		if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		    && can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if ( IS_NPC( ch ) || number_percent( ) < ( ch->pcdata->learned[sn] / 10 ) )
		        get_obj2( ch, obj, NULL );
		    else
			return;
		}
	    }

	    if ( !found ) 
	    {
		if ( arg1[3] == '\0' )
		    send_to_char(AT_WHITE, "I see nothing here.\r\n", ch );
		else
		    act(AT_WHITE, "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
	    }
	}
    }

    update_skpell(ch, sn, 0);
    return;
}


void do_put( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *container;
    OBJ_DATA *obj;
    char      arg1 [ MAX_INPUT_LENGTH ];
    char      arg2 [ MAX_INPUT_LENGTH ];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char(AT_DGREEN, "Put what in what?\r\n", ch );
	return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
	send_to_char(AT_DGREEN, "You can't do that.\r\n", ch );
	return;
    }

    if ( !( container = get_obj_here( ch, arg2 ) ) )
    {
	act(AT_DGREEN, "I see no $T here.", ch, NULL, arg2, TO_CHAR );
	return;
    }

    if ( container->item_type != ITEM_CONTAINER )
    {
	send_to_char(AT_DGREEN, "That's not a container.\r\n", ch );
	return;
    }

    if ( IS_SET( container->value[1], CONT_CLOSED ) && (container->item_type != ITEM_CORPSE_NPC) && (container->item_type != ITEM_CORPSE_PC) )
    {
	act(AT_DGREEN, "The $d is closed.", ch, NULL, container->name, TO_CHAR );
	return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
	/* 'put obj container' */
	if ( !( obj = get_obj_carry( ch, arg1 ) ) )
	{
	    send_to_char(AT_DGREEN, "You do not have that item.\r\n", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char(AT_DGREEN, "You can't fold it into itself.\r\n", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) /* && !IS_SET(obj->extra_flags2, ITEM_QUEST) */)
	{
	    send_to_char(AT_DGREEN, "You can't let go of it.\r\n", ch );
	    return;
	}

	if ( get_obj_weight( obj ) + get_obj_weight( container )
	     > container->value[0] )
	{
	    send_to_char(AT_DGREEN, "It won't fit.\r\n", ch );
	    return;
	}

	obj_from_char( obj );
	obj_to_obj( obj, container );
        ch->carry_number = ch_invcount( ch );
        ch->carry_weight = ch_weightcount( ch );
	act(AT_GREEN, "You put $p in $P.", ch, obj, container, TO_CHAR );
	act(AT_GREEN, "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	oprog_put_trigger( obj, ch, container );
    }
    else
    {
	/* 'put all container' or 'put all.obj container' */
        OBJ_DATA *obj_next;

	for ( obj = ch->carrying; obj; obj = obj_next )
	{
            obj_next = obj->next_content;

	    if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
		&& can_see_obj( ch, obj )
		&& obj->wear_loc == WEAR_NONE
		&& obj != container
		&& ( can_drop_obj( ch, obj ) /* || IS_SET( obj->extra_flags2, ITEM_QUEST) */ )
		&& get_obj_weight( obj ) + get_obj_weight( container )
		   <= container->value[0] )
	    {
		obj_from_char( obj );
		obj_to_obj( obj, container );
		act(AT_GREEN, "You put $p in $P.", ch, obj, container, TO_CHAR );
		act(AT_GREEN, "$n puts $p in $P.", ch, obj, container, TO_ROOM );
		oprog_put_trigger( obj, ch, container );
	    }
	}
    }
    ch->carry_number = ch_invcount ( ch );
    ch->carry_weight = ch_weightcount ( ch );
    return;
}



void do_drop( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];
    bool      found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_DGREEN, "Drop what?\r\n", ch );
	return;
    }

    if ( is_number( arg ) )
    {
	/* 'drop NNNN coins' */
        OBJ_DATA *obj_next;
	int       amount;

	amount   = atoi( arg );
	argument = one_argument( argument, arg );
	if ( amount <= 0 || str_prefix( arg, "coins" ) )
	{
	    send_to_char(AT_DGREEN, "Sorry, you can't do that.\r\n", ch );
	    return;
	}

	if ( ch->gold < amount )
	{
	    send_to_char(AT_DGREEN, "You haven't got that many coins.\r\n", ch );
	    return;
	}

	ch->gold -= amount;

	for ( obj = ch->in_room->contents; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( obj->deleted )
	        continue;

	    switch ( obj->pIndexData->vnum )
	    {
	    case OBJ_VNUM_MONEY_ONE:
		amount += 1;
		extract_obj( obj );
		break;

	    case OBJ_VNUM_MONEY_SOME:
		amount += obj->value[0];
		extract_obj( obj );
		break;
	    }
	}

	obj_to_room( create_money( amount ), ch->in_room );
	send_to_char(AT_YELLOW, "OK.\r\n", ch );
	act(AT_YELLOW, "$n drops some gold.", ch, NULL, NULL, TO_ROOM );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	/* 'drop obj' */
	if ( !( obj = get_obj_carry( ch, arg ) ) )
	{
	    send_to_char(AT_DGREEN, "You do not have that item.\r\n", ch );	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
//		if (IS_SET(obj->extra_flags2, ITEM_QUEST))
//		{
			send_to_char(AT_DGREEN, "You can't let go of it.\r\n", ch );
/*		} else
		{
			send_to_char(AT_DGREEN, "Your precious quest equipment?\n\r", ch);
		} */
	    return;
	}

	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	act(AT_GREEN, "You drop $p.", ch, obj, NULL, TO_CHAR );
	act(AT_GREEN, "$n drops $p.", ch, obj, NULL, TO_ROOM );
	check_permanent( ch, obj, FALSE );
	oprog_drop_trigger( obj, ch );
    }
    else
    {
	/* 'drop all' or 'drop all.obj' */
	OBJ_DATA *obj_next;

	found = FALSE;
	for ( obj = ch->carrying; obj; obj = obj_next )
	{
            obj_next = obj->next_content;

	    if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
		&& can_see_obj( ch, obj )
		&& obj->wear_loc == WEAR_NONE
		&& can_drop_obj( ch, obj ) )
	    {
		found = TRUE;
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
		act(AT_GREEN, "You drop $p.", ch, obj, NULL, TO_CHAR );
		act(AT_GREEN, "$n drops $p.", ch, obj, NULL, TO_ROOM );
		check_permanent( ch, obj, FALSE );
		oprog_drop_trigger( obj, ch );
	    }
	}

	if ( !found )
	{
	    if ( arg[3] == '\0' )
	        send_to_char(AT_DGREEN, "You are not carrying anything.", ch );
	    else
		act(AT_DGREEN, "You are not carrying any $T.",
		    ch, NULL, &arg[4], TO_CHAR );
	}
    }

    save_char_obj( ch, FALSE );	/* force save after drop -- REK */
    return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA  *obj;
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char(AT_DGREEN, "Give what to whom?\r\n", ch );
	return;
    }

    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	int amount;

	amount   = atoi( arg1 );
	if ( amount <= 0 || str_prefix( arg2, "coins" ) )
	{
	    send_to_char(AT_DGREEN, "Sorry, you can't do that.\r\n", ch );
	    return;
	}

	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char(AT_DGREEN, "Give what to whom?\r\n", ch );
	    return;
	}

	if ( !( victim = get_char_room( ch, arg2 ) ) )
	{
	    send_to_char(AT_DGREEN, "They aren't here.\r\n", ch );
	    return;
	}

	if ( ch->gold < amount )
	{
	    send_to_char(AT_YELLOW, "You haven't got that much gold.\r\n", ch );
	    return;
	}

	ch->gold     -= amount;
	victim->gold += amount;
	act(AT_YELLOW, "You give $N some gold.",  ch, NULL, victim, TO_CHAR    );
	act(AT_YELLOW, "$n gives you some gold.", ch, NULL, victim, TO_VICT    );
	act(AT_YELLOW, "$n gives $N some gold.",  ch, NULL, victim, TO_NOTVICT );
        mprog_bribe_trigger( victim, ch, amount );
	return;
    }

    if ( !( obj = get_obj_carry( ch, arg1 ) ) )
    {
	send_to_char(AT_DGREEN, "You do not have that item.\r\n", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char(AT_DGREEN, "You must remove it first.\r\n", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, arg2 ) ) )
    {
	send_to_char(AT_DGREEN, "They aren't here.\r\n", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
//		if (!IS_SET(obj->extra_flags2, ITEM_QUEST))
//		{
			send_to_char(AT_DGREEN, "You can't let go of it.\r\n", ch );
/*		} else
		{
			send_to_char(AT_DGREEN, "Your precious quest equipment?\n\r", ch);
		} */
	return;
    }

    /* make sure the carry number is correct - Ahsile */
    victim->carry_number = ch_invcount( victim );
    victim->carry_weight = ch_weightcount( victim );
    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
	act(AT_DGREEN, "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( victim->carry_weight + get_obj_weight( obj ) > can_carry_w( victim ) )
    {
	act(AT_DGREEN, "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act(AT_DGREEN, "$N can't see it.", ch, NULL, victim, TO_CHAR );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    save_char_obj( ch, FALSE );	/* force save char after giving obj  -- REK */
    act(AT_DGREEN, "You give $p to $N.", ch, obj, victim, TO_CHAR    );
    act(AT_DGREEN, "$n gives you $p.",   ch, obj, victim, TO_VICT    );
    act(AT_DGREEN, "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
    oprog_give_trigger( obj, ch, victim );
    if ( !obj || !ch || !victim )
      return;
    mprog_give_trigger( victim, ch, obj );
    return;
}




void do_fill( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    char      arg [ MAX_INPUT_LENGTH ];
    bool      found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_BLUE, "Fill what?\r\n", ch );
	return;
    }

    if ( !( obj = get_obj_carry( ch, arg ) ) )
    {
	send_to_char(AT_BLUE, "You do not have that item.\r\n", ch );
	return;
    }

    found = FALSE;
    for ( fountain = ch->in_room->contents; fountain;
	fountain = fountain->next_content )
    {
	if ( fountain->item_type == ITEM_FOUNTAIN )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char(AT_BLUE, "There is no fountain here!\r\n", ch );
	return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char(AT_BLUE, "You can't fill that.\r\n", ch );
	return;
    }

    if ( obj->value[1] != 0 && obj->value[2] != 0 )
    {
	send_to_char(AT_BLUE, "There is already another liquid in it.\r\n", ch );
	return;
    }

    if ( obj->value[1] >= obj->value[0] )
    {
	send_to_char(AT_BLUE, "Your container is full.\r\n", ch );
	return;
    }

    act(AT_LBLUE, "You fill $p.", ch, obj, NULL, TO_CHAR );
    obj->value[2] = 0;
    obj->value[1] = obj->value[0];
    oprog_fill_trigger( obj, ch, fountain );
    return;
}



void do_drink( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];
    int       amount;
    int       liquid;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
	    if ( obj->item_type == ITEM_FOUNTAIN ) 
		break;
	    if ( obj->item_type == ITEM_BLOOD    )
	        break;
	}

	if ( !obj )
	{
	    send_to_char(AT_BLUE, "Drink what?\r\n", ch );
	    return;
	}
    }
    else
    {
	if ( !( obj = get_obj_here( ch, arg ) ) )
	{
	    send_to_char(AT_BLUE, "You can't find it.\r\n", ch );
	    return;
	}
    }

    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] >= 90 )
    {
	send_to_char(AT_BLUE, "You fail to reach your mouth.  *Hic*\r\n", ch );
	return;
    }

    switch ( obj->item_type )
    {
    default:
	send_to_char(AT_BLUE, "You can't drink from that.\r\n", ch );
	break;

    case ITEM_BLOOD:
        if ( ch->class != 9 )
           {
            send_to_char( AT_WHITE, "It is not in your nature to do such things.\r\n", ch );
            return;
           }
        if ( ( ch->bp + 1 ) > ch->max_bp )
           {
            send_to_char( AT_RED, "Your hunger for blood has been abated.\r\n", ch );
            return;
           }
        ch->bp += 1;
        send_to_char(AT_RED, "You lap up the blood like a cur.\r\n", ch );
        act(AT_RED, "$n drinks from $p.", ch, obj, NULL, TO_ROOM);
        if (obj->value[0] != -1)
           obj->value[1] -= 1;
	if ( ( obj->value[1] <= 0 ) && ( obj->value[0] != -1 ) )
	{
	    act(AT_RED, "$n laps up the last of the blood.", ch, NULL, NULL, TO_ROOM );
	    send_to_char(AT_RED, "The lap up the last of the blood.\r\n", ch );
	    extract_obj( obj );
	}
	break;
    case ITEM_FOUNTAIN:
	if ( !IS_NPC( ch ) )
	    ch->pcdata->condition[COND_THIRST] = 48;
	act(AT_LBLUE, "You drink from the $p.\r\n", ch, obj, NULL, TO_CHAR );
	send_to_char(AT_BLUE, "You are not thirsty.\r\n", ch );
	act(AT_LBLUE, "$n drinks from the $p.", ch, obj, NULL, TO_ROOM );
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char(AT_BLUE, "It is already empty.\r\n", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] ) >= LIQ_MAX )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

	act(AT_LBLUE, "You drink $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_CHAR );
	act(AT_LBLUE, "$n drinks $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_ROOM );

	amount = number_range( 3, 8 );
	amount = UMIN( amount, obj->value[1] );
	
	gain_condition( ch, COND_DRUNK,
	    liq_table[liquid].liq_affect[COND_DRUNK  ] );
	gain_condition( ch, COND_FULL,
	    amount * liq_table[liquid].liq_affect[COND_FULL   ] );
	gain_condition( ch, COND_THIRST,
	    amount * liq_table[liquid].liq_affect[COND_THIRST ] );
	if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK ] > 100 )
	    ch->pcdata->condition[COND_DRUNK ] = 100;

	if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK ] > 10 )
	    send_to_char(AT_ORANGE, "You feel drunk.\r\n", ch );
	if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL  ] > 40 )
	    send_to_char(AT_BLUE, "You are full.\r\n", ch );
	if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] > 40 )
	    send_to_char(AT_BLUE, "You do not feel thirsty.\r\n", ch );
	
	if ( obj->value[3] != 0 )
	{
	    /* The shit was poisoned ! */
	    AFFECT_DATA af;

	    send_to_char(AT_GREEN, "You choke and gag.\r\n", ch );
	    act(AT_GREEN, "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
	    af.type      = skill_lookup("poison");
	    af.duration  = 3 * amount;
	    af.location  = APPLY_STR;
	    af.modifier  = -2;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	    add_poison( ch, amount );
	}
	
	obj->value[1] -= amount;
	if ( obj->value[1] <= 0 )
	{
	    send_to_char(AT_BLUE, "The empty container vanishes.\r\n", ch );
	    extract_obj( obj );
	}
	break;
    }

    return;
}



void do_eat( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];
    int       amnt;
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char(AT_ORANGE, "Eat what?\r\n", ch );
	return;
    }

    if ( !( obj = get_obj_carry( ch, arg ) ) )
    {
	send_to_char(AT_ORANGE, "You do not have that item.\r\n", ch );
	return;
    }

    if ( !IS_IMMORTAL( ch ) )
    {
	if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL && obj->item_type != ITEM_BERRY)
	{
	    send_to_char(AT_ORANGE, "That's not edible.\r\n", ch );
	    return;
	}

	if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] > 40 )
	{   
	    send_to_char(AT_ORANGE, "You are too full to eat more.\r\n", ch );
	    return;
	}
    }

    act(AT_ORANGE, "You eat $p.", ch, obj, NULL, TO_CHAR );
    act(AT_ORANGE, "$n eats $p.", ch, obj, NULL, TO_ROOM );

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
	if ( !IS_NPC( ch ) )
	{
	    int condition;

	    condition = ch->pcdata->condition[COND_FULL];
	    gain_condition( ch, COND_FULL, obj->value[0] );
	    if ( ch->pcdata->condition[COND_FULL] > 40 )
	        send_to_char(AT_ORANGE, "You are full.\r\n", ch );
	    else if ( condition == 0 && ch->pcdata->condition[COND_FULL] > 0 )
		send_to_char(AT_ORANGE, "You are no longer hungry.\r\n", ch );
	}

	if ( obj->value[3] != 0 )
	{
	    /* The shit was poisoned! */
	    AFFECT_DATA af;

	    act(AT_GREEN, "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	    send_to_char(AT_GREEN, "You choke and gag.\r\n", ch );

	    af.type      = skill_lookup("poison");
	    af.duration  = 2 * obj->value[0];
	    af.location  = APPLY_STR;
	    af.modifier  = -2;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	    add_poison( ch, obj->value[0] );
	}
	break;
    case ITEM_BERRY:
       amnt = number_range( obj->value[0], obj->value[1] );
       ch->hit = UMIN( ch->hit + amnt, ch->max_hit );
       update_pos( ch );
       send_to_char(AT_ORANGE, "You feel warm all over.\r\n", ch);
       break;        
    case ITEM_PILL:
	    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	break;
    }

    extract_obj( obj );
    return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj;

    if ( !( obj = get_eq_char( ch, iWear ) ) )
	return TRUE;

    if ( !fReplace )
	return FALSE;

    if ( IS_SET( obj->extra_flags, ITEM_NOREMOVE ) )
    {
	act(AT_RED, "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }

    if ( obj->wear_loc == WEAR_IMPLANTED1 ||
	 obj->wear_loc == WEAR_IMPLANTED2 ||
	 obj->wear_loc == WEAR_IMPLANTED3 )
    {
	act(AT_RED, "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }

    unequip_char( ch, obj );
    act(AT_WHITE, "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act(AT_WHITE, "You stop using $p.", ch, obj, NULL, TO_CHAR );
    return TRUE;
}



/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace )
{
    OBJ_DATA *obj2;
    char buf [ MAX_STRING_LENGTH ];

    if ( ch->level < obj->level )
    {
	sprintf( buf, "You must be level %d to use this object.\r\n",
	    obj->level );
	send_to_char(AT_BLUE, buf, ch );
	act(AT_BLUE, "$n tries to use $p, but is too inexperienced.",
	    ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( obj->item_type == ITEM_LIGHT )
    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
	    return;
	act(AT_WHITE, "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
	act(AT_WHITE, "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_LIGHT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
	if ( get_eq_char( ch, WEAR_FINGER_L )
	&&   get_eq_char( ch, WEAR_FINGER_R )
	&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
	    return;

	if ( !get_eq_char( ch, WEAR_FINGER_L ) )
	{
	    act(AT_BLUE, "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
	    act(AT_BLUE, "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
	    equip_char( ch, obj, WEAR_FINGER_L );
	    return;
	}

	if ( !get_eq_char( ch, WEAR_FINGER_R ) )
	{
	    act(AT_BLUE, "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
	    act(AT_BLUE, "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
	    equip_char( ch, obj, WEAR_FINGER_R );
	    return;
	}

	bug( "Wear_obj: no free finger.", 0 );
	send_to_char(AT_BLUE, "You already wear two rings.\r\n", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
	if ( get_eq_char( ch, WEAR_NECK_1 )
	&&   get_eq_char( ch, WEAR_NECK_2 )
	&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
	    return;

	if ( !get_eq_char( ch, WEAR_NECK_1 ) )
	{
	    act(AT_BLUE, "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    act(AT_BLUE, "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    equip_char( ch, obj, WEAR_NECK_1 );
	    return;
	}

	if ( !get_eq_char( ch, WEAR_NECK_2 ) )
	{
	    act(AT_BLUE, "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
	    act(AT_BLUE, "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
	    equip_char( ch, obj, WEAR_NECK_2 );
	    return;
	}

	bug( "Wear_obj: no free neck.", 0 );
	send_to_char(AT_BLUE, "You already wear two neck items.\r\n", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
	if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
	    return;
	act(AT_BLUE, "You wear $p on your body.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n wears $p on $s body.",   ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_BODY );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
	if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
	    return;
	act(AT_BLUE, "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_HEAD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ORBIT ) )
    {
	if ( get_eq_char( ch, WEAR_ORBIT )
	&&   get_eq_char( ch, WEAR_ORBIT_2 )
	&&   !remove_obj( ch, WEAR_ORBIT, fReplace )
	&&   !remove_obj( ch, WEAR_ORBIT_2, fReplace ) )
	    return;

	if ( !get_eq_char( ch, WEAR_ORBIT ) )
	{
	    act(AT_BLUE, "You start $p spinning about your head.",
		ch, obj, NULL, TO_CHAR );
	    act(AT_BLUE, "$n starts $p spinning around $s head.",
		ch, obj, NULL, TO_ROOM );
	    equip_char( ch, obj, WEAR_ORBIT );
	    return;
	}

	if ( !get_eq_char( ch, WEAR_ORBIT_2 ) )
	{
	    act(AT_BLUE, "You starts $p spinning about your head.",
		ch, obj, NULL, TO_CHAR );
	    act(AT_BLUE, "$n starts $p spinning around $s head.",
		ch, obj, NULL, TO_ROOM );
	    equip_char( ch, obj, WEAR_ORBIT_2 );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char(AT_BLUE, "There are already two things spinning about your head.\r\n", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FACE ) )
    {
	if ( !remove_obj( ch, WEAR_ON_FACE, fReplace ) )
	    return;
	act(AT_BLUE, "You place $p on your face.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n places $p on $s face.",   ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_ON_FACE );
	return;
    }

  if ( CAN_WEAR( obj, ITEM_WEAR_IMPLANTED ) )
    {
        if ( get_eq_char( ch, WEAR_IMPLANTED1 )
        &&   get_eq_char( ch, WEAR_IMPLANTED2 )
	&&   get_eq_char( ch, WEAR_IMPLANTED3 ) )
            return;
         
        if ( !get_eq_char( ch, WEAR_IMPLANTED1 ) )
        {
            act(AT_BLUE, "$p magically gets implanted in your body.", ch, obj, NULL, TO_CHAR );
            act(AT_BLUE, "$p magically gets implanted in $n's body.",   ch, obj, NULL, TO_ROOM );
            equip_char( ch, obj, WEAR_IMPLANTED1 );
            return;
        }
	if ( !get_eq_char( ch, WEAR_IMPLANTED2 ) )
        {
            act(AT_BLUE, "$p magically gets implanted in your body.", ch, obj, NULL, TO_CHAR );
            act(AT_BLUE, "$p magically gets implanted in $n's body.",   ch, obj, NULL, TO_ROOM );
            equip_char( ch, obj, WEAR_IMPLANTED2 );
            return;
        }
	if ( !get_eq_char( ch, WEAR_IMPLANTED3 ) )
        {
            act(AT_BLUE, "$p magically gets implanted in your body.", ch, obj, NULL, TO_CHAR );
            act(AT_BLUE, "$p magically gets implanted in $n's body.",   ch, obj, NULL, TO_ROOM );
            equip_char( ch, obj, WEAR_IMPLANTED3 );
            return;
        }
        
        bug( "Wear_obj: no free implant.", 0 );
        send_to_char(AT_BLUE, "You already wear three implanted items.\r\n", ch );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_CONTACT ) )
    {
	if ( !remove_obj( ch, WEAR_IN_EYES, fReplace ) )
	    return;
	act(AT_BLUE, "You stick $p into your eyes.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n sticks $p into $s eyes.",   ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_IN_EYES );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
	if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
	    return;
	act(AT_BLUE, "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_LEGS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
	if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	    return;
	act(AT_BLUE, "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_FEET );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
	if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	    return;
	act(AT_BLUE, "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_HANDS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
	if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	    return;
	act(AT_BLUE, "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_ARMS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
	if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	    return;
	act(AT_BLUE, "You wear $p about your body.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n wears $p about $s body.",   ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_ABOUT );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
	if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	    return;
	act(AT_BLUE, "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_WAIST );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
	if ( get_eq_char( ch, WEAR_WRIST_L )
	&&   get_eq_char( ch, WEAR_WRIST_R )
	&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	    return;

	if ( !get_eq_char( ch, WEAR_WRIST_L ) )
	{
	    act(AT_BLUE, "You wear $p around your left wrist.",
		ch, obj, NULL, TO_CHAR );
	    act(AT_BLUE, "$n wears $p around $s left wrist.",
		ch, obj, NULL, TO_ROOM );
	    equip_char( ch, obj, WEAR_WRIST_L );
	    return;
	}

	if ( !get_eq_char( ch, WEAR_WRIST_R ) )
	{
	    act(AT_BLUE, "You wear $p around your right wrist.",
		ch, obj, NULL, TO_CHAR );
	    act(AT_BLUE, "$n wears $p around $s right wrist.",
		ch, obj, NULL, TO_ROOM );
	    equip_char( ch, obj, WEAR_WRIST_R );
	    return;
	}

	bug( "Wear_obj: no free wrist.", 0 );
	send_to_char(AT_BLUE, "You already wear two wrist items.\r\n", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
	if ( get_eq_char( ch, WEAR_WIELD_2 ) )
	{
		send_to_char( AT_BLUE, "You cannot use a shield while dual wielding!\r\n", ch );
		return;
	}

	if ( ( obj2 = get_eq_char ( ch, WEAR_WIELD ) ) != NULL &&
	    IS_SET ( obj2->extra_flags2, ITEM_TWO_HANDED ) )
	{
	    send_to_char ( AT_BLUE, "You cannot use a shield with a two-handed weapon!\r\n", ch );
	    return;
	}

	if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
	    return;
	act(AT_BLUE, "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_SHIELD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	if ( IS_SET ( obj->extra_flags2, ITEM_TWO_HANDED ) )
        {
            if ( get_eq_char ( ch, WEAR_WIELD_2 ) )
            {
                send_to_char ( AT_BLUE, "You cannot use a two-handed weapon while dual wielding\r\n", ch );
		return;
	    }
	    if ( get_eq_char ( ch, WEAR_SHIELD ) )
            {
                send_to_char ( AT_BLUE, "You cannot use a two-handed weapon while holding a shield!\r\n", ch );
		return;
	    }
	    if ( get_eq_char ( ch, WEAR_HOLD ) )
            {
                send_to_char ( AT_BLUE, "You cannot use a two-handed weapon while holding something!\r\n", ch );
		return;
	    }
	    if ( get_eq_char ( ch, WEAR_FIREARM ) )
            {
                send_to_char ( AT_BLUE, "You cannot use a two-handed weapon while using a firearm!\r\n", ch );
		return;
	    }
	    if ( !IS_NPC ( ch ) && !ch->pcdata->learned[skill_lookup("two handed")] && obj->value[3] != flag_value( weapon_flags, "shot" ) )
            {
                send_to_char ( AT_BLUE, "You are unable to use a two-handed weapon.\r\n", ch );
		return;
		} 
	}

	if ( !remove_obj( ch, WEAR_WIELD, fReplace ) )
	    return;

	if ( get_obj_weight( obj ) > str_app[get_curr_str( ch )].wield )
	{
	    send_to_char(AT_BLUE, "It is too heavy for you to wield.\r\n", ch );
	    return;
	}

	if (( ch->class == 14 ) && (( obj->value[3] == 1 ) ||( obj->value[3] == 2 )
         ||( obj->value[3] == 3 ) ||( obj->value[3] == 4 ) ||( obj->value[3] == 5 )
         ||(obj->value[3] == 9 ) ||( obj->value[3] == 10 ) ||( obj->value[3] == 11 )
         ||( obj->value[3] == 12 )))
	   {
	    send_to_char(AT_BLUE, "Barbarians can not wield this type of weapon.\r\n", ch );
	    return;
	   }
        if (( ch->multied==14) && (( obj->value[3] == 1 ) ||( obj->value[3] == 2 )
	 ||( obj->value[3] == 3 ) ||( obj->value[3] == 4 ) ||( obj->value[3] == 5 )
	 ||( obj->value[3] == 9 ) ||(obj->value[3] == 10 ) ||(obj->value[3] == 11 )
	 ||( obj->value[3] ==12 )))
	{
	    send_to_char(AT_BLUE, "Barbarians can not wield this type of weapon.\r\n", ch );
	    return;
	}
	act(AT_BLUE, "You wield $p.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n wields $p.", ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_WIELD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
	if ( get_eq_char( ch, WEAR_WIELD_2 ) )
	{
		send_to_char( AT_BLUE, "You cannot hold something while dual wielding!\r\n", ch );
		return;
	}
	if ( ( obj2 = get_eq_char ( ch, WEAR_WIELD ) ) != NULL &&
                IS_SET ( obj2->extra_flags2, ITEM_TWO_HANDED ) )
        {
            send_to_char ( AT_BLUE, "You cannot hold something while using a two-handed weapon!", ch );
	    return;
	}

	if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
	    return;
	act(AT_BLUE, "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n holds $p in $s hands.",   ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_HOLD );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FIREARM ) )
    {
	if (get_eq_char( ch, WEAR_WIELD_2 ) )
	{
		send_to_char(AT_BLUE, "You cannot wield that while dual wielding.\r\n", ch );
		return;
	}
	if ( ( obj2 = get_eq_char ( ch, WEAR_WIELD ) ) != NULL &&
                IS_SET ( obj2->extra_flags2, ITEM_TWO_HANDED ) )
        {
            send_to_char ( AT_BLUE, "You cannot hold something while using a two-handed weapon!\r\n", ch );
	    return;
	}

	if (!remove_obj( ch, WEAR_FIREARM, fReplace ) )
	  return;
	act(AT_BLUE, "You put $p at your side.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n puts $p at his side.", ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_FIREARM );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_EARS ) )
    {
        if ( !remove_obj( ch, WEAR_EARS, fReplace ) )
	    return;
	act(AT_BLUE, "You place $p on your ears.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n places $p on $s ears.", ch, obj, NULL, TO_ROOM );
	equip_char( ch, obj, WEAR_EARS );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ANKLE ) )
    {
        if ( get_eq_char( ch, WEAR_ANKLE_L )
	&&   get_eq_char( ch, WEAR_ANKLE_R )
	&&   !remove_obj( ch, WEAR_ANKLE_L, fReplace )
	&&   !remove_obj( ch, WEAR_ANKLE_R, fReplace ) )
	    return;

	if ( !get_eq_char( ch, WEAR_ANKLE_L ) )
	{
	    act(AT_BLUE, "You wear $p on your left ankle.", ch, obj, NULL, TO_CHAR );
	    act(AT_BLUE, "$n wears $p on $s left ankle.", ch, obj, NULL, TO_ROOM );
	    equip_char( ch, obj, WEAR_ANKLE_L );
	    return;
	}

	if ( !get_eq_char( ch, WEAR_ANKLE_R ) )
	{
	    act(AT_BLUE, "You wear $p on your right ankle.", ch, obj, NULL, TO_CHAR );
	    act(AT_BLUE, "$n wears $p on $s right ankle.", ch, obj, NULL, TO_ROOM );
	    equip_char( ch, obj, WEAR_ANKLE_R );
	    return;
	}

	bug( "Wear_obj: no free ankle.", 0 );
	send_to_char(AT_BLUE, "You already have items on both ankles.\r\n", ch );
	return;
    }

    if ( fReplace )
	send_to_char(AT_BLUE, "You can't wear, wield, or hold that.\r\n", ch );

    return;
}

void do_dual( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj2;
    char      arg [ MAX_INPUT_LENGTH ];
    bool      fReplace;
    
    fReplace = TRUE;

    one_argument( argument, arg );

    if ( IS_NPC( ch ) )
       return;
       
    if ( ( get_eq_char ( ch, WEAR_SHIELD ) ) || ( get_eq_char ( ch, WEAR_FIREARM ) ) || ( get_eq_char ( ch, WEAR_HOLD ) ) )
    {
      send_to_char( AT_BLUE, "You cannot dual wield while you hold something in your hands.\r\n", ch );
      return;
    }
    if ( ( obj2 = get_eq_char ( ch, WEAR_WIELD ) ) != NULL &&
           IS_SET ( obj2->extra_flags2, ITEM_TWO_HANDED ) && !IS_NPC( ch ) )
    {
      send_to_char( AT_BLUE, "You cannot dual wield while using a two-handed weapon.\r\n", ch );
      return;
    }
    
    if ( arg[0] == '\0' )
    {
	send_to_char(AT_BLUE, "Dual wield what?\r\n", ch );
	return;
    }
    if ( !( obj = get_obj_carry( ch, arg ) ) )
    {
     send_to_char(AT_BLUE, "You do not have that item.\r\n", ch );
     return;
    }
    if ( ch->level < obj->level)
    {
      send_to_char( AT_WHITE, "You are to inexperienced.\r\n", ch );
      return;
    }
      if ( !IS_NPC( ch )
        && ( ( ch->level < skill_table[skill_lookup("dual")].skill_level[ch->class] )
        && (ch->level < skill_table[skill_lookup("dual")].skill_level[ch->multied])))
    {
	send_to_char(AT_WHITE, "You cannot.\r\n", ch );
	return;
    }
    if ( ch->pcdata->learned[skill_lookup("dual")] == 0 )
    {   
     send_to_char( AT_WHITE, "You cannot.\r\n", ch );
     return;
    }
    if (( ch->class == 14 ) && (( obj->value[3] == 1 ) ||( obj->value[3] == 2 )
     ||( obj->value[3] == 3 ) ||( obj->value[3] == 4 ) ||( obj->value[3] == 5 )
     ||( obj->value[3] == 9 ) ||( obj->value[3] == 10 ) ||(obj->value[3] == 11 ) 
    ||( obj->value[3] == 12)))
    {
    send_to_char(AT_BLUE, "Barbarians can not wield this type of weapon.\r\n", ch );
    return;
    }
    if (( ch->multied == 14 ) && (( obj->value[3] == 1 ) ||(obj->value[3] == 2 )
     ||( obj->value[3] == 3) ||( obj->value[3] == 4) || ( obj->value[3] == 5 )
     ||( obj->value[3] == 9) ||( obj->value[3] == 10 ) ||(obj->value[3] == 11 )
     ||( obj->value[3] == 12)))
    {
    send_to_char(AT_BLUE, "Barbarians can not wield this type of weapon.\r\n", ch );
    return;
    }

    if ( IS_SET ( obj->extra_flags2, ITEM_TWO_HANDED ) )
    {
        send_to_char ( AT_BLUE, "You cannot dual wield this weapon.\r\n", ch );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD )
            && !IS_SET( race_table[ ch->race ].race_abilities,
                       RACE_WEAPON_WIELD ) )
    {
	send_to_char(AT_BLUE, "You are not able to wield a weapon.\r\n", ch );
	return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    { 
      if ( !remove_obj( ch, WEAR_WIELD_2, fReplace ) )
         return;

      if ( get_obj_weight( obj ) > str_app[get_curr_str( ch )].wield )
	{
	    send_to_char(AT_BLUE, "It is too heavy for you to dual wield.\r\n", ch );
	    return;
	}

      act(AT_BLUE, "You dual wield $p.", ch, obj, NULL, TO_CHAR );
      act(AT_BLUE, "$n dual wields $p.", ch, obj, NULL, TO_ROOM );
      equip_char( ch, obj, WEAR_WIELD_2 );
    }  
    return;
}

void do_wear( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_BLUE, "Wear, wield, or hold what?\r\n", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        OBJ_DATA *obj_next;

        for ( obj = ch->carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;

            if ( obj->wear_loc != WEAR_NONE || !can_see_obj( ch, obj ) )
                continue;

            if ( CAN_WEAR( obj, ITEM_WIELD )
                && !IS_SET( race_table[ ch->race ].race_abilities,
                           RACE_WEAPON_WIELD ) )
                continue;

            wear_obj( ch, obj, FALSE );
	    
	}
	return;
    }
    else
    {
	if ( !( obj = get_obj_carry( ch, arg ) ) )
	{
	    send_to_char(AT_BLUE, "You do not have that item.\r\n", ch );
	    return;
	}

        if ( CAN_WEAR( obj, ITEM_WIELD )
            && !IS_SET( race_table[ ch->race ].race_abilities,
                       RACE_WEAPON_WIELD ) )
        {
            send_to_char(AT_BLUE, "You are not able to wield a weapon.\r\n", ch );
            return;
        }

	wear_obj( ch, obj, TRUE );
    }

    return;
}



void do_remove( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );

    if ( ch->position == POS_GHOST )
    {
	send_to_char(AT_CYAN, "A ghost can not do that.\r\n", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_BLUE, "Remove what?\r\n", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	OBJ_DATA *obj_next;
	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc != WEAR_NONE && 
			obj->wear_loc != WEAR_IMPLANTED1 &&
			obj->wear_loc != WEAR_IMPLANTED2 &&
			obj->wear_loc != WEAR_IMPLANTED3 &&
			can_see_obj( ch, obj ) )
	        remove_obj( ch, obj->wear_loc, TRUE );
	}
	return;
    }
    if ( !( obj = get_obj_wear( ch, arg ) ) )
    {
	send_to_char(AT_BLUE, "You do not have that item.\r\n", ch );
	return;
    }

    remove_obj( ch, obj->wear_loc, TRUE );
    return;
}

/* Most original code moved to obj_sac() function
   Added compatibility for sac all
     - Ahsile
*/
void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA    *obj;
    char        arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
        send_to_char(C_DEFAULT,
            "The Gods appreciate your offer and may accept it later.", ch );
        act(C_DEFAULT, "$n offers $mself to the gods, who graciously declines.",
            ch, NULL, NULL, TO_ROOM );
        return;
    }

    if ( !str_cmp( arg, "all") )
    {
      OBJ_DATA* obj_next;
      SAC_DATA sac[ MAX_SAC_TYPES ];
      int i;

      for (i=0;i<MAX_SAC_TYPES;i++)
         sac[i].reward_total = 0;

      if (ch->in_room->contents == NULL)
      {
         send_to_char(C_DEFAULT, "There's nothing here!.\r\n", ch);
         return;
      }
      
      for ( obj = ch->in_room->contents; obj; obj = obj_next )
      {
         obj_next = obj->next_content;
         if ( obj->deleted ) { continue; }
         if ( ( !IS_SET( obj->extra_flags2, ITEM_NO_SAC) ) && ( CAN_WEAR( obj, ITEM_TAKE )))
            obj_sac( obj, ch, sac );
      }
      ch->move += sac[SAC_MOVE].reward_total;
      ch->hit  += sac[ SAC_HP ].reward_total;
      ch->mana += sac[SAC_MANA].reward_total;
      ch->gold += sac[SAC_GOLD].reward_total;

      sprintf(arg, "%s gives you %dgp, %dhp, %dmv, and %dm for your sacrifice.\r\n",sac[SAC_HP].god, sac[SAC_GOLD].reward_total, sac[SAC_HP].reward_total, sac[SAC_MOVE].reward_total, sac[SAC_MANA].reward_total);
      send_to_char(C_DEFAULT, arg, ch);
      act(C_DEFAULT, "$n sacrifices everything here to the gods.", ch, NULL, NULL, TO_ROOM );
    } else
    {
       obj = get_obj_list( ch, arg, ch->in_room->contents );
       if ( !obj )
       {
           send_to_char(C_DEFAULT, "You can't find it.\r\n", ch );
           return;
       }
       obj_sac( obj, ch, NULL );
    }

}

/* Original do_sacrifice code here
   Added SAC_DATA pointer for compatibility with sac all
   (you don't get spammed a billion times if you sac a large
    amount of items)
      - Ahsile
*/
void obj_sac( OBJ_DATA* obj, CHAR_DATA* ch, SAC_DATA* sac)
{
    int              gain = 0;
    char             arg[ MAX_INPUT_LENGTH ];
    char             godname[MAX_STRING_LENGTH];
    RELIGION_DATA    *pReligion; 

#if defined (SAC_GODNAMES)
    /* Godname idea from CU-mud, code by The Maniac from Mythran */
   /* Taken out..
    static char * god_name_table [ ] =
    {
        "Walius", "Tyrion", "Ambrosia", "Acheron", "Ahsile",
        "Vision", "Zhyril", "Manaux", "Deconce", "Nexus", "Terawyn"
    };

    strcpy(godname, god_name_table[number_range(0,10)]);
#else
    strcpy(godname, "God");
*/
#endif

    pReligion = get_religion_index(ch->religion); 
    if (!sac)
       sprintf( godname, pReligion->deity );
    else
       sprintf( sac[SAC_HP].god, pReligion->deity );

    if ( !CAN_WEAR( obj, ITEM_TAKE ) )
    {
        act(C_DEFAULT, "$p is not an acceptable sacrifice.", ch, obj, NULL, TO_CHAR );
        return;
    }

    if ( IS_SET( obj->extra_flags2, ITEM_NO_SAC ) )
    {
	act(C_DEFAULT, "$p is not an acceptable sacrifice.", ch, obj, NULL, TO_CHAR );
	return;
    }

    gain = UMIN(number_range(1, obj->level), ch->level);

#if defined (SAC_VAR_REWARD)
    /* Idea by Bram (Unicorn Mud) Mythran Code by Maniac */
{
    switch(number_range(1, 10))
    {
            case 1:
                if (!sac)
                {
                   sprintf(arg, "%s gives you %d move points for your sacrifice.\r\n", godname, gain);
                   ch->move += gain;
                } else { sac[SAC_MOVE].reward_total += gain; }
                break;
            case 2:
                if (!sac)
                {
                   sprintf(arg, "%s gives you %d hit points for your sacrifice.\r\n", godname, gain);
                   ch->hit += gain;
                } else { sac[SAC_HP].reward_total += gain; }
                break;
            case 3:
                if (!sac)
                {
                   sprintf(arg, "%s gives you %d mana for your sacrifice.\r\n", godname, gain);
                   ch->mana += gain;
                } else { sac[SAC_MANA].reward_total += gain; }
                break;
            case 4:
                if (IS_NPC(ch))
                {
                        sprintf(arg, "%s gives you 1 gp for your sacrifice.\r\n", godname);
                        ch->gold += 1;
                }
                break;
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
                if (!sac)
                {
                   sprintf(arg, "%s gives you %d gold coin%s for your sacrifice.\r\n", godname, gain, ((gain == 1) ? "" : "s") );
                   ch->gold += gain;
                } else { sac[SAC_GOLD].reward_total += gain; }
                break;
    }
}
#else
{
        if (!sac)
        {
           sprintf(arg, "%s gives you 1 gold piece for your sacrifice.\r\n", godname);
           ch->gold += 1;
        } else { sac[SAC_GOLD].reward_total += gain; }
}
#endif
    if (!sac)
    {
       send_to_char(C_DEFAULT, arg, ch);
       sprintf (arg, "%s: %s(%d).\r\n", ch->name,
           obj->short_descr, obj->pIndexData->vnum );
       act(C_DEFAULT, "$n sacrifices $p to the gods.", ch, obj, NULL, TO_ROOM );
    }
    extract_obj( obj );
    return;

}


void do_quaff( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char      arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_BLUE, "Quaff what?\r\n", ch );
	return;
    }

    if ( !( obj = get_obj_carry( ch, arg ) ) )
    {
	send_to_char(AT_BLUE, "You do not have that potion.\r\n", ch );
	return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
	send_to_char(AT_BLUE, "You can quaff only potions.\r\n", ch );
	return;
    }

    act(AT_BLUE, "You quaff $p.", ch, obj, NULL ,TO_CHAR );
    act(AT_BLUE, "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
    if ( obj->value[1] == skill_lookup ( "aura of peace" )
      || obj->value[2] == skill_lookup ( "aura of peace" )
      || obj->value[3] == skill_lookup ( "aura of peace" ) )
    {
      extract_obj ( obj );
      return;
    }
    if ( obj->value[1] == skill_lookup ( "chaos field" )
      || obj->value[2] == skill_lookup ( "chaos field" )
      || obj->value[3] == skill_lookup ( "chaos field" ) )
    {
      if ( obj->value[0] > 50 )
       {
        extract_obj ( obj );
        return;
       }
    }
    if ( obj->value[1] == skill_lookup ( "blade barrier" )
      || obj->value[2] == skill_lookup ( "blade barrier" )
      || obj->value[3] == skill_lookup ( "blade barrier" ) )
    {
      if ( obj->value[0] > 50 )
      {
       extract_obj ( obj );
       return;
      }
    }
    if ( obj->value[1] == skill_lookup ( "vibrate" )
      || obj->value[2] == skill_lookup ( "vibrate" )
      || obj->value[3] == skill_lookup ( "vibrate" ) )
    {
      if ( obj->value[0] > 50 )
       {
       extract_obj ( obj );
       return;
       }
    }
    if ( obj->value[1] == skill_lookup ( "iceshield" )
      || obj->value[2] == skill_lookup ( "iceshield" )
      || obj->value[3] == skill_lookup ( "iceshield" ) )
    {
      if ( obj->value[0] > 50 )
       {
       extract_obj ( obj );
       return;
       }
    }
    if ( (obj->level > ch->level) && (ch->multied == ch->class) ) 
        act(AT_BLUE, "$p is too high level for you.", ch, obj, NULL, TO_CHAR );
    else
    {
	obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	oprog_use_trigger( obj, ch, ch );
    }

    extract_obj( obj );
    return;
}



void do_recite( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *scroll;
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( !( scroll = get_obj_carry( ch, arg1 ) ) )
    {
	send_to_char(AT_BLUE, "You do not have that scroll.\r\n", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	send_to_char(AT_BLUE, "You can recite only scrolls.\r\n", ch );
	return;
    }

    obj = NULL;
    if ( arg2[0] == '\0' )
    {
	victim = ch;
    }
    else
    {
	if ( !( victim = get_char_room ( ch, arg2 ) )
	    && !( obj  = get_obj_here  ( ch, arg2 ) ) )
	{
	    send_to_char(AT_BLUE, "You can't find it.\r\n", ch );
	    return;
	}
    }

    act(AT_BLUE, "You recite $p.", ch, scroll, NULL, TO_CHAR );
    act(AT_BLUE, "$n recites $p.", ch, scroll, NULL, TO_ROOM );

    /* Scrolls skill by Binky for EnvyMud, modified by Thelonius */
    if ( !IS_NPC( ch )
        && !( number_percent( ) < ( ch->pcdata->learned[skill_lookup("scrolls")] / 10 ) ) )
    {
        switch ( number_bits( 3 ) )
        {
        case 0:
        case 1:
        case 2:
        case 3:
            act(AT_BLUE, "You can't understand $p at all.",
                ch, scroll, NULL, TO_CHAR );
            act(AT_BLUE, "$n can't understand $p at all.",
                ch, scroll, NULL, TO_ROOM );
            return;
        case 4:
        case 5:
        case 6:
            send_to_char(AT_BLUE, "You must have said something incorrectly.\r\n",
                         ch );
            act(AT_BLUE, "$n must have said something incorrectly.",
                ch, NULL,   NULL, TO_ROOM );
            act(AT_BLUE, "$p blazes brightly, then is gone.",
                ch, scroll, NULL, TO_CHAR );
            act(AT_BLUE, "$p blazes brightly and disappears.",
                ch, scroll, NULL, TO_ROOM );
            extract_obj( scroll );
            return;
        case 7:
            act(AT_BLUE,
        "You completely botch the recitation, and $p bursts into flames!!",
                ch, scroll, NULL, TO_CHAR );
            act(AT_BLUE, "$p glows and then bursts into flame!",
                ch, scroll, NULL, TO_ROOM );
            /*
             * damage( ) call after extract_obj in case the damage would
             * have extracted ch.  This is okay because we merely mark
             * obj->deleted; it still retains all values until list_update.
             * Sloppy?  Okay, create another integer variable. ---Thelonius
             */
            extract_obj( scroll );
            damage( ch, ch, scroll->level, skill_lookup("scrolls"));
            return;
        }
    }


    if ( scroll->level > ch->level ) 
        act(AT_BLUE, "$p is too high level for you.", ch, scroll, NULL, TO_CHAR );
    else
    {
	obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
	obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
	obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
	if ( victim )
	  oprog_use_trigger( scroll, ch, victim );
	else
	  oprog_use_trigger( scroll, ch, obj );
	update_skpell( ch, skill_lookup("scrolls"), 0 );
    }

    extract_obj( scroll );
    return;
}



void do_brandish( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *staff;
    CHAR_DATA *vch;
    int        sn;

    if ( !( staff = get_eq_char( ch, WEAR_HOLD ) ) )
    {
	send_to_char(AT_BLUE, "You hold nothing in your hand.\r\n", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char(AT_BLUE, "You can brandish only with a staff.\r\n", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
	|| !is_sn(sn)
	|| skill_table[sn].spell_fun == 0 )
    {
	bug( "Do_brandish: bad sn %d.", sn );
	return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( ( staff->value[2] > 0 ) || ( staff->value[1] == -1 ) )
    {
        CHAR_DATA *vch_next;

	act(AT_BLUE, "You brandish $p.",  ch, staff, NULL, TO_CHAR );
	act(AT_BLUE, "$n brandishes $p.", ch, staff, NULL, TO_ROOM );

        /* Staves skill by Binky for EnvyMud, modified by Thelonius */
        if ( !IS_NPC( ch )
            && !( number_percent( ) < ( ch->pcdata->learned[skill_lookup("staves")] / 10 ) ) )
        {
            switch ( number_bits( 3 ) )
            {
            case 0:
            case 1:
            case 2:
            case 3:
                act(AT_BLUE, "You are unable to invoke the power of $p.",
                    ch, staff, NULL, TO_CHAR );
                act(AT_BLUE, "$n is unable to invoke the power of $p.",
                    ch, staff, NULL, TO_ROOM );
                return;
            case 4:
            case 5:
            case 6:
                act(AT_BLUE, "You summon the power of $p, but it fizzles away.",
                    ch, staff, NULL, TO_CHAR );
                act(AT_BLUE, "$n summons the power of $p, but it fizzles away.",
                    ch, staff, NULL, TO_ROOM );
                if ( --staff->value[2] <= 0 )
                {
                    act(AT_BLUE, "$p blazes bright and is gone.",
                        ch, staff, NULL, TO_CHAR );
                    act(AT_BLUE, "$p blazes bright and is gone.",
                        ch, staff, NULL, TO_ROOM );
                    extract_obj( staff );
                }
                return;
            case 7:
                act(AT_BLUE, "You can't control the power of $p, and it shatters!",
                    ch, staff, NULL, TO_CHAR );
                act(AT_BLUE, "$p shatters into tiny pieces!",
                    ch, staff, NULL, TO_ROOM );
                /*
                 * damage( ) call after extract_obj in case the damage would
                 * have extracted ch.  This is okay because we merely mark
                 * obj->deleted; it still retains all values until list_update.
                 * Sloppy?  Okay, create another integer variable. ---Thelonius
                 */
                extract_obj( staff );
                damage( ch, ch, staff->level, skill_lookup("staves"));
                return;
            }
        }


	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;

	    if ( vch->deleted )
	        continue;

	    switch ( skill_table[sn].target )
	    {
	    default:
		bug( "Do_brandish: bad target for sn %d.", sn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC( ch ) ? IS_NPC( vch ) : !IS_NPC( vch ) )
		    continue;
		break;
		
	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

	    obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
	    update_skpell( ch, skill_lookup("staves"), 0 );

	    oprog_use_trigger( staff, ch, vch );
	}
    }
    if (!(staff->value[1] == -1 ))
    if ( --staff->value[2] <= 0 )
    {
	act(AT_WHITE, "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
	act(AT_WHITE, "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
	extract_obj( staff );
    }

    return;
}


void do_stare ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *wand;
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );
    if ( arg[0] == '\0' && !ch->fighting )
    {
	send_to_char(AT_BLUE, "Stare at whom or what?\r\n", ch );
	return;
    }

    if ( !( wand = get_eq_char( ch, WEAR_IN_EYES ) ) )
    {
	send_to_char(AT_BLUE, "You have no lenses in your eyes.\r\n", ch );
	return;
    }

    if ( wand->item_type != ITEM_LENSE )
    {
	send_to_char(AT_BLUE, "You can only stare with magical lenses.\r\n", ch );
	return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting )
	{
	    victim = ch->fighting;
	}
	else
	{
	    send_to_char(AT_BLUE, "Stare at whom or what?\r\n", ch );
	    return;
	}
    }
    else
    {
	if ( !( victim = get_char_room ( ch, arg ) )
	    && !( obj  = get_obj_here  ( ch, arg ) ) )
	{
	    send_to_char(AT_BLUE, "You can't find it.\r\n", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( ( wand->value[2] > 0 ) || ( wand->value[1] == -1 ) )
    {
	if ( victim )
	{
	    act(AT_BLUE, "You stare at $N with $p.", ch, wand, victim, TO_CHAR );
	    act(AT_BLUE, "$n stares at $N with $p.", ch, wand, victim, TO_ROOM );
	}
	else
	{
	    act(AT_BLUE, "You stare at $P with $p.", ch, wand, obj, TO_CHAR );
	    act(AT_BLUE, "$n stares at $P with $p.", ch, wand, obj, TO_ROOM );
	}

	obj_cast_spell( wand->value[3], wand->level, ch, victim, obj );
	if ( victim )
	  oprog_use_trigger( wand, ch, victim );
	else
	  oprog_use_trigger( wand, ch, obj );
    }

    if (!(wand->value[1] == -1 ) )
    if ( --wand->value[2] <= 0 )
    {
	act(AT_WHITE, "Your $p melts in your eyes.", ch, wand, NULL, TO_CHAR );
	act(AT_WHITE, "$n's $p melts in $s eyes.", ch, wand, NULL, TO_ROOM );
	extract_obj( wand );
    }

    return;
}

void do_fire( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *wand;
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    char	arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg);
    if( arg[0] == '\0' && !ch->fighting )
    {
	send_to_char(AT_BLUE, "Fire at whom or what?\r\n", ch );
	return;
    }

    if ( !( wand = get_eq_char( ch, WEAR_FIREARM ) ) )
    {
	send_to_char(AT_BLUE, "You do not have a firearm equipped.\r\n", ch );
	return;
    }

    if ( wand->item_type != ITEM_GUN )
    {
	send_to_char(AT_BLUE, "You can only fire a firearm.\r\n", ch );
	return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting )
	{
	     victim = ch->fighting;
	}
	else
	{
	    send_to_char(AT_BLUE, "Fire at whom or what?\r\n", ch );
	    return;
	}
    }
    else
    {
	if ( !(victim = get_char_room ( ch, arg ) ) )
	{
	    send_to_char(AT_BLUE, "You can't find them.\r\n", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( ( wand->value[2] > 0 || wand->value[1] == -1 ) )
    {
	if ( victim )
	{
	    act(AT_BLUE, "You hit $N with $p.", ch, wand, victim, TO_CHAR );
	    act(AT_BLUE, "$n hits $N with $p.", ch, wand, victim, TO_CHAR );
	}
	else
	{
	    act(AT_BLUE, "You hit $P with $p.", ch, wand, obj, TO_CHAR );
	    act(AT_BLUE, "$n hits $P with $p.", ch, wand, obj, TO_ROOM );
	}
	obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
	if ( victim )
	  oprog_use_trigger(wand, ch, victim );
	else
	  oprog_use_trigger( wand, ch, obj );
    }

    if (!(wand->value[1] == -1 ) )
    if ( --wand->value[2] <= 0 )
    {
	act(AT_WHITE, "Your $p is out of ammo.", ch, wand, NULL, TO_CHAR );
	act(AT_WHITE, "$n's $p clicks, because it is out of ammo.", ch, wand, NULL, TO_ROOM );
    }

    return;
}

void do_zap( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *wand;
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );
    if ( arg[0] == '\0' && !ch->fighting )
    {
	send_to_char(AT_BLUE, "Zap whom or what?\r\n", ch );
	return;
    }

    if ( !( wand = get_eq_char( ch, WEAR_HOLD ) ) )
    {
	send_to_char(AT_BLUE, "You hold nothing in your hand.\r\n", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char(AT_BLUE, "You can zap only with a wand.\r\n", ch );
	return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting )
	{
	    victim = ch->fighting;
	}
	else
	{
	    send_to_char(AT_BLUE, "Zap whom or what?\r\n", ch );
	    return;
	}
    }
    else
    {
	if ( !( victim = get_char_room ( ch, arg ) )
	    && !( obj  = get_obj_here  ( ch, arg ) ) )
	{
	    send_to_char(AT_BLUE, "You can't find it.\r\n", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( ( wand->value[2] > 0 ) || ( wand->value[1] == -1 ) )
    {
	if ( victim )
	{
	    act(AT_BLUE, "You zap $N with $p.", ch, wand, victim, TO_CHAR );
	    act(AT_BLUE, "$n zaps $N with $p.", ch, wand, victim, TO_ROOM );
	}
	else
	{
	    act(AT_BLUE, "You zap $P with $p.", ch, wand, obj, TO_CHAR );
	    act(AT_BLUE, "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
	}

        /* Wands skill by Binky for EnvyMud, modified by Thelonius */
        if ( !IS_NPC( ch )
            && !( number_percent( ) < ( ch->pcdata->learned[skill_lookup("wands")] / 10 ) ) )
        {
            switch ( number_bits( 3 ) )
            {
            case 0:
            case 1:
            case 2:
            case 3:
                act(AT_BLUE, "You are unable to invoke the power of $p.",
                    ch, wand, NULL, TO_CHAR );
                act(AT_BLUE, "$n is unable to invoke the power of $p.",
                    ch, wand, NULL, TO_ROOM );
                return;
            case 4:
            case 5:
            case 6:
                act(AT_BLUE, "You summon the power of $p, but it fizzles away.",
                    ch, wand, NULL, TO_CHAR );
                act(AT_BLUE, "$n summons the power of $p, but it fizzles away.",
                    ch, wand, NULL, TO_ROOM );
                if ( --wand->value[2] <= 0 )
                {
                    act(AT_BLUE, "$p blazes bright and is gone.",
                        ch, wand, NULL, TO_CHAR );
                    act(AT_BLUE, "$p blazes bright and is gone.",
                        ch, wand, NULL, TO_ROOM );
                    extract_obj( wand );
                }
                return;
            case 7:
                act(AT_BLUE, "You can't control the power of $p, and it explodes!",
                    ch, wand, NULL, TO_CHAR );
                act(AT_BLUE, "$p explodes into fragments!",
                    ch, wand, NULL, TO_ROOM );
                /*
                 * damage( ) call after extract_obj in case the damage would
                 * have extracted ch.  This is okay because we merely mark
                 * obj->deleted; it still retains all values until list_update.
                 * Sloppy?  Okay, create another integer variable. ---Thelonius
                 */
                extract_obj( wand );
                damage( ch, ch, wand->level, skill_lookup("wands"));
                return;
            }
        }


	/* wand->value[0] is not used for wands */
	obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
	if ( victim )
	  oprog_use_trigger(wand, ch, victim );
	else
	  oprog_use_trigger( wand, ch, obj );
	update_skpell( ch, skill_lookup("wands"), 0 );

    }

    if (!(wand->value[1] == -1 ) )
    if ( --wand->value[2] <= 0 )
    {
	act(AT_WHITE, "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
	act(AT_WHITE, "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
	extract_obj( wand );
    }

    return;
}



void do_steal( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    char       buf  [ MAX_STRING_LENGTH ];
    char       arg1 [ MAX_INPUT_LENGTH  ];
    char       arg2 [ MAX_INPUT_LENGTH  ];
    int        percent;
    int		   sn = skill_lookup("steal");

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char(AT_BLOOD, "Steal what from whom?\r\n", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, arg2 ) ) )
    {
	send_to_char(AT_BLOOD, "They aren't here.\r\n", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char(AT_BLOOD, "That's pointless.\r\n", ch );
	return;
    }

    if ( IS_AFFECTED2( ch, AFF_SHADOW_PLANE ) )
    {
	if (IS_AFFECTED4( ch, AFF_THIEVESCANT ))
        {
	   send_to_char(AT_BLUE, "You quickly remove yourself from the shadow plane!\n\r", ch);
	} else
	{
	   send_to_char(AT_BLOOD, "You can not steal while in the shadow plane.\r\n", ch );
	   return;
	}
    }

    if ( IS_AFFECTED4( ch, AFF_BURROW ) )
    {
	if (IS_AFFECTED4( ch, AFF_THIEVESCANT ))
        {
	   send_to_char(AT_BLUE, "You quickly rise from the earth!\n\r", ch);
	} else
	{
	   send_to_char(AT_BLOOD, "You can not steal while burrowed!\r\n", ch );
	   return;
	}
    }

    if ( IS_NPC(ch) && !IS_NPC( victim ) )
    {
	send_to_char (AT_WHITE, "You can't do that.\r\n", ch );
	return;
    }

    if ( !IS_NPC(ch) && !IS_NPC( victim ) ) 
        {
        if ( ( ch->pkill == FALSE ) || ( victim->pkill == FALSE) ) 
           {
           send_to_char (AT_WHITE, "Can't steal from non-pkillers or can't steal when you are non-pkill.\r\n", ch);
           return;
           }
	if ( victim->pkill_timer > 0 )
		{
	send_to_char(AT_WHITE, "Can't steal from people who have recently\r\n died, wait a few...\r\n", ch);
		return;

}


	if ( ch->level < 30 && ch->pkill == TRUE )
	   {
	   send_to_char( AT_WHITE, "Can't steal from players below level 30.\r\n", ch );
	   return;
	   }
	if ( victim->level < 30 && victim->pkill == TRUE )
	   {
	   send_to_char( AT_WHITE, "Your victim must be at least level 30.\r\n", ch );
	   return;
	   }
	if ( ( (ch->level + PKILL_RANGE ) < victim->level) ||
           ((ch->level - PKILL_RANGE ) > victim->level) ) 
           {
	   send_to_char(AT_WHITE, "Can only steal from players in pkill range.\r\n", ch );
           return;  
 }
    }

    WAIT_STATE( ch, skill_table[sn].beats );
    percent  = number_percent( ) + ( IS_AWAKE( victim ) ? 10 : -50 );
    if (IS_AFFECTED4( ch, AFF_THIEVESCANT ))
    {
	update_skpell(ch, skill_lookup("thieves cant"), 0);
	percent = ((percent*2)/3);
    }

    if (!IS_NPC(ch) && ch->pcdata->learned[sn])
	update_skpell(ch, sn, 0);

    ch->pkill_timer = 0;

    if ( ch->level + PKILL_RANGE < victim->level
	|| victim->position == POS_FIGHTING
	|| ( !IS_NPC( ch ) && percent > ( ch->pcdata->learned[sn] / 10 ) ) )
    {
	/*
	 * Failure.
	 */
	send_to_char(AT_RED, "Oops.\r\n", ch );
	act(AT_RED, "$n tried to steal from you.\r\n", ch, NULL, victim, TO_VICT    );
	act(AT_RED, "$n tried to steal from $N.\r\n",  ch, NULL, victim, TO_NOTVICT );
	sprintf( buf, "%s is a bloody thief!", ch->name );
        ch->pkill_timer = 0;
	do_shout( victim, buf );
	if ( !IS_NPC( ch ) )
	{
	    if ( IS_NPC( victim ) )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
	    }
	    else
	    {
		log_string( buf, CHANNEL_GOD, -1 );  
		if ( !IS_SET( ch->act, PLR_THIEF ) )
		{
		    send_to_char(AT_RED, "You have been caught!\r\n", ch );
		    save_char_obj( ch, FALSE );
		}
	    }
	}

	return;
    }

    if (   !str_prefix( arg1, "coins" )
	|| !str_cmp   ( arg1, "gold"  ) )
    {
	int amount;

	amount = victim->gold * number_range( 1, 10 ) / 50;
	if ( amount <= 0 )
	{
	    send_to_char(AT_BLOOD, "You couldn't get any gold.\r\n", ch );
	    return;
	}

	ch->gold     += amount;
	victim->gold -= amount;
	sprintf( buf, "Jackpot!  You got %d gold coins.\r\n", amount );
	send_to_char(AT_RED, buf, ch );
	return;
    }

    if ( !( obj = get_obj_carry( victim, arg1 ) ) )
    {
	send_to_char(AT_BLOOD, "You can't find it.\r\n", ch );
	return;
    }
	
    if ( !can_drop_obj( ch, obj )
	|| IS_SET( obj->extra_flags, ITEM_INVENTORY )
	|| obj->level > ch->level
	|| IS_SET( obj->extra_flags2, ITEM_NO_STEAL ) )
    {
	if (IS_AFFECTED4( ch, AFF_THIEVESCANT ) && (number_percent( ) < 2) )
    	{
	   send_to_char(AT_BLOOD, "Your ancient knowledge allows you to pry it away!\r\n", ch);
	   send_to_char(AT_BLOOD, "Your pack feels lighter...\r\n", victim);
	} else
	{
	   send_to_char(AT_BLOOD, "You can't pry it away.\r\n", ch );
	   return;
	}
    }

    /* make sure the carry number is correct - Ahsile */
    ch->carry_number = ch_invcount( ch );
    ch->carry_weight = ch_weightcount( ch );
    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	send_to_char(AT_BLOOD, "You have your hands full.\r\n", ch );
	return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
	send_to_char(AT_BLOOD, "You can't carry that much weight.\r\n", ch );
	return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    send_to_char(AT_RED, "Ok.\r\n", ch );
    return;
}



/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;
    char       buf [ MAX_STRING_LENGTH ];

    pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
	if ( IS_NPC( keeper ) && ( pShop = keeper->pIndexData->pShop ) )
	    break;
    }

    if ( !pShop || IS_AFFECTED( keeper, AFF_CHARM ) || !IS_AWAKE(ch) )
    {
	send_to_char(C_DEFAULT, "You can't do that here.\r\n", ch );
	return NULL;
    }

    /*
     * Undesirables.
     */
    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_KILLER ) )
    {
	do_say( keeper, "Killers are not welcome!" );
	sprintf( buf, "%s the KILLER is over here!\r\n", ch->name );
	do_shout( keeper, buf );
	return NULL;
    }

    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_THIEF ) )
    {
	do_say( keeper, "Thieves are not welcome!" );
	sprintf( buf, "%s the THIEF is over here!\r\n", ch->name );
	do_shout( keeper, buf );
	return NULL;
    }

    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour )
    {
	do_say( keeper, "Sorry, come back later." );
	return NULL;
    }
    
    if ( time_info.hour > pShop->close_hour )
    {
	do_say( keeper, "Sorry, come back tomorrow." );
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
	do_say( keeper, "I don't trade with folks I can't see." );
	return NULL;
    }

    return keeper;
}



int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop;
    int        cost;

    if ( !obj || !( pShop = keeper->pIndexData->pShop ) )
	return 0;

    if ( fBuy )
    {
	cost = obj->cost * pShop->profit_buy  / 100;
    }
    else
    {
	OBJ_DATA *obj2;
	int       itype;

	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
		cost = obj->cost * pShop->profit_sell / 100;
		break;
	    }
	}

	for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
	{
	    if ( obj->pIndexData == obj2->pIndexData )
		cost /= 2;
	}
    }

    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
	cost = cost * obj->value[2] / obj->value[1];

    return cost;
}



void do_buy( CHAR_DATA *ch, char *argument )
{
    char arg [ MAX_INPUT_LENGTH ];
    char arg1[MAX_STRING_LENGTH];
    int noi = 1;
    int in = 1;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_CYAN, "Buy what?\r\n", ch );
	return;
    }
    
    if ( arg1[0] == '\0' )
      noi = 1;
    else
      noi = atoi( arg1 );

    if ( IS_SET( ch->in_room->room_flags, ROOM_PET_SHOP ) )
    {
	CHAR_DATA       *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;
	char             buf [ MAX_STRING_LENGTH ];

	if ( IS_NPC( ch ) )
	    return;

	if ( noi > 1 )
	{
	  send_to_char( AT_CYAN, "You can only buy one pet at a time.\r\n",ch);
	  return;
	}

	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( !pRoomIndexNext )
	{
	    bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char(AT_CYAN, "Sorry, you can't buy that here.\r\n", ch );
	    return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet         = get_char_room( ch, arg );
	ch->in_room = in_room;

	if ( !pet || !IS_SET( pet->act, ACT_PET ) )
	{
	    send_to_char(AT_CYAN, "Sorry, you can't buy that here.\r\n", ch );
	    return;
	}

	if ( IS_SET( ch->act, PLR_BOUGHT_PET ) )
	{
	    send_to_char(AT_CYAN, "You already bought one pet this level.\r\n", ch );
	    return;
	}

	if ( ch->gold < 10 * pet->level * pet->level )
	{
	    send_to_char(AT_CYAN, "You can't afford it.\r\n", ch );
	    return;
	}

	if ( ch->level < pet->level )
	{
	    send_to_char(AT_CYAN, "You're not ready for this pet.\r\n", ch );
	    return;
	}

	ch->gold -= 10 * pet->level * pet->level;
	pet	  = create_mobile( pet->pIndexData );
	if ( ch->level < 100 )
	SET_BIT( ch->act,          PLR_BOUGHT_PET );
	SET_BIT( pet->act,         ACT_PET        );
	SET_BIT( pet->affected_by, AFF_CHARM      );

	argument = one_argument( argument, arg );
	if ( arg[0] != '\0' )
	{
	    sprintf( buf, "%s %s", pet->name, arg );
	    free_string( pet->name );
	    pet->name = str_dup( buf );
	}

	sprintf( buf, "%sA neck tag says 'I belong to %s'.\r\n",
		pet->description, ch->name );
	free_string( pet->description );
	pet->description = str_dup( buf );

	char_to_room( pet, ch->in_room );
	add_follower( pet, ch );
	send_to_char(AT_WHITE, "Enjoy your pet.\r\n", ch );
	act(AT_WHITE, "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
	return;
    }
    else
    {
	OBJ_DATA  *obj;
	CHAR_DATA *keeper;
	int        cost;

	if ( !( keeper = find_keeper( ch ) ) )
	    return;

	obj  = get_obj_carry( keeper, arg );
	cost = get_cost( keeper, obj, TRUE ) * noi;

	if ( cost <= 0 || !can_see_obj( ch, obj ) )
	{
	    act(AT_CYAN, "$n tells you 'I don't sell that -- try 'list''.",
		keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}
	
	if ( !IS_SET( obj->extra_flags, ITEM_INVENTORY ) && noi > 1 )
	{
	  send_to_char( AT_WHITE, "You can only buy one of those at a time.\r\n", ch );
	  return;
	}
	
	if ( noi < 1 )
	{
	   send_to_char( AT_WHITE, "Buy how many?\r\n", ch );
	   return;
	}

	if ( ch->gold < cost )
	{
	  if ( noi == 1 )
	    sprintf( log_buf, "$n tells you 'You can't afford to buy $p." );
	  else
	    sprintf( log_buf, "$n tells you 'You can't afford to buy %d $ps.",
		     noi );
	  act(AT_CYAN, log_buf, keeper, obj, ch, TO_VICT );
	  ch->reply = keeper;
	  return;
	}
	
	if ( (obj->level > ch->level) && (ch->multied == ch->class) )
	{
	    act(AT_CYAN, "$n tells you 'You can't use $p yet'.",
		keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}

        /* make sure the carry number is correct - Ahsile */
        ch->carry_number = ch_invcount( ch );
	ch->carry_weight = ch_weightcount( ch );
        if ( ch->carry_number + ( get_obj_number( obj ) * noi ) > can_carry_n( ch ) )
	{
	    send_to_char(AT_CYAN, "You can't carry that many items.\r\n", ch );
	    return;
	}

	if ( ch->carry_weight + ( get_obj_weight( obj ) * noi ) > can_carry_w( ch ) )
	{
	    send_to_char(AT_CYAN, "You can't carry that much weight.\r\n", ch );
	    return;
	}

        if ( noi == 1 )
	{
	  act(AT_WHITE, "You buy $p.", ch, obj, NULL, TO_CHAR );
	  act(AT_WHITE, "$n buys $p.", ch, obj, NULL, TO_ROOM );
	}
       else
        {
          sprintf( log_buf, "You buy %d $p%s.", noi, ( noi > 1 ) ? "s" : "" );
          act(AT_WHITE, log_buf, ch, obj, NULL, TO_CHAR );
          sprintf( log_buf, "$n buys %d $p%s.", noi, ( noi > 1 ) ? "s" : "" );
          act(AT_WHITE, log_buf, ch, obj, NULL, TO_ROOM );
        }

	ch->gold     -= cost;
	keeper->gold += cost;

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
        {
          for ( in = 1; in <= noi; in++ )
          {
	    obj = create_object( obj->pIndexData, obj->level );
	    obj_to_char( obj, ch );
	  }
	}
       else
        {
	  obj_from_char( obj );
	  obj_to_char( obj, ch );
	}
	oprog_buy_trigger( obj, ch, keeper );
	
	return;
    }
}



void do_list( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH   ];
    char buf1 [ MAX_STRING_LENGTH*4 ];

    buf1[0] = '\0';

    if ( IS_SET( ch->in_room->room_flags, ROOM_PET_SHOP ) )
    {
	CHAR_DATA       *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	bool             found;

	pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
	if ( !pRoomIndexNext )
	{
	    bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
	    send_to_char(AT_CYAN, "You can't do that here.\r\n", ch );
	    return;
	}

	found = FALSE;
	for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
	{
	    if ( IS_SET( pet->act, ACT_PET ) )
	    {
		if ( !found )
		{
		    found = TRUE;
		    strcat( buf1, "Pets for sale:\r\n" );
		}
		sprintf( buf, "[%2d] %8d - %s\r\n",
			pet->level,
			10 * pet->level * pet->level,
			pet->short_descr );
		strcat( buf1, buf );
	    }
	}
	if ( !found )
	    send_to_char(AT_CYAN, "Sorry, we're out of pets right now.\r\n", ch );

	send_to_char(AT_CYAN, buf1, ch );
	return;
    }
    else
    {
	OBJ_DATA  *obj;
	CHAR_DATA *keeper;
	char       arg [ MAX_INPUT_LENGTH ];
	int        cost;
	bool       found;

	one_argument( argument, arg );

	if ( !( keeper = find_keeper( ch ) ) )
	    return;

	found = FALSE;
	for ( obj = keeper->carrying; obj; obj = obj->next_content )
	{
	    if ( obj->wear_loc != WEAR_NONE
		|| ( cost = get_cost( keeper, obj, TRUE ) ) < 0 )
	        continue;

	    if ( can_see_obj( ch, obj )
		&& ( arg[0] == '\0' || is_name( arg, obj->name ) ) )
	    {
		if ( !found )
		{
		    found = TRUE;
		    strcat( buf1, "[Lv Price] Item\r\n" );
		}

		sprintf( buf, "[%2d %5d] %s.\r\n",
			obj->level, cost, capitalize( obj->short_descr ) );
		strcat( buf1, buf );
	    }
	}

	if ( !found )
	{
	    if ( arg[0] == '\0' )
		send_to_char(AT_CYAN, "You can't buy anything here.\r\n", ch );
	    else
		send_to_char(AT_CYAN, "You can't buy that here.\r\n", ch );
	    return;
	}

	send_to_char(AT_CYAN, buf1, ch );
	return;
    }
}



void do_sell( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *obj;
    CHAR_DATA *keeper;
    char       buf [ MAX_STRING_LENGTH ];
    char       arg [ MAX_INPUT_LENGTH  ];
    int        cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_CYAN, "Sell what?\r\n", ch );
	return;
    }

    if ( !( keeper = find_keeper( ch ) ) )
	return;

    if ( !( obj = get_obj_carry( ch, arg ) ) )
    {
	act(AT_CYAN, "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ))
    {
//		if (!IS_SET(obj->extra_flags2, ITEM_QUEST))
			send_to_char(AT_CYAN, "You can't let go of it.\r\n", ch );
/*		else
			send_to_char(AT_CYAN, "Your precious quest equipment?\r\n", ch); */
	return;
    }

    if ( !can_see_obj( keeper, obj ) )
    {
	act(AT_CYAN, "$n tells you 'I can't see that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }
	
    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0
	|| obj->level > LEVEL_CHAMP )
    {
	act(AT_CYAN, "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    if ( IS_SET( obj->extra_flags, ITEM_POISONED ) )
    {
        act(AT_CYAN, "$n tells you 'I won't buy that!  It's poisoned!'",
	    keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    sprintf( buf, "You sell $p for %d gold piece%s.",
	    cost, cost == 1 ? "" : "s" );
    act(AT_WHITE, buf, ch, obj, NULL, TO_CHAR );
    act(AT_WHITE, "$n sells $p.", ch, obj, NULL, TO_ROOM );
    ch->gold     += cost;
    keeper->gold -= cost;
    if ( keeper->gold < 0 )
	keeper->gold = 0;

    oprog_sell_trigger( obj, ch, keeper );

    if ( obj->item_type == ITEM_TRASH )
    {
	extract_obj( obj );
    }
    else
    {
	obj_from_char( obj );
	obj_to_char( obj, keeper );
    }

    return;
}



void do_value( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *obj;
    CHAR_DATA *keeper;
    char       buf [ MAX_STRING_LENGTH ];
    char       arg [ MAX_INPUT_LENGTH  ];
    int        cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_CYAN, "Value what?\r\n", ch );
	return;
    }

    if ( !( keeper = find_keeper( ch ) ) )
	return;

    if ( !( obj = get_obj_carry( ch, arg ) ) )
    {
	act(AT_CYAN, "$n tells you 'You don't have that item'.",
	    keeper, NULL, ch, TO_VICT );
	ch->reply = keeper;
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
//		if ( !IS_SET(obj->extra_flags2, ITEM_QUEST) )
//		{
			send_to_char(AT_CYAN, "You can't let go of it.\r\n", ch );
/*		} else
		{
			send_to_char(AT_CYAN, "Your precious quest equipment?\r\n", ch);
		} */
	return;
    }

    if ( !can_see_obj( keeper, obj ) )
    {
        act(AT_CYAN, "$n tells you 'You are offering me an imaginary object!?!?'.",
            keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
    {
	act(AT_CYAN, "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    if ( IS_SET( obj->extra_flags, ITEM_POISONED ) )
    {
        act(AT_CYAN, "$n tells you 'I won't buy that!  It's poisoned!'",
	    keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    sprintf( buf, "$n tells you 'I'll give you %d gold coins for $p'.", cost );
    act(AT_WHITE, buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;

    return;
}

void check_permanent( CHAR_DATA *ch, OBJ_DATA *obj, bool PICKED_UP )
{
/* TRUE if picking up, FALSE if dropping */ 

    /*FILE *fp; */

/* Check to see if the room can have a permanent item */

/* if PICKED_UP is true, then remove a line from the PERMOBJ.TXT file */
/* To do this, read each line, 1 at a time, and then DON'T write the
   appropriate line */

/* if PICKED_UP is false, then add a line to the PERMOBJ.TXT file */
/* To do this, append a line to the end of the file. */

/* File format:  character who dropped, object vnum, room vnum */

/* all done */

    return;
}

/* Poison weapon by Thelonius for EnvyMud */
int skill_poison_weapon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj;

    if ( target_name[0] == '\0' )                                              
    { send_to_char(AT_DGREEN, "What are you trying to poison?\r\n",    ch ); return SKPELL_MISSED; }
    if ( ch->fighting )                                       
    { send_to_char(AT_DGREEN, "While you're fighting?  Nice try.\r\n", ch ); return SKPELL_MISSED; }
    if ( !( obj = get_obj_carry( ch, target_name ) ) )
    { send_to_char(AT_DGREEN, "You do not have that weapon.\r\n",      ch ); return SKPELL_MISSED; }
    if ( obj->item_type != ITEM_WEAPON )
    { send_to_char(AT_DGREEN, "That item is not a weapon.\r\n",        ch ); return SKPELL_MISSED; }
    if ( IS_OBJ_STAT( obj, ITEM_POISONED ) )
    { send_to_char(AT_DGREEN, "That weapon is already poisoned.\r\n",  ch ); return SKPELL_MISSED; }

    /* Now we have a valid weapon...check to see if we have the powder. */

    /* Great, we have the ingredients...but is the thief smart enough? */
    if ( !IS_NPC( ch ) && get_curr_wis( ch ) < 19 )
    {
	send_to_char(AT_DGREEN, "You can't quite remember what to do...\r\n", ch );
	return SKPELL_BOTCHED;
    }
    /* And does the thief have steady enough hands? */
    if ( !IS_NPC( ch )
	&& ( get_curr_dex( ch ) < 20
	    || ch->pcdata->condition[COND_DRUNK] > 0 ) )
    {
	send_to_char(AT_DGREEN,
	"Your hands aren't steady enough to properly mix the poison.\r\n",
								ch );
	return SKPELL_BOTCHED;
    }

    WAIT_STATE( ch, skill_table[sn].beats );


    /* Well, I'm tired of waiting.  Are you? */
    act(AT_GREEN, "You mix a deadly poison!",
	ch, NULL, NULL, TO_CHAR );
    act(AT_GREEN, "$n mixes a deadly poison!",
	ch, NULL, NULL, TO_ROOM );
    act(AT_GREEN, "You pour the poison over $p, which glistens wickedly!",
	ch, obj, NULL, TO_CHAR  );
    act(AT_GREEN, "$n pours the poison over $p, which glistens wickedly!",
	ch, obj, NULL, TO_ROOM  );
    SET_BIT( obj->extra_flags, ITEM_POISONED );
    

    /* WHAT?  All of that, just for that one bit?  How lame. ;) */
    return SKPELL_NO_DAMAGE;
}

// Count number of items a player is carrying, including items
// in containers.
// ahsile
int ch_invcount( CHAR_DATA *ch )
{
   OBJ_DATA* obj;
   int count = 0;

   if ( IS_NPC(ch) ) { return 0; }
   if ( ch->carrying == NULL ) { return 0; }

   for (obj=ch->carrying;obj;obj = obj->next_content)
   {
      count++;
      if ( obj->contains != NULL ) 
      {
        /* send_to_char(AT_WHITE,"Something inside!\n",ch) */;
        count += obj_invcount(obj->contains, FALSE);
      }
   }
   return count;
}

int obj_invcount( OBJ_DATA *obj, bool one_item )
{
   int count = 0;

   if (!one_item)
   {
      for (obj=obj;obj;obj=obj->next_content)
      {
         count++;
         if ( obj->contains != NULL ) 
         {
            count += obj_invcount(obj->contains,FALSE);
         }
      }
   } else
   {
      count++;
      if ( obj->contains != NULL )
      {
         count += obj_invcount(obj->contains,FALSE);
      }
   }
   return count;
}

// Count weight of items a player is carrying, including items
// in containers.
// ahsile
int ch_weightcount( CHAR_DATA *ch )
{
   OBJ_DATA* obj;
   int count = 0;

   if ( IS_NPC(ch) ) { return 0; }
   if ( ch->carrying == NULL ) { return 0; }

   for (obj=ch->carrying;obj;obj = obj->next_content)
   {
      count+= obj->weight;
      if ( obj->contains != NULL )
      {
        /* send_to_char(AT_WHITE,"Something inside!\n",ch) */;
        count += obj_weightcount(obj->contains,FALSE);
      }
   }
   return count;
}

int obj_weightcount( OBJ_DATA *obj, bool one_item )
{
   int count = 0;

   if (!one_item)
   {
      for (obj=obj;obj;obj=obj->next_content)
      {
         count+= obj->weight;
         if ( obj->contains != NULL )
         {
            count += obj_weightcount(obj->contains,FALSE);
         }
      }
   } else 
   {
      count+= obj->weight;
      if ( obj->contains != NULL )
      {
         count += obj_weightcount(obj->contains,FALSE);
      }
   }
   return count;
}

// Count number of items a player is carrying, including items
// in containers.
// ahsile
void ch_stripbadinv( CHAR_DATA *ch )
{
   OBJ_DATA* obj;

   if ( IS_NPC(ch) ) { return; }
   if ( ch->carrying == NULL ) { return; }

   for (obj=ch->carrying;obj;obj = obj->next_content)
   {
        // If there was an offending string found in the object, delete the object
        if ( !str_cmp(obj->name, OFFENDING_STRING) || !str_cmp(obj->short_descr, OFFENDING_STRING) || !str_cmp(obj->description, OFFENDING_STRING) || !str_cmp(obj->ac_spell, OFFENDING_STRING) )
        {
        	ch->fixed_error = TRUE;
                extract_obj(obj);
        } else if ( obj->contains != NULL ) 
        {
         	/* send_to_char(AT_WHITE,"Something inside!\n",ch) */;
        	obj_stripbadinv(obj->contains, ch);
        }
   }
}

void obj_stripbadinv( OBJ_DATA *obj, CHAR_DATA *ch )
{
      for (obj=obj;obj;obj=obj->next_content)
      {
         // If there was an offending string found in the object, delete the object
         if ( !str_cmp(obj->name, OFFENDING_STRING) || !str_cmp(obj->short_descr, OFFENDING_STRING) ||!str_cmp(obj->description, OFFENDING_STRING) || !str_cmp(obj->ac_spell, OFFENDING_STRING) )
         {
         	ch->fixed_error = TRUE;
	 	extract_obj(obj);
         } else if ( obj->contains != NULL ) 
         {
            obj_stripbadinv(obj->contains, ch);
         }
      }
}

