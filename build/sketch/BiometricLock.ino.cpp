#include <Arduino.h>
#line 1 "C:\\Users\\leviwipf\\sources\\BiometricLock\\BiometricLock.ino"
/**
 * @file BiometricLock.ino
 * @brief Main entry point for Atomic Motion Fingerprint Lock
 * @details Handles state machine, deep sleep orchestration, and admin mode.
 * RESTORED VERSION: Handles I2C failure gracefully.
 */

#include <M5AtomS3.h>
#include <WiFi.h>
#include "MotorController.h"
#include "FingerprintManager.h"
#include "PowerManager.h"
#include "WebAdmin.h"

// Modules
MotorController motor;
FingerprintManager fpScanner;
PowerManager powerMgr;
WebAdmin* webAdmin = nullptr; // Created only in Admin Mode

// Wi-Fi Credentials for Admin Mode (AP Mode)
const char* AP_SSID = "BiometricLock-Admin";
const char* AP_PASS = "12345678";

// Logic State
enum AppState {
    STATE_BOOT,
    STATE_CHECK_WAKE,
    STATE_VERIFY_FINGER,
    STATE_ACTION_OPEN,
    STATE_ACTION_CLOSE,
    STATE_ADMIN_MODE,
    STATE_PREPARE_SLEEP
};

AppState currentState = STATE_BOOT;
bool isDoorOpen = false; // Transient State tracking

// Persistent State (Survives Deep Sleep)
RTC_DATA_ATTR bool isLocked = false;

void setupAdminMode();

#line 44 "C:\\Users\\leviwipf\\sources\\BiometricLock\\BiometricLock.ino"
void setup();
#line 93 "C:\\Users\\leviwipf\\sources\\BiometricLock\\BiometricLock.ino"
void loop();
#line 44 "C:\\Users\\leviwipf\\sources\\BiometricLock\\BiometricLock.ino"
void setup() {
    auto cfg = M5.config();
    AtomS3.begin(cfg);
    Serial.begin(115200);
    // Give serial a moment to catch up
    delay(1000); 
    Serial.println("\n\n=== BIOMETRIC LOCK BOOT ===");

    AtomS3.Display.setTextColor(WHITE);
    AtomS3.Display.setTextDatum(middle_center);
    AtomS3.Display.clear();
    
    // Hardware Init
    Serial.println("[INIT] Motor Controller...");
    AtomS3.Display.drawString("Init Motor...", AtomS3.Display.width()/2, AtomS3.Display.height()/2);
    
    // Motor Safety Check - If I2C fails, warn but continue so Wake works
    if (!motor.begin()) {
        Serial.println("[ERROR] Motor I2C Failed!");
        AtomS3.Display.fillScreen(RED);
        AtomS3.Display.setTextColor(WHITE);
        AtomS3.Display.drawString("NO BASE", AtomS3.Display.width()/2, AtomS3.Display.height()/2);
        delay(2000); // Show error but continue
        AtomS3.Display.clear();
    } else {
        Serial.println("[INIT] Motor OK");
    }
    
    // Check for Admin Mode (BtnA held)
    AtomS3.update();
    if (AtomS3.BtnA.isPressed()) {
        currentState = STATE_ADMIN_MODE;
        setupAdminMode(); // Initialize Web Components
    } else {
        // Normal Boot
        AtomS3.Display.clear();
        AtomS3.Display.drawString("Init Power...", AtomS3.Display.width()/2, AtomS3.Display.height()/2);
        powerMgr.begin();
        
        // Setup Fingerprint Sensor
        AtomS3.Display.clear();
        AtomS3.Display.drawString("Init FP...", AtomS3.Display.width()/2, AtomS3.Display.height()/2);
        fpScanner.begin();
        
        // Start State Machine
        currentState = STATE_CHECK_WAKE;
    }
}

