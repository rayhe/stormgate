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

/*$Id: magic.c,v 1.57 2005/04/04 16:02:46 ahsile Exp $*/

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
 * External functions.
 */
bool    is_safe     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );


/*
 * Local functions.
 */
void    set_fighting    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	say_spell	args( ( CHAR_DATA *ch, int sn ) );
int     blood_count     args( ( OBJ_DATA *list, int amount ) ); 
void    magic_mob       args( ( CHAR_DATA *ch, OBJ_DATA *obj, int vnum ) );
int     slot_lookup     args( ( int slot ) );

/*
 * "Permament sn's": slot loading for objects -- Altrag
 */
int slot_lookup( int slot )
{
  int sn;

  for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
    if ( skill_table[sn].slot == slot )
      return sn;

  bug( "Slot_lookup: no such slot #%d", slot );
  return 0;
}

/*
 * Replacement for MAX_SKILL -- Altrag
 */
bool is_sn( int sn )
{
  int cnt;

  for ( cnt = 0; skill_table[cnt].name[0] != '\0'; cnt++ )
    if ( cnt == sn )
      return TRUE;

  return FALSE;
}

void magic_mob ( CHAR_DATA *ch, OBJ_DATA *obj, int vnum )
{
   CHAR_DATA      *victim;
   CHAR_DATA      *zombie;
   MOB_INDEX_DATA *ZombIndex;
   MOB_INDEX_DATA *pMobIndex;
   char           *name;
   char            buf [MAX_STRING_LENGTH];
 
    if ( !( pMobIndex = get_mob_index( vnum ) ) )
    {
         send_to_char(AT_BLUE, "Nothing happens.\n\r", ch);
         return;
    }
    ZombIndex = get_mob_index( 3 );
    victim = create_mobile( pMobIndex );
    zombie = create_mobile( ZombIndex );
    name = victim->short_descr;
    sprintf( buf, zombie->short_descr, name );
    free_string( zombie->short_descr );
    zombie->short_descr = str_dup(buf);
    sprintf( buf, zombie->long_descr, name );
    free_string( zombie->long_descr );
    zombie->long_descr = str_dup(buf);
    victim->max_hit /= 2;
    victim->hit = victim->max_hit;
    zombie->max_hit = victim->max_hit;
    zombie->hit = victim->hit;
    zombie->level = victim->level;
    SET_BIT( zombie->act, PLR_UNDEAD );
    SET_BIT( zombie->act, ACT_PET );
    SET_BIT( zombie->affected_by, AFF_CHARM );
    char_to_room( zombie, ch->in_room );
    add_follower( zombie, ch );
    update_pos( zombie );
    act( AT_BLUE, "$n passes $s hands over $p, $E slowly rises to serve $S new master.", ch, obj, zombie, TO_ROOM );
    act( AT_BLUE, "You animate $p, it rises to serve you.", ch, obj, NULL, TO_CHAR );
    char_to_room( victim, ch->in_room );
    extract_char ( victim, TRUE );
    return;
}
int blood_count( OBJ_DATA *list, int amount )
{
    OBJ_DATA   *obj;
    int         count;
    OBJ_DATA   *obj_next;
    
    
    count = 0;
    for ( obj = list; obj; obj = obj_next )
    {
	obj_next = obj->next_content;
	if ( obj->deleted )
	   continue;
	if ( ( obj->item_type == ITEM_BLOOD ) && ( count != amount ) )
	   {
	     count++;
             extract_obj( obj );
	   }
    }
      
    return count;
}

void update_skpell( CHAR_DATA *ch, int sn, int override )
{
	int pct = 0;
	int gain = 1;

	if (IS_NPC(ch))
		return;

	if( override == 0 )
	{
		if( ch->pcdata->learned[sn] == 0 )
		{
			return;
		}
		if( ch->pcdata->learned[sn] <= 100 )
		{
			pct = 100 + ( skill_table[sn].is_spell ? int_app[get_curr_int(ch)].bonus : dex_app[get_curr_dex(ch)].bonus );
			gain = number_fuzzy( 10 );
		}
		else if( ch->pcdata->learned[sn] <= 200 )
		{
			pct = 95 + ( skill_table[sn].is_spell ? int_app[get_curr_int(ch)].bonus : dex_app[get_curr_dex(ch)].bonus );
			gain = number_fuzzy( 9 );
		}
		else if( ch->pcdata->learned[sn] <= 300 )
		{
			pct = 90 + ( skill_table[sn].is_spell ? int_app[get_curr_int(ch)].bonus : dex_app[get_curr_dex(ch)].bonus );
			gain = number_fuzzy( 8 );
		}
		else if( ch->pcdata->learned[sn] <= 400 )
		{
			pct = 85 + ( skill_table[sn].is_spell ? int_app[get_curr_int(ch)].bonus : dex_app[get_curr_dex(ch)].bonus );
			gain = number_fuzzy( 7 );
		}
		else if( ch->pcdata->learned[sn] <= 500 )
		{
			pct = 75 + ( skill_table[sn].is_spell ? int_app[get_curr_int(ch)].bonus : dex_app[get_curr_dex(ch)].bonus );
			gain = number_fuzzy( 6 );
		}
		else if( ch->pcdata->learned[sn] <= 600 )
		{
			pct = 65 + ( skill_table[sn].is_spell ? int_app[get_curr_int(ch)].bonus : dex_app[get_curr_dex(ch)].bonus );
			gain = number_fuzzy( 4 );
		}
		else if( ch->pcdata->learned[sn] <= 750 )
		{
			pct = ( 125 - ( ch->pcdata->learned[sn] / 10 ) ) 
				+ ( skill_table[sn].is_spell ? int_app[get_curr_int(ch)].bonus : dex_app[get_curr_dex(ch)].bonus );
			gain = number_fuzzy( 2 );
		}
		else
		{
			pct = ( ( 125 - ( ch->pcdata->learned[sn] / 10 ) ) / 2 )  
				+ ( skill_table[sn].is_spell ? int_app[get_curr_int(ch)].bonus : dex_app[get_curr_dex(ch)].bonus );
			gain = 1;
		}
		if( gain < 1 )
		   gain = 1;
	}
	else
	{
		gain = override;
		pct = 101;
	}

	if ( number_percent() < pct )
	{
		if ( ch->pcdata->learned[sn] < 1000 )
		{
			if( ( ch->level < LEVEL_HERO && number_percent( ) <= 50 ) || ch->level >= LEVEL_HERO || override != 0 )
			{
				char buf[MAX_STRING_LENGTH];
				ch->pcdata->learned[sn] += gain;
				if( ch->pcdata->learned[sn] > 1000 )
				   ch->pcdata->learned[sn] = 1000;
				sprintf(buf, "Your %s \"%s\" improves to %3.1f%%!\n\r", skill_table[sn].is_spell ? "spell" : "skill" , skill_table[sn].name, ch->pcdata->learned[sn] / 10.0f );
				send_to_char(AT_CYAN, buf, ch );
				if( ch->pcdata->learned[sn] >= 1000 )
				{
					sprintf(buf, "Congratulations!  Your %s \"%s\" has reached grandmaster level!\n\r", skill_table[sn].is_spell ? "spell" : "skill" , skill_table[sn].name );
					send_to_char(AT_CYAN, buf, ch );
				}
			}
		}
	}
	return;
}

int skill_lookup( const char *name )
{
//    char buf[MAX_STRING_LENGTH];
    int sn;

    if (!str_cmp(name,""))
	return -1;
	
    for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
    {
	if ( !skill_table[sn].name )
	    break;
	if ( LOWER( name[0] ) == LOWER( skill_table[sn].name[0] )
	    && !str_prefix( name, skill_table[sn].name ) )
	    return sn;
    }
/*
    sprintf(buf, "Skill Lookup: Can't find skill: \"%s\"", name);
    bug(buf,0);
*/
    return -1;
}



