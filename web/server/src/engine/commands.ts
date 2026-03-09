/**
 * commands.ts — Command interpreter for Stormgate MUD.
 *
 * Port of interp.c from the C server. Contains the command table, the
 * interpret() dispatcher, and implementations for all core player commands.
 *
 * Commands are matched by prefix (like the original C version): typing "l"
 * matches "look", "n" matches "north", etc. The first match in the table wins.
 */

import type {
  CharData,
  ObjInstance,
  RoomIndex,
  ExitData,
} from './types.js';

import {
  Position,
  Direction,
  WearLocation,
  Sex,
  ItemType,
  SectorType,
  CharClass,
} from './types.js';

import { world } from './world.js';

import {
  charToRoom,
  charFromRoom,
  getCharRoom,
  objToRoom,
  objFromRoom,
  objToChar,
  objFromChar,
  equipChar,
  unequipChar,
  getEquipment,
  getCarriedItems,
  getEquippedItems,
  getWearSlotName,
  getInventory,
  isImmortal,
  isName,
  capitalize,
} from './handler.js';

import {
  sendToChar,
  sendToRoom,
  act,
  renderPrompt,
  colors,
  hasConnection,
  unregisterConnection,
  TO_CHAR,
  TO_ROOM,
  TO_VICT,
  TO_NOTVICT,
} from './output.js';

import {
  setFighting,
  stopFighting,
  multiHit,
  doConsider,
  doWimpy,
  doRest,
  doSleep,
  doWake,
  doStand,
} from './fight.js';

import { adminCommands } from './admin.js';
import { saveCharacter, charOwnerMap } from './save.js';

import { socialCommands, garbleMessage, getLanguageName } from './social.js';
import { questCommands } from './quest.js';
import { craftingCommands } from './crafting.js';
import { getMagicCommands } from './magic.js';

import {
  doBuy,
  doSell,
  doList,
  doValue,
  doExamine,
  doSacrifice,
  doEat,
  doDrink,
  doFill,
  doQuaff,
  doRecite,
  doBrandish,
  doZap,
  doGetEnhanced,
  doHeal,
} from './shops.js';

import {
  sendVitals,
  sendRoomData,
  sendChannelMessage,
  sendCharInfo,
  sendWhoList,
} from './protocol.js';

import {
  EX_ISDOOR,
  EX_CLOSED,
  EX_LOCKED,
  EX_BASHED,
  EX_BASHPROOF,
  EX_PICKPROOF,
  EX_PASSPROOF,
  EX_HIDDEN,
  CONT_CLOSEABLE,
  CONT_PICKPROOF,
  CONT_CLOSED,
  CONT_LOCKED,
} from './resets.js';

// ============================================================================
//  Command entry interface
// ============================================================================

export interface CommandEntry {
  name: string;
  fn: (ch: CharData, argument: string) => void;
  minPosition: Position;
  minLevel: number;
  /** 0 = no log, 1 = always log, 2 = log if args */
  log: number;
}

// ============================================================================
//  Class name lookup
// ============================================================================

const CLASS_NAMES: Record<number, string> = {
  0: 'Mage',
  1: 'Cleric',
  2: 'Thief',
  3: 'Warrior',
  4: 'Psionicist',
  5: 'Druid',
  6: 'Ranger',
  7: 'Paladin',
  8: 'Bard',
  9: 'Vampire',
  10: 'Werewolf',
  11: 'Anti-Paladin',
  12: 'Assassin',
  13: 'Monk',
  14: 'Barbarian',
  15: 'Illusionist',
  16: 'Necromancer',
  17: 'Demonologist',
  18: 'Shaman',
  19: 'Darkpriest',
};

const RACE_NAMES: Record<number, string> = {
  0: 'Human',
  1: 'Elf',
  2: 'Dwarf',
  3: 'Gnome',
  4: 'Halfling',
  5: 'Half-Elf',
  6: 'Half-Orc',
  7: 'Ogre',
  8: 'Drow',
  9: 'Pixie',
  10: 'Minotaur',
  11: 'Troll',
  12: 'Lizardman',
  13: 'Demon',
  14: 'Dragon',
  15: 'Undead',
  16: 'Vampire',
  17: 'Werewolf',
  18: 'Angel',
  19: 'Feline',
  20: 'Canine',
  21: 'Hobbit',
  22: 'Draconian',
  23: 'Shadow',
  24: 'Treant',
  25: 'Golem',
  26: 'Elemental',
};

// ============================================================================
//  Direction helpers
// ============================================================================

const DIRECTION_NAMES = ['north', 'east', 'south', 'west', 'up', 'down'];
const REVERSE_DIR: Record<number, number> = {
  [Direction.NORTH]: Direction.SOUTH,
  [Direction.SOUTH]: Direction.NORTH,
  [Direction.EAST]:  Direction.WEST,
  [Direction.WEST]:  Direction.EAST,
  [Direction.UP]:    Direction.DOWN,
  [Direction.DOWN]:  Direction.UP,
};

// ============================================================================
//  The command table
// ============================================================================

export const commandTable: CommandEntry[] = [
  // Movement — ordered first so single-letter shortcuts work
  { name: 'north',     fn: doNorth,     minPosition: Position.GHOST,    minLevel: 0, log: 0 },
  { name: 'east',      fn: doEast,      minPosition: Position.GHOST,    minLevel: 0, log: 0 },
  { name: 'south',     fn: doSouth,     minPosition: Position.GHOST,    minLevel: 0, log: 0 },
  { name: 'west',      fn: doWest,      minPosition: Position.GHOST,    minLevel: 0, log: 0 },
  { name: 'up',        fn: doUp,        minPosition: Position.GHOST,    minLevel: 0, log: 0 },
  { name: 'down',      fn: doDown,      minPosition: Position.GHOST,    minLevel: 0, log: 0 },

  // Information
  { name: 'look',      fn: doLook,      minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'score',     fn: doScore,     minPosition: Position.DEAD,     minLevel: 0, log: 0 },
  { name: 'who',       fn: doWho,       minPosition: Position.DEAD,     minLevel: 0, log: 0 },
  { name: 'inventory', fn: doInventory, minPosition: Position.DEAD,     minLevel: 0, log: 0 },
  { name: 'equipment', fn: doEquipment, minPosition: Position.DEAD,     minLevel: 0, log: 0 },
  { name: 'exits',     fn: doExits,     minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'help',      fn: doHelp,      minPosition: Position.DEAD,     minLevel: 0, log: 0 },

  // Communication
  { name: 'say',       fn: doSay,       minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'tell',      fn: doTell,      minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'shout',     fn: doShout,     minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'chat',      fn: doChat,      minPosition: Position.DEAD,     minLevel: 0, log: 0 },

  // Object manipulation
  { name: 'get',       fn: doGetEnhanced, minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'drop',      fn: doDrop,      minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'put',       fn: doPut,       minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'empty',     fn: doEmpty,     minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'wear',      fn: doWear,      minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'remove',    fn: doRemove,    minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'give',      fn: doGive,      minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'examine',   fn: doExamine,   minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'sacrifice', fn: doSacrifice, minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'eat',       fn: doEat,       minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'drink',     fn: doDrink,     minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'fill',      fn: doFill,      minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'quaff',     fn: doQuaff,     minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'recite',    fn: doRecite,    minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'brandish',  fn: doBrandish,  minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'zap',       fn: doZap,       minPosition: Position.RESTING,  minLevel: 0, log: 0 },

  // Shop commands
  { name: 'buy',       fn: doBuy,       minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'sell',      fn: doSell,      minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'list',      fn: doList,      minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'value',     fn: doValue,     minPosition: Position.RESTING,  minLevel: 0, log: 0 },

  // Healer
  { name: 'heal',      fn: doHeal,      minPosition: Position.RESTING,  minLevel: 0, log: 0 },

  // Door commands
  { name: 'open',      fn: doOpen,      minPosition: Position.STANDING, minLevel: 0, log: 0 },
  { name: 'close',     fn: doClose,     minPosition: Position.STANDING, minLevel: 0, log: 0 },
  { name: 'lock',      fn: doLock,      minPosition: Position.STANDING, minLevel: 0, log: 0 },
  { name: 'unlock',    fn: doUnlock,    minPosition: Position.STANDING, minLevel: 0, log: 0 },
  { name: 'pick',      fn: doPick,      minPosition: Position.STANDING, minLevel: 0, log: 0 },
  { name: 'knock',     fn: doKnock,     minPosition: Position.STANDING, minLevel: 0, log: 0 },

  // Actions
  { name: 'quit',      fn: doQuit,      minPosition: Position.DEAD,     minLevel: 0, log: 0 },
  { name: 'save',      fn: doSave,      minPosition: Position.DEAD,     minLevel: 0, log: 0 },
  { name: 'recall',    fn: doRecall,    minPosition: Position.GHOST,    minLevel: 0, log: 0 },

  // Combat
  { name: 'kill',      fn: doKill,      minPosition: Position.STANDING,  minLevel: 0, log: 0 },
  { name: 'flee',      fn: doFlee,      minPosition: Position.FIGHTING,  minLevel: 0, log: 0 },
  { name: 'consider',  fn: doConsiderCmd, minPosition: Position.RESTING, minLevel: 0, log: 0 },
  { name: 'wimpy',     fn: doWimpyCmd,  minPosition: Position.DEAD,      minLevel: 0, log: 0 },

  // Position
  { name: 'rest',      fn: doRestCmd,   minPosition: Position.RESTING,   minLevel: 0, log: 0 },
  { name: 'sleep',     fn: doSleepCmd,  minPosition: Position.RESTING,   minLevel: 0, log: 0 },
  { name: 'stand',     fn: doStandCmd,  minPosition: Position.SLEEPING,  minLevel: 0, log: 0 },
  { name: 'wake',      fn: doWakeCmd,   minPosition: Position.SLEEPING,  minLevel: 0, log: 0 },

  // Admin / Immortal / Builder commands (appended from admin.ts)
  ...adminCommands,

  // Social systems (clan, religion, language, economy, marriage, gambling)
  ...socialCommands,

  // Quest system
  ...questCommands,

  // Crafting system
  ...craftingCommands,

  // Magic system (cast, practice, spells, skills, slist)
  ...getMagicCommands(),
];

