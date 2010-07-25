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
/*$Id: act_multi.c,v 1.7 2005/03/28 04:01:58 tyrion Exp $*/

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

void do_remortalize( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *classmaster;
    OBJ_DATA* obj;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    char buf [ MAX_STRING_LENGTH ];
    char arg [ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char *temp_arg;
    int 	mClass;
    int		found = 0;
    int remortCost;
    int qpCost;
    bool pkRemort;

    if ( IS_NPC( ch ) )
    {
	return;
    }

    for ( classmaster = ch->in_room->people; classmaster; classmaster = classmaster->next_in_room )
    {
	if ( IS_NPC( classmaster ) && IS_SET( classmaster->act, ACT_CLASSMASTER ) )
	    break;
    }

    if ( !classmaster )
    {
	send_to_char(C_DEFAULT, "You can't do that here.\n\r", ch );
	return;
    }

    if ( ch->multied != ch->class )
    {
	send_to_char(C_DEFAULT, "You are already multiclassed.\n\r", ch );
	return;
    }

    if ( ch->level < LEVEL_HERO )
    {
	send_to_char(C_DEFAULT, "You are not experienced enough.\n\r", ch );
	return;
    }

    if ( ch->level < LEVEL_DEMIGOD )
    {
	/* Add race multiclass here */
    }


    temp_arg = one_argument( argument, arg );
    one_argument( temp_arg, arg2 );

    if ( arg[0] == '\0' )
    {
	send_to_char(C_DEFAULT, "Remort to which class?\n\r", ch );
	return;
    }

    for ( mClass = 0; mClass < MAX_CLASS; mClass++ )
    {
	if ( (!str_prefix( arg, class_table[mClass].who_long ) ) ||
	     (!str_prefix( arg, class_table[mClass].who_name ) ) )
	{

	    found = mClass;
	    break;
        }
    }

    if ( mClass == MAX_CLASS )
    {
	send_to_char( C_DEFAULT, "That's not a class.\n\r", ch );
	return;
    }

    if (ch->class == found )
    {
	send_to_char( C_DEFAULT, "You are already that class.\n\r", ch );
	return;
    }

    if (!( str_prefix( temp_arg, "pkiller") || str_prefix( temp_arg,"peaceful" )))
    {
      send_to_char (AT_RED, "Do you want to remort as a pkiller?  \n\r", ch);
      send_to_char(C_DEFAULT, "Please type: \n\rREMORTILIZE <class> pkiller\n\r", ch);
      send_to_char(C_DEFAULT, "Or:\n\rREMORTILIZE <class> peaceful\n\r", ch);
      return;
    }
    else if (!str_prefix (temp_arg, "pkiller"))
    {
      send_to_char(AT_RED, "We're going PK!\n\r", ch);

      pkRemort = 1;
    }
    else if(!str_prefix (temp_arg, "peaceful"))
    {
      send_to_char(AT_BLUE, "Peaceful is fine for those weak at heart.\n\r",ch);
      pkRemort = 0;
    }
    else
    {
      send_to_char (AT_RED, "Do you want to remort as a pkiller?  \n\r", ch);
      send_to_char(C_DEFAULT, "Please type: \n\rREMORTILIZE <class> pkiller\n\r", ch);
      send_to_char(C_DEFAULT, "Or:\n\rREMORTILIZE <class> peaceful\n\r", ch);
      return;
    }

    remortCost = 100000;
    if (pkRemort != ch->pkill)
    {
        remortCost = 250000;  /*make it more expensive if they are changing pk status --Manaux*/ 
    }

    if ( ( ch->gold < remortCost ) )
    {
      if (ch->pkill == pkRemort)
      {
        send_to_char(C_DEFAULT, "You do not have enough money, you need 100000 gold.\n\r", ch );
	return;
      }
      else
      {
        send_to_char(C_DEFAULT, "You do not have enough money, you need 250000 gold.\n\r(Changing pk status costs an additional 150000 million gold.)\n\r", ch);
        return;
      }
    }

    qpCost = 500;
    if ( ch->pkill != pkRemort )
    {
        qpCost = 750;
    }

    if ( (ch->questpoints < qpCost ) )
    {
      if (ch->pkill == pkRemort)     /*Case for not changing pk status */
      {
	  send_to_char(C_DEFAULT, "You do not have enough quest points, you need 500.\n\r", ch );
	  return;
      }
      else                          /*Else, case for changing pk status*/
      {
        send_to_char (C_DEFAULT, "You do not have enough quest points, you need 750. \n\r(Changing pkill status costs an additional 250 qp)", ch); 
        return;
      }
    }

    /* Ok, now that everything is checked, set the dude up. */

    /* First thing ya have to do is take off all his clothes. */

    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
	if( obj->wear_loc != WEAR_NONE)
	   unequip_char( ch, obj);
    }

    /* Ya know? I forgot this, strip the buggers stuff affects. */

    for( paf = ch->affected; paf; paf = paf_next )
    {
	paf_next = paf->next;
	affect_remove( ch, paf );
    }
    for( paf = ch->affected2; paf; paf = paf_next )
    {
	paf_next = paf->next;
	affect_remove2( ch, paf );
    }
    for( paf = ch->affected3; paf; paf = paf_next )
    {
	paf_next = paf->next;
	affect_remove3( ch, paf );
    }
    for( paf = ch->affected4; paf; paf = paf_next )
    {
	paf_next = paf->next;
	affect_remove4( ch, paf );
    }
    for( paf = ch->affected_powers; paf; paf = paf_next )
    {
        paf_next = paf->next;
        affect_remove_powers( ch, paf );
    }
    for( paf = ch->affected_weaknesses; paf; paf = paf_next )
    {
        paf_next = paf->next;
        affect_remove_weaknesses( ch, paf );
    }
    
    ch->affected_by = 0;
    ch->affected_by2 &= CODER;
    ch->affected_by3 = 0;
    ch->affected_by4 = 0;
    ch->affected_by_powers = 0;
    ch->affected_by_weaknesses = 0;
    ch->shields = 0;

    send_to_char(C_DEFAULT, "All your affects have been removed.\n\r", ch );
    send_to_char(C_DEFAULT, "All of your religious affects have been removed.\n\r", ch );

    ch->gold -= remortCost;
    ch->questpoints -= qpCost;

    ch->pkill = pkRemort;
    ch->multied = found;
    ch->level = 2;
    ch->max_hit = 1500;
    ch->hit = 1500;
    if (ch->class != 9 && ch->class != 11 )
    {
	ch->mana = 750;
	ch->max_mana = 750;
    }
    if (ch->class == 9 || ch->class == 11 )
    {
	ch->bp = 250;
	ch->max_bp = 250;
    }
    ch->exp = 2000;
    ch->move = 110;
    ch->max_move = 110;

    sprintf( log_buf, "%s has multiclassed to %s!", ch->name, class_table[ch->multied].who_long );
    broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(welcome2.wav V=100 L=1 P=100 T=Info)");

    send_to_char(C_DEFAULT, "Your equipment has been removed.\n\r", ch );

    SendMultiMessage( ch );

    sprintf (buf, "$n has chosen to multiclass to %s.",
	class_table[ found ].who_long );
    act(C_DEFAULT, buf, ch, NULL, NULL, TO_ROOM );

    return;
}

