/**
 * save.ts — Firestore persistence for player characters.
 *
 * Provides save/load/list/auto-save functionality for PC data.
 * The engine does not import firebase-admin directly — the Firestore
 * reference is injected at startup via initSaveSystem().
 */

import type {
  CharData,
  PcData,
  AffectData,
} from './types.js';

import {
  Position,
  WearLocation,
} from './types.js';

import { world } from './world.js';
import { getCharRoom, getEquippedItems, getCarriedItems, charToRoom } from './handler.js';
import { hasConnection } from './output.js';

// ============================================================================
//  Firestore reference (injected)
// ============================================================================

let db: FirebaseFirestore.Firestore;

/**
 * Inject the Firestore reference. Must be called once at server startup
 * before any save/load operations.
 */
export function initSaveSystem(firestore: FirebaseFirestore.Firestore): void {
  db = firestore;
}

// ============================================================================
//  Save character
// ============================================================================

/**
 * Persist a player character to Firestore.
 *
 * Saves the full CharData, PcData, equipment, inventory, and affects
 * to the `characters/{charId}` document.
 */
export async function saveCharacter(ch: CharData, uid: string): Promise<void> {
  if (!db) {
    console.error('saveCharacter: Firestore not initialized. Call initSaveSystem() first.');
    return;
  }

  if (ch.isNpc) return;

  const pcdata = world.pcData.get(ch.id);
  const roomVnum = getCharRoom(ch);

  // Serialize equipment: array of { vnum, wearLoc }
  const equipped = getEquippedItems(ch);
  const equipmentData = equipped.map((obj) => ({
    vnum: obj.indexVnum,
    wearLoc: obj.wearLoc,
  }));

  // Serialize inventory (non-equipped carried items): array of vnums
  const carried = getCarriedItems(ch);
  const inventoryData = carried.map((obj) => obj.indexVnum);

  // Serialize affects
  const serializeAffects = (affs: AffectData[]) =>
    affs
      .filter((a) => !a.deleted)
      .map((a) => ({
        type: a.type,
        level: a.level,
        duration: a.duration,
        location: a.location,
        modifier: a.modifier,
        bitvector: a.bitvector,
      }));

  // Serialize Maps to plain objects for Firestore
  const learnedObj: Record<string, number> = {};
  if (pcdata?.learned) {
    for (const [sn, pct] of pcdata.learned) {
      learnedObj[String(sn)] = pct;
    }
  }

  const aliasesObj: Record<string, string> = {};
  if (pcdata?.aliases) {
    for (const [key, val] of pcdata.aliases) {
      aliasesObj[key] = val;
    }
  }

  // Build the document
  const doc: Record<string, unknown> = {
    // Ownership
    ownerId: uid,
    roomVnum: roomVnum !== -1 ? roomVnum : 25000,
    lastSave: new Date(),

    // Core identity
    name: ch.name,
    shortDescr: ch.shortDescr,
    longDescr: ch.longDescr,
    description: ch.description,
    charClass: ch.charClass,
    race: ch.race,
    level: ch.level,
    trust: ch.trust,
    sex: ch.sex,

    // Vitals
    hp: ch.hit,
    maxHp: ch.maxHit,
    mana: ch.mana,
    maxMana: ch.maxMana,
    move: ch.move,
    maxMove: ch.maxMove,
    bp: ch.bp,
    maxBp: ch.maxBp,

    // Combat stats
    hitroll: ch.hitroll,
    damroll: ch.damroll,
    armor: ch.armor,
    savingThrow: ch.savingThrow,
    alignment: ch.alignment,

    // Resources
    gold: ch.gold,
    exp: ch.exp,
    practice: ch.practice,

    // Position — always save as STANDING
    position: Position.STANDING,

    // Social
    clan: ch.clan,
    religion: ch.religion,
    clev: ch.clev,
    language: ch.language,
    speaking: ch.speaking,
    size: ch.size,
    pkill: ch.pkill,
    shields: ch.shields,

    // Quest
    questpoints: ch.questpoints,
    nextquest: ch.nextquest,
    questobj: ch.questobj,
    questmob: ch.questmob,
    rquestpoints: ch.rquestpoints,

    // Bitvectors
    affectedBy: ch.affectedBy,
    affectedBy2: ch.affectedBy2,
    affectedBy3: ch.affectedBy3,
    affectedBy4: ch.affectedBy4,
    act: ch.act,
    act2: ch.act2,

    // Misc
    wimpy: ch.wimpy,
    damageMods: ch.damageMods,
    mounted: ch.mounted,

    // Equipment & inventory
    equipment: equipmentData,
    inventory: inventoryData,

    // Affects
    affects: serializeAffects(ch.affects),
    affects2: serializeAffects(ch.affects2),
    affects3: serializeAffects(ch.affects3),
    affects4: serializeAffects(ch.affects4),

    // PcData
    pcdata: pcdata
      ? {
          title: pcdata.title,
          prompt: pcdata.prompt,
          lname: pcdata.lname,
          bamfin: pcdata.bamfin,
          bamfout: pcdata.bamfout,
          bamfsin: pcdata.bamfsin,
          bamfsout: pcdata.bamfsout,
          whoText: pcdata.whoText,
          spouse: pcdata.spouse,
          recall: pcdata.recall,
          permStr: pcdata.permStr,
          permInt: pcdata.permInt,
          permWis: pcdata.permWis,
          permDex: pcdata.permDex,
          permCon: pcdata.permCon,
          modStr: pcdata.modStr,
          modInt: pcdata.modInt,
          modWis: pcdata.modWis,
          modDex: pcdata.modDex,
          modCon: pcdata.modCon,
          condition: pcdata.condition,
          learned: learnedObj,
          security: pcdata.security,
          bankAccount: pcdata.bankAccount,
          pagelen: pcdata.pagelen,
          shares: pcdata.shares,
          mobkills: pcdata.mobkills,
          corpses: pcdata.corpses,
          plan: pcdata.plan,
          email: pcdata.email,
          awins: pcdata.awins,
          alosses: pcdata.alosses,
          aliases: aliasesObj,
          spellSlots: pcdata.spellSlots ?? {
            spell1: (pcdata as any).spell1 ?? 0,
            spell2: (pcdata as any).spell2 ?? 0,
            spell3: (pcdata as any).spell3 ?? 0,
          },
          craftTimer: pcdata.craftTimer,
          craftType: pcdata.craftType,
        }
      : null,
  };

  try {
    await db.collection('characters').doc(ch.id).set(doc, { merge: true });
  } catch (err) {
    console.error(`saveCharacter: failed to save ${ch.name} (${ch.id}):`, err);
  }
}

