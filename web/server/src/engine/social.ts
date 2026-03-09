/**
 * social.ts -- Social systems for Stormgate MUD.
 *
 * Ported from the C sources:
 *   language.c  -- 27-language system with garbling
 *   garble.c    -- Language garbling algorithm
 *   act_clan.c  -- Clan management / clan chat
 *   religion.c  -- Religion system / pray channel
 *   economy.c   -- Banking and shares
 *   marriage.c  -- Marriage / divorce (immortal commands)
 *   games.c     -- Gambling (slots, dice, seven)
 *
 * All commands export a CommandEntry[] array (socialCommands) that can be
 * spread into the master command table in commands.ts.
 */

import type {
  CharData,
  ObjInstance,
  ClanData,
  ReligionData,
} from './types.js';

import {
  Position,
  Language,
  MAX_LANGUAGE,
  LEVEL_HERO,
  LEVEL_IMMORTAL,
} from './types.js';

import { world, charRoomMap, rollDice } from './world.js';
import {
  getCharRoom,
  isImmortal,
  isName,
  capitalize,
} from './handler.js';

import {
  sendToChar,
  act,
  colors,
  hasConnection,
  sendToAll,
  TO_ROOM,
  TO_VICT,
  TO_NOTVICT,
} from './output.js';

import { sendVitals } from './protocol.js';

import type { CommandEntry } from './commands.js';

// ============================================================================
//  Language tables
// ============================================================================

/** Language names indexed by Language const values (matches const.c lang_table). */
export const LANGUAGE_NAMES: string[] = [
  'Common',
  'Human',
  'Dwarvish',
  'Elvish',
  'Gnomish',
  'Dragon',
  'Demon',
  'Ogre',
  'Drow',
  'Elder',
  'Pixie',
  'Hobbit',
  'Minotaur',
  'Lizard',
  'Halfling',
  'Feline',
  'Canine',
  'Angel',
  'Orcish',
  'Magick',
  'Shadow Speak',
  'Spiritspeak',
  'Enlightened',
  'Satanic',
  'Animalspeak',
  'Bretonnian',
  'Gargish',
];

/**
 * Look up a language by name prefix. Returns the language id or -1.
 */
function langLookup(name: string): number {
  const lower = name.toLowerCase();
  for (let i = 0; i < LANGUAGE_NAMES.length; i++) {
    if (LANGUAGE_NAMES[i].toLowerCase().startsWith(lower)) {
      return i;
    }
  }
  return -1;
}

// ============================================================================
//  Language garbling (from garble.c)
// ============================================================================

/**
 * Garble substitution tables -- each language maps letters to different
 * random replacements so that garbled text "sounds" consistently different
 * per language.  Ported from garble.c's approach of cycling through
 * substitution characters.
 */
const GARBLE_SUBS: string[][] = [
  /* COMMON    */ 'abcdefghijklmnopqrstuvwxyz'.split(''),
  /* HUMAN     */ 'pqrstuvwxyzabcdefghijklmno'.split(''),
  /* DWARVISH  */ 'zyxwvutsrqponmlkjihgfedcba'.split(''),
  /* ELVISH    */ 'lmaonpeqbrcsftdugvhwixjykz'.split(''),
  /* GNOMISH   */ 'ghijklmnopqrstuvwxyzabcdef'.split(''),
  /* DRAGON    */ 'xyzabcdefghijklmnopqrstuvw'.split(''),
  /* DEMON     */ 'nopqrstuvwxyzabcdefghijklm'.split(''),
  /* OGRE      */ 'bcdfghjklmnpqrstvwxyzaeio '.split(''),
  /* DROW      */ 'tsrqponmlkjihgfedcbazyxwvu'.split(''),
  /* ELDER     */ 'aeioubcdfghjklmnpqrstvwxyz'.split(''),
  /* PIXIE     */ 'eioaubcdfghjklmnpqrstvwxyz'.split(''),
  /* HOBBIT    */ 'fghijklmnopqrstuvwxyzabcde'.split(''),
  /* MINOTAUR  */ 'mlkjihgfedcbazyxwvutsrqpon'.split(''),
  /* LIZARD    */ 'ssszhkrlmthssszzzhhkrlmths'.split(''),
  /* HALFLING  */ 'jihgfedcbazyxwvutsrqponmlk'.split(''),
  /* FELINE    */ 'mnopqrstuvwxyzabcdefghijkl'.split(''),
  /* CANINE    */ 'rstuvwxyzabcdefghijklmnopq'.split(''),
  /* ANGEL     */ 'aeiouaeiouaeiouaeiouaeioua'.split(''),
  /* ORCISH    */ 'kgzdrthbmfnwpsxjqlvcaeiou '.split(''),
  /* MAGICK    */ 'xqzvjkwfpbdtlrmnyshcgaeio'.split(''),
  /* SHADOW    */ 'ssshhhzzzsssxxxxxssshhhzzz'.split(''),
  /* SPIRIT    */ 'oooaaaeeeiiioooaaaeeeiiiii'.split(''),
  /* ENLIGHTEN */ 'abcdefghijklmnopqrstuvwxyz'.split(''),
  /* SATANIC   */ 'zyxwvutsrqponmlkjihgfedcba'.split(''),
  /* ANIMAL    */ 'rgrgrgrgrgrgrgrgrgrgrgrgr '.split(''),
  /* BRETONNIN */ 'defghijklmnopqrstuvwxyzabc'.split(''),
  /* GARGISH   */ 'vwxyzabcdefghijklmnopqrstu'.split(''),
];

/**
 * garbleText -- Garble a spoken message based on proficiency.
 *
 * Ported from garble.c. If proficiency >= 100, the text is returned
 * unchanged. Otherwise, letters are replaced with language-specific
 * substitution characters based on the listener's understanding.
 * Punctuation, spaces, and digits are preserved.
 *
 * @param text        The original message.
 * @param proficiency Combined understanding (0..100). 100 = perfect.
 * @param langId      The language being spoken (selects substitution table).
 */
export function garbleText(
  text: string,
  proficiency: number,
  langId: number = 0,
): string {
  if (proficiency >= 100) return text;

  const subs = GARBLE_SUBS[langId] ?? GARBLE_SUBS[0];
  const result: string[] = [];

  for (const ch of text) {
    // Preserve spaces, punctuation, digits
    if (ch === ' ' || ch === '.' || ch === '!' || ch === '?'
        || ch === ',' || ch === ';' || ch === ':' || ch === '\''
        || ch === '"' || ch === '-' || (ch >= '0' && ch <= '9')) {
      result.push(ch);
      continue;
    }

    // If the character passes the proficiency check, it comes through
    if (Math.random() * 100 < proficiency) {
      result.push(ch);
      continue;
    }

    // Replace using the substitution table
    const lower = ch.toLowerCase();
    const idx = lower.charCodeAt(0) - 'a'.charCodeAt(0);
    if (idx >= 0 && idx < 26) {
      const replacement = subs[idx % subs.length];
      // Preserve case
      result.push(ch === ch.toUpperCase() ? replacement.toUpperCase() : replacement);
    } else {
      result.push(ch);
    }
  }

  return result.join('');
}

