import express from "express";
import { createServer } from "http";
import { WebSocketServer, WebSocket } from "ws";
import { initializeApp, cert, type ServiceAccount } from "firebase-admin/app";
import { getAuth } from "firebase-admin/auth";
import { getFirestore } from "firebase-admin/firestore";
import { readFileSync, existsSync } from "fs";
import { URL } from "url";
import {
  world,
  interpret,
  charToRoom,
  charFromRoom,
  registerConnection,
  unregisterConnection,
  sendToChar,
  startGameLoop,
  stopGameLoop,
  initSaveSystem,
  initAdminSystem,
  setCharOwnerMap,
  saveCharacter,
  loadCharacter,
  listCharacters,
  startAutoSave,
  stopAutoSave,
  saveAllCharacters,
  charOwnerMap,
  loadResets,
  resetAllAreas,
  rebuildMobCounts,
  registerAllSpells,
  type CharData,
  type PcData,
  Position,
  Sex,
} from "./engine/index.js";

// ─── Firebase Admin Setup ─────────────────────────────────────────────

const serviceAccountPath =
  process.env.GOOGLE_APPLICATION_CREDENTIALS || "./service-account.json";

if (existsSync(serviceAccountPath)) {
  const sa = JSON.parse(
    readFileSync(serviceAccountPath, "utf-8")
  ) as ServiceAccount;
  initializeApp({ credential: cert(sa) });
} else {
  initializeApp();
}

const auth = getAuth();
const db = getFirestore();

// ─── Initialize persistence and admin systems ─────────────────────────

initSaveSystem(db);
initAdminSystem(db);
setCharOwnerMap(charOwnerMap);

// ─── Class name lookup (for character selection menu) ─────────────────

const CLASS_NAMES: Record<number, string> = {
  0: "Mage", 1: "Cleric", 2: "Thief", 3: "Warrior", 4: "Psionicist",
  5: "Druid", 6: "Ranger", 7: "Paladin", 8: "Bard", 9: "Vampire",
  10: "Werewolf", 11: "Anti-Paladin", 12: "Assassin", 13: "Monk",
  14: "Barbarian", 15: "Illusionist", 16: "Necromancer", 17: "Demonologist",
  18: "Shaman", 19: "Darkpriest",
};

// ─── Boot the Game World ──────────────────────────────────────────────

async function bootWorld() {
  console.log("[boot] Loading world data from Firestore...");

  // Load areas
  const areasSnap = await db.collection("areas").get();
  for (const doc of areasSnap.docs) {
    const data = doc.data();
    world.areas.set(doc.id, {
      name: data.name || doc.id,
      filename: data.filename || `${doc.id}.are`,
      lvnum: data.vnumRange?.low || 0,
      uvnum: data.vnumRange?.high || 0,
      vnum: data.vnum || 0,
      security: data.security || 0,
      builders: data.builders || "",
      recall: data.recall || 25000,
      areaFlags: data.flags || 0,
      version: data.version || 0,
      creator: data.creator || "",
      llevel: data.llevel || 0,
      ulevel: data.ulevel || 0,
    });

    // Load rooms for this area
    const roomsSnap = await db
      .collection("areas")
      .doc(doc.id)
      .collection("rooms")
      .get();
    for (const roomDoc of roomsSnap.docs) {
      const r = roomDoc.data();
      world.rooms.set(Number(roomDoc.id), {
        vnum: Number(roomDoc.id),
        name: r.name || "An empty room",
        description: r.description || "",
        sectorType: r.sectorType || 0,
        roomFlags: r.roomFlags || 0,
        light: r.light || 0,
        exits: r.exits || {},
        extraDescriptions: r.extraDescriptions || [],
        timedRoomFlags: 0,
        flagTimer: 0,
        areaKey: doc.id,
      });
    }
  }

  // Load mob templates
  const mobsSnap = await db.collection("mob_templates").get();
  for (const doc of mobsSnap.docs) {
    const m = doc.data();
    world.mobTemplates.set(Number(doc.id), m as any);
  }

  // Load obj templates
  const objsSnap = await db.collection("obj_templates").get();
  for (const doc of objsSnap.docs) {
    const o = doc.data();
    world.objTemplates.set(Number(doc.id), o as any);
  }

  // Load shops
  const shopsDoc = await db.collection("world_state").doc("shops").get();
  if (shopsDoc.exists) {
    const shopsData = shopsDoc.data();
    if (shopsData?.shops) {
      for (const shop of shopsData.shops) {
        world.shops.set(shop.keeperVnum, shop);
      }
    }
  }

  // Load resets
  const resetsDoc = await db.collection("world_state").doc("resets").get();
  if (resetsDoc.exists) {
    const resetsData = resetsDoc.data();
    if (resetsData) {
      loadResets(resetsData as Record<string, any[]>);
    }
  }

  console.log(
    `[boot] World loaded: ${world.rooms.size} rooms, ${world.mobTemplates.size} mob templates, ${world.objTemplates.size} obj templates, ${world.areas.size} areas`
  );

  // If no rooms loaded, create a minimal void room so players can connect
  if (world.rooms.size === 0) {
    console.log("[boot] No rooms found, creating void room at vnum 2");
    world.rooms.set(2, {
      vnum: 2,
      name: "The Void",
      description:
        "You float in an endless expanse of darkness. The game world\nhas not yet been migrated to Firestore. Run the migration\nscript to load area data.",
      sectorType: 0,
      roomFlags: 0,
      light: 0,
      exits: {},
      extraDescriptions: [],
      timedRoomFlags: 0,
      flagTimer: 0,
      areaKey: "limbo",
    });
  }

  // Register all spell functions into the skill table
  console.log("[boot] Registering spell/skill implementations...");
  registerAllSpells();

  // Run initial area resets to populate the world with mobs and objects
  console.log("[boot] Running initial area resets...");
  resetAllAreas();
  rebuildMobCounts();
  console.log("[boot] Initial reset complete.");
}

