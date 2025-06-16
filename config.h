#ifndef CONFIG_H
#define CONFIG_H

// ==== WiFi Config ====
const char* ssid = "LibertyM2.4_F2";
const char* password = "Neilson091796*";

// Static IP configuration
IPAddress local_IP(192, 168, 0, 15);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

#endif