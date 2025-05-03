#include <WiFi.h>
#include <WebServer.h>
#include <BleKeyboard.h>
#include "config.h"

WebServer server(80);
BleKeyboard bleKeyboard("BLE Keyboard");

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>BLE Keyboard Controller</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      max-width: 800px;
      margin: 0 auto;
      padding: 20px;
    }
    h1 {
      text-align: center;
      color: #333;
    }
    .container {
      display: flex;
      flex-direction: column;
      gap: 15px;
    }
    textarea {
      width: 100%;
      height: 200px;
      padding: 10px;
      box-sizing: border-box;
      border: 1px solid #ddd;
      border-radius: 4px;
      resize: vertical;
      font-size: 16px;
    }
    .button-group {
      display: flex;
      gap: 10px;
    }
    button {
      padding: 10px 20px;
      background-color: #4CAF50;
      color: white;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-size: 16px;
      flex: 1;
    }
    button.clear {
      background-color: #f44336;
    }
    button:hover {
      opacity: 0.8;
    }
  </style>
</head>
<body>
  <h1>BLE Keyboard Controller</h1>
  <div class="container">
    <form action='/send' method='GET'>
      <textarea name='text' placeholder='Type your text here...'></textarea>
      <div class="button-group">
        <button type='submit'>Send Text</button>
        <button type='button' class='clear' onclick='clearText()'>Clear Text</button>
      </div>
    </form>
  </div>
  <script>
    function clearText() {
      document.querySelector('textarea').value = '';
    }
  </script>
</body>
</html>
)rawliteral";

void sendText(String text) {
  Serial.println("[BLE] Attempting to send text");
  if (bleKeyboard.isConnected()) {
    Serial.printf("[BLE] Connected, sending %d characters\n", text.length());
    for (unsigned int i = 0; i < text.length(); i++) {
      char c = text.charAt(i);
      Serial.printf("[BLE] Sending char %d/%d: 0x%02X '%c'\n", 
                   i+1, text.length(), c, (isPrintable(c) ? c : ' '));
      bleKeyboard.write(c);
      delay(20);
      yield();
    }
    Serial.println("[BLE] Text sent successfully");
  } else {
    Serial.println("[BLE] Error: Keyboard not connected!");
  }
}

void handleRoot() {
  Serial.println("[WEB] Root page requested");
  server.send_P(200, "text/html", htmlPage);
}

void handleSend() {
  if (server.hasArg("text")) {
    String text = server.arg("text");
    Serial.printf("[WEB] Text received (%d chars): '%.20s%s'\n", 
                 text.length(), 
                 text.c_str(), 
                 (text.length() > 20 ? "..." : ""));
    sendText(text);
  } else {
    Serial.println("[WEB] Error: No text parameter received");
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nBLE Keyboard Web Controller");
  Serial.println("===========================");
  
  Serial.print("Connecting to WiFi ");
  WiFi.begin(ssid, password);
  
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    if (millis() - wifiStart > 10000) {
      Serial.println("\n[WIFI] Connection failed!");
      ESP.restart();
    }
  }
  
  Serial.printf("\n[WIFI] Connected! IP: %s\n", WiFi.localIP().toString().c_str());

  Serial.println("[BLE] Initializing keyboard...");
  bleKeyboard.begin();
  
  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.begin();
  Serial.println("[WEB] HTTP server started");
}

void loop() {
  static unsigned long lastStatus = 0;
  
  if (millis() - lastStatus > 5000) {
    Serial.printf("[STATUS] BLE Connected: %s\n", 
                 bleKeyboard.isConnected() ? "Yes" : "No");
    lastStatus = millis();
  }
  
  server.handleClient();
}