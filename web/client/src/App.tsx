import { useState, useEffect, useCallback } from "react";
import { onAuthChanged, signOut } from "./core/auth";
import { LoginScreen } from "./components/LoginScreen";
import { Terminal } from "./components/Terminal";
import { SidePanel } from "./components/SidePanel";
import { MobileNav } from "./components/MobileNav";
import { GameStateProvider } from "./core/gameState";
import type { User } from "firebase/auth";

export default function App() {
  const [user, setUser] = useState<User | null | undefined>(undefined);
  const [isMobile, setIsMobile] = useState(false);
  const [showMobilePanel, setShowMobilePanel] = useState(false);

  useEffect(() => {
    return onAuthChanged((u) => setUser(u));
  }, []);

  // Track window width for responsive layout
  useEffect(() => {
    function handleResize() {
      setIsMobile(window.innerWidth < 768);
    }
    handleResize();
    window.addEventListener("resize", handleResize);
    return () => window.removeEventListener("resize", handleResize);
  }, []);

  const toggleMobilePanel = useCallback(() => {
    setShowMobilePanel((prev) => !prev);
  }, []);

  const closeMobilePanel = useCallback(() => {
    setShowMobilePanel(false);
  }, []);

  // Loading auth state
  if (user === undefined) {
    return (
      <div style={styles.loading}>
        <span style={{ color: "#00d4ff" }}>Loading...</span>
      </div>
    );
  }

  // Not logged in
  if (!user) {
    return <LoginScreen onLogin={() => {}} />;
  }

  // Logged in -- show game UI
  return (
    <GameStateProvider>
      <div style={styles.app}>
        {/* Top bar */}
        <div style={styles.topBar}>
          <span style={styles.title}>Stormgate</span>
          <span style={styles.userInfo}>
            {isMobile && (
              <button onClick={toggleMobilePanel} style={styles.panelToggleBtn}>
                {showMobilePanel ? "Hide" : "Info"}
              </button>
            )}
            <span style={styles.userName}>
              {user.displayName || user.email}
            </span>
            <button onClick={signOut} style={styles.logoutBtn}>
              Logout
            </button>
          </span>
        </div>

        {/* Main content area */}
        <div style={styles.mainContent}>
          {/* Terminal area */}
          <Terminal />

          {/* Side panel -- desktop only */}
          {!isMobile && <SidePanel />}

          {/* Mobile overlay panel */}
          {isMobile && showMobilePanel && (
            <>
              <div style={styles.overlay} onClick={closeMobilePanel} />
              <SidePanel isMobileOverlay onClose={closeMobilePanel} />
            </>
          )}
        </div>

        {/* Mobile bottom navigation */}
        {isMobile && <MobileNav />}
      </div>
    </GameStateProvider>
  );
}

const styles: Record<string, React.CSSProperties> = {
  loading: {
    display: "flex",
    alignItems: "center",
    justifyContent: "center",
    height: "100vh",
    backgroundColor: "#0a0a0f",
    fontFamily: "monospace",
  },
  app: {
    display: "flex",
    flexDirection: "column",
    height: "100vh",
    backgroundColor: "#0a0a0f",
    overflow: "hidden",
  },
  topBar: {
    display: "flex",
    alignItems: "center",
    justifyContent: "space-between",
    padding: "8px 16px",
    backgroundColor: "#14141f",
    borderBottom: "1px solid #1e1e2e",
    fontFamily: "monospace",
    flexShrink: 0,
  },
  title: {
    color: "#00d4ff",
    fontWeight: "bold",
    fontSize: "16px",
  },
  userInfo: {
    color: "#a0a0b0",
    fontSize: "13px",
    display: "flex",
    alignItems: "center",
    gap: "12px",
  },
  userName: {
    maxWidth: "120px",
    overflow: "hidden",
    textOverflow: "ellipsis",
    whiteSpace: "nowrap",
  },
  logoutBtn: {
    padding: "4px 12px",
    fontSize: "12px",
    backgroundColor: "transparent",
    color: "#a0a0b0",
    border: "1px solid #333",
    borderRadius: "4px",
    cursor: "pointer",
    fontFamily: "monospace",
  },
  panelToggleBtn: {
    padding: "4px 12px",
    fontSize: "12px",
    backgroundColor: "#1e1e2e",
    color: "#00d4ff",
    border: "1px solid #00d4ff",
    borderRadius: "4px",
    cursor: "pointer",
    fontFamily: "monospace",
    minHeight: "28px",
  },
  mainContent: {
    display: "flex",
    flex: 1,
    minHeight: 0,
    position: "relative",
  },
  overlay: {
    position: "fixed",
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    backgroundColor: "rgba(0, 0, 0, 0.6)",
    zIndex: 99,
  },
};
