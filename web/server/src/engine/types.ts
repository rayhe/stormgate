/**
 * types.ts — TypeScript port of Stormgate MUD data structures
 *
 * Mirrors the C structs, #defines, and enums from merc.h (Diku/Merc/Envy lineage).
 * Enums map constant groups; interfaces map structs.
 */

// ============================================================================
//  GAME CONSTANTS
// ============================================================================

export const MAX_LEVEL = 116;
export const MAX_SKILL = 485;
export const MAX_CLASS = 21;
export const MAX_RACE = 27;
export const MAX_CLAN = 21;
export const MAX_RELIGION = 8;
export const MAX_LANGUAGE = 27;
export const MAX_WEAR = 30;
export const MAX_GROUP = 8;
export const MAX_ATTACKS = 8;

export const LEVEL_IMMORTAL = 105;
export const LEVEL_HERO = 100;

/** Pulses — heartbeat timing (PULSE_PER_SECOND ticks per real second) */
export const PULSE_PER_SECOND = 4;
export const PULSE_VIOLENCE = 12;   // 3 seconds
export const PULSE_MOBILE = 16;     // 4 seconds
export const PULSE_TICK = 120;      // 30 seconds
export const PULSE_AREA = 240;      // 60 seconds

// ============================================================================
//  ENUMS — Direction, Sex, Position, Sector
// ============================================================================

export enum Direction {
  NORTH = 0,
  EAST = 1,
  SOUTH = 2,
  WEST = 3,
  UP = 4,
  DOWN = 5,
}

export enum Sex {
  NEUTRAL = 0,
  MALE = 1,
  FEMALE = 2,
}

/**
 * Character position.
 * Note the gap: 9 is unused in the original C source; 10 = MEDITATING.
 */
export enum Position {
  DEAD = 0,
  MORTAL = 1,
  INCAP = 2,
  STUNNED = 3,
  SLEEPING = 4,
  RESTING = 5,
  GHOST = 6,
  FIGHTING = 7,
  STANDING = 8,
  // 9 is intentionally skipped
  MEDITATING = 10,
}

export enum SectorType {
  INSIDE = 0,
  CITY = 1,
  FIELD = 2,
  FOREST = 3,
  HILLS = 4,
  MOUNTAIN = 5,
  WATER_SWIM = 6,
  WATER_NOSWIM = 7,
  UNDERWATER = 8,
  AIR = 9,
  DESERT = 10,
  BADLAND = 11,
}

// ============================================================================
//  ENUMS — Item / Object types
// ============================================================================

/**
 * ITEM_* defines from merc.h.
 * Numbers match the area-file type field for objects.
 */
export enum ItemType {
  LIGHT = 1,
  SCROLL = 2,
  WAND = 3,
  STAFF = 4,
  WEAPON = 5,
  // 6–7 unused
  TREASURE = 8,
  ARMOR = 9,
  POTION = 10,
  NOTEBOARD = 11,
  FURNITURE = 12,
  TRASH = 13,
  // 14 unused
  CONTAINER = 15,
  // 16 unused
  DRINK_CON = 17,
  KEY = 18,
  FOOD = 19,
  MONEY = 20,
  // 21 unused
  BOAT = 22,
  CORPSE_NPC = 23,
  CORPSE_PC = 24,
  FOUNTAIN = 25,
  PILL = 26,
  LENSE = 27,
  BLOOD = 28,
  PORTAL = 29,
  VODOO = 30,
  BERRY = 31,
  GUN = 32,
  RUNE = 33,
  WRECK = 34,
  IMPLANTED = 35,
  SKIN = 36,
  ARROW = 37,
  BOLT = 38,
  BULLET = 39,
  BOOK = 40,
  NEEDLE = 41,
  QUILL = 42,
  HAMMER = 43,
  PESTLE = 44,
  TIMBER = 45,
  ORE = 46,
}

// ============================================================================
//  ENUMS — Wear, Damage, Apply, Weapon
// ============================================================================

/**
 * WEAR_* — equipment slot locations.
 * NONE (-1) means the object is not worn.
 */
