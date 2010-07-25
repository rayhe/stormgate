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

/*$Id: religion.c,v 1.12 2005/02/22 23:55:19 ahsile Exp $*/

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
#include "merc.h"

#define REL_QUEST_MIN 50

extern bool rel_quest_gen  (CHAR_DATA* ch, int align, int levels);
extern void rel_quest_goals(CHAR_DATA* ch, int levels, int align);
void rquest_update args(( void ));

void do_religions( CHAR_DATA *ch, char *argument )
{
    RELIGION_DATA    *pReligion;
    char          buf[MAX_STRING_LENGTH];
    char result [ MAX_STRING_LENGTH*2 ];	/* May need tweaking. */
    int           chance;

    if ( religion_first == NULL )
      return;

    sprintf( result, "[%3s] [%4s] [%12s] [%7s] [%7s] [%7s] [%6s]\n\r",
       "Num", "Name", "Deity", "Followers", "Pkills", "Pkilled", "% Wins");

    for ( pReligion = religion_first->next; pReligion; pReligion = pReligion->next )
    {
        chance = 0;
        if(pReligion->pkills != 0 || pReligion->pdeaths != 0)
        chance = (int)( ((float)pReligion->pkills / (float)(pReligion->pkills + pReligion->pdeaths) ) * 100 );

	sprintf( buf, "[%3d] [%4s] [%12s] [%9d] [%7d] [%7d] [%6d]\n\r",
	     pReligion->vnum,
	     pReligion->name,
	     pReligion->deity,
	     pReligion->members,
	     pReligion->pkills,
	     pReligion->pdeaths,
	     chance );
	     strcat( result, buf );
    }

    send_to_char(AT_WHITE, result, ch );
    return;
}

void do_religioninfo( CHAR_DATA *ch, char *argument )
{
    RELIGION_DATA    *pReligion;
    char             buf[MAX_STRING_LENGTH];
    int              num;
    char             arg1[MAX_INPUT_LENGTH];
    
    argument = one_argument(argument, arg1);
    
    if (!(is_number(arg1)))
    {
      send_to_char(AT_WHITE, "Syntax:  religioninfo <religion number>\n\r", ch );
      send_to_char(AT_WHITE, "Use the command religions to find a religion number.\n\r", ch );
      return;
    }
    num = atoi(arg1);
    if (!(pReligion = get_religion_index(num)))
    {
     send_to_char( AT_WHITE, "Illegal religion number, please try again.\n\r", ch );
     return;
    }
    
    sprintf( buf, "------------------Information on <%s>-----------------\n\r\n\r", pReligion->name );
    send_to_char(AT_WHITE, buf, ch );
    sprintf( buf, "Name:        %s\n\r", pReligion->shortdesc );
    send_to_char(AT_LBLUE, buf, ch );
    sprintf( buf, "Deity        [%12s]\n\r", pReligion->deity );
    send_to_char(AT_CYAN, buf, ch );
    sprintf( buf, "Followers    [%12d]\n\r", pReligion->members );
    send_to_char(AT_BLUE, buf, ch );
    sprintf( buf, "Pkills:      [%12d]\n\r", pReligion->pkills );
    send_to_char(AT_DBLUE, buf, ch );
    sprintf( buf, "Pkilled:     [%12d]\n\r", pReligion->pdeaths );
    send_to_char(AT_BLUE, buf, ch );
    sprintf( buf, "Mkills:      [%12d]\n\r", pReligion->mkills );
    send_to_char(AT_CYAN, buf, ch );
    sprintf( buf, "Mkilled:     [%12d]\n\r", pReligion->mdeaths );
    send_to_char(AT_LBLUE, buf, ch );
    sprintf( buf, "Description:\n\r%s", pReligion->description );
    send_to_char( AT_WHITE, buf, ch );
    return;
}

void do_relquest(CHAR_DATA* ch, char *argument)
{
    do_crusade(ch, argument);
}

