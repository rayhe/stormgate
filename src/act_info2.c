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

/*$Id: act_info2.c,v 1.17 2005/04/10 16:29:00 tyrion Exp $*/
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


/*
 * Contributed by Grodyn.
 */
void do_config( CHAR_DATA *ch, char *argument )
{
    char arg [ MAX_INPUT_LENGTH ];

    if ( IS_NPC( ch ) )
	return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char(AT_BLOOD, "[ Keyword  ] Option\n\r", ch );

	send_to_char(AT_RED,  IS_SET( ch->act, PLR_AUTOEXIT  )
            ? "[+AUTOEXIT ] You automatically see exits.\n\r"
	    : "[-autoexit ] You don't automatically see exits.\n\r"
	    , ch );

	send_to_char(AT_RED,  IS_SET( ch->act, PLR_AUTOGOLD  )
	    ? "[+AUTOGOLD ] You automatically get gold from corpses.\n\r"
	    : "[-autogold ] You don't automatically get gold from corpses.\n\r"
	    , ch );

	send_to_char(AT_RED,  IS_SET( ch->act, PLR_AUTOLOOT  )
	    ? "[+AUTOLOOT ] You automatically loot corpses.\n\r"
	    : "[-autoloot ] You don't automatically loot corpses.\n\r"
	    , ch );

	send_to_char(AT_RED,  IS_SET( ch->act, PLR_AUTOSAC   )
	    ? "[+AUTOSAC  ] You automatically sacrifice corpses.\n\r"
	    : "[-autosac  ] You don't automatically sacrifice corpses.\n\r"
	    , ch );

	send_to_char(AT_RED,  IS_SET( ch->act, PLR_ANSI     )
	    ? "[+ANSI     ] You have ansi color enabled.\n\r"
	    : "[-ansi     ] You have ansi color disabled.\n\r"
	    , ch );

	send_to_char(AT_RED,  IS_SET( ch->act, PLR_KEEPTITLE     )
	    ? "[+KEEP     ] You keep the same title when leveling.\n\r"
	    : "[-keep     ] You have the MUD select a new title for you.\n\r"
	    , ch );

	send_to_char(AT_RED,  IS_SET( ch->act, PLR_BLANK     )
	    ? "[+BLANK    ] You have a blank line before your prompt.\n\r"
	    : "[-blank    ] You have no blank line before your prompt.\n\r"
	    , ch );

	send_to_char(AT_RED,  IS_SET( ch->act, PLR_BRIEF     )
	    ? "[+BRIEF    ] You see brief descriptions.\n\r"
	    : "[-brief    ] You see long descriptions.\n\r"
	    , ch );
         
	send_to_char(AT_RED,  IS_SET( ch->act, PLR_COMBINE   )
	    ? "[+COMBINE  ] You see object lists in combined format.\n\r"
	    : "[-combine  ] You see object lists in single format.\n\r"
	    , ch );

	send_to_char(AT_RED,  IS_SET( ch->act, PLR_PROMPT    )
	    ? "[+PROMPT   ] You have a prompt.\n\r"
	    : "[-prompt   ] You don't have a prompt.\n\r"
	    , ch );

	send_to_char(AT_RED,  IS_SET( ch->act, PLR_TELNET_GA )
	    ? "[+TELNETGA ] You receive a telnet GA sequence.\n\r"
	    : "[-telnetga ] You don't receive a telnet GA sequence.\n\r"
	    , ch );

	send_to_char(AT_RED,  IS_SET( ch->act, PLR_COMBAT )
	    ? "[+COMBAT   ] You see all combat scroll.\n\r"
	    : "[-combat   ] You do not see dodge/parry/miss in combat.\n\r"
	    , ch );

	send_to_char(AT_RED, IS_SET( ch->act, PLR_SOUND )
	    ? "[+SOUND    ] You hear all sounds.  See HELP 'SOUND EFFECTS'.\n\r"
	    : "[-sound    ] You do not hear sounds.  See HELP 'SOUND EFFECTS'.\n\r"
	    , ch );

	send_to_char(AT_RED, IS_SET( ch->act, PLR_MUSIC )
	    ? "[+MUSIC    ] You hear all midi.  See HELP 'SOUND EFFECTS'.\n\r"
	    : "[-music    ] You do not hear midi.  See HELP 'SOUND EFFECTS.\n\r"
	    , ch );

	send_to_char(AT_RED, IS_SET( ch->act, PLR_GHOST )
	    ? "[+GHOST    ] You turn into a ghost upon death.\n\r"
	    : "[-ghost    ] You do not turn into a ghost upon death.\n\r"
	    , ch );

	send_to_char(AT_RED,  IS_SET( ch->act, PLR_SILENCE   )
	    ? "[+SILENCE  ] You are silenced.\n\r"
	    : ""
	    , ch );

	send_to_char(AT_RED, !IS_SET( ch->act, PLR_NO_EMOTE  )
	    ? ""
	    : "[-emote    ] You can't emote.\n\r"
	    , ch );

	send_to_char(AT_RED, !IS_SET( ch->act, PLR_NO_TELL   )
	    ? ""
	    : "[-tell     ] You can't use 'tell'.\n\r"
	    , ch );
    }
    else
    {
	char buf [ MAX_STRING_LENGTH ];
	int  bit;
	bool fSet;

	     if ( arg[0] == '+' ) fSet = TRUE;
	else if ( arg[0] == '-' ) fSet = FALSE;
	else
	{
	    send_to_char(AT_BLOOD, "Config -option or +option?\n\r", ch );
	    return;
	}

             if ( !str_cmp( arg+1, "autoexit" ) ) bit = PLR_AUTOEXIT;
	else if ( !str_cmp( arg+1, "autoloot" ) ) bit = PLR_AUTOLOOT;
	else if ( !str_cmp( arg+1, "autogold" ) ) bit = PLR_AUTOGOLD;
	else if ( !str_cmp( arg+1, "autosac"  ) ) bit = PLR_AUTOSAC;
	else if ( !str_cmp( arg+1, "autosac"  ) ) bit = PLR_AUTOGOLD;
	else if ( !str_cmp( arg+1, "blank"    ) ) bit = PLR_BLANK;
	else if ( !str_cmp( arg+1, "brief"    ) ) bit = PLR_BRIEF;
	else if ( !str_cmp( arg+1, "combine"  ) ) bit = PLR_COMBINE;
        else if ( !str_cmp( arg+1, "prompt"   ) ) bit = PLR_PROMPT;
	else if ( !str_cmp( arg+1, "telnetga" ) ) bit = PLR_TELNET_GA;
	else if ( !str_cmp( arg+1, "ansi"     ) ) bit = PLR_ANSI;
	else if ( !str_cmp( arg+1, "keep"     ) ) bit = PLR_KEEPTITLE;
	else if ( !str_cmp( arg+1, "combat"   ) ) bit = PLR_COMBAT;
	else if ( !str_cmp( arg+1, "sound"    ) ) bit = PLR_SOUND;
	else if ( !str_cmp( arg+1, "music"    )	) bit = PLR_MUSIC;
	else if ( !str_cmp( arg+1, "ghost"    ) ) bit = PLR_GHOST;
	else
	{
	    send_to_char(AT_BLOOD, "Config which option?\n\r", ch );
	    return;
	}

	if( !IS_IMMORTAL( ch ) )
	{
	    if( bit == PLR_GHOST && ch->level > 50 && !fSet)
	    {
	        send_to_char(AT_BLOOD, "You can't.\n\r", ch );
	        return;
	    }
	}

	if ( fSet )
	{
	    SET_BIT    ( ch->act, bit );
	    sprintf( buf, "%s is now ON.\n\r", arg+1 );
	    buf[0] = UPPER( buf[0] );
	    send_to_char(AT_RED, buf, ch );
	}
	else
	{
	    REMOVE_BIT ( ch->act, bit );
	    sprintf( buf, "%s is now OFF.\n\r", arg+1 );
	    buf[0] = UPPER( buf[0] );
	    send_to_char(AT_RED, buf, ch );
	}

    }

    return;
}