export enum WearLocation {
  NONE = -1,
  LIGHT = 0,
  FINGER_L = 1,
  FINGER_R = 2,
  NECK_1 = 3,
  NECK_2 = 4,
  BODY = 5,
  HEAD = 6,
  IN_EYES = 7,
  ON_FACE = 8,
  ORBIT = 9,
  ORBIT_2 = 10,
  LEGS = 11,
  FEET = 12,
  HANDS = 13,
  ARMS = 14,
  SHIELD = 15,
  ABOUT = 16,
  WAIST = 17,
  WRIST_L = 18,
  WRIST_R = 19,
  WIELD = 20,
  WIELD_2 = 21,
  HOLD = 22,
  FIREARM = 23,
  EARS = 24,
  ANKLE_L = 25,
  ANKLE_R = 26,
  IMPLANTED1 = 27,
  IMPLANTED2 = 28,
  IMPLANTED3 = 29,
}

/** DAM_* — damage types for weapons and spells. */
export enum DamType {
  NONE = 0,
  BASH = 1,
  PIERCE = 2,
  SLASH = 3,
  FIRE = 4,
  COLD = 5,
  LIGHTNING = 6,
  ACID = 7,
  POISON = 8,
  NEGATIVE = 9,
  HOLY = 10,
  ENERGY = 11,
  MENTAL = 12,
  DISEASE = 13,
  DROWNING = 14,
  LIGHT = 15,
  OTHER = 16,
  HARM = 17,
  CHARM = 18,
  SOUND = 19,
}

/** APPLY_* — what stat an affect modifies. */
export enum ApplyType {
  NONE = 0,
  STR = 1,
  DEX = 2,
  INT = 3,
  WIS = 4,
  CON = 5,
  SEX = 6,
  CLASS = 7,
  LEVEL = 8,
  AGE = 9,
  HEIGHT = 10,
  WEIGHT = 11,
  MANA = 12,
  HIT = 13,
  MOVE = 14,
  GOLD = 15,
  EXP = 16,
  AC = 17,
  HITROLL = 18,
  DAMROLL = 19,
  SAVING_PARA = 20,
  SAVING_ROD = 21,
  SAVING_PETRI = 22,
  SAVING_BREATH = 23,
  SAVING_SPELL = 24,
  BP = 25,
  ANTI_DIS = 26,
  RACE = 27,
}

export enum WeaponClass {
  EXOTIC = 0,
  SWORD = 1,
  DAGGER = 2,
  SPEAR = 3,
  MACE = 4,
  AXE = 5,
  FLAIL = 6,
  WHIP = 7,
  POLEARM = 8,
}

// ============================================================================
//  CONST OBJECTS — CharClass, Language
//  (Using `as const` objects rather than enums so values are usable as
//   array indices while keeping full type safety.)
// ============================================================================

export const CharClass = {
  MAGE: 0,
  CLERIC: 1,
  THIEF: 2,
  WARRIOR: 3,
  PSIONICIST: 4,
  DRUID: 5,
  RANGER: 6,
  PALADIN: 7,
  BARD: 8,
  VAMPIRE: 9,
  WEREWOLF: 10,
  ANTI_PALADIN: 11,
  ASSASSIN: 12,
  MONK: 13,
  BARBARIAN: 14,
  ILLUSIONIST: 15,
  NECROMANCER: 16,
  DEMONOLOGIST: 17,
  SHAMAN: 18,
  DARKPRIEST: 19,
} as const;

export type CharClassId = (typeof CharClass)[keyof typeof CharClass];

export const Language = {
  COMMON: 0,
  HUMAN: 1,
  DWARVISH: 2,
  ELVISH: 3,
  GNOMISH: 4,
  DRAGON: 5,
  DEMON: 6,
  OGRE: 7,
  DROW: 8,
  ELDER: 9,
  PIXIE: 10,
  HOBBIT: 11,
  MINOTAUR: 12,
  LIZARD: 13,
  HALFLING: 14,
  FELINE: 15,
  CANINE: 16,
  ANGEL: 17,
  ORCISH: 18,
  MAGICK: 19,
  SHADOW_SPEAK: 20,
  SPIRITSPEAK: 21,
  ENLIGHTENED: 22,
  SATANIC: 23,
  ANIMALSPEAK: 24,
  BRETONNIAN: 25,
  GARGISH: 26,
} as const;

