/**
 * update.ts — Game loop and tick system for Stormgate MUD.
 *
 * Port of update.c from the C server. Runs a setInterval-based loop at
 * PULSE_PER_SECOND frequency (4 Hz). Tracks pulse counts to fire periodic
 * updates: tick (regen/affects), area (resets), and violence (combat rounds).
 */

import {
  PULSE_PER_SECOND,
  PULSE_VIOLENCE,
  PULSE_TICK,
  PULSE_AREA,
  PULSE_MOBILE,
  Position,
  Direction,
  ItemType,
} from './types.js';

import type { CharData, AffectData, ObjInstance } from './types.js';

import { world, charRoomMap, SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING, SUN_DARK, SUN_RISE, SUN_LIGHT, SUN_SET } from './world.js';
import {
  affectRemoveFromChar,
  charToRoom,
  charFromRoom,
  getCharRoom,
  objToChar,
  objFromRoom,
  isName,
} from './handler.js';
import { sendToChar, sendToAll, act, colors, TO_ROOM } from './output.js';
import { multiHit, setFighting } from './fight.js';
import { sendVitals } from './protocol.js';
import { resetAllAreas } from './resets.js';
import { EX_CLOSED } from './resets.js';
import { questUpdate } from './quest.js';
import { craftUpdate } from './crafting.js';

// ============================================================================
//  Game loop state
// ============================================================================

let loopTimer: ReturnType<typeof setInterval> | null = null;
let pulseCount = 0;

// ============================================================================
//  Public API
// ============================================================================

/**
 * Start the game loop. Called once at server startup.
 * Fires PULSE_PER_SECOND times per real second (4 Hz by default).
 */
export function startGameLoop(): void {
  if (loopTimer !== null) {
    console.warn('Game loop already running.');
    return;
  }

  const intervalMs = Math.floor(1000 / PULSE_PER_SECOND);
  pulseCount = 0;

  loopTimer = setInterval(() => {
    try {
      updateHandler();
    } catch (err) {
      console.error('Error in game loop update:', err);
    }
  }, intervalMs);

  console.log(
    `Game loop started: ${PULSE_PER_SECOND} pulses/sec, tick every ${PULSE_TICK / PULSE_PER_SECOND}s, ` +
    `area every ${PULSE_AREA / PULSE_PER_SECOND}s, violence every ${PULSE_VIOLENCE / PULSE_PER_SECOND}s`
  );
}

/**
 * Stop the game loop. Called on server shutdown.
 */
export function stopGameLoop(): void {
  if (loopTimer !== null) {
    clearInterval(loopTimer);
    loopTimer = null;
    console.log('Game loop stopped.');
  }
}

// ============================================================================
//  Main pulse handler
// ============================================================================

/**
 * The main pulse handler, called PULSE_PER_SECOND times per second.
 * Tracks the pulse count and fires periodic sub-updates.
 */
function updateHandler(): void {
  pulseCount++;

  // Violence update — every PULSE_VIOLENCE pulses (3 seconds)
  if (pulseCount % PULSE_VIOLENCE === 0) {
    violenceUpdate();
  }

  // Mobile update — every PULSE_MOBILE pulses (4 seconds)
  if (pulseCount % PULSE_MOBILE === 0) {
    mobileUpdate();
  }

  // Tick update — every PULSE_TICK pulses (30 seconds)
  if (pulseCount % PULSE_TICK === 0) {
    tickUpdate();
  }

  // Area update — every PULSE_AREA pulses (60 seconds)
  if (pulseCount % PULSE_AREA === 0) {
    areaUpdate();
  }

  // Object timer decay — every PULSE_TICK (same as tick for simplicity)
  if (pulseCount % PULSE_TICK === 0) {
    objectTimerUpdate();
  }

  // Prevent the counter from growing unbounded
  if (pulseCount >= PULSE_AREA * PULSE_PER_SECOND * 60) {
    pulseCount = 0;
  }
}

// ============================================================================
//  Tick update — regeneration, affects, time, weather
// ============================================================================

/**
 * Tick-based updates, fired every PULSE_TICK (30 seconds).
 *
 * - Regenerate HP/mana/move for all characters
 * - Decrease affect durations, remove expired affects
 * - Advance game time
 * - Update weather
 */