void do_wizlist ( CHAR_DATA *ch, char *argument )
{

    do_help ( ch, "wizlist" );
    return;

}

void do_spells ( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH ];
    int  sn;
    int  col;

    if ( IS_NPC( ch )
	|| ( !IS_NPC( ch ) && !class_table[ch->class].fMana ) )
    {  
       send_to_char ( AT_BLUE, "You do not know how to cast spells!\n\r", ch );
       return;
    }

    col = 0;
    for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
    {
        if ( !skill_table[sn].name )
	   break;
	if ( ch->level < skill_table[sn].skill_level[ch->class] )
	   continue;
	if ( !skill_table[sn].is_spell )
	   continue;

	if (( ch->class != 9 )&&( ch->class != 11))
	   sprintf ( buf, "%26s %3dpts ",
               skill_table[sn].name, MANA_COST( ch, sn ) );
        else
           sprintf( buf, "%26s %2dpts ",
               skill_table[sn].name, (MANA_COST( ch, sn)/5) );
	send_to_char( AT_BLUE, buf, ch );
	if ( ++col % 2 == 0 )
	   send_to_char( AT_BLUE, "\n\r", ch );
    }

    if ( col % 2 != 0 )
      send_to_char( AT_BLUE, "\n\r", ch );

    return;

}

void do_slist ( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH ];
    int  sn;
    int  col;
    int  level;
    int  profession;
    int  profession2;
    int  iClass;
    bool pSpell;

    profession = ch->class;
    profession2 = ch->multied;
    for ( ;; )
      { char arg [MAX_STRING_LENGTH];
        argument = one_argument ( argument, arg );
        if ( arg[0] == '\0' ) 
          break;
        if (strlen(arg) < 3)
          {
            send_to_char(AT_GREEN, "Thats not a class\n\r", ch );
            return;
          }
        arg[3] = '\0';
        for (iClass = 0; iClass < MAX_CLASS; iClass++)
          {
            if (!str_cmp(arg,class_table[iClass].who_name))
              {
                 profession = iClass;
		 profession2 = iClass;
                 break;
              }
           }
         if (iClass == MAX_CLASS)
           {
              send_to_char(AT_GREEN, "Thats not a class\n\r", ch );
              return;
           }     
      }
    if ( IS_NPC( ch ) )
    {  
       send_to_char (AT_BLUE, "You do not need any stinking spells!\n\r", ch );
       return;
    }

    send_to_char( AT_BLUE, "Lv          Spells/Skill\n\r\n\r", ch );

    for ( level = 1; level <= LEVEL_DEMIGOD; level++ )
    {

      col = 0;
      pSpell = TRUE;

      for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
      {
	if ( !skill_table[sn].name )
	  break;
	if ( skill_table[sn].skill_level[profession] != level && skill_table[sn].skill_level[profession2] != level )
	  continue;
	  /* Added a check to stop double display of skills from both classes 
	     --Manaux */

	  if (UMIN ( skill_table[sn].skill_level[profession], 
		     skill_table[sn].skill_level[profession2] ) != level )
	    continue;

	if ( pSpell )
	{
	  sprintf ( buf, "%2d:", level );
	  send_to_char( AT_BLUE, buf, ch );
	  pSpell = FALSE;
	}

	if ( ++col % 4 == 0 )
	  send_to_char( AT_BLUE, "   ", ch );

	sprintf ( buf, "%25s", skill_table[sn].name );
	send_to_char( AT_BLUE, buf, ch );

	if ( col % 3 == 0 )
	  send_to_char(AT_BLUE, "\n\r", ch );

      }

      if ( col % 3 != 0 )
	send_to_char( AT_BLUE, "\n\r", ch );

    }

    return;

}

/* bypassing the config command - Kahn */

void do_autoexit ( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];

    ( IS_SET ( ch->act, PLR_AUTOEXIT )
     ? sprintf( buf, "-autoexit" )
     : sprintf( buf, "+autoexit" ) );

    do_config( ch, buf );

    return;

}

