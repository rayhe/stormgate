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
 *  around, comes around.
*
 ***************************************************************************/

/*$Id: act_wiz3.c,v 1.25 2005/03/15 00:10:53 tyrion Exp $*/
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
#ifndef RUN_AS_WIN32SERVICE
#include <unistd.h>
#endif
#include "merc.h"

extern char * mprog_type_to_name  args ( ( int type ) );
extern bool rel_quest_gen(CHAR_DATA* ch, int align, int levels);

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard */
const char * name_expand (CHAR_DATA *ch)
{
        int count = 1;
        CHAR_DATA *rch;
        char name[MAX_INPUT_LENGTH]; /*  HOPEFULLY no mob has a name longer than that */
        static char outbuf[MAX_INPUT_LENGTH];

        if (!IS_NPC(ch))
                return ch->name;

        one_argument (ch->name, name); /* copy the first word into name */

        if (!name[0]) /* weird mob .. no keywords */
        {
                strcpy (outbuf, ""); /* Do not return NULL, just an empty buffer */
                return outbuf;
        }

        for (rch = ch->in_room->people; rch && (rch != ch);rch =rch->next_in_room)
                if (is_name (name, rch->name))
                        count++;


        sprintf (outbuf, "%d.%s", count, name);
        return outbuf;
}



void do_pload( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA d;
  CHAR_DATA *vch = NULL;
  char buf2[MAX_STRING_LENGTH];
  int location;

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );
  if ( arg[0] == '\0' )
  {
    send_to_char( AT_WHITE, "Load which player?\n\r", ch );
    return;
  }

  for ( vch = char_list; vch != NULL; vch = vch->next )
  {
    if ( vch->deleted )
      continue;
    if ( !IS_NPC( vch ) && !str_cmp( arg, vch->name ) )
    {
      send_to_char( AT_WHITE, "Character is already playing.\n\r", ch );
      return;
    }
  }

  vch = NULL;
  d.original = NULL;

  if ( !load_char_obj( &d, arg ) && str_cmp( arg2, "newname" ) )
  {
    free_ch( d.character );
    send_to_char( AT_WHITE, "Player does not exist.\n\r", ch );
    return;
  }
  vch = d.character;
  d.character = NULL;
  vch->desc = NULL;
  vch->next = char_list;
  char_list = vch;
  location = vch->in_room->vnum;
  char_to_room( vch, ch->in_room );
  {
    char buf[MAX_INPUT_LENGTH];

    strcpy( buf, vch->name );
    free_string( vch->name );
    vch->name = str_dup( capitalize( buf ) );
  }
  sprintf( buf2, "Player loaded.  Was in room vnum %d.\n\r", location );

  send_to_char( AT_WHITE, buf2, ch );
  return;
}

/* -- Altrag */
void do_sstat( CHAR_DATA *ch, char *argument )
{
  char spbuf[MAX_STRING_LENGTH*10];
  char skbuf[MAX_STRING_LENGTH*10];
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int sn;
  int spcol = 0;
  int skcol = 0;
  CHAR_DATA *victim;

  argument = one_argument( argument, arg );

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

  strcpy(spbuf,"\n\r");
  strcpy(skbuf,"\n\r");

  /* Use 1 to skip the reserved sn -- Altrag */
  for ( sn = 1; skill_table[sn].name[0] != '\0'; sn++ )
  {
    if ( victim->pcdata->learned[sn] <= 0 &&
	 skill_table[sn].skill_level[victim->class] > victim->level )
      continue;
    sprintf( buf, "%26s %5.1f%%  ", skill_table[sn].name,
	     (UMIN (1000, victim->pcdata->learned[sn])/10.0f) );
    if ( skill_table[sn].is_spell )
    {
      strcat( spbuf, buf );
      if ( ++spcol % 2 == 0 )
	strcat( spbuf, "\n\r" );
      continue;
    }

    strcat( skbuf, buf );
    if ( ++skcol % 2 == 0 )
      strcat( skbuf, "\n\r" );
  }
  if ( spcol % 2 != 0 )
    strcat( spbuf, "\n\r" );
  if ( skcol % 2 != 0 )
    strcat( skbuf, "\n\r" );
  send_to_char(AT_PINK,
"-----------------=================[Spells]=================-----------------",
	       ch );
  send_to_char(AT_PINK, spbuf, ch );
  send_to_char(AT_PURPLE,
"-----------------=================[Skills]=================-----------------",
	       ch );
  send_to_char(AT_PURPLE, skbuf, ch );
}

void do_string( CHAR_DATA *ch, char *argument )
{
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( !authorized( ch, "string" ) )
        return;

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if (type[0]=='\0' || arg1[0]=='\0' || arg2[0]=='\0' || arg3[0]=='\0')
    {
        send_to_char(AT_WHITE,"Syntax:\n\r",ch);
        send_to_char(AT_WHITE,"  string char <name> <field> <string>\n\r",ch);
        send_to_char(AT_WHITE,"    fields: name short long desc title spec game\n\r",ch);
	send_to_char(AT_WHITE,"  string obj  <name> <field> <string>\n\r",ch);
        send_to_char(AT_WHITE,"    fields: name short long extended\n\r",ch);
        return;
    }

    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
        if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
        {
            send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
            return;
        }

        /* string something */
        if ( !str_prefix( arg2, "name" ) )
        {
            if ( !IS_NPC(victim) )
            {
                send_to_char(AT_WHITE, "Not on PC's.\n\r", ch );
                return;
            }

            free_string( victim->name );
            victim->name = str_dup( arg3 );
            return;
        }

        if ( !str_prefix( arg2, "description" ) )
        {
            free_string(victim->description);
            victim->description = str_dup(arg3);
            return;
            return;
        }
        if ( !str_prefix( arg2, "short" ) )
        {
            free_string( victim->short_descr );
            victim->short_descr = str_dup( arg3 );
            return;
        }

        if ( !str_prefix( arg2, "long" ) )
        {
            free_string( victim->long_descr );
            strcat(arg3,"\n\r");
            victim->long_descr = str_dup( arg3 );
            return;
        }

        if ( !str_prefix( arg2, "title" ) )
        {
            if ( IS_NPC(victim) )
            {
                send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
                return;
            }
            set_title( victim, arg3 );
            return;
        }

        if ( !str_prefix( arg2, "spec" ) )
        {
            if ( !IS_NPC(victim) )
            {
                send_to_char(AT_WHITE, "Not on PC's.\n\r", ch );
                return;
            }
            if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
            {
                send_to_char(AT_WHITE, "No such spec fun.\n\r", ch );
                return;
            }

            return;
        }

        if ( !str_prefix( arg2, "game" ) )
        {
            if ( !IS_NPC(victim) )
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

    }

    if (!str_prefix(type,"object"))
    {
        /* string an obj */

        if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
        {
            send_to_char(AT_WHITE, "Nothing like that in heaven or earth.\n\r", ch );
            return;
        }

        if ( !str_prefix( arg2, "name" ) )
        {
            free_string( obj->name );
            obj->name = str_dup( arg3 );
            return;
        }

        if ( !str_prefix( arg2, "short" ) )
        {
            free_string( obj->short_descr );
            obj->short_descr = str_dup( arg3 );
            return;
        }

        if ( !str_prefix( arg2, "long" ) )
        {
            free_string( obj->description );
            obj->description = str_dup( arg3 );
            return;
        }

        if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
        {
            EXTRA_DESCR_DATA *ed;

            argument = one_argument( argument, arg3 );
            if ( argument == NULL )
            {
                send_to_char(AT_WHITE, "Syntax: oset <object> ed <keyword> <string>\n\r",
                    ch );
                return;
            }

            strcat(argument,"\n\r");

	    ed = new_extra_descr();
            ed->keyword         = str_dup( arg3     );
            ed->description     = str_dup( argument );
            ed->next            = obj->extra_descr;
            obj->extra_descr    = ed;
            return;
        }
    }


    /* echo bad use message */
    do_string(ch,"");
}



