/**
 * area-parser.ts — Stormgate MUD area file parser
 *
 * Parses Stormgate .are files (Diku/Merc/Envy-derived format) and returns
 * structured TypeScript objects.  The format is the custom Stormgate variant
 * with sections: #AREADATA, #MOBILES, #OBJECTS, #ROOMDATA, #RESETS, #SHOPS,
 * #SPECIALS, #GAMES, and the #$ end marker.
 */

import { readFileSync } from 'fs';
import { join } from 'path';

// ---------------------------------------------------------------------------
// Type definitions
// ---------------------------------------------------------------------------

export interface AreaMetadata {
  name: string;
  builders: string;
  vnumLow: number;
  vnumHigh: number;
  security: number;
  recall: number;
  flags: number;
  version: number;
  creator: string;
  lowerLevel: number;
  upperLevel: number;
  sounds?: string;
  music?: string;
}

export interface MobAffect {
  location: number;
  modifier: number;
}

export interface MobProg {
  trigger: string;
  argument: string;
  commands: string;
}

export interface ParsedMobile {
  vnum: number;
  keywords: string;
  shortDescription: string;
  longDescription: string;
  description: string;
  actFlags: number;
  affectedBy: number;
  affectedBy2: number;
  affectedBy3: number;
  bodyParts: number;
  alignment: number;
  level: number;
  hitNoDice: number;
  hitSizeDice: number;
  hitPlus: number;
  hpPlus: number;
  damNoDice: number;
  damSizeDice: number;
  damPlus: number;
  gold: number;
  hitroll: number;
  damroll: number;
  ac: number;
  acUnknown: number;
  sex: number;
  size: number;
  shields: number;
  speaking: number;
  mobProgs: MobProg[];
}

export interface ObjAffect {
  location: number;
  modifier: number;
}

export interface ObjExtraDesc {
  keyword: string;
  description: string;
}

export interface ParsedObject {
  vnum: number;
  keywords: string;
  shortDescription: string;
  longDescription: string;
  itemType: number;
  extraFlags: number;
  extraFlags2: number;
  extraFlags3: number;
  extraFlags4: number;
  wearFlags: number;
  level: number;
  conditionNow: number;
  conditionMax: number;
  values: string[];
  weight: number;
  cost: number;
  costMult: number;
  charges: number;
  spellLevel: number;
  spellName: string;
  timer: number;
  timerArg: number;
  extraFlags5: number;
  extraFlags6: number;
  extraFlags7: number;
  affects: ObjAffect[];
  extraDescs: ObjExtraDesc[];
}

export interface RoomExit {
  direction: number;
  description: string;
  keyword: string;
  locks: number;
  keyVnum: number;
  toVnum: number;
}

export interface RoomExtraDesc {
  keyword: string;
  description: string;
}

export interface ParsedRoom {
  vnum: number;
  name: string;
  tilde1: string;
  tilde2: string;
  description: string;
  areaNumber: number;
  roomFlags: number;
  sectorType: number;
  roomData: string;
  exits: RoomExit[];
  extraDescs: RoomExtraDesc[];
}

export interface ParsedReset {
  command: string;
  arg1: number;
  arg2: number;
  arg3: number;
  arg4: number;
  comment: string;
}

export interface ParsedShop {
  keeper: number;
  buyType1: number;
  buyType2: number;
  buyType3: number;
  buyType4: number;
  buyType5: number;
  profitBuy: number;
  profitSell: number;
  openHour: number;
  closeHour: number;
}

export interface ParsedSpecial {
  mobVnum: number;
  specFun: string;
}

export interface ParsedGame {
  mobVnum: number;
  gameFun: string;
  args: string;
}

export interface ParsedArea {
  metadata: AreaMetadata;
  mobiles: ParsedMobile[];
  objects: ParsedObject[];
  rooms: ParsedRoom[];
  resets: ParsedReset[];
  shops: ParsedShop[];
  specials: ParsedSpecial[];
  games: ParsedGame[];
}

// ---------------------------------------------------------------------------
// Low-level parsing helpers
// ---------------------------------------------------------------------------

/**
 * Stateful reader that tracks a position through a string.
 */
class AreaReader {
  private pos = 0;
  private content: string;

  constructor(content: string) {
    this.content = content;
  }

  get position(): number {
    return this.pos;
  }

  /** True when we have consumed the entire content. */
  get eof(): boolean {
    return this.pos >= this.content.length;
  }

  /** Peek at the current character without advancing. */
  peek(): string {
    return this.content[this.pos] ?? '';
  }

