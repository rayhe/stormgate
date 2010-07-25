/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael         *
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
/* $Id: act_comm.c,v 1.27 2005/02/10 00:19:37 ahsile Exp $ */
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

#ifdef RUN_AS_WIN32SERVICE
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "merc.h"



/* Auction variables */
OBJ_DATA *auc_obj;
CHAR_DATA *auc_bid;
int auc_cost;
int auc_count = -1;
CHAR_DATA *auc_held;

/* Auction semi-local */
void auc_channel    args( ( char *auction ) );

/*
 * Local functions.
 */
char    *initial        args( ( const char *str ) );
bool	is_note_to	args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );
void	note_attach	args( ( CHAR_DATA *ch ) );
void	note_remove	args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );
void	talk_channel	args( ( CHAR_DATA *ch, char *argument,
			    int channel, const char *verb, int language ) );
/* Sigh.. this is really a slow way of purging notes..
 * but efficiency isnt as useful if it doesnt work anyways.. -- Altrag */
void    note_delete     args( ( NOTE_DATA *pnote ) );
/* More note stuff from the Alt man.. :).. -- Altrag */
bool    check_note_room args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );


/* RT code to delete yourself */

void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char(AT_WHITE, "You must type the full command to delete yourself.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, char *argument)
{
        char strsave[MAX_INPUT_LENGTH];
        char buf[MAX_STRING_LENGTH];
#if defined (RUN_AS_WIN32SERVICE)
		char tmp[MAX_STRING_LENGTH];
#endif
        RELIGION_DATA *pReligion;
 
        pReligion = get_religion_index( ch->religion );

        buf[0] = '\0';

        if (IS_NPC(ch))
                return;
    
        if(ch->combat_timer)
        {
	    send_to_char(AT_WHITE, "Your adrenaline is pumping too hard.\n\r", ch);
	    return;
        }

	if(ch->fighting)
	{
	    send_to_char(AT_WHITE, "You can't delete yourself while fighting.\n\r", ch);
	    return;
	}

	if(ch->clan != 0 )
	{
	    send_to_char(AT_WHITE, "You can't delete yourself if you are in a clan.\n\r", ch );
	    return;
	}

        if (ch->pcdata->confirm_delete)
        {
                if (argument[0] != '\0')
                {
                        send_to_char(AT_WHITE, "Delete status removed.\n\r",ch);
                        ch->pcdata->confirm_delete = FALSE;
                        sprintf (buf, "%s has decided not to self-delete.", ch->name );
			log_string(buf, CHANNEL_INFO, -1 );
                        return;
                }
                else
                {
			pReligion->members--;
                        sprintf (buf, "%s has been turned into line noise.", ch->name );
			log_string(buf, CHANNEL_INFO, -1 );
			sprintf (buf, "%s has deleted in room %d.", ch->name, ch->in_room->vnum );
			log_string(buf, CHANNEL_LOG, -1 );

#if !defined( macintosh ) && !defined( MSDOS )

	#if defined( RUN_AS_WIN32SERVICE )
			_getcwd(tmp, MAX_STRING_LENGTH);
			//getwd(tmp);
			strcat(tmp,"/");
			strcat(tmp,PLAYER_DIR);   
			sprintf( strsave, "%s%s%s%s", tmp, initial( ch->name ), "/", capitalize( ch->name ) );
			sprintf(buf, "%s%s%s%s.cps", tmp,
			    initial( ch->name ), "/", capitalize( ch->name ) );
			remove( buf );
	#else
			sprintf( strsave, "%s%s%s%s", PLAYER_DIR,
				initial( ch->name ), "/", capitalize( ch->name ) );
            sprintf(buf, "rm %s%s%s%s.cps", PLAYER_DIR,
                      initial( ch->name ), "/", capitalize( ch->name ) );
			system( buf );
	#endif		
			
#else
                        sprintf( strsave, "%s%s", PLAYER_DIR,
                        capitalize( ch->name ) );
#endif
			ch->level =1;
			ch->exp = 0;
                        do_quit(ch,"");
			remove(strsave);
                        return;
                }
        }

        if (argument[0] != '\0')
        {
                send_to_char(AT_WHITE, "Just type delete. No argument.\n\r",ch);
                return;
        }

        send_to_char(AT_WHITE, "Type delete again to confirm this command.\n\r",ch);
        send_to_char(AT_WHITE, "WARNING: this command is irreversible.\n\r",ch);
        send_to_char(AT_WHITE, "Typing delete with an argument will undo.\n\r", ch);
        ch->pcdata->confirm_delete = TRUE;

        sprintf (buf, "%s is contemplating deletion.\n\r", ch->name );
}


void note_delete( NOTE_DATA *pnote )
{
  NOTE_DATA *prev;

  if (pnote == note_list )
    note_list = pnote->next;
  else
  {
    for ( prev = note_list; prev; prev = prev->next )
    {
      if ( prev->next == pnote )
	break;
    }
    if ( !prev )
    {
      bug( "Note_delete: no note.", 0 );
      return;
    }
    prev->next = pnote->next;
  }
/*  pnote->next = note_free;
  note_free = pnote;*/
	free_note(pnote);
}

/*
 * Get rid of old notes
 * -- Altrag
 */
void note_cleanup( void )
{
  NOTE_DATA *pnote;
  NOTE_DATA *pnote_next;
  FILE *fp;

  for ( pnote = note_list;    /* 60s*60m*24h*7d -- Altrag */
        pnote && pnote->date_stamp + 604800 < current_time;
        pnote = pnote_next )
  {
    pnote_next = pnote->next;
    if ( pnote->protected )
      continue;
    note_delete( pnote );
  }
  fclose( fpReserve );
  if ( !( fp = fopen( NOTE_FILE, "w" ) ) )
  {
    perror( NOTE_FILE );
  }
  else
  {
    for ( pnote = note_list; pnote; pnote = pnote->next )
    {
      fprintf( fp, "Sender  %s~\n", pnote->sender );
      fprintf( fp, "Date    %s~\n", pnote->date );
      fprintf( fp, "Stamp   %ld\n", pnote->date_stamp );
      fprintf( fp, "To      %s~\n", pnote->to_list );
      fprintf( fp, "Subject %s~\n", pnote->subject );
      fprintf( fp, "Protect %d\n",  pnote->protected );
      fprintf( fp, "Board   %d\n",  pnote->on_board );
      fprintf( fp, "Text\n%s~\n\n", pnote->text );
    }
    fclose( fp );
  }
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}
    
bool check_note_room( CHAR_DATA *ch, NOTE_DATA *pnote )
{
  OBJ_DATA *pObj;

  if ( !ch->in_room )
    return (pnote->on_board == 0);

  for ( pObj = ch->in_room->contents; pObj; pObj = pObj->next_content )
    if ( pObj->item_type == ITEM_NOTEBOARD )
      break;

  if ( !pObj )
    return (pnote->on_board == 0);

  if ( pnote->on_board != pObj->pIndexData->vnum )
    return FALSE;

  if ( pObj->value[1] > get_trust(ch) )
  {
    OBJ_DATA *decoder;

    for ( decoder = ch->carrying; decoder; decoder = decoder->next_content )
      if ( decoder->pIndexData->vnum == pObj->value[0] )
	break;

    if ( decoder == NULL )
      return FALSE;
  }

  return TRUE;
}

bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    if ( !check_note_room( ch, pnote ) )
        return FALSE;

    if ( !str_cmp( ch->name, pnote->sender ) )
	return TRUE;

    if ( is_name( "all", pnote->to_list ) )
	return TRUE;

    if ( ch->clan == 1 && (   is_name("DOOM", pnote->to_list )  ) )
        return TRUE;

    if ( ch->clan == 2 && (   is_name("DIVINITY", pnote->to_list ) ) )
        return TRUE;

    if ( ch->clan == 3 && (   is_name("RETRIBUTION", pnote->to_list ) ) )
        return TRUE;

    if ( ch->clan == 4 && (   is_name("STRIFE", pnote->to_list ) ) )
        return TRUE;

    if ( ch->clan == 5 && (   is_name("INSANITY", pnote->to_list ) ) ) 
        return TRUE;

    if ( ch->clan == 6 && (   is_name("NULL", pnote->to_list ) ) )
        return TRUE;

    if ( ch->clan == 7 && (   is_name("NULL", pnote->to_list ) ) )
        return TRUE;

    if ( ch->clan == 8 && (   is_name("INCARNATE", pnote->to_list ) ) )
      return TRUE;

    if ( ( get_trust( ch ) > LEVEL_DEMIGOD ) && ( (   is_name( "immortal",  pnote->to_list )
			   || is_name( "immortals", pnote->to_list )
			   || is_name( "imm",       pnote->to_list )
			   || is_name( "immort",    pnote->to_list ) ) ) )
	
	return TRUE;
    
    if ( ( get_trust( ch ) > L_CON || IS_CODER( ch ) ) &&
    			    is_name( "council", pnote->to_list ) )
    	return TRUE;

    if ( IS_CODER( ch ) && ( is_name( "coder", pnote->to_list )
			  || is_name( "code", pnote->to_list )
			  || is_name( "coders", pnote->to_list ) ) )
	return TRUE;

    if ( ch->guild && ( !str_cmp( ch->guild->name, "EDEN" ) &&
		       is_name( "EDEN", pnote->to_list ) ) )
       	return TRUE;

    if ( is_name( ch->name, pnote->to_list ) )
	return TRUE;

    return FALSE;
}



void note_attach( CHAR_DATA *ch )
{
    NOTE_DATA *pnote;

    if ( ch->pnote )
	return;

    pnote = new_note();

    pnote->sender	= str_dup( ch->name );
    pnote->protected    = FALSE;
    ch->pnote		= pnote;
    return;
}