void do_update( CHAR_DATA *ch, char *argument )         /* by Maniac */
{
        char    arg[MAX_INPUT_LENGTH];
        int     value;

        if ( !authorized( ch, "update" ) )
                return;

        if ( argument[0] == '\0' )      /* No options ??? */
        {
                send_to_char(AT_WHITE, "Update, call some game functions\n\r\n\r", ch );
                send_to_char(AT_WHITE, "bank [value]: Update the share_value.\n\r", ch );
                send_to_char(AT_WHITE, "time [value]: Set time hour to <value>.\n\r", ch);
                send_to_char(AT_WHITE, "dumpdb [value]: Set OLC database dunp time to <value>.\n\r", ch );
                return;
        }

        argument = one_argument(argument, arg);

        if (!str_prefix(arg, "bank" ) )
        {
                one_argument(argument, arg);
                value = 0;
                if (arg)
                {
                        value = atoi(arg);
                        if (value)
                                share_value = value;
                }
                else
                        bank_update ( );

                sprintf(arg, "Ok...bank updated, share_value is now: %d\n\r", share_value );
                send_to_char(AT_WHITE, arg, ch);
                return;
        }
        if (!str_prefix(arg, "dumpdb" ) )
        {
                one_argument(argument,arg );
                value = 0;
                if (arg)
                        value = atoi(arg);
                if (value)
                        pulse_db_dump = value;
                sprintf (arg, "Ok... db dump time is now %d\n\r", pulse_db_dump );
                send_to_char(AT_WHITE, arg, ch);
                return;
        }
        if (!str_prefix(arg, "time" ) )
        {
                one_argument(argument, arg);
                if (arg)
                        value = atoi(arg);
                else
                {
                        sprintf (arg, "Current hour is %d.\n\r", time_info.hour);
                        send_to_char(AT_WHITE, arg, ch);
                        return;
                }
                if (!value)
                        return;
                if ((value >= 0) && (value < 24))
                        time_info.hour = value;
                return;
        }
        return;
}

void do_mpcommands( CHAR_DATA *ch, char *argument )
{
  char buf[ MAX_STRING_LENGTH ];
  char buf1[ MAX_STRING_LENGTH ];
  int cmd;
  int col;

  buf1[0] = '\0';
  col = 0;
  for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
  {
     if ( !str_prefix( "mp", cmd_table[cmd].name ) )
     {
     if ( str_cmp( cmd_table[cmd].name, "mpcommands" ) )
     {
       sprintf( buf, "%-16s", cmd_table[cmd].name );
       strcat( buf1, buf );
       if ( ++col % 5 == 0 )
          strcat( buf1, "\n\r" );
     }
     }
  }

  if ( col % 5 != 0 )
     strcat( buf1, "\n\r" );

  send_to_char( C_DEFAULT, buf1, ch );
  return;
}

const char * log_list [] = {"normal", "always", "never", "build"};
void do_restrict( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  int cmd;
  int lvl;
  bool log = FALSE;

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg[0] != '\0' && arg1[0] == '\0' )
  {
    char buf[MAX_STRING_LENGTH];

    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
      if ( !str_prefix( cmd_table[cmd].name, arg ) )
	break;
    if ( cmd_table[cmd].name[0] == '\0' || cmd_table[cmd].level > get_trust(ch) )
    {
      do_restrict( ch, "" );
      return;
    }
    sprintf( buf, "Command '%s'.  Level %d.  Logged %s.\n\r",
	     cmd_table[cmd].name, cmd_table[cmd].level,
	     log_list[cmd_table[cmd].log] );
    send_to_char( AT_PURPLE, buf, ch );
    return;
  }

  if ( arg[0] == '\0' || arg1[0] == '\0' || ( !is_number( arg1 )
     && ( ( arg2[0] == '\0' ) || !is_number( arg2 ) ) ) )
  {
     send_to_char( AT_PURPLE, "Syntax: restrict [command] [level]\n\r", ch );
     send_to_char( AT_PURPLE, "Syntax: restrict [command] log [logtype]\n\r", ch );
     send_to_char( AT_PURPLE, "Syntax: restrict [command]\n\r\n\r",ch);
     send_to_char( AT_PURPLE, "Logtype being 0-4:\n\r", ch );
     send_to_char( AT_PURPLE, "0 - LOG_NEVER\n\r", ch);
     send_to_char( AT_PURPLE, "1 - LOG_ALWAYS (on God)\n\r", ch);
     send_to_char( AT_PURPLE, "2 - LOG_NORMAL (not logged)\n\r", ch );
     send_to_char( AT_PURPLE, "3 - LOG_BUILD  (logged on build)\n\r", ch );
     return;
  }

  if ( arg2[0] != '\0' )
  {
     if ( !str_cmp( "log", arg1 ) )
     {
       lvl = atoi( arg2 );
       log = TRUE;
    }
   else
    lvl = atoi( arg1 );
  }