/**
 * garbleMessage -- Garble a spoken message for a listener who may not
 * know the language being spoken.
 *
 * If the speaker is using Common (id 0) or both speaker and listener know
 * the language well, the original message is returned.
 *
 * Otherwise characters are randomly replaced based on the listener's
 * proficiency in the language.
 */
export function garbleMessage(
  ch: CharData,
  listener: CharData,
  message: string,
): string {
  const langId = ch.speaking;

  // Common is universally understood
  if (langId === Language.COMMON) return message;

  // Speaker's own proficiency determines how well they articulate
  const speakerProf = ch.isNpc ? 100 : (ch.language[langId] ?? 0);
  const listenerProf = listener.isNpc ? 100 : (listener.language[langId] ?? 0);

  // Both know the language well -- no garbling
  if (speakerProf >= 80 && listenerProf >= 80) return message;

  // If the listener knows nothing, they hear complete gibberish
  if (listenerProf === 0) {
    return garbleText(message, 0, langId);
  }

  // Partial understanding -- garble based on average proficiency
  const understandChance = Math.floor(
    (speakerProf + listenerProf) / 2,
  );
  return garbleText(message, understandChance, langId);
}

/**
 * Return the language name for display purposes.
 */
export function getLanguageName(langId: number): string {
  return LANGUAGE_NAMES[langId] ?? 'Unknown';
}

// ============================================================================
//  Language commands
// ============================================================================

/**
 * doSpeak -- Set the character's currently spoken language.
 * Usage: speak <language>
 *
 * Ported from language.c do_speak. Checks that the player knows the
 * language via ch.language[langId] before allowing the switch.
 */
export function doSpeak(ch: CharData, argument: string): void {
  if (ch.position === Position.GHOST) {
    sendToChar(ch, `${colors.cyan}You can not speak anything but Spiritspeak while dead.\r\n${colors.reset}`);
    ch.speaking = Language.SPIRITSPEAK;
    return;
  }

  if (!argument) {
    sendToChar(ch, `${colors.cyan}You currently speak ${LANGUAGE_NAMES[ch.speaking] ?? 'Unknown'}.\r\n${colors.reset}`);
    return;
  }

  const langId = langLookup(argument);
  if (langId === -1) {
    sendToChar(ch, `${colors.cyan}${argument} is not a valid language!\r\n${colors.reset}`);
    return;
  }

  if (!ch.isNpc && ch.language[langId] === 0 && !isImmortal(ch)) {
    sendToChar(ch, `${colors.cyan}But you don't know how to speak ${LANGUAGE_NAMES[langId]}.\r\n${colors.reset}`);
    return;
  }

  ch.speaking = langId;
  sendToChar(ch, `${colors.cyan}You now speak ${LANGUAGE_NAMES[langId]}.\r\n${colors.reset}`);
}

/**
 * doLanguages -- List all languages the character knows, with proficiency.
 * Usage: languages
 *
 * Ported from language.c do_languages. Shows a table of Language — Proficiency%.
 */
export function doLanguages(ch: CharData, _argument: string): void {
  let buf = `${colors.white}Known Languages:\r\n`;
  buf += `${'Language'.padEnd(18)} ${'Proficiency'.padStart(11)}\r\n`;
  buf += `${'-'.repeat(30)}\r\n`;
  let col = 0;

  for (let ln = 0; ln < MAX_LANGUAGE && ln < LANGUAGE_NAMES.length; ln++) {
    if (ch.language[ln] > 0 || isImmortal(ch)) {
      const pct = ch.language[ln] ?? 0;
      const name = LANGUAGE_NAMES[ln].padEnd(18);
      buf += `  ${name} ${String(pct).padStart(3)}%  `;
      col++;
      if (col % 3 === 0) {
        buf += '\r\n';
      }
    }
  }
  if (col % 3 !== 0) {
    buf += '\r\n';
  }

  buf += `${colors.reset}`;
  sendToChar(ch, buf);
}

// ============================================================================
//  Clan commands (from act_clan.c)
// ============================================================================

/**
 * doClan -- Meta-command for clan operations.
 * Usage: clan list | clan info <name> | clan join <name> |
 *        clan resign | clan promote <player> | clan demote <player>
 */
export function doClan(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch,
      `${colors.white}Clan commands: list, info <name>, join <name>, resign, promote <player>, demote <player>\r\n${colors.reset}`,
    );
    return;
  }

  const parts = argument.trim().split(/\s+/);
  const sub = parts[0].toLowerCase();
  const rest = parts.slice(1).join(' ');

  switch (sub) {
    case 'list':    doClanList(ch); break;
    case 'info':    doClanInfo(ch, rest); break;
    case 'join':    doClanJoin(ch, rest); break;
    case 'resign':  doClanResign(ch); break;
    case 'promote': doClanPromote(ch, rest); break;
    case 'demote':  doClanDemote(ch, rest); break;
    default:
      sendToChar(ch,
        `${colors.white}Clan commands: list, info <name>, join <name>, resign, promote <player>, demote <player>\r\n${colors.reset}`,
      );
      break;
  }
}

/** List all clans with member counts. */
function doClanList(ch: CharData): void {
  if (world.clans.length === 0) {
    sendToChar(ch, 'There are no clans.\r\n');
    return;
  }

  let buf = `${colors.white}`;
  buf += `${'#'.padStart(3)}  ${'Clan'.padEnd(16)} ${'Leader'.padEnd(14)} ${'Members'.padStart(7)}  PK\r\n`;
  buf += `${'-'.repeat(55)}\r\n`;

  for (let i = 0; i < world.clans.length; i++) {
    const c = world.clans[i];
    buf += `${String(i + 1).padStart(3)}  ${c.name.padEnd(16)} ${c.leader.padEnd(14)} ${String(c.members).padStart(7)}  ${c.pkill ? 'Yes' : 'No'}\r\n`;
  }

  buf += colors.reset;
  sendToChar(ch, buf);
}

/**
 * doClanInfo -- Show detailed clan information.
 *
 * Standalone command ported from act_clan.c. When called with no argument,
 * lists all clans with stats (name, deity, members, pkills). When called
 * with an argument, shows detailed info for that specific clan.
 */
