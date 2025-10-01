#include "settingheader.h"

/*
    * setting.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

bool filesAreIdentical(fs::FS &fs1, const char* path1, fs::FS &fs2, const char* path2) {
    File f1 = fs1.open(path1, FILE_READ);
    File f2 = fs2.open(path2, FILE_READ);

    if (!f1 || !f2) {
        f1.close();
        f2.close();
        return false;
    }

    if (f1.size() != f2.size()) {
        f1.close();
        f2.close();
        return false;
    }

    const size_t bufSize = 512;
    uint8_t buf1[bufSize], buf2[bufSize];
    while (f1.available()) {
        size_t n1 = f1.read(buf1, bufSize);
        size_t n2 = f2.read(buf2, bufSize);
        if (n1 != n2 || memcmp(buf1, buf2, n1) != 0) {
            f1.close();
            f2.close();
            return false;
        }
    }

    f1.close();
    f2.close();
    return true;
}

bool copyFile(fs::FS &srcFS, const char* srcPath, fs::FS &dstFS, const char* dstPath) {
    File src = srcFS.open(srcPath, FILE_READ);
    File dst = dstFS.open(dstPath, FILE_WRITE);
    if (!src || !dst) {
        src.close();
        dst.close();
        return false;
    }

    const size_t bufSize = 1024;
    uint8_t buf[bufSize];
    size_t totalWritten = 0;
    int bytesRead;

    while ((bytesRead = src.read(buf, bufSize)) > 0) {
        if (dst.write(buf, bytesRead) != (size_t)bytesRead) {
            src.close();
            dst.close();
            return false;
        }
        totalWritten += bytesRead;
    }

    src.close();
    dst.close();
    Serial.printf("[INFO] Copied %u bytes from %s to %s\n", totalWritten, srcPath, dstPath);
    return true;
}

void ESP32ATSetting::resetSettings(bool useLittleFS) {
    if (!useLittleFS) {
        if (SD.exists("/ESP32AttackTool/ESP32AttackToolconfig.json")) {
            Serial.println("[INFO] Settings file found, deleting it to reset settings...");
            SD.remove("/ESP32AttackTool/ESP32AttackToolconfig.json");
        } 
    } else {
        if (LittleFS.exists("/ESP32AttackToolconfig.json")) {
            Serial.println("[INFO] Settings file found, deleting it to reset settings...");
            LittleFS.remove("/ESP32AttackToolconfig.json");
        }
    }
    
    Serial.println("[INFO] Resetting settings to default values...");
    
    JsonDocument jsonDoc;

    JsonObject defaultSetting = jsonDoc.to<JsonObject>();

    defaultSetting["spiSckPin"] = SPI_SCK_PIN; // default SPI SCK pin
    defaultSetting["spiMisoPin"] = SPI_MISO_PIN; // default SPI MISO pin
    defaultSetting["spiMosiPin"] = SPI_MOSI_PIN; // default SPI MOSI pin

    defaultSetting["displayWidth"] = SCR_WIDTH; // default display width
    defaultSetting["displayHeight"] = SCR_HEIGHT; // default display height
    defaultSetting["displayInvert"] = false; // default invert display
    defaultSetting["maxShowSelection"] = MAX_SHOW_SECLECTION; // default max show selection on display
    defaultSetting["graphLineLimit"] = GRAPH_VERTICAL_LINE_LIM; // default graph vertical line limit
    defaultSetting["displaySdaPin"] = SDA_PIN; // default display SDA pin
    defaultSetting["displaySclPin"] = SCL_PIN; // default display SCL pin

    defaultSetting["statusLedPin"] = STA_LED; // default status LED pin

    defaultSetting["evilportalSSID"] = "ESP32AttackTool"; // default evil portal SSID

    JsonObject _wifi = defaultSetting["wifi"].to<JsonObject>();
    for (const auto &pair : wifi) { _wifi[pair.first] = pair.second; }

    defaultSetting["autoConnectWiFi"] = false;
    
    defaultSetting["savepcap"] = true;

    defaultSetting["bleName"] = "ESP32AttackTool"; // default BLE name
    defaultSetting["usingSwiftpairForBLEUtilty"] = true; // default using MS SwiftPair for BLE Utility
    defaultSetting["sourappleSpamDelay"] = SOUR_APPLE_SPAM_DELAY; // default Sour Apple spam delay
    defaultSetting["applejuiceSpamDelay"] = APPLE_JUICE_SPAM_DELAY; // default Apple Juice spam delay
    defaultSetting["swiftpairSpamDelay"] = SWIFTPAIR_SPAM_DELAY; // default Swift Pair spam delay
    defaultSetting["spamallSpamDelay"] = 20; // default Spam All spam delay
    defaultSetting["useAppleJuicePaired"] = true; // default use AppleJuice Paired
    defaultSetting["useBleNameasnameofNameFlood"] = true; // default use Blename as name of Name Flood Exploit Attack
    
    defaultSetting["irTxPin"] = IR_PIN; // default IR TX pin
    defaultSetting["irRxPin"] = IR_RX_PIN; // default IR RX pin
    defaultSetting["irRepeat"] = IR_REPEAT; // default IR repeat count
    
    defaultSetting["sdCsPin"] = SD_CS_PIN; // default SD Card CS pin

    defaultSetting["nrf24CePin"] = NRF24_CE_PIN; // default NRF24 CE pin
    defaultSetting["nrf24CsPin"] = NRF24_CSN_PIN; // default NRF24 CSN pin

    defaultSetting["usingEncoder"] = true; // default using encoder for input
    defaultSetting["encPinA"] = ENC_PIN_A; // default encoder pin A
    defaultSetting["encPinB"] = ENC_PIN_B; // default encoder pin B

    defaultSetting["leftBtnPin"] = LEFT_BTN; // default left button pin
    defaultSetting["rightBtnPin"] = RIGHT_BTN; // default right button pin

    defaultSetting["selBtnPin"] = SEL_BTN; // default select button pin

    defaultSetting["timeZone"] = 0;
    defaultSetting["autoDeepSleep"] = true;
    defaultSetting["autoStandby"] = true;

    File configfile;
    if (useLittleFS) {
        configfile = LittleFS.open("/ESP32AttackToolconfig.json", FILE_WRITE);
    } else {
        configfile = SD.open("/ESP32AttackTool/ESP32AttackToolconfig.json", FILE_WRITE);
    }

    if (!configfile) {
        Serial.println("[ERROR] Failed to create config file!");
        return;
    }

    if (serializeJsonPretty(defaultSetting, configfile) < 5) {
        Serial.println("[ERROR] Failed to write new config!");
    } else {
        Serial.println("[INFO] Success write default config into config file");
    }
    configfile.close();

    if (!useLittleFS) {

        File source = SD.open("/ESP32AttackTool/ESP32AttackToolconfig.json", FILE_READ);
        File dest   = LittleFS.open("/ESP32AttackToolconfig.json", FILE_WRITE);

        if (!source) {
            Serial.println("[ERROR] Cannot open source file in SD");
            return;
        }
        if (!dest) {
            Serial.println("[ERROR] Cannot open destination file in LittleFS");
            source.close();
            return;
        }


        bool sameFile = false;
        {
            File checkDest = LittleFS.open("/ESP32AttackToolconfig.json", FILE_READ);
            if (checkDest && checkDest.size() == source.size()) {
                checkDest.seek(0);
                source.seek(0);
                sameFile = true;
                while (source.available()) {
                    if (source.read() != checkDest.read()) {
                        sameFile = false;
                        break;
                    }
                }
            }
            checkDest.close();
            source.seek(0);
        }

        if (sameFile) {
            Serial.println("[INFO] SD and LittleFS files are identical, skip copy");
            source.close();
            dest.close();
            return;
        }

        const int bufSize = 1024;
        uint8_t buf[bufSize];
        size_t bytesRead, totalWritten = 0;

        while ((bytesRead = source.read(buf, bufSize)) > 0) {
            size_t written = dest.write(buf, bytesRead);
            if (written != bytesRead) {
                Serial.printf("[ERROR] Write failed at %u bytes\n", totalWritten);
                break;
            }
            totalWritten += written;
        }

        Serial.printf("[INFO] Copy finished. Total bytes written: %u\n", totalWritten);

        source.close();
        dest.close();
    }

    Serial.println("[INFO] Settings reset successfully.");
    Serial.println("[INFO] Loading Config Attempt 2.");
    loadSettings();
}

void ESP32ATSetting::loadSPIPinsFromJson(JsonObject &_settings) {
    if (!_settings["spiSckPin"].isNull()) {
        spiSckPin = _settings["spiSckPin"].as<uint8_t>();
        Serial.println("[INFO] Loaded spiSckPin: " + String(spiSckPin));
    }
    if (!_settings["spiMisoPin"].isNull()) {
        spiMisoPin = _settings["spiMisoPin"].as<uint8_t>();
        Serial.println("[INFO] Loaded spiMisoPin: " + String(spiMisoPin));
    }
    if (!_settings["spiMosiPin"].isNull()) {
        spiMosiPin = _settings["spiMosiPin"].as<uint8_t>();
        Serial.println("[INFO] Loaded spiMosiPin: " + String(spiMosiPin));
    }
    if (!_settings["sdCsPin"].isNull()) {
        sdcardCsPin = _settings["sdCsPin"].as<uint8_t>();
        Serial.println("[INFO] Loaded sdCsPin: " + String(sdcardCsPin));
    }
}

void ESP32ATSetting::loadSettings() {
    Serial.println("[INFO] Loading settings...");

    // Bước 1: Mount LittleFS trước
    if (!LittleFS.begin(true)) {
        Serial.println("[ERROR] Failed to mount LittleFS!");
        return;
    }

    bool useLittleFS = true;
    bool mountSD = false;
    static bool reloadedOnce = false;

    SDCardSPI = &SPI;
    SDCardSPI->begin(spiSckPin, spiMisoPin, spiMosiPin, sdcardCsPin);

    if (!SD.begin(sdcardCsPin, *SDCardSPI)) {
        Serial.println("[INFO] Failed to mount SD, open LittleFS config file to check sd pin");
        if (LittleFS.exists("/ESP32AttackToolconfig.json")) {
            File configfile = LittleFS.open("/ESP32AttackToolconfig.json", FILE_READ);
            if (configfile) {
                JsonDocument jsonDoc;
                if (!deserializeJson(jsonDoc, configfile)) {
                    JsonObject _settings = jsonDoc.as<JsonObject>();
                    loadSPIPinsFromJson(_settings);
                    SDCardSPI->end();
                }
                configfile.close();
            }
        } else {
            Serial.println("[WARN] LittleFS config not found, will use default SPI + CS pins");
        }
    } else {
        mountSD = true;
        Serial.println("[INFO] SD Card mounted");
    }


    if (!mountSD) {
        SDCardSPI->begin(spiSckPin, spiMisoPin, spiMosiPin, sdcardCsPin);
        if (SD.begin(sdcardCsPin, *SDCardSPI)) {
            mountSD = true;
            Serial.println("[INFO] SD Card Attempt 2 mounted successfully");
        }
    }
    

    if (mountSD) {
        if (!SD.exists("/ESP32AttackTool/ESP32AttackToolconfig.json")) {
            Serial.println("[WARN] SD config not found");
            if (LittleFS.exists("/ESP32AttackToolconfig.json")) {
                Serial.println("[INFO] Found Configuration file in LittleFS, Copy from LittleFS");
                copyFile(LittleFS, "/ESP32AttackToolconfig.json", SD, "/ESP32AttackTool/ESP32AttackToolconfig.json");
                if (!reloadedOnce) {
                    reloadedOnce = true;
                    Serial.println("[INFO] Reloading config after copy...");
                    loadSettings();
                    return;
                }
            } else {
                Serial.println("[INFO] Creating one...");
                resetSettings(false);
            }
        }

        if (!filesAreIdentical(SD, "/ESP32AttackTool/ESP32AttackToolconfig.json", LittleFS, "/ESP32AttackToolconfig.json")) {
            Serial.println("[WARN] Config files differ, updating LittleFS from SD");
            copyFile(SD, "/ESP32AttackTool/ESP32AttackToolconfig.json", LittleFS, "/ESP32AttackToolconfig.json");
        }
        useLittleFS = false;
    } else {
        Serial.println("[WARN] SD not mounted, fallback to LittleFS");
    }

    File finalConfig;
    if (useLittleFS) {
        finalConfig = LittleFS.open("/ESP32AttackToolconfig.json", FILE_READ);
    } else {
        finalConfig = SD.open("/ESP32AttackTool/ESP32AttackToolconfig.json", FILE_READ);
    }

    if (!finalConfig) {
        Serial.println("[ERROR] Cannot open config file!");
        return;
    }

    JsonDocument jsonDoc;
    if (deserializeJson(jsonDoc, finalConfig)) {
        Serial.println("[ERROR] Failed to parse config, resetting...");
        resetSettings(useLittleFS);
        return;
    }
    finalConfig.close();

    JsonObject _settings = jsonDoc.as<JsonObject>();
    int failed_count = 0;

    if (!_settings["spiSckPin"].isNull()) {
        spiSckPin = _settings["spiSckPin"].as<uint8_t>();
        Serial.println("spiSckPin: " + String(spiSckPin));
    } else {
        Serial.println("[WARN] Failed to get 'spiSckPin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["spiMisoPin"].isNull()) {
        spiMisoPin = _settings["spiMisoPin"].as<uint8_t>();
        Serial.println("spiMisoPin: " + String(spiMisoPin));
    } else {
        Serial.println("[WARN] Failed to get 'spiMisoPin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["spiMosiPin"].isNull()) {
        spiMosiPin = _settings["spiMosiPin"].as<uint8_t>();
        Serial.println("spiMosiPin: " + String(spiMosiPin));
    } else {
        Serial.println("[WARN] Failed to get 'spiMosiPin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["displayWidth"].isNull()) {
        displayWidth = _settings["displayWidth"].as<uint16_t>();
        Serial.println("displayWidth: " + String(displayWidth));
    } else {
        Serial.println("[WARN] Failed to get 'displayWidth' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["displayHeight"].isNull()) {
        displayHeight = _settings["displayHeight"].as<uint16_t>();
        Serial.println("displayHeight: " + String(displayHeight));
    } else {
        Serial.println("[WARN] Failed to get 'displayHeight' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["displayInvert"].isNull()) {
        displayInvert = _settings["displayInvert"].as<bool>();
        if (displayInvert)
            Serial.println("displayInvert: true");
        else Serial.println("displayInvert: false");
    } else {
        Serial.println("[WARN] Failed to get 'displayInvert' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["maxShowSelection"].isNull()) {
        maxShowSelection = _settings["maxShowSelection"].as<uint8_t>();
        Serial.println("maxShowSelection: " + String(maxShowSelection));
    } else {
        Serial.println("[WARN] Failed to get 'maxShowSelection' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["graphLineLimit"].isNull()) {
        graphLineLimit = _settings["graphLineLimit"].as<uint16_t>();
        Serial.println("graphLineLimit: " + String(graphLineLimit));
    } else {
        Serial.println("[WARN] Failed to get 'graphLineLimit' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["displaySdaPin"].isNull()) {
        displaySdaPin = _settings["displaySdaPin"].as<uint8_t>();
        Serial.println("displaySdaPin: " + String(displaySdaPin));
    } else {
        Serial.println("[WARN] Failed to get 'displaySdaPin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["displaySclPin"].isNull()) {
        displaySclPin = _settings["displaySclPin"].as<uint8_t>();
        Serial.println("displaySclPin: " + String(displaySclPin));
    } else {
        Serial.println("[WARN] Failed to get 'displaySclPin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["statusLedPin"].isNull()) {
        statusLedPin = _settings["statusLedPin"].as<uint8_t>();
        Serial.println("statusLedPin: " + String(statusLedPin));
    } else {
        Serial.println("[WARN] Failed to get 'statusLedPin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["evilportalSSID"].isNull()) {
        evilportalSSID = _settings["evilportalSSID"].as<String>();
        Serial.println("evilportalSSID: " + evilportalSSID);
    } else {
        Serial.println("[WARN] Failed to get 'evilportalSSID' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["wifi"].isNull()) {
        wifi.clear();
        for (JsonPair kv : _settings["wifi"].as<JsonObject>()) wifi[kv.key().c_str()] = kv.value().as<String>();
    } else {
        Serial.println("[WARN] Failed to get 'wifiCred' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["autoConnectWiFi"].isNull()) {
        autoConnectWiFi = _settings["autoConnectWiFi"].as<bool>();
        if (autoConnectWiFi)  Serial.println("autoConnectWiFi: true");
        else Serial.println("autoConnectWiFi: false");

    } else {
        Serial.println("[WARN] Failed to get 'autoConnectWiFi' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["savepcap"].isNull()) { // Save Log or .pcap file
        savepcap = _settings["savepcap"].as<bool>();
        if (savepcap)  Serial.println("savepcap: true");
        else Serial.println("savepcap: false");
    } else {
        Serial.println("[WARN] Failed to get 'savepcap' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["bleName"].isNull()) {
        bleName = _settings["bleName"].as<String>();
        if (bleName.length() > 20) {
            Serial.println("[WARN] Name for BLE is too long, maximum 20 char! | Using default name");
            bleName = "ESP32AttackTool";
        }
        Serial.println("bleName: " + bleName);
    } else {
        Serial.println("[WARN] Failed to get 'bleName' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["usingSwiftpairForBLEUtilty"].isNull()) {
        usingSwiftpairForBLEUtilty = _settings["usingSwiftpairForBLEUtilty"].as<bool>();
        if (usingSwiftpairForBLEUtilty)  Serial.println("usingSwiftpairForBLEUtilty: true");
        else Serial.println("usingSwiftpairForBLEUtilty: false");
    } else {
        Serial.println("[WARN] Failed to get 'bleName' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["sourappleSpamDelay"].isNull()) {
        sourappleSpamDelay = _settings["sourappleSpamDelay"].as<uint16_t>();
        Serial.println("sourappleSpamDelay: " + String(sourappleSpamDelay));
    } else {
        Serial.println("[WARN] Failed to get 'sourappleSpamDelay' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["applejuiceSpamDelay"].isNull()) {
        applejuiceSpamDelay = _settings["applejuiceSpamDelay"].as<uint16_t>();
        Serial.println("applejuiceSpamDelay: " + String(applejuiceSpamDelay));
    } else {
        Serial.println("[WARN] Failed to get 'applejuiceSpamDelay' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["swiftpairSpamDelay"].isNull()) {
        swiftpairSpamDelay = _settings["swiftpairSpamDelay"].as<uint16_t>();
        Serial.println("swiftpairSpamDelay: " + String(swiftpairSpamDelay));
    } else {
        Serial.println("[WARN] Failed to get 'swiftpairSpamDelay' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["useAppleJuicePaired"].isNull()) {
        useAppleJuicePaired = _settings["useAppleJuicePaired"].as<bool>();
        if (useAppleJuicePaired)
            Serial.println("useAppleJuicePaired: true");
        else Serial.println("useAppleJuicePaired: false");
    } else {
        Serial.println("[WARN] Failed to get 'useAppleJuicePaired' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["useBleNameasnameofNameFlood"].isNull()) {
        useBleNameasnameofNameFlood = _settings["useBleNameasnameofNameFlood"].as<bool>();
        if (useBleNameasnameofNameFlood)
            Serial.println("useBleNameasnameofNameFlood: true");
        else Serial.println("useBleNameasnameofNameFlood: false");
    } else {
        Serial.println("[WARN] Failed to get 'useBleNameasnameofNameFlood' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["spamallSpamDelay"].isNull()) {
        spamAllDelay = _settings["spamallSpamDelay"].as<uint16_t>();
        Serial.println("spamAllDelay: " + String(spamAllDelay));
    } else {
        Serial.println("[WARN] Failed to get 'spamallSpamDelay' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["irTxPin"].isNull()) {
        irTxPin = _settings["irTxPin"].as<uint8_t>();
        Serial.println("irTxPin: " + String(irTxPin));
    } else {
        Serial.println("[WARN] Failed to get 'irTxPin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["irRxPin"].isNull()) {
        irRxPin = _settings["irRxPin"].as<uint8_t>();
        Serial.println("irRxPin: " + String(irRxPin));
    } else {
        Serial.println("[WARN] Failed to get 'irRxPin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["irRepeat"].isNull()) {
        irRepeat = _settings["irRepeat"].as<uint8_t>();
        Serial.println("irRepeat: " + String(irRepeat));
    } else {
        Serial.println("[WARN] Failed to get 'irRepeat' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["sdCsPin"].isNull()) {
        sdcardCsPin = _settings["sdCsPin"].as<uint8_t>();
        Serial.println("sdcardCsPin: " + String(sdcardCsPin));
    } else {
        Serial.println("[WARN] Failed to get 'sdCsPin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["nrf24CePin"].isNull()) {
        nrfCePin = _settings["nrf24CePin"].as<uint8_t>();
        Serial.println("nrfCePin: " + String(nrfCePin));
    } else {
        Serial.println("[WARN] Failed to get 'nrf24CePin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["nrf24CsPin"].isNull()) {
        nrfCsPin = _settings["nrf24CsPin"].as<uint8_t>();
        Serial.println("nrfCsPin: " + String(nrfCsPin));
    } else {
        Serial.println("[WARN] Failed to get 'nrf24CsPin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["usingEncoder"].isNull()) {
        usingEncoder = _settings["usingEncoder"].as<bool>();
        if (usingEncoder) Serial.println("usingEncoder: true");
        else Serial.println("usingEncoder: false");
    } else {
        Serial.println("[WARN] Failed to get 'usingEncoder' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["encPinA"].isNull()) {
        encPinA = _settings["encPinA"].as<uint8_t>();
        Serial.println("encPinA: " + String(encPinA));
    } else {
        Serial.println("[WARN] Failed to get 'encPinA' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["encPinB"].isNull()) {
        encPinB = _settings["encPinB"].as<uint8_t>();
        Serial.println("encPinB: " + String(encPinB));
    } else {
        Serial.println("[WARN] Failed to get 'encPinB' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["leftBtnPin"].isNull()) {
        leftBtnPin = _settings["leftBtnPin"].as<uint8_t>();
        Serial.println("leftBtnPin: " + String(leftBtnPin));
    } else {
        Serial.println("[WARN] Failed to get 'leftBtnPin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["rightBtnPin"].isNull()) {
        rightBtnPin = _settings["rightBtnPin"].as<uint8_t>();
        Serial.println("rightBtnPin: " + String(rightBtnPin));
    } else {
        Serial.println("[WARN] Failed to get 'rightBtnPin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["selBtnPin"].isNull()) {
        selectBtnPin = _settings["selBtnPin"].as<uint8_t>();
        Serial.println("selectBtnPin: " + String(selectBtnPin));
    } else {
        Serial.println("[WARN] Failed to get 'selBtnPin' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["timeZone"].isNull()) {
        timeZone = _settings["timeZone"].as<int8_t>();
        Serial.println("timeZone: " + String(timeZone));
    } else {
        Serial.println("[WARN] Failed to get 'timeZone' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["autoDeepSleep"].isNull()) {
        autoDeepSleep = _settings["autoDeepSleep"].as<bool>();
        if (autoDeepSleep) Serial.println("autoDeepSleep: true");
        else Serial.println("autoDeepSleep: false");
    } else {
        Serial.println("[WARN] Failed to get 'autoDeepSleep' configuration | Ignoring it using default");
        failed_count++;
    }
    if (!_settings["autoStandby"].isNull()) {
        autoStandby = _settings["autoStandby"].as<bool>();
        if (autoStandby) Serial.println("autoStandby: true");
        else Serial.println("autoStandby: false");
    } else {
        Serial.println("[WARN] Failed to get 'autoStandby' configuration | Ignoring it using default");
        failed_count++;
    }

    if (failed_count > 8) {
        Serial.println("[INFO] Found over 8 missing configuation, rewrite new configuration file");
        resetSettings(useLittleFS);
    }
}

String ESP32ATSetting::getApPassword(const String &ssid) const {
    auto it = wifi.find(ssid);
    if (it != wifi.end()) return it->second;
    return "";
}