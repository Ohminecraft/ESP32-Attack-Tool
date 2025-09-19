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
    Serial.println("[INFO] Starting ESP32 Attack Tool v2...");
    esp_log_level_set("*", ESP_LOG_NONE); // Disable all ESP logs
    
    espatsettings.loadSettings();
    
    // Initialize status LED
    pinMode(espatsettings.statusLedPin, OUTPUT);
    digitalWrite(espatsettings.statusLedPin, HIGH);
    
    // Initialize menu system
    if (espatsettings.sdcardCsPin > 0) {
		pinMode(espatsettings.nrfCsPin, OUTPUT);
		digitalWrite(espatsettings.nrfCsPin, HIGH);
		pinMode(espatsettings.sdcardCsPin, OUTPUT);
		digitalWrite(espatsettings.sdcardCsPin, HIGH);
	}
    menuinit();
    
    Serial.println("[INFO] System ready!");
    digitalWrite(espatsettings.statusLedPin, LOW);
    Serial.println("[SYSTEM_WELCOME] Welcome to ESP32 Attack Tool v2!");

}

void loop() {
    menuloop();
    vTaskDelay(1 / portTICK_PERIOD_MS); // Small delay to prevent excessive CPU usage
}