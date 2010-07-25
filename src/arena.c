/*$Id: arena.c,v 1.4 2005/02/22 23:55:15 ahsile Exp $*/

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "merc.h"

extern bool fight_in_progress;

/*=======================================================================*
 * function: do_challenge                                                *
 * purpose: sends initial arena match query                              *
 * written by: Doug Araya (whiplash@tft.nacs.net) 6-10-96                *
 *=======================================================================*/

void do_challenge( CHAR_DATA *ch, char *argument )
 {
  CHAR_DATA *victim; 
  char buf[MAX_STRING_LENGTH]; 

/* == First make all invalid checks == */
 if(IS_NPC(ch))
 return; 

 if( fight_in_progress )
  {
  send_to_char(AT_BLOOD,"Sorry, there is a fight in progress, please wait a few moments.\n\r",ch); 
  return; 
  }

 if(IS_SET(ch->affected_by3, AFF_CHALLENGED))
  {
  send_to_char(AT_BLOOD,"You have already been challenged, either ACCEPT or DECLINE first.\n\r",ch);
  return; 
  }

 if(ch->hit < ch->max_hit)
  {
  send_to_char(AT_BLOOD,"You must be fully healed to fight in the arena.\n\r",ch); 
  return;
  }

 if(IS_AFFECTED(ch,AFF_INVISIBLE))
 {
 send_to_char(AT_BLOOD,"Not while invisible.\n\r",ch);
 return;
 }

 if((victim = get_char_world(ch,argument)) == NULL)
  {
  send_to_char(AT_BLOOD,"They are not playing.\n\r",ch);
  return;
  }

 if(IS_NPC(victim) || IS_IMMORTAL(victim) || victim == ch )
  {
send_to_char(AT_BLOOD,"You cannot challenge NPC's, Immortals, or yourself.\n\r",ch);
return;
  }

 if(IS_AFFECTED(victim,AFF_BLIND))
 {
 send_to_char(AT_BLOOD,"That person is blind right now.\n\r",ch);
 return;
 }

 if(IS_SET(victim->affected_by3,AFF_CHALLENGER))
  {
send_to_char(AT_BLOOD,"They have already challenged someone else.\n\r",ch);
return;
  }

 if(victim->fighting != NULL )
  {
send_to_char(AT_BLOOD,"That person is engaged in battle right now.\n\r",ch); 
return; 
  }

 if(victim->hit < victim->max_hit)
  {
send_to_char(AT_BLOOD,"That player is not healthy enough to fight right now.\n\r",ch);
return;
  }

 if(victim->level <=5 )
  {
send_to_char(AT_BLOOD,"That player is just a newbie!\n\r",ch);
return; 
  }

/* == Now for the challenge == */
 SET_BIT(ch->affected_by3, AFF_CHALLENGER);
 send_to_char(AT_BLOOD,"Challenge has been sent\n\r",ch);
 act(AT_BLOOD,"$n has challenged you to a death match.",ch,NULL,victim,TO_VICT);
 sprintf(buf,"type: ACCEPT %s to meet the challenge.\n\r",ch->name);
 send_to_char(AT_BLOOD,buf,victim);
 sprintf(buf,"type: DECLINE %s to chicken out.\n\r",ch->name); 
 send_to_char(AT_BLOOD,buf,victim);
 return;
}

/*=======================================================================*
 * function: do_accept                                                   *
 * purpose: to accept the arena match, and move the players to the arena *
 * written by: Doug Araya (whiplash@tft.nacs.net) 6-10-96                *
 *=======================================================================*/
