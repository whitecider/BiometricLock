#include "MotorController.h"

MotorController::MotorController() : _stallStartTime(0), _isStallPotential(false), _isMotorRunning(false), _motorStartTime(0), _maxCurrent(0), _startCurrent(0), _sumCurrent(0), _sumVoltage(0), _sampleCount(0) {
}

bool MotorController::begin() {
    // Initialize I2C for Atomic Motion Base 
    // AtomS3 Bottom Pins: SDA=38, SCL=39 (Standard M5AtomS3 Internal/Bottom I2C)
    // (2,1 are for the Side Groove Port A)
    return atomicMotion.begin(&Wire, M5_ATOMIC_MOTION_I2C_ADDR, 38, 39, 100000);
}

void MotorController::resetStats() {
    _maxCurrent = 0;
    _startCurrent = 0;
    _sumCurrent = 0;
    _sumVoltage = 0;
    _sampleCount = 0;
}

void MotorController::open() {
    resetStats();
    atomicMotion.setMotorSpeed(MOTOR_CHANNEL, MOTOR_SPEED_OPEN);
    _isMotorRunning = true;
    _motorStartTime = millis();
}

void MotorController::close() {
    resetStats();
    atomicMotion.setMotorSpeed(MOTOR_CHANNEL, MOTOR_SPEED_CLOSE);
    _isMotorRunning = true;
    _motorStartTime = millis();
}

void MotorController::stop() {
    atomicMotion.setMotorSpeed(MOTOR_CHANNEL, MOTOR_STOP);

    // Only process stats if we were running
    if (_isMotorRunning) {
        _isMotorRunning = false;

        // Print Stats
        if (_sampleCount > 0) {
            float avgCurrent = _sumCurrent / _sampleCount;
            float avgVoltage = _sumVoltage / _sampleCount;
            Serial.printf("[MOTOR STATS] Max I: %.2f mA | Avg I: %.2f mA | Avg V: %.2f V\n", _maxCurrent, avgCurrent, avgVoltage);
        }
    }
}

void MotorController::update() {
    if (_isMotorRunning) {
        if (millis() - _motorStartTime > MOTOR_TIMEOUT_MS) {
            stop(); // Safety timeout
        } else {
            // Sampling logic
            float current = getCurrentReading();
            float voltage = getBusVoltage();
            
            if (_sampleCount == 0) {
                _startCurrent = current;
            }
            _sumCurrent += current;
            _sumVoltage += voltage;
            _sampleCount++;
            
            // Note: getCurrentReading handles _maxCurrent update internally
            
            // Stall Detection Logic
            if (current > STALL_CURRENT_THRESHOLD) {
                if (!_isStallPotential) {
                    _stallStartTime = millis();
                    _isStallPotential = true;
                } else if (millis() - _stallStartTime > STALL_TIME_MS) {
                    stop(); // Trigger Stall Stop
                    _isStallPotential = false; 
                }
            } else {
                _isStallPotential = false;
            }
        }
    }
}


bool MotorController::checkStall() {
    // We can rely on getCurrentReading calls in update() or call explicit
    // For now, let's just return false as we are using Timeouts
    return false; 
}
// Power Monitoring
float MotorController::getCurrentReading() {
    // Shunt Voltage Register 0x01
    Wire.beginTransmission(0x40);
    Wire.write(0x01);
    Wire.endTransmission();
    Wire.requestFrom(0x40, 2);
    
    int16_t rawShunt = 0;
    if (Wire.available() >= 2) {
        rawShunt = (int16_t)((Wire.read() << 8) | Wire.read());
    }
    // Convert to mA (assuming 0.02 Ohm shunt, 2.5uV LSB)
    // V = raw * 2.5uV. I = V/0.02 = raw * 125uA = raw * 0.125 mA
    float current = abs(rawShunt * 0.125f);
    
    // Track Max Current
    if (current > _maxCurrent) {
        _maxCurrent = current;
    }
    return current;
}

float MotorController::getMaxCurrent() {
    return _maxCurrent;
}

float MotorController::getAverageCurrent() {
    if (_sampleCount == 0) return 0;
    return _sumCurrent / _sampleCount;
}

float MotorController::getStartingCurrent() {
    return _startCurrent;
}

bool MotorController::isRunning() {
    return _isMotorRunning;
}

void MotorController::resetMaxCurrent() {
    _maxCurrent = 0;
}

float MotorController::getBusVoltage() {
    // Bus Voltage Register 0x02
    Wire.beginTransmission(0x40);
    Wire.write(0x02);
    Wire.endTransmission();
    Wire.requestFrom(0x40, 2);
    
    int16_t rawBus = 0;
    if (Wire.available() >= 2) {
        rawBus = (int16_t)((Wire.read() << 8) | Wire.read());
    }
    // LSB = 1.25 mV
    return (rawBus * 1.25f) / 1000.0f; 
}
