/*************************************************************************
 * memory.c
 * contains all the memroy allocation functions for the game.
 *
 * We use memory pools for efficiency and to trap memory bugs
 * items on the free list are memset to 0xd's, so tracking
 * references to them after they've been freed should be easy.
 * for a structure STRUCT, we make available 2 functions,
 * new_struct, and free_struct. new_struct will return a ready
 * to use structure, all zero'd out and ready to go.
 * free_struct takes care of freeing references, and zeroing to 0xd
 *
 * memory is never returned to the operating system.
 *
 * --Manaux
 ***********************************************************************/

/* $Id: memory.c,v 1.7 2005/02/22 23:55:18 ahsile Exp $ */

/*Adjust MEM_POOL_SIZE, smaller is less wasted memory, but slower */
/*100 seems to work fine, but anything from 10 to 1000 should work */
#define MEM_POOL_SIZE 100

#if defined(macintosh)
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

/*Hellish macros, but, makes all this automatic... --Manaux */
/* Define a function that returns a new object of type typ.
 * typ must be a struct and have a typ * next, ie, foo->next
 * the returned object is zero'd out, so it must be explicitely
 * initialized. Types that need special initialization should
 * declare as _foo, and write a wrapper function for them.
 * eg CREATE_ENTITY_ALLOC(BAR, _new_bar, bar_free, barcount, baralloc)
 * then write BAR* new_bar() as a wrapper 
 * see below.   --Manaux */

/* Added POOLS tracking to make sure of cleanup - Ahsile */

#define CREATE_ENTITY_ALLOC(typ, fcn, freelist, activecount, alloccount) \
int activecount, alloccount; \
typ * freelist; \
typ * fcn() { \
	int i;\
	typ * ret;\
	if (!freelist)\
	{\
		ret = (typ *)calloc(MEM_POOL_SIZE,sizeof(typ) );\
		if (!ret) \
		{\
			return 0;\
		}\
		for (i = 0; i < MEM_POOL_SIZE - 1; ++i)\
		{\
			ret[i].next = ret +i +1;\
		}\
		ret[MEM_POOL_SIZE-1].next = 0;\
		freelist = ret;\
		alloccount += MEM_POOL_SIZE;\
		if (!pool_first)\
		{\
			pool_first = pool_last = malloc(sizeof(POOLS));\
			pool_first->next = NULL;\
			pool_first->addr = ret;\
		} else\
		{\
			POOLS* tmp = malloc(sizeof(POOLS));\
			pool_last->next = tmp;\
			pool_last = tmp;\
			tmp->next = NULL;\
			tmp->addr = ret;\
		}\
	} \
	++activecount;\
	ret = freelist; \
	freelist = freelist->next; \
	memset(ret, 0x0, sizeof(typ));\
	return ret; \
}
	
/*Declare a function to return an object the the memory pool */
/*it will be zero'd out, so any strings contained should be
 * free'd first --Manaux */
#define CREATE_ENTITY_FREE(typ, fcn, freelist, activecount, unused) \
void fcn(typ * tmp)\
{\
	memset(tmp, 0xd0, sizeof(typ));\
	tmp->next = freelist;\
	freelist = tmp;\
	--activecount;\
	return;\
}

/*Declarations for  alloc and dealloc functions... */

