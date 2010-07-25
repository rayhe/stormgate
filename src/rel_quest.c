
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

/* Religious Quest Generation
	Idea: Tyrion
	Code: Ahsile
*/


/*$Id: rel_quest.c,v 1.18 2005/02/22 23:55:18 ahsile Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "merc.h"

/* 
	DO NOT CHANGE!
   	DO NOT CHANGE!!
   	DO NOT CHANGE!!!
   	DO NOT CHANGE!!!!
		- Ahsile
*/

#define  ROOM_MAX		25
#define  GRID_MAX             	20
#define  LEVEL_MAX            	12

#define  ROOM_START           	10

/*
	Below this point is customizable.
		- Ahsile
*/

/* Tell the mud where good/evil/neutral areas start */
#define  REL_GOOD_LOWER       	24601
#define  REL_EVIL_LOWER       	24701
#define  REL_NEUT_LOWER       	24901

/*  Object vnums. Good/Evil starts.
   (No neutral. You either start good or evil)
    Defaults are Seperated by (LEVEL_MAX-1) vnums
*/
#define  REL_PORTAL_GOOD      	24612
#define  REL_PORTAL_EVIL      	24601
#define  REL_PORTAL_EXIT      	24623

#define  REL_ITEM_QUEST		24651
#define  REL_ITEM_COUNT		1

/* 
    Religious mob equipment starts.
    Will load randomly    
*/
#define  REL_EQUIP_GOOD		24701
#define	 REL_EQUIP_EVIL		24766
#define  REL_EQUIP_NEUT		24833

/* Mob vnums */
#define  REL_MOB_EVIL_GRUNT    	24601
#define  REL_MOB_EVIL_BOSS	24650
#define  REL_MOB_NEUT_GRUNT	24701
#define	 REL_MOB_NEUT_BOSS	24750
#define  REL_MOB_GOOD_GRUNT	24801
#define  REL_MOB_GOOD_BOSS	24650

/* 
   Room stuff 
   Names, and Descriptions 
*/
#define  ROOM_HELL_0 		"This is Hell"
#define  ROOM_HELL_1    	"The Pits of Hell"
#define  ROOM_HELL_2    	"Burning Flames"
#define  ROOM_HELL_3    	"The Fall of Light"
#define  ROOM_HELL_D 		"Your are deep in hell.\n\r"

#define  ROOM_ERTH_0    	"On the Plane of Mortals"
#define  ROOM_ERTH_1    	"Walking the Earth"
#define  ROOM_ERTH_2    	"Upon the Soil"
#define  ROOM_ERTH_3    	"The Land of Men and Beasts"
#define  ROOM_ERTH_D    	"You walk the earth.\n\r"

#define  ROOM_HEAV_0 		"The Lair of the Gods"
#define  ROOM_HEAV_1		"A Kingdom of Clouds"
#define  ROOM_HEAV_2		"Holy Power"
#define  ROOM_HEAV_3		"An Air of Sweetness"
#define  ROOM_HEAV_D		"You walk the heavenly plane.\n\r"

// Room data struct
struct RoomData
{
    int index;
    ROOM_INDEX_DATA* Room;
};

// external variable for religion.c to use
bool relquest;

// external function for religion.c to use
bool rel_quest_gen  (CHAR_DATA* ch, int align, int levels);
void rel_quest_goals(CHAR_DATA* ch, int levels, int align);

// local functions
void draw_map(CHAR_DATA* ch, int level, struct RoomData[LEVEL_MAX][GRID_MAX][GRID_MAX]);
void make_and_link(struct RoomData* r, int Level, int Align, int index, int cur, int vnum, int dir1, int dir2);
void make_portal(ROOM_INDEX_DATA* r, int Level, int Align);
void generate_mobs(struct RoomData* r, int Level, int Align);
void equip_mob( CHAR_DATA* ch, int Level, int Align );


