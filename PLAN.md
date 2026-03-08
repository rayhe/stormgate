# Stormgate Web Client - Implementation Plan

## Goal

Build a web-based MUD client for Stormgate that uses Google Authentication for login and Firebase for data persistence with permission-based access control. Full TypeScript engine port, hosted entirely on free-tier infrastructure.

---

## Hosting & Infrastructure ($0/month)

### Game Server: Oracle Cloud Always Free Tier
- **Instance:** ARM Ampere A1 (up to 4 OCPUs, 24GB RAM) — massively overkill for a MUD
- Even the smaller x86 free instance (1/8 OCPU, 1GB RAM) is more than enough
- The TypeScript game engine holds the entire world in memory (~100-200MB)
- 200GB block storage, 10TB/month outbound included
- **Setup:**
  - Node.js game engine runs as a `systemd` service (auto-restart on crash/reboot)
  - Nginx or Caddy as reverse proxy for TLS termination (free Let's Encrypt cert)
  - WebSocket connections proxied through to the Node.js process

### Static Client: Firebase Hosting (Free Tier)
- 10GB storage, 360MB/day transfer
- Serves the React/xterm.js client as static files
- Custom domain support with automatic SSL

### Auth: Firebase Authentication (Free Tier)
- Unlimited Google Sign-In authentications
- No cost for any volume of MUD players

### Database: Cloud Firestore (Free Tier)
- 1GB storage, 50k reads/day, 20k writes/day
- More than sufficient — a MUD writes player saves periodically, not on every keystroke
- Game world loaded into memory at boot, Firestore is the persistence layer not the hot path

### Cost Summary

| Service | Free Tier Limit | MUD Usage | Cost |
|---------|----------------|-----------|------|
| Oracle Cloud VM | 4 OCPU / 24GB RAM | ~0.1 CPU / 200MB RAM | $0 |
| Firebase Hosting | 360MB/day | ~10MB/day (small SPA) | $0 |
| Firebase Auth | Unlimited | Any number of players | $0 |
| Cloud Firestore | 50k reads, 20k writes/day | ~5k reads, ~2k writes/day | $0 |
| Let's Encrypt TLS | Unlimited | 1 cert | $0 |
| **Total** | | | **$0/month** |

---

## Architecture Overview

```
┌─────────────────────────────────────────────────┐
│          Firebase Hosting (free tier)            │
│                                                  │
│  ┌───────────────── React SPA ────────────────┐  │
│  │  ┌───────────┐ ┌──────────┐ ┌───────────┐  │  │
│  │  │ Terminal   │ │ Map View │ │ Character │  │  │
│  │  │ (xterm.js) │ │(optional)│ │ Panel     │  │  │
│  │  └─────┬─────┘ └──────────┘ └───────────┘  │  │
│  │        │                                    │  │
│  │  ┌─────┴────────────────────────────────┐   │  │
│  │  │       WebSocket Client Layer         │   │  │
│  │  └─────┬────────────────────────────────┘   │  │
│  │        │                                    │  │
│  │  ┌─────┴────────────────────────────────┐   │  │
│  │  │  Firebase Auth (Google Sign-In)      │   │  │
│  │  └──────────────────────────────────────┘   │  │
│  └─────────────────────────────────────────────┘  │
└────────┬──────────────────────────────────────────┘
         │ WSS (wss://your-domain.com)
┌────────┴──────────────────────────────────────────┐
│    Oracle Cloud Free Tier VM (ARM A1)             │
│                                                    │
│  ┌─────────────────────────────────────────────┐   │
│  │  Nginx/Caddy (TLS via Let's Encrypt)        │   │
│  └─────┬───────────────────────────────────────┘   │
│        │                                           │
│  ┌─────┴───────────────────────────────────────┐   │
│  │  Node.js Game Engine (systemd service)      │   │
│  │  ┌───────────────────────────────────────┐  │   │
│  │  │  WebSocket Server (ws)                │  │   │
│  │  ├───────────────────────────────────────┤  │   │
│  │  │  Firebase Admin SDK                   │  │   │
│  │  │  - Auth token verification            │  │   │
│  │  │  - Firestore read/write               │  │   │
│  │  ├───────────────────────────────────────┤  │   │
│  │  │  Game Engine (TypeScript port of C)   │  │   │
│  │  │  - World state held in memory         │  │   │
│  │  │  - Periodic save to Firestore         │  │   │
│  │  └───────────────────────────────────────┘  │   │
│  └─────────────────────────────────────────────┘   │
└────────────────────┬───────────────────────────────┘
                     │
┌────────────────────┴───────────────────────────────┐
│              Firebase (free tier)                   │
│  ┌──────────────┐  ┌──────────────────────────┐    │
│  │ Auth         │  │ Firestore                │    │
│  │ (Google SSO) │  │ - users/{uid}            │    │
│  │              │  │ - characters/{charId}    │    │
│  │              │  │ - areas/{areaName}       │    │
│  │              │  │ - world_state/           │    │
│  └──────────────┘  └──────────────────────────┘    │
└────────────────────────────────────────────────────┘
```

---

## Phase 1: Foundation (Web Client + Auth)

### Step 1.1 - Project Setup
- Initialize a monorepo with `client/` and `server/` directories
- **Client:** React + TypeScript + Vite
- **Server:** Node.js + TypeScript + Express + ws (WebSocket)
- Set up Firebase project in Firebase Console
  - Enable Google Sign-In provider in Authentication
  - Create Firestore database
  - Generate service account key for server

### Step 1.2 - Google Authentication
- **Client side:**
  - Install `firebase` SDK
  - Configure Firebase with project credentials
  - Implement Google Sign-In flow using `signInWithPopup(provider)`
  - Store auth token, pass it on every WebSocket connection
- **Server side:**
  - Install `firebase-admin` SDK
  - On WebSocket connection, verify the Firebase ID token
  - Extract `uid`, `email`, `displayName` from decoded token
  - Reject unauthenticated connections

### Step 1.3 - Basic Web Terminal
- Use `xterm.js` for terminal emulation in the browser
- Connect to the backend via WebSocket
- Send player input as text commands
- Receive and render ANSI-colored output
- Handle the Stormgate ANSI color codes (`colors.h` mappings)

---

## Phase 2: Firebase Data Layer

### Step 2.1 - Firestore Schema Design

```
firestore/
├── users/{firebase_uid}
│   ├── email: string
│   ├── displayName: string
│   ├── createdAt: timestamp
│   ├── lastLogin: timestamp
│   ├── role: "player" | "builder" | "immortal" | "admin"
│   └── characterIds: string[]
│
├── characters/{charId}
│   ├── ownerId: string (firebase_uid)
│   ├── name: string
│   ├── class: number
│   ├── race: number
│   ├── level: number
│   ├── sex: number
│   ├── clan: number
│   ├── religion: number
│   ├── stats/
│   │   ├── str, int, wis, dex, con (perm + mod)
│   │   ├── hp, maxHp, mana, maxMana, move, maxMove
│   │   ├── gold, bankAccount, exp
│   │   ├── hitroll, damroll, armor
│   │   └── alignment, position
│   ├── skills: map<skillName, learnedLevel>
│   ├── equipment: map<wearSlot, objectRef>
│   ├── inventory: objectRef[]
│   ├── storage: objectRef[]
│   ├── affects: affect[]
│   ├── aliases: map<alias, command>
│   ├── quest/
│   │   ├── questpoints, nextquest, countdown
│   │   └── questobj, questmob
│   └── meta/
│       ├── played: number (seconds)
│       ├── lastLogin: timestamp
│       ├── pkill: boolean
│       ├── title: string
│       └── prompt: string
│
├── objects/{objectId}
│   ├── indexVnum: number
│   ├── name, shortDescr, description: string
│   ├── itemType, extraFlags, wearFlags: number
│   ├── values: number[4]
│   ├── weight, cost, level: number
│   ├── durability: { cur, max }
│   ├── affects: affect[]
│   └── containedIn: objectRef | null
│
├── areas/{areaSlug}
│   ├── name: string
│   ├── filename: string
│   ├── vnumRange: { low, high }
│   ├── builders: string[]
│   ├── security: number
│   ├── creatorUid: string
│   └── rooms/{vnum}
│       ├── name, description: string
│       ├── sectorType, roomFlags: number
│       ├── exits: map<direction, { toVnum, flags, key, keyword }>
│       └── extraDescriptions: map<keyword, text>
│
├── mob_templates/{vnum}
│   └── ... (mob index data)
│
├── obj_templates/{vnum}
│   └── ... (obj index data)
│
└── world_state/
    ├── time: { hour, day, month, year }
    ├── weather: { sky, sunlight, mmhg, change }
    ├── clans/{clanId}: clan data
    └── religions/{relId}: religion data
```

### Step 2.2 - Firestore Security Rules

```javascript
rules_version = '2';
service cloud.firestore {
  match /databases/{database}/documents {

    // Users can read/write their own user doc
    match /users/{userId} {
      allow read: if request.auth != null;
      allow write: if request.auth.uid == userId;
    }

    // Characters: owner can read/write, others can read name/level (for 'who' list)
    match /characters/{charId} {
      allow read: if request.auth != null;
      allow create: if request.auth.uid == request.resource.data.ownerId;
      allow update, delete: if request.auth.uid == resource.data.ownerId
                            || getUserRole(request.auth.uid) in ['immortal', 'admin'];
    }

    // Areas: readable by all authenticated, writable by builders/immortals/admins
    match /areas/{areaId} {
      allow read: if request.auth != null;
      allow write: if getUserRole(request.auth.uid) in ['builder', 'immortal', 'admin'];

      match /rooms/{roomId} {
        allow read: if request.auth != null;
        allow write: if getUserRole(request.auth.uid) in ['builder', 'immortal', 'admin'];
      }
    }

    // World state: readable by all, writable by server (admin SDK bypasses rules)
    match /world_state/{doc=**} {
      allow read: if request.auth != null;
      allow write: if false; // Server uses admin SDK
    }

    // Templates: readable by all, writable by builders+
    match /mob_templates/{vnum} {
      allow read: if request.auth != null;
      allow write: if getUserRole(request.auth.uid) in ['builder', 'immortal', 'admin'];
    }
    match /obj_templates/{vnum} {
      allow read: if request.auth != null;
      allow write: if getUserRole(request.auth.uid) in ['builder', 'immortal', 'admin'];
    }

    function getUserRole(uid) {
      return get(/databases/$(database)/documents/users/$(uid)).data.role;
    }
  }
}
```

### Step 2.3 - Data Migration Tool
- Write a Node.js script to parse the existing `.are` files and import into Firestore
  - Parse `#AREADATA`, `#MOBILES`, `#OBJECTS`, `#ROOMS`, `#RESETS`, `#SHOPS`
  - Map the Stormgate text format to the Firestore schema above
- Write a player file parser to convert `player/<letter>/Name.gz` files to Firestore character docs
- This is a one-time migration; afterwards Firestore is the source of truth

---

## Phase 3: Game Engine (Full TypeScript Port)

### Step 3.1 - Engine Architecture

The game engine is a full port of the C server to TypeScript, running as a single
long-lived Node.js process on the Oracle Cloud VM. Like the original C server, it
holds the entire game world in memory and runs a tick-based game loop. Firestore
replaces flat files as the persistence layer.

The process runs as a `systemd` service so it auto-restarts on crash or VM reboot.
Nginx/Caddy sits in front for TLS termination (Let's Encrypt), proxying WSS
connections to the Node.js process.

### Step 3.2 - Core Engine Port

Port in this order, as each layer depends on the previous:

1. **Data types** - TypeScript interfaces matching C structs (CHAR_DATA, OBJ_DATA, ROOM_INDEX_DATA, etc.)
2. **Database loader** - Read from Firestore instead of `db.c` file parsing
3. **String/Memory utilities** - Replace SSM with JS native strings
4. **Handler** - `handler.c` object/char manipulation (affect_to_char, obj_from_room, etc.)
5. **Command interpreter** - Port `interp.c` command table and dispatch
6. **Act/output** - Port `act()` function and ANSI output formatting
7. **Movement** - `act_move.c`
8. **Communication** - `act_comm.c` (say, tell, shout, channels)
9. **Info commands** - `act_info.c` (look, score, who, inventory)
10. **Object handling** - `act_obj.c` (get, drop, wear, remove)
11. **Combat** - `fight.c`, `fight2.c` (the most complex system)
12. **Magic** - `magic.c` through `magic4.c`, `gr_magic.c`
13. **Skills** - `skills.c`, `skill_table.c`, class files
14. **Update loop** - `update.c` (ticks, combat rounds, weather)
15. **Everything else** - Quests, clans, religions, economy, crafting, OLC, etc.

### Step 3.3 - Save System
- Replace `save.c` file I/O with Firestore writes
- Auto-save on a timer (existing game uses `PULSE_DB_DUMP` = 30 min)
- Save on disconnect, quit, and critical events
- Use Firestore transactions for inventory transfers between players

---

## Phase 4: Permissions & Roles

### Step 4.1 - Role Hierarchy

Map MUD immortal levels to Firebase roles:

| MUD Level | Rank | Firebase Role | Permissions |
|-----------|------|---------------|-------------|
| 1-104 | Mortal | `player` | Play the game, own characters |
| 105 | Demigod | `player` | Same as mortal (earned in-game) |
| 106 | Apprentice | `builder` | Edit assigned areas via OLC |
| 107 | Deity | `builder` | Edit areas, create objects/mobs |
| 108 | Immortal | `immortal` | All builder powers + player management |
| 109-113 | Senior Staff | `immortal` | Full game management |
| 114-115 | Admin | `admin` | Full system access, role management |

### Step 4.2 - Permission Enforcement
- **Client:** UI elements shown/hidden based on role (e.g., OLC editor only for builders)
- **Server:** Command-level permission checks (port the level checks from `interp.c` cmd_table)
- **Firestore:** Security rules enforce document-level access (see Step 2.2)
- **Role management:** Admin-only Firestore function to update user roles
- Characters owned by a user can only be played/modified by that user (or immortals+)

### Step 4.3 - Account Linking
- First login with Google creates a `users/{uid}` document
- User can create new characters (up to a configurable limit)
- Existing player files can be "claimed" by proving knowledge of the old password
- Immortal/admin can manually link legacy characters to Google accounts

---

## Phase 5: Enhanced Web Features

### Step 5.1 - Rich UI Panels
- **Character panel:** Live HP/Mana/Move bars, equipment paper-doll
- **Map panel:** Auto-map from room exits (minimap)
- **Inventory panel:** Drag-and-drop equipment management
- **Chat panels:** Separate tabs for channels (say, tell, clan, shout)
- **Combat log:** Filtered view of combat output

### Step 5.2 - Mobile Responsive
- Responsive layout that works on phones/tablets
- Touch-friendly movement buttons (N/S/E/W/U/D)
- Swipe between terminal and side panels

### Step 5.3 - OLC Web Editor (for builders)
- Visual room editor with node-graph view
- Mob/object template editors with form fields instead of text commands
- Live preview of descriptions with ANSI rendering
- Save directly to Firestore (respecting builder security levels)

---

## Tech Stack Summary

| Layer | Technology |
|-------|-----------|
| Frontend | React, TypeScript, Vite, xterm.js |
| Auth | Firebase Authentication (Google provider) |
| Database | Cloud Firestore |
| Backend | Node.js, TypeScript, Express, ws |
| Game Server | Oracle Cloud Free Tier VM (ARM A1) |
| TLS Proxy | Nginx or Caddy + Let's Encrypt |
| Process Manager | systemd |
| Static Hosting | Firebase Hosting |
| Area Migration | Node.js scripts parsing `.are` file format |

---

## Implementation Order

1. **Phase 1** (Foundation) - Get a working terminal in the browser that authenticates with Google and connects via WebSocket to the Oracle Cloud VM
2. **Phase 2** (Data Layer) - Design Firestore schema, write migration scripts to import `.are` files and player saves, set up security rules
3. **Phase 3** (Engine Port) - Port game systems from C to TypeScript in dependency order, loading/saving from Firestore, running on Oracle Cloud VM as a systemd service
4. **Phase 4** (Permissions) - Implement role system and permission checks, account linking for legacy characters
5. **Phase 5** (Enhanced UI) - Add rich panels, mobile support, visual OLC editor

## Server Deployment Checklist (Oracle Cloud VM)

1. Provision an ARM A1 instance (Ubuntu) in Oracle Cloud Always Free
2. Install Node.js (LTS), Nginx/Caddy, Certbot
3. Open ports 80, 443 in VCN security list
4. Point domain DNS A record to the VM's public IP
5. Configure Nginx/Caddy as reverse proxy: `wss://your-domain.com` -> `localhost:3000`
6. Obtain TLS cert via Let's Encrypt / Certbot
7. Place Firebase service account key on the VM (env var or file, NOT in repo)
8. Create a `systemd` unit file for the game engine (`ExecStart=node dist/server.js`)
9. Enable and start the service: `systemctl enable stormgate && systemctl start stormgate`
10. Deploy React client to Firebase Hosting: `firebase deploy --only hosting`
