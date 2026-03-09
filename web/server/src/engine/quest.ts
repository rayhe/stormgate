/**
 * quest.ts -- Automated Quest System for Stormgate MUD.
 *
 * Ported from quest.c (Vassago's Auto-Quest v2.01).
 *
 * Players interact with questmaster NPCs to request, complete, and buy
 * rewards with quests. The system generates random kill or fetch quests,
 * tracks countdown timers, and rewards quest points, gold, and XP.
 *
 * All commands export a CommandEntry[] array (questCommands) that can be
 * spread into the master command table in commands.ts.
 */

import type {
  CharData,
  ObjInstance,
} from './types.js';

import {
  Position,
  ItemType,
} from './types.js';

import { world, charRoomMap, rollDice } from './world.js';
import {
  getCharRoom,
  isName,
  objToChar,
  getCarriedItems,
} from './handler.js';

import {
  sendToChar,
  act,
  colors,
  hasConnection,
  TO_ROOM,
} from './output.js';

import { sendVitals } from './protocol.js';

import type { CommandEntry } from './commands.js';

// ============================================================================
//  Constants
// ============================================================================

/** Reward items available for purchase with quest points. */
interface QuestRewardItem {
  id: string;
  name: string;
  cost: number;
}

const QUEST_REWARDS: QuestRewardItem[] = [
  { id: '1',  name: 'Shield of Abraxyz',              cost: 500  },
  { id: '2',  name: 'Contact Lenses',                 cost: 750  },
  { id: '3',  name: 'Potion of Godlike Strength',     cost: 150  },
  { id: '4',  name: 'Parchment of Immortals',         cost: 150  },
  { id: '5',  name: 'Staff of Power',                 cost: 1000 },
  { id: '6',  name: 'Storm Lantern of Unholy Plague', cost: 1500 },
  { id: '7',  name: 'Amulet of the Lost',             cost: 2500 },
  { id: '8',  name: 'The Eye of Gardith',             cost: 4000 },
  { id: '9',  name: 'The Sword of Abraxyz',           cost: 4500 },
  { id: '10', name: 'Ring of Abraxyz',                cost: 5000 },
  { id: '11', name: 'Potion of Luck',                 cost: 500  },
  { id: '12', name: 'Black Potion',                   cost: 25   },
  { id: '13', name: 'Potion of Titan Strength',       cost: 100  },
  { id: '17', name: 'Book of the Gods',               cost: 25   },
];

const QUEST_REWARDS_CURRENCY: { id: string; name: string; cost: number; type: 'gold' | 'pracs' | 'learns'; amount: number }[] = [
  { id: 'a', name: '2,500,000 gold',  cost: 250, type: 'gold',   amount: 2500000 },
  { id: 'b', name: '175 practices',   cost: 500, type: 'pracs',  amount: 175 },
  { id: 'c', name: '25 practices',    cost: 80,  type: 'pracs',  amount: 25 },
  { id: 'd', name: '2 learns',        cost: 25,  type: 'learns', amount: 2 },
];

/** Quest object names used for fetch quests. */
const QUEST_FETCH_OBJECTS = [
  'the Holy Grail',
  'the Scepter of Ages',
  'the Crown of Kings',
  'the Staff of Wizardry',
  'the Orb of Prophecy',
];

/** Kill quest flavor text variations. */
const KILL_QUEST_INTROS = [
  (name: string) =>
    `An enemy of mine, ${name}, is making vile threats against the crown. This threat must be eliminated!`,
  (name: string) =>
    `The most heinous criminal, ${name}, has escaped from the dungeon! The penalty for this crime is death, and you are to deliver the sentence!`,
];

// ============================================================================
//  String hashCode helper (for stable pseudo-vnums from string IDs)
// ============================================================================

function hashCode(str: string): number {
  let hash = 0;
  for (let i = 0; i < str.length; i++) {
    const chr = str.charCodeAt(i);
    hash = ((hash << 5) - hash) + chr;
    hash |= 0;
  }
  return Math.abs(hash);
}

// ============================================================================
//  Quest master detection
// ============================================================================

