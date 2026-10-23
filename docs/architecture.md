# Architecture

- **Desktop:** Qt 6 Widgets application in C++20. The current UI exposes callsign, virtual channel presets, mode, receive-volume and squelch controls, a local PTT interaction, and an F5 overlay toggle on Windows.
- **Cloudflare:** Worker handles health checks and room WebSocket routing. Durable Objects own per-room signaling sessions.
- **Web:** browser UI shell for checking whether the Worker is reachable.
- **Shared:** JSON channel preset definitions.

## Planned sequence

1. Finish configuration persistence and reliable global hotkeys.
2. Add authenticated room membership and tested signaling schemas.
3. Implement WebRTC audio capture/playback and TURN fallback.
4. Add audio-device selection, reconnect logic, mute indicators, and diagnostics.
5. Add automated tests and deployment-specific origin controls.
