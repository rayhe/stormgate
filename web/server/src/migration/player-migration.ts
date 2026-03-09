/**
 * player-migration.ts — Import legacy C MUD player save files into Firestore.
 *
 * Reads the text-based player files produced by fwrite_char / fwrite_obj in
 * save.c (Diku/Merc/Envy lineage) and converts them to the web version's
 * CharData + PcData Firestore document format.
 *
 * Usage:
 *   npx tsx src/migration/player-migration.ts /path/to/player/directory
 *
 * If no directory is given, defaults to ../../player (relative to this file's
 * location at web/server/src/migration/).
 *
 * Player files may be gzip-compressed (the C server compresses them on logout).
 * This script detects gzip magic bytes and decompresses transparently.
 */

import { readFileSync, readdirSync, existsSync, statSync } from 'fs';
import { join, resolve, dirname } from 'path';
import { fileURLToPath } from 'url';
import { gunzipSync } from 'zlib';
import { createHash } from 'crypto';
import { initializeApp, cert, type ServiceAccount } from 'firebase-admin/app';
import { getFirestore, type WriteBatch } from 'firebase-admin/firestore';

// ---------------------------------------------------------------------------
// Resolve paths
// ---------------------------------------------------------------------------

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// ---------------------------------------------------------------------------
// Types for the parsed legacy player data
// ---------------------------------------------------------------------------

/** A single affect (spell/skill effect) from the legacy save file. */
export interface LegacyAffect {
  /** Slot number (saved as slot, must be slot_lookup'd for real sn) */
  slot: number;
  duration: number;
  modifier: number;
  location: number;
  bitvector: number;
}

/** A single object from #OBJECT or #STORAGE sections. */
export interface LegacyObject {
  nest: number;
  name: string;
  shortDescr: string;
  description: string;
  vnum: number;
  durabilityMax: number;
  durabilityCur: number;
  extraFlags: number;
  extraFlags2: number;
  extraFlags3: number;
  extraFlags4: number;
  wearFlags: number;
  wearLoc: number;
  itemType: number;
  weight: number;
  level: number;
  timer: number;
  cost: number;
  values: [number, number, number, number];
  acType: number;
  acVnum: number;
  acCharge: [number, number];
  acSpell: string;
  affects: LegacyAffect[];
  extraDescriptions: Array<{ keyword: string; description: string }>;
  spells: Array<{ index: number; name: string }>;
  isStorage: boolean;
}

/** A pet from the #PET section. */
export interface LegacyPet {
  vnum: number;
  hit: number;
  maxHit: number;
  act: number;
  affectedBy: number;
  affectedBy2: number;
  affectedBy3: number;
  affectedBy4: number;
}

/** Fully parsed legacy player file. */
export interface LegacyPlayer {
  // Core identity
  name: string;
  lname: string;
  shortDescr: string;
  longDescr: string;
  description: string;
  prompt: string;
  sex: number;
  charClass: number;
  multied: number;
  race: string;
  clan: number;
  clev: number;
  ctimer: number;
  updated: number;

  // Level / trust
  level: number;
  trust: number;
  pkill: number;
  poisonLevel: number;
  antidisarm: number;
  mounted: number;
  mountcharmed: number;
  mountshort: string;
  wizinvis: number;
  wizbit: number;
  security: number;

  // Arena
  awins: number;
  alosses: number;
  mobkills: number;

  // Played time
  played: number;
  lastNote: number;

  // Room
  room: number;

  // Vitals
  hit: number;
  maxHit: number;
  mana: number;
  maxMana: number;
  move: number;
  maxMove: number;
  bp: number;
  maxBp: number;

  // Resources
  gold: number;
  guildRank: number;
  guild: string;
  religion: number;
  recall: number;
  rtimer: number;
  exp: number;

  // Bitvectors
  act: number;
  act2: number;
  affectedBy: number;
  affectedBy2: number;
  affectedBy3: number;
  affectedBy4: number;
  affectedByPowers: number;
  affectedByWeaknesses: number;
  immFlags: number;
  resFlags: number;
  vulFlags: number;
  shields: number;

  // Position
  position: number;

  // Practice / saves
  practice: number;
  savingThrow: number;
  alignment: number;
  hitroll: number;
  damroll: number;
  armor: number;
  wimpy: number;
  deaf: number;
  corpses: number;

  // Quest
  questpoints: number;
  nextquest: number;
  countdown: number;
  questobj: number;
  questmob: number;
  rquestpoints: number;
  rnextquest: number;

  // Damage mods (20 entries)
  damageMods: number[];

  // Stunned (5 entries)
  stunned: number[];

  // PC-only data
  pwd: string;
  speaking: number;
  learn: number;
  bamfin: string;
  bamfout: string;
  bamfsin: string;
  bamfsout: string;
  bankAccount: number;
  shares: number;
  immskll: string;
  title: string;
  whoText: string;
  plan: string;
  email: string;
  pagelen: number;

  // Stats
  permStr: number;
  permInt: number;
  permWis: number;
  permDex: number;
  permCon: number;
  modStr: number;
  modInt: number;
  modWis: number;
  modDex: number;
  modCon: number;

  // Condition
  condition: [number, number, number];

  // Languages: array of { index, name, value }
  languages: Array<{ index: number; name: string; value: number }>;

  // Skills: { name, value }
  skills: Array<{ name: string; value: number }>;

  // Affects (per group)
  affects: LegacyAffect[];
  affects2: LegacyAffect[];
  affects3: LegacyAffect[];
  affects4: LegacyAffect[];
  affectsPowers: LegacyAffect[];
  affectsWeaknesses: LegacyAffect[];

  // Aliases
  aliases: Array<{ shortcut: string; expansion: string }>;

  // Objects
  objects: LegacyObject[];
  storage: LegacyObject[];

  // Pet
  pet: LegacyPet | null;
}

// ---------------------------------------------------------------------------
// Stateful reader for the player file format
// ---------------------------------------------------------------------------

class PlayerReader {
  private pos = 0;
  private content: string;

  constructor(content: string) {
    this.content = content;
  }

  get eof(): boolean {
    return this.pos >= this.content.length;
  }

  peek(): string {
    return this.content[this.pos] ?? '';
  }

  /**
   * Read the next non-whitespace word. Skips leading whitespace.
   * Mimics fread_word from the C server.
   */
  readWord(): string {
    this.skipWhitespace();
    // If the word starts with a quote, read until closing quote
    if (this.peek() === "'" || this.peek() === '"') {
      const quote = this.content[this.pos];
      this.pos++; // consume opening quote
      const start = this.pos;
      while (this.pos < this.content.length && this.content[this.pos] !== quote) {
        this.pos++;
      }
      const word = this.content.slice(start, this.pos);
      if (this.pos < this.content.length) this.pos++; // consume closing quote
      return word;
    }
    const start = this.pos;
    while (this.pos < this.content.length && !/\s/.test(this.content[this.pos])) {
      this.pos++;
    }
    return this.content.slice(start, this.pos);
  }

  /**
   * Read a number (fread_number equivalent). Handles negative numbers.
   */
  readNumber(): number {
    this.skipWhitespace();
    let sign = 1;
    if (this.peek() === '+') {
      this.pos++;
    } else if (this.peek() === '-') {
      sign = -1;
      this.pos++;
    }
    let num = 0;
    let hasDigit = false;
    while (this.pos < this.content.length && /[0-9]/.test(this.content[this.pos])) {
      num = num * 10 + parseInt(this.content[this.pos], 10);
      this.pos++;
      hasDigit = true;
    }
    if (!hasDigit) {
      // Try to parse the next word as a number
      const word = this.readWord();
      const parsed = parseInt(word, 10);
      return isNaN(parsed) ? 0 : parsed;
    }
    // The C fread_number also handles | for bitwise OR composition
    this.skipWhitespace();
    while (this.peek() === '|') {
      this.pos++; // consume '|'
      this.skipWhitespace();
      let next = 0;
      while (this.pos < this.content.length && /[0-9]/.test(this.content[this.pos])) {
        next = next * 10 + parseInt(this.content[this.pos], 10);
        this.pos++;
      }
      num |= next;
      this.skipWhitespace();
    }
    return sign * num;
  }

  /**
   * Read a tilde-terminated string (fread_string equivalent).
   * The C server reads chars until it hits '~', then skips to eol.
   */
  readString(): string {
    this.skipWhitespace();
    const parts: string[] = [];
    while (this.pos < this.content.length) {
      const ch = this.content[this.pos];
      if (ch === '~') {
        this.pos++; // consume tilde
        // skip to end of line
        this.skipToEol();
        break;
      }
      parts.push(ch);
      this.pos++;
    }
    // Trim trailing whitespace but preserve internal newlines
    return parts.join('').replace(/\s+$/, '');
  }

  /**
   * Read the first non-whitespace character (fread_letter equivalent).
   */
  readLetter(): string {
    this.skipWhitespace();
    if (this.pos < this.content.length) {
      return this.content[this.pos++];
    }
    return '';
  }

  /**
   * Skip to end of current line (fread_to_eol equivalent).
   */
  skipToEol(): void {
    while (this.pos < this.content.length && this.content[this.pos] !== '\n') {
      this.pos++;
    }
    if (this.pos < this.content.length) this.pos++; // consume '\n'
  }

  /**
   * Skip whitespace characters.
   */
  skipWhitespace(): void {
    while (this.pos < this.content.length && /[ \t\r\n]/.test(this.content[this.pos])) {
      this.pos++;
    }
  }
}

