/*****************************************************************************
 * Chatmode.c  - Merc-based confrence system.                                *
 *                                                                           *
 * Full chat-mode setup for Merc based MUDS, or could be used separately if  *
 * you cut and pasted the needed parts from Merc code.                       *
 * -- Altrag Dalosein, Lord of the Dragons..                                 *
 *****************************************************************************/
/*$Id*/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"


/*
 * Externals
 */
bool  check_social    args( ( CHAR_DATA *ch, char *command, char *argument ) );
extern int port;

/*
 * Locals
 */
void    start_chat_mode     args( ( DESCRIPTOR_DATA *d ) );
void    stop_chat_mode      args( ( CHAR_DATA *ch ) );
void    chat_interp         args( ( CHAR_DATA *ch, char *argument ) );
void    chat_command        args( ( CHAR_DATA *ch, char command,
				    char *argument ) );
#define CD CHAR_DATA *
CD      get_char_chat       args( ( CHAR_DATA *ch, char *argument ) );
#undef CD
void    send_room_stuff     args( ( CHAR_DATA *ch ) );
void    init_chat           args( ( void ) );
int     num_color           args( ( CHAR_DATA *ch ) );
char *  get_color           args( ( CHAR_DATA *ch ) );

struct chat_room
{
  struct chat_room *next;
  ROOM_INDEX_DATA *pRoom;
  char *invited;
};

void dispose_room( struct chat_room *room );

/*
 * Use pre-defined stuff, even though all parts aren't used for chat.
 * This eliminates the need for new functinos that are copies of old
 * functions, such as ACT or CHECK_SOCIAL.
 * -- Altrag
 */
struct chat_room *chat_rooms;
struct chat_room *last_chat_room;
CHAR_DATA *chat_list;
CHAR_DATA *old_chars;

void start_chat_mode( DESCRIPTOR_DATA *d )
{
  CHAR_DATA *ch;

  if ( !d || !d->character )
    return;

  if ( !chat_rooms )
    init_chat( );

  if ( d->original )
    do_return( d->character, "" );

  if ( d->pEdit )
  {
    d->pEdit = NULL;
    d->editor = 0;
  }

  if ( d->inEdit )
  {
    d->inEdit = NULL;
    d->editin = 0;
  }

  if ( d->character == char_list )
    char_list = char_list->next;
  else
  {
    for ( ch = char_list; ch; ch = ch->next )
      if ( ch->next == d->character )
	break;
    if ( ch )
      ch->next = d->character->next;
  }

  if ( d->character->was_in_room )
  {
    if ( !d->character->in_room )
      char_to_room( d->character, d->character->was_in_room );
  }

  if ( d->character->in_room )
  {
    d->character->was_in_room = d->character->in_room;
    char_from_room( d->character );
  }

  d->original = d->character;

  /*
   * These are the freaks who go link_dead (and lose their descriptor)
   * from inside chat mode.
   * -- Altrag
   */
  d->character->next = old_chars;
  old_chars = d->character;

  ch = alloc_mem( sizeof( *ch ) );
/*  *ch = *d->character;*/
  ch->desc = d;
  ch->name = str_dup( d->character->name );
  ch->act = d->character->act;
  ch->sex = d->character->sex;
  ch->position = POS_STANDING;
  ch->hit = 1;
  ch->max_hit = 1;

  ch->next = chat_list;
  chat_list = ch;

/*  char_to_room( ch, chat_rooms->pRoom );*/

  /*
   * These are still stored on d->original.  We don't need them here.
   * This will also stop any funky residue from the game.
   * -- Altrag
   */
/*  ch->reply = NULL;
  ch->affected = NULL;
  ch->affected2 = NULL;
  ch->carrying = NULL;
  ch->master = NULL;
  ch->leader = NULL;
  ch->fighting = NULL;
  ch->hunting = NULL;
  ch->gspell = NULL;
  ch->pcdata = NULL;
  ch->phobia = NULL;
  ch->in_room = NULL;
  ch->was_in_room = NULL;*/

  d->character = ch;
  d->connected = CON_CHATTING;
  char_to_room(ch, chat_rooms->pRoom);
  act(num_color(ch), "$n &Yhas entered the room.", ch, NULL, NULL, TO_ROOM);
  send_room_stuff( ch );
  return;
}

