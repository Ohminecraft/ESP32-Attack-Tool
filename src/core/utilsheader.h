#pragma once

#ifndef UTILSHEADER_H
#define UTILSHEADER_H

/*
    * utilsheader.h Version 2.0 (with Claude, ChatGPT Help)
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * This file contains utility functions for the ESP32 Attack Tool.
    * It includes functions for generating random names, checking memory, generating random MAC addresses,
*/

#include <Arduino.h>
#include <ESP32Encoder.h>

#define MEM_LOWER_LIM 10000
#define STA_LED 2

// Encoder pins - thay thế button pins
#define ENC_PIN_A   14  // Thay thế BTN_LEFT
#define ENC_PIN_B   12  // Thay thế BTN_RIGHT  
#define ENC_BTN     13  // Thay thế BTN_SELECT

#define MAX_BUTTON  3

// Encoder constants
#define ENCODER_THRESHOLD 2  // Threshold cho encoder movement

const char* generateRandomName();
bool checkLeftMemory();

void generateRandomMac(uint8_t* mac);
String macToString(uint8_t macAddr[6]);
void setBaseMacAddress(uint8_t macAddr[6]);

// Encoder Object
ESP32Encoder encoder;

volatile bool nextPress = false;
volatile bool prevPress = false;
volatile bool selPress = false;

int getEncoderDirection();

void handleInputs();

extern TaskHandle_t xHandle;

extern inline bool check(volatile bool &btn) {
    if (!btn) return false;
    vTaskSuspend(xHandle);
    btn = false;
    delay(10);
    vTaskResume(xHandle);
    return true;
}

#endif // UTILSHEADER_H