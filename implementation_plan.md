# Implementation Plan - Atomic Motion Fingerprint Lock

This plan outlines the steps to build the firmware for the Atomic Motion Fingerprint Lock, including the core locking logic, deep sleep power management, and the web-based registration interface.

## User Review Required
> [!IMPORTANT]
> **Library Dependencies:** Verify availability of compatible libraries for the specific Capacitive Fingerprint Sensor (likely Adafruit_Fingerprint) and the Atomic Motion Base within the PlatformIO/Arduino environment.

## Proposed Changes

### 1. Project Structure & Environment
#### [Modify] Build System: Arduino CLI
*   **Goal:** Use the existing `arduino-cli` for building the project.
*   **Dependencies (to be installed via CLI):**
    *   `M5AtomS3`
    *   `M5AtomicMotion`
    *   `Adafruit Fingerprint Sensor Library`
    *   `M5Unified` (often required dependency)
*   **Configuration:**
    *   Core: `esp32:esp32` (Confirmed Installed).
    *   Target Hardware: `m5stack:esp32:m5stack_atoms3` (or similar S3 variant).
    *   **Action:** Run `arduino-cli lib install "M5AtomS3" "M5AtomicMotion" "Adafruit Fingerprint Sensor Library"`

### 2. Modular Firmware Structure

#### [MODIFY] [BiometricLock.ino](file:///c:/Users/leviwipf/sources/Atomic%20Motion/BiometricLock.ino)
*   **Action:** Rename `AtomicMotion.ino` to `BiometricLock.ino`.
*   **Role:** Main Entry Point & Orchestrator.
*   **Logic:**
    *   `setup()`: hardware init, check boot button for Admin Mode.
    *   `loop()`: executes the main state machine (Sleep -> Wake -> Action).

#### [NEW] [MotorController.h/.cpp](file:///c:/Users/leviwipf/sources/Atomic%20Motion/MotorController.h)
*   **Role:** Manages Servo movement and Safety.
*   **Features:**
    *   `open()`, `close()`, `stop()`.
    *   `monitorStall()`: Reads INA226 current, returns true if stalled.
    *   Manages Atomic Motion Base communication.

#### [NEW] [FingerprintManager.h/.cpp](file:///c:/Users/leviwipf/sources/Atomic%20Motion/FingerprintManager.h)
*   **Role:** Handles all biometric sensor operations.
*   **Features:**
    *   `begin()`: serial setup.
    *   `scanFinger()`: returns ID or error.
    *   `enrollFinger(id)`: handles the multi-step enrollment process.
    *   `deleteFinger(id)`: removes from sensor database.

#### [NEW] [PowerManager.h/.cpp](file:///c:/Users/leviwipf/sources/Atomic%20Motion/PowerManager.h)
*   **Role:** Deep Sleep and Wake-up configuration.
*   **Features:**
    *   `configureWake(doorOpen)`: Sets wake on G8 LOW (if closed) or HIGH (if open).
    *   `enterDeepSleep()`: Shuts down peripherals and sleeps ESP32.

#### [NEW] [WebAdmin.h/.cpp](file:///c:/Users/leviwipf/sources/Atomic%20Motion/WebAdmin.h)
*   **Role:** Web Server backend logic.
*   **Features:**
    *   Handles HTTP requests and API endpoints (`/api/status`, `/api/enroll`, `/api/delete`).
    *   **Data Storage:** Syncs `UserName` <-> `ID` using `Preferences`.

#### [NEW] [index_html.h](file:///c:/Users/leviwipf/sources/Atomic%20Motion/index_html.h)
*   **Role:** Stores the specific HTML/CSS/JS content.
*   **Implementation:** Contains the compressed HTML string (likely `const char index_html[] PROGMEM = "..."`).
*   **Benefit:** Keeps the C++ logic in `WebAdmin.cpp` clean and readable.

#### [MODIFY] [BiometricLock.ino](file:///c:/Users/leviwipf/sources/Atomic%20Motion/BiometricLock.ino)
*   **Action:** Implement `RTC_DATA_ATTR` state tracking.
*   **New Logic:**
    *   `RTC_DATA_ATTR bool isLocked`: Preserves lock state across deep sleep.
    *   `STATE_ACTION_OPEN`: Sets `isLocked = false`.
    *   `STATE_ACTION_CLOSE`: Checks `!isLocked` before running motor. Sets `isLocked = true` on success.
    *   **Cold Boot:** Initializes `isLocked = false` to ensure first-time locking safety.

## Verification Plan

### Automated Tests
*   **Build Verification:** `pio run` to ensure all libraries link correctly.

### Manual Verification
1.  **Boot & Admin Mode:**
    *   Hold BtnA on boot -> Verify "ADMIN MODE" on screen.
    *   Connect to Wi-Fi/IP -> Load Webpage.
2.  **Fingerprint Management:**
    *   Click "Enroll" on Web -> Place finger -> Verify Success message.
    *   Click "Delete" -> Verify ID removal.
3.  **Lock Operation:**
    *   **Open:** Touch sensor with enrolled finger -> Servo spins Open -> Stops on Stall.
    *   **Close:** Simulate Door Close (Magnet to Switch) -> Servo spins Close -> Stops on Stall.
4.  **Power/Sleep:**
    *   Verify device enters Deep Sleep after actions.
    *   Verify wake-up from both Fingerprint (G7) and Door Open (G8).
