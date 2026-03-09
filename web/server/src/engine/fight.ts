/**
 * fight.ts — Combat system for Stormgate MUD.
 *
 * Port of fight.c from the C server. Handles all combat mechanics including
 * hit/damage rolls, multi-attack rounds, death processing, corpse creation,
 * and experience gain.
 */

import type {
  CharData,
  ObjInstance,
} from './types.js';

import {
  Position,
  WearLocation,
  WeaponClass,
  DamType,
  ItemType,
  Direction,
  LEVEL_IMMORTAL,
} from './types.js';

import { world, charRoomMap, rollDice } from './world.js';

import {
  charToRoom,
  charFromRoom,
  getCharRoom,
  objToRoom,
  objFromRoom,
  objToChar,
  objFromChar,
  getEquipment,
  getCarriedItems,
  getEquippedItems,
  getInventory,
  isImmortal,
  isNpc,
  getDex,
  capitalize,
} from './handler.js';

import {
  sendToChar,
  sendToRoom,
  act,
  colors,
  TO_ROOM,
  TO_CHAR,
  TO_VICT,
  TO_NOTVICT,
} from './output.js';

import {
  sendVitals,
  sendCombatData,
} from './protocol.js';

// ============================================================================
//  Attack noun lookup — WeaponClass to attack noun
// ============================================================================

const WEAPON_ATTACK_NOUN: Record<number, string> = {
  [WeaponClass.EXOTIC]:  'hit',
  [WeaponClass.SWORD]:   'slash',
  [WeaponClass.DAGGER]:  'stab',
  [WeaponClass.SPEAR]:   'thrust',
  [WeaponClass.MACE]:    'pound',
  [WeaponClass.AXE]:     'chop',
  [WeaponClass.FLAIL]:   'strike',
  [WeaponClass.WHIP]:    'lash',
  [WeaponClass.POLEARM]: 'pierce',
};

const BARE_HAND_NOUN = 'punch';

// ============================================================================
//  Damage message table
// ============================================================================

interface DamMessage {
  minDam: number;
  maxDam: number;
  singular: string;
  plural: string;
}

const DAM_MESSAGES: DamMessage[] = [
  { minDam: 0,   maxDam: 0,   singular: 'miss',         plural: 'misses' },
  { minDam: 1,   maxDam: 4,   singular: 'scratch',      plural: 'scratches' },
  { minDam: 5,   maxDam: 8,   singular: 'graze',        plural: 'grazes' },
  { minDam: 9,   maxDam: 12,  singular: 'hit',          plural: 'hits' },
  { minDam: 13,  maxDam: 16,  singular: 'injure',       plural: 'injures' },
  { minDam: 17,  maxDam: 20,  singular: 'wound',        plural: 'wounds' },
  { minDam: 21,  maxDam: 24,  singular: 'maul',         plural: 'mauls' },
  { minDam: 25,  maxDam: 28,  singular: 'decimate',     plural: 'decimates' },
  { minDam: 29,  maxDam: 32,  singular: 'devastate',    plural: 'devastates' },
  { minDam: 33,  maxDam: 40,  singular: 'maim',         plural: 'maims' },
  { minDam: 41,  maxDam: 60,  singular: 'MUTILATE',     plural: 'MUTILATES' },
  { minDam: 61,  maxDam: 80,  singular: 'DISEMBOWEL',   plural: 'DISEMBOWELS' },
  { minDam: 81,  maxDam: 100, singular: 'DISMEMBER',    plural: 'DISMEMBERS' },
  { minDam: 101, maxDam: 150, singular: 'MASSACRE',     plural: 'MASSACRES' },
  { minDam: 151, maxDam: 200, singular: 'MANGLE',       plural: 'MANGLES' },
  { minDam: 201, maxDam: Infinity, singular: '*** OBLITERATE ***', plural: '*** OBLITERATES ***' },
];

/**
 * Look up the damage verb pair for a given damage amount.
 */
