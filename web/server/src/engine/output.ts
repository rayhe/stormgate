/**
 * output.ts — Output formatting for Stormgate MUD.
 *
 * Port of act() / send_to_char() / send_to_room() from comm.c / act_comm.c.
 * Handles the MUD's classic $-token substitution system and ANSI color output.
 *
 * The connection registry maps character IDs to WebSocket send functions,
 * allowing the engine to push text to connected clients.
 */

import type { CharData } from './types.js';
import { Sex } from './types.js';
import { world, charRoomMap } from './world.js';
import { capitalize } from './handler.js';

// ============================================================================
//  ANSI color codes
// ============================================================================

export const colors = {
  reset: '\x1b[0m',
  bold: '\x1b[1m',
  red: '\x1b[31m',
  green: '\x1b[32m',
  yellow: '\x1b[33m',
  blue: '\x1b[34m',
  magenta: '\x1b[35m',
  cyan: '\x1b[36m',
  white: '\x1b[37m',
  brightRed: '\x1b[91m',
  brightGreen: '\x1b[92m',
  brightYellow: '\x1b[93m',
  brightBlue: '\x1b[94m',
  brightMagenta: '\x1b[95m',
  brightCyan: '\x1b[96m',
  brightWhite: '\x1b[97m',
};

// ============================================================================
//  Connection registry
// ============================================================================

/**
 * Maps character ID to a function that sends text to their WebSocket client.
 */
const connectionRegistry = new Map<string, (text: string) => void>();

/**
 * Register a connection for a character, so sendToChar can reach them.
 */
export function registerConnection(charId: string, sender: (text: string) => void): void {
  connectionRegistry.set(charId, sender);
}

/**
 * Unregister a character's connection (on disconnect or quit).
 */
export function unregisterConnection(charId: string): void {
  connectionRegistry.delete(charId);
}

/**
 * Check if a character has an active connection.
 */
export function hasConnection(charId: string): boolean {
  return connectionRegistry.has(charId);
}

// ============================================================================
//  Core send functions
// ============================================================================

/**
 * Send text to a single character's connected client.
 * No-op if the character has no connection (NPC, or player not connected).
 */
export function sendToChar(ch: CharData, text: string): void {
  const sender = connectionRegistry.get(ch.id);
  if (sender) {
    sender(text);
  }
}

/**
 * Send text to all characters in a room, optionally excluding one.
 */
export function sendToRoom(vnum: number, text: string, exclude?: CharData): void {
  const chars = world.getCharsInRoom(vnum);
  for (const ch of chars) {
    if (exclude && ch.id === exclude.id) continue;
    sendToChar(ch, text);
  }
}

/**
 * Send text to all connected players.
 */
export function sendToAll(text: string, exclude?: CharData): void {
  for (const ch of world.characters.values()) {
    if (ch.isNpc || ch.deleted) continue;
    if (exclude && ch.id === exclude.id) continue;
    sendToChar(ch, text);
  }
}

// ============================================================================
//  act() — MUD message formatting with $-token substitution
// ============================================================================

/** Target type constants for act(). */
export const TO_ROOM = 0;
export const TO_CHAR = 1;
export const TO_VICT = 2;
export const TO_NOTVICT = 3;

/**
 * The classic MUD act() function.
 *
 * Formats a message using $-tokens and sends it to the appropriate targets.
 *
 * Tokens:
 *   $n — ch's short description (NPC) or name (PC)
 *   $N — victim's short description (NPC) or name (PC)
 *   $e — he/she/it for ch
 *   $E — he/she/it for victim
 *   $m — him/her/it for ch
 *   $M — him/her/it for victim
 *   $s — his/her/its for ch
 *   $S — his/her/its for victim
 *   $p — arg1 object short description
 *   $P — arg2 object short description
 *   $t — arg1 as string
 *   $T — arg2 as string
 *
 * @param format  The format string with $-tokens.
 * @param ch      The acting character.
 * @param arg1    First argument (object, string, etc.).
 * @param arg2    Second argument (victim character, object, string, etc.).
 * @param toType  Who receives the message: TO_ROOM, TO_CHAR, TO_VICT, TO_NOTVICT.
 */
export function act(
  format: string,
  ch: CharData,
  arg1: unknown,
  arg2: unknown,
  toType: number
): void {
  if (!format) return;

  const vict = (toType === TO_VICT || toType === TO_NOTVICT)
    ? (arg2 as CharData | null)
    : null;

  // Determine the list of recipients
  const recipients: CharData[] = [];
  const chRoom = charRoomMap.get(ch.id);

  switch (toType) {
    case TO_CHAR:
      recipients.push(ch);
      break;

    case TO_VICT:
      if (vict && vict.id !== ch.id) {
        recipients.push(vict);
      }
      break;

    case TO_ROOM:
      if (chRoom !== undefined) {
        for (const other of world.getCharsInRoom(chRoom)) {
          if (other.id !== ch.id && !other.deleted) {
            recipients.push(other);
          }
        }
      }
      break;

    case TO_NOTVICT:
      if (chRoom !== undefined) {
        for (const other of world.getCharsInRoom(chRoom)) {
          if (other.id !== ch.id && (!vict || other.id !== vict.id) && !other.deleted) {
            recipients.push(other);
          }
        }
      }
      break;
  }

  // Format and send to each recipient
  for (const to of recipients) {
    const message = formatAct(format, ch, arg1, arg2, to);
    sendToChar(to, capitalize(message) + '\r\n');
  }

  // For TO_CHAR, also send (message goes to ch)
  if (toType === TO_CHAR) {
    // Already added ch to recipients above; message was sent in the loop.
  }
}

