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

/*$Id: update.c,v 1.32 2005/03/31 14:33:31 ahsile Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

/*
 * Externals
 */ 
extern  bool            merc_down;
extern  int             auc_count;
extern  void            auc_channel( char *auction );
extern  int             port;
extern  void            userl_save( void );
int             	pulse_db_dump;
extern  void 		do_timequake( CHAR_DATA *ch, char *argument );

/*
 * Globals
 */
bool    delete_obj;
bool    delete_char;

/*
 * Local functions.
 */
int	hit_gain        args( ( CHAR_DATA *ch ) );
int	mana_gain       args( ( CHAR_DATA *ch ) );
int	move_gain       args( ( CHAR_DATA *ch ) );
void	mobile_update   args( ( void ) );
void	weather_update  args( ( void ) );
void	char_update     args( ( void ) );
void	obj_update      args( ( void ) );
void	aggr_update     args( ( void ) );
void	comb_update     args( ( void ) );	/* XOR */
void    auc_update      args( ( void ) );       /* Altrag */
void    rdam_update     args( ( void ) );       /* Altrag */
void    strew_corpse    args( ( OBJ_DATA *obj, AREA_DATA *inarea ) );
void    orprog_update   args( ( void ) );
void    trap_update     args( ( void ) );
void    rtime_update    args( ( void ) );   /* Timed room progs */
void    quest_update    args( ( void ) );
void    rquest_update   args( ( void ) );
void 	craft_update	args( ( void ) );
void 	timed_room_update args(( void) );  /* temporary room flags */

#ifdef TIMEQUAKE
void    rand_update     args( ( void ) );
#endif
/*
 * Advancement stuff.
 */
void advance_level( CHAR_DATA *ch )
{
    char buf [ MAX_STRING_LENGTH ];
    int  add_hp;
    int  add_mana;
    int  add_move;
    int  add_prac;

    if ( !IS_SET( ch->act, PLR_KEEPTITLE ) )
      { 
       sprintf( buf, "the %s",
	       title_table [ch->class] [ch->level] [ch->sex == SEX_FEMALE ? 1 : 0] );
       set_title( ch, buf );
      }

    add_hp      = con_app[get_curr_con( ch )].hitp + number_range(
		    class_table[ch->class].hp_min,
		    class_table[ch->class].hp_max );
    add_mana    = class_table[ch->class].fMana
        ? number_range(2, ( 12 * get_curr_int( ch ) + get_curr_wis( ch ) ) / 6 )
	: 0;
    add_move    =
      number_range( 5, ( get_curr_con( ch ) + get_curr_dex( ch ) ) / 4 );
    add_prac    = wis_app[get_curr_wis( ch )].practice;
    if( ch->race == 0 )
    {
	add_prac += 1;
    }
    if( ch->class == ch->multied )
    {
	add_prac += 1;
    }

    add_hp               = UMAX(  1, add_hp   );
    add_mana             = UMAX(  0, add_mana );
    add_move             = UMAX( 10, add_move );
    if(ch->level == LEVEL_DEMIGOD)
    {
	add_hp = 4000;
	add_mana = 1000;
    }
    ch->max_hit 	+= add_hp;
    if (( ch->class != CLASS_VAMPIRE )&&(ch->class != CLASS_ANTI_PALADIN))
        ch->max_mana	+= add_mana;
    else
        {
         add_mana -= add_mana / 2;
         ch->max_bp	+= add_mana;
        }
    ch->max_move	+= add_move;
    ch->practice	+= add_prac;

    if ( !IS_NPC( ch ) )
	REMOVE_BIT( ch->act, PLR_BOUGHT_PET );

    if (( ch->class != CLASS_VAMPIRE )&&(ch->class != CLASS_ANTI_PALADIN))
       sprintf( buf,
	    "Your gain is: %d/%d hp, %d/%d m, %d/%d mv %d/%d prac.\n\r",
	    add_hp,	ch->max_hit,
	    add_mana,	ch->max_mana,
	    add_move,	ch->max_move,
	    add_prac,	ch->practice );
    else
       sprintf( buf,
	    "Your gain is: %d/%d hp, %d/%d bp, %d/%d mv %d/%d prac.\n\r",
	    add_hp,	ch->max_hit,
	    add_mana,	ch->max_bp,
	    add_move,	ch->max_move,
	    add_prac,	ch->practice );
    send_to_char(AT_WHITE, buf, ch );
    save_char_obj( ch, FALSE );

    if( ch->hit < ch->max_hit )
    {
	ch->hit = ch->max_hit;
    }
    if (( ch->class != CLASS_VAMPIRE) && ( ch->class != CLASS_ANTI_PALADIN ))
    {
	if( ch->mana < ch->max_mana )
	{
	    ch->mana = ch->max_mana;
	}
    }
    else
    {
	if( ch->bp < ch->max_bp )
	{
	    ch->bp = ch->max_bp;
	}
    }
    if( ch->move < ch->max_move )
    {
	ch->move = ch->max_move;
    }

    return;
}   



void gain_exp( CHAR_DATA *ch, int gain )
{
    if ( IS_NPC( ch ) || ch->exp >= MAX_EXPERIENCE )
	return;

    ch->exp = UMAX( 1000, ch->exp + gain );
    if ( ch->level < 100 && ch->exp >= 1000 * ( ch->level + 1 ) )
    {
	send_to_char(AT_BLUE, "You raise a level!!  ", ch );
	ch->level += 1;
	advance_level( ch );
	if(ch->level >= 50 )
	{
	    SET_BIT    ( ch->act, PLR_GHOST ); 
	}
        sprintf( log_buf, "%s has risen to level %d.", ch->name, ch->level );
	if( !IS_AFFECTED3( ch, AFF_STEALTH ) )
	{
	    broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(clap1.wav V=100 L=1 P=50 T=Info)");
	}
	sprintf( log_buf, "%s has risen to level %d.", ch->name, ch->level );
	log_string( log_buf, CHANNEL_LOG, -1 );

    }
    if ( ch->level == LEVEL_HERO && ch->exp >= 1500 + 1000 * (ch->level + 1) )
    {
        send_to_char(AT_BLUE, "You raise a champion level!!  ", ch );
        ch->level += 1;
        advance_level( ch );
        sprintf( log_buf, "%s has risen to level %d.", ch->name, ch->level );
	if( !IS_AFFECTED3( ch, AFF_STEALTH ) )
	{
	    broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(welcome2.wav V=100 L=1 P=75 T=Info)");
	}
    }
    if ( ch->level == LEVEL_HERO1 && ch->exp >= 5500 + 1000 * (ch->level + 1) )
    {
        send_to_char(AT_BLUE, "You raise a champion level!!  ", ch );
        ch->level += 1;
        advance_level( ch );
        sprintf( log_buf, "%s has risen to level %d.", ch->name, ch->level );
        if( !IS_AFFECTED3( ch, AFF_STEALTH ) )
        {
	    broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(welcome2.wav V=100 L=1 P=75 T=Info)");
        }
        sprintf( log_buf, "%s has risen to level %d.", ch->name, ch->level );
        log_string( log_buf, CHANNEL_LOG, -1 );
    }
    if ( ch->level == LEVEL_HERO2 && ch->exp >= 12000 + 1000 * (ch->level + 1) )
    {
        send_to_char(AT_BLUE, "You raise a champion level!!  ", ch );
        ch->level += 1;
        advance_level( ch );
        sprintf( log_buf, "%s has risen to level %d.", ch->name, ch->level );
        if( !IS_AFFECTED3( ch, AFF_STEALTH ) )
        {
	    broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(welcome2.wav V=100 L=1 P=75 T=Info)");
        }
        sprintf( log_buf, "%s has risen to level %d.", ch->name, ch->level );
        log_string( log_buf, CHANNEL_LOG, -1 );
    }
    if ( ch->level == LEVEL_HERO3 && ch->exp >= 31000 + 1000 * (ch->level + 1) )
    {
        send_to_char(AT_BLUE, "You raise a champion level!!  ", ch );
        ch->level += 1;
        advance_level( ch );
        sprintf( log_buf, "%s has risen to level %d.", ch->name, ch->level );
        if( !IS_AFFECTED3( ch, AFF_STEALTH ) )
        {
	    broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(welcome2.wav V=100 L=1 P=75 T=Info)");
        }
        sprintf( log_buf, "%s has risen to level %d.", ch->name, ch->level );
        log_string( log_buf, CHANNEL_LOG, -1 );
    }
    if ( ch->level == LEVEL_CHAMP && ch->exp >= 95000 + 1000 * (ch->level + 1 ) )
    {
	send_to_char(AT_BLUE, "You are now a Demi-God!!  ", ch );
	ch->level += 1;
	advance_level( ch );
        sprintf( log_buf, "%s has risen to level %d, and is now a Demi-God.", ch->name, ch->level );
        if( !IS_AFFECTED3( ch, AFF_STEALTH ) )
        {
	    broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(welcome2.wav V=100 L=1 P=100 T=Info)");
            log_string( log_buf, CHANNEL_INFO, -1 );
        }
        sprintf( log_buf, "%s has risen to level %d.", ch->name, ch->level );
        log_string( log_buf, CHANNEL_LOG, -1 );
    }
    if ( ch->level == LEVEL_DEMIGOD && ch->exp >= MAX_EXPERIENCE )
    {
	send_to_char(AT_BLUE, "\n\r\n\rYOU ARE NOW A LEGEND!!!\n\r\n\r", ch );
	sprintf( log_buf, "%s has attained the status of LEGEND.", ch->name );
	broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(welcome2.wav V=100 L=1 P=100 T=Info)");
	log_string( log_buf, CHANNEL_LOG, -1 );
 	ch->max_hit += 5000;
        if (( ch->class != 9 )&&(ch->class != 11))
        {
            ch->max_mana    += 2500;
	    send_to_char(AT_BLUE, "You have gained: 4000hp, 2500ma, 1000mv, and a special gift!\n\r", ch );
        }
        else 
        {
            ch->max_bp     += 2500;
	    send_to_char(AT_BLUE, "You have gained: 4000hp, 2500bp, 1000mv, and a special gift!\n\r", ch );
        }
        ch->max_move        += 1000;
	if( !IS_SET(ch->act, PLR_HOLYLIGHT) )
	{
	    SET_BIT(ch->act, PLR_HOLYLIGHT);
	}
    }
    return;

}