else
  lvl = atoi( arg1 );

  if ( !str_cmp( "all", arg ) )
  {
    int col = 0;
    cmd = 1;
    lvl = 1;
    if ( is_number( arg1 ) )
      cmd = atoi( arg1 );
    if ( is_number( arg2 ) )
      lvl = atoi( arg2 );
      send_to_char( AT_WHITE, "\n\r", ch );
    for ( ; ( cmd <= lvl ) && ( cmd_table[cmd].name[0] != '\0' ); cmd++ )
    {
      if ( get_trust( ch ) >= cmd_table[cmd].level )
      {
        sprintf( log_buf, "%-3d &R%-12s &B(&Y%3d&B)   ", cmd, cmd_table[cmd].name, cmd_table[cmd].level );
        send_to_char( AT_WHITE, log_buf, ch );
      }
      if ( ++col % 3 == 0 )
        send_to_char( AT_WHITE, "\n\r", ch );
    }
    if ( col % 3 != 0 )
      send_to_char( AT_WHITE, "\n\r", ch );
    return;
  }

  if ( ( ( lvl < 0 || lvl > 110 || lvl > get_trust( ch ) )&& ( arg2[0] == '\0' ) )
   || ( ( arg2[0] != '\0' ) && ( lvl < 0 || lvl > 3 ) ) )
  {
     send_to_char( AT_WHITE, "Invalid level.\n\r", ch );
     return;
  }

  for ( cmd = 1; cmd_table[cmd].name != '\0'; cmd++ )
  {
    if ( !str_prefix( arg, cmd_table[cmd].name ) )
    {
      strcpy( arg, cmd_table[cmd].name );

      if ( cmd_table[cmd].level > get_trust( ch ) )
      {
        if (log)
        send_to_char( AT_WHITE, "You cannot change the log_type on a command which you do not have.\n\r", ch );
      else
        send_to_char( AT_WHITE, "You cannot restrict a command which you do not have.\n\r", ch );
        return;
      }
      if (log)
        cmd_table[cmd].log = lvl;
      else
      cmd_table[cmd].level = lvl;
      if (log)
        sprintf( log_buf, "%s changing log_type of %s to %d.",
         ch->name, arg, lvl );
      else
        sprintf( log_buf, "%s restricting %s to level %d.",
        ch->name, arg, lvl );
      log_string( log_buf, CHANNEL_GOD, ch->level - 1 );
      if (log)
        sprintf( log_buf, "You change the log_type of %s to %d.\n\r",
        arg, lvl );
      else
        sprintf( log_buf, "You restrict %s to level %d.\n\r",
        arg, lvl );
      send_to_char( AT_WHITE, log_buf, ch );
      break;
    }

  }
  if ( cmd_table[cmd].name == '\0' )
  {
     sprintf( log_buf, "There is no %s command.",
       arg );
     send_to_char( AT_WHITE, log_buf, ch );
  }
  return;
}

void do_wrlist( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *room;
  ROOM_INDEX_DATA *in_room;
  MOB_INDEX_DATA *mob;
  OBJ_INDEX_DATA *obj;
  MPROG_DATA *mprog;
  char arg[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  int uvnum;
  int lvnum;
  int MR = 32767;
  int type = -1;

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  uvnum = ( is_number( arg2 ) ) ? atoi( arg2 ) : 0;
  lvnum = ( is_number( arg1 ) ) ? atoi( arg1 ) : 0;

  if ( !str_cmp( arg, "o" ) )
    type = 2;
  if ( !str_cmp( arg, "m" ) )
    type = 1;
  if ( !str_cmp( arg, "r" ) )
    type = 0;
  if ( !str_cmp( arg, "p" ) )
    type = 3;

  if ( ( uvnum - lvnum ) > 200 )
  {
    send_to_char( AT_WHITE, "That range is too large.\n\r", ch );
    return;
  }

  if ( ( ( uvnum == 0 ) && ( lvnum == 0 ) ) || ( arg[0] == '\0' )
   || ( type == -1 ) )
  {
    send_to_char( AT_PURPLE, "Syntax: wrlist [type] [lvnum] [uvnum]\n\r", ch );
    return;
  }

  if ( uvnum > MR || uvnum < 1 || lvnum > MR || lvnum < 1 || lvnum > uvnum )
  {
    send_to_char( AT_WHITE, "Invalid level(s).\n\r", ch );
    return;
  }

  in_room = ch->in_room;
  if ( type == 0 )
  {
    char_from_room( ch );
  }
  for ( MR = lvnum; MR <= uvnum; MR++ )
  {
    if ( type == 0 )
    {
      if ( ( room = get_room_index( MR ) ) )
      {
        sprintf( log_buf, "&R%-5d  &w%-20s\n\r", room->vnum, room->name );
        send_to_char( AT_WHITE, log_buf, ch );
        char_to_room( ch, room );
        do_resets( ch, "" );
        char_from_room( ch );
      }
    }
    if ( type == 2 )
    {
      if ( ( obj = get_obj_index( MR ) ) )
      {
        sprintf( log_buf, "&R%-5d  &w%-20s\n\r",  obj->vnum, obj->name );
        send_to_char( AT_WHITE, log_buf, ch );
      }
    }
    if ( type == 1 )
    {
      if ( ( mob = get_mob_index( MR ) ) )
      {
        sprintf( log_buf, "&R%-5d  &w%-20s\n\r", mob->vnum, mob->player_name );
        send_to_char( AT_WHITE, log_buf, ch );
        if ( mob->mobprogs )
          for ( mprog = mob->mobprogs; mprog; mprog = mprog->next )
          {
            sprintf( log_buf, "  :%s  %s\n\r", mprog_type_to_name( mprog->type ),
              mprog->arglist ? mprog->arglist : "(None)" );
            send_to_char( C_DEFAULT, log_buf, ch );
          }
      }
    }
    if ( type == 3 )
    {
      if ( ( mob = get_mob_index( MR ) ) )
      {
        if ( mob->mobprogs )
        {
          sprintf( log_buf, "%d: %s\n\r", mob->vnum, mob->player_name );
          send_to_char( AT_WHITE, log_buf, ch );
          for ( mprog = mob->mobprogs; mprog; mprog = mprog->next )
          {
            sprintf( log_buf, "%s&R:&B %s\n\r&w%s\n\r",
              mprog_type_to_name( mprog->type ),
              mprog->arglist ? mprog->arglist : "(None)",
              mprog->comlist ? mprog->comlist : "(None)"  );
            send_to_char( C_DEFAULT, log_buf, ch );
          }
        }
      }
  }
  }
  if ( type == 0 )
    char_to_room( ch, in_room );
  return;
}

void do_vused( CHAR_DATA *ch, char *argument )
{
  char buf1[MAX_STRING_LENGTH];
  int atvnum;
  int freevnum = 0;
  int bstart = -1;
  int bend;
  int col = 0;

  send_to_char(AT_PINK,"Used VNUMs (ignores vnums assigned but not created).\n\r",ch);
  send_to_char(AT_PINK,"Double check free vnums against assigned vnums in &Balist&P.\n\r",ch);

  for ( atvnum = 1; atvnum <= 32767; atvnum++ )
  {
    if ( get_room_index( atvnum ) || get_mob_index(atvnum) || get_obj_index(atvnum))
    {
      if ( bstart == -1 )
        bstart = atvnum;
    }
   else if ( bstart != -1 )
    {
      bend = ( atvnum - 1 );
      sprintf( buf1, "%5d&R-&W%5d  ", bstart, bend );
      send_to_char( AT_WHITE,buf1,ch);
      if ( ++col % 6 == 0 )
	send_to_char( AT_WHITE,"\n\r",ch);
      freevnum++;
      bstart = -1;
    }
   else
     freevnum++;
  }
  if ( col % 6 != 0 )
    send_to_char( AT_WHITE, "\n\r",ch );
  sprintf( buf1, "There are %d free vnums.\n\r", freevnum );
  send_to_char( AT_WHITE, buf1, ch );
  return;
}

void do_hlist( CHAR_DATA *ch, char *argument )
{
  HELP_DATA *pHelp;
  int uvnum;
  int lvnum;
  int atvnum = 1;
  char arg[MAX_STRING_LENGTH*2];
  char arg1[MAX_STRING_LENGTH*2];

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg1 );
  lvnum = is_number( arg ) ? atoi( arg ) : 0;
  uvnum = is_number( arg1 ) ? atoi( arg1 ) : 0;

  if ( lvnum == 0 || uvnum == 0 || lvnum > uvnum )
  {
    send_to_char( AT_PURPLE, "Syntax: hlist [firsthelp#] [secondhelp#]\n\r",
       ch );
    return;
  }

  for ( pHelp = help_first; pHelp; pHelp = pHelp->next, ++atvnum )
  {
    if ( atvnum >= lvnum && atvnum <= uvnum )
    {
      sprintf( arg, "[&R%-3d&B]  &w%s\n\r", atvnum, pHelp->keyword );
      send_to_char( AT_BLUE, arg, ch );
    }
  }

  return;
}

