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

/*$Id: interp.c,v 1.51 2005/04/10 16:29:00 tyrion Exp $*/

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
#ifndef RUN_AS_WIN32SERVICE
#include <unistd.h>
#endif
#include "merc.h"



bool	check_social	args( ( CHAR_DATA *ch, char *command, char *argument ) );
bool    check_alias     args( ( CHAR_DATA *ch, char *command, char *argument ) );
void    add_alias   	args( ( CHAR_DATA *ch, ALIAS_DATA *pAl, char *old, char *_new ) );
bool    check_disabled  args( ( CHAR_DATA *ch, const struct cmd_type *command ) );

bool	 can_mob_use( CHAR_DATA *ch, char *argument);

DISABLED_DATA *disabled_first;

#define END_MARKER      "END"  /* for load_disabled && save_disabled */


/*
 * Command logging types.
 */
#define LOG_NORMAL              0
#define LOG_ALWAYS              1
#define LOG_NEVER               2
/*
 * Altrag was here.
 */
#define LOG_BUILD               3


/*
 * God Levels - Check them out in merc.h
*/

#define L_HER                   LEVEL_HERO

/*
 * Log-all switch.
 */
bool				fLogAll		    = FALSE;



/*
 * Command table.
 */
struct	cmd_type	cmd_table	[ ] =
{
    /*
     * Common movement commands.
     */
    { "north",		do_north,	POS_GHOST,	 0,  LOG_NORMAL	},
    { "east",		do_east,	POS_GHOST,	 0,  LOG_NORMAL	},
    { "south",		do_south,	POS_GHOST,	 0,  LOG_NORMAL	},
    { "west",		do_west,	POS_GHOST,	 0,  LOG_NORMAL },
    { "up",		do_up,		POS_GHOST,	 0,  LOG_NORMAL },
    { "down",		do_down,	POS_GHOST,	 0,  LOG_NORMAL },

/* XOR */
    { "push",		do_push,	POS_STANDING,	 0,  LOG_NORMAL },
    { "drag",		do_drag,	POS_STANDING,	 0,  LOG_NORMAL },

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    { "buy",		do_buy,		POS_STANDING,	 0,  LOG_NORMAL	},
    { "cast",		do_cast,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "qb",		do_quickburst,  POS_FIGHTING,    0,  LOG_NORMAL },
    { "quickburst",	do_quickburst,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "use",		do_use,		POS_FIGHTING,    0,  LOG_NORMAL },
    { "dual",           do_dual,        POS_FIGHTING,	 0,  LOG_NORMAL },
    { "exits",		do_exits,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "get",		do_get,		POS_STANDING,	 0,  LOG_NORMAL	},
    { "inventory",	do_inventory,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "invoke",         do_invoke,      POS_FIGHTING,    0,  LOG_NORMAL },
    { "kill",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "look",		do_look,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "order",		do_order,	POS_RESTING,	 0,  LOG_ALWAYS	},
    { "rest",		do_rest,	POS_SLEEPING,	 0,  LOG_NORMAL	},
    { "repair",         do_repair,      POS_STANDING,    0,  LOG_NORMAL },
    { "sleep",		do_sleep,	POS_SLEEPING,	 0,  LOG_NORMAL	},
    { "stand",		do_stand,	POS_SLEEPING,	 0,  LOG_NORMAL	},
    { "tell",		do_tell,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "wield",		do_wear,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "wizhelp",	do_wizhelp,	POS_DEAD,        0,  LOG_NORMAL	},
    { "multiburst",     do_multiburst,  POS_FIGHTING,    0,  LOG_NORMAL },
    { "mb",             do_multiburst,  POS_FIGHTING,    0,  LOG_NORMAL },

    { "mpstat",         do_mpstat,      POS_DEAD,      L_APP,  LOG_NORMAL },
    { "mpcommands",     do_mpcommands,  POS_DEAD,      L_APP,  LOG_NORMAL },
    { "mpasound",       do_mpasound,    POS_DEAD,        0,  LOG_NORMAL },
    { "mpjunk",         do_mpjunk,      POS_DEAD,        0,  LOG_NORMAL },
    { "mpastatus",	do_mpastatus,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "mpdisable",      do_mpdisable,   POS_DEAD,        0,  LOG_NORMAL },
    { "mpenable",       do_mpenable,    POS_DEAD,        0,  LOG_NORMAL },
    { "mpecho",         do_mpecho,      POS_DEAD,        0,  LOG_NORMAL },
    { "mpechoat",       do_mpechoat,    POS_DEAD,        0,  LOG_NORMAL },
    { "mpechoaround",   do_mpechoaround,POS_DEAD,        0,  LOG_NORMAL },
    { "mpkill",         do_mpkill      ,POS_DEAD,        0,  LOG_NORMAL },
    { "mpmload",        do_mpmload     ,POS_DEAD,        0,  LOG_NORMAL },
    { "mpoload",        do_mpoload     ,POS_DEAD,        0,  LOG_NORMAL },
    { "mppurge",        do_mppurge     ,POS_DEAD,        0,  LOG_NORMAL },
    { "mpgoto",         do_mpgoto      ,POS_DEAD,        0,  LOG_NORMAL },
    { "mpat",           do_mpat        ,POS_DEAD,        0,  LOG_NORMAL },
    { "mptransfer",     do_mptransfer  ,POS_DEAD,        0,  LOG_NORMAL },
    { "mpforce",        do_mpforce     ,POS_DEAD,        0,  LOG_NORMAL },
    { "mpteleport",     do_mpteleport,  POS_DEAD,        0,  LOG_NORMAL },
    { "mpgainxp",       do_mpgainxp,    POS_DEAD,        0,  LOG_NORMAL },
    { "mpgainqp",       do_mpgainqp,    POS_DEAD,        0,  LOG_NORMAL },

    /*
     * Informational commands.
     */
    { "affected",       do_affectedby,  POS_RESTING,     0,  LOG_NORMAL },
    { "areas",		do_areas,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "bug",		do_bug,		POS_DEAD,	 0,  LOG_NORMAL	},
    { "commands",	do_commands,	POS_DEAD,	 0,  LOG_NORMAL },
    { "compare",	do_compare,	POS_STANDING,	 0,  LOG_NORMAL },
    { "consider",	do_consider,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "credits",	do_credits,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "enter",          do_enter,       POS_GHOST,    0,  LOG_NORMAL },
    { "equipment",	do_equipment,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "examine",	do_examine,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "finger",         do_finger,      POS_DEAD,      L_DIR,  LOG_ALWAYS },
    { "hlist",          do_hlist,       POS_DEAD,      L_APP,  LOG_NORMAL },
    { "help",		do_help,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "idea",		do_idea,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "report",		do_report,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "pagelength",     do_pagelen,     POS_DEAD,        0,  LOG_NORMAL },
    { "read",		do_look,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "score",		do_score,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "slist",          do_slist,       POS_DEAD,        0,  LOG_NORMAL },
    { "socials",	do_socials,	POS_DEAD,	 0,  LOG_NORMAL },
    { "time",		do_time,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "typo",		do_typo,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "weather",	do_weather,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "worth",          do_worth,       POS_DEAD,        0,  LOG_NORMAL },
    { "who",		do_who,		POS_DEAD,	 0,  LOG_NORMAL	},
    { "wizlist",        do_wizlist,     POS_DEAD,        0,  LOG_NORMAL },