void do_autoloot ( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];

    ( IS_SET ( ch->act, PLR_AUTOLOOT )
     ? sprintf( buf, "-autoloot" )
     : sprintf( buf, "+autoloot" ) );

    do_config( ch, buf );

    return;
}

void do_autosac ( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];

    ( IS_SET ( ch->act, PLR_AUTOSAC )
     ? sprintf( buf, "-autosac" )
     : sprintf( buf, "+autosac" ) );

    do_config( ch, buf );

    return;

}

void do_blank ( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];

    ( IS_SET ( ch->act, PLR_BLANK )
     ? sprintf( buf, "-blank" )
     : sprintf( buf, "+blank" ) );

    do_config( ch, buf );

    return;

}

void do_brief ( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];

    ( IS_SET ( ch->act, PLR_BRIEF )
     ? sprintf( buf, "-brief" )
     : sprintf( buf, "+brief" ) ) ; 

    do_config( ch, buf );

    return;

}

void do_combine ( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];

    ( IS_SET ( ch->act, PLR_COMBINE )
     ? sprintf( buf, "-combine" )
     : sprintf( buf, "+combine" ) );

    do_config( ch, buf );

    return;

}
 
void do_pagelen ( CHAR_DATA *ch, char *argument )
{
    char buf [ MAX_STRING_LENGTH ];
    char arg [ MAX_INPUT_LENGTH  ];
    int  lines;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	lines = 20;
    else
	lines = atoi( arg );

    if ( lines < 1 )
    {
	send_to_char(C_DEFAULT,
		"Negative or Zero values for a page pause are not legal.\n\r",
		     ch );
	return;
    }

    if ( lines > 60 )
    {
	send_to_char(C_DEFAULT,
		"I don't know of a screen that is larger than 60 lines!\n\r",
		     ch );
	lines = 60;
    }

    ch->pcdata->pagelen = lines;
    sprintf( buf, "Page pause set to %d lines.\n\r", lines );
    send_to_char(C_DEFAULT, buf, ch );
    return;
}

/* Do_prompt from Morgenes from Aldara Mud */
void do_prompt( CHAR_DATA *ch, char *argument )
{
   char buf [ MAX_STRING_LENGTH ];

   buf[0] = '\0';
   ch = ( ch->desc->original ? ch->desc->original : ch->desc->character );

   if ( argument[0] == '\0' )
   {
       ( IS_SET ( ch->act, PLR_PROMPT )
	? sprintf( buf, "-prompt" )
	: sprintf( buf, "+prompt" ) );

       do_config( ch, buf );

       return;
   }
   
   if( !strcmp( argument, "all" ) )
      strcat( buf, "<%hhp %mm %vmv> ");
   else
   {
      if ( strlen( argument ) > 50 )
	  argument[50] = '\0';
      smash_tilde( argument );
      if ( strlen( argument ) < 2 )
        sprintf( buf, "%s ", argument );
     else
      strcat( buf, argument );
   }

   free_string( ch->prompt );
   ch->prompt = str_dup( buf );
   send_to_char(C_DEFAULT, "Ok.\n\r", ch );
   return;
} 

void do_auto( CHAR_DATA *ch, char *argument )
{

    do_config( ch, "" );
    return;

}

void do_afk( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( IS_SET( ch->act, PLR_AFK ) )
    {
        REMOVE_BIT( ch->act, PLR_AFK );
        send_to_char(AT_WHITE, "You are back at your keyboard.\n\r", ch  );
        act(AT_WHITE, "$n has returned to $s keyboard.", ch, NULL, ch, TO_ROOM );
    }
    else
    {
        SET_BIT( ch->act, PLR_AFK );
        send_to_char(AT_WHITE, "You are now away from keyboard.\n\r", ch );
        act(AT_WHITE, "$n has left $s keyboard.", ch, NULL, ch, TO_ROOM );
    }

    return;
}

int skill_irongrip( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;
    
	if (is_affected(ch, sn))
	{
		send_to_char(AT_BLUE, "You are already holding your weapon as tightly as you can!\n\r", ch);
		return SKPELL_BOTCHED;
	}

    af.type      = sn;
    af.duration  = ch->level / 6;
    af.location  = APPLY_ANTI_DIS;
    af.modifier  = ch->level - ( ch->level / 4 );
    af.bitvector = 0;
    affect_to_char( ch, &af );
    send_to_char( AT_BLUE, "You grip your weapon tightly.\n\r", ch );
    return SKPELL_NO_DAMAGE;
}

void do_induct( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA  *victim;
    char        arg [MAX_STRING_LENGTH];
    char const *clname;
    char        buf [MAX_STRING_LENGTH];
    CLAN_DATA  *pClan;
    
    buf[0] = '\0';
    one_argument( argument, arg );
    if ( ( ch->clan == 0 )
        || ( ch->clev < 2 ) )
           return;
    if ( ! ( victim = get_char_room( ch, arg ) ) )    
       {
        send_to_char( AT_WHITE, "No such person is in the room.\n\r", ch );
        return;
       }
    if IS_NPC(victim)
       return;
    if ( victim->clan != 0 )
       return;
    pClan=get_clan_index(ch->clan);
    if ( !pClan )
      return;
    if (pClan->members > 19)
        {
        send_to_char(AT_WHITE, "Your clan contains 20 people, you can't induct anymore!\n\r",ch);
        return;
	}
    pClan->members++;
    clname = pClan->name;
    sprintf( buf + strlen( buf ), "<%s", clname ); 
    act(AT_RED, "$n has been inducted into $T.", victim, NULL, buf, TO_ROOM);
    act(AT_RED, "You are now a member of $T.", victim, NULL, buf, TO_CHAR);
    act(AT_RED, "You have inducted $N.", ch, NULL, victim, TO_CHAR);
    victim->clan = ch->clan;
    victim->clev = 0;
    return;
}

