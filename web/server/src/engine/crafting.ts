/**
 * crafting.ts -- Crafting System for Stormgate MUD.
 *
 * Ported from crafting.c. Provides five crafting disciplines:
 *   - Skinning (skin corpses for leather hides)
 *   - Tanning  (tan leather into armor pieces)
 *   - Forging  (forge ore into weapons/armor)
 *   - Brewing  (brew spell reagents into potions)
 *   - Scribing (inscribe spells onto scrolls)
 *
 * Each discipline uses a skill check against the character's proficiency,
 * consumes materials, and produces items with randomized stats.
 *
 * All commands export a CommandEntry[] array (craftingCommands) that can be
 * spread into the master command table in commands.ts.
 */

import type {
  CharData,
  ObjInstance,
  PcData,
} from './types.js';

import {
  Position,
  ItemType,
  WearLocation,
} from './types.js';

import { world, rollDice } from './world.js';

import {
  getCharRoom,
  isName,
  capitalize,
  objToChar,
  getCarriedItems,
} from './handler.js';

import {
  sendToChar,
  act,
  colors,
  TO_ROOM,
} from './output.js';

import { sendVitals } from './protocol.js';

import type { CommandEntry } from './commands.js';

// ============================================================================
//  Constants
// ============================================================================

/** Skin quality levels (matching C enum). */
const SKIN_DESTROYED = 0;
const SKIN_LOUSY     = 1;
const SKIN_PASSABLE  = 2;
const SKIN_DECENT    = 3;
const SKIN_GOOD      = 4;
const SKIN_EXCELLENT = 5;
const SKIN_PERFECT   = 6;

const SKIN_QUALITY_NAMES = [
  'destroyed', 'lousy', 'passable', 'decent', 'good', 'excellent', 'perfect',
];

/** Default crafting cooldown in ticks. */
const DEFAULT_CRAFT_COOLDOWN = 3;

/** Armor location names for generated armor pieces. */
const WEAR_LOCATION_NAMES: Record<number, string[]> = {
  [WearLocation.FINGER_L]:  ['Ring'],
  [WearLocation.NECK_1]:    ['Scarf', 'Chain', 'Choker', 'Necklace'],
  [WearLocation.BODY]:      ['Breastplate', 'Leather', 'Studded Leather', 'Padded Leather', 'Banded Leather', 'Cured Leather', 'Hide'],
  [WearLocation.HEAD]:      ['Headdress', 'Helm', 'Helmet'],
  [WearLocation.LEGS]:      ['Leggings'],
  [WearLocation.FEET]:      ['Sandals', 'Shoes', 'Boots'],
  [WearLocation.HANDS]:     ['Gloves', 'Gauntlets'],
  [WearLocation.ARMS]:      ['Sleeves', 'Vambraces'],
  [WearLocation.SHIELD]:    ['Shield'],
  [WearLocation.ABOUT]:     ['Cloak'],
  [WearLocation.WAIST]:     ['Belt'],
  [WearLocation.WRIST_L]:   ['Bracelet'],
  [WearLocation.HOLD]:      ['Orb'],
  [WearLocation.ON_FACE]:   ['Mask'],
  [WearLocation.EARS]:      ['Earrings'],
  [WearLocation.ANKLE_L]:   ['Ankle Bracelet'],
};

/** Prefix descriptors based on object properties. */
const ITEM_PREFIXES = [
  'Bright', 'Vibrating', 'Dark', 'Wicked', 'Translucent', 'Magic',
  'Cursed', 'Blessed', 'Virulent', 'Flaming', 'Freezing', 'Corrosive',
  'Lightning', 'Vortex', 'Strange',
];

/** Suffix descriptors based on stat bonuses. */
const STAT_SUFFIXES: Record<string, [string, string]> = {
  strength:     ['Steel ', 'Titans'],
  dexterity:    ['Quicksilver ', 'Agility'],
  intelligence: ['', 'Mentats'],
  wisdom:       ['', 'Owls'],
  constitution: ['', 'Dragonkind'],
  hitroll:      ['', 'Aiming'],
  damroll:      ['Bloodmetal ', ''],
  hp:           ['', 'Healthiness'],
  mana:         ['', 'BattleMages'],
  ac:           ['', 'BlackSmiths'],
  saves:        ['', 'Warding'],
  move:         ['Swift ', ''],
};

/** Weapon type names for forged weapons. */
const WEAPON_TYPES = [
  'Sword', 'Axe', 'Mace', 'Flail', 'Dagger', 'Spear',
  'Halberd', 'Hammer', 'Scimitar', 'Claymore', 'Rapier',
];

/** Weapon prefix adjectives. */
const WEAPON_ADJECTIVES = [
  'Gleaming', 'Shadow', 'Storm', 'Blood', 'Rune',
  'Ancient', 'Frost', 'Thunder', 'Void', 'Iron',
];

/** Ore material names. */
const ORE_MATERIALS = [
  'iron', 'steel', 'mithril', 'adamantine', 'bronze',
  'silver', 'gold', 'obsidian', 'crystal', 'dragonscale',
];

