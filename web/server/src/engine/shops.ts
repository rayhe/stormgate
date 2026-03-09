/**
 * shops.ts — Shop, healer, container, and extended object commands for Stormgate MUD.
 *
 * Port of act_obj.c (buy/sell/list/value, sacrifice, eat/drink/fill, examine,
 * quaff, recite, brandish, zap, containers) and healer.c from the C server.
 */

import type {
  CharData,
  ObjInstance,
  ShopData,
  AffectData,
} from './types.js';

import {
  WearLocation,
  ItemType,
  ApplyType,
} from './types.js';

import { world, charRoomMap } from './world.js';

import {
  getCharRoom,
  objToRoom,
  objFromRoom,
  objToChar,
  objFromChar,
  getCarriedItems,
  getEquipment,
  isName,
  isImmortal,
  capitalize,
  affectToChar,
} from './handler.js';

import {
  sendToChar,
  act,
  colors,
  TO_ROOM,
  TO_CHAR,
  TO_VICT,
} from './output.js';

import {
  sendVitals,
} from './protocol.js';

import {
  CONT_CLOSED,
} from './resets.js';

// ============================================================================
//  Extra-flag constants (from merc.h)
// ============================================================================

/** Object extra_flags bit: infinite stock item in shops. */
export const ITEM_FLAG_INVENTORY = 8192;

/** Object extra_flags bit: object can be taken (wearFlags bit). */
export const WEAR_FLAG_TAKE = 1;

/** NPC act flag: mob is a healer. */
export const ACT_IS_HEALER = 8;

// ============================================================================
//  Liquid table (simplified from the C source)
// ============================================================================

interface LiquidEntry {
  name: string;
  color: string;
  /** [drunk, full, thirst] modifiers */
  affect: [number, number, number];
}

const LIQ_TABLE: LiquidEntry[] = [
  { name: 'water',            color: 'clear',         affect: [0,  1, 10] },
  { name: 'beer',             color: 'amber',         affect: [3,  2,  5] },
  { name: 'wine',             color: 'rose',          affect: [5,  2,  5] },
  { name: 'ale',              color: 'brown',         affect: [2,  2,  5] },
  { name: 'dark ale',         color: 'dark',          affect: [1,  2,  5] },
  { name: 'whisky',           color: 'golden',        affect: [6,  1,  4] },
  { name: 'lemonade',         color: 'pink',          affect: [0,  1,  8] },
  { name: 'firebreather',     color: 'boiling',       affect: [10, 0,  0] },
  { name: 'local specialty',  color: 'evilly dark',   affect: [3,  1,  3] },
  { name: 'slime mold juice', color: 'green',         affect: [0,  4, -8] },
  { name: 'milk',             color: 'white',         affect: [0,  3,  6] },
  { name: 'tea',              color: 'tan',           affect: [0,  1,  6] },
  { name: 'coffee',           color: 'black',         affect: [0,  1,  6] },
  { name: 'blood',            color: 'red',           affect: [0,  2, -1] },
  { name: 'salt water',       color: 'clear',         affect: [0,  1, -2] },
  { name: 'cola',             color: 'cherry',        affect: [0,  1,  5] },
];

// ============================================================================
//  Helper functions
// ============================================================================

/**
 * Parse "arg1 arg2" from an argument string.
 * Returns [firstWord (lowercased), remainder (untrimmed rest)].
 */
function oneArgument(argument: string): [string, string] {
  const trimmed = argument.trim();
  const spaceIdx = trimmed.indexOf(' ');
  if (spaceIdx === -1) {
    return [trimmed.toLowerCase(), ''];
  }
  return [trimmed.substring(0, spaceIdx).toLowerCase(), trimmed.substring(spaceIdx + 1).trim()];
}

/**
 * Find a character in the same room as `ch` by keyword.
 */
function findCharInRoom(ch: CharData, arg: string): CharData | null {
  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) return null;
  const chars = world.getCharsInRoom(roomVnum);
  for (const mob of chars) {
    if (mob.id === ch.id) continue;
    if (isName(arg, mob.name)) {
      return mob;
    }
  }
  return null;
}

/**
 * Find a shopkeeper NPC in the same room as ch.
 * Returns [keeper, shopData] or [null, null].
 */
function findKeeper(ch: CharData): [CharData | null, ShopData | null] {
  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You can't do that here.\r\n");
    return [null, null];
  }

  const chars = world.getCharsInRoom(roomVnum);
  for (const mob of chars) {
    if (!mob.isNpc) continue;

    // Look up shop data for this mob's template vnum
    const mobVnum = getMobVnum(mob);
    if (mobVnum === 0) continue;

    for (const shop of world.shops.values()) {
      if (shop.keeperVnum === mobVnum) {
        // Check shop hours
        if (world.time.hour < shop.openHour) {
          act("$N says 'Sorry, come back later.'", ch, null, mob, TO_CHAR);
          return [null, null];
        }
        if (world.time.hour > shop.closeHour) {
          act("$N says 'Sorry, come back tomorrow.'", ch, null, mob, TO_CHAR);
          return [null, null];
        }
        return [mob, shop];
      }
    }
  }

  sendToChar(ch, "You can't do that here.\r\n");
  return [null, null];
}

/**
 * Get the mob template vnum for an NPC character.
 * We check the indexVnum field first (set during mob creation),
 * then fall back to searching templates by short description.
 */
function getMobVnum(mob: CharData): number {
  // If indexVnum is set on the mob (typical for NPC instances)
  if ((mob as any).indexVnum) {
    return (mob as any).indexVnum;
  }
  // Fallback: search templates
  for (const [vnum, template] of world.mobTemplates) {
    if (template.shortDescr === mob.shortDescr && template.name === mob.name) {
      return vnum;
    }
  }
  return 0;
}

/**
 * Get the buy cost for an item from a shop.
 */
