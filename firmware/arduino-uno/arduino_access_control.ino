/*
  Arduino RFID + Keypad Access Control (Offline)
  - RFID UID allowlist
  - Keypad PIN fallback
  - LCD feedback
  - Servo lock control
  - Clean state handling + comments

  Note: Wi-Fi / web unlock is provided in the ESP32 firmware variant.
*/

#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// -------------------- Pins --------------------
#define RFID_SS_PIN   10
#define RFID_RST_PIN  9
#define SERVO_PIN     3

// LCD
#define LCD_ADDR 0x27
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// RFID
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

// Servo angles
static const int SERVO_LOCKED_ANGLE   = 15;
static const int SERVO_UNLOCKED_ANGLE = 95;

// Timing
static const unsigned long UNLOCK_DURATION_MS = 6000;
static const unsigned long DENY_MESSAGE_MS    = 2500;

// Keypad passcode
static const char* KEYPAD_CODE = "2580";

// Keypad config
const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Adjust pins to your keypad wiring
byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {5, 6, 7, 8};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

Servo lockServo;

// Allowed UIDs (uppercase hex without spaces)
const char* ALLOWED_UIDS[] = {
  "A1B2C3D4",
  "DEADBEEF"
};
static const size_t ALLOWED_UIDS_COUNT = sizeof(ALLOWED_UIDS) / sizeof(ALLOWED_UIDS[0]);

enum class DoorState { LOCKED, UNLOCKED };
DoorState doorState = DoorState::LOCKED;

String keypadBuffer;
unsigned long unlockStartedAt = 0;

static void lcdCenterPrint(const String& a, const String& b) {
  lcd.clear();
  auto pad = [](const String& s) {
    if (s.length() >= 16) return s.substring(0,16);
    int left = (16 - s.length()) / 2;
    String out;
    for (int i=0;i<left;i++) out += " ";
    out += s;
    return out;
  };
  lcd.setCursor(0,0); lcd.print(pad(a));
  lcd.setCursor(0,1); lcd.print(pad(b));
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

static String uidToString(const MFRC522::Uid& uid) {
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

void setup() {
  Serial.begin(9600);

  // LCD init
  lcd.init();
  lcd.backlight();
  lcdCenterPrint("Booting", "Please wait");

  // Servo init
  lockServo.attach(SERVO_PIN);
  setLocked();

  // RFID init
  SPI.begin();
  rfid.PCD_Init();
  delay(150);

  lcdCenterPrint("Ready", "Scan card / PIN");
  delay(900);
  lcd.clear();
}

void loop() {
  // Auto re-lock
  if (doorState == DoorState::UNLOCKED) {
    if (millis() - unlockStartedAt > UNLOCK_DURATION_MS) {
      lcdCenterPrint("Locking...", "");
      delay(350);
      setLocked();
      lcd.clear();
    }
  }

  // RFID read
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    lcdCenterPrint("Scanning...", "");
    delay(200);

    String uid = uidToString(rfid.uid);
    Serial.print("UID: "); Serial.println(uid);

    if (uidAllowed(uid)) {
      lcdCenterPrint("Access Granted", "Unlocking...");
      setUnlocked();
      delay(900);
    } else {
      lcdCenterPrint("Access Denied", "Try again");
      delay(DENY_MESSAGE_MS);
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    lcd.clear();
  }

  // Keypad input
  char key = keypad.getKey();
  if (key) {
    if (key == '*') {
      keypadBuffer = "";
      lcdCenterPrint("PIN Cleared", "");
      delay(700);
      lcd.clear();
    }
    else if (key == '#') {
      // Submit PIN
      if (keypadBuffer == String(KEYPAD_CODE)) {
        lcdCenterPrint("Access Granted", "Unlocking...");
        setUnlocked();
        delay(900);
      } else {
        lcdCenterPrint("Access Denied", "Try again");
        delay(DENY_MESSAGE_MS);
      }
      keypadBuffer = "";
      lcd.clear();
    }
    else {
      // Append (max length safeguard)
      if (keypadBuffer.length() < 8) keypadBuffer += key;

      // Masked display
      lcd.setCursor(0,0); lcd.print("Enter PIN:");
      lcd.setCursor(0,1);
      String masked;
      for (size_t i=0; i<keypadBuffer.length(); i++) masked += "*";
      lcd.print(masked);
    }
  }

  // Idle hint (simple)
  if (doorState == DoorState::LOCKED && keypadBuffer.length() == 0) {
    lcd.setCursor(0,0); lcd.print("Ready for Access ");
    lcd.setCursor(0,1); lcd.print("Scan / Enter PIN ");
  }
}