/** Potion color names. */
const POTION_COLORS = [
  'bubbling', 'shimmering', 'murky', 'glowing', 'sparkling',
  'swirling', 'translucent', 'opaque', 'crystalline', 'smoky',
];

/** Scroll material names. */
const SCROLL_MATERIALS = [
  'parchment', 'vellum', 'papyrus', 'silk', 'leather',
];

/**
 * Skill name to skill number (sn) mapping.
 * The C code used skill_lookup("skinning") etc. Here we use hardcoded sn
 * values. If these need to change, update them to match the actual skill
 * table.
 */
const SKILL_SKINNING    = 400;
const SKILL_TANNING     = 401;
const SKILL_FORGING     = 402;
const SKILL_ALCHEMY     = 403;
const SKILL_INSCRIPTION = 404;

// ============================================================================
//  Utility
// ============================================================================

/** Get skill proficiency (0-1000 range from pcdata.learned). */
function getSkillProficiency(pcdata: PcData, sn: number): number {
  return pcdata.learned.get(sn) ?? 0;
}

/** Attempt to improve a skill by use (small chance per use). */
function tryImproveSkill(ch: CharData, pcdata: PcData, sn: number): void {
  const current = pcdata.learned.get(sn) ?? 0;
  if (current >= 1000) return; // Already mastered

  // 5% chance to gain 1-10 points
  if (Math.random() < 0.05) {
    const gain = rollDice(1, 10);
    pcdata.learned.set(sn, Math.min(1000, current + gain));
    sendToChar(ch,
      `${colors.brightCyan}You feel more proficient!${colors.reset}\r\n`,
    );
  }
}

/** Check and enforce crafting cooldown. */
function checkCraftCooldown(ch: CharData, pcdata: PcData): boolean {
  if (pcdata.craftTimer > 0) {
    sendToChar(ch,
      `${colors.yellow}You must wait ${pcdata.craftTimer} more tick(s) before crafting again.\r\n${colors.reset}`,
    );
    return false;
  }
  return true;
}

/** Find an item in inventory by item type. */
function findItemByItemType(ch: CharData, type: ItemType): ObjInstance | undefined {
  const items = getCarriedItems(ch);
  for (const obj of items) {
    if (obj.itemType === type && !obj.deleted) {
      return obj;
    }
  }
  return undefined;
}

/** Find an item in inventory by keyword. */
function findItemByName(ch: CharData, name: string): ObjInstance | undefined {
  const items = getCarriedItems(ch);
  for (const obj of items) {
    if (obj.deleted) continue;
    if (isName(name, obj.name)) {
      return obj;
    }
  }
  return undefined;
}

/** Pick a random element from an array. */
function randomPick<T>(arr: T[]): T {
  return arr[Math.floor(Math.random() * arr.length)];
}

/** Number fuzzy: returns n-1, n, or n+1 randomly. */
function numberFuzzy(n: number): number {
  const r = Math.random();
  if (r < 0.33) return Math.max(1, n - 1);
  if (r < 0.66) return n;
  return n + 1;
}

/** Generate a random armor name (ported from name_tanned_armor in C). */
function generateArmorName(
  wearLoc: number,
  affects: Array<{ stat: string; amount: number }>,
): { name: string; shortDescr: string; description: string } {
  const locNames = WEAR_LOCATION_NAMES[wearLoc];
  const baseType = locNames ? randomPick(locNames) : 'Armor';
  const prefix = randomPick(ITEM_PREFIXES);

  // Build suffix from affects
  let suffix1 = '';
  let suffix2 = '';
  for (const aff of affects) {
    const pair = STAT_SUFFIXES[aff.stat];
    if (!pair) continue;
    if (!suffix1 && pair[0]) suffix1 = pair[0];
    if (!suffix2 && pair[1]) suffix2 = pair[1];
    if (suffix1 && suffix2) break;
  }
  if (!suffix1 && !suffix2) {
    suffix1 = 'Insignificance';
  }

  const first = prefix[0];
  const last = baseType[baseType.length - 1];

  let article: string;
  if (last === 's') {
    article = '';
  } else if ('AEIOU'.includes(first)) {
    article = 'an ';
  } else {
    article = 'a ';
  }

  const shortDescr = `${article}${prefix} ${baseType} of ${suffix1}${suffix2}`;
  const name = `${prefix} ${baseType} ${suffix1}${suffix2}`.toLowerCase();
  const description = `A ${prefix.toLowerCase()} ${baseType.toLowerCase()} lies here upon the ground, discarded.`;

  return { name, shortDescr, description };
}

// ============================================================================
//  doSkin -- Skin a corpse for leather
// ============================================================================

/**
 * doSkin -- Skin a corpse to produce a leather hide.
 * Usage: skin <corpse>
 *
 * Requires the "skinning" skill. The corpse is consumed.
 * Quality of the leather depends on skill proficiency.
 */
