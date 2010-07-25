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

/*$Id: sqldb.c,v 1.10 2005/02/22 23:55:19 ahsile Exp $*/

/* 
 * Database Creation Script included as mud.sql (may be in mud.zip)
 * Turn this system on by definining SQL_SYSTEM in config.h
 *
 * File conversion from old merc format -> sql is included
 * built for win32 + mysql, but could be changed easily
 * -Ahsile 
 */

/*
 * Due to a slow machine, we coded the SQL system to use memory tables
 * and only dump everything on a timed basis. Unfortunately we never
 * made a full conversion to a db system by only pulling data on demand.
 * Data is all stored in the db, and loaded at startup just like a
 * normal mud, but rather than loading from files we load from the
 * database.
 *      - Ahsile - mcarpent@zenwerx.com
 */

/**********************
 * Mandatory Includes *
 **********************/
#ifdef RUN_AS_WIN32SERVICE
#include <windows.h>
#endif

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
/**************************
 * End Mandatory Includes *
 **************************/



/* 
 * Conditional compile section 
 * Build only if sql system is turned on 
 */

#ifdef SQL_SYSTEM

#ifdef RUN_AS_WIN32SERVICE
#pragma comment(lib,"libmysql")
#endif

#include "/usr/local/mysql/include/mysql.h"

/* prog defines */
#define LOAD_MOB 0
#define LOAD_OBJ 1
#define LOAD_ROOM 2
#define LOAD_EXIT 3

/* table name defines for functions who use the same
   sql for different tables */
#define TABLE_ROOM	"tblroomdescriptions"
#define TABLE_OBJ	"tblobjectdescriptions"
#define TABLE_INV   "tblinventory"
#define TABLE_DET   "tblinventorydetail"
#define DIRTY_INV   "fast_inv"
#define DIRTY_DET   "fast_det"

/* External Functions */
void area_sort			( AREA_DATA *pArea			);
void religion_sort		( RELIGION_DATA* pReligion	);
void clan_sort			( CLAN_DATA *pClan			);
void assign_area_vnum	( int vnum					);
void ch_stripbadinv 	( CHAR_DATA* ch				);
void link_reset			( ROOM_INDEX_DATA* pR, RESET_DATA* pReset);

/* These are for creating temporary memory tables */
bool		remove_fastboot_tables	();
bool		create_fastboot_tables	();
bool		remove_fast_invtable	();
bool		create_fast_invtable	();



/* Helper SQL Functions */
MYSQL_RES*	do_query		( char* query										);
MYSQL_ROW	get_row			( MYSQL_RES* res									);
bool		sql_system_init	( char* DBName, char* User, char* Pass, char* Host	);
char*       escape_query	( char* query										);
int			rowcount		( MYSQL_RES* res									);
void		cleanup_query	( MYSQL_RES* res									);
void sql_validate_resets	( RESET_DATA* pReset								);
bool sql_remove_old_inv		( int CharID, bool Dirty							);
long	last_id			( );

/* local variables */
MYSQL*		sql;
int			query_count;
time_t		boot_time;
CHAR_DATA*  dirty_list;

/* external globals */
extern char*			help_greeting_one;
extern char*			help_greeting_two;
extern char*			help_greeting_three;
extern char*			help_greeting_four;
extern char*			help_greeting_five;
extern int				maxSocial;
extern DISABLED_DATA*	disabled_first;

bool sql_system_init(char* DBName, char* User, char* Pass, char* Host)
{
	dirty_list	= NULL;
	query_count = 0;
	boot_time	= time(NULL);
	sql			= mysql_init(NULL);	

	return (bool)mysql_real_connect(sql, Host, User, Pass, DBName, 0, NULL, 0);

}

bool create_fastboot_tables()
{
	char* fast_exits = "INSERT INTO fast_exit  SELECT * FROM tblroomexits";
	char* fast_obj   = "INSERT INTO fast_obj   SELECT * FROM tblobjects";
	char* fast_objaff= "INSERT INTO fast_objaf SELECT * FROM tblobjectaffects";
	char* fast_mobs  = "INSERT INTO fast_mob  SELECT * FROM tblmobiles";

	// make sure we're clean
	if (!remove_fastboot_tables())
	{
		log_string("Couldn't empty fast boot tables!", CHANNEL_NONE, -1);
		return FALSE;
	}

	if (!do_query(fast_exits))
	{
		log_string("Couldn't create fast_exits!", CHANNEL_NONE, -1);
		return FALSE;
	}

	if (!do_query(fast_obj))
	{
		log_string("Couldn't create fast_obj!", CHANNEL_NONE, -1);
		return FALSE;
	}

	if (!do_query(fast_objaff))
	{

		log_string("Couldn't create fast_objaff!", CHANNEL_NONE, -1);
		return FALSE;
	}

	if (!do_query(fast_mobs))
	{
		log_string("Couldn't create fast_mobs!", CHANNEL_NONE, -1);
		return FALSE;
	}

	return TRUE;
}

bool remove_fastboot_tables()
{
	// Free up memory
	char* rem_exits		= "DELETE FROM fast_exit";
	char* rem_objs		= "DELETE FROM fast_obj";
	char* rem_objaffs	= "DELETE FROM fast_objaf";
	char* rem_mobs		= "DELETE FROM fast_mob";

	if(!do_query(rem_exits))
	{
		return FALSE;
	}
	if(!do_query(rem_objs))
	{
		return FALSE;
	}
	if(!do_query(rem_objaffs))
	{
		return FALSE;
	}
	if(!do_query(rem_mobs))
	{
		return FALSE;
	}

	return TRUE;
}

bool create_fastinv_tables()
{
	char* fast_inv	= "INSERT INTO fast_inv	SELECT * FROM tblinventory";
	char* fast_det	= "INSERT INTO fast_det SELECT * FROM tblinventorydetail";

	// make sure we're clean
	if (!remove_fastinv_tables())
	{
		char buf[MAX_STRING_LENGTH];
		strcpy(buf, mysql_error(sql));
		log_string(buf, CHANNEL_NONE, -1);
		log_string("Couldn't empty fastinv tables!", CHANNEL_NONE, -1);
		return FALSE;
	}

	if (!do_query(fast_inv))
	{
		char buf[MAX_STRING_LENGTH];
		strcpy(buf, mysql_error(sql));
		log_string(buf, CHANNEL_NONE, -1);
		log_string("Couldn't create fast_inv!", CHANNEL_NONE, -1);
		return FALSE;
	}

	if (!do_query(fast_det))
	{
		log_string("Couldn't create fast_det!", CHANNEL_NONE, -1);
		return FALSE;
	}

	return TRUE;
}

bool remove_fastinv_tables()
{
	char* rem_inv		= "DELETE FROM fast_inv";
	char* rem_det		= "DELETE FROM fast_det";

	if (!do_query(rem_inv))
	{
		return FALSE;
	}

	if (!do_query(rem_det))
	{
		return FALSE;
	}

	return TRUE;
}

long last_id()
{
	return mysql_insert_id(sql);
}
MYSQL_RES* do_query( char* query )
{
	query_count++;

	if (mysql_query(sql, query)) return NULL;

	if (strstr(query, "SELECT")!=query)
		return (MYSQL_RES*) 1;
	else
		return mysql_store_result( sql );
}

char* escape_query( char* query )
{
	char* buf		= NULL;

	if (!query)
		return NULL;

	buf = malloc(((strlen(query)*2)*sizeof(char))+sizeof(char));

	mysql_real_escape_string(sql, buf, query, strlen(query));

	return buf;
	
}

int rowcount( MYSQL_RES* res )
{
	if (res)
		return (int) mysql_num_rows(res);
	else
		return 0;
}

MYSQL_ROW get_row( MYSQL_RES* res)
{
	return mysql_fetch_row(res);
}

void cleanup_query( MYSQL_RES* res )
{
	if (res)
		mysql_free_result( res );
}