void note_remove( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    FILE      *fp;
    NOTE_DATA *prev;
    char      *to_list;
    char       to_new [ MAX_INPUT_LENGTH ];
    char       to_one [ MAX_INPUT_LENGTH ];

    /*
     * Build a new to_list.
     * Strip out this recipient.
     */
    to_new[0]	= '\0';
    to_list	= pnote->to_list;
    while ( *to_list != '\0' )
    {
	to_list	= one_argument( to_list, to_one );
	if ( to_one[0] != '\0' && str_cmp( ch->name, to_one ) )
	{
	    strcat( to_new, " "    );
	    strcat( to_new, to_one );
	}
    }

    /*
     * Just a simple recipient removal?
     */
    if ( str_cmp( ch->name, pnote->sender ) && to_new[0] != '\0' && 
	 get_trust(ch) < 105 )
    {
	free_string( pnote->to_list );
	pnote->to_list = str_dup( to_new + 1 );
	return;
    }

    /*
     * Remove note from linked list.
     */
    if ( pnote == note_list )
    {
	note_list = pnote->next;
    }
    else
    {
	for ( prev = note_list; prev; prev = prev->next )
	{
	    if ( prev->next == pnote )
		break;
	}

	if ( !prev )
	{
	    bug( "Note_remove: pnote not found.", 0 );
	    return;
	}

	prev->next = pnote->next;
    }
    free_note (pnote);


    /*
     * Rewrite entire list.
     */
    fclose( fpReserve );
    if ( !( fp = fopen( NOTE_FILE, "w" ) ) )
    {
	perror( NOTE_FILE );
    }
    else
    {
	for ( pnote = note_list; pnote; pnote = pnote->next )
	{
	    fprintf( fp, "Sender  %s~\n", pnote->sender     );
	    fprintf( fp, "Date    %s~\n", pnote->date       );
	    fprintf( fp, "Stamp   %ld\n", pnote->date_stamp );
	    fprintf( fp, "To      %s~\n", pnote->to_list    );
	    fprintf( fp, "Subject %s~\n", pnote->subject    );
	    fprintf( fp, "Protect %d\n",  pnote->protected  );
	    fprintf( fp, "Board   %d\n",  pnote->on_board   );
	    fprintf( fp, "Text\n%s~\n\n", pnote->text       );
	}
	fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


/* Date stamp idea comes from Alander of ROM */
void do_note( CHAR_DATA *ch, char *argument )
{
    NOTE_DATA *pnote =0;
    char       buf  [ MAX_STRING_LENGTH   ];
    char       buf1 [ MAX_STRING_LENGTH*7 ];
    char       arg  [ MAX_INPUT_LENGTH    ];
    int        vnum;
    int        anum;

    argument = one_argument( argument, arg );
    smash_tilde( argument );

    if ( arg[0] == '\0' )
    {
        if ( !IS_NPC(ch) )
	  do_note( ch, "read" );
	return;
    }

    if ( !str_cmp( arg, "test" ) )
    {
      time_t time_n;
      char fst[MAX_STRING_LENGTH];
      char sst[MAX_STRING_LENGTH];

      time_n = current_time + (60*60*24*180);
      act(AT_BLUE, "$t", ch, ctime(&time_n), NULL, TO_CHAR);
      strcpy(fst, ctime(&time_n));
      strcpy(sst, fst + 11);
      sst[8] = '\0';
      send_to_char(AT_BLUE,sst,ch);
      sprintf(fst, "\n\r%d %d %d", ("aA" < "aB"), ("Aa" > "aB"), ("A" < "b"));
      send_to_char(AT_BLUE,fst,ch);
      return;
    }

    if ( !str_cmp( arg, "list" ) )
    {
        char arg1[MAX_STRING_LENGTH];
        char arg2[MAX_STRING_LENGTH];
        int fn = 0;
        int ln = 0;
        
	if ( IS_NPC(ch) )
	  return;

        vnum = 0;
        buf1[0] = '\0';

        if ( argument[0] != '\0' )
        {
          argument = one_argument( argument, arg1 );
          argument = one_argument( argument, arg2 );
          fn = is_number( arg1 ) ? atoi( arg1 ) : 0;
          ln = is_number( arg2 ) ? atoi( arg2 ) : 0;
          
          if ( ( fn == 0 && ln == 0 ) || ( fn < 1 ) || ( ln < 0 )
            || ( ln < fn ) )
          {
            send_to_char( AT_DGREEN, "Invalid note range.\n\r", ch );
            return;
          }
        }
	
	for ( pnote = note_list; pnote; pnote = pnote->next )
	{
	    if ( ( is_note_to( ch, pnote ) && vnum >= fn && vnum <= ln )
	     || ( fn == 0 && ln == 0 && is_note_to( ch, pnote ) ) )
	    {
		sprintf( buf, "[%3d%s%s] %s: %s\n\r",
			vnum,
			( pnote->date_stamp > ch->last_note
			 && str_cmp( pnote->sender, ch->name ) ) ? "N" : " ",
			(get_trust(ch) >= L_CON || IS_CODER(ch)) ?
			 pnote->protected ? "P" : " " : "",
			pnote->sender, pnote->subject );
		strcat( buf1, buf );
	    }
	    
	    if ( is_note_to( ch, pnote ) )
	      vnum++;

	}
	send_to_char(AT_GREEN, buf1, ch );
	return;
    }

    if ( !str_cmp( arg, "read" ) )
    {
	bool fAll;

	if ( IS_NPC(ch) )
	  return;

	else if ( argument[0] == '\0' || !str_prefix( argument, "next" ) )
	  /* read next unread note */
	{
	    vnum    = 0;
	    buf1[0] = '\0';
	    for ( pnote = note_list; pnote; pnote = pnote->next )
	    {
		if ( is_note_to( ch, pnote )
		    && str_cmp( ch->name, pnote->sender )
		    && ch->last_note < pnote->date_stamp )
		{
		    sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
			    vnum,
			    pnote->sender,
			    pnote->subject,
			    pnote->date,
			    pnote->to_list );
		    strcat( buf1, buf );
		    strcat( buf1, pnote->text );
		    ch->last_note = UMAX( ch->last_note, pnote->date_stamp );
		    send_to_char(AT_GREEN, buf1, ch );
		    return;
		}
		else
		  if ( is_note_to( ch, pnote ) )
		    vnum++;
	    }
	    send_to_char(AT_DGREEN, "You have no unread notes.\n\r", ch );
	    return;
	}
	else if ( is_number( argument ) )
	{
	    fAll = FALSE;
	    anum = atoi( argument );
	}
	else
	{
	    send_to_char(AT_DGREEN, "Note read which number?\n\r", ch );
	    return;
	}

	vnum    = 0;
	buf1[0] = '\0';
	for ( pnote = note_list; pnote; pnote = pnote->next )
	{
	    if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) )
	    {
		sprintf( buf, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
			vnum - 1,
			pnote->sender,
			pnote->subject,
			pnote->date,
			pnote->to_list );
		strcat( buf1, buf );
		strcat( buf1, pnote->text );
		if ( !fAll )
		    send_to_char(AT_GREEN, buf1, ch );
		else
		    strcat( buf1, "\n\r" );
		ch->last_note = UMAX( ch->last_note, pnote->date_stamp );
		if ( !fAll )
		    return;
	    }
	}

	if ( !fAll )
	    send_to_char(AT_DGREEN, "No such note.\n\r", ch );
	else
	    send_to_char(AT_GREEN, buf1, ch );
	return;
    }

    if ( !str_cmp( arg, "+" ) )
    {
	note_attach( ch );
	strcpy( buf, ch->pnote->text );
	if ( strlen( buf ) + strlen( argument ) >= MAX_STRING_LENGTH - 100 )
	{
	    send_to_char(AT_DGREEN, "Note too long.\n\r", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r"   );
	free_string( ch->pnote->text );
	ch->pnote->text = str_dup( buf );
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "write" ) )
    {
      if ( IS_NPC(ch) )
	return;
      note_attach( ch );
      string_append( ch, &ch->pnote->text );
      return;
    }

    if ( !str_cmp( arg, "subject" ) )
    {
	note_attach( ch );
	free_string( ch->pnote->subject );
	ch->pnote->subject = str_dup( argument );
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "to" ) )
    {
	note_attach( ch );
	free_string( ch->pnote->to_list );
	ch->pnote->to_list = str_dup( argument );
	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "clear" ) )
    {
	if ( ch->pnote )
	{
		free_note(ch->pnote);
	    ch->pnote		= NULL;
	}

	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "show" ) )
    {
        if ( IS_NPC(ch))
	  return;

	if ( !ch->pnote )
	{
	    send_to_char(AT_DGREEN, "You have no note in progress.\n\r", ch );
	    return;
	}

	sprintf( buf, "%s: %s\n\rTo: %s\n\r",
		ch->pnote->sender,
		ch->pnote->subject,
		ch->pnote->to_list );
	send_to_char(AT_GREEN, buf, ch );
	send_to_char(AT_GREEN, ch->pnote->text, ch );
	return;
    }

    if ( !str_cmp( arg, "post" ) || !str_prefix( arg, "send" ) )
    {
	FILE *fp;
	char *strtime;
	OBJ_DATA *board;

	if ( !ch->pnote )
	{
	    send_to_char(AT_DGREEN, "You have no note in progress.\n\r", ch );
	    return;
	}

	if ( !str_cmp( ch->pnote->to_list, "" ) )
	{
	    send_to_char(AT_DGREEN,
	      "You need to provide a recipient (name, all, or immortal).\n\r",
			 ch );
	    return;
	}

	if ( !str_cmp( ch->pnote->subject, "" ) )
	{
	    send_to_char(AT_DGREEN, "You need to provide a subject.\n\r", ch );
	    return;
	}

	ch->pnote->on_board = 0;
	if ( ch->in_room )
	{
	  for ( board = ch->in_room->contents; board;
	        board = board->next_content )
	    if ( board->item_type == ITEM_NOTEBOARD )
	      break;
	  
	  if ( board )
	  if ( board->value[2] > get_trust(ch) )
	  {
	    OBJ_DATA *decoder;

	    for ( decoder = ch->carrying; decoder;
		  decoder = decoder->next_content )
	      if ( decoder->pIndexData->vnum == board->value[0] )
		break;
	    if ( decoder == NULL )
	    {
	      send_to_char( AT_WHITE, "You may not post on this board.\n\r",ch);
	      return;
	    }
	  }
	  if ( board )
	    ch->pnote->on_board = board->pIndexData->vnum;
	}

	if ( IS_NPC(ch) && ch->pnote->on_board == 0 )
	  return;

	ch->pnote->next			= NULL;
	strtime				= ctime( &current_time );
	strtime[strlen(strtime)-1]	= '\0';
	free_string( ch->pnote->date );
	ch->pnote->date			= str_dup( strtime );
	ch->pnote->date_stamp           = current_time;

	if ( !note_list )
	{
	    note_list	= ch->pnote;
	}
	else
	{
	    for ( pnote = note_list; pnote->next; pnote = pnote->next )
		;
	    pnote->next	= ch->pnote;
	}
	pnote		= ch->pnote;
	ch->pnote       = NULL;

	fclose( fpReserve );
	if ( !( fp = fopen( NOTE_FILE, "a" ) ) )
	{
	    perror( NOTE_FILE );
	}
	else
	{
	    fprintf( fp, "Sender  %s~\n", pnote->sender     );
	    fprintf( fp, "Date    %s~\n", pnote->date       );
	    fprintf( fp, "Stamp   %ld\n", pnote->date_stamp );
	    fprintf( fp, "To      %s~\n", pnote->to_list    );
	    fprintf( fp, "Subject %s~\n", pnote->subject    );
	    fprintf( fp, "Protect %d\n",  pnote->protected  );
	    fprintf( fp, "Board   %d\n",  pnote->on_board   );
	    fprintf( fp, "Text\n%s~\n\n", pnote->text       );
	    fclose( fp );
	}
	fpReserve = fopen( NULL_FILE, "r" );

	send_to_char(AT_WHITE, "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "remove" ) )
    {
        if ( IS_NPC(ch) )
	  return;

	if ( !is_number( argument ) )
	{
	    send_to_char(AT_DGREEN, "Note remove which number?\n\r", ch );
	    return;
	}

	anum = atoi( argument );
	vnum = 0;
	for ( pnote = note_list; pnote; pnote = pnote->next )
	{
	    if ( is_note_to( ch, pnote ) && vnum++ == anum )
	    {
		note_remove( ch, pnote );
		send_to_char(AT_WHITE, "Ok.\n\r", ch );
		return;
	    }
	}

	send_to_char(AT_DGREEN, "No such note.\n\r", ch );
	return;
    }

    /*
     * "Permanent" note flag.
     * -- Altrag
     */
    if ( !str_cmp( arg, "protect" ) )
    {
      if ( IS_NPC(ch) )
	return;

      if ( get_trust( ch ) < L_CON && !IS_CODER( ch ) )
      {
	send_to_char( AT_DGREEN, "Huh?  Type 'help note' for usage.\n\r", ch );
	return;
      }
      if ( argument[0] == '\0' || !is_number( argument ) )
      {
	send_to_char( AT_DGREEN, "Syntax:  note protect <#>\n\r", ch );
	return;
      }
      anum = atoi( argument );
      vnum = 0;
      for ( pnote = note_list; pnote; pnote = pnote->next )
      {
	if ( is_note_to( ch, pnote ) && vnum++ == anum )
	{
	  if ( pnote->protected )
	    pnote->protected = FALSE;
	  else
	    pnote->protected = TRUE;
	  note_cleanup ();
	  send_to_char( AT_WHITE, "Ok.\n\r", ch );
	  return;
	}
      }
      send_to_char( AT_WHITE, "No such note.\n\r", ch );
      return;
    }

    send_to_char(AT_DGREEN, "Huh?  Type 'help note' for usage.\n\r", ch );
    return;
}



/*
 * Generic channel function.
 * MAJOYRLY rewritten to allow languages by Tyrion, used only by mobs.
 */
void talk_channel_info( CHAR_DATA *ch, char *argument, int channel, const char *verb)
{
    DESCRIPTOR_DATA *d;
    char             buf [ MAX_STRING_LENGTH ];
    int              position;
    if ( argument[0] == '\0' )
    {
	sprintf( buf, "%s what?\n\r", verb );
	buf[0] = UPPER( buf[0] );
	return;
    }

    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_SILENCE ) )
    {
	sprintf( buf, "You can't %s.\n\r", verb );
	send_to_char(AT_WHITE, buf, ch );
	return;
    }
    
    if ( !IS_NPC( ch ) && IS_AFFECTED2( ch, AFF_SLIT ) )
    {
	sprintf( buf, "You can't %s.\n\r", verb );
	send_to_char(AT_WHITE, buf, ch );
	return;
    }
    
    if ( IS_SET( ch->in_room->room_flags, ROOM_SILENT ) 
         && (get_trust(ch)<106) )
    {
        send_to_char(AT_WHITE, "You can't do that here.\n\r", ch );
        return;
    }

    REMOVE_BIT( ch->deaf, channel );

    if(!IS_NPC(ch) )
	argument = makedrunk(argument, ch );

    switch ( channel )
    {
    default:
	sprintf( buf, "You %s '%s'\n\r", verb, argument );
	send_to_char(AT_LBLUE, buf, ch );
	sprintf( buf, "$n %ss '$t'",     verb );
	break;
    case CHANNEL_IMMTALK:
	sprintf( buf, "$n: $t");
	position	= ch->position;
	ch->position	= POS_STANDING;
	act(AT_YELLOW, buf, ch, argument, NULL, TO_CHAR );
	ch->position	= position;
	break;
    case CHANNEL_GUILD:
        sprintf( buf, "[%s] $n: '$t'", (ch->guild != NULL)
         ? ch->guild->name : "NONE");
        position        = ch->position;
        ch->position   = POS_STANDING;
        act((ch->guild != NULL) ? 
         ch->guild->color : AT_RED, buf, ch, argument, NULL, TO_CHAR );
        ch->position    = position;
        break;
    case CHANNEL_CLAN:
        sprintf( buf, "<%s> $n: '$t'",
	       ( get_clan_index(ch->clan) && (get_clan_index(ch->clan))->name ?
		(get_clan_index(ch->clan))->name : "Unclanned") );
        position        = ch->position;
        ch->position   = POS_STANDING;
        act(AT_RED, buf, ch, argument, NULL, TO_CHAR );
        ch->position    = position;
        break;
    case CHANNEL_GUARDIAN:
        sprintf( buf, "$n> &P'$t'" );
        position        = ch->position;
        ch->position   = POS_STANDING;
        act(AT_PURPLE, buf, ch, argument, NULL, TO_CHAR );
        ch->position     = position;
        break;
    case CHANNEL_GRATZ:
	sprintf( buf, "<Gratz> $n '$t'" );
	position 	= ch->position;
	ch->position	= POS_STANDING;
	act(AT_LBLUE, buf, ch, argument, NULL, TO_CHAR );
	ch->position	= position;
	break;
    case CHANNEL_TIMELORD:
	sprintf( buf, "$n <TL> &b'$t'" );
	position	= ch->position;
	ch->position	= POS_STANDING;
	act(AT_RED, buf, ch, argument, NULL, TO_CHAR );
	ch->position	= position;
	break;
    case CHANNEL_INFO:
	sprintf( buf, "&b[&CINFO&b]:&B $t" );
	position	= ch->position;
	ch->position	= POS_STANDING;
	act(AT_BLUE, buf, ch, argument, NULL, TO_CHAR );
	ch->position 	= position;
	break;
    case CHANNEL_OOC:
	sprintf( buf, "OOC: $n chats '$t'" );
	position	= ch->position;
	ch->position	= POS_STANDING;
	act(AT_GREEN, buf, ch, argument, NULL, TO_CHAR );
	ch->position	= position;
	break;
    case CHANNEL_MUTTER:
	sprintf( buf, "$n mutters &c'$t'" );
	position	= ch->position;
	ch->position	= POS_STANDING;
	act(AT_CYAN, buf, ch, argument, NULL, TO_CHAR );
	ch->position	= position;
	break;
    case CHANNEL_CLASS:
	sprintf( buf, "{CLASS} $n: $t" );
        position        = ch->position;
        ch->position   = POS_STANDING;
        act(AT_LBLUE, buf, ch, argument, NULL, TO_CHAR );
        ch->position    = position;
	break;
    case CHANNEL_DEMIGOD:
	sprintf( buf, "&z<Demi-God> &w$n: '$t'");
	position	= ch->position;
	ch->position	= POS_STANDING;
	act(AT_GREY, buf, ch, argument, NULL, TO_CHAR );
	ch->position	= position;
	break;
    case CHANNEL_CHAMPION:
	sprintf( buf, "&g(Champion) &G$n: '$t'" );
	position	= ch->position;
	ch->position	= POS_STANDING;
	act(AT_GREEN, buf, ch, argument, NULL, TO_CHAR );
	ch->position	= position;
	break;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	CHAR_DATA *och;
	CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;

	if ( d->connected == CON_PLAYING
	    && vch != ch
	    && !IS_SET( och->deaf, channel ) 
	    && !IS_SET( och->in_room->room_flags, ROOM_SILENT ) )
	{
	    if ( channel == CHANNEL_IMMTALK && !IS_IMMORTAL( och ) )
		continue;
	    if ( channel == CHANNEL_CHAMPION && !IS_HERO( och ) )
	        continue;
	    if ( channel == CHANNEL_DEMIGOD && ( och->level < LEVEL_DEMIGOD ) )
		continue;
	    if ( channel == CHANNEL_GUARDIAN && get_trust( och ) < L_DIR )
	        continue;
	    if ( channel == CHANNEL_TIMELORD && get_trust( och ) < L_CON )
		continue;
	    if ( channel == CHANNEL_CLASS
	        && vch->class != ch->class )
	        continue;
            if(channel == CHANNEL_GUILD
             && vch->guild != ch->guild)
              continue;
	    if ( channel == CHANNEL_CLAN 
	        && vch->clan != ch->clan )
	        continue;
	    if ( channel == CHANNEL_YELL
		&& vch->in_room->area != ch->in_room->area )
	        continue;

	    position		= vch->position;
	    if ( channel != CHANNEL_SHOUT && channel != CHANNEL_YELL )
		vch->position	= POS_STANDING;
     
            switch ( channel )
            {
             default:
	             act(AT_LBLUE, buf, ch, argument, vch, TO_VICT ); break;
             case CHANNEL_IMMTALK:
	             act(AT_YELLOW, buf, ch, argument, vch, TO_VICT ); break;
	     case CHANNEL_CHAMPION:
	             act(AT_GREEN, buf, ch, argument, vch, TO_VICT ); break;
	     case CHANNEL_DEMIGOD:
		     act(AT_GREEN, buf, ch, argument, vch, TO_VICT ); break;
             case CHANNEL_GUARDIAN:
             	     sprintf( buf, "$n> &P'$t'" );
             	     act( AT_PURPLE, buf, ch, argument, vch, TO_VICT );
	     break;/* SOMEONE forgot the break, yeesh heh */
	     case CHANNEL_TIMELORD:
		     sprintf( buf, "$n <TL> &b'$t'");
		     act( AT_RED, buf, ch, argument, vch, TO_VICT );
	     break;
	     case CHANNEL_INFO:
		     sprintf( buf, "&b[&CINFO&b]:&B $t" );
		     act( AT_BLUE, buf,ch, argument, vch, TO_VICT );
		     break;
	     case CHANNEL_OOC:
		     act( AT_GREEN, buf, ch, argument, vch, TO_VICT); break;
	     case CHANNEL_GUILD:
                     act((ch->guild != NULL) ? ch->guild->color : AT_RED,
			buf, ch, argument, vch, TO_VICT ); break;
             case CHANNEL_CLAN:
                     act(AT_RED, buf, ch, argument, vch, TO_VICT ); break;
             case CHANNEL_MUTTER:
                     act(AT_CYAN, buf, ch, argument, vch, TO_VICT ); break;
             case CHANNEL_CLASS:
                     act(AT_LBLUE, buf, ch, argument, vch, TO_VICT ); break;
            }
	    vch->position	= position;
	}
    }

    return;
}

