# Security status

OpenBand is an early-development scaffold. It is not currently safe to rely on for private or emergency communications.

## Current protections

- HTTPS/WSS should be used in deployed environments.
- Worker validates room names and filters signaling message types.
- Signaling payloads are size-limited.
- No Cloudflare credentials are embedded in the client.

## Missing before production

- User authentication and authorization for room access.
- Abuse prevention, per-IP/session rate limits, quotas, and moderation.
- Robust schema validation and message routing tests.
- WebRTC voice media, TURN relay, and secure peer negotiation.
- End-to-end encryption design and key management for both chat and voice.
- Origin allow-listing and deployment-specific CORS policy.
- Security review, dependency updates, automated tests, and observability.

Cloudflare protects the service edge and can reduce exposure, but it does not make clients or communications impossible to compromise. A room name is not authentication. Never put Cloudflare API tokens in this repository or client application.