CHAR_DATA* sql_load_char( char* name )
{
	const char* FetchCharSQL	= "SELECT * FROM tblcharacters WHERE Nm=\"%s\";";
	char buf[MAX_STRING_LENGTH] = "";
	CHAR_DATA* ch				= NULL;
	PC_DATA*   pc				= NULL;
	MYSQL_RES* res				= NULL;
	MYSQL_ROW  row;

	/* check the dirty list first (we can save a DB search) */
	for (ch = dirty_list; ch; ch=ch->next_dirty)
	{
		if (!str_cmp(name, ch->name))
		{
			sprintf(buf, "Found char %s in dirty list!", ch->name);
			bug(buf,0);
			return ch;
		}
	}

	sprintf(buf, FetchCharSQL, name);

	if (!(res = do_query(buf)))
	{
		bug("sql_load_char: Query error!",0);
		return NULL;
	}
	
	if (rowcount( res ) > 1)
	{
		sprintf(buf, "Loading character %s. There were %d records found!", name, rowcount( res ));
		bug(buf, 0);
	} else if (!rowcount( res ))
	{
		// character not found. new character!
		return NULL;
	}
	
	ch	= new_char();
	clear_char( ch );

	pc = ch->pcdata	= new_pc();

	
	if (!ch || !pc)
		return NULL;

	row = get_row( res );

	ch->CharID			= atoi(row[0]);
	ch->name			= str_dup(capitalize(name));
	ch->short_descr		= str_dup(row[2]);
	ch->long_descr		= str_dup(row[3]);
	ch->description		= str_dup(row[4]);
	ch->prompt			= str_dup(row[5]);
	ch->sex				= atoi(row[6]);
	ch->class			= atoi(row[7]);
	ch->multied			= atoi(row[8]);
	ch->race			= atoi(row[9]);
	ch->clan			= atoi(row[10]);
	ch->updated			= atoi(row[11]);
	ch->clev			= atoi(row[12]);
	ch->ctimer			= atoi(row[13]);
	ch->stunned[0]		= atoi(row[14]);
	ch->stunned[1]		= atoi(row[15]);
	ch->stunned[2]		= atoi(row[16]);
	ch->stunned[3]		= atoi(row[17]);
	ch->stunned[4]		= atoi(row[18]);
	ch->damage_mods[0]	= atoi(row[19]);
	ch->damage_mods[1]	= atoi(row[20]);
	ch->damage_mods[2]	= atoi(row[21]);
	ch->damage_mods[3]	= atoi(row[22]);
	ch->damage_mods[4]	= atoi(row[23]);
	ch->damage_mods[5]	= atoi(row[24]);
	ch->damage_mods[6]	= atoi(row[25]);
	ch->damage_mods[7]	= atoi(row[26]);
	ch->damage_mods[8]	= atoi(row[27]);
	ch->damage_mods[9]	= atoi(row[28]);
	ch->damage_mods[10]	= atoi(row[29]);
	ch->damage_mods[11]	= atoi(row[30]);
	ch->damage_mods[12]	= atoi(row[31]);
	ch->damage_mods[13]	= atoi(row[32]);
	ch->damage_mods[14]	= atoi(row[33]);
	ch->damage_mods[15]	= atoi(row[34]);
	ch->damage_mods[16]	= atoi(row[35]);
	ch->damage_mods[17]	= atoi(row[36]);
	ch->damage_mods[18]	= atoi(row[37]);
	ch->damage_mods[19]	= atoi(row[38]);
	ch->wizinvis		= atoi(row[39]);
	ch->level			= atoi(row[40]);
	ch->pkill			= atoi(row[41]);
	ch->antidisarm		= atoi(row[42]);
	ch->mounted			= atoi(row[43]);
	ch->mountcharmed	= atoi(row[44]);
	ch->mountshort		= str_dup(row[45]);
	ch->trust			= atoi(row[46]);
	pc->security		= atoi(row[47]);
	ch->wizbit			= atoi(row[48]);
	pc->awins			= atoi(row[49]);
	pc->alosses			= atoi(row[50]);
	pc->mobkills		= atoi(row[51]);
	ch->questgiver		= ( atoi(row[52]) ? get_char_world( ch, get_mob_index( atoi(row[52]) )->player_name ) : NULL );
	ch->played			= atoi(row[53]);
	ch->last_note		= atoi(row[54]);
	ch->in_room			= get_room_index( atoi( row[55] ) );
	ch->hit				= atoi(row[56]);
	ch->max_hit			= atoi(row[57]);
	ch->mana			= atoi(row[58]);
	ch->max_mana		= atoi(row[59]);
	ch->move			= atoi(row[60]);
	ch->max_move		= atoi(row[61]);
	ch->bp				= atoi(row[62]);
	ch->max_bp			= atoi(row[63]);
	ch->gold			= atoi(row[64]);
	ch->guild_rank		= atoi(row[65]);
	strcpy(buf, row[66]); // Guild. No lookup function available. Must do manually later.
	ch->religion		= atoi(row[67]);
	pc->recall			= atoi(row[68]);
	ch->rtimer			= atoi(row[69]);
	ch->exp				= atoi(row[70]);
	ch->act				= atoi(row[71]);
	ch->act2			= atoi(row[72]);
	ch->affected_by		= atoi(row[73]);
	ch->affected_by2	= atoi(row[74]);
	ch->affected_by3	= atoi(row[75]);
	ch->affected_by4	= atoi(row[76]);
	ch->affected_by_powers		= atoi(row[77]);
	ch->affected_by_weaknesses	= atoi(row[78]);
	ch->imm_flags		= atoi(row[79]);
	ch->res_flags		= atoi(row[80]);
	ch->vul_flags		= atoi(row[81]);
	ch->shields			= atoi(row[82]);
	ch->position		= atoi(row[83]);
	ch->practice		= atoi(row[84]);
	ch->saving_throw	= atoi(row[85]);
	ch->alignment		= atoi(row[86]);
	ch->hitroll			= atoi(row[87]);
	ch->damroll			= atoi(row[88]);
	ch->armor			= atoi(row[89]);
	ch->wimpy			= atoi(row[90]);
	ch->deaf			= atoi(row[91]);
	pc->corpses			= atoi(row[92]);
	ch->questpoints		= atoi(row[93]);
	ch->nextquest		= atoi(row[94]);
	ch->countdown		= atoi(row[95]);
	ch->questobj		= atoi(row[96]);
	ch->questmob		= atoi(row[97]);
	ch->rquestpoints	= atoi(row[98]);
	ch->rnextquest		= atoi(row[99]);
	pc->pwd				= str_dup( row[100] );
	ch->speaking		= atoi(row[101]);
	pc->learn			= atoi(row[102]);
	pc->bamfin			= str_dup( row[103] );
	pc->bamfout			= str_dup( row[104] );
	pc->bamfsin			= str_dup( row[105] );
	pc->bamfsout		= str_dup( row[106] );
	pc->bankaccount		= atoi(row[107]);
	pc->shares			= atoi(row[108]);
	pc->immskll			= str_dup( row[109] );
	pc->title			= str_dup( row[110] );
	pc->who_text		= str_dup( row[111] );
	pc->perm_str		= atoi(row[112]);
	pc->perm_int		= atoi(row[113]);
	pc->perm_wis		= atoi(row[114]);
	pc->perm_dex		= atoi(row[115]);
	pc->perm_con		= atoi(row[116]);
	pc->mod_str			= atoi(row[117]);
	pc->mod_int			= atoi(row[118]);
	pc->mod_wis			= atoi(row[119]);
	pc->mod_dex			= atoi(row[120]);
	pc->mod_con			= atoi(row[121]);
	pc->condition[0]	= atoi(row[122]);
	pc->condition[1]	= atoi(row[123]);
	pc->condition[2]	= atoi(row[124]);
	pc->pagelen			= atoi(row[125]);

	ch->logon			= current_time;
	ch->playing			= TRUE;


	if (strcmp(buf,str_empty))
	{
		int i;
		for(i = 0;guild_table[i].name[0] != '\0';i++)
		{
			if(!strcmp(guild_table[i].name, buf))
			{
				ch->guild = &(guild_table[i]);
				break;
			}
		}
	}

	if ( ch->trust > L_CON && ch->level < L_IMP && !IS_CODER(ch) )
		  ch->trust = 0;

	if ( !ch->in_room )
	{
	    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
	}

	if ( ch->in_room->vnum >= REL_VNUM_LOWER && ch->in_room->vnum <= REL_VNUM_UPPER )
	{
	    REMOVE_BIT(ch->act2, PLR_RELQUEST); // Just in case
	    ch->rnextquest = number_range(15, 30);
	    ch->in_room = get_room_index( ROOM_VNUM_TEMPLE );
	}
	
	/*	LEGACY CODE FROM FREAD_CHAR */

	if ( !ch->pcdata->pagelen )
	    ch->pcdata->pagelen = 20;
	if ( !ch->prompt || ch->prompt == '\0' )
	    ch->prompt = str_dup ( "<%hhp %mm %vmv> " );

	if ( ch->pcdata->pagelen > 60 )
		ch->pcdata->pagelen = 60;

	/* END LEGACY CODE */

	/* Load Other info */
	pc->alias_list	= sql_load_alias(ch->CharID			 );
	ch->affected	= sql_load_affects(ch->CharID, 1	 );
	ch->affected2	= sql_load_affects(ch->CharID, 2	 );
	ch->affected3	= sql_load_affects(ch->CharID, 3	 );
	ch->affected4	= sql_load_affects(ch->CharID, 4	 );

	sql_load_items	(0, FALSE, NULL, ch			);
	sql_load_items	(0, TRUE,  NULL, ch			);
	sql_load_skills	(ch->CharID, pc->learned	);
	sql_load_lang	(ch->CharID, ch->language	);

	ch_stripbadinv( ch );

	cleanup_query(res);
	return ch;
}

OBJ_DATA* sql_load_items( int InsideOf, bool Storage, OBJ_DATA* inside, CHAR_DATA* ch)
{
	const char* objstr			= "SELECT * FROM fast_inv LEFT JOIN fast_det ON fast_inv.InventoryID = fast_det.InventoryID WHERE CharacterID=%d AND Storage=%d AND InsideOf=%d";

	char buf[MAX_STRING_LENGTH] = "";
	OBJ_DATA* first				= NULL;
	OBJ_DATA* cur				= NULL;
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int i = 0;
	int c = 0;

	sprintf(buf, objstr, ch->CharID, Storage, InsideOf);
	
	if (!(res = do_query( buf ) ) )
	{
		// bad query
		return NULL;
	} 

	if (!rowcount( res ))
	{
		cleanup_query(res);
		// no objects found
		return NULL;
	}

	for (i = 0; i < rowcount( res ); i++)
	{
		OBJ_INDEX_DATA* oi = NULL;

		row = get_row(res);

		oi = get_obj_index( atoi( row[2] ) );

		if (!oi)
			continue;

		// make the object
		cur = create_object( oi, oi->level );
		
		if (!cur)
			continue;

		c++;

		// check for details
		if (row[6])
		{
			free_string(cur->short_descr);
			free_string(cur->description);
			free_string(cur->name);
			cur->short_descr = str_dup(row[8]);
			cur->description = str_dup(row[9]);
			cur->name = str_dup(row[10]);
			cur->ac_charge[0] = cur->ac_charge[1] - atoi(row[11]);
			cur->value[2]	  = oi->value[1] - atoi(row[12]);
		}

		sql_load_items(atoi(row[0]), FALSE, cur, ch);

		cur->wear_loc = atoi(row[5]);

		if (InsideOf)
		{
			obj_to_obj(cur, inside);
		} else
		{
			if (Storage)
			{
				obj_to_storage(cur, ch);
			} else
			{
				obj_to_char(cur, ch);
			}
		}

		if (c==1)
		{
			first = cur;
		} 
	}

	cleanup_query(res);

	return first;
}

ALIAS_DATA* sql_load_alias( int CharID )
{
	const char* alsstr			= "SELECT * FROM tblcharacteraliases WHERE CharacterID=%d";
	char buf[MAX_STRING_LENGTH] = "";
	ALIAS_DATA* first			= NULL;
	ALIAS_DATA* als				= NULL;
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int i = 0;

	sprintf(buf, alsstr, CharID);

	if (!(res = do_query(buf)))
	{
		return NULL;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return NULL;
	}

	for (i=0; i<rowcount(res);i++)
	{
		row = get_row(res);

		als = new_alias();

		if (als)
		{
			als->old  = str_dup(row[2]);
			als->_new = str_dup(row[3]);
			als->next = first;
			first = als;
		}
	}

	cleanup_query(res);

	return first;
}

AFFECT_DATA* sql_load_affects( int CharID, int AffNum )
{

	AFFECT_DATA* first			= NULL;
	AFFECT_DATA* paf			= NULL;
	const char* affstr			= "SELECT * FROM tblcharacteraffects WHERE CharacterID=%d AND FlagNum=%d";
	char buf[MAX_STRING_LENGTH] = "";
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int i = 0;

	sprintf(buf, affstr, CharID, AffNum);

	if (!(res = do_query(buf)))
	{
		return NULL;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return NULL;
	}

	for (i=0; i<rowcount(res);i++)
	{
		row = get_row(res);

		paf	= new_affect();

		if (paf)
		{

			paf->type       = slot_lookup(atoi(row[3]));
			paf->level		= atoi(row[4]);
			paf->duration	= atoi(row[5]);
			paf->location	= atoi(row[6]);
			paf->modifier	= atoi(row[7]);
			paf->bitvector	= atoi(row[8]);
			paf->deleted    = FALSE;
			paf->next		= first;
			first			= paf;

		}
	}

	cleanup_query(res);

	return first;
}

bool sql_load_skills( int CharID, int table[])
{
	const char* sklstr			= "SELECT * FROM tblcharacterskills WHERE CharacterID=%d";
	char buf[MAX_STRING_LENGTH] = "";
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int i = 0;

	sprintf(buf, sklstr, CharID);

	if (!(res = do_query(buf)))
	{
		return FALSE;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return FALSE;
	}

	for (i = 0; i < rowcount(res); i++)
	{
		row = get_row(res);
		table[atoi(row[2])] = atoi(row[3]);
	}

	cleanup_query(res);

	return TRUE;
}

bool sql_load_lang( int CharID, int table[])
{
	const char* sklstr			= "SELECT * FROM tblcharacterlanguages WHERE CharacterID=%d";
	char buf[MAX_STRING_LENGTH] = "";
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int i = 0;

	sprintf(buf, sklstr, CharID);

	if (!(res = do_query(buf)))
	{
		return FALSE;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return FALSE;
	}

	for (i = 0; i < rowcount(res); i++)
	{
		row = get_row(res);
		table[atoi(row[2])] = atoi(row[3]);
	}

	cleanup_query(res);
	return TRUE;
}

bool sql_load_areas()
{
	const char* areasql			= "SELECT * FROM tblarea";
	char buf[MAX_STRING_LENGTH] = "";
	AREA_DATA* pArea			= NULL;
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int i = 0;

	if (!create_fastboot_tables())
		return FALSE;

	strcpy(buf, areasql);

	if (!(res = do_query(buf)))
	{
		return FALSE;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return FALSE;
	}

	for (i = 0; i < rowcount(res); i++)
	{
		row = get_row(res);

		pArea = new_area();

		if (pArea)
		{
			pArea->vnum			= top_area;
			pArea->AreaID		= atoi(row[0]);
			pArea->filename		= str_dup( row[1]);
			pArea->name			= str_dup( row[2] );
				// debug test
				sprintf(buf, "Loading %s...", pArea->filename );
				log_string( buf, CHANNEL_NONE , -1 );
			pArea->lvnum		= atoi(row[3]);
			pArea->uvnum		= atoi(row[4]);
			pArea->security		= atoi(row[5]);
			pArea->builders		= str_dup( row[6] );
			pArea->reset_sound	= str_dup( row[7] );
			pArea->area_flags	= atoi(row[8]);
			pArea->recall		= atoi(row[9]);
			pArea->actual_sound = str_dup( row[10] );
			pArea->musicfile	= str_dup( row[11] );

			pArea->age			= 666; // force an area update

			area_sort(pArea);
			top_area++;

			sql_load_rooms(pArea);
			sql_load_mobiles(pArea);
			sql_load_objects(pArea);
		}
	}

	cleanup_query(res);

	remove_fastboot_tables();
	return TRUE;
}

