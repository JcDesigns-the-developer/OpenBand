const JSON_HEADERS = { "content-type": "application/json; charset=utf-8", "cache-control": "no-store" };

function json(data, status = 200) {
  return new Response(JSON.stringify(data), { status, headers: JSON_HEADERS });
}

function validRoom(room) {
  return typeof room === "string" && /^[a-zA-Z0-9_-]{1,48}$/.test(room);
}

export default {
  async fetch(request, env) {
    const url = new URL(request.url);
    if (request.method === "GET" && url.pathname === "/health") {
      return json({ app: env.APP_NAME || "OpenBand", status: "ok", stage: "early-development" });
    }

    if (url.pathname === "/ws") {
      if (request.headers.get("Upgrade")?.toLowerCase() !== "websocket") {
        return json({ error: "WebSocket upgrade required" }, 426);
      }
      const room = url.searchParams.get("room") || "general";
      if (!validRoom(room)) return json({ error: "Invalid room name" }, 400);
      const id = env.ROOMS.idFromName(room);
      return env.ROOMS.get(id).fetch(request);
    }

    if (request.method === "GET" && url.pathname === "/api/rooms") {
      const id = env.ROOMS.idFromName("__directory__");
      return env.ROOMS.get(id).fetch(new Request("https://internal/list"));
    }

    return json({ error: "Not found" }, 404);
  }
};

export class OpenBandRoom {
  constructor(state, env) {
    this.state = state;
    this.env = env;
    this.sessions = new Map();
  }

  async fetch(request) {
    const url = new URL(request.url);
    if (url.pathname === "/list") {
      return json({ rooms: await this.state.storage.get("rooms") || [] });
    }
    if (request.headers.get("Upgrade")?.toLowerCase() !== "websocket") {
      return json({ error: "WebSocket upgrade required" }, 426);
    }

    const pair = new WebSocketPair();
    const [client, server] = Object.values(pair);
    const roomName = new URL(request.url).searchParams.get("room") || "general";
    const peerId = crypto.randomUUID();
    server.accept();
    this.sessions.set(peerId, server);

    server.send(JSON.stringify({ type: "welcome", peerId, room: roomName }));
    for (const [id, socket] of this.sessions) {
      if (id !== peerId) {
        try { socket.send(JSON.stringify({ type: "peer-joined", peerId })); } catch {}
      }
    }

    server.addEventListener("message", event => {
      let message;
      try { message = JSON.parse(event.data); } catch { return; }
      if (!message || typeof message.type !== "string") return;
      // Limit signaling payload size before relaying.
      if (typeof event.data === "string" && event.data.length > 16_000) {
        try { server.send(JSON.stringify({ type: "error", message: "Message too large" })); } catch {}
        return;
      }
      const allowed = new Set(["offer", "answer", "ice-candidate", "chat", "leave"]);
      if (!allowed.has(message.type)) return;
      if (message.type === "leave") {
        this.removePeer(peerId);
        return;
      }
      if (typeof message.to !== "string" || !this.sessions.has(message.to)) return;
      const target = this.sessions.get(message.to);
      try {
        target.send(JSON.stringify({ ...message, from: peerId }));
      } catch {
        this.removePeer(message.to);
      }
    });

    const cleanup = () => this.removePeer(peerId);
    server.addEventListener("close", cleanup);
    server.addEventListener("error", cleanup);
    return new Response(client, { status: 101, webSocket: client });
  }

  removePeer(peerId) {
    if (!this.sessions.has(peerId)) return;
    this.sessions.delete(peerId);
    for (const socket of this.sessions.values()) {
      try { socket.send(JSON.stringify({ type: "peer-left", peerId })); } catch {}
    }
  }
}
