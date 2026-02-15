const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

let users = [{ username: "demo", password: "demo" }];
let sessions = new Map();
let history = new Map();

function makeToken() {
  return Math.random().toString(16).slice(2) + Date.now().toString(16);
}

export async function mockApi(path, { method = "GET", body, token } = {}) {
  await sleep(200);

  if (path === "/auth/register" && method === "POST") {
    const { username, password } = body || {};
    if (!username || !password) throw new Error("Missing username or password.");
    if (users.some((u) => u.username === username))
      throw new Error("Username already exists.");
    users.push({ username, password });
    return { ok: true };
  }

  if (path === "/auth/login" && method === "POST") {
    const { username, password } = body || {};
    const u = users.find(
      (x) => x.username === username && x.password === password
    );
    if (!u) throw new Error("Invalid credentials.");
    const tokenValue = makeToken();
    sessions.set(tokenValue, username);
    return { token: tokenValue, username };
  }

  const who = token ? sessions.get(token) : null;
  if (!who && (path === "/history" || path.startsWith("/weather"))) {
    throw new Error("Unauthorized.");
  }

  if (path === "/weather/current" && method === "POST") {
    const { city } = body || {};
    if (!city) throw new Error("City is required.");

    const summary =
      `Location: ${city}\n` +
      `Condition: Clear\n` +
      `Temperature: ${(18 + Math.random() * 10).toFixed(1)} C\n` +
      `Wind: ${(5 + Math.random() * 18).toFixed(1)} kph\n`;

    const entry = {
      timestamp: new Date().toISOString(),
      city,
      summary,
    };

    const arr = history.get(who) || [];
    arr.unshift(entry);
    history.set(who, arr);

    return { summary };
  }

  if (path === "/history" && method === "GET") {
    return history.get(who) || [];
  }

  throw new Error(`Not implemented: ${method} ${path}`);
}
