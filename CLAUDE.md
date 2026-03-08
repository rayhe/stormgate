# Stormgate MUD

## Overview

Stormgate is a text-based Multi-User Dungeon (MUD) server written in C. It descends from the **Diku -> Merc -> Envy** MUD lineage, with extensive custom modifications branded as "Mythran Mud 3.1.1". The server communicates with players over raw TCP sockets using a telnet-like protocol with ANSI color support.

## Codebase Heritage

- **Diku MUD** (1990-1991) - Original Danish MUD
- **Merc 2.1** (1993) - Major rewrite by Furey, Hatchet, Kahn
- **Envy** (1994) - Extensions by Michael Quan, David Love, et al.
- **Mythran** - Further extensions (OLC, economy, clans, religions)
- **Stormgate** - This fork with additional systems by Tyrion, Ahsile, Manaux, XOR, and others

## Directory Structure

```
stormgate/
├── src/          # C source code (~170 files including .o and project files)
├── area/         # Area data files (.are) defining the game world (~150 areas)
├── player/       # Player save files (gzipped, organized by first letter)
├── log/          # Game log files (numbered sequentially)
└── README.md
```

## Source Code Architecture

### Core Systems (src/)

| File | Purpose |
|------|---------|
| `merc.h` | Master header (~4600 lines). All structs, constants, macros, function declarations |
| `config.h` | Compile-time feature toggles (taxes, languages, economy, etc.) |
| `colors.h` | ANSI color code definitions |
| `olc.h` | Online Creation (OLC) editor definitions |
| `comm.c` | Socket I/O, main game loop, descriptor handling |
| `db.c` | Database loading - reads area files, boots the world |
| `save.c` | Player file save/load (custom text format, gzip support) |
| `interp.c` | Command interpreter and command table |
| `handler.c` | Object/character manipulation helpers |
| `update.c` | Tick-based game updates (combat, weather, healing, etc.) |
| `const.c` | Static data tables (classes, clans, guilds, XP table) |

### Game Systems

| File(s) | System |
|---------|--------|
| `fight.c`, `fight2.c` | Combat engine (multi-hit, dual wield, shields, damage classes) |
| `magic.c` - `magic4.c` | Spell system (hundreds of spells) |
| `skills.c`, `skill_table.c` | Skill definitions and skill system |
| `act_comm.c` | Communication (say, tell, shout, channels) |
| `act_info.c`, `act_info2.c` | Information commands (look, score, who, etc.) |
| `act_move.c` | Movement, doors, vehicles |
| `act_obj.c`, `act_obj2.c` | Object manipulation (get, drop, wear, etc.) |
| `act_wiz.c` - `act_wiz3.c` | Immortal/admin commands |
| `act_clan.c` | Clan system |
| `act_multi.c` | Multiclassing system |
| `act_race.c` | Race-specific abilities |
| `quest.c` | Auto-quest system |
| `religion.c`, `rel_quest.c` | Religion system with quests |
| `economy.c` | Bank system (deposits, transfers, stock market/shares) |
| `crafting.c` | Crafting system (scrolls, potions, forging) |
| `games.c` | Gambling games |
| `marriage.c` | Player marriage system |
| `language.c` | Language system (27 languages) |
| `healer.c` | NPC healer services |
| `arena.c` | PvP arena system |
| `hunt.c`, `track.c` | NPC hunting/tracking AI |
| `scan.c` | Room scanning |
| `special.c` | Special mob behaviors |
| `mob_prog.c`, `mob_commands.c` | MobProg scripting for NPCs |
| `olc.c`, `olc_act.c`, `olc_save.c` | Online area editor (OLC) |
| `social-edit.c` | Social command editor |
| `drunk.c` | Drunk speech garbling |
| `weapons.c` | Weapon system |
| `id.c` | Unique ID generation |
| `ssm.c` | Shared String Manager (memory optimization) |
| `memory.c` | Memory allocation |
| `bit.c` | Bitvector utilities |
| `string.c` | String editor for OLC |

### Class Files (`cls_*.c`)

Each class has its own source file with class-specific spells/skills:

- `cls_mag.c` - Mage
- `cls_pal.c` - Paladin
- `cls_thf.c` - Thief
- `cls_rng.c` - Ranger
- `cls_bar.c` - Barbarian
- `cls_ant.c` - Anti-Paladin
- `cls_asn.c` - Assassin
- `cls_nec.c` - Necromancer
- `cls_psi.c` - Psionicist
- `cls_shm.c` - Shaman
- `cls_vam.c` - Vampire
- `cls_none.c` - Classless/fallback

**Total classes defined:** 21 (including Mage, Cleric, Thief, Warrior, Psionicist, Druid, Ranger, Paladin, Bard, Vampire, Werewolf, Anti-Paladin, Assassin, Monk, Barbarian, Illusionist, Necromancer, Demonologist, Shaman, Darkpriest)

## Key Data Structures

- **CHAR_DATA** - Character (player or NPC). Contains stats, inventory, affects, combat state, quest data, religion, clan, guild membership
- **PC_DATA** - Player-only data (password, skills[], bank account, aliases, crafting, spouse)
- **OBJ_DATA** - Game objects (equipment, items). Has type, wear flags, durability, magical effects
- **ROOM_INDEX_DATA** - Rooms with exits (6 directions), descriptions, traps, sector types
- **AREA_DATA** - Areas containing rooms, with reset schedules and builder security
- **MOB_INDEX_DATA** - NPC templates (prototypes loaded from area files)
- **OBJ_INDEX_DATA** - Object templates

## Area File Format

Areas use a custom text format (`.are` files) with sections:
- `#AREADATA` - Area metadata (name, vnums, security, version)
- `#MOBILES` - NPC definitions
- `#OBJECTS` - Item definitions
- `#ROOMS` - Room definitions with exits
- `#RESETS` - Spawn/placement rules
- `#SHOPS` - Shop configurations
- `#SPECIALS` - Special mob behaviors
- `#MOBPROGS` - Scripted mob behaviors

Areas are listed in `area/areaTS.lst` (116 areas loaded at boot).

## Game Parameters

- **Max level:** 116 (115 = top mortal immortal rank, 100 = hero tier)
- **Max skills:** 485
- **Max races:** 27
- **Max clans:** 21
- **Max religions:** 8
- **Languages:** 27 (Common through Gargish)
- **Tick rate:** 4 pulses/second, 30-second ticks

## Build System

- `makefile` in `src/` - uses `gcc`/`g++`, links with `-lcrypt -lm`
- Optional SQL support via MySQL (`-DSQL_SYSTEM`)
- Optional Win32 support (`-DRUN_AS_WIN32SERVICE`)
- Binary output: `src/stormgate`

## Licenses

Three licenses apply (all in `src/`):
- `license.doc` - Original Diku license (no commercial use, credit required)
- `license.txt` - Merc license
- `license.nvy` - Envy license

## Player Data

- Saved as text files in `player/<first_letter>/PlayerName`
- Optional gzip compression (`AUTO_COMPRESS`)
- Contains all character state: stats, equipment, skills, aliases, quest progress
