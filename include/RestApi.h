#pragma once

namespace RestApi {

// Owns the HTTP server and exposes the public API surface:
//   GET /       short plaintext hint
//   GET /level  current sensor reading plus derived tank estimates as JSON
void begin();
void handleClient();

}  // namespace RestApi
