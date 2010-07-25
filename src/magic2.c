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

/*$Id: magic2.c,v 1.33 2005/03/24 21:42:21 mud Exp $*/

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


int spell_incinerate( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( saves_spell( level, victim ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FLAMING;
    affect_join( victim, &af );

    if ( ch != victim )
	send_to_char(AT_RED, "Ok.\n\r", ch );
    send_to_char(AT_RED, "Your body bursts into flames!\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int  spell_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_INVISIBLE ) )
	return SKPELL_MISSED;

    if ( IS_AFFECTED2( victim, AFF_IMPROVED_INVIS ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = 24;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( victim, &af );

    send_to_char(AT_GREY, "You fade out of existence.\n\r", victim );
    act(AT_GREY, "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_improved_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_IMPROVED_INVIS ) )
	return SKPELL_MISSED;

    if ( IS_AFFECTED( victim, AFF_INVISIBLE ) )
        {
	send_to_char( AT_BLUE, "You can't cast this on yourself when already affected by invisibility.\n\r", victim );
	return SKPELL_MISSED;
	}

    af.type      = sn;
    af.level	 = level;
    af.duration  = 24;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_IMPROVED_INVIS;
    affect_to_char2( victim, &af );

    send_to_char(AT_GREY, "You fade out of existence.\n\r", victim );
    act(AT_GREY, "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_phase_shift( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_PHASED ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = ch->level/6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PHASED;
    affect_to_char2( victim, &af );

    send_to_char(AT_GREY, "You phases into another plane.\n\r", victim );
    act(AT_GREY, "$n phases out of reality.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}



int  spell_know_alignment( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char      *msg;
    int        ap;

    ap = victim->alignment;

         if ( ap >  700 ) msg = "$N has an aura as white as the driven snow.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "$N's slash DISEMBOWELS you!";
    else msg = "I'd rather just not say anything at all about $N.";

    act(AT_BLUE, msg, ch, NULL, victim, TO_CHAR );
    return SKPELL_NO_DAMAGE;
}



int  spell_lightning_bolt( int sn, int level, CHAR_DATA *ch, void *vo )
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
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 6 );
    if ( IS_OUTSIDE( ch ) )  
    {
        dam = (int) (dam * 1.5);
        if ( weather_info.sky < SKY_RAINING )
        {
            dam /= 2;
        }
        if ( weather_info.sky > SKY_RAINING )
        {
            send_to_char(AT_WHITE, "The bad weather increases the spells power!\n\r", ch );
            dam *= 2;
        }
    }

    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}



int  spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo )
{
    return  SKPELL_NO_DAMAGE;
/*
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    char      buf [ MAX_INPUT_LENGTH ];
    bool      found;

    found = FALSE;

    for ( obj = object_list; obj; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !is_name( target_name, obj->name ) )
	    continue;

	if ( IS_SET( obj->extra_flags, ITEM_NO_LOCATE) && ( get_trust( ch ) < L_APP ) )
	    continue;
	    
	found = TRUE;

	for ( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by )
	{
	    sprintf( buf, "%s carried by %s.\n\r",
		    obj->short_descr, PERS( in_obj->carried_by, ch ) );
	}
	else if ( in_obj->stored_by )
	{
	    sprintf( buf, "%s in storage.\n\r",
		    obj->short_descr );
	}
	else
	{
	    sprintf( buf, "%s in %s.\n\r",
		    obj->short_descr, !in_obj->in_room

		    ? "somewhere" : in_obj->in_room->name );
	}

	buf[0] = UPPER( buf[0] );
	send_to_char(AT_BLUE, buf, ch );
    }

    if ( !found )
	send_to_char(AT_WHITE, "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return SKPELL_NO_DAMAGE;
	*/
}



int  spell_magic_missile( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim      = (CHAR_DATA *) vo;
    static const int       dam_each [ ] = 
    {
	 0,
	 3,  3,  4,  4,  5,	 6,  6,  6,  6,  6,
	 7,  7,  7,  7,  7,	 8,  8,  8,  8,  8,
	 9,  9,  9,  9,  9,	10, 10, 10, 10, 10,
	11, 11, 11, 11, 11,	12, 12, 12, 12, 12,
	13, 13, 13, 13, 13,	14, 14, 14, 14, 14,
	15, 15, 15, 15, 15,	16, 16, 16, 16, 16,
	18, 18, 18, 18, 18,	20, 20, 20, 20, 20,
	21, 21, 21, 21, 21,	22, 22, 22, 22, 22,
	24, 24, 24, 24, 24,	26, 26, 26, 26, 26,
	28, 28, 28, 28, 28,	30, 31, 32, 33, 40
    };
		 int       dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 5 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int  spell_mana( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    victim->mana = UMIN( victim->mana + victim->level * 3, victim->max_mana );
    update_pos( victim );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "You feel a surge of energy.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}


int  spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *gch;
    AFFECT_DATA af;

    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_AFFECTED( gch, AFF_INVISIBLE ) )
	    continue;

	send_to_char(AT_GREY, "You slowly fade out of existence.\n\r", gch );
	act(AT_GREY, "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );

	af.type      = sn;
        af.level     = level;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char( gch, &af );
    }
    send_to_char(AT_BLUE, "Ok.\n\r", ch );

    return SKPELL_NO_DAMAGE;
}



int  spell_null( int sn, int level, CHAR_DATA *ch, void *vo )
{
    send_to_char(AT_WHITE, "That's not a spell!\n\r", ch );
    return SKPELL_NO_DAMAGE;
}



int  spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected(victim, sn) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );

    send_to_char(AT_GREY, "You turn translucent.\n\r", victim );
    act(AT_GREY, "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_permenancy( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;

    if ( obj->item_type != ITEM_WAND
	&& obj->item_type != ITEM_STAFF
	&& obj->item_type != ITEM_LENSE )
    {
	send_to_char(AT_BLUE, "You cannot make that item permenant.\n\r", ch );
	return SKPELL_MISSED;
    }
    obj->value[2] = -1;
    obj->value[1] = -1;
    act(AT_BLUE, "You run your finger up $p, you can feel it's power growing.", ch, obj, NULL, TO_CHAR );
    act(AT_BLUE, "$n slowly runs $s finger up $p.", ch, obj, NULL, TO_ROOM );
    return  SKPELL_NO_DAMAGE;
}


int  spell_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( saves_spell( level, victim ) )
	return SKPELL_MISSED;

    if( IS_AFFECTED( victim, AFF_POISON ) )
    {
	af.type		= sn;
	af.level	= level;
	af.duration	= level/2;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	af.bitvector	= AFF_POISON;
	affect_join( victim, &af );
	add_poison( victim, 1 );
    }
    else
    {
	af.type      = sn;
	af.level	 = level;
	af.duration  = level;
	af.location  = APPLY_STR;
	af.modifier  = -2;
	af.bitvector = AFF_POISON;
	affect_join( victim, &af );
	add_poison( victim, 5 );
    }

    if ( ch != victim )
	send_to_char(AT_GREEN, "Ok.\n\r", ch );
    send_to_char(AT_GREEN, "You feel very sick.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int  spell_polymorph( int sn, int level, CHAR_DATA *ch, void *vo )
{
   CHAR_DATA          *victim = (CHAR_DATA *) vo;
  char                buf [MAX_STRING_LENGTH]; 
    AFFECT_DATA af;

   if ( !(victim = get_char_world( ch, target_name ) )
          || victim == ch
          || saves_spell( level, victim)
          || IS_AFFECTED( ch, AFF_POLYMORPH ) )
      {
         send_to_char( AT_BLUE, "You failed.\n\r", ch );
         return SKPELL_MISSED;
      }

    af.type      = sn;
    af.level	 = level;
    af.duration  = level/5;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_POLYMORPH;
    affect_to_char2( ch, &af );
    
    if (!IS_NPC(victim))
      {
       sprintf( buf, "%s %s", victim->name, victim->pcdata->title);
       free_string( ch->long_descr );
       ch->long_descr = str_dup(buf);
      }
    else
      {
       sprintf( buf, "%s", victim->long_descr );
       free_string( ch->long_descr );
       ch->long_descr = str_dup(buf);
      }
    act(AT_BLUE, "$n's form wavers and then resolidifies.", ch, NULL, NULL, TO_ROOM);
    send_to_char(AT_BLUE, "You have succesfully polymorphed.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

int  spell_portal( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    OBJ_DATA           *gate1;
    OBJ_DATA           *gate2;
    int                duration;
    
    if ( !( victim = get_char_world( ch, target_name ) )
	|| victim == ch
	|| !victim->in_room
	|| IS_SET( victim->in_room->room_flags, ROOM_SAFE      )
	|| IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
	|| IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  )
	|| IS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL_IN  )
	|| IS_SET( ch->in_room->room_flags, ROOM_NO_ASTRAL_OUT )
	|| IS_SET( victim->in_room->area->area_flags, AREA_PROTOTYPE ) )
          {
        	send_to_char(AT_BLUE, "You failed.\n\r", ch );
        	return SKPELL_MISSED;
          }

    if ( IS_NPC( victim) )
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

    duration = level/8;
    gate1 = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
    gate2 = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
    gate1->timer = duration;
    gate2->timer = duration;
    gate2->value[0] = ch->in_room->vnum;
    gate1->value[0] = victim->in_room->vnum;
    act(AT_BLUE, "A huge shimmering gate rises from the ground.", ch, NULL, NULL, TO_CHAR );
    act(AT_BLUE, "$n utters a few incantations and a gate rises from the ground.", ch, NULL, NULL, TO_ROOM );
    obj_to_room( gate1, ch->in_room );
    act(AT_BLUE, "A huge shimmering gate rises from the ground.", victim, NULL, NULL, TO_CHAR );
    act(AT_BLUE, "A huge shimmering gate rises from the ground.", victim, NULL, NULL, TO_ROOM );
    obj_to_room( gate2, victim->in_room );
    return SKPELL_NO_DAMAGE;
}


int  spell_vortex( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    OBJ_DATA           *gate1;
    OBJ_DATA           *gate2;
    int                duration;
    
    if ( !( victim = get_char_world( ch, target_name ) )
	|| victim == ch
	|| !victim->in_room
	|| IS_SET( victim->in_room->room_flags, ROOM_SAFE      )
	|| IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
	|| IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  )
	|| IS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL_IN  )
	|| IS_SET( ch->in_room->room_flags, ROOM_NO_ASTRAL_OUT )
	|| IS_SET( victim->in_room->area->area_flags, AREA_PROTOTYPE ) )
          {
        	send_to_char(AT_BLUE, "You failed.\n\r", ch );
        	return SKPELL_MISSED;
          }

    if ( !IS_NPC( victim) )
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

    duration = level/8;
    gate1 = create_object( get_obj_index( OBJ_VNUM_VORTEX ), 0 );
    gate1->timer = duration;
    gate1->value[0] = victim->in_room->vnum;
    gate2 = create_object( get_obj_index( OBJ_VNUM_VORTEX_NULL ), 0 );
    gate2->timer = duration;
    act(AT_BLUE, "A huge pulsing vortex appears!", ch, NULL, NULL, TO_CHAR );
    act(AT_BLUE, "$n utters a few incantations and a vortex appears.", ch, NULL, NULL, TO_ROOM );
    obj_to_room( gate1, ch->in_room );
    act(AT_BLUE, "A huge pulsing vortex fades in and out of existance.", victim, NULL, NULL, TO_CHAR );
    act(AT_BLUE, "A huge pulsing vortex fades in and out of existance.", victim, NULL, NULL, TO_ROOM );
    obj_to_room( gate2, victim->in_room );
    return SKPELL_NO_DAMAGE;
}

int  spell_blade_doom( int sn, int level, CHAR_DATA *ch, void *vo )
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
    blade = create_object( get_obj_index( OBJ_VNUM_BLADE_DOOM ), ch->level );
    blade->timer = duration;
    act(AT_BLUE, "A unworldly blade appears in the sky and descends into your hands.", ch, NULL, NULL, TO_CHAR );
    act(AT_BLUE, "A unworldly blade appears in the sky and descends into $n's hands.", ch, NULL, NULL, TO_ROOM );
    obj_to_char( blade, ch );
  if(ch->mana < level * 2 && ((ch->class != CLASS_VAMPIRE) || (ch->class != CLASS_ANTI_PALADIN )))
  {
    act(AT_RED,
     "You don't have enough mana to call the sword!", ch, NULL, NULL, TO_CHAR);
    return SKPELL_MISSED;
  }
  if(ch->bp < level / 2 && ((ch->class == CLASS_VAMPIRE) || (ch->class == CLASS_ANTI_PALADIN)))
  {
    act(AT_BLOOD,
     "You don't have enough blood to call the sword!", ch, NULL, NULL, TO_CHAR);
    return SKPELL_MISSED;
  }
  if((ch->class != CLASS_VAMPIRE) || (ch->class != CLASS_ANTI_PALADIN))
    ch->mana -= level * 4;
  else
    ch->bp -= level * 2;
    ch->summon_timer = 50;
    return SKPELL_NO_DAMAGE;
}

int  spell_vine_portal( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA          *victim = (CHAR_DATA *) vo;
    OBJ_DATA           *gate1;
    OBJ_DATA           *gate2;
    int                duration;
    
    if ( !( victim = get_char_world( ch, target_name ) )
	|| victim == ch
	|| !victim->in_room
	|| IS_SET( victim->in_room->room_flags, ROOM_SAFE      )
	|| IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
	|| IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  )
	|| IS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL_IN  )
	|| IS_SET( ch->in_room->room_flags, ROOM_NO_ASTRAL_OUT )
	|| IS_SET( victim->in_room->area->area_flags, AREA_PROTOTYPE ) )
          {
        	send_to_char(AT_BLUE, "You failed.\n\r", ch );
        	return SKPELL_MISSED;
          }

    if (IS_NPC( victim ) )
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

    duration = level/8;
    gate1 = create_object( get_obj_index( OBJ_VNUM_VINE_PORTAL ), 0 );
    gate2 = create_object( get_obj_index( OBJ_VNUM_VINE_PORTAL ), 0 );
    gate1->timer = duration;
    gate2->timer = duration;
    gate2->value[0] = ch->in_room->vnum;
    gate1->value[0] = victim->in_room->vnum;
    act(AT_BLUE, "Vines rise from the ground and form a walkway.", ch, NULL, NULL, TO_CHAR );
    act(AT_BLUE, "$n utters a few words and some vines form into a walkway.", ch, NULL, NULL, TO_ROOM );
    obj_to_room( gate1, ch->in_room );
    act(AT_BLUE, "Vines rise to form a walkway.", victim, NULL, NULL, TO_CHAR );
    act(AT_BLUE, "Vines rise to form a walkway.", victim, NULL, NULL, TO_ROOM );
    obj_to_room( gate2, victim->in_room );
    return SKPELL_NO_DAMAGE;
}


int  spell_protection( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_PROTECT ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = 24;
    af.location  = APPLY_DAM_UNHOLY;
    af.modifier  = 20;
    af.bitvector = AFF_PROTECT;
    affect_to_char( victim, &af );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "You feel protected.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}



int  spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    victim->move = UMIN( victim->move + level + 50, victim->max_move );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "You feel less tired.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}


/* Expulsion of ITEM_NOREMOVE addition by Katrina */
int  spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA  *obj;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        iWear;
    int        yesno  = 0;

    for ( iWear = 0; iWear < MAX_WEAR; iWear ++ )
    {
	if ( !( obj = get_eq_char( victim, iWear ) ) )
	    continue;

        if ( IS_SET( obj->extra_flags, ITEM_NODROP ))
        {
            REMOVE_BIT( obj->extra_flags, ITEM_NODROP );
            send_to_char( AT_BLUE, "You feel a burden relieved.\n\r", ch );
            yesno = 1;
        }
	if ( IS_SET( obj->extra_flags, ITEM_NOREMOVE ) )
	{
	    unequip_char( victim, obj );
	    obj_from_char( obj );
	    obj_to_room( obj, victim->in_room );
	    act(AT_BLUE, "You toss $p to the ground.",  victim, obj, NULL, TO_CHAR );
	    act(AT_BLUE, "$n tosses $p to the ground.", victim, obj, NULL, TO_ROOM );
            REMOVE_BIT( obj->extra_flags, ITEM_NOREMOVE );
	    yesno = 1;
	}
    }
    if ( is_affected( victim, skill_lookup("incinerate")) )
    {   
        affect_strip( victim, skill_lookup("incinerate"));
        send_to_char(AT_BLUE, "Your body has been extinguished.\n\r", ch);
        yesno = 1;
    }    
    if ( is_affected( victim, skill_lookup("curse") ) )
    {
	affect_strip( victim, skill_lookup("curse") );
	send_to_char(AT_BLUE, "You feel better.\n\r", victim );
	yesno = 1;
    }
    
    if ( ch != victim && yesno )
        send_to_char(AT_BLUE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

int  spell_remove_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA  *obj;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        iWear;
    int        yesno  = 0;

    for ( iWear = 0; iWear < MAX_WEAR; iWear ++ )
    {
	if ( !( obj = get_eq_char( victim, iWear ) ) )
	    continue;

        if ( IS_SET( obj->extra_flags, ITEM_INVIS ) )
        {
            REMOVE_BIT( obj->extra_flags, ITEM_INVIS );
            send_to_char( AT_BLUE, "The item is no longer invisible.\n\r", ch );
            yesno = 1;
        }    
    }
    if ( ch != victim && yesno )
        send_to_char(AT_BLUE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

int  spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_SANCTUARY ) )
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

    if ( IS_AFFECTED2( victim, AFF_GOLDEN_ARMOR ) || IS_AFFECTED4( victim, AFF_GOLDEN_SANCTUARY ) || IS_AFFECTED4( victim, AFF_BIOFEEDBACK ) || IS_AFFECTED4( victim, AFF_ESSENCE_OF_GAIA ) )
	return SKPELL_MISSED;

    if ( !IS_SHIELDABLE( victim ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = -1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af );
    victim->shields += 1;

    send_to_char(AT_WHITE, "You are surrounded by a white aura.\n\r", victim );
    act(AT_WHITE, "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_golden_armor( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    AFFECT_DATA *paf;

    if ( IS_AFFECTED2( victim, AFF_GOLDEN_ARMOR ) )
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

    if ( IS_AFFECTED4( victim, AFF_GOLDEN_SANCTUARY ) )
    {
        for ( paf = victim->affected4; paf; paf = paf->next )
        {
            if ( paf->deleted )
                continue;
            if ( skill_table[ (int) paf->type].slot == 245 )
                affect_strip(victim,(int) paf->type );
        }
        send_to_char(AT_YELLOW, "Your golden sanctuary fades.", victim );
        send_to_char(C_DEFAULT, "\n\r", victim );
        act(AT_YELLOW, "$n's golden sanctuary fades.", victim, NULL, NULL, TO_NOTVICT);
        victim->shields -= 1;
        spell_sanctuary( skill_lookup("sanctuary"), ch->level, ch, victim );
        return SKPELL_NO_DAMAGE;
    }

    if ( IS_AFFECTED( victim, AFF_SANCTUARY ) )
    {
        spell_golden_sanctuary( skill_lookup("golden sanctuary"), ch->level, ch, victim );
        return SKPELL_NO_DAMAGE;
    }

    if ( IS_AFFECTED4( victim, AFF_BIOFEEDBACK ) || IS_AFFECTED4( victim, AFF_ESSENCE_OF_GAIA ) )
    {
        return SKPELL_MISSED;
    }

    if ( !IS_SHIELDABLE( victim ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = -1;
    af.location  = APPLY_DAM_PHYSICAL;
    af.modifier  = 50;
    af.bitvector = AFF_GOLDEN_ARMOR;
    affect_to_char2( victim, &af );
    victim->shields += 1;

    send_to_char(AT_YELLOW, "Your armor glows golden.\n\r", victim );
    act(AT_YELLOW, "$n's armor takes on a golden appearance.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_golden_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    AFFECT_DATA *paf;

    if ( !IS_AFFECTED( victim, AFF_SANCTUARY ) )
    {
        send_to_char(C_DEFAULT, "This spell requires you to have a sanctuary.\n\r", victim );
        return SKPELL_MISSED;
    }

    if ( IS_AFFECTED4( victim, AFF_GOLDEN_SANCTUARY ) )
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

    for ( paf = victim->affected; paf; paf = paf->next )
    {
        if ( paf->deleted )
            continue;
        if ( skill_table[ (int) paf->type].slot == 70 )
            affect_strip(victim,(int) paf->type );
    }

    af.type      = sn;
    af.level	 = victim->level;
    af.duration  = -1;
    af.location  = APPLY_DAM_PHYSICAL;
    af.modifier  = 50;
    af.bitvector = AFF_GOLDEN_SANCTUARY;
    affect_to_char4( victim, &af );

    send_to_char(AT_YELLOW, "Your sanctuary glows golden.\n\r", victim );
    act(AT_YELLOW, "$n's sanctuary takes on a golden appearance.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_web( int sn, int level, CHAR_DATA *ch, void *vo )
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
    
    sprintf( buf, "%s lifts his hands and webs entanle you!\n\r", ch->name );
    send_to_char(AT_WHITE, buf, victim );
    act(AT_WHITE, "$n has been immobilized by a plethora of sticky webs.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_confusion( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    char        buf[MAX_STRING_LENGTH];
    
    if ( is_affected(victim, sn))
	return SKPELL_MISSED;
if ( saves_spell( level, victim ) )
   {
     send_to_char( AT_BLUE, "You failed.\n\r", ch );
     return SKPELL_MISSED;
   }

    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 10 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_CONFUSED;
    affect_to_char2( victim, &af );
    
    sprintf( buf, "You feel disorientated.\n\r" );
    send_to_char(AT_WHITE, buf, victim );
    act(AT_WHITE, "$n stares around blankly.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_fumble( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    char        buf[MAX_STRING_LENGTH];
    
    if ( IS_AFFECTED2( victim, AFF_FUMBLE ) )
	return SKPELL_MISSED;

    if ( saves_spell( level, victim ) )
    {
	send_to_char( AT_BLUE, "You failed.\n\r", ch );
	return SKPELL_MISSED;
    }

    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 10 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FUMBLE;
    affect_to_char2( victim, &af );
 
    af.location  = APPLY_HITROLL;
    af.modifier  = 0 - level / 5;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    
    sprintf( buf, "You feel clumsy.\n\r" );
    send_to_char(AT_WHITE, buf, victim );
    act(AT_WHITE, "$n looks very clumsy.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_mind_probe( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA *paf;
    CHAR_DATA   *victim;
    bool printed = FALSE;
    char         buf  [ MAX_STRING_LENGTH ];
    char         buf1 [ MAX_STRING_LENGTH ];
    
    
    if (!(victim = get_char_room( ch, target_name ) ) )
    {
      send_to_char(AT_BLUE, "You cannot find them.\n\r", ch ); 
      return SKPELL_MISSED;
    }
    sprintf(buf1, "You send your conciousness into %s's mind.\n\r", victim->name);
    send_to_char(AT_RED, buf1, ch );
    send_to_char(AT_RED, "You feel someone touch your mind.\n\r", victim );
    buf1[0] = '\0';
    
    if (IS_NPC(victim))
       {
         send_to_char (AT_WHITE, "The mind is to chaotic to merge with.\n\r", ch );
         return SKPELL_BOTCHED;
       }
    sprintf( buf,
	    "You are %s%s.\n\r",
	    victim->name,
	    IS_NPC( victim ) ? "" : victim->pcdata->title );
    send_to_char( AT_CYAN, buf, ch );
    sprintf( buf,
            "Level &C%d&c, %d years old (%d hours).\n\r",
	    victim->level,
	    get_age( victim ),
	    (get_age( victim ) - 17) * 4 );
    send_to_char( AT_CYAN, buf, ch );
    sprintf( buf, "You are a &Y%s&c and have chosen the vocation of a &Y%s&c.\n\r",
             race_table[victim->race].race_full, class_table[victim->class].who_long );
    send_to_char( AT_CYAN, buf, ch );
    if ( victim->multied != victim->class );
    {
	sprintf( buf, "You have chosen to multiclass to &Y%s&c.\n\r",
	    class_table[victim->multied].who_long );
	send_to_char( AT_CYAN, buf, ch );
    }
    if(victim->multied == victim->class )
    {
	send_to_char( AT_CYAN, "You have not yet multiclassed.\n\r", ch );
    }
    if ( victim->clan )
    {
        CLAN_DATA *clan;
        
        clan = get_clan_index( victim->clan );
        sprintf( buf, "You belong to the clan %s.\n\r", clan->name );
        send_to_char( AT_WHITE, buf, ch );
    }
    
    if ( get_trust( victim ) != victim->level )
    {
	sprintf( buf, "You have been granted the powers of a level &R%d&W.\n\r",
		get_trust( victim ) );
        send_to_char( AT_WHITE, buf, ch );
    }
    
    if (!IS_NPC( victim ))
    if ( get_trust( victim ) > LEVEL_DEMIGOD )
    {
        sprintf( buf, "Bamfin&r: &w%s.\n\r", victim->pcdata->bamfin );
        send_to_char( AT_RED, buf, ch );
        sprintf( buf, "Bamfout&r: &w%s.\n\r", victim->pcdata->bamfout );
        send_to_char( AT_RED, buf, ch );
	sprintf( buf, "Bamfsin&r: &w%s.\n\r", victim->pcdata->bamfsin );
	send_to_char( AT_RED, buf, ch );
	sprintf( buf, "Bamfsout&r: &w%s.\n\r", victim->pcdata->bamfsout );
	send_to_char( AT_RED, buf, ch );
    }

    send_to_char( AT_CYAN, "You have", ch );
    sprintf ( buf, " %d/%d", victim->hit, victim->max_hit );
    send_to_char( AT_YELLOW, buf, ch );
    send_to_char( AT_CYAN, " hit, ", ch );
    if (( victim->class != 9 )&&(victim->class  != 11))
       {
         sprintf ( buf, "%d/%d", victim->mana, victim->max_mana );
         send_to_char( AT_LBLUE, buf, ch );
         send_to_char( AT_CYAN, " mana, ", ch );
       }
    else
       {
         sprintf ( buf, "%d/%d", victim->bp, victim->max_bp );
         send_to_char( AT_RED, buf, ch );
         send_to_char( AT_CYAN, " blood, ", ch );
       }
    sprintf ( buf, "%d/%d", victim->move, victim->max_move );
    send_to_char( AT_GREEN, buf, ch );
    send_to_char( AT_CYAN, " movement, ", ch );
    sprintf ( buf, "%d", victim->practice );
    send_to_char( AT_WHITE, buf, ch );
    send_to_char( AT_CYAN, " practices.\n\r", ch );

    sprintf( buf,
	    "You are carrying %d/%d items with weight %d/%d kg.\n\r",
	    victim->carry_number, can_carry_n( victim ),
	    victim->carry_weight, can_carry_w( victim ) );
    send_to_char( AT_CYAN, buf, ch );

    sprintf( buf,
	"Str: %d&p(&P%d&p)&P  Int: %d&p(&P%d&p)&P  Wis: %d&p(&P%d&p)&P  Dex: %d&p(&P%d&p)&P  Con: %d&p(&P%d&p)&P.\n\r",
	IS_NPC(victim) ? 13: victim->pcdata->perm_str, IS_NPC(victim) ? 13: get_curr_str( victim ),
	IS_NPC(victim) ? 13: victim->pcdata->perm_int, IS_NPC(victim) ? 13: get_curr_int( victim ),
	IS_NPC(victim) ? 13: victim->pcdata->perm_wis, IS_NPC(victim) ? 13: get_curr_wis( victim ),
	IS_NPC(victim) ? 13: victim->pcdata->perm_dex, IS_NPC(victim) ? 13: get_curr_dex( victim ),
	IS_NPC(victim) ? 13: victim->pcdata->perm_con, IS_NPC(victim) ? 13: get_curr_con( victim ) );
    send_to_char( AT_PINK, buf, ch );

    send_to_char( AT_CYAN, "You have scored ", ch );
    sprintf( buf, "%d ", victim->exp );
    send_to_char( AT_WHITE, buf, ch );
    send_to_char( AT_CYAN, "exp, and have accumulated ", ch );
    sprintf( buf, "%d ", victim->gold );
    send_to_char( AT_YELLOW, buf, ch );
    send_to_char( AT_CYAN, "gold coins.\n\r", ch );

    if ( !IS_NPC( victim ) && victim->pcdata->condition[COND_DRUNK]   > 10 )
	send_to_char( AT_GREY, "You are drunk.\n\r", ch );
    if ( !IS_NPC( victim ) && victim->pcdata->condition[COND_THIRST] ==  0
	&& victim->level >= LEVEL_IMMORTAL )
	send_to_char( AT_BLUE, "You are thirsty.\n\r", ch );
    if ( !IS_NPC( victim ) && victim->pcdata->condition[COND_FULL]   ==  0
	&& victim->level >= LEVEL_IMMORTAL )
	send_to_char( AT_ORANGE, "You are hungry.\n\r", ch  );

    switch ( victim->position )
    {
    case POS_DEAD:     
	send_to_char( (AT_RED + AT_BLINK), "You are DEAD!!\n\r", ch ); break;
    case POS_MORTAL:
	send_to_char( AT_RED, "You are mortally wounded.\n\r", ch ); break;
    case POS_INCAP:
	send_to_char( AT_RED, "You are incapacitated.\n\r", ch ); break;
    case POS_STUNNED:
	send_to_char( AT_RED, "You are stunned.\n\r", ch ); break;
    case POS_SLEEPING:
	send_to_char( AT_LBLUE, "You are sleeping.\n\r", ch ); break;
    case POS_RESTING:
	send_to_char( AT_LBLUE, "You are resting.\n\r", ch ); break;
    case POS_STANDING:
	send_to_char( AT_GREEN, "You are standing.\n\r", ch ); break;
    case POS_FIGHTING:
	send_to_char( AT_BLOOD, "You are fighting.\n\r", ch ); break;
    }

    if ( ch->level >= 20 )
    {
	sprintf( buf, "AC: %d.  ", GET_AC( victim ) );
	send_to_char( AT_CYAN, buf, ch );
    }

    send_to_char( AT_GREEN, "You are ", ch );
         if ( GET_AC( victim ) >=  101 ) send_to_char( AT_GREEN, "WORSE than naked!\n\r", ch );
    else if ( GET_AC( victim ) >=   20 ) send_to_char( AT_GREEN, "naked.\n\r"           , ch );
    else if ( GET_AC( victim ) >=    0 ) send_to_char( AT_GREEN, "wearing clothes.\n\r" , ch );
    else if ( GET_AC( victim ) >= - 50 ) send_to_char( AT_GREEN, "slightly armored.\n\r", ch );
    else if ( GET_AC( victim ) >= -100 ) send_to_char( AT_GREEN, "somewhat armored.\n\r", ch );
    else if ( GET_AC( victim ) >= -250 ) send_to_char( AT_GREEN, "armored.\n\r"         , ch );
    else if ( GET_AC( victim ) >= -500 ) send_to_char( AT_GREEN, "well armored.\n\r"    , ch );
    else if ( GET_AC( victim ) >= -750 ) send_to_char( AT_GREEN, "strongly armored.\n\r", ch );
    else if ( GET_AC( victim ) >= -1000 ) send_to_char( AT_GREEN, "heavily armored.\n\r" , ch );
    else if ( GET_AC( victim ) >= -1200 ) send_to_char( AT_GREEN, "superbly armored.\n\r", ch );
    else if ( GET_AC( victim ) >= -1400 ) send_to_char( AT_GREEN, "divinely armored.\n\r", ch );
    else                           send_to_char( AT_GREEN, "invincible!\n\r", ch );

    if ( ch->level >= 12 )
    {
	sprintf( buf, "Hitroll: " );
	send_to_char(AT_BLOOD, buf, ch );
	sprintf( buf, "%d", GET_HITROLL( victim ) );
	send_to_char(AT_RED, buf, ch);
	sprintf( buf, "  Damroll: " );
	send_to_char( AT_BLOOD, buf, ch );
	sprintf( buf, "%d.\n\r", GET_DAMROLL( victim ) * 2 );
	send_to_char( AT_RED, buf, ch );
    }
    
    if ( ch->level >= 8 )
    {
	sprintf( buf, "Alignment: %d.  ", victim->alignment );
	send_to_char( AT_CYAN, buf, ch );
    }

    send_to_char( AT_CYAN, "You are ", ch );
         if ( victim->alignment >  900 ) send_to_char( AT_BLUE, "angelic.\n\r",ch );
    else if ( victim->alignment >  700 ) send_to_char( AT_BLUE, "saintly.\n\r",ch );
    else if ( victim->alignment >  350 ) send_to_char( AT_BLUE, "good.\n\r"   ,ch );
    else if ( victim->alignment >  100 ) send_to_char( AT_BLUE, "kind.\n\r"   ,ch );
    else if ( victim->alignment > -100 ) send_to_char( AT_YELLOW, "neutral.\n\r",ch );
    else if ( victim->alignment > -350 ) send_to_char( AT_RED, "mean.\n\r"    ,ch);
    else if ( victim->alignment > -700 ) send_to_char( AT_RED, "evil.\n\r"    ,ch);
    else if ( victim->alignment > -900 ) send_to_char( AT_RED, "demonic.\n\r" ,ch);
    else                             send_to_char( AT_RED, "satanic.\n\r" ,ch);
  
    if ( !IS_NPC( victim ) && IS_IMMORTAL( victim ) )
    {
      sprintf( buf, "WizInvis level: %d   WizInvis is %s\n\r",
                      victim->wizinvis,
                      IS_SET( victim->act, PLR_WIZINVIS ) ? "ON" : "OFF" );
      send_to_char( AT_YELLOW, buf, ch );
    }
    if ( victim->affected )
    {
	for ( paf = victim->affected; paf; paf = paf->next )
	{
	    if ( paf->deleted )
	        continue;

	    if ( !printed )
	    {
		send_to_char( AT_CYAN, "You are affected by:\n\r", ch );
		printed = TRUE;
	    }

	    sprintf( buf, "Spell: '%s'", skill_table[paf->type].name );
            send_to_char( AT_WHITE, buf, ch );
	    if ( ch->level >= 20 )
	    {
		sprintf( buf,
			" modifies %s by %d for %d hours",
			affect_loc_name( paf->location ),
			paf->modifier,
			paf->duration );
		send_to_char(AT_WHITE, buf, ch );
	    }

	    send_to_char( AT_WHITE, ".\n\r", ch );
	}
    }

    if ( victim->affected2 )
    {
	for ( paf = victim->affected2; paf; paf = paf->next )
	{
	    if ( paf->deleted )
	        continue;

	    if ( !printed )
	    {
		send_to_char( AT_CYAN, "You are affected by:\n\r", ch );
		printed = TRUE;
	    }

	    sprintf( buf, "Spell: '%s'", skill_table[paf->type].name );
            send_to_char( AT_WHITE, buf, ch );
	    if ( ch->level >= 20 )
	    {
		sprintf( buf,
			" modifies %s by %d for %d hours",
			affect_loc_name( paf->location ),
			paf->modifier,
			paf->duration );
		send_to_char(AT_WHITE, buf, ch );
	    }

	    send_to_char( AT_WHITE, ".\n\r", ch );
	}
    }

    send_to_char( AT_RED, "The presence lifts from your mind.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int  spell_entangle( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    char        buf[MAX_STRING_LENGTH];
    
    if ( IS_AFFECTED( victim, AFF_ANTI_FLEE ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 40 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ANTI_FLEE;
    affect_to_char( victim, &af );
    
    sprintf( buf, "%s calls forth nature to hold you in place.\n\r", ch->name );
    send_to_char(AT_GREEN, buf, victim );
    act(AT_GREEN, "Hundreds of vines reach from the ground to entangle $n.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_scry( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_SCRY ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SCRY;
    affect_to_char( victim, &af );

    send_to_char(AT_BLUE, "Your vision improves.\n\r", victim );
    return SKPELL_MISSED;
}


int  spell_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = 8 + level;
    af.location  = APPLY_AC;
    af.modifier  = -30;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char(AT_BLUE, "You are surrounded by a force shield.\n\r", victim );
    act(AT_BLUE, "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}



int  spell_shocking_grasp( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0, 20, 25, 29, 33,
	36, 39, 39, 39, 40,	40, 41, 41, 42, 42,
	43, 43, 44, 44, 45,	45, 46, 46, 47, 47,
	48, 48, 49, 49, 50,	50, 51, 51, 52, 52,
	53, 53, 54, 54, 55,	55, 56, 56, 57, 57,
	58, 58, 59, 60, 61,	62, 63, 64, 65, 66,
	67, 68, 69, 70, 71,	72, 73, 74, 75, 76,
	77, 78, 79, 80, 81,	82, 83, 84, 85, 86,
	87, 88, 89, 90, 91,	92, 93, 94, 95, 96,
	97, 98, 99,100,101,	102,103,104,105,106
    };
		 int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    damage( ch, victim, dam, sn );
    return SKPELL_MISSED;
}

int  spell_shockshield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_SHOCKSHIELD ) )
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
    af.level	 = level;
    af.duration  = -1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SHOCKSHIELD;
    affect_to_char( victim, &af );
    victim->shields += 1;

    send_to_char(AT_BLUE, "Sparks of electricity flow into your body.\n\r", victim );
    act(AT_BLUE, "Bolts of electricity flow from the ground into $n's body.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}


int  spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo )
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
    af.level	 = level;
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

int  spell_spell_bind( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;

    if ( obj->item_type != ITEM_WAND
	&& obj->item_type != ITEM_STAFF
	&& obj->item_type != ITEM_LENSE )
    {
	send_to_char(AT_BLUE, "You cannot bind magic to that item.\n\r", ch );
	return SKPELL_MISSED;
    }
    if ( obj->value[2] == obj->value[1] )
    {
        send_to_char(AT_BLUE, "That item is at full charge.\n\r", ch );
        return SKPELL_MISSED;
    }
    obj->value[2] ++;
    act(AT_BLUE, "You slowly pass your hand over $p, it vibrates slowly.", ch, obj, NULL, TO_CHAR );
    act(AT_BLUE, "$n slowly passes $s hand over $p, it vibrates slowly.", ch, obj, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_stone_skin( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = ((ch->level/2) + 10) * -1;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char(AT_GREY, "Your skin turns to stone.\n\r", victim );
    act(AT_GREY, "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}



int  spell_summon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;

    if ( !( victim = get_char_world( ch, target_name ) )
	|| victim == ch
	|| !victim->in_room
	|| IS_SET( victim->in_room->room_flags, ROOM_SAFE      )
	|| IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
	|| IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  )
	|| IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
	|| victim->level >= level + 8
	|| victim->fighting
	|| ( IS_NPC( victim ) && saves_spell( level, victim ) ) 
	|| IS_SET( victim->in_room->area->area_flags, AREA_PROTOTYPE )
	|| IS_AFFECTED( victim, AFF_NOASTRAL ) 
	|| IS_AFFECTED4( victim, AFF_NO_SUMMON )
        || IS_AFFECTED4( victim, AFF_DECEPTION ) )
    {
	send_to_char(AT_BLUE, "You failed.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PRESENT)
	&& (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
	|| IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
	send_to_char(AT_BLUE, "You cannot!!!!!!!!\n\r", ch);
	return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_FUTURE)
	&& (IS_SET( victim->in_room->area->area_flags, AREA_PRESENT )
	|| IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
	send_to_char(AT_BLUE, "You cannot!!!!!!!!\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PAST)
	&& (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
	|| IS_SET(victim->in_room->area->area_flags, AREA_PRESENT ) ) )
    {
	send_to_char(AT_BLUE, "You cannot!!!!!!!!\n\r", ch );
	return SKPELL_MISSED;
    }

    act(AT_BLUE, "$n disappears suddenly.", victim, NULL, NULL,     TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act(AT_BLUE, "$n has summoned you!",    ch,     NULL, victim,   TO_VICT );
    act(AT_BLUE, "$n arrives suddenly.",    victim, NULL, NULL,     TO_ROOM );
    do_look( victim, "auto" );

    if (!IS_NPC(victim) && victim->pcdata->craft_timer)
	destroy_craft(victim, FALSE);

    return SKPELL_NO_DAMAGE;
}



int  spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA       *victim = (CHAR_DATA *) vo;
    CHAR_DATA *pet;
    ROOM_INDEX_DATA *pRoomIndex;

    if ( !victim->in_room
	|| IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL)
	|| IS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL_OUT)
	|| ( !IS_NPC( ch ) && victim->fighting )
	|| ( victim != ch
	    && ( saves_spell( level, victim )
		|| saves_spell( level, victim ) ) ) )
    {
	send_to_char(AT_BLUE, "You failed.\n\r", ch );
	return SKPELL_MISSED;
    }

    for ( ; ; )
    {
	pRoomIndex = get_room_index( number_range( 0, 32767 ) );
	if ( pRoomIndex )
	    if (   !IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE  )
		&& !IS_SET( pRoomIndex->room_flags, ROOM_SOLITARY )
		&& !IS_SET( pRoomIndex->room_flags, ROOM_NO_ASTRAL_IN    )
		&& !IS_SET( pRoomIndex->room_flags, ROOM_NO_RECALL) 
		&& !IS_SET( pRoomIndex->room_flags, ROOM_NO_MOB) 
		&& !IS_SET( pRoomIndex->area->area_flags, AREA_PAST )
		&& !IS_SET( pRoomIndex->area->area_flags, AREA_FUTURE )
		&& !IS_SET( pRoomIndex->area->area_flags, AREA_PROTOTYPE )
                && !(IS_SET( pRoomIndex->room_flags, ROOM_SAFE && ch->pkill)))
	    break;
    }

    for ( pet = victim->in_room->people; pet; pet = pet->next_in_room )
    {
      if ( IS_NPC( pet ) )
        if ( IS_SET( pet->act, ACT_PET ) && ( pet->master == victim ) )
          break;
    }
    
    act(AT_BLUE, "$n glimmers briefly, then is gone.", victim, NULL, NULL, TO_ROOM );
    if ( pet )
    {
      act( AT_BLUE, "$n glimmers briefly, then is gone.", pet, NULL, NULL, TO_ROOM );
      char_from_room( pet );
    }
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act(AT_BLUE, "The air starts to sparkle, then $n appears from nowhere.",   victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    if ( pet )
    {
      char_to_room( pet, pRoomIndex );
      act( AT_BLUE, "The air starts to sparkle, then $n appears from nowhere.", pet, NULL, NULL, TO_ROOM );
    }
    return SKPELL_NO_DAMAGE;
}



int  spell_ventriloquate( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    char       buf1    [ MAX_STRING_LENGTH ];
    char       buf2    [ MAX_STRING_LENGTH ];
    char       speaker [ MAX_INPUT_LENGTH  ];

    target_name = one_argument( target_name, speaker );

    sprintf( buf1, "%s says '%s'.\n\r",              speaker, target_name );
    sprintf( buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name );
    buf1[0] = UPPER( buf1[0] );

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
	if ( !is_name( speaker, vch->name ) )
	    send_to_char(AT_CYAN, saves_spell( level, vch ) ? buf2 : buf1, vch );
    }

    return SKPELL_NO_DAMAGE;
}



int  spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || saves_spell( level, victim ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level / 6;
    af.location  = APPLY_STR;
    af.modifier  = -2;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    if ( ch != victim )
	send_to_char(AT_GREEN, "Ok.\n\r", ch );
    send_to_char(AT_GREEN, "You feel weaker.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}



/*
 * This is for muds that want scrolls of recall.
 */
int  spell_word_of_recall( int sn, int level, CHAR_DATA *ch, void *vo )
{
    do_recall( (CHAR_DATA *) vo, "" );
    return SKPELL_NO_DAMAGE;
}



/*
 * NPC spells.
 */
int  spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA  *obj_lose;
    OBJ_DATA  *obj_next;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam = 0;
    int        hpch;

    if ( number_percent( ) < 2 * level && !saves_spell( level, victim ) )
    {
	for ( obj_lose = victim->carrying; obj_lose; obj_lose = obj_next )
	{

	    obj_next = obj_lose->next_content;
	    if ( obj_lose->deleted )
	        continue;

	    if ( IS_SET( obj_lose->extra_flags, ITEM_NO_DAMAGE ) )
	      continue;

	    if ( number_bits( 2 ) != 0 )
		continue;

	    switch ( obj_lose->item_type )
	    {
	    case ITEM_ARMOR:
		if ( obj_lose->value[0] > 0 )
		{
		    act(AT_GREEN, "$p is pitted and etched!",
			victim, obj_lose, NULL, TO_CHAR );
		    obj_lose->cost     -= (obj_lose->cost / 20);
		    if( obj_lose->durability_cur > 10 )
			obj_lose->durability_cur -= 1;
		}
		break;

	    case ITEM_CONTAINER:
		act(AT_GREEN, "$p fumes and dissolves!",
		    victim, obj_lose, NULL, TO_CHAR );
		extract_obj( obj_lose );
		break;
	    }
	}
    }

    hpch = UMAX( 10, ch->hit );
    if(IS_NPC( ch ) )
    {
        dam = number_range( hpch / 8 + 1, hpch / 4 );
    }
    if(!IS_NPC( ch ) )
    {
        dam = number_range( ch->level * 2, ch->level * 10 );
    }
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}



int  spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA  *obj_lose;
    OBJ_DATA  *obj_next;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam=0;
    int        hpch;

    if ( number_percent( ) < 2 * level && !saves_spell( level, victim ) )
    {
	for ( obj_lose = victim->carrying; obj_lose; obj_lose = obj_next )
	{
	    char *msg;

	    obj_next = obj_lose->next_content;
	    if ( obj_lose->deleted )
	        continue;
	    if ( IS_SET( obj_lose->extra_flags, ITEM_NO_DAMAGE ) )
	      continue;
	    if ( number_bits( 2 ) != 0 )
		continue;

	    switch ( obj_lose->item_type )
	    {
	    default:             continue;
	    case ITEM_CONTAINER: msg = "$p ignites and burns!";   break;
	    case ITEM_POTION:    msg = "$p bubbles and boils!";   break;
	    case ITEM_SCROLL:    msg = "$p crackles and burns!";  break;
	    case ITEM_STAFF:     msg = "$p smokes and chars!";    break;
	    case ITEM_WAND:      msg = "$p sparks and sputters!"; break;
	    case ITEM_LENSE:     msg = "$p shrivels and dries!";  break;
	    case ITEM_FOOD:      msg = "$p blackens and crisps!"; break;
	    case ITEM_GUN:	 msg = "$p melts and drips!";	  break;
	    case ITEM_PILL:      msg = "$p melts and drips!";     break;
	    }

	    act(AT_GREEN, msg, victim, obj_lose, NULL, TO_CHAR );
	    extract_obj( obj_lose );
	}
    }

    hpch = UMAX( 10, ch->hit );
    if(IS_NPC( ch ) )
    {
        dam = number_range( hpch / 8 + 1, hpch / 4 );
    }
    if(!IS_NPC( ch ) )
    {
        dam = number_range( ch->level * 2, ch->level * 10 );
    }
    if ( saves_spell( level, victim ) )
	dam /= 2;
//    damage( ch, victim, dam, sn );
    return dam;
}



int  spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA  *obj_lose;
    OBJ_DATA  *obj_next;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam=0;
    int        hpch;

    if ( number_percent( ) < 2 * level && !saves_spell( level, victim ) )
    {
	for ( obj_lose = victim->carrying; obj_lose; obj_lose = obj_next )
	{
	    char *msg;

	    obj_next = obj_lose->next_content;
	    if ( obj_lose->deleted )
	        continue;
	    if ( IS_SET( obj_lose->extra_flags, ITEM_NO_DAMAGE ) )
	      continue;
	    if ( number_bits( 2 ) != 0 )
		continue;

	    switch ( obj_lose->item_type )
	    {
	    default:            continue;
	    case ITEM_CONTAINER:
	    case ITEM_DRINK_CON:
	    case ITEM_POTION:   msg = "$p freezes and shatters!"; break;
	    }

	    act(AT_WHITE, msg, victim, obj_lose, NULL, TO_CHAR );
	    extract_obj( obj_lose );
	}
    }

    hpch = UMAX( 10, ch->hit );
    if(IS_NPC( ch ) )
    {
        dam = number_range( hpch / 8 + 1, hpch / 4 );
    }
    if(!IS_NPC( ch ) )
    {
        dam = number_range( ch->level * 2, ch->level * 10 );
    }
    if ( saves_spell( level, victim ) )
	dam /= 2;
//    damage( ch, victim, dam, sn );
    return dam;
}



int  spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int        dam=0;
    int        hpch;

    for ( vch = ch->in_room->people; vch; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if ( vch->deleted )
	    continue;

	if ( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) )
	{
	    hpch = UMAX( 10, ch->hit );
	    if(IS_NPC( ch ) )
 	    {
        	dam = number_range( hpch / 8 + 1, hpch / 4 );
	    }
    	    if(!IS_NPC( ch ) )
	    {
        	dam = number_range( ch->level * 2, ch->level * 10 );
	    }
	    if ( saves_spell( level, vch ) )
		dam /= 2;
	    spell_poison( skill_lookup("poison"), level, ch, vch );
	    damage( ch, vch, dam, sn );
	}
    }
    return SKPELL_NO_DAMAGE;
}



int  spell_lightning_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam=0;
    int        hpch;

    hpch = UMAX( 10, ch->hit );

    if(IS_NPC( ch ) )
    {
        dam = number_range( hpch / 8 + 1, hpch / 4 );
    }
    if(!IS_NPC( ch ) )
    {
	dam = number_range( ch->level * 2, ch->level * 10 );
    }

    if ( saves_spell( level, victim ) )
	dam /= 2;
//    damage( ch, victim, dam, sn );
    return dam;
}

/*
 * Code for Psionicist spells/skills by Thelonius
 */
int  spell_adrenaline_control ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
        return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level - 5;
    af.location	 = APPLY_DEX;
    af.modifier	 = 2;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location	= APPLY_CON;
    affect_to_char( victim, &af );

    send_to_char(AT_BLUE, "You have given yourself an adrenaline rush!\n\r", ch );
    act(AT_BLUE, "$n has given $mself an adrenaline rush!", ch, NULL, NULL,
	TO_ROOM );
   
    return SKPELL_NO_DAMAGE;
}



int  spell_agitation ( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
	0,
	 0,  0,  0,  0,  0,      12, 15, 18, 21, 24,
	24, 24, 25, 25, 26,      26, 26, 27, 27, 27,
	28, 28, 28, 29, 29,      29, 30, 30, 30, 31,
	31, 31, 32, 32, 32,      33, 33, 33, 34, 34,
	34, 35, 35, 35, 36,      36, 36, 37, 37, 37,
	38, 39, 40, 41, 42,      43, 44, 45, 46, 47,
	48, 49, 50, 51, 52,      53, 54, 55, 56, 57,
	58, 59, 60, 61, 62,      63, 64, 65, 66, 67,
	68, 69, 70, 71, 72,      73, 74, 75, 76, 77,
	78, 79, 80, 81, 82,      83, 84, 85, 86, 87
    };
		 int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 2 );
   
    if ( saves_spell( level, victim ) )
      dam /= 2;
//    damage( ch, victim, dam, sn );
    return dam;
}



int  spell_aura_sight ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char      *msg;
    int        ap;
   
    ap = victim->alignment;

    if ( ap >  700 ) msg = "$N has an aura as white as the driven snow.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "Don't bring $N home to meet your family.";
    else msg = "Uh, check please!";

    act(AT_BLUE, msg, ch, NULL, victim, TO_CHAR );

	return SKPELL_NO_DAMAGE;
}



int  spell_awe ( int sn, int level, CHAR_DATA *ch, void *vo )
  {
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( victim->fighting == ch && !saves_spell( level, victim ) )
    {
	stop_fighting ( victim, TRUE);
	act(AT_BLUE, "$N is in AWE of you!", ch, NULL, victim, TO_CHAR    );
	act(AT_BLUE, "You are in AWE of $n!",ch, NULL, victim, TO_VICT    );
	act(AT_BLUE, "$N is in AWE of $n!",  ch, NULL, victim, TO_NOTVICT );
    }
    return SKPELL_NO_DAMAGE;
}



int  spell_ballistic_attack ( int sn, int level, CHAR_DATA *ch, void *vo )
  {
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
	 0,
	 3,  4,  4,  5,  6,       6,  6,  7,  7,  7,
	 7,  7,  8,  8,  8,       9,  9,  9, 10, 10,
	10, 11, 11, 11, 12,      12, 12, 13, 13, 13,
	14, 14, 14, 15, 15,      15, 16, 16, 16, 17,
	17, 17, 18, 18, 18,      19, 19, 19, 20, 20,
	21, 22, 23, 24, 25,      26, 27, 28, 29, 30,
	31, 32, 33, 34, 35,      36, 37, 38, 39, 40,
	41, 42, 43, 44, 45,      46, 47, 48, 49, 50,
	51, 52, 53, 54, 55,      56, 57, 58, 59, 60,
	61, 62, 63, 64, 65,      66, 67, 68, 69, 70
    };
		 int        dam;
	
    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim ) )
      dam /= 2;
    act(AT_BLUE, "You chuckle as a stone strikes $N.", ch, NULL, victim,
	TO_CHAR );
    //damage( ch, victim, dam, sn);
    return dam;
}



int  spell_biofeedback ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
   
    if ( IS_AFFECTED4( victim, AFF_BIOFEEDBACK ) )
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
            
    if ( IS_AFFECTED2( victim, AFF_GOLDEN_ARMOR ) || IS_AFFECTED4( victim, AFF_GOLDEN_SANCTUARY ) || IS_AFFECTED( victim, AFF_SANCTUARY ) || IS_AFFECTED4( victim, AFF_ESSENCE_OF_GAIA) )
        return SKPELL_MISSED;
         
    if ( !IS_SHIELDABLE( victim ) )
        return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = -1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_BIOFEEDBACK;
    affect_to_char4( victim, &af );
    victim->shields += 1;

    send_to_char(AT_WHITE, "You are surrounded by an electromagnetic aura.\n\r", victim );
    act(AT_WHITE, "$n is surrounded by an electromagnetic aura.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;

}

int  spell_cell_adjustment ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
   
    if ( is_affected( victim, skill_lookup("poison") ) )
    {
	victim->poison_level -= number_fuzzy(ch->level/10);
	if( victim->poison_level <= 0 )
	{
	    victim->poison_level = 0;
	    affect_strip( victim, skill_lookup("poison") );
	}
	send_to_char(AT_BLUE, "A warm feeling runs through your body.\n\r", victim );
	act(AT_BLUE, "$N looks better.", ch, NULL, victim, TO_NOTVICT );
    }
    if ( is_affected( victim, skill_lookup("curse")  ) )
    {
	affect_strip( victim, skill_lookup("curse")  );
	send_to_char(AT_BLUE, "You feel better.\n\r", victim );
    }	
    send_to_char(AT_BLUE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

int  spell_chaosfield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_CHAOS ) )
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
    af.bitvector = AFF_CHAOS;
    affect_to_char( victim, &af );
    victim->shields += 1;

    send_to_char(AT_YELLOW, "You call forth an instance of chaos from the order around you.\n\r", victim );
    act(AT_YELLOW, "$n's body is veiled in an instance or pure chaos.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_bladebarrier( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_BLADE ) )
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
    af.bitvector = AFF_BLADE;
    affect_to_char2( victim, &af );
    victim->shields += 1;

    send_to_char(AT_GREY, "You bring forth thousands of tiny spinning blades about your body.\n\r", victim );
    act(AT_GREY, "$n's body is surrounded by thousands of spinning blades.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_dancing_lights( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_DANCING ) || IS_AFFECTED(victim,AFF_BLIND))
	return SKPELL_MISSED;
    
    if ( is_affected( victim, sn ) || saves_spell( level, victim ))
    {
	send_to_char(AT_BLUE, "You have failed.\n\r", ch );
	return SKPELL_MISSED;
    }

    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 6 );
    af.location  = APPLY_DAM_PHYSICAL;
    af.modifier  = -level/3;
    af.bitvector = AFF_DANCING;
    affect_to_char2( victim, &af );

    send_to_char(AT_YELLOW, "You are surrounded by dancing lights!\n\r", 
victim);

    act(AT_WHITE, "&.Thou&.sand&.s &.of &.danci&.ng &.ligh&.ts &.surr&.ound &.you&.!&w", victim, NULL, victim, TO_VICT );
    act(AT_GREY, "&W$n's &.body &.is &.surr&.ounded &.by d&.anci&.ng l&.ights.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}


int  spell_combat_mind ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if ( victim == ch )
	  send_to_char(AT_BLUE, "You already understand battle tactics.\n\r",
		       victim );
	else
	  act(AT_BLUE, "$N already understands battle tactics.",
	      ch, NULL, victim, TO_CHAR );
	return SKPELL_MISSED;
    }

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level + 3;
    af.location	 = APPLY_HITROLL;
    af.modifier	 = level / 5;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location	 = APPLY_AC;
    af.modifier	 = - level/2 - 10;
    affect_to_char( victim, &af );

    if ( victim != ch )
        send_to_char(AT_BLUE, "OK.\n\r", ch );
    send_to_char(AT_BLUE, "You gain a keen understanding of battle tactics.\n\r",
		 victim );
    return SKPELL_NO_DAMAGE;
}


/* psi people shouldn't have this powerful healing */

int  spell_complete_healing ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = (int)(victim->max_hit * 0.24);
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    if ( ch != victim )
        send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "Ahhhhhh...You feel MUCH better!\n\r", victim );
    send_to_char(AT_BLUE, "Have a nice day.\n\r", victim);
    return SKPELL_NO_DAMAGE;
}