/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
                        CHAR_DATA *rch;
			char      *pName;
			char       buf       [ MAX_STRING_LENGTH ];
			char       buf2      [ MAX_STRING_LENGTH ];
			int        iSyl;
			int        length;

	       	 struct syl_type
	         {
		        char *	   old;
		        char *	   new;
		 };

    static const struct syl_type   syl_table [ ] =
    {
	{ " ",		" "		},
	{ "ar",		"abra"		},
	{ "au",		"kada"		},
	{ "bless",	"fido"		},
	{ "blind",	"nose"		},
	{ "bur",	"mosa"		},
	{ "cu",		"judi"		},
	{ "de",		"oculo"		},
	{ "en",		"unso"		},
	{ "light",	"dies"		},
	{ "lo",		"hi"		},
	{ "mor",	"zak"		},
	{ "move",	"sido"		},
	{ "ness",	"lacri"		},
	{ "ning",	"illa"		},
	{ "per",	"duda"		},
	{ "ra",		"gru"		},
	{ "re",		"candus"	},
	{ "son",	"sabru"		},
	{ "tect",	"infra"		},
	{ "tri",	"cula"		},
	{ "ven",	"nofo"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0';
    for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
	for ( iSyl = 0;
	     ( length = strlen( syl_table[iSyl].old ) ) != 0;
	     iSyl++ )
	{
	    if ( !str_prefix( syl_table[iSyl].old, pName ) )
	    {
		strcat( buf, syl_table[iSyl].new );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    sprintf( buf2, "$n utters the words, '%s'.", buf );
    sprintf( buf,  "$n utters the words, '%s'.", skill_table[sn].name );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
	    act(AT_BLUE, ch->class==rch->class ? buf : buf2, ch, NULL, rch, TO_VICT );
    }

    return;
}



/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA *victim )
{
    int save;
    int base = 20;
    int savebase;
    if ( IS_NPC( victim ) )
        base += 30;

    savebase = 0 - (victim->saving_throw < -440 ? -440 : victim->saving_throw);

    if ( !IS_NPC( victim ) )
        savebase /= 8;
    else
        savebase /= 2;

    save = base + ( victim->level - level ) + savebase;
    save = URANGE( 5, save, 90 );
    return number_percent( ) < save;
}



/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_acspell ( CHAR_DATA *ch, OBJ_DATA *pObj, char *argument )
{
    void      *vo;
    OBJ_DATA  *obj = NULL;
    CHAR_DATA *victim;
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int        sn;
    int        spec;
	int			dmg = 0;
 
    spec = skill_lookup( "astral walk" );
    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );

    if ( IS_NPC( ch ) )
      if ( IS_SET( ch->affected_by, AFF_CHARM ) )
        return;

    if ( ( sn = skill_lookup( arg1 ) ) < 0) 
    {
	send_to_char(AT_BLUE, "You can't do that.\n\r", ch );
	return;
    }
  
    if ( ( sn == spec )  && ( is_name( arg2, ch->name ) ) )
       {
         send_to_char( AT_BLUE, "You are already in the same room as yourself.\n\r", ch );
         return;
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
	bug( "Do_cast: bad target for sn %d.", sn );
	return;

    case TAR_GROUP_OFFENSIVE:
    case TAR_GROUP_DEFENSIVE:
    case TAR_GROUP_ALL:
    case TAR_GROUP_OBJ:
    case TAR_GROUP_IGNORE:
	group_cast( sn, URANGE( 1, ch->level, LEVEL_DEMIGOD ), ch, arg2 );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
    if ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) )
      {
       send_to_char( AT_BLUE, "Your spell has failed.\n\r", ch );
       return;
      }	   
    	if ( arg2[0] == '\0' )
	{
	    if ( !( victim = ch->fighting ) )
	    {
		send_to_char(AT_BLUE, "Cast the spell on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( !( victim = get_char_room( ch, arg2 ) ) )
	    {
		send_to_char(AT_BLUE, "They aren't here.\n\r", ch );
		return;
	    }
	}

 

    if ( !is_pkillable( ch, victim ) ) {
    	return;
    }

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
 
    if (!IS_NPC(victim))
    {
	if (victim->pkill == TRUE)
	    	ch->pkill_timer = 0;
    }

    if ( IS_AFFECTED( ch, AFF_PEACE) )
    {
	    affect_strip( ch, skill_lookup("aura of peace") );
	    REMOVE_BIT( ch->affected_by, AFF_PEACE );
    }
    	vo = (void *) victim;
	if (!pk_combat_check(ch, victim))
	  return;
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
	    send_to_char(AT_BLUE, "You cannot cast this spell on another.\n\r", ch );
	    return;
	}

	vo = (void *) ch;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char(AT_BLUE, "What should the spell be cast upon?\n\r", ch );
	    return;
	}

	if ( !( obj = get_obj_carry( ch, arg2 ) ) )
	{
	    send_to_char(AT_BLUE, "You are not carrying that.\n\r", ch );
	    return;
	}

	vo = (void *) obj;
	break;
    }


      
    WAIT_STATE( ch, skill_table[sn].beats );
      
    if ( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
      {
       send_to_char( AT_BLUE, "You failed.\n\r", ch );
       return;
      }	   

    dmg = (*skill_table[sn].spell_fun) ( sn, URANGE( 1, ch->level, LEVEL_DEMIGOD ), ch, vo );
	if (dmg > SKPELL_NO_DAMAGE)
		damage(ch, victim, dmg, sn);

    if ( vo )
    {
      oprog_invoke_trigger( pObj, ch, vo );
      if ( skill_table[sn].target == TAR_OBJ_INV )
	oprog_cast_sn_trigger( obj, ch, sn, vo );
      rprog_cast_sn_trigger( ch->in_room, ch, sn, vo );
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
bool pk_combat_check(CHAR_DATA *ch, CHAR_DATA *victim)
{
  CHAR_DATA * rch;
  bool found = FALSE;
  if (!victim->fighting || IS_NPC( victim ) || IS_NPC( ch ) )
    return TRUE;
  if (IS_SET(ch->in_room->room_flags, ROOM_PKILL ) &&
     IS_SET(victim->in_room->room_flags, ROOM_PKILL ))
  	{
	ch->pkill_timer = 0;
	victim->pkill_timer = 0;
	return TRUE;
	}


  for( rch= ch->in_room->people; rch; rch= rch->next_in_room )
    {
      if (rch->deleted || !rch->fighting || rch == victim 
	  || rch == ch || IS_NPC(rch) )
	continue;

      /* ok, a player in the room is fighting the victim, should we 
	 allow the spell? allow the spell and set their pk timer?
	 or not allow the spell? */
      
      if (rch->fighting == victim && rch != ch )
	{
	  /*peacefuls shouldn't get involved*/
	  if (ch->pkill == 0)
	    {
	      send_to_char(AT_WHITE, "You can't get involved in pkill battles!\r\n", ch);
	      return FALSE;
	    }
	  else if ( abs( ch->level - rch->level ) > PKILL_RANGE ||
		    abs( ch->level - victim->level ) > PKILL_RANGE )
	    {
	      send_to_char(AT_RED, "You can't get involved in this pk battle\r\nValid Range is +/- 8 levels.\r\n", ch);
	      return FALSE;
	    }
	  else if (abs( ch->level - rch->level ) <= PKILL_RANGE ||
		   abs( ch->level - victim->level) <= PKILL_RANGE )
	    {
	      /*This target is in range, so, set the found flag,
		there may still be other out of range ppl in the fight. */
	      found = TRUE;
	    }
	}
    }

  /*if we get to here, then it's ok to cast the spell, if we found a pk
    fight along the way, reset the caster's pk timer */
  if (found)
    ch->pkill_timer = 0;
  return TRUE;
    
}

void do_cast( CHAR_DATA *ch, char *argument )
{
    void      *vo;
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int        mana;
    int        sn = 0;
    int        spec;
    int	       intreq = 0;
	int	intreq1 = 0;
	int	intreq2 = 0;
    spec = skill_lookup( "astral walk" );
    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char(AT_BLUE, "Cast which what where?\n\r", ch );
	return;
    }

    if ( !IS_NPC( ch ) )
      if ( IS_AFFECTED4( ch, AFF_BURROW ) )
      {
         send_to_char(AT_RED, "You cannot cast while burrowed!\n\r", ch);
         return;
      }
    if ( IS_NPC( ch ) )
      if ( IS_SET( ch->affected_by, AFF_CHARM ) )
        return;
    if (IS_NPC( ch ) )
	if (ch->mana == 0)
	return;

    if ( !IS_NPC( ch ) )
    if ( ( sn = skill_lookup( arg1 ) ) < 0
	|| ( ch->level < skill_table[sn].skill_level[ch->class]
 	&& ch->level < skill_table[sn].skill_level[ch->multied] ) )
    {
	send_to_char(AT_BLUE, "You can't do that.\n\r", ch );
	return;
    }

	if (!skill_table[sn].is_spell)
	{
		send_to_char(AT_WHITE, "Casting a skill? Try using it...\n\r", ch);
		return;
	}

    
    if ( !IS_NPC( ch ) )
    if ( ch->class == CLASS_BARBARIAN  || ch->multied == CLASS_BARBARIAN )
    {
        if ( skill_table[sn].skill_level[ch->class] > 60
        && skill_table[sn].skill_level[ch->multied] > 60 ) 
	{
	    send_to_char(C_DEFAULT, "Barbarians can not cast magic of such high level.\n\r", ch );
	    return;
	}
    }

    if ( !IS_NPC( ch ) )
    if ( ( ch->race == 10 && IS_EVIL( ch ) ) ||
	 ( ch->race == 10 && IS_NEUTRAL( ch ) ) ||
	 ( ch->race == 15 && IS_GOOD( ch ) ) ||
	 ( ch->race == 15 && IS_NEUTRAL( ch ) ) ||
	 ( ch->class ==   CLASS_CLERIC && IS_EVIL( ch ) ) ||
	 ( ch->multied == CLASS_CLERIC && IS_EVIL( ch ) ) ||
	 ( ch->class ==   CLASS_PALADIN && IS_EVIL( ch ) ) ||
	 ( ch->multied == CLASS_PALADIN && IS_EVIL( ch ) ) ||
	 ( ch->class ==   CLASS_ANTI_PALADIN && IS_GOOD( ch ) ) ||
	 ( ch->multied == CLASS_ANTI_PALADIN && IS_GOOD( ch ) ) ||
	 ( ch->class ==   CLASS_DARKPRIEST && IS_GOOD( ch ) ) ||
	 ( ch->multied == CLASS_DARKPRIEST && IS_GOOD( ch ) )		)
    {
	send_to_char(AT_BLUE, "You can not cast when you are that alignment.\n\r", ch );
	return;
    }

    if ( IS_NPC( ch ) )
     if ( ( sn = skill_lookup( arg1 ) ) < 0 )
       return;

    if ( ch->position < skill_table[sn].minimum_position )
    {
	send_to_char(AT_BLUE, "You can't concentrate enough.\n\r", ch );
	return;
    }

    if IS_AFFECTED2( ch, AFF_SLIT)
	{
	send_to_char(AT_BLOOD, "Your throat is slit, you can not cast.\n\r", ch );
	return;
	}

    if ( IS_STUNNED( ch, STUN_MAGIC ) )
    {
      send_to_char(AT_LBLUE, "You're too stunned to cast spells.\n\r", ch );
      return;
    }

    mana = 0;
    if(!IS_NPC( ch ) && ch->level >= skill_table[sn].skill_level[ch->multied] )
    {
	mana = MANA_COST_MULTI(ch, sn );
        intreq1 = ( skill_table[sn].skill_level[ch->multied] / 2 ) - 5;
	if( intreq1 >= 40 )
	{
	    intreq1 = 40;
	}
    }
    if(!IS_NPC( ch ) && ch->level >= skill_table[sn].skill_level[ch->class] )
    {
	mana = MANA_COST( ch, sn );
        intreq2 = ( skill_table[sn].skill_level[ch->class] / 2) - 5;
        if( intreq2 >= 40 )
        {
            intreq2 = 40;
        }
    }
      intreq = UMIN(intreq1, intreq2);
    if ( ch->class == CLASS_VAMPIRE || ch->class == CLASS_ANTI_PALADIN )
    {
	mana /= 4;
    }

    if(ch->class != CLASS_CLERIC && ch->class != CLASS_DARKPRIEST )
    {
	if( get_curr_int( ch ) < intreq )
	{
	    send_to_char(AT_BLUE, "You do not have the required intelligence.\n\r", ch );
	    return;
	}
    }

    if(ch->class == CLASS_CLERIC || ch->class == CLASS_DARKPRIEST )
    {
	if( get_curr_wis (ch ) < intreq )
	{
	    send_to_char(AT_BLUE, "You do not have the required wisdom.\n\r", ch );
	    return;
	}
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
	bug( "Do_cast: bad target for sn %d.", sn );
	return;

    case TAR_GROUP_OFFENSIVE:
    case TAR_GROUP_DEFENSIVE:
    case TAR_GROUP_ALL:
    case TAR_GROUP_OBJ:
    case TAR_GROUP_IGNORE:
	group_cast( sn, URANGE( 1, ch->level, LEVEL_DEMIGOD ), ch, arg2 );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) )
	{
		send_to_char( AT_BLUE, "You failed.\n\r", ch );
		return;
	}

	if ( arg2[0] == '\0' )
	{
	    if ( !( victim = ch->fighting ) )
	    {
		send_to_char(AT_BLUE, "Cast the spell on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( !( victim = get_char_room( ch, arg2 ) ) )
	    {
		send_to_char(AT_BLUE, "They aren't here.\n\r", ch );
		return;
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
	    send_to_char(AT_BLUE, "You cannot cast this spell on another.\n\r", ch );
	    return;
	}

	vo = (void *) ch;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char(AT_BLUE, "What should the spell be cast upon?\n\r", ch );
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
    if ( ( ch->class != 9 ) && ( ch->class != 11) && ( ch->mana < mana ) )
       {
   	send_to_char(AT_BLUE, "You don't have enough mana.\n\r", ch );
	return;
       }
    else
       if ( ( ch->bp < mana ) && (( ch->class == 9 )||( ch->class == 11)) )
       {
   	send_to_char(AT_RED, "You are to starved to cast, you must feed.\n\r", ch );
	return;
       }
    }

    if ( str_cmp( skill_table[sn].name, "ventriloquate" ) )
	say_spell( ch, sn );
      
    if ( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
      {
       send_to_char( AT_BLUE, "You failed.\n\r", ch );
       return;
      }	   
    if ( ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) ) && ( skill_table[sn].target == TAR_CHAR_OFFENSIVE ) )
      {
       send_to_char( AT_BLUE, "You failed.\n\r", ch );
       return;
      }	   

  
    if ( !IS_NPC( ch ) )
    {
    if ( number_percent( ) > ( ch->pcdata->learned[sn] / 10 )  )
    {
	send_to_char(AT_BLUE, "You lost your concentration.\n\r", ch );
	if (( ch->class != 9 )&&( ch->class  != 11))
	   ch->mana -= mana / 2;
	else
	   ch->bp -= mana / 2;
	if( ch->pcdata->learned[sn] <= 750 )
	   update_skpell( ch, sn, 0 );
	if( !ch->fighting )
	{
		if( ch->class == CLASS_CLERIC || ch->class == CLASS_DARKPRIEST )
		{
		   WAIT_STATE( ch, skill_table[sn].beats / ( get_curr_wis( ch ) / 10 ) );
		}
		else
		{  
		   WAIT_STATE( ch, skill_table[sn].beats / ( get_curr_int( ch ) / 10 ) );
		}
	}
	else
	{
		WAIT_STATE( ch, skill_table[sn].beats );
	}
    }
    else
    {
	int dam = 0;

	WAIT_STATE( ch, skill_table[sn].beats );

	if (( ch->class != 9 )&&(ch->class != 11))
	   ch->mana -= mana;
	else
	   ch->bp -= mana;
	if ( ( IS_AFFECTED2( ch, AFF_CONFUSED ) )
	    && number_percent( ) < 10 )
	{
	   act(AT_YELLOW, "$n looks around confused at what's going on.", ch, NULL, NULL, TO_ROOM );
	   send_to_char( AT_YELLOW, "You become confused and botch the spell.\n\r", ch );
	   return;
	} 

		if (ch->pcdata->learned[skill_lookup("psionic casting")])
			update_skpell(ch, skill_lookup("psionic casting"), 0);

		dam = (*skill_table[sn].spell_fun) ( sn,
				      URANGE( 1, ch->level, LEVEL_DEMIGOD ),
				      ch, vo );
		if (dam > SKPELL_NO_DAMAGE )
		{
			damage( ch, victim, dam, sn );
		} 
		else if ( dam == SKPELL_MISSED || dam == SKPELL_BOTCHED )
		{
			 // something
		}
		
		if (dam >= SKPELL_NO_DAMAGE)
			update_skpell( ch, sn, 0 );  
    }

    }
    if ( IS_NPC( ch ) )
	{
		int dam = 0;
		if ((dam = (*skill_table[sn].spell_fun) ( sn, URANGE( 1, ch->level, LEVEL_DEMIGOD ), ch, vo )) > SKPELL_NO_DAMAGE)
		{
			damage(ch, victim, dam, sn );
		}
	}

    if ( vo )
    {
      if ( skill_table[sn].target == TAR_OBJ_INV )
	oprog_cast_sn_trigger( obj, ch, sn, vo );
      rprog_cast_sn_trigger( ch->in_room, ch, sn, vo );
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

void do_quickburst( CHAR_DATA *ch, char *argument )
{
    void      *vo;
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int        mana;
    int        sn= -1;
    int        spec;
    int        intreq = 0;
    int	       intreq1 = 0;
    int        intreq2 = 0;

    spec = skill_lookup( "astral walk" );
    target_name = one_argument( argument, arg1 );
    one_argument( target_name, arg2 );

    if ( ( !IS_NPC( ch )
		&& ((ch->level < skill_table[skill_lookup("quickburst")].skill_level[ch->class] ) 
        && (ch->level < skill_table[skill_lookup("quickburst")].skill_level[ch->multied]))))
    {
        send_to_char(C_DEFAULT,
            "You'd better leave the spellcasting to the spellcasters.\n\r", ch );
        return;
    }

    if ( IS_NPC( ch ) )
      if ( IS_SET( ch->affected_by, AFF_CHARM ) )
        return;

    if ( arg1[0] == '\0' )
    {
	send_to_char(AT_BLUE, "Cast which what where?\n\r", ch );
	return;
    }

    if ( !ch->fighting )
    {
        send_to_char(C_DEFAULT, "You aren't fighting anyone.\n\r", ch );
        return;
    }

    if (!IS_NPC(ch) && (ch->pcdata->learned[skill_lookup("quickburst")] == 0))
    {
	send_to_char(C_DEFAULT, "You can't do that.\n\r", ch );
	return;
    }
    else
    {
    if ( !IS_NPC( ch ) )
    if ( ( sn = skill_lookup( arg1 ) ) < 0
	|| ((ch->level < skill_table[sn].skill_level[ch->class]) && 
           (ch->level < skill_table[sn].skill_level[ch->multied]) ))
    {
	send_to_char(AT_BLUE, "You can't do that.\n\r", ch );
	return;
    }

	if (!skill_table[sn].is_spell)
	{
		send_to_char(AT_WHITE, "Casting a skill? Try using it...\n\r", ch);
		return;
	}

    if ( !IS_NPC( ch ) )
    if ( ch->class == CLASS_BARBARIAN  || ch->multied == CLASS_BARBARIAN )
    {
        if ( skill_table[sn].skill_level[ch->class] > 60
        && skill_table[sn].skill_level[ch->multied] > 60 )
        {
            send_to_char(C_DEFAULT, "Barbarians can not cast magic of such high level.\n\r", ch );
            return;
        }
    }

    if ( !IS_NPC( ch ) )
    if ( ( ch->race == 10 && IS_EVIL( ch ) ) ||
	 ( ch->race == 10 && IS_NEUTRAL( ch ) ) ||
	 ( ch->race == 15 && IS_GOOD( ch ) ) ||
	 ( ch->race == 15 && IS_NEUTRAL( ch ) ) ||
	 ( ch->class ==   CLASS_CLERIC && IS_EVIL( ch ) ) ||
	 ( ch->multied == CLASS_CLERIC && IS_EVIL( ch ) ) ||
	 ( ch->class ==   CLASS_PALADIN && IS_EVIL( ch ) ) ||
	 ( ch->multied == CLASS_PALADIN && IS_EVIL( ch ) ) ||
	 ( ch->class ==   CLASS_ANTI_PALADIN && IS_GOOD( ch ) ) ||
	 ( ch->multied == CLASS_ANTI_PALADIN && IS_GOOD( ch ) ) ||
	 ( ch->class ==   CLASS_DARKPRIEST && IS_GOOD( ch ) ) ||
	 ( ch->multied == CLASS_DARKPRIEST && IS_GOOD( ch ) )		)
    {
	send_to_char(AT_BLUE, "You can not cast when you are that alignment.\n\r", ch );
	return;
    }
  
    if ( IS_NPC( ch ) )
     if ( ( sn = skill_lookup( arg1 ) ) < 0 )
       return;

    if ( ch->position < skill_table[sn].minimum_position )
    {
	send_to_char(AT_BLUE, "You can't concentrate enough.\n\r", ch );
	return;
    }
  
    if IS_AFFECTED2( ch, AFF_SLIT)
	{
	send_to_char(AT_BLOOD, "Your throat is slit, you can not cast.\n\r", ch );
	return;
	}

    if ( IS_STUNNED( ch, STUN_MAGIC ) )
    {
      send_to_char(AT_LBLUE, "You're too stunned to cast spells.\n\r", ch );
      return;
    }

    victim = ch->fighting;

    mana = 0;
    if ( !IS_NPC( ch ) && ch->level >= skill_table[sn].skill_level[ch->multied] )
    {
	mana = MANA_COST_MULTI( ch, sn );
        intreq1 = ( skill_table[sn].skill_level[ch->multied] / 2 ) - 5;
        if( intreq1 >= 40 )
        {
            intreq1 = 40;
        }
    }
    if ( !IS_NPC( ch ) && ch->level >= skill_table[sn].skill_level[ch->class] )
    {
	mana = MANA_COST( ch, sn );
        intreq2 = ( skill_table[sn].skill_level[ch->class] / 2 ) - 5;
        if( intreq2 >= 40 )
        {
            intreq2 = 40;
        }
    }
    intreq = UMIN( intreq1, intreq2 );
    if ( ch->class == CLASS_VAMPIRE || ch->class == CLASS_ANTI_PALADIN )
    {
       mana /= 4;
    }

    if(ch->class != CLASS_CLERIC && ch->class != CLASS_DARKPRIEST )
    {
        if( get_curr_int( ch ) < intreq )
        {
            send_to_char(AT_BLUE, "You do not have the required intelligence.\n\r", ch );
            return;
        }
    }

    if(ch->class == CLASS_CLERIC || ch->class == CLASS_DARKPRIEST)
    {
        if( get_curr_wis( ch ) < intreq )
        {
            send_to_char(AT_BLUE, "You do not have the required wisdom.\n\r", ch );
            return;
        }
    }

	update_skpell( ch, skill_lookup("quickburst"), 0 );

    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
      
    switch ( skill_table[sn].target )
    {
    default:
	bug( "Do_cast: bad target for sn %d.", sn );
	return;

    case TAR_GROUP_OFFENSIVE:
    case TAR_GROUP_DEFENSIVE:
    case TAR_GROUP_ALL:
    case TAR_GROUP_OBJ:
    case TAR_GROUP_IGNORE:
	group_cast( sn, URANGE( 1, ch->level, LEVEL_DEMIGOD ), ch, arg2 );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) )
	{
		send_to_char( AT_BLUE, "You failed.\n\r", ch );
		return;
	}

	if ( arg2[0] == '\0' )
	{
	    if ( !( victim = ch->fighting ) )
	    {
		send_to_char(AT_BLUE, "Cast the spell on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( !( victim = get_char_room( ch, arg2 ) ) )
	    {
		send_to_char(AT_BLUE, "They aren't here.\n\r", ch );
		return;
	    }
	}
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
	
	vo = (void *) victim;
	if (!pk_combat_check(ch, victim))
	  return;
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
	    send_to_char(AT_BLUE, "You cannot cast this spell on another.\n\r", ch );
	    return;
	}

	vo = (void *) ch;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char(AT_BLUE, "What should the spell be cast upon?\n\r", ch );
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
    if ( ( ch->class != 9 ) && ( ch->class != 11) && ( ch->mana < mana ) )
       {
   	send_to_char(AT_BLUE, "You don't have enough mana.\n\r", ch );
	return;
       }
    else
       if ( ( ch->bp < mana ) && (( ch->class == 9 )||( ch->class == 11)) )
       {
   	send_to_char(AT_RED, "You are to starved to cast, you must feed.\n\r", ch );
	return;
       }
         
    }
    if ( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
      {
       send_to_char( AT_BLUE, "You failed.\n\r", ch );
       return;
      }	   
    if ( ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) ) && ( skill_table[sn].target == TAR_CHAR_OFFENSIVE ) )
      {
       send_to_char( AT_BLUE, "You failed.\n\r", ch );
       return;
      }	   
    WAIT_STATE( ch, skill_table[sn].beats / 2);
      
    if ( !IS_NPC( ch ) )
    {
    if ( number_percent( ) > ( ch->pcdata->learned[sn] / 10 ) || ( number_percent( ) > ch->pcdata->learned[skill_lookup("quickburst")] / 10 ) )
    {
	send_to_char(AT_BLUE, "You lost your concentration.\n\r", ch );
	if (( ch->class != 9 )&&( ch->class  != 11))
	   ch->mana -= mana / 2;
	else
	   ch->bp -= mana / 2;
	if( ch->pcdata->learned[sn] <= 750 )
	   update_skpell( ch, sn, 0 );
    }
    else
    {

		int dmg = 0;

		if (( ch->class != 9 )&&(ch->class != 11))
			ch->mana -= mana;
		else
			ch->bp -= mana;
		if ( ( IS_AFFECTED2( ch, AFF_CONFUSED ) ) && number_percent( ) < 10 )
		{
			act(AT_YELLOW, "$n looks around confused at what's going on.", ch, NULL, NULL, TO_ROOM );
			send_to_char( AT_YELLOW, "You become confused and botch the spell.\n\r", ch );
			return;
		} 

		if (ch->pcdata->learned[skill_lookup("psionic casting")])
			update_skpell(ch, skill_lookup("psionic casting"), 0);

		say_spell( ch, sn );
		dmg = (*skill_table[sn].spell_fun) ( sn,
				      URANGE( 1, ch->level, LEVEL_DEMIGOD ),
				      ch, vo );
    
		if ( dmg > SKPELL_NO_DAMAGE )
		{
			/*
				Insert level adjustment code here
			*/
			damage(ch, victim, dmg, sn);
		} else if ( dmg == SKPELL_BOTCHED || dmg == SKPELL_MISSED )
		{
			/* do something for failed skills, maybe*/
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

    if ( vo )
    {
      if ( skill_table[sn].target == TAR_OBJ_INV )
	oprog_cast_sn_trigger( obj, ch, sn, vo );
      rprog_cast_sn_trigger( ch->in_room, ch, sn, vo );
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
}

/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim,
		    OBJ_DATA *obj )
{
    void *vo;
	int dam = 0;

    if ( sn <= 0 )
	return;
    
    if ( !is_sn(sn) || skill_table[sn].spell_fun == 0 )
    {
	bug( "Obj_cast_spell: bad sn %d.", sn );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
      {
       send_to_char( AT_BLUE, "The magic of the item fizzles.\n\r", ch );
       return;
      }	   

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_GROUP_OFFENSIVE:
    case TAR_GROUP_DEFENSIVE:
    case TAR_GROUP_ALL:
    case TAR_GROUP_OBJ:
    case TAR_GROUP_IGNORE:
	group_cast( sn, URANGE( 1, level, LEVEL_DEMIGOD ), ch,
		    victim ? (void *)victim : (void *)obj );
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
    if ( IS_SET( ch->in_room->room_flags, ROOM_NO_OFFENSIVE ) )
      {
       send_to_char( AT_BLUE, "The magic of the item fizzles.\n\r", ch );
       return;
      }	   
	if ( !victim )
	    victim = ch->fighting;
	if ( victim && victim->deleted)
		return;
/*
	if ( !victim || ( !IS_NPC( victim ) && ch != victim ) )
	{
	    send_to_char(AT_BLUE, "You can't do that.\n\r", ch );
	    return;
	}
*/

	if (!victim)
	{
		send_to_char(AT_BLUE, "You cannot find a target.\n\r", ch);
		return;
	}

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
        if ( !is_pkillable( ch, victim ) ) {
    	    return;
        }

	vo = (void *) victim;
	if (!pk_combat_check(ch, victim))
	  return;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( !victim )
	    victim = ch;
	if (!pk_combat_check(ch, victim))
	  return;
	vo = (void *) victim;
	break;

    case TAR_CHAR_SELF:
	vo = (void *) ch;
	break;

    case TAR_OBJ_INV:
	if ( !obj )
	{
	    send_to_char(AT_BLUE, "You can't do that.\n\r", ch );
	    return;
	}
	vo = (void *) obj;
	break;
    }
/*    target_name = "";*/
    dam = (*skill_table[sn].spell_fun) ( sn, level, ch, vo );

	if (dam > SKPELL_NO_DAMAGE)
	{
		damage( ch, victim, dam, sn);
	} else if( dam == SKPELL_BOTCHED || dam == SKPELL_MISSED)
	{
		// something
	}

    if ( vo )
    {
      if ( skill_table[sn].target == TAR_OBJ_INV )
	oprog_cast_sn_trigger( obj, ch, sn, vo );
      rprog_cast_sn_trigger( ch->in_room, ch, sn, vo );
    }

    if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
	&& victim->master != ch && ch != victim ) 
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

/* Control spells.  All the extra checks are JUST IN CASE... heh */

void do_control_switch( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
 
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char(AT_WHITE, "Switch into whom?\n\r", ch );
        return;
    }
    
    if ( !ch->desc )
        return;
 
    if ( ch->desc->original )
    {
        send_to_char(AT_WHITE, "You are already switched.\n\r", ch );
        return;
    }
    
    if ( !( victim = get_char_world( ch, arg ) ) )
    {
        send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
        return;
    }
    
    if ( victim == ch )
    {
        send_to_char(AT_BLUE, "Ok.\n\r", ch );
        return;
    }
     
    if ( !IS_NPC( victim ) )
    {
        send_to_char(AT_WHITE, "You cannot switch into a player!\n\r", ch );
        return;
    }
     
    if ( victim->desc )
    {
        send_to_char(AT_WHITE, "Character in use.\n\r", ch );
        return;
    }
     
    ch->pcdata->switched  = TRUE;
    ch->desc->character   = victim;
    ch->desc->original    = ch;
    victim->desc          = ch->desc;
    victim->prompt        = ch->prompt;
    victim->deaf          = ch->deaf;
    ch->desc              = NULL;
    send_to_char(AT_BLUE, "Ok.\n\r", victim );
    return;
}

/*
 * Spell functions.
 */
int spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam;

    dam = dice( level, 8 );
    if ( saves_spell( level, victim ) )
		dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_animate( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA      *obj = (OBJ_DATA *) vo;
   OBJ_DATA      *obj_next;
   
   
    if ( obj->item_type != ITEM_CORPSE_NPC )
    {
      send_to_char(AT_BLUE, "You cannot animate that.\n\r", ch );
      return SKPELL_BOTCHED;
    }
 obj_next = obj->next;
	if (obj->deleted)
		return SKPELL_MISSED;
    magic_mob( ch, obj, obj->ac_vnum );
    extract_obj(obj);
    return SKPELL_NO_DAMAGE;
}

    

int spell_armor( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
		return SKPELL_MISSED;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level / 2;
    af.location  = APPLY_AC;
    af.modifier  = -50;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "You feel someone protecting you.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int spell_astral( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    CHAR_DATA *pet;

    if ( !( victim = get_char_world( ch, target_name ) )
	|| IS_SET( victim->in_room->room_flags, ROOM_SAFE      )
	|| IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
	|| IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  )
	|| IS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL_IN )
	|| IS_SET( ch->in_room->room_flags, ROOM_NO_ASTRAL_OUT ) 
	|| IS_SET( victim->in_room->area->area_flags, AREA_PROTOTYPE )
	|| IS_AFFECTED( victim, AFF_NOASTRAL )
        || IS_AFFECTED4( victim, AFF_DECEPTION) )
    {
	send_to_char(AT_BLUE, "You failed.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PRESENT) 
	&& (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
	|| IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) ) 
    {
	send_to_char(AT_BLUE, "You can't astral walk through time.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_FUTURE) 
	&& (IS_SET( victim->in_room->area->area_flags, AREA_PRESENT )
	|| IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) ) 
    {
	send_to_char(AT_BLUE, "You can't astral walk through time.\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PAST) 
	&& (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
	|| IS_SET( victim->in_room->area->area_flags, AREA_PRESENT ) ) ) 
    {
	send_to_char(AT_BLUE, "You can't astral walk through time.\n\r", ch );
	return SKPELL_MISSED;
    }

    for ( pet = ch->in_room->people; pet; pet = pet->next_in_room )
    {
      if ( IS_NPC( pet ) )
        if ( IS_SET( pet->act, ACT_PET ) && ( pet->master == ch ) )
          break;
    }

    act(AT_BLUE, "$n vanishes in a flash of blinding light.", ch, NULL, NULL, TO_ROOM );
    if ( ch != victim )
    {
     if ( pet )
     {
       act( AT_BLUE, "$n vanishes in a flash of blinding light.", pet, NULL, NULL, TO_ROOM );
       char_from_room( pet );
     }
     char_from_room( ch );
     char_to_room( ch, victim->in_room );
    }
    act(AT_BLUE, "$n appears in a flash of blinding light.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    if ( pet )
    {
      char_to_room( pet, victim->in_room );
      act( AT_BLUE, "$n appears in a flash of blinding light.", pet, NULL, NULL, TO_ROOM );
    }
    return SKPELL_NO_DAMAGE;
}

int spell_aura( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;


    if ( IS_AFFECTED( victim, AFF_PEACE) )
		return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 5 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PEACE;
    affect_to_char( victim, &af );

    send_to_char(AT_BLUE, "You feel a wave of peace flow lightly over your body.\n\r", victim );
    act(AT_BLUE, "$n looks very peaceful.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int spell_shadow_plane( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_SHADOW_PLANE) )
		return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SHADOW_PLANE;
    affect_to_char2( victim, &af );

    send_to_char(AT_BLUE, "You phase out of reality and into the shadow realm.\n\r", victim );
    act(AT_BLUE, "$n's phases out of reality and enters the shadow plane.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}


int spell_bless( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
		return SKPELL_BOTCHED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level / 2;
    af.location  = APPLY_HITROLL;
    af.modifier  = level / 6;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 6;
    affect_to_char( victim, &af );
    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "You feel righteous.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int spell_darkbless( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
        return SKPELL_BOTCHED;

    af.type      = sn;
    af.level     = level;
    af.duration  = 40 + level;
    af.location  = APPLY_DAMROLL;
    af.modifier  = level / 5;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_HITROLL;
    af.modifier  = level / 5;
    affect_to_char( victim, &af );

    af.location  = APPLY_HIT;
    af.modifier  = level * 8;
    affect_to_char( victim, &af );

    if ( ch != victim )
        send_to_char(AT_BLUE, "You call forth the hand of oblivion.\n\r", ch );
    send_to_char(AT_BLUE, "The hand of oblivion rests upon you.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int spell_bio_acceleration( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
	return SKPELL_BOTCHED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = 40 + level;
    af.location  = APPLY_HIT;
    af.modifier  = number_fuzzy ( level * 10 );
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_MOVE;
    af.modifier  = level * 2;
    affect_to_char( victim, &af );
    
    send_to_char( AT_BLUE, "You greatly enhance your bio-functions.\n\r", ch );
    act(AT_BLUE, "$n's body shudders briefly.", ch, NULL, NULL, TO_ROOM);
    return SKPELL_NO_DAMAGE;
}



int spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || saves_spell( level, victim )
|| IS_AFFECTED2( victim, AFF_BLINDFOLD ) )
    {
	send_to_char(AT_BLUE, "You have failed.\n\r", ch );
	return SKPELL_BOTCHED;
    }

    af.type      = sn;
    af.level	 = level;
    af.duration  = level / 20;
    af.location  = APPLY_HITROLL;
    af.modifier  = -50;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );

    act(AT_WHITE, "$N is blinded!", ch, NULL, victim, TO_CHAR    );
    send_to_char(AT_WHITE, "You are blinded!\n\r", victim );
    act(AT_WHITE, "$N is blinded!", ch, NULL, victim, TO_NOTVICT );
    return SKPELL_NO_DAMAGE;
}

int spell_blood_bath( int sn, int level, CHAR_DATA *ch, void *vo )
{
     CHAR_DATA  *victim;
     
    if ( blood_count( ch->in_room->contents, 5 ) < 2 )
       {
        send_to_char( AT_RED, "There is not enough blood in the room.\n\r", ch );
        return SKPELL_MISSED;
       }
    for ( victim = ch->in_room->people; victim; victim = victim->next_in_room )
    {
        if ( victim->deleted )
	    continue;

	if ( IS_NPC( victim ) )
	    continue;
        victim->hit = UMIN( victim->hit + 500, victim->max_hit );
        update_pos( victim );
        act( AT_RED, "You bath $N in the life giving fluid.", ch, NULL, victim, TO_CHAR );
        act( AT_RED, "$n baths $N in blood.", ch, NULL, victim, TO_ROOM );
        act( AT_RED, "$n baths you in blood.", ch, NULL, victim, TO_VICT );
    }
    send_to_char( AT_RED, "The blood bath is over.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}


int spell_burning_hands( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] = 
    {
	 0,
	 0,  0,  0,  0,	14,	17, 20, 23, 26, 29,
	29, 29, 30, 30,	31,	31, 32, 32, 33, 33,
	34, 34, 35, 35,	36,	36, 37, 37, 38, 38,
	39, 39, 40, 40,	41,	41, 42, 42, 43, 43,
	44, 44, 45, 45,	46,	46, 47, 47, 48, 48,
	48, 48, 49, 49,	49,	49, 50, 50, 50, 51,
	51, 51, 52, 52,	52,	53, 53, 53, 54, 54,
	54, 54, 54, 54,	55,	55, 55, 55, 55, 55,
	56, 56, 56, 56,	56,	57, 57, 58, 59, 60
    };
                 int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim ) )
		dam /= 2;

    //damage( ch, victim, dam, sn );
    return dam;
}



int spell_call_lightning( int sn, int level, CHAR_DATA *ch, void *vo )
{
    //CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam;

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

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 7 );

    if ( !IS_OUTSIDE( ch ) )
    {
	send_to_char(AT_WHITE, "You must be out of doors.\n\r", ch );
	return SKPELL_BOTCHED;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
	send_to_char(AT_WHITE, "You need bad weather for this spell to be effective.\n\r", ch );
	dam /= 2;
    }
    if ( weather_info.sky > SKY_RAINING )
    {
	send_to_char(AT_WHITE, "The Lightning in the air increases the spells power!\n\r", ch );
	dam *= 2;
    }
    
    send_to_char(AT_WHITE, "Lightning slashes out of the sky to strike your foe!\n\r", ch );
    act(AT_WHITE, "$n calls lightning from the sky to strike $s foe!",
	ch, NULL, NULL, TO_ROOM );

    //damage( ch, victim, dam, sn );
    return dam;
}



int spell_cause_light( int sn, int level, CHAR_DATA *ch, void *vo )
{
    //damage( ch, (CHAR_DATA *) vo, dice( 5, 10 ) + level / 3, sn );

    return (dice(5,10)+level/3);
}



int spell_cause_critical( int sn, int level, CHAR_DATA *ch, void *vo )
{
    //damage( ch, (CHAR_DATA *) vo, dice( 20, 10 ) + level, sn );
    return (dice(20,10) + level);
}



int spell_cause_serious( int sn, int level, CHAR_DATA *ch, void *vo )
{
    //damage( ch, (CHAR_DATA *) vo, dice( 10, 10 ) + level, sn );
    return (dice(10,10)+level);
}



int spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_SEX;
    do
    {
	af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    af.bitvector = 0;
    affect_to_char( victim, &af );
    if ( ch != victim )
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "You feel different.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}



int spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
	send_to_char(AT_BLUE, "You like yourself even better!\n\r", ch );
	return SKPELL_MISSED;
    }

    if ( !IS_NPC( victim ) )
       return SKPELL_BOTCHED;
    if (   IS_AFFECTED( victim, AFF_CHARM )
	|| IS_AFFECTED( ch,     AFF_CHARM )
	|| level < victim->level
	|| ( saves_spell(level + IS_SRES(victim, RES_CHARM) ? -5 : 0, victim)
	&& !(get_trust( ch ) > 100) ) )
	return SKPELL_MISSED;
    
    if(IS_SIMM(victim, IMM_CHARM))
      return SKPELL_MISSED;

    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );

    send_to_char(AT_BLUE, "Ok.\n\r", ch );
    act(AT_BLUE, "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
    return SKPELL_NO_DAMAGE;
}



int spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA  *victim       = (CHAR_DATA *) vo;
		 AFFECT_DATA af;
    static const int         dam_each [ ] = 
    {
	 0,
	 0,  0,  6,  7,  8,	 9, 12, 13, 13, 13,
	14, 14, 14, 15, 15,	15, 16, 16, 16, 17,
	17, 17, 18, 18, 18,	19, 19, 19, 20, 20,
	20, 21, 21, 21, 22,	22, 22, 23, 23, 23,
	24, 24, 24, 25, 25,	25, 26, 26, 26, 27,
	27, 27, 27, 28, 28,	28, 29, 29, 29, 30,
	30, 30, 31, 31, 31,	32, 32, 33, 33, 33,
	34, 34, 34, 35, 35,	35, 36, 36, 36, 37,
	37, 37, 37, 37, 38,	38, 38, 38, 39, 39,
	39, 39, 39, 40, 40,	40, 41, 41, 42, 43
    };
		 int         dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( !saves_spell( level, victim ) )
    {
	af.type      = sn;
	af.level     = level;
	af.duration  = 6;
	af.location  = APPLY_STR;
	af.modifier  = -1;
	af.bitvector = 0;
	affect_join( victim, &af );
    }
    else
    {
	dam /= 2;
    }

    //damage( ch, victim, dam, sn );
    return dam;
}



int spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] = 
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0,  0,  0,
	30, 35, 40, 45, 50,	55, 55, 55, 56, 57,
	58, 58, 59, 60, 61,	61, 62, 63, 64, 64,
	65, 66, 67, 67, 68,	69, 70, 70, 71, 72,
	73, 73, 74, 75, 76,	76, 77, 78, 79, 79,
        79, 80, 80, 81, 81,	82, 82, 83, 83, 84,
        84, 85, 85, 86, 86,	87, 87, 88, 88, 90,
	90, 91, 91, 92, 92,	93, 93, 94, 94, 95,
	95, 96, 96, 97, 97,	98, 98, 99, 99, 100,
	100,101,102,102,103,	104,105,106,107,120
    };
		 
    int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 4 );
    if ( saves_spell( level, victim ) )
	dam /= 2;

    //damage( ch, victim, dam, sn );
    return dam;
}