void do_outcast( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA  *victim;
    char        arg [MAX_STRING_LENGTH];
    char const *clname;
    char        buf [MAX_STRING_LENGTH];
    CLAN_DATA  *pClan;
    
    buf[0] = '\0';
    one_argument( argument, arg );
    if ( ( ch->clan == 0 )
        || ( ch->clev < 2 ) )
           return;
    if ( !( victim = get_char_room( ch, arg ) ) )
       {
        send_to_char( AT_WHITE, "No such person is in the room.\n\r", ch );
        return;
       }
    if IS_NPC(victim)
       return;
    if ( ( victim->clan == 0 ) || ( victim->clan != ch->clan ) )
       return;
    pClan=get_clan_index(ch->clan);
    if ( !pClan )
      return;
    pClan->members--;
    clname = pClan->name;
    sprintf ( buf + strlen( buf ), "<%s", clname );
    act(AT_RED, "$n has been outcast from $T.", victim, NULL, buf, TO_ROOM);
    act(AT_RED, "You are no longer a member of $T.", victim, NULL, buf, TO_CHAR);
    act(AT_RED, "You have outcast $N.", ch, NULL, victim, TO_CHAR);
    victim->clan = 0;
    victim->clev = 0;
    if ( IS_AFFECTED2(victim, AFF_DOOMSHIELD) )
      affect_strip(victim, skill_lookup("doomshield"));
    REMOVE_BIT(victim->act, PLR_CSKILL);
    return;
}

void do_setlev( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int        level;
    CLAN_DATA *pClan;
    char const *cltitle;

    cltitle = "NONE"; /* init */    

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ch->clev < 3 )
       return;
    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char(AT_WHITE, "Syntax: setlev <char> <level>.\n\r", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, arg1 ) ) )
    {
	send_to_char(AT_WHITE, "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	return;
    }
    if ( (ch->clan != victim->clan) || ( ch->clev < victim->clev ) )
       return;
       
    level = atoi( arg2 );
    if ( level > ch->clev )
      {
        send_to_char(AT_WHITE, "Not above your own level.\n\r", ch);
        return;
      }
    if ( level < 0 || level > 4 )
    {
	send_to_char(AT_WHITE, "Valid Levels are as follows\n\r", ch );
	send_to_char(AT_WHITE, "       0 -> Regular member.\n\r", ch );
	send_to_char(AT_WHITE, "       1 -> Level 1.\n\r", ch );
	send_to_char(AT_WHITE, "       2 -> Level 2.\n\r", ch );
	send_to_char(AT_WHITE, "       3 -> Level 3.\n\r", ch );
	send_to_char(AT_WHITE, "       4 -> Level 4.\n\r", ch );
	return;
    }
    pClan=get_clan_index(ch->clan);
    /* Lower a player in the Clan */
    switch ( victim->clev )
    {
    case 0: break;
    case 1:
      pClan->issecond=FALSE;
      free_string( pClan->second );
      pClan->second = str_dup( "None" );
      break;
    case 2:
      pClan->isfirst=FALSE;
      free_string( pClan->first );
      pClan->first= str_dup( "None" );
      break;
    case 3:
      pClan->isleader=FALSE;
      free_string( pClan->leader );
      pClan->leader=str_dup( "None" );
      break;
    case 4:
      pClan->ischamp=FALSE;
      free_string( pClan->champ );
      pClan->champ = str_dup( "None" );
      break;
    default: break;
   }
     switch ( level )
     {
      default: break;
      case 0: break;
      case 1:
        if (pClan->issecond)
          {
            send_to_char(AT_WHITE, "Already exists, defaulting to regular member", ch );
            level = 0;
            break;
          }
        else
          {
            pClan->issecond=TRUE;
            pClan->second = str_dup( victim->name );
            break;
          }            
      case 2:
        if (pClan->isfirst)
          {
            send_to_char(AT_WHITE, "Already exists, defaulting to regular member", ch );
            level = 0;
            break;
          }
        else
          {
            pClan->isfirst=TRUE;
            pClan->first = str_dup( victim->name );
            break;
          }            
      case 3:
        if (pClan->isleader)
          {
            send_to_char(AT_WHITE, "Already exists, defaulting to regular member", ch );
            level = 0;
            break;
          }
        else
          {
            pClan->isleader=TRUE;
            pClan->leader = str_dup( victim->name );
            break;
          }            
      case 4:
        if (pClan->ischamp)
          {
            send_to_char(AT_WHITE, "Already exists, defaulting to regular member", ch );
            level = 0;
            break;
          }
        else
          {
            pClan->ischamp=TRUE;
            pClan->champ = str_dup( victim->name );
            break;
          }            
     }
    if ( level <= victim->clev )
    {
     char  buf [MAX_STRING_LENGTH];
     buf[0] = '\0';
     switch( level )
     {
      case 0 :  cltitle = "<"; break;
      case 1 :  cltitle = "Level 1"; break;
      case 2 :  cltitle = "Level 2"; break;
      case 3 :  cltitle = "Level 3"; break;
      case 4 :  cltitle = "Level 4"; break;
      default:  cltitle = "[bug rep to imm]"; break;
     }
     sprintf( buf + strlen(buf), "%s %s>", cltitle, pClan->name );
     act(AT_BLUE, "You have been lowered to $T.", victim, NULL, buf, TO_CHAR );
     act(AT_BLUE, "Lowering a players clan level.", ch, NULL, NULL, TO_CHAR );
     act(AT_BLUE, "$n is now $T", victim, NULL, buf, TO_ROOM );
     victim->clev = level;
     return;
    }
    else
    {
     char  buf [MAX_STRING_LENGTH];
     
     buf[0] = '\0';
     switch( level )
     {
      case 0 :  cltitle = "<"; break;
      case 1 :  cltitle = "Level 1"; break;
      case 2 :  cltitle = "Level 2"; break;
      case 3 :  cltitle = "Level 3"; break;
      case 4 :  cltitle = "Level 4"; break;
     }
     sprintf( buf + strlen(buf), "%s %s", cltitle, pClan->name );
     act(AT_BLUE, "You have been raised to $T.", victim, NULL, buf, TO_CHAR );
     act(AT_BLUE, "Raising a players clan level.", ch, NULL, NULL, TO_CHAR );
     act(AT_BLUE, "$n is now $T", victim, NULL, buf, TO_ROOM );
     victim->clev = level;
     return;
    }
    return;
}

