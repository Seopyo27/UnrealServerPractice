"use strict";

// Test-only login/server directory. No external packages are required.
const http = require("node:http");

const PORT = Number(process.env.PORT || 8080);
const TEST_ACCOUNT = { userId: "testuser", password: "test1234" };
const servers = new Map();

function sendJson(response, statusCode, payload) {
  response.writeHead(statusCode, { "Content-Type": "application/json; charset=utf-8" });
  response.end(JSON.stringify(payload));
}

function readJson(request) {
  return new Promise((resolve, reject) => {
    let body = "";
    request.setEncoding("utf8");
    request.on("data", (chunk) => {
      body += chunk;
      if (body.length > 1024 * 1024) reject(new Error("Request body is too large."));
    });
    request.on("end", () => {
      try { resolve(body ? JSON.parse(body) : {}); }
      catch { reject(new Error("Request body must be JSON.")); }
    });
    request.on("error", reject);
  });
}

function remoteIp(request) {
  const address = request.socket.remoteAddress || "";
  return address.replace(/^::ffff:/, "");
}

const app = http.createServer(async (request, response) => {
  try {
    if (request.method === "GET" && request.url === "/health") {
      return sendJson(response, 200, { ok: true, registered_server_count: servers.size });
    }

    if (request.method === "POST" && request.url === "/api/servers/register") {
      const body = await readJson(request);
      const serverId = String(body.server_id || "default-server");
      const port = Number(body.port || 7777);
      if (!Number.isInteger(port) || port < 1 || port > 65535) {
        return sendJson(response, 400, { error: "A valid port is required." });
      }

      // The game server cannot safely choose its own public IP. Use the HTTP peer address instead.
      const server = { serverId, ip: remoteIp(request), port, map: String(body.map || ""), registeredAt: new Date().toISOString() };
      servers.set(serverId, server);
      console.log(`Registered ${serverId}: ${server.ip}:${server.port}`);
      return sendJson(response, 200, { ok: true, server_ip: server.ip, server_port: server.port });
    }

    if (request.method === "POST" && request.url === "/api/login") {
      const body = await readJson(request);
      if (body.user_id !== TEST_ACCOUNT.userId || body.password !== TEST_ACCOUNT.password) {
        return sendJson(response, 401, { error: "Invalid user ID or password." });
      }

      const server = servers.get("default-server") || servers.values().next().value;
      if (!server) return sendJson(response, 503, { error: "No game server is registered." });

      return sendJson(response, 200, { server_ip: server.ip, server_port: server.port });
    }

    return sendJson(response, 404, { error: "Not found." });
  } catch (error) {
    return sendJson(response, 400, { error: error.message });
  }
});

app.listen(PORT, "0.0.0.0", () => {
  console.log(`Test web server running at http://127.0.0.1:${PORT}`);
  console.log(`Test account: ${TEST_ACCOUNT.userId} / ${TEST_ACCOUNT.password}`);
});