function getCost(keeper: CharData, obj: ObjInstance, shop: ShopData, fBuy: boolean): number {
  if (!obj) return 0;

  let cost: number;

  if (fBuy) {
    cost = Math.floor(obj.cost * shop.profitBuy / 100);
  } else {
    cost = 0;
    for (let i = 0; i < 5; i++) {
      if (obj.itemType === shop.buyType[i]) {
        cost = Math.floor(obj.cost * shop.profitSell / 100);
        break;
      }
    }

    // Reduce price if keeper already has a similar item
    if (cost > 0) {
      const keeperItems = getKeeperInventory(keeper);
      for (const obj2 of keeperItems) {
        if (obj2.indexVnum === obj.indexVnum) {
          cost = Math.floor(cost / 2);
        }
      }
    }
  }

  // Adjust cost for wands/staves based on remaining charges
  if (obj.itemType === ItemType.STAFF || obj.itemType === ItemType.WAND) {
    if (obj.value[1] > 0) {
      cost = Math.floor(cost * obj.value[2] / obj.value[1]);
    }
  }

  return cost;
}

/**
 * Get inventory items carried by a keeper (not equipped).
 */
function getKeeperInventory(keeper: CharData): ObjInstance[] {
  const items: ObjInstance[] = [];
  for (const obj of world.objects.values()) {
    if (obj.carriedBy === keeper.id && obj.wearLoc === WearLocation.NONE && !obj.deleted) {
      items.push(obj);
    }
  }
  return items;
}

/**
 * Find an object in a character's carried inventory by keyword.
 */
function getObjCarry(ch: CharData, arg: string): ObjInstance | null {
  const items = getCarriedItems(ch);
  for (const obj of items) {
    if (isName(arg, obj.name)) {
      return obj;
    }
  }
  return null;
}

/**
 * Find an object either in inventory, equipped, or in the room.
 */
function getObjHere(ch: CharData, arg: string): ObjInstance | null {
  // Check inventory first
  const carried = getObjCarry(ch, arg);
  if (carried) return carried;

  // Check equipped
  for (const obj of world.objects.values()) {
    if (obj.carriedBy === ch.id && obj.wearLoc !== WearLocation.NONE
      && !obj.deleted && isName(arg, obj.name)) {
      return obj;
    }
  }

  // Check room
  const roomVnum = getCharRoom(ch);
  if (roomVnum !== -1) {
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
 * Get all objects contained inside a container object.
 */
function getContainedObjs(container: ObjInstance): ObjInstance[] {
  const items: ObjInstance[] = [];
  for (const obj of world.objects.values()) {
    if (obj.containedIn === container.id && !obj.deleted) {
      items.push(obj);
    }
  }
  return items;
}

/**
 * Put an object into a container.
 */
function objToObj(obj: ObjInstance, container: ObjInstance): void {
  obj.containedIn = container.id;
  obj.carriedBy = undefined;
  obj.inRoom = undefined;
  obj.storedBy = undefined;
  obj.wearLoc = WearLocation.NONE;
  if (!world.objects.has(obj.id)) {
    world.objects.set(obj.id, obj);
  }
}

/**
 * Remove an object from its container.
 */
function objFromObj(obj: ObjInstance): void {
  obj.containedIn = undefined;
}

/**
 * Extract (destroy) an object from the world.
 */
function extractObj(obj: ObjInstance): void {
  obj.deleted = true;
  if (obj.carriedBy) {
    objFromChar(obj);
  }
  if (obj.inRoom !== undefined) {
    objFromRoom(obj);
  }
  if (obj.containedIn) {
    objFromObj(obj);
  }
  world.objects.delete(obj.id);
}

/**
 * Find a healer mob in the room.
 */
function findHealer(ch: CharData): CharData | null {
  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) return null;

  const chars = world.getCharsInRoom(roomVnum);
  for (const mob of chars) {
    if (!mob.isNpc) continue;
    if (mob.act & ACT_IS_HEALER) {
      return mob;
    }
  }
  return null;
}

// ============================================================================
//  Shop commands
// ============================================================================

/**
 * doBuy -- Buy an item from a shopkeeper.
 */
export function doBuy(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Buy what?\r\n");
    return;
  }

  const [arg1] = oneArgument(argument);

  const [keeper, shop] = findKeeper(ch);
  if (!keeper || !shop) return;

  // Find the item in the keeper's inventory
  const keeperItems = getKeeperInventory(keeper);
  let obj: ObjInstance | null = null;
  for (const item of keeperItems) {
    if (isName(arg1, item.name)) {
      obj = item;
      break;
    }
  }

  if (!obj) {
    act("$N tells you 'I don't sell that -- try 'list''.", keeper, null, ch, TO_VICT);
    return;
  }

  const cost = getCost(keeper, obj, shop, true);
  if (cost <= 0) {
    act("$N tells you 'I don't sell that -- try 'list''.", keeper, null, ch, TO_VICT);
    return;
  }

  if (ch.gold < cost) {
    act("$N tells you 'You can't afford to buy $p.'", keeper, obj, ch, TO_VICT);
    return;
  }

  if (obj.level > ch.level) {
    act("$N tells you 'You can't use $p yet.'", keeper, obj, ch, TO_VICT);
    return;
  }

  ch.gold -= cost;
  keeper.gold += cost;

  act("You buy $p.", ch, obj, null, TO_CHAR);
  act("$n buys $p.", ch, obj, null, TO_ROOM);

  if (obj.extraFlags & ITEM_FLAG_INVENTORY) {
    // Infinite stock: create a new copy instead of transferring
    const newObj = world.createObjInstance(obj.indexVnum);
    if (newObj) {
      objToChar(newObj, ch);
    }
  } else {
    // Transfer from keeper to player
    objFromChar(obj);
    objToChar(obj, ch);
  }

  sendVitals(ch);
}

/**
 * doSell -- Sell an item to a shopkeeper.
 */