CREATE_ENTITY_ALLOC( AFFECT_DATA, _new_affect, affect_free, top_affect, top_alloc_affect)
CREATE_ENTITY_ALLOC( AREA_DATA, _new_area, area_free,  __top_area, top_alloc_area)
CREATE_ENTITY_ALLOC( CLAN_DATA, _new_clan, clan_free, top_clan, top_alloc_clan)
CREATE_ENTITY_ALLOC( RELIGION_DATA, _new_religion, religion_free, top_religion, top_alloc_religion)
CREATE_ENTITY_ALLOC( BAN_DATA, _new_ban, ban_free, top_ban, top_alloc_ban)
CREATE_ENTITY_ALLOC( USERL_DATA, _new_userl, userl_free, top_userl, top_alloc_userl)
CREATE_ENTITY_ALLOC( CHAR_DATA, _new_char, char_free, top_char, top_alloc_char)
CREATE_ENTITY_ALLOC( DESCRIPTOR_DATA, _new_descriptor, descriptor_free, top_descriptor, top_alloc_descriptor)
CREATE_ENTITY_ALLOC( EXIT_DATA, _new_exit, exit_free, top_exit, top_alloc_exit)
CREATE_ENTITY_ALLOC( EXTRA_DESCR_DATA, _new_extra_descr, extra_descr_free, top_extra_descr, top_alloc_extra_descr)
CREATE_ENTITY_ALLOC( HELP_DATA, _new_help, help_free, top_help, top_alloc_help)
CREATE_ENTITY_ALLOC( MOB_INDEX_DATA, _new_mob_index, mob_index_free,  __top_mob_index, top_alloc_mob_index)
CREATE_ENTITY_ALLOC( NOTE_DATA, _new_note, note_free, top_note, top_alloc_note)
CREATE_ENTITY_ALLOC( OBJ_DATA, _new_obj, obj_free, top_obj, top_alloc_obj)
CREATE_ENTITY_ALLOC( OBJ_INDEX_DATA, _new_obj_index, obj_index_free, top_obj_index, top_alloc_obj_index)
CREATE_ENTITY_ALLOC( PC_DATA, _new_pc, pc_free, top_pc, top_alloc_pc)
CREATE_ENTITY_ALLOC( RESET_DATA, _new_reset, reset_free, top_reset, top_alloc_reset)
CREATE_ENTITY_ALLOC( ROOM_INDEX_DATA, _new_room_index, room_index_free, top_room_index, top_alloc_room_index)
CREATE_ENTITY_ALLOC( SHOP_DATA, _new_shop, shop_free, top_shop, top_alloc_shop)
CREATE_ENTITY_ALLOC( DISABLED_DATA, _new_disabled, disabled_free, top_disabled, top_alloc_disabled)
CREATE_ENTITY_ALLOC( MPROG_DATA, _new_mprog, mprog_free, top_mprog, top_alloc_mprog)
CREATE_ENTITY_ALLOC( MPROG_ACT_LIST, _new_mprog_act, mprog_act_free, top_mprog_act, top_alloc_mprog_act)
CREATE_ENTITY_ALLOC( ALIAS_DATA, _new_alias, alias_free, top_alias, top_alloc_alias)
CREATE_ENTITY_ALLOC( PHOBIA_DATA, _new_phobia, phobia_free, top_phobia, top_alloc_phobia)
CREATE_ENTITY_ALLOC( TRAP_DATA, _new_trap, trap_free, top_trap, top_alloc_trap)
CREATE_ENTITY_ALLOC( CORRUPT_AREA_LIST, _new_corrupt_area_list, corrupt_free, top_corrupt, top_alloc_corrupt)

