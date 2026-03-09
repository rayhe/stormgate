/**
 * protocol.ts — Structured data protocol for Stormgate MUD.
 *
 * Defines message types for structured communication between server and client.
 * The server can send raw ANSI text (backward compatible) or JSON messages
 * (starting with '{') that carry structured game data for rich UI panels.
 */

// ============================================================================
//  Message types from server to client
// ============================================================================

export interface GameMessage {
  type: 'text' | 'vitals' | 'room' | 'who' | 'combat' | 'channel' | 'charInfo';
  data: any;
}

export interface VitalsData {
  hp: number;
  maxHp: number;
  mana: number;
  maxMana: number;
  move: number;
  maxMove: number;
  gold: number;
  level: number;
  position: string;
}

export interface RoomData {
  name: string;
  description: string;
  exits: string[];
  items: string[];
  characters: string[];
  vnum: number;
}

export interface CombatData {
  attacker: string;
  target: string;
  damage: number;
  message: string;
  attackerHpPct: number;
  targetHpPct: number;
}

export interface ChannelMessage {
  channel: string;
  sender: string;
  message: string;
  timestamp: number;
}

export interface CharInfoData {
  name: string;
  level: number;
  class: string;
  race: string;
  hp: number;
  maxHp: number;
  mana: number;
  maxMana: number;
  move: number;
  maxMove: number;
  gold: number;
  exp: number;
  hitroll: number;
  damroll: number;
  armor: number;
  alignment: number;
  position: string;
}

export interface WhoEntry {
  name: string;
  level: number;
  class: string;
  title: string;
}

// ============================================================================
//  Parser
// ============================================================================

/**
 * Parse a raw WebSocket message from the server.
 *
 * Messages starting with '{' are treated as JSON structured messages.
 * Everything else is treated as raw ANSI text for the terminal.
 */
export function parseServerMessage(raw: string): GameMessage {
  if (raw.startsWith('{')) {
    try {
      return JSON.parse(raw) as GameMessage;
    } catch {
      // If JSON parsing fails, treat as raw text
      return { type: 'text', data: raw };
    }
  }
  return { type: 'text', data: raw };
}