// ============================================================================
//  Load character
// ============================================================================

/**
 * Load a player character from Firestore.
 *
 * Returns the reconstructed CharData (placed in world.characters) and
 * PcData (placed in world.pcData), or null if the document does not exist.
 */
export async function loadCharacter(
  charId: string
): Promise<{ char: CharData; pcdata: PcData } | null> {
  if (!db) {
    console.error('loadCharacter: Firestore not initialized.');
    return null;
  }

  const snap = await db.collection('characters').doc(charId).get();
  if (!snap.exists) return null;

  const d = snap.data()!;
  const pd = d.pcdata || {};

  // Deserialize Maps
  const learned = new Map<number, number>();
  if (pd.learned) {
    for (const [key, val] of Object.entries(pd.learned)) {
      learned.set(Number(key), val as number);
    }
  }

  const aliases = new Map<string, string>();
  if (pd.aliases) {
    for (const [key, val] of Object.entries(pd.aliases)) {
      aliases.set(key, val as string);
    }
  }

  // Reconstruct PcData
  const pcdata: PcData = {
    pwd: '',
    title: pd.title ?? ' the Adventurer',
    prompt: pd.prompt ?? '<%h/%Hhp %m/%Mmn %v/%Vmv> ',
    lname: pd.lname ?? '',
    bamfin: pd.bamfin ?? '',
    bamfout: pd.bamfout ?? '',
    bamfsin: pd.bamfsin ?? '',
    bamfsout: pd.bamfsout ?? '',
    whoText: pd.whoText ?? '',
    spouse: pd.spouse ?? '',
    recall: pd.recall ?? 25000,
    permStr: pd.permStr ?? 13,
    permInt: pd.permInt ?? 13,
    permWis: pd.permWis ?? 13,
    permDex: pd.permDex ?? 13,
    permCon: pd.permCon ?? 13,
    modStr: pd.modStr ?? 0,
    modInt: pd.modInt ?? 0,
    modWis: pd.modWis ?? 0,
    modDex: pd.modDex ?? 0,
    modCon: pd.modCon ?? 0,
    condition: pd.condition ?? [0, 48, 48],
    learned,
    security: pd.security ?? 0,
    bankAccount: pd.bankAccount ?? 0,
    pagelen: pd.pagelen ?? 24,
    shares: pd.shares ?? 0,
    mobkills: pd.mobkills ?? 0,
    corpses: pd.corpses ?? 0,
    plan: pd.plan ?? '',
    email: pd.email ?? '',
    awins: pd.awins ?? 0,
    alosses: pd.alosses ?? 0,
    aliases,
    spellSlots: pd.spellSlots ?? {
      spell1: pd.spell1 ?? 0,
      spell2: pd.spell2 ?? 0,
      spell3: pd.spell3 ?? 0,
    },
    craftTimer: pd.craftTimer ?? 0,
    craftType: pd.craftType ?? 0,
  };

  // Deserialize affects
  const deserializeAffects = (raw: any[] | undefined): AffectData[] =>
    (raw || []).map((a: any) => ({
      type: a.type ?? 0,
      level: a.level ?? 0,
      duration: a.duration ?? 0,
      location: a.location ?? 0,
      modifier: a.modifier ?? 0,
      bitvector: a.bitvector ?? 0,
      deleted: false,
    }));

  // Reconstruct CharData
  const ch: CharData = {
    id: charId,
    name: d.name ?? 'Unknown',
    shortDescr: d.shortDescr ?? d.name ?? 'Unknown',
    longDescr: d.longDescr ?? `${d.name ?? 'Unknown'} is standing here.`,
    description: d.description ?? '',
    prompt: pcdata.prompt,
    sex: d.sex ?? 0,
    charClass: d.charClass ?? 3,
    race: d.race ?? 0,
    level: d.level ?? 1,
    trust: d.trust ?? 0,
    exp: d.exp ?? 0,
    gold: d.gold ?? 0,

    hit: d.hp ?? d.maxHp ?? 100,
    maxHit: d.maxHp ?? 100,
    mana: d.mana ?? d.maxMana ?? 100,
    maxMana: d.maxMana ?? 100,
    move: d.move ?? d.maxMove ?? 100,
    maxMove: d.maxMove ?? 100,
    bp: d.bp ?? 0,
    maxBp: d.maxBp ?? 0,

    position: Position.STANDING,
    practice: d.practice ?? 0,
    alignment: d.alignment ?? 0,
    hitroll: d.hitroll ?? 0,
    damroll: d.damroll ?? 0,
    armor: d.armor ?? 100,
    wimpy: d.wimpy ?? 0,
    savingThrow: d.savingThrow ?? 0,

    affectedBy: d.affectedBy ?? 0,
    affectedBy2: d.affectedBy2 ?? 0,
    affectedBy3: d.affectedBy3 ?? 0,
    affectedBy4: d.affectedBy4 ?? 0,
    act: d.act ?? 0,
    act2: d.act2 ?? 0,

    clan: d.clan ?? 0,
    religion: d.religion ?? 0,
    clev: d.clev ?? 0,
    language: d.language ?? new Array(27).fill(0),
    speaking: d.speaking ?? 0,
    size: d.size ?? 2,
    pkill: d.pkill ?? 0,
    shields: d.shields ?? 0,

    questpoints: d.questpoints ?? 0,
    nextquest: d.nextquest ?? 0,
    countdown: 0,
    questobj: d.questobj ?? 0,
    questmob: d.questmob ?? 0,
    rquestpoints: d.rquestpoints ?? 0,

    combatTimer: 0,
    summonTimer: 0,
    poisonLevel: 0,

    damageMods: d.damageMods ?? new Array(21).fill(0),
    mounted: d.mounted ?? 0,

    affects: deserializeAffects(d.affects),
    affects2: deserializeAffects(d.affects2),
    affects3: deserializeAffects(d.affects3),
    affects4: deserializeAffects(d.affects4),

    carryWeight: 0,
    carryNumber: 0,
    deleted: false,
    isNpc: false,
  };

  // Place character and PcData in the world
  world.characters.set(charId, ch);
  world.pcData.set(charId, pcdata);

  // Determine starting room
  const roomVnum = d.roomVnum ?? 25000;
  const targetRoom = world.getRoom(roomVnum);
  const startVnum = targetRoom
    ? roomVnum
    : world.rooms.keys().next().value ?? 2;
  charToRoom(ch, startVnum);

  // Restore equipment — create ObjInstances from stored vnums and equip
  if (Array.isArray(d.equipment)) {
    for (const eq of d.equipment) {
      const obj = world.createObjInstance(eq.vnum);
      if (obj) {
        obj.carriedBy = ch.id;
        obj.wearLoc = eq.wearLoc ?? WearLocation.NONE;
        ch.carryNumber += 1;
        ch.carryWeight += obj.weight;
      }
    }
  }

  // Restore inventory — create ObjInstances from stored vnums
  if (Array.isArray(d.inventory)) {
    for (const vnum of d.inventory) {
      const obj = world.createObjInstance(vnum);
      if (obj) {
        obj.carriedBy = ch.id;
        obj.wearLoc = WearLocation.NONE;
        ch.carryNumber += 1;
        ch.carryWeight += obj.weight;
      }
    }
  }

  return { char: ch, pcdata };
}