/**
 * Find a questmaster NPC in the character's room.
 * In the C code this checked spec_questmaster. Here we check if the
 * NPC's name or shortDescr contains "questmaster" or "quest master".
 */
function findQuestmaster(ch: CharData): CharData | undefined {
  const roomVnum = getCharRoom(ch);
  if (roomVnum === -1) return undefined;

  for (const mob of world.getCharsInRoom(roomVnum)) {
    if (!mob.isNpc || mob.deleted) continue;
    const lower = (mob.name + ' ' + mob.shortDescr).toLowerCase();
    if (lower.includes('questmaster') || lower.includes('quest master')) {
      return mob;
    }
  }
  return undefined;
}

// ============================================================================
//  Quest generation
// ============================================================================

/**
 * Generate a random quest for a character.
 * Picks a random mob in the world and assigns either a kill or fetch quest.
 */
function generateQuest(ch: CharData, questman: CharData): void {
  // Collect eligible mobs
  const eligible: CharData[] = [];
  for (const mob of world.characters.values()) {
    if (!mob.isNpc || mob.deleted) continue;
    if (mob.level < 1) continue;

    // Skip special mobs (shops, trainers, etc.) by checking name
    const lowerName = (mob.name + ' ' + mob.shortDescr).toLowerCase();
    if (lowerName.includes('questmaster') ||
        lowerName.includes('quest master') ||
        lowerName.includes('shopkeeper') ||
        lowerName.includes('banker') ||
        lowerName.includes('trainer') ||
        lowerName.includes('healer')) {
      continue;
    }

    // Level check: mob should be within range of the player
    if (!questLevelDiff(ch.level, mob.level)) continue;

    // Must have a room
    const mobRoom = charRoomMap.get(mob.id);
    if (mobRoom === undefined) continue;

    eligible.push(mob);
  }

  if (eligible.length === 0) {
    sendToChar(ch, `${colors.white}I'm sorry, but I don't have any quests available right now. Try again later.\r\n${colors.reset}`);
    return;
  }

  // Pick a random target
  const victim = eligible[Math.floor(Math.random() * eligible.length)];
  const victimRoom = charRoomMap.get(victim.id)!;
  const room = world.rooms.get(victimRoom);

  // Determine area name from room's areaKey
  let areaName = 'unknown lands';
  let roomName = 'somewhere';
  if (room) {
    roomName = room.name || 'somewhere';
    // Use the room's areaKey to look up the area
    if (room.areaKey) {
      const area = world.areas.get(room.areaKey);
      if (area) {
        areaName = area.name;
      }
    }
  }

  ch.questArea = areaName;
  ch.questRoom = roomName;

  const countdown = rollDice(1, 20) + 10; // 11-30 ticks
  ch.countdown = countdown;

  // 40% chance of a fetch quest, 60% kill quest
  if (Math.random() < 0.4) {
    // Fetch quest
    const fetchItem = QUEST_FETCH_OBJECTS[Math.floor(Math.random() * QUEST_FETCH_OBJECTS.length)];
    ch.questobj = victim.vnum || hashCode(victim.id);
    ch.questmob = 0;
    ch.questFetchItem = fetchItem;

    questmanSays(ch, questman,
      `Vile pilferers have stolen ${fetchItem} from the royal treasury!`,
    );
    questmanSays(ch, questman,
      'My court wizardess, with her magic mirror, has pinpointed its location.',
    );
    questmanSays(ch, questman,
      `Look in the general area of ${areaName} for ${roomName}!`,
    );
  } else {
    // Kill quest
    ch.questmob = victim.vnum || hashCode(victim.id);
    ch.questobj = 0;
    ch.questFetchItem = '';

    const intro = KILL_QUEST_INTROS[Math.floor(Math.random() * KILL_QUEST_INTROS.length)];
    questmanSays(ch, questman, intro(victim.shortDescr || victim.name));
    questmanSays(ch, questman,
      `Seek ${victim.shortDescr || victim.name} out somewhere in the vicinity of ${roomName}!`,
    );
    questmanSays(ch, questman,
      `That location is in the general area of ${areaName}.`,
    );
  }
}