void stop_chat_mode( CHAR_DATA *ch )
{
  DESCRIPTOR_DATA *d;
  struct chat_room *room;

  d = ch->desc;

  act(num_color(ch), "$n &Yhas left the room.", ch, NULL, NULL, TO_ROOM);
  char_from_room( ch );

  if ( ch == chat_list )
    chat_list = ch->next;
  else
  {
    CHAR_DATA *gch;

    for ( gch = chat_list; gch; gch = gch->next )
      if ( gch->next == ch )
	break;
    if ( gch )
      gch->next = ch->next;
  }

  for ( room = chat_rooms; room; room = room->next )
    if (!str_prefix( ch->name, room->pRoom->name ) ||
	 is_name( ch->name, room->pRoom->name ))
      break;
  if ( room )
  {
    if ( room == chat_rooms )
      chat_rooms = room->next;
    else
    {
      struct chat_room *rprev;
	
      for ( rprev = chat_rooms; rprev; rprev = rprev->next )
	if ( rprev->next == room )
	  break;
      if ( rprev )
	rprev->next = room->next;
      if ( !rprev->next )
	last_chat_room = rprev;
    }
    dispose_room(room);
  }

/*  save_char_chat( ch );*/
  free_ch( ch );

  if ( !d )
  {
    return;
  }

  if ( !d->original )
  {
    close_socket( d );
    return;
  }

  d->character = d->original;
  d->original = NULL;

  if ( d->character == old_chars )
    old_chars = old_chars->next;
  else
  {
    CHAR_DATA *gch;

    for ( gch = old_chars; gch; gch = gch->next )
      if ( gch->next == d->character )
	break;
    if ( gch )
      gch->next = d->character->next;
  }

  d->character->next = char_list;
  char_list = d->character;

  if ( d->character->was_in_room )
    char_to_room(d->character, d->character->was_in_room);
  else
    char_to_room(d->character, get_room_index(ROOM_VNUM_TEMPLE) );

  d->character->was_in_room = NULL;
  d->connected = CON_PLAYING;
  do_look(d->character, "");

  return;
}

void chat_interp( CHAR_DATA *ch, char *argument )
{
  char command = 0;
  char arg[MAX_STRING_LENGTH];

  while ( isspace(*argument) )
    ++argument;

  if ( !*argument )
  {
    send_room_stuff( ch );
    return;
  }

  if ( *argument == '/' )
  {
    argument++;
    while (isspace(*argument))
      ++argument;
    command = *argument;
    argument++;
    while (isspace(*argument))
      ++argument;
  }

  if ( command )
  {
    chat_command( ch, command, argument );
    return;
  }

  argument = one_argument( argument, arg );

  if ( arg[0] == '.' && check_social( ch, arg + 1, argument ) )
    return;

  act( num_color(ch), "$n: &G$t $T", ch, arg, argument, TO_ROOM );
  send_to_char( AT_RED, "-- &CMessage sent &R--\n\r",ch);
  return;
}

