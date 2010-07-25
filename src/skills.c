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

/*$Id: skills.c,v 1.25 2005/03/22 21:17:16 ahsile Exp $*/

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
 * Local functions
 */
bool 	long_range_skill( int sn );

/*
 * External functions.
 */
bool    is_safe     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );


/*
 * Local functions.
 */
void    set_fighting    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int     slot_lookup     args( ( int slot ) );


/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_use( CHAR_DATA *ch, char *argument )
{
    void      *vo;
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    char       arg3 [ MAX_INPUT_LENGTH ];
    int        move;
    int        sn = 0;
    char       buf [ MAX_STRING_LENGTH ];

    argument = target_name = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' )
    {
	send_to_char(AT_BLUE, "Use which what where?\n\r", ch );
	return;
    }

    if ( !IS_NPC( ch ) )
      if ( IS_AFFECTED4( ch, AFF_BURROW ) )
      {
         send_to_char(AT_RED, "You cannot use skills while burrowed!\n\r", ch);
         return;
      }
    if ( IS_NPC( ch ) )
      if ( IS_SET( ch->affected_by, AFF_CHARM ) )
        return;

	if ( ( sn = skill_lookup( arg1 ) ) < 0 )
	{
		send_to_char(AT_BLUE, "You can't do that.\n\r", ch );
		return;
	}

    if ( !IS_NPC( ch ) )
	{
		if ( ( ch->level < skill_table[sn].skill_level[ch->class]
 		&& ch->level < skill_table[sn].skill_level[ch->multied] ) )
		{
		send_to_char(AT_BLUE, "You can't do that.\n\r", ch );
		return;
		}
	}


	if (skill_table[sn].is_spell)
	{
		send_to_char(AT_WHITE, "Using a spell? Try casting...\n\r", ch);
		return;
	}

	if( !IS_NPC( ch ) )
	{
		/* 286 = quickburst */
		/* 395 = multiburst */
		if (skill_table[sn].spell_fun == spell_null && sn != 286 && sn != 395)
		{
			send_to_char(AT_WHITE,"You can't use passive skills.\n\r",ch);
			sprintf( buf, "Do use: passive use of skill attempted by %s for skill: %s sn: %d ", ch->name, skill_table[sn].name, sn );
			bug( buf,0 );
			return;
		}
	}

    if ( IS_NPC( ch ) )
     if ( ( sn = skill_lookup( arg1 ) ) < 0 )
       return;

    if ( ch->position < skill_table[sn].minimum_position )
    {
	send_to_char(AT_BLUE, "You can't concentrate enough.\n\r", ch );
	return;
    }

    if ( IS_STUNNED( ch, STUN_MAGIC ) )
    {
      send_to_char(AT_LBLUE, "You're too stunned to use skills.\n\r", ch );
      return;
    }

    move = 0;
    if(!IS_NPC( ch ) && ch->level >= skill_table[sn].skill_level[ch->multied] )
    {
	move = MANA_COST_MULTI(ch, sn );
    }
    if(!IS_NPC( ch ) && ch->level >= skill_table[sn].skill_level[ch->class] )
    {
	move = MANA_COST( ch, sn );
    }

    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Do_use: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if (!ch->in_room)
	{
		bug("do_use: ch->in_room is NULL!",0);
		return;
	}
	if ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) )
	{
		send_to_char( AT_BLUE, "You failed.\n\r", ch );
		return;
	}

	if ( arg2[0] == '\0' )
	{
	    if ( !( victim = ch->fighting ) )
	    {
		send_to_char(AT_BLUE, "Use that skill on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( !( victim = get_char_room( ch, arg2 ) ) )
	    {
		if ( arg3[0] == '\0' || !long_range_skill(sn) || !(victim = get_char_world( ch, arg3 ) ) )
		{
			send_to_char(AT_BLUE, "They aren't here.\n\r", ch );
			return;
		}
	    }
	}

	if( !is_pkillable(ch, victim ) ) {
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
    if ( IS_AFFECTED2( victim, AFF_SHADOW_PLANE) )
    {
      send_to_char(AT_WHITE, "You can not attack someone who is in the shadow realm.\n\r", ch);
      return;
    }
    if ( IS_AFFECTED4( ch, AFF_BURROW) )
    {
      send_to_char(AT_WHITE, "You must wait for the earth to heal you!\n\r", ch);
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
	if (!pk_combat_check(ch, victim))
	  return;
	vo = (void *) victim;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( !( victim = get_char_room( ch, arg2 ) ) )
	    {
		send_to_char(AT_BLUE, "They aren't here.\n\r", ch );
		return;
	    }

	}

	vo = (void *) victim;
	if (!pk_combat_check(ch, victim))
	  return;
	break;

    case TAR_CHAR_SELF:
	if ( arg2[0] != '\0' && !is_name( arg2, ch->name ) )
	{
	    send_to_char(AT_BLUE, "You cannot use this skill on another.\n\r", ch );
	    return;
	}

	vo = (void *) ch;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char(AT_BLUE, "What should the skill be used upon?\n\r", ch );
	    return;
	}

        if ( !(obj = get_obj_here( ch, arg2 ) ) )
        {
          send_to_char( AT_BLUE, "You can't find that.\n\r", ch );
          return;
        }
	vo = (void *) obj;
	break;
    }

    if ( !IS_NPC( ch ) )
    {
    if ( ch->move <move )
       {
        send_to_char(AT_BLUE, "You are too tired to do that.\n\r", ch );
        return;
       }
    }

    if ( ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) ) && ( skill_table[sn].target == TAR_CHAR_OFFENSIVE ) )
      {
       send_to_char( AT_BLUE, "You failed.\n\r", ch );
       return;
      }	   

    if( !IS_NPC ( ch ) )
    {
	WAIT_STATE( ch, skill_table[sn].beats );
    }
  
    
    if ( !IS_NPC( ch ) )
    {
    if ( number_percent( ) > ( ch->pcdata->learned[sn] / 10 )  )
    {
	send_to_char(AT_BLUE, "You lost your concentration.\n\r", ch );
	ch->move -= move / 2;
	if( ch->pcdata->learned[sn] <= 750 )
	   update_skpell( ch, sn, 0 );
    }
    else
    {
		int dmg = 0;
	  	ch->move -= move;
		if ( ( IS_AFFECTED2( ch, AFF_CONFUSED )  && number_percent( ) < 10 ) )
		{
			act(AT_YELLOW, "$n looks around confused at what's going on.", ch, NULL, NULL, TO_ROOM );
			send_to_char( AT_YELLOW, "You become confused and fumble your skill.\n\r", ch );
			return;
		} 
	 
		dmg = (*skill_table[sn].spell_fun) ( sn,
				      URANGE( 1, ch->level, LEVEL_DEMIGOD ),
				      ch, vo );

		if (dmg > SKPELL_NO_DAMAGE)
		{
			/*
				Insert level adjustment code here
			*/

			if (victim && !victim->deleted && victim->position!=POS_DEAD)
				damage(ch, victim, dmg, sn);
		} else if (dmg == SKPELL_BOTCHED || dmg == SKPELL_MISSED)
		{
			//send_to_char(C_DEFAULT, skill_table[sn].msg_fail, ch);
			/* do something */
		}
		if (dmg >= SKPELL_NO_DAMAGE)
			update_skpell( ch, sn, 0 );  

    }

    }

    if ( IS_NPC( ch ) )
	{
	  int dmg = 0;
      if ( (dmg = (*skill_table[sn].spell_fun) ( sn, URANGE( 1, ch->level, LEVEL_DEMIGOD ), ch, vo )) > SKPELL_NO_DAMAGE )
			damage( ch, victim, dmg, sn);
	}

   if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
        && victim->master != ch && victim != ch && IS_AWAKE( victim ) )

    {
        CHAR_DATA *vch;

        for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
        {
            if ( vch->deleted )
                continue;
            if ( victim == vch && !victim->fighting )
            {
                multi_hit( victim, ch, TYPE_UNDEFINED );
                break;
            }
        }
    }


    return;
}

