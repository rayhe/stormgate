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

/*$Id: act_obj2.c,v 1.25 2005/04/11 03:24:38 tyrion Exp $*/

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

extern char* target_name;

int skill_depoison_weapon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj;

    if ( target_name[0] == '\0' )                                              
    { send_to_char(AT_DGREEN, "What are you trying to poison?\n\r",    ch ); return SKPELL_BOTCHED; }
    if ( ch->fighting )                                       
    { send_to_char(AT_DGREEN, "While you're fighting?  I think not.\n\r", ch ); return SKPELL_BOTCHED; }
    if ( !( obj = get_obj_carry( ch, target_name ) ) )
    { send_to_char(AT_DGREEN, "You do not have that item.\n\r",      ch ); return SKPELL_BOTCHED; }
    if ( !IS_OBJ_STAT( obj, ITEM_POISONED ) )
    { send_to_char(AT_DGREEN, "That item is not poisoned.\n\r",  ch ); return SKPELL_BOTCHED; }

    /* Now we have a valid weapon...check to see if we have the powder. */

    /* Great, we have the ingredients...but is the thief smart enough? */
    if ( !IS_NPC( ch ) && get_curr_wis( ch ) < 19 )
    {
	send_to_char(AT_DGREEN, "You can't quite remember what to do...\n\r", ch );
	return SKPELL_BOTCHED;
    }
    /* And does the thief have steady enough hands? */
    if ( !IS_NPC( ch )
	&& ( get_curr_dex( ch ) < 20
	    || ch->pcdata->condition[COND_DRUNK] > 0 ) )
    {
	send_to_char(AT_DGREEN,
	"Your hands aren't steady enough to properly remove the poison.\n\r",
								ch );
	return SKPELL_BOTCHED;
    }

    WAIT_STATE( ch, skill_table[sn].beats );

  
    /* Well, I'm tired of waiting.  Are you? */
    act(AT_GREEN, "You remove the deadly poison!",
	ch, NULL, NULL, TO_CHAR );
    act(AT_GREEN, "$n removes the deadly poison!",
	ch, NULL, NULL, TO_ROOM );
    act(AT_GREEN, "You strip the poison from $p, which loses its wicked luster!",
	ch, obj, NULL, TO_CHAR  );
    act(AT_GREEN, "$n strips the poison from $p, which loses its wicked luster!",
	ch, obj, NULL, TO_ROOM  );
    REMOVE_BIT( obj->extra_flags, ITEM_POISONED );


    /* WHAT?  All of that, just for that one bit?  How lame. ;) */
    return SKPELL_NO_DAMAGE;
}

void do_acmorph ( CHAR_DATA *ch, OBJ_DATA *obj, int  vnum )
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA       *nObj;
    int             level;
    
    level = 0;
    act( AT_BLUE, "You invoke $p.", ch, obj, NULL, TO_CHAR );
    act( AT_BLUE, "$n invokes $p.", ch, obj, NULL, TO_ROOM ); 
 
    if ( !(pObjIndex = get_obj_index( vnum ) ) )
       {
         act( AT_BLUE, "$p whines and sparks, but nothing happens", ch, obj, NULL, TO_CHAR );
         return;
       }
    level = pObjIndex->level;
    nObj = create_object( pObjIndex, level );
    if ( CAN_WEAR( nObj, ITEM_TAKE ) )
    {
	obj_to_char( nObj, ch );
    }
    else
    {
	obj_to_room( nObj, ch->in_room );
    }

    act(AT_BLUE, "$p's form wavers, then solidifies as $P.", ch, obj, nObj, TO_CHAR );
    act(AT_BLUE, "$n's $p wavers in form. then solidifies as $P.", ch, obj, nObj, TO_ROOM );
    oprog_invoke_trigger( obj, ch, nObj );
    extract_obj( obj );
    return;    
}        
    
void do_acoload( CHAR_DATA *ch, OBJ_DATA *obj, int  vnum )
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA       *nObj;
    int             level;
    
    level = 0;
    act( AT_BLUE, "You invoke $p.", ch, obj, NULL, TO_CHAR );
    act( AT_BLUE, "$n invokes $p.", ch, obj, NULL, TO_ROOM ); 
 
    if ( !(pObjIndex = get_obj_index( vnum ) ) )
       {
         act( AT_BLUE, "$p whines and sparks, but nothing happens", ch, obj, NULL, TO_CHAR );
         return;
       }
    level = pObjIndex->level;
    nObj = create_object( pObjIndex, level );
    if ( CAN_WEAR( nObj, ITEM_TAKE ) )
    {
	obj_to_char( nObj, ch );
    }
    else
    {
	obj_to_room( nObj, ch->in_room );
    }
    act(AT_BLUE, "$p spawns $P.", ch, obj, nObj, TO_CHAR );
    act(AT_BLUE, "$n's $p spawns $P.", ch, obj, nObj, TO_ROOM );
    oprog_invoke_trigger( obj, ch, nObj );

    return;    
}        