export function doSell(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Sell what?\r\n");
    return;
  }

  const [arg1] = oneArgument(argument);

  const [keeper, shop] = findKeeper(ch);
  if (!keeper || !shop) return;

  const obj = getObjCarry(ch, arg1);
  if (!obj) {
    act("$N tells you 'You don't have that item.'", keeper, null, ch, TO_VICT);
    return;
  }

  if (obj.wearLoc !== WearLocation.NONE) {
    sendToChar(ch, "You must remove it first.\r\n");
    return;
  }

  const cost = getCost(keeper, obj, shop, false);
  if (cost <= 0) {
    act("$N looks uninterested in $p.", keeper, obj, ch, TO_VICT);
    return;
  }

  const costMsg = `You sell $p for ${cost} gold piece${cost === 1 ? '' : 's'}.`;
  act(costMsg, ch, obj, null, TO_CHAR);
  act("$n sells $p.", ch, obj, null, TO_ROOM);

  ch.gold += cost;
  keeper.gold -= cost;
  if (keeper.gold < 0) keeper.gold = 0;

  if (obj.itemType === ItemType.TRASH) {
    extractObj(obj);
  } else {
    objFromChar(obj);
    objToChar(obj, keeper);
  }

  sendVitals(ch);
}

/**
 * doList -- List items for sale at a shopkeeper.
 */
export function doList(ch: CharData, argument: string): void {
  const [keeper, shop] = findKeeper(ch);
  if (!keeper || !shop) return;

  const keeperItems = getKeeperInventory(keeper);
  let found = false;
  let output = '';

  const [filterArg] = oneArgument(argument);

  for (const obj of keeperItems) {
    const cost = getCost(keeper, obj, shop, true);
    if (cost <= 0) continue;

    if (filterArg && !isName(filterArg, obj.name)) continue;

    if (!found) {
      found = true;
      output += `${colors.cyan}[Lv Price] Item${colors.reset}\r\n`;
    }

    const lvStr = obj.level.toString().padStart(2, ' ');
    const costStr = cost.toString().padStart(5, ' ');
    output += `${colors.cyan}[${lvStr} ${costStr}] ${capitalize(obj.shortDescr)}.${colors.reset}\r\n`;
  }

  if (!found) {
    if (!filterArg) {
      sendToChar(ch, "You can't buy anything here.\r\n");
    } else {
      sendToChar(ch, "You can't buy that here.\r\n");
    }
    return;
  }

  sendToChar(ch, output);
}

/**
 * doValue -- Appraise an item's sell value at a shopkeeper.
 */
export function doValue(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Value what?\r\n");
    return;
  }

  const [arg1] = oneArgument(argument);

  const [keeper, shop] = findKeeper(ch);
  if (!keeper || !shop) return;

  const obj = getObjCarry(ch, arg1);
  if (!obj) {
    act("$N tells you 'You don't have that item.'", keeper, null, ch, TO_VICT);
    return;
  }

  if (obj.wearLoc !== WearLocation.NONE) {
    sendToChar(ch, "You must remove it first.\r\n");
    return;
  }

  const cost = getCost(keeper, obj, shop, false);
  if (cost <= 0) {
    act("$N looks uninterested in $p.", keeper, obj, ch, TO_VICT);
    return;
  }

  const msg = `$N tells you 'I'll give you ${cost} gold coin${cost === 1 ? '' : 's'} for $p.'`;
  act(msg, keeper, obj, ch, TO_VICT);
}

// ============================================================================
//  Extended object commands
// ============================================================================

/**
 * doExamine -- Examine an object (show description + contents if container).
 */
export function doExamine(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Examine what?\r\n");
    return;
  }

  const [arg1] = oneArgument(argument);

  const obj = getObjHere(ch, arg1);
  if (!obj) {
    sendToChar(ch, "You don't see that here.\r\n");
    return;
  }

  // Show the object's description
  sendToChar(ch, `You look at ${obj.shortDescr}.\r\n`);
  if (obj.description) {
    sendToChar(ch, `${obj.description}\r\n`);
  }

  act("$n examines $p.", ch, obj, null, TO_ROOM);

  switch (obj.itemType) {
    case ItemType.WEAPON:
    case ItemType.ARMOR: {
      // Show condition based on cost ratio vs. template cost
      const template = world.getObjTemplate(obj.indexVnum);
      const baseCost = template ? template.cost : obj.cost;
      let brk: number;
      if (baseCost > 0) {
        brk = Math.floor((obj.cost * 100) / baseCost);
      } else {
        brk = 101;
      }
      let msg: string;
      if (brk === 0)       msg = 'is utterly destroyed!';
      else if (brk <= 10)  msg = 'is almost useless.';
      else if (brk <= 20)  msg = 'should be replaced soon.';
      else if (brk <= 30)  msg = 'is in pretty bad shape.';
      else if (brk <= 40)  msg = 'has seen better days.';
      else if (brk <= 50)  msg = 'could use some repairs.';
      else if (brk <= 60)  msg = 'is in average condition.';
      else if (brk <= 70)  msg = 'has the odd dent.';
      else if (brk <= 80)  msg = 'needs a bit of polishing.';
      else if (brk <= 90)  msg = 'looks almost new.';
      else if (brk <= 100) msg = 'is in perfect condition.';
      else                 msg = 'looks almost indestructible!';

      sendToChar(ch, `Looking closer, you see that ${obj.shortDescr} ${msg}\r\n`);
      break;
    }
    case ItemType.CONTAINER:
    case ItemType.CORPSE_NPC:
    case ItemType.CORPSE_PC: {
      const contents = getContainedObjs(obj);
      if (contents.length === 0) {
        sendToChar(ch, "When you look inside, you see:\r\n  Nothing.\r\n");
      } else {
        sendToChar(ch, "When you look inside, you see:\r\n");
        for (const item of contents) {
          sendToChar(ch, `  ${capitalize(item.shortDescr)}\r\n`);
        }
      }
      break;
    }
    case ItemType.DRINK_CON: {
      if (obj.value[1] <= 0) {
        sendToChar(ch, "It is empty.\r\n");
      } else {
        const liquid = obj.value[2];
        const liqName = liquid >= 0 && liquid < LIQ_TABLE.length
          ? LIQ_TABLE[liquid].name : 'unknown liquid';
        sendToChar(ch, `It contains ${liqName}.\r\n`);
      }
      break;
    }
    default:
      break;
  }
}

