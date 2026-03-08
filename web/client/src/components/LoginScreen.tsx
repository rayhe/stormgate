import { signInWithGoogle } from "../core/auth";

interface LoginScreenProps {
  onLogin: () => void;
}

export function LoginScreen({ onLogin }: LoginScreenProps) {
  const handleLogin = async () => {
    try {
      await signInWithGoogle();
      onLogin();
    } catch (err) {
      console.error("Login failed:", err);
    }
  };

  return (
    <div style={styles.container}>
      <div style={styles.box}>
        <pre style={styles.ascii}>{`
   _____ _                                  _
  / ____| |                                | |
 | (___ | |_ ___  _ __ _ __ ___   __ _  __ _| |_ ___
  \\___ \\| __/ _ \\| '__| '_ \` _ \\ / _\` |/ _\` | __/ _ \\
  ____) | || (_) | |  | | | | | | (_| | (_| | ||  __/
 |_____/ \\__\\___/|_|  |_| |_| |_|\\__, |\\__,_|\\__\\___|
                                   __/ |
                                  |___/
`}</pre>
        <p style={styles.tagline}>A text-based adventure awaits</p>
        <button onClick={handleLogin} style={styles.button}>
          Sign in with Google
        </button>
      </div>
    </div>
  );
}

const styles: Record<string, React.CSSProperties> = {
  container: {
    display: "flex",
    alignItems: "center",
    justifyContent: "center",
    height: "100vh",
    backgroundColor: "#0a0a0f",
    color: "#e0e0e0",
    fontFamily: "monospace",
  },
  box: {
    textAlign: "center",
    padding: "2rem",
  },
  ascii: {
    fontSize: "0.7rem",
    lineHeight: 1.2,
    color: "#00d4ff",
    marginBottom: "1rem",
  },
  tagline: {
    fontSize: "1rem",
    color: "#a0a0b0",
    marginBottom: "2rem",
  },
  button: {
    padding: "12px 32px",
    fontSize: "1rem",
    backgroundColor: "#4285f4",
    color: "white",
    border: "none",
    borderRadius: "4px",
    cursor: "pointer",
    fontFamily: "monospace",
  },
};
