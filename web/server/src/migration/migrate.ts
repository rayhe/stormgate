/**
 * migrate.ts — Import parsed Stormgate MUD area files into Firestore.
 *
 * Usage:
 *   npx tsx src/migration/migrate.ts /path/to/area/directory
 *
 * If no directory is given, defaults to ../../area (relative to this file's
 * location at web/server/src/migration/).
 */

import { readFileSync, existsSync } from 'fs';
import { join, basename, resolve, dirname } from 'path';
import { fileURLToPath } from 'url';
import { initializeApp, cert, type ServiceAccount } from 'firebase-admin/app';
import { getFirestore, type WriteBatch } from 'firebase-admin/firestore';
import { parseAreaList, parseAreaFile, type ParsedArea } from './area-parser.js';

// ---------------------------------------------------------------------------
// Resolve paths
// ---------------------------------------------------------------------------

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const areaDir = resolve(process.argv[2] || join(__dirname, '..', '..', '..', '..', 'area'));

if (!existsSync(areaDir)) {
  console.error(`Area directory not found: ${areaDir}`);
  process.exit(1);
}

console.log(`Area directory: ${areaDir}`);

// ---------------------------------------------------------------------------
// Firebase Admin Setup (same pattern as server/src/index.ts)
// ---------------------------------------------------------------------------

const serviceAccountPath = process.env.GOOGLE_APPLICATION_CREDENTIALS
  || join(__dirname, '..', '..', 'service-account.json');

if (existsSync(serviceAccountPath)) {
  const sa = JSON.parse(readFileSync(serviceAccountPath, 'utf-8')) as ServiceAccount;
  initializeApp({ credential: cert(sa) });
} else {
  initializeApp();
}

const db = getFirestore();

// ---------------------------------------------------------------------------
// Batch writer helper — auto-commits every 500 operations
// ---------------------------------------------------------------------------

class BatchWriter {
  private batch: WriteBatch;
  private count = 0;
  private totalWrites = 0;

  constructor() {
    this.batch = db.batch();
  }

  set(ref: FirebaseFirestore.DocumentReference, data: Record<string, unknown>): void {
    this.batch.set(ref, data, { merge: true });
    this.count++;
    this.totalWrites++;
    if (this.count >= 500) {
      // We'll flush synchronously via the queue
      this.pendingFlushes.push(this.batch.commit());
      this.batch = db.batch();
      this.count = 0;
    }
  }

  private pendingFlushes: Promise<unknown>[] = [];