/**
 * Format a single act() string for a specific viewer.
 */
function formatAct(
  format: string,
  ch: CharData,
  arg1: unknown,
  arg2: unknown,
  _to: CharData
): string {
  let result = '';
  let i = 0;

  while (i < format.length) {
    if (format[i] === '$' && i + 1 < format.length) {
      i++;
      const token = format[i];
      switch (token) {
        case 'n':
          result += getCharName(ch);
          break;
        case 'N':
          result += arg2 ? getCharName(arg2 as CharData) : 'someone';
          break;
        case 'e':
          result += heSheIt(ch);
          break;
        case 'E':
          result += arg2 ? heSheIt(arg2 as CharData) : 'it';
          break;
        case 'm':
          result += himHerIt(ch);
          break;
        case 'M':
          result += arg2 ? himHerIt(arg2 as CharData) : 'it';
          break;
        case 's':
          result += hisHerIts(ch);
          break;
        case 'S':
          result += arg2 ? hisHerIts(arg2 as CharData) : 'its';
          break;
        case 'p':
          result += arg1 ? (arg1 as { shortDescr?: string }).shortDescr ?? 'something' : 'something';
          break;
        case 'P':
          result += arg2 ? (arg2 as { shortDescr?: string }).shortDescr ?? 'something' : 'something';
          break;
        case 't':
          result += typeof arg1 === 'string' ? arg1 : 'something';
          break;
        case 'T':
          result += typeof arg2 === 'string' ? arg2 : 'something';
          break;
        case '$':
          result += '$';
          break;
        default:
          result += '$' + token;
          break;
      }
      i++;
    } else {
      result += format[i];
      i++;
    }
  }

  return result;
}

// ============================================================================
//  Pronoun helpers
// ============================================================================

function getCharName(ch: CharData): string {
  return ch.isNpc ? ch.shortDescr : ch.name;
}

function heSheIt(ch: CharData): string {
  switch (ch.sex) {
    case Sex.MALE:   return 'he';
    case Sex.FEMALE: return 'she';
    default:         return 'it';
  }
}

function himHerIt(ch: CharData): string {
  switch (ch.sex) {
    case Sex.MALE:   return 'him';
    case Sex.FEMALE: return 'her';
    default:         return 'it';
  }
}

function hisHerIts(ch: CharData): string {
  switch (ch.sex) {
    case Sex.MALE:   return 'his';
    case Sex.FEMALE: return 'her';
    default:         return 'its';
  }
}

// ============================================================================
//  Prompt rendering
// ============================================================================

/**
 * Render the MUD prompt for a character.
 *
 * Default prompt format: <HP hp MP mana MV move>
 * Supports %-tokens in custom prompts:
 *   %h = current HP, %H = max HP
 *   %m = current mana, %M = max mana
 *   %v = current move, %V = max move
 *   %g = gold
 *   %x = experience
 *   %a = alignment
 *   %e = exits
 *   %% = literal %
 */
export function renderPrompt(ch: CharData): string {
  const template = ch.prompt || '<%h/%Hhp %m/%Mmn %v/%Vmv> ';
  let result = '';
  let i = 0;

  while (i < template.length) {
    if (template[i] === '%' && i + 1 < template.length) {
      i++;
      switch (template[i]) {
        case 'h': result += ch.hit.toString(); break;
        case 'H': result += ch.maxHit.toString(); break;
        case 'm': result += ch.mana.toString(); break;
        case 'M': result += ch.maxMana.toString(); break;
        case 'v': result += ch.move.toString(); break;
        case 'V': result += ch.maxMove.toString(); break;
        case 'g': result += ch.gold.toString(); break;
        case 'x': result += ch.exp.toString(); break;
        case 'a': result += ch.alignment.toString(); break;
        case 'e': result += formatExitsShort(ch); break;
        case '%': result += '%'; break;
        default:  result += '%' + template[i]; break;
      }
      i++;
    } else {
      result += template[i];
      i++;
    }
  }

  return result;
}

/**
 * Format exits as a compact string for the prompt (e.g., "NSEW").
 */
function formatExitsShort(ch: CharData): string {
  const vnum = charRoomMap.get(ch.id);
  if (vnum === undefined) return 'none';

  const room = world.getRoom(vnum);
  if (!room) return 'none';

  const dirLetters = ['N', 'E', 'S', 'W', 'U', 'D'];
  let exits = '';

  for (let dir = 0; dir < 6; dir++) {
    const exit = room.exits[dir as keyof typeof room.exits];
    if (exit) {
      exits += dirLetters[dir];
    }
  }

  return exits || 'none';
}
