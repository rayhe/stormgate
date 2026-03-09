/**
 * magic.ts -- Spell casting core for Stormgate MUD.
 *
 * Port of do_cast(), say_spell(), update_skpell(), saves_spell(),
 * and related functions from magic.c.
 *
 * Contains:
 *   - castSpell()       Parse "cast 'spell name' [target]", validate, call spell
 *   - saySpell()        Garble spell name via syllable substitution
 *   - manaCost()        Compute mana cost for a character using a given spell
 *   - updateSkpell()    Chance-based skill improvement on use
 *   - doCast            Command function for "cast"
 *   - doPractice        Command function for "practice"
 *   - doSpells          Command function for "spells" (list known spells)
 *   - doSkills          Command function for "skills" (list known skills)
 *   - doSlist           Command function for "slist" (list spells/skills by level)
 */

import type { CharData } from './types.js';
import {
  Position,
  DamType,
  LEVEL_HERO,
  MAX_CLASS,
  CharClass,
} from './types.js';

import { world, charRoomMap } from './world.js';
import {
  getCharRoom,
  isImmortal,
  isNpc,
  isName,
  getInt,
  getWis,
  getDex,
} from './handler.js';

import {
  sendToChar,
  act,
  colors,
  TO_ROOM,
  TO_CHAR,
  TO_VICT,
} from './output.js';

import { damage, multiHit } from './fight.js';

import {
  skillTable,
  skillLookup,
  TargetType,
  SKPELL_NO_DAMAGE,
  SKPELL_MISSED,
  SKPELL_BOTCHED,
  L_APP,
  CLASS_NAMES,
  getSkillLevel,
  type SpellFn,
} from './skills.js';

import {
  numberPercent,
  numberFuzzy,
  uRange,
  uMin,
  uMax,
  findCharInRoom,
  updatePos,
} from './spells.js';

// ============================================================================
//  Constants
// ============================================================================

/** Maximum mortal level. L_APP = 999 (not available). */
const LEVEL_DEMIGOD = L_APP - 1;  // 998 -- but really, 105 in our game

/** ACT_PRACTICE flag for NPCs that can train players. */
const ACT_PRACTICE = 32;  // Bit 5 in the act bitvector

// ============================================================================
//  Syllable table for say_spell()
// ============================================================================

const SYL_TABLE: Array<[string, string]> = [
  [' ',      ' '],
  ['ar',     'abra'],
  ['au',     'kada'],
  ['bless',  'fido'],
  ['blind',  'nose'],
  ['bur',    'mosa'],
  ['cu',     'judi'],
  ['de',     'oculo'],
  ['en',     'unso'],
  ['light',  'dies'],
  ['lo',     'hi'],
  ['mor',    'zak'],
  ['move',   'sido'],
  ['ness',   'lacri'],
  ['ning',   'illa'],
  ['per',    'duda'],
  ['ra',     'gru'],
  ['re',     'candus'],
  ['son',    'sabru'],
  ['tect',   'infra'],
  ['tri',    'cula'],
  ['ven',    'nofo'],
  ['a', 'a'], ['b', 'b'], ['c', 'q'], ['d', 'e'],
  ['e', 'z'], ['f', 'y'], ['g', 'o'], ['h', 'p'],
  ['i', 'u'], ['j', 'y'], ['k', 't'], ['l', 'r'],
  ['m', 'w'], ['n', 'i'], ['o', 'a'], ['p', 's'],
  ['q', 'd'], ['r', 'f'], ['s', 'g'], ['t', 'h'],
  ['u', 'j'], ['v', 'z'], ['w', 'x'], ['x', 'n'],
  ['y', 'l'], ['z', 'k'],
];

// ============================================================================
//  say_spell() -- garble spell name using syllable substitution
// ============================================================================

/**
 * Utter mystical words for a spell.
 *
 * Characters of the same class as the caster see the real spell name;
 * everyone else sees a garbled version built by the syllable substitution
 * table (same algorithm as the C original).
 */
function saySpell(ch: CharData, sn: number): void {
  const name = skillTable[sn].name;

  // Build the garbled version
  let garbled = '';
  let i = 0;
  while (i < name.length) {
    let found = false;
    for (const [old, replacement] of SYL_TABLE) {
      if (old.length === 0) continue;
      if (name.substring(i, i + old.length).toLowerCase() === old) {
        garbled += replacement;
        i += old.length;
        found = true;
        break;
      }
    }
    if (!found) {
      garbled += name[i];
      i++;
    }
  }

  const realMsg = `$n utters the words, '${name}'.`;
  const garbledMsg = `$n utters the words, '${garbled}'.`;

  // Send to everyone in the room except the caster
  const roomVnum = charRoomMap.get(ch.id);
  if (roomVnum === undefined) return;

  const chars = world.getCharsInRoom(roomVnum);
  for (const rch of chars) {
    if (rch.id === ch.id || rch.deleted) continue;
    // Same class sees the real spell name; others see garbled
    const msg = (ch.charClass === rch.charClass) ? realMsg : garbledMsg;
    act(msg, ch, null, rch, TO_VICT);
  }
}

