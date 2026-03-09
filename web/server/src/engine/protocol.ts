/**
 * protocol.ts — Server-side structured JSON message emitter for Stormgate MUD.
 *
 * Sends structured JSON messages alongside the normal ANSI text output.
 * The web client uses these to populate rich UI panels (vitals, room, combat).
 *
 * JSON messages start with '{' so the client can distinguish them from
 * raw ANSI text, which never starts with '{'.
 */

import type { CharData, RoomIndex } from './types.js';
import { Position } from './types.js';
import { world } from './world.js';
import { sendToChar, hasConnection } from './output.js';

// ============================================================================
//  Position name lookup
// ============================================================================

function getPositionName(pos: Position): string {
  switch (pos) {
    case Position.DEAD:       return 'dead';
    case Position.MORTAL:     return 'mortally wounded';
    case Position.INCAP:      return 'incapacitated';
    case Position.STUNNED:    return 'stunned';
    case Position.SLEEPING:   return 'sleeping';
    case Position.RESTING:    return 'resting';
    case Position.GHOST:      return 'ghost';
    case Position.FIGHTING:   return 'fighting';
    case Position.STANDING:   return 'standing';
    case Position.MEDITATING: return 'meditating';
    default:                  return 'standing';
  }
}

// ============================================================================
//  Class / Race name lookup (duplicated from commands.ts to avoid circular deps)
// ============================================================================

const CLASS_NAMES: Record<number, string> = {
  0: 'Mage', 1: 'Cleric', 2: 'Thief', 3: 'Warrior', 4: 'Psionicist',
  5: 'Druid', 6: 'Ranger', 7: 'Paladin', 8: 'Bard', 9: 'Vampire',
  10: 'Werewolf', 11: 'Anti-Paladin', 12: 'Assassin', 13: 'Monk',
  14: 'Barbarian', 15: 'Illusionist', 16: 'Necromancer', 17: 'Demonologist',
  18: 'Shaman', 19: 'Darkpriest',
};

const RACE_NAMES: Record<number, string> = {
  0: 'Human', 1: 'Elf', 2: 'Dwarf', 3: 'Gnome', 4: 'Halfling',
  5: 'Half-Elf', 6: 'Half-Orc', 7: 'Ogre', 8: 'Drow', 9: 'Pixie',
  10: 'Minotaur', 11: 'Troll', 12: 'Lizardman', 13: 'Demon', 14: 'Dragon',
  15: 'Undead', 16: 'Vampire', 17: 'Werewolf', 18: 'Angel', 19: 'Feline',
  20: 'Canine', 21: 'Hobbit', 22: 'Draconian', 23: 'Shadow', 24: 'Treant',
  25: 'Golem', 26: 'Elemental',
};

const DIRECTION_NAMES = ['north', 'east', 'south', 'west', 'up', 'down'];

// ============================================================================
//  Send functions
// ============================================================================

/**
 * Send a vitals update to the client.
 * Call this after any action that changes HP, mana, move, gold, or level.
 */
export function sendVitals(ch: CharData): void {
  if (ch.isNpc || !hasConnection(ch.id)) return;

  const msg = JSON.stringify({
    type: 'vitals',
    data: {
      hp: ch.hit,
      maxHp: ch.maxHit,
      mana: ch.mana,
      maxMana: ch.maxMana,
      move: ch.move,
      maxMove: ch.maxMove,
      gold: ch.gold,
      level: ch.level,
      position: getPositionName(ch.position),
    },
  });
  sendToChar(ch, msg);
}

/**
 * Send room data to the client.
 * Call this after doLook or whenever the player enters a new room.
 */