function getDamMessage(dam: number): DamMessage {
  for (const msg of DAM_MESSAGES) {
    if (dam >= msg.minDam && dam <= msg.maxDam) {
      return msg;
    }
  }
  // Fallback (should not happen)
  return DAM_MESSAGES[DAM_MESSAGES.length - 1];
}

// ============================================================================
//  Combat initiation and termination
// ============================================================================

/**
 * Set ch fighting victim. Links both characters into combat.
 * If victim is not already fighting, sets them fighting ch as well.
 */
export function setFighting(ch: CharData, victim: CharData): void {
  if (ch.fighting !== null) {
    // Already fighting someone
    return;
  }

  // Sanity: can't fight yourself
  if (ch.id === victim.id) {
    return;
  }

  ch.fighting = victim.id;
  ch.position = Position.FIGHTING;

  // If the victim isn't fighting yet, set them fighting back
  if (victim.fighting === null) {
    victim.fighting = ch.id;
    victim.position = Position.FIGHTING;
  }
}

/**
 * Stop fighting for a character. Clears the fighting reference and
 * sets position back to STANDING.
 */
export function stopFighting(ch: CharData): void {
  ch.fighting = null;
  if (ch.position === Position.FIGHTING) {
    ch.position = Position.STANDING;
  }
}

/**
 * Stop all combat references to a specific character (when they die or leave).
 * Called to clean up everyone who was fighting the departing character.
 */
export function stopFightingAll(victim: CharData): void {
  for (const ch of world.characters.values()) {
    if (ch.deleted) continue;
    if (ch.fighting === victim.id) {
      stopFighting(ch);
    }
  }
  // Also stop the victim's own fighting state
  stopFighting(victim);
}

// ============================================================================
//  Number of attacks per round
// ============================================================================

/**
 * Determine how many attacks a character gets per round based on level.
 */
function getNumAttacks(ch: CharData): number {
  const level = ch.level;
  if (level >= 80) return 5;
  if (level >= 60) return 4;
  if (level >= 40) return 3;
  if (level >= 20) return 2;
  return 1;
}

// ============================================================================
//  Multi-hit: one full combat round
// ============================================================================

/**
 * Process one full combat round for ch attacking victim.
 * Primary weapon attacks + extra attacks based on level + dual wield.
 */
export function multiHit(ch: CharData, victim: CharData): void {
  // Safety checks
  if (ch.deleted || victim.deleted) return;
  if (ch.position < Position.FIGHTING) return;

  // Check they are in the same room
  const chRoom = getCharRoom(ch);
  const victRoom = getCharRoom(victim);
  if (chRoom === -1 || victRoom === -1 || chRoom !== victRoom) {
    stopFighting(ch);
    return;
  }

  const numAttacks = getNumAttacks(ch);

  // Primary weapon attacks
  for (let i = 0; i < numAttacks; i++) {
    if (ch.deleted || victim.deleted) return;
    if (victim.hit <= 0) return;
    oneHit(ch, victim, -1); // -1 = use primary weapon
  }

  // Dual wield: if character has a second weapon, get +1 extra attack with it
  const offhand = getEquipment(ch, WearLocation.WIELD_2);
  if (offhand && !ch.deleted && !victim.deleted && victim.hit > 0) {
    oneHit(ch, victim, -2); // -2 = use offhand weapon
  }
}

// ============================================================================
//  One hit: a single attack
// ============================================================================

/**
 * Process a single attack from ch against victim.
 *
 * @param dt - Attack type discriminator:
 *             -1 = use primary weapon
 *             -2 = use offhand weapon
 *             >= 0 = skill/spell number
 */