// ─── Express + HTTP Server ────────────────────────────────────────────

const app = express();
app.get("/health", (_req, res) => {
  res.json({
    status: "ok",
    connections: clients.size,
    rooms: world.rooms.size,
    uptime: process.uptime(),
  });
});

const server = createServer(app);

// ─── WebSocket Server ─────────────────────────────────────────────────

const wss = new WebSocketServer({ server, path: "/ws" });

interface ConnectedClient {
  ws: WebSocket;
  uid: string;
  email: string;
  displayName: string;
  character: CharData | null;
  state: "selecting" | "playing";
}

const clients = new Map<WebSocket, ConnectedClient>();

// ─── Helper: send raw text to a WebSocket ─────────────────────────────

function wsSend(ws: WebSocket, text: string): void {
  if (ws.readyState === WebSocket.OPEN) {
    ws.send(text);
  }
}

// ─── Helper: create a fresh character for a new player ────────────────

function createNewCharacter(
  ws: WebSocket,
  uid: string,
  displayName: string,
  email: string
): CharData {
  const charName = displayName || email?.split("@")[0] || "Adventurer";
  const charId = world.nextId();

  const pcdata: PcData = {
    pwd: "",
    title: " the Adventurer",
    prompt: "<%h/%Hhp %m/%Mmn %v/%Vmv> ",
    lname: "",
    bamfin: "",
    bamfout: "",
    bamfsin: "",
    bamfsout: "",
    whoText: "",
    spouse: "",
    recall: 25000,
    permStr: 13,
    permInt: 13,
    permWis: 13,
    permDex: 13,
    permCon: 13,
    modStr: 0,
    modInt: 0,
    modWis: 0,
    modDex: 0,
    modCon: 0,
    condition: [0, 48, 48],
    learned: new Map(),
    security: 0,
    bankAccount: 0,
    pagelen: 24,
    shares: 0,
    mobkills: 0,
    corpses: 0,
    plan: "",
    email: email || "",
    awins: 0,
    alosses: 0,
    aliases: new Map(),
    spellSlots: {
      spell1: 0,
      spell2: 0,
      spell3: 0,
    },
    craftTimer: 0,
    craftType: 0,
  };

  const character: CharData = {
    id: charId,
    name: charName,
    shortDescr: charName,
    longDescr: `${charName} is standing here.`,
    description: "",
    prompt: pcdata.prompt,
    sex: Sex.NEUTRAL,
    charClass: 3, // Warrior
    race: 0,
    level: 1,
    trust: 0,
    exp: 0,
    gold: 100,
    hit: 100,
    maxHit: 100,
    mana: 100,
    maxMana: 100,
    move: 100,
    maxMove: 100,
    bp: 0,
    maxBp: 0,
    position: Position.STANDING,
    practice: 5,
    alignment: 0,
    hitroll: 0,
    damroll: 0,
    armor: 100,
    wimpy: 0,
    savingThrow: 0,
    affectedBy: 0,
    affectedBy2: 0,
    affectedBy3: 0,
    affectedBy4: 0,
    act: 0,
    act2: 0,
    clan: 0,
    religion: 0,
    clev: 0,
    language: new Array(27).fill(0),
    speaking: 0,
    size: 2,
    pkill: 0,
    shields: 0,
    questpoints: 0,
    nextquest: 0,
    countdown: 0,
    questobj: 0,
    questmob: 0,
    rquestpoints: 0,
    isQuestor: false,
    questGiver: '',
    questArea: '',
    questRoom: '',
    questFetchItem: '',
    vnum: 0,
    learn: 0,
    combatTimer: 0,
    summonTimer: 0,
    poisonLevel: 0,
    damageMods: new Array(21).fill(0),
    mounted: 0,
    affects: [],
    affects2: [],
    affects3: [],
    affects4: [],
    carryWeight: 0,
    carryNumber: 0,
    deleted: false,
    isNpc: false,
  };

  // Place character in the world
  world.characters.set(charId, character);
  world.pcData.set(charId, pcdata);

  // Determine starting room
  const startVnum = 25000;
  const targetRoom = world.getRoom(startVnum);
  const roomVnum = targetRoom
    ? startVnum
    : world.rooms.keys().next().value ?? 2;
  charToRoom(character, roomVnum);

  // Track ownership for save system
  charOwnerMap.set(charId, uid);

  // Do an initial save so the character shows up in future listings
  saveCharacter(character, uid).catch((err) => {
    console.error(`Failed to save new character ${charName}:`, err);
  });

  return character;
}