// ============================================================================
//  manaCost() -- compute the mana cost for a spell
// ============================================================================

/**
 * Compute the mana cost for a character to cast a given spell.
 *
 * Formula from merc.h:
 *   MANA_COST = UMAX(min_mana, 100 / UMAX(1, 2 + ch.level - skill_level[class]))
 *
 * NPCs pay 0 mana.
 */
export function manaCost(ch: CharData, sn: number): number {
  if (ch.isNpc) return 0;

  const entry = skillTable[sn];
  if (!entry) return 0;

  const skillLevel = entry.levels[ch.charClass] ?? L_APP;
  const divisor = uMax(1, 2 + ch.level - skillLevel);
  return uMax(entry.minMana, Math.floor(100 / divisor));
}

// ============================================================================
//  update_skpell() -- chance-based skill improvement on use
// ============================================================================

/**
 * Attempt to improve a character's proficiency in a skill/spell.
 *
 * Called after each successful use. The chance of improvement decreases
 * as proficiency increases, following the tiered system from the C source.
 *
 * @param ch       The character who used the skill.
 * @param sn       The skill number.
 * @param override If non-zero, force this amount of gain (ignoring chance).
 */
export function updateSkpell(ch: CharData, sn: number, override: number = 0): void {
  if (ch.isNpc) return;

  const pc = world.pcData.get(ch.id);
  if (!pc) return;

  const learned = pc.learned.get(sn) ?? 0;
  let pct = 0;
  let gain = 1;

  if (override === 0) {
    if (learned === 0) return;

    // Stat bonus: spells use INT, skills use DEX
    const statBonus = skillTable[sn].isSpell
      ? Math.floor(getInt(ch) / 2)
      : Math.floor(getDex(ch) / 2);

    if (learned <= 100) {
      pct = 100 + statBonus;
      gain = numberFuzzy(10);
    } else if (learned <= 200) {
      pct = 95 + statBonus;
      gain = numberFuzzy(9);
    } else if (learned <= 300) {
      pct = 90 + statBonus;
      gain = numberFuzzy(8);
    } else if (learned <= 400) {
      pct = 85 + statBonus;
      gain = numberFuzzy(7);
    } else if (learned <= 500) {
      pct = 75 + statBonus;
      gain = numberFuzzy(6);
    } else if (learned <= 600) {
      pct = 65 + statBonus;
      gain = numberFuzzy(4);
    } else if (learned <= 750) {
      pct = (125 - Math.floor(learned / 10)) + statBonus;
      gain = numberFuzzy(2);
    } else {
      pct = Math.floor((125 - Math.floor(learned / 10)) / 2) + statBonus;
      gain = 1;
    }

    if (gain < 1) gain = 1;
  } else {
    gain = override;
    pct = 101;  // Guaranteed improvement
  }

  if (numberPercent() < pct) {
    if (learned < 1000) {
      // Below LEVEL_HERO, 50% chance to actually gain; at/above LEVEL_HERO always gain
      if ((ch.level < LEVEL_HERO && numberPercent() <= 50) ||
           ch.level >= LEVEL_HERO ||
           override !== 0) {
        let newLearned = learned + gain;
        if (newLearned > 1000) newLearned = 1000;
        pc.learned.set(sn, newLearned);

        const typeWord = skillTable[sn].isSpell ? 'spell' : 'skill';
        sendToChar(ch,
          `${colors.cyan}Your ${typeWord} "${skillTable[sn].name}" improves to ${(newLearned / 10).toFixed(1)}%!\r\n`
        );

        if (newLearned >= 1000) {
          sendToChar(ch,
            `${colors.cyan}Congratulations!  Your ${typeWord} "${skillTable[sn].name}" has reached grandmaster level!\r\n`
          );
        }
      }
    }
  }
}

// ============================================================================
//  oneArgument() -- extract the first word from an input string
// ============================================================================

/**
 * Extract the first space-delimited word from a string.
 * Handles single-quoted spell names: cast 'magic missile' target
 * Returns [firstArg, rest].
 */
