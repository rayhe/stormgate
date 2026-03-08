import { initializeApp } from "firebase/app";
import {
  getAuth,
  signInWithPopup,
  signOut as firebaseSignOut,
  GoogleAuthProvider,
  onAuthStateChanged,
  type User,
} from "firebase/auth";
import { firebaseConfig } from "./config";

const app = initializeApp(firebaseConfig);
const auth = getAuth(app);
const googleProvider = new GoogleAuthProvider();

export async function signInWithGoogle(): Promise<User> {
  const result = await signInWithPopup(auth, googleProvider);
  return result.user;
}

export async function signOut(): Promise<void> {
  await firebaseSignOut(auth);
}

export function onAuthChanged(callback: (user: User | null) => void): () => void {
  return onAuthStateChanged(auth, callback);
}

export async function getIdToken(): Promise<string | null> {
  const user = auth.currentUser;
  if (!user) return null;
  return user.getIdToken();
}

export function getCurrentUser(): User | null {
  return auth.currentUser;
}