bool sql_load_rooms(AREA_DATA* pArea)
{
	const char* roomsql			= "SELECT * FROM tblrooms LEFT JOIN fast_exit ON tblrooms.vnum = fast_exit.RoomVnum WHERE AreaID=%d ORDER BY tblrooms.vnum";
	char buf[MAX_STRING_LENGTH] = "";
	ROOM_INDEX_DATA* pRoom		= NULL;
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int room_vnum = 0;
	int i = 0;
	int iHash = 0;

	sprintf(buf, roomsql, pArea->AreaID);

	if (!(res = do_query(buf)))
	{
		return FALSE;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return FALSE;
	}

	for (i = 0; i < rowcount(res); i++)
	{

		row	= get_row(res);

		if (room_vnum != atoi(row[0]))
		{

			pRoom = new_room_index();

			if (pRoom)
			{
				pRoom->vnum			= atoi(row[0]);
				pRoom->name			= str_dup( row[2] );
				pRoom->soundfile	= str_dup( row[3] );
				pRoom->musicfile 	= str_dup( row[4] );
				pRoom->description	= str_dup( row[5] );
				pRoom->room_flags	= atoi(row[6]);
				pRoom->sector_type	= atoi(row[7]);
				pRoom->area			= pArea;

				// hash stuff
				iHash					= pRoom->vnum % MAX_KEY_HASH;
				pRoom->next				= room_index_hash[iHash];
				room_index_hash[iHash]	= pRoom;
				top_room++;
				top_vnum_room = top_vnum_room < pRoom->vnum	 ? pRoom->vnum : top_vnum_room;

				if (atoi(row[8]))
					pRoom->traps = sql_load_traps( pRoom->vnum, LOAD_ROOM, 0);

				if (atoi(row[10]))
				{
					pRoom->extra_descr = sql_load_ed(pRoom->vnum, TABLE_ROOM);
				}

				room_vnum = pRoom->vnum;
			}

		}

		if (atoi(row[9]) && pRoom)
		{
			EXIT_DATA* e	= NULL;
			EXIT_DATA** x	= pRoom->exit;
			int it = 0;

			it = atoi(row[13]);

			e = x[it] = new_exit();

			if (e)
			{
				e->description	= str_dup( row[14] );
				e->keyword		= str_dup( row[15] );
				e->exit_info	= atoi(row[16]);
				e->rs_flags		= atoi(row[17]);
				e->key			= atoi(row[18]);
				e->vnum			= atoi(row[19]);
				if (atoi(row[20]))
				{
					e->traps		= sql_load_traps(pRoom->vnum , LOAD_EXIT, i+1);
				}
			}
		}
	}

	cleanup_query(res);
	return TRUE;
}

/*
 * Defunct. This took 180+ seconds
 * Memory tables + LEFT JOIN bring this down to ~5 seconds
 *
 * - Ahsile

bool sql_load_exits(int vnum, EXIT_DATA* x[])
{
	const char* exitsql			= "SELECT * FROM fast_exit WHERE vnum=%d";
	char buf[MAX_STRING_LENGTH]	= "";
	EXIT_DATA* e				= NULL;
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int i = 0;
	int it = 0;

	sprintf(buf, exitsql, vnum);

	if (!(res = do_query(buf)))
	{
		return FALSE;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return FALSE;
	} else if (rowcount(res) > MAX_DIR)
	{
		// too many exits
		cleanup_query(res);
		return FALSE;
	}

	for (i = 0; i < rowcount(res); i++)
	{
		
		row	= get_row(res);
		
		it = atoi(row[2]);
		e = x[it] = new_exit();
		
		if (e)
		{
			e->description	= str_dup( row[3] );
			e->keyword		= str_dup( row[4] );
			e->exit_info	= atoi(row[5]);
			e->rs_flags		= atoi(row[6]);
			e->key			= atoi(row[7]);
			e->vnum			= atoi(row[8]);
			if (atoi(row[9]))
			{
				e->traps		= sql_load_traps(vnum, LOAD_EXIT, i+1);
			}
		}
	}
	
	cleanup_query(res);
	return TRUE;
}
*/

TRAP_DATA* sql_load_traps( int vnum, int ProgType, int Exit )
{
	const char* trapsql			= "SELECT * FROM tblprograms WHERE vnum=%d AND LoadType=%d AND ExitNum=%d";
	char buf[MAX_STRING_LENGTH]	= "";
	TRAP_DATA* trap				= NULL;
	TRAP_DATA* first			= NULL;
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int i = 0;

	sprintf(buf, trapsql, vnum, ProgType, Exit);

	if (!(res = do_query(buf)))
	{
		return NULL;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return NULL;
	}

	for (i = 0; i<rowcount(res); i++)
	{
		row = get_row(res);

		trap = new_trap();

		if (trap)
		{
			if (ProgType==LOAD_OBJ)
			{
				OBJ_INDEX_DATA* o = get_obj_index( vnum );
				trap->on_obj = o;
			} else
			{
				ROOM_INDEX_DATA* r = get_room_index( vnum );
				if (ProgType==LOAD_ROOM)
				{
					trap->in_room = r;
				} else
				{
					trap->on_exit = r->exit[Exit-1];
				}
			}

			trap->type = atoi(row[4]);
			trap->arglist = str_dup( row[5] );
			trap->comlist = str_dup( row[6] );
			trap->next = trap->next_here = first;
			first = trap;
		}
	}

	cleanup_query(res);
	return first;
}

bool sql_load_mobiles(AREA_DATA* pArea)
{
	const char* mobsql			= "SELECT * FROM fast_mob WHERE AreaID=%d";
	char buf[MAX_STRING_LENGTH] = "";
	MOB_INDEX_DATA* pMob		= NULL;
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int i = 0;
	int iHash = 0;

	sprintf(buf, mobsql, pArea->AreaID);

	if (!(res = do_query(buf)))
	{
		return FALSE;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return FALSE;
	}

	for (i = 0; i < rowcount(res); i++)
	{
		row	= get_row(res);
	
		pMob = new_mob_index();

		if (pMob)
		{

			pMob->vnum			= atoi(row[0]);
			pMob->player_name	= str_dup( row[2] );
			pMob->short_descr	= str_dup( row[3] );
			pMob->long_descr	= str_dup( row[4] );
			pMob->act			= atoi(row[5]);
			pMob->affected_by	= atoi(row[6]);
			pMob->affected_by2	= atoi(row[7]);
			pMob->affected_by3	= atoi(row[8]);
			pMob->affected_by4	= atoi(row[9]);
			pMob->alignment		= atoi(row[10]);
			pMob->level			= atoi(row[11]);
			pMob->hitroll		= atoi(row[12]);
			pMob->damroll		= atoi(row[13]);
			pMob->hitnodice		= atoi(row[14]);
			pMob->sex			= atoi(row[15]);
			pMob->imm_flags		= atoi(row[16]);
			pMob->res_flags		= atoi(row[17]);
			pMob->vul_flags		= atoi(row[18]);
			pMob->speaking		= atoi(row[19]);
			pMob->spec_fun		= spec_lookup( str_dup( row[20] ) );
			pMob->area			= pArea;
			
			// Hash table and kill table
			iHash			= pMob->vnum % MAX_KEY_HASH;
			pMob->next		= mob_index_hash[iHash];
			mob_index_hash[iHash]	= pMob;
			top_mob_index++;
			top_vnum_mob = top_vnum_mob < pMob->vnum ? pMob->vnum : top_vnum_mob;  /* OLC */
			assign_area_vnum( pMob->vnum );				   /* OLC */
			kill_table[URANGE( 0, pMob->level, MAX_LEVEL-1 )].number++;

			if (atoi(row[21]))
			{
				MPROG_DATA* mprog;
				pMob->mobprogs = sql_load_mprog(pMob->vnum);

				for (mprog = pMob->mobprogs; mprog; mprog = mprog->next)
				{
					SET_BIT(pMob->progtypes, mprog->type);
				}
			}

			if (atoi(row[22]))
			{
				MYSQL_RES* det_res;
				MYSQL_ROW  det_row;

				sprintf(buf, "SELECT MobileDesc FROM tblmobiledescriptions WHERE vnum=%d", pMob->vnum);
				det_res = do_query(buf);
				det_row = get_row(det_res);

				pMob->description = str_dup( det_row[0] );

				cleanup_query(det_res);
			}
		}

		pMob = NULL;

	}

	cleanup_query(res);
	return TRUE;
}

MPROG_DATA* sql_load_mprog( int vnum )
{
	const char* mprgsql			= "SELECT * FROM tblprograms WHERE vnum=%d AND LoadType=%d";
	char buf[MAX_STRING_LENGTH] = "";
	MPROG_DATA* mprog			= NULL;
	MPROG_DATA* first			= NULL;
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int i = 0;

	sprintf(buf, mprgsql, vnum, LOAD_MOB, 0);

	if (!(res = do_query(buf)))
	{
		return NULL;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return NULL;
	}

	for (i = 0; i < rowcount(res); i++)
	{
		row	= get_row(res);

		mprog = new_mprog();

		if (mprog)
		{
			mprog->status	= atoi(row[2]);
			mprog->type		= atoi(row[4]);
			mprog->arglist	= str_dup( row[5] );
			mprog->comlist	= str_dup( row[6] );
			mprog->next = NULL;

			mprog->next = first;
			first = mprog;
		}
	}

	cleanup_query(res);
	return first;
}

bool sql_load_objects(AREA_DATA* pArea)
{
	const char* objsql			= "SELECT * FROM fast_obj WHERE AreaID=%d";
	char buf[MAX_STRING_LENGTH] = "";
	OBJ_INDEX_DATA* pObj		= NULL;
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int i = 0;
	int iHash = 0;

	sprintf(buf, objsql, pArea->AreaID);

	if (!(res = do_query(buf)))
	{
		return FALSE;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return FALSE;
	}

	for (i = 0; i < rowcount(res); i++)
	{
		row	= get_row(res);
	
		pObj = new_obj_index();

		if (pObj)
		{
			pObj->vnum			= atoi(row[0]);
			pObj->name			= str_dup(row[2]);
			pObj->short_descr	= str_dup(row[3]);
			pObj->description	= str_dup(row[4]);
			pObj->item_type		= atoi(row[5]);
			pObj->extra_flags	= atoi(row[6]);
			pObj->extra_flags2	= atoi(row[7]);
			pObj->extra_flags3	= atoi(row[8]);
			pObj->extra_flags4	= atoi(row[9]);
			pObj->wear_flags	= atoi(row[10]);
			pObj->level			= atoi(row[11]);
			pObj->value[0]		= atoi(row[12]);
			pObj->value[1]		= atoi(row[13]);
			pObj->value[2]		= atoi(row[14]);
			pObj->value[3]		= atoi(row[15]);
			pObj->weight		= atoi(row[16]);
			pObj->cost			= atoi(row[17]);
			pObj->ac_type		= atoi(row[18]);
			pObj->ac_vnum		= atoi(row[19]);
			pObj->ac_spell		= str_dup(row[20]);
			pObj->ac_charge[0]	= atoi(row[21]);
			pObj->ac_charge[1]	= atoi(row[22]);
			pObj->join			= atoi(row[23]);
			pObj->sep_one		= atoi(row[24]);
			pObj->sep_two		= atoi(row[25]);
			pObj->area			= pArea;

			iHash			= pObj->vnum % MAX_KEY_HASH;
			pObj->next		= obj_index_hash[iHash];
			obj_index_hash[iHash]	= pObj;
			top_obj_index++;
			top_vnum_obj = top_vnum_obj < pObj->vnum ? pObj->vnum : top_vnum_obj;  /* OLC */
			assign_area_vnum( pObj->vnum );

			if (atoi(row[26]))
				pObj->traps = sql_load_traps(pObj->vnum, LOAD_OBJ, 0);

			if (atoi(row[27]))
				pObj->extra_descr = sql_load_ed(pObj->vnum, TABLE_OBJ);

			if (atoi(row[28]))
				pObj->affected = sql_load_objaff(pObj->vnum, pObj->level);
		}
	}

	cleanup_query(res);
	return TRUE;
}