export function doClanInfo(ch: CharData, argument: string): void {
  if (!argument) {
    // List all clans with summary stats
    if (world.clans.length === 0) {
      sendToChar(ch, 'There are no clans.\r\n');
      return;
    }

    let buf = `${colors.white}`;
    buf += `${'#'.padStart(3)}  ${'Clan'.padEnd(16)} ${'Deity'.padEnd(14)} ${'Members'.padStart(7)} ${'PKills'.padStart(7)} ${'MKills'.padStart(7)}\r\n`;
    buf += `${'-'.repeat(62)}\r\n`;

    for (let i = 0; i < world.clans.length; i++) {
      const c = world.clans[i];
      buf += `${String(i + 1).padStart(3)}  ${c.name.padEnd(16)} ${(c.deity || 'None').padEnd(14)} ${String(c.members).padStart(7)} ${String(c.pkills).padStart(7)} ${String(c.mkills).padStart(7)}\r\n`;
    }

    buf += colors.reset;
    sendToChar(ch, buf);
    return;
  }

  // Detailed info for a specific clan
  const lower = argument.toLowerCase();
  const clan = world.clans.find(
    (c) => c.name.toLowerCase().startsWith(lower),
  );
  if (!clan) {
    sendToChar(ch, 'No such clan exists.\r\n');
    return;
  }

  let buf = `${colors.white}--- Clan: ${clan.name} ---\r\n`;
  buf += `${colors.cyan}Deity:       ${clan.deity || 'None'}\r\n`;
  buf += `${colors.cyan}Leader:      ${clan.leader}\r\n`;
  buf += `${colors.cyan}Champion:    ${clan.champ || 'None'}\r\n`;
  buf += `${colors.cyan}Members:     ${clan.members}\r\n`;
  buf += `${colors.cyan}PKills:      ${clan.pkills}\r\n`;
  buf += `${colors.cyan}PDeaths:     ${clan.pdeaths}\r\n`;
  buf += `${colors.cyan}MKills:      ${clan.mkills}\r\n`;
  buf += `${colors.cyan}MDeaths:     ${clan.mdeaths}\r\n`;
  if (clan.description) {
    buf += `${colors.white}Description:\r\n${clan.description}\r\n`;
  }
  buf += colors.reset;
  sendToChar(ch, buf);
}

/**
 * doClanChat -- Clan-only communication channel.
 *
 * Ported from act_comm.c CHANNEL_CLAN handling. Sends a message to all
 * online members of the same clan.
 */
export function doClanChat(ch: CharData, argument: string): void {
  if (ch.isNpc) {
    sendToChar(ch, 'NPCs cannot use clan chat.\r\n');
    return;
  }

  if (ch.clan <= 0 || ch.clan > world.clans.length) {
    sendToChar(ch, 'You are not in a clan.\r\n');
    return;
  }

  if (!argument) {
    sendToChar(ch, 'What do you want to tell your clan?\r\n');
    return;
  }

  const clan = world.clans[ch.clan - 1];

  // Show to sender
  sendToChar(ch,
    `${colors.red}<${clan.name}> You: '${argument}'${colors.reset}\r\n`,
  );

  // Broadcast to all online clan members
  for (const other of world.characters.values()) {
    if (other.isNpc || other.deleted) continue;
    if (other.id === ch.id) continue;
    if (other.clan !== ch.clan) continue;
    if (!hasConnection(other)) continue;

    sendToChar(other,
      `${colors.red}<${clan.name}> ${ch.name}: '${argument}'${colors.reset}\r\n`,
    );
  }
}

/** Request to join a clan. */
function doClanJoin(ch: CharData, argument: string): void {
  if (ch.isNpc) return;

  if (ch.clan > 0) {
    sendToChar(ch, 'You are already in a clan! Resign first.\r\n');
    return;
  }

  if (!argument) {
    sendToChar(ch, 'Join which clan?\r\n');
    return;
  }

  const lower = argument.toLowerCase();
  const idx = world.clans.findIndex(
    (c) => c.name.toLowerCase().startsWith(lower),
  );
  if (idx === -1) {
    sendToChar(ch, 'No such clan exists.\r\n');
    return;
  }

  ch.clan = idx + 1; // clans are 1-indexed (0 = no clan)
  ch.clev = 0;
  world.clans[idx].members++;

  sendToChar(ch, `${colors.green}You have joined ${world.clans[idx].name}!\r\n${colors.reset}`);
  act(`$n has joined ${world.clans[idx].name}.`, ch, null, null, TO_ROOM);
}

/** Leave current clan. */
function doClanResign(ch: CharData): void {
  if (ch.isNpc) return;

  if (ch.clan <= 0 || ch.clan > world.clans.length) {
    sendToChar(ch, 'You are not in a clan.\r\n');
    return;
  }

  const clan = world.clans[ch.clan - 1];
  clan.members = Math.max(0, clan.members - 1);

  sendToChar(ch, `${colors.yellow}You have resigned from ${clan.name}.\r\n${colors.reset}`);
  act(`$n has resigned from ${clan.name}.`, ch, null, null, TO_ROOM);

  ch.clan = 0;
  ch.clev = 0;
}

/** Promote a clan member (leader only). */
function doClanPromote(ch: CharData, argument: string): void {
  if (ch.isNpc) return;

  if (ch.clan <= 0 || ch.clan > world.clans.length) {
    sendToChar(ch, 'You are not in a clan.\r\n');
    return;
  }

  const clan = world.clans[ch.clan - 1];
  if (clan.leader.toLowerCase() !== ch.name.toLowerCase() && ch.clev < 4) {
    sendToChar(ch, 'Only the clan leader can promote members.\r\n');
    return;
  }

  if (!argument) {
    sendToChar(ch, 'Promote whom?\r\n');
    return;
  }

  const roomVnum = getCharRoom(ch);
  const victim = findCharInRoom(ch, argument, roomVnum);
  if (!victim) {
    sendToChar(ch, 'They aren\'t here.\r\n');
    return;
  }

  if (victim.clan !== ch.clan) {
    sendToChar(ch, 'They aren\'t in your clan!\r\n');
    return;
  }

  if (victim.clev >= 5) {
    sendToChar(ch, 'They are already at the highest clan rank.\r\n');
    return;
  }

  victim.clev++;
  sendToChar(ch, `${colors.green}You promote ${victim.name} to clan rank ${victim.clev}.\r\n${colors.reset}`);
  sendToChar(victim, `${colors.green}You have been promoted to clan rank ${victim.clev}!\r\n${colors.reset}`);
}

