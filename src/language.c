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
/* $Id: language.c,v 1.15 2004/11/04 14:48:10 ahsile Exp $ */

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
 * Lookup a language by name.
 */
int lang_lookup( const char *name )
{
    int ln;

    for ( ln = 0; ln < MAX_LANGUAGE; ln++ )
    {
        if ( !lang_table[ln].name )
            break;
        if ( LOWER( name[0] ) == LOWER( lang_table[ln].name[0] )
            && !str_prefix( name, lang_table[ln].name ) )
            return ln;
    }

    return -1;
}

void do_speak( CHAR_DATA *ch, char *argument )
{
	char	arg [ MAX_INPUT_LENGTH ];
	char	buf [ MAX_STRING_LENGTH];
	int	speaking;
	int	canspeak;

	argument = one_argument(argument, arg);

	buf[0] = '\0';


	if( ch->position == POS_GHOST )
	{
		send_to_char( AT_LBLUE, "You can not speak anything but spiritspeak while dead.\n\r", ch );
		ch->speaking = SPIRITSPEAK;
		return;
	}
	if (arg[0] == '\0')
	{
		sprintf (buf, "You currently speak %s.\n\r", lang_table[ch->speaking].name);
		send_to_char (AT_LBLUE, buf, ch);
	}
	else
	{
		if ((speaking = lang_lookup(arg)) != -1)
		{
			if ((canspeak = ch->language[speaking]) == 0 && !IS_NPC( ch ) && ch->position != POS_DEAD && !IS_IMMORTAL( ch ) ) 
			{
				sprintf (buf, "But you don't know how to speak %s.\n\r", lang_table[speaking].name);
				send_to_char(AT_LBLUE, buf, ch);
			}
			else
			{
				ch->speaking = speaking;
				sprintf (buf, "You will speak %s from now on.\n\r", lang_table[ch->speaking].name);
				send_to_char(AT_LBLUE, buf, ch);
			}
		}
		else
		{
			sprintf (buf, "%s is not a valid language!\n\r", arg);
			send_to_char(AT_LBLUE,  buf, ch);
		}
	}
}
		
void do_lset( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    CHAR_DATA *victim;
    char       arg1  [ MAX_INPUT_LENGTH ];
    char       arg2  [ MAX_INPUT_LENGTH ];
    char       arg3  [ MAX_INPUT_LENGTH ];
    int        value;
    int        ln;
    bool       fAll;

    rch = get_char( ch );

    if ( !authorized( rch, "lset" ) )
        return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char(AT_WHITE,  "Syntax: lset <victim> <lang> <value>\n\r",	ch );
        send_to_char(AT_WHITE,  "or:     lset <victim> all    <value>\n\r",	ch );
	send_to_char(AT_WHITE,  "or:     lset <victim> learn  <value>\n\r",	ch );
        send_to_char(AT_WHITE,  "Lang being any language.\n\r",			ch );
        return;
    }

    if ( !( victim = get_char_world( ch, arg1 ) ) )
    {
        send_to_char(AT_WHITE,  "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char(AT_WHITE,  "Not on NPC's.\n\r", ch );
        return;
    }

    if (!str_prefix(arg2, "learn") )
    {
	    if (!is_number(arg3))
	    {
		send_to_char (AT_WHITE, "Value must be numeric.\n\r", ch);
		return;
	    }
	    value = atoi(arg3);

	    if (value < 0)
		    value = 0;
	    if (value > 100)
		    value = 100;
	    victim->pcdata->learn = value;
	    send_to_char(AT_WHITE, "Ok.\n\r", ch);
	    return;
    }

    fAll = !str_cmp( arg2, "all" );
    ln   = 0;
    if ( !fAll && ( ln = lang_lookup( arg2 ) ) < 0 )
    {
        send_to_char(AT_WHITE,  "No such language.\n\r", ch );
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
        send_to_char(AT_WHITE,  "Value must be numeric.\n\r", ch );
        return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
        send_to_char(AT_WHITE,  "Value range is 0 to 100.\n\r", ch );
        return;
    }

    if ( fAll )
    {
        if ( get_trust( ch ) < L_SEN )
        {
            send_to_char(AT_WHITE,  "Only Seniors may lset all.\n\r", ch );
            return;
        }
        for ( ln = 0; ln < MAX_LANGUAGE; ln++ )
            if ( lang_table[ln].name )
                victim->language[ln] = value;
    }
    else
    {
        victim->language[ln] = value;
    }
    return;
}