/** Check if mob level is appropriate for the player's level. */
function questLevelDiff(clevel: number, mlevel: number): boolean {
  const cl = Math.min(100, clevel);
  const ml = Math.min(100, mlevel);
  if (ml < cl) return false;
  return (ml - cl < 10) || (ml >= 100 && cl >= 100);
}

/** Helper to format questmaster speech. */
function questmanSays(ch: CharData, questman: CharData, text: string): void {
  sendToChar(ch,
    `${colors.white}${questman.shortDescr || questman.name} says '${text}'\r\n${colors.reset}`,
  );
}

// ============================================================================
//  Main quest command
// ============================================================================

/**
 * doQuest -- Quest meta-command.
 * Usage: quest info | quest points | quest time | quest request |
 *        quest complete | quest refuse | quest list | quest buy <item>
 */
export function doQuest(ch: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch,
      `${colors.white}QUEST commands: POINTS INFO TIME REQUEST COMPLETE REFUSE LIST BUY.\r\nFor more information, type 'HELP QUEST'.\r\n${colors.reset}`,
    );
    return;
  }

  const parts = argument.trim().split(/\s+/);
  const sub = parts[0].toLowerCase();
  const rest = parts.slice(1).join(' ');

  // -- info, points, and time don't require a questmaster --

  if (sub === 'info') {
    doQuestInfo(ch);
    return;
  }

  if (sub === 'points') {
    sendToChar(ch, `${colors.white}You have ${ch.questpoints} quest points.\r\n${colors.reset}`);
    return;
  }

  if (sub === 'time') {
    doQuestTime(ch);
    return;
  }

  // -- All other sub-commands require a questmaster in the room --

  const questman = findQuestmaster(ch);
  if (!questman) {
    sendToChar(ch, 'You can\'t do that here. Find a questmaster.\r\n');
    return;
  }

  if (questman.fighting) {
    sendToChar(ch, 'Wait until the fighting stops.\r\n');
    return;
  }

  switch (sub) {
    case 'list':     doQuestList(ch, questman); break;
    case 'buy':      doQuestBuy(ch, questman, rest); break;
    case 'request':  doQuestRequest(ch, questman); break;
    case 'complete': doQuestComplete(ch, questman); break;
    case 'refuse':   doQuestRefuse(ch, questman); break;
    default:
      sendToChar(ch,
        `${colors.white}QUEST commands: POINTS INFO TIME REQUEST COMPLETE REFUSE LIST BUY.\r\n${colors.reset}`,
      );
      break;
  }
}

/** Show current quest details. */
function doQuestInfo(ch: CharData): void {
  if (!ch.isQuestor) {
    sendToChar(ch, 'You aren\'t currently on a quest.\r\n');
    return;
  }

  if (ch.questmob === -1) {
    sendToChar(ch,
      `${colors.brightGreen}Your quest is ALMOST complete!\r\nGet back to the questmaster before your time runs out!\r\n${colors.reset}`,
    );
    return;
  }

  if (ch.questobj > 0 && ch.questFetchItem) {
    sendToChar(ch,
      `${colors.white}You are on a quest to recover the fabled ${ch.questFetchItem}!\r\nLook in ${ch.questArea} near ${ch.questRoom}.\r\n${colors.reset}`,
    );
    return;
  }

  if (ch.questmob > 0) {
    sendToChar(ch,
      `${colors.white}You are on a quest to slay a dreaded foe!\r\nSearch in ${ch.questArea} near ${ch.questRoom}.\r\n${colors.reset}`,
    );
    return;
  }

  sendToChar(ch, 'You aren\'t currently on a quest.\r\n');
}

/** Show quest timer info. */
function doQuestTime(ch: CharData): void {
  if (!ch.isQuestor) {
    sendToChar(ch, 'You aren\'t currently on a quest.\r\n');

    if (ch.nextquest > 1) {
      sendToChar(ch,
        `There are ${ch.nextquest} minutes remaining until you can go on another quest.\r\n`,
      );
    } else if (ch.nextquest === 1) {
      sendToChar(ch,
        'There is less than a minute remaining until you can go on another quest.\r\n',
      );
    }
    return;
  }

  if (ch.countdown > 0) {
    sendToChar(ch,
      `${colors.white}Time left for current quest: ${ch.countdown} minutes.\r\n${colors.reset}`,
    );
  }
}

