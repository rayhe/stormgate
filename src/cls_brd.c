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

/*$Id: cls_brd.c,v 1.2 2005/03/10 14:54:58 tyrion Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"

int spell_sonic_boom( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
          0,   0,   0,   0,  30,         35,  40,  45,  50,  55,
         60,  65,  70,  75,  80,         82,  84,  86,  88,  90,
         92,  94,  96,  98, 100,        102, 104, 106, 108, 110,
        112, 114, 116, 118, 120,        122, 124, 126, 128, 130,
        132, 134, 136, 138, 140,        142, 144, 146, 148, 150,
        152, 154, 156, 158, 160,        162, 164, 166, 168, 170,
        172, 174, 176, 178, 180,        182, 184, 186, 188, 190,
        192, 194, 196, 198, 200,        202, 204, 206, 208, 210,
        215, 220, 225, 230, 235,        240, 245, 250, 255, 260
    };
    int        dam;
        
    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam      = number_range( dam_each[level] * 3, dam_each[level] * 10 );
        
    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;
}

int spell_sonic_blast( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim       = (CHAR_DATA *) vo;
    static const int        dam_each [ ] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
          0,   0,   0,   0,  30,         35,  40,  45,  50,  55,
         60,  65,  70,  75,  80,         82,  84,  86,  88,  90,
         92,  94,  96,  98, 100,        102, 104, 106, 108, 110,
        112, 114, 116, 118, 120,        122, 124, 126, 128, 130,
        132, 134, 136, 138, 140,        142, 144, 146, 148, 150,
        152, 154, 156, 158, 160,        162, 164, 166, 168, 170,
        172, 174, 176, 178, 180,        182, 184, 186, 188, 190,
        192, 194, 196, 198, 200,        202, 204, 206, 208, 210,
        215, 220, 225, 230, 235,        240, 245, 250, 255, 260
    };
    int        dam;

    level    = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level    = UMAX( 0, level );
    dam      = number_range( dam_each[level], dam_each[level] * 6 );

    if ( saves_spell( level, victim ) )
        dam /= 2;
    //damage( ch, victim, dam, sn );
    return dam;   
}