void do_lstat( CHAR_DATA *ch, char *argument )
/* lstat by Maniac && Canth */
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int ln;
    int col;

    one_argument( argument, arg );
    col = 0;

    if ( arg[0] == '\0' )
    {
        send_to_char(AT_WHITE, "Lstat whom\n\r?", ch);
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char(AT_WHITE, "That person isn't logged on.\n\r", ch);
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char(AT_WHITE, "Not on NPC's.\n\r", ch);
        return;
    }

    buf2[0] = '\0';

    for ( ln = 0; ln < MAX_LANGUAGE ; ln++ )
    {
        if ( lang_table[ln].name == NULL )
            break;
        sprintf( buf1, "%18s %3d %% ", lang_table[ln].name,
                victim->language[ln] );
        strcat( buf2, buf1 );
        if ( ++col %3 == 0 )
            strcat( buf2, "\n\r" );
    }
    if ( col % 3 != 0 )
         strcat( buf2, "\n\r" );
    sprintf( buf1, "%s has %d learning sessions left.\n\r", victim->name,
                victim->pcdata->learn );
    strcat( buf2, buf1 );

    send_to_char(AT_WHITE,  buf2, ch );
    return;

}

void do_learn( CHAR_DATA *ch, char *argument )
{
    char buf  [ MAX_STRING_LENGTH   ];
    char buf1 [ MAX_STRING_LENGTH*2 ];
    int  ln;
    int money;

    if( ch->level > 5 )
    {
	money = ch->level * 20;
    }
    else
    {
	money = ch->level * 5;
    }

    if ( IS_NPC( ch ) )
        return;

    buf1[0] = '\0';

    if ( argument[0] == '\0' )
    {
        CHAR_DATA *mob;
        int        col;

        for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
        {
            if ( mob->deleted )
                continue;
            if ( IS_NPC( mob ) && IS_SET( mob->act, ACT_TEACHER ) )
                break;
        }

        col    = 0;
        for ( ln = 0; ln < MAX_LANGUAGE; ln++ )
        {
            if ( !lang_table[ln].name )
                break;

            if ( ( mob ) || ( ch->language[ln] > 0 ) )
            {
                sprintf( buf, "%18s %3d%%  ",
                        lang_table[ln].name, ch->language[ln] );
                strcat( buf1, buf );
                if ( ++col % 3 == 0 )
                    strcat( buf1, "\n\r" );
            }
        }

        if ( col % 3 != 0 )
            strcat( buf1, "\n\r" );

        sprintf( buf, "You have %d learning sessions left.\n\r",
                ch->pcdata->learn);
        strcat( buf1, buf );
        sprintf( buf, "Cost of lessons is %d gold coins.\n\r", money );
        strcat( buf1, buf );
        send_to_char(AT_WHITE,  buf1, ch );
    }
    else
    {
        CHAR_DATA *mob;
        int        adept;

        if ( !IS_AWAKE( ch ) )
        {
            send_to_char(AT_WHITE,  "In your dreams, or what?\n\r", ch );
            return;
        }

        for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
        {
            if ( mob->deleted )
                continue;
            if ( IS_NPC( mob ) && IS_SET( mob->act, ACT_TEACHER ) )
                break;
        }

        if ( !mob )
        {
            send_to_char(AT_WHITE,  "You can't do that here.\n\r", ch );
            return;
        }

        if ( ch->pcdata->learn <= 0 )
        {
            send_to_char(AT_WHITE,  "You have no lessons left.\n\r", ch );
            return;
        }
        else if ( money > ch->gold )
        {
            send_to_char(AT_WHITE,  "You don't have enough money to take lessons.\n\r", ch );
            return;
        }

        if (( ln = lang_lookup( argument ) ) < 0)
        {
            send_to_char(AT_WHITE,  "That's not a language.\n\r", ch );
            return;
        }

        adept = (int) (get_curr_int(ch) * 0.5);	/* Max learned = int/2 */
	if ( ch->level < LEVEL_HERO )		/* Max is 95% */
	{
	    adept = 95;
        }
	if ( ch->level >= LEVEL_HERO )		/* Max is 100% for champion + */
	    adept = 100;

        if ( ch->language[ln] >= adept )
        {
            sprintf( buf, "You are already fluent in %s.\n\r",
                lang_table[ln].name );
	    ch->language[ln] = adept;
            send_to_char(AT_WHITE,  buf, ch );
        }
        else
        {
            ch->pcdata->learn--;
            ch->gold                -= money;
            ch->language[ln] += int_app[get_curr_int( ch )].learn;
            if ( ch->language[ln] < adept )
            {
                act(AT_WHITE,  "You take lessons in $T.",
                    ch, NULL, lang_table[ln].name, TO_CHAR );
                act(AT_WHITE,  "$n practices $T.",
                    ch, NULL, lang_table[ln].name, TO_ROOM );
            }
            else
            {
                ch->language[ln] = adept;
                act(AT_WHITE,  "You are now fluent in $T.",
                    ch, NULL, lang_table[ln].name, TO_CHAR );
                act(AT_WHITE,  "$n is now fluent in $T.",
                    ch, NULL, lang_table[ln].name, TO_ROOM );
            }
        }
    }
    return;
}