/** Demote a clan member (leader only). */
function doClanDemote(ch: CharData, argument: string): void {
  if (ch.isNpc) return;

  if (ch.clan <= 0 || ch.clan > world.clans.length) {
    sendToChar(ch, 'You are not in a clan.\r\n');
    return;
  }

  const clan = world.clans[ch.clan - 1];
  if (clan.leader.toLowerCase() !== ch.name.toLowerCase() && ch.clev < 4) {
    sendToChar(ch, 'Only the clan leader can demote members.\r\n');
    return;
  }

  if (!argument) {
    sendToChar(ch, 'Demote whom?\r\n');
    return;
  }

  const roomVnum = getCharRoom(ch);
  const victim = findCharInRoom(ch, argument, roomVnum);
  if (!victim) {
    sendToChar(ch, 'They aren\'t here.\r\n');
    return;
  }

  if (victim.clan !== ch.clan) {
    sendToChar(ch, 'They aren\'t in your clan!\r\n');
    return;
  }

  if (victim.clev <= 0) {
    sendToChar(ch, 'They are already at the lowest clan rank.\r\n');
    return;
  }

  victim.clev--;
  sendToChar(ch, `${colors.yellow}You demote ${victim.name} to clan rank ${victim.clev}.\r\n${colors.reset}`);
  sendToChar(victim, `${colors.yellow}You have been demoted to clan rank ${victim.clev}.\r\n${colors.reset}`);
}

// ============================================================================
//  Religion commands (from religion.c)
// ============================================================================

/**
 * doReligion -- Meta-command for religion operations.
 * Usage: religion list | religion info <number> |
 *        religion join <name> | religion abjure
 */
export function doReligion(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch,
      `${colors.white}Religion commands: list, info <number>, join <name>, abjure\r\n${colors.reset}`,
    );
    return;
  }

  const parts = argument.trim().split(/\s+/);
  const sub = parts[0].toLowerCase();
  const rest = parts.slice(1).join(' ');

  switch (sub) {
    case 'list':   doReligionList(ch); break;
    case 'info':   doReligionInfo(ch, rest); break;
    case 'join':   doReligionJoin(ch, rest); break;
    case 'abjure': doAbjure(ch); break;
    default:
      sendToChar(ch,
        `${colors.white}Religion commands: list, info <number>, join <name>, abjure\r\n${colors.reset}`,
      );
      break;
  }
}

/** List all religions -- ported from religion.c do_religions. */
function doReligionList(ch: CharData): void {
  if (world.religions.length === 0) {
    sendToChar(ch, 'There are no religions.\r\n');
    return;
  }

  let buf = `${colors.white}`;
  buf += `${'#'.padStart(3)}  ${'Name'.padEnd(6)} ${'Deity'.padEnd(14)} ${'Followers'.padStart(9)} ${'PKills'.padStart(7)} ${'PDeaths'.padStart(7)} ${'% Win'.padStart(6)}\r\n`;
  buf += `${'-'.repeat(60)}\r\n`;

  for (let i = 0; i < world.religions.length; i++) {
    const r = world.religions[i];
    let winPct = 0;
    if (r.pkills > 0 || r.pdeaths > 0) {
      winPct = Math.floor((r.pkills / (r.pkills + r.pdeaths)) * 100);
    }
    buf += `${String(i + 1).padStart(3)}  ${r.name.padEnd(6)} ${r.deity.padEnd(14)} ${String(r.members).padStart(9)} ${String(r.pkills).padStart(7)} ${String(r.pdeaths).padStart(7)} ${String(winPct).padStart(5)}%\r\n`;
  }

  buf += colors.reset;
  sendToChar(ch, buf);
}

/**
 * doReligions -- Standalone command to list all religions.
 *
 * Ported from religion.c do_religions. Shows religion name, deity, members,
 * pkills for all religions in the world.
 */
export function doReligions(ch: CharData, argument: string): void {
  if (argument) {
    // If an argument was given, show detailed info
    doReligionInfo(ch, argument);
    return;
  }
  doReligionList(ch);
}

/** Show info about a specific religion by number. */
function doReligionInfo(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, 'Syntax: religion info <religion number>\r\n');
    sendToChar(ch, 'Use "religion list" or "religions" to find religion numbers.\r\n');
    return;
  }

  const num = parseInt(argument, 10);
  if (isNaN(num) || num < 1 || num > world.religions.length) {
    sendToChar(ch, 'Invalid religion number.\r\n');
    return;
  }

  const r = world.religions[num - 1];
  let buf = `${colors.white}--- Information on <${r.name}> ---\r\n\r\n`;
  buf += `${colors.cyan}Name:        ${r.shortdesc || r.name}\r\n`;
  buf += `${colors.cyan}Deity:       ${r.deity}\r\n`;
  buf += `${colors.blue}Followers:   ${r.members}\r\n`;
  buf += `${colors.blue}PKills:      ${r.pkills}\r\n`;
  buf += `${colors.blue}PDeaths:     ${r.pdeaths}\r\n`;
  buf += `${colors.cyan}MKills:      ${r.mkills}\r\n`;
  buf += `${colors.cyan}MDeaths:     ${r.mdeaths}\r\n`;
  if (r.description) {
    buf += `${colors.white}Description:\r\n${r.description}\r\n`;
  }
  buf += colors.reset;
  sendToChar(ch, buf);
}

/** Join a religion. */
function doReligionJoin(ch: CharData, argument: string): void {
  if (ch.isNpc) return;

  if (ch.religion > 0) {
    sendToChar(ch, 'You already follow a religion. Use "religion abjure" to leave first.\r\n');
    return;
  }

  if (!argument) {
    sendToChar(ch, 'Join which religion? Use "religions" to see available religions.\r\n');
    return;
  }

  const num = parseInt(argument, 10);
  let idx = -1;

  if (!isNaN(num) && num >= 1 && num <= world.religions.length) {
    idx = num - 1;
  } else {
    const lower = argument.toLowerCase();
    idx = world.religions.findIndex(
      (r) => r.name.toLowerCase().startsWith(lower) ||
             r.deity.toLowerCase().startsWith(lower),
    );
  }

  if (idx === -1) {
    sendToChar(ch, 'No such religion exists.\r\n');
    return;
  }

  ch.religion = idx + 1; // religions are 1-indexed (0 = none)
  world.religions[idx].members++;

  sendToChar(ch, `${colors.brightCyan}You offer your prayers and join the religion of ${world.religions[idx].name}!\r\n${colors.reset}`);
  act(`$n has joined the religion of ${world.religions[idx].name}.`, ch, null, null, TO_ROOM);
}

/** Leave current religion. */
function doAbjure(ch: CharData): void {
  if (ch.isNpc) return;

  if (ch.religion <= 0 || ch.religion > world.religions.length) {
    sendToChar(ch, 'You do not follow a religion.\r\n');
    return;
  }

  const religion = world.religions[ch.religion - 1];
  religion.members = Math.max(0, religion.members - 1);

  sendToChar(ch, `${colors.yellow}You have abjured the religion of ${religion.name}.\r\n${colors.reset}`);
  act(`$n has abjured the religion of ${religion.name}.`, ch, null, null, TO_ROOM);

  ch.religion = 0;
}

