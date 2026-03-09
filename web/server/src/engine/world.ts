/**
 * world.ts — Central in-memory game state for Stormgate MUD.
 *
 * Mirrors the global variables from the C server (char_list, object_list,
 * room_index_hash, mob_index_hash, obj_index_hash, area_first, etc.).
 *
 * Provides lookup helpers and factory methods for instantiating mobs/objects
 * from their index templates.
 */

import type {
  RoomIndex,
  MobIndex,
  ObjIndex,
  CharData,
  ObjInstance,
  AreaData,
  ClanData,
  ReligionData,
  TimeInfo,
  WeatherData,
  NoteData,
  ShopData,
  PcData,
} from './types.js';

import {
  Position,
  WearLocation,
  ItemType,
} from './types.js';

// ============================================================================
//  ID generation
// ============================================================================

let idCounter = 0;

function generateId(): string {
  idCounter += 1;
  const timestamp = Date.now().toString(36);
  const counter = idCounter.toString(36).padStart(6, '0');
  const random = Math.random().toString(36).substring(2, 8);
  return `${timestamp}-${counter}-${random}`;
}

// ============================================================================
//  Sky / Sunlight constants for weather
// ============================================================================

export const SKY_CLOUDLESS = 0;
export const SKY_CLOUDY = 1;
export const SKY_RAINING = 2;
export const SKY_LIGHTNING = 3;

export const SUN_DARK = 0;
export const SUN_RISE = 1;
export const SUN_LIGHT = 2;
export const SUN_SET = 3;

// ============================================================================
//  World class
// ============================================================================

export class World {
  rooms: Map<number, RoomIndex> = new Map();
  mobTemplates: Map<number, MobIndex> = new Map();
  objTemplates: Map<number, ObjIndex> = new Map();
  characters: Map<string, CharData> = new Map();
  objects: Map<string, ObjInstance> = new Map();
  areas: Map<string, AreaData> = new Map();
  clans: ClanData[] = [];
  religions: ReligionData[] = [];
  notes: NoteData[] = [];
  shops: Map<number, ShopData> = new Map();

  /** PC-only data keyed by character id. NPCs do not have PcData. */
  pcData: Map<string, PcData> = new Map();

  time: TimeInfo = {
    hour: 0,
    day: 0,
    month: 0,
    year: 500,
    total: 0,
  };

  weather: WeatherData = {
    mmhg: 960,
    change: 0,
    sky: SKY_CLOUDLESS,
    sunlight: SUN_LIGHT,
  };

  // --------------------------------------------------------------------------
  //  Lookup helpers
  // --------------------------------------------------------------------------

  getRoom(vnum: number): RoomIndex | undefined {
    return this.rooms.get(vnum);
  }

  getMobTemplate(vnum: number): MobIndex | undefined {
    return this.mobTemplates.get(vnum);
  }

  getObjTemplate(vnum: number): ObjIndex | undefined {
    return this.objTemplates.get(vnum);
  }

  getCharById(id: string): CharData | undefined {
    return this.characters.get(id);
  }

  /**
   * Return every character whose roomVnum matches the given vnum.
   * Characters store their current room as the `roomVnum` runtime field
   * (added dynamically, not on the interface — we use a side map).
   */
  getCharsInRoom(vnum: number): CharData[] {
    const result: CharData[] = [];
    for (const ch of this.characters.values()) {
      if (charRoomMap.get(ch.id) === vnum && !ch.deleted) {
        result.push(ch);
      }
    }
    return result;
  }

  getObjsInRoom(vnum: number): ObjInstance[] {
    const result: ObjInstance[] = [];
    for (const obj of this.objects.values()) {
      if (obj.inRoom === vnum && !obj.deleted) {
        result.push(obj);
      }
    }
    return result;
  }

  // --------------------------------------------------------------------------
  //  Factory methods
  // --------------------------------------------------------------------------

  /**
   * Create a live mob instance from a template vnum.
   * Returns null if the template is not found.
   */
  createMobInstance(vnum: number): CharData | null {
    const template = this.mobTemplates.get(vnum);
    if (!template) return null;

    const id = this.nextId();

    // Roll hit points from dice
    const hp = rollDice(template.hitnodice, template.hitsizedice) + template.hitplus;

    const mob: CharData = {
      id,
      name: template.name,
      shortDescr: template.shortDescr,
      longDescr: template.longDescr,
      description: template.description,
      prompt: '',
      sex: template.sex,
      charClass: template.charClass,
      race: 0,
      level: template.level,
      trust: 0,
      exp: 0,
      gold: template.gold,

      hit: hp,
      maxHit: hp,
      mana: 100,
      maxMana: 100,
      move: 100,
      maxMove: 100,
      bp: 0,
      maxBp: 0,

      position: Position.STANDING,
      practice: 0,
      alignment: template.alignment,
      hitroll: template.hitroll,
      damroll: template.damroll,
      armor: template.ac,
      wimpy: 0,
      savingThrow: 0,

      affectedBy: template.affectedBy,
      affectedBy2: template.affectedBy2,
      affectedBy3: template.affectedBy3,
      affectedBy4: template.affectedBy4,
      act: template.act,
      act2: template.act2,

      clan: 0,
      religion: 0,
      clev: 0,
      language: [0],
      speaking: 0,
      size: template.size,
      pkill: 0,
      shields: template.shields,

      questpoints: 0,
      nextquest: 0,
      countdown: 0,
      questobj: 0,
      questmob: 0,
      rquestpoints: 0,

      isQuestor: false,
      questGiver: '',
      questArea: '',
      questRoom: '',
      questFetchItem: '',
      vnum: template.vnum,
      learn: 0,

      combatTimer: 0,
      summonTimer: 0,
      poisonLevel: 0,

      damageMods: new Array(21).fill(0),
      mounted: 0,

      affects: [],
      affects2: [],
      affects3: [],
      affects4: [],

      carryWeight: 0,
      carryNumber: 0,
      deleted: false,
      isNpc: true,
      fighting: null,
    };

    this.characters.set(id, mob);
    mobTemplateVnumMap.set(id, vnum);
    return mob;
  }