void do_common( CHAR_DATA *ch, char *argument)
{
	do_language(ch, argument, COMMON);
}

void do_feline( CHAR_DATA *ch, char *argument)
{
	do_language(ch, argument, FELINE);
}

void do_canine( CHAR_DATA *ch, char *argument)
{
	do_language(ch, argument, CANINE);
}

void do_human( CHAR_DATA *ch, char *argument)
{
	do_language(ch,argument, HUMAN);
}

void do_dwarvish( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, DWARVISH);
}

void do_elvish( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, ELVISH);
}

void do_gnomish( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, GNOMISH);
}

void do_dragon( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, DRAGON);
}

void do_demon( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, DEMON);
}

void do_angel( CHAR_DATA *ch, char *argument)
{
	do_language(ch,argument, ANGEL);
}

void do_shadow_speak( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, SHADOW_SPEAK);
}

void do_magick( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, MAGICK);
}

void do_spiritspeak( CHAR_DATA *ch, char *argument )
{
	do_language(ch,argument, SPIRITSPEAK );
}

void do_satanic( CHAR_DATA *ch, char *argument )
{
        do_language(ch,argument, SATANIC );
}

void do_enlightened( CHAR_DATA *ch, char *argument )
{
        do_language(ch,argument, ENLIGHTENED );
}

void do_animalspeak( CHAR_DATA *ch, char *argument )
{
	do_language(ch,argument, ANIMALSPEAK );
}

void do_bretonnian( CHAR_DATA *ch, char *argument )
{
	do_language(ch,argument, BRETONNIAN );
}

void do_orcish( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, ORCISH);
}

void do_ogre( CHAR_DATA *ch, char *argument)
{
	do_language(ch,argument, OGRE);
}

void do_drow( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, DROW);
}

void do_elder( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, ELDER);
}

void do_pixie( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, PIXIE);
}

void do_hobbit( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, HOBBIT);
}

void do_minotaur( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, MINOTAUR);
}

void do_lizard( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, LIZARD);
}

void do_halfling( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, HALFLING);
}