/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    CHAR_DATA *gch;
    int gain = 0;
    int racehpgainextra = 0;

    if ( IS_AFFECTED3( ch, AFF_CLOUD_OF_HEALING ) )
     {   
       for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
        {
         send_to_char(AT_YELLOW, "A cloud of healing engulfs the room!\n\r",gch);
         if ((gch->hit + gch->level*5) > gch->max_hit)
            gch->hit = gch->max_hit;
         else
           gch->hit = gch->hit + gch->level*5; 
        }
     }

    if ( IS_AFFECTED4( ch, AFF_BURROW ) )
    {
	gain += ch->level * 10;
        if (!str_cmp(ch->name, "Vemon"))
	{ 
		gain *= 2;
	}
    }

    if ( ( ch->race == 10 )
        || ( ch->race == 11 ) )
      {
        racehpgainextra = 2;
      }
    if ( ( ch->race == 9 )
        || ( ch->race == 13 ) )
      {
        racehpgainextra = 3;
      }
  
    if ( ch->race == 14 )
      {
        racehpgainextra = 10;
      }

    if ( ch->race == 17 )
      {
        racehpgainextra = 15;
      }

    if ( IS_NPC( ch ) )
    {
	gain = ch->level * 3 / 2;
    }
    else
    {
	gain += UMIN( 5, ch->level );

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += ch->level * 4; break;
	case POS_RESTING:  gain += get_curr_con( ch ) * 4 / 2; break;
	}

	if ( ch->pcdata->condition[COND_FULL  ] == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 10;

    if ( IS_AFFECTED3( ch, AFF_PESTILENCE ) )
        gain = 0;

    if ( IS_AFFECTED3( ch, AFF_TORTURE ) )
	gain = 0;

    if (!IS_NPC( ch ) && ( ch->pcdata->learned[skill_lookup("rapid healing")] / 10 )  > 0 )
    {
	gain *= ch->pcdata->learned[skill_lookup("rapid healing")] / 200;
	update_skpell( ch, skill_lookup("rapid healing"), 0 );
    }

    return UMIN( gain, ch->max_hit - ch->hit );
}



int mana_gain( CHAR_DATA *ch )
{
    int gain;

    if ( IS_NPC( ch ) )
    {
	gain = ch->level;
    }
    else
    {
	gain = UMIN( 20, ch->level / 2 );

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_int( ch ) * 12;	break;
	case POS_RESTING:  gain += get_curr_int( ch ) * 12 / 2;	break;
	}

	if ( ch->pcdata->condition[COND_FULL  ] == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;

    }

    if ( ( ch->race == 2 )
        || ( ch->race == 40 ) )
        gain += ch->level / 2;

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 10;

    if ( IS_AFFECTED3( ch, AFF_PESTILENCE ) )
	gain = 0;

    if ( IS_AFFECTED3( ch, AFF_TORTURE ) )
	gain = 0;

    return UMIN( gain, ch->max_mana - ch->mana );
}



int move_gain( CHAR_DATA *ch )
{
    int gain;

    if ( IS_NPC( ch ) )
    {
	gain = ch->level;
    }
    else
    {
	gain = UMAX( 15, 2 * ch->level );

	switch ( ch->position )
	{
	case POS_SLEEPING: gain += get_curr_dex( ch ) * 4; break;
	case POS_RESTING:  gain += get_curr_dex( ch ) * 2; break;
	}

	if ( ch->pcdata->condition[COND_FULL  ] == 0 )
	    gain /= 2;

	if ( ch->pcdata->condition[COND_THIRST] == 0 )
	    gain /= 2;
    }

    if ( ( ch->race == 21   )
        || ( ch->race == 25 )
        || ( ch->race == 34 ) )
        gain += ch->level / 2;

    if ( IS_AFFECTED( ch, AFF_POISON ) )
	gain /= 10;

    if ( IS_AFFECTED3( ch, AFF_PESTILENCE ) )
	gain = 0;

    if ( IS_AFFECTED3( ch, AFF_TORTURE ) )
	gain = 0;

    return UMIN( gain, ch->max_move - ch->move );
}