void SendMultiMessage( CHAR_DATA* ch )
{
    char 		buf [ MAX_STRING_LENGTH ];

    switch( GET_CLASS( ch ) )
	{
	    case CLASS_MAGE :
		sprintf( buf, "\n\rYour arcane skills seem to leave you, but you "
			      "know that they will flow\n\ragain some day." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_CLERIC :
		sprintf( buf, "\n\rThe calls of the gods become a whisper, but their "
			      "teachings will be\n\rwith you for all enternity." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_THIEF :
		sprintf( buf, "\n\rYou realize that your seedy life is over, but the "
			      "things you have learned\n\rwill remain within you." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_WARRIOR :
		sprintf( buf, "\n\rYou decide to follow the way of the sword no "
			      "longer, but are confident\n\rthat your skills will "
			      "return in time." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_PSIONICIST :
		sprintf( buf, "\n\rYour journey of the mind has come to an end, but "
			      "you feel that your training\n\rwill return one day." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_DRUID :
		sprintf( buf, "\n\rThe voices of nature fade off, but the last message "
			      "you hear is that they\n\rwill always be with you." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_RANGER :
		sprintf( buf, "\n\rThe spirits of the woods let you go on your way, "
			      "but their influence still\n\rflows within you." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_PALADIN :
		sprintf( buf, "\n\rThe calls of the gods become a whisper, but their "
			      "teachings will be\n\rwith you for all enternity." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_BARD :
		sprintf( buf, "\n\rYou You realize that song and dance are not "
			      "everything in life, but\n\ryou are confident that "
			      "your skills will return in time." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_VAMPIRE :
		sprintf( buf, "\n\rPerhaps your bloodlust will stay behind in "
			      "the past, but you realize\n\rthat this is not "
			      "possible." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_WEREWOLF :
		sprintf( buf, "\n\rYou attempt to cure yourself of the lycanthropy "
			      "that infects your blood,\n\rbut you realize that "
			      "this is not possible.  Your curse is eterna nol\n\r"
			      "matter how you try to rid yourself of it." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_ANTI_PALADIN :
		sprintf( buf, "\n\rYou decide to follow the way of Satan any  "
			      "longer, but are confident\n\rthat his blessings will "
			      "return in time." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_ASSASSIN :
		sprintf( buf, "\n\rYou decide to follow the way of the sword no "
			      "longer, but are confident\n\rthat your skills will "
			      "return in time." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_MONK :
		sprintf( buf, "\n\rThe calls of the gods become a whisper, but their "
			      "teachings will be\n\rwith you for all enternity." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_BARBARIAN :
		sprintf( buf, "\n\rYou attempt to put the ravage nature of Barbarism "
			      "Behind you, but you are sure\n\rthat your true nature "
			      "will resurface." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_ILLUSIONIST :
		sprintf( buf, "\n\rYour arcane skills seem to leave you, but you "
			      "know that they will flow\n\ragain some day." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_NECROMANCER :
		sprintf( buf, "\n\rYour arcane skills seem to leave you, but you "
			      "know that they will flow\n\ragain some day." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_DEMONOLOGIST :
		sprintf( buf, "\n\rYour arcane skills seem to leave you, but you "
			      "know that they will flow\n\ragain some day." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_SHAMAN :
		sprintf( buf, "\n\rThe voices of nature fade off, but the last message "
			      "you hear is that they\n\rwill always be with you." );
		strcat( buf, "\n\r" );
		break;
	    case CLASS_DARKPRIEST :
		sprintf( buf, "\n\rYour arcane skills seem to leave you, but you "
			      "know that they will flow\n\ragain some day." );
		strcat( buf, "\n\r" );
		break;

	    default :
			buf[ 0 ] = '\0';
    }
    send_to_char(C_DEFAULT, buf, ch );
}