export function oneHit(ch: CharData, victim: CharData, dt: number): void {
  if (ch.deleted || victim.deleted) return;
  if (victim.hit <= 0) return;

  // Determine the weapon being used
  let weapon: ObjInstance | undefined;
  if (dt === -2) {
    weapon = getEquipment(ch, WearLocation.WIELD_2);
  } else {
    weapon = getEquipment(ch, WearLocation.WIELD);
  }

  // Determine attack noun and damage type
  let attackNoun: string;
  let damType: DamType;

  if (weapon && weapon.itemType === ItemType.WEAPON) {
    const weaponClass = weapon.value[0] as WeaponClass;
    attackNoun = WEAPON_ATTACK_NOUN[weaponClass] ?? 'hit';
    damType = weapon.value[3] as DamType;
    if (damType < DamType.NONE || damType > DamType.SOUND) {
      damType = DamType.BASH;
    }
  } else {
    attackNoun = BARE_HAND_NOUN;
    damType = DamType.BASH;
  }

  // ---- Hit roll ----
  // Attacker: hitroll + level + (dex - 15)
  // Defender: random(0,20) + armor/100
  const dex = getDex(ch);
  const attackRoll = ch.hitroll + ch.level + (dex - 15) + Math.floor(Math.random() * 20);
  const defenseRoll = Math.floor(Math.random() * 20) + Math.floor(victim.armor / 100);

  if (attackRoll <= defenseRoll) {
    // Miss
    damage(ch, victim, 0, attackNoun, damType);
    return;
  }

  // ---- Damage roll ----
  let dam: number;

  if (weapon && weapon.itemType === ItemType.WEAPON) {
    // Weapon damage: value[1] d value[2]
    const diceCount = weapon.value[1];
    const diceSides = weapon.value[2];
    dam = rollDice(diceCount, diceSides);
  } else if (isNpc(ch)) {
    // NPC bare-hand: use mob template dice
    const template = world.getMobTemplate(ch.act & 0x7FFFFFFF); // try vnum from act
    // NPCs store their damage dice on the CharData via the template
    // We use a level-based formula as fallback
    const diceCount = Math.max(1, Math.floor(ch.level / 4));
    const diceSides = 4;
    dam = rollDice(diceCount, diceSides);
  } else {
    // PC bare-hand: level/2 d 4
    const diceCount = Math.max(1, Math.floor(ch.level / 2));
    dam = rollDice(diceCount, 4);
  }

  // Add damroll bonus
  dam += ch.damroll;

  // Ensure minimum 1 damage on a hit
  dam = Math.max(1, dam);

  // Apply damage type modifiers from victim
  if (victim.damageMods && victim.damageMods[damType] !== undefined) {
    const mod = victim.damageMods[damType];
    // Positive mod = resistance (reduce damage), negative = vulnerability (increase)
    if (mod !== 0) {
      const factor = 1.0 - (mod / 100);
      dam = Math.max(0, Math.floor(dam * factor));
    }
  }

  // Apply the damage
  damage(ch, victim, dam, attackNoun, damType);
}

// ============================================================================
//  Damage application
// ============================================================================

/**
 * Apply damage from ch to victim, display messages, check for death.
 *
 * @param ch        The attacker.
 * @param victim    The defender.
 * @param dam       Amount of damage.
 * @param attackNoun The attack noun (e.g., "slash", "punch").
 * @param damType   The damage type enum value.
 */
export function damage(
  ch: CharData,
  victim: CharData,
  dam: number,
  attackNoun: string,
  damType: DamType
): void {
  if (victim.deleted) return;

  // Immortals cannot be killed
  if (isImmortal(victim) && dam > 0) {
    dam = 0;
  }

  // Ensure fighting state
  if (victim.fighting === null && victim.position !== Position.DEAD) {
    setFighting(victim, ch);
  }
  if (ch.fighting === null && ch.position !== Position.DEAD) {
    setFighting(ch, victim);
  }

  // Display damage messages
  showDamageMessage(ch, victim, dam, attackNoun);

  // Apply the damage
  victim.hit -= dam;

  // Update combat timer
  ch.combatTimer = 4;
  victim.combatTimer = 4;

  // Send structured combat and vitals data to both parties
  const damMsg = getDamMessage(dam);
  sendCombatData(ch, victim, dam, `${attackNoun} ${damMsg.plural}`);
  sendVitals(ch);
  if (!victim.isNpc) {
    sendCombatData(victim, ch, dam, `${attackNoun} ${damMsg.plural}`);
    sendVitals(victim);
  }

  // Check for death
  if (victim.hit <= 0) {
    victim.hit = 0;
    victim.position = Position.DEAD;

    // Death message
    act(
      `${colors.brightRed}$N is DEAD!!${colors.reset}`,
      ch, null, victim, TO_CHAR
    );
    act(
      `${colors.brightRed}$N is DEAD!!${colors.reset}`,
      ch, null, victim, TO_NOTVICT
    );
    sendToChar(victim, `${colors.brightRed}You have been KILLED!!${colors.reset}\r\n`);

    rawKill(ch, victim);
    return;
  }

  // Wimpy check for victim (auto-flee)
  if (!victim.isNpc && victim.wimpy > 0 && victim.hit < victim.wimpy) {
    sendToChar(victim, `${colors.brightYellow}You wimp out and attempt to flee!${colors.reset}\r\n`);
    doAutoFlee(victim);
  }

  // Wimpy check for attacker (auto-flee)
  if (!ch.isNpc && ch.wimpy > 0 && ch.hit < ch.wimpy) {
    sendToChar(ch, `${colors.brightYellow}You wimp out and attempt to flee!${colors.reset}\r\n`);
    doAutoFlee(ch);
  }
}

