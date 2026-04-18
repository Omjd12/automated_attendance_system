#include "WiFiConnect.h"
#include "BLEScan.h"
#include "CheckIn.h"

#define pass (void)0

// ── config ──────────────────────────────────────────────────────────────────
const char* WIFI_SSID     = "Testing";
const char* WIFI_PASS     = "test1234";
const char* SERVER_URL    = "http://10.148.147.82:3000/api/esp32/checkin";
const char* ESP32_ID      = "esp32-gateway";
const uint32_t SCAN_INTERVAL = 30000;  // ms between scan cycles
// ────────────────────────────────────────────────────────────────────────────

wifi     wifi(WIFI_SSID, WIFI_PASS);
bleScanner scanner;
checkIn  checkin(SERVER_URL, ESP32_ID);

bool wifiReady = false;
unsigned long lastScan = 0;

void setup() {
  Serial.begin(115200);

  wifiReady = wifi.connect();
  if (!wifiReady) {
    Serial.println("terminated: no WiFi");
    while (1);
  }

  scanner.initialize();

  // register employees: addEmployee(employeeId, beaconName)
  scanner.addEmployee("EMP001", "EMP001-BEACON");
  scanner.addEmployee("EMP002", "EMP002-BEACON");
  scanner.addEmployee("EMP003", "EMP003-BEACON");

  Serial.println("Attendance system ready");
}

void loop() {
  unsigned long now = millis();

  if (now - lastScan >= SCAN_INTERVAL) {
    wifi.reconnect();  // ensure WiFi stays alive between scans

    Serial.println("Scanning...");
    uint8_t count = scanner.scan();

    if (count > 0) {
      for (uint8_t i = 0; i < count; i++) {
        const char* empId = scanner.getDetected(i);
        if (empId != nullptr) checkin.send(empId);
        else pass;
      }
    } else {
      Serial.println("No employees in range");
    }

    lastScan = millis();
  } else pass;
}
