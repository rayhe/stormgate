import { useEffect, useRef, useState, useCallback } from "react";
import type { GameConnection, ConnectionState } from "../core/connection";
import { connect } from "../core/connection";

// ANSI color map matching Stormgate's colors.h
// These are the standard 16-color ANSI codes the MUD server sends
const ANSI_COLORS: Record<number, string> = {
  30: "#2e2e2e", // black
  31: "#cc0000", // red
  32: "#00cc00", // green
  33: "#cccc00", // yellow
  34: "#5555ff", // blue
  35: "#cc00cc", // magenta
  36: "#00cccc", // cyan
  37: "#cccccc", // white (normal)
  90: "#666666", // bright black (dark gray)
  91: "#ff5555", // bright red
  92: "#55ff55", // bright green
  93: "#ffff55", // bright yellow
  94: "#7777ff", // bright blue
  95: "#ff55ff", // bright magenta
  96: "#55ffff", // bright cyan
  97: "#ffffff", // bright white
};

interface AnsiSpan {
  text: string;
  color: string;
  bold: boolean;
}

function parseAnsi(raw: string): AnsiSpan[] {
  const spans: AnsiSpan[] = [];
  let color = "#cccccc";
  let bold = false;
  const parts = raw.split(/\x1b\[/);

  for (let i = 0; i < parts.length; i++) {
    const part = parts[i];
    if (i === 0) {
      if (part) spans.push({ text: part, color, bold });
      continue;
    }
    const mIdx = part.indexOf("m");
    if (mIdx === -1) {
      spans.push({ text: part, color, bold });
      continue;
    }
    const codes = part.slice(0, mIdx).split(";").map(Number);
    const text = part.slice(mIdx + 1);

    for (const code of codes) {
      if (code === 0) {
        color = "#cccccc";
        bold = false;
      } else if (code === 1) {
        bold = true;
      } else if (ANSI_COLORS[code]) {
        color = ANSI_COLORS[code];
      }
    }
    if (text) spans.push({ text, color, bold });
  }
  return spans;
}

interface TerminalLine {
  id: number;
  spans: AnsiSpan[];
}

let lineCounter = 0;

export function Terminal() {
  const [lines, setLines] = useState<TerminalLine[]>([]);
  const [input, setInput] = useState("");
  const [history, setHistory] = useState<string[]>([]);
  const [historyIndex, setHistoryIndex] = useState(-1);
  const [connState, setConnState] = useState<ConnectionState>("disconnected");
  const connRef = useRef<GameConnection | null>(null);
  const scrollRef = useRef<HTMLDivElement>(null);
  const inputRef = useRef<HTMLInputElement>(null);
  const maxLines = 500;

  useEffect(() => {
    let mounted = true;

    connect()
      .then((conn) => {
        if (!mounted) {
          conn.disconnect();
          return;
        }
        connRef.current = conn;

        conn.onStateChange((state) => {
          if (mounted) setConnState(state);
        });

        conn.onData((data) => {
          if (!mounted) return;
          const incoming = data.split("\n").map((line) => ({
            id: lineCounter++,
            spans: parseAnsi(line),
          }));
          setLines((prev) => {
            const next = [...prev, ...incoming];
            return next.length > maxLines ? next.slice(-maxLines) : next;
          });
        });
      })
      .catch((err) => {
        console.error("Connection failed:", err);
      });

    return () => {
      mounted = false;
      connRef.current?.disconnect();
    };
  }, []);

  // Auto-scroll to bottom on new output
  useEffect(() => {
    if (scrollRef.current) {
      scrollRef.current.scrollTop = scrollRef.current.scrollHeight;
    }
  }, [lines]);

  const sendCommand = useCallback(
    (cmd: string) => {
      if (!connRef.current) return;
      connRef.current.send(cmd + "\n");
      setLines((prev) => [
        ...prev,
        {
          id: lineCounter++,
          spans: [{ text: cmd, color: "#55ffff", bold: false }],
        },
      ]);
      if (cmd.trim()) {
        setHistory((prev) => [...prev, cmd]);
      }
      setHistoryIndex(-1);
      setInput("");
    },
    []
  );

  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === "Enter") {
      sendCommand(input);
    } else if (e.key === "ArrowUp") {
      e.preventDefault();
      if (history.length === 0) return;
      const newIdx =
        historyIndex === -1 ? history.length - 1 : Math.max(0, historyIndex - 1);
      setHistoryIndex(newIdx);
      setInput(history[newIdx]);
    } else if (e.key === "ArrowDown") {
      e.preventDefault();
      if (historyIndex === -1) return;
      const newIdx = historyIndex + 1;
      if (newIdx >= history.length) {
        setHistoryIndex(-1);
        setInput("");
      } else {
        setHistoryIndex(newIdx);
        setInput(history[newIdx]);
      }
    }
  };

  // Focus input when clicking anywhere on the terminal
  const handleContainerClick = () => {
    inputRef.current?.focus();
  };

  const stateColor =
    connState === "connected"
      ? "#00cc00"
      : connState === "connecting"
        ? "#cccc00"
        : "#cc0000";

  return (
    <div style={styles.container} onClick={handleContainerClick}>
      <div style={styles.statusBar}>
        <span style={{ color: stateColor }}>
          {connState === "connected"
            ? "Connected"
            : connState === "connecting"
              ? "Connecting..."
              : "Disconnected"}
        </span>
      </div>
      <div ref={scrollRef} style={styles.output}>
        {lines.map((line) => (
          <div key={line.id} style={styles.line}>
            {line.spans.length === 0 ? (
              <span>&nbsp;</span>
            ) : (
              line.spans.map((span, i) => (
                <span
                  key={i}
                  style={{
                    color: span.color,
                    fontWeight: span.bold ? "bold" : "normal",
                  }}
                >
                  {span.text}
                </span>
              ))
            )}
          </div>
        ))}
      </div>
      <div style={styles.inputRow}>
        <span style={styles.prompt}>&gt;</span>
        <input
          ref={inputRef}
          type="text"
          value={input}
          onChange={(e) => setInput(e.target.value)}
          onKeyDown={handleKeyDown}
          style={styles.input}
          autoFocus
          spellCheck={false}
        />
      </div>
    </div>
  );
}

const styles: Record<string, React.CSSProperties> = {
  container: {
    display: "flex",
    flexDirection: "column",
    height: "100vh",
    backgroundColor: "#0a0a0f",
    fontFamily: "'Courier New', Courier, monospace",
    fontSize: "14px",
  },
  statusBar: {
    padding: "4px 12px",
    backgroundColor: "#14141f",
    borderBottom: "1px solid #1e1e2e",
    fontSize: "12px",
    flexShrink: 0,
  },
  output: {
    flex: 1,
    overflowY: "auto",
    padding: "8px 12px",
    whiteSpace: "pre-wrap",
    wordBreak: "break-word",
  },
  line: {
    minHeight: "1.3em",
    lineHeight: "1.3",
  },
  inputRow: {
    display: "flex",
    alignItems: "center",
    padding: "8px 12px",
    borderTop: "1px solid #1e1e2e",
    backgroundColor: "#14141f",
    flexShrink: 0,
  },
  prompt: {
    color: "#00d4ff",
    marginRight: "8px",
    fontWeight: "bold",
  },
  input: {
    flex: 1,
    backgroundColor: "transparent",
    border: "none",
    outline: "none",
    color: "#e0e0e0",
    fontFamily: "'Courier New', Courier, monospace",
    fontSize: "14px",
  },
};
