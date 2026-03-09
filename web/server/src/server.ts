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
  registerConnection,
  unregisterConnection,
  sendToChar,
  startGameLoop,
  stopGameLoop,
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
  character: CharData;
}

const clients = new Map<WebSocket, ConnectedClient>();

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

  // Create a character for this session
  const charName = decoded.name || decoded.email?.split("@")[0] || "Adventurer";
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
    email: decoded.email || "",
    awins: 0,
    alosses: 0,
    aliases: new Map(),
    spell1: 0,
    spell2: 0,
    spell3: 0,
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
    pkill: false,
    shields: 0,
    questpoints: 0,
    nextquest: 0,
    countdown: 0,
    questobj: 0,
    questmob: 0,
    rquestpoints: 0,
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
    roomVnum: 25000, // Temple recall
    pcdata,
    equipment: new Map(),
    inventory: [],
    fighting: null,
  };

  // Place character in the world
  world.characters.set(charId, character);

  // If start room doesn't exist, use first available room
  if (!world.rooms.has(character.roomVnum)) {
    const firstRoom = world.rooms.keys().next().value;
    if (firstRoom !== undefined) {
      character.roomVnum = firstRoom;
    }
  }

  // Register WebSocket connection for output
  registerConnection(charId, (text: string) => {
    if (ws.readyState === WebSocket.OPEN) {
      ws.send(text);
    }
  });

  const client: ConnectedClient = {
    ws,
    uid: decoded.uid,
    email: decoded.email || "",
    displayName: charName,
    character,
  };

  clients.set(ws, client);
  console.log(`[+] ${charName} (${decoded.uid}) connected`);

  // Send welcome and initial look
  sendToChar(character, "\n\x1b[1;36m   Welcome to Stormgate!\x1b[0m\n\n");
  interpret(character, "look");

  // Handle incoming commands
  ws.on("message", (data) => {
    const message = data.toString().trim();
    if (!message) return;
    interpret(character, message);
  });

  ws.on("close", () => {
    unregisterConnection(charId);
    world.characters.delete(charId);
    clients.delete(ws);
    console.log(`[-] ${charName} disconnected`);
  });
});

// ─── Start ────────────────────────────────────────────────────────────

const PORT = parseInt(process.env.PORT || "3000", 10);

bootWorld()
  .then(() => {
    startGameLoop();
    server.listen(PORT, () => {
      console.log(`Stormgate server listening on port ${PORT}`);
    });
  })
  .catch((err) => {
    console.error("Failed to boot world:", err);
    process.exit(1);
  });

// Graceful shutdown
process.on("SIGINT", () => {
  console.log("\nShutting down...");
  stopGameLoop();
  for (const client of clients.values()) {
    sendToChar(
      client.character,
      "\n\x1b[1;31mServer shutting down. Goodbye!\x1b[0m\n"
    );
    client.ws.close();
  }
  server.close();
  process.exit(0);
});
