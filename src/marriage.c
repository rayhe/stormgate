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
 * Code to create rings for married people.... Used in do_marry to give
 * the newly weds their initial rings, but can be used by imms also to
 * reimburse lost rings (for whatever reason)
 * Usage: newring <char>
 * The ring will be given to char automatically.
 * This is part of the marriage code by Canth (canth@xs4all.nl)
 * of Mythran
 */
void do_rings ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA	*spouse1;
    CHAR_DATA	*spouse2;
    char	buf  [ MAX_STRING_LENGTH ];
    OBJ_DATA 	*ring;
    EXTRA_DESCR_DATA * ed;

    if ( !authorized( ch, "rings" ) )
	return;


    if (   !(spouse1 = get_char_world( ch, argument ) )
	|| !(spouse2 = get_char_world( ch, spouse1->pcdata->spouse ) ) )
    {
	send_to_char(C_DEFAULT, "They both need to be logged on to create the ring\n\r", ch );
	return;
    }

    /* Ok, they're both logged on... Let's see what sex spous1 is,
       and give them appropriate ring, and string it properly */

    switch( spouse1->sex )
    {
	case SEX_FEMALE:
	{
	    ring = create_object( get_obj_index( OBJ_VNUM_DIAMOND_RING ), 0 );
	    switch( spouse2->sex )
	    {
		case SEX_FEMALE:
		{
		    sprintf( buf, "This is the beautiful diamond ring given to you by your lovely\n\rwife %s at your wedding. It signifies your eternal love for eachother.", spouse2->name );
		    ring->description = str_dup( buf );
		    break;
		}
		case SEX_MALE:
		{
		    sprintf( buf, "This is the beautiful diamond ring given to you by your handsome\n\rhusband %s at your wedding. It signifies your eternal love for eachother.", spouse2->name );
		    ring->description = str_dup( buf );
		    break;
		}
		case SEX_NEUTRAL:
		default:
		{
		    sprintf( buf, "This is the beautiful diamond ring given to you by your\n\rspouse %s at your wedding. It signifies your eternal love for eachother.", spouse2->name );
		    ring->description = str_dup( buf );
		    break;
		}
	    }
		ed = new_extra_descr();

	    ed->keyword = str_dup( "inscription" );
	    sprintf( buf, "The inscription reads:\n\rTo my lovely wife, yours forever, %s", spouse2->name );
	    ed->description = str_dup( buf );
	    ed->deleted = FALSE;
	    ed->next = ring->extra_descr;
	    ring->extra_descr = ed;
	    break;
	}
	case SEX_MALE:
	case SEX_NEUTRAL:
	default:
	{
	    ring = create_object( get_obj_index( OBJ_VNUM_WEDDING_BAND ), 0 );
	    switch( spouse2->sex )
	    {	/* Description of the ring, depending on sex of spouse */
		case SEX_FEMALE:
		{
		    sprintf( buf, "This is the ring given to you by your beautifull wife %s\n\rat your wedding. It signifies your eternal love for eachother.", spouse2->name );
		    ring->description = str_dup( buf );
		    break;
		}
		case SEX_MALE:
		{
		    sprintf( buf, "This is the ring given to you by your handsome husband %s\n\rat your wedding. It signifies your eternal love for eachother.", spouse2->name );
		    ring->description = str_dup( buf );
		    break;
		}
		case SEX_NEUTRAL:
		default:
		{
		    sprintf( buf, "This is the ring given to you by your spouse %s at\n\ryour wedding. It signifies your eternal love for eachother.", spouse2->name );
		    ring->description = str_dup( buf );
		    break;
		}
	    }

	    /* Extra info for "look inscription" */

		ed =new_extra_descr();

	    ed->keyword = str_dup( "inscription" );
	    ed->deleted = FALSE;
	    ed->next = ring->extra_descr;
	    ring->extra_descr = ed;


	    switch( spouse1->sex )
	    {
		default:
		case SEX_MALE:
		{
		    sprintf( buf, "The inscription reads:\n\rTo my handsome prince... Forever yours, %s", spouse2->name );
		    ed->description = str_dup( buf );
		    break;
		}
		case SEX_NEUTRAL:
		{
		    sprintf( buf,"The inscription reads:\n\rForever love, %s", spouse2->name );
		    ed->description = str_dup( buf );
		    break;
		}
	    }
	}
    }

    obj_to_char ( ring, spouse1 );

    return;
}


/*
 * Marriage code... Marries and divorces 2 ppl.
 * the chars have to be in the same room as the implementor using the command.
 * This is part of the marriage code by Canth (canth@xs4all.nl)
 * of Mythran
 */
