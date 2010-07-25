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

/*$Id: crafting.c,v 1.23 2005/04/10 16:29:00 tyrion Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "merc.h"
char* target_name;

/* local functions */
char* set_skin_quality(OBJ_DATA* skin);
char* name_tanned_armor(OBJ_DATA* armor);

/* Use this file for all ranger skills */

extern char* target_name;

void spell_imprint( int sn, int level, CHAR_DATA *ch, void *vo );

// skin corpses for hides
int skill_skin( int sn, int level, CHAR_DATA *ch, void *vo )
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	OBJ_DATA* obj  = (OBJ_DATA*) vo;
	OBJ_DATA* skin = NULL;
	
	if (IS_NPC(ch))
		return SKPELL_BOTCHED;

	if (obj->item_type == ITEM_CORPSE_NPC)
	{

		if (obj->value[0] == TAN_NONE || (obj->value[0] == TAN_PC && !IS_IMMORTAL(ch)))
		{
			send_to_char(AT_RED, "You cannot skin that!\n\r", ch);
			return SKPELL_BOTCHED;
		}
	} else if ( obj->item_type == ITEM_ARMOR )
	{
		if (!IS_SET(obj->extra_flags2, ITEM_CRAFTED))
		{
			send_to_char(AT_RED, "You can't scrape leather from non-leather armor!\n\r", ch);
			return SKPELL_BOTCHED;
		}
	} else if (obj->item_type == ITEM_SKIN)
	{
		OBJ_DATA* obj2;
		char arg [MAX_INPUT_LENGTH]="";

		target_name = one_argument(target_name, arg);
		one_argument(target_name, arg);

		if (!obj->carried_by)
		{
			send_to_char(AT_RED, "You do not have that in your inventory! Pick it up!\n\r", ch);
			return SKPELL_BOTCHED;
		}

		if (arg[0]=='\0')
		{
			send_to_char(AT_RED, "You need two rolls of leather to combine!\n\r", ch);
			return SKPELL_BOTCHED;
		}
		
		obj2 = get_obj_carry(ch, arg);
		
		if (!obj2 || (obj2 && obj2->item_type != ITEM_SKIN))
		{
			send_to_char(AT_RED, "You need two rolls of leather to combine!\n\r", ch);
			return SKPELL_BOTCHED;
		}

		if (obj==obj2)
		{
			send_to_char(AT_RED, "You cannot combine a roll of leather with itself!\n\r", ch);
			return SKPELL_BOTCHED;
		}

		if (number_percent() - 20 < ( ch->pcdata->learned[sn] / 10 ) )
		{
			int waste = 0;
			skin = create_object( get_obj_index( OBJ_VNUM_LEATHER ), obj->level );

			waste = URANGE(1, UMIN( obj->value[1], obj->value[2]), 10);
			/* waste -= waste * ch->pcdata->learned[sn] / 1000; */
			if( number_percent( ) + 10 <= ch->pcdata->learned[sn] / 10 && waste > 0 ) 
			{
				waste -= 1;
			}

			skin->item_type = ITEM_SKIN;
			skin->level	= (obj->level + obj2->level) / 2;
			skin->value[1]  = UMAX(1, obj->value[1] + obj2->value[1] - waste);
			skin->value[0]  = (obj->value[0] + obj2->value[0]) / 2;
			skin->weight    = obj->weight + obj2->weight;

			strcpy(buf2, set_skin_quality( skin ));

			send_to_char(AT_WHITE, "You skilfully combine:\n\r", ch);
			send_to_char(AT_BLUE, obj->short_descr, ch);
			send_to_char(AT_BLUE, "\n\r", ch);
			send_to_char(AT_BLUE, obj2->short_descr, ch);
			send_to_char(AT_BLUE, "\n\r", ch);
			if (waste)
			{
				sprintf(buf, "You waste %d yards of leather.\n\r", waste);
				send_to_char(AT_BLUE, buf, ch);
			}

			sprintf(buf, "Creating %d yards of %s leather!\n\r", skin->value[1], buf2);
			send_to_char(AT_RED, buf, ch);

	        	act(AT_RED, "$n combines two rolls of leather." , ch, NULL, NULL, TO_ROOM);

			extract_obj(obj);
			extract_obj(obj2);

			obj_to_char(skin, ch);

			return SKPELL_NO_DAMAGE; 
		} else
		{
			send_to_char(AT_RED, "You fail to combine the two rolls of leather, destroying both of them!\n\r", ch);
			act(AT_RED, "$n attempts to combine two rolls of leather, but fails!", ch, NULL, NULL, TO_ROOM);
			extract_obj(obj);
			extract_obj(obj2);

			return SKPELL_BOTCHED;
		}

	} else
	{
		send_to_char(AT_RED, "You can only skin corpses. What are you thinking?\n\r", ch);
		return SKPELL_BOTCHED;
	}

	if (number_percent() - 20 < ( ch->pcdata->learned[sn] / 10 ) )
	{
		int learned = ch->pcdata->learned[sn];

		skin = create_object( get_obj_index( OBJ_VNUM_LEATHER ), obj->level );

		skin->item_type = ITEM_SKIN;
		if (obj->item_type == ITEM_ARMOR)
		{
			skin->level 	= obj->level;
			skin->value[1]  = 1 + obj->level * number_range(1, learned / 10) / 100;
		}
		else
		{

			skin->level	= UMIN( 105, (get_mob_index( obj->value[1] ))->level);
			skin->value[1]  = (get_mob_index( obj->value[1] ))->size      /* mob size   		*/
					   * number_range(1, learned / 40) 	      /* x (multi by) 1 to 25 */
					   / 10					      /* / (divide by) 10       */
					   + UMAX(1, ( skin->level / 25 ) ); 	      /* + (plus) 1 to 4	*/
		}

		skin->weight = number_range( skin->value[1]/2, skin->value[1] );

		if (learned == 1000)
		{
			skin->value[0]=number_range(SKIN_GOOD, SKIN_PERFECT);
		}
		else if (learned > 750)
		{
			if (number_percent() < 25)
				skin->value[0]=number_range(SKIN_GOOD, SKIN_PERFECT);
			else
				skin->value[0]=number_range(SKIN_DECENT, SKIN_EXCELLENT);
		}
		else if (learned > 500)
		{
			if(number_percent() < 10)
				skin->value[0]=number_range(SKIN_GOOD, SKIN_PERFECT);
			else if (number_percent() < 30)
				skin->value[0]=number_range(SKIN_DECENT, SKIN_EXCELLENT);
			else
				skin->value[0]=number_range(SKIN_LOUSY, SKIN_DECENT);
		} else
		{
			if(number_percent() < 5)
				skin->value[0]=number_range(SKIN_GOOD, SKIN_PERFECT);
			else if (number_percent() < 15)
				skin->value[0]=number_range(SKIN_DECENT, SKIN_EXCELLENT);
			else if (number_percent() < 30)
				skin->value[0]=number_range(SKIN_LOUSY, SKIN_DECENT);
			else
				skin->value[0]=number_range(SKIN_DESTROYED, SKIN_PASSABLE);
		}

		strcpy(buf2, set_skin_quality(skin) );

		obj_to_char(skin, ch );

		sprintf(buf, "You skillfully scrape the leather creating %d yards of %s leather!\n\r", skin->value[1], buf2);
		send_to_char(AT_RED, buf, ch);
		sprintf(buf, "$n scrapes the %s for its leather.", obj->short_descr);
	        act(AT_RED, buf, ch, NULL, NULL, TO_ROOM);
	}
	else
	{
		send_to_char(AT_RED, "You mangle the skin. You've destroyed it!\n\r", ch);
		act(AT_RED, "$n thoroughly dices a corpse, but it yields no leather!", ch, NULL, NULL, TO_ROOM);
	}

	extract_obj(obj);

	return SKPELL_ZERO_DAMAGE;
}

