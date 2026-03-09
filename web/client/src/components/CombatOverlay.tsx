/**
 * CombatOverlay.tsx — Combat status overlay for Stormgate MUD.
 *
 * Shows at the top of the terminal when the player is in combat.
 * Displays the enemy name, enemy HP bar, and player HP bar.
 * Flashes red on big hits and auto-hides when combat ends.
 */

import { useEffect, useState, useRef } from 'react';
import { useGameState } from '../core/gameState';

export function CombatOverlay() {
  const { state } = useGameState();
  const { activeCombat, vitals } = state;
  const [visible, setVisible] = useState(false);
  const [flashRed, setFlashRed] = useState(false);
  const hideTimerRef = useRef<ReturnType<typeof setTimeout> | null>(null);
  const prevCombatRef = useRef(activeCombat);

  useEffect(() => {
    if (activeCombat) {
      setVisible(true);

      // Clear any pending hide timer
      if (hideTimerRef.current) {
        clearTimeout(hideTimerRef.current);
        hideTimerRef.current = null;
      }

      // Flash red on big hits (damage > 15% of max HP)
      if (
        prevCombatRef.current !== activeCombat &&
        activeCombat.damage > 0 &&
        vitals &&
        activeCombat.damage > vitals.maxHp * 0.15
      ) {
        setFlashRed(true);
        setTimeout(() => setFlashRed(false), 300);
      }

      prevCombatRef.current = activeCombat;

      // Auto-hide after 5 seconds of no combat updates
      hideTimerRef.current = setTimeout(() => {
        setVisible(false);
      }, 5000);
    }

    return () => {
      if (hideTimerRef.current) {
        clearTimeout(hideTimerRef.current);
      }
    };
  }, [activeCombat, vitals]);

  if (!visible || !activeCombat) return null;

  return (
    <div
      style={{
        ...styles.container,
        backgroundColor: flashRed ? '#3a0a0a' : '#14141f',
        transition: 'background-color 0.2s',
      }}
    >
      <div style={styles.row}>
        {/* Enemy info */}
        <div style={styles.combatant}>
          <span style={styles.enemyName}>{activeCombat.target}</span>
          <div style={styles.barTrack}>
            <div
              style={{
                ...styles.barFill,
                width: `${Math.max(0, Math.min(100, activeCombat.targetHpPct))}%`,
                backgroundColor: getHpColor(activeCombat.targetHpPct),
                transition: 'width 0.3s ease-out',
              }}
            />
            <span style={styles.barLabel}>
              {Math.round(activeCombat.targetHpPct)}%
            </span>
          </div>
        </div>

        <span style={styles.vs}>VS</span>

        {/* Player info */}
        <div style={styles.combatant}>
          <span style={styles.playerName}>{activeCombat.attacker}</span>
          <div style={styles.barTrack}>
            <div
              style={{
                ...styles.barFill,
                width: `${Math.max(0, Math.min(100, activeCombat.attackerHpPct))}%`,
                backgroundColor: getHpColor(activeCombat.attackerHpPct),
                transition: 'width 0.3s ease-out',
              }}
            />
            <span style={styles.barLabel}>
              {Math.round(activeCombat.attackerHpPct)}%
            </span>
          </div>
        </div>
      </div>

      {/* Last hit message */}
      {activeCombat.message && (
        <div style={styles.message}>
          {activeCombat.message} ({activeCombat.damage} dmg)
        </div>
      )}
    </div>
  );
}

function getHpColor(pct: number): string {
  if (pct > 75) return '#55ff55';
  if (pct > 50) return '#cccc00';
  if (pct > 25) return '#ff8800';
  return '#ff5555';
}

const styles: Record<string, React.CSSProperties> = {
  container: {
    padding: '6px 12px',
    borderBottom: '1px solid #ff5555',
    fontFamily: "'Courier New', Courier, monospace",
    flexShrink: 0,
  },
  row: {
    display: 'flex',
    alignItems: 'center',
    gap: '8px',
  },
  combatant: {
    flex: 1,
    minWidth: 0,
  },
  enemyName: {
    color: '#ff5555',
    fontSize: '11px',
    fontWeight: 'bold',
    display: 'block',
    marginBottom: '2px',
    whiteSpace: 'nowrap',
    overflow: 'hidden',
    textOverflow: 'ellipsis',
  },
  playerName: {
    color: '#55ffff',
    fontSize: '11px',
    fontWeight: 'bold',
    display: 'block',
    marginBottom: '2px',
    whiteSpace: 'nowrap',
    overflow: 'hidden',
    textOverflow: 'ellipsis',
  },
  vs: {
    color: '#555',
    fontSize: '10px',
    fontWeight: 'bold',
    flexShrink: 0,
  },
  barTrack: {
    position: 'relative',
    height: '10px',
    backgroundColor: '#1e1e2e',
    borderRadius: '2px',
    overflow: 'hidden',
  },
  barFill: {
    position: 'absolute',
    top: 0,
    left: 0,
    height: '100%',
    borderRadius: '2px',
  },
  barLabel: {
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
    textAlign: 'center',
    fontSize: '8px',
    lineHeight: '10px',
    color: '#e0e0e0',
    textShadow: '0 0 2px #000',
    zIndex: 1,
  },
  message: {
    color: '#888',
    fontSize: '10px',
    marginTop: '3px',
    textAlign: 'center',
    fontStyle: 'italic',
  },
};