void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;

    if ( value == 0 || IS_NPC( ch ) || ch->level >= L_APP )
	return;

    if ( ch->level >= LEVEL_HERO && iCond != COND_DRUNK )
      return;

    if ( ch->position == POS_GHOST )
	return;


    condition				= ch->pcdata->condition[ iCond ];
    ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 48 );

    if ( ch->pcdata->condition[iCond] == 0 )
    {
	switch ( iCond )
	{
	case COND_FULL:
	    send_to_char(AT_ORANGE, "You are hungry.\n\r",    ch );
/*	    if( ( ch->hit ) > (ch->level * 11 ) )
            {
		damage(ch, ch, ch->level * 10, gsn_poison);
	    } */
	    break;

	case COND_THIRST:
	    send_to_char(AT_BLUE, "You are thirsty.\n\r",   ch );
	    break;

	case COND_DRUNK:
	    if ( condition != 0 )
		send_to_char(AT_BLUE, "You are sober.\n\r", ch );
	    break;
	}
    }

    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void )
{
    CHAR_DATA *ch;
    EXIT_DATA *pexit;
    int        door;

    /* Examine all mobs. */
    for ( ch = char_list; ch; ch = ch->next )
    {
        if ( ch->deleted )
	    continue;

	if ( IS_NPC(ch) && (ch->wait -= PULSE_MOBILE) < 0 )
	  ch->wait = 0;

	if ( !IS_NPC( ch )
	    || !ch->in_room
	    || IS_AFFECTED( ch, AFF_STUN )
	    || IS_AFFECTED( ch, AFF_CHARM )
	    || ch->wait > 0 )
	    continue;

	/* Examine call for special procedure */
	if ( ch->spec_fun != 0 )
	{
	    if ( ( *ch->spec_fun ) ( ch ) )
		continue;
	}

	/* That's all for sleeping / busy monster */
	if ( ch->position < POS_STANDING )
	    continue;
        
        if ( ch->in_room->area->nplayer > 0 ) 
          { 
           mprog_random_trigger( ch );
	   if ( ch->position < POS_STANDING )
	        continue;
	  }
	  
           
 
	/* Scavenge */
	if ( IS_SET( ch->act, ACT_SCAVENGER )
	    && ch->in_room->contents
	    && number_bits( 2 ) == 0 )
	{
	    OBJ_DATA *obj;
	    OBJ_DATA *obj_best;
	    int       max;

	    max         = 1;
	    obj_best    = 0;
	    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	    {
		if ( CAN_WEAR( obj, ITEM_TAKE )
		    && obj->cost > max
		    && can_see_obj( ch, obj ) )
		{
		    obj_best    = obj;
		    max         = obj->cost;
		}
	    }

	    if ( obj_best )
	    {
		obj_from_room( obj_best );
		obj_to_char( obj_best, ch );
		act(AT_GREY, "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
	    }
	}

	/* Wander */
	if ( !IS_SET( ch->act, ACT_SENTINEL )
	    && ( door = number_bits( 5 ) ) <= 5
	    && ( pexit = ch->in_room->exit[door] )
	    &&   pexit->to_room
	    &&   !IS_SET( pexit->exit_info, EX_CLOSED )
	    &&   !IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB )
	    &&   !ch->hunting
	    && ( !IS_SET( ch->act, ACT_STAY_AREA )
		||   pexit->to_room->area == ch->in_room->area ) )
	{
	    move_char( ch, door );
	    if ( ch->position < POS_STANDING )
	        continue;
	}

	/* Flee */
	if ( ch->hit < ( ch->max_hit / 2 )
	    && ( door = number_bits( 3 ) ) <= 5
	    && ( pexit = ch->in_room->exit[door] )
	    &&   pexit->to_room
	    &&   !IS_SET( pexit->exit_info, EX_CLOSED )
	    &&   !IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) )
	{
	    CHAR_DATA *rch;
	    bool       found;

	    found = FALSE;
	    for ( rch  = pexit->to_room->people;
		  rch;
		  rch  = rch->next_in_room )
	    {
	        if ( rch->deleted )
		    continue;
		if ( !IS_NPC( rch ) )
		{
		    found = TRUE;
		    break;
		}
	    }
	    if ( !found )
		move_char( ch, door );
	}
	hunt_victim( ch );
	 
    }

    MOBtrigger = TRUE;
    return;
}



/*
 * Update the weather.
 */
void weather_update( void )
{
    DESCRIPTOR_DATA *d;
    char             buf [ MAX_STRING_LENGTH ];
    char	     buf2 [ MAX_STRING_LENGTH ];
    int              diff;
    char	     buf3 [ MAX_STRING_LENGTH ];

    buf[0] = '\0';
    buf2[0] = '\0';
    buf3[0] = '\0';

    userl_save();

    switch ( ++time_info.hour )
    {
    case  1:
	time_info.total++;
	/* Adds 1 to the TOTAL NUMBER OF DAYS the mud has been up */ 
	break;
    case  5:
	weather_info.sunlight = SUN_LIGHT;
	strcat( buf, "&cThe day has begun." );
	break;

    case  6:
	weather_info.sunlight = SUN_RISE;
	strcat( buf, "The &Ysun&B rises in the east." );
	break;

    case 19:
	weather_info.sunlight = SUN_SET;
	strcat( buf, "The &Ysun&B slowly disappears in the west." );
	break;

    case 20:
	weather_info.sunlight = SUN_DARK;
	strcat( buf, "&zThe night has begun.&w" );
	sprintf(buf3, "!!SOUND(cricket2.wav V=100 L=1 P=50 T=Background)");
	break;

    case 24:
	time_info.hour = 0;
	time_info.day++;
	break;
    }

    if ( time_info.day   >= NUMBER_OF_DAYS_TO_RESET )
    {
	time_info.day = 0;
	time_info.month++;
    }

    if ( time_info.month >= 12 )
    {
	time_info.month = 0;
	time_info.year++;
    }

    if ( !(time_info.total % NUMBER_OF_DAYS_TO_RESET) && time_info.total && !time_info.hour)
    {
        do_resetxp(NULL, "all");
    }

    if ( time_info.total >= NUMBER_OF_DAYS_TO_REBOOT - 3 && time_info.hour == 23 )
    {
	sprintf( buf2, "A system reboot will occur very soon." );
	log_string( buf2, CHANNEL_INFO, -1 );
    }

    if ( time_info.total >= NUMBER_OF_DAYS_TO_REBOOT )
    {
	send_to_all_char("You feel compelled to rest, for a little while,\n\r as the sun sets upon the end of this year.\n\r");
	end_of_game();
	merc_down = TRUE;

	return;
    }


    /*
     * Weather change.
     */
    if ( time_info.month >= 9 && time_info.month <= 16 )
	diff = weather_info.mmhg >  985 ? -2 : 2;
    else
	diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice( 1, 4 ) + dice( 2, 6 ) - dice( 2, 6 );
    weather_info.change    = UMAX( weather_info.change, -12 );
    weather_info.change    = UMIN( weather_info.change,  12 );

    weather_info.mmhg     += weather_info.change;
    weather_info.mmhg      = UMAX( weather_info.mmhg,  960 );
    weather_info.mmhg      = UMIN( weather_info.mmhg, 1040 );

    switch ( weather_info.sky )
    {
    default: 
	bug( "Weather_update: bad sky %d.", weather_info.sky );
	weather_info.sky = SKY_CLOUDLESS;
	break;

    case SKY_CLOUDLESS:
	if (     weather_info.mmhg <  990
	    || ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The sky is getting &zcloudy." );
	    weather_info.sky = SKY_CLOUDY;
	    sprintf( buf3, "!!SOUND(wind3.wav V=100 L=1 P=50 T=Background)");
	}
	break;

    case SKY_CLOUDY:
	if (     weather_info.mmhg <  970
	    || ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "It starts to rain." );
	    weather_info.sky = SKY_RAINING;
	    sprintf(buf3, "!!SOUND(rain1.wav V=100 L=1 P=50 T=Background)");
	}

	if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "The &zclouds&B disappear." );
	    weather_info.sky = SKY_CLOUDLESS;
	    sprintf(buf3, "!!SOUND(birds4.wav V=100 L=1 P=50 T=Background)");
	}
	break;

    case SKY_RAINING:
	if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
	{
	    strcat( buf, "&WLightning&B flashes in the sky." );
	    weather_info.sky = SKY_LIGHTNING;
	    sprintf(buf3, "!!SOUND(thunder5.wav V=100 L=1 P=50 T=Background)");
	}

	if (     weather_info.mmhg > 1030
	    || ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The rain stopped." );
	    weather_info.sky = SKY_CLOUDY;
	    sprintf(buf3, "!!SOUND(birds4.wav V=100 L=1 P=50 T=Background)");
	}
	break;

    case SKY_LIGHTNING:
	if (     weather_info.mmhg > 1010
	    || ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
	{
	    strcat( buf, "The &Wlightning&B has stopped." );
	    weather_info.sky = SKY_RAINING;
	    sprintf(buf3, "!!SOUND(rain1.wav V=100 L=1 P=50 T=Background)");
	    break;
	}
	break;
    }

    if ( buf[0] != '\0' )
    {
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
		&& IS_OUTSIDE( d->character )
		&& IS_AWAKE  ( d->character )
		&& !IS_SET( d->character->in_room->room_flags, ROOM_INDOORS ) )
	    {
		send_to_char(AT_BLUE, buf, d->character );
		if( IS_SET( d->character->act, PLR_SOUND ) )
		{
		    send_to_char(AT_BLUE, buf3, d->character );
		}
		send_to_char(AT_BLUE, "\n\r", d->character );
	    }
	}
    }

    return;
}

/*
 * Update the bank system
 * (C) 1996 The Maniac from Mythran Mud
 *
 * This updates the shares value (I hope)
 */
