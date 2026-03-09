/**
 * MobileNav.tsx — Mobile bottom navigation bar for Stormgate MUD.
 *
 * Fixed bottom bar shown only on mobile screens (<768px).
 * Provides direction buttons in a compass layout and quick action buttons.
 * All buttons are touch-friendly (min 44px height).
 */

import { useGameState } from '../core/gameState';

export function MobileNav() {
  const { sendCommand } = useGameState();

  const handleCmd = (cmd: string) => {
    if (sendCommand) {
      sendCommand(cmd);
    }
  };

  return (
    <div style={styles.container}>
      {/* Direction compass */}
      <div style={styles.compassSection}>
        <div style={styles.compassGrid}>
          {/* Row 1: U, N, empty */}
          <DirButton label="U" onClick={() => handleCmd('up')} />
          <DirButton label="N" onClick={() => handleCmd('north')} primary />
          <div style={styles.compassSpacer} />

          {/* Row 2: W, center, E */}
          <DirButton label="W" onClick={() => handleCmd('west')} primary />
          <div style={styles.compassCenter}>
            <span style={{ color: '#00d4ff', fontSize: '10px' }}>@</span>
          </div>
          <DirButton label="E" onClick={() => handleCmd('east')} primary />

          {/* Row 3: D, S, empty */}
          <DirButton label="D" onClick={() => handleCmd('down')} />
          <DirButton label="S" onClick={() => handleCmd('south')} primary />
          <div style={styles.compassSpacer} />
        </div>
      </div>

      {/* Quick actions */}
      <div style={styles.actionsSection}>
        <ActionBtn label="Look" onClick={() => handleCmd('look')} />
        <ActionBtn label="Score" onClick={() => handleCmd('score')} />
        <ActionBtn label="Inv" onClick={() => handleCmd('inventory')} />
        <ActionBtn label="Eq" onClick={() => handleCmd('equipment')} />
      </div>
    </div>
  );
}

function DirButton({
  label,
  onClick,
  primary,
}: {
  label: string;
  onClick: () => void;
  primary?: boolean;
}) {
  return (
    <button
      style={{
        ...styles.dirBtn,
        ...(primary ? styles.dirBtnPrimary : styles.dirBtnSecondary),
      }}
      onClick={onClick}
    >
      {label}
    </button>
  );
}

function ActionBtn({
  label,
  onClick,
}: {
  label: string;
  onClick: () => void;
}) {
  return (
    <button style={styles.actionBtn} onClick={onClick}>
      {label}
    </button>
  );
}

const styles: Record<string, React.CSSProperties> = {
  container: {
    display: 'flex',
    alignItems: 'center',
    gap: '8px',
    padding: '6px 8px',
    backgroundColor: '#14141f',
    borderTop: '1px solid #1e1e2e',
    fontFamily: "'Courier New', Courier, monospace",
    flexShrink: 0,
  },
  compassSection: {
    flexShrink: 0,
  },
  compassGrid: {
    display: 'grid',
    gridTemplateColumns: 'repeat(3, 44px)',
    gridTemplateRows: 'repeat(3, 36px)',
    gap: '2px',
  },
  compassCenter: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: '#1e1e2e',
    borderRadius: '4px',
  },
  compassSpacer: {
    // Empty spacer cell
  },
  dirBtn: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    fontSize: '13px',
    fontWeight: 'bold',
    fontFamily: "'Courier New', Courier, monospace",
    border: 'none',
    borderRadius: '4px',
    cursor: 'pointer',
    minHeight: '36px',
    padding: 0,
    WebkitTapHighlightColor: 'transparent',
  },
  dirBtnPrimary: {
    backgroundColor: '#1e2e1e',
    color: '#55ff55',
    border: '1px solid #2a4a2a',
  },
  dirBtnSecondary: {
    backgroundColor: '#1e1e2e',
    color: '#7777ff',
    border: '1px solid #2a2a4a',
  },
  actionsSection: {
    display: 'flex',
    flexDirection: 'column',
    gap: '2px',
    flex: 1,
  },
  actionBtn: {
    padding: '8px 6px',
    fontSize: '11px',
    fontFamily: "'Courier New', Courier, monospace",
    backgroundColor: '#1e1e2e',
    color: '#a0a0b0',
    border: '1px solid #333',
    borderRadius: '4px',
    cursor: 'pointer',
    minHeight: '32px',
    WebkitTapHighlightColor: 'transparent',
  },
};