/**
 * doPray -- Religion-only communication channel.
 *
 * Ported from religion.c / act_comm.c religion channel. Sends a prayer
 * message to all online members of the same religion.
 */
export function doPray(ch: CharData, argument: string): void {
  if (ch.isNpc) {
    sendToChar(ch, 'NPCs cannot pray to a religion channel.\r\n');
    return;
  }

  if (ch.religion <= 0 || ch.religion > world.religions.length) {
    sendToChar(ch, 'You do not follow a religion.\r\n');
    return;
  }

  if (!argument) {
    sendToChar(ch, 'What do you wish to pray?\r\n');
    return;
  }

  const religion = world.religions[ch.religion - 1];

  // Show to sender
  sendToChar(ch,
    `${colors.blue}<${religion.name}> You pray: '${argument}'${colors.reset}\r\n`,
  );

  // Broadcast to all online co-religionists
  for (const other of world.characters.values()) {
    if (other.isNpc || other.deleted) continue;
    if (other.id === ch.id) continue;
    if (other.religion !== ch.religion) continue;
    if (!hasConnection(other)) continue;

    sendToChar(other,
      `${colors.blue}<${religion.name}> ${ch.name} prays: '${argument}'${colors.reset}\r\n`,
    );
  }
}

// ============================================================================
//  Economy / Banking (from economy.c)
// ============================================================================

/** Share price for the in-game stock system. */
let shareValue = 150;

/** Get or set the current share price. */
export function getShareValue(): number { return shareValue; }
export function setShareValue(v: number): void { shareValue = Math.max(1, v); }

/**
 * doBank -- Banking meta-command.
 * Usage: bank balance | bank deposit <amt> | bank withdraw <amt> |
 *        bank transfer <amt> <player> | bank check |
 *        bank buy <shares> | bank sell <shares>
 *
 * Ported from economy.c. Requires a banker NPC in the room (checks
 * for 'bank' in the NPC name or short description).
 */
export function doBank(ch: CharData, argument: string): void {
  if (ch.isNpc) {
    sendToChar(ch, 'Banking services are only available to players!\r\n');
    return;
  }

  // Must be at a banker mob
  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) return;

  let hasBanker = false;
  for (const mob of world.getCharsInRoom(roomVnum)) {
    if (mob.isNpc && !mob.deleted) {
      if (mob.name.toLowerCase().includes('bank') ||
          mob.shortDescr.toLowerCase().includes('bank')) {
        hasBanker = true;
        break;
      }
    }
  }

  if (!hasBanker) {
    sendToChar(ch, 'You can\'t do that here. Find a banker.\r\n');
    return;
  }

  const pcdata = world.pcData.get(ch.id);
  if (!pcdata) return;

  if (!argument) {
    sendToChar(ch, `${colors.white}Bank Options:\r\n\r\n`);
    sendToChar(ch, 'Bank balance              -- Show your balance.\r\n');
    sendToChar(ch, 'Bank deposit <amount>     -- Deposit gold.\r\n');
    sendToChar(ch, 'Bank withdraw <amount>    -- Withdraw gold.\r\n');
    sendToChar(ch, 'Bank transfer <amt> <who> -- Transfer gold.\r\n');
    sendToChar(ch, 'Bank check                -- Check share prices.\r\n');
    sendToChar(ch, 'Bank buy <shares>         -- Buy shares.\r\n');
    sendToChar(ch, `Bank sell <shares>        -- Sell shares.\r\n${colors.reset}`);
    return;
  }

  const parts = argument.trim().split(/\s+/);
  const sub = parts[0].toLowerCase();

  if (sub === 'balance') {
    sendToChar(ch, `${colors.white}Your current balance is: ${pcdata.bankAccount} GP.\r\n${colors.reset}`);
    return;
  }

  if (sub === 'deposit') {
    const amount = parseInt(parts[1], 10);
    if (isNaN(amount) || amount <= 0) {
      sendToChar(ch, 'Deposit how much?\r\n');
      return;
    }
    if (amount > ch.gold) {
      sendToChar(ch, `How can you deposit ${amount} GP when you only have ${ch.gold}?\r\n`);
      return;
    }
    ch.gold -= amount;
    pcdata.bankAccount += amount;
    sendToChar(ch, `${colors.white}You deposit ${amount} GP. Your new balance is ${pcdata.bankAccount} GP.\r\n${colors.reset}`);
    sendVitals(ch);
    return;
  }

  if (sub === 'withdraw') {
    const amount = parseInt(parts[1], 10);
    if (isNaN(amount) || amount <= 0) {
      sendToChar(ch, 'Withdraw how much?\r\n');
      return;
    }
    if (amount > pcdata.bankAccount) {
      sendToChar(ch, `How can you withdraw ${amount} GP when your balance is ${pcdata.bankAccount}?\r\n`);
      return;
    }
    pcdata.bankAccount -= amount;
    ch.gold += amount;
    sendToChar(ch, `${colors.white}You withdraw ${amount} GP. Your new balance is ${pcdata.bankAccount} GP.\r\n${colors.reset}`);
    sendVitals(ch);
    return;
  }

  if (sub === 'transfer') {
    const amount = parseInt(parts[1], 10);
    const targetName = parts[2];
    if (isNaN(amount) || amount <= 0 || !targetName) {
      sendToChar(ch, 'Syntax: bank transfer <amount> <player>\r\n');
      return;
    }
    if (amount > pcdata.bankAccount) {
      sendToChar(ch, `How can you transfer ${amount} GP when your balance is ${pcdata.bankAccount}?\r\n`);
      return;
    }

    let victim: CharData | undefined;
    for (const other of world.characters.values()) {
      if (other.isNpc || other.deleted) continue;
      if (isName(targetName, other.name)) {
        victim = other;
        break;
      }
    }

    if (!victim) {
      sendToChar(ch, `${targetName} doesn't have a bank account.\r\n`);
      return;
    }

    const victimPc = world.pcData.get(victim.id);
    if (!victimPc) {
      sendToChar(ch, 'You can only transfer money to players.\r\n');
      return;
    }

    pcdata.bankAccount -= amount;
    victimPc.bankAccount += amount;
    sendToChar(ch, `${colors.white}You transfer ${amount} GP. Your new balance is ${pcdata.bankAccount} GP.\r\n${colors.reset}`);
    sendToChar(victim, `${colors.white}[BANK] ${ch.name} has transferred ${amount} gold to your account.\r\n${colors.reset}`);
    return;
  }

  if (sub === 'check') {
    let buf = `${colors.white}The current share price is ${shareValue} GP.`;
    if (pcdata.shares > 0) {
      buf += `  You have ${pcdata.shares} shares worth ${pcdata.shares * shareValue} gold total.`;
    }
    buf += `\r\n${colors.reset}`;
    sendToChar(ch, buf);
    return;
  }

  if (sub === 'buy') {
    const amount = parseInt(parts[1], 10);
    if (isNaN(amount) || amount <= 0) {
      sendToChar(ch, 'Buy how many shares?\r\n');
      return;
    }
    const cost = amount * shareValue;
    if (cost > pcdata.bankAccount) {
      sendToChar(ch, `${amount} shares will cost you ${cost} GP. You don't have enough in your account.\r\n`);
      return;
    }
    if (pcdata.shares + amount > 500000) {
      sendToChar(ch, 'You can only have 500,000 shares.\r\n');
      return;
    }
    pcdata.bankAccount -= cost;
    pcdata.shares += amount;
    sendToChar(ch, `${colors.white}You buy ${amount} shares for ${cost} GP. You now have ${pcdata.shares} shares.\r\n${colors.reset}`);
    return;
  }

  if (sub === 'sell') {
    const amount = parseInt(parts[1], 10);
    if (isNaN(amount) || amount <= 0) {
      sendToChar(ch, 'Sell how many shares?\r\n');
      return;
    }
    if (amount > pcdata.shares) {
      sendToChar(ch, `You only have ${pcdata.shares} shares.\r\n`);
      return;
    }
    const revenue = amount * shareValue;
    pcdata.bankAccount += revenue;
    pcdata.shares -= amount;
    sendToChar(ch, `${colors.white}You sell ${amount} shares for ${revenue} GP. You now have ${pcdata.shares} shares.\r\n${colors.reset}`);
    return;
  }

  sendToChar(ch, 'I don\'t know what you mean.\r\n');
  doBank(ch, '');
}

