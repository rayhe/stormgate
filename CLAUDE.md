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
├── src/              # C source code (~170 files including .o and project files)
├── area/             # Area data files (.are) defining the game world (~150 areas)
├── player/           # Player save files (gzipped, organized by first letter)
├── log/              # Game log files (numbered sequentially)
├── web/              # Web version (Phase 1 scaffold)
│   ├── client/       # React + TypeScript + Vite frontend
│   │   └── src/
│   │       ├── core/         # Auth, WebSocket connection, config
│   │       └── components/   # LoginScreen, Terminal
│   └── server/       # Node.js + TypeScript + Express + ws backend
│       └── src/
│           └── index.ts      # WebSocket server with Firebase Auth
├── CLAUDE.md
├── PLAN.md           # Full implementation plan (phases, hosting, schema)
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

## Web Version (web/)

### Status

All 5 phases implemented. Code is written but dependencies have not been installed yet (`npm install` needs external network access to npmjs.org).

**Completed:**
- Phase 1: Web client scaffold (React + Firebase Auth + terminal)
- Phase 2: Firestore schema, security rules, area migration script
- Phase 3: Full TypeScript game engine (33+ commands, game loop, combat, regen, weather)
- Phase 4: Permissions & roles (save/load, admin commands, character selection, auto-save)
- Phase 5: Enhanced web UI (vitals bars, minimap, combat overlay, mobile responsive)

**Remaining before playable:**
- `npm install` in `web/client/` and `web/server/` (needs npmjs.org access)
- Firebase project setup (create project, enable Google Sign-In, get service account key)
- Run area migration: `cd web/server && npx tsx src/migration/migrate.ts ../../area/`
- Deploy server to Oracle Cloud VM
- Deploy client to Firebase Hosting

### Architecture

```
web/
├── client/                    # React + TypeScript + Vite frontend
│   └── src/
│       ├── core/
│       │   ├── config.ts      # Firebase project config + WebSocket URL
│       │   ├── auth.ts        # Google Sign-In, token management
│       │   ├── connection.ts  # WebSocket client with auto-reconnect + structured messages
│       │   ├── protocol.ts    # JSON message types (vitals, room, combat, channel)
│       │   └── gameState.tsx  # React context game state store
│       └── components/
│           ├── LoginScreen.tsx   # ASCII art banner + Google sign-in
│           ├── Terminal.tsx      # ANSI color terminal renderer + combat overlay
│           ├── VitalsPanel.tsx   # HP/Mana/Move bars with animation + critical pulsing
│           ├── RoomPanel.tsx     # Compass exits + 3x3 auto-map of visited rooms
│           ├── SidePanel.tsx     # Container for vitals + room + quick actions
│           ├── MobileNav.tsx     # Touch direction buttons + actions (mobile only)
│           ├── CombatOverlay.tsx # Enemy HP bar, damage flash, auto-hide
│           └── App.tsx           # Layout with side panel, mobile responsive
├── server/                    # Node.js + TypeScript + Express + ws backend
│   └── src/
│       ├── server.ts          # Express + WebSocket server, Firebase Admin, character selection
│       ├── index.ts           # Original stub server (unused, kept for reference)
│       └── engine/
│           ├── types.ts       # All interfaces/enums from merc.h (832 lines)
│           ├── world.ts       # In-memory world state, mob/obj factories, charRoomMap
│           ├── handler.ts     # char/obj manipulation, affects, stat getters
│           ├── output.ts      # sendToChar, act() with $-tokens, prompt rendering
│           ├── commands.ts    # 33+ commands with prefix matching
│           ├── fight.ts       # Full combat: damage calc, death, corpses, NPC AI (898 lines)
│           ├── update.ts      # Game loop (4Hz), violence, regen, time, weather, mob AI
│           ├── save.ts        # Firestore character save/load, auto-save (540 lines)
│           ├── admin.ts       # 15 admin/immortal/builder commands (1002 lines)
│           ├── protocol.ts    # Structured JSON messages to client (vitals, room, combat)
│           └── index.ts       # Barrel export
│       └── migration/
│           ├── area-parser.ts # Parses all 115 .are files (1232 lines)
│           └── migrate.ts     # Batch imports into Firestore (363 lines)
├── firebase/
│   ├── firebase.json          # Hosting + Firestore config
│   └── firestore.rules        # Role-based security (player/builder/immortal/admin)
└── .gitignore
```

### Game Engine

**Commands (33+):**

| Category | Commands |
|----------|----------|
| Movement | north, east, south, west, up, down |
| Info | look, score, who, inventory, equipment, exits, help |
| Communication | say, tell, shout, chat |
| Objects | get, drop, wear, remove, give |
| Combat | kill, flee, consider, wimpy |
| Position | rest, sleep, stand, wake |
| Actions | quit, save, recall |
| Builder (106+) | rstat, mstat, ostat |
| Immortal (108+) | goto, transfer, peace, slay, stat, force, wiznet |
| Admin (115+) | setrole, advance, restore, purge |