int  spell_control_flames ( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] = 
    {
	 0,
	 0,  0,  0,  0,  0,       0,  0, 16, 20, 24,
	28, 32, 35, 38, 40,      42, 44, 45, 45, 45,
	46, 46, 46, 47, 47,      47, 48, 48, 48, 49,
	49, 49, 50, 50, 50,      51, 51, 51, 52, 52,
	52, 53, 53, 53, 54,      54, 54, 55, 55, 55,
	56, 56, 57, 57, 58,      58, 59, 59, 60, 60,
	62, 63, 63, 63, 64,      64, 64, 65, 65, 65,
	72, 73, 73, 73, 74,      74, 74, 75, 75, 75,
	82, 83, 83, 83, 84,      84, 84, 85, 85, 85,
	92, 93, 93, 93, 94,      94, 94, 95, 95, 95
    };
		 int        dam;

    if ( !get_eq_char( ch, WEAR_LIGHT ) )
    {
	send_to_char(AT_RED, "You must be carrying a light source.\n\r", ch );
	return SKPELL_MISSED;
    }

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}



int  spell_create_sound ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    char       buf1    [ MAX_STRING_LENGTH ];
    char       buf2    [ MAX_STRING_LENGTH ];
    char       speaker [ MAX_INPUT_LENGTH  ];

    target_name = one_argument( target_name, speaker );

    sprintf( buf1, "%s says '%s'.\n\r", speaker, target_name );
    sprintf( buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name );
    buf1[0] = UPPER( buf1[0] );

    for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
    {
	if ( !is_name( speaker, vch->name ) )
	    send_to_char(AT_RED, saves_spell( level, vch ) ? buf2 : buf1, vch );
    }
    return SKPELL_NO_DAMAGE;
}



