/**
 * admin.ts — Immortal and admin commands for Stormgate MUD.
 *
 * Provides builder (106+), immortal (108+), and admin (115+) commands.
 * All commands are exported as CommandEntry arrays and registered into
 * the command table via registerAdminCommands().
 */

import type {
  CharData,
  ObjInstance,
} from './types.js';

import {
  Position,
  Sex,
  LEVEL_IMMORTAL,
} from './types.js';

import { world } from './world.js';

import {
  charToRoom,
  charFromRoom,
  getCharRoom,
  getEquippedItems,
  getCarriedItems,
  isImmortal,
  isName,
  capitalize,
  getInventory,
} from './handler.js';

import {
  sendToChar,
  colors,
  hasConnection,
} from './output.js';

import { interpret } from './commands.js';

// ============================================================================
//  Firestore reference (injected, for setrole)
// ============================================================================

let db: FirebaseFirestore.Firestore;

/**
 * Inject the Firestore reference for admin commands that need it
 * (e.g. setrole). Called from server.ts at startup.
 */
export function initAdminSystem(firestore: FirebaseFirestore.Firestore): void {
  db = firestore;
}

// ============================================================================
//  Character-owner map (imported from save.ts conceptually, but we
//  keep our own reference to avoid circular deps)
// ============================================================================

/** Map character id -> owner uid. Populated by server.ts on connect. */
let charOwnerMapRef: Map<string, string> | null = null;

export function setCharOwnerMap(map: Map<string, string>): void {
  charOwnerMapRef = map;
}

// ============================================================================
//  Class / race name tables (duplicated here for stat display)
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

const SEX_NAMES: Record<number, string> = {
  [Sex.NEUTRAL]: 'Neutral',
  [Sex.MALE]: 'Male',
  [Sex.FEMALE]: 'Female',
};

const POSITION_NAMES: Record<number, string> = {
  [Position.DEAD]: 'Dead',
  [Position.MORTAL]: 'Mortally wounded',
  [Position.INCAP]: 'Incapacitated',
  [Position.STUNNED]: 'Stunned',
  [Position.SLEEPING]: 'Sleeping',
  [Position.RESTING]: 'Resting',
  [Position.GHOST]: 'Ghost',
  [Position.FIGHTING]: 'Fighting',
  [Position.STANDING]: 'Standing',
  [Position.MEDITATING]: 'Meditating',
};

// ============================================================================
//  Helper: find a character in the world by name
// ============================================================================

function findCharWorld(name: string): CharData | undefined {
  for (const ch of world.characters.values()) {
    if (ch.deleted) continue;
    if (isName(name, ch.name)) return ch;
  }
  return undefined;
}

// ============================================================================
//  Admin commands (level 115)
// ============================================================================

/**
 * setrole <player> <role> — Set a player's Firebase/game role.
 * Valid roles: player, builder, immortal, admin.
 */
function doSetRole(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, `${colors.brightRed}Syntax: setrole <player> <role>${colors.reset}\r\n`);
    sendToChar(ch, `Valid roles: player, builder, immortal, admin\r\n`);
    return;
  }

  const parts = argument.split(/\s+/);
  if (parts.length < 2) {
    sendToChar(ch, `${colors.brightRed}Syntax: setrole <player> <role>${colors.reset}\r\n`);
    return;
  }

  const targetName = parts[0];
  const roleName = parts[1].toLowerCase();

  const validRoles = ['player', 'builder', 'immortal', 'admin'];
  if (!validRoles.includes(roleName)) {
    sendToChar(ch, `${colors.brightRed}Invalid role. Valid roles: ${validRoles.join(', ')}${colors.reset}\r\n`);
    return;
  }

  const victim = findCharWorld(targetName);
  if (!victim) {
    sendToChar(ch, "They aren't online.\r\n");
    return;
  }

  if (victim.isNpc) {
    sendToChar(ch, "NPCs don't have roles.\r\n");
    return;
  }

  // Map role to a trust/level value
  const roleLevelMap: Record<string, number> = {
    player: 0,
    builder: 106,
    immortal: 108,
    admin: 115,
  };

  const newTrust = roleLevelMap[roleName];

  // Update in-game trust
  victim.trust = newTrust;

  // If the role implies a level above the current one, set it
  if (roleName === 'immortal' && victim.level < LEVEL_IMMORTAL) {
    victim.level = LEVEL_IMMORTAL;
  } else if (roleName === 'builder' && victim.level < 106) {
    victim.level = 106;
  } else if (roleName === 'admin' && victim.level < 115) {
    victim.level = 115;
  }

  // Update Firestore user document
  if (db && charOwnerMapRef) {
    const uid = charOwnerMapRef.get(victim.id);
    if (uid) {
      db.collection('users')
        .doc(uid)
        .update({ role: roleName })
        .catch((err: unknown) => {
          console.error('setrole: Firestore update failed:', err);
        });
    }
  }

  sendToChar(ch, `${colors.brightGreen}${victim.name}'s role set to ${roleName}.${colors.reset}\r\n`);
  sendToChar(victim, `${colors.brightYellow}Your role has been set to ${roleName} by ${ch.name}.${colors.reset}\r\n`);
}