CREATE_ENTITY_FREE( AFFECT_DATA, _free_affect, affect_free, top_affect, top_alloc_affect )
CREATE_ENTITY_FREE( AREA_DATA, _free_area, area_free, __top_area, top_alloc_area )
CREATE_ENTITY_FREE( CLAN_DATA, _free_clan, clan_free, top_clan, top_alloc_clan )
CREATE_ENTITY_FREE( RELIGION_DATA, _free_religion, religion_free, top_religion, top_alloc_religion )
CREATE_ENTITY_FREE( BAN_DATA, _free_ban, ban_free, top_ban, top_alloc_ban )
CREATE_ENTITY_FREE( USERL_DATA, _free_userl, userl_free, top_userl, top_alloc_userl )
CREATE_ENTITY_FREE( CHAR_DATA, _free_char, char_free, top_char, top_alloc_char )
CREATE_ENTITY_FREE( DESCRIPTOR_DATA, _free_descriptor, descriptor_free, top_descriptor, top_alloc_descriptor )
CREATE_ENTITY_FREE( EXIT_DATA, _free_exit, exit_free, top_exit, top_alloc_exit )
CREATE_ENTITY_FREE( EXTRA_DESCR_DATA, _free_extra_descr, extra_descr_free, top_extra_descr, top_alloc_extra_descr )
CREATE_ENTITY_FREE( HELP_DATA, _free_help, help_free, top_help, top_alloc_help )
CREATE_ENTITY_FREE( MOB_INDEX_DATA, _free_mob_index, mob_index_free, __top_mob_index, top_alloc_mob_index )
CREATE_ENTITY_FREE( NOTE_DATA, _free_note, note_free, top_note, top_alloc_note )
CREATE_ENTITY_FREE( OBJ_DATA, _free_obj, obj_free, top_obj, top_alloc_obj )
CREATE_ENTITY_FREE( OBJ_INDEX_DATA, _free_obj_index, obj_index_free, top_obj_index, top_alloc_obj_index )
CREATE_ENTITY_FREE( PC_DATA, _free_pc, pc_free, top_pc, top_alloc_pc )
CREATE_ENTITY_FREE( RESET_DATA, _free_reset, reset_free, top_reset, top_alloc_reset )
CREATE_ENTITY_FREE( ROOM_INDEX_DATA, _free_room_index, room_index_free, top_room_index, top_alloc_room_index )
CREATE_ENTITY_FREE( SHOP_DATA, _free_shop, shop_free, top_shop, top_alloc_shop )
CREATE_ENTITY_FREE( DISABLED_DATA, _free_disabled, disabled_free, top_disabled, top_alloc_disabled )
CREATE_ENTITY_FREE( MPROG_DATA, _free_mprog, mprog_free, top_mprog, top_alloc_mprog )
CREATE_ENTITY_FREE( MPROG_ACT_LIST, _free_mprog_act, mprog_act_free, top_mprog_act, top_alloc_mprog_act )
CREATE_ENTITY_FREE( ALIAS_DATA, _free_alias, alias_free, top_alias, top_alloc_alias )
CREATE_ENTITY_FREE( PHOBIA_DATA, _free_phobia, phobia_free, top_phobia, top_alloc_phobia )
CREATE_ENTITY_FREE( TRAP_DATA, _free_trap, trap_free, top_trap, top_alloc_trap )
CREATE_ENTITY_FREE( CORRUPT_AREA_LIST, _free_corrupt_area_list, corrupt_free, top_corrupt, top_alloc_corrupt )

DISABLED_DATA * new_disabled()
{
	DISABLED_DATA * ptr;
	ptr = _new_disabled();
	if (!ptr)
		return ptr;
	ptr->disabled_by = str_empty;
	return ptr;
}

void free_disabled(DISABLED_DATA * ptr)
{
	free_string (ptr->disabled_by);
	_free_disabled(ptr);
	return;
}
USERL_DATA * new_userl()
{
	USERL_DATA * ptr;
	ptr = _new_userl();
	if (!ptr)
		return 0;
	ptr->name = str_empty;
	ptr->user = str_empty;
	ptr->host = str_empty;
	ptr->lastlogin = str_empty;
	ptr->desc = str_empty;
	return ptr;
}
void free_userl(USERL_DATA * ptr)
{
	free_string(ptr->name);
	free_string(ptr->user);
	free_string(ptr->host);
	free_string(ptr->lastlogin);
	free_string(ptr->desc);
	_free_userl(ptr);
	return;
}
BAN_DATA * new_ban()
{
	BAN_DATA * ptr;
	ptr = _new_ban();
	if (!ptr)
		return 0;
	ptr->name = str_empty;
	ptr->user = str_empty;
	return ptr;
}
void free_ban(BAN_DATA *ptr)
{
	free_string(ptr->name);
	free_string(ptr->user);
	_free_ban(ptr);
	return;
}