void do_crusade(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA* d;
    CHAR_DATA *questman;
    CHAR_DATA *boss;
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
        send_to_char(AT_WHITE, "CRUSADE commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY ADVANCE.\n\r",ch);
        send_to_char(AT_WHITE, "For more information, type 'HELP CRUSADE'.\n\r",ch);
        return;
    }

    if (!str_cmp(arg1, "time"))
    {
	if (IS_SET(ch->act2, PLR_RELQUEST))
	{
		sprintf(buf, "You have %d minutes left on your crusade!\n\r", ch->rcountdown);
	} else
	{
		if (ch->rnextquest > 0)
		{
			sprintf(buf, "You have %d minutes left before your next crusade!\n\r", ch->rnextquest);
		} else
		{
			strcpy(buf, "You are free to do a crusade right now!\n\r");
		}
	}
	send_to_char(AT_WHITE, buf, ch);
	return;
    } else if (!str_cmp(arg1, "points"))
    {
	sprintf(buf, "You have %d relgion points.\n\r", ch->rquestpoints);
	send_to_char(AT_WHITE, buf, ch);
	return;
    } else if (!str_cmp(arg1, "info"))
    {
	OBJ_INDEX_DATA* oinfo;
	MOB_INDEX_DATA* minfo;

	if (!IS_SET(ch->act2, PLR_RELQUEST))
	{
		send_to_char(AT_WHITE, "You are not on a crusade!\n\r", ch);
		return;
	}

	if (ch->rquestobj[ch->relquest_level]==0 && ch->rquestmob[ch->relquest_level]==0)
	{
		if (ch->in_room->vnum <= REL_VNUM_UPPER && ch->in_room->vnum >= REL_VNUM_LOWER)
		{
			send_to_char(AT_WHITE, "You are almost finished your crusade.\n\rFind the portal to the next level!\n\r", ch);
			return;
		} else
		{
			send_to_char(AT_WHITE, "You are almost finished your crusade!\n\rFind your DEITY and COMPLETE you quest!\n\r",ch);
			return;
		}
	} 

	if (ch->rquestmob[ch->relquest_level])
	{
		if (ch->rquestmob[ch->relquest_level] == -1)
		{
			send_to_char(AT_WHITE, "You are almost done your crusade!\n\rFind the boss and ADVANCE!\n\r", ch);
			return;
		} else
		{
			minfo = get_mob_index( ch->rquestmob[ch->relquest_level] );
			sprintf(buf, "You are on a crusade to slay the dreaded %s!\n\r", minfo->short_descr);
		}
   	} else
	{
		oinfo = get_obj_index( ch->rquestobj[ch->relquest_level] );
		sprintf(buf, "You are on a crusade to retrieve the fabled %s!\n\r", oinfo->short_descr);
	}
	send_to_char(AT_WHITE, buf, ch);
	return; 
    }

    for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room)
    {
        if (!IS_NPC(questman)) continue;
        if (IS_SET(questman->act, ACT_IS_DEITY)) break;
    }

    for (boss = ch->in_room->people; boss; boss = boss->next_in_room)
    {
	if (!IS_NPC(boss)) continue;
	if (IS_SET(boss->act, ACT_RELBOSS)) break;
    }


    if (!str_cmp(arg1, "request"))
    {
        int member_count = 0;
	CHAR_DATA* group[MAX_GROUP];

  	if (questman==NULL)
        {
   		send_to_char(AT_WHITE, "Your deity is not here!\n\r", ch);
    	  	return;
        }


        if (IS_SET(ch->act2, PLR_RELQUEST))
	{
		send_to_char(AT_WHITE, "You are already ON a crusade!\n\r",ch);
		return;
	}
	if (relquest)
	{
		send_to_char(AT_WHITE, "The gods have already set a crusade to be finished!\n\r",ch);
		return;
	}
	if (ch->master)
	{
	    send_to_char(AT_WHITE, "Only your group leader can ask for a crusade!\n\r", ch);
	    return;
	}
	if (ch->alignment > -500 && ch->alignment < 500) /* || (ch->religion == TIME_AND_FATE */
	{
	    send_to_char(AT_WHITE, "You are neutral. You cannot go upon a crusade!\n\r",ch);
	    return;
	}
	if (ch->leader)
	{
	    send_to_char(AT_WHITE, "Only the leader of your group can request a crusade!\n\r", ch);
	    return;
	}
	for ( d = descriptor_list; d; d = d->next )
        {
	    RELIGION_DATA* rd = NULL;
	    CHAR_DATA* gch = NULL;

	    if ( d->connected != CON_PLAYING )
        	continue;

	    gch = d->character;

	    if ( !is_same_group(gch , ch ) )
                continue;

	    if ( gch->in_room != ch->in_room )
	    {
		send_to_char(AT_WHITE, "Your must gather your party before venturing forth!\n\r", ch);
		return;
	    }

	    if ( gch->rnextquest > 0 )
	    {
		sprintf(buf, "%s must wait before doing another crusade!\n\r", gch->name);
		send_to_char(AT_WHITE, "You must wait before your next Crusade!\n\r", gch);
		send_to_char(AT_WHITE, buf, ch);
		return;
	    }
          
	    if ( gch->level < REL_QUEST_MIN )
	    {
		sprintf(buf, "%s is not high enough level to do a Crusade!\n\r", gch->name);
		send_to_char(AT_WHITE, "You are not high enough level to complete a Crusade!\n\r", gch);
		send_to_char(AT_WHITE, buf, ch);
		return;
	    }

	    rd = get_religion_index( ch->religion );
/*
Quest Restriction to Deities
Out for debug
	    if (!strstr(questman->short_descr, rd->deity ) )
	    {
	        sprintf(buf, "%s would not appreciate you devoting your crusades to %s!\n\rFind your own Deity!\n\r", rd->deity, questman->short_descr);
		send_to_char(AT_WHITE, buf, ch);
		return;
	    }
*/
	    group[member_count] = gch;
	    member_count++;
	    if (ch->alignment >= 500)
	    {
		if (gch->alignment <= -500) /* || gch->religion == EVIL */
		{
		    sprintf(buf, "Your deity will not accept %s upon your crusade!\n\r", gch->name);
		    send_to_char(AT_WHITE, buf, ch);
		    return;
		}
	    } else if (ch->alignment <= -500)
	    {
		if (gch->alignment >= 500) /* || gch->religion == GOOD */
		{
		    sprintf(buf, "Your deity will not accept %s upon your crusade!\n\r", gch->name);
		    send_to_char(AT_WHITE, buf, ch);
		    return;
		}
	    }
        }
        if (member_count < 2)
	{
	    send_to_char(AT_WHITE, "You need 2 or more heroes for a crusade!\n\r", ch);
	    return;
	}
        if (ch->alignment >= 500)
        {
 	    int levels = (((((ch->level > LEVEL_DEMIGOD) ? LEVEL_DEMIGOD : ch->level )) - REL_QUEST_MIN)/(14))+1;
	    if(rel_quest_gen(ch, ALIGN_GOOD, levels))
            {
		int qtime = number_range(15, 45);
		int curmemb;
		relquest=TRUE;
		/* Made it... let's go! */

		rel_quest_goals(ch, levels, ALIGN_GOOD);
		for ( curmemb = 0; curmemb < member_count; curmemb++)
        	{
		    int i;
		    for (i = 0; i < levels; i++)
		    {
			group[curmemb]->rquestobj[i] = ch->rquestobj[i];
			group[curmemb]->rquestmob[i] = ch->rquestmob[i];
		    }
		    group[curmemb]->relquest_level = 0;
		    group[curmemb]->rcountdown = qtime;
	    	    SET_BIT(group[curmemb]->act2, PLR_RELQUEST);
		    char_from_room(group[curmemb]);
		    char_to_room  (group[curmemb], get_room_index( REL_VNUM_UPPER ) );
		    send_to_char( AT_WHITE, "The smell of honey and sweet perfume fills your nose!\n\r", group[curmemb]);
		    interpret( group[curmemb], "look" );
		    interpret( group[curmemb], "crusade info" );
		}
	        return;

	    } else
	    {
		bug("rel_quest_gen: Failed to create quest!",0);
	    }                
	} else if (ch->alignment <= -500)
	{
	    int levels = (((((ch->level > LEVEL_DEMIGOD) ? LEVEL_DEMIGOD : ch->level )) - REL_QUEST_MIN)/( 14 ))+1;
	    if(rel_quest_gen(ch, ALIGN_EVIL, levels))
     	    {
		int qtime = number_range(15, 45);
		int curmemb;
		relquest=TRUE;

		/* Made it... let's go! */
		rel_quest_goals(ch, levels, ALIGN_EVIL);
		for ( curmemb=0; curmemb < member_count; curmemb++)
        	{
		    int i;
		    for (i = 0; i < levels; i++)
		    {
			group[curmemb]->rquestobj[i] = ch->rquestobj[i];
			group[curmemb]->rquestmob[i] = ch->rquestmob[i];
		    }
		    group[curmemb]->relquest_level = 0;
		    group[curmemb]->rcountdown = qtime;
	    	    SET_BIT(group[curmemb]->act2, PLR_RELQUEST);
		    char_from_room(group[curmemb]);
		    char_to_room  (group[curmemb], get_room_index( REL_VNUM_LOWER ) );
		    send_to_char( AT_RED, "The flames of hell singe your armor!\n\r", group[curmemb]);
		    interpret( group[curmemb], "look" );
	   	    interpret( group[curmemb], "crusade info" );
		}
	        return;
	    } else
	    {
		bug("rel_quest_gen: Failed to create quest!",0);
	    }                

	}
    } else if (!str_cmp(arg1,"complete"))
    {
    	if (questman==NULL)
    	{
   		send_to_char(AT_WHITE, "Your deity is not here!\n\r", ch);
    		return;
        }

	if (IS_SET(ch->act2, PLR_RELQUEST) && ch->rcountdown > 0)
        {
	    CHAR_DATA* gch = NULL;
	    int qp;
	    qp = number_range( ch->level/4, ch->level/2);
	    if (ch->leader || ch->master)
	    {
		send_to_char(AT_WHITE, "Only the group leader can complete a crusade!\n\r", ch);
		return;
	    }
	    for (d=descriptor_list; d; d = d->next)
	    {     
	    	if ( d->connected != CON_PLAYING )
        		continue;
		if (is_same_group(d->character, ch))
		{
			if (d->character->in_room != ch->in_room)
			{
				send_to_char(AT_WHITE, "You must gather your party before completing a crusade!\n\r", ch);
				return;
			}
		}	    	
	    }
	    // the whole party is in the room
	    for (gch=ch->in_room->people; gch; gch=gch->next_in_room)
	    {
	    	if (is_same_group(ch, gch))
		{
			int gqp = number_fuzzy(qp);
			sprintf(buf, "You have gained %d religion points for your service!\n\r", gqp);
			send_to_char(AT_WHITE, "YAY! You completed your crusade!\n\r",gch);
			gch->rquestpoints += gqp;
    			gch->rnextquest = number_range(10, 30);
    			gch->rcountdown = 0;
			send_to_char(AT_WHITE, buf, gch);
	    		REMOVE_BIT(gch->act2, PLR_RELQUEST);
		}
	    }

	    relquest=FALSE; 
	    return;
        } else
	{
	    send_to_char(AT_WHITE, "You are not on a religious crusade!\n\r",ch);
	    return;
	}
    } else if(!str_cmp(arg1, "list"))
    {
    	if (questman==NULL)
    	{
   		send_to_char(AT_WHITE, "Your deity is not here!\n\r", ch);
    		return;
    	}

	send_to_char(AT_WHITE, "Nothing to list yet.\n\r", ch);
	return;
    } else if(!str_cmp(arg1, "buy"))
    {
    	if (questman==NULL)
    	{
   		send_to_char(AT_WHITE, "Your deity is not here!\n\r", ch);
    		return;
	}

	send_to_char(AT_WHITE, "Nothing to buy quite yet.\n\r", ch);
	return;
    } else if(!str_cmp(arg1, "advance"))
    {
	DESCRIPTOR_DATA* d;

	if(!IS_SET(ch->act2, PLR_RELQUEST))
	{
		send_to_char(AT_WHITE, "You cannot advance a crusade if you are not on one!\n\r", ch);
		return;
	}
	if(ch->leader || ch->master)
	{
		send_to_char(AT_WHITE, "Only the group leader can advance a crusade!\n\r", ch);
		return;
	}
	
	if (!boss)
	{
		send_to_char(AT_WHITE, "You need to find the boss of this level before you can advance!\n\r", ch);
		return;
	}

	if (ch->rquestmob[ch->relquest_level] > 0 && ch->rcountdown > 0)
	{
		send_to_char(AT_WHITE, "You have not completed this section of your crusade!\n\r", ch);
		return;
	} else if (ch->rquestobj[ch->relquest_level] > 0 && ch->rcountdown > 0)
	{
		bool found = FALSE;
		OBJ_DATA* obj;
		for (obj = ch->carrying; obj; obj = obj->next)
		{
			if (obj->pIndexData->vnum == ch->rquestobj[ch->relquest_level])
			{
				found = TRUE;
				extract_obj(obj);
			}
		}
		if (!found)
		{
			send_to_char(AT_WHITE, "You have not completed this section of your crusade!\n\r", ch);
			return;
		}
	} 
	ch->rquestobj[ch->relquest_level]=0;
	ch->rquestmob[ch->relquest_level]=0;
	for (d=descriptor_list; d; d = d->next)
        {
                if ( d->connected != CON_PLAYING )
                        continue;
                if (is_same_group(d->character, ch))
                {
			d->character->rquestmob[ch->relquest_level] = 0;
			d->character->rquestobj[ch->relquest_level] = 0;
			send_to_char(AT_WHITE, "You feel free to pursue the next part of your crusade!\n\r", d->character);
		}
	} 
    } else
    {
        send_to_char(AT_WHITE, "CRUSADE commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY ADVANCE.\n\r",ch);
        send_to_char(AT_WHITE, "For more information, type 'HELP CRUSADE'.\n\r",ch);
        return;
    }
}

