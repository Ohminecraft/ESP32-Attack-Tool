#pragma once

#ifndef SETTINGHEADER_H
#define SETTINGHEADER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <SD.h>

#include "core/utilsheader.h"
#include "configs.h"

#define UNUSED_PIN -1

class ESP32ATSetting {
    public:
        class WiFiCredential {
            public:
                String ssid;
                String pwd;
        };

        uint8_t spiMosiPin = SPI_MOSI_PIN;
        uint8_t spiMisoPin = SPI_MISO_PIN;
        uint8_t spiSckPin = SPI_SCK_PIN;

        uint16_t displayWidth = SCR_WIDTH;
        uint16_t displayHeight = SCR_HEIGHT;
        bool displayInvert = false;
        uint8_t maxShowSelection = MAX_SHOW_SECLECTION;
        uint16_t graphLineLimit = GRAPH_VERTICAL_LINE_LIM;
        uint8_t displaySdaPin = SDA_PIN;
        uint8_t displaySclPin = SCL_PIN;
        uint8_t statusLedPin = STA_LED;
        String evilportalSSID = "ESP32AttackTool";
        String connectWiFiSSID = "";
        String connectWiFiPassword = "";
        WiFiCredential wificred = {"ESP32AttackTool", "ESP32ATKTOOL"};
        bool autoConnectWiFi = false;
        String bleName = "ESP32 Attack Tool";
        uint16_t sourappleSpamDelay = SOUR_APPLE_SPAM_DELAY;
        uint16_t applejuiceSpamDelay = APPLE_JUICE_SPAM_DELAY;
        uint16_t swiftpairSpamDelay = SWIFTPAIR_SPAM_DELAY;
        uint8_t irTxPin = IR_PIN;
        uint8_t irRxPin = IR_RX_PIN;
        uint8_t irRepeat = IR_REPEAT;
        uint8_t sdcardCsPin = SD_CS_PIN;
        uint8_t nrfCePin = NRF24_CE_PIN;
        uint8_t nrfCsPin = NRF24_CSN_PIN;
        bool usingEncoder = true;
        uint8_t encPinA = ENC_PIN_A;
        uint8_t encPinB = ENC_PIN_B;
        uint8_t leftBtnPin = LEFT_BTN;
        uint8_t rightBtnPin = RIGHT_BTN;
        uint8_t selectBtnPin = SEL_BTN;
        uint8_t timeZone = 0;

        void loadSettings();
        void saveSettings();
        void resetSettings(bool useLittleFS);

        String getSetting(const String &key);
};

#endif // SETTINGHEADER_H