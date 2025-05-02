#include <WiFi.h>
#include <WebServer.h>
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include "config.h"

WebServer server(80);
NimBLEHIDDevice* hid;
NimBLECharacteristic* input;
bool isConnected = false;

// Static IP Configuration
IPAddress local_IP(192, 168, 0, 15);  // Set static IP address
IPAddress gateway(192, 168, 0, 1);    // Set gateway (your router's IP)
IPAddress subnet(255, 255, 255, 0);   // Set subnet mask

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

  // Log Wi-Fi credentials for debugging
  Serial.println("Connecting to Wi-Fi...");
  Serial.print("SSID: ");
  Serial.println(ssid); // Log the SSID being used for connection
  Serial.print("Password: ");
  Serial.println(password); // Log the password being used (for debugging only)

  // Connect to Wi-Fi with Static IP
  WiFi.config(local_IP, gateway, subnet);  // Apply static IP configuration
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected successfully!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi connection failed.");
    Serial.print("Error: ");
    Serial.println(WiFi.status());
    
    // Attempt to reconnect after 5 seconds if the Wi-Fi connection fails
    Serial.println("Retrying connection...");
    delay(5000);
    ESP.restart();  // Restart ESP32 to try reconnecting
  }

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