// ============================================================================
//  List characters for a user
// ============================================================================

/**
 * Query Firestore for all characters owned by the given Firebase uid.
 * Returns a summary array suitable for a character-selection menu.
 */
export async function listCharacters(
  uid: string
): Promise<Array<{ id: string; name: string; level: number; class: number }>> {
  if (!db) {
    console.error('listCharacters: Firestore not initialized.');
    return [];
  }

  const snap = await db
    .collection('characters')
    .where('ownerId', '==', uid)
    .get();

  return snap.docs.map((doc) => {
    const d = doc.data();
    return {
      id: doc.id,
      name: d.name ?? 'Unknown',
      level: d.level ?? 1,
      class: d.charClass ?? 3,
    };
  });
}

// ============================================================================
//  Auto-save system
// ============================================================================

/** Map character id -> owner uid (set by server.ts on connect). */
export const charOwnerMap = new Map<string, string>();

let autoSaveTimer: ReturnType<typeof setInterval> | null = null;

/**
 * Start the auto-save loop. Saves all connected PCs every `intervalMs`
 * milliseconds (default: 5 minutes).
 */
export function startAutoSave(intervalMs: number = 5 * 60 * 1000): void {
  if (autoSaveTimer !== null) {
    console.warn('Auto-save already running.');
    return;
  }

  autoSaveTimer = setInterval(() => {
    saveAllCharacters().catch((err) => {
      console.error('Auto-save error:', err);
    });
  }, intervalMs);

  console.log(
    `Auto-save started: every ${Math.round(intervalMs / 1000)} seconds`
  );
}

/**
 * Stop the auto-save loop.
 */
export function stopAutoSave(): void {
  if (autoSaveTimer !== null) {
    clearInterval(autoSaveTimer);
    autoSaveTimer = null;
    console.log('Auto-save stopped.');
  }
}

/**
 * Save every connected PC character. Called by auto-save and shutdown.
 */
export async function saveAllCharacters(): Promise<void> {
  const promises: Promise<void>[] = [];

  for (const ch of world.characters.values()) {
    if (ch.isNpc || ch.deleted) continue;
    if (!hasConnection(ch.id)) continue;

    const uid = charOwnerMap.get(ch.id);
    if (!uid) continue;

    promises.push(saveCharacter(ch, uid));
  }

  if (promises.length > 0) {
    await Promise.all(promises);
    console.log(`[auto-save] Saved ${promises.length} character(s).`);
  }
}
