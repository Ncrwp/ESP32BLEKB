#include <WiFi.h>
#include <WebServer.h>
#include <BleKeyboard.h>
#include "config.h"  // Keep this for WiFi credentials

// Minimal global variables
WebServer server(80);
BleKeyboard bleKeyboard("BLE Keyboard");  // Give the keyboard a name (optional)

// Ultra-minimal HTML (stored in PROGMEM)
const char htmlPage[] PROGMEM = R"rawliteral(
<html><body>
<form action='/send' method='GET'>
<input type='text' name='text' maxlength='50'><button>Send</button>
</form></body></html>
)rawliteral";

// Send text via BLE Keyboard (optimized loop)
void sendText(String text) {
  if (bleKeyboard.isConnected()) {
    bleKeyboard.print(text);  // Send entire string at once (faster)
  }
}

// Handle root request
void handleRoot() {
  server.send_P(200, "text/html", htmlPage);
}

// Handle send request
void handleSend() {
  if (server.hasArg("text")) {
    sendText(server.arg("text"));
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  // Skip Serial.begin() to save space
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(100);

  bleKeyboard.begin();  // Start BLE keyboard

  // Minimal server setup
  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.begin();
}

void loop() {
  server.handleClient();
}