import express from "express";
import { createServer } from "http";
import { WebSocketServer, WebSocket } from "ws";
import { initializeApp, cert, type ServiceAccount } from "firebase-admin/app";
import { getAuth } from "firebase-admin/auth";
import { getFirestore } from "firebase-admin/firestore";
import { readFileSync, existsSync } from "fs";
import { URL } from "url";

// ─── Firebase Admin Setup ─────────────────────────────────────────────

const serviceAccountPath = process.env.GOOGLE_APPLICATION_CREDENTIALS
  || "./service-account.json";

if (existsSync(serviceAccountPath)) {
  const sa = JSON.parse(readFileSync(serviceAccountPath, "utf-8")) as ServiceAccount;
  initializeApp({ credential: cert(sa) });
} else {
  // Falls back to GOOGLE_APPLICATION_CREDENTIALS env or default credentials
  initializeApp();
}

const auth = getAuth();
const db = getFirestore();

// ─── Express + HTTP Server ────────────────────────────────────────────

const app = express();
app.get("/health", (_req, res) => {
  res.json({ status: "ok", connections: clients.size });
});

const server = createServer(app);

// ─── WebSocket Server ─────────────────────────────────────────────────

const wss = new WebSocketServer({ server, path: "/ws" });

interface AuthenticatedClient {
  ws: WebSocket;
  uid: string;
  email: string;
  displayName: string;
}

const clients = new Map<WebSocket, AuthenticatedClient>();

wss.on("connection", async (ws, req) => {
  // Extract token from query string
  const url = new URL(req.url || "/", `http://${req.headers.host}`);
  const token = url.searchParams.get("token");

  if (!token) {
    ws.close(4001, "Missing auth token");
    return;
  }

  // Verify Firebase ID token
  let decoded;
  try {
    decoded = await auth.verifyIdToken(token);
  } catch {
    ws.close(4003, "Invalid auth token");
    return;
  }

  const client: AuthenticatedClient = {
    ws,
    uid: decoded.uid,
    email: decoded.email || "",
    displayName: decoded.name || decoded.email || decoded.uid,
  };

  clients.set(ws, client);
  console.log(`[+] ${client.displayName} (${client.uid}) connected`);

  // Ensure user doc exists in Firestore
  const userRef = db.collection("users").doc(decoded.uid);
  const userSnap = await userRef.get();
  if (!userSnap.exists) {
    await userRef.set({
      email: client.email,
      displayName: client.displayName,
      role: "player",
      createdAt: new Date(),
      lastLogin: new Date(),
      characterIds: [],
    });
  } else {
    await userRef.update({ lastLogin: new Date() });
  }

  // Send welcome message
  ws.send(formatMudOutput([
    "",
    "\x1b[1;36m   Welcome to Stormgate!\x1b[0m",
    "",
    `   Logged in as \x1b[1;33m${client.displayName}\x1b[0m`,
    "",
    "\x1b[0;37m   Type 'help' for a list of commands.\x1b[0m",
    "",
  ]));

  // Handle incoming commands
  ws.on("message", (data) => {
    const message = data.toString().trim();
    if (!message) return;
    handleCommand(client, message);
  });

  ws.on("close", () => {
    clients.delete(ws);
    console.log(`[-] ${client.displayName} disconnected`);
  });
});

// ─── Command Handler (stub — will be replaced by game engine) ─────────

function handleCommand(client: AuthenticatedClient, command: string) {
  const cmd = command.toLowerCase().split(" ")[0];

  switch (cmd) {
    case "help":
      client.ws.send(formatMudOutput([
        "",
        "\x1b[1;33mAvailable commands:\x1b[0m",
        "  help        - Show this help",
        "  look        - Look around",
        "  who         - Show connected players",
        "  say <msg>   - Say something",
        "  quit        - Disconnect",
        "",
        "\x1b[0;37mFull game engine coming soon.\x1b[0m",
        "",
      ]));
      break;

    case "look":
      client.ws.send(formatMudOutput([
        "",
        "\x1b[1;36mThe Void\x1b[0m",
        "\x1b[0;37mYou float in an endless expanse of darkness. The game world",
        "has not yet been loaded. In the distance, you can sense the",
        "framework of rooms and corridors waiting to be brought to life.\x1b[0m",
        "",
        "\x1b[0;32m[Exits: none]\x1b[0m",
        "",
      ]));
      break;

    case "who":
      const lines = [
        "",
        "\x1b[1;33mPlayers Online:\x1b[0m",
      ];
      for (const c of clients.values()) {
        lines.push(`  \x1b[1;37m${c.displayName}\x1b[0m`);
      }
      lines.push("");
      lines.push(`\x1b[0;37m${clients.size} player(s) connected.\x1b[0m`);
      lines.push("");
      client.ws.send(formatMudOutput(lines));
      break;

    case "say": {
      const msg = command.slice(4).trim();
      if (!msg) {
        client.ws.send(formatMudOutput(["Say what?"]));
        break;
      }
      // Broadcast to all clients
      const output = `\x1b[1;37m${client.displayName}\x1b[0;37m says '\x1b[1;32m${msg}\x1b[0;37m'\x1b[0m`;
      for (const c of clients.values()) {
        c.ws.send(formatMudOutput([output]));
      }
      break;
    }

    case "quit":
      client.ws.send(formatMudOutput([
        "",
        "\x1b[1;36mFarewell, adventurer.\x1b[0m",
        "",
      ]));
      client.ws.close();
      break;

    default:
      client.ws.send(formatMudOutput([
        `\x1b[0;37mHuh? Type 'help' for available commands.\x1b[0m`,
      ]));
  }
}

function formatMudOutput(lines: string[]): string {
  return lines.join("\n");
}

// ─── Start ────────────────────────────────────────────────────────────

const PORT = parseInt(process.env.PORT || "3000", 10);
server.listen(PORT, () => {
  console.log(`Stormgate server listening on port ${PORT}`);
});