AFFECT_DATA* sql_load_objaff(int vnum, int level)
{
	const char* affsql			= "SELECT * FROM fast_objaf WHERE vnum=%d";
	char buf[MAX_STRING_LENGTH] = "";
	AFFECT_DATA* paf			= NULL;
	AFFECT_DATA* first			= NULL;
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int i = 0;

	sprintf(buf, affsql, vnum);

	if (!(res = do_query(buf)))
	{
		return NULL;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return NULL;
	}

	for (i = 0; i < rowcount(res); i++)
	{
		row	= get_row(res);
	
		paf = new_affect();

		if (paf)
		{
			paf->location	= atoi(row[2]);
			paf->modifier	= atoi(row[3]);
			paf->level		= level;
			paf->type		= -1;
			paf->duration	= -1;
			
			paf->next = first;
			first = paf;
		}
	}

	cleanup_query(res);
	return first;
}

EXTRA_DESCR_DATA* sql_load_ed(int vnum, char* table)
{
	const char* edsql			= "SELECT * FROM %s WHERE vnum=%d";
	char buf[MAX_STRING_LENGTH] = "";
	EXTRA_DESCR_DATA* ed		= NULL;
	EXTRA_DESCR_DATA* first		= NULL;
	MYSQL_RES* res				= NULL;
	MYSQL_ROW row;
	int i = 0;

	sprintf(buf, edsql, table, vnum);

	if (!(res = do_query(buf)))
	{
		return NULL;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return NULL;
	}

	for (i = 0; i < rowcount(res); i++)
	{
		row	= get_row(res);
	
		ed = new_extra_descr();

		if (ed)
		{
			ed->keyword = str_dup( row[1] );
			ed->description = str_dup( row[2] );

			ed->next = first;
			first = ed;
		}
	}

	cleanup_query(res);
	return first;
}

bool sql_load_disabled()
{
	DISABLED_DATA *p 	= NULL;
	MYSQL_RES* res		= NULL;
	MYSQL_ROW  row;
    char *name			= NULL;
    int i = 0;
	int j = 0;

    disabled_first = NULL;

	if (!(res = do_query("SELECT * FROM tbldisabled")))
	{
		return FALSE;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return FALSE;
	}

    for (j = 0; j < rowcount(res); j++)
	{
		row = get_row(res);

		name = row[1];

        /* Find the command in the table */
        for (i = 0; cmd_table[i].name[0] ; i++)
                if (!str_cmp(cmd_table[i].name, name))
                        break;

        if (!cmd_table[i].name[0]) /* command does not exist? */
        {
                bug ("Skipping uknown command in disabled list.",0);
        }
        else /* add new disabled command */
        {
                p = alloc_mem(sizeof(DISABLED_DATA));
                p->command		= &cmd_table[i];
                p->uptolevel	= atoi(row[2]);
                p->dislevel		= atoi(row[3]);
                p->disabled_by	= str_dup(row[4]);
                p->next = disabled_first;

                disabled_first = p;

        }
	}

	cleanup_query(res);
	return TRUE;
}

bool sql_load_bans()
{
	BAN_DATA*	pBan 		= NULL;
	MYSQL_RES*	res			= NULL;
	MYSQL_ROW	row;
    int i = 0;

	if (!(res = do_query("SELECT * FROM tblbans")))
	{
		return FALSE;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return FALSE;
	}

    for (i = 0; i < rowcount(res); i++)
	{
		row = get_row(res);

		pBan = new_ban();

		if (pBan)
		{
			pBan->name = str_dup(row[1]);
			pBan->user = str_dup(row[2]);

			pBan->next = ban_list;
			
			ban_list = pBan;
		}
	}
	return TRUE;
}

bool sql_load_clans()
{
	char		buf[MAX_STRING_LENGTH] = "";
	CLAN_DATA*	pClan 		= NULL;
	MYSQL_RES*	res			= NULL;
	MYSQL_ROW	row;
    int i = 0;

	if (!(res = do_query("SELECT * FROM tblclans")))
	{
		return FALSE;
	}

	if (!rowcount(res))
	{
		cleanup_query(res);
		return FALSE;
	}

    for (i = 0; i < rowcount(res); i++)
	{
		row = get_row(res);

		pClan = new_clan();

		if (pClan)
		{
			MYSQL_RES* det_res  = NULL;
			MYSQL_ROW  det_row;

			pClan->vnum			= atoi(row[0]) - 1;
			pClan->name			= str_dup( row[1] );
			pClan->description	= str_dup( row[2] );
			pClan->deity		= str_dup( row[3] );
			pClan->champ		= str_dup( row[4] );
			pClan->leader		= str_dup( row[5] );
			pClan->first		= str_dup( row[6] );
			pClan->second		= str_dup( row[7] );
			pClan->ischamp		= atoi(row[8]);
			pClan->isleader		= atoi(row[9]);
			pClan->isfirst		= atoi(row[10]);
			pClan->issecond		= atoi(row[11]);
			pClan->recall		= atoi(row[12]);
			pClan->pkills		= atoi(row[13]);
			pClan->mkills		= atoi(row[14]);
			pClan->members		= atoi(row[15]);
			pClan->pdeaths		= atoi(row[16]);
			pClan->mdeaths		= atoi(row[17]);
			pClan->obj_vnum_1	= atoi(row[18]);
			pClan->obj_vnum_2	= atoi(row[19]);
			pClan->obj_vnum_3	= atoi(row[20]);
			pClan->pkill		= atoi(row[21]);

			sprintf(buf, "SELECT COUNT(*) FROM tblcharacter WHERE Clan=%d", pClan->vnum);
			if ((det_res = do_query(buf)))
			{
				det_row = get_row(det_res);
				pClan->members	= atoi(row[0]);
				cleanup_query(det_res);
			}

			clan_sort(pClan);
			top_clan++;
		}
	}

	return TRUE;
}

bool sql_load_helps()
{
	HELP_DATA* pHelp;
	MYSQL_RES* res	= NULL;
	MYSQL_ROW  row;
	int i = 0;

	if (!(res = do_query("SELECT * FROM tblhelp")))
	{
		// bad query
		return FALSE;
	}

	if (!rowcount(res))
	{
		// no helps
		cleanup_query(res);
		return FALSE;
	}

	for (i = 0; i < rowcount(res); i++)
	{
		row = get_row(res);

		pHelp = new_help();

		if (pHelp)
		{

			pHelp->keyword = str_dup(row[1]);
			pHelp->level   = atoi(row[2]);
			pHelp->text    = str_dup(row[3]);

			if ( !str_cmp( pHelp->keyword, "greeting1" ) )
				help_greeting_one = pHelp->text;

			if ( !str_cmp( pHelp->keyword, "greeting2" ) )
				help_greeting_two = pHelp->text;

			if ( !str_cmp( pHelp->keyword, "greeting3" ) )
				help_greeting_three = pHelp->text;

			if ( !str_cmp( pHelp->keyword, "greeting4" ) )
				help_greeting_four = pHelp->text;

			if ( !str_cmp( pHelp->keyword, "greeting5" ) )
				help_greeting_five = pHelp->text;

			if ( !help_first )
				help_first = pHelp;

			if (  help_last  )
				help_last->next = pHelp;

			help_last	= pHelp;
			pHelp->next	= NULL;
			top_help++;
		}
    }

	cleanup_query(res);
	return TRUE;
}

bool sql_load_religions()
{
	RELIGION_DATA* pRel			= NULL;
	char buf[MAX_STRING_LENGTH] = "";
	MYSQL_RES* res				= NULL;
	MYSQL_ROW  row;
	int i = 0;

	if (!(res = do_query("SELECT * FROM tblreligions")))
	{
		// bad query
		return FALSE;
	}

	if (!rowcount(res))
	{
		// no religions
		cleanup_query(res);
		return FALSE;
	}

	for (i = 0; i < rowcount(res); i++)
	{
		row = get_row(res);

		pRel = new_religion();

		if (pRel)
		{
			MYSQL_RES* det_res	= NULL;
			MYSQL_ROW  det_row;
			pRel->vnum			= atoi(row[0]) - 1;
			pRel->name			= str_dup(row[1]);
			pRel->shortdesc		= str_dup(row[2]);
			pRel->deity			= str_dup(row[3]);
			pRel->description	= str_dup(row[4]);
			pRel->recall		= atoi(row[5]);
			pRel->pkills		= atoi(row[6]);
			pRel->mkills		= atoi(row[7]);
			pRel->pdeaths		= atoi(row[8]);

			sprintf(buf, "SELECT COUNT(*) FROM tblcharacter WHERE RNumber=%d", pRel->vnum);
			if ((det_res = do_query(buf)))
			{
				det_row = get_row(det_res);
				pRel->members	= atoi(row[0]);
				cleanup_query(det_res);
			} else
			{
				pRel->members	= atoi(row[9]);
			}

			religion_sort(pRel);
		}
	}

	relquest = FALSE;
	cleanup_query(res);
	return TRUE;
}

bool sql_load_resets()
{

	RESET_DATA* pRes	= NULL;
	MYSQL_RES* res		= NULL;
	MYSQL_ROW  row;
	int i = 0;

	if (!(res = do_query("SELECT * FROM tblresets")))
	{
		// bad query
		return FALSE;
	}

	if (!rowcount(res))
	{
		// no resets
		cleanup_query(res);
		return FALSE;
	}

	for (i = 0; i < rowcount(res); i++)
	{
		row = get_row(res);

		pRes = new_reset();

		if (pRes)
		{

			pRes->command	= row[1][0];
			pRes->status	= atoi(row[2]);
			pRes->arg1		= atoi(row[3]);
			pRes->arg2		= atoi(row[4]);
			pRes->arg3		= atoi(row[5]);

			sql_validate_resets(pRes);
		}
	}

	cleanup_query(res);
	return TRUE;
}

bool sql_load_socials()
{
	SOCIAL_DATA* pSoc	= NULL;
	MYSQL_RES* res		= NULL;
	MYSQL_ROW  row;
	int i = 0;

	if (!(res = do_query("SELECT * FROM tblsocials")))
	{
		// bad query
		return FALSE;
	}

	if (!rowcount(res))
	{
		// no socials
		cleanup_query(res);
		return FALSE;
	}

	maxSocial = rowcount(res);

	// ewww. socials suck, but I'm not going to rewrite the whole
	// social system.
	// - Ahsile
	social_table = malloc (sizeof(struct social_type) * (maxSocial+1));
	memset(social_table, 0, sizeof(struct social_type) * (maxSocial+1));

	for (i = 0; i < rowcount(res); i++)
	{
		row = get_row(res);

		pSoc = &social_table[i];
		memset(pSoc, 0, sizeof(struct social_type));

		if (pSoc)
		{
			pSoc->name			= str_dup(row[1]);
			pSoc->char_no_arg	= str_dup(row[2]);
			pSoc->others_no_arg	= str_dup(row[3]);
			pSoc->char_found	= str_dup(row[4]);
			pSoc->others_found	= str_dup(row[5]);
			pSoc->vict_found	= str_dup(row[6]);
			pSoc->char_auto		= str_dup(row[7]);
			pSoc->others_auto	= str_dup(row[8]);
			
			social_table[i] = *pSoc;		
		}
	}

	social_table[maxSocial].name = str_dup(str_empty);

	cleanup_query(res);
	return TRUE;
}

