/*$Id: healer.c,v 1.3 2005/02/22 23:55:17 ahsile Exp $*/

/* Healer code written for Merc 2.0 muds by Alander 
   direct questions or comments to rtaylor@cie-2.uoregon.edu
   any use of this code must include this header */

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
//#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include "merc.h"

void do_heal(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int cost,sn;
    SPELL_FUN *spell;
    char *words;	

    /* check for healer */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_HEALER) )
            break;
    }
 
    if ( mob == NULL )
    {
        send_to_char(C_DEFAULT, "You can't do that here.\n\r", ch );
        return;
    }

    if ( mob->fighting != NULL )
    {
	act(AT_LBLUE,"$N says 'Can't you see i'm busy !!!'", ch, NULL, mob, TO_CHAR);
	return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        /* display price list */
        act(AT_LBLUE,"$N says 'I offer the following spells:'",ch,NULL,mob,TO_CHAR);
        send_to_char(AT_WHITE,"  light: cure light wounds      5 gold\n\r",ch);
        send_to_char(AT_WHITE,"  serious: cure serious wounds  8 gold\n\r",ch);
        send_to_char(AT_WHITE,"  critic: cure critical wounds 10 gold\n\r",ch);
        send_to_char(AT_WHITE,"  heal: healing spell          25 gold\n\r",ch);
        send_to_char(AT_WHITE,"  blind: cure blindness        15 gold\n\r",ch);
        send_to_char(AT_WHITE,"  poison:  cure poison         20 gold\n\r",ch);
        send_to_char(AT_WHITE,"  uncurse: remove curse        15 gold\n\r",ch);
        send_to_char(AT_WHITE,"  mana:  restore mana          10 gold\n\r",ch);
        send_to_char(AT_WHITE,"  resurrection: become whole    0 gold\n\r",ch);
        send_to_char(AT_WHITE," Type heal <type> to be healed.\n\r",ch);
        return;
	}

    switch (arg[0])
    {
        case 'l' :
            spell = spell_cure_light;
            sn    = skill_lookup("cure light");
            words = "mani";
            cost  = 5;
            break;

        case 's' :
            spell = spell_cure_serious;
            sn    = skill_lookup("cure serious");
            words = "mani";
            cost  = 8;
            break;

        case 'c' :
            spell = spell_cure_critical;
            sn    = skill_lookup("cure critical");
            words = "mani";
            cost  = 10;
            break;

        case 'h' :
            spell = spell_heal;
            sn = skill_lookup("heal");
            words = "vas mani";
            cost  = 25;
            break;

        case 'b' :
            spell = spell_cure_blindness;
            sn    = skill_lookup("cure blindness");
            words = "an mani";
            cost  = 15;
            break;

        case 'p' :
            spell = spell_cure_poison;
            sn    = skill_lookup("cure poison");
            words = "an nox";
            cost  = 20;
            break;

        case 'u' :
            spell = spell_remove_curse;
            sn    = skill_lookup("remove curse");
            words = "an rel";
            cost  = 15;
            break;

        case 'r' :
            spell =  spell_resurrection;
            sn    = skill_lookup("resurrection");
            words = "in mani corp";
            cost  = 0;
            break;

        case 'm' :
            spell = spell_mana;
            sn = skill_lookup("mana");
            words = "in grav";
            cost = 10;
            break;

	default :
	    act(AT_LBLUE,"$N says 'Type 'heal' for a list of spells.'",
	        ch,NULL,mob,TO_CHAR);
	    return;
    }

    if (cost > ch->gold)
    {
	act(AT_LBLUE,"$N says 'You do not have enough gold for my services.'",
	    ch,NULL,mob,TO_CHAR);
	return;
    }

    WAIT_STATE(ch,PULSE_VIOLENCE);

    ch->gold -= cost;
    act(C_DEFAULT,"$n utters the words '$T'.",mob,NULL,words,TO_ROOM);

    if (spell == NULL)  /* restore mana trap...kinda hackish */
    {
	ch->mana += dice(2,8) + mob->level / 4;
	ch->mana = UMIN(ch->mana,ch->max_mana);
	send_to_char(AT_BLUE,"A warm glow passes through you.\n\r",ch);
	return;
     }

     if (sn == -1)
	return;
    
     spell(sn,mob->level,mob,ch);
}