// ============================================================================
//  The main interpreter
// ============================================================================

/**
 * Process a line of input from a player.
 *
 * 1. Extract the first word as the command.
 * 2. Find a match in the command table (prefix matching).
 * 3. Check position and level requirements.
 * 4. Call the command function.
 */
export function interpret(ch: CharData, input: string): void {
  // Trim and skip empty input
  const trimmed = input.trim();
  if (!trimmed) {
    sendToChar(ch, renderPrompt(ch));
    return;
  }

  // Split into command and argument
  const spaceIdx = trimmed.indexOf(' ');
  const cmd = spaceIdx === -1 ? trimmed.toLowerCase() : trimmed.substring(0, spaceIdx).toLowerCase();
  const argument = spaceIdx === -1 ? '' : trimmed.substring(spaceIdx + 1).trim();

  // Search the command table (prefix match)
  let found: CommandEntry | undefined;
  for (const entry of commandTable) {
    if (entry.name.startsWith(cmd)) {
      found = entry;
      break;
    }
  }

  if (!found) {
    sendToChar(ch, "Huh?\r\n");
    return;
  }

  // Check level
  if (ch.level < found.minLevel) {
    sendToChar(ch, "Huh?\r\n");
    return;
  }

  // Check position
  if (ch.position < found.minPosition) {
    switch (ch.position) {
      case Position.DEAD:
        sendToChar(ch, "You are DEAD!\r\n");
        break;
      case Position.MORTAL:
      case Position.INCAP:
        sendToChar(ch, "You are hurt far too badly for that.\r\n");
        break;
      case Position.STUNNED:
        sendToChar(ch, "You are too stunned to do that.\r\n");
        break;
      case Position.SLEEPING:
        sendToChar(ch, "In your dreams, or what?\r\n");
        break;
      case Position.RESTING:
        sendToChar(ch, "Nah... You feel too relaxed...\r\n");
        break;
      default:
        sendToChar(ch, "You can't do that right now.\r\n");
        break;
    }
    return;
  }

  // Execute the command
  found.fn(ch, argument);
}

// ============================================================================
//  Movement commands
// ============================================================================

function doNorth(ch: CharData, _argument: string): void {
  moveChar(ch, Direction.NORTH);
}

function doEast(ch: CharData, _argument: string): void {
  moveChar(ch, Direction.EAST);
}

function doSouth(ch: CharData, _argument: string): void {
  moveChar(ch, Direction.SOUTH);
}

function doWest(ch: CharData, _argument: string): void {
  moveChar(ch, Direction.WEST);
}

function doUp(ch: CharData, _argument: string): void {
  moveChar(ch, Direction.UP);
}

function doDown(ch: CharData, _argument: string): void {
  moveChar(ch, Direction.DOWN);
}

/**
 * Movement cost by sector type.
 * Port of movement_loss[] from act_move.c.
 */
const MOVEMENT_COST: Record<number, number> = {
  [SectorType.INSIDE]:       1,
  [SectorType.CITY]:         2,
  [SectorType.FIELD]:        2,
  [SectorType.FOREST]:       3,
  [SectorType.HILLS]:        4,
  [SectorType.MOUNTAIN]:     5,
  [SectorType.WATER_SWIM]:   4,
  [SectorType.WATER_NOSWIM]: 1,
  [SectorType.UNDERWATER]:   6,
  [SectorType.AIR]:          10,
  [SectorType.DESERT]:       6,
  [SectorType.BADLAND]:      4,
};

/**
 * AFF_ANTI_FLEE bitvector flag — prevents fleeing and voluntary movement.
 */
const AFF_ANTI_FLEE = 1 << 25;

/**
 * Move a character in a direction. Checks for valid exit, closed doors,
 * movement points, drunk stumbling, etc. Port of move_char() from act_move.c.
 */
function moveChar(ch: CharData, dir: Direction): void {
  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You are in the void. Something is very wrong.\r\n");
    return;
  }

  const room = world.getRoom(roomVnum);
  if (!room) {
    sendToChar(ch, "You are in the void. Something is very wrong.\r\n");
    return;
  }

  // AFF_ANTI_FLEE check: blocked from voluntary movement
  if (ch.affectedBy & AFF_ANTI_FLEE) {
    sendToChar(ch, "You are unable to move!\r\n");
    return;
  }

  // Drunk stumbling: if PC is drunk, random chance to go wrong direction
  if (!ch.isNpc) {
    const pc = world.pcData.get(ch.id);
    if (pc && pc.condition[0] > 10 && Math.random() < 0.20) {
      const randomDir = Math.floor(Math.random() * 6) as Direction;
      if (randomDir !== dir) {
        sendToChar(ch, "You stumble drunkenly and go the wrong way...\r\n");
        dir = randomDir;
      }
    }
  }

  const exit = room.exits[dir];
  if (!exit) {
    sendToChar(ch, "Alas, you cannot go that way.\r\n");
    return;
  }

  const toRoom = world.getRoom(exit.toVnum);
  if (!toRoom) {
    sendToChar(ch, "Alas, you cannot go that way.\r\n");
    return;
  }

  // Check for closed doors
  if (exit.exitInfo & EX_CLOSED) {
    const doorName = exit.keyword || 'door';
    sendToChar(ch, `The ${doorName} is closed.\r\n`);
    return;
  }

  // Movement cost (PCs only, NPCs have unlimited movement)
  if (!ch.isNpc) {
    const fromCost = MOVEMENT_COST[room.sectorType] ?? 1;
    const toCost = MOVEMENT_COST[toRoom.sectorType] ?? 1;
    const moveCost = Math.max(1, Math.floor((fromCost + toCost) / 2));

    if (ch.move < moveCost) {
      sendToChar(ch, "You are too exhausted.\r\n");
      return;
    }

    ch.move -= moveCost;
  }

  // Announce departure
  act(`$n leaves ${DIRECTION_NAMES[dir]}.`, ch, null, null, TO_ROOM);

  // Move the character
  charFromRoom(ch);
  charToRoom(ch, exit.toVnum);

  // Announce arrival
  act('$n has arrived.', ch, null, null, TO_ROOM);

  // Show the new room (this also sends sendRoomData and sendVitals)
  doLook(ch, '');
}

// ============================================================================
//  Information commands
// ============================================================================

/**
 * doLook — Show room name, description, exits, objects, and characters.
 */
function doLook(ch: CharData, argument: string): void {
  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You are in the void.\r\n");
    return;
  }

  const room = world.getRoom(roomVnum);
  if (!room) {
    sendToChar(ch, "You see nothing.\r\n");
    return;
  }

  // If an argument is given, try to look at something specific
  if (argument) {
    // Look at an object in inventory or room
    if (lookAtObject(ch, argument, room)) return;
    // Look at a character in the room
    if (lookAtChar(ch, argument)) return;
    // Look at extra descriptions in the room
    for (const ed of room.extraDescriptions) {
      if (isName(argument, ed.keyword)) {
        sendToChar(ch, ed.description + '\r\n');
        return;
      }
    }
    sendToChar(ch, "You don't see that here.\r\n");
    return;
  }

  let buf = '';

  // Room name (cyan bold)
  buf += `${colors.cyan}${colors.bold}${room.name}${colors.reset}\r\n`;

  // Room description
  buf += `  ${room.description}\r\n`;

  // Exits
  buf += formatExits(room);

  // Objects on the floor
  const objs = world.getObjsInRoom(roomVnum);
  for (const obj of objs) {
    buf += `${colors.green}${obj.description}${colors.reset}\r\n`;
  }

  // Characters in the room (excluding self)
  const chars = world.getCharsInRoom(roomVnum);
  for (const other of chars) {
    if (other.id === ch.id) continue;
    if (other.isNpc) {
      buf += `${colors.yellow}${other.longDescr}${colors.reset}\r\n`;
    } else {
      const posStr = getPositionString(other);
      buf += `${colors.yellow}${other.name}${colors.reset} is ${posStr} here.\r\n`;
    }
  }

  sendToChar(ch, buf);

  // Send structured data for rich UI panels
  sendRoomData(ch, room);
  sendVitals(ch);
}