// ─── Helper: enter the game with a character ──────────────────────────

function enterGame(client: ConnectedClient, character: CharData): void {
  client.character = character;
  client.state = "playing";

  // Register WebSocket connection for output
  registerConnection(character.id, (text: string) => {
    wsSend(client.ws, text);
  });

  console.log(`[+] ${character.name} (${client.uid}) entered the game`);

  // Send welcome and initial look
  sendToChar(character, "\n\x1b[1;36m   Welcome to Stormgate!\x1b[0m\n\n");
  interpret(character, "look");
}

// ─── WebSocket connection handler ─────────────────────────────────────

wss.on("connection", async (ws, req) => {
  const url = new URL(req.url || "/", `http://${req.headers.host}`);
  const token = url.searchParams.get("token");

  if (!token) {
    ws.close(4001, "Missing auth token");
    return;
  }

  let decoded;
  try {
    decoded = await auth.verifyIdToken(token);
  } catch {
    ws.close(4003, "Invalid auth token");
    return;
  }

  // Ensure user doc exists
  const userRef = db.collection("users").doc(decoded.uid);
  const userSnap = await userRef.get();
  if (!userSnap.exists) {
    await userRef.set({
      email: decoded.email || "",
      displayName: decoded.name || decoded.email || decoded.uid,
      role: "player",
      createdAt: new Date(),
      lastLogin: new Date(),
      characterIds: [],
    });
  } else {
    await userRef.update({ lastLogin: new Date() });
  }

  const displayName =
    decoded.name || decoded.email?.split("@")[0] || "Adventurer";

  // Create the client in 'selecting' state
  const client: ConnectedClient = {
    ws,
    uid: decoded.uid,
    email: decoded.email || "",
    displayName,
    character: null,
    state: "selecting",
  };

  clients.set(ws, client);
  console.log(`[ws] ${displayName} (${decoded.uid}) connected, checking characters...`);

  // Check for existing characters
  let savedChars: Array<{ id: string; name: string; level: number; class: number }>;
  try {
    savedChars = await listCharacters(decoded.uid);
  } catch (err) {
    console.error("Failed to list characters:", err);
    savedChars = [];
  }

  if (savedChars.length === 0) {
    // No saved characters — create one automatically
    const character = createNewCharacter(
      ws,
      decoded.uid,
      displayName,
      decoded.email || ""
    );
    enterGame(client, character);
  } else {
    // Show character selection menu
    let menu = "\n\x1b[1;36m   Welcome to Stormgate!\x1b[0m\n\n";
    menu += "\x1b[1;33mSelect a character:\x1b[0m\r\n\r\n";

    for (let i = 0; i < savedChars.length; i++) {
      const sc = savedChars[i];
      const className = CLASS_NAMES[sc.class] ?? "Unknown";
      menu += `  \x1b[1;37m${i + 1}\x1b[0m. ${sc.name} (Level ${sc.level} ${className})\r\n`;
    }

    menu += `  \x1b[1;37m${savedChars.length + 1}\x1b[0m. Create new character\r\n`;
    menu += "\r\nEnter your choice: ";

    wsSend(ws, menu);

    // Store the saved character list for the selection handler
    (client as any)._savedChars = savedChars;
  }

  // Handle incoming messages
  ws.on("message", async (data) => {
    const message = data.toString().trim();
    if (!message) return;

    const currentClient = clients.get(ws);
    if (!currentClient) return;

    if (currentClient.state === "selecting") {
      // Handle character selection
      const savedChars: Array<{
        id: string;
        name: string;
        level: number;
        class: number;
      }> = (currentClient as any)._savedChars || [];

      const choice = parseInt(message, 10);

      if (isNaN(choice) || choice < 1 || choice > savedChars.length + 1) {
        wsSend(ws, "Invalid choice. Please enter a number.\r\nEnter your choice: ");
        return;
      }

      if (choice === savedChars.length + 1) {
        // Create new character
        const character = createNewCharacter(
          ws,
          currentClient.uid,
          currentClient.displayName,
          currentClient.email
        );
        delete (currentClient as any)._savedChars;
        enterGame(currentClient, character);
        return;
      }

      // Load the selected character
      const selected = savedChars[choice - 1];
      wsSend(ws, `\r\nLoading ${selected.name}...\r\n`);

      try {
        const loaded = await loadCharacter(selected.id);
        if (!loaded) {
          wsSend(ws, "\x1b[1;31mFailed to load character. Creating a new one...\x1b[0m\r\n");
          const character = createNewCharacter(
            ws,
            currentClient.uid,
            currentClient.displayName,
            currentClient.email
          );
          delete (currentClient as any)._savedChars;
          enterGame(currentClient, character);
          return;
        }

        // Track ownership
        charOwnerMap.set(loaded.char.id, currentClient.uid);

        delete (currentClient as any)._savedChars;
        enterGame(currentClient, loaded.char);
      } catch (err) {
        console.error(`Failed to load character ${selected.id}:`, err);
        wsSend(ws, "\x1b[1;31mError loading character. Creating a new one...\x1b[0m\r\n");
        const character = createNewCharacter(
          ws,
          currentClient.uid,
          currentClient.displayName,
          currentClient.email
        );
        delete (currentClient as any)._savedChars;
        enterGame(currentClient, character);
      }
    } else {
      // Normal gameplay — dispatch to interpreter
      if (currentClient.character) {
        interpret(currentClient.character, message);
      }
    }
  });

  ws.on("close", async () => {
    const closingClient = clients.get(ws);
    if (closingClient?.character) {
      const ch = closingClient.character;

      // Save before removing
      const uid = charOwnerMap.get(ch.id);
      if (uid) {
        try {
          await saveCharacter(ch, uid);
          console.log(`[save] ${ch.name} saved on disconnect.`);
        } catch (err) {
          console.error(`Failed to save ${ch.name} on disconnect:`, err);
        }
      }

      unregisterConnection(ch.id);
      charFromRoom(ch);
      ch.deleted = true;
      world.characters.delete(ch.id);
      world.pcData.delete(ch.id);
      charOwnerMap.delete(ch.id);
      console.log(`[-] ${ch.name} disconnected`);
    } else {
      console.log(`[-] ${closingClient?.displayName ?? "Unknown"} disconnected (no character)`);
    }
    clients.delete(ws);
  });
});

// ─── Start ────────────────────────────────────────────────────────────

const PORT = parseInt(process.env.PORT || "3000", 10);

bootWorld()
  .then(() => {
    startGameLoop();
    startAutoSave();
    server.listen(PORT, () => {
      console.log(`Stormgate server listening on port ${PORT}`);
    });
  })
  .catch((err) => {
    console.error("Failed to boot world:", err);
    process.exit(1);
  });

// Graceful shutdown
process.on("SIGINT", async () => {
  console.log("\nShutting down...");
  stopGameLoop();
  stopAutoSave();

  // Save all characters before shutting down
  try {
    await saveAllCharacters();
    console.log("[shutdown] All characters saved.");
  } catch (err) {
    console.error("[shutdown] Error saving characters:", err);
  }

  for (const client of clients.values()) {
    if (client.character) {
      sendToChar(
        client.character,
        "\n\x1b[1;31mServer shutting down. Your character has been saved. Goodbye!\x1b[0m\n"
      );
    }
    client.ws.close();
  }
  server.close();
  process.exit(0);
});