function oneArgument(input: string): [string, string] {
  const trimmed = input.trim();
  if (!trimmed) return ['', ''];

  // Handle quoted arguments (single quotes for spell names)
  if (trimmed[0] === '\'' || trimmed[0] === '"') {
    const quote = trimmed[0];
    const endIdx = trimmed.indexOf(quote, 1);
    if (endIdx > 0) {
      return [trimmed.substring(1, endIdx), trimmed.substring(endIdx + 1).trim()];
    }
    // No closing quote -- take rest as the argument
    return [trimmed.substring(1), ''];
  }

  const spaceIdx = trimmed.indexOf(' ');
  if (spaceIdx < 0) return [trimmed, ''];
  return [trimmed.substring(0, spaceIdx), trimmed.substring(spaceIdx + 1).trim()];
}

// ============================================================================
//  findObjHere() -- find an object in room or inventory
// ============================================================================

function findObjHere(ch: CharData, name: string): import('./types.js').ObjInstance | null {
  // Check inventory first
  for (const obj of world.objects.values()) {
    if (obj.deleted) continue;
    if (obj.carriedBy === ch.id && isName(name, obj.name)) return obj;
  }
  // Then check the room
  const roomVnum = getCharRoom(ch);
  if (roomVnum < 0) return null;
  for (const obj of world.objects.values()) {
    if (obj.deleted) continue;
    if (obj.inRoom === roomVnum && isName(name, obj.name)) return obj;
  }
  return null;
}

// ============================================================================
//  doCast -- the "cast" command
// ============================================================================

/**
 * The main cast command.
 *
 * Syntax: cast 'spell name' [target]
 *
 * 1. Look up spell by name in skillTable.
 * 2. Check if ch knows the spell and meets level requirement.
 * 3. Check that it's actually a spell (not a skill).
 * 4. Compute mana cost; check ch has enough.
 * 5. Determine target based on targetType.
 * 6. Say mystical words (saySpell).
 * 7. Roll skill check; on failure, lose half mana.
 * 8. On success, deduct full mana, call spell function, apply damage.
 * 9. Attempt skill improvement (updateSkpell).
 * 10. If offensive, victim retaliates if not already fighting.
 */