int  spell_death_field ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int        dam = 10;
    int        hpch;

     /* return; */

    /* Temporarily disabling Death Field spell */

    if ( !IS_EVIL( ch ) )
    {
	send_to_char(AT_RED, "You are not evil enough to do that!\n\r", ch);
	return SKPELL_MISSED;
    }

    send_to_char(AT_DGREY, "A black haze emanates from you!\n\r", ch );
    act (AT_DGREY, "A black haze emanates from $n!", ch, NULL, ch, TO_ROOM );
    for ( vch = ch->in_room->people; vch; vch = vch_next )
    {
      vch_next = vch->next_in_room;
	if ( vch->deleted )
	  continue;
      hpch = URANGE( 10, ch->hit, 99 );

	if ( ch != vch && IS_NPC(vch) )
	{ 

        if ( level - vch->level > 10)
        {
	    if ( !saves_spell( level, vch ) )
            {
		  dam = 10; /* Enough to compensate for shields. */
		  vch->hit = 1;
		  update_pos( vch );
		  send_to_char(AT_DGREY, "The haze envelops you!\n\r", vch );
		  act(AT_DGREY, "The haze envelops $N!", ch, NULL, vch, TO_ROOM );
            }
          else
	      dam = number_range( hpch / 16 + 1, hpch / 8 );
        }
        else
        {
          if ( !saves_spell( level, vch )
             && ( URANGE ( 0, ( 100 - (level - vch->level) * 2), 90) <number_percent()))
          {
	      dam = 10; /* Enough to compensate for shields. */
		vch->hit = 1;
		update_pos( vch );
		send_to_char(AT_DGREY, "The haze envelops you!\n\r", vch );
		act(AT_DGREY, "The haze envelops $N!", ch, NULL, vch, TO_ROOM );
          }            
          else
           dam = number_range( hpch / 16 + 1, hpch / 8 );
        } 
      }      
      if ( !IS_NPC(vch) && vch != ch)
      {
        dam = number_range( hpch / 16 + 1, hpch / 8 );
      }
      if (vch != ch)
       damage( ch, vch, dam, sn );

    }
    return SKPELL_NO_DAMAGE;
}