void do_astrip( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;

  argument = one_argument( argument, arg );

  if ( !(victim = get_char_room(ch, arg ) ) )
  {
    send_to_char(C_DEFAULT, "They aren't here.\n\r",ch);
    return;
  }

  for( paf = victim->affected; paf; paf = paf_next )
  {
    paf_next = paf->next;
    affect_remove( victim, paf );
  }

  for( paf = victim->affected2; paf; paf = paf_next )
  {
    paf_next = paf->next;
    affect_remove2( victim, paf );
  }

  for( paf = victim->affected3; paf; paf = paf_next )
  {
    paf_next = paf->next;
    affect_remove3( victim, paf );
  }

  for( paf = victim->affected4; paf; paf = paf_next )
  {
    paf_next = paf->next;
    affect_remove4( victim, paf );
  }

  victim->affected_by = 0;
  victim->affected_by2 &= CODER;
  victim->affected_by3 = 0;
  victim->affected_by4 = 0;
  victim->shields = 0;

  if ( ch != victim )
    send_to_char(C_DEFAULT, "All your affects have been removed.\n\r", victim);
  send_to_char(C_DEFAULT, "Ok.\n\r",ch);
}

void do_fset( CHAR_DATA *ch, char *argument )
{
  USERL_DATA *ul;
  USERL_DATA *ulmark;
  char arg[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];

  if ( IS_NPC( ch ) ) return;

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg1 );
  strcpy( arg2, argument );

  if ( arg[0] == '\0' || arg1[0] == '\0' ||
     ( arg2[0] == '\0' && str_cmp( arg1, "desc" )
     && str_cmp( arg1, "delete" ) ) )
  {
    send_to_char( AT_PURPLE, "Syntax: fset <playername> <field> <string>\n\r", ch );
    send_to_char( AT_PURPLE, "Syntax: fset <playername> <field>\n\r", ch );
    send_to_char( AT_PURPLE, "\n\rField being one of:\n\r", ch );
    send_to_char( AT_PURPLE, "  user host laston desc\n\r", ch );
    send_to_char( AT_PURPLE, " or: fset <playername> delete\n\r", ch );
    return;
  }

  for ( ul = user_list; ul; ul = ul->next )
  {
    if ( !str_cmp( ul->name, arg ) )
      break;
  }

  if ( !ul )
  {
    send_to_char( AT_WHITE, "No such player exists!\n\r", ch );
    return;
  }

  if ( !str_cmp( "delete", arg1 ) )
  {
    if ( ul == user_list )
      user_list = ul->next;
    for ( ulmark = user_list; ul; ul = ul->next )
    {
      if ( ul == ulmark->next )
      {
        ulmark->next = ul->next;
        free_userl( ul);
        send_to_char( AT_WHITE, "Character deleted from user list.\n\r", ch );
        return;
      }
    }
    return;
  }

  if ( !str_cmp( "desc", arg1 ) )
  {
    if ( !str_cmp( "none", arg2 ) )
    {
      free_string(ul->desc);
      ul->desc = str_dup( "(none)" );
      send_to_char( AT_WHITE, "Ok.\n\r", ch );
      return;
    }
    string_append( ch, &ul->desc );
    if ( !ul->desc )
      ul->desc = str_dup( "(none)" );
    send_to_char( AT_WHITE, "Ok.\n\r", ch );
    return;
  }

  if ( !str_cmp( "laston", arg1 ) )
  {
    free_string(ul->lastlogin);
    ul->lastlogin = str_dup( arg2 );
    send_to_char( AT_WHITE, "Ok.\n\r", ch );
    return;
  }

  if ( !str_cmp( "host", arg1 ) )
  {
    free_string(ul->host);
    ul->host = str_dup( arg2 );
    send_to_char( AT_WHITE, "Ok.\n\r", ch );
    return;
  }

  if ( !str_cmp( "user", arg1 ) )
  {
    free_string(ul->user);
    ul->user = str_dup( arg2 );
    send_to_char( AT_WHITE, "Ok.\n\r", ch );
    return;
  }

  do_fset( ch, "" );
  return;
}

void do_bodybag( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  OBJ_DATA *body = NULL;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int howmany = 0;

  argument = one_argument( argument, arg );
  return;   /* disable bodybag for now -- REK */
  for ( obj = object_list; obj; obj = obj_next )
  {
    OBJ_DATA *in;
    OBJ_DATA *in_next;

    obj_next = obj->next;
    if ( obj->deleted )
      continue;

    if ( obj->item_type != ITEM_CORPSE_PC )
      continue;
    if ( !obj->in_room )
      continue;
    if ( str_cmp( arg, obj->short_descr + 10 ) )
      continue;

    for ( in = obj->contains; in; in = in_next )
    {
      in_next = in->next_content;

      if ( howmany == 0 )
      {
	char name[MAX_INPUT_LENGTH];

	strcpy(name, obj->short_descr + 10);
	body = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0 );
        sprintf( buf, "bodybag body bag %s", name );
        body->name = str_dup( buf );
	sprintf(buf, "corpse of %s", name );
	body->short_descr = str_dup( buf );
	sprintf(buf, "The bodybag of %s.", name);
	body->description = str_dup(buf);
	obj_to_char(body, ch);
      }
      obj_from_obj(in);
      obj_to_obj(in, body);
      howmany++;
    }
    extract_obj(obj);
    body->timer = number_range( 25, 45 );
  }
  return;
}

/*
 * Rename by Erwin S Andreasen (4u2@aabc.dk)
 */

/*
 * do_rename renames a player to another name.
 * PCs only. Previous file is deleted, if it exists.
 * Char is then saved to new file.
 * New name is checked against std. checks, existing offline players and
 * online players.
 * .gz files are checked for too, just in case.
 */

bool check_parse_name (char* name);  /* comm.c */
char *initial( const char *str );    /* comm.c */