bool rel_quest_gen(CHAR_DATA* ch, int align, int levels)
{
    int  Current_Room;
    int  Chance;
    int  index;
    int  Align;
    int  TotalLevels;
    int  GridX;
    int  GridY;
    int  Level;
    int  vnum;
    char buf[MAX_STRING_LENGTH];

    /* 
	Make two arrays. 
	One is indexed by the room number
	The other is a grid for verifying links to other rooms;
    */ 
    struct RoomData Rooms[LEVEL_MAX][ROOM_MAX];
    struct RoomData Grid [LEVEL_MAX][GRID_MAX][GRID_MAX];

    /* Init variables */
    TotalLevels = 0;
    Align = -1;
    index = 0;
    Chance = -1;
    Current_Room = -1;

    /* seed random timer */
    srand( (unsigned)time( NULL ) );

    /* level paramater correct? */
    if (levels < 1 || levels > 4)
	return FALSE;

    /* calc total levels */
    TotalLevels = levels * 3;
    
    /* use our own align variable... why, I dunno */
    Align = align;

    /* align parameter not neutral or empty? */
    if (!Align)
	return FALSE;

    /* choose a starting point based on align */
    if (Align == ALIGN_EVIL)
	vnum = REL_VNUM_LOWER;
    else if (Align == ALIGN_GOOD)
	vnum = REL_VNUM_UPPER;
    else
        return FALSE;
   
    /* Clear Room Array */
    for (Level = 0; Level < TotalLevels;Level++)
    {
        for (Chance = 0; Chance < ROOM_MAX; Chance++)
 	{
                Rooms[Level][Chance].Room = NULL;
                Rooms[Level][Chance].index = -1;
	}
    }

    /* Unlink OLD dungeon */
    for (Level = REL_VNUM_LOWER; Level <= REL_VNUM_UPPER; Level++)
    {
	ROOM_INDEX_DATA* clearRoom = get_room_index(Level);
	if (clearRoom)
	{
	    CHAR_DATA* mob;
	    CHAR_DATA* people[MAX_GROUP];
	    OBJ_DATA*  obj;
	    int chCount = 0;
	    for (index = DIR_NORTH; index < DIR_UP; index++)
	    {
                if (clearRoom->exit[index])
                {
                     free_exit(clearRoom->exit[index]);
		     clearRoom->exit[index]=NULL;		  
	        }
	    }
	    // Kill objects and mobs to avoid duplication.
	    // Sanity check for PC's... no camping the relquest areas
	    for(obj = clearRoom->contents; obj; obj=obj->next_content) 
		{
			if (!obj->deleted)				 
				extract_obj ( obj ); 
		};
	    for(mob=clearRoom->people; mob; mob=mob->next_in_room  ) 
		{
			if (IS_NPC(mob))
			{ 
				if (!mob->deleted)
					extract_char( mob, TRUE );
			}
			else
			{
				people[chCount] = mob;
				chCount++;	
			} 
		};
	    for(Chance=0; Chance<chCount; Chance++)
	    {
		char_from_room( people[Chance] );
		char_to_room( people[Chance], get_room_index( ROOM_VNUM_ALTAR ) );
		send_to_char( AT_WHITE, "The gods remove you from their domain!\n\r", people[Chance]);
	   }
	}
    } 

    /* Init index */
    index = 0;
    
    /* Loop through our levels */
    for (Level = 0; Level < TotalLevels; Level++)
    {
		int loops;
        /* Clear Grid */
        for (Chance=0; Chance < GRID_MAX;Chance++) 
	{
            for (Current_Room = 0; Current_Room < GRID_MAX; Current_Room++)
	    {
		Grid[Level][Chance][Current_Room].Room = NULL;
                Grid[Level][Chance][Current_Room].index = -1;
            }
	}
        
        Grid[Level][ROOM_START][ROOM_START].index = index;
        Grid[Level][ROOM_START][ROOM_START].Room = get_room_index( vnum );
        Rooms[Level][0] = Grid[Level][ROOM_START][ROOM_START];
	
	GridX = ROOM_START;
        GridY = ROOM_START;
        
        if ((index % 25)==0) 
	{
        	/* Prime the vnum. We used one to start */
        	if (Align == ALIGN_GOOD) { vnum--; }
		else { vnum++; }
		index += 1; 
		strcpy(buf, "The Beginning of Your Quest\n\r");
		Rooms[Level][0].Room->name  	  = str_dup(buf); 
        	Rooms[Level][0].Room->description = str_dup("Welcome to the start your journey.\n\r"); 
	} /* First room is already taken care of */
        

	/* Ok, loop until all of our rooms are completed */
		loops = 0;
        do
        {
            int FindX;
            int FindY;
            int RoomIndex;
            int tempX;
            int tempY;
            float temp;

			loops++;
	    temp = ((float)rand()/(float)RAND_MAX);
            Current_Room = (int)(temp*(index - (Level * ROOM_MAX))); /* Pick a Room from 0-X */
                                                              /* Where X = Rooms on this Level */

            /* Find our position on the "grid" */
            if( index > 1)
	    {
                for (FindY = 0; FindY<GRID_MAX;FindY++)
		{
                    for (FindX = 0;FindX<GRID_MAX;FindX++)
		    {
                        if (Grid[Level][FindY][FindX].index == Rooms[Level][Current_Room].index)
			{
                            GridX = FindX;
                            GridY = FindY;
			}
		    }
		}
	    }
            
            /* No exits available, don't bother going through the rest */
            if (
		    Rooms[Level][Current_Room].Room->exit[DIR_NORTH] &&
		    Rooms[Level][Current_Room].Room->exit[DIR_EAST]  &&
		    Rooms[Level][Current_Room].Room->exit[DIR_SOUTH] &&
		    Rooms[Level][Current_Room].Room->exit[DIR_WEST] )
	    {
		continue;
	    }

            /* Translate Index into 0-24, and pick a random chance of exits
               Then, check if the exit is available to be linked */

            RoomIndex = (index - (Level * ROOM_MAX));
            Chance = number_range(0,100);
            
            if (Chance < 25)    /* North */
                {
		    tempY = GridY - 1;
                    tempX = GridX;
                    if (Rooms[Level][Current_Room].Room->exit[DIR_NORTH] ||
		 	tempY < 0 || Grid[Level][tempY][tempX].index != -1)
		    {
			continue; 
		    }

                    make_and_link(Rooms[Level], Level, Align, RoomIndex, Current_Room, vnum, DIR_SOUTH, DIR_NORTH);

		}
	     else if (Chance < 50)    /* East */
                {
		    tempY = GridY;
                    tempX = GridX + 1;
                    if (Rooms[Level][Current_Room].Room->exit[DIR_EAST] ||
			tempX > 19 || Grid[Level][tempY][tempX].index != -1) 
		    {
			continue;
		    }

                    make_and_link(Rooms[Level], Level, Align, RoomIndex, Current_Room, vnum, DIR_WEST, DIR_EAST);

		}
	    else if (Chance < 75)   /* South */
                {
		    tempY = GridY + 1;
                    tempX = GridX;
			
                    if (Rooms[Level][Current_Room].Room->exit[DIR_SOUTH] ||
			tempY > 19 || Grid[Level][tempY][tempX].index != -1)
		    {
			continue;
		    }

		    make_and_link(Rooms[Level], Level, Align, RoomIndex, Current_Room, vnum, DIR_NORTH, DIR_SOUTH);

		}
           else       /* West */
		{
                    tempY = GridY;
                    tempX = GridX - 1;

                    if (Rooms[Level][Current_Room].Room->exit[DIR_WEST] ||
			tempX < 0 || Grid[Level][tempY][tempX].index != -1)
		    {
			continue;
		    } 

		    make_and_link(Rooms[Level], Level, Align, RoomIndex, Current_Room, vnum, DIR_EAST, DIR_WEST);

		}

            /* Ok, everything went good. Finish off the room creation
               And put it on the grid */
            Rooms[Level][RoomIndex].index = index;
            Grid[Level][tempY][tempX] = Rooms[Level][RoomIndex];
            Grid[Level][GridY][GridX] = Rooms[Level][Current_Room];
	
	    if (Align == ALIGN_GOOD)
	    {
		vnum--;
	    } else
	    {
		vnum++;
	    }
            
            index++;

	} while ((index - (Level * ROOM_MAX)) < ROOM_MAX);
    
	// Make a portal to the next level
	if (Level < (TotalLevels - 1))
		make_portal(Rooms[Level][number_range(1, (ROOM_MAX-1))].Room, Level, Align);	

	// Create Mobs and equip them
	generate_mobs(Rooms[Level], Level, Align);
    }

    // Now make the exit portal
    Rooms[TotalLevels-1][number_range(1, (ROOM_MAX-1))].Room->contents = create_object( get_obj_index( REL_PORTAL_EXIT), TotalLevels-1);
    
    for (Level=0;Level<TotalLevels;Level++)
    {
	draw_map(ch,Level,Grid);
    }
    
    /* Announce the quest */
    sprintf(buf, "The ground shakes as the gods send %s's party upon a quest!\n\r",ch->name);
    talk_channel_info( ch, buf, CHANNEL_INFO, "info" );

    return TRUE;
}

