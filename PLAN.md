# Stormgate Web Client - Implementation Plan

## Goal

Build a web-based MUD client for Stormgate that uses Google Authentication for login and Firebase for data persistence with permission-based access control.

---

## Architecture Overview

```
┌─────────────────────────────────────────────────┐
│                  Frontend (React)                │
│  ┌───────────┐ ┌──────────┐ ┌────────────────┐  │
│  │ Terminal   │ │ Map View │ │ Character Panel│  │
│  │ (xterm.js) │ │(optional)│ │ (stats/inv)    │  │
│  └─────┬─────┘ └──────────┘ └────────────────┘  │
│        │                                         │
│  ┌─────┴──────────────────────────────────────┐  │
│  │         WebSocket Client Layer             │  │
│  └─────┬──────────────────────────────────────┘  │
│        │                                         │
│  ┌─────┴──────────────────────────────────────┐  │
│  │    Firebase Auth (Google Sign-In)          │  │
│  └────────────────────────────────────────────┘  │
└────────┬────────────────────────────────────────┘
         │ WSS
┌────────┴────────────────────────────────────────┐
│              Backend (Node.js)                   │
│  ┌────────────────────────────────────────────┐  │
│  │     WebSocket Server (ws)                  │  │
│  ├────────────────────────────────────────────┤  │
│  │     Firebase Admin SDK                     │  │
│  │     - Auth token verification              │  │
│  │     - Firestore read/write                 │  │
│  ├────────────────────────────────────────────┤  │
│  │     Game Engine (ported from C)            │  │
│  │     OR Telnet Proxy to existing server     │  │
│  └────────────────────────────────────────────┘  │
└─────────────────┬───────────────────────────────┘
                  │
┌─────────────────┴───────────────────────────────┐
│              Firebase                            │
│  ┌──────────────┐  ┌──────────────────────────┐  │
│  │ Auth         │  │ Firestore                │  │
│  │ (Google SSO) │  │ - players/{uid}          │  │
│  │              │  │ - characters/{charId}    │  │
│  │              │  │ - areas/{areaName}       │  │
│  │              │  │ - world_state/           │  │
│  └──────────────┘  └──────────────────────────┘  │
└──────────────────────────────────────────────────┘
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

## Phase 3: Game Engine (Server-Side)

### Step 3.1 - Strategy Decision

**Option A: Telnet Proxy (quickest path)**
- Keep the existing C server running
- Node.js server acts as a WebSocket-to-Telnet proxy
- Auth happens at the proxy layer; proxy connects to C server on behalf of authenticated user
- Firebase stores supplementary data (account linking, preferences)
- Pros: Minimal game logic rewrite
- Cons: Still need the C server running, limited integration with Firebase

**Option B: TypeScript Game Engine (recommended for full web version)**
- Port the game logic to TypeScript/Node.js
- Load world data from Firestore instead of `.are` files
- Save player data to Firestore instead of flat files
- Full integration with Firebase Auth (Google account = MUD account)
- Pros: Single stack, native Firebase integration, modern tooling
- Cons: Significant porting effort

**Recommended: Start with Option A, incrementally migrate to Option B.**

### Step 3.2 - Core Engine Port (if Option B)

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
| Hosting | Firebase Hosting (client), Cloud Run or a VPS (server) |
| Area Migration | Node.js scripts parsing `.are` file format |

---

## Implementation Order

1. **Phase 1** (Foundation) - Get a working terminal in the browser that authenticates with Google and connects via WebSocket
2. **Phase 2** (Data Layer) - Design Firestore schema, write migration scripts, set up security rules
3. **Phase 3.1** (Proxy) - Connect the web client to the existing C server via a telnet proxy for immediate playability
4. **Phase 4** (Permissions) - Implement role system and permission checks
5. **Phase 3.2+** (Engine Port) - Incrementally port game systems from C to TypeScript, switching each system to use Firestore
6. **Phase 5** (Enhanced UI) - Add rich panels, mobile support, visual OLC editor