bool sql_load_shops()
{
	SHOP_DATA* pShop	= NULL;
	MOB_INDEX_DATA* mob = NULL;
	MYSQL_RES* res		= NULL;
	MYSQL_ROW  row;
	int i = 0;

	if (!(res = do_query("SELECT * FROM tblshops")))
	{
		// bad query
		return FALSE;
	}

	if (!rowcount(res))
	{
		// no shops
		cleanup_query(res);
		return FALSE;
	}

	for (i = 0; i < rowcount(res); i++)
	{
		row = get_row(res);

		pShop = new_shop();

		if (pShop)
		{
			mob = NULL;

			pShop->keeper		= atoi(row[1]);
			pShop->profit_buy	= atoi(row[2]);
			pShop->profit_sell	= atoi(row[3]);
			pShop->open_hour	= atoi(row[4]);
			pShop->close_hour	= atoi(row[5]);
			pShop->buy_type[0]	= atoi(row[6]);
			pShop->buy_type[1]	= atoi(row[7]);
			pShop->buy_type[2]	= atoi(row[8]);
			pShop->buy_type[3]	= atoi(row[9]);
			pShop->buy_type[4]	= atoi(row[10]);
	
			if ((mob = get_mob_index( pShop->keeper )))
			{
				mob->pShop = pShop;
			}
			
			if ( !shop_first )
				shop_first = pShop;
			if (  shop_last  )
				shop_last->next = pShop;

			shop_last	= pShop;
			pShop->next	= NULL;
			top_shop++;
		}
	}

	cleanup_query(res);
	return TRUE;
}

void sql_validate_resets(RESET_DATA* pReset)
{
	/*
	 * Validate parameters.
	 * We're calling the index functions for the side effect.
	 */
	ROOM_INDEX_DATA* pRoomIndex = NULL;
	EXIT_DATA*		 pexit		= NULL;
	static int iLastRoom		= 0;
	static int iLastObj			= 0;
	char letter = pReset->command;

	switch ( letter )
	{
	default:
	    bug( "Load_resets: bad command '%c'.", letter );
	    exit( 1 );
	    break;

	case 'M':
	    get_mob_index  ( pReset->arg1 );
	    if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) )
	    {
		link_reset( pRoomIndex, pReset );
		iLastRoom = pReset->arg3;
	    }
	    break;

	case 'O':
	    get_obj_index  ( pReset->arg1 );
	    if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) )
	    {
		link_reset( pRoomIndex, pReset );
		iLastObj = pReset->arg3;
	    }
	    break;

	case 'P':
	    get_obj_index  ( pReset->arg1 );
	    if ( ( pRoomIndex = get_room_index ( iLastObj ) ) )
	    {
		link_reset( pRoomIndex, pReset );
	    }
	    break;

	case 'G':
	case 'E':
	    get_obj_index  ( pReset->arg1 );
	    if ( ( pRoomIndex = get_room_index ( iLastRoom ) ) )
	    {
		link_reset( pRoomIndex, pReset );
		iLastObj = iLastRoom;
	    }
	    break;

	case 'D':
	    pRoomIndex = get_room_index( pReset->arg1 );

	    if (   pReset->arg2 < 0
		|| pReset->arg2 > 5
		|| !pRoomIndex
		|| !( pexit = pRoomIndex->exit[pReset->arg2] )
		|| !IS_SET( pexit->rs_flags, EX_ISDOOR ) )
	    {
		bug( "Load_resets: 'D': exit %d not door.", pReset->arg2 );
		exit( 1 );
	    }

	    switch ( pReset->arg3 )	/* OLC 1.1b */
	    {
		default:
		    bug( "Load_resets: 'D': bad 'locks': %d." , pReset->arg3);
		case 0:
		    break;
		case 1: SET_BIT( pexit->rs_flags, EX_CLOSED );
		    break;
		case 2: SET_BIT( pexit->rs_flags, EX_CLOSED | EX_LOCKED );
		    break;
	    }
	    break;

	case 'R':
	    if ( pReset->arg2 < 0 || pReset->arg2 > 6 )	/* Last Door. */
	    {
		bug( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
		exit( 1 );
	    }

	    if ( ( pRoomIndex = get_room_index( pReset->arg1 ) ) )
		link_reset( pRoomIndex, pReset );

	    break;
	}
}

/*

  The "dirty" option here may need some explaining, and I'll do my best.
  We're using memory tables to do fast loading of characters, inventory,
  etc. The problem with these tables is they lose all their changes when
  the DB shuts down.

  The solution? Fresh updates. This takes time though. Roughly 2.5 seconds
  on PC being used right now (2.4 GHz P4 + 1GB Mem). Doing this for EVERY
  character save can make things very slow.

  Enter "dirty" saves. A dirty save means we're going to update the fast memory
  table. We take a risk here that we may lose some data, but we have a pulse
  defined in the update handler to do clean saves. By default this is set at
  roughly 5 minutes.

  We also keep a list of "Dirty" characters. This way we can use this list
  rather than hitting the DB again.

  Also note: The only "dirty" information is the inventory. Actual character
  data is never dirty.

  */
bool sql_save_dirty_char(CHAR_DATA* ch, bool Quit)
{
	CHAR_DATA* search;

	if (!sql_save_char(ch))
		return FALSE;

	/* Unnecessary. Causes performance hit. Save the character, but
	   leave the inventory for a clean save.
		- Ahsile

	if (!sql_remove_old_inv(ch->CharID, TRUE))
		return FALSE;

	if (!sql_save_inv(ch->carrying, ch->CharID, 0, FALSE, TRUE))
		return FALSE;

	if (!sql_save_inv(ch->pcdata->storage, ch->CharID, 0, TRUE, TRUE))
		return FALSE;

	*/

	if (!ch->next_dirty)
	{
		bool found_dirty = FALSE;

		for (search = dirty_list; search; search = search->next_dirty)
		{
			if (search==ch)
			{
				found_dirty = TRUE;
				break;
			}
		}
		if (!found_dirty)
		{
			ch->next_dirty = dirty_list;
			dirty_list = ch;
		}
	}

	if (Quit)
	{
		// TODO: This is probably where we'll handle what extract_char
		//		 was used for on PCs
	}

	return TRUE;
}

/*
 * This is where we'll force the inventory of dirty characters to save.
 */
bool sql_save_dirty_list(bool Refresh)
{
	CHAR_DATA* ch		= dirty_list;
	bool found_dirty	= FALSE;

	while(dirty_list)
	{
		if (!sql_save_char(ch))
			return FALSE;

		if (!sql_remove_old_inv(ch->CharID, FALSE))
			return FALSE;

		if (!sql_save_inv(ch->carrying, ch->CharID, 0, FALSE, FALSE))
			return FALSE;

		if (!sql_save_inv(ch->pcdata->storage, ch->CharID, 0, TRUE, FALSE))
			return FALSE;

		dirty_list = ch->next_dirty;
		ch->next_dirty = NULL;

		if (!ch->playing)
		{
			extract_char(ch, TRUE);
		}

		found_dirty = TRUE;
	}

	if (found_dirty)
	{
		log_string("Saved \"dirty\" players.", CHANNEL_LOG, -1);
		if (Refresh)
			create_fastinv_tables();
	}

	return TRUE;
}

/*
 * This function is common whether dirty or not. Character data
 * shalt always be dumped to the DB since it is more important
 * than inventory.
 */
