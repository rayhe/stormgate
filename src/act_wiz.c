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

/*$Id: act_wiz.c,v 1.25 2005/03/31 14:17:36 ahsile Exp $*/
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
//#include <unistd.h>
#include "merc.h"

/* Conversion of Immortal powers to Immortal skills done by Thelonius */

int ch_invcount    args ( ( CHAR_DATA* ch ) );
int ch_weightcount args ( ( CHAR_DATA* ch ) ); 
extern char * mprog_type_to_name  args ( ( int type ) );
bool fight_in_progress;

void do_load(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;

    rch = get_char( ch );
    if ( !authorized( rch, "load" ) )
        return;

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char(AT_BLUE, "Syntax:\n\r",ch);
	send_to_char(AT_BLUE, "  load mob <vnum>\n\r",ch);
	send_to_char(AT_BLUE, "  load obj <vnum> <level>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_mload(ch,argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_oload(ch,argument);
	return;
    }
    /* echo syntax */
    do_load(ch,"");
}

void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);
 
    if (arg[0] == '\0')
    {
	send_to_char(AT_BLUE, "Syntax:\n\r",ch);
	send_to_char(AT_BLUE, "  vnum obj <name>\n\r",ch);
	send_to_char(AT_BLUE, "  vnum mob <name>\n\r",ch);
	send_to_char(AT_BLUE, "  vnum skill <skill or spell>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_ofind(ch,string);
 	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    { 
	do_mfind(ch,string);
	return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
	do_slookup(ch,string);
	return;
    }
    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    char       buf  [ MAX_STRING_LENGTH ];
    char       buf1 [ MAX_STRING_LENGTH ];
    int        cmd;
    int		trust;
    int        col;

    rch = get_char( ch );
    
   if (get_trust( ch ) < 100 )
        return;

    buf1[0] = '\0';
    col     = 0;

     trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].name[0] == '\0' )
	    break;

        if ( ( trust < cmd_table[cmd].level ) &&
            !IS_SET( rch->affected_by2, CODER ) )
	    continue;

	if ( cmd_table[cmd].level > 99 )
	{
	    sprintf( buf, "%-16s", cmd_table[cmd].name );
	    if ( cmd_table[cmd].level == 108 )
	    strcat( buf1, "&R" );
	    else if ( cmd_table[cmd].level > 108 )
	    strcat( buf1, "&B" );
	    else if ( cmd_table[cmd].level == 107 )
	    strcat( buf1, "&C" );
	    strcat( buf1, buf );
	    strcat( buf1, "&w" );
	    if ( ++col % 5 == 0 )
		strcat( buf1, "\n\r" );
	}
    }
 
    if ( col % 5 != 0 )
	strcat( buf1, "\n\r" );
    send_to_char(AT_GREY, buf1, ch );
    return;
}



void do_bamfin( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    rch = get_char( ch );

    if ( !authorized( rch, "bamfin" ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        if ( longstring( ch, argument ) )
	    return;

	smash_tilde( argument );
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );
	send_to_char(AT_GREY, "Ok.\n\r", ch );
    }
    return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    rch = get_char( ch );

    if ( !authorized( rch, "bamfout" ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        if ( longstring( ch, argument ) )
	    return;

	smash_tilde( argument );
	free_string( ch->pcdata->bamfout );
	ch->pcdata->bamfout = str_dup( argument );
	send_to_char(C_DEFAULT, "Ok.\n\r", ch );
    }
    return;
}



void do_deny( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "deny" ) )
        return;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char(AT_RED, "Deny whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
	send_to_char(AT_RED, "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char(AT_RED, "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char(AT_RED, "You failed.\n\r", ch );
	return;
    }
    if (IS_SET( victim->act, PLR_DENY) )
    {
	REMOVE_BIT( victim->act, PLR_DENY);
	send_to_char(AT_RED, "OK.\n\r", ch);
	send_to_char(AT_RED, "You have been Undenied!\n\r", victim );
	do_save(victim, "");
	return;
    }
    SET_BIT( victim->act, PLR_DENY );
    send_to_char(AT_RED, "You are denied access!\n\r", victim );
    send_to_char(AT_RED, "OK.\n\r", ch );
    do_quit( victim, "" );

    return;
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *rch;
    CHAR_DATA       *victim;
    DESCRIPTOR_DATA *d;
    char             arg [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "disconnect" ) )
        return;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char(AT_RED, "Disconnect whom?\n\r", ch );
	return;
    }
    if ( is_number( arg ) )
    {
      for ( d = descriptor_list; d; d = d->next )
      {
	if ( d->descriptor == (unsigned int) atoi(arg) )
	{
	  close_socket( d );
	  send_to_char( AT_RED, "Ok.\n\r",ch);
	  return;
	}
      }
      send_to_char(AT_RED, "Descriptor not found.\n\r",ch);
      return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
	send_to_char(AT_RED, "They aren't here.\n\r", ch );
	return;
    }

    if ( !victim->desc )
    {
	act(AT_RED, "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char(AT_RED, "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char(AT_RED, "Descriptor not found!\n\r", ch );
    return;
}



void do_pardon( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "pardon" ) )
        return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char(AT_BLUE, "Syntax: pardon <character> <killer|thief>.\n\r", ch );
	return;
    }

    if ( !( victim = get_char_world( ch, arg1 ) ) )
    {
	send_to_char(AT_BLUE, "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char(AT_BLUE, "Not on NPC's.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "killer" ) )
    {
	if ( IS_SET( victim->act, PLR_KILLER ) )
	{
	    REMOVE_BIT( victim->act, PLR_KILLER );
	    send_to_char(AT_BLUE, "Killer flag removed.\n\r",        ch     );
	    send_to_char(AT_BLUE, "You are no longer a KILLER.\n\r", victim );
	}
	return;
    }

    if ( !str_cmp( arg2, "thief" ) )
    {
	if ( IS_SET( victim->act, PLR_THIEF  ) )
	{
	    REMOVE_BIT( victim->act, PLR_THIEF  );
	    send_to_char(AT_BLUE, "Thief flag removed.\n\r",        ch     );
	    send_to_char(AT_BLUE, "You are no longer a THIEF.\n\r", victim );
	}
	return;
    }

    send_to_char(AT_BLUE, "Syntax: pardon <character> <killer|thief>.\n\r", ch );
    return;
}



void do_echo( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    
    rch = get_char( ch );

    if ( !authorized( rch, "echo" ) )
        return;

    if ( argument[0] == '\0' )
    {
	send_to_char(AT_YELLOW, "Echo what?\n\r", ch );
	return;
    }

    strcat( argument, "\n\r" );
    send_to_all_char( argument );

    return;
}

void do_wizpwd( CHAR_DATA *ch, char *argument )
{
    char *pArg;
    char *pwdnew;
    char *p;
    CHAR_DATA *victim;
    char  arg1 [ MAX_INPUT_LENGTH ];
    char  arg2 [ MAX_INPUT_LENGTH ];
    char  cEnd;
    if ( IS_NPC( ch ) )
        return;

    if ( !authorized( ch, "wizpwd" ) )
        return;

    one_argument(argument, arg1);
    one_argument(argument, arg2);

    if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char(AT_WHITE, "Syntax: wizpwd player newpassword.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char(AT_WHITE,"That person isn't logged on.\n\r", ch);
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char(AT_WHITE,"Not on NPC's.\n\r", ch);
        return;
    }

    /*
     * Level check added by Canth (canth@xs4all.nl)
     */
    if ( get_trust(victim) >= get_trust(ch) )
    {
        send_to_char(AT_WHITE,"You may not wizpwd your peer nor your superior.\n\r",ch);
        return;
    }

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace( *argument ) )
        argument++;
    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;
    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';
    pArg = arg2;
    while ( isspace( *argument ) )
        argument++;
    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;
    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';
    *argument = '\0';
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char(AT_WHITE, "Syntax: wizpwd player newpassword.\n\r", ch );
        return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, victim->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
        if ( *p == '~' )
        {
            send_to_char(AT_WHITE,
                "New password not acceptable, try again.\n\r", ch );
            return;
        }
    }
    free_string( victim->pcdata->pwd );
    victim->pcdata->pwd = str_dup( pwdnew );
    /*
     * save_char_obj changed from ch to victim by Canth (canth@xs4all.nl)
     */
    save_char_obj( victim, FALSE );
    send_to_char(AT_WHITE, "Ok.\n\r", ch );
    return;
}