void do_marry( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *	spouse1;
    CHAR_DATA *	spouse2;
    char	arg1 [ MAX_INPUT_LENGTH ];
    char	arg2 [ MAX_INPUT_LENGTH ];
    char	buf  [ MAX_STRING_LENGTH ];

    if ( !authorized( ch, "marry" ) )
	return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    /* Make sure two arguments are given */
    if ( arg2[0] == '\0' )
    {
	send_to_char(AT_PURPLE, "Syntax:\n\r", ch );
	send_to_char(AT_WHITE, "marry <person1> <person2>\n\r", ch );
	return;
    }

    /* Make sure 2 characteres are given */
    if ( !(spouse1 = get_char_room( ch, arg1 ) )
      || !(spouse2 = get_char_room( ch, arg2 ) ) )
    {
	send_to_char(C_DEFAULT, "But they are not both present here!\n\r", ch );
	return;
    }

    /* Check for NPC's... They cannot marry. Maybe in a later version */
    if ( IS_NPC( spouse1 ) || IS_NPC( spouse2 ) )
    {
	send_to_char(C_DEFAULT, "Mobiles cannot marry.\n\r", ch );
	return;
    }

    /* Let's not marry or divorce one person to themsleves */
    if ( spouse1 == spouse2 )
    {
	send_to_char(C_DEFAULT, "You have to enter two DIFFERENT names!\n\r", ch );
	return;
    }


    /* Ok, a marriage :) */
    if ( spouse1->pcdata->spouse
      || spouse2->pcdata->spouse )
    {	
	sprintf( buf,
		"%s is already married to %s. No bigammy allowed here\n\r",
		!spouse1->pcdata->spouse
			? spouse2->name : spouse1->name,
		!spouse1->pcdata->spouse
			? spouse2->pcdata->spouse : spouse1->pcdata->spouse );
	send_to_char(C_DEFAULT, buf, ch );
	return;
    }

    spouse1->pcdata->spouse = str_dup( spouse2->name );
    spouse2->pcdata->spouse = str_dup( spouse1->name );

    /* The rings.... */
    do_rings( ch, spouse1->name );
    do_rings( ch, spouse2->name );

    sprintf( buf, "%s and %s have just married each other\n\r", spouse1->name,
	spouse2->name );
    log_string( buf, CHANNEL_INFO, -1);

    return;
    /* That's all folks! */
}

 
void do_divorce ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *	spouse1;
    CHAR_DATA *	spouse2;
    char	arg1 [ MAX_INPUT_LENGTH ];
    char	arg2 [ MAX_INPUT_LENGTH ];
    char	buf  [ MAX_STRING_LENGTH ];

    if ( !authorized( ch, "divorce" ) )
	return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg2[0] == '\0' )
    {
	send_to_char(AT_PURPLE, "Syntax:\n\r", ch );
	send_to_char(AT_WHITE, "divorce <person1> <person2>\n\r", ch );
	return;
    }

    if ( !(spouse1 = get_char_room( ch, arg1 ) )
      || !(spouse2 = get_char_room( ch, arg2 ) ) )
    {
	send_to_char(C_DEFAULT, "But they are not both present here!\n\r", ch );
	return;
    }

    if ( IS_NPC( spouse1 ) || IS_NPC( spouse2 ) )
    {
	send_to_char(C_DEFAULT, "Mobiles cannot marry.\n\r", ch );
	return;
    }

    if ( spouse1 == spouse2 )
    {
	send_to_char(C_DEFAULT, "You have to enter two DIFFERENT names!\n\r", ch );
	return;
    }


    if ( !spouse1->pcdata->spouse
      || !spouse2->pcdata->spouse )
    {
	sprintf( buf, "But %s isn't even married yet!\n\r",
	    !spouse1->pcdata->spouse ? spouse1->name
	    : spouse2->name );
	    send_to_char(C_DEFAULT, buf, ch );
	    return;
    }
    else if ( ( str_cmp( spouse1->name, spouse2->pcdata->spouse ) )
      || ( str_cmp( spouse2->name, spouse1->pcdata->spouse ) ) )
	send_to_char(C_DEFAULT, "But they're not married with each other!\n\r", ch );

	spouse1->pcdata->spouse = NULL;
	spouse2->pcdata->spouse = NULL;		/* Fix by Maniac */

    sprintf( buf, "%s and %s have just divorced each other\n\r", spouse1->name,
	spouse2->name );
    log_string( buf, CHANNEL_INFO, -1);

    return;
}