// tan hides for armor
int skill_tan( int sn, int level, CHAR_DATA *ch, void *vo )
{
	OBJ_DATA* skin 	 = (OBJ_DATA*) vo;
	OBJ_DATA* needle = get_obj_carry(ch, "needle");
	OBJ_DATA* armor  = NULL;

	char buf [MAX_STRING_LENGTH];
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	
	int yards = 0;
	int loc   = 0;
	int lvl   = 0;
	int bonus = 0;
 
	if (IS_NPC(ch))
		return SKPELL_BOTCHED;

	if (skin->item_type != ITEM_SKIN)
	{
		send_to_char(AT_ORANGE, "You need to use leather to tan!\n\r", ch);
		return SKPELL_BOTCHED;
	}

	if (!needle || needle->item_type != ITEM_NEEDLE)
	{
		send_to_char(AT_ORANGE, "You need a needle to tan leather!\n\r", ch);
		return SKPELL_BOTCHED;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_TANNERY))
	{
		send_to_char(AT_ORANGE, "You need to be in a tannery to tan leather!\n\r", ch);
		return SKPELL_BOTCHED;
	}


	target_name = one_argument(target_name, arg1);
	target_name = one_argument(target_name, arg1); /* Repeat to throw away first arg */
	target_name = one_argument(target_name, arg2); 
	
	if (arg1[0]=='\0' || !is_number(arg1))
	{
		send_to_char(AT_ORANGE, "You need to choose how many yards of leather you will use!\n\r", ch);
		return SKPELL_BOTCHED;
	}

	yards = atoi( arg1 );

	if (!yards)
	{
		send_to_char(AT_ORANGE, "You need to choose how many yards of leather you will use!\n\r", ch);
		return SKPELL_BOTCHED;
	}

	if ( yards > skin->value[1] )
	{
		sprintf(buf, "Your %s doesn't contain enough leather.\n\rChoose another roll or combine two rolls.\n\r", skin->short_descr);
		send_to_char(AT_ORANGE, buf, ch );
		return SKPELL_BOTCHED;
	} else if ( yards <= 0)
	{
		send_to_char(AT_ORANGE, "How many yards of leather do you plan on using?\n\r", ch);
		return SKPELL_BOTCHED;
	}

	if (arg2[0]=='\0' || !is_number(arg2))
	{
		lvl = ch->level;
	} else
	{
		lvl = atoi( arg2 );

		if (lvl <= 0)
			lvl = ch->level;

		if (lvl > ch->level)
		{
			send_to_char(AT_ORANGE, "You can't try to make an item high level than you are!\n\r", ch);
			return SKPELL_BOTCHED;
		}
		lvl = UMIN( lvl, ch->level );
	}


	if ( lvl > yards )
	{
		send_to_char(AT_ORANGE, "You need at least 1 yard per level of item you are creating!\n\r", ch);
		return SKPELL_BOTCHED;
	}

	/* extra leather supplied */
	bonus = yards - lvl;
        /* difficulty for levelv */
	bonus += skin->value[0] - (lvl/5);
	bonus = UMAX( bonus, 25 ); /* cap the bonus. not sure on value yet */

	do 
	{
		loc   = (int) pow(2,number_range(2, 20)) + ITEM_TAKE; /* generate a valid wear loc */
	} while (loc == ITEM_WIELD + ITEM_TAKE || loc == ITEM_PROTOTYPE + ITEM_TAKE || loc == ITEM_WEAR_CONTACT + ITEM_TAKE );

	skin->value[1] -= yards;
	if (skin->value[1] <= 0)
	{
		extract_obj( skin );	
		send_to_char( AT_ORANGE, "You use all of your leather roll.\n\r", ch );
	} else
	{
		set_skin_quality(skin);
	}

	if (number_percent() < (((ch->pcdata->learned[sn]/10)+bonus)))
	{
		int max_extra 	= 0;
		int cur_extra 	= 0;
		int bv			= 2;
		int rescount	= 0;
		int statcount	= 0;
		send_to_char(AT_ORANGE, "You skillfully tan and sew the leather!\n\r", ch);

		armor = create_object( get_obj_index(OBJ_VNUM_TANNED), lvl);

		armor->weight		= number_range(5, 15);
		armor->level 	 	= lvl;
		armor->wear_flags  	= loc;
		armor->cost	 	= lvl;
		armor->durability_max	= number_fuzzy(skin->value[0]) * number_fuzzy(lvl/10);
		armor->durability_cur	= number_range(armor->durability_max/4, armor->durability_max);

		max_extra = lvl / 17 + 1; /* 2->7 affects plus 2 possible spells */

		cur_extra = 0;
		while (bv <= ITEM_ACID && cur_extra < max_extra )
		{
			if (number_percent() < 10 && bv != ITEM_INVENTORY && bv != ITEM_PATCHED && bv != ITEM_LOCK)
			{
				cur_extra++;
				armor->extra_flags += bv;
			}
			bv *= 2;
		}

		bv = 0;
		while (bv <= ITEM_DISPEL && cur_extra < max_extra )
		{
			if (number_percent() < 10 )
			{
				cur_extra++;
				armor->extra_flags2 += bv;
			}
			bv *= 2;
		}

		SET_BIT(armor->extra_flags2, ITEM_CRAFTED);

		/* ac */
		armor->value[0]	= number_fuzzy( level / 4 + 2 );

		/* applies */
		for (cur_extra = 0; cur_extra < max_extra; cur_extra++)
		{
			int apply = 0;
			int amt   = 0;
			int pct   = 0;
			AFFECT_DATA* pAf;

			pct = number_percent();
			if (pct<5)
			{
				if (rescount >= 2) 
					continue;

				rescount++;
				
				apply = number_range(APPLY_DAM_ACID, APPLY_DAM_WATER );
				if (loc == WEAR_BODY || loc == WEAR_HOLD || loc == WEAR_SHIELD)
					amt = number_range( - lvl / 10, lvl / 4 );
				else if (loc == ITEM_WEAR_FINGER || loc == ITEM_WEAR_NECK || loc == ITEM_WEAR_ORBIT || loc == ITEM_WEAR_ANKLE || loc == ITEM_WEAR_WRIST || loc == ITEM_WEAR_WAIST )
					amt = number_range( 1, lvl / 20 );
				else
					amt = number_range( -1, lvl / 10);

			} else if (pct < 20)
			{
				apply = APPLY_SAVING_SPELL;
				amt   = 0 - number_range(lvl / 20 , lvl / 5 );
				if (lvl > 100)
					amt -= number_range(0, 20);
			} else if (pct < 50)
			{
				if (statcount >= 2)
					continue;

				statcount++;

				apply = number_range(APPLY_STR,APPLY_CON);
				amt = number_range(1, (lvl / 30) + 1 );
			} else if (pct < 75 )
			{
				apply = number_range(APPLY_SEX, APPLY_ANTI_DIS);
				switch(apply)
				{
				default:
					cur_extra--;
					continue;
					break; /* you got hosed */
				case APPLY_MANA:
					amt = number_range(lvl / 5, lvl   );
										amt = number_range(lvl / 5, lvl   );
					if (lvl > LEVEL_HERO)
						amt += number_range(0, (lvl - LEVEL_HERO) / 4);
					break;
				case APPLY_BP:
					amt = number_range(lvl / 10 , lvl / 3);
					amt = number_range(lvl / 5, lvl   );
					if (lvl > LEVEL_HERO)
						amt += number_range(0, (lvl - LEVEL_HERO) * 20);
					break;
				case APPLY_HIT:
					amt = number_range(lvl / 5, lvl   );
					if (lvl > LEVEL_HERO)
						amt += number_range(0, (lvl - LEVEL_HERO) * 20);
					break;
				case APPLY_MOVE:
					amt = number_range(lvl, lvl * 2 );
					break;
				case APPLY_AC:
					amt = 0 - number_range(lvl / 5, lvl + 50 );
					if (lvl > LEVEL_HERO)
						amt -= number_range(0, (lvl - LEVEL_HERO) * 10);
					break;
				case APPLY_HITROLL:
				case APPLY_DAMROLL:
					if (loc==ITEM_WEAR_BODY)
						amt = number_range( lvl / 5, lvl / 2 );
					else
						amt = number_range( level / 10, level / 3 );
					break;
				case APPLY_ANTI_DIS:
					amt = number_range( lvl / 5, lvl );
					break;
				}
			}

			if (apply)
			{
				bool found = FALSE;
				for (pAf = armor->affected; pAf; pAf=pAf->next)
				{
					if (pAf->location == apply)
					{
						found = TRUE;
						break;
					}
				}
				if (found)
				{
					cur_extra--;
					continue;
				}

				pAf             =   new_affect();
				pAf->location   =   apply;
				if (amt < 0)
					pAf->modifier   =  -number_fuzzy( -amt );
				else
					pAf->modifier   =   number_fuzzy(amt);
				pAf->type       =   0;
				pAf->duration   =   -1;
				pAf->bitvector  =   0;
				pAf->next       =   armor->affected;
				pAf->type		=	0;
				armor->affected =   pAf;

				/* if you got hitroll, you got damroll */
				if (apply == APPLY_HITROLL || apply == APPLY_DAMROLL)
				{
					if (apply == APPLY_HITROLL)
						apply = APPLY_DAMROLL;
					else
						apply = APPLY_HITROLL;

					pAf             =   new_affect();
					pAf->location   =   apply;
					pAf->modifier   =   number_fuzzy(amt);
					pAf->type		=	0;
					pAf->duration   =   -1;
					pAf->bitvector  =   0;
					pAf->next       =   armor->affected;
					armor->affected =   pAf;

				}
			}
		}

		/* perm spells */
		if (lvl > 30 && number_percent() < 10)
		{
			AFFECT_DATA* pAf = new_affect();
			pAf = new_affect();
			do
			{
				pAf->location = number_range(PERM_SPELL_BEGIN, PERM_SPELL_END);
			} while ( !valid_aff_loc(pAf->location) );

			pAf->type	=   skill_lookup(affect_loc_name(pAf->location));
			if (pAf->type < 0)
			{
				pAf->type = 0;
				sprintf(buf, "skill_tanning: affect name for: %s does not match sn for skill_lookup", affect_loc_name(pAf->location) );
				bug(buf, 0);
			}
			pAf->modifier   =   -1;
			pAf->duration   =   -1;
			pAf->bitvector  =   0;
			pAf->next       =   armor->affected;
			armor->affected =   pAf;
		}

		if (number_percent() < 3 && lvl > LEVEL_HERO)
		{
			AFFECT_DATA* pAf = new_affect();
			pAf = new_affect();
			do
			{
				pAf->location = number_range(PERM_SPELL_BEGIN, PERM_SPELL_END );
			} while (!valid_aff_loc(pAf->location) );

			pAf->type	=   skill_lookup(affect_loc_name(pAf->location));
			if (pAf->type < 0)
			{
				pAf->type = 0;
				sprintf(buf, "skill_tanning: affect name for: %s does not match sn for skill_lookup", affect_loc_name(pAf->location) );
				bug(buf, 0);
			}

			pAf->modifier   =   -1;
			pAf->duration   =   -1;
			pAf->bitvector  =   0;
			pAf->next       =   armor->affected;
			armor->affected =   pAf;
		}


		send_to_char(AT_ORANGE, "You manage to create:\n\r", ch);
		sprintf(buf, "%s\n\r", name_tanned_armor(armor));
		send_to_char(AT_WHITE, buf, ch);
 
		obj_to_char(armor, ch);

		if (number_percent() < 10)
		{
			send_to_char(AT_CYAN, "Your needle breaks!\n\r", ch);
			extract_obj(needle);
		}

		act(AT_ORANGE, "$n skillfully tans and stitches together some new armor!", ch, NULL, NULL, TO_ROOM);
        } else
	{
		if (number_percent() < 25)
		{
			send_to_char(AT_CYAN, "You break your needle in the attempt!\n\r", ch);
			extract_obj( needle );
		} 
		send_to_char(AT_ORANGE, "Your attempt at tanning fails miserably!\n\r", ch);
		return SKPELL_BOTCHED;
	}

	return SKPELL_ZERO_DAMAGE;
}