void auc_channel ( char *auction )
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;

  sprintf( buf, "&GAUCTION: &g%s\n\r", auction );

  for ( d = descriptor_list; d; d = d->next )
  {
    if ( d->connected != CON_PLAYING )
      continue;

    if ( !IS_SET( (d->original ? d->original : d->character)->deaf, CHANNEL_AUCTION ) )
      write_to_buffer( d, buf, 0 );
  }

  return;
}

void do_gdt( CHAR_DATA *ch, char *argument )
{
  if(ch->guild == NULL)
  {
    send_to_char(AT_BLUE, "You are not guilded.\n\r", ch);
    return;
  }
  talk_channel( ch, argument, CHANNEL_GUILD, "guildtalk", ch->speaking );
  return;
}

void do_clan( CHAR_DATA *ch, char *argument )
{
  if(ch->clan == 0)
  {
    send_to_char(AT_BLUE, "You are not clanned.\n\r", ch);
    return;
  }
  talk_channel( ch, argument, CHANNEL_CLAN, "clantalk", ch->speaking );
  return;
}

void do_class( CHAR_DATA *ch, char *argument )
{
  talk_channel( ch, argument, CHANNEL_CLASS, "classtalk", ch->speaking );
  return;
}

void do_auction( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    int bid;
    
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1 );
    bid = is_number( arg1 ) ? atoi( arg1 ) : 0;
    
    if ( IS_NPC( ch ) )
    {
      send_to_char( AT_WHITE, "You can't auction items.\n\r", ch );
      return;
    }
    
    if ( arg[0] == '\0' )
    {
      send_to_char( AT_WHITE, "Auction which item?\n\r", ch );
      return;
    }

    if ( !str_cmp( arg, "remove" ) )
    {
      if ( !auc_obj || !auc_held || auc_held != ch )
      {
	send_to_char(AT_WHITE, "You do not have an item being auctioned.\n\r",ch);
	return;
      }
      if ( auc_bid )
      {
	send_to_char(AT_WHITE, "You may not remove your item after a bid has been made.\n\r", ch );
	return;
      }
      REMOVE_BIT(ch->deaf, CHANNEL_AUCTION);
      sprintf(log_buf, "%s has been removed from the auction.\n\r", auc_obj->short_descr );
      auc_channel( log_buf );
      act( AT_DGREEN, "$p appears suddenly in your hands.", ch, auc_obj, NULL, TO_CHAR );
      act( AT_DGREEN, "$p appears suddenly in the hands of $n.", ch, auc_obj, NULL,
        TO_ROOM );
      obj_to_char( auc_obj, ch );
      auc_obj = NULL;
      auc_held = NULL;
      auc_cost = 0;
      auc_count = -1;
      return;
    }

    if ( bid <= 0 )
    {
      send_to_char(AT_WHITE, "Auction it for how much?\n\r",ch );
      return;
    }
    
    if ( bid < 100 )
    {
      send_to_char( AT_WHITE, "That is too low of a starting bidding price.\n\r", ch );
      return;
    }
    
    if ( auc_obj )
    {
      send_to_char( AT_WHITE, "There is already an item being auctioned.\n\r", ch );
      return;
    }
   else
    {
      if ( ( auc_obj = get_obj_carry( ch, arg ) ) )
      {
        if ( (auc_obj->pIndexData->vnum > 1 && auc_obj->pIndexData->vnum < 23 ) 
            || ( auc_obj->item_type == ITEM_CONTAINER && auc_obj->contains ))
        { 
          send_to_char( AT_DGREEN, "You can't auction that.\n\r", ch );
          auc_obj = NULL;
          return;
        } /*
		  else if (IS_SET(auc_obj->extra_flags2, ITEM_QUEST) && !IS_IMMORTAL(ch))
		{
			send_to_char( AT_DGREEN, "You can't auction your hard earned quest items away!\n\r", ch);
			auc_obj = NULL;
			return;
		} */
        
	REMOVE_BIT( ch->deaf, CHANNEL_AUCTION );
	act( AT_DGREEN, "$p disappears from your inventory.", ch, auc_obj, NULL,
	 TO_CHAR );
	act( AT_DGREEN, "$p disappears from the inventory of $n.", ch, auc_obj,
	 NULL, TO_ROOM );
	obj_from_char( auc_obj );
	auc_held = ch;
	auc_bid = NULL;
	auc_cost = bid;
	auc_count = 0;
        sprintf( log_buf, "%s a level %d object for %d gold coins.", auc_obj->short_descr, auc_obj->level, bid );
        auc_channel( log_buf );
        sprintf( log_buf, "%s auctioning %s (vnum %d).", auc_held->name, auc_obj->name, auc_obj->pIndexData->vnum );
        log_string( log_buf, CHANNEL_GOD, -1 );
	return;
      }
     else
      {
        send_to_char( AT_WHITE, "You are not carrying that item.\n\r", ch );
        return;
      }
    }
    
    return;
}