void do_rename (CHAR_DATA* ch, char* argument)
{
        char old_name[MAX_INPUT_LENGTH],
             new_name[MAX_INPUT_LENGTH],
             strsave [MAX_INPUT_LENGTH];

        CHAR_DATA* victim;
        FILE* file;

        if ( !authorized( ch, "rename" ) )
                return;

        argument = one_argument(argument, old_name); /* find new/old name */
        one_argument (argument, new_name);

        /* Trivial checks */
        if (!old_name[0])
        {
                send_to_char (AT_WHITE,"Rename who?\n\r",ch);
                return;
        }

        victim = get_char_world (ch, old_name);

        if (!victim)
        {
                send_to_char (AT_WHITE,"There is no such a person online.\n\r",ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char (AT_WHITE,"You cannot use Rename on NPCs.\n\r",ch);
                return;
        }

        /* allow rename self new_name,but otherwise only lower level */
        if ( (victim != ch) && (get_trust (victim) >= get_trust (ch)) )
        {
                send_to_char (AT_WHITE,"You failed.\n\r",ch);
                return;
        }

        if (!victim->desc || (victim->desc->connected != CON_PLAYING) )
        {
                send_to_char (AT_WHITE,"This player has lost his link or is inside a pager or the like.\n\r",ch);
                return;
        }

        if (!new_name[0])
        {
                send_to_char (AT_WHITE,"Rename to what new name?\n\r",ch);
                return;
        }

        /* Insert check for clan here!! */
        /*

        if (victim->clan)
        {
                send_to_char (AT_WHITE,"This player is member of a clan, remove him from there first.\n\r",ch);
                return;
        }
        */

        if (!check_parse_name(new_name))
        {
                send_to_char (AT_WHITE,"The new name is illegal.\n\r",ch);
                return;
        }

        /* First, check if there is a player named that off-line */
#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( new_name ),
                 "/", capitalize( new_name ) );
#else
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( new_name ) );
#endif

        fclose (fpReserve); /* close the reserve file */
        file = fopen (strsave, "r"); /* attempt to to open pfile */
        if (file)
        {
                send_to_char (AT_WHITE,"A player with that name already exists!\n\r",ch);
                fclose (file);
        fpReserve = fopen( NULL_FILE, "r" ); /* is this really necessary these days? */
                return;
        }
        fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */
        /* Check .gz file ! */
#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%s%s%s.gz", PLAYER_DIR, initial( new_name ),
                 "/", capitalize( new_name ) );
#else
    sprintf( strsave, "%s%s.gz", PLAYER_DIR, capitalize( new_name ) );
#endif

        fclose (fpReserve); /* close the reserve file */
        file = fopen (strsave, "r"); /* attempt to to open pfile */
        if (file)
        {
                send_to_char (AT_WHITE,"A player with that name already exists in a compressed file!\n\r",ch);
                fclose (file);
        fpReserve = fopen( NULL_FILE, "r" );
                return;
        }
        fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */
        if (get_char_world(ch,new_name)) /* check for playing level-1 non-saved */
        {
                send_to_char (AT_WHITE,"A player with the name you \
 specified already exists!\n\r",ch);
                return;
        }

        /* Save the filename of the old name */

#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( victim->name ),
                 "/", capitalize( victim->name ) );
#else
    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );
#endif


        /* Rename the character and save him to a new file */
        /* NOTE: Players who are level 1 do NOT get saved under a new name */

        free_string (victim->name);
        victim->name = str_dup (capitalize(new_name));

        save_char_obj (victim, FALSE);

        /* unlink the old file */
        unlink (strsave); /* unlink does return a value.. but we do not care */

        /* That's it! */

        send_to_char (AT_WHITE,"Character renamed.\n\r",ch);

        victim->position = POS_STANDING; /* I am laaazy */
        act (AT_WHITE,"$n has renamed you to $N!",ch,NULL,victim,TO_VICT);

}

/*
 * For by Erwin S. Andreasen (4u2@aabc.dk)
 */
void do_for (CHAR_DATA *ch, char *argument)
{
        char range[MAX_INPUT_LENGTH];
        char buf[MAX_STRING_LENGTH];
        bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere = FALSE, found;
        ROOM_INDEX_DATA *room, *old_room;
        CHAR_DATA *p, *p_next;
        int i;


        if ( !authorized( ch, "atall" ) )
                return;

        argument = one_argument (argument, range);

        if (!range[0] || !argument[0]) /* invalid usage? */
        {
                do_help (ch, "atall");
                return;
        }

        if (!str_prefix("quit", argument))
        {
                send_to_char (AT_WHITE,"Are you trying to crash the MUD or something?\n\r",ch);
                return;
        }


        if (!str_cmp (range, "all"))
        {
                fMortals = TRUE;
                fGods = TRUE;
        }
        else if (!str_cmp (range, "gods"))
                fGods = TRUE;
        else if (!str_cmp (range, "mortals"))
                fMortals = TRUE;
        else if (!str_cmp (range, "mobs"))
                fMobs = TRUE;
        else if (!str_cmp (range, "everywhere"))
                fEverywhere = TRUE;
        else
                do_help (ch, "atall"); /* show syntax */

        /* do not allow # to make it easier */
        /* Don't allow it, it causes crashes on 'for mobs tell # hi' Maniac */
        if (fEverywhere && strchr (argument, '#'))
        {
                send_to_char (AT_WHITE,"Cannot use FOR EVERYWHERE with the # thingie.\n\r",ch);
                return;
        }

        if (fMobs && strchr (argument, '#'))
        {
                send_to_char (AT_WHITE,"Cannot use FOR MOBS with the # thingie.\n\r",ch);
                return;
        }
        if (strchr (argument, '#')) /* replace # ? */
        {
                for (p = char_list; p ; p = p_next)
                {
                        p_next = p->next; /* In case someone DOES try to AT MOBS SLAY # */
                        found = FALSE;

                        if (!(p->in_room) || room_is_private(p->in_room) || (p == ch))
                                continue;

                        if (IS_NPC(p) && fMobs)
                                found = TRUE;
                        else if (!IS_NPC(p) && p->level >= LEVEL_IMMORTAL && fGods)
                                found = TRUE;
                        else if (!IS_NPC(p) && p->level < LEVEL_IMMORTAL && fMortals)
                                found = TRUE;
                                found = TRUE;

                        /* It looks ugly to me.. but it works :) */
                        if (found) /* p is 'appropriate' */
                        {
                                char *pSource = argument; /* head of buffer to be parsed */
                                char *pDest = buf; /* parse into this */

                                while (*pSource)
                                {
                                        if (*pSource == '#') /* Replace # with name of target */
                                        {
                                                const char *namebuf = name_expand (p);

                                                if (namebuf) /* in case there is no mob name ?? */
                                                        while (*namebuf)/* copy name over */
                                                                *(pDest++) = *(namebuf++);

                                                pSource++;
                                        }
                                        else
                                                *(pDest++) = *(pSource++);
                                } /* while */
                                *pDest = '\0'; /* Terminate */

                                /* Execute */
                                old_room = ch->in_room;
                                char_from_room (ch);
                                char_to_room (ch,p->in_room);
                                interpret (ch, buf);
                                char_from_room (ch);
                                char_to_room (ch,old_room);

                        } /* if found */
                } /* for every char */
        }
        else /* just for every room with the appropriate people in it */
        {
                for (i = 0; i < MAX_KEY_HASH; i++) /* run through all the buckets */

                        for (room = room_index_hash[i] ; room ; room =room->next)
                        {
                                found = FALSE;

                                /* Anyone in here at all? */
                                if (fEverywhere) /* Everywhere executes always */
                                        found = TRUE;
                                else if (!room->people) /* Skip it if room is empty */
                                        continue;


                                /* Check if there is anyone here of the requried type */
                                /* Stop as soon as a match is found or there are no more ppl in room */
                                for (p = room->people; p && !found; p =p->next_in_room)
                                {

                                        if (p == ch) /* do not execute on oneself */
                                                continue;

                                        if (IS_NPC(p) && fMobs)
                                                found = TRUE;
                                        else if (!IS_NPC(p) && (p->level>= LEVEL_IMMORTAL) && fGods)
                                                found = TRUE;
                                        else if (!IS_NPC(p) && (p->level <= LEVEL_IMMORTAL) && fMortals)
                                                found = TRUE;
                                } /* for everyone inside the room */

                                if (found && !room_is_private(room)) /* Any of the required type here AND room not private? */
                                {
                                        /* This may be ineffective. Consider moving character out of old_room
                                           once at beginning of command then moving back at the end.
                                           This however, is more safe?
                                        */

                                        old_room = ch->in_room;
                                        char_from_room (ch);
                                        char_to_room (ch, room);
                                        interpret (ch, argument);
                                        char_from_room (ch);
                                        char_to_room (ch, old_room);
                                } /* if found */
                        } /* for every room in a bucket */
        } /* if strchr */
} /* do_for */