int  spell_detonate ( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
		 int        dam;

    dam	     = dice( level, 13 );

  if (!IS_NPC( victim ) )
  {
        dam = UMAX(  20, victim->hit - dice( 1,4 ) );
        dam = UMIN( 175, dam);
        //damage( ch, victim, dam, sn );
        return dam;
  }

    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}



int  spell_disintegrate ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA  *obj_lose;
    OBJ_DATA  *obj_next;

    if ( !IS_NPC(victim) )
    {
      send_to_char(AT_BLUE, "You failed.\n\r", ch);
      return SKPELL_MISSED;
    }

    if ( IS_AFFECTED4( victim, AFF_IMMORTAL ) )
    {
	send_to_char(AT_BLUE, "You failed.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( number_percent( ) < 2 * level && !saves_spell( level, victim ) )
      for ( obj_lose = victim->carrying; obj_lose; obj_lose = obj_next )
      {
	  obj_next = obj_lose->next_content;
	  if ( obj_lose->deleted )
	      continue;

	  if ( number_bits( 2 ) != 0 )
	      continue;

	  act(AT_WHITE, "$p disintegrates!",      victim, obj_lose, NULL, TO_CHAR );
	  act(AT_WHITE, "$n's $p disintegrates!", victim, obj_lose, NULL, TO_ROOM );
	  extract_obj( obj_lose ) ;
      }

    if ( !saves_spell( level, victim ) )
    /*
     * Disintegrate char, do not generate a corpse, do not
     * give experience for kill.  Extract_char will take care
     * of items carried/wielded by victim.  Needless to say,
     * it would be bad to be a target of this spell!
     * --- Thelonius (Monk)
     */
    {
	act(AT_WHITE, "You have DISINTEGRATED $N!",         ch, NULL, victim, TO_CHAR );
	act(AT_WHITE, "You have been DISINTEGRATED by $n!", ch, NULL, victim, TO_VICT );
	act(AT_WHITE, "$n's spell DISINTEGRATES $N!",       ch, NULL, victim, TO_ROOM );
	
	if ( IS_NPC( victim ) )
	    extract_char( victim, TRUE );
	else
	    extract_char( victim, FALSE );
    }
    return SKPELL_NO_DAMAGE;
}

int  spell_disrupt( int sn, int level, CHAR_DATA *ch, void *vo )
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
    dam      = number_range( dam_each[level] * 5, dam_each[level] * 6 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}   

int  spell_displacement ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
	return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level / 2;
    af.location	 = APPLY_AC;
    af.modifier	 = (level / 2) * -1;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char(AT_GREY, "Your form shimmers, and you appear displaced.\n\r",
		 victim );
    act(AT_GREY, "$N shimmers and appears in a different location.",
	ch, NULL, victim, TO_NOTVICT );
    return SKPELL_NO_DAMAGE;
}



