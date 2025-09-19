#include "utils.h"

bool stringToMac(const String& macStr, uint8_t macAddr[6]) {
    // Ensure the input string is in the format "XX:XX:XX:XX:XX:XX"
    if (macStr.length() != 17) {
        Serial.println("[ERROR] Invalid MAC address format");
        return false;
    }

    // Parse the MAC address string and fill the uint8_t array
    for (int i = 0; i < 6; i++) {
        macAddr[i] = (uint8_t)strtol(macStr.substring(i * 3, i * 3 + 2).c_str(), nullptr, 16);
    }
    
    return true;
}

String macToString(uint8_t macAddr[6]) {
	char macStr[18]; // 17 characters for "XX:XX:XX:XX:XX:XX" + 1 null terminator
	snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
	  macAddr[0], macAddr[1], macAddr[2], 
	  macAddr[3], macAddr[4], macAddr[5]);
	return String(macStr);
}

int splitStringToVector(String str, char delimiter, std::vector<String>& result) {
    result.clear();
    int count = 0;
    int pos = 0;
    
    if (str.length() == 0) {
        return 0;
    }
    
    while (pos < str.length()) {
        int nextDelim = str.indexOf(delimiter, pos);
        String element;
        
        if (nextDelim == -1) {
            // Last element
            element = str.substring(pos);
        } else {
            // Regular element  
            element = str.substring(pos, nextDelim);
        }
        
        element.trim();
        if (element.length() > 0) {
            result.push_back(element);
            count++;
            Serial.println("DEBUG: Split element[" + String(count-1) + "] = '" + element + "'");
        }
        
        if (nextDelim == -1) break;
        pos = nextDelim + 1;
    }
    
    return count;
}