export type LanguageId = (typeof Language)[keyof typeof Language];

// ============================================================================
//  INTERFACES — Affect, Exit, Extra-description, Reset
// ============================================================================

/**
 * affect_data — a spell/skill effect currently on a character or object.
 * `type` is the sn (skill number) of the originating spell/skill.
 */
export interface AffectData {
  type: number;
  level: number;
  duration: number;
  location: ApplyType;
  modifier: number;
  bitvector: number;
  deleted: boolean;
}

/**
 * exit_data — one directional link from a room.
 * `rsFlags` stores the original flags for OLC reset purposes.
 */
export interface ExitData {
  toVnum: number;
  exitInfo: number;
  key: number;
  keyword: string;
  description: string;
  rsFlags: number;
}

export interface ExtraDescrData {
  keyword: string;
  description: string;
}

/**
 * reset_data — area reset command.
 * `command` is a single character: 'M' mob, 'O' obj, 'P' put, 'G' give,
 * 'E' equip, 'D' door, 'R' randomize, 'S' stop.
 */
export interface ResetData {
  command: string;
  arg1: number;
  arg2: number;
  arg3: number;
  status: number;
}

// ============================================================================
//  INTERFACES — Room, Area
// ============================================================================

export interface RoomIndex {
  vnum: number;
  name: string;
  description: string;
  sectorType: SectorType;
  roomFlags: number;
  light: number;
  /** Keyed by Direction enum value. Not every direction need be present. */
  exits: Partial<Record<Direction, ExitData>>;
  extraDescriptions: ExtraDescrData[];
  /** Timed room flags (e.g., temporary darkness) */
  timedRoomFlags: number;
  flagTimer: number;
  soundfile?: string;
  musicfile?: string;
  /** Key into the areas map — identifies which area owns this room. */
  areaKey: string;
}

export interface AreaData {
  name: string;
  filename: string;
  /** Lower vnum bound */
  lvnum: number;
  /** Upper vnum bound */
  uvnum: number;
  /** Area's own vnum (used for area-level references) */
  vnum: number;
  security: number;
  builders: string;
  recall: number;
  areaFlags: number;
  version: number;
  creator: string;
  /** Suggested lower level */
  llevel: number;
  /** Suggested upper level */
  ulevel: number;
  windstr?: number;
  winddir?: number;
}

// ============================================================================
//  INTERFACES — Object (template + instance)
// ============================================================================

/**
 * obj_index_data — the prototype loaded from area files.
 * Live objects clone from this template.
 */
export interface ObjIndex {
  vnum: number;
  name: string;
  shortDescr: string;
  description: string;
  itemType: ItemType;
  extraFlags: number;
  extraFlags2: number;
  extraFlags3: number;
  extraFlags4: number;
  wearFlags: number;
  /** Four generic value slots; interpretation depends on itemType. */
  value: [number, number, number, number];
  weight: number;
  cost: number;
  level: number;
  affects: AffectData[];
  extraDescriptions: ExtraDescrData[];
}

/**
 * obj_data — a live object instance in the game world.
 * `id` is a unique runtime identifier (UUID or similar).
 */
export interface ObjInstance {
  id: string;
  /** Vnum of the ObjIndex this was cloned from. */
  indexVnum: number;
  name: string;
  shortDescr: string;
  description: string;
  itemType: ItemType;
  extraFlags: number;
  extraFlags2: number;
  extraFlags3: number;
  extraFlags4: number;
  wearFlags: number;
  wearLoc: WearLocation;
  durabilityMax: number;
  durabilityCur: number;
  weight: number;
  cost: number;
  level: number;
  timer: number;
  value: [number, number, number, number];
  affects: AffectData[];
  /** Arrow/charge fields: ac_type, ac_vnum, ac_spell, ac_charge[2] */
  acType: number;
  acVnum: number;
  acSpell: string;
  acCharge: [number, number];
  deleted: boolean;
  /** Vnum or id of containing object, if inside a container. */
  containedIn?: string;
  /** Id of the character carrying this object. */
  carriedBy?: string;
  /** Id of the shopkeeper storing this object. */
  storedBy?: string;
  /** Vnum of the room this object is sitting in. */
  inRoom?: number;
}