int  spell_domination ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
	send_to_char(C_DEFAULT, "Dominate yourself?  You're weird.\n\r", ch );
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

    act(AT_BLUE, "Your will dominates $N!", ch, NULL, victim, TO_CHAR );
    act(AT_BLUE, "Your will is dominated by $n!", ch, NULL, victim, TO_VICT );
    return SKPELL_NO_DAMAGE;
}



int  spell_ectoplasmic_form ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_PASS_DOOR ) )
        return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = number_fuzzy( level / 4 );
    af.location	 = APPLY_NONE;
    af.modifier	 = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );

    send_to_char(AT_GREY, "You turn translucent.\n\r", victim );
    act(AT_GREY, "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}



int  spell_ego_whip ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || saves_spell( level, victim ) )
        return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level;
    af.location	 = APPLY_HITROLL;
    af.modifier	 = (ch->level / 10) * -1;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location	 = APPLY_SAVING_SPELL;
    af.modifier	 = (ch->level / 5);
    affect_to_char( victim, &af );

    af.location	 = APPLY_AC;
    af.modifier	 = level / 2;
    affect_to_char( victim, &af );

    act(AT_BLUE, "You ridicule $N about $S childhood.", ch, NULL, victim, TO_CHAR    );
    send_to_char(AT_BLUE, "Your ego takes a beating.\n\r", victim );
    act(AT_BLUE, "$N's ego is crushed by $n!",          ch, NULL, victim, TO_NOTVICT );

    return SKPELL_NO_DAMAGE;
}