void make_portal(ROOM_INDEX_DATA* r, int Level, int Align)
{
	// Load a portal to the next level
	OBJ_DATA* o;

	if (Align==ALIGN_GOOD)
		o = create_object( get_obj_index( REL_PORTAL_GOOD + Level ), Level);
	else
		o = create_object( get_obj_index( REL_PORTAL_EVIL + Level ), Level);

	obj_to_room( o, r);
}

void make_and_link(struct RoomData* r, int Level, int Align, int index, int cur, int vnum, int dir1, int dir2)
{
	char* name = NULL;
	char* desc = NULL;
	int   rdm  = number_range(0, 3);

	/* Make New Room */
	r[index].Room    			= get_room_index ( vnum );
        r[index].Room->exit[dir1]  	      	= new_exit();
	r[index].Room->room_flags		= ROOM_NO_RECALL | ROOM_NO_ASTRAL_IN | ROOM_NO_ASTRAL_OUT;
	r[cur].Room->exit[dir2] 	  	= new_exit();
	/* Link Rooms */
	r[index].Room->exit[dir1]->to_room     	= r[cur].Room;
        r[index].Room->exit[dir1]->vnum        	= r[cur].Room->vnum;
        r[cur].Room->exit[dir2]->to_room  	= r[index].Room;
	r[cur].Room->exit[dir2]->vnum		= vnum;

	/* Pick random names and descriptions */
	if (Level < 4)
	{
		if (Align==ALIGN_GOOD)
		{
			switch(rdm)
			{
				case 0:
					name = str_dup(ROOM_HEAV_0);
					break;
				case 1:
					name = str_dup(ROOM_HEAV_1);
					break;
				case 2:
					name = str_dup(ROOM_HEAV_2);
					break;
				case 3:
					name = str_dup(ROOM_HEAV_3);
					break;
			}
			desc = str_dup(ROOM_HEAV_D);			
		} else
		{
			switch(rdm)
			{
				case 0:
					name = str_dup(ROOM_HELL_0);
					break;
				case 1:
					name = str_dup(ROOM_HELL_1);
					break;
				case 2:
					name = str_dup(ROOM_HELL_2);
					break;
				case 3:
					name = str_dup(ROOM_HELL_3);
					break;
			}
			desc = str_dup(ROOM_HELL_D);			
		}
	}
	else if (Level < 8)
	{
		switch(rdm)
		{
			case 0:
				name = str_dup(ROOM_ERTH_0);
				break;
			case 1:
				name = str_dup(ROOM_ERTH_1);
				break;
			case 2:
				name = str_dup(ROOM_ERTH_2);
				break;
			case 3:
				name = str_dup(ROOM_ERTH_3);
				break;
		}
		desc = str_dup(ROOM_ERTH_D);			
	}
	else
	{
		if (Align==ALIGN_EVIL)
		{
			switch(rdm)
			{
				case 0:
					name = str_dup(ROOM_HEAV_0);
					break;
				case 1:
					name = str_dup(ROOM_HEAV_1);
					break;
				case 2:
					name = str_dup(ROOM_HEAV_2);
					break;
				case 3:
					name = str_dup(ROOM_HEAV_3);
					break;
			}
			desc = str_dup(ROOM_HEAV_D);			
		} else
		{
			switch(rdm)
			{
				case 0:
					name = str_dup(ROOM_HELL_0);
					break;
				case 1:
					name = str_dup(ROOM_HELL_1);
					break;
				case 2:
					name = str_dup(ROOM_HELL_2);
					break;
				case 3:
					name = str_dup(ROOM_HELL_3);
					break;
			}
			desc = str_dup(ROOM_HELL_D);			
		}
	}
	r[index].Room->name = name;
	r[index].Room->description = desc;
}

