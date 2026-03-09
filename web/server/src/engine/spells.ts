/**
 * spells.ts -- Individual spell/skill effect functions for Stormgate MUD.
 *
 * Port of spell_* and skill_* functions from magic.c, magic2.c, magic3.c,
 * magic4.c, gr_magic.c, and skills.c.
 *
 * Each spell function has signature:
 *   (sn: number, level: number, ch: CharData, vo: unknown) => number
 *
 * Return values:
 *   > 0          = damage amount (caller runs damage())
 *   SKPELL_NO_DAMAGE (-1)  = spell succeeded, no direct damage
 *   SKPELL_MISSED (-2)     = spell missed / was resisted
 *   SKPELL_BOTCHED (-3)    = spell failed / invalid target
 */

import type { CharData, ObjInstance, AffectData } from './types.js';
import { Position, ApplyType, DamType, ItemType, WearLocation } from './types.js';
import { world, charRoomMap, rollDice } from './world.js';
import {
  charToRoom,
  charFromRoom,
  getCharRoom,
  objToChar,
  affectToChar,
  affectRemoveFromChar,
  isAffected,
  isNpc,
  isImmortal,
  isName,
  getStr,
  getInt,
  getWis,
  getDex,
  getCon,
  getEquipment,
  getCarriedItems,
  getEquippedItems,
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
import { damage, setFighting, multiHit } from './fight.js';
import {
  skillTable,
  skillLookup,
  registerSpellByName,
  SKPELL_NO_DAMAGE,
  SKPELL_MISSED,
  SKPELL_BOTCHED,
  AFF_BLIND,
  AFF_INVISIBLE,
  AFF_DETECT_EVIL,
  AFF_DETECT_INVIS,
  AFF_DETECT_MAGIC,
  AFF_DETECT_HIDDEN,
  AFF_HASTE,
  AFF_SANCTUARY,
  AFF_FAERIE_FIRE,
  AFF_INFRARED,
  AFF_CURSE,
  AFF_FLAMING,
  AFF_POISON,
  AFF_PROTECT,
  AFF_SNEAK,
  AFF_HIDE,
  AFF_SLEEP,
  AFF_CHARM,
  AFF_FLYING,
  AFF_PASS_DOOR,
  AFF_FIRESHIELD,
  AFF_SHOCKSHIELD,
  AFF_ICESHIELD,
  AFF_PEACE,
  type SpellFn,
} from './skills.js';

// ============================================================================
//  Utility functions (ported from C helpers)
// ============================================================================

/** Random integer in [low, high] inclusive. */
function numberRange(low: number, high: number): number {
  if (low >= high) return low;
  return low + Math.floor(Math.random() * (high - low + 1));
}

/** Random percentage 1-100. */
function numberPercent(): number {
  return Math.floor(Math.random() * 100) + 1;
}

/** Fuzzy a number: returns num-1, num, or num+1 randomly. */
function numberFuzzy(num: number): number {
  const roll = numberRange(0, 2);
  if (roll === 0) return num - 1;
  if (roll === 2) return num + 1;
  return num;
}

/** Clamp value to [low, high]. */
function uRange(low: number, val: number, high: number): number {
  return Math.max(low, Math.min(val, high));
}

function uMin(a: number, b: number): number { return Math.min(a, b); }
function uMax(a: number, b: number): number { return Math.max(a, b); }

/** Roll dice: count d sides. */
function dice(count: number, sides: number): number {
  return rollDice(count, sides);
}

/** Simple saving throw vs. spell. */
export function savesSpell(level: number, victim: CharData): boolean {
  let base = 20;
  if (victim.isNpc) base += 30;
  let saveBase = 0 - (victim.savingThrow < -440 ? -440 : victim.savingThrow);
  if (!victim.isNpc) {
    saveBase = Math.floor(saveBase / 8);
  } else {
    saveBase = Math.floor(saveBase / 2);
  }
  let save = base + (victim.level - level) + saveBase;
  save = uRange(5, save, 90);
  return numberPercent() < save;
}

/** Create a new affect object. */
function newAffect(sn: number, level: number, duration: number, location: ApplyType, modifier: number, bitvector: number): AffectData {
  return {
    type: sn,
    level,
    duration,
    location,
    modifier,
    bitvector,
    deleted: false,
  };
}

/** Strip all affects of a given sn from a character. */
function affectStrip(ch: CharData, sn: number): void {
  const toRemove = ch.affects.filter(af => af.type === sn && !af.deleted);
  for (const af of toRemove) {
    affectRemoveFromChar(ch, af);
  }
}

/** Find a character in the same room by name keyword. */
function findCharInRoom(ch: CharData, name: string): CharData | null {
  const roomVnum = getCharRoom(ch);
  if (roomVnum < 0) return null;
  const chars = world.getCharsInRoom(roomVnum);
  for (const victim of chars) {
    if (victim.deleted) continue;
    if (isName(name, victim.name)) return victim;
  }
  return null;
}

/** Find a character in the world by name keyword. */
function findCharWorld(ch: CharData, name: string): CharData | null {
  // Check room first
  const local = findCharInRoom(ch, name);
  if (local) return local;
  for (const victim of world.characters.values()) {
    if (victim.deleted) continue;
    if (isName(name, victim.name)) return victim;
  }
  return null;
}

/** Find an object in a character's inventory by name keyword. */
function findObjCarry(ch: CharData, name: string): ObjInstance | null {
  const items = getCarriedItems(ch);
  for (const obj of items) {
    if (obj.deleted) continue;
    if (isName(name, obj.name)) return obj;
  }
  return null;
}

/** Update position based on HP. */
function updatePos(ch: CharData): void {
  if (ch.hit > 0) {
    if (ch.position <= Position.STUNNED) {
      ch.position = Position.STANDING;
    }
  } else if (ch.hit <= -11) {
    ch.position = Position.DEAD;
  } else if (ch.hit <= -6) {
    ch.position = Position.MORTAL;
  } else if (ch.hit <= -3) {
    ch.position = Position.INCAP;
  } else {
    ch.position = Position.STUNNED;
  }
}

// ============================================================================
//  DAMAGE SPELLS
// ============================================================================

function spell_acid_blast(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 8);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_burning_hands(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const damEach = [
    0, 0, 0, 0, 0, 14, 17, 20, 23, 26, 29,
    29, 29, 30, 30, 31, 31, 32, 32, 33, 33,
    34, 34, 35, 35, 36, 36, 37, 37, 38, 38,
    39, 39, 40, 40, 41, 41, 42, 42, 43, 43,
    44, 44, 45, 45, 46, 46, 47, 47, 48, 48,
    48, 48, 49, 49, 49, 49, 50, 50, 50, 51,
    51, 51, 52, 52, 52, 53, 53, 53, 54, 54,
    54, 54, 54, 54, 55, 55, 55, 55, 55, 55,
    56, 56, 56, 56, 56, 57, 57, 58, 59, 60,
  ];
  const lvl = uRange(0, level, damEach.length - 1);
  let dam = numberRange(Math.floor(damEach[lvl] / 2), damEach[lvl] * 2);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_call_lightning(sn: number, level: number, ch: CharData, vo: unknown): number {
  const damEach = [
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 36, 42, 48, 54, 60, 66,
    72, 78, 84, 90, 96, 102, 104, 106, 107, 108,
    110, 113, 115, 117, 120, 122, 124, 127, 129, 132,
    134, 137, 139, 142, 144, 147, 149, 152, 154, 157,
    159, 162, 164, 167, 169, 172, 174, 177, 179, 182,
    184, 187, 189, 192, 194, 196, 198, 200, 202, 204,
    206, 208, 210, 212, 214, 216, 218, 220, 222, 224,
    226, 228, 230, 231, 232, 233, 234, 235, 236, 237,
    238, 239, 240, 242, 244, 246, 248, 250, 255, 260,
  ];
  const lvl = uRange(0, level, damEach.length - 1);
  let dam = numberRange(Math.floor(damEach[lvl] / 2), damEach[lvl] * 7);
  sendToChar(ch, `${colors.white}Lightning slashes out of the sky to strike your foe!\r\n`);
  act('$n calls lightning from the sky to strike $s foe!', ch, null, null, TO_ROOM);
  return dam;
}

function spell_chill_touch(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const damEach = [
    0, 0, 0, 6, 7, 8, 9, 12, 13, 13, 13,
    14, 14, 14, 15, 15, 15, 16, 16, 16, 17,
    17, 17, 18, 18, 18, 19, 19, 19, 20, 20,
    20, 21, 21, 21, 22, 22, 22, 23, 23, 23,
    24, 24, 24, 25, 25, 25, 26, 26, 26, 27,
    27, 27, 27, 28, 28, 28, 29, 29, 29, 30,
    30, 30, 31, 31, 31, 32, 32, 33, 33, 33,
    34, 34, 34, 35, 35, 35, 36, 36, 36, 37,
    37, 37, 37, 37, 38, 38, 38, 38, 39, 39,
    39, 39, 39, 40, 40, 40, 41, 41, 42, 43,
  ];
  const lvl = uRange(0, level, damEach.length - 1);
  let dam = numberRange(Math.floor(damEach[lvl] / 2), damEach[lvl] * 2);
  if (!savesSpell(level, victim)) {
    const af = newAffect(sn, level, 6, ApplyType.STR, -1, 0);
    affectToChar(victim, af);
  } else {
    dam = Math.floor(dam / 2);
  }
  return dam;
}

function spell_colour_spray(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const damEach = [
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    30, 35, 40, 45, 50, 55, 55, 55, 56, 57,
    58, 58, 59, 60, 61, 61, 62, 63, 64, 64,
    65, 66, 67, 67, 68, 69, 70, 70, 71, 72,
    73, 73, 74, 75, 76, 76, 77, 78, 79, 79,
    79, 80, 80, 81, 81, 82, 82, 83, 83, 84,
    84, 85, 85, 86, 86, 87, 87, 88, 88, 90,
    90, 91, 91, 92, 92, 93, 93, 94, 94, 95,
    95, 96, 96, 97, 97, 98, 98, 99, 99, 100,
    100, 101, 102, 102, 103, 104, 105, 106, 107, 120,
  ];
  const lvl = uRange(0, level, damEach.length - 1);
  let dam = numberRange(Math.floor(damEach[lvl] / 2), damEach[lvl] * 4);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_fireball(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const damEach = [
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 30, 35, 40, 45, 50, 55,
    60, 65, 70, 75, 80, 82, 84, 86, 88, 90,
    92, 94, 96, 98, 100, 102, 104, 106, 108, 110,
    112, 114, 116, 118, 120, 122, 124, 126, 128, 130,
    132, 134, 136, 138, 140, 142, 144, 146, 148, 150,
    152, 154, 156, 158, 160, 162, 164, 166, 168, 170,
    172, 174, 176, 178, 180, 182, 184, 186, 188, 190,
    192, 194, 196, 198, 200, 202, 204, 206, 208, 210,
    215, 220, 225, 230, 235, 240, 245, 250, 255, 260,
  ];
  const lvl = uRange(0, level, damEach.length - 1);
  let dam = numberRange(Math.floor(damEach[lvl] / 2), damEach[lvl] * 6);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_flamestrike(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(6, Math.floor(level / 2));
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_harm(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = uMax(20, victim.hit - dice(1, 4));
  if (savesSpell(level, victim)) dam = uMin(50, Math.floor(dam / 4));
  dam = uMin(175, dam);
  return dam;
}

function spell_lightning_bolt(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 6);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_magic_missile(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 4);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_shocking_grasp(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 6);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_energy_drain(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (savesSpell(level, victim)) return 0;
  let dam = dice(4, level);
  // Drain gives HP to caster
  ch.hit = uMin(ch.hit + dam, ch.maxHit + 200);
  // Drain move from victim
  victim.move = uMax(0, victim.move - dam);
  ch.move = uMin(ch.move + dam, ch.maxMove + 200);
  return dam;
}

function spell_dispel_evil(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (!victim.isNpc && ch.alignment < 0) {
    sendToChar(ch, `${colors.red}You are too EVIL to cast this.\r\n`);
    return SKPELL_MISSED;
  }
  if (victim.alignment > 350) {
    act('God protects $N.', ch, null, victim, TO_ROOM);
    return SKPELL_MISSED;
  }
  if (victim.alignment > -350 && victim.alignment < 350) {
    act('$N does not seem to be affected.', ch, null, victim, TO_CHAR);
    return SKPELL_MISSED;
  }
  let dam = dice(level, 4);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_earthquake(sn: number, level: number, ch: CharData, vo: unknown): number {
  sendToChar(ch, `${colors.yellow}The earth trembles beneath your feet!\r\n`);
  act('$n makes the earth tremble and shiver.', ch, null, null, TO_ROOM);
  const roomVnum = getCharRoom(ch);
  const chars = world.getCharsInRoom(roomVnum);
  for (const vch of chars) {
    if (vch.deleted) continue;
    if (vch.id === ch.id) continue;
    if (ch.isNpc ? !vch.isNpc : vch.isNpc) {
      damage(ch, vch, level + dice(2, 8), 'earthquake', DamType.BASH);
    }
  }
  return SKPELL_NO_DAMAGE;
}

function spell_cause_light(sn: number, level: number, ch: CharData, vo: unknown): number {
  return dice(5, 10) + Math.floor(level / 3);
}

function spell_cause_serious(sn: number, level: number, ch: CharData, vo: unknown): number {
  return dice(10, 10) + level;
}

function spell_cause_critical(sn: number, level: number, ch: CharData, vo: unknown): number {
  return dice(20, 10) + level;
}

function spell_icestorm(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 10);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_holy_fires(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 20);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_general_purpose(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = numberRange(25, 100);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_high_explosive(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = numberRange(30, 120);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_chain_lightning(sn: number, level: number, ch: CharData, vo: unknown): number {
  sendToChar(ch, `${colors.blue}Bolts of electricity arc from your hands!\r\n`);
  act('Electrical energy bursts from $n\'s hands.', ch, null, null, TO_ROOM);
  const roomVnum = getCharRoom(ch);
  const chars = world.getCharsInRoom(roomVnum);
  for (const vch of chars) {
    if (vch.deleted) continue;
    if (vch.id === ch.id) continue;
    damage(ch, vch, level + dice(level, Math.floor(level / 4)), 'chain lightning', DamType.LIGHTNING);
  }
  return SKPELL_NO_DAMAGE;
}

function spell_meteor_swarm(sn: number, level: number, ch: CharData, vo: unknown): number {
  sendToChar(ch, `${colors.red}Flaming meteors fly forth from your outstretched hands!\r\n`);
  act('Hundreds of flaming meteors fly forth from $n\'s hands.', ch, null, null, TO_ROOM);
  const roomVnum = getCharRoom(ch);
  const chars = world.getCharsInRoom(roomVnum);
  for (const vch of chars) {
    if (vch.deleted) continue;
    if (vch.id === ch.id) continue;
    damage(ch, vch, level + dice(Math.floor(level / 2), Math.floor(level / 2)), 'meteor swarm', DamType.FIRE);
  }
  return SKPELL_NO_DAMAGE;
}

// ============================================================================
//  HEALING SPELLS
// ============================================================================

function spell_cure_light(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let heal = Math.floor(victim.maxHit * 0.02);
  heal = uMin(heal, 400);
  victim.hit = uMin(victim.hit + heal, victim.maxHit);
  updatePos(victim);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}You feel better!\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_cure_serious(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const heal = Math.floor(victim.maxHit * 0.05);
  victim.hit = uMin(victim.hit + heal, victim.maxHit);
  updatePos(victim);
  sendToChar(victim, `${colors.blue}You feel better!\r\n`);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_cure_critical(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const heal = Math.floor(victim.maxHit * 0.08);
  victim.hit = uMin(victim.hit + heal, victim.maxHit);
  updatePos(victim);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}You feel better!\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_heal(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const heal = Math.floor(victim.maxHit * 0.12);
  victim.hit = uMin(victim.hit + heal, victim.maxHit);
  updatePos(victim);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}A warm feeling fills your body.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_group_heal(sn: number, level: number, ch: CharData, vo: unknown): number {
  const roomVnum = getCharRoom(ch);
  const chars = world.getCharsInRoom(roomVnum);
  for (const vch of chars) {
    if (vch.deleted) continue;
    if (!vch.isNpc) {
      const heal = Math.floor(vch.maxHit * 0.10);
      vch.hit = uMin(vch.hit + heal, vch.maxHit);
      updatePos(vch);
      sendToChar(vch, `${colors.blue}A warm feeling fills your body.\r\n`);
    }
  }
  return SKPELL_NO_DAMAGE;
}

function spell_refresh(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  victim.move = uMin(victim.move + level, victim.maxMove);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}You feel less tired.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_cure_blindness(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const blindSn = skillLookup('blindness');
  if (!isAffected(victim, blindSn)) return SKPELL_MISSED;
  affectStrip(victim, blindSn);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.white}Your vision returns!\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_cure_poison(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const poisonSn = skillLookup('poison');
  if (!isAffected(victim, poisonSn)) return SKPELL_MISSED;
  victim.poisonLevel -= numberFuzzy(Math.floor(ch.level / 10));
  if (victim.poisonLevel <= 0) {
    victim.poisonLevel = 0;
    affectStrip(victim, poisonSn);
  }
  sendToChar(ch, `${colors.green}Ok.\r\n`);
  sendToChar(victim, `${colors.green}A warm feeling runs through your body.\r\n`);
  act('$N looks better.', ch, null, victim, TO_NOTVICT);
  return SKPELL_NO_DAMAGE;
}

function spell_remove_curse(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const curseSn = skillLookup('curse');
  if (isAffected(victim, curseSn)) {
    affectStrip(victim, curseSn);
    sendToChar(victim, `${colors.blue}You feel better.\r\n`);
    if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  }
  return SKPELL_NO_DAMAGE;
}

// ============================================================================
//  BUFF SPELLS (add positive affects)
// ============================================================================

function spell_armor(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af = newAffect(sn, level, Math.floor(level / 2), ApplyType.AC, -50, 0);
  affectToChar(victim, af);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}You feel someone protecting you.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_bless(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.position === Position.FIGHTING || isAffected(victim, sn))
    return SKPELL_BOTCHED;
  const af1 = newAffect(sn, level, Math.floor(level / 2), ApplyType.HITROLL, Math.floor(level / 6), 0);
  affectToChar(victim, af1);
  const af2 = newAffect(sn, level, Math.floor(level / 2), ApplyType.SAVING_SPELL, -Math.floor(level / 6), 0);
  affectToChar(victim, af2);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}You feel righteous.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_giant_strength(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const mod = 1 + (level >= 18 ? 1 : 0) + (level >= 25 ? 1 : 0) + (level >= 32 ? 1 : 0) +
    (level >= 39 ? 1 : 0) + (level >= 46 ? 1 : 0) + (level >= 70 ? 1 : 0) + (level >= 100 ? 1 : 0);
  const af = newAffect(sn, level, level, ApplyType.STR, mod, 0);
  affectToChar(victim, af);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}You feel stronger.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_haste(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af = newAffect(sn, level, Math.floor(level / 2), ApplyType.DEX, 2, AFF_HASTE);
  affectToChar(victim, af);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}You feel yourself moving faster.\r\n`);
  act('$n is moving much faster.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_invis(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_INVISIBLE) return SKPELL_MISSED;
  const af = newAffect(sn, level, 24, ApplyType.NONE, 0, AFF_INVISIBLE);
  affectToChar(victim, af);
  act('$n fades out of existence.', victim, null, null, TO_ROOM);
  sendToChar(victim, `${colors.blue}You fade out of existence.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_sanctuary(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_SANCTUARY) return SKPELL_MISSED;
  const af = newAffect(sn, level, numberFuzzy(Math.floor(level / 6)), ApplyType.NONE, 0, AFF_SANCTUARY);
  affectToChar(victim, af);
  act('$n is surrounded by a white aura.', victim, null, null, TO_ROOM);
  sendToChar(victim, `${colors.brightWhite}You are surrounded by a white aura.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_shield(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af = newAffect(sn, level, Math.floor(level / 2), ApplyType.AC, -60, 0);
  affectToChar(victim, af);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}You are surrounded by a force shield.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_stone_skin(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af = newAffect(sn, level, Math.floor(level / 2), ApplyType.AC, -80, 0);
  affectToChar(victim, af);
  act('$n\'s skin turns to stone.', victim, null, null, TO_ROOM);
  sendToChar(victim, `${colors.white}Your skin turns to stone.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_fly(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af = newAffect(sn, level, level + 3, ApplyType.NONE, 0, AFF_FLYING);
  affectToChar(victim, af);
  sendToChar(victim, `${colors.blue}Your feet rise off the ground.\r\n`);
  act('$n\'s feet rise off the ground.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_pass_door(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af = newAffect(sn, level, numberFuzzy(Math.floor(level / 4)), ApplyType.NONE, 0, AFF_PASS_DOOR);
  affectToChar(victim, af);
  sendToChar(victim, `${colors.blue}You turn translucent.\r\n`);
  act('$n turns translucent.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_detect_evil(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_DETECT_EVIL) return SKPELL_MISSED;
  const af = newAffect(sn, level, level, ApplyType.NONE, 0, AFF_DETECT_EVIL);
  affectToChar(victim, af);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}Your eyes tingle.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_detect_hidden(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_DETECT_HIDDEN) return SKPELL_MISSED;
  const af = newAffect(sn, level, level, ApplyType.NONE, 0, AFF_DETECT_HIDDEN);
  affectToChar(victim, af);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}Your awareness improves.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_detect_invis(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_DETECT_INVIS) return SKPELL_MISSED;
  const af = newAffect(sn, level, level, ApplyType.NONE, 0, AFF_DETECT_INVIS);
  affectToChar(victim, af);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}Your eyes tingle.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_detect_magic(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_DETECT_MAGIC) return SKPELL_MISSED;
  const af = newAffect(sn, level, level, ApplyType.NONE, 0, AFF_DETECT_MAGIC);
  affectToChar(victim, af);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}Your eyes tingle.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_infravision(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_INFRARED) return SKPELL_MISSED;
  const af = newAffect(sn, level, level, ApplyType.NONE, 0, AFF_INFRARED);
  affectToChar(victim, af);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}Your eyes glow red.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_protection(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_PROTECT) return SKPELL_MISSED;
  const af = newAffect(sn, level, 24, ApplyType.NONE, 0, AFF_PROTECT);
  affectToChar(victim, af);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}You feel protected.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_faerie_fire(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_FAERIE_FIRE) return SKPELL_MISSED;
  const af1 = newAffect(sn, level, level, ApplyType.AC, 2 * level, AFF_FAERIE_FIRE);
  affectToChar(victim, af1);
  const af2 = newAffect(sn, level, level, ApplyType.HITROLL, -Math.floor(level / 10), 0);
  affectToChar(victim, af2);
  sendToChar(victim, `${colors.magenta}You are surrounded by a pink outline.\r\n`);
  act('$n is surrounded by a pink outline.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_fireshield(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_FIRESHIELD) {
    affectStrip(victim, sn);
    victim.shields = uMax(0, victim.shields - 1);
    sendToChar(victim, `${colors.red}The flames around your body fade away.\r\n`);
    return SKPELL_NO_DAMAGE;
  }
  if (victim.shields >= 3) return SKPELL_MISSED;
  const af = newAffect(sn, level, -1, ApplyType.NONE, 0, AFF_FIRESHIELD);
  affectToChar(victim, af);
  victim.shields += 1;
  sendToChar(victim, `${colors.red}Your body is engulfed by unfelt flame.\r\n`);
  act('$n\'s body is engulfed in flames.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_shockshield(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_SHOCKSHIELD) {
    affectStrip(victim, sn);
    victim.shields = uMax(0, victim.shields - 1);
    sendToChar(victim, `${colors.blue}The electricity flows into the ground.\r\n`);
    return SKPELL_NO_DAMAGE;
  }
  if (victim.shields >= 3) return SKPELL_MISSED;
  const af = newAffect(sn, level, -1, ApplyType.NONE, 0, AFF_SHOCKSHIELD);
  affectToChar(victim, af);
  victim.shields += 1;
  sendToChar(victim, `${colors.blue}Crackling electricity surrounds your body.\r\n`);
  act('Crackling electricity surrounds $n\'s body.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_iceshield(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_ICESHIELD) {
    affectStrip(victim, sn);
    victim.shields = uMax(0, victim.shields - 1);
    sendToChar(victim, `${colors.cyan}The icy crust about your body melts away.\r\n`);
    return SKPELL_NO_DAMAGE;
  }
  if (victim.shields >= 3) return SKPELL_MISSED;
  const af = newAffect(sn, level, -1, ApplyType.NONE, 0, AFF_ICESHIELD);
  affectToChar(victim, af);
  victim.shields += 1;
  sendToChar(victim, `${colors.cyan}An icy crust forms about your body.\r\n`);
  act('An icy crust forms about $n\'s body.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

// ============================================================================
//  DEBUFF SPELLS
// ============================================================================

function spell_blindness(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn) || savesSpell(level, victim)) {
    sendToChar(ch, `${colors.blue}You have failed.\r\n`);
    return SKPELL_BOTCHED;
  }
  const af = newAffect(sn, level, Math.floor(level / 20), ApplyType.HITROLL, -50, AFF_BLIND);
  affectToChar(victim, af);
  act('$N is blinded!', ch, null, victim, TO_CHAR);
  sendToChar(victim, `${colors.white}You are blinded!\r\n`);
  act('$N is blinded!', ch, null, victim, TO_NOTVICT);
  return SKPELL_NO_DAMAGE;
}

function spell_curse(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) {
    sendToChar(ch, `${colors.red}A curse has already been inflicted.\r\n`);
    return SKPELL_MISSED;
  }
  if (savesSpell(level, victim)) {
    sendToChar(ch, `${colors.red}You have failed.\r\n`);
    return SKPELL_BOTCHED;
  }
  const af1 = newAffect(sn, level, Math.floor(level / 5), ApplyType.HITROLL, -50, AFF_CURSE);
  affectToChar(victim, af1);
  const af2 = newAffect(sn, level, Math.floor(level / 5), ApplyType.SAVING_SPELL, 100, 0);
  affectToChar(victim, af2);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.red}You have inflicted a curse.\r\n`);
  sendToChar(victim, `${colors.red}You feel unclean.\r\n`);
  act('$n has cursed $N!', ch, null, victim, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_poison(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (savesSpell(level, victim)) {
    sendToChar(ch, `${colors.green}You failed.\r\n`);
    return SKPELL_BOTCHED;
  }
  const af = newAffect(sn, level, level, ApplyType.STR, -2, AFF_POISON);
  affectToChar(victim, af);
  victim.poisonLevel += 1;
  sendToChar(victim, `${colors.green}You feel very sick.\r\n`);
  act('$n looks very ill.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_plague(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (savesSpell(level, victim) || isAffected(victim, sn)) {
    sendToChar(ch, `${colors.green}You failed.\r\n`);
    return SKPELL_BOTCHED;
  }
  const af = newAffect(sn, level, Math.floor(level / 2), ApplyType.STR, -3, AFF_POISON);
  affectToChar(victim, af);
  sendToChar(victim, `${colors.green}You scream in agony as plague sores erupt from your skin.\r\n`);
  act('$n screams in agony as plague sores erupt from $s skin.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_sleep(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_SLEEP || savesSpell(level, victim)) {
    return SKPELL_MISSED;
  }
  const af = newAffect(sn, level, Math.floor(level / 10), ApplyType.NONE, 0, AFF_SLEEP);
  affectToChar(victim, af);
  if (victim.position > Position.SLEEPING) {
    sendToChar(victim, `${colors.blue}You feel very sleepy ..... zzzzzz.\r\n`);
    act('$n goes to sleep.', victim, null, null, TO_ROOM);
    victim.position = Position.SLEEPING;
  }
  return SKPELL_NO_DAMAGE;
}

function spell_weaken(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn) || savesSpell(level, victim)) return SKPELL_MISSED;
  const af = newAffect(sn, level, Math.floor(level / 2), ApplyType.STR, -3, 0);
  affectToChar(victim, af);
  sendToChar(victim, `${colors.blue}You feel weaker.\r\n`);
  act('$n looks tired and weak.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_slow(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn) || savesSpell(level, victim)) return SKPELL_MISSED;
  // Slow counters haste -- strip haste if present
  const hasteSn = skillLookup('haste');
  if (isAffected(victim, hasteSn)) {
    affectStrip(victim, hasteSn);
    victim.affectedBy &= ~AFF_HASTE;
    sendToChar(victim, `${colors.blue}You feel yourself slowing down.\r\n`);
    act('$n is no longer moving so quickly.', victim, null, null, TO_ROOM);
    return SKPELL_NO_DAMAGE;
  }
  const af = newAffect(sn, level, Math.floor(level / 2), ApplyType.DEX, -2, 0);
  affectToChar(victim, af);
  sendToChar(victim, `${colors.blue}You feel yourself slowing down.\r\n`);
  act('$n begins to move in slow motion.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_confusion(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn) || savesSpell(level, victim)) return SKPELL_MISSED;
  const af = newAffect(sn, level, Math.floor(level / 6), ApplyType.HITROLL, -10, 0);
  affectToChar(victim, af);
  sendToChar(victim, `${colors.yellow}You feel very confused.\r\n`);
  act('$n looks very confused.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

// ============================================================================
//  UTILITY SPELLS
// ============================================================================

function spell_teleport(sn: number, level: number, ch: CharData, vo: unknown): number {
  // Simple teleport to a random room
  sendToChar(ch, `${colors.blue}You feel yourself dissolving...\r\n`);
  act('$n slowly fades out of existence.', ch, null, null, TO_ROOM);
  // In a full implementation, this would pick a random room. For now, no-op.
  return SKPELL_NO_DAMAGE;
}

function spell_word_of_recall(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const pcdata = world.pcData.get(victim.id);
  const recall = pcdata?.recall ?? 3001;
  if (victim.fighting) {
    sendToChar(victim, `${colors.blue}You cannot recall while fighting!\r\n`);
    return SKPELL_BOTCHED;
  }
  act('$n prays for transportation!', victim, null, null, TO_ROOM);
  charFromRoom(victim);
  charToRoom(victim, recall);
  act('$n appears in the room.', victim, null, null, TO_ROOM);
  sendToChar(victim, `${colors.brightCyan}You pray to the gods for transportation!\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_summon(sn: number, level: number, ch: CharData, vo: unknown): number {
  // Simplified summon - would need target_name global
  sendToChar(ch, `${colors.blue}You lack a target for the summon.\r\n`);
  return SKPELL_MISSED;
}

function spell_gate(sn: number, level: number, ch: CharData, vo: unknown): number {
  sendToChar(ch, `${colors.blue}You open a gate to the underworld!\r\n`);
  act('$n opens a gate to the underworld.', ch, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_identify(sn: number, level: number, ch: CharData, vo: unknown): number {
  const obj = vo as ObjInstance;
  sendToChar(ch, `${colors.cyan}Object '${obj.name}' is type ${ItemType[obj.itemType] ?? 'unknown'}.\r\n`);
  sendToChar(ch, `${colors.cyan}Weight: ${obj.weight}, Value: ${obj.cost}, Level: ${obj.level}.\r\n`);
  if (obj.affects.length > 0) {
    for (const paf of obj.affects) {
      sendToChar(ch, `${colors.cyan}Affects ${ApplyType[paf.location] ?? 'unknown'} by ${paf.modifier}.\r\n`);
    }
  }
  return SKPELL_NO_DAMAGE;
}

function spell_locate_object(sn: number, level: number, ch: CharData, vo: unknown): number {
  // Would need target_name. Simplified.
  sendToChar(ch, `${colors.blue}You sense the locations of objects...\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_charm_person(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.id === ch.id) {
    sendToChar(ch, `${colors.blue}You like yourself even better!\r\n`);
    return SKPELL_MISSED;
  }
  if (!victim.isNpc) return SKPELL_BOTCHED;
  if ((victim.affectedBy & AFF_CHARM) || (ch.affectedBy & AFF_CHARM) ||
      level < victim.level || savesSpell(level, victim)) {
    return SKPELL_MISSED;
  }
  const af = newAffect(sn, level, numberFuzzy(Math.floor(level / 4)), ApplyType.NONE, 0, AFF_CHARM);
  affectToChar(victim, af);
  sendToChar(ch, `${colors.blue}Ok.\r\n`);
  act('Isn\'t $n just so nice?', ch, null, victim, TO_VICT);
  return SKPELL_NO_DAMAGE;
}

function spell_enchant_weapon(sn: number, level: number, ch: CharData, vo: unknown): number {
  const obj = vo as ObjInstance;
  if (obj.itemType !== ItemType.WEAPON || (obj.extraFlags & 4) /* ITEM_MAGIC */ || obj.affects.length > 0) {
    sendToChar(ch, `${colors.blue}That item cannot be enchanted.\r\n`);
    return SKPELL_MISSED;
  }
  const mod = 1 + (level >= 18 ? 1 : 0) + (level >= 25 ? 1 : 0) + (level >= 45 ? 1 : 0) +
    (level >= 65 ? 1 : 0) + (level >= 90 ? 1 : 0);
  const af1 = newAffect(sn, level, -1, ApplyType.HITROLL, mod, 0);
  obj.affects.push(af1);
  const af2 = newAffect(sn, level, -1, ApplyType.DAMROLL, mod, 0);
  obj.affects.push(af2);
  obj.extraFlags |= 4; // ITEM_MAGIC
  act('$p glows.', ch, obj, null, TO_CHAR);
  sendToChar(ch, `${colors.blue}Ok.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_continual_light(sn: number, level: number, ch: CharData, vo: unknown): number {
  act('You twiddle your thumbs and a ball of light appears.', ch, null, null, TO_CHAR);
  act('$n twiddles $s thumbs and a ball of light appears.', ch, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_create_food(sn: number, level: number, ch: CharData, vo: unknown): number {
  act('A magic mushroom suddenly appears.', ch, null, null, TO_CHAR);
  act('A magic mushroom suddenly appears.', ch, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_create_water(sn: number, level: number, ch: CharData, vo: unknown): number {
  const obj = vo as ObjInstance;
  if (obj.itemType !== ItemType.DRINK_CON) {
    sendToChar(ch, `${colors.blue}It is unable to hold water.\r\n`);
    return SKPELL_BOTCHED;
  }
  act('$p is filled.', ch, obj, null, TO_CHAR);
  return SKPELL_NO_DAMAGE;
}

function spell_create_spring(sn: number, level: number, ch: CharData, vo: unknown): number {
  act('A spring flows from the ground.', ch, null, null, TO_CHAR);
  act('A spring flows from the ground.', ch, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_change_sex(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const af = newAffect(sn, level, level, ApplyType.SEX, numberRange(0, 2) === victim.sex ? 1 : numberRange(0, 2) - victim.sex, 0);
  affectToChar(victim, af);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.white}Ok.\r\n`);
  sendToChar(victim, `${colors.blue}You feel different.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_dispel_magic(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  // Remove a random affect
  if (victim.affects.length === 0) {
    sendToChar(ch, `${colors.blue}Nothing happens.\r\n`);
    return SKPELL_MISSED;
  }
  if (savesSpell(level, victim)) {
    sendToChar(ch, `${colors.blue}You failed.\r\n`);
    return SKPELL_MISSED;
  }
  // Strip a random buff
  const idx = Math.floor(Math.random() * victim.affects.length);
  const af = victim.affects[idx];
  affectRemoveFromChar(victim, af);
  sendToChar(victim, `${colors.blue}You feel a spell dissolve.\r\n`);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_cancellation(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  // Remove all dispellable affects
  const toRemove = victim.affects.filter(af => {
    if (af.deleted) return false;
    const entry = skillTable[af.type];
    return entry && entry.offMessage && !entry.offMessage.startsWith('!');
  });
  for (const af of toRemove) {
    affectRemoveFromChar(victim, af);
  }
  sendToChar(victim, `${colors.blue}You feel your magic stripped away.\r\n`);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_mass_invis(sn: number, level: number, ch: CharData, vo: unknown): number {
  const roomVnum = getCharRoom(ch);
  const chars = world.getCharsInRoom(roomVnum);
  for (const vch of chars) {
    if (vch.deleted || vch.isNpc) continue;
    if (vch.affectedBy & AFF_INVISIBLE) continue;
    const af = newAffect(sn, level, 24, ApplyType.NONE, 0, AFF_INVISIBLE);
    affectToChar(vch, af);
    sendToChar(vch, `${colors.blue}You fade out of existence.\r\n`);
  }
  act('$n causes the room to shimmer.', ch, null, null, TO_ROOM);
  sendToChar(ch, `${colors.blue}You cause the room to shimmer.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_faerie_fog(sn: number, level: number, ch: CharData, vo: unknown): number {
  sendToChar(ch, `${colors.magenta}You conjure a cloud of purple smoke.\r\n`);
  act('$n conjures a cloud of purple smoke.', ch, null, null, TO_ROOM);
  const roomVnum = getCharRoom(ch);
  const chars = world.getCharsInRoom(roomVnum);
  for (const ich of chars) {
    if (ich.id === ch.id || savesSpell(level, ich)) continue;
    affectStrip(ich, skillLookup('invis'));
    affectStrip(ich, skillLookup('mass invis'));
    affectStrip(ich, skillLookup('sneak'));
    ich.affectedBy &= ~AFF_HIDE;
    ich.affectedBy &= ~AFF_INVISIBLE;
    ich.affectedBy &= ~AFF_SNEAK;
    act('$n is revealed!', ich, null, null, TO_ROOM);
    sendToChar(ich, `${colors.magenta}You are revealed!\r\n`);
  }
  return SKPELL_NO_DAMAGE;
}

function spell_remove_invis(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  affectStrip(victim, skillLookup('invis'));
  affectStrip(victim, skillLookup('mass invis'));
  victim.affectedBy &= ~AFF_INVISIBLE;
  sendToChar(victim, `${colors.blue}You are no longer invisible.\r\n`);
  act('$n is no longer invisible.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_know_alignment(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let msg: string;
  if (victim.alignment > 700) msg = '$N has an angelic aura.';
  else if (victim.alignment > 350) msg = '$N is very good.';
  else if (victim.alignment > 100) msg = '$N is good.';
  else if (victim.alignment > -100) msg = '$N is neutral.';
  else if (victim.alignment > -350) msg = '$N is evil.';
  else if (victim.alignment > -700) msg = '$N is very evil.';
  else msg = '$N has a demonic aura.';
  act(msg, ch, null, victim, TO_CHAR);
  return SKPELL_NO_DAMAGE;
}

function spell_mana(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  victim.mana = uMin(victim.mana + level * 2, victim.maxMana);
  sendToChar(victim, `${colors.blue}You feel a surge of magical energy.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_ventriloquate(sn: number, level: number, ch: CharData, vo: unknown): number {
  // Would need target_name. Simplified.
  sendToChar(ch, `${colors.blue}You throw your voice.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_detect_poison(sn: number, level: number, ch: CharData, vo: unknown): number {
  const obj = vo as ObjInstance;
  if (obj.itemType === ItemType.DRINK_CON || obj.itemType === ItemType.FOOD) {
    if (obj.value[3] !== 0) {
      sendToChar(ch, `${colors.green}You smell poisonous fumes.\r\n`);
    } else {
      sendToChar(ch, `${colors.green}It looks very delicious.\r\n`);
    }
  } else {
    sendToChar(ch, `${colors.green}It looks very delicious.\r\n`);
  }
  return SKPELL_NO_DAMAGE;
}

// ============================================================================
//  PSIONIC SPELLS (from psionics class)
// ============================================================================

function spell_adrenaline_control(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af1 = newAffect(sn, level, Math.floor(level / 2), ApplyType.DEX, 2, 0);
  affectToChar(victim, af1);
  const af2 = newAffect(sn, level, Math.floor(level / 2), ApplyType.CON, 2, 0);
  affectToChar(victim, af2);
  sendToChar(victim, `${colors.blue}You feel a surge of adrenaline!\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_agitation(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 4);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_psychic_crush(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 8);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_mind_thrust(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 6);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_psychic_drain(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (savesSpell(level, victim)) return SKPELL_MISSED;
  let dam = dice(level, 3);
  ch.hit = uMin(ch.hit + dam, ch.maxHit);
  return dam;
}

function spell_psionic_blast(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 10);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_ultrablast(sn: number, level: number, ch: CharData, vo: unknown): number {
  sendToChar(ch, `${colors.yellow}You unleash a psychic blast!\r\n`);
  act('$n\'s eyes glow with psychic energy!', ch, null, null, TO_ROOM);
  const roomVnum = getCharRoom(ch);
  const chars = world.getCharsInRoom(roomVnum);
  for (const vch of chars) {
    if (vch.deleted || vch.id === ch.id) continue;
    damage(ch, vch, level + dice(level, 8), 'ultrablast', DamType.MENTAL);
  }
  return SKPELL_NO_DAMAGE;
}

function spell_cell_adjustment(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const heal = dice(level, 6);
  victim.hit = uMin(victim.hit + heal, victim.maxHit);
  updatePos(victim);
  sendToChar(victim, `${colors.blue}You feel your cells regenerating.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_complete_healing(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  victim.hit = victim.maxHit;
  updatePos(victim);
  sendToChar(victim, `${colors.blue}You feel completely healed!\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_psychic_healing(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const heal = Math.floor(victim.maxHit * 0.15);
  victim.hit = uMin(victim.hit + heal, victim.maxHit);
  updatePos(victim);
  sendToChar(victim, `${colors.blue}You feel much better!\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_bio_acceleration(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.position === Position.FIGHTING || isAffected(victim, sn)) return SKPELL_BOTCHED;
  const af1 = newAffect(sn, level, 40 + level, ApplyType.HIT, numberFuzzy(level * 10), 0);
  affectToChar(victim, af1);
  const af2 = newAffect(sn, level, 40 + level, ApplyType.MOVE, level * 2, 0);
  affectToChar(victim, af2);
  sendToChar(ch, `${colors.blue}You greatly enhance your bio-functions.\r\n`);
  act('$n\'s body shudders briefly.', ch, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_combat_mind(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af1 = newAffect(sn, level, Math.floor(level / 2), ApplyType.HITROLL, level >= 30 ? 3 : 2, 0);
  affectToChar(victim, af1);
  const af2 = newAffect(sn, level, Math.floor(level / 2), ApplyType.DAMROLL, level >= 30 ? 3 : 2, 0);
  affectToChar(victim, af2);
  sendToChar(victim, `${colors.blue}Your mind focuses on combat.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_inflict_pain(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 5);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_project_force(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 6);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_detonate(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 12);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_disintegrate(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 15);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

// ============================================================================
//  ADDITIONAL SPELLS (from magic2/3/4)
// ============================================================================

function spell_darkbless(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.position === Position.FIGHTING || isAffected(victim, sn)) return SKPELL_BOTCHED;
  const af1 = newAffect(sn, level, 40 + level, ApplyType.DAMROLL, Math.floor(level / 5), 0);
  affectToChar(victim, af1);
  const af2 = newAffect(sn, level, 40 + level, ApplyType.HITROLL, Math.floor(level / 5), 0);
  affectToChar(victim, af2);
  const af3 = newAffect(sn, level, 40 + level, ApplyType.HIT, level * 8, 0);
  affectToChar(victim, af3);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}You call forth the hand of oblivion.\r\n`);
  sendToChar(victim, `${colors.blue}The hand of oblivion rests upon you.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_prayer(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af = newAffect(sn, level, level, ApplyType.SAVING_SPELL, -Math.floor(level / 4), 0);
  affectToChar(victim, af);
  sendToChar(victim, `${colors.blue}You feel the gods smile upon you.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_enhanced_strength(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af = newAffect(sn, level, level, ApplyType.STR, 2 + Math.floor(level / 20), 0);
  affectToChar(victim, af);
  sendToChar(victim, `${colors.blue}You feel stronger!\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_flesh_armor(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af = newAffect(sn, level, Math.floor(level / 2), ApplyType.AC, -(level + 20), 0);
  affectToChar(victim, af);
  sendToChar(victim, `${colors.blue}Your skin hardens like armor.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_mental_barrier(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af = newAffect(sn, level, Math.floor(level / 2), ApplyType.AC, -(level + 30), 0);
  affectToChar(victim, af);
  sendToChar(victim, `${colors.blue}A mental barrier surrounds you.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_thought_shield(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af = newAffect(sn, level, Math.floor(level / 2), ApplyType.AC, -(level + 10), 0);
  affectToChar(victim, af);
  sendToChar(victim, `${colors.blue}A thought shield surrounds you.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_healing_hands(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const heal = Math.floor(victim.maxHit * 0.10);
  victim.hit = uMin(victim.hit + heal, victim.maxHit);
  updatePos(victim);
  sendToChar(victim, `${colors.blue}Healing energy flows through your body.\r\n`);
  if (ch.id !== victim.id) sendToChar(ch, `${colors.blue}Ok.\r\n`);
  return SKPELL_NO_DAMAGE;
}

function spell_golden_armor(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (isAffected(victim, sn)) return SKPELL_MISSED;
  const af = newAffect(sn, level, Math.floor(level / 2), ApplyType.AC, -(level * 2), 0);
  affectToChar(victim, af);
  sendToChar(victim, `${colors.yellow}You are encased in golden armor.\r\n`);
  act('$n is encased in golden armor.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

function spell_golden_sanctuary(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.affectedBy & AFF_SANCTUARY) return SKPELL_MISSED;
  const af = newAffect(sn, level, numberFuzzy(Math.floor(level / 4)), ApplyType.NONE, 0, AFF_SANCTUARY);
  affectToChar(victim, af);
  sendToChar(victim, `${colors.yellow}You are surrounded by a golden aura.\r\n`);
  act('$n is surrounded by a golden aura.', victim, null, null, TO_ROOM);
  return SKPELL_NO_DAMAGE;
}

// ============================================================================
//  Null spell (passive skills use this)
// ============================================================================

function spell_null(sn: number, level: number, ch: CharData, vo: unknown): number {
  return SKPELL_NO_DAMAGE;
}

// ============================================================================
//  SKILL FUNCTIONS (non-spell active abilities)
// ============================================================================

function skill_backstab(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (victim.fighting) {
    sendToChar(ch, `${colors.red}You can't backstab a fighting person!\r\n`);
    return SKPELL_BOTCHED;
  }
  if (victim.id === ch.id) {
    sendToChar(ch, `${colors.red}How can you sneak up on yourself?\r\n`);
    return SKPELL_BOTCHED;
  }
  const wield = getEquipment(ch, WearLocation.WIELD);
  if (!wield) {
    sendToChar(ch, `${colors.red}You need to wield a weapon to backstab.\r\n`);
    return SKPELL_BOTCHED;
  }
  const pcdata = world.pcData.get(ch.id);
  const learned = pcdata?.learned.get(sn) ?? 0;
  if (numberPercent() <= Math.floor(learned / 10)) {
    const baseDam = dice(wield.value[1] || 1, wield.value[2] || 4);
    const multiplier = 2 + Math.floor(ch.level / 10);
    const dam = baseDam * multiplier;
    damage(ch, victim, dam, 'backstab', DamType.PIERCE);
    return SKPELL_NO_DAMAGE;
  }
  damage(ch, victim, 0, 'backstab', DamType.PIERCE);
  return SKPELL_NO_DAMAGE;
}

function skill_bash(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (!victim) {
    sendToChar(ch, `${colors.red}Bash whom?\r\n`);
    return SKPELL_BOTCHED;
  }
  const pcdata = world.pcData.get(ch.id);
  const learned = pcdata?.learned.get(sn) ?? 0;
  if (numberPercent() <= Math.floor(learned / 10)) {
    const dam = dice(2, ch.level);
    damage(ch, victim, dam, 'bash', DamType.BASH);
    victim.position = Position.RESTING;
    sendToChar(victim, `${colors.red}You are knocked to the ground!\r\n`);
    act('$n sends $N sprawling!', ch, null, victim, TO_NOTVICT);
    sendToChar(ch, `${colors.brightRed}You slam into $N and knock them down!\r\n`);
    return SKPELL_NO_DAMAGE;
  }
  sendToChar(ch, `${colors.red}You fall flat on your face!\r\n`);
  act('$n falls flat on $s face.', ch, null, null, TO_ROOM);
  ch.position = Position.RESTING;
  return SKPELL_NO_DAMAGE;
}

function skill_kick(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const pcdata = world.pcData.get(ch.id);
  const learned = pcdata?.learned.get(sn) ?? 0;
  if (numberPercent() <= Math.floor(learned / 10)) {
    const dam = dice(3, ch.level);
    damage(ch, victim, dam, 'kick', DamType.BASH);
    return SKPELL_NO_DAMAGE;
  }
  damage(ch, victim, 0, 'kick', DamType.BASH);
  return SKPELL_NO_DAMAGE;
}

function skill_disarm(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const victimWield = getEquipment(victim, WearLocation.WIELD);
  if (!victimWield) {
    sendToChar(ch, `${colors.red}Your opponent is not wielding a weapon.\r\n`);
    return SKPELL_BOTCHED;
  }
  const pcdata = world.pcData.get(ch.id);
  const learned = pcdata?.learned.get(sn) ?? 0;
  if (numberPercent() <= Math.floor(learned / 10)) {
    act('You disarm $N!', ch, null, victim, TO_CHAR);
    act('$n disarms you!', ch, null, victim, TO_VICT);
    act('$n disarms $N!', ch, null, victim, TO_NOTVICT);
    return SKPELL_NO_DAMAGE;
  }
  sendToChar(ch, `${colors.red}You failed to disarm your opponent.\r\n`);
  return SKPELL_MISSED;
}

function skill_trip(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const pcdata = world.pcData.get(ch.id);
  const learned = pcdata?.learned.get(sn) ?? 0;
  if (numberPercent() <= Math.floor(learned / 10)) {
    act('You trip $N and $N goes down!', ch, null, victim, TO_CHAR);
    act('$n trips you and you go down!', ch, null, victim, TO_VICT);
    act('$n trips $N and $N goes down!', ch, null, victim, TO_NOTVICT);
    const dam = dice(2, 8) + Math.floor(ch.level / 3);
    damage(ch, victim, dam, 'trip', DamType.BASH);
    victim.position = Position.RESTING;
    return SKPELL_NO_DAMAGE;
  }
  sendToChar(ch, `${colors.red}You try to trip but stumble instead!\r\n`);
  return SKPELL_MISSED;
}

function skill_hide(sn: number, level: number, ch: CharData, vo: unknown): number {
  if (ch.affectedBy & AFF_HIDE) {
    ch.affectedBy &= ~AFF_HIDE;
    sendToChar(ch, `${colors.blue}You step out of the shadows.\r\n`);
    return SKPELL_NO_DAMAGE;
  }
  const pcdata = world.pcData.get(ch.id);
  const learned = pcdata?.learned.get(sn) ?? 0;
  if (numberPercent() <= Math.floor(learned / 10)) {
    ch.affectedBy |= AFF_HIDE;
    sendToChar(ch, `${colors.blue}You hide in the shadows.\r\n`);
    return SKPELL_NO_DAMAGE;
  }
  sendToChar(ch, `${colors.blue}You fail to hide.\r\n`);
  return SKPELL_MISSED;
}

function skill_sneak(sn: number, level: number, ch: CharData, vo: unknown): number {
  affectStrip(ch, sn);
  if (ch.affectedBy & AFF_SNEAK) {
    ch.affectedBy &= ~AFF_SNEAK;
    sendToChar(ch, `${colors.blue}You stop sneaking.\r\n`);
    return SKPELL_NO_DAMAGE;
  }
  const pcdata = world.pcData.get(ch.id);
  const learned = pcdata?.learned.get(sn) ?? 0;
  if (numberPercent() <= Math.floor(learned / 10)) {
    const af = newAffect(sn, level, ch.level, ApplyType.NONE, 0, AFF_SNEAK);
    affectToChar(ch, af);
    sendToChar(ch, `${colors.blue}You begin to move silently.\r\n`);
    return SKPELL_NO_DAMAGE;
  }
  sendToChar(ch, `${colors.blue}You fail to move silently.\r\n`);
  return SKPELL_MISSED;
}

function skill_pick_lock(sn: number, level: number, ch: CharData, vo: unknown): number {
  sendToChar(ch, `${colors.blue}You attempt to pick the lock...\r\n`);
  // Would need door argument logic
  return SKPELL_NO_DAMAGE;
}

function skill_steal(sn: number, level: number, ch: CharData, vo: unknown): number {
  sendToChar(ch, `${colors.blue}You attempt to steal...\r\n`);
  // Would need argument/inventory logic
  return SKPELL_NO_DAMAGE;
}

function skill_rescue(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  if (!victim.fighting) {
    sendToChar(ch, `${colors.blue}They don't need rescuing.\r\n`);
    return SKPELL_BOTCHED;
  }
  const pcdata = world.pcData.get(ch.id);
  const learned = pcdata?.learned.get(sn) ?? 0;
  if (numberPercent() <= Math.floor(learned / 10)) {
    act('You rescue $N!', ch, null, victim, TO_CHAR);
    act('$n rescues you!', ch, null, victim, TO_VICT);
    act('$n rescues $N!', ch, null, victim, TO_NOTVICT);
    return SKPELL_NO_DAMAGE;
  }
  sendToChar(ch, `${colors.blue}You fail the rescue.\r\n`);
  return SKPELL_MISSED;
}

function skill_punch(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  const pcdata = world.pcData.get(ch.id);
  const learned = pcdata?.learned.get(sn) ?? 0;
  if (numberPercent() <= Math.floor(learned / 10)) {
    const dam = dice(2, ch.level);
    damage(ch, victim, dam, 'punch', DamType.BASH);
    return SKPELL_NO_DAMAGE;
  }
  damage(ch, victim, 0, 'punch', DamType.BASH);
  return SKPELL_NO_DAMAGE;
}

// ============================================================================
//  Breath weapons
// ============================================================================

function spell_acid_breath(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 14);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_fire_breath(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 12);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_frost_breath(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 10);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

function spell_gas_breath(sn: number, level: number, ch: CharData, vo: unknown): number {
  const roomVnum = getCharRoom(ch);
  const chars = world.getCharsInRoom(roomVnum);
  for (const vch of chars) {
    if (vch.deleted || vch.id === ch.id) continue;
    const dam = dice(level, 8);
    damage(ch, vch, savesSpell(level, vch) ? Math.floor(dam / 2) : dam, 'blast of gas', DamType.POISON);
  }
  return SKPELL_NO_DAMAGE;
}

function spell_lightning_breath(sn: number, level: number, ch: CharData, vo: unknown): number {
  const victim = vo as CharData;
  let dam = dice(level, 11);
  if (savesSpell(level, victim)) dam = Math.floor(dam / 2);
  return dam;
}

// ============================================================================
//  WIRE ALL SPELL FUNCTIONS INTO THE SKILL TABLE
// ============================================================================

export function registerAllSpells(): void {
  // Damage spells
  registerSpellByName('acid blast', spell_acid_blast);
  registerSpellByName('burning hands', spell_burning_hands);
  registerSpellByName('call lightning', spell_call_lightning);
  registerSpellByName('chill touch', spell_chill_touch);
  registerSpellByName('colour spray', spell_colour_spray);
  registerSpellByName('fireball', spell_fireball);
  registerSpellByName('flamestrike', spell_flamestrike);
  registerSpellByName('harm', spell_harm);
  registerSpellByName('lightning bolt', spell_lightning_bolt);
  registerSpellByName('magic missile', spell_magic_missile);
  registerSpellByName('shocking grasp', spell_shocking_grasp);
  registerSpellByName('energy drain', spell_energy_drain);
  registerSpellByName('dispel evil', spell_dispel_evil);
  registerSpellByName('earthquake', spell_earthquake);
  registerSpellByName('cause light', spell_cause_light);
  registerSpellByName('cause serious', spell_cause_serious);
  registerSpellByName('cause critical', spell_cause_critical);
  registerSpellByName('icestorm', spell_icestorm);
  registerSpellByName('holy fires', spell_holy_fires);
  registerSpellByName('general purpose', spell_general_purpose);
  registerSpellByName('high explosive', spell_high_explosive);
  registerSpellByName('chain lightning', spell_chain_lightning);
  registerSpellByName('meteor swarm', spell_meteor_swarm);

  // Healing spells
  registerSpellByName('cure light', spell_cure_light);
  registerSpellByName('cure serious', spell_cure_serious);
  registerSpellByName('cure critical', spell_cure_critical);
  registerSpellByName('heal', spell_heal);
  registerSpellByName('group heal', spell_group_heal);
  registerSpellByName('refresh', spell_refresh);
  registerSpellByName('cure blindness', spell_cure_blindness);
  registerSpellByName('cure poison', spell_cure_poison);
  registerSpellByName('remove curse', spell_remove_curse);
  registerSpellByName('healing hands', spell_healing_hands);
  registerSpellByName('cell adjustment', spell_cell_adjustment);
  registerSpellByName('complete healing', spell_complete_healing);
  registerSpellByName('psychic healing', spell_psychic_healing);

  // Buff spells
  registerSpellByName('armor', spell_armor);
  registerSpellByName('bless', spell_bless);
  registerSpellByName('giant strength', spell_giant_strength);
  registerSpellByName('haste', spell_haste);
  registerSpellByName('invis', spell_invis);
  registerSpellByName('sanctuary', spell_sanctuary);
  registerSpellByName('shield', spell_shield);
  registerSpellByName('stone skin', spell_stone_skin);
  registerSpellByName('fly', spell_fly);
  registerSpellByName('pass door', spell_pass_door);
  registerSpellByName('detect evil', spell_detect_evil);
  registerSpellByName('detect hidden', spell_detect_hidden);
  registerSpellByName('detect invis', spell_detect_invis);
  registerSpellByName('detect magic', spell_detect_magic);
  registerSpellByName('infravision', spell_infravision);
  registerSpellByName('protection evil', spell_protection);
  registerSpellByName('faerie fire', spell_faerie_fire);
  registerSpellByName('fireshield', spell_fireshield);
  registerSpellByName('shockshield', spell_shockshield);
  registerSpellByName('iceshield', spell_iceshield);
  registerSpellByName('dark blessing', spell_darkbless);
  registerSpellByName('bio-acceleration', spell_bio_acceleration);
  registerSpellByName('prayer', spell_prayer);
  registerSpellByName('enhanced strength', spell_enhanced_strength);
  registerSpellByName('flesh armor', spell_flesh_armor);
  registerSpellByName('mental barrier', spell_mental_barrier);
  registerSpellByName('thought shield', spell_thought_shield);
  registerSpellByName('golden armor', spell_golden_armor);
  registerSpellByName('golden sanctuary', spell_golden_sanctuary);
  registerSpellByName('combat mind', spell_combat_mind);
  registerSpellByName('adrenaline control', spell_adrenaline_control);

  // Debuff spells
  registerSpellByName('blindness', spell_blindness);
  registerSpellByName('curse', spell_curse);
  registerSpellByName('poison', spell_poison);
  registerSpellByName('plague', spell_plague);
  registerSpellByName('sleep', spell_sleep);
  registerSpellByName('weaken', spell_weaken);
  registerSpellByName('slow', spell_slow);
  registerSpellByName('confusion', spell_confusion);

  // Utility spells
  registerSpellByName('teleport', spell_teleport);
  registerSpellByName('summon', spell_summon);
  registerSpellByName('gate', spell_gate);
  registerSpellByName('word of recall', spell_word_of_recall);
  registerSpellByName('identify', spell_identify);
  registerSpellByName('locate object', spell_locate_object);
  registerSpellByName('charm person', spell_charm_person);
  registerSpellByName('enchant weapon', spell_enchant_weapon);
  registerSpellByName('continual light', spell_continual_light);
  registerSpellByName('create food', spell_create_food);
  registerSpellByName('create water', spell_create_water);
  registerSpellByName('create spring', spell_create_spring);
  registerSpellByName('change sex', spell_change_sex);
  registerSpellByName('dispel magic', spell_dispel_magic);
  registerSpellByName('cancellation', spell_cancellation);
  registerSpellByName('mass invis', spell_mass_invis);
  registerSpellByName('faerie fog', spell_faerie_fog);
  registerSpellByName('remove invis', spell_remove_invis);
  registerSpellByName('know alignment', spell_know_alignment);
  registerSpellByName('mana', spell_mana);
  registerSpellByName('ventriloquate', spell_ventriloquate);
  registerSpellByName('detect poison', spell_detect_poison);

  // Psionic spells
  registerSpellByName('agitation', spell_agitation);
  registerSpellByName('psychic crush', spell_psychic_crush);
  registerSpellByName('mind thrust', spell_mind_thrust);
  registerSpellByName('psychic drain', spell_psychic_drain);
  registerSpellByName('psionic blast', spell_psionic_blast);
  registerSpellByName('ultrablast', spell_ultrablast);
  registerSpellByName('inflict pain', spell_inflict_pain);
  registerSpellByName('project force', spell_project_force);
  registerSpellByName('detonate', spell_detonate);
  registerSpellByName('disintegrate', spell_disintegrate);

  // Breath weapons
  registerSpellByName('acid breath', spell_acid_breath);
  registerSpellByName('fire breath', spell_fire_breath);
  registerSpellByName('frost breath', spell_frost_breath);
  registerSpellByName('gas breath', spell_gas_breath);
  registerSpellByName('lightning breath', spell_lightning_breath);

  // Skills
  registerSpellByName('backstab', skill_backstab);
  registerSpellByName('bash', skill_bash);
  registerSpellByName('kick', skill_kick);
  registerSpellByName('disarm', skill_disarm);
  registerSpellByName('trip', skill_trip);
  registerSpellByName('hide', skill_hide);
  registerSpellByName('sneak', skill_sneak as SpellFn);
  registerSpellByName('pick lock', skill_pick_lock);
  registerSpellByName('steal', skill_steal);
  registerSpellByName('rescue', skill_rescue);
  registerSpellByName('punch', skill_punch);

  // Passive skills get the null function
  const passiveSkills = [
    'dodge', 'enhanced dodge', 'parry', 'enhanced parry',
    'second attack', 'third attack', 'fourth attack', 'fifth attack',
    'sixth attack', 'seventh attack', 'eighth attack',
    'second strike', 'third strike', 'fourth strike', 'fifth strike',
    'sixth strike', 'seventh strike', 'eighth strike',
    'dual', 'enhanced dual', 'enhanced damage', 'enhanced damage two',
    'enhanced damage three', 'enhanced hit', 'enhanced hit two',
    'shield block', 'two handed',
    'proficiency hit', 'proficiency slice', 'proficiency pierce',
    'proficiency slash', 'proficiency crush', 'proficiency chop',
    'weaponmaster', 'unwavering reflexes', 'improved concentration',
    'scrolls', 'wands', 'staves', 'lore',
  ];
  for (const name of passiveSkills) {
    registerSpellByName(name, spell_null);
  }
}

// Export utility functions for use by magic.ts
export { numberRange, numberPercent, numberFuzzy, uRange, uMin, uMax, dice, affectStrip, updatePos, findCharInRoom };