**Combat system:**
- Hit/miss rolls with hitroll, armor, dexterity
- Weapon damage dice + damroll bonus, bare-hand fallback
- 16-tier damage messages (miss → scratch → hit → maul → OBLITERATE)
- Attack nouns by weapon type (slash, stab, thrust, pound, chop, etc.)
- 1-5 attacks/round based on level + dual wield bonus
- NPC death: corpse with inventory, XP/gold to killer
- PC death: respawn at recall, lose 10% XP
- Wimpy auto-flee
- NPC AI: wandering (10% per pulse), aggression (attack PCs), scavenging (pick up items)

**Save system:**
- Full character serialization to Firestore (stats, equipment, inventory, affects, pcdata)
- Character selection menu on login (pick existing or create new)
- Auto-save every 5 minutes
- Save on quit, disconnect, and server shutdown

**Permissions:**
- Role hierarchy: player → builder (106+) → immortal (108+) → admin (115+)
- `setrole` command updates both in-game level and Firestore user doc
- Firestore security rules enforce document-level access by role

### Setup

1. Create a Firebase project, enable Google Sign-In in Authentication
2. Copy web app config into `web/client/src/core/config.ts`
3. Place Firebase service account JSON at `web/server/service-account.json`
4. `cd web/client && npm install && npm run dev`
5. `cd web/server && npm install && npm run dev`
6. Migrate area data: `cd web/server && npx tsx src/migration/migrate.ts ../../area/`
7. Open `http://localhost:5173`

### Hosting Target

- **Game server**: Oracle Cloud Always Free ARM A1 VM (systemd + Nginx/Caddy + Let's Encrypt)
- **Client**: Firebase Hosting (static files)
- **Auth**: Firebase Authentication (Google provider)
- **Database**: Cloud Firestore
- **Cost**: $0/month (all free tier)

See `PLAN.md` for full deployment checklist.

## Deployment (Live)

- **Play:** https://new.rayhe.net/stormgate.html or `telnet 129.146.126.146 4000`
- **VM:** Oracle Cloud ARM A1 Flex, 4 OCPU / 24GB RAM (always free max)
- **Services:** stormgate (C MUD, port 4000), stormgate-web (WS proxy, port 8080), Caddy (HTTPS)
- **Frontend:** Cloudflare Pages (rayhe.net), connects via `wss://macmini.rayhe.net/ws`
- **Build:** `make -j$(nproc) -f makefile.prod` in `src/` (requires `-fcommon` for GCC 10+)

## Modernization Options

Features from other Diku/Merc forks that Stormgate could adopt. Prioritized by impact.

### High Impact

| Feature | Description | Port From | Effort |
|---------|-------------|-----------|--------|
| Copyover/hotboot | Restart server binary without disconnecting players. Save descriptor state, exec() new binary, reconnect. | ROM, SMAUG, tbaMUD | Medium |
| Split AC | 4 armor class values (pierce/bash/slash/magic) instead of 1. Adds tactical depth. | ROM 2.4 | Medium |
| Auction house | Escrow-based item trading between players | AFKMud | Low-Medium |
| Player housing | Persistent rooms owned by players with door locks | tbaMUD | Medium |

### Medium Impact

| Feature | Description | Port From |
|---------|-------------|-----------|
| Overland wilderness map | ANSI-rendered terrain map replacing boring connector zones, multiple continents | AFKMud |
| Layered equipment | Wear cloak over armor over shirt on same body slot | SMAUG |
| Socketed/runed weapons | Diablo-style gem slots for equipment bonuses | AFKMud |
| Random treasure generation | Configurable loot tables per area, Diablo-style drops | AFKMud |
| DG Scripts | Full scripting language (if/while/variables) for mobs/rooms/objects — much more powerful than MobProgs | tbaMUD |
| Dual currency | Silver and gold coins | ROM 2.4 |
| Corpse saving | Corpses persist across reboots | SMAUG |
| Levers/switches/buttons | Interactive room mechanisms for puzzles | SMAUG |

### What Stormgate Already Has (from Envy/Mythran)

These are features that some forks added but Stormgate already has:
- OLC (Online Creation) for builders
- Immunities/Resistances/Vulnerabilities (IRV)
- Clans, religions, 27 languages
- Equipment durability and repair
- Crafting (potions, scrolls, forging)
- MobProgs + object/room progs
- NPC hunting/tracking AI
- Economy with bank system
- Quest system

### Reference Codebases (all open source on GitHub)

| Codebase | Lineage | URL | Notes |
|----------|---------|-----|-------|
| Magma MUD | Envy/UltraEnvy (closest to Stormgate) | github.com/Xangis/magma | Same ancestry |
| UltraEnvy 2.2 | Envy (Mythran's direct ancestor) | github.com/DikuMUDOmnibus/Ultra-Envy | |
| ROM 2.4 | Merc | github.com/gblues/ROM | Split AC, dual currency |
| BasedMUD | ROM (modernized) | github.com/scandum/basedmud | MTH telnet support |
| AFKMud | SMAUG (most feature-rich) | github.com/Arthmoor/AFKMud | C++, overland, runes, auction |
| SmaugFUSS | SMAUG (community-maintained) | github.com/smaugmuds/_smaug_ | |
| tbaMUD | CircleMUD | github.com/tbamud/tbamud | DG Scripts, automap, housing |
| LuminariMUD | tbaMUD | github.com/LuminariMUD/Luminari-Source | D20/Pathfinder rules |
