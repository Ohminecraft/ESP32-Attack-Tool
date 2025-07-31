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
#include <RotaryEncoder.h>

#define MEM_LOWER_LIM 20000

// Encoder pins - edit in configs.h
//#define ENC_PIN_A   14 
//#define ENC_PIN_B   12   
//#define ENC_BTN     13 

#define MAX_BUTTON 3

// Encoder constants
//#define ENCODER_THRESHOLD 2  // Threshold cho encoder movement

#define GET_TOTAL_HEAP 0
#define GET_FREE_HEAP 1
#define GET_USED_HEAP 2
#define GET_USED_HEAP_PERCENT 3

uint32_t getHeap(uint8_t type);
String generateRandomName();
bool checkLeftMemory();

void generateRandomMac(uint8_t* mac);
String macToString(uint8_t macAddr[6]);
void setBaseMacAddress(uint8_t macAddr[6]);

extern volatile bool nextPress;
extern volatile bool prevPress; 
extern volatile bool selPress;

extern RotaryEncoder *encoder;
IRAM_ATTR void checkPosition();

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