int  spell_energy_containment ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
        return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level / 2 + 7;
    af.modifier	 = -level / 3;
    af.location  = APPLY_SAVING_SPELL;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char(AT_BLUE, "You can now absorb some forms of energy.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}



int  spell_enhance_armor (int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;

    if ( obj->item_type != ITEM_ARMOR
	|| IS_OBJ_STAT( obj, ITEM_MAGIC )
	|| obj->affected )
    {
	send_to_char(AT_BLUE, "That item cannot be enhanced.\n\r", ch );
	return SKPELL_MISSED;
    }

	paf	    = new_affect();

    paf->type	   = sn;
    paf->duration  = -1;
    paf->location  = APPLY_AC;
    paf->bitvector = 0;
    paf->next	   = obj->affected;
    obj->affected  = paf;

    if ( number_percent() < ( ch->pcdata->learned[sn] / 10 ) /2
	+ 3 * ( ch->level - obj->level ) )

    {
	paf->modifier   = -5;

	     if ( IS_GOOD( ch ) )
	{
	    SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	    act(AT_BLUE, "$p glows.",   ch, obj, NULL, TO_CHAR );
	}
	else if ( IS_EVIL( ch ) )
        {
	    SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	    act(AT_RED, "$p glows.",    ch, obj, NULL, TO_CHAR );
	}
	else
	{
	    SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	    SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	    act(AT_YELLOW, "$p glows.", ch, obj, NULL, TO_CHAR );
	}
       
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    }
    else
    /* Bad Enhancement ... opps! :) */
    {
	paf->modifier   = 10;
	obj->cost       = 0;

	SET_BIT( obj->extra_flags, ITEM_NODROP );
	act(AT_DGREY, "$p turns black.", ch, obj, NULL, TO_CHAR );
    }

    return SKPELL_NO_DAMAGE;
}



int  spell_enhanced_strength ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
        return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level;
    af.location	 = APPLY_STR;
    af.modifier	 = 1 + ( level >= 15 ) + ( level >= 25 );
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char(AT_BLUE, "You are HUGE!\n\r", victim );
    return SKPELL_NO_DAMAGE;
}