/**
 * advance <player> <level> — Set a player's level.
 */
function doAdvance(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, `${colors.brightRed}Syntax: advance <player> <level>${colors.reset}\r\n`);
    return;
  }

  const parts = argument.split(/\s+/);
  if (parts.length < 2) {
    sendToChar(ch, `${colors.brightRed}Syntax: advance <player> <level>${colors.reset}\r\n`);
    return;
  }

  const targetName = parts[0];
  const newLevel = parseInt(parts[1], 10);

  if (isNaN(newLevel) || newLevel < 1 || newLevel > 116) {
    sendToChar(ch, `${colors.brightRed}Level must be between 1 and 116.${colors.reset}\r\n`);
    return;
  }

  const victim = findCharWorld(targetName);
  if (!victim) {
    sendToChar(ch, "They aren't online.\r\n");
    return;
  }

  if (victim.isNpc) {
    sendToChar(ch, "You can't advance NPCs.\r\n");
    return;
  }

  if (newLevel >= ch.level && ch.level < 116) {
    sendToChar(ch, `${colors.brightRed}You can't advance someone to your level or above.${colors.reset}\r\n`);
    return;
  }

  const oldLevel = victim.level;
  victim.level = newLevel;

  sendToChar(ch, `${colors.brightGreen}${victim.name} advanced from level ${oldLevel} to level ${newLevel}.${colors.reset}\r\n`);
  sendToChar(victim, `${colors.brightYellow}You have been advanced to level ${newLevel} by ${ch.name}!${colors.reset}\r\n`);
}

/**
 * restore <player> — Fully restore HP/mana/move.
 */
function doRestore(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, `${colors.brightRed}Syntax: restore <player>${colors.reset}\r\n`);
    return;
  }

  const victim = findCharWorld(argument.trim());
  if (!victim) {
    sendToChar(ch, "They aren't online.\r\n");
    return;
  }

  victim.hit = victim.maxHit;
  victim.mana = victim.maxMana;
  victim.move = victim.maxMove;

  if (victim.maxBp > 0) {
    victim.bp = victim.maxBp;
  }

  sendToChar(ch, `${colors.brightGreen}${victim.name} has been restored.${colors.reset}\r\n`);
  sendToChar(victim, `${colors.brightYellow}You have been fully restored by ${ch.name}!${colors.reset}\r\n`);
}

/**
 * purge [target] — Remove all NPCs and objects from room, or a specific one.
 */
function doPurge(ch: CharData, argument: string): void {
  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You are in the void.\r\n");
    return;
  }

  if (!argument) {
    // Purge all NPCs and objects in the room
    const chars = world.getCharsInRoom(roomVnum);
    let purgedMobs = 0;
    for (const victim of [...chars]) {
      if (victim.id === ch.id) continue;
      if (!victim.isNpc) continue;
      charFromRoom(victim);
      victim.deleted = true;
      world.characters.delete(victim.id);
      purgedMobs++;
    }

    const objs = world.getObjsInRoom(roomVnum);
    let purgedObjs = 0;
    for (const obj of [...objs]) {
      obj.deleted = true;
      world.objects.delete(obj.id);
      purgedObjs++;
    }

    sendToChar(ch, `${colors.brightGreen}Purged ${purgedMobs} mob(s) and ${purgedObjs} object(s) from the room.${colors.reset}\r\n`);
    return;
  }

  // Try to find a specific target (NPC in the room)
  const chars = world.getCharsInRoom(roomVnum);
  for (const victim of chars) {
    if (victim.id === ch.id) continue;
    if (isName(argument, victim.name)) {
      if (!victim.isNpc) {
        sendToChar(ch, "You can't purge players. Use 'transfer' instead.\r\n");
        return;
      }
      charFromRoom(victim);
      victim.deleted = true;
      world.characters.delete(victim.id);
      sendToChar(ch, `${colors.brightGreen}${capitalize(victim.shortDescr)} purged.${colors.reset}\r\n`);
      return;
    }
  }

  // Try to find an object in the room
  const objs = world.getObjsInRoom(roomVnum);
  for (const obj of objs) {
    if (isName(argument, obj.name)) {
      obj.deleted = true;
      world.objects.delete(obj.id);
      sendToChar(ch, `${colors.brightGreen}${capitalize(obj.shortDescr)} purged.${colors.reset}\r\n`);
      return;
    }
  }

  sendToChar(ch, "You don't see that here.\r\n");
}

// ============================================================================
//  Immortal commands (level 108+)
// ============================================================================

/**
 * goto <vnum|player> — Teleport to a room vnum or another player.
 */
