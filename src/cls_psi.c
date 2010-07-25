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
 *  In order to use any part of this Envy Diku Msud, you must comply with  *
 *  the original Diku license in 'license.doc', the Merc license in        *
 *  'license.txt', as well as the Envy license in 'license.nvy'.           *
 *  In particular, you may not remove either of these copyright notices.   *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*$Id: cls_psi.c,v 1.1 2005/03/24 19:09:36 ahsile Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"

char* target_name;

int spell_transmutation( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA* obj = (OBJ_DATA*) vo;
	char buf[MAX_STRING_LENGTH];

	if (IS_SET(obj->extra_flags, ITEM_MAGIC))
	{
		send_to_char(AT_BLUE, "Your magic conflicts with the magic of the item!\n\r", ch);
		sprintf(buf, "The %s explodes!\n\r", obj->short_descr);
		send_to_char(AT_BLUE, buf, ch);
		act( AT_BLUE, "$n's $p begins humming wildly!\n\r", ch, obj, NULL, TO_ROOM);
		act( AT_BLUE, "$n's $p explodes!\n\r", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return SKPELL_BOTCHED;
	}

	send_to_char(AT_BLUE, "You try to transmute the molecules...\n\r", ch);
	act( AT_BLUE, "$n begins transmuting something...\n\r", ch, NULL, NULL, TO_ROOM);

	if (number_percent() > (IS_NPC(ch) ? 75 : ((ch->pcdata->learned[sn] + (150 - obj->level))/2)))
	{
		sprintf(buf, "The %s explodes!\n\r", obj->short_descr);
		send_to_char(AT_BLUE, buf, ch);
		act( AT_BLUE, "$n's $p explodes!\n\r", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		return SKPELL_BOTCHED;
	}

	
	if (IS_SET(obj->extra_flags2, ITEM_CRAFTED))
	{
		switch (obj->item_type)
		{
		case ITEM_SCROLL:
		case ITEM_POTION:
			obj->value[0] += number_range(1, 5);
			break;
		case ITEM_ARMOR:
			obj->value[0] += number_range(5, 10);
			break;
		case ITEM_WEAPON:
			if (IS_SET(obj->extra_flags2, ITEM_TWO_HANDED))
			{
				obj->value[1] += number_range(1, 10);
				obj->value[2] += number_range(1, 20);
			} else
			{
				obj->value[1] += number_range(1,  5);
				obj->value[2] += number_range(1, 10);
			}
		}
	} else
	{
		if (IS_SET(obj->extra_flags, ITEM_NO_DAMAGE))
		{
			send_to_char(AT_BLUE, "It's already indestructable!\n\r", ch);
			return SKPELL_NO_DAMAGE;
		}
		SET_BIT(obj->extra_flags, ITEM_NO_DAMAGE);
	}

	SET_BIT(obj->extra_flags, ITEM_MAGIC); /* flag it magic so that
						  it can't be done again */

	send_to_char(AT_BLUE, "You have succeeded in transmuting it.\n\r", ch);

	return SKPELL_NO_DAMAGE;
}
