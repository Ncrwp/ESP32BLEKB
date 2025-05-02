#include <WiFi.h>
#include <WebServer.h>
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include "config.h"  // Your Wi-Fi credentials

WebServer server(80);
NimBLEHIDDevice* hid;
NimBLECharacteristic* input;
bool isConnected = false;

// === BLE Server Callback Class ===
class MyServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    isConnected = true;
    Serial.println("BLE connected.");
  }

  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    isConnected = false;
    Serial.println("BLE disconnected.");
  }
};


void setupBLE() {
  NimBLEDevice::init("ESP32_Keyboard");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEServer* serverBLE = NimBLEDevice::createServer();
  serverBLE->setCallbacks(new MyServerCallbacks());

  hid = new NimBLEHIDDevice(serverBLE);

  const uint8_t reportMap[] = {
    0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x85, 0x01,
    0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7,
    0x15, 0x00, 0x25, 0x01,
    0x75, 0x01, 0x95, 0x08,
    0x81, 0x02,
    0x95, 0x01, 0x75, 0x08,
    0x81, 0x01,
    0x95, 0x06, 0x75, 0x08,
    0x15, 0x00, 0x25, 0x65,
    0x05, 0x07, 0x19, 0x00, 0x29, 0x65,
    0x81, 0x00,
    0xC0
  };

  hid->setReportMap((uint8_t*)reportMap, sizeof(reportMap));
  hid->startServices();

  input = hid->getInputReport(1); // Report ID 1

  NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(hid->getHidService()->getUUID());
  advertising->setAppearance(0x03C1);
  advertising->start();
}

void sendAKey() {
  if (!isConnected) return;

  uint8_t report[8] = {0x00, 0x00, 0x04}; // 'A'
  input->setValue(report, sizeof(report));
  input->notify();
  delay(10);

  memset(report, 0, sizeof(report)); // Release
  input->setValue(report, sizeof(report));
  input->notify();
}

void handleRoot() {
  String html = "<html><body><h2>ESP32 BLE Keyboard</h2>";
  html += "<p>Status: " + String(isConnected ? "Connected" : "Disconnected") + "</p>";
  html += "<form action='/send'><button type='submit'>Send 'A'</button></form>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSend() {
  sendAKey();
  server.sendHeader("Location", "/");
  server.send(303); // Redirect
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected. IP: " + WiFi.localIP().toString());

  // Start web server
  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.begin();
  Serial.println("Web server started.");

  // Start BLE HID
  setupBLE();
}

void loop() {
  server.handleClient();
}