/**
 * doSacrifice -- Sacrifice an item on the ground to the gods.
 */
export function doSacrifice(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "The gods appreciate your offer and may accept it later.\r\n");
    act("$n offers $mself to the gods, who graciously decline.", ch, null, null, TO_ROOM);
    return;
  }

  const [arg1] = oneArgument(argument);
  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You are in the void.\r\n");
    return;
  }

  if (arg1 === 'all') {
    const roomObjs = world.getObjsInRoom(roomVnum);
    if (roomObjs.length === 0) {
      sendToChar(ch, "There is nothing here to sacrifice.\r\n");
      return;
    }

    let totalGold = 0;
    const toSac = [...roomObjs];
    for (const obj of toSac) {
      if (obj.deleted) continue;
      if (!(obj.wearFlags & WEAR_FLAG_TAKE)) continue;

      const gain = Math.max(1, Math.min(
        Math.floor(Math.random() * obj.level) + 1,
        ch.level
      ));
      totalGold += gain;
      extractObj(obj);
    }

    if (totalGold === 0) {
      sendToChar(ch, "There is nothing here to sacrifice.\r\n");
      return;
    }

    ch.gold += totalGold;
    sendToChar(ch, `The gods give you ${totalGold} gold coin${totalGold === 1 ? '' : 's'} for your sacrifice.\r\n`);
    act("$n sacrifices everything here to the gods.", ch, null, null, TO_ROOM);
    sendVitals(ch);
    return;
  }

  // Find specific object in room
  const roomObjs = world.getObjsInRoom(roomVnum);
  let obj: ObjInstance | null = null;
  for (const item of roomObjs) {
    if (isName(arg1, item.name)) {
      obj = item;
      break;
    }
  }

  if (!obj) {
    sendToChar(ch, "You can't find it.\r\n");
    return;
  }

  if (!(obj.wearFlags & WEAR_FLAG_TAKE)) {
    act("$p is not an acceptable sacrifice.", ch, obj, null, TO_CHAR);
    return;
  }

  const gain = Math.max(1, Math.min(
    Math.floor(Math.random() * obj.level) + 1,
    ch.level
  ));
  ch.gold += gain;

  sendToChar(ch, `The gods give you ${gain} gold coin${gain === 1 ? '' : 's'} for your sacrifice.\r\n`);
  act("$n sacrifices $p to the gods.", ch, obj, null, TO_ROOM);
  extractObj(obj);
  sendVitals(ch);
}

/**
 * doEat -- Eat food or a pill.
 */
export function doEat(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Eat what?\r\n");
    return;
  }

  const [arg1] = oneArgument(argument);
  const obj = getObjCarry(ch, arg1);

  if (!obj) {
    sendToChar(ch, "You do not have that item.\r\n");
    return;
  }

  if (!isImmortal(ch)) {
    if (obj.itemType !== ItemType.FOOD && obj.itemType !== ItemType.PILL
      && obj.itemType !== ItemType.BERRY) {
      sendToChar(ch, "That's not edible.\r\n");
      return;
    }

    const pc = world.pcData.get(ch.id);
    if (pc && pc.condition[1] > 40) {
      sendToChar(ch, "You are too full to eat more.\r\n");
      return;
    }
  }

  act("You eat $p.", ch, obj, null, TO_CHAR);
  act("$n eats $p.", ch, obj, null, TO_ROOM);

  switch (obj.itemType) {
    case ItemType.FOOD: {
      const pc = world.pcData.get(ch.id);
      if (pc) {
        const prevCondition = pc.condition[1];
        pc.condition[1] = Math.min(48, pc.condition[1] + obj.value[0]);
        if (pc.condition[1] > 40) {
          sendToChar(ch, "You are full.\r\n");
        } else if (prevCondition === 0 && pc.condition[1] > 0) {
          sendToChar(ch, "You are no longer hungry.\r\n");
        }
      }

      // Check for poison (value[3] != 0 means poisoned food)
      if (obj.value[3] !== 0) {
        act("$n chokes and gags.", ch, null, null, TO_ROOM);
        sendToChar(ch, "You choke and gag.\r\n");

        const af: AffectData = {
          type: 0,
          level: obj.level,
          duration: 2 * obj.value[0],
          location: ApplyType.STR,
          modifier: -2,
          bitvector: 0,
          deleted: false,
        };
        affectToChar(ch, af);
      }
      break;
    }
    case ItemType.BERRY: {
      const lo = obj.value[0] || 0;
      const hi = obj.value[1] || lo;
      const amount = lo + Math.floor(Math.random() * (hi - lo + 1));
      ch.hit = Math.min(ch.hit + amount, ch.maxHit);
      sendToChar(ch, "You feel warm all over.\r\n");
      break;
    }
    case ItemType.PILL: {
      // Pills apply spell effects via obj_cast_spell
      // Apply simplified healing proportional to pill level
      applyMagicItemEffects(ch, obj.value[0], obj.value[1], ch, null);
      applyMagicItemEffects(ch, obj.value[0], obj.value[2], ch, null);
      applyMagicItemEffects(ch, obj.value[0], obj.value[3], ch, null);
      break;
    }
    default:
      break;
  }

  extractObj(obj);
  sendVitals(ch);
}

/**
 * doDrink -- Drink from a container or fountain.
 */
