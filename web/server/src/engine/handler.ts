/**
 * handler.ts — Object and character manipulation for Stormgate MUD.
 *
 * Port of handler.c from the C server. Handles moving characters and objects
 * between rooms, equipping items, applying and removing affects, and
 * computing derived stats (with modifiers from equipment and spells).
 */

import type {
  CharData,
  ObjInstance,
  AffectData,
} from './types.js';

import {
  WearLocation,
  ApplyType,
  LEVEL_IMMORTAL,
} from './types.js';

import { world, charRoomMap } from './world.js';

// ============================================================================
//  Character-Room manipulation
// ============================================================================

/**
 * Move a character into a room. Updates the character-room map and the
 * world's character tracking.
 */
export function charToRoom(ch: CharData, vnum: number): void {
  const room = world.getRoom(vnum);
  if (!room) {
    console.error(`charToRoom: room vnum ${vnum} does not exist`);
    return;
  }
  charRoomMap.set(ch.id, vnum);
  // Ensure the character is in the world's live list
  if (!world.characters.has(ch.id)) {
    world.characters.set(ch.id, ch);
  }
}

/**
 * Remove a character from its current room (but keep it in the world).
 */
export function charFromRoom(ch: CharData): void {
  charRoomMap.delete(ch.id);
}

/**
 * Get the vnum of the room a character is currently in.
 * Returns -1 if the character is not in any room.
 */
export function getCharRoom(ch: CharData): number {
  return charRoomMap.get(ch.id) ?? -1;
}

// ============================================================================
//  Object-Room manipulation
// ============================================================================

/**
 * Place an object into a room on the floor.
 */
export function objToRoom(obj: ObjInstance, vnum: number): void {
  const room = world.getRoom(vnum);
  if (!room) {
    console.error(`objToRoom: room vnum ${vnum} does not exist`);
    return;
  }
  obj.inRoom = vnum;
  obj.carriedBy = undefined;
  obj.containedIn = undefined;
  obj.storedBy = undefined;
  obj.wearLoc = WearLocation.NONE;

  if (!world.objects.has(obj.id)) {
    world.objects.set(obj.id, obj);
  }
}

/**
 * Remove an object from its room.
 */
export function objFromRoom(obj: ObjInstance): void {
  obj.inRoom = undefined;
}

// ============================================================================
//  Object-Character manipulation
// ============================================================================

/**
 * Give an object to a character (carry in inventory).
 */
export function objToChar(obj: ObjInstance, ch: CharData): void {
  obj.carriedBy = ch.id;
  obj.inRoom = undefined;
  obj.containedIn = undefined;
  obj.storedBy = undefined;
  obj.wearLoc = WearLocation.NONE;

  ch.carryNumber += 1;
  ch.carryWeight += obj.weight;

  if (!world.objects.has(obj.id)) {
    world.objects.set(obj.id, obj);
  }
}

/**
 * Remove an object from a character's inventory.
 */
export function objFromChar(obj: ObjInstance): void {
  const ch = obj.carriedBy ? world.getCharById(obj.carriedBy) : undefined;
  if (ch) {
    ch.carryNumber = Math.max(0, ch.carryNumber - 1);
    ch.carryWeight = Math.max(0, ch.carryWeight - obj.weight);
  }
  obj.carriedBy = undefined;
}

// ============================================================================
//  Equipment
// ============================================================================

/**
 * Equip an item on a character at the specified wear location.
 * The object must already be in the character's inventory (carriedBy set).
 */
export function equipChar(ch: CharData, obj: ObjInstance, wearLoc: WearLocation): void {
  // Check if something is already in that slot
  const existing = getEquipment(ch, wearLoc);
  if (existing) {
    console.error(
      `equipChar: ${ch.name} already has ${existing.shortDescr} in wear slot ${wearLoc}`
    );
    return;
  }

  obj.wearLoc = wearLoc;

  // Apply the object's affects to the character
  for (const af of obj.affects) {
    applyModifier(ch, af.location, af.modifier);
  }
}

/**
 * Unequip an item from a character.
 */
export function unequipChar(ch: CharData, obj: ObjInstance): void {
  if (obj.wearLoc === WearLocation.NONE) {
    return;
  }

  // Remove the object's affects from the character
  for (const af of obj.affects) {
    applyModifier(ch, af.location, -af.modifier);
  }

  obj.wearLoc = WearLocation.NONE;
}

/**
 * Get the item equipped in a specific wear slot.
 */
export function getEquipment(ch: CharData, wearLoc: WearLocation): ObjInstance | undefined {
  for (const obj of world.objects.values()) {
    if (obj.carriedBy === ch.id && obj.wearLoc === wearLoc && !obj.deleted) {
      return obj;
    }
  }
  return undefined;
}

/**
 * Get all objects carried by a character (inventory + equipped).
 */
export function getInventory(ch: CharData): ObjInstance[] {
  const items: ObjInstance[] = [];
  for (const obj of world.objects.values()) {
    if (obj.carriedBy === ch.id && !obj.deleted) {
      items.push(obj);
    }
  }
  return items;
}

/**
 * Get only inventory items (not equipped).
 */