/* RT set replaces sset, mset, oset, rset and lset */
void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        send_to_char(AT_WHITE,"Syntax:\n\r",ch);
        send_to_char(AT_WHITE,"  set mob   <name> <field> <value>\n\r",ch);
        send_to_char(AT_WHITE,"  set obj   <name> <field> <value>\n\r",ch);
        send_to_char(AT_WHITE,"  set room  <room> <field> <value>\n\r",ch);
        send_to_char(AT_WHITE,"  set skill <name> <spell or skill> <value>\n\r",ch);
        send_to_char(AT_WHITE,"  set lang  <name> <language> <value>\n\r",ch);
        return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
        do_mset(ch,argument);
        return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
        do_sset(ch,argument);
        return;
    }

    if (!str_prefix(arg,"language"))
    {
        do_lset(ch,argument);
        return;
    }

    if (!str_prefix(arg,"object"))
    {
        do_oset(ch,argument);
        return;
    }

    if (!str_prefix(arg,"room"))
    {
        do_rset(ch,argument);
        return;
    }
    /* echo syntax */
    do_set(ch,"");
}

/* RT to replace the 3 stat commands */
/* Maniac added lstat */
void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
        send_to_char(AT_WHITE,"Syntax:\n\r",ch);
        send_to_char(AT_WHITE,"  stat <name>\n\r",ch);
        send_to_char(AT_WHITE,"  stat obj <name>\n\r",ch);
        send_to_char(AT_WHITE,"  stat mob <name>\n\r",ch);
        send_to_char(AT_WHITE,"  stat room <number>\n\r",ch);
        send_to_char(AT_WHITE,"  stat lang <name>\n\r",ch);
        return;
   }

   if (!str_cmp(arg,"room"))
   {
        do_rstat(ch,string);
        return;
   }

   if (!str_cmp(arg,"obj"))
   {
        do_ostat(ch,string);
        return;
   }

   if (!str_cmp(arg, "lang") || !str_cmp(arg, "language"))
   {
        do_lstat(ch, string);
        return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
        do_mstat(ch,string);
        return;
   }

   /* do it the old way */
   obj = get_obj_world(ch,argument);
   if (obj != NULL)
   {
     do_ostat(ch,argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != NULL)
  {
    do_mstat(ch,argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_rstat(ch,argument);
    return;
  }

  send_to_char(AT_WHITE,"Nothing by that name found anywhere.\n\r",ch);
}


void do_pecho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if ( !authorized( ch, "pecho" ) )
        return;

    argument = one_argument(argument, arg);

    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
        send_to_char(AT_WHITE,"Pecho what?\n\r", ch);
        return;
    }

    if  ( (victim = get_char_world(ch, arg) ) == NULL )
    {
        send_to_char(AT_WHITE,"Target not found.\n\r",ch);
        return;
    }

    if ( get_trust(victim) >= get_trust(ch) )
        send_to_char(AT_RED, "pecho> ",victim);

    send_to_char(AT_YELLOW,argument,victim);
    send_to_char(AT_GREEN,"\n\r",victim);
    send_to_char(AT_WHITE, "pecho> ",ch);
    send_to_char(AT_WHITE,argument,ch);
    send_to_char(AT_WHITE,"\n\r",ch);
}



void do_recho( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *rch;
    DESCRIPTOR_DATA *d;
    
    rch = get_char( ch );

    if ( !authorized( rch, "recho" ) )
        return;

    if ( argument[0] == '\0' )
    {
	send_to_char(AT_YELLOW, "Recho what?\n\r", ch );
	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	    && d->character->in_room == ch->in_room )
	{
	    send_to_char(AT_YELLOW, argument, d->character );
	    send_to_char(AT_YELLOW, "\n\r",   d->character );
	}
    }

    return;
}



ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    OBJ_DATA  *obj;
    CHAR_DATA *victim;

    if ( is_number( arg ) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) )
	return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *rch;
    CHAR_DATA       *victim;
    DESCRIPTOR_DATA *d;
    ROOM_INDEX_DATA *location;
    char             arg1 [ MAX_INPUT_LENGTH ];
    char             arg2 [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "transfer" ) )
        return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char(AT_RED, "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
		&& d->character != ch
		&& d->character->in_room
		&& can_see( ch, d->character ) )
	    {
		char buf [ MAX_STRING_LENGTH ];

		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_transfer( ch, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( !( location = find_location( ch, arg2 ) ) )
	{
	    send_to_char(AT_RED, "No such location.\n\r", ch );
	    return;
	}

	if ( room_is_private( location ) )
	{
	    send_to_char(AT_RED, "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( !( victim = get_char_world( ch, arg1 ) ) )
    {
	send_to_char(AT_RED, "They aren't here.\n\r", ch );
	return;
    }

    if ( !victim->in_room )
    {
	send_to_char(AT_RED, "They are in limbo.\n\r", ch );
	return;
    }

    if ( location == victim->in_room )
    {
      send_to_char( C_DEFAULT, "They are already there.\n\r",ch);
      return;
    }

    if ( victim->fighting )
	stop_fighting( victim, TRUE );
    act(AT_RED, "$n disappears in a mushroom cloud.", victim, NULL, NULL,   TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act(AT_RED, "$n arrives from a puff of smoke.",   victim, NULL, NULL,   TO_ROOM );
    if ( ch != victim )
	act(AT_RED, "$n has transferred you.",        ch,     NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char(AT_RED, "Ok.\n\r", ch );
}



void do_at( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *rch;
    CHAR_DATA       *wch;
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    char             arg [ MAX_INPUT_LENGTH ];
    
    rch = get_char( ch );

    if ( !authorized( rch, "at" ) )
        return;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char(AT_RED, "At where what?\n\r", ch );
	return;
    }

    if ( !( location = find_location( ch, arg ) ) )
    {
	send_to_char(AT_RED, "No such location.\n\r", ch );
	return;
    }

    if ( room_is_private( location ) )
    {
	send_to_char(AT_RED, "That room is private right now.\n\r", ch );
	return;
    }

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    break;
	}
    }

    return;
}

void do_goto( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *rch;
    CHAR_DATA *pet;
    ROOM_INDEX_DATA *location;
    char             arg [ MAX_INPUT_LENGTH ];
    char	     sound [ MAX_STRING_LENGTH ];
    char	     bamf [ MAX_STRING_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "goto" ) )
        return;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char(AT_RED, "Goto where?\n\r", ch );
	return;
    }

    if ( !( location = find_location( ch, arg ) ) )
    {
	send_to_char(AT_RED, "No such location.\n\r", ch );
	return;
    }

    if ( room_is_private( location ) )
    {
	send_to_char(AT_RED, "That room is private right now.\n\r", ch );
	return;
    }
    
    for ( pet = ch->in_room->people; pet; pet = pet->next_in_room )
    {
      if ( IS_NPC( pet ) )
        if ( IS_SET( pet->act, ACT_PET ) && ( pet->master == ch ) )
          break;
    }

    if ( ch->fighting )
	stop_fighting( ch, TRUE );
    if ( pet && pet->fighting )
        stop_fighting( pet, TRUE );
	
    if ( !IS_SET( ch->act, PLR_WIZINVIS ) && ( ch->pcdata->bamfsout[0] != '\0' ) )
    {
	sprintf( sound, "!!SOUND(%s V=100 L=1 P=50 T=Speech)", ch->pcdata->bamfsout );
    }
    else
    {
	sprintf( sound, "!!SOUND(spellof2.wav V=100 L=1 P=50 T=Speech)");
    }
    if( !IS_SET( ch->act, PLR_WIZINVIS ) )
    {
	sprintf( bamf, 
	    ( ch->pcdata && ch->pcdata->bamfout[0] != '\0' )
	    ? ch->pcdata->bamfout : "%s leaves in a swirling mist", ch->name );
	broadcast_room( ch, bamf, sound );
    }
    if ( location == ch->in_room )
    {
      send_to_char(C_DEFAULT, "But you are already there!",ch);
      return;
    }

    char_from_room( ch );
    char_to_room( ch, location );
    if ( pet )
    {
      char_from_room( pet );
    }

    if ( !IS_SET( ch->act, PLR_WIZINVIS ) && ( ch->pcdata->bamfsin[0] != '\0') )
    {
	sprintf( sound, "!!SOUND(%s V=100 L=1 P=50 T=Speech)", ch->pcdata->bamfsin );
    }
    else
    {
	sprintf( sound, "!!SOUND(spellon2.wav V=100 L=1 P=50 T=Speech)");
    }
    if( !IS_SET( ch->act, PLR_WIZINVIS ) )
    {
	sprintf( bamf,
            ( ch->pcdata && ch->pcdata->bamfin[0] != '\0' )
            ? ch->pcdata->bamfin : "%s appears in a swirling mist", ch->name );
	broadcast_room( ch, bamf, sound );
    }

    do_look( ch, "auto" );
        
    return;
}



