/**
 * resets.ts — Area reset system for Stormgate MUD.
 *
 * Port of reset_room() and reset_area() from db.c in the C server.
 * Processes reset commands stored per-area (loaded from Firestore) to
 * repopulate mobs, objects, doors, and randomized exits.
 *
 * Reset data is stored in Firestore under world_state/resets, keyed by
 * area slug, with each reset having: command, arg1(status), arg2, arg3, arg4.
 *
 * The Firestore fields map to the C struct as follows:
 *   arg1 = status (area theme filter — 0 means always active)
 *   arg2 = C's arg1
 *   arg3 = C's arg2
 *   arg4 = C's arg3
 */

import type {
  CharData,
  ObjInstance,
  RoomIndex,
  ResetData,
} from './types.js';

import {
  Direction,
  ItemType,
  WearLocation,
} from './types.js';

import { world, charRoomMap, mobTemplateVnumMap } from './world.js';

import {
  charToRoom,
  objToRoom,
  objToChar,
  equipChar,
} from './handler.js';

// ============================================================================
//  Exit flag constants
// ============================================================================

export const EX_ISDOOR    = 1;
export const EX_CLOSED    = 2;
export const EX_LOCKED    = 4;
export const EX_BASHED    = 8;
export const EX_BASHPROOF = 16;
export const EX_PICKPROOF = 32;
export const EX_PASSPROOF = 64;
export const EX_HIDDEN    = 128;

// ============================================================================
//  Container flag constants (from merc.h CONT_*)
// ============================================================================

export const CONT_CLOSEABLE = 1;
export const CONT_PICKPROOF = 2;
export const CONT_CLOSED    = 4;
export const CONT_LOCKED    = 8;

// ============================================================================
//  ACT flag constants used by the reset system
// ============================================================================

const ITEM_INVENTORY = 1 << 2;   // ITEM_INVENTORY extra flag for shop items

// ============================================================================
//  Per-area reset data storage
// ============================================================================

/**
 * Firestore reset data as stored by the migration script.
 * arg1 = status, arg2 = C arg1, arg3 = C arg2, arg4 = C arg3.
 */
export interface FirestoreResetData {
  command: string;
  arg1: number;   // status
  arg2: number;   // C arg1
  arg3: number;   // C arg2
  arg4: number;   // C arg3
  comment?: string;
}

/**
 * Resets grouped by room vnum. Built during boot from the flat Firestore
 * list by analysing M/O/G/E/P/D/R command context (same as C load_resets).
 */
const roomResets = new Map<number, ResetData[]>();

/**
 * Runtime mob template instance count. Keyed by mob template vnum, value is
 * the number of live (non-deleted) instances currently in the world.
 */
const mobInstanceCount = new Map<number, number>();

// ============================================================================
//  Reverse direction lookup
// ============================================================================

const REV_DIR: Record<number, number> = {
  [Direction.NORTH]: Direction.SOUTH,
  [Direction.EAST]:  Direction.WEST,
  [Direction.SOUTH]: Direction.NORTH,
  [Direction.WEST]:  Direction.EAST,
  [Direction.UP]:    Direction.DOWN,
  [Direction.DOWN]:  Direction.UP,
};

// ============================================================================
//  Public API
// ============================================================================

/**
 * Load reset data from a Firestore-format object (keyed by area slug)
 * and assign resets to rooms, mirroring the C load_resets() logic.
 *
 * Called once during bootWorld in server.ts.
 */
export function loadResets(allResets: Record<string, FirestoreResetData[]>): void {
  roomResets.clear();

  for (const areaSlug of Object.keys(allResets)) {
    const resets = allResets[areaSlug];
    if (!resets || !Array.isArray(resets)) continue;

    let iLastRoom = 0;
    let iLastObj = 0;

    for (const r of resets) {
      const rd: ResetData = {
        command: r.command,
        arg1: r.arg2,   // C arg1
        arg2: r.arg3,   // C arg2
        arg3: r.arg4,   // C arg3
        status: r.arg1, // status
      };

      switch (r.command) {
        case 'M': {
          // arg1=mob_vnum, arg2=max_count, arg3=room_vnum
          const roomVnum = rd.arg3;
          if (world.getRoom(roomVnum)) {
            linkReset(roomVnum, rd);
            iLastRoom = roomVnum;
          }
          break;
        }
        case 'O': {
          // arg1=obj_vnum, arg2=0, arg3=room_vnum
          const roomVnum = rd.arg3;
          if (world.getRoom(roomVnum)) {
            linkReset(roomVnum, rd);
            iLastObj = roomVnum;
          }
          break;
        }
        case 'P': {
          // arg1=obj_vnum, arg2=0, arg3=container_vnum
          // Linked to the room of the last 'O' command
          if (world.getRoom(iLastObj)) {
            linkReset(iLastObj, rd);
          }
          break;
        }
        case 'G':
        case 'E': {
          // arg1=obj_vnum, arg2=0, arg3=wear_loc (E only, 0 for G)
          // Linked to the room of the last 'M' command
          if (world.getRoom(iLastRoom)) {
            linkReset(iLastRoom, rd);
            iLastObj = iLastRoom;
          }
          break;
        }
        case 'D': {
          // arg1=room_vnum, arg2=direction, arg3=state
          const roomVnum = rd.arg1;
          if (world.getRoom(roomVnum)) {
            linkReset(roomVnum, rd);
          }
          break;
        }
        case 'R': {
          // arg1=room_vnum, arg2=last_direction_to_randomize
          const roomVnum = rd.arg1;
          if (world.getRoom(roomVnum)) {
            linkReset(roomVnum, rd);
          }
          break;
        }
        default:
          break;
      }
    }
  }

  let totalResets = 0;
  for (const resets of roomResets.values()) {
    totalResets += resets.length;
  }
  console.log(`[resets] Loaded ${totalResets} resets across ${roomResets.size} rooms`);
}

