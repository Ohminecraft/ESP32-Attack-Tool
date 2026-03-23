#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_log.h"

#include "core/displayheader.h"
#include "core/menuheader.h"

/*
    * main.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

void setup() {
    Serial.begin(115200);
    Serial.println(" ");
    Serial.println("[INFO] Starting ESP32 Attack Tool...");

    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH); // Power on the device
    Serial.println("[INFO] Powering on the device...");
    
    espatsettings.loadSettings();
    
    // Initialize status LED
    #ifndef BUILTIN_RGB_LED
        pinMode(espatsettings.statusLedPin, OUTPUT);
        digitalWrite(espatsettings.statusLedPin, HIGH);
    #else
        pixels.setPin(espatsettings.statusLedPin);
        pixels.begin();
        pixels.setBrightness(50);
        pixels.setPixelColor(0, pixels.Color(255, 255, 255));
        pixels.show();
        Serial.println("[INFO] RGB LED Enable");
    #endif
    
    // Initialize menu system
    #ifndef BOARD_ESP32_C5_DEVKIT_C1
    if (espatsettings.sdcardCsPin > 0) {
		pinMode(espatsettings.nrfCsPin, OUTPUT);
		digitalWrite(espatsettings.nrfCsPin, HIGH);
		pinMode(espatsettings.sdcardCsPin, OUTPUT);
		digitalWrite(espatsettings.sdcardCsPin, HIGH);
	}
    #endif
    menuinit();
    
    Serial.println("[INFO] System ready!");
    #ifndef BUILTIN_RGB_LED
        digitalWrite(espatsettings.statusLedPin, LOW);
    #else
        pixels.clear();
        pixels.show();
    #endif
    Serial.println("[SYSTEM_WELCOME] Welcome to ESP32 Attack Tool!");

}

void loop() {
    menuloop();
    vTaskDelay(1 / portTICK_PERIOD_MS); // Small delay to prevent excessive CPU usage
}