void do_rstat( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA        *obj;
    CHAR_DATA       *rch;
    ROOM_INDEX_DATA *location;
    char             buf  [ MAX_STRING_LENGTH ];
    char             buf1 [ MAX_STRING_LENGTH ];
    char             arg  [ MAX_INPUT_LENGTH  ];
    int              door;
	int				tmp = 0;
	int				tmp2 = 0;
    rch = get_char( ch );

    if ( !authorized( rch, "rstat" ) )
        return;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( !location )
    {
	send_to_char(AT_RED, "No such location.\n\r", ch );
	return;
    }

    if ( ch->in_room != location && room_is_private( location ) )
    {
	send_to_char(AT_RED, "That room is private right now.\n\r", ch );
	return;
    }

    buf1[0] = '\0';

    sprintf( buf, "Name: '%s.'\n\rArea: '%s'.\n\r",
	    location->name,
	    location->area->name );
    strcat( buf1, buf );
	tmp2 = location->area->status;
	while (tmp2)
	{
	tmp2 = tmp2 >> 1;
	tmp++;
	}
	sprintf( buf, "Area Status: %d\n\r", tmp);
	strcat(buf1, buf);
    sprintf( buf,
	    "Vnum: %d.  Sector: %d.  Light: %d.\n\r",
	    location->vnum,
	    location->sector_type,
	    location->light );
    strcat( buf1, buf );

    sprintf( buf,
            "Timed flags: %s.\n\r", flag_string( timed_room_flags, location->timed_room_flags ) );
     strcat( buf1, buf );

    sprintf( buf,
            "Room flags: %s.\n\r", flag_string( room_flags, location->room_flags ) );
     strcat( buf1, buf );
     
    sprintf( buf,
	    "Room flags value: %d.\n\rDescription:\n\r%s",
	    location->room_flags,
	    location->description );
    strcat( buf1, buf );

    if ( location->extra_descr )
    {
	EXTRA_DESCR_DATA *ed;

	strcat( buf1, "Extra description keywords: '" );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    strcat( buf1, ed->keyword );
	    if ( ed->next )
		strcat( buf1, " " );
	}
	strcat( buf1, "'.\n\r" );
    }

    strcat( buf1, "Characters:" );

    /* Yes, we are reusing the variable rch.  - Kahn */
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
        /* Hide wizinvis ppl - ahsile*/
	if (!( !IS_NPC( rch ) && IS_SET( rch->act, PLR_WIZINVIS )
        && (rch->wizinvis > get_trust( ch ) ) ) )
       {
		strcat( buf1, " " );
		one_argument( rch->name, buf );
		strcat( buf1, buf );
        }
    }

    strcat( buf1, ".\n\rObjects:   " );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	strcat( buf1, " " );
	one_argument( obj->name, buf );
	strcat( buf1, buf );
    }
    strcat( buf1, ".\n\r" );

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) )
	{
	    sprintf( buf,
		    "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\r",
		    door,
		    pexit->to_room ? pexit->to_room->vnum : 0,
		    pexit->key,
		    pexit->exit_info );
	    strcat( buf1, buf );
	    sprintf( buf,
		    "Keyword: '%s'.  Description: %s",
		    pexit->keyword,
		    pexit->description[0] != '\0' ? pexit->description
		                                  : "(none).\n\r" );
	    strcat( buf1, buf );
	}
    }

    send_to_char(AT_RED, buf1, ch );
    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA    *obj;
    CHAR_DATA   *rch;
    AFFECT_DATA *paf;
    char         buf  [ MAX_STRING_LENGTH ];
    char         buf1 [ MAX_STRING_LENGTH ];
    char         arg  [ MAX_INPUT_LENGTH  ];

    rch = get_char( ch );

    if ( !authorized( rch, "ostat" ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Ostat what?\n\r", ch );
	return;
    }

    buf1[0] = '\0';

    if ( !( obj = get_obj_world( ch, arg ) ) )
    {
	send_to_char(AT_WHITE, "Nothing like that in hell, earth, or heaven.\n\r", ch);
	return;
    }

    sprintf( buf, "Name: %s.\n\r",
	    obj->name );
    strcat( buf1, buf );

    sprintf( buf, "Vnum: %d.  Type: %s.\n\r", \
	    obj->pIndexData->vnum, item_type_name( obj ) );
    strcat( buf1, buf );

    sprintf( buf, "Short description: %s.\n\rLong description: %s\n\r",
	    obj->short_descr, obj->description );
    strcat( buf1, buf );

    sprintf( buf, "Wear bits name: %s.\n\r", flag_string( wear_flags,
    	obj->wear_flags ) );
    strcat( buf1, buf );
    sprintf( buf, "Wear bits: %d.  Extra bits: %s.\n\r",
	    obj->wear_flags, extra_bit_name( obj->extra_flags ) );
    strcat( buf1, buf );
    sprintf( buf, "Extra2 bits: %s.\n\r",
	    extra_bit_name2( obj->extra_flags2 ) );
    strcat( buf1, buf );
    sprintf( buf, "Extra3 bits: %s.\n\r",
	    extra_bit_name3( obj->extra_flags3 ) );
    strcat( buf1, buf );
    sprintf( buf, "Extra4 bits: %s.\n\r",
	    extra_bit_name4( obj->extra_flags4 ) );
    strcat( buf1, buf );

    sprintf( buf, "Number: %d/%d.  Weight: %d/%d.\n\r",
	    1,           get_obj_number( obj ),
	    obj->weight, get_obj_weight( obj ) );
    strcat( buf1, buf );

    sprintf( buf, "Cost: %d.  Timer: %d.  Level: %d.\n\r",
	    obj->cost, obj->timer, obj->level );
    strcat( buf1, buf );

    sprintf( buf,
	    "In room: %d.  In object: %s.  Carried by: %s.  Stored by: %s.\n\rWear_loc: %d.\n\r",
	    !obj->in_room    ?        0 : obj->in_room->vnum,
	    !obj->in_obj     ? "(none)" : obj->in_obj->short_descr,
	    !obj->carried_by ? "(none)" : obj->carried_by->name,
	    !obj->stored_by  ? "(none)" : obj->stored_by->name,
	    obj->wear_loc );
    strcat( buf1, buf );
    
    sprintf( buf, "Values: %d %d %d %d.\n\r",
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
    strcat( buf1, buf );

    if ( obj->extra_descr || obj->pIndexData->extra_descr )
    {
	EXTRA_DESCR_DATA *ed;

	strcat( buf1, "Extra description keywords: '" );

	for ( ed = obj->extra_descr; ed; ed = ed->next )
	{
	    strcat( buf1, ed->keyword );
	    if ( ed->next )
		strcat( buf1, " " );
	}

	for ( ed = obj->pIndexData->extra_descr; ed; ed = ed->next )
	{
	    strcat( buf1, ed->keyword );
	    if ( ed->next )
		strcat( buf1, " " );
	}

	strcat( buf1, "'.\n\r" );
    }

    for ( paf = obj->affected; paf; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	strcat( buf1, buf );
    }

    for ( paf = obj->pIndexData->affected; paf; paf = paf->next )
    {
		char buf2[MAX_STRING_LENGTH];

		if (paf->location > PERM_SPELL_BEGIN)
			strcpy(buf2, "Affects '%s' by %d.\n\r");
		else
			strcpy(buf2, "Affects %s by %d.\n\r");

		sprintf( buf, buf2, affect_loc_name( paf->location ), paf->modifier );
		strcat( buf1, buf );
    }

    send_to_char(AT_RED, buf1, ch );
    return;
}


void do_mstat( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA   *rch;
    CHAR_DATA   *victim;
    AFFECT_DATA *paf;
    char         buf  [ MAX_STRING_LENGTH * 5 ];
    char         buf1 [ MAX_STRING_LENGTH * 5 ];
    char         arg  [ MAX_INPUT_LENGTH  ];
    
    rch = get_char( ch );

    if ( (!IS_NPC(rch)) && (!authorized( rch, "mstat" ) ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Mstat whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
    }
    
    buf1[0] = '\0';

    sprintf( buf, "Name: %s.\n\r",
	    victim->name );
    strcat( buf1, buf );
    
    if ( !IS_NPC( victim )  && victim->desc )
    if ( str_cmp( victim->desc->user, "(unknown)" ) )
    sprintf( buf, "Email: %s@%s\n\r",
	    victim->desc->user ? victim->desc->user : "(none)",
	    victim->desc->host );
    strcat( buf1, buf );

    sprintf( buf, "Guild: %s.\n\r",
            victim->guild ? victim->guild->name : "NONE");
    strcat( buf1, buf );            
    
    if (!IS_NPC( victim ) )
    {
    sprintf( buf, "Religion: %d.  Recall: (%d)(Clan %d)(Rel %d).\n\r",
	victim->religion, victim->pcdata->recall,
	get_clan_index(victim->clan)->recall, get_religion_index(victim->religion)->recall );
    strcat( buf1, buf );
    }

    if (!IS_NPC( victim ) )
    {
    sprintf(buf, "Clan: %d.\n\r",
            victim->clan );
    strcat( buf1, buf );
    }

    if (!IS_NPC( victim ) )
    {
    sprintf( buf, "Class: %d.\n\r",
	    victim->class );
    strcat( buf1, buf );
    }

    if (!IS_NPC( victim ) )
    {
    sprintf( buf, "Multiclass: %d.\n\r",
	    victim->multied );
    strcat( buf1, buf );
    }

    sprintf( buf, "Vnum: %d.  Sex: %s.  Room: %d.\n\r",
	    IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
	    victim->sex == SEX_MALE    ? "male"   :
	    victim->sex == SEX_FEMALE  ? "female" : "neutral",
	    !victim->in_room           ?        0 : victim->in_room->vnum );
    strcat( buf1, buf );

    if ( !IS_NPC( victim ) )
    {
    sprintf( buf, "PStr: %d.  PInt: %d.  PWis: %d.  PDex %d.  PCon: %d.\n\r",
         victim->pcdata->perm_str, victim->pcdata->perm_int,
         victim->pcdata->perm_wis, victim->pcdata->perm_dex,
         victim->pcdata->perm_con );
    strcat( buf1, buf );
    }
    
    sprintf( buf, "Str: %d.  Int: %d.  Wis: %d.  Dex: %d.  Con: %d.\n\r",
	    get_curr_str( victim ),
	    get_curr_int( victim ),
	    get_curr_wis( victim ),
	    get_curr_dex( victim ),
	    get_curr_con( victim ) );
    strcat( buf1, buf );
    
    if (!IS_NPC( victim ) )
    {
    sprintf( buf, "Stat mods:  Str(%2d)  Int(%2d)  Wis(%2d)  Dex(%2d)  Con(%2d)\n\r",
    victim->pcdata->mod_str, victim->pcdata->mod_int, victim->pcdata->mod_wis,
    victim->pcdata->mod_dex, victim->pcdata->mod_con );
    strcat( buf1, buf );
    }

    if (( victim->class != 9 )&&( victim->class != 11))
      sprintf( buf, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d.  Practices: %d.  QP: %d.\n\r",
	    victim->hit,         victim->max_hit,
	    victim->mana,        victim->max_mana,
	    victim->move,        victim->max_move,
	    victim->practice,	 victim->questpoints );
    else
      sprintf( buf, "Hp: %d/%d.  Blood: %d/%d.  Move: %d/%d.  Practices: %d.  QP: %d.\n\r",
	    victim->hit,         victim->max_hit,
	    victim->bp,          victim->max_bp,
	    victim->move,        victim->max_move,
	    victim->practice,	 victim->questpoints );
    strcat( buf1, buf );
	
    sprintf( buf,
	"Lv: %d.  Class: %d.  Align: %d.  AC: %d.  Gold: %d.  Exp: %d.\n\r",
	    victim->level,       victim->class,        victim->alignment,
	    GET_AC( victim ),    victim->gold,         victim->exp );
    strcat( buf1, buf );

    sprintf( buf,
	    "Hitroll: %d.  Damroll: %d.  Position: %d.  Wimpy: %d.  Poison Level: %d.\n\r",
	    GET_HITROLL( victim ), GET_DAMROLL( victim ) * 2,
	    victim->position,      victim->wimpy,
	    victim->poison_level );
    strcat( buf1, buf );

    if ( !IS_NPC( victim ) && get_trust( victim ) > 100 )
    {
         sprintf( buf, "Bamfin: %s.\n\r", victim->pcdata->bamfin );
         strcat( buf1, buf );
         sprintf( buf, "Bamfout: %s.\n\r", victim->pcdata->bamfout );
         strcat( buf1, buf );
    }

    if ( !IS_NPC( victim ) )
    {
	sprintf( buf, "Bank Account: %d.    ", victim->pcdata->bankaccount );
	strcat( buf1, buf );
	sprintf( buf, "Bank Shares: %d.\n\r", victim->pcdata->shares );
	strcat( buf1, buf );
    }
    
    if ( !IS_NPC( victim ) )
    {
	sprintf( buf, "Page Lines: %d.\n\r", victim->pcdata->pagelen );
	strcat( buf1, buf );
    }

    sprintf( buf, "Fighting: %s.\n\r",
	    victim->fighting ? victim->fighting->name : "(none)" );
    strcat( buf1, buf );

    if ( !IS_NPC( victim ) )
    {
	sprintf( buf,
		"Thirst: %d.  Full: %d.  Drunk: %d.  Saving throw: %d. Anti-Disarm: %d.\n\r",
		victim->pcdata->condition[COND_THIRST],
		victim->pcdata->condition[COND_FULL  ],
		victim->pcdata->condition[COND_DRUNK ],
		victim->saving_throw,
                victim->antidisarm );
	strcat( buf1, buf );
    }

    sprintf( buf, "Carry number: %d.  Carry weight: %d.\n\r",
	    victim->carry_number, victim->carry_weight );
    strcat( buf1, buf );

    sprintf( buf, "Age: %d.  Played: %d.  Timer: %d.\n\r",
	    get_age( victim ),
	    (int) victim->played,
	    victim->timer );
    strcat( buf1, buf );
    
    sprintf( buf, "Act: %s.\n\r", act_bit_name( victim->act ) );
    strcat( buf1, buf );

    if (!IS_NPC(victim))
    {
    	sprintf( buf, "Act2: %s.\n\r", act_bit_name2( victim->act ) );
    	strcat( buf1, buf );
    }

    sprintf( buf, "Immune: %s.\n\r", imm_bit_name( victim->imm_flags ) );
    strcat( buf1, buf );
    sprintf( buf, "Resist: %s.\n\r", imm_bit_name( victim->res_flags ) );
    strcat( buf1, buf);
    sprintf( buf, "Vuln: %s.\n\r", imm_bit_name( victim->res_flags ) );
    strcat( buf1, buf);
    sprintf( buf, 
"Dam Mods:       Acid [%3d]     Holy [%3d]     Magic[%3d]      Fire [%3d]  Energy [%3d]\n\r",
	victim->damage_mods[0], victim->damage_mods[1], victim->damage_mods[2],
	victim->damage_mods[3], victim->damage_mods[4]);
    strcat( buf1, buf);
    sprintf( buf,
"  Wind [%3d]   Water [%3d] Illusion [%3d]   Dispel [%3d]     Earth [%3d] Psychic [%3d]\n\r",
	victim->damage_mods[5], victim->damage_mods[6], victim->damage_mods[7],
	victim->damage_mods[8], victim->damage_mods[9], victim->damage_mods[10]);
    strcat( buf1, buf);
    sprintf( buf,
"Poison [%3d]  Breath [%3d] Summon   [%3d] Physical [%3d] Explosive [%3d]\n\r",
	victim->damage_mods[11], victim->damage_mods[12], victim->damage_mods[13],
	victim->damage_mods[14], victim->damage_mods[15]);
    strcat( buf1, buf);
    sprintf( buf,
"  Song [%3d] Nagarom [%3d] Unholy   [%3d]     Clan [%3d]\n\r",
	victim->damage_mods[16], victim->damage_mods[17], victim->damage_mods[18],
	victim->damage_mods[19]);
    strcat( buf1, buf);
    sprintf( buf, "Master: %s.  Leader: %s.\n\r",
	    victim->master      ? victim->master->name   : "(none)",
	    victim->leader      ? victim->leader->name   : "(none)" );
    strcat( buf1, buf );
    sprintf( buf, "Affected by: %s.\n\r", affect_bit_name( victim->affected_by ) );
    strcat(buf1, buf );
    sprintf( buf, "Affected by(2): %s.\n\r", affect_bit_name2( victim->affected_by2 ) );
    strcat( buf1, buf );
    sprintf( buf, "Affected by(3): %s.\n\r", affect_bit_name3( victim->affected_by3 ) );
    strcat( buf1, buf );
    sprintf( buf, "Affected by(4): %s.\n\r", affect_bit_name4( victim->affected_by4 ) );
    strcat( buf1, buf );
    sprintf( buf, "Granted Powers: %s.\n\r", affect_bit_name_powers( victim->affected_by_powers ) );
    strcat( buf1, buf );
    sprintf( buf, "Weaknesses:     %s.\n\r", affect_bit_name_weaknesses( victim->affected_by_weaknesses ) );
    strcat( buf1, buf );
    sprintf( buf, "Shields:        %d.\n\r", victim->shields );
    strcat( buf1, buf );

    if ( !IS_NPC( victim ) )
    if (!victim->pcdata->switched)	/* OLC */
    {
	sprintf( buf, "Security: %d.\n\r", victim->pcdata->security );
	strcat( buf1, buf );
    }

    sprintf( buf, "Short description: %s.\n\rLong  description: %s",
	    victim->short_descr,
	    victim->long_descr[0] != '\0' ? victim->long_descr
	                                  : "(none).\n\r" );
    strcat( buf1, buf );

    sprintf(buf, "Summon timer: %d  Combat timer: %d  Clan timer: %d  Religion timer: %d\n\r",
	victim->summon_timer,
	victim->combat_timer,
	victim->ctimer,
	victim->rtimer);
    strcat( buf1, buf );

    sprintf(buf, "Stunned: Total: %d  Command: %d  Magic: %d  "
	         "Non-Magic: %d  To-Stun: %d\n\r",
	    ch->stunned[STUN_TOTAL], ch->stunned[STUN_COMMAND],
	    ch->stunned[STUN_MAGIC], ch->stunned[STUN_NON_MAGIC],
	    ch->stunned[STUN_TO_STUN]);
    strcat( buf1, buf );

    if ( IS_NPC( victim ) && victim->spec_fun != 0 )
    {
        sprintf( buf, "Mobile has spec function: %s.\n\r", spec_string(victim->spec_fun) );
        strcat( buf1, buf );
    }
    
    if ( IS_NPC( victim ) && victim->game_fun != 0 )
    {
        sprintf( buf, "Mobile has game function: %s.\n\r", game_string(victim->game_fun) );
        strcat( buf1, buf );
    }

    for ( paf = victim->affected; paf; paf = paf->next )
    {
        if ( paf->deleted )
	    continue;
	sprintf( buf,
	  "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d (%d).\n\r",
	  skill_table[(int) paf->type].name,
	  affect_loc_name( paf->location ),
	  paf->modifier,
	  paf->duration,
	  affect_bit_name( paf->bitvector ),
	  paf->level,
	  paf->count );
	strcat( buf1, buf );
    }
    for ( paf = victim->affected2; paf; paf = paf->next )
    {
        if ( paf->deleted )
	  continue;
	sprintf( buf,
	  "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d (%d).\n\r",
	  skill_table[(int) paf->type].name,
	  affect_loc_name( paf->location ),
	  paf->modifier,
	  paf->duration,
	  affect_bit_name2( paf->bitvector ),
	  paf->level,
	  paf->count );
	strcat( buf1, buf );
    }
    for ( paf = victim->affected3; paf; paf = paf->next )
    {
        if ( paf->deleted )
	  continue;
	sprintf( buf,
	  "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
	  skill_table[(int) paf->type].name,
	  affect_loc_name( paf->location ),
	  paf->modifier,
	  paf->duration,
	  affect_bit_name3( paf->bitvector ),
	  paf->level );
	strcat( buf1, buf );
    }
    for ( paf = victim->affected4; paf; paf = paf->next )
    {
        if ( paf->deleted )
	  continue;
	sprintf( buf,
	  "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
	  skill_table[(int) paf->type].name,
	  affect_loc_name( paf->location ),
	  paf->modifier,
	  paf->duration,
	  affect_bit_name4( paf->bitvector ),
	  paf->level );
	strcat( buf1, buf );
    }
    for ( paf = victim->affected_powers; paf; paf = paf->next )
    {
        if ( paf->deleted )
          continue;  
        sprintf( buf,
          "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
          skill_table[(int) paf->type].name,
          affect_loc_name( paf->location ),
          paf->modifier,
          paf->duration,
          affect_bit_name_powers( paf->bitvector ),
          paf->level );
        strcat( buf1, buf );
    }
    for ( paf = victim->affected_weaknesses; paf; paf = paf->next )
    {
        if ( paf->deleted )
          continue;  
        sprintf( buf,
          "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
          skill_table[(int) paf->type].name,
          affect_loc_name( paf->location ),
          paf->modifier,
          paf->duration,
          affect_bit_name_weaknesses( paf->bitvector ),
          paf->level );
        strcat( buf1, buf );
    }
    send_to_char(AT_WHITE, buf1, ch );
    return;
}


void do_notestat( CHAR_DATA *ch, char *argument )
/*   notestat by Garion */
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    NOTE_DATA *note;
    int vnum;

    if ( !authorized( ch, "notestat" ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char(AT_WHITE,"Notestat whom?\n\r", ch);
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char(AT_WHITE,"There's no such person Jack.\n\r", ch);
        return;
    }

    buf1[0] = '\0';
    buf2[0] = '\0';
    vnum = 0;

    for ( note = note_list; note != NULL; note = note->next )
    {
        if ( is_note_to( victim, note ) )
        {
            sprintf( buf1, "[%3d%s] %s: %s\n\r", vnum,
                ( note->date_stamp > victim->last_note
                && str_cmp( note->sender, victim->name ) ) ? "N" : " ",
                note->sender, note->subject );
            strcat(buf2, buf1 );
            vnum++;
        }
    }

   send_to_char(AT_WHITE, buf2, ch );
   return;
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
           CHAR_DATA      *rch;
           MOB_INDEX_DATA *pMobIndex;
           char            buf  [ MAX_STRING_LENGTH   ];
           char            buf1 [ MAX_STRING_LENGTH*2 ];
           char            arg  [ MAX_INPUT_LENGTH    ];
    extern int             top_mob_index;
           int             vnum;
	   int             nMatch;
	   bool            fAll;
	   bool            found;

    rch = get_char( ch );

    if ( !authorized( rch, "mfind" ) )
        return;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Mfind whom?\n\r", ch );
	return;
    }

    buf1[0] = '\0';
    fAll    = !str_cmp( arg, "all" );
    found   = FALSE;
    nMatch  = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) )
	{
	    nMatch++;
	    if ( fAll || is_name( arg, pMobIndex->player_name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
		if ( !fAll )
		    strcat( buf1, buf );
		else
		    send_to_char(AT_RED, buf, ch );
	    }
	}
    }

    if ( !found )
    {
	send_to_char(AT_WHITE, "Nothing like that in hell, earth, or heaven.\n\r", ch);
	return;
    }

    if ( !fAll )
        send_to_char(AT_RED, buf1, ch );
    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
           CHAR_DATA      *rch;
	   OBJ_INDEX_DATA *pObjIndex;
	   char            buf  [ MAX_STRING_LENGTH   ];
	   char            buf1 [ MAX_STRING_LENGTH*2 ];
	   char            arg  [ MAX_INPUT_LENGTH    ];
    extern int             top_obj_index;
	   int             vnum;
	   int             nMatch;
	   bool            fAll;
	   bool            found;

    rch = get_char( ch );

    if ( !authorized( rch, "ofind" ) )
        return;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Ofind what?\n\r", ch );
	return;
    }

    buf1[0] = '\0';
    fAll    = !str_cmp( arg, "all" );
    found   = FALSE;
    nMatch  = 0;

    for ( vnum = 0; nMatch < top_obj_index && vnum < 65536; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) )
	{
	    nMatch++;
	    if ( fAll || is_name( arg, pObjIndex->name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
		if ( !fAll )
		    strcat( buf1, buf );
		else
		    send_to_char(AT_RED, buf, ch );
	    }
	}
    }

    if ( !found )
    {
	send_to_char(AT_WHITE, "Nothing like that in hell, earth, or heaven.\n\r", ch);
	return;
    }

    if ( !fAll )
        send_to_char(AT_RED, buf1, ch );
    return;
}