void do_seize( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *obj;
    CHAR_DATA *victim;
    bool      found;
    char       arg1 [ MAX_INPUT_LENGTH  ];
    char       arg2 [ MAX_INPUT_LENGTH  ];

    if ( !authorized( ch, "seize" ) )
        return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char(AT_BLOOD, "Seize what from whom?\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world( ch, arg2 ) ) )
    {
        send_to_char(AT_BLOOD, "They aren't in the mud.\n\r", ch );
        return;
    }

    found = FALSE;
    for(;;) {
         if ( !( obj = get_obj_carry( victim, arg1 ) ) &&
		!( obj = get_obj_storage( victim, arg1 ) ) &&
		!( obj = get_obj_wear( victim, arg1 ) ) ) {
		break;
    	 }

        found = TRUE;
    	obj_from_char( obj );
    	obj_to_char( obj, ch );
    	save_char_obj( victim, FALSE );
    	act(AT_DGREEN, "You seize $p from $N.", ch, obj, victim, TO_CHAR    );
    }

    if (!found) {
	send_to_char( AT_BLOOD, "You can't find it.\n\r", ch );
    }
    return;
}

/* Display some simple Immortal-only information to an Immortal. */
void do_iscore( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];
   char buf1[MAX_STRING_LENGTH];
   extern bool wizlock;
   extern int numlock;
   char arg1[MAX_INPUT_LENGTH];
   int col = 0;
   int cmd;

   if ( !authorized( ch, "iscore" ) )
        return;

   argument = one_argument( argument, arg1 );

   if ( arg1[0] == '\0' )
        victim = ch;
   else
   {
        if ( !( victim = get_char_world( ch, arg1) ) )
        {
            send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
            return;
        }
        if ( IS_NPC( victim ) )
        {
            send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
            return;
        }
   }

   sprintf( buf, "Bamfin:  %s\n\r",
      (victim->pcdata != NULL && victim->pcdata->bamfin[0] != '\0')
      ? victim->pcdata->bamfin : "Not changed/Switched." );
   send_to_char(AT_WHITE, buf, ch );

   sprintf( buf, "Bamfout: %s\n\r",
      (victim->pcdata != NULL && victim->pcdata->bamfout[0] != '\0' )
      ? victim->pcdata->bamfout : "Not changed/Switched." );
   send_to_char(AT_WHITE, buf, ch );

   sprintf( buf, "Bamfsin: %s\n\r",
      (victim->pcdata != NULL && victim->pcdata->bamfsin[0] != '\0' )
      ? victim->pcdata->bamfsin : "Not changed/Switched." );
   send_to_char(AT_WHITE, buf, ch );

   sprintf( buf, "Bamfsout: %s\n\r",
      (victim->pcdata != NULL && victim->pcdata->bamfsout[0] != '\0' )
      ? victim->pcdata->bamfsout : "Not changed/Switched." );
   send_to_char(AT_WHITE, buf, ch );

   /*
    * imortall skills listing added by Canth (canth@xs4all.nl)
    */

   sprintf( buf, "Imortal skills set for %s:\n\r", victim->name );
   send_to_char(AT_WHITE, buf, ch );
   buf1[0] = '\0';
   for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
   {
        if( cmd_table[cmd].level < LEVEL_HERO
            || str_infix( cmd_table[cmd].name, victim->pcdata->immskll ) )
            continue;
        sprintf( buf, "%-10s", cmd_table[cmd].name );
        strcat( buf1, buf );
        if( ++col % 8 == 0 )
            strcat( buf1, "\n\r" );
   }
   if( col % 8 != 0 )
        strcat( buf1, "\n\r" );
   send_to_char(AT_WHITE, buf1, ch );

   if ( wizlock )
     send_to_char(AT_WHITE, "The Mud is currently Wizlocked.\n\r", ch );

   /*
    * Numlock check added by Canth (canth@xs4all.nl)
    */
   if ( numlock )
   {
        sprintf( buf, "The Mud is currently Numlocked at level %d.\n\r", numlock );
        send_to_char(AT_WHITE, buf, ch );
   }

   return;
}

/*
 * Modifications to old imtlset by:
 * Canth (canth@xs4all.nl)
 * Vego (v942429@si.hhs.nl)
 */