/* Sort of a clone of spell_null
 * this will replace old skills in interp.c
 *
 * - Ahsile
 *
 */

void do_null( CHAR_DATA *ch, char *argument )
{
	send_to_char(C_DEFAULT, "You meant to USE that skill? Right?\n\r", ch);
}


void  do_null_dis( CHAR_DATA* ch, char* argument )
{
  do_null( ch, argument);
}

void do_skills ( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH ];
    int  sn;
    int  col;
    int  mana;

    if ( IS_NPC( ch ) )
    {
       send_to_char ( AT_BLUE, "You do not know how to do that!\n\r", ch );
       return;
    }

    col = 0;
    mana = 0;

    for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
    {
        if ( !skill_table[sn].name )
           break;
        if ( ch->level < skill_table[sn].skill_level[ch->class] && ch->level < skill_table[sn].skill_level[ch->multied] )
           continue;
        if ( skill_table[sn].is_spell )
           continue;

        if ( ch->level >= skill_table[sn].skill_level[ch->class ] )
        {
            mana = MANA_COST( ch, sn );
        }
        if ( ch->level >= skill_table[sn].skill_level[ch->multied ] )
        {
            mana = MANA_COST_MULTI( ch, sn );
        }

        sprintf ( buf, "%26s %3dpts ",
               skill_table[sn].name, mana );
        send_to_char( AT_BLUE, buf, ch );
        if ( ++col % 2 == 0 )
           send_to_char( AT_BLUE, "\n\r", ch );
    }

    if ( col % 2 != 0 )
      send_to_char( AT_BLUE, "\n\r", ch );

    return;
}


bool long_range_skill( int sn )
{
	if (sn == skill_lookup("lightning arrow"))
		return TRUE;
	
	return FALSE;
}