void do_smash ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *obj;
    char       buf [MAX_STRING_LENGTH];
    char       arg [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char      *name;
    
    buf[0]='\0';
    one_argument( argument, arg );
    if ( !(obj = get_obj_carry( ch, arg ) ) )
    {
     send_to_char(AT_WHITE, "You do not have that doll.\n\r", ch );
     return;
    }
    name = obj->name;
    if ( !(victim = get_char_room(ch, name) ) )
    {
     send_to_char( AT_WHITE, "That person's life cannot be sensed.\n\r", ch );
     return;
    }

    if ( !is_pkillable( ch, victim ) ) {
	return;
    }

    act(AT_RED, "You call down the Dark forces of Tyrion himself on $N.", ch, NULL, victim, TO_CHAR);
    act( AT_RED, "$n smashes $p.", ch, obj, NULL, TO_ROOM );
    if ( !victim->wait )
      act( AT_RED, "You feel a wave of nausia come over you.", victim, NULL, NULL, TO_CHAR );
    extract_obj(obj);
    if ( victim->wait )
      return;
    STUN_CHAR(victim, 3, STUN_TOTAL);
    victim->position = POS_STUNNED;
    victim->combat_timer = 90;
    update_pos( victim );
    return;
}

void do_guild(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  if(argument[0] == '\0')
  {
    send_to_char(AT_WHITE, "Guild <char>\n\r", ch);
    return;
  }
  if(ch->guild == NULL)
  {
    send_to_char(AT_BLUE, "You must be in a guild to induct someone.\n\r", ch);
    return;
  }
  if(!is_name(ch->name, ch->guild->deity))
  {
    send_to_char(AT_BLUE, "You are not the deity of this guild.\n\r", ch);
    return;
  }
  if((victim = get_char_world(ch, argument)) == NULL)
  {
    send_to_char(AT_BLUE, "Player not found.\n\r", ch);
    return;
  }
  if(IS_NPC(victim))
  {
    send_to_char(AT_BLUE, "May guild PCs only.\n\r", ch);
    return;
  }
  if(victim->guild != NULL)
  {
    send_to_char(AT_BLUE, "That person is already guilded.\n\r", ch);
  }
  victim->guild = ch->guild;
  victim->guild_rank = 0;
  act(AT_BLUE, "You guild $N.", ch, NULL, victim, TO_CHAR);
  act(AT_BLUE, "$N guilds you.", victim, NULL, ch, TO_CHAR);
  return;
}

void do_unguild(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;

  if(argument[0] == '\0')
  {
    send_to_char(AT_WHITE, "Unguild <char>\n\r", ch);
    return;
  }
  if(ch->guild == NULL)
  {
    send_to_char(AT_BLUE, "You must be in a guild to unguild someone.\n\r", ch);
    return;
  }
  if(!is_name(ch->name, ch->guild->deity))
  {
    send_to_char(AT_BLUE, "You are not the deity of this guild.\n\r", ch);
    return;
  }
  if((victim = get_char_world(ch, argument)) == NULL)
  {
    send_to_char(AT_BLUE, "Player not found.\n\r", ch);
    return;
  }
  if(victim->guild != ch->guild)
  {
    send_to_char(AT_BLUE, "That person is not in your guild.\n\r", ch);
    return;
  }
  victim->guild = NULL;
  act(AT_BLUE, "You unguild $N.", ch, NULL, victim, TO_CHAR);
  act(AT_BLUE, "$N unguilds you.", victim, NULL, ch, TO_CHAR);
  return;
}

void do_setrank(CHAR_DATA *ch, char *argument)
{
  char	arg1 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int value;

  argument = one_argument(argument, arg1);
  value = is_number(argument) ? atoi(argument) : 0;

  if(arg1[0] == '\0')
  {
    send_to_char(AT_WHITE, "Setrank <char> <rank num>\n\r", ch);
    send_to_char(AT_WHITE, " rank num > 0            \n\r", ch);
    return;
  }
  if(ch->guild == NULL)
  {
    send_to_char(AT_BLUE, "You must be in a guild to induct someone.\n\r", ch);
    return;
  }
  if(!is_name(ch->name, ch->guild->deity))
  {
    send_to_char(AT_BLUE, "You are not the deity of this guild.\n\r", ch);
    return;
  }
  if((victim = get_char_world(ch, arg1)) == NULL)
  {
    send_to_char(AT_BLUE, "Player not found.\n\r", ch);
    return;
  }
  if(victim->guild != ch->guild)
  {
    send_to_char(AT_BLUE, "That person is not in your guild.\n\r", ch);
    return;
  }
  victim->guild_rank = value;
  send_to_char(AT_BLUE, "Ok.\n\r", ch);
  return;
}

void do_clans( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA    *pClan;
    char          buf[MAX_STRING_LENGTH];
    char result [ MAX_STRING_LENGTH*2 ];	/* May need tweaking. */
    int           chance;

    if ( clan_first == NULL )
      return;

    sprintf( result, "&W[%3s&W] &W[  %13s&W] &W[  %12s&W] &W[%7s&W] &W[%6s&W] &W[%7s&W] \n\r",
       "&RNum", "&RClan Name", "&RRuler", "&RMembers", "&RPkills", "&RPkilled"  );

    for ( pClan = clan_first->next; pClan; pClan = pClan->next )
    {
        chance = 0;
        if(pClan->pkills != 0 || pClan->pdeaths != 0)
        chance = (int) ( ((float)pClan->pkills / (float)(pClan->pkills + pClan->pdeaths) ) * 100 );

	sprintf( buf, "&W[&R%3d&W] [&R%13s&W] [&R%12s&W] [&R%7d&W] [&R%6d&W] [&R%7d&W] \n\r",
	     pClan->vnum,
	     pClan->name,
	     pClan->deity,
	     pClan->members,
	     pClan->pkills,
	     pClan->pdeaths  );
	     strcat( result, buf );
    }

    send_to_char(AT_WHITE, result, ch );
    return;
}

