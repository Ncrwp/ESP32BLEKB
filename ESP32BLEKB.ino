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
<meta name="viewport" content="width=device-width, initial-scale=1.0">

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
    input[type="number"] {
      width: 100%;
      padding: 8px;
      margin-top: 5px;
      box-sizing: border-box;
      font-size: 16px;
    }
    .button-group {
      display: flex;
      gap: 10px;
    }
    .key-group {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: 10px;
      margin-bottom: 10px;
    }
    button {
      padding: 10px 20px;
      color: white;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-size: 16px;
      flex: 1;
    }
    button.send {
      background-color: #4CAF50;
    }
    button.clear {
      background-color: #f44336;
    }
    button.pair {
      background-color: #2196F3;
    }
    button.unpair {
      background-color: #FF9800;
    }
    button.key {
      background-color: #9E9E9E;
    }
    button:hover {
      opacity: 0.8;
    }
    .status {
      padding: 10px;
      border-radius: 4px;
      text-align: center;
      font-weight: bold;
    }
    .connected {
      background-color: #DFF2BF;
      color: #4F8A10;
    }
    .disconnected {
      background-color: #FFBABA;
      color: #D8000C;
    }
    @media (max-width: 600px) {
  textarea {
    height: 300px; /* taller on small screens */
    font-size: 18px;
  }

  .button-group {
    flex-direction: column;
  }

  .key-group {
    grid-template-columns: repeat(2, 1fr);
  }

  button {
    font-size: 18px;
    padding: 12px;
  }

  body {
    padding: 10px;
  }
  </style>
</head>
<body>
  <h1>BLE Keyboard Controller</h1>
  <div class="container">
    <div class="status %STATUS_CLASS%">BLE Status: %STATUS_TEXT%</div>
    <form action='/send' method='GET'>
      <textarea name='text' placeholder='Type your text here...'></textarea>
      <div>
        <label for="delay">Delay (ms between keys):</label>
        <input type="number" id="delay" name="delay" value="20" min="0" max="1000">
      </div>
      <div class="button-group">
        <button type='submit' class='send'>Send Text</button>
        <button type='button' class='clear' onclick='clearText()'>Clear Text</button>
      </div>
    </form>
    <div class="key-group">
      <button type='button' class='key' onclick='sendKey("ENTER")'>Enter</button>
      <button type='button' class='key' onclick='sendKey("BACKSPACE")'>Backspace</button>
      <button type='button' class='key' onclick='sendKey("TAB")'>Tab</button>
      <button type='button' class='key' onclick='sendKey("ESC")'>Esc</button>
      <button type='button' class='key' onclick='sendKey("UP")'>Up</button>
      <button type='button' class='key' onclick='sendKey("DOWN")'>Down</button>
      <button type='button' class='key' onclick='sendKey("LEFT")'>Left</button>
      <button type='button' class='key' onclick='sendKey("RIGHT")'>Right</button>
    </div>
    <div class="button-group">
      <button type='button' class='pair' onclick='pairDevice()'>Pair Keyboard</button>
      <button type='button' class='unpair' onclick='unpairDevice()'>Unpair Keyboard</button>
    </div>
  </div>
  <script>
    function clearText() {
      document.querySelector('textarea').value = '';
    }

    function pairDevice() {
      fetch('/pair').then(response => {
        if (response.ok) {
          alert('Pairing initiated. Please connect to "BLE Keyboard" on your device.');
          setTimeout(() => location.reload(), 2000);
        } else {
          alert('Error initiating pairing');
        }
      });
    }

    function unpairDevice() {
      if (confirm('Are you sure you want to unpair the keyboard?')) {
        fetch('/unpair').then(response => {
          if (response.ok) {
            alert('Keyboard unpaired');
            setTimeout(() => location.reload(), 1000);
          } else {
            alert('Error unpairing keyboard');
          }
        });
      }
    }

    function sendKey(key) {
      fetch('/key?key=' + encodeURIComponent(key))
        .then(response => {
          if (!response.ok) {
            alert('Error sending key');
          }
        });
    }

    setInterval(() => {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          const statusDiv = document.querySelector('.status');
          statusDiv.textContent = `BLE Status: ${data.status}`;
          statusDiv.className = `status ${data.connected ? 'connected' : 'disconnected'}`;
        });
    }, 5000);
  </script>
</body>
</html>
)rawliteral";

void sendText(String text, int delayMs = 20) {
  Serial.println("[BLE] Attempting to send text");
  if (bleKeyboard.isConnected()) {
    Serial.printf("[BLE] Connected, sending %d characters with %d ms delay\n", text.length(), delayMs);
    for (unsigned int i = 0; i < text.length(); i++) {
      char c = text.charAt(i);
      if (isPrintable(c) || c == '\n' || c == '\t') {
        Serial.printf("[BLE] Sending char %d/%d: 0x%02X '%c'\n", i + 1, text.length(), c, c);
        bleKeyboard.write(c);
        delay(delayMs);
        yield();
      } else {
        Serial.printf("[BLE] Skipping non-printable char: 0x%02X\n", c);
      }
    }
    Serial.println("[BLE] Text sent successfully");
  } else {
    Serial.println("[BLE] Error: Keyboard not connected!");
  }
}