void chat_command( CHAR_DATA *ch, char command, char *argument )
{
  char arg[MAX_STRING_LENGTH];
  CHAR_DATA *victim = NULL;
  struct chat_room *room = NULL;
  int rcount = 0;

  arg[0] = '\0';

  switch( UPPER(command) )
  {
  case 'Q':
    stop_chat_mode( ch );
    return;
  case 'P':
    argument = one_argument(argument, arg);
    while( isspace(*argument) )
      argument++;
    if ( !*argument )
    {
      send_to_char(C_DEFAULT, "Send what message?\n\r",ch);
      return;
    }
    if ( !(victim = get_char_chat(ch, arg)) )
    {
      send_to_char(C_DEFAULT, "They aren't here.\n\r",ch);
      return;
    }
    sprintf(arg, "(&RPRIV&C)&%s$n: &Y$t", get_color(ch) );
    act(AT_LBLUE, arg, ch, argument, victim,
       (ch == victim ? TO_CHAR : TO_VICT));
    act(AT_RED,"-- &CMessage sent only to &$t$N &R--", ch, get_color(ch),
	victim,	TO_CHAR);
    return;
  case 'J':
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' )
    {
      bool ToMain = FALSE;

      if ( !str_prefix( ch->name, ch->in_room->name ) || 
	   is_name( ch->name, ch->in_room->name ) )
	ToMain = TRUE;
      act( num_color(ch), "$n &Yhas left the room.", ch, NULL, NULL, TO_ROOM);
      char_from_room(ch);
      if ( ToMain )
      {
	char_to_room( ch, chat_rooms->pRoom );
	send_room_stuff(ch);
	act(num_color(ch), "$n &Yhas entered the room.", ch, NULL, NULL,
	    TO_ROOM);
	return;
      }
      for ( room = chat_rooms; room; room = room->next )
      {
	if ( is_name( ch->name, room->pRoom->name ) )
	{
	  char_to_room(ch, room->pRoom);
	  send_room_stuff(ch);
	  act( num_color(ch), "$n &Yhas entered the room.", ch, NULL, NULL,
	       TO_ROOM);
	  return;
	}
      }
      room = alloc_mem( sizeof( *room ) );
      room->invited = str_dup("");
      room->pRoom = alloc_mem( sizeof( *room->pRoom ) );
      room->pRoom->name = str_dup( ch->name );
      sprintf( arg, "%s'%s room.", ch->name,
	      (ch->name[strlen(ch->name)-1] == 's' ? "" : "s" ));
      room->pRoom->description = str_dup( arg );
      room->pRoom->people = NULL;
      last_chat_room->next = room;
      last_chat_room = room;
      char_to_room(ch, room->pRoom);
      send_to_char(AT_RED, "!! &CRoom Created &R!!\n\r", ch);
      send_room_stuff(ch);
      return;
    }
    if ( !str_prefix( arg, ch->name ) || is_name( arg, ch->name ) ||
	 !str_cmp( arg, "self" ) )
    {
      chat_command( ch, 'j', "" );
      return;
    }
    for ( room = chat_rooms; room; room = room->next )
      if ( !str_prefix( arg, room->pRoom->name ) ||
	    is_name( arg, room->pRoom->name ) )
      {
	act( num_color(ch), "$n &Yhas left the room.",ch, NULL, NULL, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, room->pRoom);
	send_room_stuff(ch);
	act( num_color(ch), "$n &Yhas entered the room.",ch,NULL,NULL,TO_ROOM);
	return;
      }
    send_to_char(AT_RED, "!! &WRoom does not exist &R!!\n\r",ch);
    return;
  case 'R':
    for ( room = chat_rooms; room; room = room->next )
    {
      sprintf( arg+strlen(arg), "%-16s&R%s&C\n\r", room->pRoom->name,
	       room->pRoom->description );
      rcount++;
    }
    sprintf(arg+strlen(arg), "There %s %d active room%s.\n\r",
	   (rcount == 1 ? "is" : "are"),
	    rcount,
	   (rcount == 1 ? "s" : ""));
    send_to_char( AT_LBLUE, arg, ch );
    return;
  case 'T':
    for ( room = chat_rooms; room; room = room->next )
      if ( room->pRoom == ch->in_room )
	break;
    if ( !room )
      return;
    if ( !is_name( ch->name, room->pRoom->name ) )
    {
      send_to_char(AT_GREEN, "You are not in your room.\n\r",ch);
      return;
    }
    free_string(room->pRoom->description);
    room->pRoom->description = str_dup( argument );
    act(num_color(ch), "$n has changed the topic to '$t'.",ch,
	argument, NULL, TO_ROOM );
    return;
  case 'W':
    for ( victim = chat_list; victim; victim = victim->next )
    {
      sprintf(arg+strlen(arg), "&%s%-16s&R%-13s&P%s&C\n\r", get_color(victim),
	      victim->name, victim->in_room->name,
	      victim->in_room->description);
      rcount++;
    }
    sprintf(arg+strlen(arg), "There %s %d %s in the conference.\n\r",
	   (rcount == 1 ? "is" : "are"),
	    rcount,
	   (rcount == 1 ? "person" : "people"));
    send_to_char(AT_CYAN, arg, ch);
    return;
  case 'H':
  case '?':
    send_to_char(AT_LBLUE, "/h     &GThis help screen\n\r", ch);
    send_to_char(AT_LBLUE, "/?     &GSame as /h\n\r",ch);
    send_to_char(AT_LBLUE, "/j[]   &GJoin a channel\n\r", ch);
    send_to_char(AT_LBLUE, "/p<>   &GSend a private message\n\r",ch);
    send_to_char(AT_LBLUE, "/<>    &GSame as /p\n\r",ch);
    send_to_char(AT_LBLUE, "/q     &GQuit the conference\n\r",ch);
    send_to_char(AT_LBLUE, "/r     &GList active rooms\n\r",ch);
    send_to_char(AT_LBLUE, "/t()   &GSet room topic\n\r",ch);
    send_to_char(AT_LBLUE, "/w     &GList people in conference\n\r\n\r",ch);
    send_to_char(AT_LBLUE, "[] &P= channel name; use /r for list\n\r",ch);
    send_to_char(AT_LBLUE, "<> &P= user name; use /w for list\n\r",ch);
    send_to_char(AT_LBLUE, "() &P= any character string\n\r",ch);
    return;
  default:
/*    sprintf(arg, "$n: &G/%c$t", command);
    act( num_color(ch), arg, ch, argument, NULL, TO_ROOM );
    send_to_char( AT_RED, "-- &CMessage sent &R--\n\r",ch);*/
    sprintf( arg, "%c%s", command, argument );
    chat_command( ch, 'p', arg );
    return;
  }
  return;
}

CHAR_DATA *get_char_chat( CHAR_DATA *ch, char *name )
{
  CHAR_DATA *vch;

  if ( !str_prefix( name, ch->name ) || is_name( name, ch->name ) )
    return ch;

  for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
    if ( !str_prefix( name, vch->name ) || is_name( name, vch->name ) )
      return vch;

  for ( vch = chat_list; vch; vch = vch->next )
    if ( !str_prefix( name, vch->name ) || is_name( name, vch->name ) )
      return vch;

  return NULL;
}

