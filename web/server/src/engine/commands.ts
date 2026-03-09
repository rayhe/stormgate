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
} from './types.js';

import {
  Position,
  Direction,
  WearLocation,
  Sex,
  ItemType,
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
  isImmortal,
  isName,
  capitalize,
} from './handler.js';

import {
  sendToChar,
  act,
  renderPrompt,
  colors,
  hasConnection,
  unregisterConnection,
  TO_ROOM,
  TO_VICT,
  TO_NOTVICT,
} from './output.js';

// ============================================================================
//  Command entry interface
// ============================================================================

interface CommandEntry {
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

const commandTable: CommandEntry[] = [
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
  { name: 'get',       fn: doGet,       minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'drop',      fn: doDrop,      minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'wear',      fn: doWear,      minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'remove',    fn: doRemove,    minPosition: Position.RESTING,  minLevel: 0, log: 0 },
  { name: 'give',      fn: doGive,      minPosition: Position.RESTING,  minLevel: 0, log: 0 },

  // Actions
  { name: 'quit',      fn: doQuit,      minPosition: Position.DEAD,     minLevel: 0, log: 0 },
  { name: 'save',      fn: doSave,      minPosition: Position.DEAD,     minLevel: 0, log: 0 },
  { name: 'recall',    fn: doRecall,    minPosition: Position.GHOST,    minLevel: 0, log: 0 },

  // Combat
  { name: 'kill',      fn: doKill,      minPosition: Position.FIGHTING, minLevel: 0, log: 0 },
  { name: 'flee',      fn: doFlee,      minPosition: Position.FIGHTING, minLevel: 0, log: 0 },
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
 * Move a character in a direction. Checks for valid exit, locked doors, etc.
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

  // Check for closed doors (exitInfo bit 1 = closed)
  if (exit.exitInfo & 1) {
    sendToChar(ch, "The door is closed.\r\n");
    return;
  }

  // Announce departure
  act(`$n leaves ${DIRECTION_NAMES[dir]}.`, ch, null, null, TO_ROOM);

  // Move the character
  charFromRoom(ch);
  charToRoom(ch, exit.toVnum);

  // Announce arrival
  act('$n has arrived.', ch, null, null, TO_ROOM);

  // Show the new room
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
      const closed = (exit.exitInfo & 1) ? ' (closed)' : '';
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
      found = true;
      const toRoom = world.getRoom(exit.toVnum);
      const closed = (exit.exitInfo & 1) ? ' (closed)' : '';
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
 * doSay — Broadcast message to current room.
 */
function doSay(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Say what?\r\n");
    return;
  }

  sendToChar(ch, `${colors.green}You say '${argument}'${colors.reset}\r\n`);
  act(`${colors.green}$n says '${argument}'${colors.reset}`, ch, null, null, TO_ROOM);
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
  sendToChar(victim, `${colors.magenta}${ch.name} tells you '${message}'${colors.reset}\r\n`);
}

/**
 * doShout — Broadcast to all connected players with [Shout] prefix.
 */
function doShout(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Shout what?\r\n");
    return;
  }

  sendToChar(ch, `${colors.brightYellow}You shout '${argument}'${colors.reset}\r\n`);