export function getCarriedItems(ch: CharData): ObjInstance[] {
  const items: ObjInstance[] = [];
  for (const obj of world.objects.values()) {
    if (obj.carriedBy === ch.id && obj.wearLoc === WearLocation.NONE && !obj.deleted) {
      items.push(obj);
    }
  }
  return items;
}

/**
 * Get only equipped items.
 */
export function getEquippedItems(ch: CharData): ObjInstance[] {
  const items: ObjInstance[] = [];
  for (const obj of world.objects.values()) {
    if (obj.carriedBy === ch.id && obj.wearLoc !== WearLocation.NONE && !obj.deleted) {
      items.push(obj);
    }
  }
  return items;
}

// ============================================================================
//  Affect manipulation
// ============================================================================

/**
 * Apply an affect to a character. Adds it to the affects list and
 * applies the stat modifier.
 */
export function affectToChar(ch: CharData, af: AffectData): void {
  const newAf: AffectData = { ...af };
  ch.affects.push(newAf);
  applyModifier(ch, af.location, af.modifier);

  // Set the bitvector flag
  if (af.bitvector !== 0) {
    ch.affectedBy |= af.bitvector;
  }
}

/**
 * Remove an affect from a character. Reverses the stat modifier and
 * removes it from the affects list.
 */
export function affectRemoveFromChar(ch: CharData, af: AffectData): void {
  applyModifier(ch, af.location, -af.modifier);

  // Clear the bitvector flag if no other affect sets it
  if (af.bitvector !== 0) {
    let stillAffected = false;
    for (const other of ch.affects) {
      if (other !== af && other.bitvector === af.bitvector && !other.deleted) {
        stillAffected = true;
        break;
      }
    }
    if (!stillAffected) {
      ch.affectedBy &= ~af.bitvector;
    }
  }

  const idx = ch.affects.indexOf(af);
  if (idx !== -1) {
    ch.affects.splice(idx, 1);
  }
}

/**
 * Check if a character is affected by a specific skill number (sn).
 */
export function isAffected(ch: CharData, sn: number): boolean {
  for (const af of ch.affects) {
    if (af.type === sn && !af.deleted) {
      return true;
    }
  }
  return false;
}

// ============================================================================
//  Character checks
// ============================================================================

/**
 * Can `ch` see `victim`? Simplified version for the web port.
 * In the full C server, this checks blind, darkness, invisibility,
 * sneaking, etc.
 */
export function canSee(ch: CharData, victim: CharData): boolean {
  // Immortals can always see
  if (isImmortal(ch)) return true;

  // Dead or deleted characters cannot be seen
  if (victim.deleted) return false;

  // For now, everyone can see everyone (TODO: invis, blind, etc.)
  return true;
}

/**
 * Is this character an NPC (mob)?
 */
export function isNpc(ch: CharData): boolean {
  return ch.isNpc;
}

/**
 * Is this character an immortal (level >= LEVEL_IMMORTAL)?
 */
export function isImmortal(ch: CharData): boolean {
  return ch.level >= LEVEL_IMMORTAL;
}

/**
 * Get the maximum hit points for a character, including modifiers from
 * equipment and affects.
 */
export function getMaxHit(ch: CharData): number {
  let maxHp = ch.maxHit;

  // Apply modifiers from affects
  for (const af of ch.affects) {
    if (af.location === ApplyType.HIT && !af.deleted) {
      maxHp += af.modifier;
    }
  }

  return Math.max(1, maxHp);
}

// ============================================================================
//  Attribute getters (with modifiers)
// ============================================================================

/**
 * Get effective STR for a character, applying modifiers from
 * equipment affects and spell affects.
 */
export function getStr(ch: CharData): number {
  let str = getPermanentStat(ch, 'str');
  str += getAffectModifier(ch, ApplyType.STR);
  return clampStat(str);
}

export function getInt(ch: CharData): number {
  let int = getPermanentStat(ch, 'int');
  int += getAffectModifier(ch, ApplyType.INT);
  return clampStat(int);
}

export function getWis(ch: CharData): number {
  let wis = getPermanentStat(ch, 'wis');
  wis += getAffectModifier(ch, ApplyType.WIS);
  return clampStat(wis);
}

export function getDex(ch: CharData): number {
  let dex = getPermanentStat(ch, 'dex');
  dex += getAffectModifier(ch, ApplyType.DEX);
  return clampStat(dex);
}

export function getCon(ch: CharData): number {
  let con = getPermanentStat(ch, 'con');
  con += getAffectModifier(ch, ApplyType.CON);
  return clampStat(con);
}

// ============================================================================
//  String helpers
// ============================================================================

/**
 * Capitalize the first letter of a string.
 */
export function capitalize(str: string): string {
  if (str.length === 0) return str;
  return str.charAt(0).toUpperCase() + str.slice(1);
}

/**
 * Name lookup — returns the first keyword from a character's name
 * that matches the given argument (prefix match).
 */
export function isName(arg: string, nameList: string): boolean {
  if (!arg || !nameList) return false;
  const argLower = arg.toLowerCase();
  const names = nameList.toLowerCase().split(/\s+/);
  return names.some((name) => name.startsWith(argLower));
}

