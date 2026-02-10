#include <Arduino.h>
#line 1 "C:\\Users\\leviwipf\\sources\\BiometricLock\\R503_Diagnostics\\R503_Diagnostics.ino"
#include <Adafruit_Fingerprint.h>
#include "HardwareConfig.h"

// Use HardwareSerial for AtomS3R (Port C usually, or custom pins)
// R503 Wiring:
// TX (Green) -> RX (Pin G19/G5 etc)
// RX (White) -> TX (Pin G22/G6 etc)
// WAKE (Blue/Yellow) -> GPIO

// Re-using definitions from HardwareConfig.h for consistency
// Ensure these match your actual physical wiring!
#define MYSERIAL Serial2

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&MYSERIAL);

#line 16 "C:\\Users\\leviwipf\\sources\\BiometricLock\\R503_Diagnostics\\R503_Diagnostics.ino"
void setup();
#line 63 "C:\\Users\\leviwipf\\sources\\BiometricLock\\R503_Diagnostics\\R503_Diagnostics.ino"
void loop();
#line 16 "C:\\Users\\leviwipf\\sources\\BiometricLock\\R503_Diagnostics\\R503_Diagnostics.ino"
void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(100);

  Serial.println("\n\n--- R503 DIAGNOSTICS START ---");
  Serial.printf("Configured Pins: RX=%d, TX=%d, WAKE=%d\n", PIN_FP_RX, PIN_FP_TX, PIN_FP_WAKE);

  pinMode(PIN_FP_WAKE, INPUT_PULLUP); // R503 Wake is Active LOW

  // Initialize Sensor Serial
  MYSERIAL.begin(FP_BAUD_RATE, SERIAL_8N1, PIN_FP_RX, PIN_FP_TX);
  finger.begin(FP_BAUD_RATE);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    Serial.println("CHECK WIRING: TX<->RX Cross connection needed.");
    Serial.println("CHECK POWER: Red=3.3V/5V, Black=GND");
    while (1) { delay(1); }
  }

  // --- FORCE CONFIGURATION ---
  Serial.println("Attempting to Lower Security Level to 1 (Min)...");
  finger.setSecurityLevel(1); 
  delay(100);

  // Dump Parameters
  Serial.println("Reading Sensor Parameters...");
  finger.getParameters();
  Serial.print("Status: 0x"); Serial.println(finger.status_reg, HEX);
  Serial.print("Sys ID: 0x"); Serial.println(finger.system_id, HEX);
  Serial.print("Capacity: "); Serial.println(finger.capacity);
  Serial.print("Security Level: "); Serial.println(finger.security_level);
  Serial.print("Device Addr: 0x"); Serial.println(finger.device_addr, HEX);
  Serial.print("Packet Len: "); Serial.println(finger.packet_len);
  Serial.print("Baud Rate: "); Serial.println(finger.baud_rate);

  Serial.println("-----------------------------");
  Serial.println("Starting Loop: Place finger on sensor.");
  Serial.println("LED will breathe Blue to indicate 'Alive'.");
  
  // Set LED to Blue Breathing
  finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 100, FINGERPRINT_LED_BLUE);
}

void loop() {
  // 1. Check GPIO Touch
  int touch = digitalRead(PIN_FP_WAKE);
  
  // 2. Check Image Command
  uint8_t p = finger.getImage();

  Serial.printf("Time:%lu | GPIO:%s | ImageCmd:0x%02X (%s)\n", 
    millis(), 
    (touch == LOW) ? "TOUCHED" : "OPEN", 
    p,
    (p == FINGERPRINT_OK) ? "OK" : 
    (p == FINGERPRINT_NOFINGER) ? "NO_FINGER" : "ERROR"
  );

  if (p == FINGERPRINT_OK) {
    Serial.println(">>> IMAGE CAPTURED SUCCESS! <<<");
    finger.LEDcontrol(FINGERPRINT_LED_ON, 0, FINGERPRINT_LED_PURPLE);
    delay(500);
    finger.LEDcontrol(FINGERPRINT_LED_BREATHING, 100, FINGERPRINT_LED_BLUE);
  }

  delay(200); // Poll at 5hz
}