DESCRIPTOR_DATA * new_descriptor()
{
	DESCRIPTOR_DATA * ptr;
	ptr = _new_descriptor();
	if (!ptr)
		return ptr;
	ptr->host = str_empty;
	ptr->user = str_empty;
	ptr->showstr_head = str_empty;
	ptr->showstr_point = str_empty;
	ptr->outbuf = str_empty;
	/* XXX need to find out how to handle char ** pString */
	return ptr;
}

void free_descriptor(DESCRIPTOR_DATA * ptr)
{
	free_string(ptr->host);
	free_string(ptr->user);
	free_string(ptr->showstr_head);
	free_string(ptr->showstr_point);
	free_string(ptr->outbuf);
	_free_descriptor(ptr);
	return;
}

HELP_DATA * new_help()
{
	HELP_DATA * ptr;
	ptr = _new_help();
	if (!ptr)
		return ptr;
	ptr->keyword = str_empty;
	ptr->text = str_empty;
	return ptr;
}

void free_help(HELP_DATA * ptr)
{
	free_string(ptr->keyword);
	free_string(ptr->text);
	_free_help(ptr);
	return;
}

SHOP_DATA * new_shop()
{
	return _new_shop();
}

void free_shop(SHOP_DATA * ptr)
{
	_free_shop(ptr);
	return;
}

NOTE_DATA * new_note()
{
	NOTE_DATA * ptr;
	ptr = _new_note();
	if (!ptr)
		return ptr;
	ptr->sender = str_empty;
	ptr->date = str_empty;
	ptr->to_list = str_empty;
	ptr->subject = str_empty;
	ptr->text = str_empty;
	return ptr;
}
void free_note(NOTE_DATA * ptr)
{
	free_string(ptr->sender);
	free_string(ptr->date);
	free_string(ptr->to_list);
	free_string(ptr->subject);
	free_string(ptr->text);
	_free_note(ptr);
	return;
}

AFFECT_DATA * new_affect()
{
	return _new_affect();
}
void free_affect(AFFECT_DATA * ptr)
{
	_free_affect(ptr);
	return;
}

MOB_INDEX_DATA * new_mob_index()
{
	MOB_INDEX_DATA * ptr;
	ptr = _new_mob_index();
	if (!ptr)
		return ptr;
	ptr->player_name = str_empty;
	ptr->short_descr = str_empty;
	ptr->long_descr = str_empty;
	ptr->description = str_empty;
	return ptr;
}
void free_mob_index(MOB_INDEX_DATA * ptr)
{
	free_string(ptr->player_name);
	free_string(ptr->short_descr);
	free_string(ptr->long_descr);
	free_string(ptr->description);
	_free_mob_index(ptr);
	return;
}

MPROG_ACT_LIST * new_mprog_act()
{
	MPROG_ACT_LIST * ptr;
	ptr = _new_mprog_act();
	if (!ptr)
		return ptr;
	ptr->buf = str_empty;
	return ptr;
}
void free_mprog_act(MPROG_ACT_LIST * ptr)
{
	free_string(ptr->buf);
	
	_free_mprog_act(ptr);
	return;
}
MPROG_DATA * new_mprog()
{
	MPROG_DATA * ptr;
	ptr = _new_mprog();
	if (!ptr)
		return ptr;
	ptr->arglist = str_empty;
	ptr->comlist = str_empty;
	return ptr;
}
void free_mprog(MPROG_DATA * ptr)
{
	free_string(ptr->arglist);
	free_string(ptr->comlist);
	
	_free_mprog(ptr);
	return;
}
CHAR_DATA * new_char()
{
	CHAR_DATA * ptr;
	ptr = _new_char();
	if (!ptr)
		return ptr;
	ptr->name = str_empty;
	ptr->short_descr = str_empty;
	ptr->long_descr = str_empty;
	ptr->description = str_empty;
	ptr->prompt = str_empty;
	/* XXX wtf is this nullstring */
	ptr->nullstring = str_empty;
	ptr->mountshort = str_empty;
	return ptr;
}
void free_char(CHAR_DATA * ptr)
{
	free_string(ptr->name);
	free_string(ptr->short_descr);
	free_string(ptr->long_descr);
	free_string(ptr->prompt);
	free_string(ptr->nullstring);
	free_string(ptr->mountshort);
	free_string(ptr->description);
	_free_char(ptr);
	return;
}