  /** Peek at the next N characters without advancing. */
  peekAhead(n: number): string {
    return this.content.slice(this.pos, this.pos + n);
  }

  /** Advance by one character and return it. */
  advance(): string {
    return this.content[this.pos++] ?? '';
  }

  /** Skip whitespace (spaces, tabs, carriage returns, newlines). */
  skipWhitespace(): void {
    while (this.pos < this.content.length && /\s/.test(this.content[this.pos])) {
      this.pos++;
    }
  }

  /** Skip to the end of the current line (past the newline). */
  skipLine(): void {
    while (this.pos < this.content.length && this.content[this.pos] !== '\n') {
      this.pos++;
    }
    if (this.pos < this.content.length) this.pos++; // skip the \n
  }

  /** Read the rest of the current line (trimmed), advancing past the newline. */
  readLine(): string {
    const start = this.pos;
    while (this.pos < this.content.length && this.content[this.pos] !== '\n' && this.content[this.pos] !== '\r') {
      this.pos++;
    }
    const line = this.content.slice(start, this.pos).trim();
    // skip \r\n or \n
    if (this.pos < this.content.length && this.content[this.pos] === '\r') this.pos++;
    if (this.pos < this.content.length && this.content[this.pos] === '\n') this.pos++;
    return line;
  }

  /**
   * Read a tilde-terminated string.  The tilde may appear mid-line or on its
   * own line.  Everything up to (but not including) the tilde is returned.
   * The tilde itself and any trailing whitespace on the same line are consumed.
   */
  readTildeString(): string {
    const parts: string[] = [];
    while (this.pos < this.content.length) {
      const ch = this.content[this.pos];
      if (ch === '~') {
        this.pos++; // consume tilde
        // consume the rest of the line after the tilde
        while (this.pos < this.content.length && this.content[this.pos] !== '\n' && this.content[this.pos] !== '\r') {
          this.pos++;
        }
        if (this.pos < this.content.length && this.content[this.pos] === '\r') this.pos++;
        if (this.pos < this.content.length && this.content[this.pos] === '\n') this.pos++;
        break;
      }
      this.pos++;
      parts.push(ch);
    }
    return parts.join('').trim();
  }

  /**
   * Read the next whitespace-delimited word.
   */
  readWord(): string {
    this.skipWhitespace();
    const start = this.pos;
    while (this.pos < this.content.length && !/\s/.test(this.content[this.pos])) {
      this.pos++;
    }
    return this.content.slice(start, this.pos);
  }

  /**
   * Read the next whitespace-delimited token as an integer.
   */
  readNumber(): number {
    const word = this.readWord();
    const n = parseInt(word, 10);
    if (isNaN(n)) {
      console.warn(`area-parser: expected number, got "${word}" near position ${this.pos}`);
      return 0;
    }
    return n;
  }

  /**
   * Read a tilde-terminated string that appears on the same line as other
   * data.  This reads up to the next tilde, consuming the tilde but NOT
   * consuming any trailing newline.  Used for inline values like "0~".
   */
  readInlineTildeString(): string {
    this.skipWhitespace();
    const parts: string[] = [];
    while (this.pos < this.content.length) {
      const ch = this.content[this.pos];
      if (ch === '~') {
        this.pos++; // consume tilde
        break;
      }
      if (ch === '\n' || ch === '\r') {
        // tilde missing, bail
        break;
      }
      this.pos++;
      parts.push(ch);
    }
    return parts.join('').trim();
  }
}

// ---------------------------------------------------------------------------
// Section parsers
// ---------------------------------------------------------------------------

