/**
 * VitalsPanel.tsx — HP/Mana/Move bar display for Stormgate MUD.
 *
 * Shows three horizontal bars for hit points, mana, and movement.
 * Bars animate on value changes and pulse when critically low.
 */

import { useEffect, useRef, useState } from 'react';
import { useGameState } from '../core/gameState';

interface BarProps {
  label: string;
  current: number;
  max: number;
  color: string;
  lowColor: string;
}

function VitalBar({ label, current, max, color, lowColor }: BarProps) {
  const pct = max > 0 ? Math.max(0, Math.min(100, (current / max) * 100)) : 0;
  const isLow = pct <= 25;
  const isCritical = pct <= 10;
  const [pulse, setPulse] = useState(false);
  const intervalRef = useRef<ReturnType<typeof setInterval> | null>(null);

  useEffect(() => {
    if (isCritical) {
      intervalRef.current = setInterval(() => {
        setPulse((p) => !p);
      }, 500);
    } else {
      if (intervalRef.current) {
        clearInterval(intervalRef.current);
        intervalRef.current = null;
      }
      setPulse(false);
    }
    return () => {
      if (intervalRef.current) clearInterval(intervalRef.current);
    };
  }, [isCritical]);

  const barColor = isLow ? lowColor : color;
  const opacity = pulse ? 0.5 : 1;

  return (
    <div style={barStyles.container}>
      <div style={barStyles.labelRow}>
        <span style={{ ...barStyles.label, color: barColor }}>{label}</span>
        <span style={barStyles.values}>
          {current}/{max}
        </span>
      </div>
      <div style={barStyles.track}>
        <div
          style={{
            ...barStyles.fill,
            width: `${pct}%`,
            backgroundColor: barColor,
            opacity,
            transition: 'width 0.3s ease-out, opacity 0.3s ease',
          }}
        />
        <span style={barStyles.pctText}>{Math.round(pct)}%</span>
      </div>
    </div>
  );
}

export function VitalsPanel() {
  const { state } = useGameState();
  const { vitals } = state;

  if (!vitals) {
    return (
      <div style={styles.container}>
        <div style={styles.header}>Vitals</div>
        <div style={styles.empty}>No data</div>
      </div>
    );
  }

  return (
    <div style={styles.container}>
      <div style={styles.header}>Vitals</div>
      <VitalBar
        label="HP"
        current={vitals.hp}
        max={vitals.maxHp}
        color="#ff5555"
        lowColor="#991111"
      />
      <VitalBar
        label="MN"
        current={vitals.mana}
        max={vitals.maxMana}
        color="#5555ff"
        lowColor="#111199"
      />
      <VitalBar
        label="MV"
        current={vitals.move}
        max={vitals.maxMove}
        color="#55ff55"
        lowColor="#119911"
      />
      <div style={styles.statsRow}>
        <span style={styles.stat}>
          <span style={{ color: '#cccc00' }}>Gold:</span> {vitals.gold}
        </span>
        <span style={styles.stat}>
          <span style={{ color: '#a0a0b0' }}>Lv:</span> {vitals.level}
        </span>
      </div>
    </div>
  );
}

const barStyles: Record<string, React.CSSProperties> = {
  container: {
    marginBottom: '8px',
  },
  labelRow: {
    display: 'flex',
    justifyContent: 'space-between',
    marginBottom: '2px',
  },
  label: {
    fontSize: '11px',
    fontWeight: 'bold',
    fontFamily: "'Courier New', Courier, monospace",
  },
  values: {
    fontSize: '11px',
    color: '#a0a0b0',
    fontFamily: "'Courier New', Courier, monospace",
  },
  track: {
    position: 'relative',
    height: '14px',
    backgroundColor: '#1e1e2e',
    borderRadius: '3px',
    overflow: 'hidden',
  },
  fill: {
    position: 'absolute',
    top: 0,
    left: 0,
    height: '100%',
    borderRadius: '3px',
  },
  pctText: {
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
    textAlign: 'center',
    fontSize: '10px',
    lineHeight: '14px',
    color: '#e0e0e0',
    fontFamily: "'Courier New', Courier, monospace",
    textShadow: '0 0 2px #000',
    zIndex: 1,
  },
};

const styles: Record<string, React.CSSProperties> = {
  container: {
    padding: '8px',
    borderBottom: '1px solid #1e1e2e',
  },
  header: {
    fontSize: '12px',
    fontWeight: 'bold',
    color: '#00d4ff',
    marginBottom: '8px',
    fontFamily: "'Courier New', Courier, monospace",
    textTransform: 'uppercase',
    letterSpacing: '1px',
  },
  empty: {
    color: '#555',
    fontSize: '11px',
    fontStyle: 'italic',
    fontFamily: "'Courier New', Courier, monospace",
  },
  statsRow: {
    display: 'flex',
    justifyContent: 'space-between',
    marginTop: '6px',
  },
  stat: {
    fontSize: '11px',
    color: '#e0e0e0',
    fontFamily: "'Courier New', Courier, monospace",
  },
};