export function doCast(ch: CharData, argument: string): void {
  // Parse: cast 'spell name' [target]
  const [arg1, arg2] = oneArgument(argument);

  if (!arg1) {
    sendToChar(ch, `${colors.blue}Cast which what where?\r\n`);
    return;
  }

  // NPCs with charm can't cast
  if (ch.isNpc && (ch.affectedBy & 262144)) { // AFF_CHARM
    return;
  }

  // NPCs with 0 mana can't cast
  if (ch.isNpc && ch.mana === 0) return;

  // Look up the spell
  const sn = skillLookup(arg1);
  if (sn < 0) {
    sendToChar(ch, `${colors.blue}You can't do that.\r\n`);
    return;
  }

  // Check level requirement for the character's class
  if (!ch.isNpc) {
    const reqLevel = skillTable[sn].levels[ch.charClass] ?? L_APP;
    if (ch.level < reqLevel) {
      sendToChar(ch, `${colors.blue}You can't do that.\r\n`);
      return;
    }
  }

  // Must be a spell, not a skill
  if (!skillTable[sn].isSpell) {
    sendToChar(ch, `Casting a skill? Try using it...\r\n`);
    return;
  }

  // Check minimum position
  if (ch.position < skillTable[sn].minPosition) {
    sendToChar(ch, `${colors.blue}You can't concentrate enough.\r\n`);
    return;
  }

  // Compute mana cost
  const mana = manaCost(ch, sn);

  // Locate targets
  let victim: CharData | null = null;
  let obj: import('./types.js').ObjInstance | null = null;
  let vo: unknown = null;

  switch (skillTable[sn].target) {
    case TargetType.IGNORE:
      break;

    case TargetType.GROUP_OFFENSIVE:
    case TargetType.GROUP_DEFENSIVE:
    case TargetType.GROUP_ALL:
    case TargetType.GROUP_OBJ:
    case TargetType.GROUP_IGNORE:
      // Group spells not yet implemented in web port
      sendToChar(ch, `${colors.blue}Group spells are not yet available.\r\n`);
      return;

    case TargetType.CHAR_OFFENSIVE:
      if (!arg2) {
        // Default to current fighting target
        if (ch.fighting) {
          victim = world.getCharById(ch.fighting) ?? null;
        }
        if (!victim) {
          sendToChar(ch, `${colors.blue}Cast the spell on whom?\r\n`);
          return;
        }
      } else {
        victim = findCharInRoom(ch, arg2);
        if (!victim) {
          sendToChar(ch, `${colors.blue}They aren't here.\r\n`);
          return;
        }
      }

      // Safety checks for offensive casting
      if (victim.id === ch.id) {
        sendToChar(ch, `${colors.blue}You can't cast offensive spells on yourself.\r\n`);
        return;
      }

      // Can't attack immortals
      if (isImmortal(victim) && !isImmortal(ch)) {
        sendToChar(ch, `${colors.blue}You failed.\r\n`);
        return;
      }

      vo = victim;
      break;

    case TargetType.CHAR_DEFENSIVE:
      if (!arg2) {
        victim = ch; // Default to self
      } else {
        victim = findCharInRoom(ch, arg2);
        if (!victim) {
          sendToChar(ch, `${colors.blue}They aren't here.\r\n`);
          return;
        }
      }
      vo = victim;
      break;

    case TargetType.CHAR_SELF:
      if (arg2 && !isName(arg2, ch.name)) {
        sendToChar(ch, `${colors.blue}You cannot cast this spell on another.\r\n`);
        return;
      }
      vo = ch;
      victim = ch;
      break;

    case TargetType.OBJ_INV:
    case TargetType.OBJ_ROOM:
      if (!arg2) {
        sendToChar(ch, `${colors.blue}What should the spell be cast upon?\r\n`);
        return;
      }
      obj = findObjHere(ch, arg2);
      if (!obj) {
        sendToChar(ch, `${colors.blue}You can't find that.\r\n`);
        return;
      }
      vo = obj;
      break;

    default:
      sendToChar(ch, `${colors.blue}You can't cast that.\r\n`);
      return;
  }

  // Check mana
  if (!ch.isNpc) {
    if (ch.mana < mana) {
      sendToChar(ch, `${colors.blue}You don't have enough mana.\r\n`);
      return;
    }
  }

  // Say the mystical words (ventriloquate skips this)
  if (skillTable[sn].name !== 'ventriloquate') {
    saySpell(ch, sn);
  }

  // Check for no-magic rooms
  const roomVnum = getCharRoom(ch);
  const room = roomVnum >= 0 ? world.getRoom(roomVnum) : null;
  if (room && (room.roomFlags & 4)) { // ROOM_NO_MAGIC
    sendToChar(ch, `${colors.blue}You failed.\r\n`);
    return;
  }

  // For PCs, check skill proficiency
  if (!ch.isNpc) {
    const pc = world.pcData.get(ch.id);
    const learned = pc?.learned.get(sn) ?? 0;

    if (numberPercent() > Math.floor(learned / 10)) {
      // Failed concentration -- lose half mana
      sendToChar(ch, `${colors.blue}You lost your concentration.\r\n`);
      ch.mana -= Math.floor(mana / 2);
      if (ch.mana < 0) ch.mana = 0;

      // Still a chance to improve if low enough
      if (learned <= 750) {
        updateSkpell(ch, sn, 0);
      }

      // Wait state is reduced when not in combat
      if (!ch.fighting) {
        const statDiv = (ch.charClass === CharClass.CLERIC || ch.charClass === CharClass.DARKPRIEST)
          ? Math.max(1, Math.floor(getWis(ch) / 10))
          : Math.max(1, Math.floor(getInt(ch) / 10));
        ch.combatTimer += Math.floor(skillTable[sn].beats / statDiv);
      } else {
        ch.combatTimer += skillTable[sn].beats;
      }
      return;
    }

    // Successful cast -- deduct full mana
    ch.mana -= mana;
    if (ch.mana < 0) ch.mana = 0;

    // Set wait state
    ch.combatTimer += skillTable[sn].beats;

    // Call the spell function
    const spellFn = skillTable[sn].spellFn;
    if (!spellFn) {
      sendToChar(ch, `${colors.blue}That spell is not yet implemented.\r\n`);
      return;
    }

    const dam = spellFn(sn, uRange(1, ch.level, LEVEL_DEMIGOD), ch, vo);

    // Handle return values
    if (dam > SKPELL_NO_DAMAGE && victim) {
      // Positive return = damage amount; apply via damage()
      const noun = skillTable[sn].damageNoun || 'spell';
      damage(ch, victim, dam, noun, DamType.ENERGY);
    }

    // Improve skill on successful (non-botched) cast
    if (dam >= SKPELL_NO_DAMAGE) {
      updateSkpell(ch, sn, 0);
    }

  } else {
    // NPC casting -- no mana check, no skill check
    const spellFn = skillTable[sn].spellFn;
    if (!spellFn) return;

    const dam = spellFn(sn, uRange(1, ch.level, LEVEL_DEMIGOD), ch, vo);
    if (dam > SKPELL_NO_DAMAGE && victim) {
      const noun = skillTable[sn].damageNoun || 'spell';
      damage(ch, victim, dam, noun, DamType.ENERGY);
    }
  }

  // Offensive spells provoke retaliation
  if (skillTable[sn].target === TargetType.CHAR_OFFENSIVE && victim) {
    if (victim.id !== ch.id && !victim.fighting && !victim.deleted) {
      if (victim.position >= Position.RESTING) {
        multiHit(victim, ch, -1);
      }
    }
  }
}

