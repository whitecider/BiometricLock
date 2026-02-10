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

// --- Servo ---
#define SERVO_CHANNEL   0

#endif // HARDWARE_CONFIG_H
