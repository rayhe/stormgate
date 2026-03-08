// Firebase configuration
// Replace these values with your Firebase project config from:
// Firebase Console > Project Settings > General > Your Apps > Web App
export const firebaseConfig = {
  apiKey: "YOUR_API_KEY",
  authDomain: "YOUR_PROJECT.firebaseapp.com",
  projectId: "YOUR_PROJECT",
  storageBucket: "YOUR_PROJECT.appspot.com",
  messagingSenderId: "YOUR_SENDER_ID",
  appId: "YOUR_APP_ID",
};

// Game server WebSocket URL
// In development, Vite proxies /ws to localhost:3000
// In production, this should be wss://your-oracle-vm-domain.com/ws
export const WS_URL = import.meta.env.PROD
  ? `wss://${window.location.host}/ws`
  : `ws://${window.location.hostname}:3000/ws`;