int  spell_flesh_armor ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
        return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level;
    af.location	 = APPLY_AC;
    af.modifier	 = -45;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char(AT_BLUE, "Your flesh turns to steel.\n\r", victim );
    act(AT_BLUE, "$N's flesh turns to steel.", ch, NULL, victim, TO_NOTVICT);
    return SKPELL_NO_DAMAGE;
}



int  spell_inertial_barrier ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *gch;
    AFFECT_DATA af;

    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_AFFECTED( gch, AFF_PROTECT ) )
	    continue;

	act(AT_BLUE, "An inertial barrier forms around $n.", gch, NULL, NULL,
	    TO_ROOM );
	send_to_char(AT_BLUE, "An inertial barrier forms around you.\n\r", gch );

	af.type	     = sn;
        af.level     = level;
	af.duration  = 24;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_PROTECT;
	affect_to_char( gch, &af );
    }
    return SKPELL_NO_DAMAGE;
}



int  spell_inflict_pain ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    //damage( ch, (CHAR_DATA *) vo, dice( 2, 10 ) + level / 2, sn );
    return (dice( 2, 10 ) + (level / 2) );
}



int  spell_intellect_fortress ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *gch;
    AFFECT_DATA af;

    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || is_affected( gch, sn ) )
	    continue;

	send_to_char(AT_BLUE, "A virtual fortress forms around you.\n\r", gch );
	act(AT_BLUE, "A virtual fortress forms around $N.", gch, NULL, gch, TO_ROOM );

	af.type	     = sn;
        af.level     = level;
	af.duration  = 24;
	af.location  = APPLY_AC;
	af.modifier  = -40;
	af.bitvector = 0;
	affect_to_char( gch, &af );
    }
    return SKPELL_NO_DAMAGE;
}