void generate_mobs(struct RoomData* r, int Level, int Align)
{
	CHAR_DATA* mob;
	int i;
	int min_grunt, max_grunt, min_boss;
	/* ****
	   not needed right now
	   ****
	   int m_lvl, m_hp, m_align;
	*/

	// Make a random mob
	for(i = 0; i < ROOM_MAX - 1; i++)
	{

		if (Level < 4)
		{
			if (Align == ALIGN_EVIL)
			{
				min_grunt = REL_MOB_GOOD_GRUNT;
				max_grunt = REL_MOB_GOOD_BOSS - 1;
				min_boss  = REL_MOB_GOOD_BOSS; 
			} else
			{
				min_grunt = REL_MOB_EVIL_GRUNT;
				max_grunt = REL_MOB_EVIL_BOSS - 1;
				min_boss  = REL_MOB_EVIL_BOSS; 
			}
		}
		else if (Level < 8)
		{
			min_grunt = REL_MOB_NEUT_GRUNT;
			max_grunt = REL_MOB_NEUT_BOSS - 1;
			min_boss  = REL_MOB_NEUT_BOSS; 

		}
		else
		{
			if (Align == ALIGN_EVIL)
			{
				min_grunt = REL_MOB_EVIL_GRUNT;
				max_grunt = REL_MOB_EVIL_BOSS - 1;
				min_boss  = REL_MOB_EVIL_BOSS; 
			} else
			{
				min_grunt = REL_MOB_GOOD_GRUNT;
				max_grunt = REL_MOB_GOOD_BOSS - 1;
				min_boss  = REL_MOB_GOOD_BOSS; 
			}
		}

		if (number_percent() < 75)
		{ 
			mob = create_mobile( get_mob_index( number_range( min_grunt, max_grunt ) ) );
			equip_mob(mob, Level, Align);
			char_to_room( mob, r[i].Room);
		}
	}

	mob = create_mobile( get_mob_index( min_boss + Level ) );
	SET_BIT(mob->act, ACT_RELBOSS);
	SET_BIT(mob->affected_by4, AFF_IMMORTAL);
	mob->long_descr = str_dup("The boss of this level.\n\r");
	equip_mob(mob, Level, Align);
	char_to_room( mob, r[ROOM_MAX-1].Room);

	return;
}