void send_room_stuff( CHAR_DATA *ch )
{
  int width = 0;
  CHAR_DATA *vch;
  if ( !ch->in_room )
    return;

  send_to_char( AT_WHITE, ch->in_room->name, ch );
  send_to_char( C_DEFAULT, "\n\r", ch );
  if ( ch->in_room->description[0] != '\0' )
    send_to_char(AT_YELLOW, ch->in_room->description, ch );
  send_to_char( C_DEFAULT, "\n\r", ch );
  for ( vch = ch->in_room->people; vch; vch = vch->next_in_room )
  {
    width += strlen( vch->name );
    if ( width >= 79 )
    {
      send_to_char(C_DEFAULT, "\n\r", ch );
      width = 0;
    }
    send_to_char(num_color(vch), vch->name, ch );
    send_to_char(C_DEFAULT, " ", ch );
  }
  send_to_char(C_DEFAULT, "\n\r\n\r", ch );
  return;
}

void do_conference( CHAR_DATA *ch, char *argument )
{
  if ( IS_NPC(ch) && (!ch->desc || !ch->desc->original ) )
    return;
  if ( ch->fighting || ch->position == POS_FIGHTING )
  {
    send_to_char( AT_WHITE, "No way!  You are fighting!.\n\r",ch);
    return;
  }
  if ( ch->combat_timer )
  {
    send_to_char(AT_WHITE, "You can't right now.\n\r",ch);
    return;
  }
  if ( ch->position < POS_STUNNED && ch->level < L_APP )
  {
    send_to_char(AT_WHITE, "You're not DEAD yet!\n\r",ch);
    return;
  }
  if ( IS_SET( ch->act, PLR_QUESTOR ) )
    REMOVE_BIT( ch->act, PLR_QUESTOR );
  ch->hunting = NULL;
  save_char_obj( ch, FALSE );
  start_chat_mode( ch->desc );
}

void init_chat( void )
{
  if ( chat_rooms )
    return;

  chat_rooms = alloc_mem( sizeof( *chat_rooms ) );
  chat_rooms->pRoom = alloc_mem(sizeof(*chat_rooms->pRoom));
  chat_rooms->invited = str_dup("");
  chat_rooms->pRoom->name = str_dup( "Main" );
  chat_rooms->pRoom->description = str_dup( "&GEye of the &BS&Ct&Wo&Cr&Bm "
					    "&GMain teleconference channel" );
  last_chat_room = chat_rooms;
  return;
}

int num_color( CHAR_DATA *ch )
{
  switch ( ch->sex )
  {
  case SEX_NEUTRAL:
    return AT_GREEN;
  case SEX_MALE:
    return AT_BLUE;
  case SEX_FEMALE:
    return AT_RED;
  }
  return C_DEFAULT;
}

char *get_color( CHAR_DATA *ch )
{
  switch ( ch->sex )
  {
  case SEX_NEUTRAL:
    return "G";
  case SEX_MALE:
    return "B";
  case SEX_FEMALE:
    return "R";
  }
  return "w";
}

/*
 * Update the chat_list and the old_chars lists.  Just kicks out linkdeads
 * if this becomes too much lag, you could just make a modified version
 * of check_reconnect in comm.c to check those lists as well as the char_list
 * list.  -- Altrag
 */
void chat_update( void )
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;

  for ( ch = old_chars; ch; ch = ch_next )
  {
    ch_next = ch->next;

    if ( !ch->desc )  /* i.e. They went into chat but dropped link */
    {
      if ( ch == old_chars )
	old_chars = ch->next;
      else
      {
	CHAR_DATA *och;

	for ( och = old_chars; och; och = och->next )
	  if ( och->next == ch )
	    break;
	if ( och )
	  och->next = ch->next;
      }
      ch->next = char_list;
      char_list = ch;
      if ( ch->was_in_room )
	char_to_room( ch, ch->was_in_room );
    }
  }
  for ( ch = chat_list; ch; ch = ch_next )
  {
    ch_next = ch->next;
    if ( !ch->desc )
    {
      stop_chat_mode( ch );
    }
  }
  return;
}

void dispose_room( struct chat_room *room )
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;

  for ( ch = room->pRoom->people; ch; ch = ch_next )
  {
    ch_next = ch->next_in_room;
    chat_command(ch, 'j', "Main");
  }
  free_string(room->pRoom->name);
  free_string(room->pRoom->description);
  free_mem(room->pRoom, sizeof(*room->pRoom));
  free_string(room->invited);
  free_mem(room, sizeof(*room));
  return;
}