  async flush(): Promise<void> {
    if (this.count > 0) {
      this.pendingFlushes.push(this.batch.commit());
      this.batch = db.batch();
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
// Load area files with filenames
// ---------------------------------------------------------------------------

interface AreaWithFilename {
  filename: string;
  area: ParsedArea;
}

function loadAreasWithFilenames(dir: string): AreaWithFilename[] {
  const listPath = join(dir, 'areaTS.lst');
  let listContent: string;
  try {
    listContent = readFileSync(listPath, 'utf-8');
  } catch (err) {
    console.error(`Could not read area list file: ${listPath}`, err);
    process.exit(1);
  }

  const filenames = parseAreaList(listContent);
  const results: AreaWithFilename[] = [];

  for (const filename of filenames) {
    const filePath = join(dir, filename);
    try {
      const content = readFileSync(filePath, 'utf-8');
      const area = parseAreaFile(content);
      results.push({ filename, area });
    } catch (err) {
      console.warn(`Skipping "${filename}": ${err}`);
    }
  }

  return results;
}

// ---------------------------------------------------------------------------
// Slug helper
// ---------------------------------------------------------------------------

function filenameToSlug(filename: string): string {
  // "limbo.are" -> "limbo", "stonegate_castle.are" -> "stonegate_castle"
  return basename(filename, '.are').toLowerCase();
}

// ---------------------------------------------------------------------------
// Convert parsed structures to plain Firestore-safe objects
// ---------------------------------------------------------------------------

function roomExitsToMap(exits: ParsedArea['rooms'][0]['exits']): Record<string, Record<string, unknown>> {
  const map: Record<string, Record<string, unknown>> = {};
  for (const exit of exits) {
    map[String(exit.direction)] = {
      direction: exit.direction,
      description: exit.description,
      keyword: exit.keyword,
      locks: exit.locks,
      keyVnum: exit.keyVnum,
      toVnum: exit.toVnum,
    };
  }
  return map;
}

// ---------------------------------------------------------------------------
// Main migration
// ---------------------------------------------------------------------------

async function migrate(): Promise<void> {
  console.log('Loading area files...');
  const areasWithFilenames = loadAreasWithFilenames(areaDir);
  console.log(`Loaded ${areasWithFilenames.length} area(s).`);

  if (areasWithFilenames.length === 0) {
    console.log('No areas to import. Exiting.');
    return;
  }

  const writer = new BatchWriter();
  let areaCount = 0;

  for (const { filename, area } of areasWithFilenames) {
    const slug = filenameToSlug(filename);
    const meta = area.metadata;
    areaCount++;

    console.log(`[${areaCount}/${areasWithFilenames.length}] Importing area: ${meta.name || slug} (${filename})`);

    // --- areas/{areaSlug} ---
    const areaRef = db.collection('areas').doc(slug);
    writer.set(areaRef, {
      slug,
      name: meta.name,
      filename,
      vnumRange: { low: meta.vnumLow, high: meta.vnumHigh },
      builders: meta.builders,
      security: meta.security,
      recall: meta.recall,
      flags: meta.flags,
      version: meta.version,
      creator: meta.creator,
      llevel: meta.lowerLevel,
      ulevel: meta.upperLevel,
    });

    // --- areas/{areaSlug}/rooms/{vnum} ---
    for (const room of area.rooms) {
      const roomRef = areaRef.collection('rooms').doc(String(room.vnum));
      writer.set(roomRef, {
        vnum: room.vnum,
        name: room.name,
        description: room.description,
        sectorType: room.sectorType,
        roomFlags: room.roomFlags,
        exits: roomExitsToMap(room.exits),
        extraDescriptions: room.extraDescs.map((ed) => ({
          keyword: ed.keyword,
          description: ed.description,
        })),
      });
    }

    // --- mob_templates/{vnum} ---
    for (const mob of area.mobiles) {
      const mobRef = db.collection('mob_templates').doc(String(mob.vnum));
      writer.set(mobRef, {
        vnum: mob.vnum,
        areaSlug: slug,
        keywords: mob.keywords,
        shortDescription: mob.shortDescription,
        longDescription: mob.longDescription,
        description: mob.description,
        actFlags: mob.actFlags,
        affectedBy: mob.affectedBy,
        affectedBy2: mob.affectedBy2,
        affectedBy3: mob.affectedBy3,
        bodyParts: mob.bodyParts,
        alignment: mob.alignment,
        level: mob.level,
        hitNoDice: mob.hitNoDice,
        hitSizeDice: mob.hitSizeDice,
        hitPlus: mob.hitPlus,
        hpPlus: mob.hpPlus,
        damNoDice: mob.damNoDice,
        damSizeDice: mob.damSizeDice,
        damPlus: mob.damPlus,
        gold: mob.gold,
        hitroll: mob.hitroll,
        damroll: mob.damroll,
        ac: mob.ac,
        acUnknown: mob.acUnknown,
        sex: mob.sex,
        size: mob.size,
        shields: mob.shields,
        speaking: mob.speaking,
        mobProgs: mob.mobProgs.map((mp) => ({
          trigger: mp.trigger,
          argument: mp.argument,
          commands: mp.commands,
        })),
      });
    }

    // --- obj_templates/{vnum} ---
    for (const obj of area.objects) {
      const objRef = db.collection('obj_templates').doc(String(obj.vnum));
      writer.set(objRef, {
        vnum: obj.vnum,
        areaSlug: slug,
        keywords: obj.keywords,
        shortDescription: obj.shortDescription,
        longDescription: obj.longDescription,
        itemType: obj.itemType,
        extraFlags: obj.extraFlags,
        extraFlags2: obj.extraFlags2,
        extraFlags3: obj.extraFlags3,
        extraFlags4: obj.extraFlags4,
        wearFlags: obj.wearFlags,
        level: obj.level,
        conditionNow: obj.conditionNow,
        conditionMax: obj.conditionMax,
        values: obj.values,
        weight: obj.weight,
        cost: obj.cost,
        costMult: obj.costMult,
        charges: obj.charges,
        spellLevel: obj.spellLevel,
        spellName: obj.spellName,
        timer: obj.timer,
        timerArg: obj.timerArg,
        extraFlags5: obj.extraFlags5,
        extraFlags6: obj.extraFlags6,
        extraFlags7: obj.extraFlags7,
        affects: obj.affects.map((af) => ({
          location: af.location,
          modifier: af.modifier,
        })),
        extraDescs: obj.extraDescs.map((ed) => ({
          keyword: ed.keyword,
          description: ed.description,
        })),
      });
    }

    // --- world_state/resets ---
    // Store resets grouped by area slug
    if (area.resets.length > 0) {
      const resetsRef = db.collection('world_state').doc('resets');
      writer.set(resetsRef, {
        [slug]: area.resets.map((r) => ({
          command: r.command,
          arg1: r.arg1,
          arg2: r.arg2,
          arg3: r.arg3,
          arg4: r.arg4,
          comment: r.comment,
        })),
      });
    }

    // --- world_state/shops ---
    if (area.shops.length > 0) {
      const shopsRef = db.collection('world_state').doc('shops');
      writer.set(shopsRef, {
        [slug]: area.shops.map((s) => ({
          keeper: s.keeper,
          buyType1: s.buyType1,
          buyType2: s.buyType2,
          buyType3: s.buyType3,
          buyType4: s.buyType4,
          buyType5: s.buyType5,
          profitBuy: s.profitBuy,
          profitSell: s.profitSell,
          openHour: s.openHour,
          closeHour: s.closeHour,
        })),
      });
    }

    // --- world_state/specials ---
    if (area.specials.length > 0) {
      const specialsRef = db.collection('world_state').doc('specials');
      writer.set(specialsRef, {
        [slug]: area.specials.map((sp) => ({
          mobVnum: sp.mobVnum,
          specFun: sp.specFun,
        })),
      });
    }

    console.log(
      `  -> ${area.rooms.length} rooms, ${area.mobiles.length} mobs, ` +
      `${area.objects.length} objects, ${area.resets.length} resets, ` +
      `${area.shops.length} shops, ${area.specials.length} specials`
    );
  }

  // Flush any remaining writes
  console.log(`\nFlushing ${writer.total} total write(s) to Firestore...`);
  await writer.flush();

  console.log('Migration complete!');
}

// ---------------------------------------------------------------------------
// Run
// ---------------------------------------------------------------------------

migrate().catch((err) => {
  console.error('Migration failed:', err);
  process.exit(1);
});