bool sql_save_char(CHAR_DATA* ch)
{ 
	char* desc_str;
	char* title_str;
	char* prompt_str;
	char* bamfin_str;
	char* bamfout_str;
	char* bamfsin_str;
	char* bamfsout_str;
	char* mountshort_str;
	char* buf = malloc( MAX_STRING_LENGTH * sizeof(char) * 20 );
	const char* pcstr = "UPDATE tblcharacters SET  Nm=\"%s\", ShtDsc=\"%s\", LngDsc=\"%s\", Dscr=\"%s\", Prmpt=\"%s\", Sx=%d, Cla=%d, Mlt=%d, Race=%d, \
							Clan=%d, Updated=%d, Clvl=%d, Ctmr=%d, StunOne=%d, StunTwo=%d, StunThree=%d, StunFour=%d, StunFive=%d, ResAcid=%d, \
							ResHoly=%d, ResMagic=%d, ResFire=%d, ResEnergy=%d, ResWind=%d, ResWater=%d, ResIllusion=%d, ResDispel=%d, ResEarth=%d, \
							ResPsychic=%d, ResPoison=%d, ResBreath=%d, ResSUmmon=%d, ResPhysical=%d, ResExplosive=%d, ResSong=%d,ResNagarom=%d, ResUnholy=%d, \
							ResClan=%d, WizLev=%d, Lvl=%d, Pkill=%d, Antidisarm=%d, Mounted=%d, Mountcharmed=%d, Mountshort=\"%s\", \
							Trst=%d, Security=%d, Wizbt=%d, ArenaWins=%d, ArenaLoses=%d, MobKills=%d, QuestGiver=%d, Playd=%d, Note=%d, Room=%d, \
							Hp=%d, MaxHP=%d, Mn=%d, MaxMn=%d, Mv=%d, MaxMv=%d, Bp=%d, MaxBp=%d, Gold=%d, \
							GRank=%d, Guild=\"%s\", RNumber=%d, Recall=%d, Rtimer=%d, Exp=%d, Act=%d, Act2=%d, AffdBy=%d, AffdBy2=%d, AffdBy3=%d, \
							AffdBy4=%d, AffdByp=%d,AffdByw=%d, ImmBits=%d, ResBits=%d, VulBits=%d, Shields=%d, \
							Pos=%d, Prac=%d, SavThr=%d, Align=%d, Hit=%d, Dam=%d, Armr=%d, Wimp=%d, Deaf=%d, Corpses=%d, QuestPnts=%d, QuestNext=%d, \
							QuestCount=%d, QuestObj=%d, QuestMob=%d, RQuestPnts=%d, RQuestNext=%d, Paswd=\"%s\", \
							Speak=%d, Learn=%d, Bmfin=\"%s\", Bmfout=\"%s\", Bmfsin=\"%s\", Bmfsout=\"%s\", Bank=%d, BankShares=%d, Immskll=\"%s\", \
							Ttle=\"%s\", WhoTxt=\"%s\", AtrPrmStr=%d, AtrPrmInt=%d, AtrPrmWis=%d, AtrPrmDex=%d, AtrPrmCon=%d, \
							AtrMdStr=%d, AtrMdInt=%d, AtrMdWis=%d, AtrMdDex=%d, AtrMdCon=%d, CondOne=%d, CondTwo=%d, CondThree=%d, Pglen=%d \
							WHERE CharacterID=%d";
	const char* newpcstr = "INSERT INTO tblcharacters VALUES ( NULL, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, %d, %d, \
							%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \
							%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \"%s\", \
							%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \
							%d, \"%s\", %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \
							%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \"%s\", \
							%d, %d, \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, \"%s\", \"%s\", \"%s\", %d, %d, %d, %d, %d, \
							%d, %d, %d, %d, %d, %d, %d, %d, %d )";

	memset( buf, 0, MAX_STRING_LENGTH * sizeof(char) * 20);


	desc_str = escape_query(ch->description);
	title_str = escape_query(ch->pcdata->title);
	prompt_str = escape_query(ch->prompt);
	bamfin_str = escape_query(ch->pcdata->bamfin);
	bamfout_str = escape_query(ch->pcdata->bamfout);
	bamfsin_str = escape_query(ch->pcdata->bamfsin);
	bamfsout_str = escape_query(ch->pcdata->bamfsout);

	if (ch->mountshort)
		mountshort_str = escape_query(ch->mountshort);
	else
		mountshort_str = strdup("");

	if (!ch->new)
	{
		sprintf(buf, pcstr, ch->name, ch->short_descr, ch->long_descr, desc_str, prompt_str, ch->sex, ch->class, ch->multied, \
				ch->race, ch->clan, ch->updated, ch->clev, ch->ctimer, ch->stunned[0], ch->stunned[1], ch->stunned[2], \
				ch->stunned[3], ch->stunned[4], ch->damage_mods[0], ch->damage_mods[1], ch->damage_mods[2], ch->damage_mods[3], \
				ch->damage_mods[4], ch->damage_mods[5], ch->damage_mods[6], ch->damage_mods[7], ch->damage_mods[8], ch->damage_mods[9], \
				ch->damage_mods[10], ch->damage_mods[11], ch->damage_mods[12], ch->damage_mods[13], ch->damage_mods[14], \
				ch->damage_mods[15], ch->damage_mods[16], ch->damage_mods[17], ch->damage_mods[18], ch->damage_mods[19], \
				ch->wizinvis, ch->level, ch->pkill, ch->antidisarm, ch->mounted, ch->mountcharmed, mountshort_str, \
				ch->trust, ch->pcdata->security, ch->wizbit, ch->pcdata->awins, ch->pcdata->alosses, ch->pcdata->mobkills, \
				ch->questgiver, ch->played, ch->last_note, ( ch->in_room ? ch->in_room->vnum : ( ch->last_room ? ch->last_room->vnum : 0)), \
				ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, \
				ch->max_move, ch->bp, ch->max_bp, ch->gold, ch->guild_rank, ch->guild, ch->religion, ch->pcdata->recall, \
				ch->rtimer, ch->exp, ch->act, ch->act2, ch->affected_by, ch->affected_by2, ch->affected_by3, ch->affected_by4, \
				ch->affected_by_powers, ch->affected_by_weaknesses, ch->imm_flags, ch->res_flags, ch->vul_flags, \
				ch->shields, ch->position, ch->practice, ch->saving_throw, ch->alignment, ch->hitroll, ch->damroll, \
				ch->armor, ch->wimpy, ch->deaf, ch->pcdata->corpses, ch->questpoints, ch->nextquest, ch->countdown, \
				ch->questobj, ch->questmob, ch->rquestpoints, ch->rnextquest, ch->pcdata->pwd, ch->speaking, \
				ch->pcdata->learn, bamfin_str, bamfout_str,	bamfsin_str, bamfsout_str, \
				ch->pcdata->bankaccount, ch->pcdata->shares, ch->pcdata->immskll, title_str, ch->pcdata->who_text, \
				ch->pcdata->perm_str, ch->pcdata->perm_int, ch->pcdata->perm_wis, ch->pcdata->perm_dex, \
				ch->pcdata->perm_con, ch->pcdata->mod_str, ch->pcdata->mod_int, ch->pcdata->mod_wis, ch->pcdata->mod_dex, \
				ch->pcdata->mod_con, ch->pcdata->condition[0], ch->pcdata->condition[1], ch->pcdata->condition[2], \
				ch->pcdata->pagelen, ch->CharID);
	} else
	{
		sprintf(buf, newpcstr, ch->name, ch->short_descr, ch->long_descr, desc_str, prompt_str, ch->sex, ch->class, ch->multied, \
				ch->race, ch->clan, ch->updated, ch->clev, ch->ctimer, ch->stunned[0], ch->stunned[1], ch->stunned[2], \
				ch->stunned[3], ch->stunned[4], ch->damage_mods[0], ch->damage_mods[1], ch->damage_mods[2], ch->damage_mods[3], \
				ch->damage_mods[4], ch->damage_mods[5], ch->damage_mods[6], ch->damage_mods[7], ch->damage_mods[8], ch->damage_mods[9], \
				ch->damage_mods[10], ch->damage_mods[11], ch->damage_mods[12], ch->damage_mods[13], ch->damage_mods[14], \
				ch->damage_mods[15], ch->damage_mods[16], ch->damage_mods[17], ch->damage_mods[18], ch->damage_mods[19], \
				ch->wizinvis, ch->level, ch->pkill, ch->antidisarm, ch->mounted, ch->mountcharmed, mountshort_str, \
				ch->trust, ch->pcdata->security, ch->wizbit, ch->pcdata->awins, ch->pcdata->alosses, ch->pcdata->mobkills, \
				ch->questgiver, ch->played, ch->last_note, ( ch->in_room ? ch->in_room->vnum : ( ch->last_room ? ch->last_room->vnum : 0)), \
				ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, \
				ch->max_move, ch->bp, ch->max_bp, ch->gold, ch->guild_rank, ch->guild, ch->religion, ch->pcdata->recall, \
				ch->rtimer, ch->exp, ch->act, ch->act2, ch->affected_by, ch->affected_by2, ch->affected_by3, ch->affected_by4, \
				ch->affected_by_powers, ch->affected_by_weaknesses, ch->imm_flags, ch->res_flags, ch->vul_flags, \
				ch->shields, ch->position, ch->practice, ch->saving_throw, ch->alignment, ch->hitroll, ch->damroll, \
				ch->armor, ch->wimpy, ch->deaf, ch->pcdata->corpses, ch->questpoints, ch->nextquest, ch->countdown, \
				ch->questobj, ch->questmob, ch->rquestpoints, ch->rnextquest, ch->pcdata->pwd, ch->speaking, \
				ch->pcdata->learn, bamfin_str, bamfout_str,	bamfsin_str, bamfsout_str, \
				ch->pcdata->bankaccount, ch->pcdata->shares, ch->pcdata->immskll, ch->pcdata->title, ch->pcdata->who_text, \
				ch->pcdata->perm_str, ch->pcdata->perm_int, ch->pcdata->perm_wis, ch->pcdata->perm_dex, \
				ch->pcdata->perm_con, ch->pcdata->mod_str, ch->pcdata->mod_int, ch->pcdata->mod_wis, ch->pcdata->mod_dex, \
				ch->pcdata->mod_con, ch->pcdata->condition[0], ch->pcdata->condition[1], ch->pcdata->condition[2], \
				ch->pcdata->pagelen);
	}
 	    	    	 
	free(desc_str);
	free(title_str);
	free(prompt_str);
	free(bamfin_str);
	free(bamfout_str);
	free(bamfsin_str);
	free(bamfsout_str);
	free(mountshort_str);

	if (!do_query(buf))
	{
		sprintf(buf, "Unable to save %s!", ch->name);
		bug(buf,0);
		free(buf);
		return FALSE;
	} else
	{
		const char* alsdel = "DELETE FROM tblcharacteraliases WHERE CharacterID=%d";
		const char* alsstr = "INSERT INTO tblcharacteraliases VALUES (NULL, %d, \"%s\", \"%s\");";
		const char* lngdel = "DELETE FROM tblcharacterlanguages WHERE CharacterID=%d";
		const char* lngstr = "INSERT INTO tblcharacterlanguages VALUES (NULL, %d, %d, %d);";
		const char* skldel = "DELETE FROM tblcharacterskills WHERE CharacterID=%d";
		const char* sklstr = "INSERT INTO tblcharacterskills VALUES (NULL, %d, %d, %d);";
		ALIAS_DATA* a;
		int i;

		if (ch->new)
		{
			ch->CharID = (int) last_id();
			ch->new = FALSE;
		}

		// aliases
		sprintf(buf, alsdel, ch->CharID);
		if (do_query(buf))
		{
			for (a = ch->pcdata->alias_list; a; a = a->next)
			{
				desc_str = escape_query(a->old);
				title_str = escape_query(a->_new);

				sprintf(buf, alsstr, ch->CharID, desc_str, title_str);
				do_query(buf);
			}
		}

		// languages
		sprintf(buf, lngdel, ch->CharID);
		if (do_query(buf))
		{
			for(i=0; i< MAX_LANGUAGE; i++)
			{
				if (ch->language[i])
				{
					sprintf(buf, lngstr, ch->CharID, i, ch->language[i]);
					do_query(buf);
				}
			}
		}

		// skills
		sprintf(buf, skldel, ch->CharID);
		if (do_query(buf))
		{
			for (i=0; i< MAX_SKILL; i++)
			{
				if (ch->pcdata->learned[i])
				{
					sprintf(buf, sklstr, ch->CharID, i, ch->pcdata->learned[i]);
					do_query(buf);
				}
			}
		}

		// affects
		sql_save_aff(ch->affected,  ch->CharID, 1);
		sql_save_aff(ch->affected2, ch->CharID, 2);
		sql_save_aff(ch->affected3, ch->CharID, 3);
		sql_save_aff(ch->affected4, ch->CharID, 4);
	}

	ch->save_time = current_time;

	free(buf);
	return TRUE;
}

void sql_save_aff(AFFECT_DATA* aff, int CharID, int vector)
{
	const char* affstr = "INSERT INTO tblcharacteraffects VALUES ( NULL, %d, %d, %d, %d, %d, %d, %d, %d );";
	const char* delaff = "DELETE FROM tblcharacteraffects WHERE CharacterID=%d AND FlagNum=%d";
	char buf[MAX_STRING_LENGTH*10];
	AFFECT_DATA* a;

	sprintf(buf, delaff, CharID, vector);
	if (do_query(buf))
	{
		for (a = aff; a; a = a->next)
		{	
			if (a->deleted)
				continue;

			sprintf(buf, affstr, CharID, vector, a->type, a->level, a->duration, a->location, a->modifier, a->bitvector);
			do_query(buf);
		}
	}
}

/*
 * Rather than trying to search for old inventory, it's easier to
 * delete and re-insert. Otherwise we're doing too many record seeks
 * via UPDATE calls.
 */
bool sql_remove_old_inv(int CharID, bool Dirty)
{
	char InvTable[100]			= "";
	char DetTable[100]			= "";
	char buf[MAX_STRING_LENGTH]	= "";
	char lst[MAX_STRING_LENGTH] = "";
	MYSQL_RES* res				= NULL;
	MYSQL_ROW  row;
	int i						= 0;
	
	if (Dirty)
	{
		strcpy(InvTable, DIRTY_INV);
		strcpy(DetTable, DIRTY_DET);
	} else
	{
		strcpy(InvTable, TABLE_INV);
		strcpy(DetTable, TABLE_DET);
	}

	sprintf(buf, "SELECT InventoryID FROM %s WHERE CharacterID=%d", InvTable, CharID );

	if ((res=do_query(buf)))
	{
		int rows = rowcount(res);
		
		for (i = 0; i < rows; i++)
		{
			row = get_row(res);
			strcat(lst, row[0]);
			if ( i != (rows - 1) )
				strcat(lst, ",");
		}
		
	}

	sprintf(buf, "DELETE FROM %s WHERE CharacterID=%d", InvTable, CharID);
	
	if (!do_query(buf))
		return FALSE;

	sprintf(buf, "DELETE FROM %s WHERE InventoryID IN ( %s )", DetTable, lst);

	if (!do_query(buf))
		return FALSE;

	return TRUE;
}

/* 
 * Exact same function whether dirty or not. The only difference
 * is the table that gets updated. We probably don't need to update
 * the dirty table because we keep a list of dirty characters in mem
 * until they're dumped to the db, but this is for safety. This might
 * be good for something like a disconnect.
 */

/*
 * Current Algorithm:
 *  		       Delete All, For Each Item (Insert, Check for Custom )
 * Could be faster to:
 * 		       Select All, For Each Item ( Compare, If Different, Update )
 * But, this would require searching the linked list, and special cases for dleted
 * and new items.
 *
 * I don't feel like doing the math...
 * - Ahsile
 */
