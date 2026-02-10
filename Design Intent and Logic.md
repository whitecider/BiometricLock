# Atomic Motion - Design Intent & Functional Specification

This document outlines the functional goals, control logic, and power management strategy for the Atomic Motion Servo project.

## 1. Project Overview
The goal is to build a low-power, motor-driven locking mechanism (or similar actuator) using the M5AtomS3R and Atomic Motion Base. The system relies on biometric authentication to open and a magnetic sensor to auto-close, with integrated stall detection for safety.

## 2. Core Functional Logic

The system operates based on three primary signals:

### A. Open Cycle (Unlock)
*   **Trigger:** Valid Fingerprint detected.
*   **Signal:** Fingerprint Sensor pulls **G7 HIGH** (Wake).
*   **Action:**
    1.  Verify Fingerprint ID.
    2.  If authorized, drive Motor **FORWARD** (Open direction).
    3.  Continue movement until **Stall Detected** (> 80mA for 200ms) or **Timeout** (2000ms).

### B. Close Cycle (Lock)
*   **Trigger:** Door Closed / Magnet Detected.
*   **Signal:** Mag Switch **OPENS** circuit (Wake via state change/interrupt).
*   **Action:**
    1.  Detect "Door Closed" state (Magnet Present).
    2.  Drive Motor **BACKWARD** (Close direction).
    3.  Continue movement until **Stall Detected** (> 80mA for 200ms) or **Timeout** (2000ms).

### C. Safety & Termination (Stall Detection)
*   **Mechanism:** INA226 Current Monitor on Atomic Motion Base.
*   **Logic:**
    *   Monitor current during motor movement.
    *   If `Current > Threshold` (80mA) for `Time > Limit` (200ms):
    *   **IMMEDIATELY STOP** the motor.
    *   Consider the action complete.

## 3. Power Management Strategy

The device is designed for battery operation and relies on the ESP32's **Deep Sleep** mode.

### Sleep State
*   **Condition:** Device enters Deep Sleep after any action (Open/Close) or timeout, regardless of door position.
*   **Dynamic Wake Configuration:** The code must check the current door state before sleeping:
    *   **If Door is Closed (G8 High):** Configure Wake on **LOW**. (Power: ~0mA).
    *   **If Door is Open (G8 Low):** Configure Wake on **HIGH**. (Power: Higher, as switch is conducting).
*   **Power Down:**
    *   AtomS3R CPU: Off.
    *   Motion Base STM32: Low-power wait state.

### Wake Sources
1.  **Fingerprint Sensor (G7):**
    *   Configured as `ext0` or `ext1` wake-up source.
    *   Wakes device when a finger is placed on the sensor (logic HIGH).
2.  **Mag Switch (G8):**
    *   **Logic:** ESP32 Deep Sleep uses **Level Triggering**.
    *   **State at Sleep:** Door is Closed (Magnet Present) &rarr; Switch Open &rarr; **G8 HIGH**.
    *   **Wake Configuration:** Configure `ext0` to wake when **G8 is LOW**.
    *   **Trigger Event:** Door Opens &rarr; Switch Closes &rarr; **G8 pulled LOW** &rarr; Wake Up.

### Critical Power Note (Mag Switch)
To achieve near-zero power consumption while the door is locked (idle state):
*   **Switch Type:** SPDT (Changeover) using **Normally Closed (N/C)** contacts.
*   **Behavior:** When the magnet is present (Door Closed), the switch physically **OPENS** the circuit.
*   **Result:** **0mA** current flow through the wake pin while the device is sleeping and the door is closed.

## 4. Administration & Fingerprint Registry (Web Interface)

The system includes a self-hosted web interface for managing fingerprints, served directly by the AtomS3R.

### Activation (Admin Mode)
*   **Trigger:** Hold the **Main Button (BtnA)** while powering on or resetting the device.
*   **Indication:** Screen displays " ADMIN MODE " and the IP address.
*   **Timeout:** If no client connects within 2 minutes, the device restarts into Normal Operation.

### Web Interface Features
1.  **Dashboard:** Shows device status and stored fingerprint count.
2.  **Enroll New Finger:**
    *   **Input:** "User Name" field (to label the fingerprint).
    *   UI prompts user to place a finger on the sensor.
    *   Real-time feedback on the web page (e.g., "Remove finger", "Place again").
    *   Assigns a new ID automatically and maps it to the Name.
3.  **Manage Database:**
    *   List all stored Fingerprint IDs.
    *   Delete specific IDs (revoke access).
    *   "Erase All" function (Factory Reset).
4.  **Test/Control:**
    *   Manual buttons to triggers Open/Close motor actions for testing.