// ---------------------------------------------------------------------------
// Language name table (mirrors lang_table in the C server)
// ---------------------------------------------------------------------------

const LANG_NAMES = [
  'common', 'human', 'dwarvish', 'elvish', 'gnomish',
  'dragon', 'demon', 'ogre', 'drow', 'elder',
  'pixie', 'hobbit', 'minotaur', 'lizard', 'halfling',
  'feline', 'canine', 'angel', 'orcish', 'magick',
  'shadow speak', 'spiritspeak', 'enlightened', 'satanic', 'animalspeak',
  'bretonnian', 'gargish',
];

function langLookup(name: string): number {
  const lower = name.toLowerCase().replace(/'/g, '');
  for (let i = 0; i < LANG_NAMES.length; i++) {
    if (LANG_NAMES[i] === lower) return i;
  }
  return -1;
}

// ---------------------------------------------------------------------------
// File reader utility (handles gzip)
// ---------------------------------------------------------------------------

/**
 * Read a player file, decompressing gzip if needed.
 * Returns the file content as a UTF-8 string.
 */
export function readPlayerFile(filePath: string): string {
  const raw = readFileSync(filePath);
  // Check for gzip magic bytes: 0x1f 0x8b
  if (raw.length >= 2 && raw[0] === 0x1f && raw[1] === 0x8b) {
    const decompressed = gunzipSync(raw);
    return decompressed.toString('utf-8');
  }
  return raw.toString('utf-8');
}

// ---------------------------------------------------------------------------
// Player file parser
// ---------------------------------------------------------------------------

function createDefaultPlayer(): LegacyPlayer {
  return {
    name: '',
    lname: '',
    shortDescr: '',
    longDescr: '',
    description: '',
    prompt: '<%hhp %mm %vmv> ',
    sex: 0,
    charClass: 0,
    multied: 0,
    race: '',
    clan: 0,
    clev: 0,
    ctimer: 0,
    updated: 0,
    level: 1,
    trust: 0,
    pkill: 0,
    poisonLevel: 0,
    antidisarm: 0,
    mounted: 0,
    mountcharmed: 0,
    mountshort: '',
    wizinvis: 0,
    wizbit: 0,
    security: 0,
    awins: 0,
    alosses: 0,
    mobkills: 0,
    played: 0,
    lastNote: 0,
    room: 25000,
    hit: 100,
    maxHit: 100,
    mana: 100,
    maxMana: 100,
    move: 100,
    maxMove: 100,
    bp: 0,
    maxBp: 0,
    gold: 0,
    guildRank: 0,
    guild: '',
    religion: 0,
    recall: 25000,
    rtimer: 0,
    exp: 0,
    act: 0,
    act2: 0,
    affectedBy: 0,
    affectedBy2: 0,
    affectedBy3: 0,
    affectedBy4: 0,
    affectedByPowers: 0,
    affectedByWeaknesses: 0,
    immFlags: 0,
    resFlags: 0,
    vulFlags: 0,
    shields: 0,
    position: 8, // STANDING
    practice: 0,
    savingThrow: 0,
    alignment: 0,
    hitroll: 0,
    damroll: 0,
    armor: 100,
    wimpy: 0,
    deaf: 0,
    corpses: 0,
    questpoints: 0,
    nextquest: 0,
    countdown: 0,
    questobj: 0,
    questmob: 0,
    rquestpoints: 0,
    rnextquest: 0,
    damageMods: new Array(20).fill(0),
    stunned: new Array(5).fill(0),
    pwd: '',
    speaking: 0,
    learn: 0,
    bamfin: '',
    bamfout: '',
    bamfsin: '',
    bamfsout: '',
    bankAccount: 0,
    shares: 0,
    immskll: '',
    title: '',
    whoText: '',
    plan: '',
    email: '',
    pagelen: 20,
    permStr: 13,
    permInt: 13,
    permWis: 13,
    permDex: 13,
    permCon: 13,
    modStr: 0,
    modInt: 0,
    modWis: 0,
    modDex: 0,
    modCon: 0,
    condition: [0, 48, 48],
    languages: [],
    skills: [],
    affects: [],
    affects2: [],
    affects3: [],
    affects4: [],
    affectsPowers: [],
    affectsWeaknesses: [],
    aliases: [],
    objects: [],
    storage: [],
    pet: null,
  };
}

function createDefaultObject(): LegacyObject {
  return {
    nest: 0,
    name: '',
    shortDescr: '',
    description: '',
    vnum: 0,
    durabilityMax: 100,
    durabilityCur: 100,
    extraFlags: 0,
    extraFlags2: 0,
    extraFlags3: 0,
    extraFlags4: 0,
    wearFlags: 0,
    wearLoc: -1,
    itemType: 0,
    weight: 0,
    level: 0,
    timer: 0,
    cost: 0,
    values: [0, 0, 0, 0],
    acType: 0,
    acVnum: 0,
    acCharge: [0, 0],
    acSpell: '',
    affects: [],
    extraDescriptions: [],
    spells: [],
    isStorage: false,
  };
}

/**
 * Parse the #PLAYER section of a player file.
 */
function parseCharSection(reader: PlayerReader, player: LegacyPlayer): void {
  for (;;) {
    if (reader.eof) return;
    const word = reader.readWord();
    if (!word) continue;

    const upper = word[0].toUpperCase();
    let matched = false;

    switch (upper) {
      case '*':
        // Comment line
        reader.skipToEol();
        matched = true;
        break;

      case 'A': {
        if (word === 'Act') { player.act = reader.readNumber(); matched = true; break; }
        if (word === 'Act2') { player.act2 = reader.readNumber(); matched = true; break; }
        if (word === 'AffdBy') { player.affectedBy = reader.readNumber(); matched = true; break; }
        if (word === 'AffdBy2') { player.affectedBy2 = reader.readNumber(); matched = true; break; }
        if (word === 'AffdBy3') { player.affectedBy3 = reader.readNumber(); matched = true; break; }
        if (word === 'AffdBy4') { player.affectedBy4 = reader.readNumber(); matched = true; break; }
        if (word === 'AffdByp') { player.affectedByPowers = reader.readNumber(); matched = true; break; }
        if (word === 'AffdByw') { player.affectedByWeaknesses = reader.readNumber(); matched = true; break; }
        if (word === 'Align') { player.alignment = reader.readNumber(); matched = true; break; }
        if (word === 'Antidisarm') { player.antidisarm = reader.readNumber(); matched = true; break; }
        if (word === 'ArenaWins') { player.awins = reader.readNumber(); matched = true; break; }
        if (word === 'ArenaLoses') { player.alosses = reader.readNumber(); matched = true; break; }
        if (word === 'Armr') { player.armor = reader.readNumber(); matched = true; break; }

        if (word === 'Aff') {
          const aff: LegacyAffect = {
            slot: reader.readNumber(),
            duration: reader.readNumber(),
            modifier: reader.readNumber(),
            location: reader.readNumber(),
            bitvector: reader.readNumber(),
          };
          player.affects.push(aff);
          matched = true;
          break;
        }
        if (word === 'Aff2') {
          const aff: LegacyAffect = {
            slot: reader.readNumber(),
            duration: reader.readNumber(),
            modifier: reader.readNumber(),
            location: reader.readNumber(),
            bitvector: reader.readNumber(),
          };
          player.affects2.push(aff);
          matched = true;
          break;
        }
        if (word === 'Aff3') {
          const aff: LegacyAffect = {
            slot: reader.readNumber(),
            duration: reader.readNumber(),
            modifier: reader.readNumber(),
            location: reader.readNumber(),
            bitvector: reader.readNumber(),
          };
          player.affects3.push(aff);
          matched = true;
          break;
        }
        if (word === 'Aff4') {
          const aff: LegacyAffect = {
            slot: reader.readNumber(),
            duration: reader.readNumber(),
            modifier: reader.readNumber(),
            location: reader.readNumber(),
            bitvector: reader.readNumber(),
          };
          player.affects4.push(aff);
          matched = true;
          break;
        }
        if (word === 'Affp') {
          const aff: LegacyAffect = {
            slot: reader.readNumber(),
            duration: reader.readNumber(),
            modifier: reader.readNumber(),
            location: reader.readNumber(),
            bitvector: reader.readNumber(),
          };
          player.affectsPowers.push(aff);
          matched = true;
          break;
        }
        if (word === 'Affw') {
          const aff: LegacyAffect = {
            slot: reader.readNumber(),
            duration: reader.readNumber(),
            modifier: reader.readNumber(),
            location: reader.readNumber(),
            bitvector: reader.readNumber(),
          };
          player.affectsWeaknesses.push(aff);
          matched = true;
          break;
        }
        if (word === 'Alias') {
          const shortcut = reader.readString();
          const expansion = reader.readString();
          player.aliases.push({ shortcut, expansion });
          matched = true;
          break;
        }
        if (word === 'AtrMd') {
          player.modStr = reader.readNumber();
          player.modInt = reader.readNumber();
          player.modWis = reader.readNumber();
          player.modDex = reader.readNumber();
          player.modCon = reader.readNumber();
          matched = true;
          break;
        }
        if (word === 'AtrPrm') {
          player.permStr = reader.readNumber();
          player.permInt = reader.readNumber();
          player.permWis = reader.readNumber();
          player.permDex = reader.readNumber();
          player.permCon = reader.readNumber();
          matched = true;
          break;
        }
        break;
      }

      case 'B': {
        if (word === 'Bmfin') { player.bamfin = reader.readString(); matched = true; break; }
        if (word === 'Bmfout') { player.bamfout = reader.readString(); matched = true; break; }
        if (word === 'Bmfsin') { player.bamfsin = reader.readString(); matched = true; break; }
        if (word === 'Bmfsout') { player.bamfsout = reader.readString(); matched = true; break; }
        if (word === 'Bank') { player.bankAccount = reader.readNumber(); matched = true; break; }
        if (word === 'BankShares') { player.shares = reader.readNumber(); matched = true; break; }
        break;
      }

      case 'C': {
        if (word === 'Clan') { player.clan = reader.readNumber(); matched = true; break; }
        if (word === 'Clvl') { player.clev = reader.readNumber(); matched = true; break; }
        if (word === 'Ctmr') { player.ctimer = reader.readNumber(); matched = true; break; }
        if (word === 'Cla') { player.charClass = reader.readNumber(); matched = true; break; }
        if (word === 'Corpses') { player.corpses = reader.readNumber(); matched = true; break; }
        if (word === 'Cond') {
          player.condition[0] = reader.readNumber();
          player.condition[1] = reader.readNumber();
          player.condition[2] = reader.readNumber();
          matched = true;
          break;
        }
        break;
      }

      case 'D': {
        if (word === 'Dam') { player.damroll = reader.readNumber(); matched = true; break; }
        if (word === 'Deaf') { player.deaf = reader.readNumber(); matched = true; break; }
        if (word === 'Dscr') { player.description = reader.readString(); matched = true; break; }
        break;
      }

      case 'E': {
        if (word === 'End') return;
        if (word === 'Exp') { player.exp = reader.readNumber(); matched = true; break; }
        if (word === 'Email') { player.email = reader.readString(); matched = true; break; }
        break;
      }

      case 'G': {
        if (word === 'Gold') { player.gold = reader.readNumber(); matched = true; break; }
        if (word === 'GRank') { player.guildRank = reader.readNumber(); matched = true; break; }
        if (word === 'Guild') { player.guild = reader.readString(); matched = true; break; }
        break;
      }

      case 'H': {
        if (word === 'Hit') { player.hitroll = reader.readNumber(); matched = true; break; }
        if (word === 'HpMnMvBp') {
          player.hit = reader.readNumber();
          player.maxHit = reader.readNumber();
          player.mana = reader.readNumber();
          player.maxMana = reader.readNumber();
          player.move = reader.readNumber();
          player.maxMove = reader.readNumber();
          player.bp = reader.readNumber();
          player.maxBp = reader.readNumber();
          matched = true;
          break;
        }
        // Legacy HpMnMv format (6 values, no bp)
        if (word === 'HpMnMv') {
          player.hit = reader.readNumber();
          player.maxHit = reader.readNumber();
          player.mana = reader.readNumber();
          player.maxMana = reader.readNumber();
          player.move = reader.readNumber();
          player.maxMove = reader.readNumber();
          matched = true;
          break;
        }
        break;
      }

      case 'I': {
        if (word === 'Immskll') { player.immskll = reader.readString(); matched = true; break; }
        if (word === 'ImmBits') { player.immFlags = reader.readNumber(); matched = true; break; }
        break;
      }

      case 'L': {
        if (word === 'Lnm') { player.lname = reader.readString(); matched = true; break; }
        if (word === 'Lvl') { player.level = reader.readNumber(); matched = true; break; }
        if (word === 'Learn') { player.learn = reader.readNumber(); matched = true; break; }
        if (word === 'Lang') {
          const value = reader.readNumber();
          const langName = reader.readWord(); // reads 'name' (in quotes)
          const idx = langLookup(langName);
          if (idx >= 0) {
            player.languages.push({ index: idx, name: langName, value });
          }
          matched = true;
          break;
        }
        if (word === 'LngDsc') {
          // Long desc is tilde-terminated: LngDsc       Some description~
          // The C server skips it (auto-generated), but we preserve it for migration
          const longD = reader.readString();
          if (longD) {
            player.longDescr = longD;
          }
          matched = true;
          break;
        }
        break;
      }

      case 'M': {
        if (word === 'Mlt') { player.multied = reader.readNumber(); matched = true; break; }
        if (word === 'Mounted') { player.mounted = reader.readNumber(); matched = true; break; }
        if (word === 'Mountcharmed') { player.mountcharmed = reader.readNumber(); matched = true; break; }
        if (word === 'Mountshort') { player.mountshort = reader.readString(); matched = true; break; }
        if (word === 'MobKills') { player.mobkills = reader.readNumber(); matched = true; break; }
        break;
      }

      case 'N': {
        if (word === 'Nm') {
          // Read the name from the tilde-terminated string on this line.
          // Format: Nm           SomeName~
          // We read it to capture the canonical name from the file.
          const nameFromFile = reader.readString();
          if (nameFromFile) {
            player.name = nameFromFile;
          }
          matched = true;
          break;
        }
        if (word === 'Note') { player.lastNote = reader.readNumber(); matched = true; break; }
        break;
      }

      case 'P': {
        if (word === 'Pglen') { player.pagelen = reader.readNumber(); matched = true; break; }
        if (word === 'Paswd') { player.pwd = reader.readString(); matched = true; break; }
        if (word === 'Playd') { player.played = reader.readNumber(); matched = true; break; }
        if (word === 'PoisonLevel') { player.poisonLevel = reader.readNumber(); matched = true; break; }
        if (word === 'Pos') { player.position = reader.readNumber(); matched = true; break; }
        if (word === 'Prac') { player.practice = reader.readNumber(); matched = true; break; }
        if (word === 'Pkill') { player.pkill = reader.readNumber(); matched = true; break; }
        if (word === 'Prmpt') { player.prompt = reader.readString(); matched = true; break; }
        if (word === 'Plan') { player.plan = reader.readString(); matched = true; break; }
        break;
      }

      case 'Q': {
        if (word === 'QuestPnts') { player.questpoints = reader.readNumber(); matched = true; break; }
        if (word === 'QuestNext') { player.nextquest = reader.readNumber(); matched = true; break; }
        if (word === 'QuestCount') { player.countdown = reader.readNumber(); matched = true; break; }
        if (word === 'QuestObj') { player.questobj = reader.readNumber(); matched = true; break; }
        if (word === 'QuestMob') { player.questmob = reader.readNumber(); matched = true; break; }
        if (word === 'QuestGiver') { reader.readNumber(); matched = true; break; } // skip mob vnum
        break;
      }

      case 'R': {
        if (word === 'Rce') { /* legacy numeric race */ reader.readNumber(); matched = true; break; }
        if (word === 'Race') { player.race = reader.readString(); matched = true; break; }
        if (word === 'Recall') { player.recall = reader.readNumber(); matched = true; break; }
        if (word === 'Religion') { reader.readString(); matched = true; break; } // deprecated string
        if (word === 'ReligionN') { reader.readString(); matched = true; break; } // deprecated
        if (word === 'RNumber') { player.religion = reader.readNumber(); matched = true; break; }
        if (word === 'ResBits') { player.resFlags = reader.readNumber(); matched = true; break; }
        if (word === 'Room') { player.room = reader.readNumber(); matched = true; break; }
        if (word === 'Rtimer') { player.rtimer = reader.readNumber(); matched = true; break; }
        if (word === 'RQuestPnts') { player.rquestpoints = reader.readNumber(); matched = true; break; }
        if (word === 'RQuestNext') { player.rnextquest = reader.readNumber(); matched = true; break; }
        if (word === 'RQuestCount') { reader.readNumber(); matched = true; break; } // temp
        if (word === 'RQuestObj') { reader.readNumber(); matched = true; break; } // temp
        if (word === 'RQuestMob') { reader.readNumber(); matched = true; break; } // temp

        // Damage mod resistances
        if (word === 'ResAcid') { player.damageMods[0] = reader.readNumber(); matched = true; break; }
        if (word === 'ResHoly') { player.damageMods[1] = reader.readNumber(); matched = true; break; }
        if (word === 'ResMagic') { player.damageMods[2] = reader.readNumber(); matched = true; break; }
        if (word === 'ResFire') { player.damageMods[3] = reader.readNumber(); matched = true; break; }
        if (word === 'ResEnergy') { player.damageMods[4] = reader.readNumber(); matched = true; break; }
        if (word === 'ResWind') { player.damageMods[5] = reader.readNumber(); matched = true; break; }
        if (word === 'ResWater') { player.damageMods[6] = reader.readNumber(); matched = true; break; }
        if (word === 'ResIllusion') { player.damageMods[7] = reader.readNumber(); matched = true; break; }
        if (word === 'ResDispel') { player.damageMods[8] = reader.readNumber(); matched = true; break; }
        if (word === 'ResEarth') { player.damageMods[9] = reader.readNumber(); matched = true; break; }
        if (word === 'ResPsychic') { player.damageMods[10] = reader.readNumber(); matched = true; break; }
        if (word === 'ResPoison') { player.damageMods[11] = reader.readNumber(); matched = true; break; }
        if (word === 'ResBreath') { player.damageMods[12] = reader.readNumber(); matched = true; break; }
        if (word === 'ResSummon' || word === 'ResSUmmon') { player.damageMods[13] = reader.readNumber(); matched = true; break; }
        if (word === 'ResPhysical') { player.damageMods[14] = reader.readNumber(); matched = true; break; }
        if (word === 'ResExplosive') { player.damageMods[15] = reader.readNumber(); matched = true; break; }
        if (word === 'ResSong') { player.damageMods[16] = reader.readNumber(); matched = true; break; }
        if (word === 'ResNagarom') { player.damageMods[17] = reader.readNumber(); matched = true; break; }
        if (word === 'ResUnholy') { player.damageMods[18] = reader.readNumber(); matched = true; break; }
        if (word === 'ResClan') { player.damageMods[19] = reader.readNumber(); matched = true; break; }
        break;
      }

      case 'S': {
        if (word === 'SavThr') { player.savingThrow = reader.readNumber(); matched = true; break; }
        if (word === 'Sx') { player.sex = reader.readNumber(); matched = true; break; }
        if (word === 'Security') { player.security = reader.readNumber(); matched = true; break; }
        if (word === 'Shields') { player.shields = reader.readNumber(); matched = true; break; }
        if (word === 'Speak') { player.speaking = reader.readNumber(); matched = true; break; }
        if (word === 'Stun') {
          player.stunned[0] = reader.readNumber();
          player.stunned[1] = reader.readNumber();
          player.stunned[2] = reader.readNumber();
          player.stunned[3] = reader.readNumber();
          player.stunned[4] = reader.readNumber();
          matched = true;
          break;
        }
        if (word === 'ShtDsc') {
          // Short desc is tilde-terminated: ShtDsc       SomeName~
          // The C server skips it (auto-generated), but we preserve it for migration
          const shortD = reader.readString();
          if (shortD) {
            player.shortDescr = shortD;
          }
          matched = true;
          break;
        }
        if (word === 'Skll') {
          const value = reader.readNumber();
          const skillName = reader.readWord(); // reads 'skill name' (in quotes)
          player.skills.push({ name: skillName, value });
          matched = true;
          break;
        }
        break;
      }

      case 'T': {
        if (word === 'Trst') { player.trust = reader.readNumber(); matched = true; break; }
        if (word === 'Ttle') {
          player.title = reader.readString();
          // Ensure title starts with a space if it begins with alphanumeric
          if (player.title.length > 0 && /^[a-zA-Z0-9]/.test(player.title)) {
            player.title = ' ' + player.title;
          }
          matched = true;
          break;
        }
        break;
      }

      case 'U': {
        if (word === 'Updated') { player.updated = reader.readNumber(); matched = true; break; }
        break;
      }

      case 'V': {
        if (word === 'VulBits') { player.vulFlags = reader.readNumber(); matched = true; break; }
        if (word === 'Vnum') { reader.readNumber(); matched = true; break; } // NPC vnum, skip
        break;
      }

      case 'W': {
        if (word === 'Wimp') { player.wimpy = reader.readNumber(); matched = true; break; }
        if (word === 'WhoTxt') { player.whoText = reader.readString(); matched = true; break; }
        if (word === 'Wizbt') { player.wizbit = reader.readNumber(); matched = true; break; }
        if (word === 'WizLev') { player.wizinvis = reader.readNumber(); matched = true; break; }
        break;
      }
    }

    if (!matched) {
      // Unknown keyword -- skip rest of line
      reader.skipToEol();
    }
  }
}

/**
 * Parse the #OBJECT or #STORAGE section of a player file.
 */
function parseObjectSection(reader: PlayerReader, isStorage: boolean): LegacyObject {
  const obj = createDefaultObject();
  obj.isStorage = isStorage;

  for (;;) {
    if (reader.eof) return obj;
    const word = reader.readWord();
    if (!word) continue;

    const upper = word[0].toUpperCase();
    let matched = false;

    switch (upper) {
      case '*':
        reader.skipToEol();
        matched = true;
        break;

      case 'A': {
        if (word === 'Activates') {
          obj.acType = reader.readNumber();
          obj.acVnum = reader.readNumber();
          obj.acCharge[0] = reader.readNumber();
          obj.acCharge[1] = reader.readNumber();
          matched = true;
          break;
        }
        if (word === 'AcSpell') {
          obj.acSpell = reader.readString();
          matched = true;
          break;
        }
        if (word === 'Affect') {
          const aff: LegacyAffect = {
            slot: reader.readNumber(),
            duration: reader.readNumber(),
            modifier: reader.readNumber(),
            location: reader.readNumber(),
            bitvector: reader.readNumber(),
          };
          obj.affects.push(aff);
          matched = true;
          break;
        }
        break;
      }

      case 'C': {
        if (word === 'Cost') { obj.cost = reader.readNumber(); matched = true; break; }
        break;
      }

      case 'D': {
        if (word === 'Description') { obj.description = reader.readString(); matched = true; break; }
        if (word === 'DurabilityMax') { obj.durabilityMax = reader.readNumber(); matched = true; break; }
        if (word === 'DurabilityCur') { obj.durabilityCur = reader.readNumber(); matched = true; break; }
        break;
      }

      case 'E': {
        if (word === 'ExtraFlags') { obj.extraFlags = reader.readNumber(); matched = true; break; }
        if (word === 'ExtraFlags2') { obj.extraFlags2 = reader.readNumber(); matched = true; break; }
        if (word === 'ExtraFlags3') { obj.extraFlags3 = reader.readNumber(); matched = true; break; }
        if (word === 'ExtraFlags4') { obj.extraFlags4 = reader.readNumber(); matched = true; break; }
        if (word === 'ExtraDescr') {
          const keyword = reader.readString();
          const desc = reader.readString();
          obj.extraDescriptions.push({ keyword, description: desc });
          matched = true;
          break;
        }
        if (word === 'End') return obj;
        break;
      }

      case 'I': {
        if (word === 'ItemType') { obj.itemType = reader.readNumber(); matched = true; break; }
        break;
      }

      case 'L': {
        if (word === 'Level') { obj.level = reader.readNumber(); matched = true; break; }
        break;
      }

      case 'N': {
        if (word === 'Name') { obj.name = reader.readString(); matched = true; break; }
        if (word === 'Nest') { obj.nest = reader.readNumber(); matched = true; break; }
        break;
      }

      case 'S': {
        if (word === 'ShortDescr') { obj.shortDescr = reader.readString(); matched = true; break; }
        if (word === 'Spell') {
          const idx = reader.readNumber();
          const spellName = reader.readWord(); // reads 'spell name' (in quotes)
          obj.spells.push({ index: idx, name: spellName });
          matched = true;
          break;
        }
        break;
      }

      case 'T': {
        if (word === 'Timer') { obj.timer = reader.readNumber(); matched = true; break; }
        break;
      }

      case 'V': {
        if (word === 'Values') {
          obj.values[0] = reader.readNumber();
          obj.values[1] = reader.readNumber();
          obj.values[2] = reader.readNumber();
          obj.values[3] = reader.readNumber();
          matched = true;
          break;
        }
        if (word === 'Vnum') { obj.vnum = reader.readNumber(); matched = true; break; }
        break;
      }

      case 'W': {
        if (word === 'WearFlags') { obj.wearFlags = reader.readNumber(); matched = true; break; }
        if (word === 'WearLoc') { obj.wearLoc = reader.readNumber(); matched = true; break; }
        if (word === 'Weight') { obj.weight = reader.readNumber(); matched = true; break; }
        break;
      }
    }

    if (!matched) {
      reader.skipToEol();
    }
  }
}

/**
 * Parse the #PET section of a player file.
 * Format: vnum hit maxHit act affectedBy affectedBy2 affectedBy3 affectedBy4
 */
function parsePetSection(reader: PlayerReader): LegacyPet {
  const vnum = reader.readNumber();
  const hit = reader.readNumber();
  const maxHit = reader.readNumber();
  const act = reader.readNumber();
  const affectedBy = reader.readNumber();
  const affectedBy2 = reader.readNumber();
  const affectedBy3 = reader.readNumber();
  const affectedBy4 = reader.readNumber();
  return { vnum, hit, maxHit, act, affectedBy, affectedBy2, affectedBy3, affectedBy4 };
}

/**
 * Parse an entire player save file into a LegacyPlayer struct.
 */
export function parsePlayerFile(content: string): LegacyPlayer {
  const reader = new PlayerReader(content);
  const player = createDefaultPlayer();

  for (;;) {
    if (reader.eof) break;

    const letter = reader.readLetter();
    if (!letter) break;

    // Skip comment lines starting with *
    if (letter === '*') {
      reader.skipToEol();
      continue;
    }

    // Expect '#' for section headers
    if (letter !== '#') {
      continue;
    }

    const sectionWord = reader.readWord();

    if (sectionWord === 'PLAYER' || sectionWord === 'MOB') {
      // Extract name from file before parsing (the Nm field in the file
      // is skipped by fread_char, name is set from the filename)
      parseCharSection(reader, player);
    } else if (sectionWord === 'OBJECT') {
      const obj = parseObjectSection(reader, false);
      player.objects.push(obj);
    } else if (sectionWord === 'STORAGE') {
      const obj = parseObjectSection(reader, true);
      player.storage.push(obj);
    } else if (sectionWord === 'PET') {
      player.pet = parsePetSection(reader);
    } else if (sectionWord === 'END') {
      break;
    }
    // Unknown sections are silently skipped
  }

  return player;
}

// ---------------------------------------------------------------------------
// Race name -> race index mapping
// ---------------------------------------------------------------------------

const RACE_NAMES = [
  'human', 'elf', 'dwarf', 'halfling', 'pixie',
  'half-elf', 'half-orc', 'half-ogre', 'gnome', 'drow',
  'elder', 'dragon', 'minotaur', 'demon', 'troll',
  'werewolf', 'vampire', 'undead', 'angel', 'catfolk',
  'canine', 'lizardman', 'ogre', 'shadow', 'goblin',
  'hobbit', 'golem',
];

function raceLookup(name: string): number {
  if (!name) return 0;
  const lower = name.toLowerCase();
  for (let i = 0; i < RACE_NAMES.length; i++) {
    if (RACE_NAMES[i] === lower) return i;
  }
  // Try partial match
  for (let i = 0; i < RACE_NAMES.length; i++) {
    if (lower.startsWith(RACE_NAMES[i]) || RACE_NAMES[i].startsWith(lower)) {
      return i;
    }
  }
  return 0; // default to human
}

// ---------------------------------------------------------------------------
// Convert LegacyPlayer to Firestore document format
// (matches the format used by save.ts saveCharacter / loadCharacter)
// ---------------------------------------------------------------------------

/**
 * Convert a parsed LegacyPlayer into the Firestore document format
 * used by the web version's save.ts.
 */
export function legacyToFirestoreDoc(
  player: LegacyPlayer,
  charId: string,
): Record<string, unknown> {
  // Convert affects: the legacy format stores slot numbers, which were
  // converted to sn via slot_lookup in the C server. Since we don't have
  // the skill table here, we store the slot directly as the type (the web
  // server uses numeric skill IDs anyway).
  const convertAffects = (affs: LegacyAffect[]) =>
    affs.map((a) => ({
      type: a.slot,
      level: 0, // Not stored in legacy format (was implicit from spell)
      duration: a.duration,
      location: a.location,
      modifier: a.modifier,
      bitvector: a.bitvector,
    }));

  // Convert skills to learned map: { [sn_string]: percentage }
  // Since we only have skill names and not numeric sn values, we store
  // the skill name as the key in a separate legacySkills map and set
  // up the learned map as empty (to be populated when skill_table is available)
  const learnedObj: Record<string, number> = {};
  const legacySkillsObj: Record<string, number> = {};
  for (const skill of player.skills) {
    legacySkillsObj[skill.name] = skill.value;
  }

  // Convert aliases to plain object
  const aliasesObj: Record<string, string> = {};
  for (const alias of player.aliases) {
    aliasesObj[alias.shortcut] = alias.expansion;
  }

  // Convert language array
  const languageArr = new Array(27).fill(0);
  for (const lang of player.languages) {
    if (lang.index >= 0 && lang.index < 27) {
      languageArr[lang.index] = lang.value;
    }
  }

  // Pad damageMods to 21 entries (web version uses DamType.NONE through DamType.SOUND = 20)
  const damageMods = [...player.damageMods];
  while (damageMods.length < 21) {
    damageMods.push(0);
  }

  // Equipment: objects worn (wearLoc >= 0)
  const equipment = player.objects
    .filter((o) => o.wearLoc >= 0 && !o.isStorage)
    .map((o) => ({
      vnum: o.vnum,
      wearLoc: o.wearLoc,
    }));

  // Inventory: objects not worn (wearLoc === -1, NONE), nest 0
  const inventory = player.objects
    .filter((o) => o.wearLoc < 0 && !o.isStorage)
    .map((o) => o.vnum);

  // Storage items (separate from inventory)
  const storageVnums = player.storage.map((o) => o.vnum);

  // Resolve race from name to index
  const raceIndex = raceLookup(player.race);

  const doc: Record<string, unknown> = {
    // Ownership — no owner yet until linked
    ownerId: null,
    legacyImport: true,
    importedAt: new Date(),

    // Room
    roomVnum: player.room || 25000,
    lastSave: new Date(),

    // Core identity
    name: player.name,
    shortDescr: player.shortDescr || player.name,
    longDescr: player.longDescr || `${player.name} is standing here.`,
    description: player.description || '',
    charClass: player.charClass,
    race: raceIndex,
    level: player.level,
    trust: player.trust,
    sex: player.sex,

    // Vitals
    hp: player.hit,
    maxHp: player.maxHit,
    mana: player.mana,
    maxMana: player.maxMana,
    move: player.move,
    maxMove: player.maxMove,
    bp: player.bp,
    maxBp: player.maxBp,

    // Combat stats
    hitroll: player.hitroll,
    damroll: player.damroll,
    armor: player.armor,
    savingThrow: player.savingThrow,
    alignment: player.alignment,

    // Resources
    gold: player.gold,
    exp: player.exp,
    practice: player.practice,

    // Position — always STANDING for saved characters
    position: 8, // Position.STANDING

    // Social
    clan: player.clan,
    religion: player.religion,
    clev: player.clev,
    language: languageArr,
    speaking: player.speaking,
    size: 2, // default medium
    pkill: player.pkill,
    shields: player.shields,

    // Quest
    questpoints: player.questpoints,
    nextquest: 0, // reset on import
    questobj: 0,
    questmob: 0,
    rquestpoints: player.rquestpoints,

    // Bitvectors
    affectedBy: player.affectedBy,
    affectedBy2: player.affectedBy2,
    affectedBy3: player.affectedBy3,
    affectedBy4: player.affectedBy4,
    act: player.act,
    act2: player.act2,

    // Misc
    wimpy: player.wimpy,
    damageMods,
    mounted: player.mounted,

    // Equipment & inventory
    equipment,
    inventory,

    // Storage (separate field for items in player storage)
    storage: storageVnums,

    // Affects
    affects: convertAffects(player.affects),
    affects2: convertAffects(player.affects2),
    affects3: convertAffects(player.affects3),
    affects4: convertAffects(player.affects4),

    // PcData
    pcdata: {
      title: player.title || ' the Adventurer',
      prompt: player.prompt || '<%hhp %mm %vmv> ',
      lname: player.lname || '',
      bamfin: player.bamfin || '',
      bamfout: player.bamfout || '',
      bamfsin: player.bamfsin || '',
      bamfsout: player.bamfsout || '',
      whoText: player.whoText || '',
      spouse: '',
      recall: player.recall || 25000,
      permStr: player.permStr,
      permInt: player.permInt,
      permWis: player.permWis,
      permDex: player.permDex,
      permCon: player.permCon,
      modStr: player.modStr,
      modInt: player.modInt,
      modWis: player.modWis,
      modDex: player.modDex,
      modCon: player.modCon,
      condition: player.condition,
      learned: learnedObj,
      legacySkills: legacySkillsObj,
      security: player.security,
      bankAccount: player.bankAccount,
      pagelen: player.pagelen || 20,
      shares: player.shares,
      mobkills: player.mobkills,
      corpses: player.corpses,
      plan: player.plan || '',
      email: player.email || '',
      awins: player.awins,
      alosses: player.alosses,
      aliases: aliasesObj,
      spellSlots: { spell1: 0, spell2: 0, spell3: 0 },
      craftTimer: 0,
      craftType: 0,
    },

    // Legacy metadata for linking
    legacy: {
      played: player.played,
      lastNote: player.lastNote,
      guild: player.guild,
      guildRank: player.guildRank,
      raceName: player.race,
      multied: player.multied,
      immFlags: player.immFlags,
      resFlags: player.resFlags,
      vulFlags: player.vulFlags,
      wizinvis: player.wizinvis,
      wizbit: player.wizbit,
      immskll: player.immskll,
      updated: player.updated,
    },

    // Pet data (if any)
    pet: player.pet
      ? {
          vnum: player.pet.vnum,
          hit: player.pet.hit,
          maxHit: player.pet.maxHit,
          act: player.pet.act,
          affectedBy: player.pet.affectedBy,
          affectedBy2: player.pet.affectedBy2,
          affectedBy3: player.pet.affectedBy3,
          affectedBy4: player.pet.affectedBy4,
        }
      : null,

    // Full object data for items that need recreation with custom values
    legacyObjects: player.objects.map((o) => ({
      vnum: o.vnum,
      nest: o.nest,
      name: o.name,
      shortDescr: o.shortDescr,
      description: o.description,
      wearLoc: o.wearLoc,
      itemType: o.itemType,
      extraFlags: o.extraFlags,
      extraFlags2: o.extraFlags2,
      extraFlags3: o.extraFlags3,
      extraFlags4: o.extraFlags4,
      wearFlags: o.wearFlags,
      durabilityMax: o.durabilityMax,
      durabilityCur: o.durabilityCur,
      weight: o.weight,
      level: o.level,
      timer: o.timer,
      cost: o.cost,
      values: o.values,
      acType: o.acType,
      acVnum: o.acVnum,
      acCharge: o.acCharge,
      acSpell: o.acSpell,
      affects: o.affects.map((a) => ({
        slot: a.slot,
        duration: a.duration,
        modifier: a.modifier,
        location: a.location,
        bitvector: a.bitvector,
      })),
    })),
  };

  return doc;
}

// ---------------------------------------------------------------------------
// Batch writer (same pattern as migrate.ts)
// ---------------------------------------------------------------------------

class BatchWriter {
  private batch: WriteBatch;
  private count = 0;
  private totalWrites = 0;
  private db: FirebaseFirestore.Firestore;
  private pendingFlushes: Promise<unknown>[] = [];

  constructor(db: FirebaseFirestore.Firestore) {
    this.db = db;
    this.batch = db.batch();
  }

  set(ref: FirebaseFirestore.DocumentReference, data: Record<string, unknown>): void {
    this.batch.set(ref, data, { merge: true });
    this.count++;
    this.totalWrites++;
    if (this.count >= 500) {
      this.pendingFlushes.push(this.batch.commit());
      this.batch = this.db.batch();
      this.count = 0;
    }
  }

  async flush(): Promise<void> {
    if (this.count > 0) {
      this.pendingFlushes.push(this.batch.commit());
      this.batch = this.db.batch();
      this.count = 0;
    }
    await Promise.all(this.pendingFlushes);
    this.pendingFlushes = [];
  }

  get total(): number {
    return this.totalWrites;
  }
}

// ---------------------------------------------------------------------------
// Password verification for legacy account linking
// ---------------------------------------------------------------------------

/**
 * Verify a plaintext password against a legacy crypt() hash.
 *
 * The C server uses libc crypt() which can produce:
 *   - DES hashes (13 chars, 2-char salt)
 *   - MD5 hashes ($1$salt$hash)
 *   - SHA-256 hashes ($5$salt$hash)
 *   - SHA-512 hashes ($6$salt$hash)
 *
 * Node.js crypto module does not have a direct crypt() equivalent,
 * so we use a reimplementation approach.
 */
function verifyLegacyPassword(plaintext: string, hash: string): boolean {
  if (!hash || !plaintext) return false;

  // MD5 crypt: $1$salt$hash
  if (hash.startsWith('$1$')) {
    return verifyMd5Crypt(plaintext, hash);
  }

  // SHA-256 crypt: $5$salt$hash
  if (hash.startsWith('$5$')) {
    return verifySha256Crypt(plaintext, hash);
  }

  // SHA-512 crypt: $6$salt$hash
  if (hash.startsWith('$6$')) {
    return verifySha512Crypt(plaintext, hash);
  }

  // DES crypt: 13 characters, first 2 are salt
  if (hash.length === 13) {
    return verifyDesCrypt(plaintext, hash);
  }

  // Unknown hash format
  return false;
}

// MD5 crypt implementation (compatible with glibc $1$ format)
const MD5_CRYPT_ITOA64 = './0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';

function md5(data: Buffer): Buffer {
  return createHash('md5').update(data).digest();
}

function to64(value: number, n: number): string {
  let result = '';
  let v = value;
  while (--n >= 0) {
    result += MD5_CRYPT_ITOA64[v & 0x3f];
    v >>= 6;
  }
  return result;
}

function verifyMd5Crypt(plaintext: string, hash: string): boolean {
  // Parse: $1$salt$hash
  const parts = hash.split('$');
  if (parts.length < 4) return false;
  const salt = parts[2].substring(0, 8); // max 8 chars
  const computed = computeMd5Crypt(plaintext, salt);
  return computed === hash;
}

function computeMd5Crypt(password: string, salt: string): string {
  const magic = '$1$';
  const pw = Buffer.from(password, 'utf-8');
  const sl = Buffer.from(salt, 'utf-8');

  // Start digest A
  let ctxBuf = Buffer.concat([pw, Buffer.from(magic), sl]);

  // Digest B
  let altResult = md5(Buffer.concat([pw, sl, pw]));

  // Add bytes from alt to ctx
  let plen = pw.length;
  while (plen > 0) {
    const chunk = plen > 16 ? 16 : plen;
    ctxBuf = Buffer.concat([ctxBuf, altResult.subarray(0, chunk)]);
    plen -= chunk;
  }

  // Add bits from password length
  for (let i = pw.length; i > 0; i >>= 1) {
    if (i & 1) {
      ctxBuf = Buffer.concat([ctxBuf, Buffer.from([0])]);
    } else {
      ctxBuf = Buffer.concat([ctxBuf, Buffer.from([pw[0]])]);
    }
  }

  altResult = md5(ctxBuf);

  // 1000 rounds
  for (let i = 0; i < 1000; i++) {
    let roundBuf = Buffer.alloc(0);
    if (i & 1) {
      roundBuf = Buffer.concat([roundBuf, pw]);
    } else {
      roundBuf = Buffer.concat([roundBuf, altResult]);
    }
    if (i % 3) {
      roundBuf = Buffer.concat([roundBuf, sl]);
    }
    if (i % 7) {
      roundBuf = Buffer.concat([roundBuf, pw]);
    }
    if (i & 1) {
      roundBuf = Buffer.concat([roundBuf, altResult]);
    } else {
      roundBuf = Buffer.concat([roundBuf, pw]);
    }
    altResult = md5(roundBuf);
  }

  // Encode result
  let output = magic + salt + '$';
  output += to64((altResult[0] << 16) | (altResult[6] << 8) | altResult[12], 4);
  output += to64((altResult[1] << 16) | (altResult[7] << 8) | altResult[13], 4);
  output += to64((altResult[2] << 16) | (altResult[8] << 8) | altResult[14], 4);
  output += to64((altResult[3] << 16) | (altResult[9] << 8) | altResult[15], 4);
  output += to64((altResult[4] << 16) | (altResult[10] << 8) | altResult[5], 4);
  output += to64(altResult[11], 2);

  return output;
}

/**
 * DES crypt verification.
 * DES crypt is largely obsolete but some very old player files may use it.
 * We use a simplified verification: try to match using Node crypto.
 */
function verifyDesCrypt(plaintext: string, hash: string): boolean {
  // DES crypt uses a 2-character salt and produces a 13-character hash.
  // Node.js does not natively support DES crypt.
  // We attempt a basic comparison using the traditional DES algorithm.
  // For production use, consider the 'unix-crypt-td-js' npm package.
  try {
    const salt = hash.substring(0, 2);
    const computed = computeDesCrypt(plaintext, salt);
    return computed === hash;
  } catch {
    return false;
  }
}

/**
 * Minimal DES crypt implementation following the traditional Unix crypt(3) algorithm.
 * Based on the public domain implementation from Eric Young.
 */
function computeDesCrypt(password: string, salt: string): string {
  // Initial permutation table
  const IP: number[] = [
    58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7,
  ];

  // Final permutation table
  const FP: number[] = [
    40, 8, 48, 16, 56, 24, 64, 32, 39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30, 37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28, 35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26, 33, 1, 41, 9, 49, 17, 57, 25,
  ];

  // Expansion table
  const E: number[] = [
    32, 1, 2, 3, 4, 5, 4, 5, 6, 7, 8, 9, 8, 9, 10, 11,
    12, 13, 12, 13, 14, 15, 16, 17, 16, 17, 18, 19, 20, 21, 20, 21,
    22, 23, 24, 25, 24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32, 1,
  ];

  // Permutation P
  const P: number[] = [
    16, 7, 20, 21, 29, 12, 28, 17, 1, 15, 23, 26, 5, 18, 31, 10,
    2, 8, 24, 14, 32, 27, 3, 9, 19, 13, 30, 6, 22, 11, 4, 25,
  ];

  // S-boxes
  const S: number[][] = [
    [14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13],
    [15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9],
    [10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12],
    [7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14],
    [2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3],
    [12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13],
    [4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12],
    [13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,1,15,13,8,10,3,7,4,12,5,6,2,0,14,9,11,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,13,11,14,4,1,5,8,2,13,0,15,6,9,10,3,12,7],
  ];

  // PC1 permutation
  const PC1: number[] = [
    57, 49, 41, 33, 25, 17, 9, 1, 58, 50, 42, 34, 26, 18,
    10, 2, 59, 51, 43, 35, 27, 19, 11, 3, 60, 52, 44, 36,
    63, 55, 47, 39, 31, 23, 15, 7, 62, 54, 46, 38, 30, 22,
    14, 6, 61, 53, 45, 37, 29, 21, 13, 5, 28, 20, 12, 4,
  ];

  // PC2 permutation
  const PC2: number[] = [
    14, 17, 11, 24, 1, 5, 3, 28, 15, 6, 21, 10,
    23, 19, 12, 4, 26, 8, 16, 7, 27, 20, 13, 2,
    41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32,
  ];

  // Key rotation schedule
  const ROTATIONS: number[] = [1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1];

  function getBit(data: number[], pos: number): number {
    const byteIndex = Math.floor((pos - 1) / 8);
    const bitIndex = 7 - ((pos - 1) % 8);
    return (data[byteIndex] >> bitIndex) & 1;
  }

  function setBit(data: number[], pos: number, val: number): void {
    const byteIndex = Math.floor((pos - 1) / 8);
    const bitIndex = 7 - ((pos - 1) % 8);
    if (val) {
      data[byteIndex] |= (1 << bitIndex);
    } else {
      data[byteIndex] &= ~(1 << bitIndex);
    }
  }

  function permute(input: number[], table: number[], n: number): number[] {
    const output = new Array(Math.ceil(n / 8)).fill(0);
    for (let i = 0; i < n; i++) {
      const val = getBit(input, table[i]);
      setBit(output, i + 1, val);
    }
    return output;
  }

  function leftRotate(half: number[], count: number): number[] {
    const bits: number[] = [];
    for (let i = 1; i <= 28; i++) {
      bits.push(getBit(half, i));
    }
    const rotated = [...bits.slice(count), ...bits.slice(0, count)];
    const result = new Array(4).fill(0);
    for (let i = 0; i < 28; i++) {
      setBit(result, i + 1, rotated[i]);
    }
    return result;
  }

  function xorBlocks(a: number[], b: number[], len: number): number[] {
    const result = new Array(Math.ceil(len / 8)).fill(0);
    for (let i = 0; i < Math.ceil(len / 8); i++) {
      result[i] = (a[i] || 0) ^ (b[i] || 0);
    }
    return result;
  }

  function desRound(right: number[], subkey: number[]): number[] {
    // Expand right half from 32 to 48 bits
    const expanded = permute(right, E, 48);
    // XOR with subkey
    const xored = xorBlocks(expanded, subkey, 48);

    // S-box substitution
    const sboxOutput = new Array(4).fill(0);
    for (let i = 0; i < 8; i++) {
      const bitOffset = i * 6;
      // Extract 6 bits
      let sixBits = 0;
      for (let b = 0; b < 6; b++) {
        sixBits = (sixBits << 1) | getBit(xored, bitOffset + b + 1);
      }
      const row = ((sixBits >> 5) << 1) | (sixBits & 1);
      const col = (sixBits >> 1) & 0xf;
      const sVal = S[i][row * 16 + col];
      // Place 4-bit result
      const outOffset = i * 4;
      for (let b = 0; b < 4; b++) {
        setBit(sboxOutput, outOffset + b + 1, (sVal >> (3 - b)) & 1);
      }
    }

    // P permutation
    return permute(sboxOutput, P, 32);
  }

  // Convert password to 8-byte key
  const keyBytes = new Array(8).fill(0);
  for (let i = 0; i < 8 && i < password.length; i++) {
    keyBytes[i] = password.charCodeAt(i);
  }
  // Set parity bits (bit 0 of each byte)
  for (let i = 0; i < 8; i++) {
    keyBytes[i] = (keyBytes[i] << 1) & 0xfe;
  }

  // Convert salt to E-box modifications
  const saltChars = [
    MD5_CRYPT_ITOA64.indexOf(salt[0]) || 0,
    MD5_CRYPT_ITOA64.indexOf(salt[1]) || 0,
  ];
  const saltBits = (saltChars[0] | (saltChars[1] << 6));

  // Generate 16 subkeys
  const pc1Result = permute(keyBytes, PC1, 56);

  let c = new Array(4).fill(0);
  let d = new Array(4).fill(0);
  // Split into C and D halves
  for (let i = 1; i <= 28; i++) {
    setBit(c, i, getBit(pc1Result, i));
    setBit(d, i, getBit(pc1Result, i + 28));
  }

  const subkeys: number[][] = [];
  for (let round = 0; round < 16; round++) {
    c = leftRotate(c, ROTATIONS[round]);
    d = leftRotate(d, ROTATIONS[round]);
    // Combine C and D
    const cd = new Array(7).fill(0);
    for (let i = 1; i <= 28; i++) {
      setBit(cd, i, getBit(c, i));
      setBit(cd, i + 28, getBit(d, i));
    }
    subkeys.push(permute(cd, PC2, 48));
  }

  // Apply salt to expansion table
  const modifiedE = [...E];
  for (let i = 0; i < 12; i++) {
    if ((saltBits >> i) & 1) {
      const tmp = modifiedE[i];
      modifiedE[i] = modifiedE[i + 24];
      modifiedE[i + 24] = tmp;
    }
  }

  // Encrypt: 25 rounds of DES on a zero block
  let block = new Array(8).fill(0);

  for (let iter = 0; iter < 25; iter++) {
    // Initial permutation
    const ipResult = permute(block, IP, 64);

    let left = new Array(4).fill(0);
    let right = new Array(4).fill(0);
    for (let i = 1; i <= 32; i++) {
      setBit(left, i, getBit(ipResult, i));
      setBit(right, i, getBit(ipResult, i + 32));
    }

    // 16 Feistel rounds
    for (let round = 0; round < 16; round++) {
      const fResult = desRound(right, subkeys[round]);
      const newRight = xorBlocks(left, fResult, 32);
      left = right;
      right = newRight;
    }

    // Combine (swap left and right)
    const combined = new Array(8).fill(0);
    for (let i = 1; i <= 32; i++) {
      setBit(combined, i, getBit(right, i));
      setBit(combined, i + 32, getBit(left, i));
    }

    // Final permutation
    block = permute(combined, FP, 64);
  }

  // Encode output
  let output = salt;
  const encode = (a: number, b: number, c: number, n: number): string => {
    let v = (block[a] << 16) | (block[b] << 8) | block[c];
    let s = '';
    for (let i = 0; i < n; i++) {
      s += MD5_CRYPT_ITOA64[v & 0x3f];
      v >>= 6;
    }
    return s;
  };

  output += encode(0, 1, 2, 4);
  output += encode(3, 4, 5, 4);
  output += to64((block[6] << 8) | block[7], 3);

  return output;
}

function verifySha256Crypt(plaintext: string, hash: string): boolean {
  // SHA-256 crypt is complex. For migration purposes, we store the hash
  // and allow verification via external tools if needed.
  // This is a placeholder that could be replaced with a full implementation.
  const parts = hash.split('$');
  if (parts.length < 4) return false;
  const salt = parts[2];
  const computed = computeShaCrypt(plaintext, salt, 'sha256', '$5$');
  return computed === hash;
}

function verifySha512Crypt(plaintext: string, hash: string): boolean {
  const parts = hash.split('$');
  if (parts.length < 4) return false;
  const salt = parts[2];
  const computed = computeShaCrypt(plaintext, salt, 'sha512', '$6$');
  return computed === hash;
}

function computeShaCrypt(password: string, salt: string, algo: string, prefix: string): string {
  const pw = Buffer.from(password, 'utf-8');
  const sl = Buffer.from(salt, 'utf-8');
  const hashLen = algo === 'sha256' ? 32 : 64;

  // Step 1-3: Digest B
  const digestB = createHash(algo).update(pw).update(sl).update(pw).digest();

  // Step 4-8: Digest A
  const ctxA = createHash(algo);
  ctxA.update(pw);
  ctxA.update(sl);
  // Step 9
  let plen = pw.length;
  while (plen > hashLen) {
    ctxA.update(digestB);
    plen -= hashLen;
  }
  ctxA.update(digestB.subarray(0, plen));

  // Step 10
  for (let i = pw.length; i > 0; i >>= 1) {
    if (i & 1) {
      ctxA.update(digestB);
    } else {
      ctxA.update(pw);
    }
  }
  let digestA = ctxA.digest();

  // Step 12: Digest DP (password hash)
  const ctxDP = createHash(algo);
  for (let i = 0; i < pw.length; i++) {
    ctxDP.update(pw);
  }
  const digestDP = ctxDP.digest();

  // Step 13: P string
  const P = Buffer.alloc(pw.length);
  let offset = 0;
  while (offset + hashLen <= pw.length) {
    digestDP.copy(P, offset, 0, hashLen);
    offset += hashLen;
  }
  if (offset < pw.length) {
    digestDP.copy(P, offset, 0, pw.length - offset);
  }

  // Step 15: Digest DS (salt hash)
  const ctxDS = createHash(algo);
  for (let i = 0; i < 16 + digestA[0]; i++) {
    ctxDS.update(sl);
  }
  const digestDS = ctxDS.digest();

  // Step 16: S string
  const S = Buffer.alloc(sl.length);
  offset = 0;
  while (offset + hashLen <= sl.length) {
    digestDS.copy(S, offset, 0, hashLen);
    offset += hashLen;
  }
  if (offset < sl.length) {
    digestDS.copy(S, offset, 0, sl.length - offset);
  }

  // Step 17: 5000 rounds
  const rounds = 5000;
  for (let i = 0; i < rounds; i++) {
    const ctx = createHash(algo);
    if (i & 1) ctx.update(P); else ctx.update(digestA);
    if (i % 3) ctx.update(S);
    if (i % 7) ctx.update(P);
    if (i & 1) ctx.update(digestA); else ctx.update(P);
    digestA = ctx.digest();
  }

  // Encode
  const ITOA64 = './0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';
  const enc = (a: number, b: number, c: number, n: number): string => {
    let v = (digestA[a] << 16) | (digestA[b] << 8) | digestA[c];
    let s = '';
    for (let i = 0; i < n; i++) {
      s += ITOA64[v & 0x3f];
      v >>= 6;
    }
    return s;
  };

  let result = prefix + salt + '$';
  if (algo === 'sha256') {
    result += enc(0, 10, 20, 4) + enc(21, 1, 11, 4) + enc(12, 22, 2, 4);
    result += enc(3, 13, 23, 4) + enc(24, 4, 14, 4) + enc(15, 25, 5, 4);
    result += enc(6, 16, 26, 4) + enc(27, 7, 17, 4) + enc(18, 28, 8, 4);
    result += enc(9, 19, 29, 4) + to64((digestA[30] << 8) | digestA[31], 3);
  } else {
    result += enc(0, 21, 42, 4) + enc(22, 43, 1, 4) + enc(44, 2, 23, 4);
    result += enc(3, 24, 45, 4) + enc(25, 46, 4, 4) + enc(47, 5, 26, 4);
    result += enc(6, 27, 48, 4) + enc(28, 49, 7, 4) + enc(50, 8, 29, 4);
    result += enc(9, 30, 51, 4) + enc(31, 52, 10, 4) + enc(53, 11, 32, 4);
    result += enc(12, 33, 54, 4) + enc(34, 55, 13, 4) + enc(56, 14, 35, 4);
    result += enc(15, 36, 57, 4) + enc(37, 58, 16, 4) + enc(59, 17, 38, 4);
    result += enc(18, 39, 60, 4) + enc(40, 61, 19, 4) + enc(62, 20, 41, 4);
    result += to64(digestA[63], 2);
  }

  return result;
}

// ---------------------------------------------------------------------------
// Account linking
// ---------------------------------------------------------------------------

/**
 * Link a legacy character to a Firebase Auth user.
 *
 * Finds the legacy character by name, verifies the password hash matches
 * the one stored during migration, and if so, sets the character's ownerId
 * to the Firebase uid.
 *
 * @param uid - Firebase Auth user ID
 * @param charName - Legacy character name (case-insensitive)
 * @param password - Plaintext password to verify against legacy hash
 * @param db - Firestore instance
 * @returns true if the account was linked successfully
 */
export async function linkLegacyAccount(
  uid: string,
  charName: string,
  password: string,
  db: FirebaseFirestore.Firestore,
): Promise<boolean> {
  // Look up the legacy account mapping
  const mappingSnap = await db
    .collection('legacy_accounts')
    .where('nameLower', '==', charName.toLowerCase())
    .limit(1)
    .get();

  if (mappingSnap.empty) {
    console.log(`linkLegacyAccount: no legacy character found with name "${charName}"`);
    return false;
  }

  const mappingDoc = mappingSnap.docs[0];
  const mapping = mappingDoc.data();

  // Check if already linked
  if (mapping.linkedUid) {
    console.log(`linkLegacyAccount: "${charName}" is already linked to uid ${mapping.linkedUid}`);
    return false;
  }

  // Verify password
  const legacyHash = mapping.passwordHash as string;
  if (!legacyHash) {
    console.log(`linkLegacyAccount: no password hash stored for "${charName}"`);
    return false;
  }

  if (!verifyLegacyPassword(password, legacyHash)) {
    console.log(`linkLegacyAccount: password verification failed for "${charName}"`);
    return false;
  }

  // Link the account: update the character document and the mapping
  const charId = mapping.characterId as string;

  const batch = db.batch();

  // Update the character document
  batch.update(db.collection('characters').doc(charId), {
    ownerId: uid,
  });

  // Update the legacy_accounts mapping
  batch.update(mappingDoc.ref, {
    linkedUid: uid,
    linkedAt: new Date(),
  });

  // Add the character to the user's character list
  const userRef = db.collection('users').doc(uid);
  const userSnap = await userRef.get();
  if (userSnap.exists) {
    const userData = userSnap.data()!;
    const charIds = (userData.characterIds as string[]) || [];
    if (!charIds.includes(charId)) {
      batch.update(userRef, {
        characterIds: [...charIds, charId],
      });
    }
  }

  await batch.commit();

  console.log(`linkLegacyAccount: successfully linked "${charName}" (${charId}) to uid ${uid}`);
  return true;
}

// ---------------------------------------------------------------------------
// Scan player directory
// ---------------------------------------------------------------------------

interface PlayerFileInfo {
  name: string;
  filePath: string;
}

/**
 * Scan the player directory for all player files.
 * The directory is organized as player/A/, player/B/, etc.
 * Each file inside is named after the character (capitalized).
 * Files may also have .gz extension.
 */
function scanPlayerDirectory(playerDir: string): PlayerFileInfo[] {
  const results: PlayerFileInfo[] = [];

  if (!existsSync(playerDir)) {
    console.error(`Player directory not found: ${playerDir}`);
    return results;
  }

  const entries = readdirSync(playerDir);

  for (const entry of entries) {
    const subPath = join(playerDir, entry);
    const stat = statSync(subPath);

    if (stat.isDirectory()) {
      // Subdirectory (e.g., A/, B/, etc.)
      const subEntries = readdirSync(subPath);
      for (const subEntry of subEntries) {
        const filePath = join(subPath, subEntry);
        const fileStat = statSync(filePath);
        if (fileStat.isFile()) {
          // Strip .gz extension if present for the name
          const name = subEntry.replace(/\.gz$/, '');
          results.push({ name, filePath });
        }
      }
    } else if (stat.isFile()) {
      // File directly in the player directory
      const name = entry.replace(/\.gz$/, '');
      results.push({ name, filePath: subPath });
    }
  }

  return results;
}

// ---------------------------------------------------------------------------
// Main migration function
// ---------------------------------------------------------------------------

/**
 * Migrate all player files from the legacy C MUD format into Firestore.
 *
 * @param playerDir - Path to the player directory (e.g., /path/to/player/)
 * @param db - Firestore instance
 * @returns Migration statistics
 */
export async function migrateAllPlayers(
  playerDir: string,
  db: FirebaseFirestore.Firestore,
): Promise<{ migrated: number; errors: number; skipped: number }> {
  const files = scanPlayerDirectory(playerDir);
  console.log(`Found ${files.length} player file(s) in ${playerDir}`);

  if (files.length === 0) {
    return { migrated: 0, errors: 0, skipped: 0 };
  }

  const writer = new BatchWriter(db);
  let migrated = 0;
  let errors = 0;
  let skipped = 0;

  for (let i = 0; i < files.length; i++) {
    const fileInfo = files[i];
    const progress = `[${i + 1}/${files.length}]`;

    try {
      // Read and decompress the file
      const content = readPlayerFile(fileInfo.filePath);

      // Skip empty files
      if (!content.trim()) {
        console.log(`${progress} Skipping empty file: ${fileInfo.name}`);
        skipped++;
        continue;
      }

      // Parse the player file
      const player = parsePlayerFile(content);

      // Set the name from the filename if not read from file content
      if (!player.name) {
        player.name = fileInfo.name;
      }

      // Skip level 1 characters (the C server doesn't save them either,
      // but there might be stale files)
      if (player.level < 2) {
        console.log(`${progress} Skipping level ${player.level} character: ${player.name}`);
        skipped++;
        continue;
      }

      // Generate a deterministic character ID based on the name
      // This ensures idempotent re-runs of the migration
      const charId = `legacy_${player.name.toLowerCase()}`;

      // Convert to Firestore document
      const doc = legacyToFirestoreDoc(player, charId);

      // Write character document
      const charRef = db.collection('characters').doc(charId);
      writer.set(charRef, doc);

      // Write legacy account mapping (for account linking)
      const mappingRef = db.collection('legacy_accounts').doc(charId);
      writer.set(mappingRef, {
        characterId: charId,
        name: player.name,
        nameLower: player.name.toLowerCase(),
        level: player.level,
        charClass: player.charClass,
        race: player.race,
        passwordHash: player.pwd || null,
        linkedUid: null,
        linkedAt: null,
        importedAt: new Date(),
      });

      migrated++;
      console.log(
        `${progress} Migrated: ${player.name} (Level ${player.level}, ` +
        `Class ${player.charClass}, ${player.objects.length} objects, ` +
        `${player.skills.length} skills)`
      );

    } catch (err) {
      errors++;
      console.error(`${progress} ERROR migrating ${fileInfo.name}:`, err);
    }
  }

  // Flush remaining writes
  console.log(`\nFlushing ${writer.total} write(s) to Firestore...`);
  await writer.flush();

  return { migrated, errors, skipped };
}

// ---------------------------------------------------------------------------
// CLI entry point
// ---------------------------------------------------------------------------

async function main(): Promise<void> {
  const playerDir = resolve(process.argv[2] || join(__dirname, '..', '..', '..', '..', 'player'));

  if (!existsSync(playerDir)) {
    console.error(`Player directory not found: ${playerDir}`);
    console.error('Usage: npx tsx src/migration/player-migration.ts /path/to/player/directory');
    process.exit(1);
  }

  console.log(`Player directory: ${playerDir}`);
  console.log('');

  // ---------------------------------------------------------------------------
  // Firebase Admin Setup
  // ---------------------------------------------------------------------------

  const serviceAccountPath = process.env.GOOGLE_APPLICATION_CREDENTIALS
    || join(__dirname, '..', '..', 'service-account.json');

  if (existsSync(serviceAccountPath)) {
    const sa = JSON.parse(readFileSync(serviceAccountPath, 'utf-8')) as ServiceAccount;
    try {
      initializeApp({ credential: cert(sa) });
    } catch {
      // App may already be initialized
    }
  } else {
    try {
      initializeApp();
    } catch {
      // App may already be initialized
    }
  }

  const db = getFirestore();

  // ---------------------------------------------------------------------------
  // Run migration
  // ---------------------------------------------------------------------------

  console.log('Starting player migration...');
  console.log('='.repeat(60));

  const startTime = Date.now();
  const stats = await migrateAllPlayers(playerDir, db);
  const elapsed = ((Date.now() - startTime) / 1000).toFixed(1);

  console.log('');
  console.log('='.repeat(60));
  console.log('Migration complete!');
  console.log(`  Migrated: ${stats.migrated}`);
  console.log(`  Errors:   ${stats.errors}`);
  console.log(`  Skipped:  ${stats.skipped}`);
  console.log(`  Time:     ${elapsed}s`);
  console.log('');

  if (stats.errors > 0) {
    console.log('Some files had errors. Review the output above for details.');
    process.exit(1);
  }
}

// Run if this file is executed directly
const isMain = process.argv[1] && resolve(process.argv[1]) === resolve(__filename);
if (isMain) {
  main().catch((err) => {
    console.error('Migration failed:', err);
    process.exit(1);
  });
}