// ============================================================================
//  doPractice -- the "practice" command
// ============================================================================

/**
 * Practice a skill/spell at a guild trainer.
 *
 * With no argument: list all available skills/spells and their proficiency.
 * With an argument: practice that skill (requires a trainer mob in the room
 * and available practice sessions).
 */
export function doPractice(ch: CharData, argument: string): void {
  if (ch.isNpc) return;

  const pc = world.pcData.get(ch.id);
  if (!pc) return;

  if (ch.level < 3) {
    sendToChar(ch, 'You must be third level to practice.  Go train instead!\r\n');
    return;
  }

  // Check for a practice trainer in the room
  const roomVnum = getCharRoom(ch);
  let mob: CharData | null = null;
  if (roomVnum >= 0) {
    const chars = world.getCharsInRoom(roomVnum);
    for (const rch of chars) {
      if (rch.deleted) continue;
      if (rch.isNpc && (rch.act & ACT_PRACTICE)) {
        mob = rch;
        break;
      }
    }
  }

  if (!argument.trim()) {
    // List all available skills/spells
    let output = '';
    let col = 0;

    for (let sn = 0; sn < skillTable.length; sn++) {
      const entry = skillTable[sn];
      if (!entry.name || entry.name === 'reserved') continue;

      const reqLevel = entry.levels[ch.charClass] ?? L_APP;
      if (ch.level < reqLevel) continue;

      const learned = pc.learned.get(sn) ?? 0;
      // If there's no trainer, only show skills the player already knows
      if (!mob && learned <= 0) continue;

      const profStr = (Math.min(1000, learned) / 10).toFixed(1).padStart(5);
      const nameStr = entry.name.padEnd(26);
      output += `${nameStr} ${profStr}%  `;
      col++;
      if (col % 2 === 0) {
        output += '\r\n';
      }
    }

    if (col % 2 !== 0) output += '\r\n';
    output += `You have ${ch.practice} practice sessions left.\r\n`;
    sendToChar(ch, output);
    return;
  }

  // Practice a specific skill
  if (ch.position < Position.RESTING) {
    sendToChar(ch, 'In your dreams, or what?\r\n');
    return;
  }

  if (!mob) {
    sendToChar(ch, "You can't do that here.\r\n");
    return;
  }

  if (ch.practice <= 0) {
    sendToChar(ch, 'You have no practice sessions left.\r\n');
    return;
  }

  const sn = skillLookup(argument.trim());
  if (sn < 0) {
    sendToChar(ch, "You can't practice that.\r\n");
    return;
  }

  const reqLevel = skillTable[sn].levels[ch.charClass] ?? L_APP;
  if (ch.level < reqLevel) {
    sendToChar(ch, "You can't practice that.\r\n");
    return;
  }

  // Compute the adept level (maximum from practice)
  // Simplified from the C version: base adept is proportional to skill level
  const lvl = Math.min(reqLevel, L_APP);
  const inverse = 10.0 / Math.max(1, lvl);
  const classAdept = 75; // class_table[ch.class].skill_adept -- default 75%
  const learnBonus = Math.floor(getInt(ch) / 2);
  const rawMax = Math.floor(inverse * 1000) + learnBonus + classAdept;
  const adept = Math.min(classAdept * 10, rawMax);

  const currentLearned = pc.learned.get(sn) ?? 0;

  if (currentLearned >= adept) {
    sendToChar(ch, `You are already an adept of ${skillTable[sn].name}.\r\n`);
    return;
  }

  ch.practice--;
  const practiceGain = learnBonus * Math.max(1, Math.floor((Math.floor(getWis(ch) / 2) + 1) / 2));
  let newLearned = currentLearned + practiceGain;

  if (newLearned < adept) {
    pc.learned.set(sn, newLearned);
    act(`You practice $T.`, ch, null, skillTable[sn].name, TO_CHAR);
    act(`$n practices $T.`, ch, null, skillTable[sn].name, TO_ROOM);
  } else {
    newLearned = adept;
    pc.learned.set(sn, newLearned);
    act(`You are now an adept of $T.`, ch, null, skillTable[sn].name, TO_CHAR);
    act(`$n is now an adept of $T.`, ch, null, skillTable[sn].name, TO_ROOM);
  }
}

