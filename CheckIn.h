#ifndef CHECKIN_H
#define CHECKIN_H

#include <HTTPClient.h>
#include <WiFi.h>

class checkIn {
private:
  const char* serverUrl;
  const char* esp32Id;
  uint32_t timeout = 5000;  // ms

  String buildPayload(const char* employeeId) {
    String payload = "{\"employeeId\":\"";
    payload += employeeId;
    payload += "\",\"esp32Id\":\"";
    payload += esp32Id;
    payload += "\"}";
    return payload;
  }

public:
  checkIn(const char* url, const char* id) {
    serverUrl = url;
    esp32Id = id;
  }

  // returns HTTP status code; -1 on WiFi failure
  int16_t send(const char* employeeId) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi not connected, skipping check-in");
      return -1;
    }

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(timeout);

    String payload = buildPayload(employeeId);
    int16_t statusCode = http.POST(payload);
    String response = http.getString();

    Serial.print("Employee: ");
    Serial.print(employeeId);
    Serial.print(" | Status: ");
    Serial.print(statusCode);
    Serial.print(" | Response: ");
    Serial.println(response);

    http.end();
    return statusCode;
  }
};

#endif