void do_bid( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int bid;

  if ( !auc_obj )
  {
    send_to_char( AT_WHITE, "There is no auction at the moment.\n\r", ch );
    return;
  }

  if ( !auc_held )
  {
    bug( "Do_bid: auc_obj found without auc_held.\n\r",0);
    return;
  }

  if ( ch == auc_held )
  {
    send_to_char( AT_WHITE, "If you want your item back, you should 'auction remove' it.\n\r", ch );
    return;
  }

  if ( auc_bid && ch == auc_bid )
  {
    send_to_char( AT_WHITE, "You already hold the highest bid.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );
  bid = is_number( arg ) ? atoi( arg ) : 0;

  if( bid < 0 )
  {
    send_to_char( AT_WHITE,  "You can't do that.\n\r", ch );
    return;
  }

  if ( auc_cost > bid - 100 )
  {
    sprintf( buf, "You must bid at least %d gold coins in this auction.\n\r", auc_cost + 100 );
    send_to_char( AT_WHITE, buf, ch );
    return;
  }

  if ( ch->gold < bid )
  {
    send_to_char( AT_WHITE, "You are not carrying that much gold.\n\r", ch );
    return;
  }

  REMOVE_BIT(ch->deaf, CHANNEL_AUCTION);
  sprintf( buf, "%d gold coins bid on %s.", bid, auc_obj->short_descr );
  auc_channel( buf );

  auc_cost = bid;
  auc_count = 0;
  auc_bid = ch;
  return;
}


void do_chat( CHAR_DATA *ch, char *argument )
{
  talk_channel( ch, argument, CHANNEL_CHAT, "chat", ch->speaking );
  return;
}


void do_music( CHAR_DATA *ch, char *argument )
{
  talk_channel( ch, argument, CHANNEL_MUSIC, "music", ch->speaking );
  return;
}

void do_ooc( CHAR_DATA *ch, char *argument )
{
  talk_channel( ch, argument, CHANNEL_OOC, "ooc", ch->speaking );
  return;
}

void do_question( CHAR_DATA *ch, char *argument )
{
  talk_channel( ch, argument, CHANNEL_QUESTION, "question", ch->speaking );
  return;
}

void do_answer( CHAR_DATA *ch, char *argument )
{
  talk_channel( ch, argument, CHANNEL_QUESTION, "answer", ch->speaking );
  return;
}

void do_shout( CHAR_DATA *ch, char *argument )
{
  talk_channel( ch, argument, CHANNEL_SHOUT, "shout", ch->speaking );
  WAIT_STATE( ch, 12 );
  return;
}

void do_yell( CHAR_DATA *ch, char *argument )
{
  talk_channel( ch, argument, CHANNEL_YELL, "yell", ch->speaking );
  return;
}

void do_guard( CHAR_DATA *ch, char *argument )
{
    if ( get_trust( ch ) < 108 )
    {
       send_to_char(AT_WHITE,"You don't have access to that channel.\n\r", ch );
       return;
    }

    talk_channel( ch, argument, CHANNEL_GUARDIAN, "guard", ch->speaking );
    return;
}

void do_timelord( CHAR_DATA *ch, char *argument )
{
    if ( get_trust( ch ) < L_CON )
    {
       send_to_char(AT_WHITE,"You don't have access to that channel.\n\r", ch );
       return;
    }
    talk_channel( ch, argument, CHANNEL_TIMELORD, "timelord", ch->speaking );
    return;
}

void do_info( CHAR_DATA *ch, char *argument )
{
    if(IS_NPC( ch ) )
    {
	return;
    }
    if( !IS_IMMORTAL ( ch ) )
    {
	return;
    }

     talk_channel_info( ch, argument, CHANNEL_INFO, "info" );
     return;
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
    if(!IS_NPC( ch ) && !IS_IMMORTAL(ch))
    {
        send_to_char(AT_WHITE,"You are still but mortal.\n\r", ch );
        return;
    }
  talk_channel( ch, argument, CHANNEL_IMMTALK, "immtalk", ch->speaking );
  return;
}

void do_hero( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC( ch ) && ch->level < LEVEL_HERO )
    {
        send_to_char(AT_WHITE,"You are not yet a champion.\n\r", ch );
        return;
    }
    talk_channel( ch, argument, CHANNEL_CHAMPION, "champion", ch->speaking );
    return;
}

void do_demigod( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC( ch ) && ch->level < LEVEL_CHAMP )
    {
	send_to_char(AT_WHITE,"You are not yet a Demi-God.\n\r", ch );
	return;
    }

    talk_channel( ch, argument, CHANNEL_DEMIGOD, "demi-god", ch->speaking );
    return;
}

void do_mutter( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_MUTTER, "mutter", ch->speaking );
    return;
}

void do_gratz( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_GRATZ, "gratz", ch->speaking );
    return;
}

void do_say( CHAR_DATA *ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
	send_to_char(AT_BLUE, "Say what?\n\r", ch );
	return;
    }

#if defined ( FORCE_LANGUAGE )
/*        if (!IS_NPC(ch)) */
	if (!IS_NPC( ch ) || IS_NPC( ch ) )
        {
                do_language(ch, argument, ch->speaking);
                mprog_speech_trigger( argument, ch );
                return;
        }
        else
        {
                act(AT_DGREY, "$n says &W'$T'", ch, NULL, argument, TO_ROOM );
                act(AT_DGREY, "You say &W'$T'", ch, NULL, argument, TO_CHAR );
                mprog_speech_trigger( argument, ch );
                return;
        }
#endif

}