    /*
     * Clan commands.
     */
    { "smash",         do_smash,        POS_STANDING,   30,  LOG_NORMAL },
    { "induct",        do_induct,       POS_STANDING,    0,  LOG_ALWAYS },
    { "outcast",       do_outcast,      POS_STANDING,    0,  LOG_ALWAYS },
    { "setlev",        do_setlev,       POS_STANDING,    0,  LOG_ALWAYS },
    { "cinfo",         do_cinfo,        POS_DEAD,        0,  LOG_NORMAL },
    { "clanview",      do_clanview,	POS_SLEEPING,   30,  LOG_NORMAL },
    { "clanheal",      do_clan_heal,	POS_STANDING,   30,  LOG_NORMAL },
    { "utopian healing",do_utopian_healing,POS_STANDING, 30, LOG_NORMAL },

    /*
     * Guild commands
     */
    { "guild",		do_guild,	POS_DEAD,	 0,  LOG_ALWAYS },
    { "unguild",	do_unguild,	POS_DEAD,	 0,  LOG_ALWAYS	},
    { "setrank",	do_setrank,	POS_DEAD,	 0,  LOG_ALWAYS	},
    { "gdt",		do_gdt,		POS_SLEEPING,	 0,  LOG_NORMAL },
    { "guildtalk",	do_gdt,		POS_SLEEPING,	 0,  LOG_NORMAL },

    /*
     * Configuration commands.
     */
    { "alias",          do_alias,       POS_DEAD,        0,  LOG_NORMAL },
    { "auto",           do_auto,        POS_DEAD,        0,  LOG_NORMAL },
    { "autoexit",       do_autoexit,    POS_DEAD,        0,  LOG_NORMAL },
    { "autoloot",       do_autoloot,    POS_DEAD,        0,  LOG_NORMAL },
    { "autogold",       do_autogold,    POS_DEAD,        0,  LOG_NORMAL },
    { "autosac",        do_autosac,     POS_DEAD,        0,  LOG_NORMAL },
    { "blank",          do_blank,       POS_DEAD,        0,  LOG_NORMAL },
    { "brief",          do_brief,       POS_DEAD,        0,  LOG_NORMAL },
    { "channels",	do_channels,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "combine",        do_combine,     POS_DEAD,        0,  LOG_NORMAL },
    { "config",		do_config,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "description",	do_description,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "password",	do_password,	POS_DEAD,	 0,  LOG_NEVER	},
    { "speak",          do_speak,       POS_RESTING,     0,  LOG_NORMAL },
    { "learn",          do_learn,       POS_STANDING,    0,  LOG_NORMAL },
    { "sedit",          do_sedit,       POS_DEAD,      L_IMM, LOG_ALWAYS },
    { "prompt",         do_prompt,      POS_DEAD,        0,  LOG_NORMAL },
    { "title",		do_title,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "wimpy",		do_wimpy,	POS_STANDING,    0,  LOG_NORMAL	},
    { "countcommands",  do_countcommands,POS_DEAD,     L_CON,  LOG_NORMAL },
    { "timequake",	do_timequake,   POS_DEAD,      L_OVD, LOG_ALWAYS },
    { "recall",		do_noob_recall,	POS_GHOST,	 0,  LOG_NORMAL },
    /*
     * Communication commands.
     */
    { "answer",		do_answer,	POS_SLEEPING,	 0,  LOG_NORMAL },
    { "auction",	do_auction,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "bid",            do_bid,         POS_STANDING,    0,  LOG_ALWAYS },
    { "chat",		do_chat,	POS_SLEEPING,	 0,  LOG_NORMAL	},
    { "mutter",		do_mutter,	POS_SLEEPING,    0,  LOG_NORMAL },
    { "<",		do_mutter,	POS_SLEEPING,	 0,  LOG_NORMAL },
    { ".",		do_chat,	POS_SLEEPING,	 0,  LOG_NORMAL	},
    { "emote",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL	},
    { ",",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "gtell",		do_gtell,	POS_DEAD,	 0,  LOG_NORMAL	},
    { ";",		do_gtell,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "music",		do_music,	POS_SLEEPING,	 0,  LOG_NORMAL },
    { "note",		do_note,	POS_SLEEPING,	 0,  LOG_NORMAL	},
    { "pose",		do_pose,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "quest",          do_quest,       POS_STANDING,    0,  LOG_NORMAL },
    { "question",	do_question,	POS_SLEEPING,	 0,  LOG_NORMAL },
    { "reply",		do_reply,	POS_SLEEPING,	 0,  LOG_NORMAL },
    { "say",		do_say,		POS_RESTING,	 0,  LOG_NORMAL	},
    { "'",		do_say,		POS_RESTING,	 0,  LOG_NORMAL	},
    { "shout",		do_shout,	POS_RESTING,	 3,  LOG_NORMAL	},
    { "yell",		do_yell,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "clantalk",       do_clan,        POS_DEAD,        0,  LOG_NORMAL },
    { "class",          do_class,       POS_DEAD,        0,  LOG_NORMAL },
    { ">",              do_class,       POS_DEAD,        0,  LOG_NORMAL },
    { "guard",          do_guard,       POS_DEAD,      L_DIR,  LOG_NORMAL },
    { "[",		do_guard,	POS_DEAD,      L_DIR,  LOG_NORMAL },
    { "timelord",	do_timelord,	POS_DEAD,      L_CON,  LOG_NORMAL },
    { "stormlord",      do_timelord,    POS_DEAD,      L_CON,  LOG_NORMAL },
    { "=",              do_timelord,    POS_DEAD,      L_CON,  LOG_NORMAL },
    { "tl",		do_timelord,	POS_DEAD,      L_CON,  LOG_NORMAL },
    { "ooc", 		do_ooc,		POS_DEAD,	 0,  LOG_NORMAL },
    { "info",		do_info,	POS_DEAD,	 0,  LOG_ALWAYS },
    /*
     * Object manipulation commands.
     */
    { "brandish",	do_brandish,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "close",		do_close,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "drink",		do_drink,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "drop",		do_drop,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "eat",		do_eat,		POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "fill",		do_fill,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "forge",          do_forge,       POS_STANDING,   30,  LOG_ALWAYS }, 
    { "give",		do_give,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "hold",		do_wear,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "join",           do_join,        POS_FIGHTING,	 0,  LOG_NORMAL },
    { "list",		do_list,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "lock",		do_lock,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "open",		do_open,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "patch",          do_patch,       POS_STANDING,    0,  LOG_NORMAL },
    { "put",		do_put,		POS_FIGHTING,	 0,  LOG_NORMAL },
    { "quaff",		do_quaff,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "recite",		do_recite,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "remove",		do_remove,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "search",         do_search,      POS_STANDING,    0,  LOG_NORMAL },
    { "sell",		do_sell,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "take",		do_get,		POS_STANDING,	 0,  LOG_NORMAL	},
    { "sacrifice",	do_sacrifice,	POS_GHOST,	 0,  LOG_NORMAL	},
    { "separate",       do_separate,    POS_FIGHTING,	 0,  LOG_NORMAL },
    { "unlock",		do_unlock,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "value",		do_value,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "wear",		do_wear,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "zap",		do_zap,		POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "fire", 		do_fire,	POS_FIGHTING,    0,  LOG_NORMAL },
    { "stare",         do_stare,       POS_STANDING,	 0,  LOG_NORMAL },
    /*
     * Combat commands.
     */
    { "flee",		do_flee,	POS_FIGHTING,	 0,  LOG_NORMAL },
    { "murde",		do_murde,	POS_FIGHTING,	 5,  LOG_NORMAL	},
    { "murder",		do_murder,	POS_FIGHTING,	 5,  LOG_NORMAL	},
    { "throw",	        do_throw,	POS_STANDING,    0,  LOG_NORMAL }, 
    /*
     * Race Commands *
     */
    { "breathe",	do_breathe_fire, POS_STANDING,	 0,  LOG_NORMAL },
    { "fly",		do_race_fly,	POS_STANDING,	 0,  LOG_NORMAL },
    /*
     * Miscellaneous commands.
     */
    { "afk",            do_afk,         POS_SLEEPING,    0,  LOG_NORMAL },
    { "bet",            do_bet,         POS_STANDING,    0,  LOG_NORMAL },
    { "delet",          do_delet,       POS_DEAD,        0,  LOG_ALWAYS },
    { "delete",         do_delete,      POS_DEAD,        0,  LOG_NORMAL },
    { "account",        do_account,     POS_RESTING,     0,  LOG_NORMAL },
    { "bodybag",        do_bodybag,     POS_DEAD,    L_DEI,  LOG_ALWAYS },
    { "follow",		do_follow,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "lose",           do_lose,        POS_RESTING,     0,  LOG_NORMAL },
    { "group",		do_group,	POS_SLEEPING,	 0,  LOG_NORMAL	},
    { "heal",           do_heal,        POS_RESTING,     0,  LOG_NORMAL },
    { "palm",           do_palm,        POS_STANDING,    0,  LOG_NORMAL },
    { "ban",            do_ban,         POS_DEAD,    L_DIR,  LOG_ALWAYS },
    { "bank",           do_bank,        POS_STANDING,    0, LOG_NORMAL },
    { "practice",	do_practice,	POS_STANDING,	 0,  LOG_NORMAL},
    { "qui",		do_qui,		POS_DEAD,	 0,  LOG_NORMAL	},
    { "quit",		do_quit,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "rent",		do_rent,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "save",		do_save,	POS_DEAD,	 2,  LOG_NORMAL	},
    { "sleep",		do_sleep,	POS_SLEEPING,	 0,  LOG_NORMAL	},
    { "spells",         do_spells2,     POS_SLEEPING,    0,  LOG_NORMAL },
    { "skills",         do_skills,     POS_SLEEPING,    0,  LOG_NORMAL },
    { "split",		do_split,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "steal",		do_steal,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "study",		do_study,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "train",		do_train,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "visible",	do_visible,	POS_SLEEPING,	 0,  LOG_NORMAL },
    { "reality",	do_reality,	POS_SLEEPING,	 0,  LOG_NORMAL },
    { "wake",		do_wake,	POS_SLEEPING,	 0,  LOG_NORMAL	},
    { "where",		do_where,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "pray",           do_pray,        POS_STANDING,    0,  LOG_NORMAL },
    { "mount",		do_mount,	POS_STANDING,	 0,  LOG_NORMAL },
    { "dismount", 	do_dismount,	POS_STANDING,	 0,  LOG_NORMAL },
 /* { "challenge",      do_challenge,  	POS_STANDING, 	 0,  LOG_NORMAL },
    { "accept",		do_accept,	POS_STANDING,    0,  LOG_NORMAL },
    { "decline",	do_decline,  	POS_STANDING,    0,  LOG_NORMAL },*/