function tickUpdate(): void {
  // Advance game time
  advanceTime();

  // Update weather
  updateWeather();

  // Process all characters
  for (const ch of world.characters.values()) {
    if (ch.deleted) continue;

    // Regeneration
    regenChar(ch);

    // Process affects — decrease durations and remove expired
    processAffects(ch);

    // Send updated vitals to connected PCs after regen
    if (!ch.isNpc) {
      sendVitals(ch);
    }
  }

  world.time.total++;

  // Quest timer update (countdown, nextquest cooldowns)
  questUpdate();

  // Crafting timer update (timed crafts in progress)
  craftUpdate();
}

/**
 * Regenerate HP, mana, and move for a character based on position.
 */
function regenChar(ch: CharData): void {
  // Dead/incap/mortal characters don't regen normally
  if (ch.position <= Position.STUNNED) return;

  // Base regen amounts per tick — position matters
  let hpGain = 0;
  let manaGain = 0;
  let moveGain = 0;

  switch (ch.position) {
    case Position.SLEEPING:
      hpGain = Math.max(1, Math.floor(ch.maxHit / 20));
      manaGain = Math.max(1, Math.floor(ch.maxMana / 15));
      moveGain = Math.max(1, Math.floor(ch.maxMove / 15));
      break;
    case Position.RESTING:
      hpGain = Math.max(1, Math.floor(ch.maxHit / 30));
      manaGain = Math.max(1, Math.floor(ch.maxMana / 20));
      moveGain = Math.max(1, Math.floor(ch.maxMove / 20));
      break;
    case Position.MEDITATING:
      hpGain = Math.max(1, Math.floor(ch.maxHit / 40));
      manaGain = Math.max(1, Math.floor(ch.maxMana / 10));
      moveGain = Math.max(1, Math.floor(ch.maxMove / 25));
      break;
    case Position.STANDING:
    case Position.GHOST:
    default:
      hpGain = Math.max(1, Math.floor(ch.maxHit / 50));
      manaGain = Math.max(1, Math.floor(ch.maxMana / 30));
      moveGain = Math.max(1, Math.floor(ch.maxMove / 30));
      break;
    case Position.FIGHTING:
      hpGain = 0;
      manaGain = 0;
      moveGain = 0;
      break;
  }

  // Apply regen (don't exceed max)
  ch.hit = Math.min(ch.maxHit, ch.hit + hpGain);
  ch.mana = Math.min(ch.maxMana, ch.mana + manaGain);
  ch.move = Math.min(ch.maxMove, ch.move + moveGain);
}

/**
 * Process spell/skill affects — decrease durations and remove expired.
 */
function processAffects(ch: CharData): void {
  const toRemove: AffectData[] = [];

  for (const af of ch.affects) {
    if (af.deleted) continue;

    if (af.duration > 0) {
      af.duration -= 1;

      // Warn when about to expire
      if (af.duration === 1) {
        sendToChar(ch, 'You feel a spell weakening.\r\n');
      }
    } else if (af.duration === 0) {
      // Duration expired — mark for removal
      toRemove.push(af);
    }
    // duration === -1 means permanent, never expires
  }

  for (const af of toRemove) {
    sendToChar(ch, 'A spell has worn off.\r\n');
    affectRemoveFromChar(ch, af);
  }

  // Also process affects2, affects3, affects4
  processAffectGroup(ch, ch.affects2);
  processAffectGroup(ch, ch.affects3);
  processAffectGroup(ch, ch.affects4);
}

/**
 * Process a secondary affect group (affects2/3/4).
 */
function processAffectGroup(ch: CharData, affectList: AffectData[]): void {
  const toRemove: AffectData[] = [];

  for (const af of affectList) {
    if (af.deleted) continue;

    if (af.duration > 0) {
      af.duration -= 1;
    } else if (af.duration === 0) {
      toRemove.push(af);
    }
  }

  for (const af of toRemove) {
    const idx = affectList.indexOf(af);
    if (idx !== -1) {
      affectList.splice(idx, 1);
    }
  }
}

// ============================================================================
//  Time system
// ============================================================================

/** Hours per day in the MUD world. */
const HOURS_PER_DAY = 24;
/** Days per month. */
const DAYS_PER_MONTH = 35;
/** Months per year. */
const MONTHS_PER_YEAR = 17;

/**
 * Advance the in-game clock by one hour.
 */