/**
 * Reset a single room. Processes all reset commands assigned to this room.
 * Port of reset_room() from db.c.
 */
export function resetRoom(room: RoomIndex): void {
  if (!room) return;

  // First pass: reset all exit flags to their original rs_flags
  // (unless the exit has been bashed open)
  for (let dir = 0; dir < 6; dir++) {
    const exit = room.exits[dir as Direction];
    if (!exit) continue;
    if (exit.exitInfo & EX_BASHED) continue;

    exit.exitInfo = exit.rsFlags;

    // Also reset the reverse side
    const toRoom = world.getRoom(exit.toVnum);
    if (toRoom) {
      const revDir = REV_DIR[dir];
      const revExit = toRoom.exits[revDir as Direction];
      if (revExit && !(revExit.exitInfo & EX_BASHED)) {
        revExit.exitInfo = revExit.rsFlags;
      }
    }
  }

  // Get resets for this room
  const resets = roomResets.get(room.vnum);
  if (!resets || resets.length === 0) return;

  let lastMob: CharData | null = null;
  let level = 0;
  let last = false;

  for (const pReset of resets) {
    switch (pReset.command) {
      case 'M': {
        // Spawn a mob: arg1=mob_vnum, arg2=max_count, arg3=room_vnum
        const mobTemplate = world.getMobTemplate(pReset.arg1);
        if (!mobTemplate) {
          console.warn(`resetRoom: 'M': bad mob vnum ${pReset.arg1} in room ${room.vnum}`);
          continue;
        }

        // Check if instance count has reached max
        const currentCount = getMobCount(pReset.arg1);
        if (currentCount >= pReset.arg2) {
          last = false;
          break;
        }

        const mob = world.createMobInstance(pReset.arg1);
        if (!mob) {
          last = false;
          break;
        }

        charToRoom(mob, room.vnum);
        incrementMobCount(pReset.arg1);

        lastMob = mob;
        level = Math.max(0, Math.min(mob.level - 2, 100));
        last = true;
        break;
      }

      case 'O': {
        // Place object in room: arg1=obj_vnum, arg2=0, arg3=room_vnum
        const objTemplate = world.getObjTemplate(pReset.arg1);
        if (!objTemplate) {
          console.warn(`resetRoom: 'O': bad obj vnum ${pReset.arg1} in room ${room.vnum}`);
          continue;
        }

        // Don't place if players are in the area or object already present
        if (hasPlayersInArea(room.areaKey)) break;
        if (countObjInRoom(pReset.arg1, room.vnum) > 0) break;

        const obj = world.createObjInstance(pReset.arg1);
        if (!obj) break;

        objToRoom(obj, room.vnum);
        break;
      }

      case 'P': {
        // Put object in container: arg1=obj_vnum, arg2=0, arg3=container_vnum
        const objTemplate = world.getObjTemplate(pReset.arg1);
        if (!objTemplate) {
          console.warn(`resetRoom: 'P': bad obj vnum ${pReset.arg1} in room ${room.vnum}`);
          continue;
        }

        // Don't place if players are in the area
        if (hasPlayersInArea(room.areaKey)) break;

        // Find a live container instance by vnum
        const container = findObjByVnum(pReset.arg3);
        if (!container) break;

        // Check if item is already in the container
        if (countObjInContainer(pReset.arg1, container.id) > 0) break;

        const obj = world.createObjInstance(pReset.arg1);
        if (!obj) break;

        // Put object inside the container
        obj.containedIn = container.id;
        obj.inRoom = undefined;
        obj.carriedBy = undefined;

        // Reset container flags to template values
        if (container.itemType === ItemType.CONTAINER) {
          const cTemplate = world.getObjTemplate(container.indexVnum);
          if (cTemplate) {
            container.value[1] = cTemplate.value[1];
          }
        }
        break;
      }

      case 'G':
      case 'E': {
        // Give/Equip object to last spawned mob
        const objTemplate = world.getObjTemplate(pReset.arg1);
        if (!objTemplate) {
          console.warn(`resetRoom: '${pReset.command}': bad obj vnum ${pReset.arg1} in room ${room.vnum}`);
          continue;
        }

        if (!last) break;
        if (!lastMob) {
          last = false;
          break;
        }

        // Check if lastMob is a shopkeeper
        const templateVnum = mobTemplateVnumMap.get(lastMob.id) ?? 0;
        const isShopkeeper = world.shops.has(templateVnum);

        const obj = world.createObjInstance(pReset.arg1);
        if (!obj) break;

        if (isShopkeeper && pReset.command === 'G') {
          obj.extraFlags |= ITEM_INVENTORY;
        }

        objToChar(obj, lastMob);

        if (pReset.command === 'E') {
          const wearLoc = pReset.arg3 as WearLocation;
          equipChar(lastMob, obj, wearLoc);
        }

        last = true;
        break;
      }

      case 'D': {
        // Door reset: handled by the first pass (exit flag reset) above.
        // The 'D' command is a no-op here since we already restored rsFlags.
        break;
      }

      case 'R': {
        // Randomize exits: arg1=room_vnum, arg2=number of exits to shuffle
        const targetRoom = world.getRoom(pReset.arg1);
        if (!targetRoom) {
          console.warn(`resetRoom: 'R': bad room vnum ${pReset.arg1} in room ${room.vnum}`);
          continue;
        }

        const numExits = pReset.arg2;
        for (let d0 = 0; d0 < numExits; d0++) {
          const d1 = d0 + Math.floor(Math.random() * (numExits - d0));
          const temp = targetRoom.exits[d0 as Direction];
          targetRoom.exits[d0 as Direction] = targetRoom.exits[d1 as Direction];
          targetRoom.exits[d1 as Direction] = temp;
        }
        break;
      }

      default:
        console.warn(`resetRoom: unknown command '${pReset.command}' in room ${room.vnum}`);
        break;
    }
  }
}