int  spell_lend_health ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        hpch;

    if ( ch == victim )
    {
	send_to_char(AT_BLUE, "Lend health to yourself?  Easily done.\n\r", ch );
	return SKPELL_MISSED;
    }
    hpch = UMIN( 50, victim->max_hit - victim->hit );
    if ( hpch == 0 )
    {
	act(AT_BLUE, "Nice thought, but $N doesn't need healing.", ch, NULL,
	    victim, TO_CHAR );
	return SKPELL_MISSED;
    }
    if ( ch->hit-hpch < 50 )
    {
	send_to_char(AT_BLUE, "You aren't healthy enough yourself!\n\r", ch );
	return SKPELL_MISSED;
    }
    victim->hit += hpch;
    ch->hit     -= hpch;
    update_pos( victim );
    update_pos( ch );

    act(AT_BLUE, "You lend some of your health to $N.", ch, NULL, victim, TO_CHAR );
    act(AT_BLUE, "$n lends you some of $s health.",     ch, NULL, victim, TO_VICT );

    return SKPELL_NO_DAMAGE;
}



int  spell_levitation ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_FLYING ) )
        return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level + 3;
    af.location	 = APPLY_NONE;
    af.modifier	 = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );

    send_to_char(AT_BLUE, "Your feet rise off the ground.\n\r", victim );
    act(AT_BLUE, "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_spectral_wings ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_FLYING ) )
        return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level + 3;
    af.location	 = APPLY_NONE;
    af.modifier	 = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );

    send_to_char(AT_BLUE, "You sprout wings and your feet rise off of the ground.\n\r", victim );
    act(AT_BLUE, "Wings emerge from $n's back and their feet rise off of the ground.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}



int  spell_mental_barrier ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
        return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = 24;
    af.location	 = APPLY_AC;
    af.modifier	 = -20;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char(AT_BLUE, "You erect a mental barrier around yourself.\n\r",
		 victim );
    return SKPELL_NO_DAMAGE;
}



int  spell_mind_thrust ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    //damage( ch, (CHAR_DATA *) vo, dice( 1, 10 ) + level / 2, sn );
    return (dice(1,10)+(level/2));
}



int  spell_project_force ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    //damage( ch, (CHAR_DATA *) vo, dice( 4, 6 ) + level, sn );
    return (dice(4,6) + level);
}



int  spell_psionic_blast ( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
	  0,
	  0,   0,   0,   0,   0,        0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,        0,  45,  50,  55,  60,
	 64,  68,  72,  76,  80,       82,  84,  86,  88,  90,
	 92,  94,  96,  98, 100,      102, 104, 106, 108, 100,
	112, 114, 116, 118, 120,      122, 124, 126, 128, 130,
	132, 134, 136, 138, 140,      142, 144, 146, 148, 150,
	152, 154, 156, 158, 160,      162, 164, 166, 168, 170,
	182, 184, 186, 188, 190,      192, 194, 196, 198, 200,
	202, 204, 206, 208, 210,      212, 214, 216, 218, 220,
	222, 224, 226, 228, 230,      232, 234, 236, 238, 240
    };
		 int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}



int  spell_psychic_crush ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    //damage( ch, (CHAR_DATA *) vo, dice( 3, 5 ) + level, sn );
    return (dice(3,5)+level);
}



int  spell_psychic_drain ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || saves_spell( level, victim ) )
        return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level / 2;
    af.location	 = APPLY_STR;
    af.modifier	 = -1 - ( level >= 10 ) - ( level >= 20 ) - ( level >= 30 );
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char(AT_GREEN, "You feel drained.\n\r", victim );
    act(AT_BLUE, "$n appears drained of strength.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}



int  spell_psychic_healing ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = (int)(victim->max_hit * 0.08);
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );

    send_to_char(AT_BLUE, "You feel better!\n\r", victim );
    return SKPELL_NO_DAMAGE;
}



int  spell_share_strength ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
	send_to_char(AT_BLUE, "You can't share strength with yourself.\n\r", ch );
	return SKPELL_MISSED;
    }
    if ( is_affected( victim, sn ) )
    {
	act(AT_BLUE, "$N already shares someone's strength.", ch, NULL, victim,
	    TO_CHAR );
	return SKPELL_MISSED;
    }
    if ( get_curr_str( ch ) <= 5 )
    {
	send_to_char(AT_BLUE, "You are too weak to share your strength.\n\r", ch );
	return SKPELL_MISSED;
    }

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level;
    af.location	 = APPLY_STR;
    af.modifier	 =  1 + ( level >= 20 ) + ( level >= 30 );
    af.bitvector = 0;
    affect_to_char( victim, &af );
    
    af.modifier	 = -1 - ( level >= 20 ) - ( level >= 30 );
    affect_to_char( ch,     &af );

    act(AT_BLUE, "You share your strength with $N.", ch, NULL, victim, TO_CHAR );
    act(AT_BLUE, "$n shares $s strength with you.",  ch, NULL, victim, TO_VICT );
    return SKPELL_NO_DAMAGE;
}



int  spell_thought_shield ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
        return SKPELL_MISSED;

    af.type	 = sn;
    af.level	 = level;
    af.duration	 = level;
    af.location	 = APPLY_AC;
    af.modifier	 = -20;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    send_to_char(AT_BLUE, "You have created a shield around yourself.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

int  spell_ultrablast ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int        dam;
    int        hpch;

    for ( vch = ch->in_room->people; vch; vch = vch_next )
    {
        vch_next = vch->next_in_room;
	if ( vch->deleted )
	    continue;

	if ( ch != vch && ( IS_NPC( ch ) ? !IS_NPC( vch ) : 1 ) )
	{
	    hpch = UMAX( 10, ch->hit );
	    dam  = number_range( hpch / 8, hpch / 3 );
	    if ( saves_spell( level, vch ) )
	        dam /= 2;
	    damage( ch, vch, dam, sn );
	}
    }
    return SKPELL_NO_DAMAGE;
}

/* XORPHOX summon mobs */
int  spell_summon_swarm(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_BLUE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_INSECTS));
  mob->level = URANGE(15, level, 55) - 5;
  mob->max_hit = mob->level * 20 + dice(1,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/5;
  ch->summon_timer = 10;
  char_to_room(mob, ch->in_room);
  act(AT_BLUE, "You summon $N.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 && ((ch->class != 9) || (ch->class != 11 )))
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
  act(AT_GREEN, "$n summons $N.", ch, NULL, mob, TO_ROOM);

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
    act(AT_BLUE, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_BLUE, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    fch->fighting = mob;
    mob->fighting = fch;
  }
  return SKPELL_NO_DAMAGE;
}

int  spell_summon_pack(int sn, int level, CHAR_DATA *ch, void *vo)
{
  CHAR_DATA *mob;
  CHAR_DATA *fch;
  AFFECT_DATA af;

  if(ch->summon_timer > 0)
  {
    send_to_char(AT_BLUE,
     "You cast the spell, but nothing appears.\n\r", ch);
    return SKPELL_MISSED;
  }

  mob = create_mobile(get_mob_index(MOB_VNUM_WOLFS));
  mob->level = URANGE(31, level, 90) - 5;
  mob->max_hit = mob->level * 20 + dice(1,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/8;
  ch->summon_timer = 15;
  char_to_room(mob, ch->in_room);
  act(AT_GREEN, "You summon $N.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 && ((ch->class != 9) || (ch->class != 11 )))
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
  act(AT_GREEN, "$N comes to $n aid.", ch, NULL, mob, TO_ROOM);

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
    act(AT_BLUE, "$n rescues you!", mob, NULL, ch, TO_VICT    );
    act(AT_BLUE, "$n rescues $N!",  mob, NULL, ch, TO_NOTVICT );

    fch = ch->fighting;
    stop_fighting(fch, FALSE );
    stop_fighting( ch, FALSE );
    set_fighting(mob, fch);
    set_fighting(fch, mob);
  }
  return SKPELL_NO_DAMAGE;
}

int  spell_summon_demon(int sn, int level, CHAR_DATA *ch, void *vo)
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

  mob = create_mobile(get_mob_index(MOB_VNUM_DEMON));
  mob->level = URANGE(51, level, 100) - 5;
  mob->max_hit = mob->level * 20 + dice(1,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_RED, "You summon $N from the abyss.", ch, NULL, mob, TO_CHAR);
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
  act(AT_RED, "$n summons $N from the abyss.", ch, NULL, mob, TO_ROOM);

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

int  spell_summon_angel(int sn, int level, CHAR_DATA *ch, void *vo)
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

  mob = create_mobile(get_mob_index(MOB_VNUM_ANGEL));
  mob->level = URANGE(51, level, 100) - 5;
  mob->max_hit = mob->level * 20 + dice(10,mob->level);
  mob->hit = mob->max_hit;
  mob->summon_timer = level/10;
  ch->summon_timer = 16;
  char_to_room(mob, ch->in_room);
  act(AT_WHITE, "You summon $N from heaven.", ch, NULL, mob, TO_CHAR);
  if(ch->mana < level * 2 )
  {
    act(AT_WHITE,
     "You don't have enough mana to bind $N!", ch, NULL, mob, TO_CHAR);
    extract_char(mob, TRUE);
    return SKPELL_MISSED;
  }
  ch->mana -= level * 2;
  act(AT_WHITE, "$n calls forth $N from Heaven.", ch, NULL, mob, TO_ROOM);

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