function parseAreaData(reader: AreaReader): AreaMetadata {
  const meta: AreaMetadata = {
    name: '',
    builders: '',
    vnumLow: 0,
    vnumHigh: 0,
    security: 0,
    recall: 0,
    flags: 0,
    version: 0,
    creator: '',
    lowerLevel: 0,
    upperLevel: 0,
  };

  while (!reader.eof) {
    const line = reader.readLine();
    if (!line || line === 'End') break;

    // Parse key/value.  The key is the first word.
    const keyMatch = line.match(/^(\S+)\s+(.*)/);
    if (!keyMatch) continue;
    const key = keyMatch[1];
    let value = keyMatch[2].trim();

    switch (key) {
      case 'Name': {
        // Name may have a level range in braces: { 50-70} StoneGate Castle~
        // Strip the tilde at the end.
        const nameMatch = value.match(/(?:\{[^}]*\}\s*)?(.*)~?$/);
        meta.name = nameMatch ? nameMatch[1].replace(/~$/, '').trim() : value.replace(/~$/, '').trim();
        // Also keep the level range part intact in the name
        meta.name = value.replace(/~$/, '').trim();
        break;
      }
      case 'Builders':
        meta.builders = value.replace(/~$/, '').trim();
        break;
      case 'VNUMs': {
        const parts = value.split(/\s+/);
        meta.vnumLow = parseInt(parts[0], 10) || 0;
        meta.vnumHigh = parseInt(parts[1], 10) || 0;
        break;
      }
      case 'Security':
        meta.security = parseInt(value, 10) || 0;
        break;
      case 'Recall':
        meta.recall = parseInt(value, 10) || 0;
        break;
      case 'Flags':
        meta.flags = parseInt(value, 10) || 0;
        break;
      case 'Version':
        meta.version = parseInt(value, 10) || 0;
        break;
      case 'Creator':
        meta.creator = value.replace(/~$/, '').trim();
        break;
      case 'Llevel':
        meta.lowerLevel = parseInt(value, 10) || 0;
        break;
      case 'Ulevel':
        meta.upperLevel = parseInt(value, 10) || 0;
        break;
      case 'Sounds':
        meta.sounds = value.replace(/~$/, '').trim();
        break;
      case 'Music':
        meta.music = value.replace(/~$/, '').trim();
        break;
      default:
        // Unknown key — just skip it
        break;
    }
  }

  return meta;
}

/**
 * Parse the dice notation "NdS+P" (e.g. "300000d0+0") into the three
 * components.  Also handles negative values ("0d0+0").
 */
function parseDice(s: string): { num: number; size: number; plus: number } {
  const m = s.match(/^(-?\d+)d(-?\d+)\+(-?\d+)$/);
  if (!m) {
    console.warn(`area-parser: bad dice notation "${s}"`);
    return { num: 0, size: 0, plus: 0 };
  }
  return {
    num: parseInt(m[1], 10),
    size: parseInt(m[2], 10),
    plus: parseInt(m[3], 10),
  };
}

