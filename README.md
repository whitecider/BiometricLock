# Biometric Lock - Atomic Motion & R503

A robust biometric locking system built with the **M5Stack Atomic Motion Base**, **AtomS3R**, and an **R503 / Adafruit 4651 Fingerprint Sensor**. This project provides secure access control with a web-based management interface, motor control for locking mechanisms, and power-efficient operation.

## Feature Overview

*   **Biometric Access**: Fast and secure fingerprint recognition using the Grow R503 / Adafruit 4651 sensor.
*   **Web Management**: Built-in web server for easy fingerprint enrollment, deletion, and file management.
*   **Motor Control**: Precise motor control using the Atomic Motion Base to drive locking mechanisms.
*   **Smart Power Management**: Deep sleep support to conserve battery life, with wake-up sources from both the fingerprint sensor and door position switch.
*   **Stall Detection**: Integrated current monitoring to detect motor stalls and prevent hardware damage.
*   **Status Indicators**: Visual feedback via the AtomS3R's built-in LCD (Lock status, WiFi IP, Battery level).

## Hardware Components

*   **Controller**: M5Stack AtomS3R (ESP32-S3)
*   **Motor Driver**: M5Stack Atomic Motion Base (STM32G030F6P6 + 4x DRV8825 equivalent)
*   **Motor**: GA12-N20 DC 6V 60RPM Gear Motor
*   **Fingerprint Sensor**: Grow R503 / Adafruit 4651 (Capacitive)
*   **Power**: 3.7V LiPo Battery
*   **Sensors**: Magnetic Reed Switch (Door position)

## Documentation

*   [BOM and Connections](BOM%20and%20Connections.md): Detailed wiring guide and pinout information.
*   [Design Intent and Logic](Design%20Intent%20and%20Logic.md): Explanation of the system's core logic, state machines, and power states.

## Getting Started

1.  **Hardware Setup**: Follow the [wiring guide](BOM%20and%20Connections.md) to connect the sensor and motor to the Atomic Motion Base.
2.  **Firmware**: Flash the `BiometricLock.ino` to the AtomS3R using the Arduino IDE or PlatformIO.
3.  **Enrollment**:
    *   Connect to the device's WiFi point (IP displayed on screen).
    *   Navigate to the web interface.
    *   Use the "Enroll" feature to register new fingerprints.

## License

This project is licensed under the [MIT License](LICENSE).

## References

*   [R503 Fingerprint Module User Manual](Docs/4651_R503%20fingerprint%20module%20user%20manual.pdf)
*   [Technical Datasheet](Docs/0900766b80027e1f.pdf)