PC_DATA * new_pc()
{
	PC_DATA * ptr;
	ptr = _new_pc();
	if (!ptr)
		return ptr;
	ptr->pwd = str_empty;
	ptr->bamfin = str_empty;
	ptr->bamfout = str_empty;
	ptr->bamfsin = str_empty;
	ptr->bamfsout = str_empty;
	ptr->immskll = str_empty;
	ptr->title = str_empty;
	ptr->prompt = str_empty;
	ptr->lname = str_empty;
	ptr->who_text = str_empty;
	ptr->spouse = str_empty;
	ptr->plan = str_empty;
	ptr->email = str_empty;
	return ptr;
}
void free_pc(PC_DATA * ptr)
{
	free_string(ptr->pwd);
	free_string(ptr->bamfin);
	free_string(ptr->bamfout);
	free_string(ptr->bamfsin);
	free_string(ptr->bamfsout);
	free_string(ptr->immskll);
	free_string(ptr->title);
	free_string(ptr->prompt);
	free_string(ptr->lname);
	free_string(ptr->who_text);
	free_string(ptr->spouse);
	free_string(ptr->plan);
	free_string(ptr->email);
	_free_pc(ptr);
	return;
}

ALIAS_DATA * new_alias()
{
	ALIAS_DATA * ptr;
	ptr = _new_alias();
	if (!ptr)
		return ptr;
	ptr->old = str_empty;
	ptr->_new = str_empty;
	return ptr;
}
void free_alias(ALIAS_DATA * ptr)
{
	free_string(ptr->old);
	free_string(ptr->_new);
	_free_alias(ptr);
	return;
}

PHOBIA_DATA * new_phobia()
{
		return _new_phobia();
}
void free_phobia(PHOBIA_DATA * ptr)
{
	_free_phobia(ptr);
	return;
}

TRAP_DATA * new_trap()
{
	TRAP_DATA * ptr;
	ptr = _new_trap();
	if (!ptr)
		return ptr;
	ptr->arglist = str_empty;
	ptr->comlist = str_empty;
	return ptr;
}

void free_trap(TRAP_DATA * ptr)
{
	free_string(ptr->arglist);
	free_string(ptr->comlist);
	_free_trap(ptr);
	return;
}

OBJ_INDEX_DATA * new_obj_index()
{
	OBJ_INDEX_DATA * ptr;
	ptr = _new_obj_index();
	if (!ptr)
		return ptr;
	ptr->name = str_empty;
	ptr->short_descr = str_empty;
	ptr->description = str_empty;
	ptr->ac_spell = str_empty;
	return ptr;
}

void free_obj_index(OBJ_INDEX_DATA * ptr)
{
	free_string(ptr->name);
	free_string(ptr->short_descr);
	free_string(ptr->description);
	free_string(ptr->ac_spell);
	_free_obj_index(ptr);
	return;
}

OBJ_DATA * new_obj()
{
	OBJ_DATA * ptr;
	ptr = _new_obj();
	if (!ptr)
		return ptr;
	ptr->name = str_empty;
	ptr->short_descr = str_empty;
	ptr->description = str_empty;
	ptr->ac_spell = str_empty;
	return ptr;
}

void free_obj(OBJ_DATA * ptr)
{
	free_string(ptr->name);
	free_string(ptr->short_descr);
	free_string(ptr->description);
	free_string(ptr->ac_spell);
	_free_obj(ptr);
	return;
}