int spell_continual_light( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *light;

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    obj_to_room( light, ch->in_room );

    act(AT_BLUE, "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
    act(AT_BLUE, "$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}



int spell_control_weather( int sn, int level, CHAR_DATA *ch, void *vo )
{
    if ( !str_cmp( target_name, "better" ) )
	weather_info.change += dice( level / 3, 4 );
    else if ( !str_cmp( target_name, "worse" ) )
	weather_info.change -= dice( level / 3, 4 );
    else
	send_to_char (AT_BLUE, "Do you want it to get better or worse?\n\r", ch );

    send_to_char(AT_BLUE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}



int spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
    mushroom->value[0] = 5 + level;
    obj_to_room( mushroom, ch->in_room );

    act(AT_ORANGE, "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
    act(AT_ORANGE, "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}



int spell_create_spring( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *spring;

    spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = level;
    obj_to_room( spring, ch->in_room );

    act(AT_BLUE, "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
    act(AT_BLUE, "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}



int spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj   = (OBJ_DATA *) vo;
    int       water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char(AT_BLUE, "It is unable to hold water.\n\r", ch );
	return SKPELL_BOTCHED;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
	send_to_char(AT_BLUE, "It contains some other liquid.\n\r", ch );
	return SKPELL_BOTCHED;
    }

    water = UMIN( level * ( weather_info.sky >= SKY_RAINING ? 4 : 2 ),
		 obj->value[0] - obj->value[1] );
  
    if ( water > 0 )
    {
	obj->value[2] = LIQ_WATER;
	obj->value[1] += water;
	if ( !is_name( "water", obj->name ) )
	{
	    char buf [ MAX_STRING_LENGTH ];

	    sprintf( buf, "%s water", obj->name );
	    free_string( obj->name );
	    obj->name = str_dup( buf );
	}
	act(AT_BLUE, "$p is filled.", ch, obj, NULL, TO_CHAR );
    }

    return SKPELL_NO_DAMAGE;
}



int spell_cure_blindness( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, skill_lookup("blindness") ) )
	return SKPELL_MISSED;

    affect_strip( victim, skill_lookup("blindness") );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_WHITE, "Your vision returns!\n\r", victim );
    return SKPELL_NO_DAMAGE;
}



int spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        heal;

    heal = (int)(victim->max_hit * 0.08);
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "You feel better!\n\r", victim );
    return SKPELL_NO_DAMAGE;
}



int spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        heal;

    heal = (int)(victim->max_hit * 0.02);

    heal = UMIN(heal, 400);

    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "You feel better!\n\r", victim );
    return SKPELL_NO_DAMAGE;
}



int spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, skill_lookup("poison") ) )
        return SKPELL_MISSED;

    victim->poison_level -= number_fuzzy(ch->level/10);
    if( victim->poison_level <= 0 )
    {
	victim->poison_level = 0;
	affect_strip( victim, skill_lookup("poison") );
    }

    send_to_char(AT_GREEN, "Ok.\n\r",                                    ch     );
    send_to_char(AT_GREEN, "A warm feeling runs through your body.\n\r", victim );
    act(AT_GREEN, "$N looks better.", ch, NULL, victim, TO_NOTVICT );

    return SKPELL_NO_DAMAGE;
}

int spell_create_illusion( int sn, int level, CHAR_DATA *ch, void *vo )
{
    char speaker[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim = NULL;
    char	buf[ MAX_STRING_LENGTH ];

    target_name = one_argument( target_name, speaker );
    victim = get_char_world( ch, target_name );
    if ((victim = get_char_world(ch, speaker)))
    {
      send_to_char( AT_GREY, target_name, victim );
      send_to_char( AT_GREY, "\n\r", victim );
      sprintf( buf, "Create Illusion (%s): %s: Sent to %s", ch->name, target_name, victim->name );
      log_string( buf, CHANNEL_LOG, -1 );
    }
    else
    {
     send_to_char(AT_GREY, "You do not find them.\n\r", ch);
    }
     return SKPELL_NO_DAMAGE;
}

int spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        heal;

    heal = (int)(victim->max_hit * 0.05);
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char(AT_BLUE, "You feel better!\n\r", victim );
    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}