function doGoto(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, `${colors.brightRed}Syntax: goto <vnum|player>${colors.reset}\r\n`);
    return;
  }

  let targetVnum: number;

  // Try as a vnum first
  const vnum = parseInt(argument, 10);
  if (!isNaN(vnum) && world.getRoom(vnum)) {
    targetVnum = vnum;
  } else {
    // Try as a player name
    const victim = findCharWorld(argument);
    if (!victim) {
      sendToChar(ch, "No such room or player found.\r\n");
      return;
    }
    const victimRoom = getCharRoom(victim);
    if (victimRoom === -1) {
      sendToChar(ch, "That player is not in any room.\r\n");
      return;
    }
    targetVnum = victimRoom;
  }

  const currentRoom = getCharRoom(ch);

  // Announce departure
  if (currentRoom !== -1) {
    const pcdata = world.pcData.get(ch.id);
    const bamfout = pcdata?.bamfout || `${ch.name} vanishes in a puff of smoke.`;
    const chars = world.getCharsInRoom(currentRoom);
    for (const other of chars) {
      if (other.id === ch.id || other.deleted) continue;
      sendToChar(other, `${colors.brightMagenta}${bamfout}${colors.reset}\r\n`);
    }
  }

  charFromRoom(ch);
  charToRoom(ch, targetVnum);

  // Announce arrival
  const pcdata = world.pcData.get(ch.id);
  const bamfin = pcdata?.bamfin || `${ch.name} appears in a flash of light.`;
  const chars = world.getCharsInRoom(targetVnum);
  for (const other of chars) {
    if (other.id === ch.id || other.deleted) continue;
    sendToChar(other, `${colors.brightMagenta}${bamfin}${colors.reset}\r\n`);
  }

  // Show the room to the teleporting character
  interpret(ch, 'look');
}

/**
 * transfer <player> [vnum] — Teleport a player to your location or to a vnum.
 */
function doTransfer(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, `${colors.brightRed}Syntax: transfer <player> [vnum]${colors.reset}\r\n`);
    return;
  }

  const parts = argument.split(/\s+/);
  const targetName = parts[0];
  const targetVnumStr = parts[1];

  const victim = findCharWorld(targetName);
  if (!victim) {
    sendToChar(ch, "They aren't online.\r\n");
    return;
  }

  if (victim.id === ch.id) {
    sendToChar(ch, "You can't transfer yourself.\r\n");
    return;
  }

  let destVnum: number;

  if (targetVnumStr) {
    destVnum = parseInt(targetVnumStr, 10);
    if (isNaN(destVnum) || !world.getRoom(destVnum)) {
      sendToChar(ch, "That room does not exist.\r\n");
      return;
    }
  } else {
    destVnum = getCharRoom(ch);
    if (destVnum === -1) {
      sendToChar(ch, "You are not in a room.\r\n");
      return;
    }
  }

  const victimRoom = getCharRoom(victim);
  if (victimRoom !== -1) {
    const roomChars = world.getCharsInRoom(victimRoom);
    for (const other of roomChars) {
      if (other.id === victim.id || other.deleted) continue;
      sendToChar(other, `${colors.brightMagenta}${victim.name} vanishes!${colors.reset}\r\n`);
    }
  }

  charFromRoom(victim);
  charToRoom(victim, destVnum);

  sendToChar(victim, `${colors.brightYellow}${ch.name} has transferred you!${colors.reset}\r\n`);
  interpret(victim, 'look');

  sendToChar(ch, `${colors.brightGreen}${victim.name} transferred to room ${destVnum}.${colors.reset}\r\n`);
}

/**
 * peace — Stop all fighting in the current room.
 */
function doPeace(ch: CharData, _argument: string): void {
  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You are in the void.\r\n");
    return;
  }

  const chars = world.getCharsInRoom(roomVnum);
  let stopped = 0;

  for (const victim of chars) {
    if (victim.position === Position.FIGHTING) {
      victim.position = Position.STANDING;
      stopped++;
    }
  }

  sendToChar(ch, `${colors.brightGreen}Peace has been restored. ${stopped} combatant(s) stopped.${colors.reset}\r\n`);

  for (const other of chars) {
    if (other.id === ch.id) continue;
    sendToChar(other, `${colors.brightYellow}A wave of calm passes over the room.${colors.reset}\r\n`);
  }
}

/**
 * slay <target> — Instantly kill an NPC (or a PC if admin level).
 */
function doSlay(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, `${colors.brightRed}Syntax: slay <target>${colors.reset}\r\n`);
    return;
  }

  const roomVnum = getCharRoom(ch);
  const chars = world.getCharsInRoom(roomVnum);

  for (const victim of chars) {
    if (victim.id === ch.id) continue;
    if (isName(argument, victim.name)) {
      // Can't slay PCs unless you're admin level (115+)
      if (!victim.isNpc && ch.level < 115) {
        sendToChar(ch, `${colors.brightRed}You can only slay NPCs at your level. Admin access required for players.${colors.reset}\r\n`);
        return;
      }

      sendToChar(ch, `${colors.brightRed}You slay ${victim.name} in cold blood!${colors.reset}\r\n`);
      sendToChar(victim, `${colors.brightRed}${ch.name} slays you in cold blood!${colors.reset}\r\n`);

      for (const other of chars) {
        if (other.id === ch.id || other.id === victim.id || other.deleted) continue;
        sendToChar(other, `${colors.brightRed}${ch.name} slays ${victim.name} in cold blood!${colors.reset}\r\n`);
      }

      // Kill the victim
      victim.hit = -10;
      victim.position = Position.DEAD;

      if (victim.isNpc) {
        charFromRoom(victim);
        victim.deleted = true;
        world.characters.delete(victim.id);
      } else {
        // For PCs, set them to dead but don't delete — they'll need to be restored
        victim.position = Position.DEAD;
        sendToChar(victim, `${colors.brightRed}You are DEAD!${colors.reset}\r\n`);
      }

      return;
    }
  }

  sendToChar(ch, "They aren't here.\r\n");
}