    { "remortalize",	do_remortalize, POS_STANDING,	 0,  LOG_ALWAYS },
    { "devote",		do_devote,	POS_STANDING,	 0,  LOG_ALWAYS },
    /*
     * Immortal commands.
     */
    { "vused",          do_vused,       POS_DEAD,    L_DEI,  LOG_NORMAL },
    { "vnum",		do_vnum,	POS_DEAD,    L_DEI,  LOG_NORMAL	},
    { "load",		do_load,	POS_DEAD,    L_IMP,  LOG_NORMAL	},
    { "restrict",       do_restrict,    POS_DEAD,    L_SEN,  LOG_NORMAL },
    { "astatus",	change_area_status, POS_DEAD, L_CON, LOG_ALWAYS },
    { "authorize",	do_authorize,	POS_DEAD,    L_APP,  LOG_ALWAYS },
    { "advance",	do_advance,	POS_DEAD,    L_CON,  LOG_ALWAYS	},
    { "imtlset",        do_imtlset,     POS_DEAD,    L_SEN,  LOG_ALWAYS },
    { "sstime",		do_sstime,	POS_DEAD,    L_IMP,  LOG_ALWAYS },
    { "trust",		do_trust,	POS_DEAD,    L_CON,  LOG_ALWAYS },
    { "dostat",         do_dostat,      POS_DEAD,    L_CON,  LOG_ALWAYS },
    { "allow",		do_allow,	POS_DEAD,    L_SEN,  LOG_ALWAYS	},
    { "deny",		do_deny,	POS_DEAD,    L_DIR,  LOG_ALWAYS	},
    { "disconnect",	do_disconnect,	POS_DEAD,    L_APP,  LOG_ALWAYS	},
    { "force",		do_force,	POS_DEAD,    L_DEI,  LOG_ALWAYS	},
    { "freeze",		do_freeze,	POS_DEAD,    L_DEI,  LOG_ALWAYS	},
    { "fset",           do_fset,        POS_DEAD,    L_OVD,  LOG_ALWAYS },
    { "at",		do_at,		POS_DEAD,    L_APP,  LOG_NORMAL },
    { "atall",		do_for, 	POS_DEAD,    L_OVD,  LOG_ALWAYS	},
    { "log",		do_log,		POS_DEAD,    L_DIR,  LOG_ALWAYS	},
    { "mset",		do_mset,	POS_DEAD,    L_DEI,  LOG_BUILD	},
    { "newcorpse",	do_newcorpse,	POS_DEAD,    L_IMM,  LOG_ALWAYS },
    { "lset",		do_lset,	POS_DEAD,    L_DEI,  LOG_BUILD	},
    { "noemote",	do_noemote,	POS_DEAD,    L_DEI,  LOG_NORMAL },
    { "notell",		do_notell,	POS_DEAD,    L_DEI,  LOG_NORMAL	},
    { "notestat",	do_notestat,	POS_DEAD,    L_DEI,  LOG_NORMAL	},
    { "numlock",	do_numlock,	POS_DEAD,    L_CON,  LOG_ALWAYS	},
    { "oload",		do_oload,	POS_DEAD,    L_APP,  LOG_BUILD	},
    { "oset",		do_oset,	POS_DEAD,    L_IMM,  LOG_BUILD	},
    { "pardon",		do_pardon,	POS_DEAD,    L_IMP,  LOG_ALWAYS	},
    { "purge",		do_purge,	POS_DEAD,    L_APP,  LOG_NORMAL	},
    { "reboo",		do_reboo,	POS_DEAD,    L_ARC,  LOG_NORMAL	},
    { "reboot",		do_reboot,	POS_DEAD,    L_ARC,  LOG_ALWAYS	},
    { "restore",	do_restore,	POS_DEAD,    L_DEI,  LOG_ALWAYS	},
    { "shutdow",	do_shutdow,	POS_DEAD,    L_KPR,  LOG_NORMAL	},
    { "shutdown",	do_shutdown,	POS_DEAD,    L_KPR,  LOG_ALWAYS	},
    { "silence",	do_silence,	POS_DEAD,    L_DEI,  LOG_ALWAYS },
    { "set",            do_set,         POS_DEAD,    L_IMP,  LOG_ALWAYS },
    { "sla",		do_sla,		POS_DEAD,    L_DEI,  LOG_NORMAL	},
    { "slay",		do_slay,	POS_DEAD,    L_DEI,  LOG_ALWAYS	},
    { "sset",		do_sset,	POS_DEAD,    L_OVD,  LOG_ALWAYS },
    { "sstat",          do_sstat,       POS_DEAD,    L_APP,  LOG_NORMAL },
    { "stat",           do_stat,        POS_DEAD,    L_IMP,  LOG_ALWAYS },
    { "string",         do_string,      POS_DEAD,    L_APP,  LOG_ALWAYS },
    { "transfer",	do_transfer,	POS_DEAD,    L_APP,  LOG_ALWAYS	},
    { "users",		do_users,	POS_DEAD,    L_IMM,  LOG_NORMAL	},
    { "wizify", 	do_wizify,	POS_DEAD,    L_CON,  LOG_ALWAYS	},
    { "wizlock",	do_wizlock,	POS_DEAD,    L_CON,  LOG_ALWAYS	},
    { "disable",        do_disable,     POS_DEAD,    L_CON,  LOG_ALWAYS },
    { "iscore",         do_iscore,      POS_DEAD,    L_APP,  LOG_ALWAYS },
    { "echo",		do_echo,	POS_DEAD,    L_ARC,  LOG_ALWAYS },
    { "memory",		do_memory,	POS_DEAD,    L_APP,  LOG_NORMAL	},
    { "mload",		do_mload,	POS_DEAD,    L_DEI,  LOG_BUILD	},
    { "mfind",		do_mfind,	POS_DEAD,    L_DEI,  LOG_NORMAL },
    { "mstat",		do_mstat,	POS_DEAD,    L_DEI,  LOG_NORMAL	},
    { "lstat",		do_lstat,	POS_DEAD,    L_APP,  LOG_NORMAL	},
    { "mwhere",		do_mwhere,	POS_DEAD,    L_DEI,  LOG_NORMAL },
    { "newlock",	do_newlock,	POS_DEAD,    L_CON,  LOG_ALWAYS	},
    { "ofind",		do_ofind,	POS_DEAD,    L_APP,  LOG_NORMAL },
    { "ostat",		do_ostat,	POS_DEAD,    L_DEI,  LOG_NORMAL	},
    { "owhere",		do_owhere,	POS_DEAD,    L_DEI,  LOG_NORMAL	},
    { "pecho",          do_pecho,       POS_DEAD,    L_DEI,  LOG_ALWAYS },
    { "rename",         do_rename,      POS_DEAD,    L_OVD,  LOG_ALWAYS },
    { "wizpwd",         do_wizpwd,      POS_DEAD,    L_CON,  LOG_ALWAYS },
    { "smite",          do_smite,       POS_DEAD,    L_DEI,  LOG_ALWAYS },
    { "dog",            do_dog,         POS_DEAD,    L_OVD,  LOG_ALWAYS },
    { "seize",          do_seize,       POS_DEAD,    L_DIR,  LOG_ALWAYS },
    { "update",         do_update,      POS_DEAD,    L_CON,  LOG_ALWAYS },
    { "peace",		do_peace,	POS_DEAD,    L_APP,  LOG_NORMAL },
    { "pload",          do_pload,       POS_DEAD,    L_DIR,  LOG_ALWAYS },
    { "top",		do_top,  	POS_DEAD,    L_KPR,  LOG_NORMAL },
    { "pwhere", 	do_pwhere,      POS_DEAD,    L_APP,  LOG_NORMAL },
    { "recho",		do_recho,	POS_DEAD,    L_DEI,  LOG_ALWAYS	},
    { "realmemory",	do_realmemory,	POS_DEAD,    L_OVD,  LOG_BUILD  },
    { "return",		do_return,	POS_DEAD,    L_APP,  LOG_NORMAL	},
    { "rset",		do_rset,	POS_DEAD,    L_IMM,  LOG_BUILD	},
    { "rstat",		do_rstat,	POS_DEAD,    L_DEI,  LOG_NORMAL	},
    { "showcorrupt",    do_showcorrupt, POS_DEAD,    L_OVD,  LOG_NORMAL },
    { "slookup",	do_slookup,	POS_DEAD,    0,  LOG_NORMAL},
    { "snoop",		do_snoop,	POS_DEAD,    L_SEN,  LOG_ALWAYS	},
    { "switch",		do_switch,	POS_DEAD,    L_CON,  LOG_ALWAYS	},
    { "wizinvis",	do_invis,	POS_DEAD,    L_APP,  LOG_NORMAL	},
    { "wrlist",         do_wrlist,      POS_DEAD,    L_DEI,  LOG_NORMAL },