void loop() {
    AtomS3.update(); // Update buttons/system
    motor.update();  // Handle Motor Safety Timeout
    
    switch (currentState) {
        case STATE_ADMIN_MODE:
            // Handle Web Server
            if (webAdmin) webAdmin->handleClient();
            
            // Timeout Check (Exit Admin Mode after 2 mins idle?)
            if (millis() > 120000 && WiFi.softAPgetStationNum() == 0) {
                 ESP.restart();
            }
            break;

        case STATE_CHECK_WAKE: {
            esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
            
            bool pinState = digitalRead(PIN_DOOR_SWITCH);
            isDoorOpen = (pinState == LOW); 
            
            Serial.printf("Door Pin: %d -> Status: %s\n", pinState, isDoorOpen ? "OPEN" : "CLOSED");
            
            if (cause == ESP_SLEEP_WAKEUP_EXT1) {
                 // Fingerprint Wake (PIN_FP_WAKE)
                 currentState = STATE_VERIFY_FINGER;
            } 
            else if (cause == ESP_SLEEP_WAKEUP_EXT0) {
                 // Door Wake (PIN_DOOR_SWITCH)
                 // If woken by door, check state
                 if (isDoorOpen) {
                     // Door Opened -> Prepare to sleep (or alarm?)
                     // For now, just sleep.
                     currentState = STATE_PREPARE_SLEEP;
                 } else {
                     // Door Closed -> Lock it immediately
                     // This is the "Lock on Close" logic
                     currentState = STATE_ACTION_CLOSE;
                 }
            } 
            else {
                 // Reset/Power On
                 // Default to Locking checking logic
                 currentState = STATE_ACTION_CLOSE;
            }
            break;
        }

        case STATE_VERIFY_FINGER: {
            AtomS3.Display.clear();
            AtomS3.Display.drawString("Scanning...", AtomS3.Display.width()/2, AtomS3.Display.height()/2);
            
            int id = -1;
            unsigned long scanStart = millis();
            
            // Retry loop for 3 seconds
            while (millis() - scanStart < 3000) {
                id = fpScanner.scanFinger();
                
                if (id >= 0) break; // Match found!
                
                // If error is just "No Finger", keep trying. 
                // If it's a hardware error, we technically could stop, but retrying might clear it.
                delay(100); 
            }

            if (id >= 0) {
                AtomS3.Display.fillScreen(GREEN);
                AtomS3.Display.setTextColor(BLACK);
                AtomS3.Display.drawString("UNLOCK", AtomS3.Display.width()/2, AtomS3.Display.height()/2);
                delay(500);
                currentState = STATE_ACTION_OPEN;
            } else {
                AtomS3.Display.fillScreen(RED);
                AtomS3.Display.setTextColor(WHITE);
                AtomS3.Display.drawString("DENIED", AtomS3.Display.width()/2, AtomS3.Display.height()/2);
                delay(1000);
                currentState = STATE_PREPARE_SLEEP;
            }
            break;
        }

        case STATE_ACTION_OPEN:
            motor.open();
            // Wait for motor to finish (Stop or Timeout or Stall)
            // Safety timeout of 3000ms just in case logic fails, but motor has internal 2000ms timeout
            {
                unsigned long startAction = millis();
                while (motor.isRunning() && millis() - startAction < 3000) {
                    motor.update();
                    delay(10);
                }
            }
            motor.stop(); // Ensure stops
            isLocked = false;
            currentState = STATE_PREPARE_SLEEP;
            break;

        case STATE_ACTION_CLOSE:
            motor.close();
            {
                unsigned long startAction = millis();
                while (motor.isRunning() && millis() - startAction < 3000) {
                    motor.update();
                    delay(10);
                }
            }
            motor.stop(); // Ensure stops
            isLocked = true;
            currentState = STATE_PREPARE_SLEEP;
            break;

        case STATE_PREPARE_SLEEP:
            // Display Final Status
            AtomS3.Display.clear();
            AtomS3.Display.setTextSize(2);
            if (isLocked) {
                AtomS3.Display.fillScreen(RED);
                AtomS3.Display.setTextColor(WHITE);
                AtomS3.Display.drawString("LOCKED", AtomS3.Display.width()/2, AtomS3.Display.height()/2);
            } else {
                 AtomS3.Display.fillScreen(GREEN);
                 AtomS3.Display.setTextColor(BLACK);
                 AtomS3.Display.drawString("UNLOCKED", AtomS3.Display.width()/2, AtomS3.Display.height()/2);
            }
            // Dynamic Sleep Wait Loop
            unsigned long sleepTimer = millis();
            bool lastDoorState = digitalRead(PIN_DOOR_SWITCH);
            const int SLEEP_TIMEOUT = 5000;

            while (millis() - sleepTimer < SLEEP_TIMEOUT) {
                // 1. Check Door Activity
                bool currentDoorState = digitalRead(PIN_DOOR_SWITCH);
                if (currentDoorState != lastDoorState) {
                    sleepTimer = millis(); // Reset Timer
                    lastDoorState = currentDoorState;
                    Serial.printf("[ACTIVITY] Door Changed -> %s (Reset Timer)\n", currentDoorState ? "OPEN" : "CLOSED");
                    
                    // Note: If we just transitioned to CLOSED (HIGH)
                    if (currentDoorState == HIGH) {
                        Serial.println("[AUTOLOCK] Door Closed -> Locking...");
                        currentState = STATE_ACTION_CLOSE;
                        // IMPORTANT: We break the inner loop to handle the action immediately.
                        // The action state will return us to PREPARE_SLEEP for a fresh timer.
                        break; 
                    }
                    // If transitioned to OPEN, just keep waiting (timer reset)
                }

                // 2. Check Fingerprint Activity (PIN_FP_WAKE is Fingerprint INT)
                // Sensor is Active HIGH (configured with PULLDOWN in PowerManager)
                if (digitalRead(PIN_FP_WAKE) == LOW) { 
                     if (millis() - sleepTimer > 1000) { 
                        Serial.println("[ACTIVITY] Fingerprint Sensor Touched (Reset Timer)");
                     }
                     sleepTimer = millis(); 
                }

                // Status Update (approx every second)
                // static unsigned long lastPrint = 0;
                // if (millis() - lastPrint > 1000) {
                //     lastPrint = millis();
                //     int waiting = (SLEEP_TIMEOUT - (millis() - sleepTimer)) / 1000;
                //     Serial.printf("Status: %s (Sleep in %ds)\n", lastDoorState ? "OPEN" : "CLOSED", waiting);
                // }
                
                delay(10); // Check frequently
            }

            // Check final state before sleeping
            // Logic: Low = Open (Switch Closed to Ground)
            bool isDoorOpenNow = (digitalRead(PIN_DOOR_SWITCH) == LOW);
            
            AtomS3.Display.clear();
            AtomS3.Display.setBrightness(0);
            
            powerMgr.enterDeepSleep(isDoorOpenNow);
            break;
    }
}

void setupAdminMode() {
    AtomS3.Display.fillScreen(BLUE);
    AtomS3.Display.setTextSize(2);
    AtomS3.Display.drawString("ADMIN MODE", AtomS3.Display.width()/2, 40);
    
    WiFi.softAP(AP_SSID, AP_PASS);
    IPAddress IP = WiFi.softAPIP();
    
    AtomS3.Display.setTextSize(1);
    AtomS3.Display.drawString(IP.toString(), AtomS3.Display.width()/2, 80);
    
    fpScanner.begin(); 
    webAdmin = new WebAdmin(&fpScanner);
    webAdmin->begin();
}