int spell_curse( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ))
    {
	send_to_char(AT_RED, "A curse has already been inflicted.\n\r", ch );
	return SKPELL_MISSED;
    }

    if (saves_spell( level, victim )) {
	send_to_char(AT_RED, "You have failed.\n\r", ch);
	return SKPELL_BOTCHED;
    }

    af.type      = sn;
    af.level	 = level;
    af.duration  = level / 5;
    af.location  = APPLY_HITROLL;
    af.modifier  = -50;
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 100;
    affect_to_char( victim, &af );

    if ( ch != victim )
	send_to_char(AT_RED, "You have inflicted a curse.\n\r", ch );
    send_to_char(AT_RED, "You feel unclean.\n\r", victim );
    act(AT_RED, "$n has cursed $N!", ch, NULL, victim, TO_ROOM);
    return SKPELL_NO_DAMAGE;
}



int spell_detect_evil( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_DETECT_EVIL ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_EVIL;
    affect_to_char( victim, &af );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "Your eyes tingle.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}



int spell_detect_hidden( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_DETECT_HIDDEN ) )
		return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( victim, &af );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "Your awareness improves.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}



int spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_DETECT_INVIS ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( victim, &af );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "Your eyes tingle.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int spell_truesight( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED2( victim, AFF_TRUESIGHT ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_TRUESIGHT;
    affect_to_char2( victim, &af );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "Your eyes tingle.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}



int spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_DETECT_MAGIC ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char( victim, &af );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "Your eyes tingle.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}



int spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if ( obj->value[3] != 0 )
	    send_to_char(AT_GREEN, "You smell poisonous fumes.\n\r", ch );
	else
	    send_to_char(AT_GREEN, "It looks very delicious.\n\r", ch );
    }
    else
    {
	send_to_char(AT_GREEN, "It looks very delicious.\n\r", ch );
    }

    return SKPELL_NO_DAMAGE;
}

