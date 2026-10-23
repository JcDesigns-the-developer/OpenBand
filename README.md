# OpenBand

OpenBand is a cross-platform, software-only radio-room project with a Qt 6/C++ desktop client, a browser client, and a Cloudflare Worker signaling service.

> **Status:** early development scaffold. The desktop UI and Worker foundation are being established first. Production voice transport, TURN relay, robust authentication, and end-to-end encrypted voice are not yet implemented. Do not use this as a security-sensitive communications system yet.

## Repository layout

- `desktop/` — Qt 6 / C++ desktop application and configurable radio UI.
- `cloudflare/` — Cloudflare Worker + Durable Object room/signaling service.
- `web/` — browser client shell.
- `shared/` — shared virtual channel presets.
- `docs/` — build, architecture, and security notes.

## Desktop build

Install Qt 6 with the **Widgets** module and CMake 3.21+.

```sh
cmake -S desktop -B build
cmake --build build --config Release
```

Open `desktop/CMakeLists.txt` in Qt Creator if preferred.

## Cloudflare deployment

Requires Node.js, npm, and a Cloudflare account with Workers and Durable Objects enabled.

```sh
cd cloudflare
npm install
npx wrangler login
npx wrangler deploy
```

The Worker serves the browser client from `web/` only after the asset directory is configured for deployment (see `cloudflare/wrangler.toml`). Never put API tokens or private credentials in browser or desktop code.

## Hotkeys

- **F5** toggles the compact radio overlay (default).
- **Right Ctrl** is the default push-to-talk binding in the settings UI.
- Windows global hotkeys use the native Windows hotkey API. Global hotkeys on Linux depend on the desktop session; Wayland intentionally restricts global key capture for security.

## Important radio note

Channel names/frequencies in OpenBand are virtual labels for online rooms. The application does not transmit radio-frequency signals.