/* Helper function to avoid repetitive code */
char* set_skin_quality(OBJ_DATA* skin)
{
	static char buf    [MAX_STRING_LENGTH];
	static char quality[MAX_STRING_LENGTH];

	if (skin->item_type != ITEM_SKIN)
	{
		bug("set_skin_quality: object is not a skin!", 0);
		return NULL;
	}

	strcpy(quality, flag_string(quality_flags, skin->value[0]));

	sprintf(buf, (get_obj_index( OBJ_VNUM_LEATHER ))->short_descr, skin->value[1], quality);	
	free_string(skin->short_descr);
	skin->short_descr = str_dup(buf);

	sprintf(buf, (get_obj_index( OBJ_VNUM_LEATHER ))->description, quality);	
	free_string(skin->description);
	skin->description = str_dup(buf);
	
	return quality;
}

char* name_tanned_armor(OBJ_DATA* armor)
{

	char prefix1	[MAX_STRING_LENGTH] = "";
	char prefix2	[MAX_STRING_LENGTH] = "";
	char location	[MAX_STRING_LENGTH] = "";
	char suffix1	[MAX_STRING_LENGTH] = "";
	char suffix2	[MAX_STRING_LENGTH] = "";
	
	char format	[MAX_STRING_LENGTH] = "%s%s%s of %s%s";

	static char final[MAX_STRING_LENGTH] = "";

	char first = 0;
	char last  = 0;

	int e1 = armor->extra_flags;
	int e2 = armor->extra_flags2;

	AFFECT_DATA* paf = NULL;

	switch (armor->wear_flags - ITEM_TAKE)
	{
	case ITEM_WEAR_FINGER: strcpy(location, "ring"); break;
	case ITEM_WEAR_NECK: 
	{
		int pct = number_percent();
		if (pct < 20)
			strcpy(location, "Scarf");
		else if (pct < 40)
			strcpy(location, "Chain");
		else if (pct < 60)
			strcpy(location, "Choker");
		else
			strcpy(location, "Necklace");
	}
	break;
	case ITEM_WEAR_BODY:
	{
		int pct = number_percent();
		if (pct < 10)
			strcpy(location, "Breastplate");
		else if (pct < 20)
			strcpy(location, "Leather");
		else if (pct < 30)
			strcpy(location, "Studded Leather");
		else if (pct < 40)
			strcpy(location, "Padded Leather");
		else if (pct < 50)
			strcpy(location, "Banded Leather");
		else if (pct < 75)
			strcpy(location, "Cured Leather");
		else	
			strcpy(location, "Hide");
	}
	break;
	case ITEM_WEAR_HEAD:
	{
		int pct = number_percent();
		if (pct < 30)
			strcpy(location, "Headdress");
		else if (pct < 60)
			strcpy(location, "Helm");
		else 
			strcpy(location, "Helmet");
	}
	break;
	case ITEM_WEAR_LEGS: strcpy(location, "Leggings"); break;
	case ITEM_WEAR_FEET:
	{
		int pct = number_percent();
		if (pct < 25)
			strcpy(location, "Sandals");
		else if (pct < 50)
			strcpy(location, "Shoes");
		else	
			strcpy(location, "Boots");
	}
	break;
	case ITEM_WEAR_HANDS:
	if (number_percent() < 50)
		strcpy(location, "Gloves");
	else
		strcpy(location, "Gauntlets");
	break;
	case ITEM_WEAR_ARMS:	
	if (number_percent() < 50)
		strcpy(location, "Sleeves");
	else
		strcpy(location, "Vambraces");
	break;
	case ITEM_WEAR_SHIELD: strcpy(location, "Shield"); break;
	case ITEM_WEAR_ABOUT: strcpy(location, "Cloak"); break;
	case ITEM_WEAR_WAIST: strcpy(location, "Belt"); break;
	case ITEM_WEAR_WRIST: strcpy(location, "Bracelet"); break;
	case ITEM_HOLD:	strcpy(location, "Orb"); break;
	case ITEM_WEAR_ORBIT: strcpy(location, "Ball"); break;
	case ITEM_WEAR_FACE: strcpy(location, "Mask"); break;
	case ITEM_WEAR_EARS: strcpy(location, "Earrings"); break; 
	case ITEM_WEAR_ANKLE: strcpy(location, "Ankle Bracelet"); break;
	}

	/* Checked in reverse order. extra2 high->low, extra1 high->low
         * since low bits are more likely due to the algorithm, this gives
         * high order bits precedence for naming. (Otherwise you see a lot
         * of glowing and humming stuff)
         */
	if (IS_SET(e2, ITEM_DISPEL))
	{
		strcpy(prefix2, "Vortex ");
	} else if (IS_SET(e2, ITEM_SPARKING ))
	{
		strcpy(prefix2, "Lightning ");
	} else if (IS_SET(e2, ITEM_HIDDEN))
	{
		strcpy(prefix2, "Shadey ");
	} else if (IS_SET(e1, ITEM_ACID))
	{
		strcpy(prefix2, "Corrosive ");
	} else if (IS_SET(e1, ITEM_ICY))
	{
		strcpy(prefix2, "Freezing ");
	} else if (IS_SET(e1, ITEM_FLAME))
	{
		strcpy(prefix2, "Flaming ");
	} else if (IS_SET(e1, ITEM_POISONED))
	{
		strcpy(prefix2, "Virulent ");
	} else if (IS_SET(e1, ITEM_BLESS) || IS_SET(e2, ITEM_HOLY))
	{
		strcpy(prefix2, "Blessed ");
	} else if (IS_SET(e1, ITEM_NODROP) || IS_SET(e1, ITEM_NOREMOVE))
	{
		strcpy(prefix2, "Cursed ");
	} else if (IS_SET(e1, ITEM_MAGIC))
	{
		strcpy(prefix2, "Magic ");
	} else if (IS_SET(e1, ITEM_INVIS))
	{
		strcpy(prefix2, "Translucent ");
	} else if (IS_SET(e1, ITEM_EVIL))
	{
		strcpy(prefix2, "Wicked ");
	} else if (IS_SET(e1, ITEM_DARK))
	{
		strcpy(prefix2, "Dark ");
	} else if (IS_SET(e1, ITEM_HUM))
	{
		strcpy(prefix2, "Vibrating ");
	} else if (IS_SET(e1, ITEM_GLOW))
	{
		strcpy(prefix2, "Bright ");
	} else
	{
		if (number_percent() < 50 )
			strcpy(prefix2, "Strange ");
		else
			strcpy(prefix2, "Queer ");
	}

	first = prefix2[0];
	last  = location[strlen(location)-1];

	if (last == 's')
		strcpy(prefix1, "");
	else if (first == 'A' || first == 'E' || first == 'I' || first == 'O' || first =='U')
		strcpy(prefix1, "an ");
	else
		strcpy(prefix1, "a ");		


	for (paf = armor->affected ; paf; paf = paf->next)
	{
		if (strcmp(suffix1, "") && strcmp(suffix2, ""))
			break;

		switch (paf->location)
		{
		case APPLY_STR: if (!strcmp(suffix1, "")) { strcpy(suffix1, "Steel "); }; break;
		case APPLY_DEX:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Quicksilver "); }; break;
		case APPLY_INT:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Mentats"); }; break;
		case APPLY_WIS:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Owls"); }; break;
		case APPLY_CON:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Dragonkind"); }; break;
		case APPLY_MANA:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "BattleMages"); }; break;
 		case APPLY_BP:      if (!strcmp(suffix2, "")) { strcpy(suffix2, "Vampires"); }; break;
    		case APPLY_ANTI_DIS:  if (!strcmp(suffix1, "")) { strcpy(suffix1, "Iron "); }; break;
    		case APPLY_HIT:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Healthiness"); }; break;
    		case APPLY_MOVE:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Swift "); }; break;
    		case APPLY_AC:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "BlackSmiths"); }; break;
    		case APPLY_HITROLL:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Aiming"); }; break;
    		case APPLY_DAMROLL:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Bloodmetal "); }; break;

	  	case APPLY_AGE_SPELL:		if (!strcmp(suffix1, "")) { strcpy(suffix1, "Old "); }; break;
      		case APPLY_ANGELIC_AURA: if (!strcmp(suffix1, "")) { strcpy(suffix1, "Angelic "); }; break;	
	  	case APPLY_ANTI_FLEE: if (!strcmp(suffix2, "")) { strcpy(suffix2, "Spiders"); }; break;
      		case APPLY_AURA_ANTI_MAGIC: if (!strcmp(suffix1, "")) { strcpy(suffix1, "Dispelling "); }; break;		
	  	case APPLY_BEND_LIGHT:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Prismatic "); }; break;
      		case APPLY_BIOFEEDBACK: if (!strcmp(suffix1, "")) { strcpy(suffix1, "Controlled "); }; break;
	  	case APPLY_BLADE: if (!strcmp(suffix2, "")) { strcpy(suffix2, "Blades"); }; break;
      		case APPLY_BLESS:		if (!strcmp(suffix1, "")) { strcpy(suffix1, "Blessed "); }; break;
	  	case APPLY_BLIND:		if (!strcmp(suffix2, "")) { strcpy(suffix2, "Justice"); }; break;
      		case APPLY_BLOODSHIELD: 	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Bloodrayne"); }; break;	
	  	case APPLY_CHANGE_SEX:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Homaphradite "); }; break;
      		case APPLY_CHAOS:         if (!strcmp(suffix2, "")) { strcpy(suffix2, "Insanity"); }; break;		
	  	case APPLY_CLOAKING:		if (!strcmp(suffix2, "")) { strcpy(suffix2, "Thieves"); }; break;
	  	case APPLY_CLOUD_OF_HEALING: if (!strcmp(suffix1, "")) { strcpy(suffix1, "Druidic "); }; break;
      		case APPLY_COFIRE:        if (!strcmp(suffix1, "")) { strcpy(suffix1, "Fire "); }; break;		
      		case APPLY_COMBAT_MIND:   if (!strcmp(suffix2, "")) { strcpy(suffix2, "Monks"); }; break;
	  	case APPLY_CONFUSED:		if (!strcmp(suffix1, "")) { strcpy(suffix1, "Dimwitted "); }; break;
	  	case APPLY_CURSE:			if (!strcmp(suffix2, "")) { strcpy(suffix2, "Caring"); }; break;
	  	case APPLY_CURSE_NATURE:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Nature's "); }; break;
	  	case APPLY_DANCING:		if (!strcmp(suffix2, "")) { strcpy(suffix2, "Twinkling"); }; break;
      		case APPLY_DETECT_EVIL:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Inquisition"); }; break;	
	  	case APPLY_DETECT_GOOD:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Slayers "); }; break;
      		case APPLY_DETECT_HIDDEN:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Eagle-Eyed "); }; break;	
      		case APPLY_DETECT_INVIS:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Astral-Eyed "); }; break;	
      		case APPLY_DETECT_MAGIC:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Magic-Eyed "); }; break;	
      		case APPLY_EARTHSHIELD: if (!strcmp(suffix2, "")) { strcpy(suffix2, "Gaia"); }; break;		
      		case APPLY_ESSENCE_OF_GAIA: if (!strcmp(suffix2, "")) { strcpy(suffix2, "Mother Earth"); }; break;		
      		case APPLY_ETHEREAL_SNAKE: if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Serpent "); }; break;		
      		case APPLY_ETHEREAL_WOLF: if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Wolf "); }; break;		
      		case APPLY_FAERIE_FIRE:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Visibility"); }; break;	
     	 	case APPLY_FIRESHIELD:    if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Fire Elemental "); }; break;
	  	case APPLY_FLAMING:		if (!strcmp(suffix1, "")) { strcpy(suffix1, "Everflaming "); }; break;
      		case APPLY_FLYING:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Winged "); }; break;	
      		case APPLY_FORCE_OF_NATURE: if (!strcmp(suffix2, "")) { strcpy(suffix2, "Nature Warriors"); }; break;		
      		case APPLY_FORESTWALK:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Rangers"); }; break;	
	  	case APPLY_FUMBLE:		if (!strcmp(suffix2, "")) { strcpy(suffix2, "Butterfingers"); }; break;
      		case APPLY_GHOST_SHIELD:  if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Dead "); }; break;		
      		case APPLY_GIANT_STRENGTH:if (!strcmp(suffix2, "")) { strcpy(suffix2, "Cloud Giants"); }; break;		
      		case APPLY_GOLDEN_ARMOR:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Shiny "); }; break;	
      		case APPLY_GOLDEN_SANCTUARY: if (!strcmp(suffix2, "")) { strcpy(suffix2, "Rightousness"); }; break;		
      		case APPLY_HASTE:		if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Hare "); }; break;
      		case APPLY_HEIGHTEN_SENSES:      if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Hound "); }; break;		
      		case APPLY_HIDE:		if (!strcmp(suffix2, "")) { strcpy(suffix2, "Stealth"); }; break;
      		case APPLY_HOLY_PROTECTION: if (!strcmp(suffix2, "")) { strcpy(suffix2, "Gods Hand"); }; break;		
      		case APPLY_ICESHIELD:     if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Arctic "); }; break;  		
      		case APPLY_IMPROVED_HIDE: if (!strcmp(suffix1, "")) { strcpy(suffix1, "Thieves "); }; break;		
      		case APPLY_IMPROVED_INVIS:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Forgotten"); }; break;	
	  	case APPLY_INERTIAL:		if (!strcmp(suffix2, "")) { strcpy(suffix2, "Wall"); }; break;
      		case APPLY_INFRARED:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Dwarf "); }; break;	
      		case APPLY_INVISIBLE:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Unseen"); }; break;	
      		case APPLY_LEAF_SHIELD: if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Forest "); }; break;		
      		case APPLY_LIQUID_SKIN: if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Fish "); }; break;		
	  	case APPLY_MALIGNIFY:		if (!strcmp(suffix1, "")) { strcpy(suffix1, "Weakness "); }; break;
      		case APPLY_MIST: 		if (!strcmp(suffix1, "")) { strcpy(suffix1, "Fog "); }; break;
      		case APPLY_MOUNTAINWALK:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Cherpa "); }; break;	
      		case APPLY_NAGAROMS_CURSE: if (!strcmp(suffix1, "")) { strcpy(suffix1, "NAGAROM'S UNHOLY WRATH!!!111oneone!1"); }; break;		
      		case APPLY_OCCULUTUS_VISUM: if (!strcmp(suffix2, "")) { strcpy(suffix2, "Visage"); }; break;		
      		case APPLY_PASS_DOOR:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Phantom Formed "); }; break;	
	  	case APPLY_PEACE: if (!strcmp(suffix2, "")) { strcpy(suffix2, "Mediation"); }; break;
	  	case APPLY_PESTILENCE:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Plague "); }; break;
      		case APPLY_PLAINSWALK:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Foal "); }; break;	
	  	case APPLY_POWER_LEAK:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Fatigue"); }; break;
      		case APPLY_PRAYER:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Holiness"); }; break;	
      		case APPLY_PROTECT:	if (!strcmp(suffix1, "")) { strcpy(suffix1, "Darks Bane "); }; break;	
	  	case APPLY_PROTECTION_GOOD: if (!strcmp(suffix1, "")) { strcpy(suffix1, "Gods Bane "); }; break;
      		case APPLY_QUICKNESS:      if (!strcmp(suffix1, "")) { strcpy(suffix1, "Speed "); }; break;		
      		case APPLY_RANDOMSHIELD: if (!strcmp(suffix1, "")) { strcpy(suffix1, "the Unknown "); }; break;		
      		case APPLY_SANCTUARY:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Safety"); }; break;	
      		case APPLY_SATANIC_INFERNO: if (!strcmp(suffix2, "")) { strcpy(suffix2, "HellFury"); }; break;		
     		case APPLY_SCRY:          if (!strcmp(suffix1, "")) { strcpy(suffix1, "Farseeing "); }; break;		
      		case APPLY_SHADOW_IMAGE:  if (!strcmp(suffix1, "")) { strcpy(suffix1, "Ever-Ecplipsing "); }; break;		
      		case APPLY_SHOCKSHIELD:   if (!strcmp(suffix1, "")) { strcpy(suffix1, "Shocking "); }; break;		
	  	case APPLY_SLIT:		if (!strcmp(suffix1, "")) { strcpy(suffix1, "Gagging "); }; break;
      		case APPLY_SNEAK:		if (!strcmp(suffix1, "")) { strcpy(suffix1, "Silence "); }; break;
      		case APPLY_SWAMPWALK:	if (!strcmp(suffix2, "")) { strcpy(suffix2, "Moors"); }; break;	
	  	case APPLY_TALE_OF_TERROR: if (!strcmp(suffix2, "")) { strcpy(suffix2, "Fear"); }; break;
      		case APPLY_TITAN_STRENGTH: if (!strcmp(suffix1, "")) { strcpy(suffix1, "Titan "); }; break;		
      		case APPLY_TONGUES: if (!strcmp(suffix2, "")) { strcpy(suffix2, "Babel"); }; break;
	  	case APPLY_TORTURE: if (!strcmp(suffix1, "")) { strcpy(suffix1, "Soul "); }; break;
	  	case APPLY_TRUESIGHT: if (!strcmp(suffix2, "")) { strcpy(suffix2, "Visions"); }; break;
		case APPLY_UNHOLY_STRENGTH: if (!strcmp(suffix2, "")) { strcpy(suffix2, "Demon Strength"); }; break;
		}
	}

	if (!strcmp(suffix1,"") && !strcmp(suffix2, ""))
	{
		strcpy(suffix1, "Insignificance");
	}

	sprintf(final, format, prefix1, prefix2, location, suffix1, suffix2);
	free_string(armor->short_descr);
	armor->short_descr = str_dup(final);

	sprintf(format, "A %s lies here upon the groud, discarded.", location);
	free_string(armor->description);
	armor->description = str_dup(format);

	sprintf(format, "%s %s %s%s", prefix2, location, suffix1, suffix2);
	free_string(armor->name);
	armor->name = str_dup(format);

	return final;
}