export function sendRoomData(ch: CharData, room: RoomIndex): void {
  if (ch.isNpc || !hasConnection(ch.id)) return;

  // Collect exit directions
  const exits: string[] = [];
  for (let dir = 0; dir < 6; dir++) {
    const exit = room.exits[dir as keyof typeof room.exits];
    if (exit && !(exit.exitInfo & 1)) {
      exits.push(DIRECTION_NAMES[dir]);
    }
  }

  // Collect items on the floor
  const items: string[] = [];
  const objs = world.getObjsInRoom(room.vnum);
  for (const obj of objs) {
    items.push(obj.shortDescr);
  }

  // Collect characters in the room (excluding the viewer)
  const characters: string[] = [];
  const chars = world.getCharsInRoom(room.vnum);
  for (const other of chars) {
    if (other.id === ch.id) continue;
    if (other.isNpc) {
      characters.push(other.shortDescr);
    } else {
      characters.push(other.name);
    }
  }

  const msg = JSON.stringify({
    type: 'room',
    data: {
      name: room.name,
      description: room.description,
      exits,
      items,
      characters,
      vnum: room.vnum,
    },
  });
  sendToChar(ch, msg);
}

/**
 * Send combat data to the client.
 * Call this after each hit in the combat system.
 */
export function sendCombatData(
  ch: CharData,
  target: CharData,
  damage: number,
  message: string,
): void {
  if (ch.isNpc || !hasConnection(ch.id)) return;

  const attackerHpPct = ch.maxHit > 0 ? (ch.hit / ch.maxHit) * 100 : 0;
  const targetHpPct = target.maxHit > 0 ? (target.hit / target.maxHit) * 100 : 0;

  const attackerName = ch.isNpc ? ch.shortDescr : ch.name;
  const targetName = target.isNpc ? target.shortDescr : target.name;

  const msg = JSON.stringify({
    type: 'combat',
    data: {
      attacker: attackerName,
      target: targetName,
      damage,
      message,
      attackerHpPct: Math.max(0, Math.round(attackerHpPct)),
      targetHpPct: Math.max(0, Math.round(targetHpPct)),
    },
  });
  sendToChar(ch, msg);
}

/**
 * Send a channel message to the client.
 * Call this after say, tell, shout, or chat commands.
 */
export function sendChannelMessage(
  ch: CharData,
  channel: string,
  sender: string,
  message: string,
): void {
  if (ch.isNpc || !hasConnection(ch.id)) return;

  const msg = JSON.stringify({
    type: 'channel',
    data: {
      channel,
      sender,
      message,
      timestamp: Date.now(),
    },
  });
  sendToChar(ch, msg);
}

/**
 * Send character info to the client.
 * Call this after the score command.
 */
export function sendCharInfo(ch: CharData): void {
  if (ch.isNpc || !hasConnection(ch.id)) return;

  const className = CLASS_NAMES[ch.charClass] ?? 'Unknown';
  const raceName = RACE_NAMES[ch.race] ?? 'Unknown';

  const msg = JSON.stringify({
    type: 'charInfo',
    data: {
      name: ch.name,
      level: ch.level,
      class: className,
      race: raceName,
      hp: ch.hit,
      maxHp: ch.maxHit,
      mana: ch.mana,
      maxMana: ch.maxMana,
      move: ch.move,
      maxMove: ch.maxMove,
      gold: ch.gold,
      exp: ch.exp,
      hitroll: ch.hitroll,
      damroll: ch.damroll,
      armor: ch.armor,
      alignment: ch.alignment,
      position: getPositionName(ch.position),
    },
  });
  sendToChar(ch, msg);
}

/**
 * Send the who list to the client.
 * Call this after the who command.
 */
export function sendWhoList(ch: CharData): void {
  if (ch.isNpc || !hasConnection(ch.id)) return;

  const entries: Array<{ name: string; level: number; class: string; title: string }> = [];

  for (const other of world.characters.values()) {
    if (other.isNpc || other.deleted) continue;
    if (!hasConnection(other.id)) continue;

    const className = CLASS_NAMES[other.charClass] ?? 'Unknown';
    const pcData = world.pcData.get(other.id);
    const title = pcData?.title ?? '';

    entries.push({
      name: other.name,
      level: other.level,
      class: className,
      title,
    });
  }

  const msg = JSON.stringify({
    type: 'who',
    data: entries,
  });
  sendToChar(ch, msg);
}