// ============================================================================
//  doSpells -- the "spells" command
// ============================================================================

/**
 * List all spells available to the character, with mana costs.
 */
export function doSpells(ch: CharData, _argument: string): void {
  if (ch.isNpc) {
    sendToChar(ch, `${colors.blue}You do not know how to cast spells!\r\n`);
    return;
  }

  let output = '';
  let col = 0;

  for (let sn = 0; sn < skillTable.length; sn++) {
    const entry = skillTable[sn];
    if (!entry.name || entry.name === 'reserved') continue;

    // Must meet level requirement
    const reqLevel = entry.levels[ch.charClass] ?? L_APP;
    if (ch.level < reqLevel) continue;

    // Must be a spell (not a skill)
    if (!entry.isSpell) continue;

    const cost = manaCost(ch, sn);
    const nameStr = entry.name.padEnd(26);
    const costStr = cost.toString().padStart(3);
    output += `${nameStr} ${costStr}pts `;
    col++;
    if (col % 2 === 0) {
      output += '\r\n';
    }
  }

  if (col % 2 !== 0) output += '\r\n';

  if (col === 0) {
    sendToChar(ch, `${colors.blue}You do not know any spells.\r\n`);
  } else {
    sendToChar(ch, `${colors.blue}${output}`);
  }
}

// ============================================================================
//  doSkills -- the "skills" command
// ============================================================================

/**
 * List all skills (non-spell abilities) available to the character.
 */
export function doSkills(ch: CharData, _argument: string): void {
  if (ch.isNpc) {
    sendToChar(ch, `${colors.blue}You do not know how to do that!\r\n`);
    return;
  }

  let output = '';
  let col = 0;

  for (let sn = 0; sn < skillTable.length; sn++) {
    const entry = skillTable[sn];
    if (!entry.name || entry.name === 'reserved') continue;

    // Must meet level requirement
    const reqLevel = entry.levels[ch.charClass] ?? L_APP;
    if (ch.level < reqLevel) continue;

    // Must be a skill (not a spell)
    if (entry.isSpell) continue;

    const cost = manaCost(ch, sn);
    const nameStr = entry.name.padEnd(26);
    const costStr = cost.toString().padStart(3);
    output += `${nameStr} ${costStr}pts `;
    col++;
    if (col % 2 === 0) {
      output += '\r\n';
    }
  }

  if (col % 2 !== 0) output += '\r\n';

  if (col === 0) {
    sendToChar(ch, `${colors.blue}You do not know any skills.\r\n`);
  } else {
    sendToChar(ch, `${colors.blue}${output}`);
  }
}

// ============================================================================
//  doSlist -- the "slist" command (list spells/skills by level)
// ============================================================================

/**
 * List all spells and skills organized by level for the character's class.
 */
export function doSlist(ch: CharData, _argument: string): void {
  if (ch.isNpc) {
    sendToChar(ch, `${colors.blue}You do not need any stinking spells!\r\n`);
    return;
  }

  let output = `${colors.blue}Lv          Spells/Skill\r\n\r\n`;

  for (let level = 1; level <= LEVEL_DEMIGOD && level <= 105; level++) {
    let col = 0;
    let levelHeader = false;

    for (let sn = 0; sn < skillTable.length; sn++) {
      const entry = skillTable[sn];
      if (!entry.name || entry.name === 'reserved') continue;

      const reqLevel = entry.levels[ch.charClass] ?? L_APP;
      if (reqLevel !== level) continue;

      if (!levelHeader) {
        output += `${level.toString().padStart(2)}: `;
        levelHeader = true;
      }

      if (col > 0 && col % 4 === 0) {
        output += '   ';
      }

      output += entry.name.padStart(25);
      col++;
    }

    if (levelHeader) {
      output += '\r\n';
    }
  }

  sendToChar(ch, output);
}

// ============================================================================
//  Initialization
// ============================================================================

/**
 * Register the magic commands into the command table.
 * Called from the main initialization path.
 */