int skill_forestry( int sn, int level, CHAR_DATA *ch, void *vo )
{
	int i;
	int forest_count;
	ROOM_INDEX_DATA* room = ch->in_room;
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch)) { return SKPELL_MISSED; }

	if (!IS_OUTSIDE(ch) || !IS_FOREST(ch))
	{
		send_to_char(AT_DGREEN, "You have to be outside in a forest to chop wood!\n\r", ch);
		return SKPELL_BOTCHED;
	}

	if (IS_SET(room->timed_room_flags, ROOM_TIMED_DEFORESTED))
	{
		send_to_char(AT_DGREEN, "This forest has already been harvested!\n\r", ch);
		return SKPELL_BOTCHED;
	}

	forest_count = 1;
	
	for (i = 0; i < MAX_DIR; i++)
	{
		if (room->exit[i] && IS_OUTSIDE(ch) && IS_FOREST(ch))
			forest_count++;
	}

	sprintf(buf, "%d forest rooms found!\n\r", forest_count);
	bug(buf, 0);

	set_timed_room_flags( ch->in_room, ROOM_TIMED_DEFORESTED, 600 );

	send_to_char(AT_GREEN, "Forestry isn't completed yet!\n\r", ch);
	return SKPELL_NO_DAMAGE;
}

int skill_fletching( int sn, int level, CHAR_DATA *ch, void *vo )
{
	send_to_char(AT_YELLOW, "Fletching isn't completed yet!\n\r", ch);
	return SKPELL_NO_DAMAGE;
}