/**
 * Try to look at an object in the character's inventory or the room.
 * Returns true if something was found and displayed.
 */
function lookAtObject(ch: CharData, arg: string, room: RoomIndex): boolean {
  // Check inventory
  const carried = getCarriedItems(ch);
  for (const obj of carried) {
    if (isName(arg, obj.name)) {
      sendToChar(ch, obj.description + '\r\n');
      return true;
    }
  }

  // Check equipped items
  const equipped = getEquippedItems(ch);
  for (const obj of equipped) {
    if (isName(arg, obj.name)) {
      sendToChar(ch, obj.description + '\r\n');
      return true;
    }
  }

  // Check room objects
  const roomObjs = world.getObjsInRoom(room.vnum);
  for (const obj of roomObjs) {
    if (isName(arg, obj.name)) {
      sendToChar(ch, obj.description + '\r\n');
      return true;
    }
  }

  return false;
}

/**
 * Try to look at a character in the same room.
 * Returns true if someone was found and displayed.
 */
function lookAtChar(ch: CharData, arg: string): boolean {
  const roomVnum = getCharRoom(ch);
  const chars = world.getCharsInRoom(roomVnum);

  for (const vict of chars) {
    if (vict.id === ch.id) continue;
    if (isName(arg, vict.name)) {
      let buf = '';
      buf += `${colors.bold}${vict.name}${colors.reset}\r\n`;
      if (vict.description) {
        buf += vict.description + '\r\n';
      }
      buf += `${vict.name} is in ${getConditionString(vict)} condition.\r\n`;

      // Show equipment
      const equip = getEquippedItems(vict);
      if (equip.length > 0) {
        buf += `${vict.name} is using:\r\n`;
        for (const obj of equip) {
          buf += `${getWearSlotName(obj.wearLoc)}${obj.shortDescr}\r\n`;
        }
      }

      sendToChar(ch, buf);
      act('$n looks at you.', ch, null, vict, TO_VICT);
      act('$n looks at $N.', ch, null, vict, TO_NOTVICT);
      return true;
    }
  }
  return false;
}

/**
 * Format exits for room display.
 */
function formatExits(room: RoomIndex): string {
  const exitParts: string[] = [];

  for (let dir = 0; dir < 6; dir++) {
    const exit = room.exits[dir as keyof typeof room.exits];
    if (exit) {
      if (exit.exitInfo & EX_HIDDEN) continue;
      const closed = (exit.exitInfo & EX_CLOSED) ? ' (closed)' : '';
      exitParts.push(`${DIRECTION_NAMES[dir]}${closed}`);
    }
  }

  if (exitParts.length === 0) {
    return `${colors.brightCyan}[Exits: none]${colors.reset}\r\n`;
  }

  return `${colors.brightCyan}[Exits: ${exitParts.join(' ')}]${colors.reset}\r\n`;
}

/**
 * doExits — Show available exits.
 */
function doExits(ch: CharData, _argument: string): void {
  const roomVnum = getCharRoom(ch);
  const room = world.getRoom(roomVnum);
  if (!room) {
    sendToChar(ch, "You are in the void.\r\n");
    return;
  }

  let buf = "Obvious exits:\r\n";
  let found = false;

  for (let dir = 0; dir < 6; dir++) {
    const exit = room.exits[dir as keyof typeof room.exits];
    if (exit) {
      if (exit.exitInfo & EX_HIDDEN) continue;
      found = true;
      const toRoom = world.getRoom(exit.toVnum);
      const closed = (exit.exitInfo & EX_CLOSED) ? ' (closed)' : '';
      const roomName = toRoom ? toRoom.name : 'unknown';
      buf += `  ${capitalize(DIRECTION_NAMES[dir])}${closed} - ${roomName}\r\n`;
    }
  }

  if (!found) {
    buf += "  None.\r\n";
  }

  sendToChar(ch, buf);
}

/**
 * doScore — Show character stats.
 */
function doScore(ch: CharData, _argument: string): void {
  const className = CLASS_NAMES[ch.charClass] ?? 'Unknown';
  const raceName = RACE_NAMES[ch.race] ?? 'Unknown';
  const sexName = ch.sex === Sex.MALE ? 'Male' : ch.sex === Sex.FEMALE ? 'Female' : 'Neutral';

  let buf = '';
  buf += `${colors.brightCyan}+------------------------------------------------------+${colors.reset}\r\n`;
  buf += `${colors.brightCyan}|${colors.reset}  ${colors.bold}${ch.name}${colors.reset}`;
  // Pad the name line
  const nameLen = ch.name.length;
  buf += ' '.repeat(Math.max(1, 52 - nameLen));
  buf += `${colors.brightCyan}|${colors.reset}\r\n`;
  buf += `${colors.brightCyan}+------------------------------------------------------+${colors.reset}\r\n`;

  buf += `  Level: ${colors.brightWhite}${ch.level}${colors.reset}`;
  buf += `    Race: ${colors.brightWhite}${raceName}${colors.reset}`;
  buf += `    Class: ${colors.brightWhite}${className}${colors.reset}\r\n`;

  buf += `  Sex: ${colors.brightWhite}${sexName}${colors.reset}`;
  buf += `    Alignment: ${colors.brightWhite}${ch.alignment}${colors.reset}\r\n`;
  buf += '\r\n';

  buf += `  ${colors.brightRed}HP:${colors.reset} ${ch.hit}/${ch.maxHit}`;
  buf += `    ${colors.brightBlue}Mana:${colors.reset} ${ch.mana}/${ch.maxMana}`;
  buf += `    ${colors.brightGreen}Move:${colors.reset} ${ch.move}/${ch.maxMove}\r\n`;
  if (ch.maxBp > 0) {
    buf += `  ${colors.red}Blood:${colors.reset} ${ch.bp}/${ch.maxBp}\r\n`;
  }
  buf += '\r\n';

  buf += `  Hitroll: ${colors.brightWhite}${ch.hitroll}${colors.reset}`;
  buf += `    Damroll: ${colors.brightWhite}${ch.damroll}${colors.reset}`;
  buf += `    Armor: ${colors.brightWhite}${ch.armor}${colors.reset}\r\n`;

  buf += `  Gold: ${colors.brightYellow}${ch.gold}${colors.reset}`;
  buf += `    Experience: ${colors.brightWhite}${ch.exp}${colors.reset}\r\n`;
  buf += `  Practices: ${colors.brightWhite}${ch.practice}${colors.reset}\r\n`;
  buf += '\r\n';

  buf += `  Position: ${getPositionString(ch)}\r\n`;

  if (ch.affects.length > 0) {
    buf += '\r\n  Active affects:\r\n';
    for (const af of ch.affects) {
      if (!af.deleted) {
        buf += `    Spell ${af.type}: modifies ${getApplyName(af.location)} by ${af.modifier} for ${af.duration} ticks\r\n`;
      }
    }
  }

  buf += `${colors.brightCyan}+------------------------------------------------------+${colors.reset}\r\n`;
  sendToChar(ch, buf);

  // Send structured data for rich UI panels
  sendCharInfo(ch);
  sendVitals(ch);
}

/**
 * doWho — List all connected players.
 */
function doWho(ch: CharData, _argument: string): void {
  let buf = `${colors.brightCyan}--- Players Online ---${colors.reset}\r\n`;
  let count = 0;

  for (const other of world.characters.values()) {
    if (other.isNpc || other.deleted) continue;
    if (!hasConnection(other.id)) continue;

    count++;
    const className = CLASS_NAMES[other.charClass] ?? 'Unknown';
    const raceName = RACE_NAMES[other.race] ?? 'Unknown';
    const levelStr = other.level.toString().padStart(3, ' ');
    const classStr = className.padEnd(12, ' ');

    let line = `[${levelStr} ${classStr}] ${other.name}`;

    // Show title from PcData if available
    const pcData = world.pcData.get(other.id);
    if (pcData?.title) {
      line += ` ${pcData.title}`;
    }

    if (isImmortal(other)) {
      line += ` ${colors.brightYellow}(Immortal)${colors.reset}`;
    }

    buf += line + '\r\n';
  }

  buf += `\r\n${colors.brightCyan}${count} player${count !== 1 ? 's' : ''} online.${colors.reset}\r\n`;
  sendToChar(ch, buf);

  // Send structured who list for UI panel
  sendWhoList(ch);
}

/**
 * doInventory — List carried (non-equipped) items.
 */
function doInventory(ch: CharData, _argument: string): void {
  const items = getCarriedItems(ch);

  if (items.length === 0) {
    sendToChar(ch, "You are not carrying anything.\r\n");
    return;
  }

  let buf = "You are carrying:\r\n";
  for (const obj of items) {
    buf += `  ${obj.shortDescr}\r\n`;
  }
  sendToChar(ch, buf);
}

/**
 * doEquipment — List equipped items by wear slot.
 */