    { "marry",		do_marry,	POS_DEAD,    L_SEN,  LOG_ALWAYS },
    { "divorce",	do_divorce,	POS_DEAD,    L_SEN,  LOG_ALWAYS },
    { "rings",		do_rings,	POS_DEAD,    L_IMP,  LOG_ALWAYS },

    { "makelegend",     do_makelegend,  POS_DEAD,    L_OVD,  LOG_ALWAYS },
    { "astrip",         do_astrip,      POS_DEAD,    L_DEI,  LOG_NORMAL },
    { "bamfin",		do_bamfin,	POS_DEAD,    L_APP,  LOG_NORMAL	},
    { "bamfout",	do_bamfout,	POS_DEAD,    L_APP,  LOG_NORMAL	},
    { "bamfsin",	do_bamfsin,	POS_DEAD,    L_APP,  LOG_NORMAL },
    { "bamfsout",	do_bamfsout,	POS_DEAD,    L_APP,  LOG_NORMAL },
    { "goto",		do_goto,	POS_DEAD,    L_APP,  LOG_NORMAL	},
    { "holylight",	do_holylight,	POS_DEAD,    L_APP,  LOG_NORMAL	},

    { "immtalk",	do_immtalk,	POS_DEAD,    L_APP,  LOG_NORMAL	},
    { ":",		do_immtalk,	POS_DEAD,    L_APP,  LOG_NORMAL	},
    { "champion",           do_hero,        POS_DEAD,    LEVEL_HERO,    LOG_NORMAL },
    { "-",              do_hero,        POS_DEAD,    LEVEL_HERO,    LOG_NORMAL },
    { "demigod",         do_demigod,	POS_DEAD,    LEVEL_DEMIGOD,    LOG_NORMAL },
    { "]",		do_demigod,	POS_DEAD,    LEVEL_DEMIGOD,    LOG_NORMAL },
    { "gratz", 		do_gratz,       POS_DEAD,    1,      LOG_NORMAL },
    /*
     * Clan commands -- Altrag
     */
    { "clans",          do_clans,       POS_DEAD,      0,     LOG_NORMAL },
    { "transmute",      do_transmute,   POS_RESTING,  30,     LOG_NORMAL },
    { "bestow",         do_bestow,      POS_DEAD,    L_DEI,   LOG_NORMAL },
    { "image",          do_image,       POS_STANDING, 30,     LOG_NORMAL },

