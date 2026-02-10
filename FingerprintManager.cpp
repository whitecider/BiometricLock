#include "FingerprintManager.h"
#include "HardwareConfig.h" 
#include "enrol.h" 

// Define Serial2 pins 
// Using definitions from HardwareConfig.h

FingerprintManager::FingerprintManager() : finger(&MYSERIAL) {
}

bool FingerprintManager::begin() {
    MYSERIAL.begin(FP_BAUD_RATE, SERIAL_8N1, PIN_FP_RX, PIN_FP_TX);
    finger.begin(FP_BAUD_RATE);
    
    if (finger.verifyPassword()) {
        return true;
    }
    return false;
}

int FingerprintManager::scanFinger() {
    static uint8_t lastP = 0xFF; // Start with a value that isn't a likely error

    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK) {
        // Only print if different from last time
        if (p != lastP) {
            Serial.printf("[FP] scanFinger: getImage returned 0x%02X (%s)\n", p, (p == FINGERPRINT_NOFINGER) ? "No Finger" : "Error");
            lastP = p;
        }
        if (p == FINGERPRINT_NOFINGER) return -2;
        return -1;
    }
    
    // Reset lastP on success so next error is printed
    lastP = FINGERPRINT_OK; 
    
    Serial.println("[FP] Image taken");

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK) {
        Serial.printf("[FP] image2Tz failed: 0x%02X\n", p);
        return -1;
    }
    Serial.println("[FP] Image converted");

    p = finger.fingerFastSearch();
    if (p == FINGERPRINT_OK) {
        Serial.printf("[FP] FOUND MATCH: ID #%d, Confidence: %d\n", finger.fingerID, finger.confidence);
        return finger.fingerID;
    } else {
         Serial.printf("[FP] No match found: 0x%02X\n", p);
    }
    return -1;
}

// Complex multi-stage enrollment
// Note: This is blocking. For a web interface, we might need a non-blocking state machine later.
// For now, implementing the standard blocking flow to verify hardware interaction.
// NOTE: This will time out the web request if not handled via async or frequent status polling.
// PROPOSAL: Change this to a 'step' based approach for Web Admin compatibility.
// Refactoring for Step-based enrollment would be better for Web API.
// BUT, sticking to plan for "Core Logic" first. Let's make it return typical Adafruit codes.

// Use standard enrollment logic from enrol.h
int FingerprintManager::enrollFinger(int id, LogCallback logger) {
    // Note: getFingerprintEnroll uses Serial directly, bypassing the logger for now.
    // This is intentional per request to use the example code "exactly as is".
    
    uint8_t result = getFingerprintEnroll(finger, id);
    
    if (result == true || result == FINGERPRINT_OK) { // check for success (true is 1, FINGERPRINT_OK is 0 usually, but the example returns true on success)
         // The example function returns 'true' (1) on success at the very end.
         // It returns error codes (usually > 1) on failure.
         // Wait, typical Adafruit error codes are uint8_t.
         // Let's assume non-zero return (if it's an error code) or check logic.
         // The function returns 'true' (1) on success.
         return 1; 
    }
    return -1;
}

bool FingerprintManager::deleteFinger(int id) {
    return finger.deleteModel(id) == FINGERPRINT_OK;
}

bool FingerprintManager::deleteAll() {
    return finger.emptyDatabase() == FINGERPRINT_OK;
}

uint16_t FingerprintManager::getTemplateCount() {
    finger.getTemplateCount();
    return finger.templateCount;
}