function doEquipment(ch: CharData, _argument: string): void {
  const items = getEquippedItems(ch);

  let buf = "You are using:\r\n";
  let found = false;

  // Show in slot order
  for (let slot = WearLocation.LIGHT; slot <= WearLocation.IMPLANTED3; slot++) {
    const obj = items.find((o) => o.wearLoc === slot);
    if (obj) {
      found = true;
      buf += `${getWearSlotName(slot)}${obj.shortDescr}\r\n`;
    }
  }

  if (!found) {
    buf += "  Nothing.\r\n";
  }

  sendToChar(ch, buf);
}

// ============================================================================
//  Communication commands
// ============================================================================

/**
 * doSay -- Broadcast message to current room, garbled by language.
 */
function doSay(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Say what?\r\n");
    return;
  }

  const langName = getLanguageName(ch.speaking);
  sendToChar(ch, `${colors.green}You say (in ${langName}) '${argument}'${colors.reset}\r\n`);

  // Send structured channel message to the speaker
  sendChannelMessage(ch, 'say', ch.name, argument);

  // Send to others in the room, garbled by language proficiency
  const sayRoomVnum = getCharRoom(ch);
  if (sayRoomVnum !== -1) {
    for (const other of world.getCharsInRoom(sayRoomVnum)) {
      if (other.id === ch.id || other.isNpc) continue;
      const garbled = garbleMessage(ch, other, argument);
      sendToChar(other, `${colors.green}${ch.name} says (in ${langName}) '${garbled}'${colors.reset}\r\n`);
      sendChannelMessage(other, 'say', ch.name, garbled);
    }
  }
}

/**
 * doTell — Send a private message to another player.
 */
function doTell(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Tell whom what?\r\n");
    return;
  }

  const [targetName, ...messageParts] = argument.split(' ');
  const message = messageParts.join(' ');

  if (!message) {
    sendToChar(ch, "Tell them what?\r\n");
    return;
  }

  // Find the target player
  let victim: CharData | undefined;
  for (const other of world.characters.values()) {
    if (other.isNpc || other.deleted) continue;
    if (isName(targetName, other.name)) {
      victim = other;
      break;
    }
  }

  if (!victim) {
    sendToChar(ch, "They aren't here.\r\n");
    return;
  }

  if (victim.id === ch.id) {
    sendToChar(ch, "Talking to yourself again?\r\n");
    return;
  }

  if (!hasConnection(victim.id)) {
    sendToChar(ch, "They aren't connected right now.\r\n");
    return;
  }

  sendToChar(ch, `${colors.magenta}You tell ${victim.name} '${message}'${colors.reset}\r\n`);
  const garbled = garbleMessage(ch, victim, message);
  sendToChar(victim, `${colors.magenta}${ch.name} tells you '${garbled}'${colors.reset}\r\n`);

  // Send structured channel messages
  sendChannelMessage(ch, 'tell', ch.name, `(to ${victim.name}) ${message}`);
  sendChannelMessage(victim, 'tell', ch.name, message);
}

/**
 * doShout -- Broadcast to all connected players, garbled by language.
 */
function doShout(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Shout what?\r\n");
    return;
  }

  const langName = getLanguageName(ch.speaking);
  sendToChar(ch, `${colors.brightYellow}You shout (in ${langName}) '${argument}'${colors.reset}\r\n`);

  // Send structured channel message to the shouter
  sendChannelMessage(ch, 'shout', ch.name, argument);

  for (const other of world.characters.values()) {
    if (other.isNpc || other.deleted || other.id === ch.id) continue;
    if (!hasConnection(other.id)) continue;
    const garbled = garbleMessage(ch, other, argument);
    sendToChar(other, `${colors.brightYellow}${ch.name} shouts (in ${langName}) '${garbled}'${colors.reset}\r\n`);

    // Send structured channel message to listeners
    sendChannelMessage(other, 'shout', ch.name, garbled);
  }
}

/**
 * doChat — Global channel broadcast.
 */
function doChat(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Chat what?\r\n");
    return;
  }

  sendToChar(ch, `${colors.brightMagenta}[Chat] You: ${argument}${colors.reset}\r\n`);

  // Send structured channel message to the chatter
  sendChannelMessage(ch, 'chat', ch.name, argument);

  for (const other of world.characters.values()) {
    if (other.isNpc || other.deleted || other.id === ch.id) continue;
    if (!hasConnection(other.id)) continue;
    sendToChar(
      other,
      `${colors.brightMagenta}[Chat] ${ch.name}: ${argument}${colors.reset}\r\n`
    );

    // Send structured channel message to listeners
    sendChannelMessage(other, 'chat', ch.name, argument);
  }
}

// ============================================================================
//  Object manipulation commands
// ============================================================================

// NOTE: The 'get' command uses doGetEnhanced from shops.ts which handles
// containers, money objects, and take-flag checks.

/**
 * doDrop — Drop an object from inventory to the room.
 */
function doDrop(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Drop what?\r\n");
    return;
  }

  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You are in the void.\r\n");
    return;
  }

  // Handle "drop all"
  if (argument.toLowerCase() === 'all') {
    const items = getCarriedItems(ch);
    if (items.length === 0) {
      sendToChar(ch, "You aren't carrying anything to drop.\r\n");
      return;
    }
    for (const obj of [...items]) {
      objFromChar(obj);
      objToRoom(obj, roomVnum);
      sendToChar(ch, `You drop ${obj.shortDescr}.\r\n`);
      act('$n drops $p.', ch, obj, null, TO_ROOM);
    }
    return;
  }

  // Find the object in inventory
  const items = getCarriedItems(ch);
  for (const obj of items) {
    if (isName(argument, obj.name)) {
      objFromChar(obj);
      objToRoom(obj, roomVnum);
      sendToChar(ch, `You drop ${obj.shortDescr}.\r\n`);
      act('$n drops $p.', ch, obj, null, TO_ROOM);
      return;
    }
  }

  sendToChar(ch, "You don't have that.\r\n");
}

/**
 * Determine the wear location for an item based on its wearFlags.
 * Returns the first available slot that the item can go in.
 */
function getWearLocation(obj: ObjInstance, ch: CharData): WearLocation {
  const flags = obj.wearFlags;

  // wearFlags bit positions map to wear locations
  // Bit 0 = take (not a wear slot), bit 1+ = actual wear slots
  const wearBits: Array<{ bit: number; loc: WearLocation }> = [
    { bit: 1 << 1,  loc: WearLocation.FINGER_L },
    { bit: 1 << 2,  loc: WearLocation.NECK_1 },
    { bit: 1 << 3,  loc: WearLocation.BODY },
    { bit: 1 << 4,  loc: WearLocation.HEAD },
    { bit: 1 << 5,  loc: WearLocation.LEGS },
    { bit: 1 << 6,  loc: WearLocation.FEET },
    { bit: 1 << 7,  loc: WearLocation.HANDS },
    { bit: 1 << 8,  loc: WearLocation.ARMS },
    { bit: 1 << 9,  loc: WearLocation.SHIELD },
    { bit: 1 << 10, loc: WearLocation.ABOUT },
    { bit: 1 << 11, loc: WearLocation.WAIST },
    { bit: 1 << 12, loc: WearLocation.WRIST_L },
    { bit: 1 << 13, loc: WearLocation.WIELD },
    { bit: 1 << 14, loc: WearLocation.HOLD },
  ];

  // Special case: light items
  if (obj.itemType === ItemType.LIGHT) {
    return WearLocation.LIGHT;
  }

  for (const { bit, loc } of wearBits) {
    if (flags & bit) {
      // Check if the slot is available; try alternate slots for paired ones
      if (!getEquipment(ch, loc)) {
        return loc;
      }
      // For paired slots (fingers, neck, wrists), try the other side
      if (loc === WearLocation.FINGER_L && !getEquipment(ch, WearLocation.FINGER_R)) {
        return WearLocation.FINGER_R;
      }
      if (loc === WearLocation.NECK_1 && !getEquipment(ch, WearLocation.NECK_2)) {
        return WearLocation.NECK_2;
      }
      if (loc === WearLocation.WRIST_L && !getEquipment(ch, WearLocation.WRIST_R)) {
        return WearLocation.WRIST_R;
      }
      if (loc === WearLocation.WIELD && !getEquipment(ch, WearLocation.WIELD_2)) {
        return WearLocation.WIELD_2;
      }
    }
  }

  return WearLocation.NONE;
}

/**
 * doWear — Equip an item from inventory.
 */
function doWear(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Wear what?\r\n");
    return;
  }

  // Handle "wear all"
  if (argument.toLowerCase() === 'all') {
    const items = getCarriedItems(ch);
    let wornSomething = false;
    for (const obj of [...items]) {
      const loc = getWearLocation(obj, ch);
      if (loc !== WearLocation.NONE) {
        equipChar(ch, obj, loc);
        sendToChar(ch, `You wear ${obj.shortDescr}.\r\n`);
        act('$n wears $p.', ch, obj, null, TO_ROOM);
        wornSomething = true;
      }
    }
    if (!wornSomething) {
      sendToChar(ch, "You can't wear anything else.\r\n");
    }
    return;
  }

  // Find the item in inventory
  const items = getCarriedItems(ch);
  for (const obj of items) {
    if (isName(argument, obj.name)) {
      const loc = getWearLocation(obj, ch);
      if (loc === WearLocation.NONE) {
        sendToChar(ch, "You can't wear that.\r\n");
        return;
      }
      equipChar(ch, obj, loc);
      sendToChar(ch, `You wear ${obj.shortDescr}.\r\n`);
      act('$n wears $p.', ch, obj, null, TO_ROOM);
      return;
    }
  }

  sendToChar(ch, "You don't have that.\r\n");
}