// ============================================================================
//  INTERFACES — Mob (template) and Shop
// ============================================================================

/**
 * mob_index_data — NPC prototype loaded from area files.
 */
export interface MobIndex {
  vnum: number;
  name: string;
  shortDescr: string;
  longDescr: string;
  description: string;
  level: number;
  sex: Sex;
  charClass: number;
  /** ACT_* bitvector flags */
  act: number;
  act2: number;
  /** AFF_* bitvector flags */
  affectedBy: number;
  affectedBy2: number;
  affectedBy3: number;
  affectedBy4: number;
  /** Immunity flags */
  immFlags: number;
  /** Resistance flags */
  resFlags: number;
  /** Vulnerability flags */
  vulFlags: number;
  alignment: number;
  hitroll: number;
  damroll: number;
  /** Armor class */
  ac: number;
  /** Hit dice: hitnodice d hitsizedice + hitplus */
  hitnodice: number;
  hitsizedice: number;
  hitplus: number;
  /** Damage dice: damnodice d damsizedice + damplus */
  damnodice: number;
  damsizedice: number;
  damplus: number;
  gold: number;
  size: number;
  shields: number;
  speaking: number;
  /** Mob programs (optional; not all mobs have progs). */
  mobprogs?: MobProgData[];
}

/** Mob program trigger reference — kept minimal for the type system. */
export interface MobProgData {
  type: number;
  argList: string;
  comList: string;
}

export interface ShopData {
  keeperVnum: number;
  /** Up to 5 item types the shop will buy. */
  buyType: [number, number, number, number, number];
  profitBuy: number;
  profitSell: number;
  openHour: number;
  closeHour: number;
}

// ============================================================================
//  INTERFACES — Character (live PC/NPC) and PC-only data
// ============================================================================

/**
 * char_data — a live character (player or NPC) in the game world.
 *
 * `isNpc` discriminates between player and NPC at runtime.
 * NPC-only or PC-only fields are on the mob index or PcData respectively.
 */
export interface CharData {
  id: string;
  name: string;
  shortDescr: string;
  longDescr: string;
  description: string;
  prompt: string;
  sex: Sex;
  charClass: number;
  race: number;
  level: number;
  trust: number;
  exp: number;
  gold: number;

  // Vitals
  hit: number;
  maxHit: number;
  mana: number;
  maxMana: number;
  move: number;
  maxMove: number;
  /** Blood points (vampire resource) */
  bp: number;
  maxBp: number;

  position: Position;
  practice: number;
  alignment: number;
  hitroll: number;
  damroll: number;
  armor: number;
  wimpy: number;
  savingThrow: number;

  // Bitvector flag groups
  affectedBy: number;
  affectedBy2: number;
  affectedBy3: number;
  affectedBy4: number;
  act: number;
  act2: number;

  // Social / faction
  clan: number;
  religion: number;
  /** Clan level / rank */
  clev: number;
  /** Languages known — indexed by Language const values */
  language: number[];
  /** Currently speaking language id */
  speaking: number;
  size: number;
  /** Player-kill flag / count */
  pkill: number;
  shields: number;

  // Quest system
  questpoints: number;
  nextquest: number;
  countdown: number;
  questobj: number;
  questmob: number;
  rquestpoints: number;

  // Timers
  combatTimer: number;
  summonTimer: number;
  poisonLevel: number;

  /**
   * Per-damage-type modifier array, indexed by DamType.
   * Length should be 21 (DamType.NONE through DamType.SOUND + 1).
   */
  damageMods: number[];

  /** Vnum of the mount, or 0 if not mounted. */
  mounted: number;

  // Affects — one list per bitvector group
  affects: AffectData[];
  affects2: AffectData[];
  affects3: AffectData[];
  affects4: AffectData[];

