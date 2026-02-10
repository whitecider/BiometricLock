# Atomic Motion Wiring Guide

This document details the physical connection and wiring for the M5AtomS3R system.

## 1. Bill of Materials (BOM)

| Component | Description | Source / Link |
| :--- | :--- | :--- |
| **Controller** | M5Stack AtomS3R | [M5Stack Product Page](https://shop.m5stack.com/products/atoms3r-esp32-s3-dev-kit) |
| **Motion Base** | Atomic Motion Base v1.2 | [M5Stack Product Page](https://shop.m5stack.com/products/atomic-motion-base-v1-2) |
| **Fingerprint Sensor** | Capacitive Fingerprint Sensor | [Mouser Electronics](https://www.mouser.co.uk/ProductDetail/Adafruit/4651?qs=hWgE7mdIu5QeotkqGbcDDQ%3D%3D) |
| **Mag Switch** | ASSEMtech S1367 (SPDT) | [RS Online](https://uk.rs-online.com/web/p/magnetic-proximity-switches/2897806) |
| **Magnet** | Initial Magnet | [RS Online](https://uk.rs-online.com/web/p/sensor-accessories/2897812) |
| **Servo** | M5Stack 360 Servo | [M5Stack Product Page](https://shop.m5stack.com/products/m5stack-servo-kit-360) |

## 2. Port Reference (Atomic Motion Base)

| Port Name | Color | Location | Pins Used |
| :--- | :--- | :--- | :--- |
| **Port B** | Black | Base Side | **G7, G8** |
| **Port C** | Blue | Base Side | **G5, G6** |
| **Port S1** | - | Base Servo Header | **Servo Ch 1** |

---

## 3. Wiring Instructions

### A. Fingerprint Sensor
**Requires splitting cable to two ports.**

| Sensor Wire | Function | Connect To | Target Port | Pin on Port |
| :--- | :--- | :--- | :--- | :--- |
| **Yellow** | TX | Atomic Motion | **Port C** (Blue) | White Pin (**G6**) |
| **Brown** | RX | Atomic Motion | **Port C** (Blue) | Yellow Pin (**G5**) |
| **Red** | VCC | Atomic Motion | **Port C** (Blue) | Red Pin (**VCC**) |
| **Black** | GND | Atomic Motion | **Port C** (Blue) | Black Pin (**GND**) |
| **Blue** | WAKE | Atomic Motion | **Port B** (Black) | White Pin (**G7**) |
| **White** | Touch Pwr | Atomic Motion | **Port B** (Black) | Red Pin (**VCC**) |

### B. Mag Switch (ASSEMtech S1367)
**Connect to Port B (Sharing with Fingerprint WAKE/Touch Pwr).**

| Switch Wire | Function | Connect To | Target Port | Pin on Port |
| :--- | :--- | :--- | :--- | :--- |
| **White** | Common | Atomic Motion | **Port B** (Black) | Black Pin (**GND**) |
| **Brown** | N/C | Atomic Motion | **Port B** (Black) | Yellow Pin (**G8**) |
| **Green** | N/O | **(Do Not Connect)** | - | - |

*(Note: Insulate the unused Green wire)*

### C. Servo
**Connect to Servo Interface.**

| Component | Connect To | Target Port |
| :--- | :--- | :--- |
| **M5Stack 360 Servo** | Atomic Motion | **S1** |