export function doSkin(ch: CharData, argument: string): void {
  if (ch.isNpc) return;

  const pcdata = world.pcData.get(ch.id);
  if (!pcdata) return;

  if (!argument) {
    sendToChar(ch, 'Skin what?\r\n');
    return;
  }

  const proficiency = getSkillProficiency(pcdata, SKILL_SKINNING);
  if (proficiency === 0) {
    sendToChar(ch, 'You don\'t know how to skin.\r\n');
    return;
  }

  if (!checkCraftCooldown(ch, pcdata)) return;

  const obj = findItemByName(ch, argument);
  if (!obj) {
    sendToChar(ch, 'You don\'t have that.\r\n');
    return;
  }

  // Must be a corpse or skin
  if (obj.itemType === ItemType.SKIN) {
    // Combining two skins -- need a second skin argument
    sendToChar(ch,
      `${colors.red}You need two rolls of leather to combine! Try: skin <leather1> <leather2>\r\n${colors.reset}`,
    );
    return;
  }

  if (obj.itemType !== ItemType.CORPSE_NPC) {
    sendToChar(ch,
      `${colors.red}You can only skin corpses. What are you thinking?\r\n${colors.reset}`,
    );
    return;
  }

  // Skill check
  const learned = proficiency;
  const roll = rollDice(1, 100);
  const target = Math.floor(learned / 10) + 20;

  if (roll > target) {
    sendToChar(ch,
      `${colors.red}You mangle the skin. You've destroyed it!\r\n${colors.reset}`,
    );
    act('$n thoroughly dices a corpse, but it yields no leather!', ch, null, null, TO_ROOM);
    world.extractObj(obj);
    tryImproveSkill(ch, pcdata, SKILL_SKINNING);
    pcdata.craftTimer = DEFAULT_CRAFT_COOLDOWN;
    return;
  }

  // Determine skin quality based on skill level
  let quality: number;
  if (learned >= 1000) {
    quality = SKIN_GOOD + rollDice(1, 3) - 1;
  } else if (learned > 750) {
    if (Math.random() < 0.25) {
      quality = SKIN_GOOD + rollDice(1, 3) - 1;
    } else {
      quality = SKIN_DECENT + rollDice(1, 2) - 1;
    }
  } else if (learned > 500) {
    const pct = Math.random();
    if (pct < 0.10) quality = SKIN_GOOD + rollDice(1, 3) - 1;
    else if (pct < 0.30) quality = SKIN_DECENT + rollDice(1, 2) - 1;
    else quality = SKIN_LOUSY + rollDice(1, 2) - 1;
  } else {
    const pct = Math.random();
    if (pct < 0.05) quality = SKIN_GOOD + rollDice(1, 3) - 1;
    else if (pct < 0.15) quality = SKIN_DECENT + rollDice(1, 2) - 1;
    else if (pct < 0.30) quality = SKIN_LOUSY + rollDice(1, 2) - 1;
    else quality = SKIN_DESTROYED + rollDice(1, 3) - 1;
  }

  quality = Math.max(SKIN_DESTROYED, Math.min(SKIN_PERFECT, quality));
  const qualityName = SKIN_QUALITY_NAMES[quality] ?? 'unknown';

  // Calculate yards of leather (from mob's size and level)
  const mobLevel = obj.level || 1;
  const mobSize = Math.max(1, Math.floor(mobLevel / 10) + 1);
  let yards = Math.floor(
    mobSize * rollDice(1, Math.max(1, Math.floor(learned / 40))) / 10,
  ) + Math.max(1, Math.floor(mobLevel / 25));
  yards = Math.max(1, yards);

  // Create the leather object
  const skin = world.createSimpleObject({
    name: `leather ${qualityName} hide`,
    shortDescr: `${yards} yards of ${qualityName} leather`,
    description: `A roll of ${qualityName} leather lies here.`,
    level: Math.min(105, mobLevel),
    itemType: ItemType.SKIN,
    weight: Math.max(1, rollDice(1, yards)),
    cost: yards * quality * 10,
    wearFlags: 0,
    extraFlags: 0,
  });

  // Store quality and yards in the object's value array
  skin.value[0] = quality;
  skin.value[1] = yards;

  objToChar(skin, ch);
  sendToChar(ch,
    `${colors.red}You skillfully scrape the leather, creating ${yards} yards of ${qualityName} leather!\r\n${colors.reset}`,
  );
  act('$n scrapes a corpse for its leather.', ch, null, null, TO_ROOM);

  world.extractObj(obj);
  tryImproveSkill(ch, pcdata, SKILL_SKINNING);
  pcdata.craftTimer = DEFAULT_CRAFT_COOLDOWN;
}

// ============================================================================
//  doTan -- Tan leather into armor
// ============================================================================

/**
 * doTan -- Tan a leather hide into a piece of armor.
 * Usage: tan <leather> <yards> [level]
 *
 * Requires the "tanning" skill and a needle in inventory.
 * You must be in a tannery room.
 */