/**
 * doRemove — Unequip an item.
 */
function doRemove(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Remove what?\r\n");
    return;
  }

  // Find the equipped item
  const items = getEquippedItems(ch);
  for (const obj of items) {
    if (isName(argument, obj.name)) {
      unequipChar(ch, obj);
      sendToChar(ch, `You stop using ${obj.shortDescr}.\r\n`);
      act('$n stops using $p.', ch, obj, null, TO_ROOM);
      return;
    }
  }

  sendToChar(ch, "You aren't using that.\r\n");
}

/**
 * doGive — Transfer an object to another character.
 */
function doGive(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Give what to whom?\r\n");
    return;
  }

  const parts = argument.split(' ');
  if (parts.length < 2) {
    sendToChar(ch, "Give what to whom?\r\n");
    return;
  }

  const objArg = parts[0];
  const targetArg = parts[parts.length - 1];

  // Find the object in inventory
  const items = getCarriedItems(ch);
  let obj: ObjInstance | undefined;
  for (const item of items) {
    if (isName(objArg, item.name)) {
      obj = item;
      break;
    }
  }

  if (!obj) {
    sendToChar(ch, "You don't have that.\r\n");
    return;
  }

  // Find the target character in the room
  const roomVnum = getCharRoom(ch);
  const chars = world.getCharsInRoom(roomVnum);
  let victim: CharData | undefined;
  for (const other of chars) {
    if (other.id === ch.id) continue;
    if (isName(targetArg, other.name)) {
      victim = other;
      break;
    }
  }

  if (!victim) {
    sendToChar(ch, "They aren't here.\r\n");
    return;
  }

  objFromChar(obj);
  objToChar(obj, victim);
  sendToChar(ch, `You give ${obj.shortDescr} to ${victim.name}.\r\n`);
  sendToChar(victim, `${ch.name} gives you ${obj.shortDescr}.\r\n`);
  act('$n gives $p to $N.', ch, obj, victim, TO_NOTVICT);
}

// ============================================================================
//  Door commands — open, close, lock, unlock, pick, knock
// ============================================================================

/**
 * Find a door by direction name or exit keyword.
 * Returns the direction number and the exit data, or null if not found.
 * Port of find_door() from act_move.c.
 */
function findDoor(ch: CharData, arg: string): { dir: number; exit: ExitData } | null {
  const roomVnum = getCharRoom(ch);
  const room = world.getRoom(roomVnum);
  if (!room) return null;

  // First, try matching as a direction name
  const dirIndex = DIRECTION_NAMES.findIndex((name) => name.startsWith(arg.toLowerCase()));
  if (dirIndex >= 0) {
    const exit = room.exits[dirIndex as Direction];
    if (!exit) {
      sendToChar(ch, "There is no exit in that direction.\r\n");
      return null;
    }
    if (!(exit.exitInfo & EX_ISDOOR) && !(exit.rsFlags & EX_ISDOOR)) {
      sendToChar(ch, "There is no door in that direction.\r\n");
      return null;
    }
    return { dir: dirIndex, exit };
  }

  // Otherwise, try matching as an exit keyword
  for (let dir = 0; dir < 6; dir++) {
    const exit = room.exits[dir as Direction];
    if (exit && exit.keyword && isName(arg, exit.keyword)) {
      return { dir, exit };
    }
  }

  sendToChar(ch, "You don't see that here.\r\n");
  return null;
}

/**
 * Check if a character has a key with the given vnum in their inventory.
 */
function hasKey(ch: CharData, keyVnum: number): boolean {
  if (keyVnum <= 0) return false;
  const items = getInventory(ch);
  for (const obj of items) {
    if (obj.itemType === ItemType.KEY && obj.indexVnum === keyVnum) {
      return true;
    }
  }
  return false;
}

/**
 * Get the name of a door/exit for display purposes.
 */
function getDoorName(exit: ExitData): string {
  return exit.keyword || 'door';
}

/**
 * doOpen — Open a door or container.
 */
function doOpen(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Open what?\r\n");
    return;
  }

  // Try to open a container in inventory or room first
  const roomVnum = getCharRoom(ch);
  const container = findContainerForChar(ch, argument, roomVnum);
  if (container && container.itemType === ItemType.CONTAINER) {
    if (!(container.value[1] & CONT_CLOSEABLE)) {
      sendToChar(ch, "You can't do that.\r\n");
      return;
    }
    if (!(container.value[1] & CONT_CLOSED)) {
      sendToChar(ch, "It's already open.\r\n");
      return;
    }
    if (container.value[1] & CONT_LOCKED) {
      sendToChar(ch, "It's locked.\r\n");
      return;
    }

    container.value[1] &= ~CONT_CLOSED;
    sendToChar(ch, `You open ${container.shortDescr}.\r\n`);
    act('$n opens $p.', ch, container, null, TO_ROOM);
    return;
  }

  // Try to open a door/exit
  const result = findDoor(ch, argument);
  if (!result) return;

  const { dir, exit } = result;

  if (!(exit.exitInfo & EX_CLOSED)) {
    sendToChar(ch, "It's already open.\r\n");
    return;
  }

  if (exit.exitInfo & EX_LOCKED) {
    sendToChar(ch, "It's locked.\r\n");
    return;
  }

  // Open the door
  exit.exitInfo &= ~EX_CLOSED;

  const doorName = getDoorName(exit);
  sendToChar(ch, `You open the ${doorName}.\r\n`);
  act(`$n opens the ${doorName}.`, ch, null, null, TO_ROOM);

  // Open the reverse side
  const toRoom = world.getRoom(exit.toVnum);
  if (toRoom) {
    const revDir = REVERSE_DIR[dir];
    const revExit = toRoom.exits[revDir as Direction];
    if (revExit && (revExit.exitInfo & EX_CLOSED)) {
      revExit.exitInfo &= ~EX_CLOSED;
      // Notify players in the other room
      sendToRoom(exit.toVnum, `The ${getDoorName(revExit)} opens.\r\n`);
    }
  }
}

/**
 * doClose — Close a door or container.
 */
function doClose(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Close what?\r\n");
    return;
  }

  // Try container first
  const roomVnum = getCharRoom(ch);
  const container = findContainerForChar(ch, argument, roomVnum);
  if (container && container.itemType === ItemType.CONTAINER) {
    if (!(container.value[1] & CONT_CLOSEABLE)) {
      sendToChar(ch, "You can't do that.\r\n");
      return;
    }
    if (container.value[1] & CONT_CLOSED) {
      sendToChar(ch, "It's already closed.\r\n");
      return;
    }

    container.value[1] |= CONT_CLOSED;
    sendToChar(ch, `You close ${container.shortDescr}.\r\n`);
    act('$n closes $p.', ch, container, null, TO_ROOM);
    return;
  }

  // Try door/exit
  const result = findDoor(ch, argument);
  if (!result) return;

  const { dir, exit } = result;

  if (exit.exitInfo & EX_CLOSED) {
    sendToChar(ch, "It's already closed.\r\n");
    return;
  }

  exit.exitInfo |= EX_CLOSED;

  const doorName = getDoorName(exit);
  sendToChar(ch, `You close the ${doorName}.\r\n`);
  act(`$n closes the ${doorName}.`, ch, null, null, TO_ROOM);

  // Close the reverse side
  const toRoom = world.getRoom(exit.toVnum);
  if (toRoom) {
    const revDir = REVERSE_DIR[dir];
    const revExit = toRoom.exits[revDir as Direction];
    if (revExit && !(revExit.exitInfo & EX_CLOSED)) {
      revExit.exitInfo |= EX_CLOSED;
      sendToRoom(exit.toVnum, `The ${getDoorName(revExit)} closes.\r\n`);
    }
  }
}

/**
 * doLock — Lock a door or container. Requires a key in inventory.
 */
function doLock(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Lock what?\r\n");
    return;
  }

  // Try container first
  const roomVnum = getCharRoom(ch);
  const container = findContainerForChar(ch, argument, roomVnum);
  if (container && container.itemType === ItemType.CONTAINER) {
    if (!(container.value[1] & CONT_CLOSEABLE)) {
      sendToChar(ch, "You can't do that.\r\n");
      return;
    }
    if (!(container.value[1] & CONT_CLOSED)) {
      sendToChar(ch, "It's not closed.\r\n");
      return;
    }
    if (container.value[1] & CONT_LOCKED) {
      sendToChar(ch, "It's already locked.\r\n");
      return;
    }
    // Containers use value[2] as the key vnum
    if (!hasKey(ch, container.value[2])) {
      sendToChar(ch, "You lack the key.\r\n");
      return;
    }

    container.value[1] |= CONT_LOCKED;
    sendToChar(ch, `You lock ${container.shortDescr}.\r\n`);
    act('$n locks $p.', ch, container, null, TO_ROOM);
    return;
  }

  // Try door/exit
  const result = findDoor(ch, argument);
  if (!result) return;

  const { dir, exit } = result;

  if (!(exit.exitInfo & EX_CLOSED)) {
    sendToChar(ch, "It's not closed.\r\n");
    return;
  }

  if (exit.exitInfo & EX_LOCKED) {
    sendToChar(ch, "It's already locked.\r\n");
    return;
  }

  if (!hasKey(ch, exit.key)) {
    sendToChar(ch, "You lack the key.\r\n");
    return;
  }

  exit.exitInfo |= EX_LOCKED;

  const doorName = getDoorName(exit);
  sendToChar(ch, `You lock the ${doorName}.\r\n`);
  act(`$n locks the ${doorName}.`, ch, null, null, TO_ROOM);

  // Lock the reverse side
  const toRoom = world.getRoom(exit.toVnum);
  if (toRoom) {
    const revDir = REVERSE_DIR[dir];
    const revExit = toRoom.exits[revDir as Direction];
    if (revExit && !(revExit.exitInfo & EX_LOCKED)) {
      revExit.exitInfo |= EX_LOCKED;
    }
  }
}