// ============================================================================
//  Wear slot display names
// ============================================================================

const WEAR_SLOT_NAMES: Record<number, string> = {
  [WearLocation.LIGHT]:      '<used as light>     ',
  [WearLocation.FINGER_L]:   '<worn on finger>    ',
  [WearLocation.FINGER_R]:   '<worn on finger>    ',
  [WearLocation.NECK_1]:     '<worn around neck>  ',
  [WearLocation.NECK_2]:     '<worn around neck>  ',
  [WearLocation.BODY]:       '<worn on body>      ',
  [WearLocation.HEAD]:       '<worn on head>      ',
  [WearLocation.IN_EYES]:    '<worn in eyes>      ',
  [WearLocation.ON_FACE]:    '<worn on face>      ',
  [WearLocation.ORBIT]:      '<orbiting>          ',
  [WearLocation.ORBIT_2]:    '<orbiting>          ',
  [WearLocation.LEGS]:       '<worn on legs>      ',
  [WearLocation.FEET]:       '<worn on feet>      ',
  [WearLocation.HANDS]:      '<worn on hands>     ',
  [WearLocation.ARMS]:       '<worn on arms>      ',
  [WearLocation.SHIELD]:     '<worn as shield>    ',
  [WearLocation.ABOUT]:      '<worn about body>   ',
  [WearLocation.WAIST]:      '<worn about waist>  ',
  [WearLocation.WRIST_L]:    '<worn around wrist> ',
  [WearLocation.WRIST_R]:    '<worn around wrist> ',
  [WearLocation.WIELD]:      '<wielded>           ',
  [WearLocation.WIELD_2]:    '<dual wielded>      ',
  [WearLocation.HOLD]:       '<held>              ',
  [WearLocation.FIREARM]:    '<firearm>           ',
  [WearLocation.EARS]:       '<worn on ears>      ',
  [WearLocation.ANKLE_L]:    '<worn on ankle>     ',
  [WearLocation.ANKLE_R]:    '<worn on ankle>     ',
  [WearLocation.IMPLANTED1]: '<implanted>         ',
  [WearLocation.IMPLANTED2]: '<implanted>         ',
  [WearLocation.IMPLANTED3]: '<implanted>         ',
};

export function getWearSlotName(wearLoc: WearLocation): string {
  return WEAR_SLOT_NAMES[wearLoc] ?? '<unknown>           ';
}

// ============================================================================
//  Internal helpers
// ============================================================================

/**
 * Apply a stat modifier to a character.
 */
function applyModifier(ch: CharData, location: ApplyType, modifier: number): void {
  switch (location) {
    case ApplyType.NONE:
      break;
    case ApplyType.HIT:
      ch.maxHit += modifier;
      break;
    case ApplyType.MANA:
      ch.maxMana += modifier;
      break;
    case ApplyType.MOVE:
      ch.maxMove += modifier;
      break;
    case ApplyType.AC:
      ch.armor += modifier;
      break;
    case ApplyType.HITROLL:
      ch.hitroll += modifier;
      break;
    case ApplyType.DAMROLL:
      ch.damroll += modifier;
      break;
    case ApplyType.SAVING_PARA:
    case ApplyType.SAVING_ROD:
    case ApplyType.SAVING_PETRI:
    case ApplyType.SAVING_BREATH:
    case ApplyType.SAVING_SPELL:
      ch.savingThrow += modifier;
      break;
    // STR, INT, WIS, DEX, CON are computed on-the-fly via getStr() etc.
    // so we don't modify the base stat here — the affect is already on the list.
    default:
      break;
  }
}

/**
 * Get the sum of all affect modifiers for a given ApplyType.
 */
function getAffectModifier(ch: CharData, location: ApplyType): number {
  let total = 0;

  // From spell/skill affects
  for (const af of ch.affects) {
    if (af.location === location && !af.deleted) {
      total += af.modifier;
    }
  }

  // From equipped item affects
  for (const obj of world.objects.values()) {
    if (obj.carriedBy === ch.id && obj.wearLoc !== WearLocation.NONE && !obj.deleted) {
      for (const af of obj.affects) {
        if (af.location === location) {
          total += af.modifier;
        }
      }
    }
  }

  return total;
}

/**
 * Get the permanent (base) stat for a character. For PCs, this is
 * from PcData.permStr etc. For NPCs, use a default based on level.
 */
function getPermanentStat(ch: CharData, stat: 'str' | 'int' | 'wis' | 'dex' | 'con'): number {
  if (!ch.isNpc) {
    const pc = world.pcData.get(ch.id);
    if (pc) {
      switch (stat) {
        case 'str': return pc.permStr;
        case 'int': return pc.permInt;
        case 'wis': return pc.permWis;
        case 'dex': return pc.permDex;
        case 'con': return pc.permCon;
      }
    }
  }
  // NPC default: 13 + level/10, capped at 25
  return Math.min(25, 13 + Math.floor(ch.level / 10));
}

/**
 * Clamp a stat to valid range [1, 25].
 */
function clampStat(value: number): number {
  return Math.max(1, Math.min(25, value));
}