void sendKey(String key) {
  Serial.printf("[BLE] Attempting to send key: %s\n", key.c_str());
  if (bleKeyboard.isConnected()) {
    if (key == "ENTER") bleKeyboard.write(KEY_RETURN);
    else if (key == "BACKSPACE") bleKeyboard.write(KEY_BACKSPACE);
    else if (key == "TAB") bleKeyboard.write(KEY_TAB);
    else if (key == "ESC") bleKeyboard.write(KEY_ESC);
    else if (key == "UP") bleKeyboard.write(KEY_UP_ARROW);
    else if (key == "DOWN") bleKeyboard.write(KEY_DOWN_ARROW);
    else if (key == "LEFT") bleKeyboard.write(KEY_LEFT_ARROW);
    else if (key == "RIGHT") bleKeyboard.write(KEY_RIGHT_ARROW);
    Serial.printf("[BLE] Key %s sent successfully\n", key.c_str());
  } else {
    Serial.println("[BLE] Error: Keyboard not connected!");
  }
}

String getHtmlPage() {
  String page = FPSTR(htmlPage);
  bool isConnected = bleKeyboard.isConnected();
  page.replace("%STATUS_CLASS%", isConnected ? "connected" : "disconnected");
  page.replace("%STATUS_TEXT%", isConnected ? "Connected" : "Disconnected");
  return page;
}

void handleRoot() {
  Serial.println("[WEB] Root page requested");
  server.send(200, "text/html", getHtmlPage());
}

void handleSend() {
  int delayMs = 20;
  if (server.hasArg("delay")) {
    delayMs = constrain(server.arg("delay").toInt(), 0, 1000);
    Serial.printf("[WEB] Custom delay set to %d ms\n", delayMs);
  }

  if (server.hasArg("text")) {
    String text = server.arg("text");
    Serial.printf("[WEB] Text received (%d chars): '%.20s%s'\n",
                  text.length(), text.c_str(),
                  (text.length() > 20 ? "..." : ""));
    sendText(text, delayMs);
  } else {
    Serial.println("[WEB] Error: No text parameter received");
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleKey() {
  if (server.hasArg("key")) {
    String key = server.arg("key");
    Serial.printf("[WEB] Key received: %s\n", key.c_str());
    sendKey(key);
  } else {
    Serial.println("[WEB] Error: No key parameter received");
  }
  server.send(200, "text/plain", "Key sent");
}

void handlePair() {
  Serial.println("[BLE] Pairing requested");
  if (!bleKeyboard.isConnected()) {
    Serial.println("[BLE] Starting advertising for pairing");
    bleKeyboard.begin();
    server.send(200, "text/plain", "Pairing initiated. Connect to 'BLE Keyboard' on your device.");
  } else {
    Serial.println("[BLE] Already connected");
    server.send(200, "text/plain", "Keyboard is already connected.");
  }
}

void handleUnpair() {
  Serial.println("[BLE] Unpairing requested");
  if (bleKeyboard.isConnected()) {
    Serial.println("[BLE] Disconnecting keyboard");
    bleKeyboard.end();
    server.send(200, "text/plain", "Keyboard unpaired.");
  } else {
    Serial.println("[BLE] Not currently connected");
    server.send(200, "text/plain", "Keyboard is not currently connected.");
  }
}

void handleStatus() {
  bool isConnected = bleKeyboard.isConnected();
  String statusJson = "{\"connected\":" + String(isConnected ? "true" : "false") + ",\"status\":\"" + (isConnected ? "Connected" : "Disconnected") + "\"}";
  server.send(200, "application/json", statusJson);
}

#define BLUE_LED_PIN 2  // Built-in LED on ESP32 WROOM32

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nBLE Keyboard Web Controller");
  Serial.println("===========================");

  pinMode(BLUE_LED_PIN, OUTPUT);
  digitalWrite(BLUE_LED_PIN, LOW);  // LED off at start

  Serial.print("Connecting to WiFi ");
  if (!WiFi.config(local_IP, gateway, subnet, dns)) {
    Serial.println("Static IP config failed");
  } else {
    Serial.println("Static IP configured");
  }
  WiFi.begin(ssid, password);
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

  // Turn LED solid ON when connected
  digitalWrite(BLUE_LED_PIN, HIGH);

  Serial.println("[BLE] Initializing keyboard...");
  bleKeyboard.begin();

  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.on("/key", handleKey);
  server.on("/pair", handlePair);
  server.on("/unpair", handleUnpair);
  server.on("/status", handleStatus);
  server.begin();
  Serial.println("[WEB] HTTP server started");
}

void loop() {
  static unsigned long lastStatus = 0;
  static unsigned long lastBlink = 0;
  static bool ledState = false;

  if (millis() - lastStatus > 5000) {
    Serial.printf("[STATUS] BLE Connected: %s\n", bleKeyboard.isConnected() ? "Yes" : "No");
    lastStatus = millis();
  }

  server.handleClient();

  if (WiFi.status() != WL_CONNECTED) {
    // Blink LED when not connected
    if (millis() - lastBlink >= 500) {
      ledState = !ledState;
      digitalWrite(BLUE_LED_PIN, ledState ? HIGH : LOW);
      lastBlink = millis();
    }
  } else {
    // Solid ON if connected
    digitalWrite(BLUE_LED_PIN, HIGH);
  }
}
