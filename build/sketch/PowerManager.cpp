#line 1 "C:\\Users\\leviwipf\\sources\\BiometricLock\\PowerManager.cpp"
#include "PowerManager.h"

#include <driver/rtc_io.h>

void PowerManager::begin() {
    // Determine wake reason for debug
    printWakeReason();
    
    // Ensure pins are setup for reading
    pinMode(PIN_FP_WAKE, INPUT_PULLUP); // Active LOW
    pinMode(PIN_DOOR_SWITCH, INPUT_PULLUP);     // Mag switch logic (NC -> Open when Closed -> pulled HIGH)
}

void PowerManager::enterDeepSleep(bool isDoorOpen) {
    Serial.println("Entering Deep Sleep...");
    
    // 1. Wake on Fingerprint (Always Active)
    // Sensor pulls G7 HIGH when touched
    // IMPORTANT: Enable RTC Pull-down to prevent floating trigger
    rtc_gpio_pullup_en((gpio_num_t)PIN_FP_WAKE);
    rtc_gpio_pulldown_dis((gpio_num_t)PIN_FP_WAKE);
    esp_sleep_enable_ext1_wakeup(1ULL << PIN_FP_WAKE, ESP_EXT1_WAKEUP_ALL_LOW);

    // 2. Wake on Door State Change (Dynamic)
    // SPDT Switch Re-Wired (Power Optimized):
    // - Door Closed (Magnet Present) -> Switch OPEN   -> Pin HIGH (Internal Pullup) -> Current = 0
    // - Door Open (Magnet Away)      -> Switch CLOSED -> Pin LOW (Grounded)       -> Current = 70uA
    
    if (!isDoorOpen) {
        // Door is currently CLOSED (Pin HIGH).
        // Wake when Door OPENS (Pin goes LOW).
        
        // Keep Pull-up enabled so it stays HIGH until switch closes (grounds it)
        rtc_gpio_pulldown_dis((gpio_num_t)PIN_DOOR_SWITCH);
        rtc_gpio_pullup_en((gpio_num_t)PIN_DOOR_SWITCH);
        
        esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_DOOR_SWITCH, 0); // Wake on LOW
        Serial.println("Wake Config: Door is CLOSED (High) -> Wake on LOW (Door Open)");
    } else {
        // Door is currently OPEN (Pin LOW).
        // Wake when Door CLOSES (Pin goes HIGH).
        
        // Keep Pull-up enabled so it goes HIGH when switch opens
        rtc_gpio_pulldown_dis((gpio_num_t)PIN_DOOR_SWITCH);
        rtc_gpio_pullup_en((gpio_num_t)PIN_DOOR_SWITCH);
        
        esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_DOOR_SWITCH, 1); // Wake on HIGH
        Serial.println("Wake Config: Door is OPEN (Low) -> Wake on HIGH (Door Close)");
    }

    Serial.flush(); 
    esp_deep_sleep_start();
}

void PowerManager::printWakeReason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch(wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup: EXT0 (Door)"); break;
        case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup: EXT1 (Fingerprint)"); break;
        case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wakeup: Timer"); break;
        default: 
            Serial.printf("Wakeup: Reset/Other (%d) -> ", wakeup_reason);
            
            // Check Reset Reason since it wasn't a Sleep Wake
            esp_reset_reason_t r = esp_reset_reason();
            switch (r) {
                case ESP_RST_POWERON: Serial.println("Reset: Power On"); break;
                case ESP_RST_SW:      Serial.println("Reset: Software/Restart"); break;
                case ESP_RST_PANIC:   Serial.println("Reset: Exception/Panic"); break;
                case ESP_RST_INT_WDT: Serial.println("Reset: Watchdog (Interrupt)"); break;
                case ESP_RST_TASK_WDT:Serial.println("Reset: Watchdog (Task)"); break;
                case ESP_RST_BROWNOUT:Serial.println("Reset: Brownout"); break;
                case ESP_RST_DEEPSLEEP:Serial.println("Reset: Deep Sleep (Unexpected)"); break;
                default: Serial.printf("Reset: Unknown (%d)\n", r); break;
            }
            break;
    }
}