int skill_mining( int sn, int level, CHAR_DATA *ch, void *vo )
{
	send_to_char(AT_DGREY, "Mining isn't completed yet!\n\r", ch);
	return SKPELL_NO_DAMAGE;
}

int skill_forging( int sn, int level, CHAR_DATA *ch, void *vo )
{
	send_to_char(AT_DGREY, "Forging isn't completed yet!\n\r", ch);
	return SKPELL_NO_DAMAGE;
}

void destroy_craft( CHAR_DATA* ch, bool failed )
{
	char buf[MAX_STRING_LENGTH];
	char* name; 
	PC_DATA* pc;

	if (IS_NPC(ch))
		return;

	pc = ch->pcdata;

	name = item_type_name(pc->craft_target);

	if (failed)
	{
		char verb[MAX_STRING_LENGTH];
		if (pc->craft_type == CRAFT_SCROLL)
			strcpy(verb, "burst into flames");
		else
			strcpy(verb, "explodes into pieces");

		sprintf(buf, "Your %s %s and disappears!\n\r", name, verb);
		send_to_char(AT_RED, buf, ch);
		send_to_char(AT_RED, "You must have made a mistake somewhere!\n\r\n\r", ch);
	}
	else
	{
		sprintf(buf, "The momentary distraction makes you ruin the %s!\n\r\n\r", name);
		send_to_char(AT_RED, "Your attempt at crafting is interrupted!\n\r", ch);
		send_to_char(AT_RED, buf, ch);
	}

	if (number_percent() < 25)
	{
		OBJ_DATA* o;
		int type = 0;

		if (pc->craft_type == CRAFT_SCROLL)
			type = ITEM_QUILL;
		else if (pc->craft_type == CRAFT_POTION)
			type = ITEM_PESTLE;

		if (type)
		{
			for (o = ch->carrying; o; o = o->next_content)
			{
				if ( o->item_type == type)
					break;
			}
			sprintf(buf, "Your %s breaks!\n\r", item_type_name(o) );
			send_to_char(AT_WHITE, buf, ch);
			extract_obj(o);
		}
	}

	pc->craft_timer = 0;
	pc->craft_type = 0;
	pc->spell1 = 0;
	pc->spell2 = 0;
	pc->spell3 = 0;
	extract_obj(pc->craft_target);
}

