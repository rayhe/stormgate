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

/*$Id: act_wiz2.c,v 1.12 2005/03/17 02:41:08 tyrion Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <unistd.h>
#include "merc.h"

#ifdef RUN_AS_WIN32SERVICE
#define strncasecmp _strnicmp
#endif

char    *initial        args( ( const char *str ) );

void do_slookup( CHAR_DATA *ch, char *argument )
{
 /*   CHAR_DATA *rch; */
    char       buf  [ MAX_STRING_LENGTH ];
    char       buf1 [ MAX_STRING_LENGTH*3];
    char       arg  [ MAX_INPUT_LENGTH ];
    int        sn;
    char       temp [ MAX_STRING_LENGTH ];
    char       tempp [ MAX_STRING_LENGTH ];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Slookup what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	send_to_char(AT_WHITE, "ALL is disabled.  Use spell/skill names.\n\r", ch );
	return;

        buf1[0] = '\0';
	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( !skill_table[sn].name )
		break;
	    sprintf( buf, "Slot: %4d Sn: %4d Skill/spell: '%s'\n\r\n\r",
		    skill_table[sn].slot, sn, skill_table[sn].name );
	    strcat( buf1, buf );
	}
	send_to_char(AT_WHITE, buf1, ch );
    }
    else
    {
        int ccount;
	int l;

	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char(AT_WHITE, "No such skill or spell.\n\r", ch );
	    return;
	}

	sprintf( buf, "Slot: %4d Sn: %4d Skill/spell: '%s'\n\r",
		skill_table[sn].slot, sn, skill_table[sn].name );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( temp, "Yes" );
	sprintf( tempp,  "Yes");
	if( skill_table[sn].dispel_bit == DISPEL_NO )
	{
	    sprintf( temp, "No" );
	}
	if( skill_table[sn].cancel_bit == CANCEL_NO )
	{
	    sprintf( tempp, "No" );
	}
	sprintf( buf, "Dispellable: %s    Cancellable: %s\n\r", temp, tempp );
	send_to_char(AT_WHITE, buf, ch );
	sprintf( temp, "Yes" );
	sprintf( tempp, "Yes") ;
	if( skill_table[sn].shield_bit == SHIELD_NO )
	{
		sprintf(tempp, "No" );
	}
	if( skill_table[sn].damclass == DAMCLASS_NULL ) { sprintf(temp, "Null"); }
	if( skill_table[sn].damclass == DAMCLASS_ACID ) { sprintf(temp, "Acid"); }
	if( skill_table[sn].damclass == DAMCLASS_HOLY ) { sprintf(temp, "Holy"); }
	if( skill_table[sn].damclass == DAMCLASS_MAGIC ) { sprintf(temp, "Magic"); }
	if( skill_table[sn].damclass == DAMCLASS_FIRE ) { sprintf(temp, "Fire"); }
	if( skill_table[sn].damclass == DAMCLASS_ENERGY ) { sprintf(temp, "Energy"); }
	if( skill_table[sn].damclass == DAMCLASS_WIND ) { sprintf(temp, "Wind"); }
	if( skill_table[sn].damclass == DAMCLASS_WATER ) { sprintf(temp, "Water"); }
	if( skill_table[sn].damclass == DAMCLASS_ILLUSION ) { sprintf(temp, "Illusion"); }
	if( skill_table[sn].damclass == DAMCLASS_DISPEL ) { sprintf(temp, "Dispel"); }
	if( skill_table[sn].damclass == DAMCLASS_EARTH ) { sprintf(temp, "Earth"); }
	if( skill_table[sn].damclass == DAMCLASS_PSYCHIC ) { sprintf(temp, "Psychic"); }
	if( skill_table[sn].damclass == DAMCLASS_POISON ) { sprintf(temp, "Poison"); }
	if( skill_table[sn].damclass == DAMCLASS_BREATH ) { sprintf(temp, "Breath"); }
	if( skill_table[sn].damclass == DAMCLASS_SUMMON ) { sprintf(temp, "Summon"); }
	if( skill_table[sn].damclass == DAMCLASS_PHYSICAL ) { sprintf(temp, "Physical"); }
	if( skill_table[sn].damclass == DAMCLASS_EXPLOSIVE ) { sprintf(temp, "Explosive"); }
	if( skill_table[sn].damclass == DAMCLASS_SONG ) { sprintf(temp, "Song"); }
	if( skill_table[sn].damclass == DAMCLASS_NAGAROM ) { sprintf(temp, "Nagarom"); }
	if( skill_table[sn].damclass == DAMCLASS_UNHOLY ) { sprintf(temp, "Unholy"); }
	if( skill_table[sn].damclass == DAMCLASS_CLAN ) { sprintf(temp, "Clan"); }
	sprintf( buf, "Shield: %s   Damage class: %s\n\r", tempp, temp );
	send_to_char(AT_WHITE, buf, ch) ;
	
	if (skill_table[sn].is_spell)
		sprintf(temp, "Spell");
	else
		sprintf(temp, "Skill");

	strcpy(tempp, "");

	if (skill_table[sn].craftable == CRAFT_NONE)
		strcat(tempp, "None");	
	else
	{
	if (IS_SET(skill_table[sn].craftable, CRAFT_POTION)) { strcat( tempp, "Potion "); }
	if (IS_SET(skill_table[sn].craftable, CRAFT_SCROLL)) { strcat( tempp, "Scroll "); }
	if (IS_SET(skill_table[sn].craftable, CRAFT_ARMOR))  { strcat( tempp, "Armor " ); }
	if (IS_SET(skill_table[sn].craftable, CRAFT_WEAPON)) { strcat( tempp, "Weapon "); }
	}

	sprintf( buf, "Type: %s  Craftable: %s\n\r\n\r", temp, tempp);
	send_to_char(AT_WHITE, buf, ch);

	l = 0;

	do_help( ch, skill_table[sn].name );
	send_to_char(AT_WHITE, "\n\r", ch );
	for ( ccount = 0; ccount < MAX_CLASS; ccount++ )
	{
	  sprintf( buf, "%s: %3d   ", class_table[ccount].who_name,
		  skill_table[sn].skill_level[ccount] );
	  l += strlen( buf );
	  if( l > 79 )
	  {
	    send_to_char(C_DEFAULT, "\n\r", ch );
	    l = strlen( buf );
	  }
	  send_to_char(AT_WHITE, buf, ch );
	}
	send_to_char(AT_WHITE, "\n\r", ch );
    }

    return;
}