void do_acmload( CHAR_DATA *ch, OBJ_DATA *obj, int vnum )
{
    CHAR_DATA      *victim;
    MOB_INDEX_DATA *pMobIndex;
    AFFECT_DATA af;
    
    act( AT_BLUE, "You invoke $p.", ch, obj, NULL, TO_CHAR );
    act( AT_BLUE, "$n invokes $p.", ch, obj, NULL, TO_ROOM ); 

    if ( !( pMobIndex = get_mob_index( vnum ) ) )
    {
         act( AT_BLUE, "$p whines and sparks, but nothing happens", ch, obj, NULL, TO_CHAR );
         return;
    }
    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    
    act(AT_BLUE, "$p spawns $N.", ch, obj, victim, TO_CHAR );
    act(AT_BLUE, "$n's $p spawns $N.", ch, obj, victim, TO_ROOM );
    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    af.type      = skill_lookup( "charm person" );
    af.duration  = 50;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    oprog_invoke_trigger( obj, ch, victim );
    
    return;
}

void do_actrans( CHAR_DATA *ch, OBJ_DATA *obj, int vnum )
{
    ROOM_INDEX_DATA *location;

    act( AT_BLUE, "You invoke $p.", ch, obj, NULL, TO_CHAR );
    act( AT_BLUE, "$n invokes $p.", ch, obj, NULL, TO_ROOM ); 

    if ( !( location = get_room_index( vnum ) ) )
    {
	act(AT_BLUE, "$p whines and sparks, but nothing happens.", ch, obj, NULL, TO_CHAR );
	return;
    }

    if ( room_is_private( location ) )
    {
	send_to_char(AT_BLUE, "That room is private right now.\n\r", ch );
	return;
    }

    if ( ch->fighting )
    {
        act( AT_BLUE, "$p pulses lightly, but fails to function.", ch, obj, NULL, TO_CHAR );
        return;
    }

    if ( IS_SET(location->room_flags, ROOM_SAFE) && ch->pkill && (ch->combat_timer>0))
    {
	send_to_char(AT_BLUE, "Your blood runs to hot to go into that room!\n\r", ch);
        return;
    } 

    act(AT_BLUE, "Everything begins to spin, when it clears you are elsewhere.", ch, obj, NULL, TO_CHAR );
    act(AT_BLUE, "$n invokes $p and vanishes in a swirling red mist.", ch, obj, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act(AT_BLUE, "$n arrives in a swirling red mist.", ch, obj, NULL, TO_ROOM);
    do_look( ch, "auto" );
    oprog_invoke_trigger( obj, ch, ch );
    return;
}

void do_invoke( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA       *obj;
    CHAR_DATA      *rch;
    CHAR_DATA      *victim;
    char            arg1 [ MAX_INPUT_LENGTH ];
    char            arg2 [ MAX_INPUT_LENGTH ];
    char            spellarg [ MAX_INPUT_LENGTH ];
    
    if ( IS_NPC(ch) )
      return;

    rch = get_char( ch );
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
 
    if ( !(obj = get_obj_carry( ch, arg1 ) ) && ( !(obj = get_obj_wear( ch, arg1 ) ) ) )
    {
	send_to_char( AT_WHITE, "You can't find it.\n\r", ch );
	return;
    }
    
    if ( (ch->level < obj->level) && (ch->multied == ch->class) )
    {
	send_to_char(AT_BLUE, "You have not attained the level of mastery to use this item", ch );
	act(AT_BLUE, "$n tries to use $p, but is too inexperienced.",
	    ch, obj, NULL, TO_ROOM );
	return;
    }
    
    if ( obj->ac_type <= 0 || obj->ac_type >= 6 )
    {
        act( AT_WHITE, "$p cannot be invoked.", ch, obj, NULL, TO_CHAR );
        return;
    }
    if ( obj->ac_type == 5 && !obj->ac_spell )
    {
      sprintf( log_buf, "Obj[%d] AcType Spell with no Spellname",
         obj->pIndexData->vnum );
      bug( log_buf, 0 );
      act( AT_WHITE, "$p cannot be invoked.", ch, obj, NULL, TO_CHAR );
      return;
    }
    
    if ( arg2[0] == '\0' )
       victim = rch;
    else
       if ( !(victim = get_char_world( ch, arg2 ) ) )
          {
           send_to_char( AT_WHITE, "There is no such person in existance.\n\r", ch );
           return;
          }

    if ( !IS_NPC( ch ) ) {
    if (   ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL     ) && IS_EVIL   ( ch ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_GOOD     ) && IS_GOOD   ( ch ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL  ) && IS_NEUTRAL( ch ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_MAGE     ) && ( ch->class == CLASS_MAGE || ch->multied == CLASS_MAGE ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_CLERIC   ) && ( ch->class == CLASS_CLERIC || ch->multied == CLASS_CLERIC ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_THIEF    ) && ( ch->class == CLASS_THIEF || ch->multied == CLASS_THIEF ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_WARRIOR  ) && ( ch->class == CLASS_WARRIOR || ch->multied == CLASS_WARRIOR ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_PSI      ) && ( ch->class == CLASS_PSIONICIST || ch->multied == CLASS_PSIONICIST ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_DRUID    ) && ( ch->class == CLASS_DRUID || ch->multied == CLASS_DRUID ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_RANGER   ) && ( ch->class == CLASS_RANGER || ch->multied == CLASS_RANGER ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_PALADIN  ) && ( ch->class == CLASS_PALADIN || ch->multied == CLASS_PALADIN ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_BARD     ) && ( ch->class == CLASS_BARD || ch->multied == CLASS_BARD ) )
	|| ( IS_OBJ_STAT( obj, ITEM_ANTI_VAMP     ) && ( ch->class == CLASS_VAMPIRE || ch->multied == CLASS_VAMPIRE) ) 
	|| ( IS_OBJ_STAT2( obj, ITEM_ANTI_WEREWOLF ) && ( ch->class == CLASS_WEREWOLF || ch->multied == CLASS_WEREWOLF ) ) 
	|| ( IS_OBJ_STAT2( obj, ITEM_ANTI_ANTIPAL  ) && ( ch->class == CLASS_ANTI_PALADIN || ch->multied == CLASS_ANTI_PALADIN ) ) 
	|| ( IS_OBJ_STAT2( obj, ITEM_ANTI_ASSASSIN ) && ( ch->class == CLASS_ASSASSIN || ch->multied == CLASS_ASSASSIN ) )
	|| ( IS_OBJ_STAT2( obj, ITEM_ANTI_MONK     ) && ( ch->class == CLASS_MONK || ch->multied == CLASS_MONK ) )
	|| ( IS_OBJ_STAT2( obj, ITEM_ANTI_BARBARIAN) && ( ch->class == CLASS_BARBARIAN || ch->multied == CLASS_BARBARIAN ) )
	|| ( IS_OBJ_STAT2( obj, ITEM_ANTI_ILLUSIONIST) && ( ch->class == CLASS_ILLUSIONIST || ch->multied == CLASS_ILLUSIONIST ) ) 
      || ( IS_OBJ_STAT2( obj, ITEM_ANTI_NECROMANCER ) && ( ch->class == CLASS_NECROMANCER || ch->multied == CLASS_NECROMANCER ) )
      || ( IS_OBJ_STAT2( obj, ITEM_ANTI_DEMONOLOGIST ) && ( ch->class == CLASS_DEMONOLOGIST || ch->multied == CLASS_DEMONOLOGIST ) )
	|| ( IS_OBJ_STAT2( obj, ITEM_ANTI_SHAMAN ) && ( ch->class == CLASS_SHAMAN || ch->multied == CLASS_SHAMAN ) )
      || ( IS_OBJ_STAT2( obj, ITEM_ANTI_DARKPRIEST ) && ( ch->class == CLASS_DARKPRIEST || ch->multied == CLASS_DARKPRIEST ) )
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
	act(AT_BLUE, "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
	act(AT_BLUE, "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	return;
    } else if (IS_OBJ_STAT2( obj, ITEM_LEGEND ) && !IS_LEGEND( ch ) )
    {
	act(AT_BLUE, "You somehow don't feel powerful enough for $p and set it on the ground...", ch, obj, NULL, TO_CHAR);
        act(AT_BLUE, "$n attempts to wear $p, thinks better of it and sets it on the ground.",  ch, obj, NULL, TO_ROOM );

        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        return;
    }
    }

    switch ( obj->ac_type )
    {
    default:   break;
    case 1:    do_acoload( ch, obj, obj->ac_vnum ); break;
    case 2:    do_acmload( ch, obj, obj->ac_vnum ); break;
    case 3:    do_actrans( ch, obj, obj->ac_vnum ); break;
    case 4:    do_acmorph( ch, obj, obj->ac_vnum ); break;
    case 5:    
        {
         spellarg[0] = '\0';
         sprintf( spellarg, "'%s' %s", obj->ac_spell, arg2 );
         do_acspell( ch, obj, spellarg );
         break;
        }
    }
    if ( obj->ac_charge[1] != -1 )
    if ( -- obj->ac_charge[0] <= 0 )
    {
	act(AT_WHITE, "Your $p sputters and sparks.", ch, obj, NULL, TO_CHAR );
	act(AT_WHITE, "$n's $p sputters and sparks..", ch, obj, NULL, TO_ROOM );
	obj->ac_type = 0;
	obj->ac_spell = " ";
	obj->ac_vnum = 0;
    }

    return;
}      

int skill_voodo ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA   *obj;
    CHAR_DATA  *victim;
    char        buf [MAX_STRING_LENGTH];
    char       *name;
    

    if( ch->summon_timer > 0 )
    {
	send_to_char(C_DEFAULT, "You try to get it to work, but nothing happens.\n\r", ch );
	return SKPELL_MISSED;
    }

    buf[0] = '\0';
    
    if ( !(victim = get_char_world( ch, target_name ) ) )
       {
        send_to_char( AT_RED, "No such person exists.", ch );
        return SKPELL_MISSED;
       }
    if ( saves_spell( ch->level, victim ) )
     {
      send_to_char( AT_RED, "You failed.\n\r", ch );
      return SKPELL_MISSED;
     } 
    if (IS_NPC(victim))
       name	  = victim->short_descr;
    else
       name       = victim->name;
    obj = create_object ( get_obj_index( OBJ_VNUM_DOLL ), 0 );
    sprintf( buf, obj->short_descr, name );
    free_string( obj->short_descr );
    obj->short_descr = str_dup( buf );
    free_string( obj->name );
    obj->name = str_dup( target_name );
    obj->timer = 10;
    ch->summon_timer = 10;
    obj_to_char(obj, ch);
    act(AT_RED, "You call upon the Gods to create $p.", ch, obj, NULL, TO_CHAR );
    act(AT_RED, "$n calls upon the Gods to create $p.", ch, obj, NULL, TO_ROOM );
    
	return SKPELL_NO_DAMAGE;
}     

