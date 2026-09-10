# Test web server

Run from this folder:

```powershell
node server.js
```

Test account:

```text
ID: testuser
Password: test1234
```

Endpoints: `GET /health`, `POST /api/servers/register`, and `POST /api/login`.

This is intentionally a test server: users and registered servers are stored only in memory and disappear when the process stops. Do not use the plaintext test-account pattern in production.