void bank_update(void)
{
        int     value = 0;
        FILE    *fp;

        if ((time_info.hour < 9) || (time_info.hour > 17))
                return;         /* Bank is closed, no market... */

        value = number_range ( 0, 200);
        value -= 100;
        value /= 10;

        share_value += value;
        if( share_value < 5 || share_value > 195 )
	{
		share_value = 100;
	}
        if ( !( fp = fopen ( BANK_FILE, "w" ) ) )
        {
                bug( "bank_update:  fopen of BANK_FILE failed", 0 );
                return;
        }
        fprintf (fp, "SHARE_VALUE %d\n\r", share_value);
        fclose(fp);
}

/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void char_update( void )
{   
    CHAR_DATA *ch;
    CHAR_DATA *ch_save;
    CHAR_DATA *ch_quit;
    CHAR_DATA *ch_next;	/* XOR */
    time_t     save_time;

    ch_save	= NULL;
    ch_quit	= NULL;
    save_time	= current_time;

    for ( ch = char_list; ch; ch = ch->next )
    {
	AFFECT_DATA *paf;

	if ( ch->deleted )
	    continue;

	/*
	 * Find dude with oldest save time.
	 */
	if ( !IS_NPC( ch )
	    && ( !ch->desc || ch->desc->connected == CON_PLAYING )
	    &&   ch->level >= 2
	    &&   ch->save_time < save_time )
	{
	    ch_save	= ch;
	    save_time	= ch->save_time;
	}

	if ( ch->position >= POS_STUNNED )
	{
	    if ( ch->hit  < ch->max_hit  )
            {
		ch->hit  += hit_gain( ch );
	    }
	 
    	    if (IS_AFFECTED4(ch, AFF_BURROW) && ch->hit >= ch->max_hit)
            {
	   	affect_strip(ch, skill_lookup("tomba di vemon"));
		send_to_char(AT_ORANGE, "The earth has healed you!\n\r",ch);
	  	act(AT_ORANGE, "$n rises from the earth!\n\r",ch,NULL,NULL,TO_ROOM);
	    }

	    if ( ch->mana < ch->max_mana )
		ch->mana += mana_gain( ch );

	    if ( ch->move < ch->max_move )
		ch->move += move_gain( ch );
	    if ( ch->position == POS_STUNNED )
	      { 
	       ch->position = POS_STANDING; 
	       update_pos( ch );
	      }
	}
        if (is_affected(ch, skill_lookup("berserk"))
          && !ch->fighting )
        {
          affect_strip( ch, skill_lookup("berserk") );
          send_to_char(AT_WHITE, "The rage leaves you.\n\r", ch );
        }

        if (IS_AFFECTED2(ch, AFF_BERSERK)
          && !ch->fighting )
        {
          REMOVE_BIT( ch->affected_by2, AFF_BERSERK );
        }
        
        if (is_affected(ch, skill_lookup("bloodthirsty"))
          && !ch->fighting )
        {
          affect_strip( ch, skill_lookup("bloodthirsty") );
          send_to_char(AT_WHITE, "Your lust for blood fades.\n\r", ch );
        }

        if (IS_AFFECTED3(ch, AFF_BLOODTHIRSTY)
          && !ch->fighting )
        {
          REMOVE_BIT( ch->affected_by3, AFF_BLOODTHIRSTY );
        }

	if (is_affected(ch, skill_lookup("force of nature"))
	  && !ch->fighting )
	{
	  affect_strip(ch, skill_lookup("force of nature") );
	  ch->shields -= 1;
	  send_to_char(AT_GREEN, "The force of nature leaves your soul.\n\r", ch );
	}

	if (IS_AFFECTED4(ch, AFF_FORCE_OF_NATURE)
	  && !ch->fighting )
	{
	  REMOVE_BIT( ch->affected_by4, AFF_FORCE_OF_NATURE );
	}       
        
	if ( ch->position == POS_STUNNED )
	    update_pos( ch );

        if ( !IS_NPC( ch ) && (( time(NULL) - ch->pkill_timer) > 600 ) ){
           ch->pkill_timer = 0;
	}

	if ( ch->mountcharmed > 0 )
	{
	    ch->mountcharmed--;
	}
	
	if ( !IS_NPC( ch ) && ( ch->level < LEVEL_IMMORTAL
			       || ( !ch->desc && !IS_SWITCHED( ch ) ) ) )
	{
	    OBJ_DATA *obj;

	    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) )
		&& obj->item_type == ITEM_LIGHT
		&& obj->value[2] > 0 )
	    {
		if ( --obj->value[2] == 0 && ch->in_room )
		{
		    --ch->in_room->light;
		    act(C_DEFAULT, "$p goes out.", ch, obj, NULL, TO_ROOM );
		    act(C_DEFAULT, "$p goes out.", ch, obj, NULL, TO_CHAR );
		    extract_obj( obj );
		}
	    }

	    if ( ++ch->timer >= 10 )
	    {
		if ( !ch->was_in_room && ch->in_room )
		{
		    ch->was_in_room = ch->in_room;
		    if ( ch->fighting )
			stop_fighting( ch, TRUE );
		    send_to_char(AT_GREEN, "You disappear into the void.\n\r", ch );
		    act(AT_GREEN, "$n disappears into the void.",
			ch, NULL, NULL, TO_ROOM );
		    save_char_obj( ch, FALSE );
		    char_from_room( ch );
		    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
		}
	    }

	    if ( ch->timer > 20 && !IS_SWITCHED( ch ) )
		ch_quit = ch;

	    gain_condition( ch, COND_DRUNK,  -8 );
	    gain_condition( ch, COND_FULL,   -1 );
	    gain_condition( ch, COND_THIRST, -1 );
	}

	for ( paf = ch->affected; paf; paf = paf->next )
	{
	    if ( paf->deleted )
	        continue;
	    if ( paf->duration > 0 )
		paf->duration--;
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( !paf->next
		    || paf->next->type != paf->type
		    || paf->next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, ch );
			send_to_char(C_DEFAULT, "\n\r", ch );
		    }
		}
	  
		affect_remove( ch, paf );
            if ( paf->type > 0 && skill_table[paf->type].shield_bit == SHIELD_YES )
            {
                ch->shields -= 1;
            }
		if ( paf->type == AFF_FLYING )
		  check_nofloor( ch );
	    }
	}
	for ( paf = ch->affected2; paf; paf = paf->next )
	{
	    if ( paf->deleted )
	        continue;
	    if ( paf->duration > 0 )
		paf->duration--;
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( !paf->next
		    || paf->next->type != paf->type
		    || paf->next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, ch );
			send_to_char(C_DEFAULT, "\n\r", ch );
		    }
		}
	  
		affect_remove2( ch, paf );
            if ( paf->type > 0 && skill_table[paf->type].shield_bit == SHIELD_YES )
            {
                ch->shields -= 1;
            }

             }
	  }

	for ( paf = ch->affected3; paf; paf = paf->next )
	{
	    if ( paf->deleted )
	        continue;
	    if ( paf->duration > 0 )
		paf->duration--;
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( !paf->next
		    || paf->next->type != paf->type
		    || paf->next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, ch );
			send_to_char(C_DEFAULT, "\n\r", ch );
		    }
		}
	  
		affect_remove3( ch, paf );
            if ( paf->type > 0 && skill_table[paf->type].shield_bit == SHIELD_YES )
            {
                ch->shields -= 1;
            }

             }
	  }

	for ( paf = ch->affected4; paf; paf = paf->next )
	{
	    if ( paf->deleted )
	        continue;
	    if ( paf->duration > 0 )
		paf->duration--;
	    else if ( paf->duration < 0 )
		;
	    else
	    {
		if ( !paf->next
		    || paf->next->type != paf->type
		    || paf->next->duration > 0 )
		{
		    if ( paf->type > 0 && skill_table[paf->type].msg_off )
		    {
			send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, ch );
			send_to_char(C_DEFAULT, "\n\r", ch );
		    }
		}
	  
		affect_remove4( ch, paf );
            if ( paf->type > 0 && skill_table[paf->type].shield_bit == SHIELD_YES )
            {
                ch->shields -= 1;
            }

             }
	  }
        for ( paf = ch->affected_powers; paf; paf = paf->next )
        {
            if ( paf->deleted )
                continue;
            if ( paf->duration > 0 )
                paf->duration--;
            else if ( paf->duration < 0 )
                ;
            else
            {
                if ( !paf->next
                    || paf->next->type != paf->type
                    || paf->next->duration > 0 )
                {
                    if ( paf->type > 0 && skill_table[paf->type].msg_off )
                    {
                        send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, ch );
                        send_to_char(C_DEFAULT, "\n\r", ch );
                    }
                }
                
                affect_remove_powers( ch, paf );
            if ( paf->type > 0 && skill_table[paf->type].shield_bit == SHIELD_YES )
            {
                ch->shields -= 1;
            }

             }
          }

        for ( paf = ch->affected_weaknesses; paf; paf = paf->next )
        {
            if ( paf->deleted )
                continue;
            if ( paf->duration > 0 )
                paf->duration--;
            else if ( paf->duration < 0 )
                ;
            else
            {
                if ( !paf->next
                    || paf->next->type != paf->type
                    || paf->next->duration > 0 )
                {
                    if ( paf->type > 0 && skill_table[paf->type].msg_off )
                    {
                        send_to_char(C_DEFAULT, skill_table[paf->type].msg_off, ch );
                        send_to_char(C_DEFAULT, "\n\r", ch );
                    }
                }
                
                affect_remove_weaknesses( ch, paf );
            if ( paf->type > 0 && skill_table[paf->type].shield_bit == SHIELD_YES )
            {
                ch->shields -= 1;
            }

             }
          }

	if ( ch->gspell && --ch->gspell->timer <= 0 )
	{
	  send_to_char(AT_BLUE, "You slowly lose your concentration.\n\r",ch);
	  end_gspell( ch );
	}

	if ( ch->ctimer > 0 )
	  ch->ctimer--;

	if ( ch->rtimer > 0 )
	  ch->rtimer--;
	/*
	 * Careful with the damages here,
	 *   MUST NOT refer to ch after damage taken,
	 *   as it may be lethal damage (on NPC).
	 */

	if ( IS_AFFECTED3( ch, AFF_POWER_LEAK ) )
	{
	    int temp = number_range(ch->level * 3, ch->level * 10 );
            send_to_char(AT_CYAN, "You lose some energy.\n\r", ch );
            act(AT_CYAN, "$n face looks drained.", ch, NULL, NULL,TO_ROOM);
	    if ((ch->mana - temp) < 0) 
	       ch->mana = 0;
	    else
               ch->mana = ch->mana - temp;
	}

	if ( IS_AFFECTED( ch, AFF_POISON ) )
	{
	    send_to_char(AT_GREEN, "You shiver and suffer.\n\r", ch );
	    act(AT_GREEN, "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
/*
	    damage( ch, ch, 10, gsn_poison );
*/
	    damage( ch, ch, ( ( ch->level/10 ) +1 ) * ( ch->poison_level ), skill_lookup("poison") );
	    if ( ch->poison_level > 0 )
	    {
		ch->poison_level--;
	    }
	    if ( ch->poison_level == 0 )
	    {
		affect_strip( ch, AFF_POISON );
	    }


	}
	else if
	(
	  (
	    !IS_NPC(ch) && ch->in_room->sector_type == SECT_UNDERWATER
	    && (!IS_IMMORTAL( ch ) && !IS_AFFECTED3( ch, AFF_GILLS )
	    && !IS_SET( race_table[ch->race].race_abilities,
	    RACE_WATERBREATH ) && ch->position != POS_GHOST )
	    )
	    ||
	    (
	      ( !IS_NPC( ch )
		&& ch->in_room->sector_type != SECT_UNDERWATER
                       && ch->in_room->sector_type != SECT_WATER_NOSWIM
                       && ch->in_room->sector_type != SECT_WATER_SWIM )
                    && IS_SET( race_table[ ch->race ].race_abilities,
                              RACE_WATERBREATH )
                    && ( strcmp( race_table[ ch->race ].race_name, "Object" )
                        && strcmp( race_table[ ch->race ].race_name, "God" ) )
                )
            )
        {
            send_to_char( AT_BLUE, "You can't breathe!\n\r", ch );
            act( AT_BLUE, "$n sputters and chokes!", ch, NULL, NULL, TO_ROOM );
            damage( ch, ch, number_fuzzy(ch->level) + number_percent( ), /* TYPE_UNDEFINED */ skill_lookup("breathe water") );
        }

	if ( IS_AFFECTED3( ch, AFF_PESTILENCE ) )
	{
	    send_to_char(AT_GREEN, "You suffer the wrath of Pestilence.\n\r", ch );
	    act(AT_GREEN, "$n's body breaks out in spores as they suffer the Pestilence.", ch, NULL, NULL, TO_ROOM );
	    damage( ch, ch, 400, skill_lookup("pestilence") );
	}
	if ( IS_AFFECTED3( ch, AFF_NAGAROMS_CURSE ) )
	{
	    send_to_char(AT_GREEN, "You feel Nagarom watching you.\n\r", ch );
	}
	if ( IS_AFFECTED3( ch, AFF_TORTURE )
	&& (IS_SET(ch->in_room->room_flags, ROOM_SAFE) ) )
	{
	    send_to_char(AT_BLOOD, "Your soul feels great pain!\n\r", ch );
	    act(AT_BLOOD, "$n's soul is tortured!", ch, NULL, NULL, TO_ROOM );
	    damage( ch, ch, 500, skill_lookup("tortured soul") );
	}
	if ( IS_AFFECTED3( ch, AFF_DEADLY_POISON ) )
	{
	    send_to_char(AT_GREEN, "Your body is ravaged by a deadly poison!\n\r", ch );
	    act(AT_GREEN, "$n's body is ravaged by a deadly poison!", ch, NULL, NULL, TO_ROOM );
	    damage( ch, ch, ch->hit/4, skill_lookup("deadly poison") );
	}
	if ( IS_AFFECTED( ch, AFF_FLAMING ) )
	{
	    send_to_char(AT_RED, "Your skin blisters and burns.\n\r", ch );
	    act(AT_RED, "$n's body blisters and burns as it is licked in flames.", ch, NULL, NULL, TO_ROOM );
	    damage( ch, ch, ch->level/4, skill_lookup("incinerate") );
	}
	else if ( ch->position == POS_INCAP )
	{
	    damage( ch, ch, 1, TYPE_UNDEFINED );
	}
	else if ( ch->position == POS_MORTAL )
	{
	    damage( ch, ch, 2, TYPE_UNDEFINED );
	}
    }

    /*
     * Autosave and autoquit.
     * Check that these chars still exist.
     */
    if ( ch_save || ch_quit )
    {
	for ( ch = char_list; ch; ch = ch->next )
	{
	    if ( ch->deleted )
	        continue;
	    if ( ch == ch_save )
		save_char_obj( ch, FALSE );
	    if ( ch == ch_quit )
		do_quit( ch, "" );
	}
    }
