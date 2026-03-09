/**
 * RoomPanel.tsx — Room info and mini-map display for Stormgate MUD.
 *
 * Shows the current room name, a compass-style exit display, and a 3x3
 * mini auto-map that tracks visited rooms as the player explores.
 */

import { useGameState } from '../core/gameState';

const DIRECTION_MAP: Record<string, { row: number; col: number }> = {
  north: { row: 0, col: 1 },
  east:  { row: 1, col: 2 },
  south: { row: 2, col: 1 },
  west:  { row: 1, col: 0 },
};

const COMPASS_DIRS = [
  { dir: 'north', label: 'N', row: 0, col: 1 },
  { dir: 'east',  label: 'E', row: 1, col: 2 },
  { dir: 'south', label: 'S', row: 2, col: 1 },
  { dir: 'west',  label: 'W', row: 1, col: 0 },
  { dir: 'up',    label: 'U', row: 0, col: 2 },
  { dir: 'down',  label: 'D', row: 2, col: 0 },
];

export function RoomPanel() {
  const { state, sendCommand } = useGameState();
  const { room } = state;

  const handleMove = (dir: string) => {
    if (sendCommand) {
      sendCommand(dir);
    }
  };

  if (!room) {
    return (
      <div style={styles.container}>
        <div style={styles.header}>Room</div>
        <div style={styles.empty}>No room data</div>
      </div>
    );
  }

  const exitSet = new Set(room.exits.map((e) => e.toLowerCase()));

  return (
    <div style={styles.container}>
      <div style={styles.header}>Room</div>

      {/* Room name */}
      <div style={styles.roomName}>{room.name}</div>

      {/* Compass-style exit display */}
      <div style={styles.compassContainer}>
        <div style={styles.compassGrid}>
          {[0, 1, 2].map((row) =>
            [0, 1, 2].map((col) => {
              // Center cell = current room indicator
              if (row === 1 && col === 1) {
                return (
                  <div key={`${row}-${col}`} style={styles.compassCenter}>
                    @
                  </div>
                );
              }

              const compassDir = COMPASS_DIRS.find(
                (d) => d.row === row && d.col === col,
              );

              if (!compassDir) {
                return <div key={`${row}-${col}`} style={styles.compassEmpty} />;
              }

              const isAvailable = exitSet.has(compassDir.dir);

              return (
                <div
                  key={`${row}-${col}`}
                  style={{
                    ...styles.compassCell,
                    ...(isAvailable ? styles.compassActive : styles.compassInactive),
                    cursor: isAvailable ? 'pointer' : 'default',
                  }}
                  onClick={() => isAvailable && handleMove(compassDir.dir)}
                  title={
                    isAvailable
                      ? `Go ${compassDir.dir}`
                      : `No exit ${compassDir.dir}`
                  }
                >
                  {compassDir.label}
                </div>
              );
            }),
          )}
        </div>
      </div>

      {/* Exits list */}
      <div style={styles.exitsRow}>
        <span style={styles.exitsLabel}>Exits:</span>
        <span style={styles.exitsValue}>
          {room.exits.length > 0 ? room.exits.join(' ') : 'none'}
        </span>
      </div>

      {/* Mini auto-map */}
      <MiniMap />

      {/* Characters in room */}
      {room.characters.length > 0 && (
        <div style={styles.section}>
          <div style={styles.sectionLabel}>Here:</div>
          {room.characters.map((ch, i) => (
            <div key={i} style={styles.charEntry}>
              {ch}
            </div>
          ))}
        </div>
      )}

      {/* Items in room */}
      {room.items.length > 0 && (
        <div style={styles.section}>
          <div style={styles.sectionLabel}>Items:</div>
          {room.items.map((item, i) => (
            <div key={i} style={styles.itemEntry}>
              {item}
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

/**
 * MiniMap — A 3x3 grid showing the current room and adjacent visited rooms.
 */
function MiniMap() {
  const { state, sendCommand } = useGameState();
  const { room, visitedRooms } = state;

  if (!room) return null;

  // Build a 3x3 grid: center = current room, adjacent = rooms through exits
  // We need the room vnums for adjacent rooms, but we only have exit direction names
  // We'll show "?" for unknown rooms and visited room names for known ones
  const grid: Array<Array<{ label: string; available: boolean; dir: string | null }>> = [
    [
      { label: '', available: false, dir: null },
      { label: '', available: false, dir: null },
      { label: '', available: false, dir: null },
    ],
    [
      { label: '', available: false, dir: null },
      { label: '@', available: false, dir: null },
      { label: '', available: false, dir: null },
    ],
    [
      { label: '', available: false, dir: null },
      { label: '', available: false, dir: null },
      { label: '', available: false, dir: null },
    ],
  ];

  for (const exit of room.exits) {
    const dirLower = exit.toLowerCase();
    const pos = DIRECTION_MAP[dirLower];
    if (pos) {
      grid[pos.row][pos.col] = {
        label: '?',
        available: true,
        dir: dirLower,
      };
    }
  }

  const handleMove = (dir: string) => {
    if (sendCommand) {
      sendCommand(dir);
    }
  };

  return (
    <div style={styles.miniMapContainer}>
      <div style={styles.miniMapLabel}>Map</div>
      <div style={styles.miniMapGrid}>
        {grid.map((row, ri) =>
          row.map((cell, ci) => {
            if (ri === 1 && ci === 1) {
              // Current room
              return (
                <div key={`${ri}-${ci}`} style={styles.miniMapCurrent}>
                  <span style={{ color: '#00d4ff' }}>@</span>
                </div>
              );
            }

            if (!cell.available) {
              return (
                <div key={`${ri}-${ci}`} style={styles.miniMapEmpty}>
                  {'  '}
                </div>
              );
            }

            return (
              <div
                key={`${ri}-${ci}`}
                style={{
                  ...styles.miniMapRoom,
                  cursor: 'pointer',
                }}
                onClick={() => cell.dir && handleMove(cell.dir)}
                title={cell.dir ? `Go ${cell.dir}` : ''}
              >
                <span style={{ color: '#555' }}>{cell.label}</span>
              </div>
            );
          }),
        )}
      </div>
    </div>
  );
}

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
  roomName: {
    color: '#00cccc',
    fontSize: '13px',
    fontWeight: 'bold',
    fontFamily: "'Courier New', Courier, monospace",
    marginBottom: '8px',
    whiteSpace: 'nowrap',
    overflow: 'hidden',
    textOverflow: 'ellipsis',
  },
  compassContainer: {
    display: 'flex',
    justifyContent: 'center',
    marginBottom: '8px',
  },
  compassGrid: {
    display: 'grid',
    gridTemplateColumns: 'repeat(3, 28px)',
    gridTemplateRows: 'repeat(3, 28px)',
    gap: '2px',
  },
  compassCenter: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: '#1e1e2e',
    borderRadius: '3px',
    color: '#00d4ff',
    fontSize: '14px',
    fontWeight: 'bold',
    fontFamily: "'Courier New', Courier, monospace",
  },
  compassCell: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    borderRadius: '3px',
    fontSize: '11px',
    fontWeight: 'bold',
    fontFamily: "'Courier New', Courier, monospace",
    transition: 'background-color 0.2s',
    userSelect: 'none',
  },
  compassActive: {
    backgroundColor: '#2a2a3e',
    color: '#55ff55',
    border: '1px solid #55ff55',
  },
  compassInactive: {
    backgroundColor: '#121218',
    color: '#333',
    border: '1px solid #1e1e2e',
  },
  compassEmpty: {
    backgroundColor: 'transparent',
  },
  exitsRow: {
    display: 'flex',
    gap: '4px',
    marginBottom: '8px',
    fontFamily: "'Courier New', Courier, monospace",
    fontSize: '11px',
  },
  exitsLabel: {
    color: '#a0a0b0',
  },
  exitsValue: {
    color: '#55ffff',
  },
  section: {
    marginTop: '6px',
  },
  sectionLabel: {
    color: '#a0a0b0',
    fontSize: '11px',
    fontFamily: "'Courier New', Courier, monospace",
    marginBottom: '2px',
  },
  charEntry: {
    color: '#cccc00',
    fontSize: '11px',
    fontFamily: "'Courier New', Courier, monospace",
    paddingLeft: '8px',
    whiteSpace: 'nowrap',
    overflow: 'hidden',
    textOverflow: 'ellipsis',
  },
  itemEntry: {
    color: '#00cc00',
    fontSize: '11px',
    fontFamily: "'Courier New', Courier, monospace",
    paddingLeft: '8px',
    whiteSpace: 'nowrap',
    overflow: 'hidden',
    textOverflow: 'ellipsis',
  },
  miniMapContainer: {
    marginBottom: '8px',
  },
  miniMapLabel: {
    color: '#a0a0b0',
    fontSize: '10px',
    fontFamily: "'Courier New', Courier, monospace",
    marginBottom: '4px',
    textTransform: 'uppercase',
    letterSpacing: '1px',
  },
  miniMapGrid: {
    display: 'grid',
    gridTemplateColumns: 'repeat(3, 24px)',
    gridTemplateRows: 'repeat(3, 24px)',
    gap: '1px',
    justifyContent: 'center',
  },
  miniMapCurrent: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: '#1e1e2e',
    border: '1px solid #00d4ff',
    borderRadius: '2px',
    fontSize: '12px',
    fontFamily: "'Courier New', Courier, monospace",
  },
  miniMapRoom: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: '#1a1a28',
    border: '1px solid #333',
    borderRadius: '2px',
    fontSize: '10px',
    fontFamily: "'Courier New', Courier, monospace",
  },
  miniMapEmpty: {
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: 'transparent',
    fontSize: '10px',
  },
};