export function getMagicCommands(): Array<{
  name: string;
  fn: (ch: CharData, argument: string) => void;
  minPosition: Position;
  minLevel: number;
  log: number;
}> {
  return [
    { name: 'cast',     fn: doCast,     minPosition: Position.FIGHTING, minLevel: 0, log: 0 },
    { name: 'use',      fn: doUse,      minPosition: Position.FIGHTING, minLevel: 0, log: 0 },
    { name: 'practice', fn: doPractice, minPosition: Position.RESTING,  minLevel: 0, log: 0 },
    { name: 'spells',   fn: doSpells,   minPosition: Position.DEAD,     minLevel: 0, log: 0 },
    { name: 'skills',   fn: doSkills,   minPosition: Position.DEAD,     minLevel: 0, log: 0 },
    { name: 'slist',    fn: doSlist,    minPosition: Position.DEAD,     minLevel: 0, log: 0 },
  ];
}

// ============================================================================
//  Spell function registry (maps string names to implementations)
// ============================================================================

/**
 * Registry of spell functions by name.
 * Used to resolve string-based spell function references at runtime.
 */
const spellFunctions = new Map<string, SpellFn>();

/**
 * Register a spell function by its C-style name (e.g., "spell_armor").
 * This allows the skill table to reference spell functions by string name
 * and have them resolved to actual implementations at startup.
 */
export function registerSpell(name: string, fn: SpellFn): void {
  spellFunctions.set(name, fn);
}

/**
 * Look up a registered spell function by its C-style name.
 * Returns the function or undefined if not registered.
 */
export function getRegisteredSpell(name: string): SpellFn | undefined {
  return spellFunctions.get(name);
}

// ============================================================================
//  savingThrow -- simplified saving throw vs. spells
// ============================================================================

/**
 * Determine if a victim saves against a spell effect.
 *
 * The saving throw chance is based on the level difference between
 * caster and victim, modified by the victim's savingThrow stat.
 *
 * @param level   The effective level of the caster/spell.
 * @param victim  The character attempting to resist.
 * @returns true if the victim successfully saves (spell is resisted).
 */
export function savingThrow(level: number, victim: CharData): boolean {
  let save = 50 + (victim.level - level) * 2 - victim.savingThrow;
  save = Math.max(5, Math.min(95, save));
  return Math.floor(Math.random() * 100) < save;
}

// ============================================================================
//  doUse -- the "use" command (non-spell skills)
// ============================================================================

/**
 * The main skill use command.
 *
 * Syntax: use <skill name> [target]
 *
 * Similar to doCast but for skills (isSpell === false).
 * Uses move points instead of mana as the resource cost.
 *
 * Ported from skills.c do_use().
 */