// ============================================================================
//  Damage display messages
// ============================================================================

/**
 * Display the combat damage message to all relevant parties.
 * Format: "$n's <attack> <verb> $N" with color.
 */
function showDamageMessage(
  ch: CharData,
  victim: CharData,
  dam: number,
  attackNoun: string
): void {
  const msg = getDamMessage(dam);

  // Choose color based on damage severity
  let damColor: string;
  if (dam === 0) {
    damColor = colors.white;
  } else if (dam <= 12) {
    damColor = colors.red;
  } else if (dam <= 40) {
    damColor = colors.brightRed;
  } else {
    damColor = colors.brightRed + colors.bold;
  }

  // Message to the attacker (ch): "Your <attack> <verb> <victim>!"
  if (dam === 0) {
    sendToChar(
      ch,
      `${damColor}Your ${attackNoun} ${msg.plural} ${getCharDisplayName(victim)}.${colors.reset}\r\n`
    );
  } else {
    sendToChar(
      ch,
      `${damColor}Your ${attackNoun} ${msg.plural} ${getCharDisplayName(victim)}! [${dam}]${colors.reset}\r\n`
    );
  }

  // Message to the victim: "<attacker>'s <attack> <verb> you!"
  if (dam === 0) {
    sendToChar(
      victim,
      `${damColor}${capitalize(getCharDisplayName(ch))}'s ${attackNoun} ${msg.plural} you.${colors.reset}\r\n`
    );
  } else {
    sendToChar(
      victim,
      `${damColor}${capitalize(getCharDisplayName(ch))}'s ${attackNoun} ${msg.plural} you! [${dam}]${colors.reset}\r\n`
    );
  }

  // Message to the room (others): "<attacker>'s <attack> <verb> <victim>."
  const roomMsg = dam === 0
    ? `${damColor}$n's ${attackNoun} ${msg.plural} $N.${colors.reset}`
    : `${damColor}$n's ${attackNoun} ${msg.plural} $N! [${dam}]${colors.reset}`;
  act(roomMsg, ch, null, victim, TO_NOTVICT);
}

/**
 * Get a display name for a character (short description for NPCs, name for PCs).
 */
function getCharDisplayName(ch: CharData): string {
  return ch.isNpc ? ch.shortDescr : ch.name;
}

// ============================================================================
//  Death handling
// ============================================================================

/**
 * Handle character death. Creates corpse, transfers inventory,
 * grants XP (NPC death) or teleports to recall (PC death).
 */
