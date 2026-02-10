#include "WebAdmin.h"
#include "MotorController.h"

extern MotorController motor; // Access global instance or pass via constructor. 
// Passing via constructor is better but requires changing BiometricLock.ino too.
// For now, using extern for simplicity as they are singletons effectively.

WebAdmin::WebAdmin(FingerprintManager* fpManager) : server(80), _fpManager(fpManager) {
}

void WebAdmin::begin() {
    preferences.begin("biolock", false);
    
    server.on("/", HTTP_GET, std::bind(&WebAdmin::handleRoot, this));
    server.on("/api/list", HTTP_GET, std::bind(&WebAdmin::handleList, this));
    server.on("/api/enroll", HTTP_GET, std::bind(&WebAdmin::handleEnroll, this)); // Using GET for simplicity with params
    server.on("/api/delete", HTTP_GET, std::bind(&WebAdmin::handleDelete, this));
    server.on("/api/status", HTTP_GET, std::bind(&WebAdmin::handleStatus, this));
    server.on("/api/motor", HTTP_GET, std::bind(&WebAdmin::handleMotor, this));
    
    server.begin();
    Serial.println("Web Server started");
}

void WebAdmin::handleMotor() {
    if (!server.hasArg("action")) {
        server.send(400, "text/plain", "Missing action");
        return;
    }
    String action = server.arg("action");
    
    motor.resetMaxCurrent(); // Reset Max tracking for this run

    String responseMsg = "";
    if (action == "open") {
        motor.open();
        responseMsg = "Motor Opening";
    } else if (action == "close") {
        motor.close();
        responseMsg = "Motor Closing";
    } else if (action == "stop") {
        motor.stop();
        server.send(200, "text/plain", "Motor Stopped");
        return;
    } else {
        server.send(400, "text/plain", "Invalid action");
        return;
    }

    // Run loop until motor stops (Timeout logic in MotorController handles the stop)
    // We poll motor.update() to keep it alive and sampling
    Serial.println("[MOTOR TEST] Running...");
    
    while (motor.isRunning()) {
        motor.update(); // Checks timeout and samples current
        delay(10);      // Small delay to yield
    }
    
    // Motor has stopped, collect stats
    float startCurrent = motor.getStartingCurrent();
    float avgCurrent = motor.getAverageCurrent();
    float maxCurrent = motor.getMaxCurrent();
    
    Serial.printf("[MOTOR TEST] Start: %.2fmA, Avg: %.2fmA, Max: %.2fmA\n", startCurrent, avgCurrent, maxCurrent);
    
    responseMsg += " (Start: " + String(startCurrent, 1) + "mA, Avg: " + String(avgCurrent, 1) + "mA, Max: " + String(maxCurrent, 1) + "mA)";
    server.send(200, "text/plain", responseMsg);
}

void WebAdmin::handleClient() {
    server.handleClient();
}

void WebAdmin::handleRoot() {
    server.send(200, "text/html", index_html);
}

void WebAdmin::handleList() {
    // Return JSON list of {id, name}
    // We iterate through known storage on the sensor. 
    // Optimization: FingerprintManager has getTemplateCount, but usually no easy "list all IDs" without scanning e.g. 0-127.
    // For now, we will scan the first 127 locations. A bit slow but standard for these sensors.
    
    String json = "[";
    bool first = true;
    
    // Check valid IDs from 1 to 127 (Capacity of typical sensors)
    // NOTE: This might be slow (127 serial checks). 
    // Alternative: Just trust our Preferences list? No, should be in sync.
    // Let's rely on FingerprintManager having a verification method, or just NVS for now?
    // Correct way: Load from Sensor. 
    // For V1 Speed: Let's assume NVS is the source of truth for NAMES, but we should verify ID exists.
    
    // Better Approach: Iterate ids 1-20 (limit for speed) or add a 'getEnrolledIDs' to FingerprintManager later.
    // Using a safe loop for now.
    for (int i = 1; i <= 20; i++) {
        // We'd ideally check _fpManager->checkId(i) here.
        // Assuming if name exists in prefs, it's likely valid. 
        // Or we check sensor.loadModel(i). 
        
        String name = getNameForId(i);
        if (name.length() > 0) {
            if (!first) json += ",";
            json += "{\"id\":" + String(i) + ",\"name\":\"" + name + "\"}";
            first = false;
        }
    }
    json += "]";
    server.send(200, "application/json", json);
}

void WebAdmin::handleEnroll() {
    if (!server.hasArg("name")) {
        server.send(400, "text/plain", "Missing name");
        return;
    }
    String name = server.arg("name");
    
    // Finds first empty ID
    int id = -1;
    for (int i = 1; i <= 127; i++) {
        if (getNameForId(i).length() == 0) {
            id = i;
            break;
        }
    }
    
    if (id == -1) {
        server.send(500, "text/plain", "Database Full");
        return;
    }
    
    // Start Streaming Response
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/plain", ""); 
    
    // Lambda to stream logs
    auto logger = [&](const char* msg) {
        // Send as chunk
        server.sendContent(String(msg) + "\n");
    };

    logger(("Starting enrollment for " + name + " (ID #" + String(id) + ")").c_str());

    // Execute Enrollment (BLOCKING)
    int result = _fpManager->enrollFinger(id, logger);
    
    if (result == 1) {
        saveNameForId(id, name);
        logger("ENROLLMENT COMPLETE");
    } else {
        logger("ENROLLMENT FAILED");
    }
    
    server.client().stop(); // Close connection
}

void WebAdmin::handleDelete() {
    if (server.hasArg("all")) {
        _fpManager->deleteAll();
        preferences.clear();
        server.send(200, "text/plain", "Factory Reset Complete");
        return;
    }
    
    if (server.hasArg("id")) {
        int id = server.arg("id").toInt();
        if (_fpManager->deleteFinger(id)) {
            removeNameForId(id);
            server.send(200, "text/plain", "Deleted ID#" + String(id));
        } else {
            // Even if sensor fails (maybe empty), remove from prefs
            removeNameForId(id);
            server.send(200, "text/plain", "Removed ID#" + String(id) + " (Sensor mismatch handled)");
        }
    }
}


void WebAdmin::handleStatus() {
    float voltage = motor.getBusVoltage();
    float current = motor.getCurrentReading();
    // Assuming isDoorOpen is global or we fetch it. 
    // Let's rely on reading the pin directly or extern.
    // Ideally WebAdmin should have access to system state.
    // For now, let's just return power metrics.
    
    String json = "{";
    json += "\"voltage\":" + String(voltage, 2) + ",";
    json += "\"current\":" + String(current, 2);
    json += "}";
    
    server.send(200, "application/json", json);
}

String WebAdmin::getNameForId(int id) {
    return preferences.getString(String(id).c_str(), "");
}

void WebAdmin::saveNameForId(int id, String name) {
    preferences.putString(String(id).c_str(), name);
}

void WebAdmin::removeNameForId(int id) {
    preferences.remove(String(id).c_str());
}