void do_repair ( CHAR_DATA *ch, char *argument )
{

  char       arg[MAX_STRING_LENGTH];
  OBJ_DATA  *pObj;
  int        cost;
  char	     buf[MAX_STRING_LENGTH];
  
  if (IS_NPC(ch))
     return;
  if ( !IS_SET( ch->in_room->room_flags, ROOM_SMITHY ) )
  {
    send_to_char(AT_WHITE, "You are not within a smithy.\n\r", ch );
    return;
  }
  one_argument( argument, arg );
  if ( !str_cmp( arg, "all" ) )
    {
      char buf[MAX_STRING_LENGTH];
      for ( pObj = ch->carrying; pObj; pObj = pObj->next_content )
	{
	  if ( pObj->wear_loc == WEAR_NONE )
	    continue;
	  if ( pObj->durability_cur >= pObj->durability_max )
	    continue;
	  if ( pObj->durability_max <= 10 )
	  {
		sprintf( buf, "Your '%s' can no longer be repaired.\n\r", pObj->short_descr );
		send_to_char(AT_WHITE, buf, ch );
		continue;
	  }
	  cost = (pObj->durability_max - pObj->durability_cur) * pObj->weight * ( pObj->level / 4 );
	  switch (pObj->item_type)
	  {
	  case ITEM_WEAPON:
	    break;
	  case ITEM_ARMOR:
	    break;
	  default:
	    bug("Do_repair: Item not weapon or armor.",0);
	    break;
	  }

	  if ( cost <= 0 )
	    cost = 1;

	  if ( cost > ch->gold )
	    {
	      sprintf(buf, "$p will cost %d, you lack the funds.", cost );
	      act(AT_WHITE,buf,ch,pObj,NULL,TO_CHAR);
	      continue;
	    }
	  ch->gold -= cost;
	  pObj->cost = pObj->pIndexData->cost;
	  pObj->durability_max -= 1;
	  pObj->durability_cur = pObj->durability_max;	
	  REMOVE_BIT(pObj->extra_flags, ITEM_PATCHED);
	  sprintf(buf, "You are charged %d for repairing $p.", cost );
	  act(AT_WHITE,buf,ch,pObj,NULL,TO_CHAR);
	  if ( pObj->durability_max <= 10 )
	  {
		sprintf( buf, "Your '%s' can no longer be repaired.\n\r", pObj->short_descr );
		send_to_char(AT_WHITE, buf, ch );
	  }
	}
      return;
    }

  if (!( pObj = get_obj_carry ( ch, arg ) ) )
    {
      send_to_char (AT_WHITE, "You do not see that here.\n\r", ch );
      return;
    }
   
  if ( pObj->durability_max <= 10 )
   {
     sprintf( buf, "Your '%s' can not be repaired.\n\r", pObj->short_descr );
     send_to_char(AT_WHITE, buf, ch );
     return;  
   }

  if ( pObj->durability_max == pObj->durability_cur )
   {
     send_to_char(AT_WHITE, "That item is not damaged.\n\r", ch );
     return;  
   }

  cost = (pObj->pIndexData->cost - pObj->cost);
  switch( pObj->item_type )
  {
  case ITEM_WEAPON:
    cost = cost * pObj->value[1] / pObj->value[2];
    break;
  case ITEM_ARMOR:
    cost = cost * pObj->level / pObj->value[0];
    break;
  default:
    bug("Do_repair: Item not weapon or armor.", 0);
    break;
  }

  if ( cost <= 0 )
    cost = 1;

  if ( ch->gold < cost )
   {
     char    buf[MAX_STRING_LENGTH];
     
     sprintf(buf, "That item will cost %d, you lack the funds.\n\r", cost );
     send_to_char( AT_WHITE, buf, ch );
     return;
   }
  else
   {
     char     buf[MAX_STRING_LENGTH];
     
     sprintf( buf, "You are charged %d for repairing %s.\n\r", cost, pObj->short_descr );
     ch->gold -= cost;
     send_to_char ( AT_WHITE, buf, ch );
     pObj->cost = pObj->pIndexData->cost;
     pObj->durability_max -= 1;
     if ( pObj->durability_max <= 10 )
     {
	sprintf( buf, "Your '%s' can no longer be repaired.\n\r", pObj->short_descr );
	send_to_char(AT_WHITE, buf, ch );
     }
     pObj->durability_cur = pObj->durability_max;	

     REMOVE_BIT(pObj->extra_flags, ITEM_PATCHED);
     return;
   }
  return;

}

