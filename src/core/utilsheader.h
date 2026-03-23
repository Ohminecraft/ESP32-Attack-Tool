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
#include <LinkedList.h>
#include <SPI.h>
#include <vector>

#include "esp_wifi.h"
#include "esp_wifi_types.h"

#include "configs.h"

#ifdef BOARD_ESP32_C5_DEVKIT_C1
  extern "C" {
    #include "esp_netif.h"
    #include "esp_netif_net_stack.h"
  }
  #include "esp_system.h"
#endif

#ifdef BUILTIN_RGB_LED
  #include <Adafruit_NeoPixel.h>
#endif


#define MEM_LOWER_LIM 20000

#define GET_TOTAL_HEAP 0
#define GET_FREE_HEAP 1
#define GET_USED_HEAP 2
#define GET_USED_HEAP_PERCENT 3

#define GET_SIZE(x) (sizeof(x) / sizeof(x[0]))

#ifdef BOARD_ESP32_C5_DEVKIT_C1
  extern "C" esp_err_t esp_base_mac_addr_set(uint8_t *Mac);
#endif

#define bitAt(x, n) (((x) >> (n)) & 1)
#define g5(x, a, b, c, d, e) (bitAt(x, a) + bitAt(x, b) * 2 + bitAt(x, c) * 4 + bitAt(x, d) * 8 + bitAt(x, e) * 16)

// CRC-64-ECMA constants
const uint64_t CRC64_ECMA_POLY = 0x42F0E1EBA9EA3693; // Polynomial for CRC-64-ECMA
const uint64_t CRC64_ECMA_INIT = 0xFFFFFFFFFFFFFFFF; // Initial value

uint32_t getHeap(uint8_t type);
const String alfa = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789-=[];',./`\\_+{}:\"<>?~|!@#$%^&*()";
String generateRandomString();
void generateRandomString(char* buffer, size_t length);
bool checkLeftMemory();

void generateRandomMac(uint8_t* mac);
void getMAC(char *addr, uint8_t* data, uint16_t offset);
void getMAC(uint8_t* mac, const uint8_t* data, uint16_t offset);
String hexDump(const uint8_t *buf, size_t len);
void stringToMac(const String& macStr, uint8_t macAddr[6]);
String macToString(uint8_t macAddr[6]);
uint8_t stringToHex(const String& hex_str);
int splitStringToVector(String str, char delimiter, std::vector<String>& result);
void setBaseMacAddress(uint8_t macAddr[6]);
String uint32ToString(uint32_t value);
String uint32ToStringInverted(uint32_t value);
uint32_t swap32(uint32_t value);
uint8_t hexCharToDecimal(char c);
bool getNextLine(const String &src, int &index, String &line);
uint64_t reverse_bits(uint64_t num, uint8_t bits);
uint64_t crc64_ecma(const std::vector<int> &data);
char *dec2binWzerofill(uint64_t Dec, unsigned int bitLength);
String hexStrToBinStr(const String &hexStr);
void decimalToHexString(uint64_t decimal, char *output);
uint32_t hexStringToDecimal(const char *hexString);

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

extern LinkedList<String> *selection_list;

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