void do_sset( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg1  [ MAX_INPUT_LENGTH ];
    char       arg2  [ MAX_INPUT_LENGTH ];
    char       arg3  [ MAX_INPUT_LENGTH ];
    int        value;
    int        sn;
    bool       fAll;

    rch = get_char( ch );

    if ( !authorized( rch, "sset" ) )
        return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Syntax: sset <victim> <skill> <value>\n\r",	ch );
	send_to_char(AT_WHITE, "or:     sset <victim> all     <value>\n\r",	ch );
	send_to_char(AT_WHITE, "Skill being any skill or spell.\n\r",		ch );
	return;
    }

    if ( !( victim = get_char_world( ch, arg1 ) ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char(AT_WHITE, "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char(AT_WHITE, "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 1000 )
    {
	send_to_char(AT_WHITE, "Value range is 0 to 1000.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( skill_table[sn].name )
	    {
	        if ( skill_table[sn].skill_level[victim->class] <= LEVEL_DEMIGOD || 
		    skill_table[sn].skill_level[victim->multied] <= LEVEL_DEMIGOD )
		    victim->pcdata->learned[sn]	= value;
		else
		    if ( skill_table[sn].skill_level[victim->class] <= get_trust( victim ) ||
			skill_table[sn].skill_level[victim->multied] <= get_trust( victim ) )
		        victim->pcdata->learned[sn] = value;
	    }
	}
    }
    else
    {
        victim->pcdata->learned[sn] = value;
    }

    return;
}



void do_mset( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim = 0;
    char       buf  [ MAX_STRING_LENGTH ] = "";
    char       arg1 [ MAX_INPUT_LENGTH  ] = "";
    char       arg2 [ MAX_INPUT_LENGTH  ] = "";
    char       arg3 [ MAX_INPUT_LENGTH  ] = "";
    char       arg4 [ MAX_INPUT_LENGTH  ] = "";
    int        value;
    int        max;
    MOB_INDEX_DATA *pMob = NULL;
    bool       p = FALSE;

    rch = get_char( ch );

    if ( !authorized( rch, "mset" ) )
        return;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Syntax: mset <victim> <field>  <value>\n\r",	ch );
	send_to_char(AT_WHITE, "or:     mset <victim> <string> <value>\n\r",	ch );
	send_to_char(AT_WHITE, "\n\r",						ch );
	send_to_char(AT_WHITE, "&pField being one of:\n\r",			ch );
	send_to_char(AT_WHITE, "  str int wis dex con class sex level\n\r", ch );
	send_to_char(AT_WHITE, "  gold hp mana blood move practice align\n\r",		ch );
	send_to_char(AT_WHITE, "  thirst drunk full security affected_by2\n\r",				ch );
	send_to_char(AT_WHITE, "  affected_by clan clvl act mstr mint mwis\n\r", ch );
	send_to_char(AT_WHITE, "  mdex mcon bank carryn carryw save race\n\r", ch );
	send_to_char(AT_WHITE, "  lname affected_by3 affected_by4 language\n\r",ch );
	send_to_char(AT_WHITE, "  learn qp qtime qnext whotext pkill multi\n\r",ch );
	send_to_char(AT_WHITE, "  powers weaknesses religion shares poison_level\n\r", ch );
	send_to_char(AT_WHITE, "&pString being one of:\n\r",			ch );
	send_to_char(AT_WHITE, "  name short long title spec game\n\r",   ch );
	return;
    }

    if ( !is_number( arg1 ) )
    if ( !( victim = get_char_world( ch, arg1 ) ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
    }

    if ( is_number( arg1 ) )
    {
      if ( !(pMob = get_mob_index( atoi( arg1  )) ) )
      {
         send_to_char(AT_WHITE, "Nothing like that in hell, earth, or heaven.\n\r", ch );
         return;
      }
      else 
      {
         p = TRUE;
         victim = get_char_world( ch, pMob->player_name );
      }
     
    }
    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */
     
    /*
     * Whotext by Canth, canth@xs4all.nl
     */
    if ( !str_cmp( arg2, "whotext" ) )
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
            return;
        }

        /* If the string is longer than 12 chars, cut it off */
        if ( strlen ( arg3 ) > 14 )
            arg3[14] = '\0';
        /*
         * Chil (bbailey@mail.public.lib.ga.us) uses a boolean routine
         * to check if it is more than 23 chars.   CHANGED TO 8 CHARS
         * This way he can call it everytime he needs it.
         */

        /*
         * Bugfix for resetting who_text by Chil (bbailey@mail.public.lib.ga.us)
         */
        if ( !str_cmp( arg3, "@" ) )
        {
            victim->pcdata->who_text = str_dup( "@" );
            return;
        }

        /* add (23-length of word)/2 spaces behind arg3 */
        /* This is needed to centre the text in the who listing */

        max = ( ( 14 - strlen(arg3) ) / 2 );
        while ( max >=1 )
        {
            strcat ( arg3, " " );
            max--;
        }
        free_string ( victim->pcdata->who_text );
        victim->pcdata->who_text = str_dup( arg3 );
        return;
    }

    if ( !str_cmp( arg2, "pkill" ) ) {
	if( IS_NPC( victim ) ) {
	  send_to_char( AT_WHITE, "Cannot be done on mobiles.\n\r", ch );
	  return;
	}
        if( (value != 0) && (value != 1) ) {
	  send_to_char( AT_WHITE, "Value has to be 1 or 0: 1 for pkill, 0 for peaceful.\n\r", ch);
	  return;
        }
        victim->pkill = value;
        return;
    }
	


    if ( !str_cmp( arg2, "lname" ) )
    {
      if ( IS_NPC( victim ) )
        return;
      if ( victim->pcdata->lname )
        free_string( victim->pcdata->lname );
      if ( !str_cmp( "none", arg3 ) )
      {
        victim->pcdata->lname = NULL;
        send_to_char( AT_WHITE, "Cleared.\n\r", ch );
        return;
      }
      victim->pcdata->lname = str_dup( arg3 );
      send_to_char( AT_WHITE, "Ok.\n\r", ch );
      return;
    }
    
    if ( !str_cmp( arg2, "carryn" ) )
    {
      victim->carry_number = value;
      send_to_char( AT_WHITE, "Ok.\n\r", ch );
      return;
    }
    
    if ( !str_cmp( arg2, "carryw" ) )
    {
      victim->carry_weight = value;
      send_to_char( AT_WHITE, "Ok.\n\r", ch );
      return;
    }
    
    if ( !str_cmp( arg2, "bank" ) )
    {
      if ( IS_NPC( victim ) )
        return;
      victim->pcdata->bankaccount = value;
      send_to_char( AT_WHITE, "Ok.\n\r", ch );
      return;
    }
    
    if ( !str_cmp( arg2, "shares" ) )
    {
      if ( IS_NPC( victim ) )
        return;
      victim->pcdata->shares = value;
      send_to_char( AT_WHITE, "Ok.\n\r", ch );
      return;
    }

    if ( !str_cmp( arg2, "save" ) )
    {
      if ( IS_NPC( victim ) )
        return;
      victim->saving_throw = value;
      send_to_char( AT_WHITE, "Ok.\n\r", ch );
      return;
    }
    
    if ( !str_cmp( arg2, "mstr" ) )
    {
      if ( IS_NPC( victim ) )
      {
         send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
         return;
      }
      send_to_char( AT_WHITE, "Ok.\n\r", ch );
      victim->pcdata->mod_str = value;
      return;
    }
    
    if ( !str_cmp( arg2, "mint" ) )
    {
      if ( IS_NPC( victim ) )
      {
        send_to_char( AT_WHITE, "Not on NPC's.\n\r", ch );
        return;
      }
      send_to_char( AT_WHITE, "Ok.\n\r", ch );
      victim->pcdata->mod_int = value;
      return;
    }
    
    if ( !str_cmp( arg2, "mwis" ) )
    {
      if ( IS_NPC( victim ) )
      {
        send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
        return;
      }
      send_to_char( AT_WHITE, "Ok.\n\r", ch );
      victim->pcdata->mod_wis = value;
      return;
    }
    
    if ( !str_cmp( arg2, "mdex" ) )
    {
      if ( IS_NPC( victim ) )
      {
        send_to_char(AT_WHITE, "Not on NPCS's\n\r", ch );
        return;
      }
      send_to_char( AT_WHITE, "Ok.\n\r", ch );
      victim->pcdata->mod_dex = value;
      return;
    }
    
    if ( !str_cmp( arg2, "mcon" ) )
    {
      if (IS_NPC( victim ) )
      {
        send_to_char( AT_WHITE, "Not on NPC's\n\r", ch );
        return;
      }
      send_to_char( AT_WHITE, "Ok.\n\r", ch );
      victim->pcdata->mod_con = value;
      return;
    }
    
    if ( !str_cmp( arg2, "str" ) )
    {
	if ( IS_NPC( victim ) )
	{
	    send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( class_table[ch->class].attr_prime == APPLY_STR )
	    max = 40;
	else
	    max = 35;

	if ( value < 3 || value > max )
	{
	    sprintf( buf, "Strength range is 3 to %d.\n\r", max );
	    send_to_char(AT_WHITE, buf, ch );
	    return;
	}

	victim->pcdata->perm_str = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
	if ( IS_NPC( victim ) )
	{
	    send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( class_table[ch->class].attr_prime == APPLY_INT )
	    max = 40;
	else
	    max = 35;

	if ( value < 3 || value > max )
	{
	    sprintf( buf, "Intelligence range is 3 to %d.\n\r", max );
	    send_to_char(AT_WHITE, buf, ch );
	    return;
	}

	victim->pcdata->perm_int = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	if ( IS_NPC( victim ) )
	{
	    send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( class_table[ch->class].attr_prime == APPLY_WIS )
	    max = 40;
	else
	    max = 35;

	if ( value < 3 || value > max )
	{
	    sprintf( buf, "Wisdom range is 3 to %d.\n\r", max );
	    send_to_char(AT_WHITE, buf, ch );
	    return;
	}

	victim->pcdata->perm_wis = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( IS_NPC( victim ) )
	{
	    send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( class_table[ch->class].attr_prime == APPLY_DEX )
	    max = 40;
	else
	    max = 35;

	if ( value < 3 || value > max )
	{
	    sprintf( buf, "Dexterity range is 3 to %d.\n\r", max );
	    send_to_char(AT_WHITE, buf, ch );
	    return;
	}

	victim->pcdata->perm_dex = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( IS_NPC( victim ) )
	{
	    send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( class_table[ch->class].attr_prime == APPLY_CON )
	    max = 40;
	else
	    max = 35;

	if ( value < 3 || value > max )
	{
	    sprintf( buf, "Constitution range is 3 to %d.\n\r", max );
	    send_to_char(AT_WHITE, buf, ch );
	    return;
	}

	victim->pcdata->perm_con = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }
    
    if ( ( !str_cmp( arg2, "clvl" ) && get_trust( ch ) >= 109 ) ||
    ( IS_SET( ch->affected_by2, CODER ) && !str_cmp( arg2, "clvl" ) ) )
    {
       if IS_NPC( victim)
       {
          send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
          return;
       }
       victim->clev = value;
       send_to_char(AT_WHITE, "Ok.\n\r", ch );
       return;
    }
    else if ( !str_cmp( arg2, "clvl" ) && get_trust( ch ) < 109 )
    {
       send_to_char(AT_WHITE, "You are too low of trust to set one's clan level.\n\r", ch );
       return;
    }
       
    if ( ( !str_cmp( arg2, "clan" ) && get_trust( ch ) >= 109 ) ||
    ( IS_SET( ch->affected_by2, CODER ) && !str_cmp( arg2, "clan" ) ) )
    {
       if IS_NPC( victim )
       {
          send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
          return;
       }
       if ( !get_clan_index(value) )
       {
          send_to_char(AT_WHITE, "Invalid clan.\n\r", ch );
          return;
       }
       victim->clan = value;
       send_to_char(AT_WHITE, "Ok.\n\r", ch );
       return;
    }
    else if ( !str_cmp( arg2, "clan" ) && get_trust( ch ) < 109 )
    {
       send_to_char(AT_WHITE, "You are too low of trust to set one's clan.\n\r", ch );
       return;
    }
    
    if ( !str_cmp( arg2, "act" ) )
    {   
       if (!IS_NPC( victim ) )
       if ( get_trust( ch ) < 108 && !IS_SET( ch->affected_by2, CODER ) )
       {
          send_to_char(AT_WHITE, "You are too low of trust to set ones actflags.\n\r", ch );
          return;
       }
       if (IS_NPC( victim ) )
       {
          if (!str_cmp( arg3, "sentinel" ) || !str_cmp( arg3, "se" ) )
            value = 2;
          if (!str_cmp( arg3, "scavenger" ) || !str_cmp( arg3, "sc" ) )
            value = 4;
          if (!str_cmp( arg3, "aggressive" ) || !str_cmp( arg3, "ag" ) )
            value = 32;
          if (!str_cmp( arg3, "stayarea" ) || !str_cmp( arg3, "sa" ) )
            value = 64;
          if (!str_cmp( arg3, "wimpy" ) || !str_cmp( arg3, "w" ) )
            value = 128;
          if (!str_cmp( arg3, "pet" ) || !str_cmp( arg3, "pe" ) )
            value = 256;
          if (!str_cmp( arg3, "trainer" ) || !str_cmp( arg3, "t" ) )
            value = 512;
          if (!str_cmp( arg3, "practicer" ) || !str_cmp( arg3, "pr" ) )
            value = 1024;
          if (!str_cmp( arg3, "gambler" ) || !str_cmp( arg3, "g" ) )
            value = 2048;
          if ( (!str_cmp( arg3, "prototype" ) || !str_cmp( arg3, "pro" ) )
            && ( get_trust( ch ) >= 109 || IS_SET( ch->affected_by2, CODER ) ) )
            value = 4096;
       }
       
       if (!IS_NPC( victim ) )
       {
           if (!str_cmp( arg3, "boughtpet" ) || !str_cmp( arg3, "bp" ) )
             value = 2;
           if (!str_cmp( arg3, "autoexit" ) || !str_cmp( arg3, "ae" ) )
             value = 8;
           if (!str_cmp( arg3, "autoloot" ) || !str_cmp( arg3, "al" ) )
             value = 16;
           if (!str_cmp( arg3, "autosac" ) || !str_cmp( arg3, "as" ) )
             value = 32;
           if (!str_cmp( arg3, "blank" ) || !str_cmp( arg3, "bl" ) )
             value = 64;
           if (!str_cmp( arg3, "brief" ) || !str_cmp( arg3, "br" ) )
             value = 128;
           if (!str_cmp( arg3, "combine" ) || !str_cmp( arg3, "c" ) )
             value = 512;
           if (!str_cmp( arg3, "prompt" ) || !str_cmp( arg3, "p" ) )
             value = 1024;
         if (!str_cmp( arg3, "telnetga" ) || !str_cmp( arg3, "tga" ) )
           value = 2048;
         if (!str_cmp( arg3, "holylight" ) || !str_cmp( arg3, "h" ) )
           value = 4096;
         if (!str_cmp( arg3, "wizinvis" ) || !str_cmp( arg3, "w" ) )
           value = 8192;
         if (!str_cmp( arg3, "thief" ) || !str_cmp( arg3, "t" ) )
           value = 4194304;
         if (!str_cmp( arg3, "killer" ) || !str_cmp( arg3, "k" ) )
           value = 8388608;
         if (!str_cmp( arg3, "ansi" ) || !str_cmp( arg3, "a" ) )
           value = 16777216;
         if (!str_cmp( arg3, "autogold" ) || !str_cmp( arg3, "ag" ) )
           value = 33554432;
         if (!str_cmp( arg3, "keeptitle" ) || !str_cmp( arg3, "kt" ) )
           value = 67108864;
         if (!str_cmp( arg3, "undead" ) || !str_cmp( arg3, "u" ) )
           value = 134217728;
       }
       
       if (IS_NPC( victim ) )
       if ( ( value & ACT_PROTOTYPE ) && ( get_trust( ch ) < 109 ) &&
       !IS_SET( ch->affected_by2, CODER ) )
       {
          send_to_char(AT_WHITE, "You cannot toggle the prototype flag at your current level.\n\r", ch );
          return;
       }
            
       if ( 1073741824 & value )
       {      
         if (p) 
         {
           pMob->act ^= value;
           SET_BIT( victim->in_room->area->area_flags, AREA_CHANGED );
         }
          else
         {victim->act ^= value;}
         send_to_char(AT_WHITE, "Ok.\n\r", ch );
         return;
       }
       else
       {
         send_to_char(AT_WHITE, "Invalid bit.", ch );
         return;
       }
    }
    
    if ( !str_cmp( arg2, "affected_by" ) )
    {
       if ( (get_trust( ch ) < 108) && (!IS_NPC( victim )) )
       {
	  send_to_char(AT_WHITE, "You cannot set a player's affected_by from your current level.\n\r", ch );
          return;
       }
       if (p)
       { pMob->affected_by ^= value; }
       else
       { 
       if ( value == 0 ) 
         victim->affected_by = 0;
        else
         victim->affected_by ^= value; 
       }
       send_to_char(AT_WHITE, "Ok.\n\r", ch );
       return;
    }
    
    if ( !str_cmp( arg2, "affected_by2" ) )
    {
       if ( get_trust( ch ) < 108 && !IS_NPC( victim ) )
       {
	  send_to_char(AT_WHITE, "You are too low of level to set a player's affected_by2.\n\r", ch );
          return;
       }
       if (ch->name != "Tyrion") 
       {
          send_to_char(AT_WHITE, "YOU HAVE BEEN FLAGGED FOR CHEATING. TYRION HAS BEEN ALERTED.  PREPARE TO EXPLAIN YOURSELF OR BE DELETED.\n\r", ch );
          return;
       }
       if (p)
       { pMob->affected_by2 ^= value;
         SET_BIT( victim->in_room->area->area_flags, AREA_CHANGED ); }
       else
       { 
       if ( value == 0 )
         victim->affected_by2 = 0;
        else
         victim->affected_by2 ^= value; 
       }
       send_to_char(AT_WHITE, "Ok.\n\r", ch );
       return;
    }

    if ( !str_cmp( arg2, "affected_by3" ) )
    {
       if ( get_trust( ch ) < 108 && !IS_NPC( victim ) )
       {
	  send_to_char(AT_WHITE, "You are too low of level to set a player's affected_by3.\n\r", ch );
          return;
       }
       if (p)
       { pMob->affected_by3 ^= value;
         SET_BIT( victim->in_room->area->area_flags, AREA_CHANGED ); }
       else
       { 
       if ( value == 0 )
         victim->affected_by3 = 0;
        else
         victim->affected_by3 ^= value; 
       }
       send_to_char(AT_WHITE, "Ok.\n\r", ch );
       return;
    }

    if ( !str_cmp( arg2, "affected_by4" ) )
    {
       if ( get_trust( ch ) < 108 && !IS_NPC( victim ) )
       {
	  send_to_char(AT_WHITE, "You are too low of level to set a player's affected_by4.\n\r", ch );
          return;
       }
       if (p)
       { pMob->affected_by4 ^= value;
         SET_BIT( victim->in_room->area->area_flags, AREA_CHANGED ); }
       else
       { 
       if ( value == 0 )
         victim->affected_by4 = 0;
        else
         victim->affected_by4 ^= value; 
       }
       send_to_char(AT_WHITE, "Ok.\n\r", ch );
       return;
    }

    if ( !str_cmp( arg2, "powers" ) )   
    {
       if ( get_trust( ch ) < 108 && !IS_NPC( victim ) )
       {
          send_to_char(AT_WHITE, "You are too low of level to set a player's powers.\n\r", ch );
          return;
       }
       if (p)
       { pMob->affected_by_powers ^= value;
         SET_BIT( victim->in_room->area->area_flags, AREA_CHANGED ); }
       else
       {
       if ( value == 0 )
         victim->affected_by_powers = 0;
        else
         victim->affected_by_powers ^= value;
       }
       send_to_char(AT_WHITE, "Ok.\n\r", ch );
       return;
    }

    if ( !str_cmp( arg2, "weaknesses" ) )   
    {
       if ( get_trust( ch ) < 108 && !IS_NPC( victim ) )
       {
          send_to_char(AT_WHITE, "You are too low of level to set a player's weaknesses.\n\r", ch );
          return;
       }
       if (p)
       { pMob->affected_by_weaknesses ^= value;
         SET_BIT( victim->in_room->area->area_flags, AREA_CHANGED ); }
       else
       {
       if ( value == 0 )
         victim->affected_by_weaknesses = 0;
        else
         victim->affected_by_weaknesses ^= value;
       }
       send_to_char(AT_WHITE, "Ok.\n\r", ch );
       return;
    }

    if ( !str_cmp( arg2, "class" ) )
    {
	if ( value < 0 || value >= MAX_CLASS )
	{
	    char buf [ MAX_STRING_LENGTH ];

	    sprintf( buf, "Class range is 0 to %d.\n", MAX_CLASS-1 );
	    send_to_char(AT_WHITE, buf, ch );
	    return;
	}
	victim->class = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "multi" ) )
    {
	if ( value < -1 || value >= MAX_CLASS )
	{
	    char buf [ MAX_STRING_LENGTH ];

	    sprintf( buf, "Class range is -1 to %d.\n", MAX_CLASS-1 );
	    send_to_char(AT_WHITE, buf, ch );
	    return;
	}
	victim->multied = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "religion" ) )
    {
	RELIGION_DATA *pReligion;

	if IS_NPC( victim )
	{
	    send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	    return;
	}
	if ( !get_religion_index(value) )
	{
	    send_to_char(AT_WHITE, "Invalid religion.\n\r", ch );
	    return;
	}
	pReligion = get_religion_index( victim->religion );
	pReligion->members--;
	victim->religion = value;
	pReligion = get_religion_index( victim->religion );
	pReligion->members++;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "sex" ) )
    {
        if ( IS_AFFECTED( victim, AFF_CHANGE_SEX ) )
        {
            send_to_char(AT_WHITE, "This person is affect by change sex.\n\r", ch );
            send_to_char(AT_WHITE, "Try again later.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 2 )
        {
            send_to_char(AT_WHITE, "Sex range is 0 to 2.\n\r", ch );
            return;
        }

        victim->sex = value;

        return;
    }


    if ( !str_cmp( arg2, "race" ) )
    {

        OBJ_DATA *wield;
        OBJ_DATA *wield2;
        int       race;

        if ( IS_AFFECTED( victim, AFF_POLYMORPH ) )
        {
            send_to_char(AT_WHITE, "This person is affected by polymorph other.\n\r",
                         ch );
            send_to_char(AT_WHITE, "Try again later.\n\r", ch );
            return;
        }

        race = race_lookup( arg3 );

        if ( race < 0 )
        {
            send_to_char(AT_RED, "Invalid race.\n\r", ch );
            return;
        }

        if (  !IS_SET( race_table[ race ].race_abilities, RACE_PC_AVAIL )
            && get_trust( ch ) < L_DIR )
        {
            send_to_char(AT_WHITE, "You may not set a race not available to PC's.\n\r",
                         ch );
            return;
        }

        victim->race = race;

        if ( ( wield = get_eq_char( victim, WEAR_WIELD ) )
            && !IS_SET( race_table[ victim->race ].race_abilities,
                       RACE_WEAPON_WIELD ) )
        {
            act(AT_RED, "You drop $p.", victim, wield, NULL, TO_CHAR );
            act(AT_RED, "$n drops $p.", victim, wield, NULL, TO_ROOM );
            obj_from_char( wield );
            obj_to_room( wield, victim->in_room );
        }

        if ( ( wield2 = get_eq_char( victim, WEAR_WIELD_2 ) )
            && !IS_SET( race_table[ victim->race ].race_abilities,
                       RACE_WEAPON_WIELD ) )
        {
            act(AT_RED, "You drop $p.", victim, wield2, NULL, TO_CHAR );
            act(AT_RED, "$n drops $p.", victim, wield2, NULL, TO_ROOM );
            obj_from_char( wield2 );
            obj_to_room( wield2, victim->in_room );
        }

        return;


      }

    if ( !str_cmp( arg2, "level" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    send_to_char(AT_WHITE, "Not on PC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 200 )
	{
	    send_to_char(AT_WHITE, "Level range is 0 to 200.\n\r", ch );
	    return;
	}
	victim->level = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "gold" ) )
    {
	victim->gold = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( ( !str_cmp( arg2, "qp" ) ) && ( get_trust( ch ) >= 112 ) )
    {
        victim->questpoints = value;
        return;
    }

    if ( !str_prefix( arg2, "qtime" ) )
    {
        if (value == 0)
                value++;
        victim->countdown = value;
        return;
    }

    if ( !str_prefix( arg2, "qnext" ) )
    {
        victim->nextquest = value;
        return;
    }

    if ( !str_prefix( arg2, "poison_level" ) )
    {
	if ( value > MAX_POISON_LEVEL || value < 0 )
	{
	    send_to_char(AT_WHITE, "Out of range.\n\r", ch );
	    return;
	}
	victim->poison_level = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }  

    if ( !str_cmp( arg2, "hp" ) )
    {
	if ( value < -10 || value > 60000 )
	{
	    send_to_char(AT_WHITE, "Hp range is -10 to 60,000 hit points.\n\r", ch );
	    return;
	}
	if ( victim->fighting && value < 0 )
	{
	    send_to_char(AT_WHITE, "You cannot set a fighting person's hp below 0.\n\r",
			 ch );
	    return;
	}
	victim->max_hit = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "mana" ) )
    {
	if ( value < 0 || value > 60000 )
	{
	    send_to_char(AT_WHITE, "Mana range is 0 to 60,000 mana points.\n\r", ch );
	    return;
	}
	victim->max_mana = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "blood" ) )
    {
	if ( value < 0 || value > 20000 )
	{
	    send_to_char(AT_WHITE, "Blood range is 0 to 20,000 mana points.\n\r", ch );
	    return;
	}
	victim->max_bp = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "move" ) )
    {
	if ( value < 0 || value > 30000 )
	{
	    send_to_char(AT_WHITE, "Move range is 0 to 30,000 move points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "practice" ) )
    {
	if ( value < 0 || value > 1000 )
	{
	    send_to_char(AT_WHITE, "Practice range is 0 to 1000 sessions.\n\r", ch );
	    return;
	}
	victim->practice = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "learn" ) )            /* Maniac */
    {
        if (IS_NPC(ch))
                send_to_char(AT_WHITE,"Not on NPC's\n\r", ch);
        else
                victim->pcdata->learn = value;
        return;
    }


    if ( !str_cmp( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char(AT_WHITE, "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "thirst" ) )
    {
	if ( IS_NPC( victim ) )
	{
	    send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( ( value < 0 || value > 100 )
	    && get_trust( victim ) < LEVEL_IMMORTAL )
	{
	    send_to_char(AT_WHITE, "Thirst range is 0 to 100.\n\r", ch );
	    return;
	}
	else
	    if ( value < -1 || value > 100 )
	    {
		send_to_char(AT_WHITE, "Thirst range is -1 to 100.\n\r", ch );
		return;
	    }

	victim->pcdata->condition[COND_THIRST] = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "drunk" ) )
    {
	if ( IS_NPC( victim ) )
	{
	    send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    send_to_char(AT_WHITE, "Drunk range is 0 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "full" ) )
    {
	if ( IS_NPC( victim ) )
	{
	    send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( ( value < 0 || value > 100 )
	    && get_trust( victim ) < LEVEL_IMMORTAL )
	{
	    send_to_char(AT_WHITE, "Full range is 0 to 100.\n\r", ch );
	    return;
	}
	else
	    if ( value < -1 || value > 100 )
	    {
		send_to_char(AT_WHITE, "Full range is -1 to 100.\n\r", ch );
		return;
	    }

	victim->pcdata->condition[COND_FULL] = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    send_to_char(AT_WHITE, "Not on PC's.\n\r", ch );
	    return;
	}

	if ( longstring( ch, arg3 ) )
	    return;

	free_string( victim->name );
	victim->name = str_dup( arg3 );
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "short" ) )
    {
        if ( !IS_NPC(victim) )
	{
	  send_to_char(AT_WHITE, "Not on PC's.\n\r",ch);
	  return;
	}
        if ( longstring( ch, arg3 ) )
	    return;

	free_string( victim->short_descr );
	victim->short_descr = str_dup( arg3 );
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
        if ( !IS_NPC(victim) )
	{
	  send_to_char(AT_WHITE, "Not on PC's.\n\r",ch);
	  return;
	}
        if ( longstring( ch, arg3 ) )
	    return;

	free_string( victim->long_descr );
	strcat( arg3, "\n\r" );
	victim->long_descr = str_dup( arg3 );
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "title" ) )
    {
	if ( IS_NPC( victim ) )
	{
	    send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	    return;
	}

	set_title( victim, arg3 );
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "spec" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    send_to_char(AT_WHITE, "Not on PC's.\n\r", ch );
	    return;
	}

	if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	{
	    send_to_char(AT_WHITE, "No such spec fun.\n\r", ch );
	    return;
	}
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "game" ) )
    {
        if ( !IS_NPC( victim ) )
        {
            send_to_char(AT_WHITE, "Not on PC's.\n\r", ch );
            return;
        }

        if ( ( victim->game_fun = game_lookup( arg3 ) ) == 0 )
        {
            send_to_char(AT_WHITE, "No such game fun.\n\r", ch );
            return;
        }

        return;
    }



    if (!str_cmp(arg2, "language"))             /* Call to do_lset by Maniac */
    {
        if (arg4[0] == '\0')
        {
            do_mset( ch, "" );
            return;
        }
        if ( IS_NPC(victim) )
        {
            send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
            return;
        }
        sprintf( buf, "%s %s %s", victim->name, arg3, arg4 );
        do_lset( ch, buf );
        return;
    }


    if ( !str_cmp( arg2, "security" ) )	/* OLC */
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
            return;
        }

	if ( (value > ch->pcdata->security && !IS_SET(ch->affected_by2, CODER)) || value < 0 )
	{
	    if ( ch->pcdata->security != 0 )
	    {
		sprintf( buf, "Valid security is 0-%d.\n\r",
		    ch->pcdata->security );
		send_to_char(AT_WHITE, buf, ch );
	    }
	    else
	    {
		send_to_char(AT_WHITE, "Valid security is 0 only.\n\r", ch );
	    }
          return;
	}
	victim->pcdata->security = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }
/* XOR */
    if(!str_cmp(arg2, "immune"))
    {
      if(!IS_NPC(victim))
      {
        send_to_char(AT_WHITE, "Not on PC's.\n\r", ch );
        return;
      }
      victim->imm_flags ^= value;
      send_to_char(AT_WHITE, "Ok.\n\r", ch );
      return;
    }
/* END */

    /*
     * Generate usage message.
     */
    do_mset( ch, "" );
    return;
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *obj;
    CHAR_DATA *rch;
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    char       arg3 [ MAX_INPUT_LENGTH ];
    char       arg4 [ MAX_INPUT_LENGTH ];
    int        value;

    rch = get_char( ch );

    if ( !authorized( rch, "oset" ) )
        return;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !str_cmp( arg2, "apply" ) )
    {
      argument = one_argument( argument, arg3 );
      argument = one_argument( argument, arg4 );
    }
    else
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char(AT_WHITE, "&pSyntax: oset <object> <field>  <value>\n\r",	ch );
	send_to_char(AT_WHITE, "&por:     oset <object> <string> <value>\n\r",	ch );
	send_to_char(AT_WHITE, "\n\r",						ch );
	send_to_char(AT_WHITE, "&pField being one of:\n\r",			ch );
	send_to_char(AT_WHITE, "  value0 value1 value2 value3\n\r",		ch );
	send_to_char(AT_WHITE, "  durability_max, durability_cur\n\r",		ch );
	send_to_char(AT_WHITE, "  extra extra2 extra3 extra4 wear level weight cost timer\n\r", ch );
	send_to_char(AT_WHITE, "  apply delapply\n\r", ch );
	send_to_char(AT_WHITE, "\n\r",						ch );
	send_to_char(AT_WHITE, "&pString being one of:\n\r",			ch );
	send_to_char(AT_WHITE, "  name short long ed\n\r",			ch );
	return;
    }

    if ( !( obj = get_obj_world( ch, arg1 ) ) )
    {
	send_to_char(AT_WHITE, "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;     
    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	obj->value[2] = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "durability_cur" ) || !str_cmp( arg2, "dur_cur" ) )
    {
	obj->durability_cur = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "durability_max" ) || !str_cmp( arg2, "dur_max" ) )
    {
	obj->durability_max= value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "delapply" ) )
    {
      AFFECT_DATA *paf;
      AFFECT_DATA *paf_next;
      int cnt = 0;
      
      if ( value == -1 )
      {
        send_to_char( AT_WHITE, "&pSyntax: oset object delapply [number]\n\r", ch );
        return;
      }
      
      if ( !( paf = obj->affected ) )
      {
          send_to_char( AT_WHITE, "Non-existant apply.\n\r", ch );
          return;
      }
      
      
      if ( value == 0 )
      {
        paf = obj->affected;
        obj->affected = paf->next;
        free_affect( paf );
      }
      else
      {
        while ( ( paf_next = paf->next ) && ( ++cnt < value ) )
            paf = paf_next;
            
        if ( paf_next )
        {
           paf->next = paf_next->next;
           free_affect( paf_next );
        }
        else
        {
           send_to_char( AT_WHITE, "No such affect.\n\r", ch );
           return;
        }
      }      
      return;
    }

    if ( !str_cmp( arg2, "apply" ) )
    {
        AFFECT_DATA *paf;
        
        if ( is_number( arg3 ) || arg4[0] == '\0' || !is_number( arg4 ) )
        {
          send_to_char( AT_WHITE, "&pSyntax: oset object apply type modifier\n\r", ch );
          return;
        }

	paf = new_affect();
	paf->location = flag_value( apply_flags, arg3 );
	paf->modifier = atoi( arg4 );
	paf->type     = skill_lookup(arg3);
	if ( paf->type < 0 )
	  paf->type = 0;
	paf->duration = -1;
	paf->bitvector = 0;
	paf->next = obj->affected;
	obj->affected = paf;        
	
	send_to_char( AT_WHITE, "Apply added.\n\r", ch );
	return;
    }
        
        

    if ( !str_cmp( arg2, "extra" ) )
    {
        value=0; 
        if (!str_cmp( arg3, "glow" ) || !str_cmp( arg3, "g" ) )
          value = 1;
        if (!str_cmp( arg3, "hum" ) || !str_cmp( arg3, "h" ) )
          value = 2;
        if (!str_cmp( arg3, "dark" ) || !str_cmp( arg3, "d" ) )
          value = 4;
        if (!strcmp( arg3, "lock" ) || !str_cmp( arg3, "l" ) )
          value = 8;
        if (!str_cmp( arg3, "evil" ) || !str_cmp( arg3, "e" ) )
          value = 16;
        if (!str_cmp( arg3, "invis" ) || !str_cmp( arg3, "i" ) )
          value = 32;
        if (!str_cmp( arg3, "magic" ) || !str_cmp( arg3, "m" ) )
          value = 64;
        if (!str_cmp( arg3, "nodrop" ) || !str_cmp( arg3, "nd" ) )
          value = 128;
        if (!str_cmp( arg3, "bless" ) || !str_cmp( arg3, "bl" ) )
          value = 256;
        if (!str_cmp( arg3, "anti-good" ) || !str_cmp( arg3, "ag" ) )
          value = 512;
        if (!str_cmp( arg3, "anti-evil" ) || !str_cmp( arg3, "ae" ) )
          value = 1024;
        if (!str_cmp( arg3, "anti-neutral" ) || !str_cmp( arg3, "an" ) )
          value = 2048;
        if (!str_cmp( arg3, "noremove" ) || !str_cmp( arg3, "nr" ) )
          value = 4096;
        if (!str_cmp( arg3, "inventory" ) || !str_cmp( arg3, "in" ) )
          value = 8192;
        if (!str_cmp( arg3, "poisoned" ) || !str_cmp( arg3, "p" ) )
          value = 16384;
        if (!str_cmp( arg3, "anti-mage" ) || !str_cmp( arg3, "am" ) )
          value = 32768;
        if (!str_cmp( arg3, "anti-cleric" ) || !str_cmp( arg3, "ac" ) )
          value = 65536;
        if (!str_cmp( arg3, "anti-thief" ) || !str_cmp( arg3, "at" ) )
          value = 131072;
        if (!str_cmp( arg3, "anti-warrior" ) || !str_cmp( arg3, "aw" ) )
          value = 262144;
        if (!str_cmp( arg3, "anti-psi" ) || !str_cmp( arg3, "ap" ) )
          value = 524288;
        if (!str_cmp( arg3, "anti-druid" ) || !str_cmp( arg3, "ad" ) )
          value = 1048576;
        if (!str_cmp( arg3, "anti-ranger" ) || !str_cmp( arg3, "ar" ) )
          value = 2097152;
        if (!str_cmp( arg3, "anti-paladin" ) || !str_cmp( arg3, "apa" ) )
          value = 4194304;
        if (!str_cmp( arg3, "anti-bard" ) || !str_cmp( arg3, "ab" ) )
          value = 8388608;
        if (!str_cmp( arg3, "anti-vamp" ) || !str_cmp( arg3, "av" ) )
          value = 16777216;
        if (!str_cmp( arg3, "flame" ) || !str_cmp( arg3, "de" ) )
          value = 33554432;
        if (!str_cmp( arg3, "chaos" ) || !str_cmp( arg3, "dr" ) )
          value = 67108864;
        if (!str_cmp( arg3, "frosty" ) || !str_cmp( arg3, "fr" ) )
          value = 134217728;
        if (!str_cmp( arg3, "nodamage" ) || !str_cmp( arg3, "indestructable" ) )
          value = 536870912;
	if ( !str_cmp( arg3, "none" ) )
	{
	   obj->extra_flags = 0;
	   send_to_char(AT_WHITE,"Ok.\n\r", ch );
	   return;
	}
	if ( 0xFFFFFFFF & value )
	{
          obj->extra_flags ^= value;
	  send_to_char(AT_WHITE, "Ok.\n\r", ch );
	  return;
	}
	else
	{
	  send_to_char(AT_WHITE, "Invalid bit.\n\r", ch );
	  return;
	}
    }

    if ( !str_cmp( arg2, "extra2" ) )
    {
	value=0;
        if (!str_cmp( arg3, "anti-barbarian" ) || !str_cmp( arg3, "ab" ) )
          value = 1;
        if (!str_cmp( arg3, "anti-antipal" ) || !str_cmp( arg3, "aap" ) )
          value = 2;
        if (!str_cmp( arg3, "anti-monk" ) || !str_cmp( arg3, "amo" ) )
          value = 4;
        if (!str_cmp( arg3, "anti-assassin" ) || !str_cmp( arg3, "aas" ) )
          value = 8;
        if (!str_cmp( arg3, "anti-werewolf" ) || !str_cmp( arg3, "awf" ) )
          value = 16;
        if (!str_cmp( arg3, "anti-illusionist" ) || !str_cmp( arg3, "ail" ) )
          value = 32;
        if (!str_cmp( arg3, "holy" ) || !str_cmp( arg3, "ho" ) )
          value = 64;
        if (!str_cmp( arg3, "nopurge" ) || !str_cmp( arg3, "anti-purge" ) )
          value = 128;
        if (!str_cmp( arg3, "hidden" ) || !str_cmp( arg3, "hide" ) )
          value = 256;
	if (!str_cmp( arg3, "anti-necromancer" ) || !str_cmp( arg3, "ane" ) )
	  value = 512;
	if (!str_cmp( arg3, "anti-demonologist" ) || !str_cmp( arg3, "ade" ) )
	  value = 1024;
	if (!str_cmp( arg3, "anti-beastmaster" ) || !str_cmp( arg3, "abe" ) )
	  value = 2048;
	if (!str_cmp( arg3, "anti-darkpriest" ) || !str_cmp( arg3, "adk" ) )
	  value = 4096;
	if (!str_cmp( arg3, "sparking" ) || !str_cmp( arg3, "sparkly" ) )
	  value = 8192;
	if (!str_cmp( arg3, "two-handed" ) || !str_cmp( arg3, "two" ) )
	  value = ITEM_TWO_HANDED;
	if ( !str_cmp( arg3, "none" ) )
	{
	   obj->extra_flags2 = 0;
	   send_to_char(AT_WHITE,"Ok.\n\r", ch );
	   return;
	}
	if ( 0xFFFFFFFF & value )
	{
          obj->extra_flags2 ^= value;
	  send_to_char(AT_WHITE, "Ok.\n\r", ch );
	  return;
	}
	else
	{
	  send_to_char(AT_WHITE, "Invalid bit.\n\r", ch );
	  return;
	}
    }

    if ( !str_cmp( arg2, "extra3" ) )
    {
	value=0;
	if ( !str_cmp( arg3, "none" ) )
	{
	    obj->extra_flags3 = 0;
	    send_to_char(AT_WHITE,"Ok.\n\r", ch );
	    return;
	}
	if ( 0xFFFFFFFF & value )
	{
	    obj->extra_flags3 ^= value;
	    send_to_char(AT_WHITE, "Ok.\n\r", ch );
	    return;
	}
	else
	{
	  send_to_char(AT_WHITE, "Invalid bit.\n\r", ch );
	  return;
	}
    }

    if ( !str_cmp( arg2, "extra4" ) )
    {
        value=0;
        if ( !str_cmp( arg3, "none" ) )
        {
            obj->extra_flags4 = 0;
            send_to_char(AT_WHITE,"Ok.\n\r", ch );
            return;
        }
        if ( 0xFFFFFFFF & value )
        {
            obj->extra_flags4 ^= value;
            send_to_char(AT_WHITE, "Ok.\n\r", ch );
            return;
        }
        else
        {
          send_to_char(AT_WHITE, "Invalid bit.\n\r", ch );
          return;
        }
    } 

    if ( !str_cmp( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
	obj->level = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }
	
    if ( !str_cmp( arg2, "weight" ) )
    {
	obj->weight = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "cost" ) )
    {
	obj->cost = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "timer" ) )
    {
	obj->timer = value;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }
	
    if ( !str_cmp( arg2, "name" ) )
    {
        if ( longstring( ch, arg3 ) )
	    return;

	free_string( obj->name );
	obj->name = str_dup( arg3 );
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "short" ) )
    {
        if ( longstring( ch, arg3 ) )
	    return;

	free_string( obj->short_descr );
	obj->short_descr = str_dup( arg3 );
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
        if ( longstring( ch, arg3 ) )
	    return;

	free_string( obj->description );
	obj->description = str_dup( arg3 );
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "ed" ) )
    {
	EXTRA_DESCR_DATA *ed;

	argument = one_argument( argument, arg3 );
	argument = one_argument( argument, arg4 );
	if ( arg4[0] == '\0' )
	{
	    send_to_char(AT_WHITE, "Syntax: oset <object> ed <keyword> <string>\n\r",
		ch );
	    return;
	}

	ed = new_extra_descr();

	ed->keyword		= str_dup( arg3     );
	ed->description		= str_dup( argument );
	ed->deleted             = FALSE;
	ed->next		= obj->extra_descr;
	obj->extra_descr	= ed;
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *rch;
    ROOM_INDEX_DATA *location;
    char             arg1 [ MAX_INPUT_LENGTH ];
    char             arg2 [ MAX_INPUT_LENGTH ];
    char             arg3 [ MAX_INPUT_LENGTH ];
    int              value;

    rch = get_char( ch );

    if ( !authorized( rch, "rset" ) )
        return;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Syntax: rset <location> <field> value\n\r",	ch );
	send_to_char(AT_WHITE, "\n\r",						ch );
	send_to_char(AT_WHITE, "Field being one of:\n\r",			ch );
	send_to_char(AT_WHITE, "  flags sector\n\r",				ch );
	return;
    }

    if ( !( location = find_location( ch, arg1 ) ) )
    {
	send_to_char(AT_WHITE, "No such location.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char(AT_WHITE, "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "flags" ) )
    {
	location->room_flags	= value;
	return;
    }

    if ( !str_cmp( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}


char *con_type_name( int connected )
{
  if ( connected == CON_PLAYING ) return "Playing";
  if ( connected == CON_GET_NAME ) return "Get Name";
  if ( connected == CON_GET_OLD_PASSWORD ) return "Old PW";
  if ( connected == CON_CONFIRM_NEW_NAME ) return "Name Conf";
  if ( connected == CON_GET_NEW_PASSWORD ) return "New PW";
  if ( connected == CON_CONFIRM_NEW_PASSWORD ) return "PW Conf";
  if ( connected == CON_GET_NEW_SEX ) return "New Sex";
  if ( connected == CON_GET_NEW_CLASS ) return "New Class";
  if ( connected == CON_READ_MOTD ) return "Read MOTD";
  if ( connected == CON_GET_NEW_RACE ) return "New Race";
  if ( connected == CON_CONFIRM_RACE ) return "Race Conf";
  if ( connected == CON_CONFIRM_CLASS ) return "Class Conf";
  if ( connected == CON_CHOICE_MULTICLASS ) return "Multi Y-N";
  if ( connected == CON_GET_MULTICLASS ) return "New Multi";
  if ( connected == CON_CONFIRM_MULTICLASS ) return "Multi Conf";
  if ( connected == CON_GET_PKILL ) return "PK Choice";
  if ( connected == CON_CONFIRM_PKILL ) return "PK Conf";
  if ( connected == CON_CHOSE_RELIGION ) return "Ch Relig";
  if ( connected == CON_CONFIRM_RELIGION) return "Conf Relig";
  if ( connected == CON_DISPLAY_RELIGION) return "Disp Relig";
  if ( connected == CON_DISPLAY_ATTRIBUTES ) return "Disp Attr";
  if ( connected == CON_CHOSE_ATTRIBUTES ) return "Ch Attr";
  if ( connected == CON_CONFIRM_ATTRIBUTES ) return "Conf Attr";
  if ( connected == CON_GET_ANSI ) return "Get ANSI";
  if ( connected == CON_AUTHORIZE_NAME ) return "Authorize";
  if ( connected == CON_AUTHORIZE_NAME1 ) return "Auth One";
  if ( connected == CON_AUTHORIZE_NAME2 ) return "Auth Two";
  if ( connected == CON_AUTHORIZE_NAME3 ) return "Auth Three";
  if ( connected == CON_AUTHORIZE_LOGOUT ) return "Auth Logout";
  if ( connected == CON_CHATTING ) return "Chatting";
  return "Unknown";
}

void do_users( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *rch;
    DESCRIPTOR_DATA *d;
    char            buf  [ MAX_STRING_LENGTH ];
    char            buf2 [ MAX_STRING_LENGTH ];
    int             count;
    CHAR_DATA	    *victim;

    rch = get_char( ch );

    if ( !authorized( rch, "users" ) )
        return;

    count	= 0;
    buf[0]	= '\0';
    buf2[0]     = '\0';
    strcpy( buf, "[Desc Connected  ] Name&W!&BUser&W@&GHost&R\n\r" );
    for ( d = descriptor_list; d; d = d->next )
    {
	victim = ( d->original ) ? d->original : d->character;
	if ( ( d->character && can_see( ch, d->character ) ) &&
	   ( ( d->character->level >= L_APP && ch->level >= L_CON ) || (
d->character->level < L_APP ) ) )
	{
	    count++;
	    sprintf( buf + strlen(buf), "[%4d %-11s] %s&W!&B%s&W@&G%s&R\n\r",
		    d->descriptor,
		    con_type_name(d->connected),
		    d->original  ? d->original->name  :
		    d->character ? d->character->name : "(none)",
		    d->user ? d->user : "(none)",
		    d->host );
	}
    }

    sprintf( buf2, "%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat( buf, buf2 );
/*
    if( !( IS_SET( victim->act, PLR_WIZINVIS ) && victim->wizinvis > get_trust( ch ) ) )
    {
*/
        send_to_char(AT_RED, buf, ch );
/*
    }
*/
    return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    char       arg [ MAX_INPUT_LENGTH ];
    char       arg1[ MAX_INPUT_LENGTH ];
    int        trust;
    int        cmd;
    int        llvlr = 0;
    int        hlvlr = L_IMP;

    rch = get_char( ch );

    if ( !authorized( rch, "force" ) )
        return;

    argument = one_argument( argument, arg );
    if ( is_number( arg ) )
    {
      llvlr = atoi( arg );
      argument = one_argument( argument, arg1 );
      if ( is_number( arg1 ) )
        hlvlr = atoi( arg1 );
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Force whom to do what?\n\r", ch );
	return;
    }


    /*
     * Look for command in command table.
     */
  if (!strncasecmp(argument, "delete", 6))
    {
	send_to_char(AT_WHITE, "I don't think so.\n\r", ch );
	return;
    }
  else
    {
    trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( argument[0] == cmd_table[cmd].name[0]
	    && !str_prefix( argument, cmd_table[cmd].name )
	    && ( cmd_table[cmd].level > trust ) )
	{
	  send_to_char(AT_WHITE, "You can't even do that yourself!\n\r", ch );
	  return;
	}
    }

    if ( !str_cmp( arg, "all" ) || is_number( arg ) )
    {
	CHAR_DATA *vch;

	for ( vch = char_list; vch; vch = vch->next )
	{
	    if ( vch->deleted )
	        continue;

	    if ( !IS_NPC( vch ) && get_trust( vch ) < get_trust( ch ) 
	      && ( vch->level >= llvlr && vch->level <= hlvlr ) )
	    {
	      if ( !is_number( arg ) || ( is_number( arg ) && is_number( arg1 ) ) )
	      {
		act(AT_RED, "$n forces you to '$t'.", ch, argument, vch, TO_VICT );
		interpret( vch, argument );
		continue;
              }
              if ( !is_number( arg1 ) )
              {
                sprintf( log_buf, "%s %s", arg1, argument );
                act( AT_RED, "$n forces you to '$t'.", ch, log_buf, vch, TO_VICT );
                interpret( vch, log_buf );
              }
              
	    }
	}
    }
    else
    {
	CHAR_DATA *victim;

	if ( !( victim = get_char_world( ch, arg ) ) )
	{
	    send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char(AT_WHITE, "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
	    send_to_char(AT_WHITE, "Do it yourself!\n\r", ch );
	    return;
	}

	act(AT_RED, "$n forces you to '$t'.", ch, argument, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char(AT_WHITE, "Ok.\n\r", ch );
    return;
    }
}
    
void do_invis( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int level; 

    if ( IS_NPC(ch) )
        return; 

    argument = one_argument( argument, arg ); 
    if ( arg && arg[0] != '\0' )
    {
        if ( !is_number( arg ) )
        {
           send_to_char( AT_WHITE, "Usage: invis | invis <level>\n\r", ch ); 
           return; 
        }
        level = atoi( arg ); 
        if ( level < 2 || level > get_trust( ch ) )
        {
            send_to_char( AT_WHITE, "Invalid level.\n\r", ch ); 
            return; 
        }
        ch->wizinvis = level; 
        sprintf( arg, "Wizinvis level set to %d.\n\r", level ); 
        send_to_char( AT_WHITE, arg, ch ); 
        return; 
    }

    if ( ch->wizinvis < 2 )
      ch->wizinvis = ch->level; 

    if ( IS_SET(ch->act, PLR_WIZINVIS) )
    {
        REMOVE_BIT(ch->act, PLR_WIZINVIS); 
    act( AT_YELLOW, "$n slowly fades into existence.", ch, NULL, NULL,TO_ROOM ); 
        send_to_char( AT_WHITE, "You slowly fade back into existence.\n\r", 
ch ); 
    }
    else
    {
        SET_BIT(ch->act, PLR_WIZINVIS); 
    act( AT_YELLOW, "$n slowly fades into thin air.", ch, NULL, NULL,
TO_ROOM ); 
        send_to_char( AT_WHITE, "You slowly vanish into thin air.\n\r", 
ch ); 
    }
    return; 
 
 
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
	return;

    if ( !authorized( ch, "holylight" ) )
        return;

    if ( IS_SET( ch->act, PLR_HOLYLIGHT ) )
    {
	REMOVE_BIT( ch->act, PLR_HOLYLIGHT );
	send_to_char(AT_WHITE, "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(    ch->act, PLR_HOLYLIGHT );
	send_to_char(AT_WHITE, "Holy light mode on.\n\r", ch );
    }

    return;
}

/* Wizify and Wizbit sent in by M. B. King */

void do_wizify( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg1 [ MAX_INPUT_LENGTH ];
  
    rch = get_char( ch );

    if ( !authorized( rch, "wizify" ) )
        return;

    argument = one_argument( argument, arg1  );
    if ( arg1[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Syntax: wizify <name>\n\r" , ch );
	return;
    }
    if ( !( victim = get_char_world( ch, arg1 ) ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r" , ch );
	return;
    }
    if ( IS_NPC( victim ) )
    {
	send_to_char(AT_WHITE, "Not on mobs.\n\r", ch );
	return;
    }
    victim->wizbit = !victim->wizbit;
    if ( victim->wizbit ) 
    {
	act(AT_RED, "$N wizified.",         ch, NULL, victim, TO_CHAR );
	act(AT_RED, "$n has wizified you!", ch, NULL, victim, TO_VICT );
    }
    else
    {
	act(AT_RED, "$N dewizzed.",         ch, NULL, victim, TO_CHAR );
	act(AT_RED, "$n has dewizzed you!", ch, NULL, victim, TO_VICT ); 
    }

    do_save( victim, "");
    return;
}

/* Idea from Talen of Vego's do_where command */

void do_owhere( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *obj;
    OBJ_DATA  *in_obj;
    CHAR_DATA *rch;
    char       buf  [ MAX_STRING_LENGTH   ];
    char       buf1 [ MAX_STRING_LENGTH*10 ];
    char       arg  [ MAX_INPUT_LENGTH    ];
    int        obj_counter = 1;
    bool       found = FALSE;

    rch = get_char( ch );

    if ( !authorized( rch, "owhere" ) )
        return;

    one_argument( argument, arg );

    if( arg[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Syntax:  owhere <object>.\n\r", ch );
	return;
    }
    else
    {
        buf1[0] = '\0';
	for ( obj = object_list; obj; obj = obj->next )
	{
	    if ( !can_see_obj( ch, obj ) || !is_name( arg, obj->name ) )
	        continue;

	    found = TRUE;

	    for ( in_obj = obj; in_obj->in_obj;
		 in_obj = in_obj->in_obj )
	        ;

	    if ( in_obj->carried_by )
	    {
	        if ( !can_see( ch, in_obj->carried_by ) )
		    continue;
		sprintf( buf, "[%2d] %s carried by %s at [%4d].\n\r",
			obj_counter, obj->short_descr,
			PERS( in_obj->carried_by, ch ),
			in_obj->carried_by->in_room->vnum );
	    }
	    else if ( in_obj->stored_by )
	    {
	      sprintf( buf, "[%2d] %s in %s's storage box.\n\r",
		      obj_counter, obj->short_descr,
		      PERS( in_obj->stored_by, ch ) );
	    }
	    else
	    {
		sprintf( buf, "[%2d] %s in %s at [%4d].\n\r", obj_counter,
			obj->short_descr, ( !in_obj->in_room ) ?
			"somewhere" : in_obj->in_room->name,
			( !in_obj->in_room ) ?
			0 : in_obj->in_room->vnum );
	    }
	    
	    obj_counter++;
	    buf[0] = UPPER( buf[0] );
	    strcat( buf1, buf );
	}

	send_to_char(AT_GREEN, buf1, ch );
    }

    if ( !found )
	send_to_char(AT_WHITE,
		"Nothing like that in hell, earth, or heaven.\n\r" , ch );

    return;


}

void do_numlock( CHAR_DATA *ch, char *argument )  /*By Globi*/
{
           CHAR_DATA *rch;
	   char       buf  [ MAX_STRING_LENGTH ];
	   char       arg1 [ MAX_INPUT_LENGTH  ];
    extern int        numlock;
           int        temp;

    rch = get_char( ch );

    if ( !authorized( rch, "numlock" ) )
        return;

    argument = one_argument( argument, arg1 );
    temp = atoi( arg1 );

    if ( arg1[0] == '\0' ) /* Prints out the current value */
    {
	sprintf( buf, "Current numlock setting is:  %d.\n\r", numlock );
	send_to_char(AT_RED, buf, ch );
	return;
    }

    if ( ( temp < 0 ) || ( temp > LEVEL_DEMIGOD ) )
    {
	sprintf( buf, "Level must be between 0 and %d.\n\r", LEVEL_DEMIGOD );
	send_to_char(AT_WHITE, buf, ch );
	return;
    }

    numlock = temp;  /* Only set numlock if arg supplied and within range */

    if ( numlock != 0 )
    {
	sprintf( buf, "Game numlocked to levels %d and below.\n\r", numlock );
	send_to_char(AT_RED, buf, ch );
    }
    else
        send_to_char(AT_RED, "Game now open to all levels.\n\r", ch );

    return;

}

void do_smite( CHAR_DATA *ch, char *argument )          /* by Garion */
{
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char log_buf[MAX_STRING_LENGTH];

    if ( !authorized( ch, "smite" ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char(AT_WHITE,"Smite whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char(AT_WHITE,"That person is not here.\n\r", ch);
        return;
    }

    if ( victim == ch )
    {
        send_to_char(AT_WHITE,"Take it somewhere else, Jack.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) && get_trust(victim) > get_trust(ch) )
    {
        send_to_char(AT_WHITE,"You failed.\n\r", ch);
        act(AT_BLOOD,"$n tried to smite you.", ch, NULL, victim, TO_VICT );
        return;
    }

    if ( victim->fighting != NULL )
        stop_fighting( victim, TRUE );

sprintf(buf,"       *     W     W   H  H   AA    M   M   !!     *       \n\r");
strcat(buf, "     *****    W W W    HHHH  AAAA   M M M   !!   *****     \n\r");
strcat(buf, "       *      W   W    H  H  A  A  M     M  !!     *       \n\r");
strcat( buf, "\n\r" );
send_to_char(AT_BLOOD, buf, victim );

    act(AT_WHITE,"$n raises $s hand and smites you!", ch, NULL, victim, TO_VICT );
    act(AT_WHITE,"$n raises $s hand and smites $N!", ch, NULL, victim, TO_NOTVICT );
    act(AT_WHITE,"You raise your hand and smite $N!", ch, NULL, victim, TO_CHAR );
    sprintf( log_buf, "%s unleashes the wrath of the gods on %s!", ch->name, victim->name );
    log_string( log_buf, CHANNEL_INFO, ch->level -1 );
    if ( victim->hit >= 2 )
        victim->hit /= 2;

    if ( ( obj = get_eq_char( victim, WEAR_FEET ) ) != NULL )
    {
    obj_from_char( obj );
    send_to_char(AT_WHITE,"You are blown out of your shoes and right onto your ass!\n\r", victim );
    act(AT_WHITE,"$N is blown out of $s shoes and right onto $s ass!", ch, NULL, victim, TO_NOTVICT );
    act(AT_WHITE,"$N is blown out of $s shoes and right onto $s ass!", ch, NULL, victim, TO_CHAR );
    obj_to_room( obj, victim->in_room );
    }
    else
    {
        send_to_char(AT_WHITE,"You are knocked on your ass!\n\r", victim );
        act(AT_WHITE,"$N is knocked on his ass!", ch, NULL, victim, TO_NOTVICT );
        act(AT_WHITE,"$N is knocked on his ass!", ch, NULL, victim, TO_CHAR );
    }

    victim->position = POS_RESTING;
    return;
}

void do_dog( CHAR_DATA *ch, char *argument )
{
   /* A real silly command which switches the (mortal) victim into
    * a mob.  As the victim is mortal, they won't be able to use
    * return ;P  So will have to be released by someone...
    * -S-
    */

    ROOM_INDEX_DATA *location;
    MOB_INDEX_DATA  *pMobIndex;
    CHAR_DATA       *mob;
    CHAR_DATA       *victim;

    if ( !authorized( ch, "dog" ) )
        return;

    if ( argument[0] == '\0' )
    {
       send_to_char(AT_WHITE, "Turn WHO into a little doggy?\n\r", ch );
       return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
        send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
       send_to_char(AT_WHITE, "Cannot do this to mobs, only pcs.\n\r",ch);
       return;
    }

    if ( ( pMobIndex = get_mob_index( MOB_VNUM_DOGGY ) ) == NULL )
    {
       send_to_char(AT_WHITE, "Couldn't find the doggy's vnum!!\n\r", ch );
       return;
    }

    if ( victim->desc == NULL )
    {
       send_to_char(AT_WHITE, "Already switched, like.\n\r", ch );
       return;
    }

    if ( get_trust(victim) >= get_trust(ch) )
    {
        send_to_char(AT_WHITE, "You cannot dog your peer nor your superior.\n\r", ch );
        return;
    }

    mob = create_mobile( pMobIndex );
    location = victim->in_room;         /* Remember where to load doggy! */
    char_from_room( victim );

    char_to_room( victim, get_room_index( ROOM_VNUM_LIMBO ) );
    char_to_room( mob, location );


    /* Instead of calling do switch, just do the relevant bit here */
    victim->desc->character = mob;
    victim->desc->original  = victim;
    mob->desc               =  victim->desc;
    victim->desc            = NULL;

    act(AT_WHITE, "$n is suddenly turned into a small doggy!!", victim,NULL, NULL, TO_ROOM );
    send_to_char(AT_WHITE, "You suddenly turn into a small doggy!", victim );
    send_to_char(AT_WHITE, "Ok.\n\r", ch );
    return;
}


void do_newlock( CHAR_DATA *ch, char *argument )
{
           CHAR_DATA *rch;
    extern int        numlock;
           char       buf [ MAX_STRING_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "newlock" ) )
        return;

    if ( numlock != 0 && get_trust( ch ) < L_SEN )
    {
	send_to_char(AT_WHITE, "You may not change the current numlock setting\n\r",
		     ch );
	sprintf( buf, "Game numlocked to levels %d and below.\n\r", numlock );
	send_to_char(AT_RED, buf, ch );
	return;
    }

    if ( numlock != 0 )
    {
	sprintf( buf, "Game numlocked to levels %d and below.\n\r", numlock );
	send_to_char(AT_RED, buf, ch );
	send_to_char(AT_RED, "Changing to: ", ch );
    }

    numlock = 1;
    send_to_char(AT_RED, "Game locked to new characters.\n\r", ch );
    return;

}

bool sreset = TRUE; /*Is it changable anymore?*/
void do_sstime( CHAR_DATA *ch, char *argument )
{
           CHAR_DATA *rch;
    extern char      *down_time;
    extern char      *warning1;
    extern char      *warning2;
    extern int        stype;
           char       buf  [ MAX_STRING_LENGTH ];
           char       arg1 [ MAX_INPUT_LENGTH  ];
           char       arg2 [ MAX_INPUT_LENGTH  ];

    rch = get_char( ch );

    if ( !authorized( rch, "sstime" ) )
        return;

    if ( !sreset )
    {
      sprintf(buf,"%s locked!\n\r\n\r", (stype == 0 ? "Reboot" : "Shutdown"));
      send_to_char(AT_WHITE+AT_BLINK, buf, ch);
      sprintf(buf,
	      "1st warning:  %s\n\r2nd warning:  %s\n\r   Downtime:  %s\n\r",
	      warning1, warning2, down_time );
      send_to_char(AT_RED, buf, ch);
      sprintf(buf, "Sstime type: %s.\n\r", (stype == 0 ? "reboot" : "shutdown"));
      send_to_char(AT_RED, buf, ch);
      return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    strcpy ( arg2, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' ||
       ( strlen( arg2 ) != 8 && str_cmp(arg2, "*") &&
	str_cmp(arg1, "type") ) )
	
      {
	send_to_char(AT_WHITE, "Syntax: sstime <field> <value>\n\r",               ch );
	send_to_char(AT_WHITE, "\n\r",                                             ch );
	send_to_char(AT_WHITE, "Field being one of:\n\r",                          ch );
	send_to_char(AT_WHITE, "  downtime  1warning  2warning\n\r",               ch );
	send_to_char(AT_WHITE, "\n\r",                                             ch );
	send_to_char(AT_WHITE, "Value being format of:\n\r",                       ch );
	send_to_char(AT_WHITE, "  hh:mm:ss  (military time) or  *  (for off)\n\r", ch );
	send_to_char(AT_WHITE, "\n\r",                                             ch );
	sprintf( buf,
		"1st warning:  %s\n\r2nd warning:  %s\n\r   Downtime:  %s\n\r",
		warning1, warning2, down_time );
	send_to_char(AT_RED, buf,                                                ch );
	sprintf( buf, "Sstime type: %s.\n\r", ( stype == 0 ? "reboot" : "shutdown" ) );
	send_to_char(AT_RED, buf, ch );
	return;
      }

    /* Set something */

    if ( !str_infix( arg1, "downtime" ) )
      {
	free_string( down_time );
	down_time = str_dup( arg2 );
	sprintf( buf, "Downtime is now set to:  %s\n\r", down_time );
	send_to_char(AT_RED, buf, ch );
	return;
      }
    if ( !str_infix( arg1, "1warning" ) )
      {
	free_string( warning1 );
	warning1 = str_dup( arg2 );
	sprintf( buf, "First warning will be given at:  %s\n\r", warning1 );
	send_to_char(AT_RED, buf, ch );
	return;
      }
    if ( !str_infix( arg1, "2warning" ) )
      {
	free_string( warning2 );
	warning2 = str_dup( arg2 );
	sprintf( buf, "Second warning will be given at:  %s\n\r", warning2 );
	send_to_char(AT_RED, buf, ch );
	return;
      }
    if ( !str_infix( arg1, "type" ) )
    {
      if ( is_number( arg2 ) )
        stype = atoi( arg2 );
      else
       {
         if ( !str_cmp( arg2, "reboot" ) )
           stype = 0;
         else if ( !str_cmp( arg2, "shutdown" ) )
           stype = 1;
         else
           {
             send_to_char( AT_RED, "Invalid sstime type.\n\r", ch );
             return;
           }
       }
       sprintf( buf, "SStype set to: %s.\n\r", ( stype == 0 ? "reboot" : 
"shutdown" ) );
       send_to_char( AT_RED, buf, ch );
       return;
    }

    /* Generate usage mesage */

    do_sstime( ch, "" );
    return;

}


void do_pwhere( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *wch;
  CHAR_DATA *victim;

  if (IS_NPC( ch ) )
    return;
  
  if ( argument[0] == '\0' )
  {
    for ( wch = descriptor_list; wch; wch = wch->next )
    {
	victim = (wch->original ) ? wch->original : wch->character;

	if ( wch->connected != CON_PLAYING )
	    continue;
	if ( !( wch->character ) )
	    continue;
	if ( IS_SET( victim->act, PLR_WIZINVIS ) && victim->wizinvis > get_trust( ch ) )
	    continue;
	sprintf( buf, "%-10s  &B%-5d &C%-62s\n\r", (wch->character->name) ?
	wch->character->name : "", (wch->character->in_room->vnum) ?
	wch->character->in_room->vnum : 0, 
	    (wch->character->in_room->name) ? wch->character->in_room->name :
	    "" );
	send_to_char( AT_WHITE, buf, ch );
    }
    return;
  }
  else
  {
    argument = one_argument( argument, arg1 );
    for ( wch = descriptor_list; wch; wch = wch->next )
    {
      victim = (wch->original ) ? wch->original : wch->character;
      if ( wch->connected != CON_PLAYING )
	continue;
      if ( !( wch->character ) )
        continue;
      if ( !is_name( wch->character->name, arg1 ) )
        continue;
      if ( IS_SET( victim->act, PLR_WIZINVIS ) && victim->wizinvis > get_trust( ch ) )
        continue;
      sprintf( buf, "%-10s  &B%-5d &C%-62s\n\r", wch->character->name, wch->character->in_room->vnum,
         wch->character->in_room->name );
      send_to_char( AT_WHITE, buf, ch );
       return;
    }
  }
  return;
}
  
void do_newcorpse( CHAR_DATA *ch, char *argument )
{   
    FILE      *fp;
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    OBJ_DATA  *corpse;
    OBJ_DATA  *obj;
    char       strsave [ MAX_INPUT_LENGTH  ];
    char       buf     [ MAX_STRING_LENGTH ];
    char       arg1    [ MAX_INPUT_LENGTH  ];
    char       arg2    [ MAX_INPUT_LENGTH  ];
    int        c, number;
    int        corpse_cont[1024];
    int        item_level[1024];
    int        checksum1, checksum2;
        
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    rch = get_char( ch );

    if ( !authorized( rch, "newcorpse" ) )
        return;
   
    /* Show usage */
    if ( arg1[0] == '\0' && arg2[0] == '\0' )
    {
       send_to_char( AT_GREY, "Syntax:  newcorpse <playername>\n\r",            ch );
       send_to_char( AT_GREY, "         newcorpse <playername> <corpse #>\n\r", ch );
       return;
    }

    if ( !( victim = get_char_world( ch, arg1 ) ) || victim->in_room != ch->in_room )
    {
        send_to_char( AT_GREY, "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( C_DEFAULT, "Only on players.\n\r", ch );
        return;
    }   

    if ( victim->pcdata->corpses == 0 )
    {
        send_to_char( AT_GREY, "That player has no corpses on file.\n\r", ch );
        return;
    }
    /* Okay, the victim has a corpse file let's get 'em*/
    
    
    fclose( fpReserve );
    
    /* player files parsed directories by Yaz 4th Realm */
#if !defined( macintosh ) && !defined( MSDOS )
    sprintf( strsave, "%s%s%s%s.cps", PLAYER_DIR, 
	initial( victim->name ), "/", capitalize( victim->name ) );
#else
    sprintf( strsave, "%s%s.cps", PLAYER_DIR, capitalize( victim->name ) );
#endif
       
    if ( !( fp = fopen( strsave, "r" ) ) )
    {
        sprintf( buf, "New Corpse: fopen %s: ", victim->name );
        bug( buf, 0 );
        sprintf( buf, "No existing corpse file for %s.\n\r", 
                 victim->name );
        send_to_char( AT_WHITE, buf, ch );
        perror( strsave );
        return;
    }
    else
    {
        corpse_cont[0] = fread_number( fp );
        item_level[0]  = fread_number( fp );
    
        for ( c = 1 ; c < corpse_cont[0]+2 ; c++ )
            {
            corpse_cont[c] = fread_number ( fp );
            item_level[c]  = fread_number ( fp );
            }
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

    /* Ok now we have the data on all the corpses. now what? */
    
    if ( arg2[0] == '\0' ) /* show 'em */
    {
        sprintf( buf, "%s's corpse:\n\r", victim->name );
        send_to_char( AT_GREY, buf, ch );
        send_to_char( AT_GREY, "Corpse    Contents   Flag\n\r", ch   );
        send_to_char( AT_GREY, "---------------------------\n\r", ch );
        sprintf( buf, "  %d          %3d   ", 1, corpse_cont[0] );
        send_to_char( AT_GREY, buf, ch );
        checksum1=0;
        checksum2=0;
        for ( c = 1 ; c < corpse_cont[0]+1; c++ )
            {
            checksum1 += corpse_cont[c];
            checksum2 += item_level[c];
            }
        if ( checksum1 == corpse_cont[c]
             && checksum2 == item_level[c] )
            send_to_char( AT_GREY, "Valid\n\r", ch );
        else 
            send_to_char( AT_RED, "Invalid\n\r", ch );
        return;  
    }

    if (is_number( arg2 ) )
    {
        number = atoi( arg2 );
        if ( number != 1 )
        {
            send_to_char( AT_GREY, "Corpse number must be 1\n\r", ch );
            return;
        }
        /* Ok now we've done all the checks, let's try making a corpse */
        
        corpse        = create_object( get_obj_index( OBJ_VNUM_CORPSE_PC ),
                                   0 );
        corpse->timer = -1;
             
        sprintf( buf, corpse->short_descr, victim->name );
        free_string( corpse->short_descr );
        corpse->short_descr = str_dup( buf );
        
        sprintf( buf, corpse->description, victim->name );
        free_string( corpse->description );
        corpse->description = str_dup( buf );
            
        for ( c = 1 ; c < corpse_cont[0]+1 ; c++ )
        {

             obj = create_object( get_obj_index( corpse_cont[c] ),
                                  item_level[c] );
             obj_to_obj( obj, corpse );
        }
        act( AT_GREY, "You create a $p.", ch, corpse, NULL, TO_CHAR );
        act( AT_GREY, "$n has created a $p!", ch, corpse, NULL, TO_ROOM );

        obj_to_room( corpse , ch->in_room );
        
    }
    return;
}