void do_cinfo( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA    *pClan;
    char          buf[MAX_STRING_LENGTH];
    int           num;
    char          arg1[MAX_INPUT_LENGTH];
    
    argument = one_argument(argument, arg1);
    
    if (!(is_number(arg1)))
    {
      send_to_char(AT_WHITE, "Syntax:  cinfo <clan number>\n\r", ch );
      send_to_char(AT_WHITE, "Use the command clans to find a clan number.\n\r", ch );
      return;
    }
    num = atoi(arg1);
    if (!(pClan = get_clan_index(num)))
    {
     send_to_char( AT_WHITE, "Illegal clan number, please try again.\n\r", ch );
     return;
    }
    
    sprintf( buf, "------------------Information on <%s>-----------------\n\r\n\r", pClan->name );
    send_to_char(AT_WHITE, buf, ch );
    sprintf( buf, "Clan Owner   [%12s]\n\r", pClan->deity );
    send_to_char(AT_LBLUE, buf, ch );
    sprintf( buf, "Rank 4:      [%12s]\n\r", pClan->champ );
    send_to_char( AT_CYAN, buf, ch );
    sprintf( buf, "Rank 3:      [%12s]\n\r", pClan->leader );
    send_to_char( AT_BLUE, buf, ch );
    sprintf( buf, "Rank 2:      [%12s]\n\r", pClan->first );
    send_to_char( AT_DBLUE, buf, ch );
    sprintf( buf, "Rank 1:      [%12s]\n\r", pClan->second );
    send_to_char( AT_DBLUE, buf, ch );
    sprintf( buf, "Members:     [%12d]\n\r", pClan->members );
    send_to_char(AT_DBLUE, buf, ch );
     sprintf( buf, "Pkills:      [%12d]\n\r", pClan->pkills );
     send_to_char(AT_BLUE, buf, ch );
     sprintf( buf, "Pkilled:     [%12d]\n\r", pClan->pdeaths );
     send_to_char(AT_CYAN, buf, ch );
    sprintf( buf, "Mkills:      [%12d]\n\r", pClan->mkills );
    send_to_char(AT_LBLUE, buf, ch );
    sprintf( buf, "Description:\n\r%s", pClan->description );
    send_to_char( AT_WHITE, buf, ch );
    return;
}  
    
void do_autogold( CHAR_DATA *ch, char *argument )
{
   if ( IS_NPC( ch ) )
   	return;

   if ( IS_SET( ch->act, PLR_AUTOGOLD ) )
   { 
     do_config( ch, "-autogold" );
     return;
   }
   if ( !IS_SET( ch->act, PLR_AUTOGOLD ) )
   {
     do_config( ch, "+autogold" );
     return;
   }
   return;
}

/*
void do_vsi( CHAR_DATA *ch )
{
  char limbm[MAX_STRING_LENGTH];
  int limb;
  for ( limb = 1; limb <= 2; limb++ )
  {
  switch( ch->arm[limb]->hp )
  {
     case 1:
       sprintf( limbm, "&RYour fingers from your %s hand are missing, and they are %s\n\r",
        ( limb == 1 ) ? "left" : "right", bleedinglev( ch->arm[limb]->bl );
       send_to_char( AT_WHITE, limbm, ch );
       break;
     case 2:
       sprintf( limbm, "&RYour %s hand is missing, and it is %s\n\r", 
        ( limb == 1 ) ? "left" : "right", bleedinglev( ch->arm[limb]->bl );
       send_to_char( AT_WHITE, limbm, ch );
       break;
     case 3:
       sprintf( limbm, "&RYour %s arm is missing past the elbow, and it is %s\n\r", 
        ( limb == 1 ) ? "left" : "right", bleedinglev( ch->arm[limb]->bl );
       send_to_char( AT_WHITE, limbm, ch );
       break;
     case 4:
       sprintf( limbm, "&RYou &B*&RWHOLE&B*&R %s arm is missing!, and it is %s\n\r",
        ( limb == 1 ) ? "left" : "right", bleedinglev( ch->arm[limb]->bl ); 
       send_to_char( AT_WHITE, limbm, ch );
       break;
     default:
       break;
   }
   }
   
   switch( ch->neck->hp )
   {
     case 1:
       sprintf( limbm, "&RYour neck is lightly hurt, and it is %s\n\r",
        bleedinglev( ch->neck->bl ) );
       send_to_char( AT_WHITE, limbm, ch );
       break;
     case 2:
       sprintf( limbm, "&RYour neck is severely hurt, and it is %s\n\r",
        bleedinglev( ch->neck->bl ) );
       send_to_char( AT_WHITE, limbm, ch );
       break;
     case 3:
       sprintf( limbm, "&RYour neck is broken!, and it is %s\n\r",
         bleedinglev( ch->neck->bl ) );
       send_to_char( AT_WHITE, limbm, ch );
       break;
     case 4:
       sprintf( limbm, "&RYour head is missing!, and your neck is %s\n\r",
        bleedinglev( ch->neck->bl ) );
       send_to_char( AT_WHITE, limbm, ch );
       break;
     default:
       break;
   }

   switch( ch->eyes->hp )
   {
     case 1:
       sprintf( limbm, "&ROne of your eyes has been gouged out, and it is %s\n\r",
        bleedinglev( ch->eyes->bl ) );
       send_to_char( AT_WHITE, limbm, ch );
       break;
     case 2:
       sprintf( limbm, "&RBoth of your eyes have been gouged out!, they are %s\n\r",
        bleedinglev( ch->eyes->bl ) );
       send_to_char( AT_WHITE, limbm, ch );
       break;
     default:
       break;
   }
   
   switch( ch->torso->hp )
   {
     case 1:
       sprintf( limbm, "&RYour torso is lightly wounded, and it is %s\n\r",
         bleedinglev( ch->torso->bl ) );
       send_to_char( AT_WHITE, limbm, ch );
       break;
     case 2:
       sprintf( limbm, "&RYour torso is badly wounded, and it is %s\n\r",
         bleedinglev( ch->torso->bl ) );
       send_to_char( AT_WHITE, limbm, ch );
       break;
     case 3:
       sprintf( limbm, "&RYour torso is severely wounded!, and it is %s\n\r",
         bleedinglev( ch->torso->bl ) );
       send_to_char( AT_WHITE, limbm, ch );
       break;
     case 4:
       sprintf( limbm, "&RYour bodyis missing halfway through your torso and down!,and it is %s\n\r",
        bleedintlev( ch->torso->bl ) );
       send_to_char( AT_WHITE, limbm, ch );
       break;
     default:
       break;
   }
   
   for ( limb = 1; limb <= 2; limb++ )
   {
     switch( ch->leg[limb]->hp )
     {
       case 1:
         sprintf( limbm, "&RYour %s foot is missing, and it is %s\n\r",
          ( limb == 1 ) ? "left" : "right", bleedinglev( ch->leg[limb]->bl ) );
         send_to_char( AT_WHITE, limbm, ch );
         break;
       case 2:
         sprintf( limbm, "&RYour %s leg is missing from the knee down, and it is %s\n\r",
          (limb == 1 ) ? "left" : "right", bleedinglev( ch->leg[limb]->bl ) );
         send_to_char( AT_WHITE, limbm, ch );
         break;
       case 3:
         sprintf( limbm, "&RYour whole %s leg is missing!, and it is %s\n\r",
          ( limb == 1 ) ? "left" : "right", bleedinglev( ch->leg[limb]->bl ) );
         send_to_char( AT_WHITE, limbm, ch );
         break;
       default:
         break;
     }
   }
 return;
 }
 */

