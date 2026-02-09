/*
  ESP32 IoT RFID + Keypad Access Control
  - RFID UID allowlist
  - Keypad passcode fallback
  - LCD animations
  - Servo lock/unlock with auto re-lock
  - Wi-Fi + Web control (unlock/lock/status)
  - Token-protected endpoints for demo purposes

  Notes:
  - This is a prototype-grade implementation. For production, you'd add TLS, proper auth,
    secure storage, rate limiting, and tamper protection.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// -------------------- Configuration --------------------
static const char* WIFI_SSID     = "YOUR_WIFI_SSID";
static const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Simple token (demo). Change it before pushing public if you want.
static const char* API_TOKEN = "change_me_demo_token";

// Lock behavior
static const uint32_t UNLOCK_DURATION_MS = 6000;  // auto relock after 6s
static const uint32_t DENY_MESSAGE_MS    = 2500;

// Keypad passcode (demo). You can store hashed in EEPROM for more "real" design.
static const char* KEYPAD_CODE = "2580";

// -------------------- Pin Mapping --------------------
#define RFID_SS_PIN   5
#define RFID_RST_PIN  22

#define SERVO_PIN     15

// I2C LCD address (most common: 0x27 or 0x3F)
#define LCD_ADDR      0x27
#define LCD_COLS      16
#define LCD_ROWS      2

// -------------------- RFID --------------------
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

// Allowed UID list (uppercase hex without spaces)
const char* ALLOWED_UIDS[] = {
  "A1B2C3D4",      // Example
  "DEADBEEF"       // Example
};
static const size_t ALLOWED_UIDS_COUNT = sizeof(ALLOWED_UIDS) / sizeof(ALLOWED_UIDS[0]);

// -------------------- Keypad --------------------
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// -------------------- LCD + Servo --------------------
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
Servo lockServo;

// Servo angles (tune for your hardware)
static const int SERVO_LOCKED_ANGLE   = 15;
static const int SERVO_UNLOCKED_ANGLE = 95;

// -------------------- Web Server --------------------
WebServer server(80);

// -------------------- State --------------------
enum class DoorState { LOCKED, UNLOCKED };
DoorState doorState = DoorState::LOCKED;

String keypadBuffer;
uint32_t unlockStartedAt = 0;

uint32_t lastUiTick = 0;
uint8_t idleAnimFrame = 0;

// -------------------- Helpers --------------------
static String toUidString(const MFRC522::Uid& uid) {
  String s;
  for (byte i = 0; i < uid.size; i++) {
    if (uid.uidByte[i] < 0x10) s += "0";
    s += String(uid.uidByte[i], HEX);
  }
  s.toUpperCase();
  return s;
}

static bool uidAllowed(const String& uid) {
  for (size_t i = 0; i < ALLOWED_UIDS_COUNT; i++) {
    if (uid == String(ALLOWED_UIDS[i])) return true;
  }
  return false;
}

static void setLocked() {
  lockServo.write(SERVO_LOCKED_ANGLE);
  doorState = DoorState::LOCKED;
}

static void setUnlocked() {
  lockServo.write(SERVO_UNLOCKED_ANGLE);
  doorState = DoorState::UNLOCKED;
  unlockStartedAt = millis();
}

static void lcdCenterPrint(const String& line1, const String& line2) {
  lcd.clear();
  auto pad = [](const String& s) {
    if (s.length() >= 16) return s.substring(0,16);
    int left = (16 - s.length()) / 2;
    String out;
    for (int i=0;i<left;i++) out += " ";
    out += s;
    return out;
  };
  lcd.setCursor(0, 0);
  lcd.print(pad(line1));
  lcd.setCursor(0, 1);
  lcd.print(pad(line2));
}

static void showIdleAnimation() {
  // Non-annoying “system ready” animation
  const char* dots[] = {"   ", ".  ", ".. ", "..."};
  lcd.setCursor(0, 0);
  lcd.print("Ready for Access");
  lcd.setCursor(0, 1);
  lcd.print("Scan/Card or PIN");
  lcd.setCursor(13, 0);
  lcd.print(dots[idleAnimFrame % 4]);
  idleAnimFrame++;
}

static void showScanAnimation() {
  lcdCenterPrint("Scanning...", "Hold card near");
}

static void showGrantedAnimation() {
  lcdCenterPrint("Access Granted", "Unlocking...");
}

static void showDeniedAnimation() {
  lcdCenterPrint("Access Denied", "Try again");
}

static void showLockingAnimation() {
  lcdCenterPrint("Locking...", "Please wait");
}

static bool checkToken() {
  if (!server.hasArg("token")) return false;
  return server.arg("token") == String(API_TOKEN);
}

// -------------------- Web Handlers --------------------
static void handleRoot() {
  String html;
  html += "<!doctype html><html><head><meta charset='utf-8'/>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'/>";
  html += "<title>Access Control</title></head><body style='font-family:Arial;padding:16px;max-width:700px'>";
  html += "<h2>IoT Access Control</h2>";
  html += "<p>Status: <b>";
  html += (doorState == DoorState::LOCKED ? "LOCKED" : "UNLOCKED");
  html += "</b></p>";
  html += "<p>Use endpoints with token (demo):</p>";
  html += "<pre>POST /unlock?token=...\nPOST /lock?token=...\nGET  /status</pre>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

static void handleStatus() {
  String json = "{";
  json += "\"state\":\"";
  json += (doorState == DoorState::LOCKED ? "LOCKED" : "UNLOCKED");
  json += "\",\"wifi_ip\":\"";
  json += WiFi.localIP().toString();
  json += "\"}";
  server.send(200, "application/json", json);
}

static void handleUnlock() {
  if (!checkToken()) {
    server.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  showGrantedAnimation();
  setUnlocked();
  server.send(200, "application/json", "{\"ok\":true,\"action\":\"unlock\"}");
}

static void handleLock() {
  if (!checkToken()) {
    server.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  showLockingAnimation();
  delay(350);
  setLocked();
  server.send(200, "application/json", "{\"ok\":true,\"action\":\"lock\"}");
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);

  // LCD
  Wire.begin(21, 4);     // SDA, SCL
  lcd.init();
  lcd.backlight();
  lcdCenterPrint("Booting", "Please wait");

  // Servo
  lockServo.setPeriodHertz(50);
  lockServo.attach(SERVO_PIN, 500, 2400);
  setLocked();

  // RFID
  SPI.begin();
  rfid.PCD_Init();
  delay(100);

  // Wi-Fi
  lcdCenterPrint("Wi-Fi", "Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < 12000) {
    delay(250);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
    lcdCenterPrint("Wi-Fi Connected", WiFi.localIP().toString());
    delay(1200);
  } else {
    Serial.println("\nWiFi failed. Running offline mode.");
    lcdCenterPrint("Wi-Fi Failed", "Offline Mode");
    delay(1200);
  }

  // Web server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/unlock", HTTP_POST, handleUnlock);
  server.on("/lock", HTTP_POST, handleLock);
  server.begin();

  lcd.clear();
  lastUiTick = millis();
}

// -------------------- Loop --------------------
void loop() {
  server.handleClient();

  // Auto re-lock
  if (doorState == DoorState::UNLOCKED) {
    if (millis() - unlockStartedAt > UNLOCK_DURATION_MS) {
      showLockingAnimation();
      delay(350);
      setLocked();
      lcd.clear();
    }
  }

  // RFID handling
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    showScanAnimation();
    delay(200);

    String uid = toUidString(rfid.uid);
    Serial.println("RFID UID: " + uid);

    if (uidAllowed(uid)) {
      showGrantedAnimation();
      setUnlocked();
      delay(800);
    } else {
      showDeniedAnimation();
      delay(DENY_MESSAGE_MS);
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    lcd.clear();
  }

  // Keypad handling
  char key = keypad.getKey();
  if (key) {
    // Basic UI behavior
    if (key == '*') {
      keypadBuffer = "";
      lcdCenterPrint("PIN Cleared", "");
      delay(700);
      lcd.clear();
    } else if (key == '#') {
      // Submit PIN
      if (keypadBuffer == String(KEYPAD_CODE)) {
        showGrantedAnimation();
        setUnlocked();
        delay(800);
      } else {
        showDeniedAnimation();
        delay(DENY_MESSAGE_MS);
      }
      keypadBuffer = "";
      lcd.clear();
    } else {
      // Add digit/char
      if (keypadBuffer.length() < 8) {
        keypadBuffer += key;
      }
      // Show masked PIN entry
      lcd.setCursor(0, 0);
      lcd.print("Enter PIN:");
      lcd.setCursor(0, 1);
      String masked;
      for (size_t i = 0; i < keypadBuffer.length(); i++) masked += "*";
      lcd.print(masked);
    }
  }

  // Idle animation update (non-blocking)
  if (millis() - lastUiTick > 500) {
    lastUiTick = millis();
    if (doorState == DoorState::LOCKED && keypadBuffer.length() == 0) {
      showIdleAnimation();
    }
  }
}
