/**
 * gameState.tsx — Reactive game state store for Stormgate MUD.
 *
 * Uses React context to provide structured game state to all UI components.
 * The Terminal component updates this state when structured JSON messages
 * arrive from the server. Side panels (vitals, room, combat) read from it.
 */

import { createContext, useContext, useState, useCallback, type ReactNode } from 'react';
import type {
  VitalsData,
  RoomData,
  CombatData,
  ChannelMessage,
  CharInfoData,
  WhoEntry,
  GameMessage,
} from './protocol';

// ============================================================================
//  Map state for tracking visited rooms
// ============================================================================

export interface MapRoom {
  vnum: number;
  name: string;
  exits: string[];
}

// ============================================================================
//  Game state shape
// ============================================================================

export interface GameState {
  vitals: VitalsData | null;
  room: RoomData | null;
  combatLog: CombatData[];
  activeCombat: CombatData | null;
  channels: ChannelMessage[];
  charInfo: CharInfoData | null;
  who: WhoEntry[];
  visitedRooms: Map<number, MapRoom>;
}

export interface GameStateContextValue {
  state: GameState;
  dispatch: (msg: GameMessage) => void;
  sendCommand: ((cmd: string) => void) | null;
  setSendCommand: (fn: (cmd: string) => void) => void;
}

// ============================================================================
//  Context
// ============================================================================

const GameStateContext = createContext<GameStateContextValue | null>(null);

const MAX_COMBAT_LOG = 50;
const MAX_CHANNEL_LOG = 200;

// ============================================================================
//  Provider
// ============================================================================

export function GameStateProvider({ children }: { children: ReactNode }) {
  const [state, setState] = useState<GameState>({
    vitals: null,
    room: null,
    combatLog: [],
    activeCombat: null,
    channels: [],
    charInfo: null,
    who: [],
    visitedRooms: new Map(),
  });

  // Use state for sendCommand so components re-render when it's set
  const [sendCommandFn, setSendCommandFn] = useState<((cmd: string) => void) | null>(null);

  const setSendCommand = useCallback((fn: (cmd: string) => void) => {
    // Wrap in a function to avoid React treating `fn` as a state updater
    setSendCommandFn(() => fn);
  }, []);

  const dispatch = useCallback((msg: GameMessage) => {
    setState((prev) => {
      switch (msg.type) {
        case 'vitals': {
          const vitals = msg.data as VitalsData;
          return { ...prev, vitals };
        }

        case 'room': {
          const room = msg.data as RoomData;
          // Track visited rooms for the mini-map
          const visitedRooms = new Map(prev.visitedRooms);
          visitedRooms.set(room.vnum, {
            vnum: room.vnum,
            name: room.name,
            exits: room.exits,
          });
          return { ...prev, room, visitedRooms };
        }

        case 'combat': {
          const combat = msg.data as CombatData;
          const combatLog = [...prev.combatLog, combat];
          if (combatLog.length > MAX_COMBAT_LOG) {
            combatLog.splice(0, combatLog.length - MAX_COMBAT_LOG);
          }
          return { ...prev, combatLog, activeCombat: combat };
        }

        case 'channel': {
          const channel = msg.data as ChannelMessage;
          const channels = [...prev.channels, channel];
          if (channels.length > MAX_CHANNEL_LOG) {
            channels.splice(0, channels.length - MAX_CHANNEL_LOG);
          }
          return { ...prev, channels };
        }

        case 'charInfo': {
          const charInfo = msg.data as CharInfoData;
          return { ...prev, charInfo };
        }

        case 'who': {
          const who = msg.data as WhoEntry[];
          return { ...prev, who };
        }

        default:
          return prev;
      }
    });
  }, []);

  const value: GameStateContextValue = {
    state,
    dispatch,
    sendCommand: sendCommandFn,
    setSendCommand,
  };

  return (
    <GameStateContext.Provider value={value}>
      {children}
    </GameStateContext.Provider>
  );
}

// ============================================================================
//  Hook
// ============================================================================

export function useGameState(): GameStateContextValue {
  const ctx = useContext(GameStateContext);
  if (!ctx) {
    throw new Error('useGameState must be used within a GameStateProvider');
  }
  return ctx;
}

/**
 * Convenience hook: returns a send function that dispatches a command
 * through the WebSocket connection.
 */
export function useSendCommand(): (cmd: string) => void {
  const { sendCommand } = useGameState();
  return useCallback(
    (cmd: string) => {
      if (sendCommand) {
        sendCommand(cmd);
      }
    },
    [sendCommand],
  );
}
