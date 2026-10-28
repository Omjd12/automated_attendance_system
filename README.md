# silver-spoon
ESP32 firmware to support an automated system of taking employee attendance. Checks for BLE beacons from employees' smartphones, associates them with the registered list, and generates attendance events through backend API to record attendance in Firebase.
# 📡 IoT Layer — Attendance System

Proximity sensing layer using ESP32 for the automatic employee attendance system. Detects BLE beacon signals from employees and logs the check-in events to the backend API.
---

## How It Works

```
Employee's Phone          ESP32 Device             Backend Server
(BLE Peripheral App)  →  (BLE Scanner)        →   /api/esp32/checkin
  "EMP001-BEACON"         detects beacon            writes to Firebase
```

1. Every employee uses the BLE Advertising application running on his/her phone to broadcast the unique employee ID (for example, `EMP001-BEACON`).
2. The ESP32 is constantly scanning for the registered beacon ID within the range.
3. If there is a match, the ESP32 sends a `POST` request to the back-end containing the `employeeId`.
4. The back-end checks attendance in Firebase and rejects multiple check-ins during the same day.
---

## File Structure

```
├── attendance.ino      # Main sketch — wiring and loop
├── WiFiConnect.h       # WiFi init and reconnect logic
├── BLEScan.h           # BLE scan, employee roster, callback-based matching
└── CheckIn.h           # HTTP POST builder and response handler
```

---

## Configuration

Open `attendance.ino` and update the four constants at the top:

```cpp
const char* WIFI_SSID   = "YOUR_WIFI_SSID";
const char* WIFI_PASS   = "YOUR_WIFI_PASSWORD";
const char* SERVER_URL  = "http://192.168.1.10:3000/api/esp32/checkin";
const char* ESP32_ID    = "esp32-gateway";
```

> If the backend is deployed publicly, replace the LAN IP with the live domain:
> `https://yourapp.com/api/esp32/checkin`

---

## Registering Employees

In `setup()` inside `attendance.ino`, call `addEmployee()` for each employee:

```cpp
scanner.addEmployee("EMP001", "EMP001-BEACON");
scanner.addEmployee("EMP002", "EMP002-BEACON");
scanner.addEmployee("EMP003", "EMP003-BEACON");
```

| Parameter    | Description                                          |
|--------------|------------------------------------------------------|
| `employeeId` | Must exactly match the ID stored in Firebase         |
| `beaconName` | Must exactly match the advertised BLE device name    |

---

## Employee Phone Setup

The phone serves as a BLE peripheral device. As Android does not perform BLE advertisements in passive mode, there is a requirement for a BLE advertisement application:

- **App**: [BLE Peripheral Simulator](https://play.google.com/store/apps/details?id=io.github.webbluetoothcg.bletestperipheral) *(free, Google Play Store)*
- Launch app → **Add Advertiser** → set Device Name to `EMP001-BEACON` → **Start** 
- Advertisement is required in order to detect the device
---

## API Contract

The ESP32 sends to:

```
POST /api/esp32/checkin
Content-Type: application/json
```

```json
{
  "employeeId": "EMP001",
  "esp32Id":    "esp32-gateway"
}
```

### Response Codes

| Code  | Meaning                              | ESP32 action       |
|-------|--------------------------------------|--------------------|
| `200` | Attendance marked successfully       | Logs success       |
| `409` | Already marked today                 | Ignored (expected) |
| `400` | Bad request — malformed payload      | Logs error         |
| `404` | Employee ID not found in database    | Logs error         |
| `500` | Server / Firebase error              | Logs error         |
| `< 0` | Network error — never reached server | Logs error code    |

---

## Debug Mode

In `BLEScan.h`, set the debug flag to print every nearby BLE device to Serial:

```cpp
#define BLE_DEBUG true
```

**Serial output during scan:**
```
── BLE scan start ──────────────────
  [BLE] name: (no name)       rssi: -71
  [BLE] name: EMP001-BEACON   rssi: -52
  >> Matched: EMP001
── scan end | matched: 1 ──────────────────
```

Set to `false` in production to reduce Serial noise.

---

## Dependencies

Install via Arduino Library Manager:

| Library               | Purpose                  |
|-----------------------|--------------------------|
| `ESP32 BLE Arduino`   | BLE scan and advertising |
| `HTTPClient` (built-in)| HTTP POST requests       |
| `WiFi` (built-in)     | WiFi connection          |

**Board:** ESP32 Dev Module  
**Arduino Core:** ESP32 by Espressif ≥ v2.0

---

## Notes

- BLE scan interval is initially set to `30000 ms`. You can change it as per your surroundings by modifying the value of `SCAN_INTERVAL` in `attendance.ino`
- The ESP32 will be hard stopped (`while(1)`) in case of WiFi failure upon initialization; this is purposeful
- Backend takes care of the fact that same employee does not record duplicate attendances on the same day; ESP32 doesn’t have to deal with this