void rquest_update(void)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;
    int qcount = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
      {
        if (d->character != NULL && d->connected == CON_PLAYING)
          {

            ch = d->character;

            if (ch->rnextquest > 0)
              {
                ch->rnextquest--;
                if (ch->rnextquest == 0)
                  {
                    send_to_char(AT_WHITE, "The gods allow you to crusade again.\n\r",ch);
                  }
              }
            else if (IS_SET(ch->act2,PLR_RELQUEST))
              {
		qcount++;
                if (--ch->rcountdown <= 0)
                  {
                    char buf [MAX_STRING_LENGTH];
		    int vnum;

                    ch->rnextquest = number_range(10, 30);
                    sprintf(buf, "You have run out of time for your crusade!\n\rYou may crusade again in %d minutes.\n\r",ch->rnextquest);
                    send_to_char(AT_WHITE, buf, ch);
                    REMOVE_BIT(ch->act2, PLR_RELQUEST);
                    ch->rcountdown = 0;
			
		    vnum = ch->in_room->vnum;
		    if (vnum >= REL_VNUM_LOWER && vnum <= REL_VNUM_UPPER)
		    {
			send_to_char(AT_YELLOW, "Your vision becomes fuzzy and you wake up in Methidoral!\n\r", ch);
		        char_from_room( ch );
			char_to_room( ch, get_room_index( ROOM_VNUM_ALTAR ) );
			interpret( ch, "look" );
		    }
                  }
                if (ch->rcountdown > 0 && ch->rcountdown < 6)
                  {
                    send_to_char(AT_WHITE, "Better hurry, you're almost out of time for your crusade!\n\r",ch);
                  }
              }
          }
      }
    if (!qcount)
    {
	relquest = FALSE;
    }
    return;
}

void do_map(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    FILE* fp;
    
    if (!IS_SET(ch->act2, PLR_RELQUEST))
    {
	send_to_char(AT_WHITE, "You can only see maps while on crusades!\n\r", ch);
	return;
    }

    sprintf(buf, "Crusade Map for Level %d\n\r\n\r", ch->relquest_level+1);
    send_to_char(AT_WHITE, buf, ch);
    sprintf(buf, "level%d.map", ch->relquest_level);

    fp = fopen(buf, "r");

    buf[1]='\0';

    while (!feof(fp))
    {
	fread(buf, sizeof(char), 1, fp);
        if (strcmp(buf, "\n")==0)
		send_to_char(AT_WHITE, "\n\r", ch);
        else
        	send_to_char(AT_WHITE, buf, ch);
    };

    send_to_char(AT_WHITE, "\n\rX: Room\n\rO: Start\n\r#: Exit\n\r", ch);

    return;
}