function parseMobiles(reader: AreaReader): ParsedMobile[] {
  const mobs: ParsedMobile[] = [];

  while (!reader.eof) {
    reader.skipWhitespace();
    if (reader.peek() !== '#') break;
    reader.advance(); // consume '#'

    const vnumStr = reader.readWord();
    if (vnumStr === '0') break; // end of section

    const vnum = parseInt(vnumStr, 10);
    if (isNaN(vnum)) {
      console.warn(`area-parser: bad mob vnum "${vnumStr}"`);
      break;
    }

    // keywords~
    const keywords = reader.readTildeString();
    // short_description~
    const shortDescription = reader.readTildeString();
    // long_description (may span lines, terminated by ~)
    const longDescription = reader.readTildeString();
    // description (may span lines, terminated by ~)
    const description = reader.readTildeString();

    // Line: act_flags affected_by [affected_by2] [affected_by3] [body_parts] alignment S|B|number
    // The number of fields varies but always ends with S (or a letter).
    // Format: act_flags affected_by affected_by2 affected_by3 body_parts alignment S
    // From the files: "1 0 0 0 8 0 S" or "524353 0 0 0 8 -1000 S"
    const flagLine = reader.readLine();
    const flagParts = flagLine.split(/\s+/);

    let actFlags = 0;
    let affectedBy = 0;
    let affectedBy2 = 0;
    let affectedBy3 = 0;
    let bodyParts = 0;
    let alignment = 0;

    // The last token should be 'S' (or 'B' or a number for extended format).
    // Typical: actFlags affectedBy affectedBy2 affectedBy3 bodyParts alignment S
    if (flagParts.length >= 7) {
      actFlags = parseInt(flagParts[0], 10) || 0;
      affectedBy = parseInt(flagParts[1], 10) || 0;
      affectedBy2 = parseInt(flagParts[2], 10) || 0;
      affectedBy3 = parseInt(flagParts[3], 10) || 0;
      bodyParts = parseInt(flagParts[4], 10) || 0;
      alignment = parseInt(flagParts[5], 10) || 0;
    } else if (flagParts.length >= 4) {
      actFlags = parseInt(flagParts[0], 10) || 0;
      affectedBy = parseInt(flagParts[1], 10) || 0;
      alignment = parseInt(flagParts[2], 10) || 0;
    }

    // Line: level hitnodice hitsizedice hitplus hpplus damnodice_dice damplus
    // Format: level hitNoDice hitSizeDice hitPlus hpPlus hpNoDice_d_hpSizeDice+hpPlus damNoDice_d_damSizeDice+damPlus
    // Example: "115 1 230 230 300000 300000d0+0 0d0+0"
    const statsLine = reader.readLine();
    const statParts = statsLine.split(/\s+/);

    let level = 0;
    let hitNoDice = 0;
    let hitSizeDice = 0;
    let hitPlus = 0;
    let hpPlus = 0;
    let damNoDice = 0;
    let damSizeDice = 0;
    let damPlus = 0;

    if (statParts.length >= 7) {
      level = parseInt(statParts[0], 10) || 0;
      hitNoDice = parseInt(statParts[1], 10) || 0;
      hitSizeDice = parseInt(statParts[2], 10) || 0;
      hitPlus = parseInt(statParts[3], 10) || 0;

      // hpPlus and hp dice: "300000 300000d0+0"
      // statParts[4] is hpPlus as a straight number
      hpPlus = parseInt(statParts[4], 10) || 0;

      // statParts[5] is hpDice in NdS+P format (redundant hp info)
      // statParts[6] is damDice in NdS+P format
      const damDice = parseDice(statParts[6]);
      damNoDice = damDice.num;
      damSizeDice = damDice.size;
      damPlus = damDice.plus;
    }

    // Line: gold hitroll damroll
    const goldLine = reader.readLine();
    const goldParts = goldLine.split(/\s+/);
    const gold = parseInt(goldParts[0], 10) || 0;
    const hitroll = parseInt(goldParts[1], 10) || 0;
    const damroll = parseInt(goldParts[2], 10) || 0;

    // Line: ac acUnknown sex
    const acLine = reader.readLine();
    const acParts = acLine.split(/\s+/);
    const ac = parseInt(acParts[0], 10) || 0;
    const acUnknown = parseInt(acParts[1], 10) || 0;
    const sex = parseInt(acParts[2], 10) || 0;

    // Line: size shields speaking
    const sizeLine = reader.readLine();
    const sizeParts = sizeLine.split(/\s+/);
    const size = parseInt(sizeParts[0], 10) || 0;
    const shields = parseInt(sizeParts[1], 10) || 0;
    const speaking = parseInt(sizeParts[2], 10) || 0;

    // Now check for mob programs (lines starting with >)
    const mobProgs: MobProg[] = [];
    while (!reader.eof) {
      reader.skipWhitespace();
      const ch = reader.peek();
      if (ch === '>') {
        // Mob program trigger line: >trigger_type argument~
        reader.advance(); // consume '>'
        const triggerLine = reader.readTildeString();
        // triggerLine is like "fight_prog 10" or "speech_prog hello hi"
        const spaceIdx = triggerLine.indexOf(' ');
        let trigger: string;
        let argument: string;
        if (spaceIdx >= 0) {
          trigger = triggerLine.slice(0, spaceIdx).trim();
          argument = triggerLine.slice(spaceIdx + 1).trim();
        } else {
          trigger = triggerLine.trim();
          argument = '';
        }

        // Read commands until ~
        const commands = reader.readTildeString();

        mobProgs.push({ trigger, argument, commands });
      } else if (ch === '|') {
        // End of mob programs for this mob
        reader.advance(); // consume '|'
        reader.skipLine();
        break;
      } else {
        // No mob programs or end of programs
        break;
      }
    }

    mobs.push({
      vnum,
      keywords,
      shortDescription,
      longDescription,
      description,
      actFlags,
      affectedBy,
      affectedBy2,
      affectedBy3,
      bodyParts,
      alignment,
      level,
      hitNoDice,
      hitSizeDice,
      hitPlus,
      hpPlus,
      damNoDice,
      damSizeDice,
      damPlus,
      gold,
      hitroll,
      damroll,
      ac,
      acUnknown,
      sex,
      size,
      shields,
      speaking,
      mobProgs,
    });
  }

  return mobs;
}

