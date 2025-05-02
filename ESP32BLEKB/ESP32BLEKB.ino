#include <WebServer.h>
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include "wifi_connection.h"
#include "hid_report.h"
#include "config.h"

WebServer server(80);
WiFiConnection wifi;
bool bleConnected = false;
NimBLEHIDDevice* hid;
NimBLECharacteristic* input;

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
        bleConnected = true;
        Serial.println("BLE connected");
        pServer->updateConnParams(connInfo.getConnHandle(), 6, 12, 0, 400);
    }
    
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
        bleConnected = false;
        Serial.println("BLE disconnected");
        NimBLEDevice::startAdvertising();
    }
};

void setupBLE() {
    NimBLEDevice::init(DEVICE_NAME);
    NimBLEDevice::setPower(BLE_POWER);
    
    NimBLEServer* serverBLE = NimBLEDevice::createServer();
    serverBLE->setCallbacks(new ServerCallbacks());
    
    hid = new NimBLEHIDDevice(serverBLE);
    
    // Corrected manufacturer and HID info setup
    hid->setManufacturer("ESP32");
    hid->setHidInfo(0x00, 0x02);
    hid->setReportMap((uint8_t*)reportMap, sizeof(reportMap));
    hid->startServices();
    
    input = hid->getInputReport(1);
    
    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    advertising->addServiceUUID(hid->getHidService()->getUUID());
    advertising->setAppearance(HID_APPEARANCE);
    advertising->start();
}

void sendKey(uint8_t keyCode) {
    if (!bleConnected) {
        Serial.println("Not connected - can't send key");
        return;
    }
    
    uint8_t report[8] = {0x00, 0x00, keyCode};
    input->setValue(report, sizeof(report));
    input->notify();
    delay(10);
    
    memset(report, 0, sizeof(report));
    input->setValue(report, sizeof(report));
    input->notify();
}

void setupWebServer() {
    server.on("/", []() {
        String html = "<html><body><h2>ESP32 BLE Keyboard</h2>"
                     "<p>Status: " + String(bleConnected ? "Connected" : "Disconnected") + "</p>"
                     "<form action='/send'><button>Send 'A'</button></form>"
                     "</body></html>";
        server.send(200, "text/html", html);
    });

    server.on("/send", []() {
        sendKey(0x04); // Send 'A' key
        server.sendHeader("Location", "/");
        server.send(303);
    });

    server.begin();
    Serial.println("Web server started");
}

void setup() {
    Serial.begin(115200);
    
    if (!wifi.connect()) {
        delay(5000);
        ESP.restart();
    }
    
    setupBLE();
    setupWebServer();
}

void loop() {
    server.handleClient();
}