  for (const other of world.characters.values()) {
    if (other.isNpc || other.deleted || other.id === ch.id) continue;
    if (!hasConnection(other.id)) continue;
    sendToChar(other, `${colors.brightYellow}${ch.name} shouts '${argument}'${colors.reset}\r\n`);
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

  for (const other of world.characters.values()) {
    if (other.isNpc || other.deleted || other.id === ch.id) continue;
    if (!hasConnection(other.id)) continue;
    sendToChar(
      other,
      `${colors.brightMagenta}[Chat] ${ch.name}: ${argument}${colors.reset}\r\n`
    );
  }
}

// ============================================================================
//  Object manipulation commands
// ============================================================================

/**
 * doGet — Pick up an object from the room.
 */
function doGet(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Get what?\r\n");
    return;
  }

  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You are in the void.\r\n");
    return;
  }

  // Handle "get all"
  if (argument.toLowerCase() === 'all') {
    const objs = world.getObjsInRoom(roomVnum);
    if (objs.length === 0) {
      sendToChar(ch, "There is nothing here to get.\r\n");
      return;
    }
    let gotSomething = false;
    for (const obj of [...objs]) {
      objFromRoom(obj);
      objToChar(obj, ch);
      sendToChar(ch, `You get ${obj.shortDescr}.\r\n`);
      act('$n gets $p.', ch, obj, null, TO_ROOM);
      gotSomething = true;
    }
    if (!gotSomething) {
      sendToChar(ch, "There is nothing here to get.\r\n");
    }
    return;
  }

  // Find the specific object in the room
  const objs = world.getObjsInRoom(roomVnum);
  for (const obj of objs) {
    if (isName(argument, obj.name)) {
      objFromRoom(obj);
      objToChar(obj, ch);
      sendToChar(ch, `You get ${obj.shortDescr}.\r\n`);
      act('$n gets $p.', ch, obj, null, TO_ROOM);
      return;
    }
  }

  sendToChar(ch, "You don't see that here.\r\n");
}

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
//  Action commands
// ============================================================================

/**
 * doQuit — Disconnect the player.
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

  sendToChar(ch, "Alas, all adventures must come to an end.\r\n");
  act('$n has left the game.', ch, null, null, TO_ROOM);

  // Remove from world
  charFromRoom(ch);
  ch.deleted = true;
  world.characters.delete(ch.id);
  unregisterConnection(ch.id);
}

/**
 * doSave — Save character to persistent store.
 */
function doSave(ch: CharData, _argument: string): void {
  if (ch.isNpc) return;
  sendToChar(ch, `${colors.brightGreen}Character saved.${colors.reset}\r\n`);
  // TODO: Actually save to Firestore
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
//  Combat commands (stubs)
// ============================================================================

/**
 * doKill — Initiate combat (stub).
 */
function doKill(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Kill whom?\r\n");
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

      sendToChar(ch, `${colors.brightRed}You attack ${victim.shortDescr}!${colors.reset}\r\n`);
      act('$n attacks $N!', ch, null, victim, TO_NOTVICT);
      sendToChar(victim, `${colors.brightRed}${ch.name} attacks you!${colors.reset}\r\n`);
      ch.position = Position.FIGHTING;
      // TODO: Implement full combat system
      sendToChar(ch, `${colors.yellow}[Combat system coming soon]${colors.reset}\r\n`);
      return;
    }
  }

  sendToChar(ch, "They aren't here.\r\n");
}

/**
 * doFlee — Leave combat (stub).
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
    sendToChar(ch, "PANIC! You couldn't escape!\r\n");
    return;
  }

  const availableDirs: Direction[] = [];
  for (let dir = 0; dir < 6; dir++) {
    const exit = room.exits[dir as keyof typeof room.exits];
    if (exit && !(exit.exitInfo & 1)) {
      availableDirs.push(dir as Direction);
    }
  }

  if (availableDirs.length === 0) {
    sendToChar(ch, "PANIC! You couldn't escape!\r\n");
    return;
  }

  const dir = availableDirs[Math.floor(Math.random() * availableDirs.length)];
  ch.position = Position.STANDING;
  sendToChar(ch, `${colors.brightYellow}You flee from combat!${colors.reset}\r\n`);
  act('$n has fled!', ch, null, null, TO_ROOM);
  moveChar(ch, dir);
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
    'Communication': ['say', 'tell', 'shout', 'chat'],
    'Objects': ['get', 'drop', 'wear', 'remove', 'give'],
    'Actions': ['quit', 'save', 'recall'],
    'Combat': ['kill', 'flee'],
  };

  for (const [category, cmds] of Object.entries(categories)) {
    buf += `  ${colors.brightYellow}${category}:${colors.reset} ${cmds.join(', ')}\r\n`;
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