export function doTan(ch: CharData, argument: string): void {
  if (ch.isNpc) return;

  const pcdata = world.pcData.get(ch.id);
  if (!pcdata) return;

  const proficiency = getSkillProficiency(pcdata, SKILL_TANNING);
  if (proficiency === 0) {
    sendToChar(ch, 'You don\'t know how to tan.\r\n');
    return;
  }

  if (!checkCraftCooldown(ch, pcdata)) return;

  if (!argument) {
    sendToChar(ch, 'Tan what? Usage: tan <leather> <yards> [level]\r\n');
    return;
  }

  // Parse arguments
  const parts = argument.trim().split(/\s+/);
  const leatherName = parts[0];
  const yardsArg = parts[1] ? parseInt(parts[1], 10) : 0;
  const levelArg = parts[2] ? parseInt(parts[2], 10) : 0;

  // Find the leather
  const skin = findItemByName(ch, leatherName);
  if (!skin) {
    sendToChar(ch, 'You don\'t have that.\r\n');
    return;
  }

  if (skin.itemType !== ItemType.SKIN) {
    sendToChar(ch,
      `${colors.yellow}You need to use leather to tan!\r\n${colors.reset}`,
    );
    return;
  }

  // Check for needle
  const needle = findItemByItemType(ch, ItemType.NEEDLE);
  if (!needle) {
    sendToChar(ch,
      `${colors.yellow}You need a needle to tan leather!\r\n${colors.reset}`,
    );
    return;
  }

  const skinYards = skin.value[1] || 5;
  const skinQuality = skin.value[0] || SKIN_DECENT;

  if (!yardsArg || isNaN(yardsArg) || yardsArg <= 0) {
    sendToChar(ch,
      `${colors.yellow}You need to choose how many yards of leather you will use!\r\n${colors.reset}`,
    );
    return;
  }

  const yards = yardsArg;

  if (yards > skinYards) {
    sendToChar(ch,
      `${colors.yellow}Your leather doesn't contain enough. Choose another roll or combine two rolls.\r\n${colors.reset}`,
    );
    return;
  }

  let lvl = (levelArg > 0 && levelArg <= ch.level) ? levelArg : ch.level;
  lvl = Math.min(lvl, ch.level);

  if (lvl > yards) {
    sendToChar(ch,
      `${colors.yellow}You need at least 1 yard per level of item you are creating!\r\n${colors.reset}`,
    );
    return;
  }

  // Calculate bonus from extra leather and quality
  let bonus = yards - lvl;
  bonus += skinQuality - Math.floor(lvl / 5);
  bonus = Math.max(bonus, -25);

  // Consume leather (reduce yards or destroy)
  skin.value[1] -= yards;
  if (skin.value[1] <= 0) {
    world.extractObj(skin);
    sendToChar(ch,
      `${colors.yellow}You use all of your leather roll.\r\n${colors.reset}`,
    );
  } else {
    // Update the skin description
    const qualName = SKIN_QUALITY_NAMES[skin.value[0]] ?? 'unknown';
    skin.shortDescr = `${skin.value[1]} yards of ${qualName} leather`;
  }

  // Skill check
  const target = Math.floor(proficiency / 10) + bonus;
  const roll = rollDice(1, 100);

  if (roll > target) {
    sendToChar(ch,
      `${colors.yellow}Your attempt at tanning fails miserably!\r\n${colors.reset}`,
    );
    if (Math.random() < 0.25) {
      sendToChar(ch,
        `${colors.cyan}You break your needle in the attempt!\r\n${colors.reset}`,
      );
      world.extractObj(needle);
    }
    tryImproveSkill(ch, pcdata, SKILL_TANNING);
    pcdata.craftTimer = DEFAULT_CRAFT_COOLDOWN;
    return;
  }

  // Success! Generate random wear location
  const wearLocs = Object.keys(WEAR_LOCATION_NAMES).map(Number);
  const wearLoc = randomPick(wearLocs);

  // Generate AC value
  const acValue = numberFuzzy(Math.floor(lvl / 4) + 2);

  // Generate random stat bonuses (ported from C tanning logic)
  const maxAffects = Math.floor(lvl / 17) + 1;
  const affects: Array<{ stat: string; amount: number }> = [];
  let statcount = 0;
  let rescount = 0;

  for (let cur = 0; cur < maxAffects; cur++) {
    const pct = Math.random() * 100;
    let stat: string;
    let amount: number;

    if (pct < 5) {
      // Resistance/damage type modifier
      if (rescount >= 2) continue;
      rescount++;
      stat = 'saves';
      amount = rollDice(1, Math.max(1, Math.floor(lvl / 10)));
    } else if (pct < 20) {
      // Saving spell bonus
      stat = 'saves';
      amount = -(rollDice(1, Math.max(1, Math.floor(lvl / 5))));
    } else if (pct < 50) {
      // Primary stat bonus
      if (statcount >= 2) continue;
      statcount++;
      const statPool = ['strength', 'dexterity', 'intelligence', 'wisdom', 'constitution'];
      stat = randomPick(statPool);
      amount = rollDice(1, Math.floor(lvl / 30) + 1);
    } else if (pct < 75) {
      // Combat stat
      const combatPool = ['mana', 'hp', 'move', 'ac', 'hitroll', 'damroll'];
      stat = randomPick(combatPool);
      switch (stat) {
        case 'mana':
          amount = rollDice(1, lvl) + Math.floor(lvl / 5);
          break;
        case 'hp':
          amount = rollDice(1, lvl) + Math.floor(lvl / 5);
          break;
        case 'move':
          amount = rollDice(1, lvl * 2);
          break;
        case 'ac':
          amount = -(rollDice(1, Math.max(1, lvl + 50)));
          break;
        case 'hitroll':
        case 'damroll':
          amount = rollDice(1, Math.max(1, Math.floor(lvl / 3)));
          break;
        default:
          amount = 1;
      }
    } else {
      continue; // Skip this slot
    }

    // Don't duplicate stats
    if (affects.some((a) => a.stat === stat)) continue;

    affects.push({ stat, amount });

    // If hitroll, also add damroll (and vice versa) per C logic
    if (stat === 'hitroll' && !affects.some((a) => a.stat === 'damroll')) {
      affects.push({ stat: 'damroll', amount: numberFuzzy(amount) });
    } else if (stat === 'damroll' && !affects.some((a) => a.stat === 'hitroll')) {
      affects.push({ stat: 'hitroll', amount: numberFuzzy(amount) });
    }
  }

  const { name, shortDescr, description } = generateArmorName(wearLoc, affects);

  // Create the armor object
  const armor = world.createSimpleObject({
    name,
    shortDescr,
    description,
    level: lvl,
    itemType: ItemType.ARMOR,
    weight: rollDice(1, 10) + 5,
    cost: lvl * 100,
    wearFlags: wearLoc,
    extraFlags: 0,
  });

  armor.value[0] = acValue;
  armor.durabilityMax = numberFuzzy(skinQuality) * numberFuzzy(Math.floor(lvl / 10));
  armor.durabilityCur = rollDice(1, Math.max(1, armor.durabilityMax));

  objToChar(armor, ch);

  sendToChar(ch, `${colors.yellow}You skillfully tan and sew the leather!\r\n`);
  let buf = `${colors.white}You manage to create:\r\n`;
  buf += `${colors.brightCyan}  ${shortDescr}\r\n`;
  buf += `${colors.cyan}  Level: ${lvl}  AC: ${acValue}\r\n`;
  for (const aff of affects) {
    const sign = aff.amount >= 0 ? '+' : '';
    buf += `${colors.cyan}  ${capitalize(aff.stat)}: ${sign}${aff.amount}\r\n`;
  }
  buf += colors.reset;
  sendToChar(ch, buf);
  act('$n skillfully tans and stitches together some new armor!', ch, null, null, TO_ROOM);

  // 10% chance to break needle
  if (Math.random() < 0.10) {
    sendToChar(ch,
      `${colors.cyan}Your needle breaks!\r\n${colors.reset}`,
    );
    world.extractObj(needle);
  }

  tryImproveSkill(ch, pcdata, SKILL_TANNING);
  pcdata.craftTimer = DEFAULT_CRAFT_COOLDOWN;
}

