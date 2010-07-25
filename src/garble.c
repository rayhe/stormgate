/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael         *
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

/* $Id: garble.c,v 1.2 2005/02/22 23:55:16 ahsile Exp $ */


/* optional garble code - ahsile */

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"

char* translation_garbled( char* speech, int miss_by, char* buf, int buflen )
{
	char*	speech_copy		= NULL;
	char	arg[MAX_STRING_LENGTH]	= "";
	char**	wordarray		= NULL;
	int	wordcount		= 1;
	int	i			= 0; 

	if (miss_by <= 0)
		return speech;

	strncpy(buf, speech, buflen);

	speech_copy = buf;

	while ( strcmp( ( speech_copy = one_argument( speech_copy, arg ) ), "") )
	{
		wordcount++;
	}

	speech_copy = buf;

	wordarray = malloc(sizeof(char**) * wordcount);

	for (i = 0; i < wordcount; i++)
	{
		speech_copy  = one_argument(speech_copy, arg);
		wordarray[i] = malloc(sizeof(char) * strlen(arg) + 1);
		strcpy(wordarray[i], arg);
	}

	for (i = 0; i < wordcount; i++)
	{
		unsigned int j;
		for (j = 0; j < strlen(wordarray[i]); j++)
		{
			if (wordarray[i][j]> 64)
				if (number_percent() < miss_by )
				{
					if (isupper(wordarray[i][j])) 
						wordarray[i][j]+= (number_range(65, 90) - wordarray[i][j]);
					else 
						wordarray[i][j]+= (number_range(97,122) - wordarray[i][j]);
				}
		}
	}
	
	strcpy(buf, "");

	for (i = 0; i < wordcount; i++)
	{
		strcat(buf, wordarray[i]);	
		if (i != wordcount -1)
			strcat(buf, " ");
		free(wordarray[i]);
	}
	free(wordarray);

	return buf;

}