/*
char *bleedinglev( int blvl )
{
  switch( blvl )
  {
    case 0:
      return "not bleeding.";
    case 1:
      return "bleeding lightly.";
    case 2:
      return "bleeding a lot.";
    case 3: 
      return "bleeding severely!";
    default:
      return "";
  }
  return "";
}
*/

void do_farsight( CHAR_DATA *ch, char *argument )
{
	 send_to_char(C_DEFAULT, "Huh?\n\r", ch );
     return;
	 /*
    CHAR_DATA       *victim;
    char             target_name[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *from_room;
    
    if (IS_NPC(ch))
      return;
    if (ch->clan != -1 )
    {
      send_to_char(C_DEFAULT, "Huh?\n\r", ch );
      return;
    }
    one_argument( argument, target_name );
    
    if ( !( victim = get_char_world( ch, target_name ) )
	|| IS_SET( victim->in_room->room_flags, ROOM_PRIVATE   )
	|| IS_SET( victim->in_room->room_flags, ROOM_SOLITARY  )
	|| IS_SET( victim->in_room->area->area_flags, AREA_PROTOTYPE )
	|| IS_AFFECTED( victim, AFF_NOASTRAL ) 
        || IS_AFFECTED4(victim, AFF_DECEPTION) )
    {
	send_to_char(AT_BLUE, "You failed.\n\r", ch );
	return;
    }
    if ( IS_SET(ch->in_room->area->area_flags, AREA_PRESENT)
        && (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
        || IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
        send_to_char(AT_BLUE, "You cannot!!!!!!!\n\r", ch );
        return;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_FUTURE)
        && (IS_SET( victim->in_room->area->area_flags, AREA_PRESENT )
        || IS_SET( victim->in_room->area->area_flags, AREA_PAST ) ) )
    {
        send_to_char(AT_BLUE, "You cannot!!!!!!!\n\r", ch );
        return;
    }

    if ( IS_SET(ch->in_room->area->area_flags, AREA_PAST)
        && (IS_SET( victim->in_room->area->area_flags, AREA_FUTURE )
        || IS_SET( victim->in_room->area->area_flags, AREA_PRESENT ) ) )
    {
        send_to_char(AT_BLUE, "You cannot!!!!!!!\n\r", ch );
        return;
    }


    from_room = ch->in_room;
    if ( ch != victim )
    {
     char_from_room( ch );
     char_to_room( ch, victim->in_room );
    }
    do_look( ch, "auto" );
    if ( ch != victim )
    {
     char_from_room( ch );
     char_to_room( ch, from_room );
    }
    return;
	*/
}

void do_worth( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC( ch ) )
    return;
    
  if ( ch->level < LEVEL_HERO )
  {
  sprintf( log_buf, "You have scored a total of %dxp, and are %dxp off from your"
     " next level.\n\r", ch->exp, ( ( ch->level + 1 ) * 1000 ) - ch->exp );
  send_to_char( AT_RED, log_buf, ch );
  }
  if ( ch->level == LEVEL_HERO )
  {
  sprintf( log_buf, "You have scored a total of %dxp, and are %dxp off from your"
     " next level.\n\r", ch->exp, ( ( ch->level + 1 ) * 1000 ) + 1500 - ch->exp );
  send_to_char( AT_RED, log_buf, ch );
  }
  if ( ch->level == LEVEL_HERO1 )
  {
  sprintf( log_buf, "You have scored a total of %dxp, and are %dxp off from your"
     " next level.\n\r", ch->exp, ( ( ch->level + 1 ) * 1000 ) + 5500 - ch->exp );
  send_to_char( AT_RED, log_buf, ch );
  }
  if ( ch->level == LEVEL_HERO2 )
  {
  sprintf( log_buf, "You have scored a total of %dxp, and are %dxp off from your"
     " next level.\n\r", ch->exp, ( ( ch->level + 1 ) * 1000 ) + 12000 - ch->exp );
  send_to_char( AT_RED, log_buf, ch );
  }
  if ( ch->level == LEVEL_HERO3 )
  {
  sprintf( log_buf, "You have scored a total of %dxp, and are %dxp off from your"
     " next level.\n\r", ch->exp, ( ( ch->level + 1 ) * 1000 ) + 31000 - ch->exp );
  send_to_char( AT_RED, log_buf, ch );
  }
  if ( ch->level == LEVEL_CHAMP )
  {
  sprintf( log_buf, "You have scored a total of %dxp and are %dxp off from your"
     " next level.\n\r", ch->exp, ( ( ch->level + 1 ) * 1000 ) + 95000 - ch->exp );
  send_to_char( AT_RED, log_buf, ch );
  }
  if ( ch->level > LEVEL_CHAMP )
  {
    sprintf( log_buf, "You have scored a total of %dxp.\n\r", ch->exp );
    send_to_char( AT_RED, log_buf, ch );
  }
  if ( ch->exp >= MAX_EXPERIENCE )
  {
    send_to_char( AT_RED, "You perceive that you can level no higher by natural means.\n\r", ch );
  }
  sprintf( log_buf, "Your current stats are:\n  Str: %d.  Wis: %d.  Int: %d.  Dex: %d.  Con: %d.\n\r",
    get_curr_str( ch ), get_curr_wis( ch ), get_curr_int( ch ),
    get_curr_dex( ch ), get_curr_con( ch ) );
  send_to_char( AT_RED, log_buf, ch );
  sprintf( log_buf, "You are currently carrying %d coins.\n\r", ch->gold );
  send_to_char( AT_RED, log_buf, ch );
  return;
}