function parseObjects(reader: AreaReader): ParsedObject[] {
  const objects: ParsedObject[] = [];

  while (!reader.eof) {
    reader.skipWhitespace();
    if (reader.peek() !== '#') break;
    reader.advance(); // consume '#'

    const vnumStr = reader.readWord();
    if (vnumStr === '0') break;

    const vnum = parseInt(vnumStr, 10);
    if (isNaN(vnum)) {
      console.warn(`area-parser: bad obj vnum "${vnumStr}"`);
      break;
    }

    // keywords~
    const keywords = reader.readTildeString();
    // short_description~
    const shortDescription = reader.readTildeString();
    // long_description~  (or multi-line terminated by ~)
    const longDescription = reader.readTildeString();
    // Extra tilde for extended description (usually empty)
    // In limbo.are the pattern is: long_desc~\n~\n
    // But readTildeString already consumed the first ~.
    // The second ~ is the empty extended description.
    // Actually looking at the data more carefully:
    // line: "Dummy object is used for loading non-existant objects~"
    // line: "~"   <-- this is the empty extended description
    // So we need to read one more tilde string (which will be empty).
    // BUT: Some objects have the long desc terminated with ~ on same line,
    // then an empty ~ on the next line.
    //
    // The pattern in the files is:
    //   short_desc~
    //   long_desc~   (or multi-line ending with ~)
    //   ~            (always present - terminates a 4th string field)
    //
    // We already read keywords~, shortDesc~, longDesc~
    // Now we need to consume the empty ~
    reader.readTildeString(); // consume the empty material/unused field

    // Line: item_type extra_flags extra_flags2 extra_flags3 extra_flags4 wear_flags level
    // Example: "13 0 0 0 0 0 0"
    const typeLine = reader.readLine();
    const typeParts = typeLine.split(/\s+/);
    const itemType = parseInt(typeParts[0], 10) || 0;
    const extraFlags = parseInt(typeParts[1], 10) || 0;
    const extraFlags2 = parseInt(typeParts[2], 10) || 0;
    const extraFlags3 = parseInt(typeParts[3], 10) || 0;
    const extraFlags4 = parseInt(typeParts[4], 10) || 0;
    const wearFlags = parseInt(typeParts[5], 10) || 0;
    const level = parseInt(typeParts[6], 10) || 0;

    // Line: condition_now condition_max
    // Example: "100 100"
    const condLine = reader.readLine();
    const condParts = condLine.split(/\s+/);
    const conditionNow = parseInt(condParts[0], 10) || 0;
    const conditionMax = parseInt(condParts[1], 10) || 0;

    // Line: value0~ value1~ value2~ value3~
    // Example: "0~ 0~ 0~ 0~"
    // Each value is terminated by ~, all on one line.
    const values: string[] = [];
    for (let i = 0; i < 4; i++) {
      values.push(reader.readInlineTildeString());
    }
    // Skip to end of this line
    reader.readLine();

    // Line: weight cost costMult
    // Example: "0 1 0"
    const weightLine = reader.readLine();
    const weightParts = weightLine.split(/\s+/);
    const weight = parseInt(weightParts[0], 10) || 0;
    const cost = parseInt(weightParts[1], 10) || 0;
    const costMult = parseInt(weightParts[2], 10) || 0;

    // Line: charges spellLevel
    // Example: "0 0"
    const chargeLine = reader.readLine();
    const chargeParts = chargeLine.split(/\s+/);
    const charges = parseInt(chargeParts[0], 10) || 0;
    const spellLevel = parseInt(chargeParts[1], 10) || 0;

    // Line: spellName~   (tilde-terminated)
    // Example: "reserved~" or "~" or "haste~"
    const spellName = reader.readTildeString();

    // Line: timer timerArg
    // Example: "0 0"
    const timerLine = reader.readLine();
    const timerParts = timerLine.split(/\s+/);
    const timer = parseInt(timerParts[0], 10) || 0;
    const timerArg = parseInt(timerParts[1], 10) || 0;

    // Line: extraFlags5 extraFlags6 extraFlags7
    // Example: "0 0 0"
    const efLine = reader.readLine();
    const efParts = efLine.split(/\s+/);
    const extraFlags5 = parseInt(efParts[0], 10) || 0;
    const extraFlags6 = parseInt(efParts[1], 10) || 0;
    const extraFlags7 = parseInt(efParts[2], 10) || 0;

    // Now read optional A (affect), E (extra desc), F (extra flags) lines
    const affects: ObjAffect[] = [];
    const extraDescs: ObjExtraDesc[] = [];

    while (!reader.eof) {
      reader.skipWhitespace();
      const ch = reader.peek();

      if (ch === 'A') {
        reader.advance(); // consume 'A'
        reader.readLine(); // consume rest of line (should be empty or just 'A')
        // Next line: location modifier
        const affLine = reader.readLine();
        const affParts = affLine.split(/\s+/);
        affects.push({
          location: parseInt(affParts[0], 10) || 0,
          modifier: parseInt(affParts[1], 10) || 0,
        });
      } else if (ch === 'E') {
        reader.advance(); // consume 'E'
        reader.readLine(); // consume rest of line
        // keyword~
        const keyword = reader.readTildeString();
        // description~
        const desc = reader.readTildeString();
        extraDescs.push({ keyword, description: desc });
      } else if (ch === 'F') {
        reader.advance(); // consume 'F'
        reader.readLine(); // consume rest of line -- skip F lines
        reader.readLine(); // consume the data line
      } else if (ch === '>') {
        // Object programs (mob-prog-style triggers attached to objects).
        // Skip all >prog blocks until the | terminator.
        while (!reader.eof) {
          reader.skipWhitespace();
          const progCh = reader.peek();
          if (progCh === '>') {
            reader.advance(); // consume '>'
            reader.readTildeString(); // trigger line up to ~
            reader.readTildeString(); // commands up to ~
          } else if (progCh === '|') {
            reader.advance();
            reader.readLine();
            break;
          } else {
            break;
          }
        }
      } else {
        break; // next object or section
      }
    }

    objects.push({
      vnum,
      keywords,
      shortDescription,
      longDescription,
      itemType,
      extraFlags,
      extraFlags2,
      extraFlags3,
      extraFlags4,
      wearFlags,
      level,
      conditionNow,
      conditionMax,
      values,
      weight,
      cost,
      costMult,
      charges,
      spellLevel,
      spellName,
      timer,
      timerArg,
      extraFlags5,
      extraFlags6,
      extraFlags7,
      affects,
      extraDescs,
    });
  }

  return objects;
}

