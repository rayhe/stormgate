/*
 * The Mythran Mud Economy Snippet Version 2 (used to be banking.c)
 *
 * Copyrights and rules for using the economy system:
 *
 *	The Mythran Mud Economy system was written by The Maniac, it was
 *	loosly based on the rather simple 'Ack!'s banking system'
 *
 *	If you use this code you must follow these rules.
 *		-Keep all the credits in the code.
 *		-Mail Maniac (v942346@si.hhs.nl) to say you use the code
 *		-Send a bug report, if you find 'it'
 *		-Credit me somewhere in your mud.
 *		-Follow the envy/merc/diku license
 *		-If you want to: send me some of your code
 *
 * All my snippets can be found on http://www.hhs.nl/~v942346/snippets.html
 * Check it often because it's growing rapidly	-- Maniac --
 */

/*$Id: economy.c,v 1.3 2005/02/22 23:55:16 ahsile Exp $*/

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


int	share_value = SHARE_VALUE;	/* External share_value by Maniac */

/*
 * External functions.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort,
                        bool fShowNothing );
int obj_invcount    args( ( OBJ_DATA* obj, bool one_item ) );


void do_bank( CHAR_DATA *ch, char *argument )
{
	/* The Mythran mud economy system (bank and trading)
	*
	* based on:
	* Simple banking system. by -- Stephen --
	*
	* The following changes and additions where
	* made by the Maniac from Mythran Mud
	* (v942346@si.hhs.nl)
	*
	* History:
	* 18/05/96:	Added the transfer option, enables chars to transfer
	*		money from their account to other players' accounts
        * 18/05/96:	Big bug detected, can deposit/withdraw/transfer
	*		negative amounts (nice way to steal is
	*		bank transfer -(lots of dogh) <some rich player>
	*		Fixed it (thought this was better... -= Maniac =-)
	* 21/06/96:	Fixed a bug in transfer (transfer to MOBS)
	*		Moved balance from ch->balance to ch->pcdata->balance
	* 21/06/96:	Started on the invest option, so players can invest
	*		money in shares, using buy, sell and check
	*		Finished version 1.0 releasing it monday 24/06/96
	* 24/06/96:	Mythran Mud Economy System V1.0 released by Maniac
	*
	*/
    
	CHAR_DATA *mob;
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	if ( IS_NPC( ch ) )
	{
		send_to_char(AT_WHITE, "Banking Services are only available to players!\n\r", ch );
		return;
	}
  
	/* Check for mob with act->banker */
	for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
	{
		if ( IS_NPC(mob) && IS_SET(mob->act, ACT_BANKER ) )
			break;
	}
 
	if ( mob == NULL )
	{
		send_to_char(AT_WHITE, "You can't do that here.\n\r", ch );
		return;
	}

	if ( argument[0] == '\0' )
	{
		send_to_char(AT_WHITE, "Bank Options:\n\r\n\r", ch );
		send_to_char(AT_WHITE, "Bank balance: Displays your balance.\n\r", ch );
		send_to_char(AT_WHITE, "Bank deposit <amount>: Deposit gold into your account.\n\r", ch );
		send_to_char(AT_WHITE, "Bank withdraw <amount>: Withdraw gold from your account.\n\r", ch );
		send_to_char(AT_WHITE, "Bank retrieve <item>: Retrieve a stored item from the bank.\n\r", ch );
		send_to_char(AT_WHITE, "Bank store <item>: Store an item in the bank.\n\r", ch );
#if defined BANK_TRANSFER
		send_to_char(AT_WHITE, "Bank transfer <amount> <player>: Transfer <amount> gold to <player> account.\n\r", ch); 
#endif
#if defined BANK_INVEST
		send_to_char(AT_WHITE, "Bank check: check the current shares price.\n\r", ch );
		send_to_char(AT_WHITE, "Bank buy <amount>: Buy <amount> shares.\n\r", ch );
		send_to_char(AT_WHITE, "Bank sell <amount>: Sell <amount> shares.\n\r", ch );
#endif

		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
   
	/* Now work out what to do... */
	if ( !str_prefix( arg1, "balance" ) )
	{
		sprintf(buf,"Your current balance is: %d GP.", ch->pcdata->bankaccount );
		send_to_char(AT_WHITE, buf, ch);
		return;
	}

        if ( !str_prefix( arg1, "deposit" ) )
        {
                int amount;

                if ( is_number ( arg2 ) )
                {
                        amount = atoi( arg2 );
                        if (amount > ch->gold )
                        {
                                sprintf( buf, "How can you deposit %d GP when you only have %d?", amount, ch->gold );
                                do_say(mob, buf );
                                return;
                        }

                        if (amount < 0 )
                        {
                                do_say (mob, "Only positive amounts allowed...");
                                return;
                        }

                        ch->gold -= amount;
                        ch->pcdata->bankaccount += amount;
                        sprintf ( buf, "You deposit %d GP.  Your new balance is %d GP.\n\r",
                        amount, ch->pcdata->bankaccount );
                        send_to_char(AT_WHITE, buf, ch );
                        do_save( ch, "" );
                        return;
                }
        }

	/* We only allow transfers if this is true... so define it... */

#if defined BANK_TRANSFER
	if ( !str_prefix( arg1, "transfer" ) )
	{
		int amount;
		CHAR_DATA *victim;

		if ( is_number ( arg2 ) )
		{
			amount = atoi( arg2 );
			if ( amount > ch->pcdata->bankaccount )
			{
				sprintf( buf, "How can you transfer %d GP when your balance is %d?",
				amount, ch->pcdata->bankaccount );
				send_to_char(AT_WHITE, buf, ch);
				return;
			}

                        if (amount < 0 )
                        {
				send_to_char(AT_WHITE, "Only positive amounts allowed...", ch);
                                return;
                        }


			if ( !( victim = get_char_world( ch, argument ) ) )
			{
				sprintf (buf, "%s doesn't have a bank account.", argument );
				send_to_char(AT_WHITE, buf, ch);
				return;
			}

			if (IS_NPC(victim))
			{
				send_to_char(AT_WHITE, "You can only transfer money to players.", ch);
				return;
			}

			ch->pcdata->bankaccount     -= amount;
 			victim->pcdata->bankaccount += amount;
			sprintf( buf, "You transfer %d GP. Your new balance is %d GP.\n\r",
			amount, ch->pcdata->bankaccount );
			send_to_char(AT_WHITE, buf, ch );
			sprintf (buf, "[BANK] %s has transferred %d gold's to your account.\n\r", ch->name, amount);
			send_to_char(AT_WHITE, buf, victim );
			do_save( ch, "" );
			do_save( victim, "");
			return;
		}
	}
#endif

        if ( !str_prefix( arg1, "withdraw" ) )
        {
                int amount;

                if ( is_number ( arg2 ) )
                {
                        amount = atoi( arg2 );
                        if ( amount > ch->pcdata->bankaccount )
                        {
                                sprintf( buf, "How can you withdraw %d GP when your balance is %d?",
                                amount, ch->pcdata->bankaccount );
                                do_say (mob, buf );
                                return;
                        }

                        if (amount < 0 )
                        {
                                do_say( mob, "Only positive amounts allowed...");
                                return;
                        }

                        ch->pcdata->bankaccount -= amount;
                        ch->gold += amount;
                        sprintf( buf, "You withdraw %d GP.  Your new balance is %d GP.", amount, ch->pcdata->bankaccount );
                        send_to_char(AT_WHITE, buf, ch );
                        do_save( ch, "" );
                        return;
                }
        }



	if ( !str_prefix( arg1, "store" ) )
	{
              OBJ_DATA *obj;
              int store_cost = 0; /* temp variable for storage costs... don't want stuff to change on us - Ahsile */
	      
              if ( !str_prefix( arg2, " " ) )
		  {
		    send_to_char( AT_WHITE, "Your storage box contains:\n\r", ch );
		    show_list_to_char( ch->pcdata->storage, ch, TRUE, TRUE );
		    return;
		  }

	      if ( !( obj = get_obj_carry( ch, arg2 ) ) )
		  {
		    send_to_char( AT_WHITE, "You are not carrying that item.\n\r", ch );
		    return;
		  }

              /* Check container object count - Ahsile */
              if ( (ch->pcdata->storcount + obj_invcount( obj, TRUE ) ) >= (ch->level * 2 ) )
		  {
                    sprintf(buf,"%s%d%s","You may only have ",(ch->level * 2)," items in your storage box at your level.\n\r");
		    send_to_char( AT_WHITE, buf , ch );
		    return;
                  }

	      if ( obj->item_type == ITEM_KEY ) 
		  {
		    send_to_char( AT_WHITE,
			 "You can't store that type of item.\n\r",
		   	 ch );
		    return;	
		  }

              /* Storage cost based on share prices - Ahsile */
              #if defined BANK_INVEST
                 store_cost = (share_value + 100) * obj_invcount( obj, TRUE );
              #else
                 store_cost = 200 * obj_invcount( obj, TRUE );
              #endif

	      if ( ch->pcdata->bankaccount < store_cost )
		  {
                    sprintf(buf,"%s%d%s","Storing costs ", store_cost ,"gp, which you do not have in your bank account.\n\r");
		    send_to_char( AT_WHITE, buf, ch );
		    return;
		  }

		  ch->pcdata->bankaccount -= store_cost;

		  oprog_store_trigger( obj, ch );

		  obj_from_char( obj );
		  obj_to_storage( obj, ch );
                  sprintf(buf,"%s%d%s","The bank deducts ",store_cost,"gp from your account.\n\r");
		  send_to_char( AT_WHITE, buf, ch );
	          do_save( ch, "" );
		  return;
 		}

	if ( !str_prefix( arg1, "retrieve" ) )
	{
              OBJ_DATA *obj;

	       if ( !str_prefix( arg2, " " ) )
		  {
		    send_to_char( AT_WHITE, "Retrieve what?\n\r", ch );
		    return;
		  }

		  if ( !( obj = get_obj_storage( ch, arg2 ) ) )
		  {
		    send_to_char(AT_WHITE, "You do not have that object in storage.\n\r", ch);
	            send_to_char(AT_WHITE, "Use 'bank store' to see what you have in storage.\n\r",ch);
   	            return;
		  }

                  if ( obj_invcount( obj, TRUE ) + ch->carry_number > can_carry_n( ch ) )
                  {
                    send_to_char(AT_WHITE, "You will be carrying too many items!",ch);
                    return;
                  }

         	  obj_from_storage( obj );
		  obj_to_char( obj, ch );
		  oprog_retrieve_trigger( obj, ch );
		  send_to_char( AT_WHITE, "You retrieve it from storage.\n\r", ch );
	          do_save( ch, "" );
		  return;
 		}

	/* If you want to have an invest option... define BANK_INVEST */

#if defined BANK_INVEST
        if ( !str_prefix( arg1, "buy" ) )
        {
                int amount;
		if (share_value < 1)
		{
			send_to_char(AT_WHITE, "There is something wrong with shares, notify the GODS.", ch);
			return;
		}

                if ( is_number ( arg2 ) )
                {
                        amount = atoi( arg2 );
                        if ( (amount * share_value) > ch->pcdata->bankaccount )
                        {
                                sprintf( buf, "%d shares will cost you %d, get more money.", amount, (amount * share_value) );
				send_to_char(AT_WHITE, buf, ch);
                                return;
                        }

                        if (amount < 0 )
                        {
				send_to_char(AT_WHITE, "If you want to sell shares you have to say so...", ch);
                                return;
                        }
			if( ( amount + ch->pcdata->shares ) > 500000 )
			{
				send_to_char(AT_WHITE, "You can only have 500000 shares.\n\r", ch );
				return;
			}
                        ch->pcdata->bankaccount -= (amount * share_value);
                        ch->pcdata->shares  += amount;
                        sprintf( buf, "You buy %d shares for %d GP, you now have %d shares.\n\r", amount, (amount * share_value), ch->pcdata->shares );
			send_to_char(AT_WHITE, buf, ch);
                        do_save( ch, "" );
                        return;
                }
        }

        if ( !str_prefix( arg1, "sell" ) )
        {
                int amount;

		if (share_value < 1)
		{
			send_to_char(AT_WHITE, "There is something wrong with the shares, notify the GODS.", ch);
			return;
		}

                if ( is_number ( arg2 ) )
                {
                        amount = atoi( arg2 );
                        if ( amount > ch->pcdata->shares )
                        {
                                sprintf( buf, "You only have %d shares.", ch->pcdata->shares );
				send_to_char(AT_WHITE, buf, ch);
                                return;
                        }

                        if (amount < 0 )
                        {
				send_to_char(AT_WHITE, "If you want to buy shares you have to say so...", ch);
                                return;
                        }
			if( ( ( amount * share_value ) + ch->pcdata->bankaccount ) > 2000000000 )
			{
				send_to_char(AT_WHITE, "Your bank account can not handle that much gold.\n\r", ch );
				return;
			}
                        ch->pcdata->bankaccount += (amount * share_value);
                        ch->pcdata->shares  -= amount;
                        sprintf( buf, "You sell %d shares for %d GP, you now have %d shares.\n\r", amount, (amount * share_value), ch->pcdata->shares );
			send_to_char(AT_WHITE, buf, ch);
                        do_save( ch, "" );
                        return;
                }
        }

        if ( !str_prefix( arg1, "check" ) )
        {
		sprintf (buf, "The current shareprice is %d.",share_value);
		send_to_char(AT_WHITE, buf, ch);
		if (ch->pcdata->shares)
		{
		    sprintf (buf, "  You have %d shares, (%d a share) worth a total of %d gold.",
			ch->pcdata->shares, share_value, (ch->pcdata->shares * share_value) );
		    send_to_char(AT_WHITE, buf, ch);
		}
		send_to_char(AT_WHITE, "\n\r", ch );
                return;
        }
#endif

	send_to_char(AT_WHITE, "I don't know what you mean.\n\r", ch);
	do_bank( ch, "" );		/* Generate Instructions */
	return;
}