/* XOR */
    for(ch = char_list;ch != NULL;ch = ch_next)
    {
      ch_next = ch->next;
      if(ch->summon_timer <= 0)
        ;
      else
        ch->summon_timer--;
      if(IS_NPC(ch) && ch->summon_timer == 0)
      {
        extract_char(ch, TRUE);
      }
    }
/* END */
    return;
}



/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void )
{   
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    for ( obj = object_list; obj; obj = obj_next )
    {
	CHAR_DATA *rch;
	char      *message;

	obj_next = obj->next;
	if ( obj->deleted )
	    continue;

	if ( obj->timer < -1 )
	    obj->timer = -1;

	if ( obj->timer < 0 )
	    continue;

	/*
	 *  Bug fix:  used to shift to obj_free if an object whose
	 *  timer ran out contained obj->next.  Bug was reported
	 *  only when the object(s) inside also had timer run out
	 *  during the same tick.     --Thelonius (Monk)
	 */
	if ( --obj->timer == 0 )
	{
	  AREA_DATA *inarea = NULL;
	  bool pccorpse = FALSE;
	  
	    switch ( obj->item_type )
	    {
	    	default:              message = "$p vanishes.";         break;
    		case ITEM_FOUNTAIN:   message = "$p dries up.";         break;
    		case ITEM_CORPSE_NPC: message = "$p decays into dust."; break;
    		case ITEM_CORPSE_PC:  message = "$p decays into dust."; 
    		                      pccorpse = TRUE; break;
    		case ITEM_FOOD:       message = "$p decomposes.";       break;
    		case ITEM_BLOOD:      message = "$p soaks into the ground."; break;
    		case ITEM_PORTAL:     message = "$p shimmers and is gone."; break;
	        case ITEM_VODOO:      message = "$p slowly fades out of existance."; break;
	        case ITEM_BERRY:      message = "$p rots away."; break;
	    }
    
	    if ( obj->carried_by )
	    {
	        act(C_DEFAULT, message, obj->carried_by, obj, NULL, TO_CHAR );
	    }
	    else
	      if ( obj->in_room
		  && ( rch = obj->in_room->people ) )
	      {
		  act(C_DEFAULT, message, rch, obj, NULL, TO_ROOM );
		  act(C_DEFAULT, message, rch, obj, NULL, TO_CHAR );
	      }
            
            if ( obj->in_room )
              inarea = obj->in_room->area;
            
	    if ( obj == object_list )
	    {
	      if ( !pccorpse || !inarea )
	      {
	        extract_obj( obj );
	        obj_next = object_list;
	      }
	     else
	       strew_corpse( obj, inarea );
	    }
	    else				/* (obj != object_list) */
	    {
	        OBJ_DATA *previous;
   
	        for ( previous = object_list; previous;
		     previous = previous->next )
	        {
		    if ( previous->next == obj )
	     		break;
	        }
   
		if ( !previous )  /* Can't see how, but... */
		    bug( "Obj_update: obj %d no longer in object_list",
    			obj->pIndexData->vnum );
    
              if ( !pccorpse || !inarea )
              {
	        extract_obj( obj );
	        obj_next = previous->next;
	      }
	     else
	       strew_corpse( obj, inarea );
	    }
	}
    }

    return;
}