// ============================================================================
//  Marriage (from marriage.c -- immortal commands)
// ============================================================================

/**
 * doMarry -- Marry two players (immortal command).
 *
 * Ported from marriage.c do_marry. An immortal-level command that
 * takes two player names and sets pcData.spouse on both.
 * Syntax: marry <player1> <player2>
 */
export function doMarry(ch: CharData, argument: string): void {
  if (!isImmortal(ch)) {
    sendToChar(ch, 'Only immortals can perform marriages.\r\n');
    return;
  }

  if (!argument) {
    sendToChar(ch, 'Syntax: marry <player1> <player2>\r\n');
    return;
  }

  const parts = argument.trim().split(/\s+/);
  if (parts.length < 2) {
    sendToChar(ch, 'Syntax: marry <player1> <player2>\r\n');
    return;
  }

  const name1 = parts[0];
  const name2 = parts[1];

  // Find both players online
  let victim1: CharData | undefined;
  let victim2: CharData | undefined;

  for (const other of world.characters.values()) {
    if (other.isNpc || other.deleted) continue;
    if (!victim1 && isName(name1, other.name)) {
      victim1 = other;
    } else if (!victim2 && isName(name2, other.name)) {
      victim2 = other;
    }
    if (victim1 && victim2) break;
  }

  if (!victim1) {
    sendToChar(ch, `${name1} is not online.\r\n`);
    return;
  }

  if (!victim2) {
    sendToChar(ch, `${name2} is not online.\r\n`);
    return;
  }

  if (victim1.id === victim2.id) {
    sendToChar(ch, 'A person cannot marry themselves!\r\n');
    return;
  }

  const pc1 = world.pcData.get(victim1.id);
  const pc2 = world.pcData.get(victim2.id);

  if (!pc1 || !pc2) {
    sendToChar(ch, 'Both players must have valid player data.\r\n');
    return;
  }

  if (pc1.spouse) {
    sendToChar(ch, `${victim1.name} is already married to ${pc1.spouse}.\r\n`);
    return;
  }

  if (pc2.spouse) {
    sendToChar(ch, `${victim2.name} is already married to ${pc2.spouse}.\r\n`);
    return;
  }

  // Perform the marriage
  pc1.spouse = victim2.name;
  pc2.spouse = victim1.name;

  sendToChar(ch,
    `${colors.brightMagenta}You join ${victim1.name} and ${victim2.name} in holy matrimony!${colors.reset}\r\n`,
  );
  sendToChar(victim1,
    `${colors.brightMagenta}${ch.name} has married you to ${victim2.name}!${colors.reset}\r\n`,
  );
  sendToChar(victim2,
    `${colors.brightMagenta}${ch.name} has married you to ${victim1.name}!${colors.reset}\r\n`,
  );
  sendToAll(
    `${colors.brightMagenta}${victim1.name} and ${victim2.name} have been joined in marriage by ${ch.name}!${colors.reset}\r\n`,
  );
}

/**
 * doDivorce -- Divorce two players (immortal command).
 *
 * Ported from marriage.c do_divorce. An immortal-level command that
 * clears the spouse field on both players. Takes one player name
 * and automatically clears the spouse of both parties.
 * Syntax: divorce <player>
 */
export function doDivorce(ch: CharData, argument: string): void {
  if (!isImmortal(ch)) {
    sendToChar(ch, 'Only immortals can grant divorces.\r\n');
    return;
  }

  if (!argument) {
    sendToChar(ch, 'Syntax: divorce <player>\r\n');
    return;
  }

  const targetName = argument.trim().split(/\s+/)[0];

  // Find the player
  let victim: CharData | undefined;
  for (const other of world.characters.values()) {
    if (other.isNpc || other.deleted) continue;
    if (isName(targetName, other.name)) {
      victim = other;
      break;
    }
  }

  if (!victim) {
    sendToChar(ch, `${targetName} is not online.\r\n`);
    return;
  }

  const pcdata = world.pcData.get(victim.id);
  if (!pcdata) {
    sendToChar(ch, 'Cannot find player data.\r\n');
    return;
  }

  if (!pcdata.spouse) {
    sendToChar(ch, `${victim.name} is not married.\r\n`);
    return;
  }

  const spouseName = pcdata.spouse;

  // Find the spouse online and clear their side too
  for (const other of world.characters.values()) {
    if (other.isNpc || other.deleted) continue;
    if (other.name.toLowerCase() === spouseName.toLowerCase()) {
      const otherPc = world.pcData.get(other.id);
      if (otherPc) {
        otherPc.spouse = '';
        sendToChar(other,
          `${colors.yellow}${ch.name} has granted a divorce between you and ${victim.name}.${colors.reset}\r\n`,
        );
      }
      break;
    }
  }

  pcdata.spouse = '';

  sendToChar(ch,
    `${colors.yellow}You have granted a divorce between ${victim.name} and ${spouseName}.${colors.reset}\r\n`,
  );
  sendToChar(victim,
    `${colors.yellow}${ch.name} has granted a divorce between you and ${spouseName}.${colors.reset}\r\n`,
  );
  sendToAll(
    `${colors.yellow}${victim.name} and ${spouseName} have been divorced by ${ch.name}.${colors.reset}\r\n`,
  );
}