void finish_craft( CHAR_DATA* ch )
{
	PC_DATA* pc;
	char	 buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
		return;

	pc = ch->pcdata;

	/* take care of stuff like having sleep cast on you */
	if (ch->position != POS_STANDING)
		destroy_craft(ch, FALSE);

	switch (pc->craft_type)
	{
	default:
		send_to_char(AT_RED, "You finished something...\n\r", ch);
		break;
	case CRAFT_POTION:
	{
		int lvl1, lvl2, lvl3;
		int pct1, pct2, pct3;
		int pct_scribe = pc->learned[skill_lookup("alchemy")]/10;
		if (pc->spell3)
		{
			int target = 0;

			lvl1 = UMIN( skill_table[pc->spell1].skill_level[ch->class], skill_table[pc->spell1].skill_level[ch->multied] );
			lvl2 = UMIN( skill_table[pc->spell2].skill_level[ch->class], skill_table[pc->spell2].skill_level[ch->multied] );
			lvl3 = UMIN( skill_table[pc->spell3].skill_level[ch->class], skill_table[pc->spell3].skill_level[ch->multied] );
			pct1 = pc->learned[pc->spell1]/10;
			pct2 = pc->learned[pc->spell2]/10;
			pct3 = pc->learned[pc->spell3]/10;

			target = (pct_scribe + ( (120-lvl1) + pct1) + ( (120-lvl2) + pct2/2) + ( (120-lvl3) + pct3/4) ) / 7;

			if (number_percent() < target )
			{
				spell_imprint(pc->spell1, ch->level, ch, pc->craft_target);
				spell_imprint(pc->spell2, ch->level, ch, pc->craft_target);
				spell_imprint(pc->spell3, ch->level, ch, pc->craft_target);
				act(AT_WHITE, "$n finishes brewing a potion.",  ch, NULL, NULL, TO_ROOM );
			} else
			{
				destroy_craft( ch, TRUE );
				act(AT_WHITE, "$n failed to brew a potion.",  ch, NULL, NULL, TO_ROOM );
				return;
			}
		
		} else if (pc->spell2)
		{
			int target = 0;

			lvl1 = UMIN( skill_table[pc->spell1].skill_level[ch->class], skill_table[pc->spell1].skill_level[ch->multied] );
			lvl2 = UMIN( skill_table[pc->spell2].skill_level[ch->class], skill_table[pc->spell2].skill_level[ch->multied] );
			pct1 = pc->learned[pc->spell1]/10;
			pct2 = pc->learned[pc->spell2]/10;

			target = (pct_scribe + ( (120-lvl1) + pct1) + ( (120-lvl2) + pct2/2) )  / 5;

			if (number_percent() < target )
			{
				spell_imprint(pc->spell1, ch->level, ch, pc->craft_target);
				spell_imprint(pc->spell2, ch->level, ch, pc->craft_target);
				act(AT_WHITE, "$n finishes brewing a potion.",  ch, NULL, NULL, TO_ROOM );
			} else
			{
				destroy_craft( ch, TRUE );
				act(AT_WHITE, "$n failed to brew a potion.",  ch, NULL, NULL, TO_ROOM );
				return;
			}
				
  		} else
		{
			int target = 0;

			lvl1 = UMIN( skill_table[pc->spell1].skill_level[ch->class], skill_table[pc->spell1].skill_level[ch->multied] );
			pct1 = pc->learned[pc->spell1]/10;

			target = (pct_scribe + ( (120-lvl1) + pct1) ) / 3;

			if (number_percent() < target )
			{
				spell_imprint(pc->spell1, ch->level, ch, pc->craft_target);
				act(AT_WHITE, "$n finishes brewing a potion.",  ch, NULL, NULL, TO_ROOM );
			} else
			{
				destroy_craft( ch, TRUE );
				act(AT_WHITE, "$n failed to brew a potion.",  ch, NULL, NULL, TO_ROOM );
				return;
			}

		}
	}
	break;	
	case CRAFT_SCROLL:
	{
		int lvl1, lvl2, lvl3;
		int pct1, pct2, pct3;
		int pct_scribe = pc->learned[skill_lookup("inscription")]/10;
		if (pc->spell3)
		{
			int target = 0;

			lvl1 = UMIN( skill_table[pc->spell1].skill_level[ch->class], skill_table[pc->spell1].skill_level[ch->multied] );
			lvl2 = UMIN( skill_table[pc->spell2].skill_level[ch->class], skill_table[pc->spell2].skill_level[ch->multied] );
			lvl3 = UMIN( skill_table[pc->spell3].skill_level[ch->class], skill_table[pc->spell3].skill_level[ch->multied] );
			pct1 = pc->learned[pc->spell1]/10;
			pct2 = pc->learned[pc->spell2]/10;
			pct3 = pc->learned[pc->spell3]/10;

			target = (pct_scribe + ( (120-lvl1) + pct1) + ( (120-lvl2) + pct2/2) + ( (120-lvl3) + pct3/4) ) / 7;

			if (number_percent() < target )
			{
				spell_imprint(pc->spell1, ch->level, ch, pc->craft_target);
				spell_imprint(pc->spell2, ch->level, ch, pc->craft_target);
				spell_imprint(pc->spell3, ch->level, ch, pc->craft_target);
				act(AT_WHITE, "$n finishes writing a scroll.",  ch, NULL, NULL, TO_ROOM );

			} else
			{
				destroy_craft( ch, TRUE );
				act(AT_WHITE, "$n failed to write a scroll.",  ch, NULL, NULL, TO_ROOM );
				return;
			}
		
		} else if (pc->spell2)
		{
			int target = 0;

			lvl1 = UMIN( skill_table[pc->spell1].skill_level[ch->class], skill_table[pc->spell1].skill_level[ch->multied] );
			lvl2 = UMIN( skill_table[pc->spell2].skill_level[ch->class], skill_table[pc->spell2].skill_level[ch->multied] );
			pct1 = pc->learned[pc->spell1]/10;
			pct2 = pc->learned[pc->spell2]/10;

			target = (pct_scribe + ( (120-lvl1) + pct1) + ( (120-lvl2) + pct2/2) )  / 5;

			if (number_percent() < target )
			{
				spell_imprint(pc->spell1, ch->level, ch, pc->craft_target);
				spell_imprint(pc->spell2, ch->level, ch, pc->craft_target);
				act(AT_WHITE, "$n finishes writing a scroll.",  ch, NULL, NULL, TO_ROOM );
			} else
			{
				destroy_craft( ch, TRUE );
				act(AT_WHITE, "$n failed to write a scroll.",  ch, NULL, NULL, TO_ROOM );
				return;
			}
				
  		} else
		{
			int target = 0;

			lvl1 = UMIN( skill_table[pc->spell1].skill_level[ch->class], skill_table[pc->spell1].skill_level[ch->multied] );
			pct1 = pc->learned[pc->spell1]/10;

			target = (pct_scribe + ( (120-lvl1) + pct1) ) / 3;

			if (number_percent() < target )
			{
				spell_imprint(pc->spell1, ch->level, ch, pc->craft_target);
				act(AT_WHITE, "$n finishes writing a scroll.",  ch, NULL, NULL, TO_ROOM );
			} else
			{
				destroy_craft( ch, TRUE );
				act(AT_WHITE, "$n failed to write a scroll.",  ch, NULL, NULL, TO_ROOM );
				return;
			}

		}
	}
	break;
	}

	if ( number_percent() < 10 )
	{
		OBJ_DATA* o;
		int type = 0;

		if (pc->craft_type == CRAFT_SCROLL)
			type = ITEM_QUILL;
		else if (pc->craft_type == CRAFT_POTION)
			type = ITEM_PESTLE;

		if (type)
		{
			for (o = ch->carrying; o; o = o->next_content)
			{
				if ( o->item_type == type)
					break;
			}
			sprintf(buf, "Your %s breaks!\n\r", item_type_name(o) );
			send_to_char(AT_WHITE, buf, ch);
			extract_obj(o);
		}
	}

	pc->craft_target->level = ch->level;
	pc->craft_target->value[0] = ch->level;
	SET_BIT(pc->craft_target->extra_flags2, ITEM_CRAFTED);

	pc->craft_target = NULL;
	pc->craft_timer  = 0;
	pc->craft_type   = 0;
	pc->spell1		 = 0;
	pc->spell2		 = 0;
	pc->spell3		 = 0;
}

