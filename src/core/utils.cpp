#include "utilsheader.h"
#include "configs.h"

/*
	* utils.cpp Version 2.0 (with Claude Help)
	* /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
	* Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
	* Licensed under the MIT License.
*/

String generateRandomName() {
	int len = rand() % 10 + 1; // Limit length to 1-10 characters
	String randomName = "";
	
	// Reserve memory upfront to prevent fragmentation
	randomName.reserve(len + 1);
	
	for (int i = 0; i < len; i++) {
		randomName += alfa[rand() % alfa.length()];
	}
	
	return randomName;
}

uint32_t getHeap(uint8_t type) {
	if (type == GET_TOTAL_HEAP) return ESP.getHeapSize();
	else if (type == GET_FREE_HEAP) return ESP.getFreeHeap();
	else if (type == GET_USED_HEAP) return (ESP.getHeapSize() - ESP.getFreeHeap());
	else if (type == GET_USED_HEAP_PERCENT) {
		size_t freeHeap = ESP.getFreeHeap();
		size_t totalHeap = ESP.getHeapSize();
		if (totalHeap == 0) return 0; // Avoid division by zero
		return ((totalHeap - freeHeap) * 100) / totalHeap; // Return used heap percentage
	}
	else return 0; // Invalid type
}

bool checkLeftMemory() {
	Serial.printf("[INFO] Free heap: %d bytes\n", String(getHeap(GET_FREE_HEAP)).toInt());
	Serial.printf("[INFO] Used heap: %d bytes\n", String(getHeap(GET_USED_HEAP)).toInt());
	Serial.printf("[INFO] Used: %d%%\n", String(getHeap(GET_USED_HEAP_PERCENT)).toInt());
	
	if (getHeap(GET_FREE_HEAP) <= MEM_LOWER_LIM + 2000) {
		Serial.println("[WARN] Warning: Low memory!");
		Serial.println("[WARN] Please restart the device or free up memory.");
		return true;
	}
	else if (getHeap(GET_FREE_HEAP) <= MEM_LOWER_LIM) {
		Serial.println("[WARN] Warning: Critical low memory!");
		return false;
	}
	return true;
}

void getMAC(char *addr, uint8_t* data, uint16_t offset) {
	sprintf(addr, "%02x:%02x:%02x:%02x:%02x:%02x", data[offset+0], data[offset+1], data[offset+2], data[offset+3], data[offset+4], data[offset+5]);
}

void generateRandomMac(uint8_t* mac) {
	// Set the locally administered bit and unicast bit for the first byte
	mac[0] = 0x02; // The locally administered bit is the second least significant bit
  
	// Generate the rest of the MAC address
	for (int i = 1; i < 6; i++) {
	  mac[i] = random(0, 255);
	}
}

void stringToMac(const String& macStr, uint8_t macAddr[6]) {
    // Ensure the input string is in the format "XX:XX:XX:XX:XX:XX"
    if (macStr.length() != 17) {
        Serial.println("[ERROR] Invalid MAC address format");
        return;
    }

    // Parse the MAC address string and fill the uint8_t array
    for (int i = 0; i < 6; i++) {
        macAddr[i] = (uint8_t)strtol(macStr.substring(i * 3, i * 3 + 2).c_str(), nullptr, 16);
    }
}

String macToString(uint8_t macAddr[6]) {
	char macStr[18]; // 17 characters for "XX:XX:XX:XX:XX:XX" + 1 null terminator
	snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
	  macAddr[0], macAddr[1], macAddr[2], 
	  macAddr[3], macAddr[4], macAddr[5]);
	return String(macStr);
}
	
void setBaseMacAddress(uint8_t macAddr[6]) {
	esp_err_t err = esp_base_mac_addr_set(macAddr);
	
	// Check for success or handle errors
	if (err == ESP_OK) {
		//Serial.println("MAC address set successfully");
		return;
	} else if (err == ESP_ERR_INVALID_ARG) {
		Serial.println("[ERROR] Error: Invalid MAC address argument.");
	} else {
		Serial.printf("[ERROR] Error: Failed to set MAC address. Code: %d\n", err);
	}
}

String uint32ToString(uint32_t value) {
    char buffer[12] = {0}; // 8 hex digits + 3 spaces + 1 null terminator
    snprintf(
        buffer,
        sizeof(buffer),
        "%02X %02X %02X %02X",
        value & 0xFF,
        (value >> 8) & 0xFF,
        (value >> 16) & 0xFF,
        (value >> 24) & 0xFF
    );
    return String(buffer);
}

String uint32ToStringInverted(uint32_t value) {
    char buffer[12] = {0}; // 8 hex digits + 3 spaces + 1 null terminator
    snprintf(
        buffer,
        sizeof(buffer),
        "%02X %02X %02X %02X",
        (value >> 24) & 0xFF,
        (value >> 16) & 0xFF,
        (value >> 8) & 0xFF,
        value & 0xFF
    );
    return String(buffer);
}

uint32_t swap32(uint32_t value) {
    return ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) | ((value & 0x00FF0000) >> 8) |
           ((value & 0xFF000000) >> 24);
}

uint8_t hexCharToDecimal(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return 0;
}

volatile bool nextPress = false;
volatile bool prevPress = false;
volatile bool selPress = false;
volatile bool anykeyPress = false;
bool ble_initialized = false; // BLE Initialized Flag
bool wifi_initialized = false; // WiFi Initialized Flag
bool wifi_connected = false;
bool first_scan = true;
bool rtl8720dn_ready = false; // RTL8720DN Ready Flag
bool low_memory_warning = false; // Low Memory Warning Flag

// Encoder Object
RotaryEncoder *encoder = nullptr;

IRAM_ATTR void checkPosition() {
    encoder->tick(); // just call tick() to check the state.
}

TaskHandle_t xHandle;

SPIClass *SDCardSPI;