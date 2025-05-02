#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <WiFi.h>
#include "config.h"

class WiFiConnection {
public:
    bool connect() {
        WiFi.config(local_IP, gateway, subnet);
        WiFi.begin(ssid, password);
        
        Serial.print("Connecting to WiFi");
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi connected");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
            return true;
        }
        
        Serial.println("\nWiFi connection failed");
        return false;
    }
};

#endif