#pragma once

#ifndef UTILSHEADER_H
#define UTILSHEADER_H

/*
    * utilsheader.h
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License
*/

#include <Arduino.h>
#include <RotaryEncoder.h>
#include <SPI.h>

#include "configs.h"

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
void stringToMac(const String& macStr, uint8_t macAddr[6]);
String macToString(uint8_t macAddr[6]);
void setBaseMacAddress(uint8_t macAddr[6]);
String uint32ToString(uint32_t value);
String uint32ToStringInverted(uint32_t value);
uint32_t swap32(uint32_t value);
uint8_t hexCharToDecimal(char c);

extern volatile bool nextPress;
extern volatile bool prevPress; 
extern volatile bool selPress;
extern volatile bool anykeyPress;
extern bool ble_initialized;
extern bool wifi_initialized;
extern bool wifi_connected;
extern bool first_scan;
extern bool rtl8720dn_ready;
extern bool low_memory_warning;

extern RotaryEncoder *encoder;
IRAM_ATTR void checkPosition();

//void handleInputs();

extern TaskHandle_t xHandle;

extern SPIClass *SDCardSPI;

extern inline bool check(volatile bool &btn) {
    if (!btn) return false;
    vTaskSuspend(xHandle);
    btn = false;
    delay(10);
    vTaskResume(xHandle);
    return true;
}

#endif // UTILSHEADER_H