export function rawKill(killer: CharData, victim: CharData): void {
  // Stop all fighting references to the victim
  stopFightingAll(victim);

  // Create a corpse
  const corpse = createCorpse(victim);

  if (victim.isNpc) {
    // ---- NPC death ----

    // Grant XP to killer
    if (!killer.isNpc) {
      const xpGain = victim.level * 50 + Math.floor(victim.gold / 4);
      killer.exp += xpGain;
      sendToChar(
        killer,
        `${colors.brightYellow}You receive ${xpGain} experience points.${colors.reset}\r\n`
      );

      // Transfer gold to killer
      if (victim.gold > 0) {
        killer.gold += victim.gold;
        sendToChar(
          killer,
          `${colors.brightYellow}You get ${victim.gold} gold coins from the corpse.${colors.reset}\r\n`
        );
      }

      // Update vitals after XP and gold gain
      sendVitals(killer);
    }

    // Remove NPC from the world
    charFromRoom(victim);
    victim.deleted = true;
    world.characters.delete(victim.id);
  } else {
    // ---- PC death ----

    sendToChar(
      victim,
      `${colors.brightRed}You have been KILLED!${colors.reset}\r\n`
    );

    // Lose 10% XP
    const xpLoss = Math.floor(victim.exp * 0.10);
    victim.exp = Math.max(0, victim.exp - xpLoss);
    if (xpLoss > 0) {
      sendToChar(
        victim,
        `${colors.red}You lose ${xpLoss} experience points.${colors.reset}\r\n`
      );
    }

    // Teleport to recall
    const RECALL_VNUM = 25000;
    const pcData = world.pcData.get(victim.id);
    const recallVnum = pcData?.recall || RECALL_VNUM;

    charFromRoom(victim);

    // Restore to 1 hp, some mana/move
    victim.hit = 1;
    victim.mana = Math.max(1, Math.floor(victim.maxMana / 4));
    victim.move = Math.max(1, Math.floor(victim.maxMove / 4));
    victim.position = Position.RESTING;

    charToRoom(victim, recallVnum);
    sendToChar(
      victim,
      `${colors.brightCyan}You awake in the temple, battered but alive.${colors.reset}\r\n`
    );

    // Send vitals update after revival
    sendVitals(victim);

    // Show the room to the revived player
    act('$n appears in a flash of light, looking dazed.', victim, null, null, TO_ROOM);
  }
}

/**
 * Create a corpse object and transfer the victim's inventory into it.
 * The corpse is placed in the room where the victim died.
 */
function createCorpse(victim: CharData): ObjInstance {
  const roomVnum = getCharRoom(victim);
  const isNpcVictim = victim.isNpc;

  const corpseId = world.nextId();
  const corpseName = `corpse ${victim.name}`;
  const corpseShort = `the corpse of ${getCharDisplayName(victim)}`;
  const corpseLong = `The corpse of ${getCharDisplayName(victim)} is lying here.`;

  const corpse: ObjInstance = {
    id: corpseId,
    indexVnum: 0,
    name: corpseName,
    shortDescr: corpseShort,
    description: corpseLong,
    itemType: isNpcVictim ? ItemType.CORPSE_NPC : ItemType.CORPSE_PC,
    extraFlags: 0,
    extraFlags2: 0,
    extraFlags3: 0,
    extraFlags4: 0,
    wearFlags: 0,
    wearLoc: WearLocation.NONE,
    durabilityMax: 100,
    durabilityCur: 100,
    weight: 0,
    cost: 0,
    level: victim.level,
    timer: isNpcVictim ? 8 : 40, // NPC corpses decay faster
    value: [0, 0, 0, 0],
    affects: [],
    acType: 0,
    acVnum: 0,
    acSpell: '',
    acCharge: [0, 0],
    deleted: false,
  };

  // Register the corpse in the world
  world.objects.set(corpseId, corpse);

  // Transfer victim's inventory to the corpse (stored as containedIn)
  const inventory = getInventory(victim);
  for (const obj of inventory) {
    objFromChar(obj);
    obj.containedIn = corpseId;
    obj.inRoom = undefined;
    obj.carriedBy = undefined;
  }

  // Put gold into the corpse description
  if (victim.gold > 0 && victim.isNpc) {
    // For NPC deaths where killer is PC, gold is already given above.
    // But we still note it was there. Gold transfer is handled in rawKill.
    // Clear the mob's gold.
    victim.gold = 0;
  }

  // Place corpse in room
  if (roomVnum !== -1) {
    objToRoom(corpse, roomVnum);
  }

  return corpse;
}