EXIT_DATA * new_exit()
{
	EXIT_DATA * ptr;
	ptr = _new_exit();
	if (!ptr)
		return ptr;
	ptr->keyword = str_empty;
	ptr->description = str_empty;
	return ptr;
}

void free_exit(EXIT_DATA * ptr)
{
	free_string(ptr->keyword);
	free_string(ptr->description);
	_free_exit(ptr);
	return;
}

RESET_DATA * new_reset()
{
	return _new_reset();
}
void free_reset(RESET_DATA * ptr)
{
	_free_reset(ptr);
	return;
}

AREA_DATA * new_area()
{
	AREA_DATA * ptr;
	ptr = _new_area();
	if (!ptr)
		return ptr;
	ptr->name = str_empty;
	ptr->filename = str_empty;
	ptr->builders = str_empty;
	ptr->reset_sound = str_empty;
	ptr->actual_sound = str_empty;
	ptr->musicfile = str_empty;
	return ptr;
}

void free_area(AREA_DATA * ptr)
{
	free_string(ptr->name);
	free_string(ptr->filename);
	free_string(ptr->builders);
	free_string(ptr->reset_sound);
	free_string(ptr->actual_sound);
	free_string(ptr->musicfile);
	_free_area(ptr);
	return;
}

CLAN_DATA * new_clan()
{
	CLAN_DATA * ptr;
	ptr = _new_clan();
	if (!ptr)
		return ptr;
	ptr->name = str_empty;
	ptr->deity = str_empty;
	ptr->description = str_empty;
	ptr->leader = str_empty;
	ptr->first = str_empty;
	ptr->second = str_empty;
	ptr->champ = str_empty;
	return ptr;
}

void free_clan(CLAN_DATA * ptr)
{
	free_string(ptr->name);
	free_string(ptr->deity);
	free_string(ptr->description);
	free_string(ptr->leader);
	free_string(ptr->first);
	free_string(ptr->second);
	free_string(ptr->champ);
	_free_clan(ptr);
	return;
}

RELIGION_DATA * new_religion()
{
	RELIGION_DATA * ptr;
	ptr = _new_religion();
	if (!ptr)
		return ptr;
	ptr->name = str_empty;
	ptr->deity = str_empty;
	ptr->description = str_empty;
	ptr->shortdesc = str_empty;
	return ptr;
}

void free_religion(RELIGION_DATA * ptr)
{
	free_string(ptr->name);
	free_string(ptr->deity);
	free_string(ptr->description);
	free_string(ptr->shortdesc);
	_free_religion(ptr);
	return;
}

ROOM_INDEX_DATA * new_room_index()
{
	ROOM_INDEX_DATA * ptr;
	ptr = _new_room_index();
	if (!ptr)
		return ptr;
	ptr->name = str_empty;
	ptr->description = str_empty;
	ptr->soundfile = str_empty;
	ptr->musicfile = str_empty;
	return ptr;
}

void free_room_index(ROOM_INDEX_DATA * ptr)
{
	free_string(ptr->name);
	free_string(ptr->description);
	free_string(ptr->soundfile);
	free_string(ptr->musicfile);
	_free_room_index(ptr);
	return;
}

EXTRA_DESCR_DATA * new_extra_descr()
{
	EXTRA_DESCR_DATA * ptr;
	ptr = _new_extra_descr();
	if (!ptr)
		return ptr;
	ptr->keyword = str_empty;
	ptr->description = str_empty;
	return ptr;
}

void free_extra_descr(EXTRA_DESCR_DATA * ptr)
{
	free_string(ptr->keyword);
	free_string(ptr->description);
	_free_extra_descr(ptr);
	return;
}

CORRUPT_AREA_LIST* new_corruptl()
{
	CORRUPT_AREA_LIST* ptr;
	ptr = _new_corrupt_area_list();
	if (!ptr)
		return ptr;
	ptr->vnum = 0;
	ptr->area = NULL;
	return ptr;
}