function advanceTime(): void {
  const time = world.time;

  time.hour++;

  if (time.hour >= HOURS_PER_DAY) {
    time.hour = 0;
    time.day++;
  }

  if (time.day >= DAYS_PER_MONTH) {
    time.day = 0;
    time.month++;
  }

  if (time.month >= MONTHS_PER_YEAR) {
    time.month = 0;
    time.year++;
  }

  // Update sunlight position based on hour
  if (time.hour === 5) {
    world.weather.sunlight = SUN_RISE;
    sendToAll('The sun rises in the east.\r\n');
  } else if (time.hour === 6) {
    world.weather.sunlight = SUN_LIGHT;
    sendToAll('The day has begun.\r\n');
  } else if (time.hour === 19) {
    world.weather.sunlight = SUN_SET;
    sendToAll('The sun slowly disappears in the west.\r\n');
  } else if (time.hour === 20) {
    world.weather.sunlight = SUN_DARK;
    sendToAll('The night has begun.\r\n');
  }
}

// ============================================================================
//  Weather system
// ============================================================================

/**
 * Update weather conditions. Based on the C server's weather_update().
 */
function updateWeather(): void {
  const w = world.weather;

  // Random pressure change
  const diff = Math.floor(Math.random() * 7) - 3; // -3 to +3
  w.change += diff;

  // Keep change in a reasonable range
  if (w.change < -10) w.change = -10;
  if (w.change > 10) w.change = 10;

  w.mmhg += w.change;

  // Keep pressure in reasonable range
  if (w.mmhg < 920) w.mmhg = 920;
  if (w.mmhg > 1040) w.mmhg = 1040;

  // Sky transitions based on pressure
  const prevSky = w.sky;

  if (w.mmhg <= 940) {
    // Very low pressure — storms
    if (w.sky < SKY_LIGHTNING) w.sky = SKY_LIGHTNING;
  } else if (w.mmhg <= 960) {
    if (w.sky < SKY_RAINING) w.sky = SKY_RAINING;
  } else if (w.mmhg <= 980) {
    if (w.sky === SKY_CLOUDLESS) w.sky = SKY_CLOUDY;
    if (w.sky === SKY_LIGHTNING) w.sky = SKY_RAINING;
  } else if (w.mmhg >= 1020) {
    // High pressure — clearing
    if (w.sky > SKY_CLOUDLESS) w.sky = Math.max(SKY_CLOUDLESS, w.sky - 1) as typeof w.sky;
  }

  // Announce weather changes
  if (w.sky !== prevSky) {
    switch (w.sky) {
      case SKY_CLOUDLESS:
        sendToAll('The clouds disappear.\r\n');
        break;
      case SKY_CLOUDY:
        if (prevSky < SKY_CLOUDY) {
          sendToAll('The sky is getting cloudy.\r\n');
        } else {
          sendToAll('The rain has stopped.\r\n');
        }
        break;
      case SKY_RAINING:
        if (prevSky < SKY_RAINING) {
          sendToAll('It starts to rain.\r\n');
        } else {
          sendToAll('The lightning has stopped.\r\n');
        }
        break;
      case SKY_LIGHTNING:
        sendToAll('Lightning flashes in the sky.\r\n');
        break;
    }
  }
}

// ============================================================================
//  Area update
// ============================================================================

/**
 * Area-based updates, fired every PULSE_AREA (60 seconds).
 * Handles resetting areas (respawning mobs, resetting doors, etc.).
 */
function areaUpdate(): void {
  resetAllAreas();
}

// ============================================================================
//  Violence update
// ============================================================================

/**
 * Violence updates, fired every PULSE_VIOLENCE (3 seconds).
 * Processes combat rounds for all fighting characters.
 */
function violenceUpdate(): void {
  // Collect characters in combat (iterate a snapshot to avoid mutation issues)
  const fighters: CharData[] = [];
  for (const ch of world.characters.values()) {
    if (ch.deleted) continue;
    if (ch.position !== Position.FIGHTING) continue;
    if (ch.fighting === null) continue;
    fighters.push(ch);
  }

  for (const ch of fighters) {
    if (ch.deleted) continue;
    if (ch.position !== Position.FIGHTING) continue;
    if (ch.fighting === null) continue;

    const victim = world.getCharById(ch.fighting);
    if (!victim || victim.deleted) {
      // Target is gone, stop fighting
      ch.fighting = null;
      if (ch.position === Position.FIGHTING) {
        ch.position = Position.STANDING;
      }
      continue;
    }

    // Check they are still in the same room
    const chRoom = getCharRoom(ch);
    const victRoom = getCharRoom(victim);
    if (chRoom === -1 || victRoom === -1 || chRoom !== victRoom) {
      ch.fighting = null;
      if (ch.position === Position.FIGHTING) {
        ch.position = Position.STANDING;
      }
      continue;
    }

    // Process one combat round
    multiHit(ch, victim);
  }
}

// ============================================================================
//  Mobile (NPC) update
// ============================================================================