void equip_mob( CHAR_DATA* mob, int Level, int Align )
{
	int i = 0;
	int MAX_EQP = 27;
	OBJ_DATA* obj;
	// Randomly equip mob
	for (i = 0; i < MAX_EQP; i++)
	{
		if (number_percent() < 30)
		{
			obj = create_object( get_obj_index( REL_EQUIP_GOOD + i), mob->level);
			if (!str_cmp(obj->name, str_empty))
			{
				obj->name 	= str_dup(wear_flags[i].name);
				obj->short_descr= str_dup(wear_flags[i].name);
			}
			/*
			{
				char buf[MAX_STRING_LENGTH];
				sprintf(buf, "Equipping mob %s with %s", mob->short_descr, obj->short_descr);
				bug(buf, 0);
			}
			*/
			obj_to_char( obj, mob);
			equip_char( mob, obj, i);
		}	
	}
}

void draw_map(CHAR_DATA* ch, int level, struct RoomData Grid[LEVEL_MAX][GRID_MAX][GRID_MAX])
{
   // Draw the map
   int Row = 0;
   int Col = 0;
   int rIndex = 0;
   char line[MAX_STRING_LENGTH];
   char fname[MAX_STRING_LENGTH];
   FILE* fp;

   /* Open a file for the MAP command to read */
   sprintf(fname, "level%d.map", level);
   fp = fopen(fname,"w");
   
   /* 	
	The second reason for the grid!
	Easy drawing! Dump the grid to the file
   */
   for (Row = 0;Row<GRID_MAX; Row++)
   {
	// start blank line
	strcpy(line,"");
	for (Col = 0;Col<GRID_MAX;Col++)
	{
	    // is there a room here?
	    if (Grid[level][Row][Col].index != -1)
	    {
		char buf[MAX_STRING_LENGTH];
		// calculate the index (0 to ROOM_MAX-1)
		rIndex = (Grid[level][Row][Col].index - (level * ROOM_MAX));
		sprintf(buf, "Level: %d Row: %d Col: %d Orig Idx: %d New Idx: %d", level, Row, Col, Grid[level][Row][Col].index, rIndex);
	  	log_string(buf, CHANNEL_NONE, -1);

		// starting point
		if (rIndex == 0)
			strcat(line, "O");
		// a portal (the only room contents on start)
		// if someone drops something, it gets killed later
		else if (Grid[level][Row][Col].Room->contents)
			strcat(line, "#");
		// Regular room
		else
			strcat(line, "X");
	    } else
	    {
		// blank
		strcat(line, " ");
	    }
	}
	// is there a room on this line? if not, don't bother spitting
	// it out. 
	if (strstr(line,"X") || strstr(line,"#") || strstr(line,"O"))
	{
	   strcat(line,"\n");
	   fprintf(fp,"%s",line);
	}
    }
    fclose(fp);
}