int  spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam;
  
    if ( !IS_NPC( ch ) && IS_EVIL( ch ) )
    {
	send_to_char(AT_RED, "You are too EVIL to cast this.\n\r", ch );
	return SKPELL_MISSED;
    }
  
    if ( IS_GOOD( victim ) )
    {
	act(AT_BLUE, "God protects $N.", ch, NULL, victim, TO_ROOM );
	return SKPELL_MISSED;
    }

    if ( IS_NEUTRAL( victim ) )
    {
	act(AT_BLUE, "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
	return SKPELL_MISSED;
    }

    dam = dice( level, 4 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}



int  spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;

    send_to_char(AT_ORANGE, "The earth trembles beneath your feet!\n\r", ch );
    act(AT_ORANGE, "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

	if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE))
	{
	send_to_char(AT_WHITE, "You are in a safe room!!!!", ch);
	return SKPELL_MISSED;
	}

    for ( vch = char_list; vch; vch = vch->next )
    {
        if ( vch->deleted || !vch->in_room )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && ( IS_NPC( ch ) ? !IS_NPC( vch )
			                     :  IS_NPC( vch ) ) )
		damage( ch, vch, level + dice( 2, 8 ), sn );
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char(AT_ORANGE, "The earth trembles and shivers.\n\r", vch );
    }

    return SKPELL_NO_DAMAGE;
}

int  spell_chain_lightning( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;

    send_to_char(AT_BLUE, "Bolts of electricity arc from your hands!\n\r", ch );
    act(AT_BLUE, "Electrical energy bursts from $n's hands.", ch, NULL, NULL, TO_ROOM );

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE))
	{
	send_to_char(AT_WHITE, "You are in a safe room!!!!", ch);
	return SKPELL_MISSED;
	}

    for ( vch = char_list; vch; vch = vch->next )
    {
        if ( vch->deleted || !vch->in_room )
	    continue;
	if ( vch->in_room == ch->in_room && vch != ch )
	{
		damage( ch, vch, level + dice( level, level/4 ), sn );
                       
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char(AT_BLUE, "The air fills with static.\n\r", vch );
    }

    return SKPELL_NO_DAMAGE;
}

int  spell_psychic_quake( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;

    send_to_char(AT_YELLOW, "You let the chaos free from your mind!\n\r", ch );
    act(AT_YELLOW, "$n's face becomes blank and concentrated.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch; vch = vch->next )
    {
        if ( vch->deleted || !vch->in_room )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && ( IS_NPC( ch ) ? !IS_NPC( vch )
			                     :  1 ) )
		if (vch != ch )
		    damage( ch, vch, level + dice( level, 6 ), sn );
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char(AT_BLUE, "A wave of chaos brushes your mind.\n\r", vch );
    }

    return SKPELL_NO_DAMAGE;
}

int  spell_meteor_swarm( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
   AFFECT_DATA af;
   
    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE))
	{
	send_to_char(AT_WHITE, "You are in a safe room!!!!\n\r", ch);
	return SKPELL_MISSED;
	}

    send_to_char(AT_RED, "Flaming meteors fly forth from your outstreched hands!\n\r", ch );
    act(AT_RED, "Hundreds of flaming meteors fly forth from $n's hands.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch; vch = vch->next )
    {
        if ( vch->deleted || !vch->in_room )
	    continue;
	if ( vch->in_room == ch->in_room && vch != ch && vch->position != POS_GHOST )
	{
	    damage( ch, vch, level + dice( level/2, level/2 ), sn );
            if ( vch != ch && !is_affected(vch, sn) && !IS_AFFECTED4(vch,AFF_BURROW))
            {
                af.type      = sn;
                af.level     = level;
                af.duration  = level / 20;
                af.location  = APPLY_NONE;
                af.modifier  = 0;
                af.bitvector = AFF_FLAMING;
                affect_join( vch, &af );
	        send_to_char(AT_RED, "Your body bursts into flames!\n\r", vch);
	     }
	    continue;
	}

    }

    return SKPELL_NO_DAMAGE;
}


int  spell_enchant_weapon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;

    if ( obj->item_type != ITEM_WEAPON
	|| IS_OBJ_STAT( obj, ITEM_MAGIC )
	|| obj->affected )
    {
	send_to_char(AT_BLUE, "That item cannot be enchanted.\n\r", ch );
	return SKPELL_MISSED;
    }

	paf		= new_affect();

    paf->type		= sn;
    paf->duration	= -1;
    paf->location	= APPLY_HITROLL;
    paf->modifier	= 1 + (level >= 18) + (level >= 25) + (level >= 45) + (level >= 65) +(level >= 90);
    paf->bitvector	= 0;
    paf->next		= obj->affected;
    obj->affected	= paf;

	paf		= new_affect();

    paf->type		= sn;
    paf->duration	= -1;
    paf->location	= APPLY_DAMROLL;
    paf->modifier	= 1 + (level >= 18) + (level >= 25) + (level >= 45) + (level >= 65) +(level >= 90);;
    paf->bitvector	= 0;
    paf->next		= obj->affected;
    obj->affected	= paf;

    if ( IS_GOOD( ch ) )
    {
	SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL);
	act(AT_BLUE, "$p glows.",   ch, obj, NULL, TO_CHAR );
    }
    else if ( IS_EVIL( ch ) )
    {
	SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	act(AT_RED, "$p glows",    ch, obj, NULL, TO_CHAR );
    }
    else
    {
	SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	act(AT_YELLOW, "$p glows.", ch, obj, NULL, TO_CHAR );
    }

    send_to_char(AT_BLUE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

int  spell_flame_blade( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;

    if ( obj->item_type != ITEM_WEAPON
	|| IS_OBJ_STAT( obj, ITEM_MAGIC )
	|| IS_OBJ_STAT( obj, ITEM_FLAME )
	|| obj->affected )
    {
	send_to_char(AT_RED, "That item cannot be enchanted.\n\r", ch );
	return SKPELL_BOTCHED;
    }
    SET_BIT( obj->extra_flags, ITEM_MAGIC);
    SET_BIT( obj->extra_flags, ITEM_FLAME );
    send_to_char(AT_RED, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

/*
void spell_acid_blade( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;
    
    if ( obj->item_type != ITEM_WEAPON
        || IS_OBJ_STAT( obj, ITEM_MAGIC )
        || IS_OBJ_STAT( obj, ITEM_ACID )
        || obj->affected )
    {
        send_to_char(AT_GREEN, "That item cannot be enchanted.\n\r", ch );
        return;
    }
    SET_BIT( obj->extra_flags, ITEM_MAGIC);
    SET_BIT( obj->extra_flags, ITEM_ACID );
    send_to_char(AT_GREEN, "You bestow the enchantment of acid blade.\n\r", ch );
    return;
}
*/
int  spell_dispel_blade( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;

    if ( obj->item_type != ITEM_WEAPON
	|| IS_OBJ_STAT( obj, ITEM_MAGIC )
	|| IS_OBJ_STAT2( obj, ITEM_DISPEL )
	|| obj->affected )
    {
	send_to_char(AT_RED, "That item cannot be enchanted.\n\r", ch );
	return SKPELL_BOTCHED;
    }
    SET_BIT( obj->extra_flags, ITEM_MAGIC);
    SET_BIT( obj->extra_flags2, ITEM_DISPEL );
    send_to_char(AT_RED, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

int spell_chaos_blade( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;

    if ( obj->item_type != ITEM_WEAPON
	|| IS_OBJ_STAT( obj, ITEM_MAGIC )
	|| IS_OBJ_STAT( obj, ITEM_CHAOS )
	|| obj->affected )
    {
	send_to_char(AT_YELLOW, "That item cannot be enchanted.\n\r", ch );
	return SKPELL_BOTCHED;
    }
    SET_BIT( obj->extra_flags, ITEM_MAGIC);
    SET_BIT( obj->extra_flags, ITEM_CHAOS );
    send_to_char(AT_YELLOW, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

int  spell_frost_blade( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;

    if ( obj->item_type != ITEM_WEAPON
	|| IS_OBJ_STAT( obj, ITEM_MAGIC )
	|| IS_OBJ_STAT( obj, ITEM_ICY )
	|| obj->affected )
    {
	send_to_char(AT_LBLUE, "That item cannot be enchanted.\n\r", ch );
	return SKPELL_BOTCHED;
    }
    SET_BIT( obj->extra_flags, ITEM_MAGIC);
    SET_BIT( obj->extra_flags, ITEM_ICY );
    send_to_char(AT_LBLUE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

int  spell_holysword( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;

    if ( obj->item_type != ITEM_WEAPON
	|| IS_OBJ_STAT( obj, ITEM_MAGIC )
	|| obj->affected )
    {
	send_to_char(AT_BLUE, "That item cannot be consecrated.\n\r", ch );
	return SKPELL_BOTCHED;
    }

	paf		= new_affect();

    paf->type		= sn;
    paf->duration	= -1;
    paf->location	= APPLY_HITROLL;
    paf->modifier	= 16 + (level >= 18) + (level >= 25) + (level >= 40) + (level >= 60) +(level >= 90);
    paf->bitvector	= 0;
    paf->next		= obj->affected;
    obj->affected	= paf;

	paf		= new_affect();

    paf->type		= sn;
    paf->duration	= -1;
    paf->location	= APPLY_DAMROLL;
    paf->modifier	= 16 + (level >= 18) + (level >= 25) + (level >= 45) + (level >= 65) +(level >= 90);;
    paf->bitvector	= 0;
    paf->next		= obj->affected;
    obj->affected	= paf;

	paf		= new_affect();
    paf->type		= sn;
    paf->duration	= -1;
    paf->location	= APPLY_HIT;
    paf->modifier	= 90 + ( (level >=18) * 10) + ( (level >=25) * 10) + ( (level >= 45) * 10) + ( (level >= 65) * 10) + ( ( level >= 90) * 10);
    paf->bitvector	= 0;
    paf->next		= obj->affected;
    obj->affected	= paf;

    obj->ac_type	= 5;
    obj->ac_spell	= skill_table[skill_lookup("holy enchantment")].name;
    obj->ac_charge[0]	= 1;
    obj->ac_charge[1]	= 1;

    if ( IS_GOOD( ch ) )
    {
	SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL);
	act(AT_YELLOW, "$p glows.",   ch, obj, NULL, TO_CHAR );
    }
    else if ( IS_EVIL( ch ) )
    {
	SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	act(AT_RED, "$p glows",    ch, obj, NULL, TO_CHAR );
    }
    else
    {
	SET_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
	SET_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
	act(AT_WHITE, "$p glows.", ch, obj, NULL, TO_CHAR );
    }

    if( ch->multied == CLASS_MAGE || ch->class == CLASS_MAGE ) { SET_BIT( obj->extra_flags3, ITEM_PRO_MAGE ); }
    if( ch->multied == CLASS_CLERIC || ch->class == CLASS_CLERIC ) { SET_BIT( obj->extra_flags3, ITEM_PRO_CLERIC ); }
    if( ch->multied == CLASS_THIEF || ch->class == CLASS_THIEF ) { SET_BIT( obj->extra_flags3, ITEM_PRO_THIEF ); }
    if( ch->multied == CLASS_WARRIOR || ch->class == CLASS_WARRIOR ) { SET_BIT( obj->extra_flags3, ITEM_PRO_WARRIOR ); }
    if( ch->multied == CLASS_PSIONICIST || ch->class == CLASS_PSIONICIST ) { SET_BIT( obj->extra_flags3, ITEM_PRO_PSI ); }
    if( ch->multied == CLASS_DRUID || ch->class == CLASS_DRUID ) { SET_BIT( obj->extra_flags3, ITEM_PRO_DRUID ); }
    if( ch->multied == CLASS_RANGER || ch->class == CLASS_RANGER ) { SET_BIT( obj->extra_flags3, ITEM_PRO_RANGER ); }
    if( ch->multied == CLASS_PALADIN || ch->class == CLASS_PALADIN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_PALADIN ); }
    if( ch->multied == CLASS_BARD || ch->class == CLASS_BARD ) { SET_BIT( obj->extra_flags3, ITEM_PRO_BARD ); }
    if( ch->multied == CLASS_VAMPIRE || ch->class == CLASS_VAMPIRE ) { SET_BIT( obj->extra_flags3, ITEM_PRO_VAMP ); }
    if( ch->multied == CLASS_WEREWOLF || ch->class == CLASS_WEREWOLF ) { SET_BIT( obj->extra_flags3, ITEM_PRO_WEREWOLF ); }
    if( ch->multied == CLASS_ANTI_PALADIN || ch->class == CLASS_ANTI_PALADIN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_ANTIPAL ); }
    if( ch->multied == CLASS_ASSASSIN || ch->class == CLASS_ASSASSIN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_ASSASSIN ); }
    if( ch->multied == CLASS_MONK || ch->class == CLASS_MONK ) { SET_BIT( obj->extra_flags3, ITEM_PRO_MONK ); }
    if( ch->multied == CLASS_BARBARIAN || ch->class == CLASS_BARBARIAN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_BARBARIAN ); }
    if( ch->multied == CLASS_ILLUSIONIST || ch->class == CLASS_ILLUSIONIST ) { SET_BIT( obj->extra_flags3, ITEM_PRO_ILLUSIONIST ); }
    if( ch->multied == CLASS_NECROMANCER || ch->class == CLASS_NECROMANCER ) { SET_BIT( obj->extra_flags3, ITEM_PRO_NECROMANCER ); }
    if( ch->multied == CLASS_DEMONOLOGIST || ch->class == CLASS_DEMONOLOGIST ) { SET_BIT( obj->extra_flags3, ITEM_PRO_DEMONOLOGIST ); }
    if( ch->multied == CLASS_SHAMAN || ch->class == CLASS_SHAMAN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_SHAMAN ); }
    if( ch->multied == CLASS_DARKPRIEST || ch->class == CLASS_DARKPRIEST ) { SET_BIT( obj->extra_flags3, ITEM_PRO_DARKPRIEST ); }

    send_to_char(AT_BLUE, "Ok.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
int  spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam;

    if ( saves_spell( level, victim ) )
	return SKPELL_ZERO_DAMAGE;

    if ( victim->level <= 2 )
    {
	dam		 = ch->hit + 1;
    }
    else
    {
	dam		 = dice( 4, level );
	if ( ( ch->hit + dam ) > ( ch->max_hit + 200 ) ) 
	    ch->hit = ( ch->max_hit + 200 );
	 else
	  ch->hit		+= dam;

	if (victim->move - dam >= 0)
		victim->move -= dam;
	else
		victim->move = 0;


	if (ch->move + dam > ch->max_move + 200)
		ch->move = ch->move + 200;
	else
		ch->move += dam;
    }

    //damage( ch, victim, dam, sn );

    return dam;
}

int  spell_farsight( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
   ROOM_INDEX_DATA *blah;
    
    if ( !( victim = get_char_world( ch, target_name ) )
	|| IS_SET( victim->in_room->room_flags, ROOM_SAFE      )
	|| IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
	|| IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  )
	|| IS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL_IN )
	|| IS_SET( ch->in_room->room_flags, ROOM_NO_ASTRAL_OUT ) )
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

int  spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo )
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
    dam	     = number_range( dam_each[level] / 2, dam_each[level] * 6 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int  spell_molecular_unbind( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim       = (CHAR_DATA *) vo;
    OBJ_DATA  *obj_lose;
    OBJ_DATA  *obj_next;
   
    if(saves_spell ( level, victim ))
    {
	send_to_char(AT_BLUE, "You failed.\n\r", ch );
	return SKPELL_MISSED;
    }
     
    for ( obj_lose = victim->carrying; obj_lose; obj_lose = obj_next )
    {
	char *msg;

	obj_next = obj_lose->next_content;
	if ( obj_lose->deleted )
	    continue;
	if ( obj_lose->wear_loc == WEAR_NONE )
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
	      msg = "Your $p shatters!";
	      extract_obj( obj_lose );
	      break;
            case ITEM_WEAPON:
	    case ITEM_ARMOR:
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
		extract_obj( obj_lose );
 		obj_to_room ( pObj, victim->in_room );
              	    
		act(AT_YELLOW, msg, victim, obj_lose, NULL, TO_CHAR );
		act(AT_YELLOW, "You destroyed $p!", ch, obj_lose, NULL, TO_CHAR );

		break;
	    }
	}
        return SKPELL_NO_DAMAGE;
    }

    return SKPELL_NO_DAMAGE;
}

int  spell_shatter( int sn, int level, CHAR_DATA *ch, void *vo )
{
   CHAR_DATA *victim       = (CHAR_DATA *) vo;
   OBJ_DATA  *obj_lose;
   OBJ_DATA  *obj_next;
   
    if(saves_spell ( level, victim ))
    {
	send_to_char(AT_BLUE, "You failed.\n\r", ch );
	return SKPELL_MISSED;
    }
     
    for ( obj_lose = victim->carrying; obj_lose; obj_lose = obj_next )
    {
	char *msg;

	obj_next = obj_lose->next_content;
	if ( obj_lose->deleted )
	    continue;
	if ( obj_lose->wear_loc == WEAR_NONE )
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
	      msg = "Your $p shatters!";
	      extract_obj( obj_lose );
	      break;
            case ITEM_WEAPON:
	    case ITEM_ARMOR:
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
		extract_obj( obj_lose );
		obj_to_room ( pObj, victim->in_room );

		act(AT_YELLOW, msg, victim, obj_lose, NULL, TO_CHAR );
		act(AT_YELLOW, "You destroyed $p!", ch, obj_lose, NULL, TO_CHAR );

		    break;
	    }
        }
        return SKPELL_NO_DAMAGE;
    }

    return SKPELL_NO_DAMAGE;
}
 