/**
 * stat <target> — Show detailed stats of a character or object.
 */
function doStat(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, `${colors.brightRed}Syntax: stat <target>${colors.reset}\r\n`);
    return;
  }

  const roomVnum = getCharRoom(ch);

  // Try to find a character first (in room or world-wide)
  let victim = findCharWorld(argument);
  if (victim) {
    statCharacter(ch, victim);
    return;
  }

  // Try to find an object in inventory
  if (roomVnum !== -1) {
    const inventory = getInventory(ch);
    for (const obj of inventory) {
      if (isName(argument, obj.name)) {
        statObject(ch, obj);
        return;
      }
    }

    // Try to find an object in the room
    const roomObjs = world.getObjsInRoom(roomVnum);
    for (const obj of roomObjs) {
      if (isName(argument, obj.name)) {
        statObject(ch, obj);
        return;
      }
    }
  }

  sendToChar(ch, "Nothing by that name found.\r\n");
}

/**
 * Display detailed character stats.
 */
function statCharacter(ch: CharData, victim: CharData): void {
  const className = CLASS_NAMES[victim.charClass] ?? 'Unknown';
  const raceName = RACE_NAMES[victim.race] ?? 'Unknown';
  const sexName = SEX_NAMES[victim.sex] ?? 'Unknown';
  const posName = POSITION_NAMES[victim.position] ?? 'Unknown';
  const victimRoom = getCharRoom(victim);
  const pcdata = world.pcData.get(victim.id);

  let buf = '';
  buf += `${colors.brightCyan}+--- Character Stat: ${victim.name} ---+${colors.reset}\r\n`;
  buf += `  ${colors.brightWhite}ID:${colors.reset} ${victim.id}\r\n`;
  buf += `  ${colors.brightWhite}Name:${colors.reset} ${victim.name}  ${colors.brightWhite}Short:${colors.reset} ${victim.shortDescr}\r\n`;
  buf += `  ${colors.brightWhite}NPC:${colors.reset} ${victim.isNpc ? 'Yes' : 'No'}  ${colors.brightWhite}Level:${colors.reset} ${victim.level}  ${colors.brightWhite}Trust:${colors.reset} ${victim.trust}\r\n`;
  buf += `  ${colors.brightWhite}Class:${colors.reset} ${className} (${victim.charClass})  ${colors.brightWhite}Race:${colors.reset} ${raceName} (${victim.race})\r\n`;
  buf += `  ${colors.brightWhite}Sex:${colors.reset} ${sexName}  ${colors.brightWhite}Position:${colors.reset} ${posName}\r\n`;
  buf += `  ${colors.brightWhite}Room:${colors.reset} ${victimRoom}\r\n`;
  buf += `\r\n`;
  buf += `  ${colors.brightWhite}HP:${colors.reset} ${victim.hit}/${victim.maxHit}  ${colors.brightWhite}Mana:${colors.reset} ${victim.mana}/${victim.maxMana}  ${colors.brightWhite}Move:${colors.reset} ${victim.move}/${victim.maxMove}\r\n`;
  if (victim.maxBp > 0) {
    buf += `  ${colors.brightWhite}BP:${colors.reset} ${victim.bp}/${victim.maxBp}\r\n`;
  }
  buf += `  ${colors.brightWhite}Hitroll:${colors.reset} ${victim.hitroll}  ${colors.brightWhite}Damroll:${colors.reset} ${victim.damroll}  ${colors.brightWhite}Armor:${colors.reset} ${victim.armor}\r\n`;
  buf += `  ${colors.brightWhite}Saving:${colors.reset} ${victim.savingThrow}  ${colors.brightWhite}Alignment:${colors.reset} ${victim.alignment}\r\n`;
  buf += `  ${colors.brightWhite}Gold:${colors.reset} ${victim.gold}  ${colors.brightWhite}Exp:${colors.reset} ${victim.exp}  ${colors.brightWhite}Practice:${colors.reset} ${victim.practice}\r\n`;
  buf += `\r\n`;
  buf += `  ${colors.brightWhite}Clan:${colors.reset} ${victim.clan}  ${colors.brightWhite}Religion:${colors.reset} ${victim.religion}  ${colors.brightWhite}QP:${colors.reset} ${victim.questpoints}\r\n`;
  buf += `  ${colors.brightWhite}Carry:${colors.reset} ${victim.carryNumber} items, ${victim.carryWeight} lbs\r\n`;
  buf += `\r\n`;
  buf += `  ${colors.brightWhite}AffBy:${colors.reset} ${victim.affectedBy}  ${colors.brightWhite}AffBy2:${colors.reset} ${victim.affectedBy2}  ${colors.brightWhite}AffBy3:${colors.reset} ${victim.affectedBy3}  ${colors.brightWhite}AffBy4:${colors.reset} ${victim.affectedBy4}\r\n`;
  buf += `  ${colors.brightWhite}Act:${colors.reset} ${victim.act}  ${colors.brightWhite}Act2:${colors.reset} ${victim.act2}\r\n`;

  if (pcdata) {
    buf += `\r\n  ${colors.brightYellow}--- PcData ---${colors.reset}\r\n`;
    buf += `  ${colors.brightWhite}Title:${colors.reset} ${pcdata.title}\r\n`;
    buf += `  ${colors.brightWhite}Stats:${colors.reset} Str:${pcdata.permStr} Int:${pcdata.permInt} Wis:${pcdata.permWis} Dex:${pcdata.permDex} Con:${pcdata.permCon}\r\n`;
    buf += `  ${colors.brightWhite}ModStats:${colors.reset} Str:${pcdata.modStr} Int:${pcdata.modInt} Wis:${pcdata.modWis} Dex:${pcdata.modDex} Con:${pcdata.modCon}\r\n`;
    buf += `  ${colors.brightWhite}Bank:${colors.reset} ${pcdata.bankAccount}  ${colors.brightWhite}Security:${colors.reset} ${pcdata.security}\r\n`;
    buf += `  ${colors.brightWhite}Recall:${colors.reset} ${pcdata.recall}  ${colors.brightWhite}Email:${colors.reset} ${pcdata.email}\r\n`;
    buf += `  ${colors.brightWhite}Skills:${colors.reset} ${pcdata.learned.size} learned\r\n`;
    buf += `  ${colors.brightWhite}Aliases:${colors.reset} ${pcdata.aliases.size} defined\r\n`;
  }

  // Affects
  if (victim.affects.length > 0) {
    buf += `\r\n  ${colors.brightYellow}--- Affects ---${colors.reset}\r\n`;
    for (const af of victim.affects) {
      if (af.deleted) continue;
      buf += `  Sn: ${af.type}  Lvl: ${af.level}  Dur: ${af.duration}  Mod: ${af.modifier}  Loc: ${af.location}  Bits: ${af.bitvector}\r\n`;
    }
  }

  // Equipment
  const equip = getEquippedItems(victim);
  if (equip.length > 0) {
    buf += `\r\n  ${colors.brightYellow}--- Equipment ---${colors.reset}\r\n`;
    for (const obj of equip) {
      buf += `  [${obj.wearLoc}] ${obj.shortDescr} (vnum ${obj.indexVnum})\r\n`;
    }
  }

  // Inventory
  const inv = getCarriedItems(victim);
  if (inv.length > 0) {
    buf += `\r\n  ${colors.brightYellow}--- Inventory ---${colors.reset}\r\n`;
    for (const obj of inv) {
      buf += `  ${obj.shortDescr} (vnum ${obj.indexVnum})\r\n`;
    }
  }

  buf += `${colors.brightCyan}+--- End Stat ---+${colors.reset}\r\n`;
  sendToChar(ch, buf);
}