void do_tell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
    int        position;

    if ( !IS_NPC( ch ) && (   IS_SET( ch->act, PLR_SILENCE )
			   || IS_SET( ch->act, PLR_NO_TELL ) || IS_AFFECTED2(ch, AFF_SLIT)) )
    {
	send_to_char(AT_WHITE, "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SILENT ) )
    {
        send_to_char(AT_WHITE, "You can't do that here.\n\r", ch );
        return;
    }
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( !( victim = get_char_world( ch, arg ) )
	|| ( IS_NPC( victim ) && victim->in_room != ch->in_room ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_SET( victim->in_room->room_flags, ROOM_SILENT ) )
    {
        act( AT_WHITE, "$E can't hear you.", ch, 0, victim, TO_CHAR );
        return;
    }
    
    if ( !IS_IMMORTAL( ch ) && !IS_AWAKE( victim ) )
    {
	act(AT_WHITE, "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }
    
    if ( !IS_NPC( victim ) && ( !( victim->desc ) ) )
    {
        act(AT_WHITE, "$E is link-dead.", ch, 0, victim, TO_CHAR );
        return;
    }

    act(AT_WHITE, "You tell $N '$t'", ch, argument, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;
    act(AT_WHITE, "$n tells you '$t'", ch, argument, victim, TO_VICT );
    victim->position	= position;
    victim->reply	= ch;

    if ( IS_SET( victim->act, PLR_AFK ) )
        act(AT_WHITE, "Just so you know, $E is AFK.", ch, NULL, victim, TO_CHAR );

    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int        position;

    if ( !IS_NPC( ch ) && (IS_SET( ch->act, PLR_SILENCE ) || IS_AFFECTED2( ch, AFF_SLIT)) )
    {
	send_to_char(AT_WHITE, "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( !( victim = ch->reply ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char(AT_WHITE, "Reply what?\n\r", ch );
        return;
    }

    if ( ( !IS_IMMORTAL( ch ) && !IS_AWAKE( victim ) ) 
           || ( IS_SET( victim->in_room->room_flags, ROOM_SILENT ) 
           && (get_trust(ch) < L_APP ) ) )
    {
	act(AT_WHITE, "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    act(AT_WHITE, "You tell $N '$t'",  ch, argument, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;
    act(AT_WHITE, "$n tells you '$t'", ch, argument, victim, TO_VICT );
    victim->position	= position;
    victim->reply	= ch;

    if ( IS_SET( victim->act, PLR_AFK ) )
        act(AT_WHITE, "Just so you know, $E is AFK.", ch, NULL, victim, TO_CHAR );

    return;
}



void do_emote( CHAR_DATA *ch, char *argument )
{
    char  buf [ MAX_STRING_LENGTH ];
    char *plast;

    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_NO_EMOTE ) )
    {
	send_to_char(AT_PURPLE, "You are an emotionless blob.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char(AT_PURPLE, "Emote what?\n\r", ch );
	return;
    }

    for ( plast = argument; *plast != '\0'; plast++ )
	;

    strcpy( buf, argument );
    if ( isalpha( plast[-1] ) )
	strcat( buf, "." );

    act(AT_PINK, "$n $T", ch, NULL, buf, TO_ROOM );
    act(AT_PINK, "$n $T", ch, NULL, buf, TO_CHAR );
    return;
}



/*
 * All the posing stuff.
 */
struct	pose_table_type
{
    char * message[ 2*MAX_CLASS ];
};

const	struct	pose_table_type	pose_table	[]	=
{
    {
	{
/*  0 */	"You sizzle with energy.",
/*  0 */	"$n sizzles with energy.",
/*  1 */	"You feel very holy.",
/*  1 */	"$n looks very holy.",
/*  2 */	"You perform a small card trick.",
/*  2 */	"$n performs a small card trick.",
/*  3 */	"You show your bulging muscles.",
/*  3 */	"$n shows $s bulging muscles.",
/*  4 */	"Stop it with the Ouija board, will ya?",
/*  4 */	"Great, $n is playing with $s Ouija board again.",
/*  5 */	"You talk to some nearby bugs.",
/*  5 */	"$n converses with nature."
/*  6 */	"You show off your bulging muscles.",
/*  6 */	"$n shows $s bulging muscles.",
/*  7 */	"You preach about the wonders of the church.",
/*  7 */	"$n gives you a sermon.",
/*  8 */	"You play a short rift on your lute.",
/*  8 */	"$n plays a short rift on $s lute.",
/*  9 */	"You proclaim 'I vant to suck your blood!'",
/*  9 */	"$n proclaims 'I vant to suck your blood!'."
/* 10 */
/* 10 */
/* 11 */
/* 11 */
/* 12 */
/* 12 */
/* 13 */
/* 13 */
/* 14 */
/* 14 */
/* 15 */
/* 15 */
/* 16 */
/* 16 */
/* 17 */
/* 17 */
/* 18 */
/* 18 */
/* 19 */
/* 19 */
	}
    },

    {
	{
	    "You turn into a butterfly, then return to your normal shape.",
	    "$n turns into a butterfly, then returns to $s normal shape.",
	    "You nonchalantly turn wine into water.",
	    "$n nonchalantly turns wine into water.",
	    "You wiggle your ears alternately.",
	    "$n wiggles $s ears alternately.",
	    "You crack nuts between your fingers.",
	    "$n cracks nuts between $s fingers.",
            "You read everyone's mind....and shudder with disgust.",
            "$n reads your mind...eww, you pervert!",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "Blue sparks fly from your fingers.",
	    "Blue sparks fly from $n's fingers.",
	    "A halo appears over your head.",
	    "A halo appears over $n's head.",
	    "You nimbly tie yourself into a knot.",
	    "$n nimbly ties $mself into a knot.",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean.",
            "You show everyone your awards for perfect school attendance",
            "You aren't impressed by $n's school attendance awards.  Geek.",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "Little red lights dance in your eyes.",
	    "Little red lights dance in $n's eyes.",
	    "You recite words of wisdom.",
	    "$n recites words of wisdom.",
	    "You juggle with daggers, apples, and eyeballs.",
	    "$n juggles with daggers, apples, and eyeballs.",
	    "You hit your head, and your eyes roll.",
	    "$n hits $s head, and $s eyes roll.",
            "A will-o-the-wisp arrives with your slippers.",
            "A will-o-the-wisp affives with $n's slippers.",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
	    "Deep in prayer, you levitate.",
	    "Deep in prayer, $n levitates.",
	    "You steal the underwear off every person in the room.",
	    "Your underwear is gone!  $n stole it!",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle.",
            "What's with the extra leg?",
            "Why did $n sprout an extra leg just now?",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "You turn everybody into a little pink elephant.",
	    "You are turned into a little pink elephant by $n.",
	    "An angel consults you.",
	    "An angel consults $n.",
	    "The dice roll ... and you win again.",
	    "The dice roll ... and $n wins again.",
	    "... 98, 99, 100 ... you do pushups.",
	    "... 98, 99, 100 ... $n does pushups.",
            "The spoons flee as you begin to concentrate.",
            "The spoons flee as $n begins to concentrate.",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
	    "Your body glows with an unearthly light.",
	    "$n's body glows with an unearthly light.",
	    "You count the money in everyone's pockets.",
	    "Check your money, $n is counting it.",
	    "Arnold Schwarzenegger admires your physique.",
	    "Arnold Schwarzenegger admires $n's physique.",
            "Stop wiggling your brain at people.",
            "Make $n stop wiggling $s brain at you!",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "A spotlight hits you.",
	    "A spotlight hits $n.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders.",
            "MENSA called...they want your opinion on something.",
            "MENSA just called $n for consultation.",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "The light flickers as you rap in magical languages.",
	    "The light flickers as $n raps in magical languages.",
	    "Everyone levitates as you pray.",
	    "You levitate as $n prays.",
	    "You produce a coin from everyone's ear.",
	    "$n produces a coin from your ear.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder.",
            "Chairs fly around the room at your slightest whim.",
            "Chairs fly around the room at $n's slightest whim.",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "Your head disappears.",
	    "$n's head disappears.",
	    "A cool breeze refreshes you.",
	    "A cool breeze refreshes $n.",
	    "You step behind your shadow.",
	    "$n steps behind $s shadow.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear.",
            "Oof...maybe you shouldn't summon any more hippopotamuses.",
            "Oof!  Guess $n won't be summoning any more hippos for a while.",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "The sun pierces through the clouds to illuminate you.",
	    "The sun pierces through the clouds to illuminate $n.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug.",
            "Oops...your hair is sizzling from thinking too hard.",
            "Oops...$n's hair is sizzling from thinking too hard.",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
	    "The ocean parts before you.",
	    "The ocean parts before $n.",
	    "You deftly steal everyone's weapon.",
	    "$n deftly steals your weapon.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop splits a tree.",
            "What?  You were too busy concentrating.",
            "What?  Oh, $n was lost in thought...again.",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s mighty thews.",
            "Will you get down here before you get hurt?",
            "Quick, get a stick, $n is doing $s pinata impression again.",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The Burning Man speaks to you.",
	    "The Burning Man speaks to $n.",
	    "Everyone's pocket explodes with your fireworks.",
	    "Your pocket explodes with $n's fireworks.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown.",
            "Careful...don't want to disintegrate anyone!",
            "LOOK OUT!  $n is trying to disintegrate something!",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing.",
	    "An eye in a pyramid winks at you.",
	    "An eye in a pyramid winks at $n.",
	    "Everyone discovers your dagger a centimeter from their eye.",
	    "You discover $n's dagger a centimeter from your eye.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding.",
            "You run off at the mouth about 'mind over matter'.",
            "Yeah, yeah, mind over matter.  Shut up, $n.",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "Valentine Michael Smith offers you a glass of water.",
	    "Valentine Michael Smith offers $n a glass of water.",
	    "Where did you go?",
	    "Where did $n go?",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot.",
            "Thud.",
            "Thud.",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    },

    {
	{
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "The great god Mota gives you a staff.",
	    "The great god Mota gives $n a staff.",
	    "Click.",
	    "Click.",
	    "Atlas asks you to relieve him.",
	    "Atlas asks $n to relieve him.",
            "You charm the pants off everyone...and refuse to give them back.",
            "Your pants are charmed off by $n, and $e won't give them back.",
            "You talk to some nearby bugs.",
            "$n converses with nature.",
            "You show off your bulging muscles.",
            "$n shows $s bulging muscles.",
            "You preach about the wonders of the church.",
            "$n gives you a sermon.",
            "You play a short rift on your lute.",
            "$n plays a short rift on $s lute.",
            "You proclaim 'I vant to suck your blood!'",
            "$n proclaims 'I vant to suck your blood!'."
	}
    }
};

void do_pose( CHAR_DATA *ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC( ch ) )
	return;

    level = UMIN( ch->level,
		 sizeof( pose_table ) / sizeof( pose_table[0] ) - 1 );
    pose  = number_range( 0, level );

    act(AT_PINK, pose_table[pose].message[2*ch->class+0], ch, NULL, NULL, TO_CHAR );
    act(AT_PINK, pose_table[pose].message[2*ch->class+1], ch, NULL, NULL, TO_ROOM );

    return;
}



void do_bug( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char *strtime;

    strtime                    = ctime( &current_time );
    strtime[strlen( strtime )-1] = '\0';
	sprintf(buf, "%s :: %s\n", strtime, argument);
    append_file( ch, BUG_FILE,  buf );
    send_to_char(AT_WHITE, "Ok.  Thanks.\n\r", ch );
    return;
}



void do_idea( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char *strtime;

    strtime                    = ctime( &current_time );
    strtime[strlen( strtime )-1] = '\0';
	sprintf(buf, "%s :: %s\n", strtime, argument);
    append_file( ch, IDEA_FILE, buf );
    send_to_char(AT_WHITE, "Ok.  Thanks.\n\r", ch );
    return;
}



void do_typo( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char *strtime;

    strtime                    = ctime( &current_time );
    strtime[strlen( strtime )-1] = '\0';
	sprintf(buf, "%s :: %s\n", strtime, argument);
    append_file( ch, TYPO_FILE, buf );
    send_to_char(AT_WHITE, "Ok.  Thanks.\n\r", ch );
    return;
}



void do_rent( CHAR_DATA *ch, char *argument )
{
    send_to_char(AT_WHITE, "Rent?! Ther's no stinkin rent here!  Just save and quit.\n\r", ch );
    return;
}



void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char(AT_WHITE, "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}



void do_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *PeT;
    USERL_DATA *ul;

    if ( IS_NPC( ch ) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char(AT_WHITE, "No way! You are fighting.\n\r", ch );
	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	send_to_char(AT_WHITE, "You're not DEAD yet.\n\r", ch );
	return;
    }

    if ( ch->combat_timer && !IS_IMMORTAL( ch ) )
    {
        send_to_char(AT_WHITE, "Your adrenaline is pumping too hard.\n\r",ch);
	return;
    }

    if ( ch->gold > 500000 )
    {
      int tax;
      
      tax = (int) (ch->gold * 0.15) ;
      
      sprintf( log_buf, "You have been charged %d coin%s as a fee for the out of bank protection of your gold while you are away.\n\r",
               tax, tax > 1 ? "s" : "" );
      send_to_char( AT_WHITE, log_buf, ch );
      ch->gold -= tax;
   }

    send_to_char(AT_BLUE, "[ The clear sky of reality slowly crosses the horizon.\n\r", ch );
    send_to_char(AT_BLUE, "  With much effort you tear yourself free of the realm, but\n\r", ch );
    send_to_char(AT_BLUE, "  you realize that you can only stay away for so long, you\n\r", ch );
    send_to_char(AT_BLUE, "  will return... ]\n\r\n\r", ch );
    send_to_char(C_DEFAULT, "", ch);
    act(AT_BLOOD, "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    if (ch->level <= 115)
      { 
       sprintf( log_buf, "%s has quit in room vnum %d.", ch->name, ch->in_room->vnum );
       log_string( log_buf, CHANNEL_LOG, ch->level - 1 );
       if ( !IS_SET( ch->act, PLR_WIZINVIS ) && !IS_AFFECTED( ch, AFF_INVISIBLE ) 
	    && !IS_AFFECTED2( ch, AFF_IMPROVED_INVIS ) 
	    && ((ch->desc==NULL) || (ch->desc->connected == CON_PLAYING)))
       {
       sprintf( log_buf, "%s has left the game.", ch->name );
       broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(spellof1.wav V=100 L=1 P=50 T=Logon)");
       }
      }     
       for ( ul = user_list; ul; ul = ul->next )
       {
         if ( !str_cmp( ch->name, ul->name ) )
         {
           sprintf( log_buf, "Last on: %s", ctime( &current_time ) );
	   *(log_buf + strlen(log_buf) - 1) = '\0';
	   free_string(ul->lastlogin);
           ul->lastlogin = str_dup( log_buf );
           ul->level = ch->level;
           ul->class = ch->class;
           ul->multi = ch->multied;
	   if (ch->desc)
	   {
     	   	free_string(ul->host);
	   	ul->host = str_dup( ch->desc->host );
	   	free_string(ul->user);
	   	ul->user = str_dup( ch->desc->user );
	   }
           break;
         }
       }

    /*
     * After extract_char the ch is no longer valid!
     */
    if ( auc_held && ch == auc_held && auc_obj )
    {
      if ( auc_bid )
      {
	if ( auc_bid->gold < auc_cost )
	{
	  sprintf(log_buf, "Holder of %s has left; bidder cannot pay for item; returning to owner.", auc_obj->short_descr);
	  obj_to_char( auc_obj, ch );
	}
	else
	{
	  sprintf(log_buf, "Holder of %s has left; selling item to last bidder.",
		  auc_obj->short_descr );
	  obj_to_char( auc_obj, auc_bid );
	  auc_bid->gold -= auc_cost;
	  ch->gold += auc_cost;
	}
      }
      else
      {
	sprintf(log_buf, "Holder of %s has left; removing item from auction.",
		auc_obj->short_descr );
	auc_channel( log_buf );
	obj_to_char( auc_obj, ch );
      }
      auc_obj = NULL;
      auc_bid = NULL;
      auc_held = NULL;
      auc_cost = 0;
      auc_count = -1;
    }

    if ( auc_bid && auc_bid == ch && auc_obj )
    {
      sprintf(log_buf, "Highest bidder for %s has left; returning item to owner.", auc_obj->short_descr );
      if ( auc_held )
	obj_to_char( auc_obj, auc_held );
      auc_channel( log_buf);
      auc_obj = NULL;
      auc_bid = NULL;
      auc_held = NULL;
      auc_cost = 0;
      auc_count = -1;
    }
	
	save_char_obj( ch, TRUE );
    for ( PeT = ch->in_room->people; PeT; PeT = PeT->next_in_room )
    {
       if ( IS_NPC( PeT ) )
	  if ( IS_SET( PeT->act, ACT_PET ) && ( PeT->master == ch ) )
	  {
	    extract_char( PeT, TRUE );
	    break;
          }
    }       
    d = ch->desc;

	#ifndef SQL_SYSTEM
		extract_char( ch, TRUE );
	#else
		d->character = NULL;
		d->original = NULL;
		ch->desc = NULL;
		unlink_char(ch);
	#endif

    if ( d )
	close_socket( d );

    return;
}

#ifdef SQL_SYSTEM

void unlink_char(CHAR_DATA* ch)
{
	ch->playing		= FALSE;
	ch->last_room	= ch->in_room;
	
	char_from_room(ch);

	if (ch == char_list)
	{
		char_list = ch->next;
	} else
	{
		CHAR_DATA* prev = NULL;
		for ( prev = char_list; prev; prev = prev->next )
		{
			if ( prev->next == ch )
			{
				prev->next = ch->next;
				break;
			 }
		}
	}
}

void link_char(CHAR_DATA* ch)
{
	ch->playing = TRUE;
	ch->in_room = ch->last_room;
	ch->last_room = NULL;
}

#endif

void do_save( CHAR_DATA *ch, char *argument )
{
	if ( IS_NPC( ch ) )
	return;

    if ( ch->level < 2 )
    {
	send_to_char(AT_WHITE, "You must be at least second level to save.\n\r", ch );
	return;
    }

    save_char_obj( ch, FALSE );

	send_to_char(AT_WHITE, "Ok.\n\r", ch );
    return;
}



void do_follow( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_DGREEN, "Follow whom?\n\r", ch );
	return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char(AT_DGREEN, "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master )
    {
	act(AT_DGREEN, "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( !ch->master )
	{
	    send_to_char(AT_DGREEN, "Silly...you already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower( ch );
	return;
    }

    if ( ( ch->level - victim->level < -8
	  || ch->level - victim->level >  8 )
	&& !IS_HERO( ch ) )
    {
	send_to_char(AT_DGREEN, "You feel unworthy to follow.\n\r", ch );
	return;
    }

    if ( ch->master )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}


void do_lose( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char(AT_GREEN, "Lose whom?\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world( ch, arg ) ) )
    {
        send_to_char(AT_GREEN, "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        if ( !ch->master )
        {
            send_to_char(AT_GREEN, "You can't lose yourself.\n\r", ch );
            return;
        }
        return;
    }

    if (victim->master != ch)
    {
        send_to_char(AT_GREEN, "That player is not following you.\n\r", ch);
        return;
    }

    /*  This code came from drop_follower, but it's modified a bit
        to make it more apropriate for the lose command */
    if ( !victim->master )
    {
        bug( "Stop_follower: null master.", 0 );
        return;
    }

    if ( IS_AFFECTED( victim, AFF_CHARM ) )
    {
        REMOVE_BIT( victim->affected_by, AFF_CHARM );
        affect_strip( victim, skill_lookup("charm person") );
        affect_strip( victim, skill_lookup("domination")   );
    }

    if ( can_see( victim->master, victim ) )
        act(AT_GREEN, "You lost $n.",
            victim, NULL, victim->master, TO_VICT );
    act(AT_GREEN, "$N doesn't want you following $M around, lose it buster.",
        victim, NULL, victim->master, TO_CHAR );

    victim->master = NULL;
    victim->leader = NULL;
    return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
  
    if ( ch->master )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act(AT_GREEN, "$n now follows you.", ch, NULL, master, TO_VICT );

    act(AT_GREEN, "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{

    if ( !ch->master )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, skill_lookup("charm person") );
	affect_strip( ch, skill_lookup("domination"	 ) );
    }

    if ( can_see( ch->master, ch ) )
	act(AT_GREEN, "$n stops following you.",
	    ch, NULL, ch->master, TO_VICT );
    act(AT_GREEN, "You stop following $N.",
	ch, NULL, ch->master, TO_CHAR );

    ch->master = NULL;
    ch->leader = NULL;
    return;
}



void die_follower( CHAR_DATA *ch, char *name )
{
    CHAR_DATA *fch;

    if ( ch->master )
	stop_follower( ch );

    ch->leader = NULL;

    for ( fch = char_list; fch; fch = fch->next )
    {
        if ( fch->deleted )
	    continue;
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = NULL;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    char       arg [ MAX_INPUT_LENGTH ];
    bool       found;
    bool       fAll;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char(AT_GREY, "Order whom to do what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char(AT_GREY, "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( !( victim = get_char_room( ch, arg ) ) )
	{
	    send_to_char(AT_GREY, "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char(AT_GREY, "Aye aye, right away!\n\r", ch );
	    return;
	}

	if ( !IS_AFFECTED( victim, AFF_CHARM ) || victim->master != ch )
	{
	    send_to_char(AT_GREY, "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och; och = och_next )
    {
        och_next = och->next_in_room;

        if ( och->deleted )
	    continue;

	if ( IS_AFFECTED( och, AFF_CHARM )
	    && och->master == ch
	    && ( fAll || och == victim ) )
	{
	    found = TRUE;
	    if (( argument[0] != 'm' && argument[1] != 'p' ))
	    {  /* Don't let them order mobs to perform mobprog commands! */
	      act( AT_GREY, "$n orders you to '$t'.", ch, argument, och,
	           TO_VICT );
	      interpret( och, argument );
	    }
	}
    }

    if ( found )
	send_to_char(AT_GREY, "Ok.\n\r", ch );
    else
	send_to_char(AT_GREY, "You have no followers here.\n\r", ch );
    return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    CHAR_DATA *group;
    char       buf [ MAX_STRING_LENGTH ];
    char       arg [ MAX_INPUT_LENGTH  ];
    int        member_count = 0;
	
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = ( ch->leader ) ? ch->leader : ch;
	sprintf( buf, "%s's group:\n\r", PERS( leader, ch ) );
	send_to_char(AT_DGREEN, buf, ch );

	for ( gch = char_list; gch; gch = gch->next )
	{
	    if ( gch->deleted )
	        continue;
	    if ( is_same_group( gch, ch ) )
	    {
		if (( gch->class != 9 )&&( gch->class != 11))
		  sprintf( buf,
		  "[%2d %s] %-16s %4d/%4d hp %4d/%4d mana %4d/%4d mv %5d xp\n\r",
			gch->level,
			IS_NPC( gch ) ? "Mob"
			              : class_table[gch->class].who_name,
			capitalize( PERS( gch, ch ) ),
			gch->hit,   gch->max_hit,
			gch->mana,  gch->max_mana,
			gch->move,  gch->max_move,
			gch->exp );
		else 
		  sprintf( buf,
		  "[%2d %s] %-16s %4d/%4d hp %4d/%4d bp %4d/%4d mv %5d xp\n\r",
			gch->level,
			IS_NPC( gch ) ? "Mob"
			              : class_table[gch->class].who_name,
			capitalize( PERS( gch, ch ) ),
			gch->hit,   gch->max_hit,
			gch->bp,    gch->max_bp,
			gch->move,  gch->max_move,
			gch->exp );
		send_to_char(AT_GREEN, buf, ch );
	    }
	}
	return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	send_to_char(AT_DGREEN, "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master || ( ch->leader && ch->leader != ch ) )
    {
	send_to_char(AT_DGREEN, "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act(AT_DGREEN, "$N isn't following you.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act(AT_GREEN, "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
	act(AT_GREEN, "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
	act(AT_GREEN, "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
	return;
    }

    for ( group = ch->in_room->people; group; group = group->next_in_room )
    {
        if ( group->deleted )
 	    continue;
	if ( is_same_group (group, ch) )
	    member_count++;
    }

    if (member_count >= MAX_GROUP)
    {
	act(AT_DGREEN, "$N cannot join your group, it is full!",  ch, NULL, victim, TO_CHAR       );
        act(AT_DGREEN, "$n's group is already full!", ch, NULL, victim, TO_VICT       );
        act(AT_DGREEN, "$N cannot join $n's group, it is full!",  ch, NULL, victim, TO_NOTVICT    );
        return;
    }

    if (   ch->level - victim->level < -8
	|| ch->level - victim->level >  8 )
    {
	act(AT_DGREEN, "$N cannot join your group.",  ch, NULL, victim, TO_CHAR       );
	act(AT_DGREEN, "You cannot join $n's group.", ch, NULL, victim, TO_VICT       );
	act(AT_DGREEN, "$N cannot join $n's group.",  ch, NULL, victim, TO_NOTVICT    );
	return;
    }

    victim->leader = ch;
    act(AT_GREEN, "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    act(AT_GREEN, "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act(AT_GREEN, "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;
    char       buf [ MAX_STRING_LENGTH ];
    char       arg [ MAX_INPUT_LENGTH  ];
    int        members;
    int        amount;
    int        share;
    int        extra;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char(AT_YELLOW, "Split how much?\n\r", ch );
	return;
    }
    
    amount = atoi( arg );

    if ( amount < 0 )
    {
	send_to_char(AT_YELLOW, "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount == 0 )
    {
	send_to_char(AT_YELLOW, "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->gold < amount )
    {
	send_to_char(AT_YELLOW, "You don't have that much gold.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
        if ( gch->deleted )
	    continue;
	if ( is_same_group( gch, ch ) )
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char(AT_YELLOW, "Just keep it all.\n\r", ch );
	return;
    }
	    
    share = amount / members;
    extra = amount % members;

    if ( share == 0 )
    {
	send_to_char(AT_YELLOW, "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->gold -= amount;
    ch->gold += share + extra;

    sprintf( buf,
	"You split %d gold coins.  Your share is %d gold coins.\n\r",
	amount, share + extra );
    send_to_char(AT_YELLOW, buf, ch );

    sprintf( buf, "$n splits %d gold coins.  Your share is %d gold coins.",
	amount, share );

    for ( gch = ch->in_room->people; gch; gch = gch->next_in_room )
    {
        if ( gch->deleted )
	    continue;
	if ( gch != ch && is_same_group( gch, ch ) )
	{
	    act(C_DEFAULT, buf, ch, NULL, gch, TO_VICT );
	    gch->gold += share;
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;
    char       buf [ MAX_STRING_LENGTH ];

    if ( argument[0] == '\0' )
    {
	send_to_char(AT_GREEN, "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->act, PLR_NO_TELL ) )
    {
	send_to_char(AT_GREEN, "Your message didn't get through!\n\r", ch );
	return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
    sprintf( buf, "%s tells the group '%s'.\n\r", ch->name, argument );
    for ( gch = char_list; gch; gch = gch->next )
    {
        if ( gch->deleted )
	    continue;
	if ( is_same_group( gch, ch ) )
	    send_to_char(AT_GREEN, buf, gch );
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->deleted || bch->deleted )
      return FALSE;
    if ( ach->leader ) ach = ach->leader;
    if ( bch->leader ) bch = bch->leader;
    if ( ach->deleted || bch->deleted )
      return FALSE;
    return ach == bch;
}

void do_pray( CHAR_DATA *ch, char *argument )
{
  if ( !IS_GOOD( ch ) )
  {
    send_to_char(AT_BLUE, "The Gods forgives some of your sins.\n\r", ch);
    ch->alignment += 100;
  }
  else
  {
    send_to_char( AT_BLUE, "The Gods frown at your greed.\n\r", ch );
    ch->alignment -= 100;
  }

  send_to_char( AT_GREY, "You fall to the ground, unconscious.\n\r", ch );
  act( AT_GREY, "$n falls to the ground, unconscious.", ch, NULL, NULL, TO_ROOM );
  STUN_CHAR( ch, 4, STUN_COMMAND );
  return;
}

/*
 * Generic channel function.
 * MAJOYRLY rewritten to allow languages by Tyrion, this is for all mobs
 * This routine does NOT handle PC INFO messages, the talk_channel_info
 * routine does, because languages over the INFO channel would be annoying.
 */

void talk_channel( CHAR_DATA *ch, char *argument, int channel, const char *verb, int language )
{
    DESCRIPTOR_DATA *d;
    char           buf [ MAX_STRING_LENGTH ];
    char	   buf2 [ MAX_STRING_LENGTH ];
    char	   buf3 [ MAX_STRING_LENGTH ];
    char	   buf4 [ MAX_STRING_LENGTH ];
    char	   buf5 [ MAX_STRING_LENGTH ];
    char	   buf6 [ MAX_STRING_LENGTH ];
    int		   bufcolour = C_DEFAULT;
    int		   position;
    int		   chance;
    int		   chance2;
    int		   receiver_chance;
    int		   speaker_chance;
    char		   *lan_str;
    char		   charname [ MAX_STRING_LENGTH];

    lan_str = lang_table[language].name;

    if( IS_NPC( ch ) )
    {
	ch->language[language] = 100;
    }
    chance = ch->language[language];
    speaker_chance = number_percent();

    if( chance  == 0 && !IS_NPC( ch ) )
    {
	sprintf(buf, "You don't know how to speak %s.\n\r", lan_str);
	send_to_char(AT_LBLUE, buf ,ch);
	return;
    }

    if( ( ch->speaking == COMMON ) && ( !IS_IMMORTAL( ch )) && ( !IS_NPC ( ch ) ) )
    {
        sprintf(buf, "Mortals can not speak %s.\n\r", lan_str );
        send_to_char(C_DEFAULT, buf, ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
	sprintf( buf, "%s what?\n\r", verb );
	buf[0] = UPPER( buf[0] );
	return;
    }

    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_SILENCE ) )
    {
	sprintf( buf, "You are silenced and can't %s.\n\r", verb );
	send_to_char(AT_WHITE, buf, ch );
	return;
    }
    
    if ( !IS_NPC( ch ) && IS_AFFECTED2( ch, AFF_SLIT ) )
    {
	sprintf( buf, "You are slit and can't %s.\n\r", verb );
	send_to_char(AT_WHITE, buf, ch );
	return;
    }
    
    if ( IS_SET( ch->in_room->room_flags, ROOM_SILENT ) 
         && (get_trust(ch)<106) )
    {
        send_to_char(AT_WHITE, "You are in a silent room.\n\r", ch );
        return;
    }

    REMOVE_BIT( ch->deaf, channel );

    argument = makedrunk(argument, ch );

  for ( d = descriptor_list; d; d = d->next )
  {
    CHAR_DATA *och;
    CHAR_DATA *vch;

    och = d->original ? d->original : d->character;
    vch = d->character;

    if( ch->position == POS_GHOST )
    {
        sprintf( charname, "The Ghost of %s", ch->name );
    }
    else
    {
        sprintf( charname, ch->name );
    }

    if( ( d->connected != CON_PLAYING || !can_see( och, ch ) ) && och != ch )
    {
	sprintf( charname, "Someone" );
    }
    if( IS_NPC( ch ) )
    {
	sprintf( charname, ch->short_descr );
    }

    switch ( channel )
    {
    default:
	sprintf( buf, "In %s, You %s '%s'\n\r", lan_str, verb, argument );
	sprintf( buf2, "In %s, %s %ss '%s'\n\r", lan_str, charname, verb, argument );
	sprintf( buf3, "In %s you try to %s '%s', but it doesn't sound correct.\n\r", lan_str, verb, argument );
	sprintf( buf4, "In a wierd form of %s, %s %ss something incomprehensible.\n\r", lan_str, charname, verb );
	sprintf( buf5, "In %s, %s %ss something you can't understand.\n\r", lan_str, charname, verb );
	sprintf( buf6, "%s %ss something in a strange tongue.\n\r", charname, verb );
	bufcolour = AT_LBLUE;
	break;
    case CHANNEL_MUTTER:
        sprintf( buf, "In %s, You %s '%s'\n\r", lan_str, verb, argument );
        sprintf( buf2, "In %s, %s %ss '%s'\n\r", lan_str, charname, verb, argument );
        sprintf( buf3, "In %s you try to %s '%s', but it doesn't sound correct.\n\r", lan_str, verb, argument );
        sprintf( buf4, "In a wierd form of %s, %s %ss something incomprehensible.\n\r", lan_str, charname, verb );
        sprintf( buf5, "In %s, %s %ss something you can't understand.\n\r", lan_str, charname, verb );
        sprintf( buf6, "%s %ss something in a strange tongue.\n\r", charname, verb );
        bufcolour = AT_CYAN;
        break;
    case CHANNEL_OOC:
        sprintf( buf, "In %s, You %s '%s'\n\r", lan_str, verb, argument );  
        sprintf( buf2, "In %s, %s %ss '%s'\n\r", lan_str, charname, verb, argument );
        sprintf( buf3, "In %s you try to %s '%s', but it doesn't sound correct.\n\r", lan_str, verb, argument );
        sprintf( buf4, "In a wierd form of %s, %s %s's something incomprehensible.\n\r", lan_str, charname, verb );
        sprintf( buf5, "In %s, %s %s's something you can't understand.\n\r", lan_str, charname, verb );
        sprintf( buf6, "%s %s's something in a strange tongue.\n\r", charname, verb );
        bufcolour = AT_GREEN;
        break;
    case CHANNEL_IMMTALK:
	sprintf( buf, "%s (%s): %s\n\r", ch->name, lan_str, argument );
	sprintf( buf2, "%s (%s): %s\n\r", charname, lan_str, argument );
	sprintf( buf3, "In %s you try to %s '%s', but it doesn't sound correct.\n\r", lan_str, verb, argument );
	sprintf( buf4, "%s (%s): something incomprehensible.\n\r", charname, lan_str );
	sprintf( buf5, "%s (%s): something you can't understand.\n\r", charname, lan_str );
	sprintf( buf6, "%s (unknown): something in a strange tongue.\n\r", charname );
	bufcolour = AT_YELLOW;
	break;
    case CHANNEL_GUILD:
	sprintf( buf, "[%s] %s (%s): %s\n\r", ( ch->guild != NULL) ? ch->guild->name : "NONE", ch->name, lan_str, argument );
	sprintf( buf2, "[%s] %s (%s): %s\n\r", ( ch->guild != NULL) ? ch->guild->name : "NONE", charname, lan_str, argument );
	sprintf( buf3, "In %s you try to %s '%s', but it doesn't sound correct.\n\r", lan_str, verb, argument );
	sprintf( buf4, "[%s] %s (%s): something incomprehensible.\n\r",( ch->guild != NULL) ? ch->guild->name : "NONE", charname, lan_str );
	sprintf( buf5, "[%s] %s (%s): something you can't understand.\n\r", ( ch->guild != NULL) ? ch->guild->name : "NONE", charname, lan_str );
	sprintf( buf6, "[%s] %s (unknown): something in a strange tongue.\n\r", ( ch->guild != NULL) ? ch->guild->name : "NONE", charname );
	bufcolour = ( ch->guild != NULL ? ch->guild->color : AT_RED );
	break;
    case CHANNEL_CLAN:
	sprintf( buf, "<%s> %s (%s): %s\n\r",
		( get_clan_index(ch->clan) && (get_clan_index(ch->clan))->name ?
		(get_clan_index(ch->clan))->name : "Unclanned"), ch->name, lan_str, argument );
	sprintf( buf2, "<%s> %s (%s): %s\n\r",
		( get_clan_index(ch->clan) && (get_clan_index(ch->clan))->name ?
		(get_clan_index(ch->clan))->name : "Unclanned"), charname, lan_str, argument );
	sprintf( buf3, "In %s you try to %s '%s', but it doesn't sound correct.\n\r", lan_str, verb, argument );
	sprintf( buf4, "<%s> %s (%s): something incomprehensible.\n\r", 
		( get_clan_index(ch->clan) && (get_clan_index(ch->clan))->name ?
		(get_clan_index(ch->clan))->name : "Unclanned"), charname, lan_str );
	sprintf( buf5, "<%s> %s (%s): something you can't understand.\n\r", 
		( get_clan_index(ch->clan) && (get_clan_index(ch->clan))->name ?
		(get_clan_index(ch->clan))->name : "Unclanned"), charname, lan_str );
	sprintf( buf6, "<%s> %s (unknown): something in a strange tongue.\n\r",
		( get_clan_index(ch->clan) && (get_clan_index(ch->clan))->name ?
		(get_clan_index(ch->clan))->name : "Unclanned"), charname );
	bufcolour = AT_RED;
	break;
    case CHANNEL_GUARDIAN:
	sprintf( buf, "%s (%s)> &P%s\n\r", ch->name, lan_str, argument );
	sprintf( buf2, "%s (%s)> &P%s\n\r", charname, lan_str, argument );
	sprintf( buf3, "In %s you try to %s '%s', but it doesn't sound correct.\n\r", lan_str, verb, argument );
	sprintf( buf4, "%s (%s)> &Psomething incomprehensible.\n\r", charname, lan_str );
	sprintf( buf5, "%s (%s)> &Psomething you can't understand.\n\r", charname, lan_str );
	sprintf( buf6, "%s (unknown)> &Psomething in a strange tongue.\n\r", charname );
	bufcolour = AT_PURPLE;
	break;
    case CHANNEL_GRATZ:
	sprintf( buf, "<Gratz> %s (%s) %s\n\r", ch->name, lan_str, argument );
	sprintf( buf2, "<Gratz> %s (%s) %s\n\r", charname, lan_str, argument );
	sprintf( buf3, "In %s you try to %s '%s', but it doesn't sound correct.\n\r", lan_str, verb, argument );
	sprintf( buf4, "<Gratz> %s (%s) something incomprehensible.\n\r", charname, lan_str );
	sprintf( buf5, "<Gratz> %s (%s) something you can't understand.\n\r", charname, lan_str );
	sprintf( buf6, "<Gratz> %s (unknown) something in a strange tongue.\n\r", charname );
	bufcolour = AT_LBLUE;
	break;
    case CHANNEL_TIMELORD:
	sprintf( buf, "%s (%s) <TL> &b%s\n\r", ch->name, lan_str, argument );
	sprintf( buf2, "%s (%s) <TL> &b%s\n\r", charname, lan_str, argument );
	sprintf( buf3, "In %s you try to %s '%s', but it doesn't sound correct.\n\r", lan_str, verb, argument );
	sprintf( buf4, "%s (%s) <TL> &bsomething incomprehensible.\n\r", charname, lan_str );
	sprintf( buf5, "%s (%s) <TL> &bsomething you can't understand.\n\r", charname, lan_str );
	sprintf( buf6, "%s (unknown) <TL> &bsomething in a strange tongue.\n\r", charname );
	bufcolour = AT_RED;
	break;
    case CHANNEL_CLASS:
	sprintf( buf, "{CLASS} %s (%s): %s\n\r", ch->name, lan_str, argument );
	sprintf( buf2, "{CLASS} %s (%s): %s\n\r", charname, lan_str, argument );
	sprintf( buf3, "In %s you try to %s '%s', but it doesn't sound correct.\n\r", lan_str, verb, argument );
	sprintf( buf4, "{CLASS} %s (%s): something incomprehensible.\n\r", charname, lan_str );
	sprintf( buf5, "{CLASS} %s (%s): something you can't understand.\n\r", charname, lan_str );
	sprintf( buf6, "{CLASS} %s (unknown): something in a strange tongue.\n\r", charname );
	bufcolour = AT_LBLUE;
	break;
    case CHANNEL_DEMIGOD:
	sprintf( buf, "&z<Demi-God> &w%s (&z%s&w): %s\n\r", ch->name, lan_str, argument );
	sprintf( buf2, "&z<Demi-God> &w%s (&z%s&w): %s\n\r", charname, lan_str, argument );
	sprintf( buf3, "&wIn &z%s &wyou try to %s '%s', but it doesn't sound correct.\n\r", lan_str, verb, argument );
	sprintf( buf4, "&z<Demi-God> &w%s (&z%s&w): something incomprehensible.\n\r", charname, lan_str );
	sprintf( buf5, "&z<Demi-God> &w%s (&z%s&w): something you can't understand.\n\r", charname, lan_str );
	sprintf( buf6, "&z<Demi-God> &w%s (&zunknown&w): something in a strange tongue.\n\r", charname );
	bufcolour = AT_GREY;
	break;
    case CHANNEL_CHAMPION:
	sprintf( buf, "&g(Champion) &G%s (&g%s&G): %s\n\r", ch->name, lan_str, argument );
	sprintf( buf2, "&g(Champion) &G%s (&g%s&G): %s\n\r", charname, lan_str, argument );
	sprintf( buf3, "&GIn &g%s&G you try to %s '%s', but it doesn't sound correct.\n\r", lan_str, verb, argument );
	sprintf( buf4, "&g(Champion) &G%s (&g%s&G): something incomprehensible.\n\r", charname, lan_str );
	sprintf( buf5, "&g(Champion) &G%s (&g%s&G): something you can't understand.\n\r", charname, lan_str );
	sprintf( buf6, "&g(Champion) &G%s (&gunknown&G): something in a strange tongue.\n\r", charname );
	bufcolour = AT_GREEN;
	break;
    }
	if ( d->connected == CON_PLAYING
	    && vch != ch
	    && !IS_SET( och->deaf, channel ) 
	    && !IS_SET( och->in_room->room_flags, ROOM_SILENT ) )
	{
	    if ( channel == CHANNEL_IMMTALK && !IS_IMMORTAL( och ) )
		continue;
	    if ( channel == CHANNEL_CHAMPION && !IS_HERO( och ) )
		continue;
	    if ( channel == CHANNEL_DEMIGOD && ( och->level < LEVEL_DEMIGOD ) )
		continue;
	    if ( channel == CHANNEL_GUARDIAN && get_trust( och ) < L_DIR )
		continue;
	    if ( channel == CHANNEL_TIMELORD && get_trust( och ) < L_CON )
		continue;
	    if ( channel == CHANNEL_CLASS && vch->class != ch->class )
		continue;
	    if( channel == CHANNEL_GUILD && vch->guild != ch->guild )
		continue;
	    if ( channel == CHANNEL_CLAN && vch->clan != ch->clan )
		continue;
	    if ( channel == CHANNEL_YELL && vch->in_room->area != ch->in_room->area )
		continue;

	    position		= vch->position;
	    if ( channel != CHANNEL_SHOUT && channel != CHANNEL_YELL )
		vch->position	= POS_STANDING;
     
	    vch->position	= position;

            if( IS_NPC( och ) )
            {
                och->language[language] = 100;
            }
            chance2 = och->language[language]; 
	    receiver_chance = number_percent ( );

	    if( speaker_chance <= chance )
	    {
		if( chance2 == 0 )
		{
		    send_to_char( bufcolour, buf6, och );
		    if( IS_AFFECTED4( och, AFF_TONGUES ) )
		    {
			send_to_char( bufcolour, "Your tongues spell translates the speech, and you understand the following:\n\r", och );
			send_to_char( bufcolour, buf2, och );
		    }
		}
		else
		{
		    if( receiver_chance <= chance2 )
		    {
			if( och->position >= POS_RESTING )
			send_to_char( bufcolour, buf2, och );
		    }
		    else
		    {
			send_to_char( bufcolour, buf5, och );
			if( IS_AFFECTED4( och, AFF_TONGUES ) )
			{
			    send_to_char( bufcolour, "Your tongues spell translates the speech, and you understand the following:\n\r", och );
			    send_to_char( bufcolour, buf2, och );
			}

		    }
		}
	    }
	    else
	    {
		if( chance2 == 0)
		{
		    send_to_char( bufcolour, buf6, och );
		    if( IS_AFFECTED4( och, AFF_TONGUES ) )
		    {
			send_to_char( bufcolour, "Your tongues spell translates the speech, and you understand the following:\n\r", och );
			send_to_char( bufcolour, buf2, och );
		    }
		}
		else
		{
		    if( receiver_chance <= chance2 )
		    {
			send_to_char(bufcolour, buf4, och );
			if( IS_AFFECTED4( och, AFF_TONGUES ) )
			{
			    send_to_char( bufcolour, "Your tongues spell translates the speech, and you understand the following:\n\r", och );
			    send_to_char( bufcolour, buf2, och );
			}
		    }
		    else
		    {
			send_to_char(bufcolour, buf5, och );
			if( IS_AFFECTED4( och, AFF_TONGUES ) )
			{
			    send_to_char( bufcolour, "Your tongues spell translates the speech, and you understand the following:\n\r", och );
			    send_to_char( bufcolour, buf2, och );
			}
                    }
		}
	    }
	}
    }
    if( speaker_chance <= chance )
    {
	send_to_char( bufcolour, buf, ch );
    }
    else
    {
	send_to_char( bufcolour, buf3, ch );
    }

    return;
}

/*
 * Info channel Broadcast.  Has SOUND argument.  Used only in generated 
 * messages, IE, [INFO]: Player has risen to level 5!  A player does not
 * "talk" on this routine.
 */
void broadcast_channel( CHAR_DATA *ch, char *argument, int channel, const char *sound)
{
    DESCRIPTOR_DATA *d;
    char             buf [ MAX_STRING_LENGTH ];
    int              position;

    for ( d = descriptor_list; d; d = d->next )
    {
	CHAR_DATA *och;
	CHAR_DATA *vch;

	och = d->original ? d->original : d->character;
	vch = d->character;

	if ( d->connected == CON_PLAYING
	    && !IS_SET( och->deaf, channel ) 
	    && !IS_SET( och->in_room->room_flags, ROOM_SILENT ) )
	{

	    position		= vch->position;

	    if(!IS_NPC( och ) && IS_SET( och->act, PLR_SOUND ) )
	    {
		sprintf(buf, "&b[&CINFO&b]:&B %s\n\r %s\n\r", argument, sound );
	    }
	    else
	    {
		sprintf(buf, "&b[&CINFO&b]:&B %s\n\r", argument );
	    }
	    send_to_char( AT_BLUE, buf, och );
	}
    }

    return;
}

void broadcast_room( CHAR_DATA *ch, char *argument, const char *sound )
{
    char	buf [ MAX_STRING_LENGTH ];
    CHAR_DATA   *och;

    for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
    {
	if( och == ch )
           continue;

	if(!IS_NPC( och ) && IS_SET( och->act, PLR_SOUND ) )
	{
	    sprintf(buf, "%s %s\n\r", argument, sound );
	}
	else
	{
	    sprintf(buf, "%s\n\r", argument );
	}
	send_to_char( AT_BLUE, buf, och );
    }

    return;
}