function parseRooms(reader: AreaReader): ParsedRoom[] {
  const rooms: ParsedRoom[] = [];

  while (!reader.eof) {
    reader.skipWhitespace();
    if (reader.peek() !== '#') break;
    reader.advance(); // consume '#'

    const vnumStr = reader.readWord();
    if (vnumStr === '0') break;

    const vnum = parseInt(vnumStr, 10);
    if (isNaN(vnum)) {
      console.warn(`area-parser: bad room vnum "${vnumStr}"`);
      break;
    }

    // name~
    const name = reader.readTildeString();
    // Two tilde-terminated strings (often empty) — observed in the files
    // Pattern: name~\n~\n~\n  or  name~\nsubtitle~\n~\n
    const tilde1 = reader.readTildeString();
    const tilde2 = reader.readTildeString();
    // description (multi-line, terminated by ~)
    const description = reader.readTildeString();

    // Line: area_number room_flags sector_type
    // Example: "0 393228 1"
    const flagLine = reader.readLine();
    const flagParts = flagLine.split(/\s+/);
    const areaNumber = parseInt(flagParts[0], 10) || 0;
    const roomFlags = parseInt(flagParts[1], 10) || 0;
    const sectorType = parseInt(flagParts[2], 10) || 0;

    // Line: "Rd     0" (room data — appears to be a constant in these files)
    const roomData = reader.readLine();

    // Now read optional lines until 'S' (end of room)
    const exits: RoomExit[] = [];
    const extraDescs: RoomExtraDesc[] = [];

    while (!reader.eof) {
      reader.skipWhitespace();
      const ch = reader.peek();
      const ahead = reader.peekAhead(2);

      if (ch === 'S') {
        // Check if it's just 'S' on a line by itself (end of room)
        // vs a section header like '#SPECIALS'
        reader.advance(); // consume 'S'
        reader.readLine(); // consume rest of line

        // Some rooms have mob-program-style triggers (>prog) attached
        // after the S marker.  Skip them so the next room can be read.
        while (!reader.eof) {
          reader.skipWhitespace();
          const nextCh = reader.peek();
          if (nextCh === '>') {
            // Skip the trigger line (>trigger_type arg~)
            reader.advance(); // consume '>'
            reader.readTildeString(); // consume trigger line up to ~
            // Skip the commands until ~
            reader.readTildeString();
          } else if (nextCh === '|') {
            // End of programs marker
            reader.advance();
            reader.readLine();
            break;
          } else {
            break;
          }
        }

        break;
      } else if (ch === 'D' && /^D[0-5]/.test(ahead)) {
        // Exit: D0 through D5
        reader.advance(); // consume 'D'
        const dirStr = reader.readLine(); // rest of line = direction number
        const direction = parseInt(dirStr, 10) || 0;
        // description~
        const exitDesc = reader.readTildeString();
        // keyword~
        const keyword = reader.readTildeString();
        // locks key_vnum to_vnum
        const exitLine = reader.readLine();
        const exitParts = exitLine.split(/\s+/);
        exits.push({
          direction,
          description: exitDesc,
          keyword,
          locks: parseInt(exitParts[0], 10) || 0,
          keyVnum: parseInt(exitParts[1], 10) || 0,
          toVnum: parseInt(exitParts[2], 10) || 0,
        });
      } else if (ch === 'E') {
        reader.advance(); // consume 'E'
        reader.readLine(); // consume rest of line
        // keyword~
        const keyword = reader.readTildeString();
        // description~
        const desc = reader.readTildeString();
        extraDescs.push({ keyword, description: desc });
      } else {
        // Unknown line — skip it
        reader.readLine();
      }
    }

    rooms.push({
      vnum,
      name,
      tilde1,
      tilde2,
      description,
      areaNumber,
      roomFlags,
      sectorType,
      roomData,
      exits,
      extraDescs,
    });
  }

  return rooms;
}

