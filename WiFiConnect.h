#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include <WiFi.h>

class wifi {
private:
  const char* ssid;
  const char* password;
  uint8_t retryLimit = 20;

public:
  wifi(const char* s, const char* p) {
    ssid = s;
    password = p;
  }

  bool connect() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(1000);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    uint8_t attempts = 0;
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");



    while (WiFi.status() != WL_CONNECTED && attempts < retryLimit) {
      delay(500);
      Serial.print(".");
      attempts++;

      Serial.print(" Status: ");
      Serial.println(WiFi.status());
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nFailed to connect");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
      return true;
    } else {
      Serial.println("\nWiFi failed");
      return false;
    }
  }

  bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
  }

  void reconnect() {
    if (!isConnected()) {
      Serial.println("Reconnecting WiFi...");
      WiFi.disconnect();
      connect();
    }
  }
};

#endif
