/******************************************************************************
 *  SSM v2.2 (shared string manager)                                          *
 *                                                                            *
 *  Copyright(C) 1996 Melvin Smith (Fusion) for EnvyMUD 2.2                   *
 *                                                                            *
 *  Due to alignment differences on 32 bit and 64 bit machines, memory        *
 *  usage is now virtually identical to standard Merc on 32-bit               *
 *  architecture, but slightly larger on 64-bit. Memory usage is still        *
 *  smaller than SSM 2.0 or earlier. The manager now uses short ints for      *
 *  the count and size of chunks, so to increase MAX_STRING you must          *
 *  increase MAX_CHUNKS instead. String have a max reference count of         *
 *  +32766 and max size of CHUNK_SIZE (0xfff0). Fragmentation is also         *
 *  handled more efficiently by marking failed chunks with -1 to temporarily  *
 *  disable them until a defrag_heap() recycles them. This helps when a       *
 *  4 byte chunk is freed low in the heap, so string_dup() doesn't walk       *
 *  the whole heap every time.                                                *
 *                                                                            *
 *  <msmith@falcon.mercer.peachnet.edu>                                       *
 *                                                                            *
 *  ROM2.4 modifications by Tom Adriaenssen (Jan 1996) -- Wreck               *
 *                                                                            *
 *  <tadriaen@zorro.ruca.ua.ac.be>                                            *
 *                                                                            *
 *  Removed ROM 2.4 modifications as Envy doesnt need *fread_string_eol -Kahn *
 *                                                                            *
 *****************************************************************************/

/*$Id: ssm.c,v 1.14 2005/02/22 23:55:19 ahsile Exp $*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#if !defined( ultrix )
#include <memory.h>
#endif


#include "merc.h"

#define intType        short int
#define uintType       unsigned intType
#define intTypeSize  ( sizeof( intType ) )
#define addrType       void * 
#define addrTypeSize ( sizeof( addrType ) )
#define addrSizeMask ( sizeof( addrType ) - 1 )

#define MAX_STR_KEY  21173
#define HASH(pt)    (strlen(pt) < 3)?((unsigned int)pt%MAX_STR_KEY):(((((((unsigned int)pt[0]<< 3) ^ (unsigned int)pt[2])<< 3)^(unsigned int)pt[1])) % MAX_STR_KEY)

/* To allocate more memory increase MAX_CHUNKS in merc.h. */
#define CHUNK_SIZE   0xfff0                  /* DO NOT mess with this! */ 
long    MAX_STRING = MAX_CHUNKS * CHUNK_SIZE;
int     HEADER_SIZE;


typedef struct BE BufEntry;

struct BE
{
    BufEntry *next;
    uintType  size;   /* size of the chunk (regardless of NULL CHAR) */ 
    intType   usage;  /* how many pointers to the string */ 
    char      buf[1]; /* chunk starts here */
};

/*
 * This is for the temporary hashing of strings at bootup to speedup
 * comparison/crunching of the string space. The temp_string_hash will
 * be freed after boot_done() is called.
 */
/*
typedef struct TH TempHash;

struct TH
{
    TempHash *next;
    uintType  len;
    char     *str;
};

TempHash **temp_string_hash;
*/
typedef struct STR_HASH STRHASH;
typedef STRHASH* STR_HASH_PTR;

struct STR_HASH {
 	char	*str;
	int usage;
	STR_HASH_PTR next;
};

/* These are the original Merc vars in db.c */
char         str_empty[1];
char        *string_space;
char        *top_string;
long         nAllocString;
long         sAllocString;
long         nOverFlowString;
long         sOverFlowString;

bool         Full;

char         *str_dup        ( const char * );
char         *fread_string   ( FILE * );

#ifdef MEM_DEBUG

void          f_string    ( char * );
STR_HASH_PTR StrTableStart = NULL;
STR_HASH_PTR StrTableEnd = NULL;
bool 	     free_on_stack = FALSE;

#else

void          free_string    ( char * );
STR_HASH_PTR  StrTable[MAX_STR_KEY];

#endif
/*
 * ssm_buf_head points to start of shared space,
 * ssm_buf_free points to next free block
 */ 
BufEntry *ssm_buf_head, *ssm_buf_free;

/*
 * Not sure what is a good value for MAX_FREE 
 * If a dup fails str_dup will not defrag unless the number
 * of numFree >= MAX_FREE. numFree is NOT the current number of free blocks,
 * it is just a counter so defrag doesnt start dragging the game in the
 * case of a lot of failed dups.
 */
#define MAX_FREE     8000

void init_string_space()
{
#ifndef MEM_DEBUG
/*
    temp_string_hash    = (TempHash **)calloc( 
					      MAX_KEY_HASH,sizeof(TempHash *) );
*/
    memset( StrTable, 0, sizeof(STR_HASH_PTR) * MAX_STR_KEY );
    str_empty[0] = '\0';
    nAllocString=0;
	  sAllocString=0;
#endif    
}
    
/*
 * Dup a string into shared space. If string exists, the usage count
 * gets incremented and the reference is returned. If the string does
 * not exist in heap, space is allocated and usage is 1.
 * This algorithm uses a hash table to keep track of strings. Ensuring that
 * only one copy of a string can exists in that table at any time.
 */