int skill_brew ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA* pestle;
    OBJ_DATA* vial;
    char buf   [MAX_STRING_LENGTH];
    char spell1[MAX_INPUT_LENGTH];
    char spell2[MAX_INPUT_LENGTH];
    char spell3[MAX_INPUT_LENGTH];
    int sn1;
    int sn2;
    int sn3;

    target_name = one_argument( target_name, spell1 );
    target_name = one_argument( target_name, spell2 );
    target_name = one_argument( target_name, spell3 );


    if ( spell1[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Use alchemy on what spell?\n\r", ch );
	return SKPELL_BOTCHED;
    }

    if ( (vial = get_obj_carry(ch, spell1)) )
    {
	if (vial->item_type != ITEM_POTION)
	{
		send_to_char(AT_WHITE, "You can't empty an item that isn't a potion!\n\r", ch);
		return SKPELL_BOTCHED;
	}

	extract_obj(vial);

	if (number_percent() > ( ( ch->pcdata->learned[skill_lookup("alchemy")]/10 + (120-vial->level) + (120-vial->value[0]) ) /2 ) )
	{
		send_to_char(AT_WHITE, "Oh no! You crack the vial. It's ruined!\n\r", ch);
		act( AT_WHITE, "$n cracks a vial while trying to empty it.\n\r", ch, NULL, NULL, TO_ROOM);
		return SKPELL_BOTCHED;
	}

	act( AT_WHITE, "$n empties a vial..\n\r", ch, NULL, NULL, TO_ROOM);
	send_to_char(AT_WHITE, "You empty the vial.\n\r", ch); 

	vial = create_object( get_obj_index( OBJ_VNUM_FLASK ), ch->level);
	obj_to_char(vial, ch);
	return SKPELL_NO_DAMAGE;
    }

    if ( (sn1 = skill_lookup(spell1)) < 0 || !ch->pcdata->learned[sn1] || !can_use_skspell(ch, sn1) )
    {
	sprintf(buf, "You don't know '%s'! How can you brew it?\n\r", spell1);
	send_to_char(AT_WHITE, buf, ch);
	return SKPELL_BOTCHED;
    }
    else if (!skill_table[sn1].is_spell)
    {
	sprintf(buf, "'%s' is not a spell!\n\r", spell1);
	send_to_char(AT_WHITE, buf, ch);
	return SKPELL_BOTCHED;
    }
    else if (!IS_SET(skill_table[sn1].craftable, CRAFT_POTION))
    {
	sprintf( buf, "You are unable to brew the spell '%s'\n\r", spell1);
	send_to_char(AT_WHITE, buf, ch);
	return SKPELL_BOTCHED;
    }


    if (spell2[0])
    {
        if ( (sn2 = skill_lookup(spell2) ) < 0 || !ch->pcdata->learned[sn2] || !can_use_skspell(ch, sn2) )

        {
		sprintf(buf, "You don't know '%s'! How can you brew it?\n\r", spell2);
		send_to_char(AT_WHITE, buf, ch);
		return SKPELL_BOTCHED;
	}
	else if (!skill_table[sn2].is_spell)
	{
		sprintf(buf, "'%s' is not a spell!\n\r", spell2);
		send_to_char(AT_WHITE, buf, ch);
		return SKPELL_BOTCHED;
	}
    	else if (!IS_SET(skill_table[sn2].craftable, CRAFT_POTION))
    	{
		sprintf( buf, "You are unable to brew the spell '%s'\n\r", spell2);
		send_to_char(AT_WHITE, buf, ch);
		return SKPELL_BOTCHED;
    	}

    } else
    {
	sn2 = 0;
    }

    if (spell3[0])
    {
        if ( (sn3 = skill_lookup(spell3) ) < 0 || !ch->pcdata->learned[sn3] || !can_use_skspell(ch, sn3) )

        {
		sprintf(buf, "You don't know '%s'! How can you brew it?\n\r", spell3);
		send_to_char(AT_WHITE, buf, ch);
		return SKPELL_BOTCHED;
	}
	else if (!skill_table[sn3].is_spell)
	{
		sprintf(buf, "'%s' is not a spell!\n\r", spell3);
		send_to_char(AT_WHITE, buf, ch);
		return SKPELL_BOTCHED;
	}
    	else if (!IS_SET(skill_table[sn3].craftable, CRAFT_POTION))
    	{
		sprintf( buf, "You are unable to brew the spell '%s'\n\r", spell3);
		send_to_char(AT_WHITE, buf, ch);
		return SKPELL_BOTCHED;
 	}
    } else
    {
	sn3 = 0;
    }

    /* Do we have a parchment to scribe spells? */
    for ( vial = ch->carrying; vial; vial = vial->next_content )
    {
	if ( vial->item_type == ITEM_POTION && 
		!(vial->value[0] || vial->value[1] || vial->value[2] || vial->value[3]) )
	    break;
    }
    if ( !vial )
    {
	send_to_char(AT_WHITE, "You do not have an empty vial.\n\r", ch );
	return SKPELL_BOTCHED;
    }   

    for ( pestle = ch->carrying; pestle; pestle = pestle->next_content )
    {
	if ( pestle->item_type == ITEM_PESTLE )
	    break;
    }
    if ( !pestle )
    {
	send_to_char(AT_WHITE, "You do not have a pestle and mortar.\n\r", ch );
	return SKPELL_BOTCHED;
    }
    
    act(AT_WHITE, "$n begins brewing a potion.", ch, vial, NULL, TO_ROOM );
    send_to_char(AT_WHITE, "You begin to carefully brew the potion.\n\r", ch);

    ch->pcdata->spell1 		= sn1;
    ch->pcdata->spell2 		= sn2;
    ch->pcdata->spell3 		= sn3;
    ch->pcdata->craft_target 	= vial;
    ch->pcdata->craft_type   	= CRAFT_POTION;
    ch->pcdata->craft_timer  	= ( MANA_COST( ch, sn1)
					+ (sn2 ? MANA_COST(ch, sn2) : 0 ) 
					+ (sn3 ? MANA_COST(ch, sn3) : 0 ) );


    return SKPELL_NO_DAMAGE;
}

int skill_inscription ( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA* quill;
    OBJ_DATA* parch;
    char buf   [MAX_STRING_LENGTH];
    char spell1[MAX_INPUT_LENGTH];
    char spell2[MAX_INPUT_LENGTH];
    char spell3[MAX_INPUT_LENGTH];
    int sn1;
    int sn2;
    int sn3;

    target_name = one_argument( target_name, spell1 );
    target_name = one_argument( target_name, spell2 );
    target_name = one_argument( target_name, spell3 );


    if ( spell1[0] == '\0' )
    {
	send_to_char(AT_WHITE, "Use inscription what spell?\n\r", ch );
	return SKPELL_BOTCHED;
    }

    if ( (parch = get_obj_carry(ch, spell1)) )
    {
	if (parch->item_type != ITEM_SCROLL)
	{
		send_to_char(AT_WHITE, "You can't scrape an item that isn't a scroll!\n\r", ch);
		return SKPELL_BOTCHED;
	}

	extract_obj(parch);

	if (number_percent() > ( ( ch->pcdata->learned[skill_lookup("inscription")]/10 + (120-parch->level) + (120-parch->value[0]) ) /2 ) )
	{
		send_to_char(AT_WHITE, "Oh no! You made a mistake. The scroll is ruined!\n\r", ch);
		act( AT_WHITE, "$n tears a scroll while trying to clear it.\n\r", ch, NULL, NULL, TO_ROOM);
		return SKPELL_BOTCHED;
	}

	act( AT_WHITE, "$n scrapes the words from a scroll.\n\r", ch, NULL, NULL, TO_ROOM);
	send_to_char(AT_WHITE, "You scrape the text from the scroll.\n\r", ch); 

	parch = create_object( get_obj_index( OBJ_VNUM_PARCHMENT ), ch->level);
	obj_to_char(parch, ch);
	return SKPELL_NO_DAMAGE;
    }

    if ( (sn1 = skill_lookup(spell1)) < 0 || !ch->pcdata->learned[sn1] || !can_use_skspell(ch, sn1) )
    {
	sprintf(buf, "You don't know '%s'! How can you scribe it?\n\r", spell1);
	send_to_char(AT_WHITE, buf, ch);
	return SKPELL_BOTCHED;
    }
    else if (!skill_table[sn1].is_spell)
    {
	sprintf(buf, "'%s' is not a spell!\n\r", spell1);
	send_to_char(AT_WHITE, buf, ch);
	return SKPELL_BOTCHED;
    }
    else if (!IS_SET(skill_table[sn1].craftable, CRAFT_SCROLL))
    {
	sprintf( buf, "You are unable to scribe the spell '%s'\n\r", spell1);
	send_to_char(AT_WHITE, buf, ch);
	return SKPELL_BOTCHED;
    }


    if (spell2[0])
    {
        if ( (sn2 = skill_lookup(spell2) ) < 0 || !ch->pcdata->learned[sn2] || !can_use_skspell(ch, sn2) )

        {
		sprintf(buf, "You don't know '%s'! How can you scribe it?\n\r", spell2);
		send_to_char(AT_WHITE, buf, ch);
		return SKPELL_BOTCHED;
	}
	else if (!skill_table[sn2].is_spell)
	{
		sprintf(buf, "'%s' is not a spell!\n\r", spell2);
		send_to_char(AT_WHITE, buf, ch);
		return SKPELL_BOTCHED;
	}
    	else if (!IS_SET(skill_table[sn2].craftable, CRAFT_SCROLL))
    	{
		sprintf( buf, "You are unable to scribe the spell '%s'\n\r", spell2);
		send_to_char(AT_WHITE, buf, ch);
		return SKPELL_BOTCHED;
    	}

    } else
    {
	sn2 = 0;
    }

    if (spell3[0])
    {
        if ( (sn3 = skill_lookup(spell3) ) < 0 || !ch->pcdata->learned[sn3] || !can_use_skspell(ch, sn3) )

        {
		sprintf(buf, "You don't know '%s'! How can you scribe it?\n\r", spell3);
		send_to_char(AT_WHITE, buf, ch);
		return SKPELL_BOTCHED;
	}
	else if (!skill_table[sn3].is_spell)
	{
		sprintf(buf, "'%s' is not a spell!\n\r", spell3);
		send_to_char(AT_WHITE, buf, ch);
		return SKPELL_BOTCHED;
	}
    	else if (!IS_SET(skill_table[sn3].craftable, CRAFT_SCROLL))
    	{
		sprintf( buf, "You are unable to scribe the spell '%s'\n\r", spell3);
		send_to_char(AT_WHITE, buf, ch);
		return SKPELL_BOTCHED;
 	}
    } else
    {
	sn3 = 0;
    }

    /* Do we have a parchment to scribe spells? */
    for ( parch = ch->carrying; parch; parch = parch->next_content )
    {
	if ( parch->item_type == ITEM_SCROLL && 
		!(parch->value[0] || parch->value[1] || parch->value[2] || parch->value[3] ))
	    break;
    }
    if ( !parch )
    {
	send_to_char(AT_WHITE, "You do not have a parchment.\n\r", ch );
	return SKPELL_BOTCHED;
    }

    for ( quill = ch->carrying; quill; quill = quill->next_content )
    {
	if ( quill->item_type == ITEM_QUILL )
	    break;
    }
    if ( !quill )
    {
	send_to_char(AT_WHITE, "You do not have a quill.\n\r", ch );
	return SKPELL_BOTCHED;
    }
    
    act(AT_WHITE, "$n begins writing a scroll.", ch, parch, NULL, TO_ROOM );
    send_to_char(AT_WHITE, "You begin to carefully write the scroll.\n\r", ch);

    ch->pcdata->spell1 		= sn1;
    ch->pcdata->spell2 		= sn2;
    ch->pcdata->spell3 		= sn3;
    ch->pcdata->craft_target 	= parch;
    ch->pcdata->craft_type   	= CRAFT_SCROLL;
    ch->pcdata->craft_timer  	= ( MANA_COST( ch, sn1)
					+ (sn2 ? MANA_COST(ch, sn2) : 0 ) 
					+ (sn3 ? MANA_COST(ch, sn3) : 0 ) );


    return SKPELL_NO_DAMAGE;
}