int  spell_fireshield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_FIRESHIELD ) )
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
/*    af.duration  = number_fuzzy( level / 8 );*/
    af.duration	 = -1;
    af.location  = APPLY_DAM_WATER;
    af.modifier  = 75;
    af.bitvector = AFF_FIRESHIELD;
    affect_to_char( victim, &af );
    victim->shields += 1;

    send_to_char(AT_RED, "Your body is engulfed by unfelt flame.\n\r", victim );
    act(AT_RED, "$n's body is engulfed in flames.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int  spell_demonshield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED3( victim, AFF_DEMONSHIELD ) )
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
    af.bitvector = AFF_DEMONSHIELD;
    affect_to_char3( victim, &af );
    victim->shields += 1;

    send_to_char(AT_RED, "Your body is engulfed by swirling demons.\n\r",
victim );
    act(AT_RED, "$n's body is engulfed by demons.", victim, NULL, NULL,
TO_ROOM );
    return SKPELL_NO_DAMAGE;
}
int  spell_acid_shield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED3( victim, AFF_ACIDSHIELD ) )
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
	return SKPELL_NO_DAMAGE;

    af.type      = sn;
    af.level	 = level;
    af.duration  = -1;
    af.location  = APPLY_DAM_ACID;
    af.modifier  = 30;
    af.bitvector = AFF_ACIDSHIELD;
    affect_to_char3( victim, &af );
    victim->shields += 1;

    send_to_char(AT_GREEN, "Your body is surrounded by bubbling acid.\n\r", victim );
    act(AT_GREEN, "$n's body is drenched in bubbling acid.", victim, NULL,
NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}


int  spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam;

    dam = dice( 6, level/2 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}



int  spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_FAERIE_FIRE ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );

    af.location  = APPLY_HITROLL;
    af.modifier  = 0 - level/10;
    affect_to_char( victim, &af );
    
    send_to_char(AT_PINK, "You are surrounded by a pink outline.\n\r", victim );
    act(AT_PINK, "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}



int  spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *ich;

    send_to_char(AT_PURPLE, "You conjure a cloud of purple smoke.\n\r", ch );
    act(AT_PURPLE, "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );

    for ( ich = ch->in_room->people; ich; ich = ich->next_in_room )
    {
	if ( !IS_NPC( ich ) && IS_SET( ich->act, PLR_WIZINVIS ) )
	    continue;

	if ( ich == ch || saves_spell( level, ich ) )
	    continue;

	affect_strip ( ich, skill_lookup("invis")			);
	affect_strip ( ich, skill_lookup("improved invis")	);
	affect_strip ( ich, skill_lookup("mass invis")		);
	affect_strip ( ich, skill_lookup("sneak")			);
	affect_strip ( ich, skill_lookup("shadow")			);
	affect_strip ( ich, skill_lookup("phase shift")		);
	affect_strip ( ich, skill_lookup("shadow plane")	);
	REMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
	REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE	);
	REMOVE_BIT   ( ich->affected_by2, AFF_IMPROVED_INVIS	);
	REMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
	REMOVE_BIT   ( ich->affected_by2, AFF_SHADOW_PLANE );
	REMOVE_BIT   ( ich->affected_by2, AFF_PHASED    );
	
	act(AT_PURPLE, "$n is revealed!", ich, NULL, NULL, TO_ROOM );
	send_to_char(AT_PURPLE, "You are revealed!\n\r", ich );
    }

    return SKPELL_NO_DAMAGE;
}



