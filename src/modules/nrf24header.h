#pragma once

#ifndef NRF24HEADER_H
#define NRF24HEADER_H

/*
*/

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <EEPROM.h>

#include "core/utilsheader.h"

#define NRF24_CONFIG      0x00
#define NRF24_EN_AA       0x01
#define NRF24_RF_CH       0x05
#define NRF24_RF_SETUP    0x06
#define NRF24_RPD         0x09

#define N 128
uint8_t values[N];

#define CE  5
#define CS 17

#define ANA_CHANNELS  64
int ana_channel[ANA_CHANNELS];

#define SCAN_CHANNELS 64
int scan_channel[SCAN_CHANNELS];

int line;
char grey[] = " .:-=+*aRW";

#define EEPROM_ADDRESS_SENSOR_ARRAY 2

unsigned long lastSaveTime = 0;
const unsigned long saveInterval = 5000;

byte sensorArray[129];

const byte bluetooth_channels[] =        {32, 34, 46, 48, 50, 52, 0, 1, 2, 4, 6, 8, 22, 24, 26, 28, 30, 74, 76, 78, 80};
const byte ble_channels[] =              {2, 26, 80};
const byte WiFi_channels[] =             {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
const byte usbWireless_channels[] =      {40, 50, 60};
const byte videoTransmitter_channels[] = {70, 75, 80};
const byte rc_channels[] =               {1, 3, 5, 7};
const byte zigbee_channels[] =           {11, 15, 20, 25};
const byte full_channels[] = {
        1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
        22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
        43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
        64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
        85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100
        //, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112,
        // 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124
    };

enum NRFJammerMode {
    Wifi,
    BLE,
    Bluetooth,
    Zigbee,
    RC,
    Video_Transmitter,
    Usb_Wireless,
    Full_Channel
};  

class NRF24Modules
{
    private:
        byte getRegister(byte r);
        void setRegister(byte r, byte v);
        void setRX();
        void writeRegister(uint8_t reg, uint8_t value);
        uint8_t readRegister(uint8_t reg);
    public:
        void initNRF();
        void shutdownNRF();
        void enableCE();
        void disableCE();
        void analyzerScanChannels();
        void setChannel(uint8_t channel);
        bool carrierDetected();
        void analyzerSetup();

        void jammerNRFRadioSetup();
        void jammerNRFRadioMain(NRFJammerMode mode);
        void shutdownNRFJammer();

        void scanChannel();
        void loadPreviousGraph();
        void saveGraphtoEEPROM();
        void scannerSetup();

};

#endif // NRF24HEADER_H