// ============================================================================
//  Gambling (from games.c)
// ============================================================================

/** Slot machine symbols. */
const SLOT_SYMBOLS = ['Cherry', 'Lemon', 'Orange', 'Bell', 'Bar', 'Seven', 'Diamond'];

/** Colored ASCII art representations of slot symbols. */
const SLOT_ART: Record<string, string> = {
  Cherry:  `${colors.red}[Cherry ]${colors.reset}`,
  Lemon:   `${colors.yellow}[Lemon  ]${colors.reset}`,
  Orange:  `${colors.brightYellow}[Orange ]${colors.reset}`,
  Bell:    `${colors.brightCyan}[Bell   ]${colors.reset}`,
  Bar:     `${colors.white}[  BAR  ]${colors.reset}`,
  Seven:   `${colors.brightRed}[  777  ]${colors.reset}`,
  Diamond: `${colors.brightMagenta}[Diamond]${colors.reset}`,
};

/**
 * doSlots -- Play the slot machine.
 * Usage: slots <bet>
 *
 * Ported from games.c. Spins three reels; three of a kind is a jackpot,
 * two of a kind pays double, otherwise you lose.
 */
export function doSlots(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, 'Syntax: slots <bet amount>\r\n');
    return;
  }

  const bet = parseInt(argument, 10);
  if (isNaN(bet) || bet <= 0) {
    sendToChar(ch, 'You must bet a positive amount.\r\n');
    return;
  }

  if (bet > 1000) {
    sendToChar(ch, 'The maximum bet is 1,000 gold.\r\n');
    return;
  }

  if (bet > ch.gold) {
    sendToChar(ch, 'You don\'t have that much gold!\r\n');
    return;
  }

  ch.gold -= bet;

  // Spin three reels
  const reel1 = SLOT_SYMBOLS[Math.floor(Math.random() * SLOT_SYMBOLS.length)];
  const reel2 = SLOT_SYMBOLS[Math.floor(Math.random() * SLOT_SYMBOLS.length)];
  const reel3 = SLOT_SYMBOLS[Math.floor(Math.random() * SLOT_SYMBOLS.length)];

  // Display the slot machine
  let buf = '';
  buf += `${colors.brightYellow}\r\n`;
  buf += `  +-----------+-----------+-----------+\r\n`;
  buf += `  | ${SLOT_ART[reel1]}${colors.brightYellow} | ${SLOT_ART[reel2]}${colors.brightYellow} | ${SLOT_ART[reel3]}${colors.brightYellow} |\r\n`;
  buf += `  +-----------+-----------+-----------+\r\n`;
  buf += `${colors.reset}\r\n`;

  sendToChar(ch, buf);
  act('$n pulls the lever on the slot machine!', ch, null, null, TO_ROOM);

  // Calculate winnings
  if (reel1 === reel2 && reel2 === reel3) {
    // Three of a kind -- jackpot!
    let multiplier = 10;
    if (reel1 === 'Seven') multiplier = 50;
    if (reel1 === 'Diamond') multiplier = 25;

    const winnings = bet * multiplier;
    ch.gold += winnings;
    sendToChar(ch,
      `${colors.brightGreen}*** JACKPOT! Three ${reel1}s! You win ${winnings} gold! ***${colors.reset}\r\n`,
    );
    act('$n hits the JACKPOT on the slot machine!', ch, null, null, TO_ROOM);
  } else if (reel1 === reel2 || reel2 === reel3 || reel1 === reel3) {
    // Two of a kind
    const winnings = bet * 2;
    ch.gold += winnings;
    sendToChar(ch,
      `${colors.green}Two of a kind! You win ${winnings} gold!${colors.reset}\r\n`,
    );
  } else {
    sendToChar(ch,
      `${colors.red}No match. You lose ${bet} gold. Better luck next time!${colors.reset}\r\n`,
    );
  }

  sendVitals(ch);
}

/**
 * doDice -- Roll dice game (simplified craps).
 * Usage: dice <bet>
 *
 * Ported from games.c. Roll 2d6 vs the house (also rolls 2d6).
 * Higher roll wins and pays 2x the bet. Tie goes to the house.
 */
export function doDice(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, 'Syntax: dice <bet amount>\r\n');
    return;
  }

  const bet = parseInt(argument, 10);
  if (isNaN(bet) || bet <= 0) {
    sendToChar(ch, 'You must bet a positive amount.\r\n');
    return;
  }

  if (bet > 1000) {
    sendToChar(ch, 'The maximum bet is 1,000 gold.\r\n');
    return;
  }

  if (bet > ch.gold) {
    sendToChar(ch, 'You don\'t have that much gold!\r\n');
    return;
  }

  ch.gold -= bet;

  // Player rolls 2d6
  const playerDie1 = rollDice(1, 6);
  const playerDie2 = rollDice(1, 6);
  const playerTotal = playerDie1 + playerDie2;

  // House rolls 2d6
  const houseDie1 = rollDice(1, 6);
  const houseDie2 = rollDice(1, 6);
  const houseTotal = houseDie1 + houseDie2;

  sendToChar(ch,
    `${colors.brightYellow}You roll the dice: ${playerDie1} and ${playerDie2}, totalling ${playerTotal}.${colors.reset}\r\n`,
  );
  sendToChar(ch,
    `${colors.yellow}The house rolls: ${houseDie1} and ${houseDie2}, totalling ${houseTotal}.${colors.reset}\r\n`,
  );
  act('$n rolls the dice!', ch, null, null, TO_ROOM);

  if (playerTotal > houseTotal) {
    const winnings = bet * 2;
    ch.gold += winnings;
    sendToChar(ch,
      `${colors.brightGreen}You win ${winnings} gold!${colors.reset}\r\n`,
    );
  } else if (playerTotal === houseTotal) {
    sendToChar(ch,
      `${colors.yellow}It's a tie! The house wins. You lose ${bet} gold.${colors.reset}\r\n`,
    );
  } else {
    sendToChar(ch,
      `${colors.red}The house wins! You lose ${bet} gold!${colors.reset}\r\n`,
    );
  }

  sendVitals(ch);
}

/**
 * doSeven -- Under-and-over-seven dice game.
 * Usage: seven <bet> <under|over|seven>
 */