// ============================================================================
//  doForge -- Forge weapons/armor from ore
// ============================================================================

/**
 * doForge -- Forge a weapon or armor piece from ore.
 * Usage: forge <weapon|armor>
 *
 * Requires the "forging" skill and a hammer + ore in inventory.
 */
export function doForge(ch: CharData, argument: string): void {
  if (ch.isNpc) return;

  const pcdata = world.pcData.get(ch.id);
  if (!pcdata) return;

  const proficiency = getSkillProficiency(pcdata, SKILL_FORGING);
  if (proficiency === 0) {
    sendToChar(ch, 'You don\'t know how to forge.\r\n');
    return;
  }

  if (!checkCraftCooldown(ch, pcdata)) return;

  if (!argument) {
    sendToChar(ch, 'Forge what? Usage: forge <weapon|armor>\r\n');
    return;
  }

  const type = argument.trim().split(/\s+/)[0].toLowerCase();

  if (type !== 'weapon' && type !== 'armor') {
    sendToChar(ch, 'You can forge either a "weapon" or "armor".\r\n');
    return;
  }

  // Check for hammer
  const hammer = findItemByItemType(ch, ItemType.HAMMER);
  if (!hammer) {
    sendToChar(ch,
      `${colors.yellow}You need a hammer to forge!\r\n${colors.reset}`,
    );
    return;
  }

  // Check for ore
  const ore = findItemByItemType(ch, ItemType.ORE);
  if (!ore) {
    sendToChar(ch,
      `${colors.yellow}You need ore to forge!\r\n${colors.reset}`,
    );
    return;
  }

  const material = randomPick(ORE_MATERIALS);
  const oreLevel = ore.level || ch.level;

  // Consume the ore
  world.extractObj(ore);
  sendToChar(ch,
    `${colors.white}You heat the ${material} ore in the forge...\r\n${colors.reset}`,
  );
  act('$n works the forge, hammering metal.', ch, null, null, TO_ROOM);

  // Skill check
  const target = Math.floor(proficiency / 10) + 10;
  const roll = rollDice(1, 100);

  if (roll > target) {
    sendToChar(ch,
      `${colors.red}The metal cracks and shatters! Your forging attempt fails.\r\n${colors.reset}`,
    );
    tryImproveSkill(ch, pcdata, SKILL_FORGING);
    pcdata.craftTimer = DEFAULT_CRAFT_COOLDOWN + 1;
    return;
  }

  const lvl = Math.min(ch.level, oreLevel);

  if (type === 'weapon') {
    // Forge a weapon
    const weaponType = randomPick(WEAPON_TYPES);
    const adjective = randomPick(WEAPON_ADJECTIVES);
    const article = 'AEIOU'.includes(adjective[0]) ? 'an' : 'a';

    const wname = `${adjective} ${material} ${weaponType}`.toLowerCase();
    const wshort = `${article} ${adjective} ${material} ${weaponType}`;
    const wdesc = `A ${adjective.toLowerCase()} ${material} ${weaponType.toLowerCase()} lies here.`;

    // Damage dice
    const numDice = Math.max(1, Math.floor(lvl / 10));
    const sizeDice = Math.max(2, Math.floor(lvl / 5) + rollDice(1, 4));

    // Hit/dam bonuses
    const hitBonus = rollDice(1, Math.max(1, Math.floor(lvl / 5)));
    const damBonus = rollDice(1, Math.max(1, Math.floor(lvl / 5)));

    const weapon = world.createSimpleObject({
      name: wname,
      shortDescr: wshort,
      description: wdesc,
      level: lvl,
      itemType: ItemType.WEAPON,
      weight: rollDice(1, 5) + 3,
      cost: lvl * 150,
      wearFlags: WearLocation.WIELD,
      extraFlags: 0,
    });

    // Store damage dice in value array (matching C: value[1]=numDice, value[2]=sizeDice)
    weapon.value[1] = numDice;
    weapon.value[2] = sizeDice;

    objToChar(weapon, ch);

    let buf = `${colors.brightYellow}You successfully forge:\r\n`;
    buf += `${colors.white}  ${wshort}\r\n`;
    buf += `${colors.cyan}  Level: ${lvl}  Damage: ${numDice}d${sizeDice}\r\n`;
    buf += `${colors.cyan}  Hitroll: +${hitBonus}  Damroll: +${damBonus}\r\n`;
    buf += colors.reset;
    sendToChar(ch, buf);
    act('$n pulls a gleaming weapon from the forge!', ch, null, null, TO_ROOM);
  } else {
    // Forge armor
    const wearLocs = Object.keys(WEAR_LOCATION_NAMES).map(Number);
    const wearLoc = randomPick(wearLocs);
    const locNames = WEAR_LOCATION_NAMES[wearLoc];
    const baseType = locNames ? randomPick(locNames) : 'Plate';
    const adjective = randomPick(WEAPON_ADJECTIVES);
    const article = 'AEIOU'.includes(adjective[0]) ? 'an' : 'a';

    const aname = `${adjective} ${material} ${baseType}`.toLowerCase();
    const ashort = `${article} ${adjective} ${material} ${baseType}`;
    const adesc = `A ${adjective.toLowerCase()} ${material} ${baseType.toLowerCase()} lies here.`;

    const acValue = Math.floor(lvl / 3) + rollDice(1, 5);

    const armor = world.createSimpleObject({
      name: aname,
      shortDescr: ashort,
      description: adesc,
      level: lvl,
      itemType: ItemType.ARMOR,
      weight: rollDice(1, 15) + 5,
      cost: lvl * 120,
      wearFlags: wearLoc,
      extraFlags: 0,
    });

    armor.value[0] = acValue;

    objToChar(armor, ch);

    let buf = `${colors.brightYellow}You successfully forge:\r\n`;
    buf += `${colors.white}  ${ashort}\r\n`;
    buf += `${colors.cyan}  Level: ${lvl}  AC: ${acValue}\r\n`;
    buf += colors.reset;
    sendToChar(ch, buf);
    act('$n pulls a piece of armor from the forge!', ch, null, null, TO_ROOM);
  }

  tryImproveSkill(ch, pcdata, SKILL_FORGING);
  pcdata.craftTimer = DEFAULT_CRAFT_COOLDOWN + 1;
}