void do_account( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_STRING_LENGTH];
  
  if (IS_NPC( ch ) )
    return;
  else
  {
    if ( !IS_SET( ch->in_room->room_flags, ROOM_BANK ) )
    {
      send_to_char(AT_WHITE, "You are not in a bank!\n\r", ch );
      return;
    }
    
    if ( ch->pcdata->bankaccount > 0 )
    {
      sprintf( arg, "You have %d coin%s in your account.\n\r",
               ch->pcdata->bankaccount, 
               ch->pcdata->bankaccount > 1 ? "s" : "" );
      send_to_char(AT_WHITE, arg, ch );
      return;
    }
    else
    {
    int len = 0;
    len = strlen( ch->name );
      send_to_char(AT_WHITE, "You have nothing in your account!\n\r", ch );
      sprintf( arg, "&wFrom the shocked look on $n'%s face, you can tell that they have nothing in their account.",
      		    ch->name[len] == 's' ? "" : "s" );
      act(AT_WHITE, arg, ch, NULL, NULL, TO_ROOM );
      return;
    }
  }
  return;
}         
  
void do_separate( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *Obj;
  OBJ_DATA *aObj;
  OBJ_DATA *bObj;
  OBJ_INDEX_DATA *pIndex;

  if ( !( Obj = get_obj_carry( ch, argument ) ) )
  {
    send_to_char( AT_WHITE, "You are not carrying that item.\n\r", ch );
    return;
  }

  if ( !get_obj_index( Obj->pIndexData->sep_one ) ||
       !get_obj_index( Obj->pIndexData->sep_two ) )
  {
    send_to_char( AT_WHITE, "It cannot be separated.\n\r", ch );
    return;
  }

  pIndex = get_obj_index( Obj->pIndexData->sep_one );
  aObj = create_object( pIndex, pIndex->level );

  pIndex = get_obj_index( Obj->pIndexData->sep_two );
  bObj = create_object( pIndex, pIndex->level );
  sprintf( log_buf, "$n separates $p into %s and %s.\n\r",
           aObj->name, bObj->name );
  act( AT_WHITE, log_buf, ch, Obj, NULL, TO_ROOM );

  oprog_separate_trigger( Obj, ch );
  obj_from_char( Obj );
  extract_obj( Obj );
  obj_to_char( aObj, ch );
  obj_to_char( bObj, ch );
  send_to_char( AT_WHITE, "The object is now separated.\n\r", ch );
}