/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes .2% of total CPU time.
 *
 * -Kahn
 */
void aggr_update( void )
{
    CHAR_DATA       *ch;
    CHAR_DATA       *mch;
    CHAR_DATA       *vch;
    CHAR_DATA       *victim;
    DESCRIPTOR_DATA *d;
    /*
     * Let's not worry about link dead characters. -Kahn
     */
    for ( d = descriptor_list; d; d = d->next )
    {
	ch = d->character;

	if ( d->connected != CON_PLAYING
	    || !ch
	    || !ch->in_room )
	    continue;

	/* mch wont get hurt */
	for ( mch = ch->in_room->people; mch; mch = mch->next_in_room )
	{
	    int count;
        bool hate = FALSE;

		/*
		if ( IS_NPC( mch ) && mch->mpactnum > 0 )
	    {
			MPROG_ACT_LIST * tmp_act, *tmp2_act;
			for ( tmp_act = mch->mpact; tmp_act != NULL; tmp_act = tmp_act->next )
			{
				mprog_wordlist_check( tmp_act->buf,mch, tmp_act->ch, tmp_act->obj, tmp_act->vo, ACT_PROG );
			}
			for ( tmp_act = mch->mpact; tmp_act != NULL; tmp_act = tmp2_act )
			{
				mch->mpactnum--;
				tmp2_act = tmp_act->next;
				free_mprog_act( tmp_act );
			}
			if(mch->mpactnum > 0)
				bug("Not all mprogs released for %d!", mch->pIndexData->vnum);
			mch->mpactnum = 0;
			mch->mpact    = NULL;
	    }
		*/

	    if ( !IS_NPC( mch )
		|| mch->deleted
		|| IS_AFFECTED( mch, AFF_STUN )
		|| mch->fighting
		|| IS_AFFECTED( mch, AFF_CHARM )
		|| !IS_AWAKE( mch )
		|| !can_see( mch, ch )
                || ( !IS_SET( mch->act, ACT_AGGRESSIVE )
                    && ( str_infix( race_table[ch->race].race_full,
                                   race_table[mch->race].hate )
                        || ( !str_infix( race_table[ch->race].race_full,
                                        race_table[mch->race].hate )
                            && abs( mch->level - ch->level ) > 4 ) ) )
		|| mch->wait > 0
		|| ch->wait > 0 ) 
		continue;

        if ( !str_infix( race_table[ch->race].race_full,
                            race_table[mch->race].hate ) )
                hate = TRUE;
	    
	    if ( !IS_SET( mch->act, ACT_AGGRESSIVE )
	      || (IS_SET( mch->act, ACT_WIMPY ) && IS_AWAKE( ch ) )
	      || IS_AFFECTED( ch, AFF_PEACE ) || ch->position == POS_GHOST
	      || IS_IMMORTAL( ch ) )
	      continue;

	    /*
	     * Ok we have a 'ch' player character and a 'mch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     */
            count = 0;
	    victim = NULL;
	    for ( vch = mch->in_room->people; vch; vch = vch->next_in_room )
	    {
	        if ( IS_NPC( vch )
		    || vch->deleted
		    || vch->level >= LEVEL_IMMORTAL )
		    continue;

		if ( ( !IS_SET( mch->act, ACT_WIMPY ) || !IS_AWAKE( vch ) )
		    && can_see( mch, vch ) )
		{
                    if (   !hate
                        || ( hate
                            && !str_infix( race_table[vch->race].race_full,
                                          race_table[mch->race].hate ) ) )
                    {
                        if ( number_range( 0, count ) == 0 )
                            victim = vch;
                        count++;
                    }
                }
	    }

	    if ( !victim )
	        continue;

		if (!is_safe(mch, victim))
		    multi_hit( mch, victim, TYPE_UNDEFINED );


	} /* mch loop */

    } /* descriptor loop */

    return;
}

/* Update the check on time for autoshutdown */
void time_update( void )
{
    FILE            *fp;
    char            *curr_time;
    char             buf [ MAX_STRING_LENGTH ];
    
    if ( down_time == "*" )
        return;
    curr_time = ctime( &current_time );
    if ( !str_infix( warning1, curr_time ) )
    {
	sprintf( buf, "First Warning!\n\r%s at %s system time\n\r"
		      "Current time is %s\n\r",
		(stype == 0 ? "Reboot" : "Shutdown"),
		down_time,
		curr_time );
	send_to_all_char( buf );
	free_string( warning1 );
	warning1 = str_dup( "*" );
    }
    if ( !str_infix( warning2, curr_time ) )
    {
	sprintf( buf, "Second Warning!\n\r%s at %s system time\n\r"
		      "Current time is %s\n\r",
		(stype == 0 ? "Reboot" : "Shutdown"),
		down_time,
		curr_time );
	send_to_all_char( buf );
	free_string( warning2 );
	warning2 = str_dup( "*" );
    }
    if ( !str_infix( down_time, curr_time ) )
    {
	/* OLC 1.1b */
	do_asave( NULL, "" );

        if ( stype == 1 )
        {
	send_to_all_char( "Shutdown by system.\n\r" );
	log_string( "Shutdown by system.", CHANNEL_GOD, -1 );
	
	end_of_game( );
	}
       else
        {
          send_to_all_char( "Reboot by system.\n\r" );
          log_string( "Reboot by system.", CHANNEL_GOD, -1 );
          end_of_game( );
          merc_down = TRUE;
          return;
        }

	fclose( fpReserve );
	if ( !( fp = fopen( SHUTDOWN_FILE, "a" ) ) )
	{
	    perror( SHUTDOWN_FILE );
	    bug( "Could not open the Shutdown file!", 0 );
	}
	else
	{
	    fprintf( fp, "Shutdown by System\n" );
	    fclose ( fp );
	}
	fpReserve = fopen ( NULL_FILE, "r" );
	merc_down = TRUE;
    }
    
    return;

}

/*
 * Remove deleted EXTRA_DESCR_DATA from objects.
 * Remove deleted AFFECT_DATA from chars and objects.
 * Remove deleted CHAR_DATA and OBJ_DATA from char_list and object_list.
 */