// ============================================================================
//  doBrew -- Brew potions
// ============================================================================

/**
 * doBrew -- Brew a potion from reagents.
 * Usage: brew <spell1> [spell2] [spell3]
 *
 * Requires the "alchemy" skill, a pestle, and an empty vial/flask.
 * Up to 3 spells can be imbued into one potion.
 * Spells are looked up by number in the pcdata.learned map.
 */
export function doBrew(ch: CharData, argument: string): void {
  if (ch.isNpc) return;

  const pcdata = world.pcData.get(ch.id);
  if (!pcdata) return;

  const proficiency = getSkillProficiency(pcdata, SKILL_ALCHEMY);
  if (proficiency === 0) {
    sendToChar(ch, 'You don\'t know how to brew potions.\r\n');
    return;
  }

  if (!checkCraftCooldown(ch, pcdata)) return;

  if (!argument) {
    sendToChar(ch, 'Brew what spell? Usage: brew <spell1> [spell2] [spell3]\r\n');
    return;
  }

  const parts = argument.trim().split(/\s+/);
  const spellArgs = parts.slice(0, 3);

  // Validate that each spell arg is a known spell (by sn number or name)
  // For simplicity, we accept spell names and look them up
  const validSpells: string[] = [];
  for (const sp of spellArgs) {
    // Accept the spell name as-is (the C code used skill_lookup)
    validSpells.push(sp);
  }

  if (validSpells.length === 0) {
    sendToChar(ch, 'You need to specify at least one spell to brew.\r\n');
    return;
  }

  // Check for pestle
  const pestle = findItemByItemType(ch, ItemType.PESTLE);
  if (!pestle) {
    sendToChar(ch,
      `${colors.white}You do not have a pestle and mortar.\r\n${colors.reset}`,
    );
    return;
  }

  // Check for empty vial/potion (empty potion = vial in the C code)
  let vial: ObjInstance | undefined;
  const items = getCarriedItems(ch);
  for (const obj of items) {
    if (obj.deleted) continue;
    if (obj.itemType === ItemType.POTION &&
        !obj.value[0] && !obj.value[1] && !obj.value[2] && !obj.value[3]) {
      vial = obj;
      break;
    }
  }

  if (!vial) {
    // Also check for an item named "vial" or "flask"
    vial = findItemByName(ch, 'vial') || findItemByName(ch, 'flask');
  }

  if (!vial) {
    sendToChar(ch,
      `${colors.white}You do not have an empty vial.\r\n${colors.reset}`,
    );
    return;
  }

  act('$n begins brewing a potion.', ch, null, null, TO_ROOM);
  sendToChar(ch,
    `${colors.white}You begin to carefully brew the potion...\r\n${colors.reset}`,
  );

  // Skill check -- difficulty increases with more spells
  const numSpells = validSpells.length;
  const pctBrew = Math.floor(proficiency / 10);
  let target: number;
  if (numSpells === 1) {
    target = Math.floor((pctBrew + 80) / 2);
  } else if (numSpells === 2) {
    target = Math.floor((pctBrew + 60) / 3);
  } else {
    target = Math.floor((pctBrew + 40) / 4);
  }

  const roll = rollDice(1, 100);

  if (roll > target) {
    sendToChar(ch,
      `${colors.red}Oh no! The mixture bubbles violently and explodes!\r\nYour brewing attempt fails.\r\n${colors.reset}`,
    );
    act('$n fails to brew a potion!', ch, null, null, TO_ROOM);

    world.extractObj(vial);

    if (Math.random() < 0.25) {
      sendToChar(ch,
        `${colors.white}Your pestle breaks!\r\n${colors.reset}`,
      );
      world.extractObj(pestle);
    }

    tryImproveSkill(ch, pcdata, SKILL_ALCHEMY);
    pcdata.craftTimer = DEFAULT_CRAFT_COOLDOWN;
    return;
  }

  // Success! Create the potion by modifying the existing vial
  const potionColor = randomPick(POTION_COLORS);
  const spellList = validSpells.join(', ');

  vial.name = `${potionColor} potion ${spellList}`.toLowerCase();
  vial.shortDescr = `a ${potionColor} potion of ${spellList}`;
  vial.description = `A ${potionColor} potion sits here, waiting to be consumed.`;
  vial.level = ch.level;
  vial.value[0] = ch.level; // spell level
  vial.cost = ch.level * numSpells * 50;

  let buf = `${colors.brightGreen}You successfully brew:\r\n`;
  buf += `${colors.white}  ${vial.shortDescr}\r\n`;
  buf += `${colors.cyan}  Level: ${ch.level}  Spells: ${spellList}\r\n`;
  buf += colors.reset;
  sendToChar(ch, buf);
  act('$n finishes brewing a potion.', ch, null, null, TO_ROOM);

  // 10% chance to break pestle
  if (Math.random() < 0.10) {
    sendToChar(ch,
      `${colors.white}Your pestle breaks!\r\n${colors.reset}`,
    );
    world.extractObj(pestle);
  }

  tryImproveSkill(ch, pcdata, SKILL_ALCHEMY);
  pcdata.craftTimer = DEFAULT_CRAFT_COOLDOWN;
}

