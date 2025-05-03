#include <WiFi.h>
#include <WebServer.h>
#include <BleKeyboard.h>
#include "config.h"  // Keep this for WiFi credentials

// Minimal global variables
WebServer server(80);
BleKeyboard bleKeyboard("BLE Keyboard");  // Give the keyboard a name (optional)

// HTML with resizable textarea (description box)
const char htmlPage[] PROGMEM = R"rawliteral(
<html>
  <head>
    <style>
      textarea {
        width: 300px;
        height: 150px;
        font-size: 14px;
        resize: both; /* Allows resizing in both directions */
      }
      button {
        padding: 8px 16px;
        font-size: 16px;
        margin-top: 10px;
      }
    </style>
  </head>
  <body>
    <h2>BLE Keyboard</h2>
    <form action='/send' method='GET'>
      <textarea name='text' placeholder='Type your text here...'></textarea><br>
      <button type='submit'>Send</button>
    </form>
  </body>
</html>
)rawliteral";

// Improved sendText() with delay between characters
void sendText(String text) {
  if (bleKeyboard.isConnected()) {
    for (unsigned int i = 0; i < text.length(); i++) {
      bleKeyboard.write(text.charAt(i));  // More reliable than print()
      delay(20);  // 20ms delay prevents stuck keys
      yield();    // Prevent watchdog timer issues
    }
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
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(100);

  bleKeyboard.begin();  // Start BLE keyboard

  // Server setup
  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.begin();
}

void loop() {
  server.handleClient();
}