export function doUse(ch: CharData, argument: string): void {
  // Parse: use <skill name> [target]
  const [arg1, arg2] = oneArgument(argument);

  if (!arg1) {
    sendToChar(ch, `${colors.blue}Use which what where?\r\n`);
    return;
  }

  // NPCs with charm can't use skills
  if (ch.isNpc && (ch.affectedBy & 262144)) { // AFF_CHARM
    return;
  }

  // Look up the skill
  const sn = skillLookup(arg1);
  if (sn < 0) {
    sendToChar(ch, `${colors.blue}You can't do that.\r\n`);
    return;
  }

  // Check level requirement for the character's class
  if (!ch.isNpc) {
    const reqLevel = skillTable[sn].levels[ch.charClass] ?? L_APP;
    if (ch.level < reqLevel) {
      sendToChar(ch, `${colors.blue}You can't do that.\r\n`);
      return;
    }
  }

  // Must be a skill, not a spell
  if (skillTable[sn].isSpell) {
    sendToChar(ch, `Using a spell? Try casting...\r\n`);
    return;
  }

  // Check that it has an implementation (not a passive skill with spell_null)
  if (!ch.isNpc && !skillTable[sn].spellFn) {
    sendToChar(ch, `You can't use passive skills.\r\n`);
    return;
  }

  // Check minimum position
  if (ch.position < skillTable[sn].minPosition) {
    sendToChar(ch, `${colors.blue}You can't concentrate enough.\r\n`);
    return;
  }

  // Compute move cost (skills use move instead of mana)
  const moveCost = manaCost(ch, sn);

  // Locate targets
  let victim: CharData | null = null;
  let obj: import('./types.js').ObjInstance | null = null;
  let vo: unknown = null;

  switch (skillTable[sn].target) {
    case TargetType.IGNORE:
      break;

    case TargetType.CHAR_OFFENSIVE:
      if (!arg2) {
        // Default to current fighting target
        if (ch.fighting) {
          victim = world.getCharById(ch.fighting) ?? null;
        }
        if (!victim) {
          sendToChar(ch, `${colors.blue}Use that skill on whom?\r\n`);
          return;
        }
      } else {
        victim = findCharInRoom(ch, arg2);
        if (!victim) {
          sendToChar(ch, `${colors.blue}They aren't here.\r\n`);
          return;
        }
      }

      // Safety checks
      if (victim.id === ch.id) {
        sendToChar(ch, `${colors.blue}You can't use offensive skills on yourself.\r\n`);
        return;
      }

      if (isImmortal(victim) && !isImmortal(ch)) {
        sendToChar(ch, `${colors.blue}You failed.\r\n`);
        return;
      }

      vo = victim;
      break;

    case TargetType.CHAR_DEFENSIVE:
      if (!arg2) {
        victim = ch; // Default to self
      } else {
        victim = findCharInRoom(ch, arg2);
        if (!victim) {
          sendToChar(ch, `${colors.blue}They aren't here.\r\n`);
          return;
        }
      }
      vo = victim;
      break;

    case TargetType.CHAR_SELF:
      if (arg2 && !isName(arg2, ch.name)) {
        sendToChar(ch, `${colors.blue}You cannot use this skill on another.\r\n`);
        return;
      }
      vo = ch;
      victim = ch;
      break;

    case TargetType.OBJ_INV:
    case TargetType.OBJ_ROOM:
      if (!arg2) {
        sendToChar(ch, `${colors.blue}What should the skill be used upon?\r\n`);
        return;
      }
      obj = findObjHere(ch, arg2);
      if (!obj) {
        sendToChar(ch, `${colors.blue}You can't find that.\r\n`);
        return;
      }
      vo = obj;
      break;

    default:
      sendToChar(ch, `${colors.blue}You can't use that.\r\n`);
      return;
  }

  // Check move points
  if (!ch.isNpc) {
    if (ch.move < moveCost) {
      sendToChar(ch, `${colors.blue}You are too tired to do that.\r\n`);
      return;
    }
  }

  // Check for no-offensive rooms
  const roomVnum = getCharRoom(ch);
  const room = roomVnum >= 0 ? world.getRoom(roomVnum) : null;
  if (room && (room.roomFlags & 4) && skillTable[sn].target === TargetType.CHAR_OFFENSIVE) {
    sendToChar(ch, `${colors.blue}You failed.\r\n`);
    return;
  }

  // Set wait state
  if (!ch.isNpc) {
    ch.combatTimer += skillTable[sn].beats;
  }

  // For PCs, check skill proficiency
  if (!ch.isNpc) {
    const pc = world.pcData.get(ch.id);
    const learned = pc?.learned.get(sn) ?? 0;

    if (numberPercent() > Math.floor(learned / 10)) {
      // Failed -- lose half move
      sendToChar(ch, `${colors.blue}You lost your concentration.\r\n`);
      ch.move -= Math.floor(moveCost / 2);
      if (ch.move < 0) ch.move = 0;

      // Still a chance to improve if low enough
      if (learned <= 750) {
        updateSkpell(ch, sn, 0);
      }
      return;
    }

    // Successful use -- deduct full move
    ch.move -= moveCost;
    if (ch.move < 0) ch.move = 0;

    // Call the skill function
    const spellFn = skillTable[sn].spellFn;
    if (!spellFn) {
      sendToChar(ch, `${colors.blue}That skill is not yet implemented.\r\n`);
      return;
    }

    const dam = spellFn(sn, uRange(1, ch.level, LEVEL_DEMIGOD), ch, vo);

    // Handle return values
    if (dam > SKPELL_NO_DAMAGE && victim && !victim.deleted && victim.position !== Position.DEAD) {
      const noun = skillTable[sn].damageNoun || 'skill';
      damage(ch, victim, dam, noun, DamType.BASH);
    }

    // Improve skill on successful (non-botched) use
    if (dam >= SKPELL_NO_DAMAGE) {
      updateSkpell(ch, sn, 0);
    }

  } else {
    // NPC usage -- no move check, no skill check
    const spellFn = skillTable[sn].spellFn;
    if (!spellFn) return;

    const dam = spellFn(sn, uRange(1, ch.level, LEVEL_DEMIGOD), ch, vo);
    if (dam > SKPELL_NO_DAMAGE && victim && !victim.deleted && victim.position !== Position.DEAD) {
      const noun = skillTable[sn].damageNoun || 'skill';
      damage(ch, victim, dam, noun, DamType.BASH);
    }
  }

  // Offensive skills provoke retaliation
  if (skillTable[sn].target === TargetType.CHAR_OFFENSIVE && victim) {
    if (victim.id !== ch.id && !victim.fighting && !victim.deleted) {
      if (victim.position >= Position.RESTING) {
        multiHit(victim, ch, -1);
      }
    }
  }
}