function parseResets(reader: AreaReader): ParsedReset[] {
  const resets: ParsedReset[] = [];

  while (!reader.eof) {
    const line = reader.readLine();
    if (!line) continue;

    // 'S' alone ends the section
    if (line === 'S') break;

    // Lines starting with * are comments — skip
    if (line.startsWith('*')) continue;

    const parts = line.split(/\s+/);
    if (parts.length < 2) continue;

    const command = parts[0];

    // Different reset commands have different numbers of args:
    // M mob_vnum max_count room_vnum  (4 args + optional comment)
    // O obj_vnum 0 room_vnum          (4 args)
    // P obj_vnum 0 container_vnum     (4 args)
    // G obj_vnum 0                    (3 args)
    // E obj_vnum 0 wear_location      (4 args)
    // D room_vnum direction state     (4 args)
    // R room_vnum direction           (3 args)
    const arg1 = parseInt(parts[1], 10) || 0;
    const arg2 = parseInt(parts[2], 10) || 0;
    const arg3 = parseInt(parts[3], 10) || 0;
    const arg4 = parts.length > 4 ? (parseInt(parts[4], 10) || 0) : 0;

    // Anything after the numeric args is a comment
    let comment = '';
    // Find where the comment starts (after the last number)
    const numArgs = command === 'G' || command === 'R' ? 3 : command === 'S' ? 0 : 4;
    if (parts.length > numArgs + 1) {
      comment = parts.slice(numArgs + 1).join(' ');
    }

    resets.push({ command, arg1, arg2, arg3, arg4, comment });
  }

  return resets;
}

function parseShops(reader: AreaReader): ParsedShop[] {
  const shops: ParsedShop[] = [];

  while (!reader.eof) {
    const line = reader.readLine();
    if (!line) continue;

    // '0' alone ends the section
    if (line.trim() === '0') break;

    const parts = line.split(/\s+/);
    if (parts.length < 10) {
      console.warn(`area-parser: short shop line: "${line}"`);
      continue;
    }

    shops.push({
      keeper: parseInt(parts[0], 10) || 0,
      buyType1: parseInt(parts[1], 10) || 0,
      buyType2: parseInt(parts[2], 10) || 0,
      buyType3: parseInt(parts[3], 10) || 0,
      buyType4: parseInt(parts[4], 10) || 0,
      buyType5: parseInt(parts[5], 10) || 0,
      profitBuy: parseInt(parts[6], 10) || 0,
      profitSell: parseInt(parts[7], 10) || 0,
      openHour: parseInt(parts[8], 10) || 0,
      closeHour: parseInt(parts[9], 10) || 0,
    });
  }

  return shops;
}

function parseSpecials(reader: AreaReader): ParsedSpecial[] {
  const specials: ParsedSpecial[] = [];

  while (!reader.eof) {
    const line = reader.readLine();
    if (!line) continue;

    if (line === 'S') break;

    // Format: M vnum spec_fun_name
    const parts = line.split(/\s+/);
    if (parts.length < 3 || parts[0] !== 'M') {
      console.warn(`area-parser: unexpected specials line: "${line}"`);
      continue;
    }

    specials.push({
      mobVnum: parseInt(parts[1], 10) || 0,
      specFun: parts[2],
    });
  }

  return specials;
}

function parseGames(reader: AreaReader): ParsedGame[] {
  const games: ParsedGame[] = [];

  while (!reader.eof) {
    const line = reader.readLine();
    if (!line) continue;

    if (line === 'S') break;

    // Format: M vnum game_fun~ args...
    const parts = line.split(/\s+/);
    if (parts.length < 3 || parts[0] !== 'M') {
      console.warn(`area-parser: unexpected games line: "${line}"`);
      continue;
    }

    const mobVnum = parseInt(parts[1], 10) || 0;
    // game_fun is terminated by ~ and may contain the rest of the args on the same line
    const afterVnum = line.slice(line.indexOf(parts[2]));
    const tildeIdx = afterVnum.indexOf('~');
    const gameFun = tildeIdx >= 0 ? afterVnum.slice(0, tildeIdx).trim() : parts[2];
    const args = tildeIdx >= 0 ? afterVnum.slice(tildeIdx + 1).trim() : parts.slice(3).join(' ');

    games.push({ mobVnum, gameFun, args });
  }

  return games;
}