// ============================================================================
//  Auto-flee (wimpy system)
// ============================================================================

/**
 * Attempt to auto-flee a character from combat.
 */
function doAutoFlee(ch: CharData): void {
  if (ch.position !== Position.FIGHTING) return;

  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) return;

  const room = world.getRoom(roomVnum);
  if (!room) return;

  // Collect available exits
  const availableDirs: Direction[] = [];
  for (let dir = 0; dir < 6; dir++) {
    const exit = room.exits[dir as keyof typeof room.exits];
    if (exit && !(exit.exitInfo & 1)) {
      availableDirs.push(dir as Direction);
    }
  }

  if (availableDirs.length === 0) {
    sendToChar(ch, `${colors.brightRed}PANIC! You couldn't escape!${colors.reset}\r\n`);
    return;
  }

  // Pick a random direction
  const dir = availableDirs[Math.floor(Math.random() * availableDirs.length)];
  const exit = room.exits[dir as keyof typeof room.exits];
  if (!exit) return;

  const toRoom = world.getRoom(exit.toVnum);
  if (!toRoom) {
    sendToChar(ch, `${colors.brightRed}PANIC! You couldn't escape!${colors.reset}\r\n`);
    return;
  }

  // XP penalty for fleeing
  const xpLoss = Math.floor(ch.level * 5);
  if (!ch.isNpc) {
    ch.exp = Math.max(0, ch.exp - xpLoss);
    sendToChar(ch, `${colors.yellow}You lose ${xpLoss} experience points for fleeing.${colors.reset}\r\n`);
  }

  // Stop fighting
  stopFighting(ch);

  // Move
  const DIRECTION_NAMES = ['north', 'east', 'south', 'west', 'up', 'down'];
  act(`$n flees ${DIRECTION_NAMES[dir]}!`, ch, null, null, TO_ROOM);
  charFromRoom(ch);
  charToRoom(ch, exit.toVnum);
  act('$n arrives in a panic!', ch, null, null, TO_ROOM);
  sendToChar(ch, `${colors.brightYellow}You flee ${DIRECTION_NAMES[dir]}!${colors.reset}\r\n`);
}

// ============================================================================
//  Consider (compare levels)
// ============================================================================

/**
 * Compare your level to a mob's level and show a difficulty rating.
 */
export function doConsider(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Consider killing whom?\r\n");
    return;
  }

  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You are in the void.\r\n");
    return;
  }

  const chars = world.getCharsInRoom(roomVnum);
  let victim: CharData | undefined;

  for (const other of chars) {
    if (other.id === ch.id) continue;
    if (other.name.toLowerCase().startsWith(argument.toLowerCase()) ||
        (other.isNpc && other.shortDescr.toLowerCase().includes(argument.toLowerCase()))) {
      victim = other;
      break;
    }
  }

  if (!victim) {
    sendToChar(ch, "They aren't here.\r\n");
    return;
  }

  if (victim.id === ch.id) {
    sendToChar(ch, "You consider yourself. You look pretty tough.\r\n");
    return;
  }

  const diff = victim.level - ch.level;
  let message: string;

  if (diff <= -10) {
    message = `${colors.brightGreen}You could kill $N with a sneeze.${colors.reset}`;
  } else if (diff <= -5) {
    message = `${colors.green}$N is no match for you.${colors.reset}`;
  } else if (diff <= -2) {
    message = `${colors.green}$N looks like an easy kill.${colors.reset}`;
  } else if (diff <= 1) {
    message = `${colors.yellow}The perfect match!${colors.reset}`;
  } else if (diff <= 4) {
    message = `${colors.yellow}$N says 'Do you feel lucky, punk?'${colors.reset}`;
  } else if (diff <= 9) {
    message = `${colors.red}$N laughs at you mercilessly.${colors.reset}`;
  } else {
    message = `${colors.brightRed}Death will thank you for your gift.${colors.reset}`;
  }

  act(message, ch, null, victim, TO_CHAR);
}

// ============================================================================
//  Wimpy command
// ============================================================================

/**
 * Set wimpy threshold -- auto-flee when HP drops below this value.
 */