/** Show the quest rewards list. */
function doQuestList(ch: CharData, questman: CharData): void {
  act('$n asks $N for a list of quest items.', ch, null, questman, TO_ROOM);
  sendToChar(ch,
    `${colors.white}You ask ${questman.shortDescr || questman.name} for a list of quest items.\r\n`,
  );

  let buf = `\r\n${colors.brightCyan}Current Quest Items available for Purchase:\r\n`;
  buf += '============================================\r\n';

  for (const r of QUEST_REWARDS) {
    buf += `${r.id.padStart(3)}) ${String(r.cost).padStart(5)}qp - ${r.name}\r\n`;
  }
  buf += '============================================\r\n';
  for (const r of QUEST_REWARDS_CURRENCY) {
    buf += `${r.id.toUpperCase().padStart(3)}) ${String(r.cost).padStart(5)}qp - ${r.name}\r\n`;
  }
  buf += `\r\nTo buy an item, type 'QUEST BUY <item>'.${colors.reset}\r\n`;

  sendToChar(ch, buf);
}

/** Buy a quest reward. */
function doQuestBuy(ch: CharData, questman: CharData, argument: string): void {
  if (!argument) {
    sendToChar(ch, 'To buy an item, type \'QUEST BUY <item>\'.\r\n');
    return;
  }

  const choice = argument.toLowerCase();

  // Check currency rewards first (a, b, c, d)
  const currReward = QUEST_REWARDS_CURRENCY.find((r) => r.id === choice);
  if (currReward) {
    if (ch.questpoints < currReward.cost) {
      questmanSays(ch, questman,
        `Sorry, ${ch.name}, but you don't have enough quest points for that.`,
      );
      return;
    }
    ch.questpoints -= currReward.cost;

    switch (currReward.type) {
      case 'gold':
        ch.gold += currReward.amount;
        sendToChar(ch,
          `${colors.white}${questman.shortDescr || questman.name} transfers ${currReward.amount} gold to your account.\r\n${colors.reset}`,
        );
        sendVitals(ch);
        break;
      case 'pracs':
        ch.practice += currReward.amount;
        sendToChar(ch,
          `${colors.white}${questman.shortDescr || questman.name} gives you ${currReward.amount} practices.\r\n${colors.reset}`,
        );
        break;
      case 'learns':
        ch.learn += currReward.amount;
        sendToChar(ch,
          `${colors.white}${questman.shortDescr || questman.name} gives you ${currReward.amount} learns.\r\n${colors.reset}`,
        );
        break;
    }
    return;
  }

  // Check item rewards
  const itemReward = QUEST_REWARDS.find((r) => r.id === choice);
  if (!itemReward) {
    questmanSays(ch, questman, `I don't have that item, ${ch.name}.`);
    return;
  }

  if (ch.questpoints < itemReward.cost) {
    questmanSays(ch, questman,
      `Sorry, ${ch.name}, but you don't have enough quest points for that.`,
    );
    return;
  }

  ch.questpoints -= itemReward.cost;

  // Create a quest reward object
  const obj = world.createSimpleObject({
    name: itemReward.name.toLowerCase(),
    shortDescr: itemReward.name,
    description: `${itemReward.name} lies here.`,
    level: ch.level,
    itemType: ItemType.TREASURE,
    weight: 1,
    cost: itemReward.cost * 10,
    wearFlags: 0,
    extraFlags: 0,
  });

  objToChar(obj, ch);
  sendToChar(ch,
    `${colors.white}${questman.shortDescr || questman.name} gives you ${itemReward.name}.\r\n${colors.reset}`,
  );
  act('$N gives $p to $n.', ch, obj, questman, TO_ROOM);
}

