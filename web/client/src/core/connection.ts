import { WS_URL } from "./config";
import { getIdToken } from "./auth";
import { parseServerMessage, type GameMessage } from "./protocol";

export type ConnectionState = "disconnected" | "connecting" | "connected";

export interface GameConnection {
  send(command: string): void;
  disconnect(): void;
  onData(callback: (data: string) => void): void;
  onMessage(callback: (msg: GameMessage) => void): void;
  onStateChange(callback: (state: ConnectionState) => void): void;
}

export async function connect(): Promise<GameConnection> {
  const token = await getIdToken();
  if (!token) {
    throw new Error("Not authenticated");
  }

  const dataCallbacks: ((data: string) => void)[] = [];
  const messageCallbacks: ((msg: GameMessage) => void)[] = [];
  const stateCallbacks: ((state: ConnectionState) => void)[] = [];

  let ws: WebSocket | null = null;
  let reconnectTimer: ReturnType<typeof setTimeout> | null = null;
  let reconnectDelay = 1000;
  let intentionalClose = false;

  function setState(state: ConnectionState) {
    stateCallbacks.forEach((cb) => cb(state));
  }

  function doConnect() {
    setState("connecting");

    // Pass token as a query parameter for auth
    const url = `${WS_URL}?token=${encodeURIComponent(token)}`;
    ws = new WebSocket(url);

    ws.onopen = () => {
      reconnectDelay = 1000;
      setState("connected");
    };

    ws.onmessage = (event) => {
      const raw = event.data as string;
      const parsed = parseServerMessage(raw);

      if (parsed.type === 'text') {
        // Raw text — pass to terminal data callbacks
        dataCallbacks.forEach((cb) => cb(parsed.data));
      } else {
        // Structured message — pass to message callbacks
        messageCallbacks.forEach((cb) => cb(parsed));
      }
    };

    ws.onclose = () => {
      setState("disconnected");
      if (!intentionalClose) {
        reconnectTimer = setTimeout(() => {
          reconnectDelay = Math.min(reconnectDelay * 2, 30000);
          doConnect();
        }, reconnectDelay);
      }
    };

    ws.onerror = () => {
      // onclose will fire after this
    };
  }

  doConnect();

  return {
    send(command: string) {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(command);
      }
    },
    disconnect() {
      intentionalClose = true;
      if (reconnectTimer) clearTimeout(reconnectTimer);
      if (ws) ws.close();
    },
    onData(callback: (data: string) => void) {
      dataCallbacks.push(callback);
    },
    onMessage(callback: (msg: GameMessage) => void) {
      messageCallbacks.push(callback);
    },
    onStateChange(callback: (state: ConnectionState) => void) {
      stateCallbacks.push(callback);
    },
  };
}