/**
 * Mobile updates, fired every PULSE_MOBILE (4 seconds).
 * Handles NPC behavior: wandering, scavenging, aggressive attacks.
 */
function mobileUpdate(): void {
  // ACT flag bit definitions
  const ACT_SENTINEL   = 1 << 2;  // bit 2: mob does not wander
  const ACT_AGGRESSIVE = 1 << 5;  // bit 5: mob attacks PCs on sight
  const ACT_SCAVENGER  = 1 << 6;  // bit 6: mob picks up items

  // Snapshot the character list to avoid mutation during iteration
  const mobs: CharData[] = [];
  for (const ch of world.characters.values()) {
    if (!ch.isNpc || ch.deleted) continue;
    mobs.push(ch);
  }

  for (const ch of mobs) {
    if (ch.deleted) continue;

    // Skip mobs that are fighting, sleeping, etc.
    if (ch.position < Position.STANDING) continue;
    if (ch.position === Position.FIGHTING) continue;

    const roomVnum = getCharRoom(ch);
    if (roomVnum === -1) continue;

    const room = world.getRoom(roomVnum);
    if (!room) continue;

    // ---- Scavenger behavior ----
    // Mobs with ACT_SCAVENGER pick up a random item from the room
    if (ch.act & ACT_SCAVENGER) {
      if (Math.random() < 0.10) {
        const roomObjs = world.getObjsInRoom(roomVnum);
        if (roomObjs.length > 0) {
          const obj = roomObjs[Math.floor(Math.random() * roomObjs.length)];
          // Don't pick up corpses
          if (obj.itemType !== ItemType.CORPSE_NPC && obj.itemType !== ItemType.CORPSE_PC) {
            objFromRoom(obj);
            objToChar(obj, ch);
            act('$n gets $p.', ch, obj, null, TO_ROOM);
          }
        }
      }
    }

    // ---- Aggressive behavior ----
    // Mobs with ACT_AGGRESSIVE attack any PC in the room
    if (ch.act & ACT_AGGRESSIVE) {
      const charsInRoom = world.getCharsInRoom(roomVnum);
      for (const victim of charsInRoom) {
        if (victim.isNpc || victim.deleted) continue;
        if (victim.position < Position.STANDING) continue;
        // Don't attack immortals
        if (victim.level >= 105) continue;

        // Attack!
        act(`${colors.brightRed}$n screams and attacks!${colors.reset}`, ch, null, null, TO_ROOM);
        setFighting(ch, victim);
        multiHit(ch, victim);
        break; // Only attack one target per pulse
      }
      // If we started fighting, skip wandering
      if (ch.position === Position.FIGHTING) continue;
    }

    // ---- Wandering behavior ----
    // Mobs without ACT_SENTINEL have a 10% chance to move randomly
    if (!(ch.act & ACT_SENTINEL)) {
      if (Math.random() < 0.10) {
        // Collect available exits
        const availableDirs: Direction[] = [];
        for (let dir = 0; dir < 6; dir++) {
          const exit = room.exits[dir as keyof typeof room.exits];
          if (exit && !(exit.exitInfo & EX_CLOSED)) {
            // Check that the destination room exists
            const destRoom = world.getRoom(exit.toVnum);
            if (destRoom) {
              availableDirs.push(dir as Direction);
            }
          }
        }

        if (availableDirs.length > 0) {
          const dir = availableDirs[Math.floor(Math.random() * availableDirs.length)];
          const exit = room.exits[dir as keyof typeof room.exits];
          if (exit) {
            const DIRECTION_NAMES = ['north', 'east', 'south', 'west', 'up', 'down'];
            act(`$n leaves ${DIRECTION_NAMES[dir]}.`, ch, null, null, TO_ROOM);
            charFromRoom(ch);
            charToRoom(ch, exit.toVnum);
            act('$n has arrived.', ch, null, null, TO_ROOM);
          }
        }
      }
    }
  }
}

// ============================================================================
//  Object timer update
// ============================================================================

/**
 * Decrement object timers. Objects with timer=0 are destroyed (corpses, etc.).
 */
function objectTimerUpdate(): void {
  const toDelete: string[] = [];

  for (const obj of world.objects.values()) {
    if (obj.deleted) continue;
    if (obj.timer <= 0) continue;

    obj.timer -= 1;

    if (obj.timer === 0) {
      // Timer expired — destroy the object
      toDelete.push(obj.id);
    }
  }

  for (const id of toDelete) {
    const obj = world.objects.get(id);
    if (obj) {
      obj.deleted = true;
      world.objects.delete(id);
    }
  }
}