export function doDrink(ch: CharData, argument: string): void {
  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You are in the void.\r\n");
    return;
  }

  let obj: ObjInstance | null = null;

  if (!argument) {
    // Look for a fountain in the room
    const roomObjs = world.getObjsInRoom(roomVnum);
    for (const item of roomObjs) {
      if (item.itemType === ItemType.FOUNTAIN) {
        obj = item;
        break;
      }
    }
    if (!obj) {
      sendToChar(ch, "Drink what?\r\n");
      return;
    }
  } else {
    const [arg1] = oneArgument(argument);
    obj = getObjHere(ch, arg1);
    if (!obj) {
      sendToChar(ch, "You can't find it.\r\n");
      return;
    }
  }

  switch (obj.itemType) {
    case ItemType.FOUNTAIN: {
      const pc = world.pcData.get(ch.id);
      if (pc) {
        pc.condition[2] = 48; // full thirst
      }
      act("You drink from $p.", ch, obj, null, TO_CHAR);
      sendToChar(ch, "You are not thirsty.\r\n");
      act("$n drinks from $p.", ch, obj, null, TO_ROOM);
      break;
    }
    case ItemType.DRINK_CON: {
      if (obj.value[1] <= 0) {
        sendToChar(ch, "It is already empty.\r\n");
        return;
      }

      const liquid = Math.max(0, Math.min(obj.value[2], LIQ_TABLE.length - 1));
      const liqName = LIQ_TABLE[liquid].name;

      act(`You drink ${liqName} from $p.`, ch, obj, null, TO_CHAR);
      act(`$n drinks ${liqName} from $p.`, ch, obj, null, TO_ROOM);

      const amount = Math.min(3 + Math.floor(Math.random() * 6), obj.value[1]);

      const pc = world.pcData.get(ch.id);
      if (pc) {
        pc.condition[0] = Math.min(100, pc.condition[0] + LIQ_TABLE[liquid].affect[0]);
        pc.condition[1] = Math.min(48, pc.condition[1] + amount * LIQ_TABLE[liquid].affect[1]);
        pc.condition[2] = Math.min(48, pc.condition[2] + amount * LIQ_TABLE[liquid].affect[2]);

        if (pc.condition[0] > 10) {
          sendToChar(ch, "You feel drunk.\r\n");
        }
        if (pc.condition[1] > 40) {
          sendToChar(ch, "You are full.\r\n");
        }
        if (pc.condition[2] > 40) {
          sendToChar(ch, "You do not feel thirsty.\r\n");
        }
      }

      // Check for poison
      if (obj.value[3] !== 0) {
        sendToChar(ch, "You choke and gag.\r\n");
        act("$n chokes and gags.", ch, null, null, TO_ROOM);

        const af: AffectData = {
          type: 0,
          level: obj.level,
          duration: 3 * amount,
          location: ApplyType.STR,
          modifier: -2,
          bitvector: 0,
          deleted: false,
        };
        affectToChar(ch, af);
      }

      obj.value[1] -= amount;
      if (obj.value[1] <= 0) {
        sendToChar(ch, "The empty container vanishes.\r\n");
        extractObj(obj);
      }
      break;
    }
    default:
      sendToChar(ch, "You can't drink from that.\r\n");
      return;
  }

  sendVitals(ch);
}

/**
 * doFill -- Fill a drink container from a fountain.
 */
export function doFill(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Fill what?\r\n");
    return;
  }

  const [arg1] = oneArgument(argument);
  const obj = getObjCarry(ch, arg1);

  if (!obj) {
    sendToChar(ch, "You do not have that item.\r\n");
    return;
  }

  if (obj.itemType !== ItemType.DRINK_CON) {
    sendToChar(ch, "You can't fill that.\r\n");
    return;
  }

  // Find a fountain in the room
  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You are in the void.\r\n");
    return;
  }

  const roomObjs = world.getObjsInRoom(roomVnum);
  let fountain: ObjInstance | null = null;
  for (const item of roomObjs) {
    if (item.itemType === ItemType.FOUNTAIN) {
      fountain = item;
      break;
    }
  }

  if (!fountain) {
    sendToChar(ch, "There is no fountain here!\r\n");
    return;
  }

  // Can't fill if already has a different liquid (value[2] is liquid type,
  // value[1] is current amount -- if non-empty and not water, reject)
  if (obj.value[1] !== 0 && obj.value[2] !== 0) {
    sendToChar(ch, "There is already another liquid in it.\r\n");
    return;
  }

  if (obj.value[1] >= obj.value[0]) {
    sendToChar(ch, "Your container is full.\r\n");
    return;
  }

  act("You fill $p.", ch, obj, null, TO_CHAR);
  act("$n fills $p.", ch, obj, null, TO_ROOM);
  obj.value[2] = 0; // water
  obj.value[1] = obj.value[0]; // fill to max capacity
}

/**
 * doQuaff -- Drink a potion.
 */
export function doQuaff(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Quaff what?\r\n");
    return;
  }

  const [arg1] = oneArgument(argument);
  const obj = getObjCarry(ch, arg1);

  if (!obj) {
    sendToChar(ch, "You do not have that potion.\r\n");
    return;
  }

  if (obj.itemType !== ItemType.POTION) {
    sendToChar(ch, "You can quaff only potions.\r\n");
    return;
  }

  act("You quaff $p.", ch, obj, null, TO_CHAR);
  act("$n quaffs $p.", ch, obj, null, TO_ROOM);

  if (obj.level > ch.level) {
    sendToChar(ch, "That potion is too high level for you.\r\n");
  } else {
    // Apply potion spell effects
    // value[0] = spell level, value[1..3] = spell slot numbers
    applyMagicItemEffects(ch, obj.value[0], obj.value[1], ch, null);
    applyMagicItemEffects(ch, obj.value[0], obj.value[2], ch, null);
    applyMagicItemEffects(ch, obj.value[0], obj.value[3], ch, null);
  }

  extractObj(obj);
  sendVitals(ch);
}

/**
 * doRecite -- Read a scroll to cast its spells.
 */