void spell_imprint( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int       sp_slot, i, mana;
    char      buf[ MAX_STRING_LENGTH ];

     if (obj->deleted) { return; };

      if (skill_table[sn].spell_fun == spell_null )
      {
	send_to_char(AT_WHITE,"That is not a spell.\n\r",ch);
	return;
      }

    /* counting the number of spells contained within */

    for (sp_slot = i = 1; i < 4; i++) 
	if (obj->value[i] != skill_lookup("reserved"))
	    sp_slot++;

    if (sp_slot > 3)
    {
	act (AT_WHITE,"$p cannot contain any more spells.", ch, obj, NULL, TO_CHAR);
	return;
    }

   /* scribe/brew costs 4 times the normal mana required to cast the spell */

    mana = 4 * MANA_COST(ch, sn);
	    
    if ( !IS_NPC(ch) && ch->mana < mana )
    {
	send_to_char(AT_WHITE, "You don't have enough mana.\n\r", ch );
	return;
    }
      
    if ( IS_NPC( ch ) || ( number_percent() > ( ch->pcdata->learned[sn] / 10 ) && ch->level < L_SEN ) )
    {
	send_to_char(AT_WHITE, "You lost your concentration.\n\r", ch );
	ch->mana -= mana / 2;
	return;
    }

    /* executing the imprinting process */
    ch->mana -= mana;
    obj->value[sp_slot] = sn;

    /* Making it successively harder to pack more spells into potions or 
       scrolls - JH */ 

    if (ch->level < L_SEN )
    {
    switch( sp_slot )
    {
   
    default:
	bug( "sp_slot has more than %d spells.", sp_slot );
	return;

    case 1:
	break;
    case 2:
        if ( number_percent() > 80 )
        { 
          sprintf(buf, "The magic enchantment has failed --- the %s vanishes.\n\r", item_type_name(obj) );
	  send_to_char(AT_WHITE, buf, ch );
	  extract_obj( obj );
	  return;
	}     
	break;

    case 3:
        if ( number_percent() > 60 )
        { 
          sprintf(buf, "The magic enchantment has failed --- the %s vanishes.\n\r", item_type_name(obj) );
	  send_to_char(AT_WHITE, buf, ch );
	  extract_obj( obj );
	  return;
	}     
	break;
    }
    } /* If level < L_SEN */ 
  

    /* labeling the item */

    free_string (obj->short_descr);
    sprintf ( buf, "a %s of ", item_type_name(obj) ); 
    for (i = 1; i <= sp_slot ; i++)
      if (obj->value[i] != -1)
      {
	strcat (buf, skill_table[obj->value[i]].name);
        (i != sp_slot ) ? strcat (buf, ", ") : strcat (buf, "") ; 
      }
    obj->short_descr = str_dup(buf);

    free_string( obj->name );
    sprintf( buf, "%s ", item_type_name( obj ) );
    for( i = 1; i <= sp_slot; i++ )
	if( obj->value[i] != -1 )
	{
	    strcat( buf, skill_table[obj->value[i]].name);
	    (i != sp_slot ) ? strcat( buf, " ") : strcat( buf, "" );
	}
    obj->name = strdup( buf );

    free_string( obj->description );
    sprintf( buf, "A %s is here. (", item_type_name( obj ) );
    for( i = 1; i <= sp_slot; i++ )
	if( obj->value[i] != -1 )
	{
	    strcat( buf, skill_table[obj->value[i]].name);
	    (i != sp_slot ) ? strcat( buf, ", ") : strcat( buf, "" );
	}
    sprintf( buf, "%s).", buf );
    obj->description = strdup( buf );

    sprintf(buf, "You have imbued a new spell to the %s.\n\r", item_type_name(obj) );
    send_to_char(AT_WHITE, buf, ch );

    return; 
}

/* Study skill... converted from Rom by Maniac */ 
int skill_study( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *scroll;

    return SKPELL_MISSED;

    if (( scroll = get_obj_carry(ch,target_name)) == NULL)
    {
        send_to_char(AT_WHITE,"You don't have that scroll.\n\r",ch);
        return SKPELL_BOTCHED;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
        send_to_char(AT_WHITE,"You can only study scrolls.\n\r",ch);
        return SKPELL_BOTCHED;
    }

    if ( ch->level < scroll->level )
    {
        send_to_char(AT_WHITE,"You are not a high enough level to use this scroll.\n\r", ch );
        return SKPELL_MISSED;
    }

    if (( skill_table[scroll->value[1]].skill_level[ch->class] >= 72 )
        && ( !IS_IMMORTAL(ch) ))
    {
        send_to_char(AT_WHITE,"Your class may not learn that spell.\n\r",ch);
        return SKPELL_MISSED;
    }

    if ( skill_table[scroll->value[1]].skill_level[ch->class] > ch->level )
    {
        send_to_char(AT_WHITE,"This spell is beyond your grasp. Perhaps in a few levels...\n\r", ch );
        return SKPELL_MISSED;
    }

    if ( ch->pcdata->learned[scroll->value[1]] > 0 )
    {
        send_to_char(AT_WHITE,"You know that spell already!\n\r",ch);
        return SKPELL_MISSED;
    }

    act(AT_WHITE,"$n studies $p.",ch,scroll,NULL,TO_ROOM);
    act(AT_WHITE,"You study $p.",ch,scroll,NULL,TO_CHAR);

    if (number_percent() >= (20 + ( ch->pcdata->learned[skill_lookup("scrolls")] / 10 ) ) * 4/5)
    {
        send_to_char(AT_WHITE,"You mess up and the scroll vanishes!\n\r",ch);
        act(AT_WHITE,"$n screams in anger.",ch,NULL,NULL,TO_ROOM);
    }

    else
    {
        act(AT_WHITE,"You learn the spell!",ch,NULL,NULL,TO_CHAR);
        act(AT_WHITE,"$n learned the spell!",ch,NULL,NULL,TO_ROOM);
        ch->pcdata->learned[scroll->value[1]] = 50;
    }
    extract_obj(scroll);

    return SKPELL_NO_DAMAGE;
}

void strip_timed_room_flags( ROOM_INDEX_DATA* room)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf, "Resetting flags for room: %d", room->vnum);
	log_string(  buf, CHANNEL_LOG, L_APP);
	room->timed_room_flags = 0;
}

void set_timed_room_flags( ROOM_INDEX_DATA* room, int flag, int timer )
{

	if (!room)
		return;

	SET_BIT(room->timed_room_flags, flag);
	room->flag_timer = UMAX( room->flag_timer, timer);

	room->next_timed_room = timed_room_list;
	timed_room_list = room;

	switch (flag)
	{
	case ROOM_TIMED_DEFORESTED:
	case ROOM_TIMED_MINED:
	case ROOM_TIMED_CAMP:
	default:
		/* do something. like... load a camp fire */
		break;
	}
}
 