    { "cleanstat",	do_cleanstat,	POS_DEAD,	0,    LOG_ALWAYS },
    { "religions",	do_religions,	POS_DEAD,	0,    LOG_NORMAL },
    { "religioninfo",	do_religioninfo,POS_DEAD,	0,    LOG_NORMAL },
    { "relinfo",	do_religioninfo,POS_DEAD,	0,    LOG_NORMAL },
    { "relquest",	do_relquest,    POS_DEAD,	0,    LOG_NORMAL },
    { "crusade",	do_crusade,    POS_DEAD,	0,    LOG_NORMAL },
    { "map",		do_map,    	POS_DEAD,	0,    LOG_NORMAL },
    /*
     * OLC 1.1b
     */
    { "aedit",		do_aedit,	POS_DEAD,   L_CON,  LOG_BUILD  },
    { "cedit",          do_cedit,       POS_DEAD,   L_CON,  LOG_BUILD  },
    { "hedit",          do_hedit,       POS_DEAD,   L_IMP,  LOG_BUILD  },
    { "redit",		do_redit,	POS_DEAD,   L_DEI,  LOG_BUILD  },
    { "oedit",		do_oedit,	POS_DEAD,   L_DEI,  LOG_BUILD  },
    { "medit",		do_medit,	POS_DEAD,   L_DEI,  LOG_BUILD  },
    { "asave",          do_asave,	POS_DEAD,   L_DEI,  LOG_NORMAL },
    { "alist",		do_alist,	POS_DEAD,   L_DEI,  LOG_NORMAL },
    { "resets",		do_resets,	POS_DEAD,   L_DEI,  LOG_NORMAL },
    { "reledit",	do_reledit,	POS_DEAD,   L_CON,  LOG_BUILD  },

    { "resetxp",	do_resetxp, 	POS_DEAD,    L_OVD,  LOG_ALWAYS }, /* Moved for OLC conflicts */

    /*
     * End of list.
     */
    { "",		0,		POS_DEAD,	 0,  LOG_NORMAL	}
};