/** Request a new quest. */
function doQuestRequest(ch: CharData, questman: CharData): void {
  act('$n asks $N for a quest.', ch, null, questman, TO_ROOM);
  sendToChar(ch,
    `${colors.white}You ask ${questman.shortDescr || questman.name} for a quest.\r\n${colors.reset}`,
  );

  if (ch.isQuestor) {
    questmanSays(ch, questman, 'But you\'re already on a quest!');
    return;
  }

  if (ch.nextquest > 0) {
    questmanSays(ch, questman,
      `You're very brave, ${ch.name}, but let someone else have a chance.`,
    );
    questmanSays(ch, questman, 'Come back later.');
    return;
  }

  questmanSays(ch, questman, `Thank you, brave ${ch.name}!`);

  ch.questmob = 0;
  ch.questobj = 0;
  ch.questFetchItem = '';

  generateQuest(ch, questman);

  if (ch.questmob > 0 || ch.questobj > 0) {
    ch.isQuestor = true;
    ch.questGiver = questman.name;
    questmanSays(ch, questman,
      `You have ${ch.countdown} minutes to complete this quest.`,
    );
    questmanSays(ch, questman, 'May the gods go with you!');
  }
}

/** Complete a quest at the questmaster. */
function doQuestComplete(ch: CharData, questman: CharData): void {
  act('$n informs $N they have completed their quest.', ch, null, questman, TO_ROOM);
  sendToChar(ch,
    `${colors.white}You inform ${questman.shortDescr || questman.name} you have completed your quest.\r\n${colors.reset}`,
  );

  if (ch.questGiver && ch.questGiver.toLowerCase() !== questman.name.toLowerCase()) {
    questmanSays(ch, questman,
      'I never sent you on a quest! Perhaps you\'re thinking of someone else.',
    );
    return;
  }

  if (!ch.isQuestor) {
    if (ch.nextquest > 0) {
      questmanSays(ch, questman, 'But you didn\'t complete your quest in time!');
    } else {
      questmanSays(ch, questman,
        `You have to REQUEST a quest first, ${ch.name}.`,
      );
    }
    return;
  }

  // Kill quest completed (questmob set to -1 on kill)
  if (ch.questmob === -1 && ch.countdown > 0) {
    completeQuestReward(ch, questman);
    return;
  }

  // Fetch quest -- check if player has the quest object
  if (ch.questobj > 0 && ch.countdown > 0) {
    // Check if player has a quest object in their inventory
    const items = getCarriedItems(ch);
    let foundQuestObj = false;
    for (const obj of items) {
      if (obj.indexVnum === ch.questobj ||
          obj.name.toLowerCase().includes('quest') ||
          (ch.questFetchItem && obj.shortDescr.includes(ch.questFetchItem))) {
        foundQuestObj = true;
        // Remove the quest object
        world.extractObj(obj);
        break;
      }
    }

    if (foundQuestObj) {
      completeQuestReward(ch, questman);
      return;
    } else {
      questmanSays(ch, questman,
        'You haven\'t completed the quest yet, but there is still time!',
      );
      return;
    }
  }

  // Still in progress
  if ((ch.questmob > 0 || ch.questobj > 0) && ch.countdown > 0) {
    questmanSays(ch, questman,
      'You haven\'t completed the quest yet, but there is still time!',
    );
    return;
  }
}

/** Award quest completion rewards. */
function completeQuestReward(ch: CharData, questman: CharData): void {
  const goldReward = rollDice(1, 42500) + 2500; // 2500-45000
  let pointReward = Math.floor(ch.level / 4) + rollDice(1, Math.max(1, Math.floor(ch.level / 4)));
  pointReward = Math.max(pointReward, 15);

  questmanSays(ch, questman, 'Congratulations on completing your quest!');
  questmanSays(ch, questman,
    `As a reward, I am giving you ${pointReward} quest points, and ${goldReward} gold.`,
  );

  // 25% chance of bonus practices
  if (Math.random() < 0.25) {
    const pracReward = rollDice(1, 5);
    sendToChar(ch,
      `${colors.brightGreen}You gain ${pracReward} practices!\r\n${colors.reset}`,
    );
    ch.practice += pracReward;
  }

  ch.isQuestor = false;
  ch.questGiver = '';
  ch.countdown = 0;
  ch.questmob = 0;
  ch.questobj = 0;
  ch.questFetchItem = '';
  ch.nextquest = 10; // cooldown: 10 ticks before next quest
  ch.gold += goldReward;
  ch.questpoints += pointReward;

  sendVitals(ch);
}