  carryWeight: number;
  carryNumber: number;
  deleted: boolean;
  isNpc: boolean;
}

/**
 * pc_data — player-character-only data, stored alongside CharData.
 * Not present for NPCs.
 */
export interface PcData {
  /** Hashed password (never sent to client). */
  pwd: string;
  title: string;
  prompt: string;
  /** Last name */
  lname: string;

  // Bamf (immortal enter/exit) messages
  bamfin: string;
  bamfout: string;
  bamfsin: string;
  bamfsout: string;

  whoText: string;
  spouse: string;
  recall: number;

  // Permanent stats
  permStr: number;
  permInt: number;
  permWis: number;
  permDex: number;
  permCon: number;

  // Temporary stat modifiers (from equipment, spells, etc.)
  modStr: number;
  modInt: number;
  modWis: number;
  modDex: number;
  modCon: number;

  /**
   * Condition array: [drunk, full, thirst].
   * Values 0–48; negative means "not applicable" (immortals).
   */
  condition: [number, number, number];

  /**
   * Skill/spell proficiency.
   * Key = skill number (sn), Value = learned percentage (0–100+).
   */
  learned: Map<number, number>;

  /** OLC security level. */
  security: number;
  bankAccount: number;
  pagelen: number;
  /** Shares in the in-game stock system. */
  shares: number;
  mobkills: number;
  corpses: number;
  plan: string;
  email: string;

  // Arena stats
  awins: number;
  alosses: number;

  /** Command aliases: shortcut -> expansion */
  aliases: Map<string, string>;

  /** Spell slots available per spell circle (circle 1–3). */
  spellSlots: {
    spell1: number;
    spell2: number;
    spell3: number;
  };

  craftTimer: number;
  craftType: number;
}

// ============================================================================
//  INTERFACES — Clan, Religion
// ============================================================================

export interface ClanData {
  name: string;
  deity: string;
  description: string;
  leader: string;
  first: string;
  second: string;
  champ: string;
  /** Clan hall vnum */
  vnum: number;
  recall: number;
  pkills: number;
  mkills: number;
  members: number;
  pdeaths: number;
  mdeaths: number;
  /** Three clan-specific object vnums (e.g., pit, badge, banner). */
  objVnum1: number;
  objVnum2: number;
  objVnum3: number;
  /** Whether this clan participates in PK. */
  pkill: boolean;
}

export interface ReligionData {
  name: string;
  deity: string;
  description: string;
  shortdesc: string;
  /** Religion hall vnum */
  vnum: number;
  recall: number;
  /** Starting room vnum for new followers. */
  start: number;
  pkills: number;
  mkills: number;
  pdeaths: number;
  mdeaths: number;
  members: number;
}

// ============================================================================
//  INTERFACES — Time, Weather, World State
// ============================================================================

export interface TimeInfo {
  hour: number;
  day: number;
  month: number;
  year: number;
  /** Total elapsed ticks since epoch (used for duration math). */
  total: number;
}

export interface WeatherData {
  /** Barometric pressure */
  mmhg: number;
  /** Pressure change direction */
  change: number;
  /** Sky condition (clear, cloudy, raining, lightning) */
  sky: number;
  /** Sun position (dark, rise, light, set) */
  sunlight: number;
}

export interface WorldState {
  time: TimeInfo;
  weather: WeatherData;
}

// ============================================================================
//  INTERFACES — Notes (bulletin boards)
// ============================================================================

export interface NoteData {
  sender: string;
  senderUid?: string;
  date: string;
  toList: string;
  subject: string;
  text: string;
  dateStamp: number;
  protected: boolean;
  /** Board vnum or identifier this note is posted on. */
  onBoard: number;
}

// ============================================================================
//  WEB / AUTH TYPES
// ============================================================================

export type UserRole = "player" | "builder" | "immortal" | "admin";

/** Firestore user document — ties a Firebase Auth user to in-game characters. */
export interface UserData {
  uid: string;
  email: string;
  displayName: string;
  role: UserRole;
  createdAt: Date;
  lastLogin: Date;
  /** Array of CharData ids owned by this user. */
  characterIds: string[];
}
