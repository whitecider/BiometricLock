#line 1 "C:\\Users\\leviwipf\\sources\\BiometricLock\\MotorController.h"
#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "M5AtomicMotion.h"
#include "HardwareConfig.h"

// Stall Detection Configuration
#define STALL_CURRENT_THRESHOLD 80.0f // mA (Tuned: Normal ~60mA, Stall ~93mA)
#define STALL_TIME_MS 200             // Time over threshold to trigger stall

class MotorController {
public:
    MotorController();
    bool begin();
    
    // DC Motor Control
    void open();
    void close();
    void stop();
    
    // Update loop for safely handling timeouts
    void update(); 

    // Returns true if a stall is detected
    bool checkStall();

    // Power Monitoring
    float getBusVoltage(); // Returns Volts
    float getCurrentReading(); // Returns mA
    
    // Max Current Tracking
    float getMaxCurrent();
    void resetMaxCurrent();
    
    // Run Diagnostics
    float getAverageCurrent();
    float getStartingCurrent();
    bool isRunning();

private:
    M5AtomicMotion atomicMotion;

    uint32_t _stallStartTime;
    bool _isStallPotential;

    // Timeout Management
    uint32_t _motorStartTime;
    bool _isMotorRunning;
    
    // Diagnostics
    float _maxCurrent;
    float _startCurrent;
    float _sumCurrent;
    float _sumVoltage;
    uint32_t _sampleCount;
    
    void resetStats();
};

#endif
