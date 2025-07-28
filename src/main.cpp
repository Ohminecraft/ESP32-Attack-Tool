#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"

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
    
    // Initialize status LED
    pinMode(STA_LED, OUTPUT);
    digitalWrite(STA_LED, HIGH);
    
    // Initialize menu system
    menuinit();
    
    Serial.println("[INFO] System ready!");
    digitalWrite(STA_LED, LOW);
    Serial.println("[SYSTEM_WELCOME] Welcome to ESP32 Attack Tool v2!");

}

void loop() {
    menuloop();
    vTaskDelay(1 / portTICK_PERIOD_MS); // Small delay to prevent excessive CPU usage
}