export function doSeven(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, 'Syntax: seven <bet> <under|over|seven>\r\n');
    return;
  }

  const parts = argument.trim().split(/\s+/);
  if (parts.length < 2) {
    sendToChar(ch, 'Syntax: seven <bet> <under|over|seven>\r\n');
    return;
  }

  const bet = parseInt(parts[0], 10);
  const choice = parts[1].toLowerCase();

  if (isNaN(bet) || bet <= 0) {
    sendToChar(ch, 'You must bet a positive amount.\r\n');
    return;
  }

  if (bet > 1000) {
    sendToChar(ch, 'The maximum bet is 1,000 gold.\r\n');
    return;
  }

  if (bet > ch.gold) {
    sendToChar(ch, 'You don\'t have that much gold!\r\n');
    return;
  }

  let ichoice: number;
  if (choice === 'under' || choice === '-') {
    ichoice = 1;
  } else if (choice === 'over' || choice === '+') {
    ichoice = 2;
  } else if (choice === 'seven' || choice === '7') {
    ichoice = 3;
  } else {
    sendToChar(ch, 'What do you wish to bet: under(-), over(+), or seven(7)?\r\n');
    return;
  }

  ch.gold -= bet;

  const die1 = rollDice(1, 6);
  const die2 = rollDice(1, 6);
  const total = die1 + die2;

  sendToChar(ch,
    `${colors.brightYellow}You place ${bet} gold on '${choice}'. The dice roll: ${die1} and ${die2}, totalling ${total}.${colors.reset}\r\n`,
  );
  act('$n plays an under-over-seven dice game!', ch, null, null, TO_ROOM);

  if (total === 7) {
    if (ichoice === 3) {
      const winnings = bet * 5;
      ch.gold += winnings;
      sendToChar(ch,
        `${colors.brightGreen}It's a SEVEN! You win ${winnings} gold!${colors.reset}\r\n`,
      );
    } else {
      sendToChar(ch,
        `${colors.red}It's a SEVEN! You lose!${colors.reset}\r\n`,
      );
    }
  } else if ((total < 7 && ichoice === 1) || (total > 7 && ichoice === 2)) {
    const winnings = bet * 2;
    ch.gold += winnings;
    sendToChar(ch,
      `${colors.brightGreen}You win ${winnings} gold!${colors.reset}\r\n`,
    );
  } else {
    sendToChar(ch,
      `${colors.red}Sorry, better luck next time!${colors.reset}\r\n`,
    );
  }

  sendVitals(ch);
}

/**
 * doHighDice -- High dice game: player vs croupier, each roll 2d6,
 * higher total wins (tie goes to the house).
 * Usage: highdice <bet>
 */
export function doHighDice(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, 'Syntax: highdice <bet amount>\r\n');
    return;
  }

  const bet = parseInt(argument, 10);
  if (isNaN(bet) || bet <= 0) {
    sendToChar(ch, 'You must bet a positive amount.\r\n');
    return;
  }

  if (bet > 1000) {
    sendToChar(ch, 'The maximum bet is 1,000 gold.\r\n');
    return;
  }

  if (bet > ch.gold) {
    sendToChar(ch, 'You don\'t have that much gold!\r\n');
    return;
  }

  ch.gold -= bet;

  const die1 = rollDice(1, 6);
  const die2 = rollDice(1, 6);
  const playerTotal = die1 + die2;

  const die3 = rollDice(1, 6);
  const die4 = rollDice(1, 6);
  const houseTotal = die3 + die4;

  const taunts = [
    'Roll some dice!',
    'Let\'s see those dice!',
    'You\'d better let me win!',
    'Here\'s some cash, now roll \'em!',
    'Today\'s my lucky day...',
  ];
  const taunt = taunts[Math.floor(Math.random() * taunts.length)];

  sendToChar(ch,
    `${colors.brightYellow}You place ${bet} gold on the table. "${taunt}"${colors.reset}\r\n`,
  );
  sendToChar(ch,
    `${colors.yellow}Your dice: ${die1} and ${die2}, totalling ${playerTotal}.${colors.reset}\r\n`,
  );
  sendToChar(ch,
    `${colors.yellow}House dice: ${die3} and ${die4}, totalling ${houseTotal}.${colors.reset}\r\n`,
  );
  act('$n plays a high dice game!', ch, null, null, TO_ROOM);

  if (playerTotal > houseTotal) {
    const winnings = bet * 2;
    ch.gold += winnings;
    sendToChar(ch,
      `${colors.brightGreen}You win ${winnings} gold!${colors.reset}\r\n`,
    );
  } else {
    sendToChar(ch,
      `${colors.red}Sorry, better luck next time!${colors.reset}\r\n`,
    );
  }

  sendVitals(ch);
}

// ============================================================================
//  Helper: find a character in a room by name
// ============================================================================

function findCharInRoom(
  ch: CharData,
  name: string,
  roomVnum: number,
): CharData | undefined {
  if (roomVnum === -1) return undefined;
  for (const other of world.getCharsInRoom(roomVnum)) {
    if (other.deleted || other.id === ch.id) continue;
    if (isName(name, other.name)) return other;
  }
  return undefined;
}

// ============================================================================
//  Exported command table entries
// ============================================================================

export const socialCommands: CommandEntry[] = [
  // Language
  { name: 'speak',      fn: doSpeak,      minPosition: Position.RESTING,  minLevel: 0,   log: 0 },
  { name: 'languages',  fn: doLanguages,  minPosition: Position.DEAD,     minLevel: 0,   log: 0 },

  // Clan
  { name: 'clan',       fn: doClan,       minPosition: Position.RESTING,  minLevel: 0,   log: 0 },
  { name: 'claninfo',   fn: doClanInfo,   minPosition: Position.DEAD,     minLevel: 0,   log: 0 },
  { name: 'clanchat',   fn: doClanChat,   minPosition: Position.RESTING,  minLevel: 0,   log: 0 },

  // Religion
  { name: 'religion',   fn: doReligion,   minPosition: Position.RESTING,  minLevel: 0,   log: 0 },
  { name: 'religions',  fn: doReligions,  minPosition: Position.DEAD,     minLevel: 0,   log: 0 },
  { name: 'pray',       fn: doPray,       minPosition: Position.RESTING,  minLevel: 0,   log: 0 },

  // Economy / Banking
  { name: 'bank',       fn: doBank,       minPosition: Position.STANDING, minLevel: 0,   log: 0 },

  // Marriage (immortal commands)
  { name: 'marry',      fn: doMarry,      minPosition: Position.DEAD,     minLevel: 108, log: 1 },
  { name: 'divorce',    fn: doDivorce,    minPosition: Position.DEAD,     minLevel: 108, log: 1 },

  // Gambling
  { name: 'slots',      fn: doSlots,      minPosition: Position.STANDING, minLevel: 0,   log: 0 },
  { name: 'dice',       fn: doDice,       minPosition: Position.STANDING, minLevel: 0,   log: 0 },
  { name: 'seven',      fn: doSeven,      minPosition: Position.RESTING,  minLevel: 0,   log: 0 },
  { name: 'highdice',   fn: doHighDice,   minPosition: Position.RESTING,  minLevel: 0,   log: 0 },
];
