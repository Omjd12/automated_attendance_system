#ifndef BLE_SCAN_H
#define BLE_SCAN_H

#include <BLEDevice.h>
#include <BLEScan.h>

#define MAX_EMPLOYEES 10
#define pass (void)0

struct Employee {
  const char* employeeId;
  const char* beaconName;
};

class bleScanner {
private:
  BLEScan* pBLEScan;
  uint8_t scanDuration = 5;    // seconds per scan cycle
  int8_t rssiThreshold = -80;  // ignore weak signals beyond this

  Employee roster[MAX_EMPLOYEES];
  uint8_t rosterSize = 0;

  char detectedIds[MAX_EMPLOYEES][16];  // store matched employeeIds this scan
  uint8_t detectedCount = 0;

public:
  void initialize() {
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    Serial.println("BLE scanner ready");
  }
  bool alreadyExists(const char* id) {
  for (uint8_t i = 0; i < detectedCount; i++) {
    if (strcmp(detectedIds[i], id) == 0) {
      return true;
    }
  }
  return false;
}

  void addEmployee(const char* empId, const char* beacon) {
    if (rosterSize >= MAX_EMPLOYEES) {
      Serial.println("Roster full");
      return;
    }
    roster[rosterSize].employeeId = empId;
    roster[rosterSize].beaconName = beacon;
    rosterSize++;
  }

  // scan and populate detectedIds[], returns count of matches
  uint8_t scan() {
    detectedCount = 0;
    BLEScanResults* found = pBLEScan->start(scanDuration, false);
    uint8_t total = found->getCount();

    for (uint8_t i = 0; i < total; i++) {
      BLEAdvertisedDevice device = found->getDevice(i);
      if (!device.haveName()) continue;
      //if (device.getRSSI() < rssiThreshold) continue;  // too far, skip

      String name = device.getName().c_str();
      

      for (uint8_t j = 0; j < rosterSize; j++) {
        
        if (name == roster[j].beaconName) {
          if (!alreadyExists(roster[j].employeeId)) {
            strncpy(detectedIds[detectedCount], roster[j].employeeId, 15);
            detectedIds[detectedCount][15] = '\0';
            detectedCount++;
            Serial.print("Detected: ");
            Serial.println(roster[j].employeeId);
            break;
          } else pass;
        } else pass;
      }
    }

    pBLEScan->clearResults();
    return detectedCount;
  }

  const char* getDetected(uint8_t index) {
    if (index <= detectedCount) return detectedIds[index];
    return nullptr;
  }

  uint8_t getDetectedCount() {
    return detectedCount;
  }
};

#endif