/**
 * Reset all rooms in an area. Port of reset_area() from db.c.
 */
export function resetArea(areaKey: string): void {
  const area = world.areas.get(areaKey);
  if (!area) return;

  for (let vnum = area.lvnum; vnum <= area.uvnum; vnum++) {
    const room = world.getRoom(vnum);
    if (room) {
      resetRoom(room);
    }
  }
}

/**
 * Reset all areas in the world. Called during boot and periodically
 * from the area update pulse.
 */
export function resetAllAreas(): void {
  for (const areaKey of world.areas.keys()) {
    resetArea(areaKey);
  }
}

// ============================================================================
//  Mob instance counting
// ============================================================================

/**
 * Get the current live instance count for a mob template vnum.
 */
export function getMobCount(vnum: number): number {
  return mobInstanceCount.get(vnum) ?? 0;
}

/**
 * Increment the instance count for a mob template vnum.
 */
export function incrementMobCount(vnum: number): void {
  mobInstanceCount.set(vnum, (mobInstanceCount.get(vnum) ?? 0) + 1);
}

/**
 * Decrement the instance count for a mob template vnum.
 * Called when a mob is killed or otherwise removed from the world.
 */
export function decrementMobCount(vnum: number): void {
  const current = mobInstanceCount.get(vnum) ?? 0;
  if (current > 0) {
    mobInstanceCount.set(vnum, current - 1);
  }
}

/**
 * Rebuild mob instance counts by scanning all live characters.
 * Called during boot after all mobs have been spawned.
 */
export function rebuildMobCounts(): void {
  mobInstanceCount.clear();
  for (const ch of world.characters.values()) {
    if (ch.isNpc && !ch.deleted) {
      const vnum = mobTemplateVnumMap.get(ch.id);
      if (vnum !== undefined && vnum > 0) {
        incrementMobCount(vnum);
      }
    }
  }
}

// ============================================================================
//  Internal helpers
// ============================================================================

function linkReset(roomVnum: number, reset: ResetData): void {
  let list = roomResets.get(roomVnum);
  if (!list) {
    list = [];
    roomResets.set(roomVnum, list);
  }
  list.push(reset);
}

function hasPlayersInArea(areaKey: string): boolean {
  const area = world.areas.get(areaKey);
  if (!area) return false;

  for (const ch of world.characters.values()) {
    if (ch.isNpc || ch.deleted) continue;
    const roomVnum = charRoomMap.get(ch.id);
    if (roomVnum !== undefined && roomVnum >= area.lvnum && roomVnum <= area.uvnum) {
      return true;
    }
  }
  return false;
}

function countObjInRoom(objVnum: number, roomVnum: number): number {
  let count = 0;
  for (const obj of world.objects.values()) {
    if (obj.deleted) continue;
    if (obj.inRoom === roomVnum && obj.indexVnum === objVnum) {
      count++;
    }
  }
  return count;
}

function countObjInContainer(objVnum: number, containerId: string): number {
  let count = 0;
  for (const obj of world.objects.values()) {
    if (obj.deleted) continue;
    if (obj.containedIn === containerId && obj.indexVnum === objVnum) {
      count++;
    }
  }
  return count;
}

function findObjByVnum(vnum: number): ObjInstance | null {
  for (const obj of world.objects.values()) {
    if (obj.deleted) continue;
    if (obj.indexVnum === vnum) {
      return obj;
    }
  }
  return null;
}