void do_join( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *aObj;
  OBJ_DATA *bObj;
  OBJ_DATA *Obj;
  OBJ_INDEX_DATA *pIndex;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( !(aObj = get_obj_carry(ch, arg1)) )
  {
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, "You are not carrying any %s.\n\r", arg1 );
    send_to_char( AT_WHITE, buf, ch );
    return;
  }

  if ( !(bObj = get_obj_carry(ch, arg2)) )
  {
    char buf[MAX_STRING_LENGTH];

    if (strlen( arg2 ) > 0 )
    {
    sprintf( buf, "You are not carrying any %s.\n\r", arg2 );
    send_to_char( AT_WHITE, buf, ch );
    return;
    }
    else 
    if (strlen( arg2 ) <= 0 )
    {
    send_to_char( AT_WHITE, "What's that?\n\r", ch );
    return;
    }
  }

  if ( aObj->pIndexData->join != bObj->pIndexData->join ||
       aObj->pIndexData == bObj->pIndexData || 
      !get_obj_index( aObj->pIndexData->join ) )
  {
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, "%s cannot be joined with %s.\n\r",
	    capitalize( aObj->short_descr ), bObj->short_descr );
    send_to_char( AT_WHITE, buf, ch );
    return;
  }
  oprog_join_trigger( aObj, ch, bObj );
  pIndex = get_obj_index( aObj->pIndexData->join );
  Obj = create_object( pIndex, pIndex->level );
  obj_to_char( Obj, ch );
  sprintf( log_buf, "$n joins $p to $P to create %s.\n\r", Obj->short_descr );
  act( AT_WHITE, log_buf, ch, aObj, bObj, TO_ROOM );

  obj_from_char( aObj );
  extract_obj( aObj );
  obj_from_char( bObj );
  extract_obj( bObj );
  send_to_char( AT_WHITE, "Objects joined.\n\r", ch );
}

