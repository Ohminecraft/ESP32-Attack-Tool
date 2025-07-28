#include "utilsheader.h"

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

const char* generateRandomName() {
		const char* charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		int len = rand() % 10 + 1; // Generate a random length between 1 and 10
		char* randomName = (char*)malloc((len + 1) * sizeof(char)); // Allocate memory for the random name
		for (int i = 0; i < len; ++i) {
			randomName[i] = charset[rand() % strlen(charset)]; // Select random characters from the charset
		}
		randomName[len] = '\0'; // Null-terminate the string
		return randomName;
}

bool checkLeftMemory() {
	size_t freeHeap = esp_get_free_heap_size();
	Serial.printf("[INFO] Free heap: %d bytes\n", freeHeap);
	
	if (freeHeap <= MEM_LOWER_LIM + 2000) {
		Serial.println("[WARN] Warning: Low memory!");
		Serial.println("[WARN] Please restart the device or free up memory.");
		return true;
	}
	else if (freeHeap <= MEM_LOWER_LIM) {
		Serial.println("[WARN] Warning: Critical low memory!");
		return false;
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

int getEncoderDirection() {
    long currentPos = encoder.getCount();
    int direction = 0;
    
    if (abs(currentPos) >= ENCODER_THRESHOLD) {
        if (currentPos > 0) {
            direction = 1;
        } else {
            direction = -1;
        }
        encoder.clearCount();
    }
    
    return direction;
}

void handleInputs() {
	static unsigned long tm = millis();  // debauce for buttons
    static unsigned long tm2 = millis(); // delay between Select and encoder (avoid missclick)
    static int encoderDir = 0;
	bool sel = HIGH;
	encoderDir = getEncoderDirection();

	if (millis() - tm > 300) {
		sel = digitalRead(ENC_BTN);
	}

	if (encoderDir > 0) {
		nextPress = true;
		tm2 = millis();
	}

	if (encoderDir < 0) {
		prevPress = true;
		tm2 = millis();
	}

	if (sel == LOW && millis() - tm2 > 300) {
		encoderDir = 0;
		selPress = true;
		tm = millis();
	}
}

TaskHandle_t xHandle;