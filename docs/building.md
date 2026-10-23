# Build and deploy

## Qt desktop

Install Qt 6.4+ with Qt Widgets and CMake 3.21+.

```sh
cmake -S desktop -B build
cmake --build build --config Release
```

The desktop application currently uses the local preset file at `shared/channel-presets.json` when launched from the repository root, or a fallback relative path. Runtime packaging should copy the file beside the application in a future packaging step.

## Cloudflare Worker

```sh
cd cloudflare
npm install
npx wrangler dev
npx wrangler deploy
```

The current Worker includes `/health`, `/api/rooms`, and `/ws?room=...`. The room listing is a minimal placeholder and needs a proper directory Durable Object before production use.

## Browser UI

```sh
cd web
npm install
npm run dev
```

The web app expects the Worker to serve `/health` at the same origin in deployment. Local Vite proxy configuration has not yet been added.
