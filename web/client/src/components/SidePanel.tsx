/**
 * SidePanel.tsx — Container panel for vitals, room info, and quick actions.
 *
 * Wraps VitalsPanel, RoomPanel, and quick action buttons in a scrollable
 * sidebar. On mobile, this is rendered as a toggleable overlay.
 */

import { VitalsPanel } from './VitalsPanel';
import { RoomPanel } from './RoomPanel';
import { useGameState } from '../core/gameState';

interface SidePanelProps {
  onClose?: () => void;
  isMobileOverlay?: boolean;
}

export function SidePanel({ onClose, isMobileOverlay }: SidePanelProps) {
  const { sendCommand } = useGameState();

  const handleAction = (cmd: string) => {
    if (sendCommand) {
      sendCommand(cmd);
    }
  };

  return (
    <div
      style={{
        ...styles.container,
        ...(isMobileOverlay ? styles.mobileOverlay : {}),
      }}
    >
      {isMobileOverlay && (
        <div style={styles.mobileHeader}>
          <span style={styles.mobileTitle}>Game Info</span>
          <button style={styles.closeBtn} onClick={onClose}>
            X
          </button>
        </div>
      )}

      <div style={styles.scrollArea}>
        <VitalsPanel />
        <RoomPanel />

        {/* Quick Actions */}
        <div style={styles.actionsSection}>
          <div style={styles.actionsHeader}>Quick Actions</div>
          <div style={styles.actionsGrid}>
            <ActionButton label="Look" onClick={() => handleAction('look')} />
            <ActionButton label="Score" onClick={() => handleAction('score')} />
            <ActionButton label="Inv" onClick={() => handleAction('inventory')} />
            <ActionButton label="Eq" onClick={() => handleAction('equipment')} />
            <ActionButton label="Who" onClick={() => handleAction('who')} />
            <ActionButton label="Exits" onClick={() => handleAction('exits')} />
          </div>
        </div>
      </div>
    </div>
  );
}

function ActionButton({ label, onClick }: { label: string; onClick: () => void }) {
  return (
    <button
      style={styles.actionBtn}
      onClick={onClick}
      onMouseEnter={(e) => {
        (e.target as HTMLButtonElement).style.backgroundColor = '#2a2a3e';
        (e.target as HTMLButtonElement).style.borderColor = '#00d4ff';
      }}
      onMouseLeave={(e) => {
        (e.target as HTMLButtonElement).style.backgroundColor = '#14141f';
        (e.target as HTMLButtonElement).style.borderColor = '#333';
      }}
    >
      {label}
    </button>
  );
}

const styles: Record<string, React.CSSProperties> = {
  container: {
    display: 'flex',
    flexDirection: 'column',
    width: '220px',
    backgroundColor: '#0e0e18',
    borderLeft: '1px solid #1e1e2e',
    fontFamily: "'Courier New', Courier, monospace",
    flexShrink: 0,
    overflow: 'hidden',
  },
  mobileOverlay: {
    position: 'fixed',
    top: 0,
    right: 0,
    bottom: 0,
    width: '280px',
    zIndex: 100,
    boxShadow: '-4px 0 20px rgba(0,0,0,0.8)',
  },
  mobileHeader: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'space-between',
    padding: '8px 12px',
    borderBottom: '1px solid #1e1e2e',
    backgroundColor: '#14141f',
    flexShrink: 0,
  },
  mobileTitle: {
    color: '#00d4ff',
    fontSize: '14px',
    fontWeight: 'bold',
  },
  closeBtn: {
    background: 'transparent',
    border: '1px solid #555',
    color: '#a0a0b0',
    fontSize: '12px',
    fontFamily: "'Courier New', Courier, monospace",
    cursor: 'pointer',
    padding: '4px 8px',
    borderRadius: '3px',
  },
  scrollArea: {
    flex: 1,
    overflowY: 'auto',
    overflowX: 'hidden',
  },
  actionsSection: {
    padding: '8px',
  },
  actionsHeader: {
    fontSize: '12px',
    fontWeight: 'bold',
    color: '#00d4ff',
    marginBottom: '8px',
    textTransform: 'uppercase',
    letterSpacing: '1px',
  },
  actionsGrid: {
    display: 'grid',
    gridTemplateColumns: 'repeat(2, 1fr)',
    gap: '4px',
  },
  actionBtn: {
    padding: '6px 4px',
    fontSize: '11px',
    fontFamily: "'Courier New', Courier, monospace",
    backgroundColor: '#14141f',
    color: '#a0a0b0',
    border: '1px solid #333',
    borderRadius: '3px',
    cursor: 'pointer',
    transition: 'background-color 0.15s, border-color 0.15s',
    minHeight: '32px',
  },
};