int  spell_fly( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected( victim, sn ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level + 3;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );

    send_to_char(AT_BLUE, "Your feet rise off the ground.\n\r", victim );
    act(AT_BLUE, "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}



int   spell_gate( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *gch;
    int        npccount  = 0;
    int        pccount   = 0;

    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
        if ( IS_NPC( gch ) && !IS_AFFECTED( gch, AFF_CHARM ) )
	    npccount++;
	if ( !IS_NPC( gch ) ||
	    ( IS_NPC( gch ) && IS_AFFECTED( gch, AFF_CHARM ) ) )
	    pccount++;
    }

    if ( npccount > pccount )
    {
	do_say( ch, "There are too many of us here!  One must die!" );
        return SKPELL_NO_DAMAGE;
    }

    do_say( ch, "Come brothers!  Join me in this glorious bloodbath!" );
    char_to_room( create_mobile( get_mob_index( MOB_VNUM_DEMON1 ) ),
		 ch->in_room );
    return SKPELL_NO_DAMAGE;
}



/*
 * Spell for mega1.are from Glop/Erkenbrand.
 */
int   spell_general_purpose( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam;

    dam = number_range( 25, 100 );
    if ( saves_spell( level, victim ) )
		dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int   spell_giant_strength( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32) + (level >= 39) + (level >=46) + (level >= 70) + (level >= 100);
    af.bitvector = 0;
    affect_to_char( victim, &af );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "You feel stronger.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int   spell_titan_strength( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;   
   
    if ( is_affected( victim, sn ) )
        return SKPELL_MISSED;

    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_STR;
    af.modifier  = ch->level/7;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    if ( ch != victim )
        send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "You feel much stronger.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}

int   spell_goodberry( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;
   OBJ_DATA     *berry;    

    if ( obj->item_type != ITEM_FOOD
	|| IS_OBJ_STAT( obj, ITEM_MAGIC ) )
    {
	send_to_char(AT_BLUE, "You can do nothing to that item.\n\r", ch );
	return SKPELL_MISSED;
    }

    act(AT_BLUE, "You pass your hand over $p slowly.", ch, obj, NULL, TO_CHAR );
    act(AT_BLUE, "$n has created a goodberry.", ch, NULL, NULL, TO_ROOM );
    berry = create_object( get_obj_index( OBJ_VNUM_BERRY ), 0 );
    berry->timer = ch->level;
    berry->value[0] = ch->level * 2;
    berry->value[1] = ch->level * 5;
    extract_obj( obj );
    obj_to_char( berry, ch );
    return SKPELL_NO_DAMAGE;
}


int   spell_harm( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int        dam;

    dam = UMAX(  20, victim->hit - dice( 1,4 ) );
    if ( saves_spell( level, victim ) )
	dam = UMIN( 50, dam / 4 );
    dam = UMIN( 175, dam );
    //damage( ch, victim, dam, sn );
    return dam;
}



int   spell_heal( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = (int)(victim->max_hit * 0.12);
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );

    if ( ch != victim )
	send_to_char(AT_BLUE, "Ok.\n\r", ch );
    send_to_char(AT_BLUE, "A warm feeling fills your body.\n\r", victim );
    return SKPELL_NO_DAMAGE;
}



/*
 * Spell for mega1.are from Glop/Erkenbrand.
 */
int   spell_high_explosive( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range( 30, 120 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int   spell_iceshield( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_ICESHIELD ) )
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
    af.location  = APPLY_DAM_FIRE;
    af.modifier  = 75;
    af.bitvector = AFF_ICESHIELD;
    affect_to_char( victim, &af );
    victim->shields += 1;

    send_to_char(AT_LBLUE, "An Icy crust forms about your body.\n\r", victim );
    act(AT_LBLUE, "An icy crust forms about $n's body.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}

int   spell_icestorm( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
                 int     dam;

    dam = dice( level, 10 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}
int   spell_holy_fires( int sn, int level, CHAR_DATA *ch, void *vo )
{
                 CHAR_DATA *victim       = (CHAR_DATA *) vo;
                 int     dam;

    dam = dice( level, 20 );
    if ( saves_spell( level, victim ) )
	dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int   spell_identify( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA    *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    char         buf [ MAX_STRING_LENGTH * 4];
    int          spn;

    sprintf( buf,
	    "Object '%s' is type %s, extra flags %s %s %s %s.\n\r",
	    obj->name,
	    item_type_name( obj ),
	    extra_bit_name( obj->extra_flags ),
	    extra_bit_name2( obj->extra_flags2 ),
	    extra_bit_name3( obj->extra_flags3 ),
	    extra_bit_name4( obj->extra_flags4 ));
    send_to_char(AT_CYAN, buf, ch );
    sprintf( buf,
	    "Weight : %d, value : %d, level : %d.\n\r",
	    obj->weight,
	    obj->cost,
	    obj->level );
    send_to_char(AT_CYAN, buf, ch );
    sprintf( buf, "Durability: [%d/%d]\n\r", obj->durability_cur, obj->durability_max);
    send_to_char(AT_CYAN, buf, ch );

    switch ( obj->item_type )
    {
    case ITEM_RUNE:
	sprintf( buf, "Destination: Unmarked.\n\r" );
	if( obj->value[0] != 0 )
	{

	if (get_room_index(obj->value[0]))
	    sprintf( buf, "Destination: %s.\n\r", get_room_index(obj->value[0] )->name );
	else
	sprintf( buf, "Destination: Unknown Location.\n\r" ); 
	}
	send_to_char(AT_CYAN, buf, ch );
	break;

    case ITEM_PILL:  
    case ITEM_SCROLL: 
    case ITEM_POTION:
	sprintf( buf, "Level %d spells of:", obj->value[0] );
	send_to_char(AT_CYAN, buf, ch );

	if ( is_sn(obj->value[1]) )
	{
	    send_to_char(AT_CYAN, " '", ch );
	    send_to_char(AT_WHITE, skill_table[obj->value[1]].name, ch );
	    send_to_char(AT_CYAN, "'", ch );
	}

	if ( is_sn(obj->value[2]) )
	{
	    send_to_char(AT_CYAN, " '", ch );
	    send_to_char(AT_WHITE, skill_table[obj->value[2]].name, ch );
	    send_to_char(AT_CYAN, "'", ch );
	}

	if ( is_sn(obj->value[3]) )
	{
	    send_to_char(AT_CYAN, " '", ch );
	    send_to_char(AT_WHITE, skill_table[obj->value[3]].name, ch );
	    send_to_char(AT_CYAN, "'", ch );
	}

	send_to_char(AT_CYAN, ".\n\r", ch );
	break;

    case ITEM_BOOK:
	sprintf( buf, "Gain %3.1f%% of:", obj->value[0] / 10.0f );
	send_to_char(AT_CYAN, buf, ch );

	if ( is_sn(obj->value[1]) )
	{
	    send_to_char(AT_CYAN, " '", ch );
	    send_to_char(AT_WHITE, skill_table[obj->value[1]].name, ch );
	    send_to_char(AT_CYAN, "'", ch );
	}

	if ( is_sn(obj->value[2]) )
	{
	    send_to_char(AT_CYAN, " '", ch );
	    send_to_char(AT_WHITE, skill_table[obj->value[2]].name, ch );
	    send_to_char(AT_CYAN, "'", ch );
	}

	if ( is_sn(obj->value[3]) )
	{
	    send_to_char(AT_CYAN, " '", ch );
	    send_to_char(AT_WHITE, skill_table[obj->value[3]].name, ch );
	    send_to_char(AT_CYAN, "'", ch );
	}

	send_to_char(AT_CYAN, ".\n\r", ch );
	break;

	case ITEM_SKIN:
	sprintf(buf, "Leather: %d yards\n\r", obj->value[1]);
	send_to_char(AT_CYAN, buf, ch);
	sprintf(buf, "Quality: %d (%s)", obj->value[0], flag_string(quality_flags, obj->value[0]));
	send_to_char(AT_CYAN, buf, ch);
	break;
    case ITEM_BULLET:
    case ITEM_BOLT:
    case ITEM_ARROW:
	sprintf(buf, "Remaining: %d.  ", obj->value[0]);
	send_to_char(AT_CYAN, buf, ch );
	sprintf(buf, "Damage Class:  %s.  ", flag_string( damage_flags, obj->value[1]) );
	send_to_char(AT_CYAN, buf, ch );
	sprintf(buf, "Damage Bonus: %d to %d (average %d).\n\r", obj->value[2], obj->value[3], ( (obj->value[2] + obj->value[3]) / 2 ) );
	send_to_char(AT_CYAN, buf, ch );
	break;

    case ITEM_WAND: 
    case ITEM_LENSE:
    case ITEM_STAFF: 
	if (!(obj->value[1] == -1 ) )
	    sprintf( buf, "Has %d(%d) charges of level %d",
		   obj->value[1], obj->value[2], obj->value[0] );
	else 
	    sprintf( buf, "Has unlimited charges of level %d", obj->value[0] );
	
	send_to_char(AT_CYAN, buf, ch );
      
	if ( is_sn(obj->value[3]) )
	{
	    send_to_char(AT_CYAN, " '", ch );
	    send_to_char(AT_WHITE, skill_table[obj->value[3]].name, ch );
	    send_to_char(AT_CYAN, "'", ch );
	}

	send_to_char(AT_CYAN, ".\n\r", ch );
	break;

    case ITEM_GUN:
	if (!(obj->value[1] == -1 ) )
	    sprintf( buf, "Has %d(%d) shots of level %d",
		obj->value[1], obj->value[2], obj->value[0] );
	else
	    sprintf( buf, "Has unlimited shots of level %d", obj->value[0] );
	send_to_char(AT_CYAN, buf, ch );

	if ( is_sn(obj->value[3]) )
	{
	    send_to_char(AT_CYAN, " '", ch );
	    send_to_char(AT_WHITE, skill_table[obj->value[3]].name, ch );
	    send_to_char(AT_CYAN, "'", ch );
	}

	send_to_char(AT_CYAN, ".\n\r", ch );
	break;

    case ITEM_WEAPON:
	sprintf( buf, "Damage is %d to %d (average %d).\n\r",
		obj->value[1], obj->value[2],
		( obj->value[1] + obj->value[2] ) / 2 );
	send_to_char(AT_RED, buf, ch );
	break;

    case ITEM_ARMOR:
	sprintf( buf, "Armor class is %d.\n\r", obj->value[0] );
	send_to_char(AT_CYAN, buf, ch );
	break;
    }
    if ( obj->ac_type != 0 )
    {
      switch( obj->ac_type )
      {
       default:  send_to_char(AT_CYAN, "Invoke Type Unknown.\n\r", ch ); break;
       case 1 :
         {
           if ( obj->ac_charge[1] != -1 )
              sprintf( buf, "Object creation invoke \
              , with [%d/%d] charges.\n\r",
                    obj->ac_charge[0], obj->ac_charge[1] );
           else
              sprintf( buf, "Object creation invoke, with unlimited charges.\n\r" );
           send_to_char(AT_CYAN, buf, ch );
           break;
         }
       case 2 :
         {
           if ( obj->ac_charge[1] != -1 )
              sprintf( buf, "Monster creation invoke, with [%d/%d] charges.\n\r",
                    obj->ac_charge[0], obj->ac_charge[1] );
           else
              sprintf( buf, "Monster creation invoke, with unlimited charges.\n\r" );
           send_to_char(AT_CYAN, buf, ch );
           break;        
         }
       case 3 :
         {
           if ( obj->ac_charge[1] != -1 )
              sprintf( buf, "Transfer invoke, with [%d/%d] charges.\n\r",
                    obj->ac_charge[0], obj->ac_charge[1] );
           else
              sprintf( buf, "Transfer invoke, with unlimited charges.\n\r" );
           send_to_char(AT_CYAN, buf, ch );
           break;
         }
       case 4 :
         {
           if ( obj->ac_charge[1] != -1 )
              sprintf( buf, "Object morph invoke, with [%d/%d] charges.\n\r",
                    obj->ac_charge[0], obj->ac_charge[1] );
           else
              sprintf( buf, "Object morph invoke, with unlimited charges.\n\r" );
           send_to_char(AT_CYAN, buf, ch );
           break;
         }
       case 5 :
         {
           if ( obj->ac_charge[1] != -1 )
              sprintf( buf, "Spell invoke, has [%d/%d] charges of ",
                    obj->ac_charge[0], obj->ac_charge[1] );
           else
              sprintf( buf, "Spell invoke, with unlimited charges of " );
           send_to_char(AT_CYAN, buf, ch );
	   spn = skill_lookup( obj->ac_spell );
	   if ( is_sn(spn) )
	   {
	    send_to_char(AT_CYAN, " '", ch );
	    send_to_char(AT_WHITE, spn ? obj->ac_spell : "(none)", ch );
	    send_to_char(AT_CYAN, "'\n\r", ch );
	   }
	   break;
         }
      }   
    } 
    for ( paf = obj->pIndexData->affected; paf; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
	    sprintf( buf, "Affects %s by %d.\n\r",
		    affect_loc_name( paf->location ), paf->modifier );
	    send_to_char(AT_BLUE, buf, ch );
	}
    }

    for ( paf = obj->affected; paf; paf = paf->next )
    {
	if ( paf->location != APPLY_NONE && paf->modifier != 0 )
	{
		char buf2[MAX_STRING_LENGTH];
		
		if (paf->location > PERM_SPELL_BEGIN)
			strcpy(buf2, "Affects '%s' by %d.\n\r");
		else
			strcpy(buf2, "Affects %s by %d.\n\r");

		sprintf( buf, buf2, affect_loc_name( paf->location ), paf->modifier );
	    send_to_char(AT_BLUE, buf, ch );
	}
    }

    return SKPELL_NO_DAMAGE;
}

int   spell_inertial( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_INERTIAL ) )
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
    af.bitvector = AFF_INERTIAL;
    affect_to_char( victim, &af );
    victim->shields += 1;

    send_to_char(AT_LBLUE, "You set up a complex set of vibrations around your body.\n\r", victim );
    act(AT_LBLUE, "$n's body begins to vibrate.", victim, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}


int   spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA  *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_INFRARED ) )
	return SKPELL_MISSED;

    af.type      = sn;
    af.level	 = level;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );

    send_to_char(AT_RED, "Your eyes glow.\n\r", victim );
    act(AT_RED, "$n's eyes glow.\n\r", ch, NULL, NULL, TO_ROOM );
    return SKPELL_NO_DAMAGE;
}