/**
 * doUnlock — Unlock a door or container. Requires a key in inventory.
 */
function doUnlock(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Unlock what?\r\n");
    return;
  }

  // Try container first
  const roomVnum = getCharRoom(ch);
  const container = findContainerForChar(ch, argument, roomVnum);
  if (container && container.itemType === ItemType.CONTAINER) {
    if (!(container.value[1] & CONT_CLOSEABLE)) {
      sendToChar(ch, "You can't do that.\r\n");
      return;
    }
    if (!(container.value[1] & CONT_CLOSED)) {
      sendToChar(ch, "It's not closed.\r\n");
      return;
    }
    if (!(container.value[1] & CONT_LOCKED)) {
      sendToChar(ch, "It isn't locked.\r\n");
      return;
    }
    if (!hasKey(ch, container.value[2])) {
      sendToChar(ch, "You lack the key.\r\n");
      return;
    }

    container.value[1] &= ~CONT_LOCKED;
    sendToChar(ch, `You unlock ${container.shortDescr}.\r\n`);
    act('$n unlocks $p.', ch, container, null, TO_ROOM);
    return;
  }

  // Try door/exit
  const result = findDoor(ch, argument);
  if (!result) return;

  const { dir, exit } = result;

  if (!(exit.exitInfo & EX_CLOSED)) {
    sendToChar(ch, "It's not closed.\r\n");
    return;
  }

  if (!(exit.exitInfo & EX_LOCKED)) {
    sendToChar(ch, "It isn't locked.\r\n");
    return;
  }

  if (!hasKey(ch, exit.key)) {
    sendToChar(ch, "You lack the key.\r\n");
    return;
  }

  exit.exitInfo &= ~EX_LOCKED;

  const doorName = getDoorName(exit);
  sendToChar(ch, `You unlock the ${doorName}.\r\n`);
  act(`$n unlocks the ${doorName}.`, ch, null, null, TO_ROOM);

  // Unlock the reverse side
  const toRoom = world.getRoom(exit.toVnum);
  if (toRoom) {
    const revDir = REVERSE_DIR[dir];
    const revExit = toRoom.exits[revDir as Direction];
    if (revExit && (revExit.exitInfo & EX_LOCKED)) {
      revExit.exitInfo &= ~EX_LOCKED;
    }
  }
}

/**
 * doPick — Pick a lock on a door or container. Thief skill check.
 * Port of do_pick() from act_move.c.
 */
function doPick(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Pick what?\r\n");
    return;
  }

  // Check for thief class or immortal
  const THIEF_CLASS = CharClass.THIEF;
  const ASSASSIN_CLASS = CharClass.ASSASSIN;
  if (!ch.isNpc && ch.charClass !== THIEF_CLASS && ch.charClass !== ASSASSIN_CLASS && !isImmortal(ch)) {
    sendToChar(ch, "You don't know how to pick locks.\r\n");
    return;
  }

  // Try container first
  const roomVnum = getCharRoom(ch);
  const container = findContainerForChar(ch, argument, roomVnum);
  if (container && container.itemType === ItemType.CONTAINER) {
    if (!(container.value[1] & CONT_CLOSED)) {
      sendToChar(ch, "It's not closed.\r\n");
      return;
    }
    if (!(container.value[1] & CONT_LOCKED)) {
      sendToChar(ch, "It isn't locked.\r\n");
      return;
    }
    if (container.value[1] & CONT_PICKPROOF) {
      sendToChar(ch, "You failed.\r\n");
      return;
    }

    // Skill check: base 50% + 2% per level, max 95%
    const chance = Math.min(95, 50 + ch.level * 2);
    if (Math.floor(Math.random() * 100) >= chance) {
      sendToChar(ch, "You failed.\r\n");
      return;
    }

    container.value[1] &= ~CONT_LOCKED;
    sendToChar(ch, `You pick the lock on ${container.shortDescr}.\r\n`);
    act('$n picks the lock on $p.', ch, container, null, TO_ROOM);
    return;
  }

  // Try door/exit
  const result = findDoor(ch, argument);
  if (!result) return;

  const { dir, exit } = result;

  if (!(exit.exitInfo & EX_CLOSED)) {
    sendToChar(ch, "It's not closed.\r\n");
    return;
  }

  if (!(exit.exitInfo & EX_LOCKED)) {
    sendToChar(ch, "It isn't locked.\r\n");
    return;
  }

  if (exit.exitInfo & EX_PICKPROOF) {
    sendToChar(ch, "You failed.\r\n");
    return;
  }

  // Skill check
  const chance = Math.min(95, 50 + ch.level * 2);
  if (Math.floor(Math.random() * 100) >= chance) {
    sendToChar(ch, "You failed.\r\n");
    return;
  }

  exit.exitInfo &= ~EX_LOCKED;

  const doorName = getDoorName(exit);
  sendToChar(ch, `You pick the lock on the ${doorName}.\r\n`);
  act(`$n picks the lock on the ${doorName}.`, ch, null, null, TO_ROOM);

  // Also unlock reverse side
  const toRoom = world.getRoom(exit.toVnum);
  if (toRoom) {
    const revDir = REVERSE_DIR[dir];
    const revExit = toRoom.exits[revDir as Direction];
    if (revExit && (revExit.exitInfo & EX_LOCKED)) {
      revExit.exitInfo &= ~EX_LOCKED;
    }
  }
}

/**
 * doKnock — Knock on a door to notify people on the other side.
 */
function doKnock(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Knock on what?\r\n");
    return;
  }

  const result = findDoor(ch, argument);
  if (!result) return;

  const { dir, exit } = result;

  if (!(exit.exitInfo & EX_CLOSED)) {
    sendToChar(ch, "Why knock? It's open.\r\n");
    return;
  }

  const doorName = getDoorName(exit);
  sendToChar(ch, `You knock on the ${doorName}.\r\n`);
  act(`$n knocks on the ${doorName}.`, ch, null, null, TO_ROOM);

  // Notify the other side
  const toRoom = world.getRoom(exit.toVnum);
  if (toRoom) {
    sendToRoom(exit.toVnum, `Someone knocks on the ${doorName} from the other side.\r\n`);
  }
}

// ============================================================================
//  Container commands — put, empty
// ============================================================================

/**
 * doPut — Put an item from inventory into a container.
 * Syntax: "put <item> <container>"
 */
function doPut(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Put what in what?\r\n");
    return;
  }

  const parts = argument.split(/\s+/);
  if (parts.length < 2) {
    sendToChar(ch, "Put what in what?\r\n");
    return;
  }

  // Handle "put <item> in <container>" or "put <item> <container>"
  let itemArg = parts[0];
  let containerArg: string;

  if (parts.length >= 3 && parts[1].toLowerCase() === 'in') {
    containerArg = parts.slice(2).join(' ');
  } else {
    containerArg = parts.slice(1).join(' ');
  }

  const roomVnum = getCharRoom(ch);

  // Find the container in inventory or room
  const container = findContainerForChar(ch, containerArg, roomVnum);
  if (!container) {
    sendToChar(ch, "You don't see that container here.\r\n");
    return;
  }

  if (container.itemType !== ItemType.CONTAINER) {
    sendToChar(ch, "That's not a container.\r\n");
    return;
  }

  if (container.value[1] & CONT_CLOSED) {
    sendToChar(ch, "It is closed.\r\n");
    return;
  }

  // Handle "put all <container>"
  if (itemArg.toLowerCase() === 'all') {
    const items = getCarriedItems(ch);
    if (items.length === 0) {
      sendToChar(ch, "You aren't carrying anything.\r\n");
      return;
    }

    let putSomething = false;
    for (const obj of [...items]) {
      if (obj.id === container.id) continue;  // Can't put container in itself

      // Check weight capacity (value[0] = max weight)
      const containerWeight = getContainerContentWeight(container.id);
      if (container.value[0] !== 0 && containerWeight + obj.weight > container.value[0]) {
        sendToChar(ch, `${capitalize(obj.shortDescr)} won't fit.\r\n`);
        continue;
      }

      objFromChar(obj);
      obj.containedIn = container.id;
      obj.inRoom = undefined;
      obj.carriedBy = undefined;
      sendToChar(ch, `You put ${obj.shortDescr} in ${container.shortDescr}.\r\n`);
      act('$n puts $p in $P.', ch, obj, container as unknown as CharData, TO_ROOM);
      putSomething = true;
    }
    if (!putSomething) {
      sendToChar(ch, "You have nothing to put in there.\r\n");
    }
    return;
  }

  // Find the specific item in inventory
  const items = getCarriedItems(ch);
  let obj: ObjInstance | undefined;
  for (const item of items) {
    if (isName(itemArg, item.name)) {
      obj = item;
      break;
    }
  }

  if (!obj) {
    sendToChar(ch, "You don't have that.\r\n");
    return;
  }

  if (obj.id === container.id) {
    sendToChar(ch, "You can't fold it into itself.\r\n");
    return;
  }

  // Check weight capacity
  const containerWeight = getContainerContentWeight(container.id);
  if (container.value[0] !== 0 && containerWeight + obj.weight > container.value[0]) {
    sendToChar(ch, "It won't fit.\r\n");
    return;
  }

  objFromChar(obj);
  obj.containedIn = container.id;
  obj.inRoom = undefined;
  obj.carriedBy = undefined;
  sendToChar(ch, `You put ${obj.shortDescr} in ${container.shortDescr}.\r\n`);
  act('$n puts $p in $P.', ch, obj, container as unknown as CharData, TO_ROOM);
}

