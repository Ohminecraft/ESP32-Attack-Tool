#include "utilsheader.h"
#include "configs.h"

/*
	* utils.cpp Version 2.0 (with Claude Help)
	* /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
	* Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
	* This file contains utility functions for the ESP32 Attack Tool.
	* It includes functions for generating random names, checking memory, generating random MAC addresses,
	* and setting the base MAC address.
	* It also includes a function to check available memory and print warnings if memory is low.
	* It is used to ensure the system has enough resources before starting attacks.
*/

String generateRandomName() {
    const char* charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int len = rand() % 8 + 3; // Limit length to 3-10 characters
    String randomName = "";
    
    // Reserve memory upfront to prevent fragmentation
    randomName.reserve(len + 1);
    
    for (int i = 0; i < len; i++) {
        randomName += charset[rand() % strlen(charset)];
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

String macToString(uint8_t macAddr[6]) {
	char macStr[18]; // 17 characters for "XX:XX:XX:XX:XX:XX" + 1 null terminator
	snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
	  macAddr[0], macAddr[1], macAddr[2], 
	  macAddr[3], macAddr[4], macAddr[5]);
	return String(macStr);
}
	
void generateRandomMac(uint8_t* mac) {
	esp_fill_random(mac, 6); // Fill MAC address with random bytes
    mac[0] &= 0xFE; // Unicast MAC address (least significant bit of the first byte should be 0)
    mac[0] |= 0x02; // Locally administered MAC address (set the second least significant bit)
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

volatile bool nextPress = false;
volatile bool prevPress = false;
volatile bool selPress = false;

#ifdef USING_ENCODER
// Encoder Object
RotaryEncoder *encoder = nullptr;

IRAM_ATTR void checkPosition() {
    encoder->tick(); // just call tick() to check the state.
}

void handleInputs() {
	static unsigned long tm = millis();  // debauce for buttons
    static unsigned long tm2 = millis(); // delay between Select and encoder (avoid missclick)
	static int encoderDir = 0; // Encoder direction
	encoderDir = (int)encoder->getDirection();
	bool sel = HIGH;

	if (millis() - tm > 300) {
		sel = digitalRead(SEL_BTN);
	}

	if (encoderDir < 0) {
		encoderDir = 0;
		nextPress = true;
		tm2 = millis();
	}

	if (encoderDir > 0) {
		encoderDir = 0;
		prevPress = true;
		tm2 = millis();
	}

	if (sel == LOW && millis() - tm2 > 300) {
		encoderDir = 0;
		selPress = true;
		tm = millis();
	}
}
#elif defined(USING_BUTTON)
void handleInputs() {
	static unsigned long tm = 0;
    if (millis() - tm < 300) return;

    bool leftPressed = (digitalRead(LEFT_BTN) == LOW);
    bool selPressed = (digitalRead(SEL_BTN) == LOW);
    bool rightPressed = (digitalRead(RIGHT_BTN) == LOW);

    if (leftPressed || selPressed || rightPressed) tm = millis();

	selPress = selPressed;
	nextPress = rightPressed;
	prevPress = leftPressed;
}
#endif

TaskHandle_t xHandle;