/**
 * Display detailed object stats.
 */
function statObject(ch: CharData, obj: ObjInstance): void {
  let buf = '';
  buf += `${colors.brightCyan}+--- Object Stat: ${obj.shortDescr} ---+${colors.reset}\r\n`;
  buf += `  ${colors.brightWhite}ID:${colors.reset} ${obj.id}\r\n`;
  buf += `  ${colors.brightWhite}Vnum:${colors.reset} ${obj.indexVnum}  ${colors.brightWhite}Type:${colors.reset} ${obj.itemType}\r\n`;
  buf += `  ${colors.brightWhite}Name:${colors.reset} ${obj.name}\r\n`;
  buf += `  ${colors.brightWhite}Short:${colors.reset} ${obj.shortDescr}\r\n`;
  buf += `  ${colors.brightWhite}Long:${colors.reset} ${obj.description}\r\n`;
  buf += `  ${colors.brightWhite}Level:${colors.reset} ${obj.level}  ${colors.brightWhite}Weight:${colors.reset} ${obj.weight}  ${colors.brightWhite}Cost:${colors.reset} ${obj.cost}\r\n`;
  buf += `  ${colors.brightWhite}WearFlags:${colors.reset} ${obj.wearFlags}  ${colors.brightWhite}WearLoc:${colors.reset} ${obj.wearLoc}\r\n`;
  buf += `  ${colors.brightWhite}ExtraFlags:${colors.reset} ${obj.extraFlags} / ${obj.extraFlags2} / ${obj.extraFlags3} / ${obj.extraFlags4}\r\n`;
  buf += `  ${colors.brightWhite}Values:${colors.reset} [${obj.value.join(', ')}]\r\n`;
  buf += `  ${colors.brightWhite}Durability:${colors.reset} ${obj.durabilityCur}/${obj.durabilityMax}\r\n`;
  buf += `  ${colors.brightWhite}Timer:${colors.reset} ${obj.timer}\r\n`;

  if (obj.carriedBy) {
    const carrier = world.getCharById(obj.carriedBy);
    buf += `  ${colors.brightWhite}Carried by:${colors.reset} ${carrier?.name ?? obj.carriedBy}\r\n`;
  }
  if (obj.inRoom !== undefined) {
    buf += `  ${colors.brightWhite}In room:${colors.reset} ${obj.inRoom}\r\n`;
  }

  if (obj.affects.length > 0) {
    buf += `\r\n  ${colors.brightYellow}--- Affects ---${colors.reset}\r\n`;
    for (const af of obj.affects) {
      buf += `  Loc: ${af.location}  Mod: ${af.modifier}  Bits: ${af.bitvector}\r\n`;
    }
  }

  buf += `${colors.brightCyan}+--- End Stat ---+${colors.reset}\r\n`;
  sendToChar(ch, buf);
}