void rel_quest_goals(CHAR_DATA* ch, int levels, int align)
{
	int i, start, end;
	OBJ_DATA* obj;
	ROOM_INDEX_DATA* room;
	if (align==ALIGN_GOOD)
	{
		start = (levels * 3) - 1;
		end = -1;
	} else
	{
		start = 0;
		end = levels *3;
	}
	for (i = 0; i < (levels * 3); i++)
	{
		while(1)
		{	
			room = get_room_index( number_range( REL_VNUM_LOWER + (start*ROOM_MAX) + 1, REL_VNUM_LOWER + ((start+1)*ROOM_MAX - 2 ) ) );

			if (!room)
				continue;

			if (number_percent() < 40)
			{
				obj = create_object( get_obj_index( number_range( REL_ITEM_QUEST, REL_ITEM_QUEST + REL_ITEM_COUNT - 1) ), 1 );
				if (!obj)
					continue;

				obj->timer = ch->rcountdown * 2;
				obj_to_room( obj, room);
				ch->rquestobj[i] = obj->pIndexData->vnum;
				break;
			} else
			{
				if (!room->people)
					continue;
			
				ch->rquestmob[i] = room->people->pIndexData->vnum;
				break;
			}
		};
		if (start > end)
		    start--;
		else
		    start++;

	}
}
