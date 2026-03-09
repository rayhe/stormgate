/**
 * index.ts — Engine barrel export for Stormgate MUD.
 *
 * Re-exports all public symbols from the engine modules so consumers
 * can import from a single entry point:
 *
 *   import { world, interpret, sendToChar, startGameLoop } from './engine/index.js';
 */

export { world, World, charRoomMap, mobTemplateVnumMap } from './world.js';
export * from './types.js';
export * from './handler.js';
export * from './output.js';
export * from './commands.js';
export * from './fight.js';
export * from './shops.js';
export { startGameLoop, stopGameLoop } from './update.js';
export * from './protocol.js';
export * from './resets.js';
export {
  initSaveSystem,
  saveCharacter,
  loadCharacter,
  listCharacters,
  startAutoSave,
  stopAutoSave,
  saveAllCharacters,
  charOwnerMap,
} from './save.js';
export {
  initAdminSystem,
  setCharOwnerMap,
  adminCommands,
} from './admin.js';
export * from './skills.js';
export * from './magic.js';
export { registerAllSpells, savesSpell } from './spells.js';
export * from './social.js';
export { questCommands, questUpdate, questCheckKill } from './quest.js';
export { craftingCommands, craftUpdate } from './crafting.js';