/**
 * Skip the #HELPS section.  Each help entry is: level keywords~\ndescription\n~
 * The section ends with an entry whose keywords contain "$".
 */
function skipHelps(reader: AreaReader): void {
  while (!reader.eof) {
    // Read the header line (level keywords~)
    const headerLine = reader.readTildeString();
    // If keywords contain '$', this is the end-of-section marker
    if (headerLine.includes('$')) break;
    // Read the description (terminated by ~)
    reader.readTildeString();
  }
}

/**
 * Skip an unrecognised section by reading until the next section header
 * (line starting with #) or end of file.
 */
function skipSection(reader: AreaReader): void {
  while (!reader.eof) {
    reader.skipWhitespace();
    if (reader.peek() === '#') return; // next section header
    reader.readLine();
  }
}

// ---------------------------------------------------------------------------
// Main parser
// ---------------------------------------------------------------------------

/**
 * Parse a single Stormgate .are file and return structured data.
 */
export function parseAreaFile(content: string): ParsedArea {
  const reader = new AreaReader(content);
  const area: ParsedArea = {
    metadata: {
      name: '',
      builders: '',
      vnumLow: 0,
      vnumHigh: 0,
      security: 0,
      recall: 0,
      flags: 0,
      version: 0,
      creator: '',
      lowerLevel: 0,
      upperLevel: 0,
    },
    mobiles: [],
    objects: [],
    rooms: [],
    resets: [],
    shops: [],
    specials: [],
    games: [],
  };

  while (!reader.eof) {
    reader.skipWhitespace();
    if (reader.eof) break;

    // Expect a section header starting with #
    if (reader.peek() !== '#') {
      reader.readLine(); // skip junk line
      continue;
    }

    const header = reader.readLine().trim();

    switch (header) {
      case '#AREADATA':
        area.metadata = parseAreaData(reader);
        break;
      case '#MOBILES':
        area.mobiles = parseMobiles(reader);
        break;
      case '#OBJECTS':
        area.objects = parseObjects(reader);
        break;
      case '#ROOMDATA':
        area.rooms = parseRooms(reader);
        break;
      case '#RESETS':
        area.resets = parseResets(reader);
        break;
      case '#SHOPS':
        area.shops = parseShops(reader);
        break;
      case '#SPECIALS':
        area.specials = parseSpecials(reader);
        break;
      case '#GAMES':
        area.games = parseGames(reader);
        break;
      case '#HELPS':
        skipHelps(reader);
        break;
      case '#$':
        return area; // end of file marker
      default:
        // Unknown section — try to skip it
        console.warn(`area-parser: unknown section "${header}", skipping`);
        skipSection(reader);
        break;
    }
  }

  return area;
}

// ---------------------------------------------------------------------------
// List file parser
// ---------------------------------------------------------------------------

/**
 * Parse an area list file (areaTS.lst) into an array of area file names.
 * The list file contains one filename per line, terminated by a line
 * containing just "$".
 */
export function parseAreaList(listContent: string): string[] {
  const files: string[] = [];
  const lines = listContent.split(/\r?\n/);
  for (const line of lines) {
    const trimmed = line.trim();
    if (!trimmed) continue;
    if (trimmed === '$') break;
    files.push(trimmed);
  }
  return files;
}

// ---------------------------------------------------------------------------
// Batch loader
// ---------------------------------------------------------------------------

/**
 * Read the areaTS.lst file from `areaDir`, then parse every area file listed
 * in it.  Returns an array of ParsedArea objects.
 *
 * Uses synchronous file I/O (the area files are small), but is declared
 * async for interface compatibility.
 */
export async function loadAllAreas(areaDir: string): Promise<ParsedArea[]> {
  const listPath = join(areaDir, 'areaTS.lst');
  let listContent: string;
  try {
    listContent = readFileSync(listPath, 'utf-8');
  } catch (err) {
    console.warn(`area-parser: could not read list file "${listPath}":`, err);
    return [];
  }

  const filenames = parseAreaList(listContent);
  const areas: ParsedArea[] = [];

  for (const filename of filenames) {
    const filePath = join(areaDir, filename);
    try {
      const content = readFileSync(filePath, 'utf-8');
      const parsed = parseAreaFile(content);
      areas.push(parsed);
    } catch (err) {
      console.warn(`area-parser: could not parse "${filePath}":`, err);
    }
  }

  return areas;
}