void do_imtlset( CHAR_DATA *ch, char *argument )
{

    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg1 [ MAX_INPUT_LENGTH  ];
    char       buf  [ MAX_STRING_LENGTH ];
    char       buf1 [ MAX_STRING_LENGTH ];
    char      *buf2;
    char      *buf3 = NULL;
    char      *skill;
    int        cmd;
    int        col = 0;
    int        i = 0;


    rch = get_char( ch );

    if ( ( !authorized( rch, "imtlset" ) && ( ch->level < L_CON ) ) )
        return;

/*
    bug("Attempting relquest",0);
    if (rel_quest_gen(ch, ALIGN_GOOD, 3))
    {
       bug("Successful",0);
    } else
    {
       bug("Failed",0);
    }
*/
    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char(AT_WHITE, "Syntax: imtlset <victim> +|- <immortal skill>\n\r", ch );
        send_to_char(AT_WHITE, "or:     imtlset <victim> +|- all\n\r",              ch );
        send_to_char(AT_WHITE, "or:     imtlset <victim>\n\r",                      ch );
        return;
    }

    if ( !( victim = get_char_world( rch, arg1 ) ) )
    {
        send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch );
        return;
    }

    if ( argument[0] == '+' || argument[0] == '-' )
    {
        buf[0] = '\0';
        smash_tilde( argument );
        if ( argument[0] == '+' )
        {
                argument++;
                if ( !str_cmp( "all", argument ) )
                /*
                 * Imtlset <victim> + all by:
                 * Canth (canth@xs4all.nl)
                 */
                {
                        for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
                        {
                                if ( cmd_table[cmd].level > get_trust( rch ) )
                                     continue;
                                if ( cmd_table[cmd].level <= victim->level && cmd_table[cmd].level >= LEVEL_HERO )
                                {
                                        strcat(buf, cmd_table[cmd].name);
                                        strcat(buf, " ");
                                }
                        }
                }
                else
                {
                        if ( victim->pcdata->immskll )
                                strcat( buf, victim->pcdata->immskll );
                        while ( isspace( *argument ) )
                                argument++;

                        for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
                        {
                                if ( cmd_table[cmd].level > get_trust( rch ) )
                                        continue;
                                if ( !str_cmp( argument, cmd_table[cmd].name ) )
                                        break;
                        }
                        if ( cmd_table[cmd].name[0] == '\0' )
                        {
                                send_to_char(AT_WHITE,"That is not an immskill.\n\r",ch);
                                return;
                        }
                        if ( !str_infix( argument, victim->pcdata->immskll ) )
                        {
                                send_to_char(AT_WHITE,"That skill has already been set.\n\r",ch);
                                return;
                        }
                strcat( buf, argument );
                strcat( buf, " " );
                }
        }

        if ( argument[0] == '-' )
        {
            argument++;
            one_argument( argument, arg1 );
            if ( !str_cmp( "all", arg1 ) )
            {
                free_string( victim->pcdata->immskll );
                victim->pcdata->immskll = str_dup( "" );
                send_to_char(AT_WHITE, "All immskills have been deleted.\n\r", ch );
                return;
            }
            else if ( arg1 )
            {
                /*
                 * Cool great imtlset <victim> - <skill> code...
                 * Idea from Canth (canth@xs4all.nl)
                 * Code by Vego (v942429@si.hhs.nl)
                 * Still needs memory improvements.... (I think)
                 */
                buf2 = str_dup( victim->pcdata->immskll );
                buf3 = buf2;
                if ( (skill = strstr( buf2, arg1 ) ) == NULL )
                {
                    send_to_char(AT_WHITE, "That person doesn't have that immskill", ch);
                    return;
                }
                else
                {
                    while ( buf2 != skill )
                        buf[i++] = *(buf2++);
                    while ( !isspace ( *(buf2++) ) );
                    buf[i] = '\0';
                    strcat ( buf, buf2 );
                }
            }
            else
            {
                send_to_char (AT_WHITE, "That's not an immskill\n\r", ch );
                return;
            }
        }

        free_string( buf3 );
        skill = buf2 = buf3 = NULL;
        free_string( victim->pcdata->immskll );
        victim->pcdata->immskll = str_dup( buf );
    }

    sprintf( buf, "Immortal skills set for %s:\n\r", victim->name );
    send_to_char(AT_WHITE, buf, ch );
    buf1[0] = '\0';
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].level < LEVEL_HERO
            || str_infix( cmd_table[cmd].name, victim->pcdata->immskll ) )
             continue;

        sprintf( buf, "%-10s", cmd_table[cmd].name );
        strcat( buf1, buf );
        if ( ++col % 8 == 0 )
            strcat( buf1, "\n\r" );
    }

    if ( col % 8 != 0 )
        strcat( buf1, "\n\r" );
    send_to_char(AT_WHITE, buf1, ch );

    return;

}
void change_area_status(CHAR_DATA * ch, char * argument)
{
	int	i;
	char arg1 [MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg1);

	if (!authorized(ch, "astatus"))
	{
		send_to_char(C_DEFAULT, "Huh?\n\r", ch);
		return;
	}
	if (!strcmp(arg1, "enable"))
	{
		if (atoi(argument) < 1 || atoi(argument) > 31)
		{
			send_to_char(C_DEFAULT, "Syntax: astatus <enable|disable> <state>\n\r", ch);
			send_to_char(C_DEFAULT, "where  1<= state <= 31\n\r", ch);
			return;
		}
		ch->in_room->area->status |= (1 <<(atoi(argument) -1));
		sprintf(arg1, "State %d enabled for current area\n\r", atoi(argument));
		send_to_char(C_DEFAULT, arg1, ch);
		return;
	}
	else if (!strcmp(arg1, "disable"))
	{
		if (!isdigit(*argument))
		{
			send_to_char(C_DEFAULT, "Syntax: astatus <enable|disable> <state>\n\r", ch);
			send_to_char(C_DEFAULT, "where 1<= state <= 31\n\r", ch);
			return;
		}
		ch->in_room->area->status &= (~(1<<(atoi(argument) -1)));
		sprintf(arg1, "State %d disabled for current area.\n\r", atoi(argument));
		send_to_char(C_DEFAULT, arg1, ch);
		return;
	}
	else if (arg1[0] == '\0')
	{
		send_to_char(C_DEFAULT, "The following states are enabled for the current area:\n\r", ch);
	for (i = 1; i < 32; i++)
	{
		if ( ch->in_room->area->status & (1<<(i -1)))
		{
			sprintf(arg1, " %d", i);
			send_to_char(C_DEFAULT, arg1, ch);
		}
	}
	return;
	}
	else
	{
		send_to_char(C_DEFAULT, "Syntax: astatus <enable|disable> <state>\n\r", ch);
		send_to_char(C_DEFAULT, "Syntax: astatus\n\r", ch);
		send_to_char(C_DEFAULT, "With no arguments, shows current states.\n\r", ch);
		return;
	}

	return;
}
void print_dead_obj_tree(CHAR_DATA * ch, DEAD_OBJ_DATA * node )
{
    char buf[100];

  if (!node)
    return;
  print_dead_obj_tree(ch, node->left);
  sprintf(buf, "%d - %d\n\r ", node->low_vnum, node->high_vnum );
  send_to_char(AT_WHITE, buf, ch);
  print_dead_obj_tree(ch, node->right);
}
void do_dostat(CHAR_DATA * ch, char * arg)
{
    char 	buf[MAX_STRING_LENGTH];
    DEAD_OBJ_LIST   *list = dead_object_list;
    if (!authorized(ch, "dostat"))
    {
	return;
    }
    sprintf( buf, "Dead Objects Marked for Deletion\n\r");
    send_to_char(AT_WHITE, buf, ch);

    for( ; list; list = list->next )
    {
	sprintf(buf, "Update version: %d\n\r", list->update);
	send_to_char(AT_WHITE, buf, ch);
	print_dead_obj_tree(ch, list->head);
    }
    return;
}