void do_finger( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH*5];
  USERL_DATA *ul;
  int nou = 0;
  int llvlr = 0;
  int hlvlr = L_IMP;
  
  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  
  if ( is_number( arg1 ) )
    llvlr = atoi( arg1 );
  if ( is_number( arg2 ) )
    hlvlr = atoi( arg2 );
  
  if ( IS_NPC( ch ) )
    return;
  
  if ( arg[0] == '\0' )
  {
    send_to_char( AT_PURPLE, "Syntax: finger <playername>\n\r", ch );
    return;
  }
  
  if ( !str_cmp( arg, "all" ) )
  {
    if ( get_trust( ch ) < L_APP )
    {
      send_to_char( AT_WHITE, "You cannot list all.\n\r", ch );
      return;
    }
    for ( ul = user_list; ul; ul = ul->next )
    {
      if ( ul->level >= llvlr && ul->level <= hlvlr )
      {
        nou++;
        sprintf( log_buf, "%-20s  Level:%3d  Class:%s/%s\n\r", ul->name, ul->level, (ul->level >= LEVEL_IMMORTAL ? "Immortal" : (ul->class == 20 ? "Unknown" : class_table[ul->class].who_long)), (ul->class == ul->multi ? "NONE" : class_table[ul->multi].who_long));
        send_to_char( AT_WHITE, log_buf, ch );
      }
    }
    sprintf( log_buf, "There are %d people in the user list.\n\r", nou );
    send_to_char( AT_WHITE, log_buf, ch );
    return;
  }
  
  if ( !str_cmp( arg, "desc" ) )
  {
    for ( ul = user_list; ul; ul = ul->next )
    {
      if ( !str_cmp( ch->name, ul->name ) )
        break;
    }
    if ( !ul )
    {
      sprintf( log_buf, "Player with no ul [%s]", ch->name );
      bug( log_buf, 0 );
      return;
    }
    if ( !str_cmp( ul->desc, "(none)" ) )
      free_string( ul->desc );
    string_append( ch, &ul->desc );
    if ( !ul->desc )
      ul->desc = str_dup( "(none)" );
    send_to_char( AT_WHITE, "Finger description changed.\n\r", ch );
    return;
  }
  
  for( ul = user_list; ul; ul = ul->next )
  {
    if ( !str_cmp( ul->name, arg ) )
      break;
  }
  
  if ( !ul )
  {
    send_to_char( AT_WHITE, "No such person exists here.\n\r", ch );
    return;
  }
 else
  {
    sprintf( log_buf, "Name: %s. ", ul->name );
    if ( ch->level > 100 )
      sprintf( buf, "Email: %s@%s  Last updated level: %d\n\r", 
           ul->user, ul->host, ul->level );
    else
      sprintf( buf, "\n\r" );
    strcat( log_buf, buf );
    send_to_char( AT_WHITE, log_buf, ch );
    sprintf(buf, "Class: %s\n\r", (ul->level >= LEVEL_IMMORTAL ? "Immortal" : (ul->class == 20 ? "Unknown" : class_table[ul->class].who_long)));
    send_to_char( AT_WHITE, buf, ch);
    sprintf(buf, "Multi: %s\n\r", (ul->class == ul->multi ? "NONE" : class_table[ul->multi].who_long));
    send_to_char( AT_WHITE, buf, ch);
    sprintf( log_buf, "Last Login: %s.\n\r", 
          ( ul->lastlogin ? ul->lastlogin : "(none)" ) );
    if ( ( ( get_trust( ch ) > 100 ) && ( ul->level > 100 ) ) ||
       ul->level < L_APP )
    send_to_char( AT_WHITE, log_buf, ch );
    sprintf( buf, "Description:\n\r%s\n\r", ul->desc );
    send_to_char( AT_WHITE, buf, ch );
    return;
  }
  return;
}
 
void do_spells2 ( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH ];
    int  sn;
    int  col;
    int  mana;

    if ( IS_NPC( ch )
	|| ( !IS_NPC( ch ) && !class_table[ch->class].fMana && !class_table[ch->multied].fMana ) )
    {  
       send_to_char ( AT_BLUE, "You do not know how to cast spells!\n\r", ch );
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
	if ( !skill_table[sn].is_spell )
	   continue;

	if ( ch->level >= skill_table[sn].skill_level[ch->class ] )
	{
	    mana = MANA_COST( ch, sn );
	}
	if ( ch->level >= skill_table[sn].skill_level[ch->multied ] )
	{
	    mana = MANA_COST_MULTI( ch, sn );
	}
	if ( ch->class == CLASS_VAMPIRE || ch->class == CLASS_ANTI_PALADIN )
	{
	    mana /= 4;
	}

	if (( ch->class != CLASS_VAMPIRE )&&( ch->class != CLASS_ANTI_PALADIN))
	   sprintf ( buf, "%26s %3dpts ",
               skill_table[sn].name, mana );
        else
           sprintf( buf, "%26s %2dpts  ",
               skill_table[sn].name, mana );
	send_to_char( AT_BLUE, buf, ch );
	if ( ++col % 2 == 0 )
	   send_to_char( AT_BLUE, "\n\r", ch );
    }

    if ( col % 2 != 0 )
      send_to_char( AT_BLUE, "\n\r", ch );

    return;

}