/**
 * force <player> <command> — Force a player to execute a command.
 */
function doForce(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, `${colors.brightRed}Syntax: force <player> <command>${colors.reset}\r\n`);
    return;
  }

  const spaceIdx = argument.indexOf(' ');
  if (spaceIdx === -1) {
    sendToChar(ch, `${colors.brightRed}Force them to do what?${colors.reset}\r\n`);
    return;
  }

  const targetName = argument.substring(0, spaceIdx);
  const command = argument.substring(spaceIdx + 1).trim();

  if (!command) {
    sendToChar(ch, `${colors.brightRed}Force them to do what?${colors.reset}\r\n`);
    return;
  }

  // Handle "force all <command>"
  if (targetName.toLowerCase() === 'all') {
    sendToChar(ch, `${colors.brightGreen}Forcing all players to: ${command}${colors.reset}\r\n`);
    for (const victim of world.characters.values()) {
      if (victim.isNpc || victim.deleted || victim.id === ch.id) continue;
      if (!hasConnection(victim.id)) continue;
      if (victim.level >= ch.level) continue;
      sendToChar(victim, `${colors.brightYellow}${ch.name} forces you to '${command}'.${colors.reset}\r\n`);
      interpret(victim, command);
    }
    return;
  }

  const victim = findCharWorld(targetName);
  if (!victim) {
    sendToChar(ch, "They aren't online.\r\n");
    return;
  }

  if (victim.id === ch.id) {
    sendToChar(ch, "You can just type the command yourself.\r\n");
    return;
  }

  if (!victim.isNpc && victim.level >= ch.level) {
    sendToChar(ch, "You can't force someone of equal or higher level.\r\n");
    return;
  }

  sendToChar(ch, `${colors.brightGreen}You force ${victim.name} to '${command}'.${colors.reset}\r\n`);
  sendToChar(victim, `${colors.brightYellow}${ch.name} forces you to '${command}'.${colors.reset}\r\n`);
  interpret(victim, command);
}

/**
 * wiznet <message> — Send message to all immortals online.
 */
function doWiznet(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, `${colors.brightRed}Syntax: wiznet <message>${colors.reset}\r\n`);
    return;
  }

  const msg = `${colors.brightCyan}[Wiznet] ${ch.name}: ${argument}${colors.reset}\r\n`;

  for (const other of world.characters.values()) {
    if (other.isNpc || other.deleted) continue;
    if (!hasConnection(other.id)) continue;
    if (!isImmortal(other)) continue;
    sendToChar(other, msg);
  }
}

// ============================================================================
//  Builder commands (level 106+)
// ============================================================================

/**
 * rstat — Show current room's detailed data.
 */
function doRstat(ch: CharData, _argument: string): void {
  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You are in the void.\r\n");
    return;
  }

  const room = world.getRoom(roomVnum);
  if (!room) {
    sendToChar(ch, "Error: room not found.\r\n");
    return;
  }

  let buf = '';
  buf += `${colors.brightCyan}+--- Room Stat ---+${colors.reset}\r\n`;
  buf += `  ${colors.brightWhite}Vnum:${colors.reset} ${room.vnum}\r\n`;
  buf += `  ${colors.brightWhite}Name:${colors.reset} ${room.name}\r\n`;
  buf += `  ${colors.brightWhite}Area:${colors.reset} ${room.areaKey}\r\n`;
  buf += `  ${colors.brightWhite}Sector:${colors.reset} ${room.sectorType}\r\n`;
  buf += `  ${colors.brightWhite}Flags:${colors.reset} ${room.roomFlags}\r\n`;
  buf += `  ${colors.brightWhite}Light:${colors.reset} ${room.light}\r\n`;
  buf += `  ${colors.brightWhite}Description:${colors.reset}\r\n    ${room.description}\r\n`;

  // Exits
  buf += `\r\n  ${colors.brightYellow}--- Exits ---${colors.reset}\r\n`;
  const DIRECTION_NAMES = ['North', 'East', 'South', 'West', 'Up', 'Down'];
  let hasExits = false;
  for (let dir = 0; dir < 6; dir++) {
    const exit = room.exits[dir as keyof typeof room.exits];
    if (exit) {
      hasExits = true;
      buf += `  ${DIRECTION_NAMES[dir]}: to ${exit.toVnum}  key: ${exit.key}  flags: ${exit.exitInfo}`;
      if (exit.keyword) buf += `  keyword: "${exit.keyword}"`;
      buf += `\r\n`;
    }
  }
  if (!hasExits) {
    buf += `  None.\r\n`;
  }

  // Extra descriptions
  if (room.extraDescriptions.length > 0) {
    buf += `\r\n  ${colors.brightYellow}--- Extra Descriptions ---${colors.reset}\r\n`;
    for (const ed of room.extraDescriptions) {
      buf += `  Keyword: "${ed.keyword}"\r\n`;
    }
  }

  // Characters in room
  const charsInRoom = world.getCharsInRoom(roomVnum);
  if (charsInRoom.length > 0) {
    buf += `\r\n  ${colors.brightYellow}--- Characters (${charsInRoom.length}) ---${colors.reset}\r\n`;
    for (const other of charsInRoom) {
      buf += `  ${other.name} [${other.isNpc ? 'NPC' : 'PC'}] Lvl ${other.level}\r\n`;
    }
  }

  // Objects in room
  const objsInRoom = world.getObjsInRoom(roomVnum);
  if (objsInRoom.length > 0) {
    buf += `\r\n  ${colors.brightYellow}--- Objects (${objsInRoom.length}) ---${colors.reset}\r\n`;
    for (const obj of objsInRoom) {
      buf += `  ${obj.shortDescr} (vnum ${obj.indexVnum})\r\n`;
    }
  }

  buf += `${colors.brightCyan}+--- End Room Stat ---+${colors.reset}\r\n`;
  sendToChar(ch, buf);
}

