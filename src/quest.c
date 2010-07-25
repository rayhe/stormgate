/****************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   * 
*  code is allowed provided you add a credit line to the effect of:         *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     *
*  of the standard diku/rom credits. If you use this or a modified version  *
*  of this code, let me know via email: moongate@moongate.ams.com. Further  *
*  updates will be posted to the rom mailing list. If you'd like to get     *
*  the latest version of quest.c, please send a request to the above add-   *
*  ress. Quest Code v2.01. Please do not remove this notice from this file. *
****************************************************************************/

/*$Id: quest.c,v 1.24 2005/04/28 01:04:22 ahsile Exp $*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

DECLARE_DO_FUN( do_say );

/* Object vnums for Quest Rewards */

#define QUEST_ITEM1 35 /* Shield */
#define QUEST_ITEM2 36 /* Contact */
#define QUEST_ITEM3 37 /* Potion of Godlike */
#define QUEST_ITEM4 38 /* Parchment of Immortals */
#define QUEST_ITEM5 39 /* Staff of Power */
#define QUEST_ITEM6 46 /* Storm Lantern */
#define QUEST_ITEM7 51 /* Amulet of the Lost */
#define QUEST_ITEM8 43 /* Eye of Gardith */
#define QUEST_ITEM9 47 /* Sword of Abraxyz */
#define QUEST_ITEM10 27 /* Ring of Abraxyz */
#define QUEST_ITEM11 48 /* Potion of Luck */ 
#define QUEST_ITEM12 44 /* Black Potion */
#define QUEST_ITEM13 29	/* Potion of Titan Strength */
#define QUEST_ITEM14	/* Potion of Shielding */
#define QUEST_ITEM15    /* Potion of Resistance */
#define QUEST_ITEM16    /* Potion of Regeneration */
#define QUEST_ITEM17 49 /* Book of the Gods */

#define QUEST_OBJQUEST1 30
#define QUEST_OBJQUEST2 31
#define QUEST_OBJQUEST3 32
#define QUEST_OBJQUEST4 33
#define QUEST_OBJQUEST5 34

/* Local functions */

void generate_quest	args(( CHAR_DATA *ch, CHAR_DATA *questman ));
void quest_update	args(( void ));
bool quest_level_diff   args(( int clevel, int mlevel));
bool chance		args(( int num ));
ROOM_INDEX_DATA 	*find_location( CHAR_DATA *ch, char *arg );
int  get_random_skpell	args(( ));

/* CHANCE function. I use this everywhere in my code, very handy :> */
/* otherwise known as number_percent - tyrion */

bool chance(int num)
{
    if (number_range(1,100) <= num) return TRUE;
    else return FALSE;
}

/* The main quest function */

void do_quest(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *questman;
    OBJ_DATA *obj=NULL, *obj_next;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf [MAX_STRING_LENGTH * 2];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    int pointreward;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
        send_to_char(AT_WHITE, "QUEST commands: POINTS INFO TIME REQUEST COMPLETE REFUSE LIST BUY.\n\r",ch);
        send_to_char(AT_WHITE, "For more information, type 'HELP QUEST'.\n\r",ch);
	return;
    }
    if (!strcmp(arg1, "info"))
    {
	if (IS_SET(ch->act, PLR_QUESTOR))
	{
	    if (ch->questmob == -1 && ch->questgiver->short_descr != NULL)
	    {
		sprintf(buf, "Your quest is ALMOST complete!\n\rGet back to %s before your time runs out!\n\r",ch->questgiver->short_descr);
		send_to_char(AT_WHITE, buf, ch);
	    }
	    else if (ch->questobj > 0)
	    {
                questinfoobj = get_obj_index(ch->questobj);
		if (questinfoobj != NULL)
		{
		    sprintf(buf, "You are on a quest to recover the fabled %s!\n\r",questinfoobj->name);
		    send_to_char(AT_WHITE, buf, ch);
		}
		else send_to_char(AT_WHITE, "You aren't currently on a quest.\n\r",ch);
		return;
	    }
	    else if (ch->questmob > 0)
	    {
                questinfo = get_mob_index(ch->questmob);
		if (questinfo != NULL)
		{
	            sprintf(buf, "You are on a quest to slay the dreaded %s!\n\r",questinfo->short_descr);
		    send_to_char(AT_WHITE, buf, ch);
		}
		else send_to_char(AT_WHITE, "You aren't currently on a quest.\n\r",ch);
		return;
	    }
	}
	else
	    send_to_char(AT_WHITE, "You aren't currently on a quest.\n\r",ch);
	return;
    }
    if (!strcmp(arg1, "points"))
    {
	sprintf(buf, "You have %d quest points.\n\r",ch->questpoints);
	send_to_char(AT_WHITE, buf, ch);
	return;
    }
    else if (!strcmp(arg1, "time"))
    {
	if (!IS_SET(ch->act, PLR_QUESTOR))
	{
	    send_to_char(AT_WHITE, "You aren't currently on a quest.\n\r",ch);
	    if (ch->nextquest > 1)
	    {
		sprintf(buf, "There are %d minutes remaining until you can go on another quest.\n\r",ch->nextquest);
		send_to_char(AT_WHITE, buf, ch);
	    }
	    else if (ch->nextquest == 1)
	    {
		sprintf(buf, "There is less than a minute remaining until you can go on another quest.\n\r");
		send_to_char(AT_WHITE, buf, ch);
	    }
	}
        else if (ch->countdown > 0)
        {
	    sprintf(buf, "Time left for current quest: %d minutes.\n\r",ch->countdown);
	    send_to_char(AT_WHITE, buf, ch);
	}
	return;
    }

