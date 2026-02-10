#line 1 "C:\\Users\\leviwipf\\sources\\BiometricLock\\WebAdmin.h"
#ifndef WEB_ADMIN_H
#define WEB_ADMIN_H

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include "FingerprintManager.h"
#include "index_html.h"

class WebAdmin {
public:
    WebAdmin(FingerprintManager* fpManager);
    void begin();
    void handleClient();

private:
    WebServer server;
    FingerprintManager* _fpManager;
    Preferences preferences;

    void handleRoot();
    void handleList();
    void handleEnroll();
    void handleDelete();
    void handleStatus();
    void handleMotor();
    
    String getNameForId(int id);
    void saveNameForId(int id, String name);
    void removeNameForId(int id);
};

#endif