  /**
   * Create a live object instance from a template vnum.
   * Returns null if the template is not found.
   */
  createObjInstance(vnum: number): ObjInstance | null {
    const template = this.objTemplates.get(vnum);
    if (!template) return null;

    const id = this.nextId();

    const obj: ObjInstance = {
      id,
      indexVnum: template.vnum,
      name: template.name,
      shortDescr: template.shortDescr,
      description: template.description,
      itemType: template.itemType,
      extraFlags: template.extraFlags,
      extraFlags2: template.extraFlags2,
      extraFlags3: template.extraFlags3,
      extraFlags4: template.extraFlags4,
      wearFlags: template.wearFlags,
      wearLoc: WearLocation.NONE,
      durabilityMax: 100,
      durabilityCur: 100,
      weight: template.weight,
      cost: template.cost,
      level: template.level,
      timer: 0,
      value: [...template.value] as [number, number, number, number],
      affects: template.affects.map((a) => ({ ...a })),
      acType: 0,
      acVnum: 0,
      acSpell: '',
      acCharge: [0, 0],
      deleted: false,
    };

    this.objects.set(id, obj);
    return obj;
  }

  // --------------------------------------------------------------------------
  //  ID generation
  // --------------------------------------------------------------------------

  nextId(): string {
    return generateId();
  }

  // --------------------------------------------------------------------------
  //  Object helpers for crafting / quest systems
  // --------------------------------------------------------------------------

  /**
   * Create a simple object instance without requiring a template vnum.
   * Used by crafting, quest rewards, and similar dynamic object creation.
   */
  createSimpleObject(opts: {
    name: string;
    shortDescr: string;
    description: string;
    level: number;
    itemType: ItemType;
    weight: number;
    cost: number;
    wearFlags: number;
    extraFlags: number;
  }): ObjInstance {
    const id = this.nextId();
    const obj: ObjInstance = {
      id,
      indexVnum: 0,
      name: opts.name,
      shortDescr: opts.shortDescr,
      description: opts.description,
      itemType: opts.itemType,
      extraFlags: opts.extraFlags,
      extraFlags2: 0,
      extraFlags3: 0,
      extraFlags4: 0,
      wearFlags: opts.wearFlags,
      wearLoc: WearLocation.NONE,
      durabilityMax: 100,
      durabilityCur: 100,
      weight: opts.weight,
      cost: opts.cost,
      level: opts.level,
      timer: 0,
      value: [0, 0, 0, 0],
      affects: [],
      acType: 0,
      acVnum: 0,
      acSpell: '',
      acCharge: [0, 0],
      deleted: false,
    };
    this.objects.set(id, obj);
    return obj;
  }

  /**
   * Remove an object from the game world entirely.
   * Clears all ownership references and marks it deleted.
   */
  extractObj(obj: ObjInstance): void {
    obj.deleted = true;
    obj.carriedBy = undefined;
    obj.inRoom = undefined;
    obj.containedIn = undefined;
    obj.storedBy = undefined;
    this.objects.delete(obj.id);
  }
}

// ============================================================================
//  Character-room mapping (side map)
// ============================================================================

/**
 * Maps character id -> room vnum. This avoids extending the CharData interface
 * with a runtime-only field. handler.ts charToRoom/charFromRoom maintain this.
 */
export const charRoomMap = new Map<string, number>();

/**
 * Maps NPC character id -> mob template vnum. Set in createMobInstance so
 * the reset system can efficiently look up template vnums for mob counting.
 */
export const mobTemplateVnumMap = new Map<string, number>();

// ============================================================================
//  Singleton instance
// ============================================================================

export const world = new World();

// ============================================================================
//  Utility
// ============================================================================

export function rollDice(count: number, sides: number): number {
  if (count <= 0 || sides <= 0) return 0;
  let total = 0;
  for (let i = 0; i < count; i++) {
    total += Math.floor(Math.random() * sides) + 1;
  }
  return total;
}