/**
 * mstat <mob> — Show a mob's template data.
 */
function doMstat(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, `${colors.brightRed}Syntax: mstat <mob vnum | mob name>${colors.reset}\r\n`);
    return;
  }

  // Try as vnum first
  const vnum = parseInt(argument, 10);
  if (!isNaN(vnum)) {
    const template = world.getMobTemplate(vnum);
    if (template) {
      let buf = '';
      buf += `${colors.brightCyan}+--- Mob Template Stat ---+${colors.reset}\r\n`;
      buf += `  ${colors.brightWhite}Vnum:${colors.reset} ${template.vnum}\r\n`;
      buf += `  ${colors.brightWhite}Name:${colors.reset} ${template.name}\r\n`;
      buf += `  ${colors.brightWhite}Short:${colors.reset} ${template.shortDescr}\r\n`;
      buf += `  ${colors.brightWhite}Long:${colors.reset} ${template.longDescr}\r\n`;
      buf += `  ${colors.brightWhite}Level:${colors.reset} ${template.level}  ${colors.brightWhite}Class:${colors.reset} ${CLASS_NAMES[template.charClass] ?? template.charClass}\r\n`;
      buf += `  ${colors.brightWhite}Sex:${colors.reset} ${SEX_NAMES[template.sex] ?? template.sex}\r\n`;
      buf += `  ${colors.brightWhite}HP Dice:${colors.reset} ${template.hitnodice}d${template.hitsizedice}+${template.hitplus}\r\n`;
      buf += `  ${colors.brightWhite}Dam Dice:${colors.reset} ${template.damnodice}d${template.damsizedice}+${template.damplus}\r\n`;
      buf += `  ${colors.brightWhite}Hitroll:${colors.reset} ${template.hitroll}  ${colors.brightWhite}Damroll:${colors.reset} ${template.damroll}  ${colors.brightWhite}AC:${colors.reset} ${template.ac}\r\n`;
      buf += `  ${colors.brightWhite}Alignment:${colors.reset} ${template.alignment}  ${colors.brightWhite}Gold:${colors.reset} ${template.gold}\r\n`;
      buf += `  ${colors.brightWhite}Size:${colors.reset} ${template.size}  ${colors.brightWhite}Shields:${colors.reset} ${template.shields}\r\n`;
      buf += `  ${colors.brightWhite}Act:${colors.reset} ${template.act}  ${colors.brightWhite}Act2:${colors.reset} ${template.act2}\r\n`;
      buf += `  ${colors.brightWhite}AffBy:${colors.reset} ${template.affectedBy}  ${colors.brightWhite}AffBy2:${colors.reset} ${template.affectedBy2}\r\n`;
      buf += `  ${colors.brightWhite}Imm:${colors.reset} ${template.immFlags}  ${colors.brightWhite}Res:${colors.reset} ${template.resFlags}  ${colors.brightWhite}Vul:${colors.reset} ${template.vulFlags}\r\n`;
      if (template.mobprogs && template.mobprogs.length > 0) {
        buf += `  ${colors.brightWhite}MobProgs:${colors.reset} ${template.mobprogs.length}\r\n`;
      }
      buf += `${colors.brightCyan}+--- End Mob Template ---+${colors.reset}\r\n`;
      sendToChar(ch, buf);
      return;
    }
  }

  // Try as a name — find a mob in the room
  const roomVnum = getCharRoom(ch);
  const chars = world.getCharsInRoom(roomVnum);
  for (const victim of chars) {
    if (!victim.isNpc) continue;
    if (isName(argument, victim.name)) {
      // Use the stat character function for live instances
      statCharacter(ch, victim);
      return;
    }
  }

  sendToChar(ch, "No mob found with that vnum or name.\r\n");
}

/**
 * ostat <obj> — Show an object's template data.
 */