export function doRecite(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Recite what?\r\n");
    return;
  }

  const [arg1, rest] = oneArgument(argument);
  const [arg2] = oneArgument(rest);

  const scroll = getObjCarry(ch, arg1);
  if (!scroll) {
    sendToChar(ch, "You do not have that scroll.\r\n");
    return;
  }

  if (scroll.itemType !== ItemType.SCROLL) {
    sendToChar(ch, "You can recite only scrolls.\r\n");
    return;
  }

  // Determine target
  let victim: CharData | null = ch;
  let targetObj: ObjInstance | null = null;

  if (arg2) {
    // Try to find a character in the room
    const foundChar = findCharInRoom(ch, arg2);
    if (foundChar) {
      victim = foundChar;
    } else {
      // Try to find an object
      targetObj = getObjHere(ch, arg2);
      if (!targetObj) {
        sendToChar(ch, "You can't find it.\r\n");
        return;
      }
      victim = null;
    }
  }

  act("You recite $p.", ch, scroll, null, TO_CHAR);
  act("$n recites $p.", ch, scroll, null, TO_ROOM);

  if (scroll.level > ch.level) {
    sendToChar(ch, "That scroll is too high level for you.\r\n");
  } else {
    applyMagicItemEffects(ch, scroll.value[0], scroll.value[1], victim, targetObj);
    applyMagicItemEffects(ch, scroll.value[0], scroll.value[2], victim, targetObj);
    applyMagicItemEffects(ch, scroll.value[0], scroll.value[3], victim, targetObj);
  }

  extractObj(scroll);
  sendVitals(ch);
}

/**
 * doBrandish -- Use a staff to affect all targets in the room.
 */
export function doBrandish(ch: CharData, argument: string): void {
  const staff = getEquipment(ch, WearLocation.HOLD);

  if (!staff) {
    sendToChar(ch, "You hold nothing in your hand.\r\n");
    return;
  }

  if (staff.itemType !== ItemType.STAFF) {
    sendToChar(ch, "You can brandish only with a staff.\r\n");
    return;
  }

  // Check charges: value[1] = max charges (-1 = infinite), value[2] = current charges
  if (staff.value[2] <= 0 && staff.value[1] !== -1) {
    sendToChar(ch, "Your staff has no more charges.\r\n");
    return;
  }

  act("You brandish $p.", ch, staff, null, TO_CHAR);
  act("$n brandishes $p.", ch, staff, null, TO_ROOM);

  // Apply to appropriate targets in the room
  const roomVnum = getCharRoom(ch);
  if (roomVnum !== -1) {
    const chars = world.getCharsInRoom(roomVnum);
    for (const vch of chars) {
      if (vch.deleted) continue;
      applyMagicItemEffects(ch, staff.value[0], staff.value[3], vch, null);
    }
  }

  // Decrement charges
  if (staff.value[1] !== -1) {
    staff.value[2]--;
    if (staff.value[2] <= 0) {
      act("Your $p blazes bright and is gone.", ch, staff, null, TO_CHAR);
      act("$n's $p blazes bright and is gone.", ch, staff, null, TO_ROOM);
      extractObj(staff);
    }
  }

  sendVitals(ch);
}

/**
 * doZap -- Use a wand on a target.
 */
export function doZap(ch: CharData, argument: string): void {
  const wand = getEquipment(ch, WearLocation.HOLD);

  if (!wand) {
    sendToChar(ch, "You hold nothing in your hand.\r\n");
    return;
  }

  if (wand.itemType !== ItemType.WAND) {
    sendToChar(ch, "You can zap only with a wand.\r\n");
    return;
  }

  // Check charges
  if (wand.value[2] <= 0 && wand.value[1] !== -1) {
    sendToChar(ch, "Your wand has no more charges.\r\n");
    return;
  }

  let victim: CharData | null = null;
  let targetObj: ObjInstance | null = null;

  if (!argument) {
    // If fighting, target the opponent
    if (ch.fighting) {
      victim = world.getCharById(ch.fighting) ?? null;
    }
    if (!victim) {
      sendToChar(ch, "Zap whom or what?\r\n");
      return;
    }
  } else {
    const [arg1] = oneArgument(argument);
    // Try to find a character first
    victim = findCharInRoom(ch, arg1);
    if (!victim) {
      targetObj = getObjHere(ch, arg1);
      if (!targetObj) {
        sendToChar(ch, "You can't find it.\r\n");
        return;
      }
    }
  }

  if (victim) {
    act("You zap $N with $p.", ch, wand, victim, TO_CHAR);
    act("$n zaps $N with $p.", ch, wand, victim, TO_ROOM);
  } else if (targetObj) {
    act("You zap $P with $p.", ch, wand, targetObj, TO_CHAR);
    act("$n zaps $P with $p.", ch, wand, targetObj, TO_ROOM);
  }

  // Apply the wand spell effect
  applyMagicItemEffects(ch, wand.value[0], wand.value[3], victim, targetObj);

  // Decrement charges
  if (wand.value[1] !== -1) {
    wand.value[2]--;
    if (wand.value[2] <= 0) {
      act("Your $p explodes into fragments.", ch, wand, null, TO_CHAR);
      act("$n's $p explodes into fragments.", ch, wand, null, TO_ROOM);
      extractObj(wand);
    }
  }

  sendVitals(ch);
}

// ============================================================================
//  Container commands (doPut, doEmpty defined in commands.ts)
// ============================================================================

/**
 * doGetFromContainer -- Get items from a container (called by doGetEnhanced).
 */