void free_corruptl(CORRUPT_AREA_LIST* ptr)
{
	_free_corrupt_area_list(ptr);
}

void do_memory( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    char       buf [ MAX_STRING_LENGTH ];

    rch = get_char( ch );
    
    if ( authorized( rch, "memory" ) )
    {
      sprintf( buf, "Structure  |        Allocated     |     In Active Use\n\r"); send_to_char(C_DEFAULT, buf, ch);
      sprintf( buf, "Affects          %5d                    %5d\n\r", top_alloc_affect, top_affect    ); send_to_char(C_DEFAULT, buf, ch );
      sprintf( buf, "Areas            %5d                    %5d\n\r", top_alloc_area, top_area      ); send_to_char(C_DEFAULT, buf, ch );
      sprintf( buf, "ExDes            %5d                    %5d\n\r", top_alloc_extra_descr, top_extra_descr ); send_to_char(C_DEFAULT, buf, ch );
      sprintf( buf, "Exits            %5d                    %5d\n\r", top_alloc_exit, top_exit      ); send_to_char(C_DEFAULT, buf, ch );
      sprintf( buf, "Helps            %5d                    %5d\n\r", top_alloc_help, top_help      ); send_to_char(C_DEFAULT, buf, ch );
      sprintf( buf, "Characters       %5d                    %5d\n\r", top_alloc_char, top_char ); send_to_char(C_DEFAULT, buf, ch );
      sprintf( buf, "Mob Index        %5d                    %5d\n\r", top_alloc_mob_index, top_mob_index ); send_to_char(C_DEFAULT, buf, ch );
      sprintf( buf, "Objs             %5d                    %5d\n\r", top_alloc_obj, top_obj ); send_to_char(C_DEFAULT, buf, ch );
      sprintf( buf, "Obj Index        %5d                    %5d\n\r", top_alloc_obj_index, top_obj_index ); send_to_char(C_DEFAULT, buf, ch );
      sprintf( buf, "Resets           %5d                    %5d\n\r", top_alloc_reset, top_reset     ); send_to_char(C_DEFAULT, buf, ch );
      sprintf( buf, "Rooms            %5d                    %5d\n\r", top_alloc_room_index, top_room_index      ); send_to_char(C_DEFAULT, buf, ch );
      sprintf( buf, "Shops            %5d                    %5d\n\r", top_alloc_shop, top_shop      ); send_to_char(C_DEFAULT, buf, ch );

      /*
      sprintf( buf, "Strings %5d strings of %7d bytes (max %d).\n\r",
	      nAllocString, sAllocString, MAX_STRING );
      send_to_char(C_DEFAULT, buf, ch );

      sprintf( buf, "Perms   %5d blocks of %7d bytes.  Max %7d bytes.\n\r",
	      nAllocPerm, sAllocPerm, 10*1024*1024 );
      send_to_char(C_DEFAULT, buf, ch );
      sprintf( buf, "%3d%% of max used.\n\r", (sAllocPerm*10) / (1024*1024) );
      send_to_char(C_DEFAULT, buf, ch );
      sprintf( buf, "Mem used: %d bytes of %d max.\n\r",
	     sAllocPerm, 10*1024*1024 );
      sprintf( buf + strlen(buf), "%3d%% of max used.\n\r",
	     (sAllocPerm*10) / (1024*1024) );
      send_to_char(C_DEFAULT, buf, ch );
      */
      return;
    }

    else
    {
    send_to_char(C_DEFAULT, "You are not authorized to use the memory command.\r\n", ch );
    return;
    }
}

// Ahsile
void free_allocated_mem( void )
{
	POOLS* ptr = pool_first;
	POOLS* ptr_next = NULL;

	cleanup_strings();
	cleanup_disabled();

	while(ptr)
	{
		ptr_next = ptr->next;
		free(ptr->addr);
		free(ptr);		
		ptr = ptr_next;
	};

	free( social_table );
}