function doOstat(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, `${colors.brightRed}Syntax: ostat <obj vnum | obj name>${colors.reset}\r\n`);
    return;
  }

  // Try as vnum first
  const vnum = parseInt(argument, 10);
  if (!isNaN(vnum)) {
    const template = world.getObjTemplate(vnum);
    if (template) {
      let buf = '';
      buf += `${colors.brightCyan}+--- Obj Template Stat ---+${colors.reset}\r\n`;
      buf += `  ${colors.brightWhite}Vnum:${colors.reset} ${template.vnum}\r\n`;
      buf += `  ${colors.brightWhite}Name:${colors.reset} ${template.name}\r\n`;
      buf += `  ${colors.brightWhite}Short:${colors.reset} ${template.shortDescr}\r\n`;
      buf += `  ${colors.brightWhite}Long:${colors.reset} ${template.description}\r\n`;
      buf += `  ${colors.brightWhite}Type:${colors.reset} ${template.itemType}  ${colors.brightWhite}Level:${colors.reset} ${template.level}\r\n`;
      buf += `  ${colors.brightWhite}Weight:${colors.reset} ${template.weight}  ${colors.brightWhite}Cost:${colors.reset} ${template.cost}\r\n`;
      buf += `  ${colors.brightWhite}WearFlags:${colors.reset} ${template.wearFlags}\r\n`;
      buf += `  ${colors.brightWhite}ExtraFlags:${colors.reset} ${template.extraFlags} / ${template.extraFlags2} / ${template.extraFlags3} / ${template.extraFlags4}\r\n`;
      buf += `  ${colors.brightWhite}Values:${colors.reset} [${template.value.join(', ')}]\r\n`;
      if (template.affects.length > 0) {
        buf += `\r\n  ${colors.brightYellow}--- Affects ---${colors.reset}\r\n`;
        for (const af of template.affects) {
          buf += `  Loc: ${af.location}  Mod: ${af.modifier}  Bits: ${af.bitvector}\r\n`;
        }
      }
      if (template.extraDescriptions.length > 0) {
        buf += `\r\n  ${colors.brightYellow}--- Extra Descriptions ---${colors.reset}\r\n`;
        for (const ed of template.extraDescriptions) {
          buf += `  Keyword: "${ed.keyword}"\r\n`;
        }
      }
      buf += `${colors.brightCyan}+--- End Obj Template ---+${colors.reset}\r\n`;
      sendToChar(ch, buf);
      return;
    }
  }

  // Try as a name — find in inventory or room
  const roomVnum = getCharRoom(ch);

  const inventory = getInventory(ch);
  for (const obj of inventory) {
    if (isName(argument, obj.name)) {
      statObject(ch, obj);
      return;
    }
  }

  if (roomVnum !== -1) {
    const roomObjs = world.getObjsInRoom(roomVnum);
    for (const obj of roomObjs) {
      if (isName(argument, obj.name)) {
        statObject(ch, obj);
        return;
      }
    }
  }

  sendToChar(ch, "No object found with that vnum or name.\r\n");
}

// ============================================================================
//  Command registration
// ============================================================================

export interface AdminCommandEntry {
  name: string;
  fn: (ch: CharData, argument: string) => void;
  minPosition: Position;
  minLevel: number;
  log: number;
}

/**
 * All admin/immortal/builder commands, ready to be merged into
 * the main command table.
 */
export const adminCommands: AdminCommandEntry[] = [
  // Builder commands (level 106+)
  { name: 'rstat',    fn: doRstat,    minPosition: Position.DEAD,     minLevel: 106, log: 1 },
  { name: 'mstat',    fn: doMstat,    minPosition: Position.DEAD,     minLevel: 106, log: 1 },
  { name: 'ostat',    fn: doOstat,    minPosition: Position.DEAD,     minLevel: 106, log: 1 },

  // Immortal commands (level 108+)
  { name: 'goto',     fn: doGoto,     minPosition: Position.DEAD,     minLevel: 108, log: 1 },
  { name: 'transfer', fn: doTransfer, minPosition: Position.DEAD,     minLevel: 108, log: 1 },
  { name: 'peace',    fn: doPeace,    minPosition: Position.DEAD,     minLevel: 108, log: 1 },
  { name: 'slay',     fn: doSlay,     minPosition: Position.STANDING, minLevel: 108, log: 1 },
  { name: 'stat',     fn: doStat,     minPosition: Position.DEAD,     minLevel: 108, log: 1 },
  { name: 'force',    fn: doForce,    minPosition: Position.DEAD,     minLevel: 108, log: 1 },
  { name: 'wiznet',   fn: doWiznet,   minPosition: Position.DEAD,     minLevel: 108, log: 1 },

  // Admin commands (level 115)
  { name: 'setrole',  fn: doSetRole,  minPosition: Position.DEAD,     minLevel: 115, log: 1 },
  { name: 'advance',  fn: doAdvance,  minPosition: Position.DEAD,     minLevel: 115, log: 1 },
  { name: 'restore',  fn: doRestore,  minPosition: Position.DEAD,     minLevel: 115, log: 1 },
  { name: 'purge',    fn: doPurge,    minPosition: Position.DEAD,     minLevel: 115, log: 1 },
];