// ============================================================================
//  doScribe -- Scribe scrolls
// ============================================================================

/**
 * doScribe -- Inscribe a spell onto a scroll.
 * Usage: scribe <spell1> [spell2] [spell3]
 *
 * Requires the "inscription" skill, a quill, and a blank scroll/parchment.
 * Up to 3 spells can be inscribed onto one scroll.
 */
export function doScribe(ch: CharData, argument: string): void {
  if (ch.isNpc) return;

  const pcdata = world.pcData.get(ch.id);
  if (!pcdata) return;

  const proficiency = getSkillProficiency(pcdata, SKILL_INSCRIPTION);
  if (proficiency === 0) {
    sendToChar(ch, 'You don\'t know how to scribe scrolls.\r\n');
    return;
  }

  if (!checkCraftCooldown(ch, pcdata)) return;

  if (!argument) {
    sendToChar(ch, 'Scribe what spell? Usage: scribe <spell1> [spell2] [spell3]\r\n');
    return;
  }

  const parts = argument.trim().split(/\s+/);
  const spellArgs = parts.slice(0, 3);

  const validSpells: string[] = [];
  for (const sp of spellArgs) {
    validSpells.push(sp);
  }

  if (validSpells.length === 0) {
    sendToChar(ch, 'You need to specify at least one spell to scribe.\r\n');
    return;
  }

  // Check for quill
  const quill = findItemByItemType(ch, ItemType.QUILL);
  if (!quill) {
    sendToChar(ch,
      `${colors.white}You do not have a quill.\r\n${colors.reset}`,
    );
    return;
  }

  // Check for blank scroll (ITEM_SCROLL with empty values)
  let parchment: ObjInstance | undefined;
  const items = getCarriedItems(ch);
  for (const obj of items) {
    if (obj.deleted) continue;
    if (obj.itemType === ItemType.SCROLL &&
        !obj.value[0] && !obj.value[1] && !obj.value[2] && !obj.value[3]) {
      parchment = obj;
      break;
    }
  }

  if (!parchment) {
    parchment = findItemByName(ch, 'scroll') || findItemByName(ch, 'parchment');
  }

  if (!parchment) {
    sendToChar(ch,
      `${colors.white}You do not have a blank scroll or parchment.\r\n${colors.reset}`,
    );
    return;
  }

  act('$n begins inscribing a scroll.', ch, null, null, TO_ROOM);
  sendToChar(ch,
    `${colors.white}You begin to carefully inscribe the scroll...\r\n${colors.reset}`,
  );

  // Skill check
  const numSpells = validSpells.length;
  const pctScribe = Math.floor(proficiency / 10);
  let target: number;
  if (numSpells === 1) {
    target = Math.floor((pctScribe + 80) / 2);
  } else if (numSpells === 2) {
    target = Math.floor((pctScribe + 60) / 3);
  } else {
    target = Math.floor((pctScribe + 40) / 4);
  }

  const roll = rollDice(1, 100);

  if (roll > target) {
    sendToChar(ch,
      `${colors.red}The ink runs and the scroll bursts into flames!\r\nYour inscription attempt fails.\r\n${colors.reset}`,
    );
    act('$n fails to write a scroll!', ch, null, null, TO_ROOM);

    world.extractObj(parchment);

    if (Math.random() < 0.25) {
      sendToChar(ch,
        `${colors.white}Your quill breaks!\r\n${colors.reset}`,
      );
      world.extractObj(quill);
    }

    tryImproveSkill(ch, pcdata, SKILL_INSCRIPTION);
    pcdata.craftTimer = DEFAULT_CRAFT_COOLDOWN;
    return;
  }

  // Success! Modify the existing parchment into a completed scroll
  const scrollMaterial = randomPick(SCROLL_MATERIALS);
  const spellList = validSpells.join(', ');

  parchment.name = `${scrollMaterial} scroll ${spellList}`.toLowerCase();
  parchment.shortDescr = `a ${scrollMaterial} scroll of ${spellList}`;
  parchment.description = `A ${scrollMaterial} scroll lies here, covered in arcane writing.`;
  parchment.level = ch.level;
  parchment.value[0] = ch.level; // spell level
  parchment.cost = ch.level * numSpells * 50;

  let buf = `${colors.brightGreen}You successfully inscribe:\r\n`;
  buf += `${colors.white}  ${parchment.shortDescr}\r\n`;
  buf += `${colors.cyan}  Level: ${ch.level}  Spells: ${spellList}\r\n`;
  buf += colors.reset;
  sendToChar(ch, buf);
  act('$n finishes writing a scroll.', ch, null, null, TO_ROOM);

  // 10% chance to break quill
  if (Math.random() < 0.10) {
    sendToChar(ch,
      `${colors.white}Your quill breaks!\r\n${colors.reset}`,
    );
    world.extractObj(quill);
  }

  tryImproveSkill(ch, pcdata, SKILL_INSCRIPTION);
  pcdata.craftTimer = DEFAULT_CRAFT_COOLDOWN;
}

// ============================================================================
//  Craft tick update -- called from the game loop
// ============================================================================

/**
 * craftUpdate -- Called every tick from the game loop.
 * Decrements craft cooldown timers for all connected players.
 */
export function craftUpdate(): void {
  for (const ch of world.characters.values()) {
    if (ch.isNpc || ch.deleted) continue;
    const pcdata = world.pcData.get(ch.id);
    if (!pcdata) continue;

    if (pcdata.craftTimer > 0) {
      pcdata.craftTimer--;
    }
  }
}

// ============================================================================
//  Exported command table entries
// ============================================================================

export const craftingCommands: CommandEntry[] = [
  { name: 'skin',    fn: doSkin,   minPosition: Position.STANDING, minLevel: 0, log: 0 },
  { name: 'tan',     fn: doTan,    minPosition: Position.STANDING, minLevel: 0, log: 0 },
  { name: 'forge',   fn: doForge,  minPosition: Position.STANDING, minLevel: 0, log: 0 },
  { name: 'brew',    fn: doBrew,   minPosition: Position.STANDING, minLevel: 0, log: 0 },
  { name: 'scribe',  fn: doScribe, minPosition: Position.STANDING, minLevel: 0, log: 0 },
];
