// I centralize all HTTP calls here so the UI never
// talks to fetch() directly.
const BASE_URL = "http://localhost:8080";

async function request(path, { method = "GET", body, token } = {}) {
  const res = await fetch(`${BASE_URL}${path}`, {
    method,
    headers: {
      "Content-Type": "application/json",
      ...(token ? { Authorization: `Bearer ${token}` } : {}),
    },
    body: body ? JSON.stringify(body) : undefined,
  });

  const text = await res.text();
  let data;
  try {
    data = JSON.parse(text);
  } catch {
    data = { error: text };
  }

  if (!res.ok) {
    throw new Error(data.error || "Request failed");
  }

  return data;
}

export const api = {
  register: (username, password) =>
    request("/auth/register", {
      method: "POST",
      body: { username, password },
    }),

  login: (username, password) =>
    request("/auth/login", {
      method: "POST",
      body: { username, password },
    }),

  weather: (city, token) =>
    request("/weather/current", {
      method: "POST",
      token,
      body: { city },
    }),

  history: (token) =>
    request("/history", {
      token,
    }),
};