void do_cleanstat(CHAR_DATA *ch, char *args)
{
    CHAR_DATA * victim;

    if (args[0] != '\0')
    {
	if (get_trust(ch) > LEVEL_DEMIGOD)
	{
	    if (!(victim = get_char_world(ch, args)))
	    send_to_char(AT_YELLOW, " Syntax: cleanstat <player>\n\r", ch);
	}
	else
	{
	    send_to_char(AT_YELLOW, "Only Immortals can cleanstat others\n\r", ch);
	    return;
	}
    }
    else victim = ch;
    DeadObjPrntOnly = TRUE;
    if (IS_NPC(victim))
        return;
    clean_player_objects(victim);
    DeadObjPrntOnly = FALSE;
}


void do_resetxp(CHAR_DATA* ch, char* argument)
{
	int vnum					= 0;
	char arg[MAX_INPUT_LENGTH]	= "";
	char buf[MAX_STRING_LENGTH] = "";
	MOB_INDEX_DATA* mob			= NULL;

	if (ch) // allow auto resets
	{
		if (!IS_IMMORTAL(ch))
		{
			send_to_char(C_DEFAULT, "Huh?\n\r",ch);
			return;
		}

		if (!authorized(ch, "resetxp"))
		{
			return;
		}

	}

	if (argument[0] != '\0')
	{
		// Reset kill counters
		do
		{
			argument = one_argument(argument, arg);
			if (!str_cmp(arg,"all"))
			{
				for (vnum = 0; vnum < 0x7FFF; vnum++)
				{
					mob = get_mob_index(vnum);
					if (mob)
						if (mob->killed > 0)
							mob->killed = 0;
				}
				strcpy(buf, "resetxp: All mobiles reset!\n\r");
				break;
			} else
			{
				if (is_number(arg))
				{
					vnum = atoi(arg);
					mob = get_mob_index(vnum);
					if (mob)
						if (mob->killed > 0)
							mob->killed = 0;
				}
				if (buf[0]=='\0')
				{
					strcpy(buf,"resetxp: The specified mobiles have been reset (if found).\n\r");
				}
			}
		} while (argument[0] != '\0');

		// Reset kill table
		for( vnum = 0; vnum < (MAX_LEVEL - 1); vnum++)
		{
			kill_table[vnum].killed = 0;
		}

		if (ch)
		{
			send_to_char(AT_WHITE, buf, ch);

			strcpy(buf,"You feel the power of the gods flow over the land renewing life!\n\r");
		} else
		{
			strcpy(buf, "You feel refreshed as the sun rises upon a new month!\n\r");
		}

		send_to_all_char(buf);

	} else
	{
		send_to_char(AT_WHITE, "Usage: \"resetxp all\" or \"resetxp vnum1 [vnum2] [vnum3] ...\"", ch);
	}
}

// putmeontop:
// put a VERY trusted imm at the top of the descriptor list
// and henceforth first on WHO.
// This is purely for fun, it has no real need
// - ahsile
void do_top(CHAR_DATA* ch, char* argument)
{
	DESCRIPTOR_DATA* d;
	DESCRIPTOR_DATA* last;

	if (!IS_IMMORTAL(ch))
        {
                send_to_char(C_DEFAULT, "Huh?\n\r",ch);
                return;
        }

        if (!authorized(ch, "top"))
        {
                return;
        }

	if (ch->desc->next)
	{
		if (ch->desc == descriptor_list)
			descriptor_list = ch->desc->next;

		// may as well start from this one instead of the beginning
		last = ch->desc->next;
		for (d=descriptor_list; d ; d = d->next)
		{
			if (d->next == ch->desc)
			{
				d->next = last;
				break;
			}
		}
	} else
	{
		// you're already on top!
		return;
	}

	for (d=last; d; d=d->next)
	{
		if (!d->next)
		{
			d->next = ch->desc;
			ch->desc->next = NULL;
		}
	}

	send_to_char(AT_WHITE, "You are now on top!\n\r",ch);

	return;

}


void do_realmemory(CHAR_DATA* ch, char* argument)
{
	char buf[MAX_STRING_LENGTH];
	//sMStats stat;

	if (!IS_IMMORTAL(ch))
	{
		send_to_char(C_DEFAULT, "Huh?\n\r",ch);
		return;
	}

	if (!authorized(ch, "realmemory"))
	{
		send_to_char(C_DEFAULT,"Not Authorized!\n\r",ch);
		return;
	}
        /*
	stat = m_getMemoryStatistics();

	sprintf(buf, "Real Memory Statistics:\n"); send_to_char(C_DEFAULT, buf, ch);
	sprintf(buf, "\tCurrent Reported Mem.... %u KB\n", stat.totalReportedMemory); send_to_char(C_DEFAULT, buf, ch);
        sprintf(buf, "\tCurrent Actual Mem...... %u KB\n", stat.totalActualMemory); send_to_char(C_DEFAULT, buf, ch);
	sprintf(buf, "\tPeak Reported Mem....... %u KB\n", stat.peakReportedMemory); send_to_char(C_DEFAULT, buf, ch);
	sprintf(buf, "\tPeak Actual Mem......... %u KB\n", stat.peakActualMemory); send_to_char(C_DEFAULT, buf, ch);
	sprintf(buf, "\tAccumulated Reported Mem %u KB\n", stat.accumulatedReportedMemory); send_to_char(C_DEFAULT, buf, ch);
	sprintf(buf, "\tAccumulated Actual Mem.. %u KB\n", stat.accumulatedActualMemory); send_to_char(C_DEFAULT, buf, ch);
	sprintf(buf, "\tCurrent Allocations..... %u\n", stat.totalAllocUnitCount); send_to_char(C_DEFAULT, buf, ch);
	sprintf(buf, "\tPeak Allocations........ %u\n", stat.peakAllocUnitCount); send_to_char(C_DEFAULT, buf, ch);
	sprintf(buf, "\tAccumulated Allocations. %u\n\r", stat.accumulatedAllocUnitCount); send_to_char(C_DEFAULT, buf, ch);
        */
}

void do_showcorrupt(CHAR_DATA* ch, char* argument)
{
        char buf[MAX_STRING_LENGTH];
	CORRUPT_AREA_LIST* cal = NULL;

        if (!IS_IMMORTAL(ch))
        {
                send_to_char(C_DEFAULT, "Huh?\n\r",ch);
                return;
        }

	send_to_char(C_DEFAULT, "Area\t\tObject VNUM\n\r", ch);
	if (!corrupt)
	{
		send_to_char(C_DEFAULT, "None!\n\r", ch);
	} else
	{
		for (cal=corrupt; cal; cal = cal->next)
		{
			sprintf(buf, "%s\t%d\n\r", cal->area->filename, cal->vnum);
			send_to_char(C_DEFAULT, buf, ch);
		}
	}
}