void list_update( void )
{
            CHAR_DATA *ch;
            CHAR_DATA *ch_next;
            OBJ_DATA  *obj;
            OBJ_DATA  *obj_next;
    extern  bool       delete_obj;
    extern  bool       delete_char;

    if ( delete_char )
        for ( ch = char_list; ch; ch = ch_next )
	  {
	    AFFECT_DATA *paf;
	    AFFECT_DATA *paf_next;
	    
	    for ( paf = ch->affected; paf; paf = paf_next )
	    {
	      paf_next = paf->next;
		
	      if ( paf->deleted || ch->deleted )
	      {
		if ( ch->affected == paf )
		{
		  ch->affected = paf->next;
		}
		else
		{
		  AFFECT_DATA *prev;
		  
		  for ( prev = ch->affected; prev; prev = prev->next )
		  {
		    if ( prev->next == paf )
		    {
		      prev->next = paf->next;
		      break;
		    }
		  }
			
		  if ( !prev )
		  {
		    bug( "List_update: cannot find paf on ch.", 0 );
		    sprintf( log_buf, "Char: %s", ch->name );
		    bug( log_buf, 0 );
		    continue;
		  }
		}
		    
		free_affect( paf);
	      }
	    }

	    
	    for ( paf = ch->affected2; paf; paf = paf_next )
	      {
		paf_next = paf->next;
		
		if ( paf->deleted || ch->deleted )
		  {
		    if ( ch->affected2 == paf )
		      {
			ch->affected2 = paf->next;
		      }
		    else
		      {
			AFFECT_DATA *prev;
			
			for ( prev = ch->affected2; prev; prev = prev->next )
			  {
			    if ( prev->next == paf )
			      {
				prev->next = paf->next;
				break;
			      }
			  }
			
			if ( !prev )
			  {
			    bug( "List_update2: cannot find paf on ch.", 0 );
			    sprintf( log_buf, "Char: %s", ch->name ? ch->name
			     : "(Unknown)" );
			    bug( log_buf, 0 );
			    continue;
			  }
		      }
		    
		free_affect( paf);
		  }
	      }

	    ch_next = ch->next;
	    
	    if ( ch->deleted )
	    {
		  if ( ch == char_list )
		  {
		    char_list = ch->next;
		  
		  } else
		  {
		    CHAR_DATA *prev;

		    for ( prev = char_list; prev; prev = prev->next )
		    {
				if ( prev->next == ch )
				{
					prev->next = ch->next;
					break;
				}
		    }
		    
		    if ( !prev )
		    {
				char buf [ MAX_STRING_LENGTH ];
			
				sprintf( buf, "List_update: char %s not found.", ch->name );
				bug( buf, 0 );
				continue;
		     }
		  }

		  if (ch->in_room)
		  {
			  char buf [MAX_STRING_LENGTH];
			  sprintf(buf, "List_update: char %s being deleted, but still in room \"%s\" (%d)", ch->name, ch->in_room->name, ch->in_room->vnum);
			  bug(buf,0);
			  extract_char(ch, TRUE);
		  }
		
		  free_ch( ch );
	    }
	  }

    if ( delete_obj )
      for ( obj = object_list; obj; obj = obj_next )
	{
	  AFFECT_DATA      *paf;
	  AFFECT_DATA      *paf_next;
	  EXTRA_DESCR_DATA *ed;
	  EXTRA_DESCR_DATA *ed_next;

	  for ( ed = obj->extra_descr; ed; ed = ed_next )
	    {
	      ed_next = ed->next;
	      
	      if ( obj->deleted )
		{
		  free_extra_descr( ed  );
		}
	    }

	  for ( paf = obj->affected; paf; paf = paf_next )
	    {
	      paf_next = paf->next;
	      
	      if ( obj->deleted )
		{
		  if ( obj->affected == paf )
		    {
		      obj->affected = paf->next;
		    }
		  else
		    {
		      AFFECT_DATA *prev;
		      
		      for ( prev = obj->affected; prev; prev = prev->next )
			{
			  if ( prev->next == paf )
			    {
			      prev->next = paf->next;
			      break;
			    }
			}

		      if ( !prev )
			{
			  bug( "List_update: cannot find paf on obj.", 0 );
			  continue;
			}
		    }
		  
		  free_affect( paf);
		}
	    }

	  obj_next = obj->next;

	  if ( obj->deleted )
	    {
	      if ( obj == object_list )
		{
		  object_list = obj->next;
		}
	      else
		{
		  OBJ_DATA *prev;
		  
		  for ( prev = object_list; prev; prev = prev->next )
		    {
		      if ( prev->next == obj )
			{
			  prev->next = obj->next;
			  break;
			}
		    }
		  
		  if ( !prev )
		    {
		      bug( "List_update: obj %d not found.",
			  obj->pIndexData->vnum );
		      continue;
		    }
		}


	      --obj->pIndexData->count;

	      free_obj( obj );
	    }
	}

    delete_obj  = FALSE;
    delete_char = FALSE;
    return;
}

#ifdef TIMEQUAKE

void rand_update( void )
{
      bug("Random Timequake Triggered", 0);
      if (number_range(0,1000) > 990)
      {
	do_timequake(NULL, NULL);
      }
}

#endif

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 */
void update_handler( void )
{
    static int pulse_quest; 	/* Non-Variable pulse for Timers - Ahsile */
    static int pulse_area;
    static int pulse_mobile;
    static int pulse_violence;
    static int pulse_point;
    static int pulse_combat;	/* XOR pkill */
#ifdef SQL_SYSTEM
	static int pulse_dirty;		/* Save dirty chars */

	if ( --pulse_dirty <= 0 )
	{
		sql_save_dirty_list(TRUE);
		pulse_dirty = PULSE_DIRTY;
	}
#endif

#ifndef DISABLE_EDITS_SAVES
    /* OLC 1.1b */
    if ( --pulse_db_dump  <= 0 )
    {
	pulse_db_dump	= PULSE_DB_DUMP; 
	do_asave( NULL, "" );
    } 
#endif

    if ( --pulse_quest	  <= 0 )
    {
	pulse_quest	= PULSE_QUEST;
        quest_update    ( );
	rquest_update   ( );
    }

    if ( --pulse_area     <= 0 )
    {
	pulse_area	= number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
	area_update	( );
	#ifdef TIMEQUAKE
   	rand_update     ( );
	#endif
    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence  = PULSE_VIOLENCE;
	violence_update ( );
	craft_update( );
	timed_room_update( );
    }

    if ( --pulse_mobile   <= 0 )
    {
	pulse_mobile    = PULSE_MOBILE;
	mobile_update   ( );
	rdam_update ( );
	orprog_update( );
    }

#if defined (AUTO_WORLD_SAVE)
    /* OLC 1.1b */
    if ( --pulse_db_dump  <= 0 )
    {
        wiznet(NULL, WIZ_TICKS, L_DIR, "Dump Area pulse (OLC)" );
        pulse_db_dump   = PULSE_DB_DUMP;
        do_asave( NULL, "" );
    }
#endif

    if ( --pulse_point    <= 0 )
    {
	pulse_point     = number_range( PULSE_TICK / 2, 3 * PULSE_TICK / 2 );
	weather_update  ( );
	rtime_update    ( );
	char_update     ( );
	obj_update      ( );
	list_update     ( );
        bank_update     ( );
    }

/* XOR causing alot of lag */
/* yeah yeah, see if its fixed now =) */
    if(--pulse_combat <= 0)
    {
      pulse_combat = PULSE_PER_SECOND;
      comb_update();
    }
/* END */

/* Auction timer update -- Altrag */
    if ( auc_count >= 0 && ++auc_count % (8 * PULSE_PER_SECOND) == 0 )
      auc_update ( );


    time_update( );
    aggr_update( );
    tail_chain( );
    return;
}

/* X combat timer update */
void comb_update()
{
  CHAR_DATA *ch;
  for ( ch = char_list; ch != NULL; ch = ch->next )
  {
    if ( ch->deleted )
      continue;
    if(--ch->combat_timer < 0)
      ch->combat_timer = 0;
  }
}

