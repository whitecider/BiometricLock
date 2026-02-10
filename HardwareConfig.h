#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <Arduino.h>

// ==========================================
// Hardware Pin Definitions
// ==========================================

// --- Fingerprint Sensor (Port C & B) ---
// UART Communication (Port C)
#define PIN_FP_RX       5   // Yellow Pin on Port C
#define PIN_FP_TX       6   // White Pin on Port C
#define FP_BAUD_RATE    57600

// Wake/Touch Signal (Port B)
// Confirmed: Fingerprint Sensor Wake is connected to PIN 7 (Active LOW)
#define PIN_FP_WAKE     G7  

// --- Door Sensor (Port B) ---
// Magnetic Reed Switch
#define PIN_DOOR_SWITCH G8  // Yellow Pin on Port B

// --- DC Motor (Channel 0 / M1) ---
// Note: Atomic Motion uses 0-indexed channels. M1 is channel 0.
#define MOTOR_CHANNEL   0  

// Speed Control (-127 to 127)
#define MOTOR_SPEED_OPEN   127  
#define MOTOR_SPEED_CLOSE  -127 
#define MOTOR_STOP         0

// Safety Timeout (2 Seconds as requested)
#define MOTOR_TIMEOUT_MS   2000

#endif // HARDWARE_CONFIG_H