/*
 * -- Altrag
 */
/*
 * -- Altrag Dalosein
 */
void do_patch( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  int ammount;
  int sn = skill_lookup("patched");

  if ( IS_NPC( ch ) )
    return;

  if ( ch->pcdata->learned[sn] <= 0 )
  {
    send_to_char(C_DEFAULT, "You don't know how to patch equipment.\n\r",ch);
    return;
  }

  if ( !( obj = get_obj_carry( ch, argument ) ) )
  {
    send_to_char(C_DEFAULT, "You do not have that item.\n\r",ch);
    return;
  }

  if ( obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR )
  {
    send_to_char(C_DEFAULT, "You may only repair weapons and armor.\n\r",ch);
    return;
  }

  if ( IS_SET(obj->extra_flags, ITEM_PATCHED) )
  {
    send_to_char(C_DEFAULT, "You can't do much more for it.\n\r",ch);
    return;
  }

  if ( obj->cost >= obj->pIndexData->cost )
  {
    send_to_char(C_DEFAULT, "It already looks like new.\n\r",ch);
    return;
  }

  ammount = ch->pcdata->learned[sn] / 20;
  ammount = (ammount * (obj->pIndexData->cost - obj->cost)) / 100;
  obj->cost += ammount;
  SET_BIT(obj->extra_flags, ITEM_PATCHED);

  act(AT_WHITE,"$n repairs his $p a bit.",ch,obj,NULL,TO_ROOM);
  act(AT_WHITE,"You do your best to repair your $p.",ch,obj,NULL,TO_CHAR);
  return;
}

