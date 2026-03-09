/**
 * index.ts — Engine barrel export for Stormgate MUD.
 *
 * Re-exports all public symbols from the engine modules so consumers
 * can import from a single entry point:
 *
 *   import { world, interpret, sendToChar, startGameLoop } from './engine/index.js';
 */

export { world, World, charRoomMap } from './world.js';
export * from './types.js';
export * from './handler.js';
export * from './output.js';
export * from './commands.js';
export { startGameLoop, stopGameLoop } from './update.js';