void do_clan_heal(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int cost,sn;
    SPELL_FUN *spell;
    char *words;	
    int wait;

    /* check for healer */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && IS_SET(mob->act, ACT_IS_CLAN_HEALER) )
            break;
    }
 
    if ( mob == NULL )
    {
        send_to_char(C_DEFAULT, "You can't do that here.\n\r", ch );
        return;
    }

    if ( mob->fighting != NULL )
    {
	act(AT_LBLUE,"$N says 'Can't you see i'm busy !!!'", ch, NULL, mob, TO_CHAR);
	return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        /* display price list */
        act(AT_LBLUE,"$N says 'I offer the following spells:'",ch,NULL,mob,TO_CHAR);
        send_to_char(AT_WHITE,"  light:     cure light wounds       5 gold\n\r",ch);
        send_to_char(AT_WHITE,"  serious:   cure serious wounds     8 gold\n\r",ch);
        send_to_char(AT_WHITE,"  critic:    cure critical wounds   10 gold\n\r",ch);
        send_to_char(AT_WHITE,"  heal:      healing spell          25 gold\n\r",ch);
        send_to_char(AT_WHITE,"  blind:     cure blindness         15 gold\n\r",ch);
        send_to_char(AT_WHITE,"  poison:    cure poison            20 gold\n\r",ch);
        send_to_char(AT_WHITE,"  uncurse:   remove curse           15 gold\n\r",ch);
        send_to_char(AT_WHITE,"  mana:      restore mana           10 gold\n\r",ch);
	send_to_char(AT_WHITE,"  fly:       fly                     3 gold\n\r",ch);
        send_to_char(AT_WHITE,"  giant:     giant strength          5 gold\n\r",ch);
        send_to_char(AT_WHITE,"  invis:     invis                   5 gold\n\r",ch);
        send_to_char(AT_WHITE,"  resurrection: become whole         0 gold\n\r",ch);
	send_to_char(AT_WHITE,"\n\r",ch);
        send_to_char(AT_WHITE," Type clanheal <type> to be healed.\n\r",ch);
        return;
	}

    switch (arg[0])
    {
        case 'l' :
            spell = spell_cure_light;
            sn    = skill_lookup("cure light");
            words = "mani";
            cost  = 5;
	    wait  = 1;
            break;

	case 'f' :
	    spell = spell_fly;
	    sn    = skill_lookup("fly");
	    words = "uus ylem";
	    cost  = 3;
	    wait  = 1;
	    break;

        case 'g' :
	    spell = spell_giant_strength;
	    sn    = skill_lookup("giant strength");
	    words = "in vas grav";
	    cost  = 5;
	    wait  = 1;
	    break;

	case 'i' :
	    spell = spell_invis;
	    sn    = skill_lookup("invis");
	    words = "sanct lor";
	    cost  = 5;
	    wait  = 1;
	    break;

        case 's' :
            spell = spell_cure_serious;
            sn    = skill_lookup("cure serious");
            words = "mani";
            cost  = 8;
	    wait  = 1;
            break;

        case 'c' :
            spell = spell_cure_critical;
            sn    = skill_lookup("cure critical");
            words = "mani";
            cost  = 10;
	    wait  = 1;
            break;

        case 'h' :
            spell = spell_heal;
            sn = skill_lookup("heal");
            words = "vas mani";
            cost  = 25;
	    wait  = 1;
            break;

        case 'b' :
            spell = spell_cure_blindness;
            sn    = skill_lookup("cure blindness");
            words = "an mani";
            cost  = 15;
	    wait  = 1;
            break;

        case 'p' :
            spell = spell_cure_poison;
            sn    = skill_lookup("cure poison");
            words = "an nox";
            cost  = 20;
	    wait  = 1;
            break;

        case 'u' :
            spell = spell_remove_curse;
            sn    = skill_lookup("remove curse");
            words = "an rel";
            cost  = 15;
	    wait  = 1;
            break;

        case 'r' :
            spell =  spell_resurrection;
            sn    = skill_lookup("resurrection");
            words = "in mani corp";
            cost  = 0;
            wait = 0;
            break;

        case 'm' :
            spell = spell_mana;
            sn = skill_lookup("mana");
            words = "in grav";
            cost = 10;
	    wait = 0;
            break;

	default :
	    act(AT_LBLUE,"$N says 'Type 'heal' for a list of spells.'",
	        ch,NULL,mob,TO_CHAR);
	    return;
    }

    if (cost > ch->gold)
    {
	act(AT_LBLUE,"$N says 'You do not have enough gold for my services.'",
	    ch,NULL,mob,TO_CHAR);
	return;
    }

    if( wait == 1)
    {
   	WAIT_STATE(ch,PULSE_VIOLENCE);
    }

    ch->gold -= cost;
    act(C_DEFAULT,"$n utters the words '$T'.",mob,NULL,words,TO_ROOM);

    if( sn == -1)
	return;

    spell(sn,mob->level,mob,ch);
}