/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
    char command [ MAX_INPUT_LENGTH ];
    char logline [ MAX_INPUT_LENGTH ];
    int  cmd;
    int  trust;
    bool found;

    if ( ch->wait > 0 )
      return;


    if (!IS_NPC(ch) && ch->pcdata->craft_timer > 0)
	destroy_craft( ch, FALSE );

    /*
     * Strip leading spaces.
     */
    while ( isspace( *argument ) )
	argument++;
    if ( argument[0] == '\0' )
	return;

    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AFK ) )
	{
	send_to_char(AT_WHITE, "You're marked as AFK, do something about it!\n\r", ch);
	}

    if ( IS_AFFECTED3( ch, AFF_TORTURE )
        && (IS_SET(ch->in_room->room_flags, ROOM_SAFE) ) )
	send_to_char(AT_BLOOD, "Your tortured soul demands vengeance!\n\r", ch);

    /*
     * Implement freeze command.
     */
    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_FREEZE ) )
    {
	send_to_char(AT_LBLUE, "You're totally frozen!\n\r", ch );
	return;
    }

    /*
     * Implement stun.
     */
    if ( IS_STUNNED( ch, STUN_COMMAND ) ||
	 IS_STUNNED( ch, STUN_TOTAL ) )
    {
      send_to_char(AT_LBLUE, "You're too stunned to do anything!\n\r", ch );
      return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy( logline, argument );
    if ( !isalpha( argument[0] ) && !isdigit( argument[0] ) )
    {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;
	while ( isspace( *argument ) )
	    argument++;
    }
    else
    {
	argument = one_argument( argument, command );
    }

    /*
     * Look for command in command table.
     */
    found = FALSE;
    trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == cmd_table[cmd].name[0]
	    && !str_prefix( command, cmd_table[cmd].name )
	    && ( ( cmd_table[cmd].level <= trust ) 
	    || ( IS_SET( ch->affected_by2, CODER ) ) ) )
	{
	    found = TRUE;
	    break;
	}
    }

    /*
     * Log and snoop.
     */
    if ( cmd_table[cmd].log == LOG_NEVER )
	strcpy( logline, "XXXXXXXX XXXXXXXX XXXXXXXX" );


	/*
	 * Builder Logs.
	 * Added by Altrag.
	 */
	if ( cmd_table[cmd].log == LOG_BUILD )
	{
	sprintf( log_buf, "%s: %s", ch->name, logline );
	log_string( log_buf, CHANNEL_BUILD , get_trust( ch ) );
	}

    if ( ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_LOG ) )
	|| fLogAll
	|| cmd_table[cmd].log == LOG_ALWAYS )
    {
      sprintf( log_buf, "%s: %s", ch->name, logline );
      log_string( log_buf, CHANNEL_GOD, ch->level - 1 );
    }

    if ( ch->desc && ch->desc->snoop_by )
    {
        sprintf( log_buf, "%s%%", ch->name );
	write_to_buffer( ch->desc->snoop_by, log_buf,    0 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    if ( IS_AFFECTED2(ch,AFF_BERSERK) && trust < L_SEN)
    {
      if ( !found || (str_cmp( cmd_table[cmd].name, "flee" ) &&
		      str_cmp( cmd_table[cmd].name, "look" ) &&
		      str_cmp( cmd_table[cmd].name, "retreat" ) &&
		      str_cmp( cmd_table[cmd].name, "get"  ) &&
		      str_cmp( cmd_table[cmd].name, "wield") &&
		      str_cmp( cmd_table[cmd].name, "dual" ) &&
		      str_cmp( cmd_table[cmd].name, "score")) )
      {
	send_to_char( AT_BLOOD, "You cannot do that in such a rage!\n\r",ch);
	return;
      }
    }

    if ( IS_AFFECTED3(ch,AFF_BLOODTHIRSTY) && trust < L_SEN)
    {
      if ( !found || (str_cmp( cmd_table[cmd].name, "look" ) &&
		      str_cmp( cmd_table[cmd].name, "get"  ) &&
		      str_cmp( cmd_table[cmd].name, "wield") &&
		      str_cmp( cmd_table[cmd].name, "dual" ) &&
		      str_cmp( cmd_table[cmd].name, "score")) )
      {
	send_to_char( AT_BLOOD, "You cannot do that while so bloodthirsty!\n\r",ch);
	return;
      }
    }

    if ( IS_AFFECTED4(ch,AFF_BURROW) && trust < L_APP)
    {
      if ( !found || (str_cmp( cmd_table[cmd].name, "look" ) &&
		      str_cmp( cmd_table[cmd].name, "who"  ) &&
		      str_cmp( cmd_table[cmd].name, "score")) )
      {
	send_to_char( AT_BLOOD, "You must wait until the earth releases you!\n\r",ch);
	return;
      }
    }

    if ( !found )
    {
        /*
         * Look for command in socials table.
         */
        if ( IS_NPC(ch) || !check_alias( ch, command, argument ) )
        if ( !check_social( ch, command, argument ) )
            send_to_char(C_DEFAULT, "Huh?\n\r", ch );
        return;
    }
    else if ( check_disabled ( ch, &cmd_table[cmd] ) )
    {
        send_to_char (AT_WHITE, "This command has been temporarilly disabled by the Gods\n\r", ch );
        return;
    }


    /*
     * Character not in position for command?
     */
    if ( ch->position < cmd_table[cmd].position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char(AT_RED, "Lie still; you are DEAD.\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char(AT_RED, "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char(AT_WHITE, "You are too stunned to do that.\n\r",    ch );
	    break;

	case POS_SLEEPING:
	    send_to_char(AT_BLUE, "In your dreams, or what?\n\r",           ch );
	    break;

	case POS_RESTING:
	    send_to_char(AT_BLUE, "Nah... You feel too relaxed...\n\r",     ch );
	    break;

	case POS_GHOST:
	    send_to_char(AT_LBLUE, "You are a ghost, and can't do that!\n\r", ch );
	    break;

	case POS_FIGHTING:
	    send_to_char(AT_BLOOD, "No way!  You are still fighting!\n\r",   ch );
	    break;

	}
	return;
    }


    if ( IS_NPC(ch) ) /* PC's are logged in comm.c */
    {
	/*	
      char combuf [MAX_STRING_LENGTH];
      extern int port;

      sprintf(combuf, "%s: [%d] %s %s\n\r", ctime( &current_time), ch->pIndexData->vnum, command, argument);
      append_file( ch, filname, combuf );
	*/
    }

    /*
     * Dispatch the command.
     */
/*
    if ( IS_NPC( ch ) && !can_mob_use( ch, argument ) )
	return;
*/
    (*cmd_table[cmd].do_fun) ( ch, argument );
    tail_chain( );
    return;
}


bool check_alias( CHAR_DATA *ch, char *command, char *argument )
{
   ALIAS_DATA *al;
   char arg[MAX_STRING_LENGTH];
   char newarg[MAX_STRING_LENGTH];
   bool found;
   
   arg[0] = '\0';
   newarg[0] = '\0';
   
   while ( isspace(*argument) )
     argument++;
   strcpy( arg, argument );
   
   if ( IS_NPC( ch ) )
     return FALSE;
     
   if ( !(ch->pcdata->alias_list) )
     return FALSE;
   
   found = FALSE;

   for ( al = ch->pcdata->alias_list; al; al = al->next )
   {
     if ( !str_cmp( al->old, command ) )
     {
         strcpy( newarg, al->_new );
         strcat( newarg, " " );
         strcat( newarg, arg );
         interpret( ch, newarg );
       
       found = TRUE;
       break;
     }
   }
   return found;
}


bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    CHAR_DATA *victim;
    char       arg [ MAX_INPUT_LENGTH ];
    int        cmd;
    bool       found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	    && !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_NO_EMOTE ) )
    {
	send_to_char(AT_LBLUE, "You are anti-social!\n\r", ch );
	return TRUE;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char(AT_RED, "Lie still; you are DEAD.\n\r",             ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char(AT_RED, "You are hurt far too badly for that.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char(AT_WHITE, "You are too stunned to do that.\n\r",      ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
	send_to_char(AT_BLUE, "In your dreams, or what?\n\r",             ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act(AT_PINK, social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
	act(AT_PINK, social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
    }
    else if ( !( victim = get_char_world( ch, arg ) ) )
    {
	send_to_char(AT_WHITE, "They aren't here.\n\r",                    ch );
    }
    else if ( victim == ch )
    {
	act(AT_PINK, social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
	act(AT_PINK, social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
    }
    else if ( !get_char_room( ch, arg ) && can_see( ch, victim ) )
    {
        if ( !IS_NPC( victim ) )
        {
            ROOM_INDEX_DATA *original;
            char            *ldbase                      = "From far away, ";
            char             ldmsg [ MAX_STRING_LENGTH ];

            original = ch->in_room;
            char_from_room( ch );
            char_to_room( ch, victim->in_room );

            strcpy( ldmsg, ldbase );
            strcat( ldmsg, social_table[cmd].char_found );
            act(AT_PINK, ldmsg,                       ch, NULL, victim, TO_CHAR    );

            strcpy( ldmsg, ldbase );
            strcat( ldmsg, social_table[cmd].vict_found );
            act(AT_PINK, ldmsg,                       ch, NULL, victim, TO_VICT    );

            char_from_room( ch );
            char_to_room( ch, original );
        }
        else
        {
            send_to_char(AT_WHITE, "They aren't here.\n\r",                ch );
        }
    }
    else
    {
	act(AT_PINK, social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
	act(AT_PINK, social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );
	act(AT_PINK, social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );

	if ( !IS_NPC( ch )
	    && IS_NPC( victim )
	    && !IS_AFFECTED( victim, AFF_CHARM )
	    && IS_AWAKE( victim )
	    && ( !victim->pIndexData->mobprogs ) )
	    
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:
	        if ( ( victim->level < ch->level )
		    && !( victim->fighting ) )
		    multi_hit( victim, ch, TYPE_UNDEFINED );
		break;

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act(AT_PINK, social_table[cmd].char_found,
		    victim, NULL, ch, TO_CHAR    );
		act(AT_PINK, social_table[cmd].vict_found,
		    victim, NULL, ch, TO_VICT    );
		act(AT_PINK, social_table[cmd].others_found,
		    victim, NULL, ch, TO_NOTVICT );
		break;

	    case 9: case 10: case 11: case 12:
		act(AT_PINK, "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		act(AT_PINK, "$n slaps you.", victim, NULL, ch, TO_VICT    );
		act(AT_PINK, "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		break;
	    }
	}
    }

    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number( char *arg )
{
    if ( *arg == '\0' )
	return FALSE;

    if ( *arg == '+' || *arg == '-' )
        arg++;

    for ( ; *arg != '\0'; arg++ )
    {
	if ( !isdigit( *arg ) )
	    return FALSE;
    }

    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int   number;

	if (!str_cmp(argument, str_empty))
	{
		strcpy(arg, "");
		return 1;
	}
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace( *argument ) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER( *argument );
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace( *argument ) )
	argument++;

    return argument;
}

/*
 * Disable by: Erwin S. Andreasen (4u2@aabc.dk)
 * Disable upto level & disable all added by: Canth (canth@xs4all.nl)
 */
void do_disable (CHAR_DATA *ch, char *argument)
{
        int             i;
        DISABLED_DATA   *p,*q;
        char            buf[100];
        char            arg1 [ MAX_INPUT_LENGTH ];
        char            arg2 [ MAX_INPUT_LENGTH ];

        if ( !authorized( ch, "disable" ) )
                return;

        if (IS_NPC(ch))
        {
                send_to_char (AT_WHITE, "RETURN first.\n\r",ch);
                return;
        }

        if (!argument[0]) /* Nothing specified. Show disabled commands. */
        {
                if (!disabled_first) /* Any disabled at all ? */
                {
                        send_to_char (AT_WHITE, "There are no commands disabled.\n\r",ch);
                        return;
                }

                send_to_char (AT_WHITE, "Disabled commands:\n\r"
                              "Command      To Level    By Level Disabled by\n\r",ch);

                for (p = disabled_first; p; p = p->next)
                {
                        sprintf (buf, "%-12s %5d       %5d      %-12s\n\r",p->command->name, p->uptolevel, p->dislevel, p->disabled_by);
                        send_to_char (AT_WHITE, buf,ch);
                }
                return;
        }

        /* command given */

        argument = one_argument( argument, arg1 );
        one_argument( argument, arg2 );

        /* First check if it is one of the disabled commands */
        for (p = disabled_first; p ; p = p->next)
                if (!str_cmp(arg1, p->command->name))
                        break;

        if (p) /* this command is disabled */
        {
        /* Optional: The level of the imm to enable the command must equal or exceed level
           of the one that disabled it */

                if (get_trust(ch) < p->dislevel)
                {
                        send_to_char (AT_WHITE, "This command was disabledby a higher power.\n\r",ch);
                        return;
                }

                /* Remove */

                if (disabled_first == p) /* node to be removed == head ? */
                        disabled_first = p->next;
                else /* Find the node before this one */
                {
                        for (q = disabled_first; q->next != p; q = q->next); /* empty for */
                        q->next = p->next;
                }

                free_disabled(p);
                save_disabled(); /* save to disk */
                send_to_char (AT_WHITE, "Command enabled.\n\r",ch);
        }
        /*
         * Disable all to re-enable all disabled commands by Canth (canth@xs4all.nl)
         */
        else if ( !str_cmp( arg1, "all" ) ) /* re-enable all commands */
        {
                p = disabled_first;
                while( p )
                {
                        disabled_first = p->next;
                        free_disabled(p);
                        p = disabled_first;
                }
                save_disabled();
                send_to_char(AT_WHITE,  "All commands re-enabled", ch );
        }
        else /* not a disabled command, check if that command exists */
        {
                /* IQ test */
                if (!str_cmp(arg1,"disable"))
                {
                        send_to_char (AT_WHITE, "You cannot disable the disable command.\n\r",ch);
                        return;
                }

                /* Search for the command */
                for (i = 0; cmd_table[i].name[0] != '\0'; i++)
                        if (!str_cmp(cmd_table[i].name, arg1))
                                break;

                /* Found? */
                if (cmd_table[i].name[0] == '\0')
                {
                        send_to_char (AT_WHITE, "No such command.\n\r",ch);
                        return;
                }

                /* Can the imm use this command at all ? */
                if (cmd_table[i].level > get_trust(ch))
                {
                        send_to_char (AT_WHITE, "You dot have access to that command; you cannot disable it.\n\r",ch);
                        return;
                }
                /* Disable the command */

                p = alloc_mem (sizeof(DISABLED_DATA));
                p->command = &cmd_table[i];
                p->disabled_by = str_dup (ch->name); /* save name of disabler */
                p->dislevel = get_trust(ch); /* save trust */
                if ( is_number( arg2 ) ) /* unusable upto & including level arg2 */
                        p->uptolevel = atoi( arg2 ); /* (no check for out of range) */
                else
                        p->uptolevel = get_trust( ch );
                p->next = disabled_first;
                disabled_first = p; /* add before the current first element */

                send_to_char (AT_WHITE, "Command disabled.\n\r",ch);
                save_disabled(); /* save to disk */
        }
}

/* Check if that command is disabled
   Note that we check for equivalence of the do_fun pointers; this means
   that disabling 'chat' will also disable the '.' command
*/
bool check_disabled ( CHAR_DATA *ch, const struct cmd_type *command )
{
        DISABLED_DATA *p;

        for (p = disabled_first; p ; p = p->next)
                if (p->command->do_fun == command->do_fun && ch->level <= p->uptolevel)
                        return TRUE;

        return FALSE;
}


/* Load disabled commands */
void load_disabled()
{
        FILE *fp;
        DISABLED_DATA *p;
        char *name;
        int i;

        disabled_first = NULL;

        fp = fopen (DISABLED_FILE, "r");

        if (!fp) /* No disabled file.. no disabled commands : */
                return;

        name = fread_word (fp);

        while (str_cmp(name, END_MARKER)) /* as long as name is NOT END_MARKER :) */
        {
                /* Find the command in the table */
                for (i = 0; cmd_table[i].name[0] ; i++)
                        if (!str_cmp(cmd_table[i].name, name))
                                break;

                if (!cmd_table[i].name[0]) /* command does not exist? */
                {
                        bug ("Skipping uknown command in " DISABLED_FILE " file.",0);
                        fread_number(fp); /* uptolevel */
                        fread_number(fp); /* dislevel */
                        fread_word(fp); /* disabled_by */
                }
                else /* add new disabled command */
                {
                        p = alloc_mem(sizeof(DISABLED_DATA));
                        p->command = &cmd_table[i];
                        p->uptolevel = fread_number(fp);
                        p->dislevel = fread_number(fp);
                        p->disabled_by = str_dup(fread_word(fp));
                        p->next = disabled_first;

                        disabled_first = p;

                }

                name = fread_word(fp);
        }

        fclose (fp);
}

/* Save disabled commands */
void save_disabled()
{
#ifdef sql_system
	sql_save_disabled();
#else
    FILE *fp;
    DISABLED_DATA *p;

    if (!disabled_first) /* delete file if no commands are disabled */
    {
            unlink (DISABLED_FILE);
            return;
    }

    fp = fopen (DISABLED_FILE, "w");

    if (!fp)
    {
            bug ("Could not open " DISABLED_FILE " for writing",0);
            return;
    }

    for (p = disabled_first; p ; p = p->next)
            fprintf (fp, "%s %d %d %s\n", p->command->name, p->uptolevel, p->dislevel, p->disabled_by);

    fprintf (fp, "%s\n",END_MARKER);

    fclose (fp);
#endif
}

void cleanup_disabled()
{
	DISABLED_DATA* dptr = disabled_first;
	
	while(dptr)
	{
		disabled_first = dptr->next;
		free(dptr);
		dptr = disabled_first;
	}
}

void do_alias( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  ALIAS_DATA *al;
  ALIAS_DATA *oAl;
 
  if( IS_NPC( ch ) ) return;
 
  smash_tilde( argument );
  argument = one_argument( argument, arg );
  strcpy( arg1, argument );
  
  if ( arg[0] == '\0' )
  {
    if ( !ch->pcdata->alias_list )
    {
      send_to_char( AT_WHITE, "You have no alias' set.\n\r", ch );
      return;
    }
    
    send_to_char( AT_WHITE, "Your currently set alias' are:\n\r", ch );
  
    for ( al = ch->pcdata->alias_list; al; al = al->next )
    {
      sprintf( log_buf, "  %-10s   &B%s&w\n\r", al->old, al->_new );
      send_to_char( AT_RED, log_buf, ch );
    }
    send_to_char( AT_WHITE, "\n\r", ch );
    return;
  }
  
  if ( arg1[0] == '\0' && ( !( ch->pcdata->alias_list ) ) )
  {
    send_to_char( AT_WHITE, "You have no command alias' to remove!\n\r", ch );
    return;
  }
  
  if ( arg1[0] != '\0' )
  {
    for ( oAl = ch->pcdata->alias_list; oAl; oAl = oAl->next )
    {
      if ( is_name( arg1, oAl->old ) || !str_prefix( arg, arg1 ) )
      {
        send_to_char( AT_WHITE, "You cannot alias other alias' into your [new] string.\n\r", ch );
        return;
      }
    }
  }        
  
  for ( al = ch->pcdata->alias_list; al; al = al->next )
    if ( !str_cmp( al->old, arg ) )
      break;
  if ( arg1[0] == '\0' )
  {
    if ( !al )
    {
      send_to_char( C_DEFAULT, "Alias does not exist.\n\r", ch );
      return;
    }
    if ( al == ch->pcdata->alias_list )
      ch->pcdata->alias_list = al->next;
    else
    {
      for ( oAl = ch->pcdata->alias_list; oAl; oAl = oAl->next )
	if ( oAl->next == al )
	  break;
      if ( !oAl )
      {
	sprintf( log_buf, "Do_alias: bad alias - ch (%s) - [%s]", ch->name, arg );
	bug( log_buf, 0 );
	send_to_char(C_DEFAULT, "Alias does not exist.\n\r", ch );
	return;
      }
      oAl->next = al->next;
    }
    free_alias( al);
    act( AT_WHITE, "Alias '$t' cleared.\n\r", ch, arg, NULL, TO_CHAR );
    return;
  }

  if ( al )
  {
    free_string( al->_new );
    al->_new = str_dup( arg1 );
    sprintf( log_buf, "Alias '%s' remade.\n\r", arg );
    send_to_char( AT_WHITE, log_buf, ch );  
    return;
  }
  
  add_alias( ch, al, arg, arg1 );
  sprintf( log_buf, "Alias '%s' added.\n\r", arg );
  send_to_char( AT_WHITE, log_buf, ch );
  return;
}

void add_alias( CHAR_DATA *ch, ALIAS_DATA *pAl, char *old, char *_new )
{
  pAl = new_alias();
  pAl->old = str_dup( old );
  pAl->_new = str_dup( _new );
  pAl->next = ch->pcdata->alias_list;  ch->pcdata->alias_list = pAl;
  return;
}

void do_countcommands( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  int scnt;

  for ( scnt = 0; skill_table[scnt].name[0] != '\0'; scnt++ );

  sprintf(buf, "Command table size: %d\n\r"
	       "Social table size:  %d\n\r"
	       "GSkill table size:  %d\n\r"
	       "Skill table size:   %d\n\r",
	  /* -1 from each for blank index entry at end. */
	  sizeof(   cmd_table) / sizeof(   cmd_table[0]) - 1,
	  sizeof(social_table) / sizeof(social_table[0]) - 1,
	  sizeof(gskill_table) / sizeof(gskill_table[0])    ,
	  scnt); /* Someone wanna explain why theres an error with
		    sizeof(skill_table)?? */
  send_to_char(AT_PINK, buf, ch );
  return;
}

bool can_mob_use( CHAR_DATA *ch, char *argument) 
{
    if( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) && (
	   !str_prefix( argument, "stun")
	|| !str_prefix( argument, "slam" )
	|| !str_prefix( argument, "slit" )
	|| !str_prefix( argument, "steal" )
	|| !str_prefix( argument, "backstab")
	|| !str_prefix( argument, "bs")
	|| !str_prefix( argument, "snare")
	|| !str_prefix( argument, "untangle")
	|| !str_prefix( argument, "poison")
	|| !str_prefix( argument, "depoison")
	|| !str_prefix( argument, "double")
	|| !str_prefix( argument, "dbs")
	|| !str_prefix( argument, "assassinate")
	|| !str_prefix( argument, "dim")
	|| !str_prefix( argument, "patch")
	|| !str_prefix( argument, "berserk")
	|| !str_prefix( argument, "bloodthirsty")
	|| !str_prefix( argument, "weaponmaster")
	|| !str_prefix( argument, "break")
	|| !str_prefix( argument, "thick")
	|| !str_prefix( argument, "backkick")
	|| !str_prefix( argument, "bk")
	|| !str_prefix( argument, "claw")
	|| !str_prefix( argument, "gouge")
	|| !str_prefix( argument, "paralyse")
	|| !str_prefix( argument, "quickness") ) )
	{
		return FALSE;
	}

	return TRUE;
}