void do_accept(CHAR_DATA *ch, char *argument)
 {
 CHAR_DATA *victim; 
 char buf[MAX_STRING_LENGTH]; 
 int char_room; 
 int vict_room; 
 extern bool fight_in_progress; 

 /*== Default to NO fight in progress ==*/
 fight_in_progress = !fight_in_progress;

 /*== the room VNUM's for our arena.are ==*/
 /* we use 1051 thru 1066 for a 4x4 arena */
 char_room = number_range(2700,2799);
 vict_room = number_range(2700,2799);

/* == first make all invalid checks == */
 if(IS_NPC(ch))
 return;

 if((victim = get_char_world(ch,argument)) == NULL)
  {
send_to_char(AT_BLOOD,"They aren't logged in!\n\r",ch);
return;
  }

 if(victim == ch)
  {
send_to_char(AT_BLOOD,"You haven't challenged yourself!\n\r",ch);
return;
  }

 if(!IS_SET(victim->affected_by3, AFF_CHALLENGER))
  {
send_to_char(AT_BLOOD,"That player hasn't challenged you!\n\r",ch); 
return; 
  }

/* == now get to business == */
SET_BIT(ch->affected_by3, AFF_CHALLENGED); 
send_to_char(AT_BLOOD,"You have accepted the challenge!\n\r",ch);
act(AT_BLOOD,"$n accepts your challenge!",ch,NULL,victim,TO_VICT);
/* == announce the upcoming event == */
sprintf(buf,"[Arena] %s (%d wins) (%d losses)\n\r",
victim->name,victim->pcdata->awins,victim->pcdata->alosses);
log_string( buf, CHANNEL_INFO, -1 );
/*NEWS(buf);*/
sprintf(buf,"[Arena] %s (%d wins) (%d losses)\n\r",ch->name,ch->pcdata->awins,ch->pcdata->alosses); 
log_string( buf, CHANNEL_INFO, -1 );
/*NEWS(buf); */
/* == now move them both to an arena for the fun == */
send_to_char(AT_BLOOD,"You make your way into the arena.\n\r",ch);
char_from_room(ch); 
char_to_room(ch,get_room_index(char_room));
send_to_char(AT_BLOOD,"You make your way to the arena.\n\r",victim); 
char_from_room(victim); 
char_to_room(victim,get_room_index(vict_room)); 
/*NEWS("[Arena] To wager on the fight, type: wager <amount> <player 
name>\n\r");*/ 
sprintf(buf,"[Arena] To wager on the fight, type: wager <amount> <player name>\n\r");
log_string( buf, CHANNEL_INFO, -1 );
fight_in_progress = TRUE; 
return; 
}

/*=======================================================================*
 * function: do_decline                                                  *
 * purpose: to chicken out from a sent arena challenge                   *
 * written by: Doug Araya (whiplash@tft.nacs.net) 6-10-96                *
 *=======================================================================*/
void do_decline(CHAR_DATA *ch, char *argument )
 {
  CHAR_DATA *victim; 

/*== make all invalid checks == */
  if(IS_NPC(ch))
  return;
  
 if((victim = get_char_world(ch,argument)) == NULL)
  {
send_to_char(AT_BLOOD,"They aren't logged in!\n\r",ch);
return;
  }

 if(!IS_SET(victim->affected_by3, AFF_CHALLENGER))
  {
send_to_char(AT_BLOOD,"That player hasn't challenged you.\n\r",ch);
return;
  }

 if(victim == ch)
 return; 

/*== now actually decline == */
REMOVE_BIT(victim->affected_by3, AFF_CHALLENGER); 
send_to_char(AT_BLOOD,"Challenge declined!\n\r",ch);
act(AT_BLOOD,"$n has declined your challenge.",ch,NULL,victim,TO_VICT);
return;
}

/*======================================================================*
 * function: do_bet                                                     *
 * purpose: to allow players to wager on the outcome of arena battles   *
 * written by: Doug Araya (whiplash@tft.nacs.net) 6-10-96               *
 *======================================================================*/
void do_wager(CHAR_DATA *ch, char *argument)
 {
 char arg[MAX_INPUT_LENGTH]; 
 char buf[MAX_STRING_LENGTH]; 
 CHAR_DATA *fighter; 
 int wager; 

 argument = one_argument(argument, arg); 

 if(argument[0] == '\0' || !is_number(arg))
  {
send_to_char(AT_BLOOD,"Syntax: WAGER [amount] [player]\n\r",ch); 
return;
  }

/*== disable the actual fighters from betting ==*/
 if(IS_SET(ch->affected_by3,AFF_CHALLENGER) || IS_SET(ch->affected_by3,AFF_CHALLENGED))
  {
send_to_char(AT_BLOOD,"You can't bet on this battle.\n\r",ch); 
return; 
  }

/*== make sure the choice is valid ==*/
 if((fighter = get_char_world(ch,argument)) == NULL)
  {
send_to_char(AT_BLOOD,"That player is not in the arena.\n\r",ch); 
return; 
  }

/*== do away with the negative number trickery ==*/
 if(!str_prefix("-",arg))
  {
send_to_char(AT_BLOOD,"Error: Invalid argument!\n\r",ch); 
return; 
  }

 wager   = atoi(arg);

 if(wager > 50000 || wager < 1)
  {
send_to_char(AT_BLOOD,"Wager range is between 1 and 1000\n\r",ch);
return; 
  }

/*== make sure they have the cash ==*/
 if(wager > ch->gold)
  {
send_to_char(AT_BLOOD,"You don't have that much gold to wager!\n\r",ch); 
return; 
  }

/*== now set the info ==*/
ch->gladiator = fighter;
ch->pcdata->plr_wager = wager;
sprintf(buf,"You have placed a %d gold wager on %s\n\r",wager,fighter->name);
send_to_char(AT_BLOOD,buf,ch);
return; 
}
