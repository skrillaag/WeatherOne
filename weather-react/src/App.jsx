import React, { useMemo, useState } from "react";
import { api } from "./api";

// I keep Field generic so it can be reused across auth and weather forms.
function Field({ label, value, onChange, type = "text" }) {
  return (
    <label style={{ display: "grid", gap: 6 }}>
      <span style={{ fontSize: 13, opacity: 0.9 }}>{label}</span>
      <input
        type={type}
        value={value}
        onChange={(e) => onChange(e.target.value)}
        style={{
          padding: "10px 12px",
          borderRadius: 10,
          border: "1px solid rgba(255,255,255,0.15)",
          background: "rgba(255,255,255,0.06)",
          color: "white",
          outline: "none",
        }}
      />
    </label>
  );
}

// I centralize button styling so disabled and busy states behave consistently.
function Button({ children, onClick, disabled, type = "button" }) {
  return (
    <button
      type={type}
      onClick={onClick}
      disabled={disabled}
      style={{
        padding: "10px 12px",
        borderRadius: 10,
        border: "1px solid rgba(255,255,255,0.15)",
        background: "rgba(255,255,255,0.12)",
        color: "white",
        cursor: disabled ? "not-allowed" : "pointer",
        opacity: disabled ? 0.6 : 1,
      }}
    >
      {children}
    </button>
  );
}

// I use Card as a layout primitive to keep structure consistent.
function Card({ title, children, right }) {
  return (
    <div
      style={{
        border: "1px solid rgba(255,255,255,0.14)",
        background: "rgba(0,0,0,0.25)",
        borderRadius: 16,
        padding: 16,
      }}
    >
      <div
        style={{
          display: "flex",
          justifyContent: "space-between",
          alignItems: "center",
          marginBottom: 12,
        }}
      >
        <div style={{ fontSize: 16, fontWeight: 600 }}>{title}</div>
        {right}
      </div>
      {children}
    </div>
  );
}

export default function App() {
  const [mode, setMode] = useState("login");
  const [token, setToken] = useState("");
  const [username, setUsername] = useState("");

  const [authUser, setAuthUser] = useState("");
  const [authPass, setAuthPass] = useState("");

  const [city, setCity] = useState("");
  const [weatherText, setWeatherText] = useState("");
  const [history, setHistory] = useState([]);

  const [busy, setBusy] = useState(false);
  const [error, setError] = useState("");

  const isAuthed = useMemo(() => Boolean(token), [token]);

  async function handleRegister(e) {
    e.preventDefault();
    setError("");
    setBusy(true);
    try {
      await api.register(authUser, authPass);
      setMode("login");
      setAuthPass("");
    } catch (err) {
      setError(err.message);
    } finally {
      setBusy(false);
    }
  }

  async function handleLogin(e) {
    e.preventDefault();
    setError("");
    setBusy(true);
    try {
      const res = await api.login(authUser, authPass);
      setToken(res.token);
      setUsername(res.username);
      setAuthPass("");
      setWeatherText("");
      setHistory([]);
    } catch (err) {
      setError(err.message);
    } finally {
      setBusy(false);
    }
  }

  function handleLogout() {
    setToken("");
    setUsername("");
    setCity("");
    setWeatherText("");
    setHistory([]);
    setError("");
    setMode("login");
  }

  async function handleWeather(e) {
    e.preventDefault();
    setError("");
    setBusy(true);
    try {
      const res = await api.weather(city, token);
      setWeatherText(res.summary);
    } catch (err) {
      setError(err.message);
    } finally {
      setBusy(false);
    }
  }

  async function handleHistory() {
    setError("");
    setBusy(true);
    try {
      const res = await api.history(token);
      setHistory(res);
    } catch (err) {
      setError(err.message);
    } finally {
      setBusy(false);
    }
  }

  return (
    <div
      style={{
        minHeight: "100vh",
        background: "#0b0f1a",
        color: "white",
        padding: 24,
        fontFamily: "system-ui, sans-serif",
      }}
    >
      <div style={{ maxWidth: 900, margin: "0 auto", display: "grid", gap: 16 }}>
        <div style={{ display: "flex", justifyContent: "space-between" }}>
          <div>
            <div style={{ fontSize: 22, fontWeight: 700 }}>WeatherApp</div>
            <div style={{ fontSize: 13, opacity: 0.8 }}>
              {isAuthed ? `Signed in as ${username}` : "Not signed in"}
            </div>
          </div>
          {isAuthed ? (
            <Button onClick={handleLogout}>Logout</Button>
          ) : (
            <div style={{ display: "flex", gap: 8 }}>
              <Button onClick={() => setMode("login")}>Login</Button>
              <Button onClick={() => setMode("register")}>Register</Button>
            </div>
          )}
        </div>

        {error && (
          <div
            style={{
              padding: 12,
              borderRadius: 12,
              background: "rgba(255,0,0,0.12)",
              border: "1px solid rgba(255,0,0,0.3)",
            }}
          >
            {error}
          </div>
        )}

        {!isAuthed ? (
          <Card title={mode === "login" ? "Login" : "Register"}>
            <form
              onSubmit={mode === "login" ? handleLogin : handleRegister}
              style={{ display: "grid", gap: 12 }}
            >
              <Field label="Username" value={authUser} onChange={setAuthUser} />
              <Field
                label="Password"
                type="password"
                value={authPass}
                onChange={setAuthPass}
              />
              <Button disabled={busy || !authUser || !authPass} type="submit">
                {busy ? "Working..." : mode === "login" ? "Login" : "Register"}
              </Button>
            </form>
          </Card>
        ) : (
          <div style={{ display: "grid", gridTemplateColumns: "1.2fr 0.8fr", gap: 16 }}>
            <Card title="Current weather">
              <form onSubmit={handleWeather} style={{ display: "grid", gap: 12 }}>
                <Field label="City" value={city} onChange={setCity} />
                <Button disabled={busy || !city} type="submit">
                  {busy ? "Fetching..." : "Get weather"}
                </Button>
              </form>

              {weatherText && (
                <pre
                  style={{
                    marginTop: 12,
                    padding: 12,
                    borderRadius: 12,
                    background: "rgba(255,255,255,0.05)",
                    whiteSpace: "pre-wrap",
                    fontSize: 13,
                  }}
                >
                  {weatherText}
                </pre>
              )}
            </Card>

            <Card title="History" right={<Button onClick={handleHistory}>Refresh</Button>}>
              {history.length === 0 ? (
                <div style={{ fontSize: 13, opacity: 0.8 }}>No history.</div>
              ) : (
                <div style={{ display: "grid", gap: 10 }}>
                  {history.map((h, i) => (
                    <div
                      key={i}
                      style={{
                        padding: 10,
                        borderRadius: 12,
                        background: "rgba(255,255,255,0.04)",
                      }}
                    >
                      <div style={{ fontWeight: 600 }}>{h.city}</div>
                      <div style={{ fontSize: 12, opacity: 0.7 }}>
                        {h.timestamp}
                      </div>
                      <div style={{ marginTop: 6, whiteSpace: "pre-wrap" }}>
                        {h.summary}
                      </div>
                    </div>
                  ))}
                </div>
              )}
            </Card>
          </div>
        )}
      </div>
    </div>
  );
}
