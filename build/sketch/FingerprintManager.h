#line 1 "C:\\Users\\leviwipf\\sources\\BiometricLock\\FingerprintManager.h"
#ifndef FINGERPRINT_MANAGER_H
#define FINGERPRINT_MANAGER_H

#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <functional>

typedef std::function<void(const char*)> LogCallback;

// Sensor Pin Definitions are now in HardwareConfig.h

#define MYSERIAL Serial2 

class FingerprintManager {
public:
    FingerprintManager();
    bool begin();
    
    // Returns finger ID if matched, -1 if no match, -2 if no finger
    int scanFinger();
    
    // Returns true if enrollment successful
    int enrollFinger(int id, LogCallback logger = nullptr);
    
    bool deleteFinger(int id);
    bool deleteAll();
    
    uint16_t getTemplateCount();

private:
    Adafruit_Fingerprint finger;

};

#endif