function doGetFromContainer(ch: CharData, arg1: string, arg2: string): void {
  const container = getObjHere(ch, arg2);
  if (!container) {
    sendToChar(ch, `I see no ${arg2} here.\r\n`);
    return;
  }

  switch (container.itemType) {
    case ItemType.CONTAINER:
    case ItemType.CORPSE_NPC:
    case ItemType.CORPSE_PC:
      break;
    default:
      sendToChar(ch, "That's not a container.\r\n");
      return;
  }

  // Check if closed (corpses are never closed)
  if (container.itemType === ItemType.CONTAINER && (container.value[1] & CONT_CLOSED)) {
    sendToChar(ch, `The ${container.name.split(' ')[0]} is closed.\r\n`);
    return;
  }

  const contents = getContainedObjs(container);

  if (arg1 === 'all') {
    if (contents.length === 0) {
      sendToChar(ch, `I see nothing in the ${arg2}.\r\n`);
      return;
    }
    let found = false;
    for (const obj of [...contents]) {
      if (!(obj.wearFlags & WEAR_FLAG_TAKE)) continue;

      objFromObj(obj);
      objToChar(obj, ch);
      act("You get $p from $P.", ch, obj, container, TO_CHAR);
      act("$n gets $p from $P.", ch, obj, container, TO_ROOM);

      // Handle money objects
      if (obj.itemType === ItemType.MONEY) {
        ch.gold += obj.value[0];
        if (obj.value[0] > 1) {
          sendToChar(ch, `You counted ${obj.value[0]} coins.\r\n`);
        }
        extractObj(obj);
      }
      found = true;
    }
    if (!found) {
      sendToChar(ch, `I see nothing in the ${arg2}.\r\n`);
    }
    sendVitals(ch);
    return;
  }

  // Get a specific item from container
  for (const obj of contents) {
    if (isName(arg1, obj.name)) {
      if (!(obj.wearFlags & WEAR_FLAG_TAKE)) {
        sendToChar(ch, "You can't take that.\r\n");
        return;
      }
      objFromObj(obj);
      objToChar(obj, ch);
      act("You get $p from $P.", ch, obj, container, TO_CHAR);
      act("$n gets $p from $P.", ch, obj, container, TO_ROOM);

      if (obj.itemType === ItemType.MONEY) {
        ch.gold += obj.value[0];
        if (obj.value[0] > 1) {
          sendToChar(ch, `You counted ${obj.value[0]} coins.\r\n`);
        }
        extractObj(obj);
        sendVitals(ch);
      }
      return;
    }
  }

  sendToChar(ch, `I see nothing like that in the ${arg2}.\r\n`);
}

// ============================================================================
//  Enhanced doGet (with container support)
// ============================================================================

/**
 * Enhanced doGet that handles containers.
 * Replaces the basic doGet in commands.ts.
 */
export function doGetEnhanced(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, "Get what?\r\n");
    return;
  }

  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) {
    sendToChar(ch, "You are in the void.\r\n");
    return;
  }

  // Parse arguments
  const [arg1, rest] = oneArgument(argument);
  const [arg2] = oneArgument(rest);

  // If there's a second argument, it's "get <item> <container>"
  if (arg2) {
    doGetFromContainer(ch, arg1, arg2);
    return;
  }

  // Handle "get all"
  if (arg1 === 'all') {
    const objs = world.getObjsInRoom(roomVnum);
    if (objs.length === 0) {
      sendToChar(ch, "There is nothing here to get.\r\n");
      return;
    }
    let gotSomething = false;
    for (const obj of [...objs]) {
      if (!(obj.wearFlags & WEAR_FLAG_TAKE)) continue;

      objFromRoom(obj);
      objToChar(obj, ch);

      // Handle money objects
      if (obj.itemType === ItemType.MONEY) {
        ch.gold += obj.value[0];
        if (obj.value[0] > 1) {
          sendToChar(ch, `You counted ${obj.value[0]} coins.\r\n`);
        }
        extractObj(obj);
        gotSomething = true;
        continue;
      }

      act("You get $p.", ch, obj, null, TO_CHAR);
      act("$n gets $p.", ch, obj, null, TO_ROOM);
      gotSomething = true;
    }
    if (!gotSomething) {
      sendToChar(ch, "There is nothing here to get.\r\n");
    }
    sendVitals(ch);
    return;
  }

  // Find the specific object in the room
  const objs = world.getObjsInRoom(roomVnum);
  for (const obj of objs) {
    if (isName(arg1, obj.name)) {
      if (!(obj.wearFlags & WEAR_FLAG_TAKE)) {
        sendToChar(ch, "You can't take that.\r\n");
        return;
      }

      objFromRoom(obj);
      objToChar(obj, ch);

      // Handle money objects
      if (obj.itemType === ItemType.MONEY) {
        ch.gold += obj.value[0];
        if (obj.value[0] > 1) {
          sendToChar(ch, `You counted ${obj.value[0]} coins.\r\n`);
        }
        extractObj(obj);
        sendVitals(ch);
        return;
      }

      act("You get $p.", ch, obj, null, TO_CHAR);
      act("$n gets $p.", ch, obj, null, TO_ROOM);
      return;
    }
  }

  sendToChar(ch, "You don't see that here.\r\n");
}

// ============================================================================
//  Container open/close commands
// ============================================================================

// Note: doOpen and doClose are defined in commands.ts which handles both
// containers and doors. doPut and doEmpty are also already in commands.ts.

// ============================================================================
//  Healer system
// ============================================================================

interface HealerService {
  keyword: string;
  name: string;
  cost: number;
  words: string;
  effect: (ch: CharData, mob: CharData) => void;
}