void do_gargoyle( CHAR_DATA *ch, char *argument)
{
        do_language(ch,argument, GARGISH);
}
/* ========== Language ======================== */
void do_language( CHAR_DATA *ch, char *argument, int language)
{
	CHAR_DATA	*och;
	int		chance = 0;
	int		chance2 = 0;
	int		receiver_chance;
        int		speaker_chance;
	char		*lan_str;
	char		charname [ MAX_STRING_LENGTH];
	char		buf	[ MAX_STRING_LENGTH ];
	char		buf2	[ MAX_STRING_LENGTH ];
	char		buf3	[ MAX_STRING_LENGTH ];
	char		buf4	[ MAX_STRING_LENGTH ];
	char		buf5	[ MAX_STRING_LENGTH ];
	char		buf6	[ MAX_STRING_LENGTH ];
        char            buf7  [ MAX_STRING_LENGTH ];
     	int		bufcolour = AT_DGREY;

	if( IS_NPC( ch ) )
	{
	    ch->language[language] = 100;
	}
	chance = ch->language[language];
	speaker_chance = number_percent();

        if( ch->position == POS_GHOST )
        {
	    language = SPIRITSPEAK;
	    speaker_chance = 100;
	    chance = 100;
	    ch->speaking = SPIRITSPEAK;
	    bufcolour = AT_LBLUE;
        }
	lan_str = lang_table[language].name;

	buf[0] = '\0';
	if( chance == 0 && !IS_NPC( ch ) && ch->position != POS_GHOST )
	{
		sprintf(buf, "You don't know how to speak %s.\n\r", lan_str);
		send_to_char(C_DEFAULT, buf ,ch);
		return;
	}

        if( ch->speaking == COMMON && !IS_IMMORTAL( ch ) && !IS_NPC( ch ) )
        {
		sprintf(buf, "Mortals can not speak %s.\n\r", lan_str );
		send_to_char(C_DEFAULT, buf, ch );
		return;
	}

	if(argument[0] == '\0')
	{
		buf[0] = '\0';
		sprintf(buf, "Say WHAT in %s ??\n\r", lan_str);
		send_to_char(C_DEFAULT, buf ,ch);
		return;
	}

	if (!IS_NPC(ch))
		argument = makedrunk(argument,ch);

      sprintf( buf, "In %s, You say &W'%s'\n\r", lan_str, argument );
      sprintf( buf3, "In %s you try to say &W'%s'", lan_str, argument );
      sprintf( buf7, ", but it doesn't sound correct.\n\r" );

  for(och = ch->in_room->people; och != NULL; och = och->next_in_room )
  {
      if( och == ch )
	continue;

      sprintf( charname, ch->name );

      if( ( !can_see( och, ch ) ) && och != ch )
      {
	    sprintf( charname, "Someone" );
      }
      if( IS_NPC( ch ) )
      {
	    sprintf( charname, ch->short_descr );
      }

      if( IS_NPC( och ) )
      {
	    och->language[language] = 100;
      }
      chance2 = och->language[language];
      receiver_chance = number_percent( );
      if(ch->position != POS_GHOST )
      {
            sprintf( buf2, "In %s, %s says &W'%s'\n\r", lan_str, charname, argument );
            sprintf( buf4, "In a wierd form of %s, %s says something incomprehensible.\n\r", lan_str, charname );
            sprintf( buf5, "In %s, %s says something you can't understand.\n\r", lan_str, charname );
            sprintf( buf6, "%s says something in a strange tongue.\n\r", charname );
      }
      else
      {
            sprintf( buf2, "In %s, The Ghost of %s says &W'%s'\n\r", lan_str, charname, argument );
            sprintf( buf4, "In a wierd form of %s, The Ghost of %s says something incomprehensible.\n\r", lan_str, charname );
            sprintf( buf5, "In %s, The Ghost of %s says something you can't understand.\n\r", lan_str, charname );
            sprintf( buf6, "The Ghost of %s says something in a strange tongue.\n\r", charname );
      }

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
    if( speaker_chance <= chance )
    {
	send_to_char( bufcolour, buf, ch );
    }
    else
    {
        send_to_char( bufcolour, buf3, ch );
        send_to_char( bufcolour, buf7, ch );
    }
  return;
}
