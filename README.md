# silver-spoon
ESP32 firmware for an automated employee attendance system. Scans for BLE beacons on employee phones, matches them against a registered roster, and posts check-in events to the backend API which logs attendance in Firebase.

# 📡 IoT Layer — Attendance System

ESP32-based proximity detection layer for the automated employee attendance system. Scans for BLE beacons carried by employees and posts check-in events to the backend API.

---

## How It Works

```
Employee's Phone          ESP32 Device             Backend Server
(BLE Peripheral App)  →  (BLE Scanner)        →   /api/esp32/checkin
  "EMP001-BEACON"         detects beacon            writes to Firebase
```

1. Each employee runs a BLE advertising app on their phone broadcasting a unique name (e.g. `EMP001-BEACON`)
2. The ESP32 continuously scans for registered beacon names in range
3. On a match, the ESP32 sends a POST request to the backend with the `employeeId`
4. The backend marks attendance in Firebase — duplicate check-ins on the same day are automatically rejected

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

The phone acts as a BLE peripheral. Since Android does not advertise BLE passively, employees need a BLE advertising app:

- **App:** [BLE Peripheral Simulator](https://play.google.com/store/apps/details?id=io.github.webbluetoothcg.bletestperipheral) *(free, Play Store)*
- Open app → **Add Advertiser** → set Device Name to `EMP001-BEACON` → **Start**
- The app must be running and advertising for detection to work

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

- BLE scan interval is set to `30000 ms` (30 seconds) by default. Adjust `SCAN_INTERVAL` in `attendance.ino` to suit your environment
- The ESP32 will hard-stop (`while(1)`) if WiFi fails on boot — this is intentional to avoid silent failures
- The backend prevents duplicate attendance records for the same employee on the same day; the ESP32 does not need to track this itself