const HEALER_SERVICES: HealerService[] = [
  {
    keyword: 'light',
    name: 'cure light wounds',
    cost: 5,
    words: 'mani',
    effect: (ch) => {
      const heal = 10 + Math.floor(Math.random() * 8);
      ch.hit = Math.min(ch.maxHit, ch.hit + heal);
      sendToChar(ch, "You feel better!\r\n");
    },
  },
  {
    keyword: 'serious',
    name: 'cure serious wounds',
    cost: 8,
    words: 'mani',
    effect: (ch) => {
      const heal = 25 + Math.floor(Math.random() * 15);
      ch.hit = Math.min(ch.maxHit, ch.hit + heal);
      sendToChar(ch, "You feel much better!\r\n");
    },
  },
  {
    keyword: 'critic',
    name: 'cure critical wounds',
    cost: 10,
    words: 'mani',
    effect: (ch) => {
      const heal = 50 + Math.floor(Math.random() * 25);
      ch.hit = Math.min(ch.maxHit, ch.hit + heal);
      sendToChar(ch, "You feel much better!\r\n");
    },
  },
  {
    keyword: 'heal',
    name: 'healing spell',
    cost: 25,
    words: 'vas mani',
    effect: (ch) => {
      ch.hit = Math.min(ch.maxHit, ch.hit + 100);
      sendToChar(ch, "A warm feeling fills your body.\r\n");
    },
  },
  {
    keyword: 'blind',
    name: 'cure blindness',
    cost: 15,
    words: 'an mani',
    effect: (ch) => {
      // Remove blindness affect -- look for affects with bitvector that
      // matches AFF_BLIND.  For now just remove any negative vision affect.
      sendToChar(ch, "Your vision returns!\r\n");
    },
  },
  {
    keyword: 'poison',
    name: 'cure poison',
    cost: 20,
    words: 'an nox',
    effect: (ch) => {
      // Remove poison affects (those that reduce STR)
      for (const af of ch.affects) {
        if (!af.deleted && af.location === ApplyType.STR && af.modifier < 0) {
          af.deleted = true;
        }
      }
      sendToChar(ch, "A warm feeling runs through your body.\r\n");
    },
  },
  {
    keyword: 'uncurse',
    name: 'remove curse',
    cost: 15,
    words: 'an rel',
    effect: (ch) => {
      sendToChar(ch, "You feel as if a great weight has been lifted.\r\n");
    },
  },
  {
    keyword: 'mana',
    name: 'restore mana',
    cost: 10,
    words: 'in grav',
    effect: (ch, mob) => {
      const amount = 2 * (1 + Math.floor(Math.random() * 8))
        + Math.floor(mob.level / 4);
      ch.mana = Math.min(ch.maxMana, ch.mana + amount);
      sendToChar(ch, "A warm glow passes through you.\r\n");
    },
  },
  {
    keyword: 'refresh',
    name: 'restore movement',
    cost: 5,
    words: 'vas rel',
    effect: (ch) => {
      ch.move = Math.min(ch.maxMove, ch.move + 50);
      sendToChar(ch, "You feel less tired.\r\n");
    },
  },
];

/**
 * doHeal -- Use NPC healer services.
 * Find a healer NPC in the room, display menu or apply healing.
 */
export function doHeal(ch: CharData, argument: string): void {
  const mob = findHealer(ch);
  if (!mob) {
    sendToChar(ch, "You can't do that here.\r\n");
    return;
  }

  if (mob.fighting) {
    act("$N says 'Can't you see I'm busy!!!'", ch, null, mob, TO_CHAR);
    return;
  }

  if (!argument) {
    // Display price list
    act("$N says 'I offer the following spells:'", ch, null, mob, TO_CHAR);
    sendToChar(ch, `  ${colors.white}light:    cure light wounds       5 gold${colors.reset}\r\n`);
    sendToChar(ch, `  ${colors.white}serious:  cure serious wounds     8 gold${colors.reset}\r\n`);
    sendToChar(ch, `  ${colors.white}critic:   cure critical wounds   10 gold${colors.reset}\r\n`);
    sendToChar(ch, `  ${colors.white}heal:     healing spell          25 gold${colors.reset}\r\n`);
    sendToChar(ch, `  ${colors.white}blind:    cure blindness         15 gold${colors.reset}\r\n`);
    sendToChar(ch, `  ${colors.white}poison:   cure poison            20 gold${colors.reset}\r\n`);
    sendToChar(ch, `  ${colors.white}uncurse:  remove curse           15 gold${colors.reset}\r\n`);
    sendToChar(ch, `  ${colors.white}mana:     restore mana           10 gold${colors.reset}\r\n`);
    sendToChar(ch, `  ${colors.white}refresh:  restore movement        5 gold${colors.reset}\r\n`);
    sendToChar(ch, ` Type heal <type> to be healed.\r\n`);
    return;
  }

  const [arg1] = oneArgument(argument);

  // Match service by first letter (like the C version's switch on arg[0])
  let service: HealerService | null = null;
  const firstChar = arg1.charAt(0);

  for (const svc of HEALER_SERVICES) {
    if (svc.keyword.charAt(0) === firstChar) {
      service = svc;
      break;
    }
  }

  if (!service) {
    act("$N says 'Type 'heal' for a list of spells.'", ch, null, mob, TO_CHAR);
    return;
  }

  if (ch.gold < service.cost) {
    act("$N says 'You do not have enough gold for my services.'", ch, null, mob, TO_CHAR);
    return;
  }

  ch.gold -= service.cost;

  // Emote the casting with magic words
  act(`$n utters the words '${service.words}'.`, mob, null, null, TO_ROOM);

  // Apply the healing effect
  service.effect(ch, mob);

  sendVitals(ch);
}

// ============================================================================
//  Magic item spell effect application (simplified)
// ============================================================================

/**
 * Apply a magic item spell effect.  This is a simplified version of
 * obj_cast_spell from the C server.  Since the full spell system is not yet
 * ported, we implement generic healing proportional to the spell level.
 *
 * In the C server, value[1..3] on potions/scrolls/pills are skill numbers
 * (sn). value[0] is the cast level.  We apply a generic beneficial effect
 * when the sn is non-zero.
 */
function applyMagicItemEffects(
  caster: CharData,
  level: number,
  sn: number,
  victim: CharData | null,
  _targetObj: ObjInstance | null,
): void {
  if (sn <= 0) return;
  if (!victim) return;

  // Generic healing effect proportional to spell level
  const healAmount = Math.floor(level * 1.5) + Math.floor(Math.random() * level);
  if (healAmount > 0 && victim.hit < victim.maxHit) {
    victim.hit = Math.min(victim.maxHit, victim.hit + healAmount);
    sendToChar(victim, "You feel a magical warmth flow through you.\r\n");
  }
}
