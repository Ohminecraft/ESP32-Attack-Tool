#include "communication.h"

// Using AmebaD SDK

String readMasterResponse() {
    if (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) {
            return line;
        }
    }
    return "";
    delay(10);
}