void do_mwhere( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       buf  [ MAX_STRING_LENGTH   ];
    char       buf1 [ MAX_STRING_LENGTH*5 ];
    char       arg  [ MAX_INPUT_LENGTH    ];
    bool       found;

    rch = get_char( ch );

    if ( !authorized( rch, "mwhere" ) )
        return;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Mwhere whom?\n\r", ch );
	return;
    }

    buf1[0] = '\0';
    found   = FALSE;
    for ( victim = char_list; victim; victim = victim->next )
    {
	if ( IS_NPC( victim )
	    && victim->in_room
	    && is_name( arg, victim->name ) )
	{
	    found = TRUE;
	    sprintf( buf, "[%5d] %-28s [%5d] %s\n\r",
		    victim->pIndexData->vnum,
		    victim->short_descr,
		    victim->in_room->vnum,
		    victim->in_room->name );
	    strcat( buf1, buf );
	}
    }

    if ( !found )
    {
	act(AT_WHITE, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
	return;
    }

    send_to_char(AT_RED, buf1, ch );
    return;
}



void do_reboo( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    rch = get_char( ch );

    if ( !authorized( rch, "reboot" ) )
        return;

    send_to_char(AT_WHITE, "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
           CHAR_DATA *rch;
           char       buf [ MAX_STRING_LENGTH ];
    extern bool       merc_down;

 if ( ch )
 {
    rch = get_char( ch );

    if ( !authorized( rch, "reboot" ) )
        return;

    sprintf( buf, "Reboot by %s.", ch->name );
    do_echo( ch, buf );
}
    end_of_game( );

    merc_down = TRUE;
    return;
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    rch = get_char( ch );

    if ( !authorized( rch, "shutdown" ) )
        return;

    send_to_char(AT_WHITE, "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
           CHAR_DATA *rch;
           char       buf [ MAX_STRING_LENGTH ];
    extern bool       merc_down;

    rch = get_char( ch );

    if ( !authorized( rch, "shutdown" ) )
        return;

    sprintf( buf, "Shutdown by %s.", ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    do_echo( ch, buf );

    end_of_game( );

    merc_down = TRUE;
    return;
}



void do_snoop( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *rch;
    CHAR_DATA       *victim;
    DESCRIPTOR_DATA *d;
    char             arg [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "snoop" ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Snoop whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
    }

    if ( !victim->desc )
    {
	send_to_char(AT_WHITE, "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char(AT_BLUE, "Cancelling all snoops.\n\r", ch );
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by )
    {
	send_to_char(AT_WHITE, "Busy already.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) > get_trust( ch ) )
    {
	send_to_char(AT_WHITE, "You failed.\n\r", ch );
	return;
    }

    if ( ch->desc )
    {
	for ( d = ch->desc->snoop_by; d; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char(AT_WHITE, "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    send_to_char(AT_BLUE, "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "switch" ) )
        return;

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

    /*
     * Pointed out by Da Pub (What Mud)
     */
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



void do_return( CHAR_DATA *ch, char *argument )
{
    if ( !ch->desc )
	return;

    if ( !ch->desc->original )
    {
	send_to_char(AT_WHITE, "You aren't switched.\n\r", ch );
	return;
    }

    if ( !IS_NPC( ch ) && authorized(ch, "return"))
    {
        send_to_char(AT_WHITE, "You are not authorized to use this command.\n\r", ch );
        return;
    }

    send_to_char(AT_BLUE, "You return to your original body.\n\r", ch );
    ch->desc->original->pcdata->switched = FALSE;
    ch->desc->character                  = ch->desc->original;
    ch->desc->original                   = NULL;
    ch->desc->character->desc            = ch->desc;
    ch->prompt                           = NULL;
    ch->desc                             = NULL;
    return;
}



void do_mload( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA      *rch;
    CHAR_DATA      *victim;
    MOB_INDEX_DATA *pMobIndex;
    char            arg [ MAX_INPUT_LENGTH ];
    
    rch = get_char( ch );

    if ( !authorized( rch, "mload" ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
	send_to_char(AT_WHITE, "Syntax: mload <vnum>.\n\r", ch );
	return;
    }

    if ( !( pMobIndex = get_mob_index( atoi( arg ) ) ) )
    {
	send_to_char(AT_WHITE, "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    victim->spec_fun = pMobIndex->spec_fun;     /* Add special function */
    victim->game_fun = pMobIndex->game_fun;     /* Add game function */
    char_to_room( victim, ch->in_room );
    send_to_char(AT_RED, "Ok.\n\r", ch );
    act(AT_RED, "$n has created $N!", ch, NULL, victim, TO_ROOM );
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA       *obj;
    CHAR_DATA      *rch;
    OBJ_INDEX_DATA *pObjIndex;
    char            arg1 [ MAX_INPUT_LENGTH ];
    char            arg2 [ MAX_INPUT_LENGTH ];
    char	arg3 [ MAX_INPUT_LENGTH ];
    int noi = 1;
    int in = 1;
    int             level;

    rch = get_char( ch );

    if ( !authorized( rch, "oload" ) )
        return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg2 );
    noi = is_number( arg3 ) ? atoi( arg3 ) : 1;
    if( noi > 100 )
    {
	send_to_char(AT_WHITE, "Do not attempt loading more than 100 of an item.\n\r", ch );
	return;
    }
 
    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        send_to_char(AT_WHITE, "Syntax: oload <vnum> [number] [level].\n\r", ch );
        return;
    }
 
    if ( arg2[0] == '\0' )
    {
	level = get_trust( ch );
    }
    else
    {
	/*
	 * New feature from Alander.
	 */
        if ( !is_number( arg2 ) )
        {
	    send_to_char(AT_WHITE, "Syntax: oload <vnum> [number] [level].\n\r", ch );
	    return;
        }
        level = atoi( arg2 );
	if ( level < 0 || level > get_trust( ch ) )
        {
	    send_to_char(AT_WHITE, "Limited to your trust level.\n\r", ch );
	    return;
        }
    }

    if ( !( pObjIndex = get_obj_index( atoi( arg1 ) ) ) )
    {
	send_to_char(AT_WHITE, "No object has that vnum.\n\r", ch );
	return;
    }

    /* Item count check - Ahsile */
    ch->carry_number = ch_invcount( ch );
    ch->carry_weight = ch_weightcount( ch );
    
    if (ch->carry_number + noi > can_carry_n( ch ) )
    {
       send_to_char(AT_WHITE, "You can't carry that many items!\n\r",ch);
       return;
    }

    level = pObjIndex->level;
    for ( in = 1; in <= noi; in++ )
    {
      obj = create_object( pObjIndex, level );
      if ( CAN_WEAR( obj, ITEM_TAKE ) )
      {
  	obj_to_char( obj, ch );
      }
     else
      {
	obj_to_room( obj, ch->in_room );
      }
    }
    obj = create_object( pObjIndex, level );
      
    sprintf( log_buf, "$n has created %d $p%s!", noi, noi > 1 ? "s" : "" );
	act(AT_RED, log_buf, ch, obj, NULL, TO_ROOM );
    send_to_char(AT_RED, "Ok.\n\r", ch );
    extract_obj( obj );
    /* Ahsile - Unnecessary, obj_to_char handles the counting
    ch->carry_number += noi;
    ch->carry_weight += (obj->weight * noi);
    */
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *obj;
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "purge" ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
        OBJ_DATA  *obj_next;
	CHAR_DATA *vnext;

	for ( victim = ch->in_room->people; victim; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( victim->deleted )
	        continue;

	    if ( IS_NPC( victim ) && victim != ch )
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->deleted )
	        continue;
	    extract_obj( obj );
	}

	send_to_char(AT_RED, "Ok.\n\r", ch );
	act(AT_RED, "You purge the room!", ch, NULL, NULL, TO_CHAR);
	act(AT_RED, "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	return;
    }


    if ( !( victim = get_char_room( ch, arg ) ) )
    {
    if ( ( obj = get_obj_list( ch, arg, ch->in_room->contents ) ) )
    {
    	act(AT_RED, "You purge $P.\n\r", ch, NULL, obj, TO_CHAR );
    	act(AT_RED, "$n purges $P.\n\r", ch, NULL, obj, TO_ROOM );
    	extract_obj( obj );
    	return;
    }
      if ( !( victim = get_char_world( ch, arg ) ) )
      {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
      }
    }
    
    if ( !IS_NPC( victim ) )
    {
	send_to_char(AT_WHITE, "Not on PC's.\n\r", ch );
	return;
    }
    
    act(AT_RED, "You purge $N.", ch, NULL, victim, TO_CHAR );
    act(AT_RED, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}



void do_advance( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int        level;
    int        iLevel;

    rch = get_char( ch );

    if ( !authorized( rch, "advance" ) )
        return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char(AT_WHITE, "Syntax: advance <char> <level>.\n\r", ch );
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
    
    level = atoi( arg2 );

    if ( level < 1 || level > MAX_LEVEL )
    {
	char buf [ MAX_STRING_LENGTH ];

	sprintf( buf, "Advance within range 1 to %d.\n\r", MAX_LEVEL );
	send_to_char(AT_WHITE, buf, ch );
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char(AT_WHITE, "Limited to your trust level.\n\r", ch );
	return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
   if ( level < L_APP )
   {
    if ( level <= victim->level )
    {
        if ( victim->level > LEVEL_DEMIGOD )
          do_help( victim, "demm_mortal" );

/*        int sn; */
	
	send_to_char(AT_RED, "Lowering a player's level!\n\r", ch );
	send_to_char(AT_RED, "**** ARGHHHHHHHHHHHHHHH ****\n\r",    victim );
	victim->level    = 1;
	victim->exp      = 1000;
	victim->max_hit  = 10;
	victim->max_mana = 100;
	victim->max_bp   = 20;
	victim->max_move = 100;
/*	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	  victim->pcdata->learned[sn] = 0;*/
	victim->practice = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->bp       = victim->max_bp;
	victim->move     = victim->max_move;
	advance_level( victim );
    }
    else
    {
	send_to_char(AT_RED, "Raising a player's level!\n\r", ch );
	send_to_char(AT_RED, "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	send_to_char(AT_RED, "You raise a level!!  ", victim );
	victim->level += 1;
	advance_level( victim );
    }
   }
    if ( level > LEVEL_DEMIGOD && level > victim->level )
    {
      victim->level = level;
      sprintf( log_buf, "advm_%d", level );
      do_help( victim, log_buf );
    }
    if ( level > LEVEL_DEMIGOD && level < victim->level )
    {
      victim->level = level;
      sprintf( log_buf, "demm_%d", level );
      do_help( victim, log_buf );
    }
    if ( victim->level < LEVEL_HERO1 )
    {
    victim->exp   = 1000 * UMAX( 1, victim->level );
    }
    if ( victim->level == LEVEL_HERO1 )
    {
    victim->exp   = 1500 + 1000 * UMAX( 1, victim->level );
    }
    if ( victim->level == LEVEL_HERO2 )
    {
    victim->exp   = 5500 + 1000 * UMAX( 1, victim->level );
    }
    if ( victim->level == LEVEL_HERO3 )
    {
    victim->exp   = 12000 + 1000 * UMAX( 1, victim->level );
    }
    if ( victim->level == LEVEL_CHAMP )
    {
    victim->exp   = 31000 + 1000 * UMAX( 1, victim->level );
    }
    if ( victim->level == LEVEL_DEMIGOD )
    {
    victim->exp   = 95000 + 1000 * UMAX( 1, victim->level );
    }

    if (victim->level >= 50)	
	    SET_BIT( ch->act, PLR_GHOST );
	

    if ( !IS_CODER(victim) )
      victim->trust = 0;
    return;
}

void do_makelegend(CHAR_DATA* ch, char* argument)
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg1 [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "makelegend" ) )
        return;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char(AT_WHITE, "Syntax: makelegend <char>.\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world( ch, arg1 ) ) )
    {
        send_to_char(AT_WHITE, "That player is not here.\n\r", ch );
        return;
    }

    if (victim->exp == MAX_EXPERIENCE)
    {
       send_to_char(AT_WHITE, "That player is already a legend!\n\r",ch );
       return;
    }

    if (victim->level != LEVEL_DEMIGOD)
    {
       send_to_char(AT_WHITE, "That player is not a DEMIGOD yet. Use advance!", ch);
       return;
       /*  - Another possibility
           do_advance(ch, argument);
           if (victim->level != LEVEL_DEMIGOD) { return; }
       */
    }

    victim->exp = (MAX_EXPERIENCE - 1);
    gain_exp(victim,1);
    return;
}

void do_trust( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int        level;

    rch = get_char( ch );

    if ( !authorized( rch, "trust" ) )
        return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char(AT_WHITE, "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( !( victim = get_char_world( ch, arg1 ) ) )
    {
	send_to_char(AT_WHITE, "That player is not here.\n\r", ch );
	return;
    }

    level = atoi( arg2 );

    if ( level < 0 || level > MAX_LEVEL )
    {
	char buf [ MAX_STRING_LENGTH ];

	sprintf( buf, "Trust within range 0 to %d.\n\r", MAX_LEVEL );
	send_to_char(AT_WHITE, buf, ch );
	return;
    }

    if ( level >= get_trust( ch ) )
    {
	send_to_char(AT_WHITE, "Limited to your trust.\n\r", ch );
	return;
    }

    victim->trust = level;
    return;
}


void do_restore( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "restore" ) )
        return;

    one_argument( argument, arg );

    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */

        for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
        {
	    if ( IS_NPC( rch ) )
		continue;

            affect_strip(rch,skill_lookup("poison"));
            affect_strip(rch,skill_lookup("blindness"));
            affect_strip(rch,skill_lookup("curse"));
			affect_strip(rch,skill_lookup("sleep"));

            rch->hit    = rch->max_hit;
            rch->mana   = rch->max_mana;
            rch->move   = rch->max_move;
            rch->bp     = rch->max_bp;
	    rch->poison_level = 0;
            update_pos( rch);
            act(AT_BLUE,"$n has restored you.",ch,NULL, rch,TO_VICT);
        }
        send_to_char(AT_BLUE,"Room restored.\n\r",ch);
        return;

    }

    /* Restore All feature coded by Katrina */
    if ( !str_cmp( arg, "all" ) )
    {
        for ( victim = char_list; victim; victim = victim->next )
        {
            if ( victim->deleted )
                continue;

	    if ( IS_NPC( victim ) )
		continue;

	    if ( victim->pkill )
		continue;
	
            affect_strip(rch,skill_lookup("poison"));
            affect_strip(rch,skill_lookup("blindness"));
            affect_strip(rch,skill_lookup("curse"));
			affect_strip(rch,skill_lookup("sleep"));

            victim->hit = victim->max_hit;
            victim->mana = victim->max_mana;
            victim->move = victim->max_move;
            victim->bp  = victim->max_bp;
	    victim->poison_level = 0;

            update_pos( victim );
            act(AT_BLUE, "$n has restored you.", ch, NULL, victim, TO_VICT );
        }
        send_to_char(AT_BLUE, "Aww...how sweet :)...Done.\n\r", ch );
    }

    if ( !str_cmp( arg, "pkill" ) )
    {
        for ( victim = char_list; victim; victim = victim->next )
        {
            if ( victim->deleted )
                continue;

	    if ( IS_NPC( victim ) )
		continue;

	    if ( !victim->pkill )
		continue;
	
            affect_strip(rch,skill_lookup("poison"));
            affect_strip(rch,skill_lookup("blindness"));
            affect_strip(rch,skill_lookup("curse"));
			affect_strip(rch,skill_lookup("sleep"));

            victim->hit = victim->max_hit;
            victim->mana = victim->max_mana;
            victim->move = victim->max_move;
            victim->bp  = victim->max_bp;
	    victim->poison_level = 0;

            update_pos( victim );
            act(AT_BLUE, "$n has restored you.", ch, NULL, victim, TO_VICT );
        }
        send_to_char(AT_BLUE, "Aww...how sweet :)...Done.\n\r", ch );
    } else
    {
        if ( !( victim = get_char_world( ch, arg ) ) )
        {
                send_to_char(AT_BLUE, "They aren't here.\n\r", ch );
                return;
        }

            affect_strip(rch,skill_lookup("poison"));
            affect_strip(rch,skill_lookup("blindness"));
            affect_strip(rch,skill_lookup("curse"));
			affect_strip(rch,skill_lookup("sleep"));

        victim->hit  = victim->max_hit;
        victim->mana = victim->max_mana;
        victim->move = victim->max_move;
        victim->bp   = victim->max_bp;
	victim->poison_level = 0;
        update_pos( victim );
        act(AT_BLUE, "$n has restored you.", ch, NULL, victim, TO_VICT );
        send_to_char(AT_BLUE, "Ok.\n\r", ch );
    }

    return;
}

void do_freeze( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "freeze" ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Freeze whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char(AT_WHITE, "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET( victim->act, PLR_FREEZE ) )
    {
	REMOVE_BIT( victim->act, PLR_FREEZE );
	send_to_char(AT_LBLUE, "FREEZE removed.\n\r",     ch     );
	send_to_char(AT_LBLUE, "You can play again.\n\r", victim );
    }
    else
    {
	SET_BIT(    victim->act, PLR_FREEZE );
	send_to_char(AT_LBLUE, "FREEZE set.\n\r",            ch     );
	send_to_char(AT_LBLUE, "You can't do ANYthing!\n\r", victim );
    }

    save_char_obj( victim, FALSE );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "log" ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_RED, "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char(AT_RED, "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char(AT_RED, "Log ALL on.\n\r",  ch );
	}
	return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET( victim->act, PLR_LOG ) )
    {
	REMOVE_BIT( victim->act, PLR_LOG );
	send_to_char(AT_WHITE, "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(    victim->act, PLR_LOG );
	send_to_char(AT_WHITE, "LOG set.\n\r",     ch );
    }

    return;
}



void do_noemote( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "noemote" ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Noemote whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char(AT_WHITE, "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET( victim->act, PLR_NO_EMOTE ) )
    {
	REMOVE_BIT( victim->act, PLR_NO_EMOTE );
	send_to_char(AT_RED, "NO_EMOTE removed.\n\r",    ch     );
	send_to_char(AT_RED, "You can emote again.\n\r", victim );
    }
    else
    {
	SET_BIT(    victim->act, PLR_NO_EMOTE );
	send_to_char(AT_RED, "You can't emote!\n\r",    victim );
	send_to_char(AT_RED, "NO_EMOTE set.\n\r",       ch     );
    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "notell" ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Notell whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char(AT_WHITE, "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET( victim->act, PLR_NO_TELL ) )
    {
	REMOVE_BIT( victim->act, PLR_NO_TELL );
	send_to_char(AT_RED, "NO_TELL removed.\n\r",    ch );
	send_to_char(AT_RED, "You can tell again.\n\r", victim );
    }
    else
    {
	SET_BIT(    victim->act, PLR_NO_TELL );
	send_to_char(AT_RED, "NO_TELL set.\n\r",        ch     );
	send_to_char(AT_RED, "You can't tell!\n\r",     victim );
    }

    return;
}



void do_silence( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    rch = get_char( ch );

    if ( !authorized( rch, "silence" ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Silence whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char(AT_WHITE, "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET( victim->act, PLR_SILENCE ) )
    {
	REMOVE_BIT( victim->act, PLR_SILENCE );
	send_to_char(AT_RED, "You can use channels again.\n\r", victim );
	send_to_char(AT_RED, "SILENCE removed.\n\r",            ch     );
    }
    else
    {
	SET_BIT(    victim->act, PLR_SILENCE );
	send_to_char(AT_RED, "You can't use channels!\n\r",     victim );
	send_to_char(AT_RED, "SILENCE set.\n\r",                ch     );
    }

    return;
}


void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    rch = get_char( ch );

    if ( !authorized( rch, "peace" ) )
        return;

    /* Yes, we are reusing rch.  -Kahn */
    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch->fighting )
	{
	    stop_fighting( rch, TRUE );
		rch->hunting = NULL;
	}
    }

    send_to_char(AT_BLUE, "Ok.\n\r", ch );
    return;
}



BAN_DATA *		ban_free;
BAN_DATA *		ban_list;

void do_ban( CHAR_DATA *ch, char *argument )
{
    BAN_DATA  *pban;
    CHAR_DATA *rch;
    char       buf [ MAX_STRING_LENGTH ];
    char       arg [ MAX_INPUT_LENGTH  ];
    char       arg1[ MAX_INPUT_LENGTH  ];
    FILE      *fp = NULL;

    if ( IS_NPC( ch ) )
	return;

    rch = get_char( ch );

    if ( !authorized( rch, "ban" ) )
        return;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );

    if ( arg[0] == '\0' )
    {
	strcpy( buf, "Banned sites and users:\n\r" );
	for ( pban = ban_list; pban; pban = pban->next )
	{
	    strcat( buf, pban->name );
	    if ( pban->user )
	    {
	      strcat( buf, " " );
	      strcat( buf, pban->user );
	    }
	    strcat( buf, "\n\r" );
	}
	send_to_char(AT_BLOOD, buf, ch );
	return;
    }

    for ( pban = ban_list; pban; pban = pban->next )
    {
	if ( ( !str_cmp( arg, pban->name ) && arg1[0] == '\0' ) 
	  || ( arg1[0] != '\0' && pban->user && !str_cmp( arg1, pban->user ) 
	  && !str_cmp( arg, pban->name ) ) )
	{
	    send_to_char(AT_RED, "That site is already banned!\n\r", ch );
	    return;
	}
    }

	pban		= new_ban();

    pban->name	= str_dup( arg );
    if ( arg1[0] != '\0' )
      pban->user = str_dup( arg1 );
    pban->next	= ban_list;
    ban_list	= pban;

#ifdef SQL_SYSTEM
	sql_save_bans();
#else
    /* write ban list to perm file  - 6/9/96 REK */
    if ( ( fp = fopen ( "banlist.txt", "w" ) ) != NULL )
    {
      for ( pban = ban_list; pban; pban = pban->next )
      {
        strcpy ( buf, pban->name );
        if ( pban->user )
        {
          strcat ( buf, " " );
          strcat ( buf, pban->user );
        }
        strcat ( buf, "\n" );
        fputs(buf, fp);
      }
      fclose ( fp );
    }
#endif
      
    send_to_char(AT_RED, "Ok.\n\r", ch );
    return;
}



void do_allow( CHAR_DATA *ch, char *argument )
{
    BAN_DATA  *prev;
    BAN_DATA  *curr;
    BAN_DATA  *pban;
    CHAR_DATA *rch;
    char       buf [ MAX_STRING_LENGTH ];
    char       arg [ MAX_INPUT_LENGTH  ];
    char       arg1 [ MAX_INPUT_LENGTH ];
    FILE      *fp;
    
    rch = get_char( ch );

    if ( !authorized( rch, "allow" ) )
        return;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_RED, "Remove which site or user from the ban list?\n\r", ch );
	return;
    }

    prev = NULL;
    for ( curr = ban_list; curr; prev = curr, curr = curr->next )
    {
	if ( ( !str_cmp( arg, curr->name ) && !curr->user && 
	   arg1[0] == '\0' ) || 
	( curr->user && !str_cmp( arg, curr->name ) &&
	    !str_cmp( arg1, curr->user ) ) )
	{
	    if ( !prev )
		ban_list   = ban_list->next;
	    else
		prev->next = curr->next;

	    free_ban( curr);

    /* write ban list to perm file  - 6/9/96 REK */
            if ( ( fp = fopen ( "banlist.txt", "w" ) ) != NULL )
            {
              for ( pban = ban_list; pban; pban = pban->next )
              {
                strcpy ( buf, pban->name );
                if ( pban->user )
                {
                  strcat ( buf, " " );
                  strcat ( buf, pban->user );
                }
                strcat ( buf, "\n" );
                fputs ( buf, fp );
              }
              fclose ( fp );
            }
      
	    send_to_char(AT_RED, "Ok.\n\r", ch );
	    return;
	}
    }

    send_to_char(AT_RED, "That site or user is not banned.\n\r", ch );
    return;
}



void do_wizlock( CHAR_DATA *ch, char *argument )
{
           CHAR_DATA *rch;
    extern bool       wizlock;

    rch = get_char( ch );

    if ( !authorized( rch, "wizlock" ) )
        return;

    wizlock = !wizlock;

    if ( wizlock )
	send_to_char(AT_BLUE, "Game wizlocked.\n\r", ch );
    else
	send_to_char(AT_BLUE, "Game un-wizlocked.\n\r", ch );


    return;
}

void do_bamfsin( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    rch = get_char( ch );

    if ( !authorized( rch, "bamfsin" ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        if ( longstring( ch, argument ) )
            return;

        smash_tilde( argument );
        free_string( ch->pcdata->bamfsin );
        ch->pcdata->bamfsin = str_dup( argument );
        send_to_char(AT_GREY, "Ok.\n\r", ch );
    }
    return;
}

void do_bamfsout( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    rch = get_char( ch );

    if ( !authorized( rch, "bamfsout" ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        if ( longstring( ch, argument ) )
            return;

        smash_tilde( argument );
        free_string( ch->pcdata->bamfsout );
        ch->pcdata->bamfsout = str_dup( argument );
        send_to_char(C_DEFAULT, "Ok.\n\r", ch );
    }
    return;
}