bool sql_save_inv(OBJ_DATA* obj, int CharID, int InsideOf, bool Storage, bool Dirty)
{
	char InvTable[100]			= "";
	char DetTable[100]			= "";
	char buf[MAX_STRING_LENGTH]	= "";
	const char* objstr			= "INSERT INTO %s VALUES ( %d, %d, %d, %d, %d, %d);";
	const char* detstr			= "INSERT INTO %s VALUES ( %d, %d, \"%s\", \"%s\", \"%s\", %d, %d);";
	MYSQL_RES* res				= NULL;
	MYSQL_ROW  row;
	OBJ_DATA* o					= NULL;
	int  last_obj				= 0;

	if (Dirty)
	{
		strcpy(InvTable, DIRTY_INV);
		strcpy(DetTable, DIRTY_DET);
	} else
	{
		strcpy(InvTable, TABLE_INV);
		strcpy(DetTable, TABLE_DET);
	}

	for (o = obj; o; o = o->next_content)
	{
		if (o->deleted)
			continue;

		/* clunky for now. mysql version doesn't support auto-increment for memory tables */
		sprintf(buf, "SELECT MAX(InventoryID) FROM %s", InvTable);
		if (!(res=do_query(buf)))
			return FALSE;
		else
		{
			row = get_row(res);
			last_obj = atoi(row[0])+1;
		}

		sprintf(buf, objstr, InvTable, last_obj, CharID, o->pIndexData->vnum, InsideOf, Storage, o->wear_loc);
		if (do_query(buf))
		{
			OBJ_INDEX_DATA* od = o->pIndexData;
			int det_id;

			// object details
			if ((((strcmp(o->short_descr,od->short_descr)!=0) || (o->ac_charge[0] != od->ac_charge[0]) || (o->ac_charge[1] != od->ac_charge[1])
				|| (o->value[1] != od->value[1]) || (o->value[2] != od->value[2]) || (strcmp(o->name,od->name)!=0) 
				|| (strcmp(o->description, od->description)!=0))) && (o->pIndexData->vnum != 1) )
			{
				char* short_desc_buf;
				char* long_desc_buf;
				char* name_buf;

				int invokes_used = o->ac_charge[1] - o->ac_charge[0];
				int charges_used = o->value[1] - o->value[2];

				if (invokes_used < 0 || invokes_used > o->ac_charge[1])
					invokes_used = 0;

				if (charges_used < 0 || charges_used > o->value[1])
					charges_used = 0;
				
				sprintf(buf, "SELECT MAX(InventoryDetailID) FROM %s", DetTable);
				if (!(res=do_query(buf)))
					return FALSE;
				else
				{
					row = get_row(res);
					det_id = atoi(row[0])+1;
				}

				
				short_desc_buf = escape_query(o->short_descr);
				long_desc_buf = escape_query(o->description);
				name_buf = escape_query(o->name);

				sprintf(buf, detstr, DetTable, det_id, last_obj, short_desc_buf, long_desc_buf, name_buf, invokes_used, charges_used);


				do_query(buf);
				
				free(short_desc_buf);
				free(long_desc_buf);
				free(name_buf);
				
			}

			// items inside
			if (o->contains)
			{
				 sql_save_inv(o->contains, CharID, last_obj, Storage, Dirty);	
			}
		}
	}

	return TRUE;
}

bool sql_save_area(AREA_DATA* pArea)
{
	char buf[MAX_STRING_LENGTH];

	if (!pArea->AreaID)	
	{
		const char* areastring = "INSERT INTO tblarea (AreaID, ShortName, Name, VnumLower, VnumHigh, Security, Builders, ResetString, Flags, Recall, AreaSound, MusicFile) VALUES (NULL, \"%s\", \"%s\", %d, %d, %d, \"%s\", \"%s\", %d, %d, \"%s\", \"%s\");";
		sprintf(buf, areastring, pArea->filename, pArea->name, pArea->lvnum, pArea->uvnum, pArea->security, pArea->builders, pArea->reset_sound, pArea->area_flags, pArea->recall, pArea->actual_sound, pArea->musicfile);
		if (do_query(buf))
			pArea->AreaID = (int) last_id();
		else
			return FALSE;
	} else
	{
		const char* areastring = "UPDATE tblarea SET ShortName=\"%s\", Name=\"%s\", VnumLower=%d, VnumHigh=%d, Security=%d, Builders=\"%s\", ResetString=\"%s\", Flags=%d, Recall=%d, AreaSound=\"%s\", MusicFile=\"%s\" WHERE AreaID=%d";
		sprintf(buf, areastring, pArea->filename, pArea->name, pArea->lvnum, pArea->uvnum, pArea->security, pArea->builders, pArea->reset_sound, pArea->area_flags, pArea->recall, pArea->actual_sound, pArea->musicfile, pArea->AreaID);
		if (!do_query(buf))
			return FALSE;
	}

	// save rooms
	if (!sql_save_rooms(pArea))
		return FALSE;

	// save mobiles
	if (!sql_save_mobiles(pArea))
		return FALSE;

	// save objects
	if (!sql_save_objects(pArea))
		return FALSE;

	if (!sql_save_resets(pArea))
		return FALSE;

	return TRUE;
}

bool sql_save_rooms(AREA_DATA* pArea)
{
	char buf[MAX_STRING_LENGTH*10]	= "";
	const char* roomstr			= "INSERT INTO tblrooms VALUES( %d, %d, \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, 0, 0, 0)";
	const char* edstr			= "INSERT INTO tblroomdescriptions VALUES(NULL, %d, \"%s\", \"%s\");";
	const char* exitstr			= "INSERT INTO tblroomexits VALUES (NULL, %d, %d, \"%s\", \"%s\", %d, %d, %d, %d, 0);";
	ROOM_INDEX_DATA* room		= NULL;
	int i;

	// delete old rooms
	sprintf(buf, "DELETE FROM tblrooms WHERE AreaID=%d", pArea->AreaID);
	if (!do_query(buf))
		return FALSE;

	// from lowvnum to uvnum
	for (i=pArea->lvnum; i <= pArea->uvnum; i++)
	{
		// getroomindex
		room = get_room_index(i);
		
		// room exists
		if (room)
		{
			EXTRA_DESCR_DATA* ed;
			int dir;
			bool hasProg = FALSE;
			bool hasED = FALSE;
			bool hasExit = FALSE;
			char* name_str = escape_query(room->name);
			char* desc_str = escape_query(room->description);

			// insert
			sprintf(buf, roomstr, room->vnum, pArea->AreaID, name_str, room->soundfile, room->musicfile,
						  desc_str, room->room_flags, room->sector_type);
			
			free(name_str);
			free(desc_str);

			if (!do_query(buf))
				continue;

			// save progs
			sprintf(buf, "DELETE FROM tblprograms WHERE Vnum=%d AND LoadType=%d", room->vnum, LOAD_ROOM); 
			if (do_query(buf))
			{			
				if (room->traps)
				{
					hasProg = TRUE;
					sql_save_traps(room->traps, room->vnum, LOAD_ROOM, 0);
				}
			}

			// save exits
			sprintf(buf, "DELETE FROM tblroomexits WHERE RoomVnum=%d", room->vnum);
			if (do_query(buf))
			{
				for (dir = 0; dir < MAX_DIR; dir++)
				{
					if (room->exit[dir])
					{
						EXIT_DATA* exit = room->exit[dir];

						char* exit_desc_str = escape_query(exit->description);
						char* exit_keyw_str = escape_query(exit->keyword);

						sprintf(buf, exitstr, room->vnum, dir, exit_desc_str, exit_keyw_str, exit->exit_info, exit->rs_flags, exit->key, exit->vnum);

						free(exit_desc_str);
						free(exit_keyw_str);

						if (do_query(buf))
						{
							hasExit = TRUE;

							// sigh, exits have traps (or at least the possibility to have a trap
							// even though not a single one exists on the mud )
							if (exit->traps)
							{
								int exit_id = (int) last_id(); // have to grab id here. save_traps
															   // may change this value

								sprintf(buf, "DELETE FROM tblprograms WHERE Vnum=%d And LoadType=%d", room->vnum, LOAD_EXIT);
								if (do_query(buf))
								{
									if(sql_save_traps(exit->traps, room->vnum, LOAD_EXIT, dir))
									{
										sprintf(buf, "UPDATE tblroomexits SET HasTrap=%d WHERE ExitID=%d", TRUE, exit_id);
										do_query(buf);
									}
								}
							}
						}
					}
				}
			}
			
			// save ed
			sprintf(buf, "DELETE FROM tblroomdescriptions WHERE vnum=%d", room->vnum);
			if (do_query(buf))
			{
				for (ed = room->extra_descr; ed; ed = ed->next)
				{
					char* ed_desc_str = escape_query(ed->description);

					sprintf(buf, edstr, room->vnum, ed->keyword, ed_desc_str);

					free(ed_desc_str);

					if (do_query(buf))
						hasED = TRUE;	
				}
			}

			sprintf(buf, "UPDATE tblrooms SET HasTrap=%d, HasExit=%d, HasED=%d WHERE vnum=%d", hasProg, hasExit, hasED, room->vnum);
			do_query(buf);

		}
	}

	return TRUE;
}

bool sql_save_mobiles(AREA_DATA* pArea)
{
	char buf[MAX_STRING_LENGTH*10] = "";
	const char* mobstr			= "INSERT INTO tblmobiles VALUES (%d, %d, \"%s\", \"%s\", \"%s\", %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \"%s\", 0, 0);";
	MOB_INDEX_DATA* mob			= NULL;
	int i;

	// delete old mobs
	sprintf(buf, "DELETE FROM tblmobiles WHERE AreaID=%d", pArea->AreaID);
	if (!do_query(buf))
		return FALSE;
	
	// from low vnum to uvnum
	for (i=pArea->lvnum; i<=pArea->uvnum; i++)
	{
		bool hasProg = FALSE;
		bool hasDesc = FALSE;

		// get mob index
		mob = get_mob_index( i );

		// mob exists
		if (mob)
		{
			char* mob_ldesc_str = escape_query(mob->long_descr);
			char* mob_sdesc_str = escape_query(mob->short_descr);
			
			// insert
			sprintf(buf, mobstr, mob->vnum, pArea->AreaID, mob->player_name, mob_sdesc_str, mob_ldesc_str,
						  mob->act, mob->affected_by, mob->affected_by2,
						  mob->affected_by3, mob->affected_by4, mob->alignment, mob->level,
						  mob->hitroll, mob->damroll, mob->hitnodice, mob->sex, mob->imm_flags,
						  mob->res_flags, mob->vul_flags, mob->speaking, spec_string( mob->spec_fun ));

			free(mob_ldesc_str);
			free(mob_sdesc_str);
			
			if (!do_query(buf))
				continue;

			// save progs
			sprintf(buf, "DELETE FROM tblprograms WHERE Vnum=%d AND LoadType=%d", mob->vnum, LOAD_MOB);
			if (do_query(buf))
			{
				if (mob->mobprogs)
				{
					if (sql_save_mprogs(mob->mobprogs, mob->vnum))
					{
						hasProg = TRUE;
					}
				}
			}

			// save desc
			sprintf(buf, "DELETE FROM tblmobiledescriptions WHERE Vnum=%d", mob->vnum);
			if (do_query(buf))
			{
				if (str_cmp(mob->description,str_empty))
				{
					char* mob_desc_str = escape_query(mob->description);
					sprintf(buf, "INSERT INTO tblmobiledescriptions VALUES (NULL, %d, %s)", mob->vnum, mob_desc_str);
					free(mob_desc_str);
					if (do_query(buf))
						hasDesc = TRUE;	
				}
			}

			sprintf(buf, "UPDATE tblmobiles SET HasProg=%d, HasDesc=%d WHERE Vnum=%d", hasProg, hasDesc, mob->vnum);
			do_query(buf);

		}
	}
			
	return TRUE;
}

