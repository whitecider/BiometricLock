#line 1 "C:\\Users\\leviwipf\\sources\\BiometricLock\\PowerManager.h"
#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "HardwareConfig.h"

class PowerManager {
public:
    void begin();
    
    // Configures wake sources based on current door state logic
    // isDoorOpen: TRUE if the door is currently detected as OPEN
    void enterDeepSleep(bool isDoorOpen);

private:
    void printWakeReason();
};

#endif