/**
 * doEmpty — Empty a container's contents onto the floor or into another container.
 * Syntax: "empty <container>" or "empty <container> into <container>"
 */
function doEmpty(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Empty what?\r\n");
    return;
  }

  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You are in the void.\r\n");
    return;
  }

  // Parse "empty <container> into <container>"
  const parts = argument.split(/\s+/);
  let sourceArg = parts[0];
  let destArg: string | null = null;

  if (parts.length >= 3 && parts[1].toLowerCase() === 'into') {
    destArg = parts.slice(2).join(' ');
  }

  // Find source container
  const source = findContainerForChar(ch, sourceArg, roomVnum);
  if (!source) {
    sendToChar(ch, "You don't see that container here.\r\n");
    return;
  }

  if (source.itemType !== ItemType.CONTAINER && source.itemType !== ItemType.DRINK_CON) {
    sendToChar(ch, "That's not a container.\r\n");
    return;
  }

  if (source.itemType === ItemType.CONTAINER && (source.value[1] & CONT_CLOSED)) {
    sendToChar(ch, "It is closed.\r\n");
    return;
  }

  // Handle drink containers
  if (source.itemType === ItemType.DRINK_CON) {
    if (source.value[1] <= 0) {
      sendToChar(ch, "It's already empty.\r\n");
      return;
    }
    source.value[1] = 0;
    sendToChar(ch, `You empty ${source.shortDescr}.\r\n`);
    act('$n empties $p.', ch, source, null, TO_ROOM);
    return;
  }

  const contents = getContainerContents(source.id);
  if (contents.length === 0) {
    sendToChar(ch, "It's already empty.\r\n");
    return;
  }

  // Find destination container (if specified)
  let dest: ObjInstance | null = null;
  if (destArg) {
    dest = findContainerForChar(ch, destArg, roomVnum);
    if (!dest) {
      sendToChar(ch, "You don't see that container here.\r\n");
      return;
    }
    if (dest.itemType !== ItemType.CONTAINER) {
      sendToChar(ch, "That's not a container.\r\n");
      return;
    }
    if (dest.value[1] & CONT_CLOSED) {
      sendToChar(ch, "The destination is closed.\r\n");
      return;
    }
    if (dest.id === source.id) {
      sendToChar(ch, "You can't empty a container into itself.\r\n");
      return;
    }
  }

  for (const obj of [...contents]) {
    obj.containedIn = undefined;
    if (dest) {
      // Check weight
      const destWeight = getContainerContentWeight(dest.id);
      if (dest.value[0] !== 0 && destWeight + obj.weight > dest.value[0]) {
        sendToChar(ch, `${capitalize(obj.shortDescr)} won't fit in ${dest.shortDescr}.\r\n`);
        continue;
      }
      obj.containedIn = dest.id;
      obj.inRoom = undefined;
      obj.carriedBy = undefined;
    } else {
      // Drop to room
      objToRoom(obj, roomVnum);
    }
  }

  if (dest) {
    sendToChar(ch, `You empty ${source.shortDescr} into ${dest.shortDescr}.\r\n`);
    act('$n empties $p.', ch, source, null, TO_ROOM);
  } else {
    sendToChar(ch, `You empty ${source.shortDescr} onto the ground.\r\n`);
    act('$n empties $p onto the ground.', ch, source, null, TO_ROOM);
  }
}

// ============================================================================
//  Container helper functions
// ============================================================================

/**
 * Find a container object in the character's inventory or in the room.
 */
function findContainerForChar(ch: CharData, arg: string, roomVnum: number): ObjInstance | null {
  // Check inventory first
  const carried = getCarriedItems(ch);
  for (const obj of carried) {
    if (isName(arg, obj.name)) {
      return obj;
    }
  }

  // Check room objects
  if (roomVnum >= 0) {
    const roomObjs = world.getObjsInRoom(roomVnum);
    for (const obj of roomObjs) {
      if (isName(arg, obj.name)) {
        return obj;
      }
    }
  }

  return null;
}

/**
 * Get all objects contained within a container (by container id).
 */
function getContainerContents(containerId: string): ObjInstance[] {
  const items: ObjInstance[] = [];
  for (const obj of world.objects.values()) {
    if (obj.deleted) continue;
    if (obj.containedIn === containerId) {
      items.push(obj);
    }
  }
  return items;
}

/**
 * Get the total weight of items inside a container.
 */
function getContainerContentWeight(containerId: string): number {
  let total = 0;
  for (const obj of world.objects.values()) {
    if (obj.deleted) continue;
    if (obj.containedIn === containerId) {
      total += obj.weight;
    }
  }
  return total;
}

// ============================================================================
//  Action commands
// ============================================================================

/**
 * doQuit — Save the character and disconnect the player.
 */
function doQuit(ch: CharData, _argument: string): void {
  if (ch.isNpc) {
    sendToChar(ch, "NPCs cannot quit.\r\n");
    return;
  }

  if (ch.position === Position.FIGHTING) {
    sendToChar(ch, "No way! You are fighting.\r\n");
    return;
  }

  // Save before quitting
  const uid = charOwnerMap.get(ch.id);
  if (uid) {
    saveCharacter(ch, uid).catch((err: unknown) => {
      console.error(`doQuit: failed to save ${ch.name}:`, err);
    });
  }

  sendToChar(ch, "Your character has been saved. Alas, all adventures must come to an end.\r\n");
  act('$n has left the game.', ch, null, null, TO_ROOM);

  // Remove from world
  charFromRoom(ch);
  ch.deleted = true;
  world.characters.delete(ch.id);
  world.pcData.delete(ch.id);
  charOwnerMap.delete(ch.id);
  unregisterConnection(ch.id);
}

/**
 * doSave — Save character to Firestore.
 */
function doSave(ch: CharData, _argument: string): void {
  if (ch.isNpc) return;

  const uid = charOwnerMap.get(ch.id);
  if (!uid) {
    sendToChar(ch, `${colors.brightRed}Error: no account linked. Contact an admin.${colors.reset}\r\n`);
    return;
  }

  saveCharacter(ch, uid)
    .then(() => {
      sendToChar(ch, `${colors.brightGreen}Character saved.${colors.reset}\r\n`);
    })
    .catch((err: unknown) => {
      console.error(`doSave: failed to save ${ch.name}:`, err);
      sendToChar(ch, `${colors.brightRed}Save failed. Please try again.${colors.reset}\r\n`);
    });
}

/**
 * doRecall — Teleport to the recall point (room 25000).
 */
function doRecall(ch: CharData, _argument: string): void {
  const RECALL_VNUM = 25000;

  if (ch.position === Position.FIGHTING) {
    sendToChar(ch, "You can't recall while fighting!\r\n");
    return;
  }

  const currentRoom = getCharRoom(ch);
  if (currentRoom === RECALL_VNUM) {
    sendToChar(ch, "You are already at your recall point.\r\n");
    return;
  }

  // Check if the PC has a custom recall point
  const pcData = world.pcData.get(ch.id);
  const recallVnum = pcData?.recall || RECALL_VNUM;
  const recallRoom = world.getRoom(recallVnum);

  if (!recallRoom) {
    sendToChar(ch, "Your recall point seems to have vanished!\r\n");
    return;
  }

  act('$n prays for transportation!', ch, null, null, TO_ROOM);
  charFromRoom(ch);
  charToRoom(ch, recallVnum);
  act('$n appears in the room.', ch, null, null, TO_ROOM);
  sendToChar(ch, `${colors.brightCyan}You pray to the gods for transportation!${colors.reset}\r\n`);
  doLook(ch, '');
}

// ============================================================================
//  Combat commands
// ============================================================================

/**
 * doKill — Initiate combat with an NPC.
 */