/* Checks for a character in the room with spec_questmaster set. This special
   procedure must be defined in special.c. You could instead use an 
   ACT_QUESTMASTER flag instead of a special procedure. */

    for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room )
    {
	if (!IS_NPC(questman)) continue;
        if (questman->spec_fun == spec_lookup( "spec_questmaster" )) break;
    }

    if (questman == NULL || questman->spec_fun != spec_lookup( "spec_questmaster" ))
    {
        send_to_char(AT_WHITE, "You can't do that here.\n\r",ch);
        return;
    }

    if ( questman->fighting != NULL)
    {
	send_to_char(AT_WHITE, "Wait until the fighting stops.\n\r",ch);
        return;
    }

    ch->questgiver = questman;

    if (!strcmp(arg1, "list"))
    {
        act(AT_WHITE,  "$n asks $N for a list of quest items.", ch, NULL, questman, TO_ROOM); 
	act (AT_WHITE, "You ask $N for a list of quest items.",ch, NULL, questman, TO_CHAR);
	#ifdef RUN_AS_WIN32SERVICE
	sprintf(buf, "Current Quest Items available for Purchase:\n\r \
============================================\r \
1)    500qp - Shield of Abraxyz\r \
2)    750qp - Contact Lenses\r \
3)    150qp - Potion of Godlike Strength\r \
4)    150qp - Parchment of Immortals\r \
5)   1000qp - Staff of Power\r \
6)   1500qp - Storm Lantern of Unholy Plague\r \
7)   2500qp - Amulet of the Lost\r \
8)   4000qp - The Eye of Gardith\r \
9)   4500qp - The Sword of Abraxyz (LEVEL 104)\r \
10)  5000qp - Ring of Abraxyz\r \
11)   500qp - Potion of Luck\r \
12)    25qp - Black Potion\r \
13)   100qp - Potion of Titan Strength\r \
14)   100qp - Potion of Shielding\r \
15)   100qp - Potion of Resistance\r \
16)   100qp - Potion of Regeneration\r \
17)    25qp - Book of the Gods\r \
============================================\r \
A)   250qp - 2,500,000 gold\r \
B)   500qp - 175 pracs\r \
C)    80qp - 25 pracs\r \
D)    25qp - 2 learns\r \
\r \
14 through 16 are currently unavailable\r \
\r \
To buy an item, type 'QUEST BUY <item>'.  For example, 'QUEST BUY 1' for the Shield of Abraxyz.\n\r");
	#else
	sprintf(buf, "Current Quest Items available for Purchase:\n\r \
============================================\r \
1)    500qp - Shield of Abraxyz\r \
2)    750qp - Contact Lenses\r \
3)    150qp - Potion of Godlike Strength\r \
4)    150qp - Parchment of Immortals\r \
5)   1000qp - Staff of Power\r \
6)   1500qp - Storm Lantern of Unholy Plague\r \
7)   2500qp - Amulet of the Lost\r \
8)   4000qp - The Eye of Gardith\r \
9)   4500qp - The Sword of Abraxyz (LEVEL 104)\r \
10)  5000qp - Ring of Abraxyz\r \
11)   500qp - Potion of Luck\r \
12)    25qp - Black Potion\r \
13)   100qp - Potion of Titan Strength\r \
14)   100qp - Potion of Shielding\r \
15)   100qp - Potion of Resistance\r \
16)   100qp - Potion of Regeneration\r \
17)    25qp - Book of the Gods\r \
============================================\r \
A)   250qp - 2,500,000 gold\r \
B)   500qp - 175 pracs\r \
C)    80qp - 25 pracs\r \
D)    25qp - 2 learns\r \
\r \
14 through 16 are currently unavailable\r \
\r \
To buy an item, type 'QUEST BUY <item>'.  For example, 'QUEST BUY 1' for the Shield of Abraxyz.\n\r");
	#endif
	send_to_char(AT_WHITE, buf, ch);
	return;
    }

    else if (!strcmp(arg1, "buy"))
    {
	if (arg2[0] == '\0')
	{
	    send_to_char(AT_WHITE, "To buy an item, type 'QUEST BUY <item>'.\n\r",ch);
	    return;
	}
	if (is_name(arg2, "1"))
	{
	    if (ch->questpoints >= 500)
	    {
		ch->questpoints -= 500;
	        obj = create_object(get_obj_index(QUEST_ITEM1),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "2"))
	{
	    if (ch->questpoints >= 750)
	    {
		ch->questpoints -= 750;
	        obj = create_object(get_obj_index(QUEST_ITEM2),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "3"))
	{
	    if (ch->questpoints >= 150)
	    {
		ch->questpoints -= 150;
	        obj = create_object(get_obj_index(QUEST_ITEM3),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "4"))
	{
	    if (ch->questpoints >= 150)
	    {
		ch->questpoints -= 150;
	        obj = create_object(get_obj_index(QUEST_ITEM4),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "5"))
	{
	    if (ch->questpoints >= 1000)
	    {
		ch->questpoints -= 1000;
	        obj = create_object(get_obj_index(QUEST_ITEM5),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "6"))
        {
            if (ch->questpoints >= 1500)
            {
                ch->questpoints -= 1500;
                obj = create_object(get_obj_index(QUEST_ITEM6),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
	else if (is_name(arg2, "7"))
	{
	    if (ch->questpoints >= 2500)
	    {
		ch->questpoints -= 2500;
	        obj = create_object(get_obj_index(QUEST_ITEM7),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "8"))
	{
	    if (ch->questpoints >= 4000)
	    {
		ch->questpoints -= 4000;
	        obj = create_object(get_obj_index(QUEST_ITEM8),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "9"))
	{
	    if (ch->questpoints >= 4500)
	    {
		ch->questpoints -= 4500;
	        obj = create_object(get_obj_index(QUEST_ITEM9),ch->level);
              obj->value[1] = 50;
              obj->value[2] = 95;
              obj->level = 104;
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "10"))
	{
	    if (ch->questpoints >= 5000)
	    {
		ch->questpoints -= 5000;
		obj = create_object(get_obj_index(QUEST_ITEM10),ch->level);
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
        else if (is_name(arg2, "11"))
        {
            if (ch->questpoints >= 500)
            {
                ch->questpoints -= 500;
                obj = create_object(get_obj_index(QUEST_ITEM11),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (is_name(arg2, "12"))
        {
            if (ch->questpoints >= 25)
            {
                ch->questpoints -= 25;
                obj = create_object(get_obj_index(QUEST_ITEM12),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (is_name(arg2, "13"))
        {
            if (ch->questpoints >= 100)
            {
                ch->questpoints -= 100;
                obj = create_object(get_obj_index(QUEST_ITEM13),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
/*
        else if (is_name(arg2, "14"))
        {
            if (ch->questpoints >= 5000)
            {
                ch->questpoints -= 5000;
                obj = create_object(get_obj_index(QUEST_ITEM14),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (is_name(arg2, "15"))
        {
            if (ch->questpoints >= 5000)
            {
                ch->questpoints -= 5000;
                obj = create_object(get_obj_index(QUEST_ITEM15),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
        else if (is_name(arg2, "16"))
        {
            if (ch->questpoints >= 5000)
            {
                ch->questpoints -= 5000;
                obj = create_object(get_obj_index(QUEST_ITEM16),ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
*/
        else if (is_name(arg2, "17"))
        {
            if (ch->questpoints >= 25)
            {
		int skpell;
		char objname[ MAX_STRING_LENGTH ];
		char objshort_descr[ MAX_STRING_LENGTH ];
		char objdescription[ MAX_STRING_LENGTH ];

                ch->questpoints -= 25;
                obj = create_object(get_obj_index(QUEST_ITEM17),ch->level);
		skpell = get_random_skpell( );
		obj->value[1] = skpell;
		sprintf( objname, "%s %s", obj->name, skill_table[obj->value[1]].name );
		sprintf( objshort_descr, "%s (%s)", obj->short_descr, skill_table[obj->value[1]].name );
		sprintf( objdescription, "A Book of the Gods is here (%s).", skill_table[obj->value[1]].name );
		if( number_percent( ) <= 50 )
		{
			skpell = get_random_skpell( );
			obj->value[2] = skpell;
			sprintf( objname, "%s %s %s", obj->name, skill_table[obj->value[1]].name, skill_table[obj->value[2]].name );
			sprintf( objshort_descr, "%s (%s, %s)", obj->short_descr, skill_table[obj->value[1]].name, skill_table[obj->value[2]].name );
			sprintf( objdescription, "A Book of the Gods is here (%s, %s).", skill_table[obj->value[1]].name, skill_table[obj->value[2]].name );
			if( number_percent( ) <= 50 )
			{
				skpell = get_random_skpell( );
				obj->value[3] = skpell;
				sprintf( objname, "%s %s %s %s", obj->name, skill_table[obj->value[1]].name, skill_table[obj->value[2]].name, skill_table[obj->value[3]].name );
				sprintf( objshort_descr, "%s (%s, %s, %s)", obj->short_descr, skill_table[obj->value[1]].name, skill_table[obj->value[2]].name, skill_table[obj->value[3]].name );
				sprintf( objdescription, "A Book of the Gods is here (%s, %s, %s).", skill_table[obj->value[1]].name, skill_table[obj->value[2]].name, skill_table[obj->value[3]].name );
			}
		}
		obj->value[0] = number_range(1, 100);
		free_string( obj->name );
		free_string( obj->short_descr );
		free_string( obj->description );
		obj->name = str_dup( objname );
		obj->short_descr = str_dup( objshort_descr );
		obj->description = str_dup( objdescription );
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
                do_say(questman,buf);
                return;
            }
        }
	else if (is_name(arg2, "a"))
	{
	    if (ch->questpoints >= 250)
	    {
		ch->questpoints -= 250;
	        ch->gold += 2500000;
    	        act(AT_WHITE,  "$N gives 5,000,000 gold pieces to $n.", ch, NULL, questman, TO_ROOM );
    	        act(AT_WHITE,  "$N has 5,000,000 in gold transfered from $s Swiss account to your balance.",   ch, NULL, questman, TO_CHAR );
	        return;
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "b"))
	{
	    if (ch->questpoints >= 500)
	    {
		ch->questpoints -= 500;
	        ch->practice += 175;
    	        act(AT_WHITE,  "$N gives 175 practices to $n.", ch, NULL, questman, TO_ROOM );
    	        act(AT_WHITE,  "$N gives you 175 practices.",   ch, NULL, questman, TO_CHAR );
	        return;
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "c"))
	{
	    if (ch->questpoints >= 80)
	    {
		ch->questpoints -= 80;
	        ch->practice += 25;
    	        act(AT_WHITE,  "$N gives 25 practices to $n.", ch, NULL, questman, TO_ROOM );
    	        act(AT_WHITE,  "$N gives you 25 practices.",   ch, NULL, questman, TO_CHAR );
	        return;
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else if (is_name(arg2, "d"))
	{
	    if (ch->questpoints >= 25)
	    {
		ch->questpoints -= 25;
	        ch->pcdata->learn += 2;
    	        act(AT_WHITE,  "$N gives 2 learns to $n.", ch, NULL, questman, TO_ROOM );
    	        act(AT_WHITE,  "$N gives you 1 learns.",   ch, NULL, questman, TO_CHAR );
	        return;
	    }
	    else
	    {
		sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.",ch->name);
		do_say(questman,buf);
		return;
	    }
	}
	else
	{
	    sprintf(buf, "I don't have that item, %s.",ch->name);
	    do_say(questman, buf);
	}
	if (obj != NULL)
	{
	    SET_BIT(obj->extra_flags2, 	ITEM_QUEST 	);
		/*
	    SET_BIT(obj->extra_flags2, 	ITEM_NO_STEAL	);

		// Class/Multi Bits
		if( ch->multied == CLASS_MAGE || ch->class == CLASS_MAGE ) { SET_BIT( obj->extra_flags3, ITEM_PRO_MAGE ); }
		if( ch->multied == CLASS_CLERIC || ch->class == CLASS_CLERIC ) { SET_BIT( obj->extra_flags3, ITEM_PRO_CLERIC ); }
		if( ch->multied == CLASS_THIEF || ch->class == CLASS_THIEF ) { SET_BIT( obj->extra_flags3, ITEM_PRO_THIEF ); }
		if( ch->multied == CLASS_WARRIOR || ch->class == CLASS_WARRIOR ) { SET_BIT( obj->extra_flags3, ITEM_PRO_WARRIOR ); }
		if( ch->multied == CLASS_PSIONICIST || ch->class == CLASS_PSIONICIST ) { SET_BIT( obj->extra_flags3, ITEM_PRO_PSI ); }
		if( ch->multied == CLASS_DRUID || ch->class == CLASS_DRUID ) { SET_BIT( obj->extra_flags3, ITEM_PRO_DRUID ); }
		if( ch->multied == CLASS_RANGER || ch->class == CLASS_RANGER ) { SET_BIT( obj->extra_flags3, ITEM_PRO_RANGER ); }
		if( ch->multied == CLASS_PALADIN || ch->class == CLASS_PALADIN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_PALADIN ); }
		if( ch->multied == CLASS_BARD || ch->class == CLASS_BARD ) { SET_BIT( obj->extra_flags3, ITEM_PRO_BARD ); }
		if( ch->multied == CLASS_VAMPIRE || ch->class == CLASS_VAMPIRE ) { SET_BIT( obj->extra_flags3, ITEM_PRO_VAMP ); }
		if( ch->multied == CLASS_WEREWOLF || ch->class == CLASS_WEREWOLF ) { SET_BIT( obj->extra_flags3, ITEM_PRO_WEREWOLF ); }
		if( ch->multied == CLASS_ANTI_PALADIN || ch->class == CLASS_ANTI_PALADIN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_ANTIPAL ); }
		if( ch->multied == CLASS_ASSASSIN || ch->class == CLASS_ASSASSIN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_ASSASSIN ); }
		if( ch->multied == CLASS_MONK || ch->class == CLASS_MONK ) { SET_BIT( obj->extra_flags3, ITEM_PRO_MONK ); }
		if( ch->multied == CLASS_BARBARIAN || ch->class == CLASS_BARBARIAN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_BARBARIAN ); }
		if( ch->multied == CLASS_ILLUSIONIST || ch->class == CLASS_ILLUSIONIST ) { SET_BIT( obj->extra_flags3, ITEM_PRO_ILLUSIONIST ); }
		if( ch->multied == CLASS_NECROMANCER || ch->class == CLASS_NECROMANCER ) { SET_BIT( obj->extra_flags3, ITEM_PRO_NECROMANCER ); }
		if( ch->multied == CLASS_DEMONOLOGIST || ch->class == CLASS_DEMONOLOGIST ) { SET_BIT( obj->extra_flags3, ITEM_PRO_DEMONOLOGIST ); }
		if( ch->multied == CLASS_SHAMAN || ch->class == CLASS_SHAMAN ) { SET_BIT( obj->extra_flags3, ITEM_PRO_SHAMAN ); }
		if( ch->multied == CLASS_DARKPRIEST || ch->class == CLASS_DARKPRIEST ) { SET_BIT( obj->extra_flags3, ITEM_PRO_DARKPRIEST ); }
		// Race Bits
		if      (ch->race == race_lookup("Human"))		{ SET_BIT(obj->extra_flags3, ITEM_PRO_HUMAN ); }
		else if (ch->race == race_lookup("Elf"))		{ SET_BIT(obj->extra_flags3, ITEM_PRO_ELF ); }
		else if (ch->race == race_lookup("Halfelf"))	{ SET_BIT(obj->extra_flags3, ITEM_PRO_HALFELF ); }
		else if (ch->race == race_lookup("Orc"))		{ SET_BIT(obj->extra_flags3, ITEM_PRO_ORC ); }
		else if (ch->race == race_lookup("Drow"))		{ SET_BIT(obj->extra_flags3, ITEM_PRO_DROW ); }
		else if (ch->race == race_lookup("Dwarf"))		{ SET_BIT(obj->extra_flags3, ITEM_PRO_DWARF ); }
		else if (ch->race == race_lookup("Halfdwarf"))	{ SET_BIT(obj->extra_flags3, ITEM_PRO_HALFDWARF ); }
		else if (ch->race == race_lookup("Hobbit"))		{ SET_BIT(obj->extra_flags3, ITEM_PRO_HOBBIT ); }
		else if (ch->race == race_lookup("Giant"))		{ SET_BIT(obj->extra_flags3, ITEM_PRO_GIANT ); }
		else if (ch->race == race_lookup("Ogre"))		{ SET_BIT(obj->extra_flags3, ITEM_PRO_OGRE ); }
		// Crossover from extra3 -> extra4 here 
		else if (ch->race == race_lookup("Angel"))		{ SET_BIT(obj->extra_flags4, ITEM_PRO_ANGEL ); }
		else if (ch->race == race_lookup("Minotaur"))	{ SET_BIT(obj->extra_flags4, ITEM_PRO_MINOTAUR ); }
		else if (ch->race == race_lookup("Feline"))		{ SET_BIT(obj->extra_flags4, ITEM_PRO_FELINE ); }
		else if (ch->race == race_lookup("Gargoyle"))	{ SET_BIT(obj->extra_flags4, ITEM_PRO_GARGOYLE ); }
		else if (ch->race == race_lookup("Canine"))		{ SET_BIT(obj->extra_flags4, ITEM_PRO_CANINE ); }
		else if (ch->race == race_lookup("Demon"))		{ SET_BIT(obj->extra_flags4, ITEM_PRO_DEMON ); }
		else if (ch->race == race_lookup("Pixie"))		{ SET_BIT(obj->extra_flags4, ITEM_PRO_PIXIE ); }
		else if (ch->race == race_lookup("Elder"))		{ SET_BIT(obj->extra_flags4, ITEM_PRO_ELDER ); }
		else if (ch->race == race_lookup("Lizardman"))	{ SET_BIT(obj->extra_flags4, ITEM_PRO_LIZARDMAN ); }
		else if (ch->race == race_lookup("Gnome"))		{ SET_BIT(obj->extra_flags4, ITEM_PRO_GNOME ); }

*/
    	    act(AT_WHITE,  "$N gives $p to $n.", ch, obj, questman, TO_ROOM );
    	    act(AT_WHITE,  "$N gives you $p.",   ch, obj, questman, TO_CHAR );
	    obj_to_char(obj, ch);
	}
	return;
    }
    else if (!strcmp(arg1, "request"))
    {
        act(AT_WHITE,  "$n asks $N for a quest.", ch, NULL, questman, TO_ROOM); 
	act (AT_WHITE, "You ask $N for a quest.",ch, NULL, questman, TO_CHAR);
	if (IS_SET(ch->act, PLR_QUESTOR))
	{
	    sprintf(buf, "But you're already on a quest!");
	    do_say(questman, buf);
	    return;
	}
	if (ch->nextquest > 0)
	{
	    sprintf(buf, "You're very brave, %s, but let someone else have a chance.",ch->name);
	    do_say(questman, buf);
	    sprintf(buf, "Come back later.");
	    do_say(questman, buf);
	    return;
	}

	sprintf(buf, "Thank you, brave %s!",ch->name);
	do_say(questman, buf);
        ch->questmob = 0;
	ch->questobj = 0;

	generate_quest(ch, questman);

        if (ch->questmob > 0 || ch->questobj > 0)
	{
	    SET_BIT(ch->act, PLR_QUESTOR);
	    sprintf(buf, "You have %d minutes to complete this quest.",ch->countdown);
	    do_say(questman, buf);
	    sprintf(buf, "May the gods go with you!");
	    do_say(questman, buf);
	}
	return;
    }
    else if (!strcmp(arg1, "refuse"))
    {
	act(AT_WHITE, "$n offers to refuse the quest.",ch,NULL,questman,TO_ROOM);
	act(AT_WHITE, "You ask $N to refuse your quest.",ch,NULL,questman,TO_CHAR);
	if(!IS_SET(ch->act, PLR_QUESTOR))
	{
	    sprintf(buf, "But you are not on a quest!");
	    do_say(questman, buf);
	    return;
	}
	pointreward = number_range(ch->level / 4, ch->level / 2);
        pointreward = UMAX( pointreward, 15);
	if(ch->questpoints < pointreward)
	{
	    sprintf(buf, "You do not have enough quest points to refuse a quest.");
	    do_say(questman, buf);
	    return;
	}
	sprintf(buf, "As a penalty, I am going to take %d quest points from you.", pointreward );
	do_say(questman, buf);

	REMOVE_BIT(ch->act, PLR_QUESTOR);
        ch->questgiver = NULL;
        ch->countdown = 0;
        ch->questmob = 0;
        ch->questobj = 0;
	ch->nextquest = 1;
	ch->questpoints -= pointreward;
	return;
    }	
    else if (!strcmp(arg1, "complete"))
    {
        act(AT_WHITE,  "$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM); 
	act (AT_WHITE, "You inform $N you have completed $s quest.",ch, NULL, questman, TO_CHAR);
	if (ch->questgiver != questman)
	{
	    sprintf(buf, "I never sent you on a quest! Perhaps you're thinking of someone else.");
	    do_say(questman,buf);
	    return;
	}

	if (IS_SET(ch->act, PLR_QUESTOR))
	{
	    if (ch->questmob == -1 && ch->countdown > 0)
	    {
		int reward, pointreward, pracreward;

	    	reward = number_range(2500,45000);
	    	pointreward = number_range(ch->level / 4, ch->level / 2);
		pointreward = UMAX( pointreward, 15);
		sprintf(buf, "Congratulations on completing your quest!");
		do_say(questman,buf);
		sprintf(buf,"As a reward, I am giving you %d quest points, and %d gold.",pointreward,reward);
		do_say(questman,buf);
		if (chance(25))
		{
		    pracreward = number_range(1,5);
		    sprintf(buf, "You gain %d practices!\n\r",pracreward);
		    send_to_char(AT_WHITE, buf, ch);
		    ch->practice += pracreward;
		}

	        REMOVE_BIT(ch->act, PLR_QUESTOR);
	        ch->questgiver = NULL;
	        ch->countdown = 0;
	        ch->questmob = 0;
		ch->questobj = 0;
	        ch->nextquest = 10;
		ch->gold += reward;
		ch->questpoints += pointreward;

	        return;
	    }
	    else if (ch->questobj > 0 && ch->countdown > 0)
	    {
		bool obj_found = FALSE;

    		for (obj = ch->carrying; obj != NULL; obj= obj_next)
    		{
        	    obj_next = obj->next_content;
        
		    if (obj != NULL && obj->pIndexData->vnum == ch->questobj)
		    {
			obj_found = TRUE;
            	        break;
		    }
        	}
		if (obj_found == TRUE)
		{
		    int reward, pointreward, pracreward;

		    reward = number_range(2500,45000);
		    pointreward = number_range(ch->level / 4,ch->level / 2);
		    pointreward = UMAX(pointreward, 15);
		    act(AT_WHITE, "You hand $p to $N.",ch, obj, questman, TO_CHAR);
		    act(AT_WHITE, "$n hands $p to $N.",ch, obj, questman, TO_ROOM);

	    	    sprintf(buf, "Congratulations on completing your quest!");
		    do_say(questman,buf);
		    sprintf(buf,"As a reward, I am giving you %d quest points, and %d gold.",pointreward,reward);
		    do_say(questman,buf);
		    if (chance(25))
		    {
		        pracreward = number_range(1,5);
		        sprintf(buf, "You gain %d practices!\n\r",pracreward);
		        send_to_char(AT_WHITE, buf, ch);
		        ch->practice += pracreward;
		    }

	            REMOVE_BIT(ch->act, PLR_QUESTOR);
	            ch->questgiver = NULL;
	            ch->countdown = 0;
	            ch->questmob = 0;
		    ch->questobj = 0;
	            ch->nextquest = 10;
		    ch->gold += reward;
		    ch->questpoints += pointreward;
		    extract_obj(obj);
		    return;
		}
		else
		{
		    sprintf(buf, "You haven't completed the quest yet, but there is still time!");
		    do_say(questman, buf);
		    return;
		}
		return;
	    }
	    else if ((ch->questmob > 0 || ch->questobj > 0) && ch->countdown > 0)
	    {
		sprintf(buf, "You haven't completed the quest yet, but there is still time!");
		do_say(questman, buf);
		return;
	    }
	}
	if (ch->nextquest > 0)
	    sprintf(buf,"But you didn't complete your quest in time!");
	else sprintf(buf, "You have to REQUEST a quest first, %s.",ch->name);
	do_say(questman, buf);
	return;
    }

    send_to_char(AT_WHITE, "QUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY.\n\r",ch);
    send_to_char(AT_WHITE, "For more information, type 'HELP QUEST'.\n\r",ch);
    return;
}

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
    CHAR_DATA *victim;
    MOB_INDEX_DATA *vsearch;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *questitem;
    char buf [MAX_STRING_LENGTH];
    long mcounter;
    int mob_vnum;

    for (mcounter = 0; mcounter < 99999; mcounter ++)
    {
	mob_vnum = number_range(50, 32000);

	if ( (vsearch = get_mob_index(mob_vnum) ) != NULL )
	{
            if (vsearch->pShop == NULL
    		&& !IS_SET(vsearch->act, ACT_TRAIN)
    		&& !IS_SET(vsearch->act, ACT_PRACTICE)
                && !IS_SET(vsearch->act, ACT_IS_HEALER)
    		&& !IS_SET(vsearch->act, ACT_BANKER)
    		&& !IS_SET(vsearch->act, ACT_TEACHER)
    		&& !IS_SET(vsearch->act, ACT_PROTOTYPE)
		&& !IS_SET(vsearch->act, ACT_PET)
		&& !IS_SET(vsearch->affected_by, AFF_PEACE)
		&& !IS_SET(vsearch->affected_by, AFF_CHARM)
		&& !IS_SET(vsearch->affected_by, AFF_INVISIBLE)
		&& !IS_SET(vsearch->affected_by4, AFF_IMMORTAL)
		&& chance(40)
		&& quest_level_diff(ch->level, vsearch->level) == TRUE) break;
	    /*else break; */
	}
    }

    if ( vsearch == NULL || ( victim = get_char_world( ch, vsearch->player_name ) ) == NULL)
    {
	generate_quest(ch, questman);
        return;
    }

    if ( (  room = find_location( ch, victim->name ) ) == NULL || ( IS_SET( victim->in_room->room_flags, ROOM_SAFE) || IS_SET( victim->in_room->area->area_flags, AREA_PROTOTYPE )|| IS_SET( victim->in_room->area->area_flags, AREA_NOQUEST ) || IS_SET (victim->in_room->area->area_flags, AREA_RANDOM)))
    {
	generate_quest(ch, questman);
        return;
    }

    /*  40% chance it will send the player on a 'recover item' quest. */

    if (IS_SET( room->area->area_flags, AREA_PROTOTYPE )|| IS_SET( room->area->area_flags, AREA_NOQUEST ) || IS_SET(room->area->area_flags, AREA_RANDOM))
    {
	generate_quest(ch, questman);
        return;
    }

    if (chance(40))
    {
	int objvnum = 0;

	switch(number_range(0,4))
	{
	    case 0:
	    objvnum = QUEST_OBJQUEST1;
	    break;

	    case 1:
	    objvnum = QUEST_OBJQUEST2;
	    break;

	    case 2:
	    objvnum = QUEST_OBJQUEST3;
	    break;

	    case 3:
	    objvnum = QUEST_OBJQUEST4;
	    break;

	    case 4:
	    objvnum = QUEST_OBJQUEST5;
	    break;
	}

        questitem = create_object( get_obj_index(objvnum), ch->level );
	ch->countdown = number_range(10,30);
	questitem->timer = ch->countdown * 2;
	obj_to_room(questitem, room);
	ch->questobj = questitem->pIndexData->vnum;

	sprintf(buf, "Vile pilferers have stolen %s from the royal treasury!",questitem->short_descr);
	do_say(questman, buf);
	do_say(questman, "My court wizardess, with her magic mirror, has pinpointed its location.");

	sprintf(buf, "Look in the general area of %s for %s!",room->area->name, room->name);
	do_say(questman, buf);
	return;
    }

    /* Quest to kill a mob */

    else 
    {
    switch(number_range(0,1))
    {
	case 0:
        sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.",victim->short_descr);
        do_say(questman, buf);
        sprintf(buf, "This threat must be eliminated!");
        do_say(questman, buf);
	break;

	case 1:
	sprintf(buf, "Rune's most heinous criminal, %s, has escaped from the dungeon!",victim->short_descr);
	do_say(questman, buf);
	sprintf(buf, "Since the escape, %s has murdered %d civillians!",victim->short_descr, number_range(2,20));
	do_say(questman, buf);
	do_say(questman,"The penalty for this crime is death, and you are to deliver the sentence!");
	break;
    }

    if (room->name != NULL)
    {
        sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",victim->short_descr,room->name);
        do_say(questman, buf);

	/* I changed my area names so that they have just the name of the area
	   and none of the level stuff. You may want to comment these next two
	   lines. - Vassago */

	sprintf(buf, "That location is in the general area of %s.",room->area->name);
	do_say(questman, buf);
    }
    ch->questmob = victim->pIndexData->vnum;
    ch->countdown = number_range(10,30);
    }
    return;
}

bool quest_level_diff(int clevel, int mlevel)
{
  clevel = UMIN (100, clevel);
  mlevel = UMIN (100, mlevel);
  /* Can't have quests to kill mobs lower than the player. */
  if (mlevel < clevel)
    return FALSE;
  /* if the mob is within 10 levels, or both are greater than 100 return true*/
  return ((mlevel - clevel < 10) || (mlevel >= 100 && clevel >= 100));

  /* return (((mlevel - clevel > 0 ) || (mlevel >= 100 && clevel >= 100)) && ((clevel > 100) || (mlevel - clevel < 10) ));
 		if (clevel < 6 && mlevel < 15) return TRUE;
		else if (clevel > 6 && clevel < 15 && mlevel < 30) return TRUE;
		else if (clevel > 14 && clevel < 25 && mlevel > 29 && mlevel < 45) return TRUE;
		else if (clevel > 24 && clevel < 35 && mlevel > 44 && mlevel < 55) return TRUE;
		else if (clevel > 34 && clevel < 60 && mlevel > 54 && mlevel < 80) return TRUE;
		else if (clevel > 59 && mlevel > 79) return TRUE;
		else return FALSE; */
}
		
/* Called from update_handler() by pulse_area */

void quest_update(void)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;
    
    for ( d = descriptor_list; d != NULL; d = d->next )
      {
        if (d->character != NULL && d->connected == CON_PLAYING)
	  {
	    
	    ch = d->character;
	    
	    if (ch->nextquest > 0)
	      {
		ch->nextquest--;
		if (ch->nextquest == 0)
		  {
		    send_to_char(AT_WHITE, "You may now quest again.\n\r",ch);
		    continue;
		  }
	      }
	    else if (IS_SET(ch->act,PLR_QUESTOR))
	      {
		if (--ch->countdown <= 0)
		  {
		    char buf [MAX_STRING_LENGTH];
		    
		    ch->nextquest = 10;
		    sprintf(buf, "You have run out of time for your quest!\n\rYou may quest again in %d minutes.\n\r",ch->nextquest);
		    send_to_char(AT_WHITE, buf, ch);
		    REMOVE_BIT(ch->act, PLR_QUESTOR);
		    ch->questgiver = NULL;
		    ch->countdown = 0;
		    ch->questmob = 0;
		  }
		if (ch->countdown > 0 && ch->countdown < 6)
		  {
		    send_to_char(AT_WHITE, "Better hurry, you're almost out of time for your quest!\n\r",ch);
		    continue;
		  }
	      }
	  }
      }
    return;
}

int get_random_skpell( )
{
	int count;
	int num;
	int cls;

	num = 0;
        
	do
	{
		cls = 0;
		num = number_range( 1, MAX_SKILL ) - 1;
		for ( count = 0; count < MAX_CLASS; count++ )
		{
			if( skill_table[num].skill_level[count] < L_APP )
			{
				cls = count;
				break;
			}
		}
	} while (!cls || skill_table[num].slot == 0);

	return num;
}                
