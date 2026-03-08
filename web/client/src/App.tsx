import { useState, useEffect } from "react";
import { onAuthChanged, signOut } from "./core/auth";
import { LoginScreen } from "./components/LoginScreen";
import { Terminal } from "./components/Terminal";
import type { User } from "firebase/auth";

export default function App() {
  const [user, setUser] = useState<User | null | undefined>(undefined);

  useEffect(() => {
    return onAuthChanged((u) => setUser(u));
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

  // Logged in — show terminal
  return (
    <div style={styles.app}>
      <div style={styles.topBar}>
        <span style={styles.title}>Stormgate</span>
        <span style={styles.userInfo}>
          {user.displayName || user.email}
          <button onClick={signOut} style={styles.logoutBtn}>
            Logout
          </button>
        </span>
      </div>
      <Terminal />
    </div>
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
};
