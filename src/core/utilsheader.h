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
#include <SPI.h>

#include "configs.h"

extern SPIClass *SDCardSPI;

#define MEM_LOWER_LIM 20000

#define GET_TOTAL_HEAP 0
#define GET_FREE_HEAP 1
#define GET_USED_HEAP 2
#define GET_USED_HEAP_PERCENT 3

#define GET_SIZE(x) (sizeof(x) / sizeof(x[0]))

uint32_t getHeap(uint8_t type);
const String alfa = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789-=[];',./`\\_+{}:\"<>?~|!@#$%^&*()";
String generateRandomName();
bool checkLeftMemory();

void generateRandomMac(uint8_t* mac);
void getMAC(char *addr, uint8_t* data, uint16_t offset);
String macToString(uint8_t macAddr[6]);
void setBaseMacAddress(uint8_t macAddr[6]);
String uint32ToString(uint32_t value);
String uint32ToStringInverted(uint32_t value);
uint32_t swap32(uint32_t value);
uint8_t hexCharToDecimal(char c);

extern volatile bool nextPress;
extern volatile bool prevPress; 
extern volatile bool selPress;
extern bool ble_initialized;
extern bool wifi_initialized;
extern bool wifi_connected;
extern bool low_memory_warning;

#if defined(USING_ENCODER)
extern RotaryEncoder *encoder;
IRAM_ATTR void checkPosition();
#endif

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