int skill_devour( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj;
    int       amnt;

    if ( target_name[0] == '\0' )
    {
	send_to_char(AT_ORANGE, "Devour what?\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !( obj = get_obj_carry( ch, target_name ) ) )
    {
	send_to_char(AT_ORANGE, "You do not have that item.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !IS_IMMORTAL( ch ) )
    {
	if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] > 40 )
	{   
	    send_to_char(AT_ORANGE, "You are too full to eat more.\n\r", ch );
	    return SKPELL_BOTCHED;
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
	        send_to_char(AT_ORANGE, "You are full.\n\r", ch );
	    else if ( condition == 0 && ch->pcdata->condition[COND_FULL] > 0 )
		send_to_char(AT_ORANGE, "You are no longer hungry.\n\r", ch );
	}

	if ( obj->value[3] != 0 )
	{
	    /* The shit was poisoned! */
	    AFFECT_DATA af;

	    act(AT_GREEN, "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	    send_to_char(AT_GREEN, "You choke and gag.\n\r", ch );

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
       send_to_char(AT_ORANGE, "You feel warm all over.\n\r", ch);
       break;        
    case ITEM_PILL:
	    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	break;
    }

    extract_obj( obj );
    return SKPELL_NO_DAMAGE;
}

/*
 * Hide objects... by Maniac!
 */
int find_door(CHAR_DATA *ch, char *arg);

void do_hide_obj(CHAR_DATA *ch, char *argument)
{
    char        arg[MAX_INPUT_LENGTH];
    int         chance;
    int         door;
    OBJ_DATA    *obj;

    one_argument(argument,arg);
    if ( arg[0] == '\0' )
    {
        send_to_char(AT_WHITE,"What do you want to hide?\n\r", ch);
        return;
    }


    if ( (door = find_door( ch, arg)) >=0 )  /* re-hide hidden exits --Manaux */
    {
      if ( !IS_SET(ch->in_room->exit[door]->exit_info, EX_HIDDEN)
         && IS_SET(ch->in_room->exit[door]->rs_flags, EX_HIDDEN) )
      {
        SET_BIT( ch->in_room->exit[door]->exit_info, EX_HIDDEN);
        send_to_char(AT_WHITE, "You successfully re-hide it.", ch);
        return;
      }
      send_to_char(AT_WHITE, "You can't hide that!", ch);
      return;
    } 

    if ( !( obj = get_obj_carry( ch, arg ) ) )
    {
        send_to_char(AT_WHITE, "You do not have that item.\n\r", ch );
        return;
    }
    if ( !can_drop_obj( ch, obj ) )
    {
//		if ( !IS_SET(obj->extra_flags2, ITEM_QUEST) )
			send_to_char(AT_WHITE, "You can't let go of it.\n\r", ch );
/*		else
			send_to_char(AT_WHITE, "Your precious quest equipment?\n\r", ch); */
        return;
    }

    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );

    if (IS_SET(obj->extra_flags2, ITEM_HIDDEN))  /* no use in hiding it again */
        return;

    chance = number_range(1, 5);
    if (ch->class == 6)      /* Rangers are better */
        chance += 2;
    if (ch->class == 2)       /* Thieves as wel, but not so good */
        chance++;
    if (chance > 5)                     /* Let's not push it... */
        chance = 5;
    if (ch->level > LEVEL_HERO )        /* Immortals will manage */
        chance = 5;
    switch (chance)                     /* Let's see what we've got */
    {
        case 1:
                act(AT_WHITE,"$n is on all fours trying to hide $p.", ch, obj, NULL, TO_ROOM );
                act(AT_WHITE,"You try to hide $p, but fail misarably.", ch, obj, NULL, TO_CHAR );
                break;
        case 2:
                act(AT_WHITE,"$n is on all fours digging in the dirt.", ch, NULL, NULL, TO_ROOM );
                act(AT_WHITE,"You hide $p, but you did it quite obvious.", ch, obj, NULL, TO_CHAR );
                SET_BIT(obj->extra_flags2, ITEM_HIDDEN);
                break;
        case 3:
                act(AT_WHITE, "You fail to hide $p.", ch, obj, NULL, TO_CHAR );
                break;
        case 4:
        case 5:
                act(AT_WHITE, "You hide $p.", ch, obj, NULL, TO_CHAR );
                SET_BIT(obj->extra_flags2, ITEM_HIDDEN);
                break;
    }
}

void do_search(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char buf[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    int found = FALSE;
    bool foundExit = FALSE;
    int chance;
    int door;
    int percent;
    int sn = skill_lookup("search");
    EXIT_DATA *pexit;
 
    buf[0] = '\0';
    buf2[0] = '\0';


    WAIT_STATE( ch, skill_table[sn].beats );
    percent  =  number_percent( );

  if ( !IS_NPC( ch ) && percent < ( ch->pcdata->learned[sn] / 10 ) + 35  )
  {
    if (!IS_NPC(ch) && ch->pcdata->learned[sn])
	update_skpell(ch, sn, 0);

    for ( obj = ch->in_room->contents; obj && (!found); obj = obj->next_content )
    {
        if ( IS_SET(obj->extra_flags2, ITEM_HIDDEN) )
        {
                chance = number_range( 1, 5);
                if ((ch->class == 6) || (ch->class == 2))
                        chance++;
                if (chance > 3)
                {
                        sprintf(buf, "You find %s.\n\r", obj->short_descr);
                        strcat(buf2, buf);
                        buf[0] = '\0';
                        REMOVE_BIT(obj->extra_flags2, ITEM_HIDDEN);
                        found = TRUE;
                }
        }
    }
    /*Make search find hidden exits. */

    for ( door = 0; door < 6 ; door++)
    {
      
      if ((pexit = ch->in_room->exit[door])
           &&IS_SET(pexit->exit_info, EX_HIDDEN) )
      {
        REMOVE_BIT( pexit->exit_info , EX_HIDDEN ) ;
        if (!foundExit)
        {
          foundExit = TRUE;
          send_to_char(AT_WHITE, "You revealed a hidden exit.\n\r", ch);
        }
      }  
    }

   }
   if (!found && !foundExit)
        strcat(buf2, "You find nothing.\n\t");
    send_to_char(AT_WHITE,buf2, ch);
    buf2[0] = '\0';
}

int skill_lore( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj;


    if ( target_name[0] == '\0' )
    {
        send_to_char(AT_WHITE,"What do you want to lore?\n\r", ch);
        return SKPELL_BOTCHED;
    }

    obj = get_obj_carry(ch,target_name);
    if ( obj == NULL )
    {
        send_to_char(AT_WHITE,"You aren't carrying that.\n\r", ch);
        return SKPELL_BOTCHED;
    }

    WAIT_STATE(ch, skill_table[sn].beats);

    if ( !IS_NPC(ch) && number_percent( ) > ( ch->pcdata->learned[sn] / 10 ) )
    {
        act(AT_WHITE,"You look at $p, but you can't find out any additional information.",            ch,obj,NULL,TO_CHAR);
        act(AT_WHITE,"$n looks at $p but cannot find out anything.", ch, obj, NULL, TO_ROOM);
        return SKPELL_MISSED;
    }
    else
    {
        act(AT_WHITE,"$n studies $p, discovering all of it's hidden powers.",ch,obj,NULL,TO_ROOM);
        if ( ( number_percent( ) * number_percent( ) ) < 40 )
        {
            obj->cost += (int) ( (ch->pcdata->learned[sn]) * 0.01 * obj->cost);
            send_to_char(AT_WHITE, "Your understanding of the lore behind it increases its worth!\n\r",  ch );
        }

        spell_identify(sn,( 4 * obj->level )/3,ch,obj);
        /* check_improve(ch,gsn_lore,TRUE,4); */
    }

    return SKPELL_NO_DAMAGE;
}

int skill_embalm( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = 0;


    return SKPELL_MISSED;

    if ( target_name[0] == '\0' )
    {
        send_to_char(AT_WHITE,"What do you want to embalm?\n\r", ch);
        return SKPELL_BOTCHED;
    }

    if ( obj->item_type != ITEM_CORPSE_PC )
    {
	send_to_char(AT_WHITE, "You can't embalm that!\n\r", ch);
	return SKPELL_BOTCHED;
    }

    if (IS_SET(obj->wear_flags, ITEM_TAKE))  
        return SKPELL_NO_DAMAGE;

    if ( ( obj->item_type = ITEM_CORPSE_PC ) )
	{
	obj->wear_flags = 1;
	send_to_char(AT_WHITE, "You have embalmed the player's corpse successfully!", ch);
	return SKPELL_NO_DAMAGE;
	}

	return SKPELL_MISSED;
}

int skill_gravebind( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj;
    int         gain = 0;

      if ( !IS_NPC( ch )
        && ( ( ch->level < skill_table[sn].skill_level[ch->class] )   
        && (ch->level < skill_table[sn].skill_level[ch->multied])))    
    {
	send_to_char(C_DEFAULT,
	     "You'd better leave the arts of the dead to the necromancers.\n\r", ch);
	return SKPELL_BOTCHED;
    }

    if ( target_name[0] == '\0' )
    {
        send_to_char(C_DEFAULT, "Gravebind what?\n\r", ch );
        return SKPELL_BOTCHED;
    }

    obj = (OBJ_DATA*) vo;//get_obj_list( ch, target_name, ch->in_room->contents );
    if ( !obj )
    {
        send_to_char(C_DEFAULT, "You can't find it.\n\r", ch );
        return SKPELL_BOTCHED;
    }

    if ( obj->item_type != ITEM_CORPSE_NPC )
    {
        send_to_char(C_DEFAULT, "You can only gravebind corpses of non-players.\n\r", ch );
        return SKPELL_BOTCHED;
    }

    if ( ( obj->item_type = ITEM_CORPSE_NPC ) )
    {
    gain = number_range((int)(ch->max_hit * 0.3), (int)(ch->max_hit * 0.5)) + obj->value[1];

    if (ch->max_hit >= (ch->hit + gain))
	ch->hit = (ch->hit + gain);
    else
	{
	gain = (ch->max_hit - ch->hit);
	ch->hit = (ch->hit + gain);
	}
	
	ch->move -= (int)(gain/12);	/* Added penalty due to new heal range */

    send_to_char(AT_GREY, "You absorb the lasting strength of the corpse.\n\r", ch);
    act(AT_GREY, "$n drains the corpse of its lasting energy.", ch, NULL, NULL, TO_ROOM);

    extract_obj( obj );
    }
return SKPELL_NO_DAMAGE;
}  

int skill_gorge( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA  * obj;
    int         amount;

    if ( IS_NPC( ch ) )
    {
	return SKPELL_BOTCHED;
    }

    if ( target_name[ 0 ] == '\0' )
    {
        for (obj = ch->in_room->contents; obj; obj = obj->next_content )
        {
            if ( obj->item_type == ITEM_BLOOD )
                break;
        }
        if ( !obj )
        {
            send_to_char ( AT_RED, "Gorge from what?\n\r", ch );
            return SKPELL_MISSED;
        }
    }
    else
    {
        if ( !( obj = get_obj_here ( ch, target_name ) ) )
        {
            send_to_char ( AT_BLUE, "You can't find it.\n\r", ch );
            return SKPELL_MISSED; 
        }
    }

    if ( obj->item_type != ITEM_BLOOD )
    {
	send_to_char ( AT_RED, "You can't gorge yourself from that.\n\r", ch );
        return SKPELL_BOTCHED;
    }

    if ( ( ch->bp + 1 ) > ch->max_bp )
    {
        send_to_char ( AT_RED, "Your thirst for blood has been abated.\n\r", ch );
        return SKPELL_NO_DAMAGE;
    }
/*
    amount = UMIN ( (ch->max_bp - ch->bp ), obj->value[1] );
    amount = UMIN ( amount, 25 );
*/
    if( !IS_NPC( ch ) )
    {
	amount = number_fuzzy( ch->pcdata->learned[sn] / 25 ) + number_fuzzy( ch->level / 10 );
    }
    else
    {
	amount = number_fuzzy( ch->level / 10 );
    }
    ch->bp += amount;
    if ( obj->value[0] != -1 )
        obj->value[1] -= amount;
    if ( obj->value[1] > 0 )
    {
        act ( AT_RED, "You gorge yourself from $p.", ch, obj, NULL, TO_CHAR );
        act ( AT_RED, "$n gorges $mself from $p.", ch, obj, NULL, TO_ROOM );
    }
    else
    {
        act ( AT_RED, "You gorge yourself from $p, which vanishes.", ch, obj, NULL, TO_CHAR );

        act ( AT_RED, "$n gorges $mself from $p, which vanishes.", ch, obj, NULL, TO_ROOM );
        extract_obj ( obj );
    }

	return SKPELL_NO_DAMAGE;
}