/* Auctioneer timer update -- Altrag */
void auc_update()
{
  extern OBJ_DATA *auc_obj;
  extern CHAR_DATA *auc_held;
  extern CHAR_DATA *auc_bid;
  extern int auc_cost;
  char buf[MAX_STRING_LENGTH];

  if ( !auc_obj )
    return;

  if ( !auc_held )
  {
    bug( "Auc_update: auc_obj found without auc_held.",0);
    return;
  }

  switch ( auc_count / (8 * PULSE_PER_SECOND) )
  {
  case 1:
    sprintf( buf, "%s for %d gold coins (going ONCE).", auc_obj->short_descr, auc_cost );
    auc_channel( buf );
    sprintf( buf, "%s auctioning %s.", auc_held->name, auc_obj->name );
    log_string( buf, CHANNEL_GOD, -1 );
    return;
  case 2:
    sprintf( buf, "%s for %d gold coins (going TWICE).", auc_obj->short_descr, auc_cost );
    auc_channel( buf );
    sprintf( buf, "%s auctioning %s.", auc_held->name, auc_obj->name );
    log_string( buf, CHANNEL_GOD, -1 );
    return;
  case 3:
    sprintf( buf, "%s for %d gold coins (going THRICE).", auc_obj->short_descr, auc_cost );
    auc_channel( buf );
    sprintf( buf, "%s auctioning %s.", auc_held->name, auc_obj->name );
    log_string( buf, CHANNEL_GOD, -1 );
    return;
  }

  if ( auc_bid && auc_bid->gold >= auc_cost )
  {
    sprintf( buf, "%s for %d gold coins SOLD! to %s.", auc_obj->short_descr, auc_cost,
       auc_bid->name );
    auc_bid->gold -= auc_cost;
    auc_held->gold += auc_cost;
    obj_to_char( auc_obj, auc_bid );
    act( AT_DGREEN, "$p appears in your hands.", auc_bid, auc_obj, NULL, TO_CHAR );
    act( AT_DGREEN, "$p appears in the hands of $n.", auc_bid, auc_obj, NULL,
     TO_ROOM );
  }
  else if ( auc_bid )
   {
     sprintf( buf, "%d gold coins not carried for %s, ending auction.", auc_cost, auc_obj->short_descr );
     obj_to_char( auc_obj, auc_held );
     act( AT_DGREEN, "$p appears in your hands.", auc_held, auc_obj, NULL, TO_CHAR );
     act( AT_DGREEN, "$p appears in the hands of $n.", auc_held, auc_obj, NULL, TO_ROOM );
   }
  else
   {
     sprintf( buf, "%s not sold, ending auction.", auc_obj->short_descr );
     obj_to_char( auc_obj, auc_held );
     act( AT_DGREEN, "$p appears in your hands.", auc_held, auc_obj, NULL, TO_CHAR );
     act( AT_DGREEN, "$p appears in the hands of $n.", auc_held, auc_obj, NULL,
       TO_ROOM );
   }
  auc_channel( buf );

  auc_count = -1;
  auc_cost = 0;
  auc_obj = NULL;
  auc_held = NULL;
  auc_bid = NULL;
  return;
}

void rdam_update( )
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *ch;

  for ( d = descriptor_list; d; d = d->next )
  {
    if ( d->connected != CON_PLAYING )
      continue;

   ch = d->original ? d->original : d->character;
    if ( !( ch->in_room ) )
      continue;

   if ( ch->level < L_APP && ch->position != POS_GHOST )
    if ( IS_SET( ch->in_room->room_flags, ROOM_DAMAGE ) )
    {
      if ( ch->hit <= ch->in_room->rd )
      {
         send_to_char( AT_RED, "Pain shoots through your body.\n\r", ch );
      }
      damage( ch, ch, ch->in_room->rd, TYPE_UNDEFINED );
    }
  }

  return;
}


/* Wind timer routines.. needs updates for weather stuff.. -- Altrag */
const char *dir_wind [] = {"north", "northeast", "east", "southeast",
			   "south", "southwest", "west", "northwest"};
/* END */

void strew_corpse( OBJ_DATA *obj, AREA_DATA *inarea )
{
  OBJ_DATA *currobj;
  ROOM_INDEX_DATA *newroom;
  OBJ_DATA *cobj_next;
  
  for ( currobj = obj->contains; currobj; currobj = cobj_next )
  {
     cobj_next = currobj->next_content;
     switch( currobj->item_type )
     {
       case ITEM_FOOD:
       case ITEM_DRINK_CON:
              currobj->value[3] = 1;
              break;
       case ITEM_POTION:
              if ( number_percent( ) < 20 )
              {
                extract_obj( currobj );
                continue;
              }
              break;
       default: break;
     }
     
     if ( number_percent( ) < 2 )
     {
       extract_obj( currobj );
       continue;
     }
     if ( number_percent( ) < 30 )
     {
       obj_from_obj( currobj );
       obj_to_room( currobj, obj->in_room );
     }
    else
     {
       obj_from_obj( currobj );
       newroom = get_room_index( number_range( inarea->lvnum, inarea->uvnum ) );
       for ( ; !newroom; )
         newroom = get_room_index( number_range( inarea->lvnum, inarea->uvnum ) );
       obj_to_room( currobj, newroom );
     }
  }
  extract_obj( obj );
  return;
}

void orprog_update( void )
{
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  AREA_DATA *pArea;

  for ( obj = object_list; obj; obj = obj_next )
  {
    obj_next = obj->next;
    if ( obj->deleted )
      continue;
    /* ie: carried or in room */
    if ( !obj->in_obj && !obj->stored_by &&
	((obj->in_room && obj->in_room->area->nplayer) ||
	 (obj->carried_by && obj->carried_by->in_room &&
	  obj->carried_by->in_room->area->nplayer)) )
      oprog_random_trigger( obj );
  }

  for ( pArea = area_first; pArea; pArea = pArea->next )
    if ( pArea->nplayer > 0 )
    {
      int room;
      ROOM_INDEX_DATA *pRoom;

      for ( room = pArea->lvnum; room <= pArea->uvnum; room++ )
	if ( (pRoom = get_room_index( room )) )
	  rprog_random_trigger( pRoom );
    }

  return;
}

void trap_update( void )
{
  TRAP_DATA *pTrap;

  for ( pTrap = trap_list; pTrap; pTrap = pTrap->next )
    if ( --pTrap->disarm_dur <= 0 )
    {
      pTrap->disarm_dur = 0;
      pTrap->disarmed = FALSE;
    }
  return;
}

void rtime_update( void )
{
  AREA_DATA *pArea;

  for ( pArea = area_first; pArea; pArea = pArea->next )
    if ( pArea->nplayer )
    {
      int room;
      ROOM_INDEX_DATA *pRoom;

      for ( room = pArea->lvnum; room <= pArea->uvnum; room++ )
	if ( (pRoom = get_room_index( room )) )
	  rprog_time_trigger( pRoom, time_info.hour );
    }

  return;
}

void craft_update( void )
{
	CHAR_DATA* ch;	

	for (ch = char_list; ch; ch = ch->next)
	{
		if (IS_NPC(ch)) { continue; }
		if (!ch->pcdata->craft_timer) { continue; }

		ch->pcdata->craft_timer -= PULSE_VIOLENCE;

		if (ch->pcdata->craft_timer <= 0)
		{
			ch->pcdata->craft_timer = 0;
			finish_craft( ch );
		}
	}
}

void timed_room_update( void )
{
	ROOM_INDEX_DATA* room;
	ROOM_INDEX_DATA* last = NULL;

	for (room = timed_room_list; room; room = room->next_timed_room)
	{
		room->flag_timer -= PULSE_VIOLENCE;

		if (room->flag_timer <= 0)
		{
			room->flag_timer = 0;
		
			strip_timed_room_flags(room);

			if (room == timed_room_list)
				timed_room_list = room->next_timed_room;
			else
				last->next_timed_room = room->next_timed_room;
		
			room->next_timed_room = NULL;
		}

		last = room;
	}
}