function doKill(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Kill whom?\r\n");
    return;
  }

  if (ch.position === Position.FIGHTING) {
    sendToChar(ch, "You are already fighting!\r\n");
    return;
  }

  const roomVnum = getCharRoom(ch);
  const chars = world.getCharsInRoom(roomVnum);

  for (const victim of chars) {
    if (victim.id === ch.id) continue;
    if (isName(argument, victim.name)) {
      if (!victim.isNpc) {
        sendToChar(ch, "You must MURDER players.\r\n");
        return;
      }

      if (isImmortal(victim)) {
        sendToChar(ch, "You can't attack them.\r\n");
        return;
      }

      sendToChar(ch, `${colors.brightRed}You attack ${victim.shortDescr}!${colors.reset}\r\n`);
      act('$n attacks $N!', ch, null, victim, TO_NOTVICT);
      sendToChar(victim, `${colors.brightRed}${ch.name} attacks you!${colors.reset}\r\n`);

      // Initiate combat
      setFighting(ch, victim);

      // Perform the first round of attacks immediately
      multiHit(ch, victim);
      return;
    }
  }

  sendToChar(ch, "They aren't here.\r\n");
}

/**
 * doFlee — Attempt to flee from combat.
 */
function doFlee(ch: CharData, _argument: string): void {
  if (ch.position !== Position.FIGHTING) {
    sendToChar(ch, "You aren't fighting anyone.\r\n");
    return;
  }

  // Try to find a random exit
  const roomVnum = getCharRoom(ch);
  const room = world.getRoom(roomVnum);
  if (!room) {
    sendToChar(ch, `${colors.brightRed}PANIC! You couldn't escape!${colors.reset}\r\n`);
    return;
  }

  const availableDirs: Direction[] = [];
  for (let dir = 0; dir < 6; dir++) {
    const exit = room.exits[dir as keyof typeof room.exits];
    if (exit && !(exit.exitInfo & EX_CLOSED)) {
      availableDirs.push(dir as Direction);
    }
  }

  if (availableDirs.length === 0) {
    sendToChar(ch, `${colors.brightRed}PANIC! You couldn't escape!${colors.reset}\r\n`);
    return;
  }

  // 75% chance of success
  if (Math.random() > 0.75) {
    sendToChar(ch, `${colors.brightRed}PANIC! You couldn't escape!${colors.reset}\r\n`);
    return;
  }

  const dir = availableDirs[Math.floor(Math.random() * availableDirs.length)];
  const exit = room.exits[dir as keyof typeof room.exits];
  if (!exit) return;

  const toRoom = world.getRoom(exit.toVnum);
  if (!toRoom) {
    sendToChar(ch, `${colors.brightRed}PANIC! You couldn't escape!${colors.reset}\r\n`);
    return;
  }

  // XP loss for fleeing
  const xpLoss = Math.floor(ch.level * 5);
  if (!ch.isNpc) {
    ch.exp = Math.max(0, ch.exp - xpLoss);
    sendToChar(ch, `${colors.yellow}You lose ${xpLoss} experience points for fleeing.${colors.reset}\r\n`);
  }

  // Stop fighting
  stopFighting(ch);

  // Move
  const DIRECTION_LABELS = ['north', 'east', 'south', 'west', 'up', 'down'];
  sendToChar(ch, `${colors.brightYellow}You flee ${DIRECTION_LABELS[dir]} from combat!${colors.reset}\r\n`);
  act('$n has fled!', ch, null, null, TO_ROOM);
  charFromRoom(ch);
  charToRoom(ch, exit.toVnum);
  act('$n arrives in a panic!', ch, null, null, TO_ROOM);

  // Show the new room
  doLook(ch, '');
}

/**
 * Wrapper for the consider command from fight.ts.
 */
function doConsiderCmd(ch: CharData, argument: string): void {
  doConsider(ch, argument);
}

/**
 * Wrapper for the wimpy command from fight.ts.
 */
function doWimpyCmd(ch: CharData, argument: string): void {
  doWimpy(ch, argument);
}

/**
 * Wrapper for the rest command from fight.ts.
 */
function doRestCmd(ch: CharData, argument: string): void {
  doRest(ch, argument);
}

/**
 * Wrapper for the sleep command from fight.ts.
 */
function doSleepCmd(ch: CharData, argument: string): void {
  doSleep(ch, argument);
}

/**
 * Wrapper for the stand command from fight.ts.
 */
function doStandCmd(ch: CharData, argument: string): void {
  doStand(ch, argument);
}

/**
 * Wrapper for the wake command from fight.ts.
 */
function doWakeCmd(ch: CharData, argument: string): void {
  doWake(ch, argument);
}

/**
 * doHelp — Show available commands.
 */
function doHelp(ch: CharData, argument: string): void {
  if (argument) {
    // Try to find a specific command
    const cmd = argument.toLowerCase();
    const entry = commandTable.find((e) => e.name.startsWith(cmd));
    if (entry) {
      sendToChar(ch, `${colors.brightCyan}${capitalize(entry.name)}${colors.reset}: Level ${entry.minLevel}+ command.\r\n`);
      return;
    }
    sendToChar(ch, "No help found for that topic.\r\n");
    return;
  }

  let buf = `${colors.brightCyan}--- Available Commands ---${colors.reset}\r\n\r\n`;

  // Group commands by category
  const categories: Record<string, string[]> = {
    'Movement': ['north', 'east', 'south', 'west', 'up', 'down'],
    'Information': ['look', 'score', 'who', 'inventory', 'equipment', 'exits', 'help'],
    'Communication': ['say', 'tell', 'shout', 'chat', 'speak', 'languages'],
    'Objects': ['get', 'drop', 'put', 'empty', 'wear', 'remove', 'give'],
    'Doors': ['open', 'close', 'lock', 'unlock', 'pick', 'knock'],
    'Actions': ['quit', 'save', 'recall'],
    'Combat': ['kill', 'flee', 'consider', 'wimpy'],
    'Position': ['rest', 'sleep', 'stand', 'wake'],
    'Social': ['clan', 'religion', 'propose', 'accept', 'divorce'],
    'Economy': ['bank'],
    'Quest': ['quest'],
    'Crafting': ['skin', 'tan', 'forge', 'brew', 'scribe'],
    'Gambling': ['slots', 'dice', 'seven', 'highdice'],
  };

  for (const [category, cmds] of Object.entries(categories)) {
    buf += `  ${colors.brightYellow}${category}:${colors.reset} ${cmds.join(', ')}\r\n`;
  }

  // Show admin/immortal/builder commands if the character qualifies
  if (ch.level >= 106) {
    const builderCmds = ['rstat', 'mstat', 'ostat'];
    buf += `  ${colors.brightYellow}Builder:${colors.reset} ${builderCmds.join(', ')}\r\n`;
  }
  if (ch.level >= 108) {
    const immCmds = ['goto', 'transfer', 'peace', 'slay', 'stat', 'force', 'wiznet'];
    buf += `  ${colors.brightYellow}Immortal:${colors.reset} ${immCmds.join(', ')}\r\n`;
  }
  if (ch.level >= 115) {
    const adminCmds = ['setrole', 'advance', 'restore', 'purge'];
    buf += `  ${colors.brightYellow}Admin:${colors.reset} ${adminCmds.join(', ')}\r\n`;
  }

  buf += `\r\nType ${colors.brightCyan}help <command>${colors.reset} for more information on a specific command.\r\n`;
  sendToChar(ch, buf);
}

// ============================================================================
//  Display helpers
// ============================================================================

function getPositionString(ch: CharData): string {
  switch (ch.position) {
    case Position.DEAD:       return 'DEAD';
    case Position.MORTAL:     return 'mortally wounded';
    case Position.INCAP:      return 'incapacitated';
    case Position.STUNNED:    return 'stunned';
    case Position.SLEEPING:   return 'sleeping';
    case Position.RESTING:    return 'resting';
    case Position.GHOST:      return 'a ghost';
    case Position.FIGHTING:   return 'fighting';
    case Position.STANDING:   return 'standing';
    case Position.MEDITATING: return 'meditating';
    default:                  return 'standing';
  }
}

function getConditionString(ch: CharData): string {
  const pct = ch.maxHit > 0 ? Math.floor((ch.hit * 100) / ch.maxHit) : 0;
  if (pct >= 100) return 'perfect';
  if (pct >= 90)  return 'excellent';
  if (pct >= 75)  return 'good';
  if (pct >= 50)  return 'fair';
  if (pct >= 30)  return 'wounded';
  if (pct >= 15)  return 'badly wounded';
  return 'nearly dead';
}

function getApplyName(location: number): string {
  const names: Record<number, string> = {
    0: 'none', 1: 'strength', 2: 'dexterity', 3: 'intelligence',
    4: 'wisdom', 5: 'constitution', 6: 'sex', 7: 'class',
    8: 'level', 9: 'age', 12: 'mana', 13: 'hit points',
    14: 'movement', 15: 'gold', 17: 'armor class',
    18: 'hitroll', 19: 'damroll', 20: 'save vs paralysis',
    21: 'save vs rod', 22: 'save vs petrification',
    23: 'save vs breath', 24: 'save vs spell',
  };
  return names[location] ?? 'unknown';
}