char *str_dup( const char *str )
{
#ifndef MEM_DEBUG
    char     *str_new;
    int       len;
    STR_HASH_PTR ptr;
    int       iIdx;

    if( !str || !*str )
        return str_empty;

    iIdx = HASH(str);
    for( ptr = StrTable[iIdx]; ptr; ptr = ptr->next ) {
      if( !strcmp(str, ptr->str) ) {
/*
	if (find_corruption(ptr->str))
	{
		char buf[MAX_STRING_LENGTH];
		sprintf(buf, "A corrupted string has been found in the string table. Address:0x%x", (unsigned int)ptr->str);
		bug(buf,0);
		bug("A dummy string has been used in its place.",0);
		return OFFENDING_STRING;
	} else
	{
	}
*/
        	ptr->usage++;
        	return ptr->str;

      }
    }
    
    len = (int)strlen( str ) + 1;
    ptr = malloc( sizeof(STRHASH));
    str_new = (char *)calloc( len , sizeof(char) );
    
    if( !str_new || !ptr) {
	    return str_empty;
    }
    strcpy( str_new, str ); 
    ptr->str = str_new;
    ptr->usage = 1;
    ptr->next = StrTable[iIdx];
    StrTable[iIdx] = ptr;

    nAllocString++;
    sAllocString+= len;

    return str_new;
#else
    /* Our brand spanking new str_dup function
       		- Ahsile
    */
    char* str_new = NULL;

    if (!str || str == str_empty)
	return str_empty;

    str_new = (char*)calloc( strlen( str ) + 1, sizeof(char));
    strcpy( str_new, str );

    if(!StrTableStart)
    {
	StrTableStart = (STR_HASH_PTR)calloc(1, sizeof(STRHASH));
        StrTableEnd   = StrTableStart;
    } else
    {
        STR_HASH_PTR  temp = (STR_HASH_PTR)calloc(1, sizeof(STRHASH));		
	StrTableEnd->next  = temp;
	StrTableEnd 	   = temp;
    }
    StrTableEnd->next = NULL;
    StrTableEnd->str  = str_new;

    return str_new;
#endif
}

/*
 * If string is in shared space, decrement usage, if usage then is 0,
 * free the string and the space used for the hash pointer str.
 */
#ifdef MEM_DEBUG
void f_string( char *str )
#else
void free_string( char *str )
#endif
{
#ifndef MEM_DEBUG
  STR_HASH_PTR ptr,ptr_last;
  int       iIdx;

  if( !str || str == str_empty )
    return;

  iIdx = HASH(str);
  for( ptr = StrTable[iIdx],ptr_last=NULL; ptr; ptr_last=ptr, ptr = ptr->next ) {
    if( /*!strcmp(str, ptr->str)*/ str == ptr->str ) {
      ptr->usage--;
      if( ptr->usage <= 0 ) {
		 sAllocString-= strlen(str);
         free(ptr->str);
         nAllocString--;
         if(ptr_last == NULL ) {
           StrTable[iIdx] = ptr->next;
         } else {
           ptr_last->next = ptr->next;
         }
         free(ptr);
         return;
      } else
	  {
		  break;
	  }
    }
  }
  return;
#else
  /* Our brand spanking new free_string function
			- Ahsile
  */
  STR_HASH_PTR cur  = NULL;
  STR_HASH_PTR last = NULL;
  char	       buf[MAX_STRING_LENGTH];
  if (!str || str==str_empty)
	return;
  
  for(cur = StrTableStart; cur; cur = cur->next )
  {
	if (str==cur->str)
	{
		free( str );
		last->next = cur->next;
		free( cur );	
		return;
	} else
	{
	    last = cur;
	}
  } 

  // avoid recursive calling of free_string
  free_on_stack = TRUE;

  if (!free_on_stack)
  {  
	sprintf(buf, "Function: %s is tried to free string 0x%x but it doesn't exist", callee_name, str);
  	bug(buf, 0);
  }

  free_on_stack = FALSE;

  return;
#endif
}



/*
 * Read and allocate space for a string from a file.
 * This replaces db.c fread_string
 * This is modified version of Furey's fread_string from Merc
 */
char *fread_string( FILE *fp )
{
    char buf[ MAX_STRING_LENGTH*4 ];
    char *ptr = buf;
    char  c;
    bool err = FALSE;

    do { c = getc( fp ); }
    while( isspace( c ) );

    if( ( *ptr++ = c ) == '~' )
        return str_empty;

    for ( ;; )
    {
	switch ( *ptr = getc( fp ) )
	{
	    default:
	    {
		// make sure string is in valid printable range
		// or TAB
	        if ((*ptr < 32 || *ptr > 126) && (*ptr != 9)) 
		{
			err = TRUE;
		}
		ptr++;
		break;
  	    }
	    case EOF:
		bug( "Fread_string: EOF", 0 );
		exit( 1 );
		break;

	    case '\n':
		ptr++;
		*ptr++ = '\r';
		break;

	    case '\r':
		break;

	    case '~':
		if (err)
			return str_dup(OFFENDING_STRING);

		*ptr = '\0';

		return str_dup( buf );
	}
    }
}

void cleanup_strings( void )
{
	int i = 0;

	for (i = 0; i < MAX_STR_KEY; i++)
	{
		if (StrTable[i])
		{
			STR_HASH_PTR ptr = StrTable[i];
			STR_HASH_PTR ptr_next = NULL;
			while(ptr)
			{
				ptr_next = ptr->next;
				if (ptr->str)
				{
					free( ptr->str );
				}
				free( ptr );
				ptr = ptr_next;
			}
		}
	}
}