export function doWimpy(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, `Your current wimpy threshold is ${ch.wimpy} hit points.\r\n`);
    return;
  }

  const wimpy = parseInt(argument, 10);

  if (isNaN(wimpy)) {
    sendToChar(ch, "Specify a number of hit points.\r\n");
    return;
  }

  if (wimpy < 0) {
    sendToChar(ch, "Your wimpy threshold must be 0 or higher.\r\n");
    return;
  }

  if (wimpy > ch.maxHit) {
    sendToChar(ch, "Your wimpy threshold can't exceed your max hit points.\r\n");
    return;
  }

  ch.wimpy = wimpy;

  if (wimpy === 0) {
    sendToChar(ch, `${colors.brightCyan}Wimpy disabled. You will fight to the death!${colors.reset}\r\n`);
  } else {
    sendToChar(ch, `${colors.brightCyan}Wimpy set to ${wimpy} hit points.${colors.reset}\r\n`);
  }
}

// ============================================================================
//  Position change commands
// ============================================================================

/**
 * Rest -- sit down and rest for faster regeneration.
 */
export function doRest(ch: CharData, _argument: string): void {
  switch (ch.position) {
    case Position.SLEEPING:
      sendToChar(ch, "You are already sleeping. Wake up first.\r\n");
      return;
    case Position.RESTING:
      sendToChar(ch, "You are already resting.\r\n");
      return;
    case Position.FIGHTING:
      sendToChar(ch, "Not while you're fighting!\r\n");
      return;
    case Position.STANDING:
    case Position.MEDITATING:
      ch.position = Position.RESTING;
      sendToChar(ch, "You sit down and rest.\r\n");
      act('$n sits down and rests.', ch, null, null, TO_ROOM);
      return;
    default:
      sendToChar(ch, "You can't rest right now.\r\n");
      return;
  }
}

/**
 * Sleep -- go to sleep for maximum regeneration.
 */
export function doSleep(ch: CharData, _argument: string): void {
  switch (ch.position) {
    case Position.SLEEPING:
      sendToChar(ch, "You are already sleeping.\r\n");
      return;
    case Position.FIGHTING:
      sendToChar(ch, "Not while you're fighting!\r\n");
      return;
    case Position.RESTING:
    case Position.STANDING:
    case Position.MEDITATING:
      ch.position = Position.SLEEPING;
      sendToChar(ch, "You lie down and go to sleep.\r\n");
      act('$n lies down and goes to sleep.', ch, null, null, TO_ROOM);
      return;
    default:
      sendToChar(ch, "You can't sleep right now.\r\n");
      return;
  }
}

/**
 * Wake -- wake up from sleep.
 */
export function doWake(ch: CharData, _argument: string): void {
  if (ch.position === Position.SLEEPING) {
    ch.position = Position.RESTING;
    sendToChar(ch, "You wake up and sit up.\r\n");
    act('$n wakes up.', ch, null, null, TO_ROOM);
    return;
  }

  if (ch.position === Position.RESTING) {
    sendToChar(ch, "You are already awake. Try 'stand' to get up.\r\n");
    return;
  }

  sendToChar(ch, "You aren't sleeping.\r\n");
}

/**
 * Stand -- stand up from resting or sleeping.
 */
export function doStand(ch: CharData, _argument: string): void {
  switch (ch.position) {
    case Position.SLEEPING:
      ch.position = Position.STANDING;
      sendToChar(ch, "You wake up and stand up.\r\n");
      act('$n wakes up and stands up.', ch, null, null, TO_ROOM);
      return;
    case Position.RESTING:
    case Position.MEDITATING:
      ch.position = Position.STANDING;
      sendToChar(ch, "You stand up.\r\n");
      act('$n stands up.', ch, null, null, TO_ROOM);
      return;
    case Position.STANDING:
      sendToChar(ch, "You are already standing.\r\n");
      return;
    case Position.FIGHTING:
      sendToChar(ch, "You are fighting!\r\n");
      return;
    default:
      sendToChar(ch, "You can't stand up right now.\r\n");
      return;
  }
}