bool sql_save_objects(AREA_DATA* pArea)
{
	char buf[MAX_STRING_LENGTH*10] = "";
	const char* objstr			= "INSERT INTO tblobjects VALUES (%d, %d, \"%s\", \"%s\", \"%s\", %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \"%s\", %d, %d, %d, %d, %d, 0, 0, 0);";
	const char* edstr			= "INSERT INTO tblobjectdescriptions VALUES ( NULL, %d, \"%s\", \"%s\" );";
	const char* affstr			= "INSERT INTO tblobjectaffects VALUES ( NULL, %d, %d, %d );";
	OBJ_INDEX_DATA* obj			= NULL;
	int i;

	// delete old objs
	sprintf(buf, "DELETE FROM tblobjects WHERE AreaID=%d", pArea->AreaID);
	if (!do_query(buf))
		return FALSE;

	// from low vnum to uvnum
	for (i = pArea->lvnum; i <= pArea->uvnum; i++)
	{
		bool hasProg = FALSE;
		bool hasAff = FALSE;
		bool hasED = FALSE;

		// get obj index
		obj = get_obj_index( i );

		// obj exists
		if (obj)
		{
			char* desc_str	= escape_query(obj->description);
			char* short_str	= escape_query(obj->short_descr);

			// insert
			sprintf( buf, objstr, obj->vnum, pArea->AreaID, obj->name, short_str, desc_str, obj->item_type,
			  obj->extra_flags, obj->extra_flags2, obj->extra_flags3, obj->extra_flags4,
			  obj->wear_flags, obj->level, obj->value[0], obj->value[1], obj->value[2], obj->value[3],
			  obj->weight, obj->cost, obj->ac_type, obj->ac_vnum, obj->ac_spell, obj->ac_charge[0],
			  obj->ac_charge[1], obj->join, obj->sep_one, obj->sep_two);

			free(desc_str);
			free(short_str);

			if (!do_query(buf))
				continue;

			// save progs
			sprintf(buf, "DELETE FROM tblprograms WHERE Vnum=%d AND LoadType=%d", obj->vnum, LOAD_OBJ);
			if (do_query(buf))
			{
				if (obj->traps)
				{
					if (sql_save_traps(obj->traps, obj->vnum, LOAD_OBJ, 0))
					{
						hasProg = TRUE;
					}
				}
			}
			
			
			// save aff
			sprintf(buf, "DELETE FROM tblobjectaffects WHERE Vnum=%d", obj->vnum);
			if (do_query(buf))
			{
				if (obj->affected)
				{
					AFFECT_DATA* aff;

					for (aff = obj->affected; aff; aff = aff->next)
					{
						sprintf(buf, affstr, obj->vnum, aff->location, aff->modifier);
						if (do_query(buf))
							hasAff = TRUE;
					}
				}
			}

			// save ed
			sprintf(buf, "DELETE FROM tblobjectdescriptions WHERE Vnum=%d", obj->vnum);
			if (do_query(buf))
			{
				if (obj->extra_descr)
				{
					EXTRA_DESCR_DATA* ed;

					for (ed = obj->extra_descr; ed; ed = ed->next)
					{
						char* ed_descr_str = escape_query(ed->description);
						sprintf(buf, edstr, obj->vnum, ed->keyword, ed_descr_str);
						free(ed_descr_str);

						if (do_query(buf))
							hasED = TRUE;
					}
				}
			}
		}
	}

	return TRUE;
}

bool sql_save_traps(TRAP_DATA* traplist, int Vnum, int Type, int Exit)
{
	char buf[MAX_STRING_LENGTH*10] = "";
	const char* trapstr			= "INSERT INTO tblprograms VALUES(NULL,%d, %d, %d, %d, \"%s\", \"%s\");";
	TRAP_DATA* trap				= NULL;

	for (trap = traplist; trap; trap = trap->next)
	{
		char* arglist_str = escape_query( trap->arglist );
		char* comlist_str = escape_query( trap->comlist );

		sprintf(buf, trapstr, Vnum, Exit, Type, trap->type, arglist_str, comlist_str);

		free( arglist_str );
		free( comlist_str );

		if (!do_query(buf))
			return FALSE;

	}
	return TRUE;
}

bool sql_save_mprogs(MPROG_DATA* mplist, int Vnum)
{
	char buf[MAX_STRING_LENGTH*10] = "";
	const char* mpstr		= "INSERT INTO tblprograms VALUES(NULL,%d, %d, %d, %d, \"%s\", \"%s\");";
	MPROG_DATA* mprog		= NULL;

	for (mprog = mplist; mprog; mprog = mprog->next)
	{
		char* arglist_str = escape_query( mprog->arglist );
		char* comlist_str = escape_query( mprog->comlist );

		sprintf(buf, mpstr, Vnum, mprog->status, LOAD_MOB, mprog->type, arglist_str, comlist_str);

		free( arglist_str );
		free( comlist_str );

		if (!do_query(buf))
			return FALSE;

	}
	return TRUE;
}

bool sql_save_bans()
{
	BAN_DATA* ban				= NULL;
	char buf[MAX_STRING_LENGTH] = "";

	if (!do_query("TRUNCATE tblbans"))
		return FALSE;

	for (ban = ban_list; ban; ban = ban->next)
	{
		sprintf(buf, "INSERT INTO tblbans VALUES (NULL, \"%s\", \"%s\");", ban->name, ban->user);
		do_query(buf);
	}
	return TRUE;
}

bool sql_save_disabled()
{
	DISABLED_DATA* dis			= NULL;
	char buf[MAX_STRING_LENGTH] = "";

	if (!do_query("TRUNCATE tbldisabled"))
		return FALSE;

	for (dis = disabled_first; dis; dis = dis->next)
	{
		sprintf(buf, "INSERT INTO tbldisabled VALUES (NULL, \"%s\", %d, %d, \"%s\");", dis->command->name, dis->uptolevel, dis->dislevel, dis->disabled_by);
		do_query(buf);
	}
	
	return TRUE;
}

bool sql_save_clans()
{
	CLAN_DATA* clan					= NULL;
	char buf[MAX_STRING_LENGTH * 10]= "";

	if (!do_query("TRUNCATE tblclans"))
		return FALSE;

	for (clan = clan_first; clan; clan = clan->next)
	{
		char* clan_descr = escape_query(clan->description);

		sprintf(buf, "INSERT INTO tblclans VALUES (NULL, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
							clan->name, clan_descr, clan->deity, clan->champ, clan->leader, clan->first, clan->second, clan->ischamp, clan->isleader, clan->isfirst, clan->issecond,
							clan->recall, clan->pkills, clan->mkills, clan->members, clan->pdeaths, clan->mdeaths, clan->obj_vnum_1, clan->obj_vnum_2, clan->obj_vnum_3, clan->pkill);

		free(clan_descr);

		do_query(buf);
	}

	return TRUE;
}

bool sql_save_religions()
{
	RELIGION_DATA* rel				= NULL;
	char buf[MAX_STRING_LENGTH * 10]= "";

	if (!do_query("TRUNCATE tblreligions"))
		return FALSE;

	for (rel = religion_first; rel; rel = rel->next)
	{
		char* descr_str = escape_query(rel->description);

		sprintf(buf, "INSERT INTO tblreligions VALUES ( NULL, \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, %d, %d, %d, %d );", 
					rel->name,  rel->shortdesc, rel->deity, descr_str,
					rel->recall, rel->pkills, rel->mkills, rel->pdeaths,
					rel->mdeaths, rel->members);

		free(descr_str);

		do_query(buf);
	}
	return TRUE;
}

bool sql_save_helps()
{
	HELP_DATA* help					= NULL;
	char buf[MAX_STRING_LENGTH * 10]= "";
	
	if (!do_query("TRUNCATE tblhelp"))
		return FALSE;

	for (help = help_first; help; help = help->next)
	{
		char* help_str = escape_query(help->text);

		sprintf(buf, "INSERT INTO tblhelp VALUES (NULL, \"%s\", %d, \"%s\");", help->keyword, help->level, help_str);

		free(help_str);

		do_query(buf);

	}
	return TRUE;
}

bool sql_save_socials()
{
	char buf[MAX_STRING_LENGTH * 10]= "";
	char* socstr					= "INSERT INTO tblsocials VALUES (NULL, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\");";
	int i = 0;

	if (!do_query("TRUNCATE tblsocials"))
		return FALSE;

	for (i = 0; i < maxSocial; i++)
	{
		SOCIAL_DATA* soc = &social_table[i];	
		char* char_no_arg_str = escape_query(soc->char_no_arg);
		char* others_no_arg_str = escape_query(soc->others_no_arg);
		char* char_found_str = escape_query(soc->char_found);
		char* others_found_str = escape_query(soc->others_found);
		char* vict_found_str = escape_query(soc->vict_found);
		char* char_auto_str = escape_query(soc->char_auto);
		char* others_auto_str = escape_query(soc->others_auto);

		sprintf(buf, socstr, soc->name, char_no_arg_str, others_no_arg_str, char_found_str, others_found_str, vict_found_str, char_auto_str, others_auto_str);
	 
		if(char_no_arg_str)
			free(char_no_arg_str);
		if (others_no_arg_str)
			free(others_no_arg_str);
		if (char_found_str)
			free(char_found_str);
		if (others_found_str)
			free(others_found_str);
		if (vict_found_str)
			free(vict_found_str);
		if (char_auto_str)
			free(char_auto_str);
		if (others_auto_str)
			free(others_auto_str);

		do_query(buf);

	}
	return TRUE;
}

/* Resets are a ROYAL pain in the ass!
 * They need to be in the correct order to work.
 * Ripped original function
 * - Ahsile
 */
bool sql_save_resets(AREA_DATA* pArea)
{
	char buf[MAX_STRING_LENGTH]	= "";
	int vnum = 0;
	RESET_DATA* pReset		= NULL;
	ROOM_INDEX_DATA* pRoomIndex	= NULL;
	MOB_INDEX_DATA* pLastMob	= NULL;
	OBJ_INDEX_DATA* pLastObj	= NULL;


	sprintf(buf, "DELETE FROM tblresets WHERE AreaID=%d", pArea->AreaID);
	if (!do_query(buf))
		return FALSE;

	for( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    	{
		if( ( pRoomIndex = get_room_index(vnum) ) )
		{
			if ( pRoomIndex->area == pArea )
			{
				for ( pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next )
				{
					switch ( pReset->command )
					{
						default:
							bug( "Save_resets: bad command %c.", pReset->command );
							continue;
							break;
						case 'M':
							pLastMob = get_mob_index( pReset->arg1 );
							sprintf(buf, "INSERT INTO tblresets VALUES (NULL, \"M\", %d, %d, %d, %d, %d)", 
										pReset->status, pReset->arg1, pReset->arg2, pReset->arg3, pArea->AreaID );
							break;
						case 'O':
							pLastObj = get_obj_index( pReset->arg1 );
							sprintf(buf, "INSERT INTO tblresets VALUES (NULL, \"O\", %d, %d, 0, %d, %d)", 
										pReset->status, pReset->arg1, pReset->arg3, pArea->AreaID );
							break;
						case 'P':
							pLastObj = get_obj_index( pReset->arg1 );
							sprintf(buf, "INSERT INTO tblresets VALUES (NULL, \"P\", %d, %d, 0, %d, %d)", 
										pReset->status,	pReset->arg1, pReset->arg3, pArea->AreaID  );
							break;
						case 'G':
							sprintf(buf, "INSERT INTO tblresets VALUES (NULL, \"G\", %d, %d, 0, 0, %d)", pReset->status, pReset->arg1, pArea->AreaID );
							if ( !pLastMob )
							{
								sprintf( buf,
									"Save_resets: !NO_MOB! in [%s]", pArea->filename );
								bug( buf, 0 );
							}
							break;
						case 'E':
							sprintf(buf, "INSERT INTO tblresets VALUES (NULL, \"E\", %d, %d, 0, %d, %d",
								pReset->status,	pReset->arg1, pReset->arg3, pArea->AreaID );
							if ( !pLastMob )
							{
								sprintf( buf,
									"Save_resets: !NO_MOB! in [%s]", pArea->filename );
								bug( buf, 0 );
							}
							break;
						case 'D':
							break;
						case 'R':
							sprintf(buf, "INSERT INTO tblresets VALUES (NULL, \"R\", %d, %d, %d, 0, %d)", 
								pReset->status, pReset->arg1, pReset->arg2, pArea->AreaID );
							break;
					}

					do_query(buf);
				}
			}
		}
    }

	return TRUE;
}

bool sql_save_shops()
{
	char buf[MAX_STRING_LENGTH]	= "";
	SHOP_DATA* shop				= NULL;

	if (!do_query("TRUNCATE tblshops"))
		return FALSE;

	for ( shop = shop_first; shop; shop = shop->next )
	{
		sprintf(buf, "INSERT INTO tblshops VALUES (NULL, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d);",
						shop->keeper, shop->profit_buy, shop->profit_sell, shop->open_hour, shop->close_hour,
						shop->buy_type[0], shop->buy_type[1], shop->buy_type[2], shop->buy_type[3], shop->buy_type[4]);
		do_query(buf);
	}

	return TRUE;
}

#endif // SQL_SYSTEM