/** Refuse / abandon the current quest (costs quest points). */
function doQuestRefuse(ch: CharData, questman: CharData): void {
  act('$n offers to refuse the quest.', ch, null, questman, TO_ROOM);
  sendToChar(ch,
    `${colors.white}You ask ${questman.shortDescr || questman.name} to refuse your quest.\r\n${colors.reset}`,
  );

  if (!ch.isQuestor) {
    questmanSays(ch, questman, 'But you are not on a quest!');
    return;
  }

  let penalty = Math.floor(ch.level / 4) + rollDice(1, Math.max(1, Math.floor(ch.level / 4)));
  penalty = Math.max(penalty, 15);

  if (ch.questpoints < penalty) {
    questmanSays(ch, questman,
      'You do not have enough quest points to refuse a quest.',
    );
    return;
  }

  questmanSays(ch, questman,
    `As a penalty, I am going to take ${penalty} quest points from you.`,
  );

  ch.isQuestor = false;
  ch.questGiver = '';
  ch.countdown = 0;
  ch.questmob = 0;
  ch.questobj = 0;
  ch.questFetchItem = '';
  ch.nextquest = 1;
  ch.questpoints -= penalty;
}

// ============================================================================
//  Quest tick update -- called from the game loop every tick
// ============================================================================

/**
 * questUpdate -- Called every tick from the game loop.
 * Decrements countdowns and cooldowns for all connected players.
 */
export function questUpdate(): void {
  for (const ch of world.characters.values()) {
    if (ch.isNpc || ch.deleted) continue;
    if (!hasConnection(ch.id)) continue;

    // Cooldown between quests
    if (ch.nextquest > 0) {
      ch.nextquest--;
      if (ch.nextquest === 0) {
        sendToChar(ch,
          `${colors.white}You may now quest again.\r\n${colors.reset}`,
        );
      }
      continue;
    }

    // Active quest countdown
    if (ch.isQuestor) {
      ch.countdown--;

      if (ch.countdown <= 0) {
        ch.nextquest = 10;
        sendToChar(ch,
          `${colors.red}You have run out of time for your quest!\r\nYou may quest again in ${ch.nextquest} minutes.\r\n${colors.reset}`,
        );
        ch.isQuestor = false;
        ch.questGiver = '';
        ch.countdown = 0;
        ch.questmob = 0;
        ch.questobj = 0;
        ch.questFetchItem = '';
        continue;
      }

      if (ch.countdown > 0 && ch.countdown < 6) {
        sendToChar(ch,
          `${colors.yellow}Better hurry, you're almost out of time for your quest!\r\n${colors.reset}`,
        );
      }
    }
  }
}

// ============================================================================
//  Quest kill hook -- call from rawKill in fight.ts
// ============================================================================

/**
 * questCheckKill -- Called when a mob is killed.
 * If the killer is on a kill quest targeting this mob, mark it complete.
 */
export function questCheckKill(ch: CharData, victim: CharData): void {
  if (!ch.isQuestor || ch.questmob <= 0) return;

  // Check if the victim matches the quest target
  const victimVnum = victim.vnum || hashCode(victim.id);
  if (victimVnum === ch.questmob) {
    ch.questmob = -1; // Mark as completed
    sendToChar(ch,
      `${colors.brightGreen}You have almost completed your quest! Return to the questmaster before your time runs out!\r\n${colors.reset}`,
    );
  }
}

// ============================================================================
//  Exported command table entries
// ============================================================================

export const questCommands: CommandEntry[] = [
  { name: 'quest', fn: doQuest, minPosition: Position.RESTING, minLevel: 0, log: 0 },
];
