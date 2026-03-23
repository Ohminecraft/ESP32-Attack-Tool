#include "rfheader.h"

/*
    * rfcommon.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
    * This file contains common functions and definitions for RF modules.
*/

SPIClass *CC1101_SPI;

void RFModules::setFrequency(float freqMHz) {
    if (freqMHz < 300.0 || freqMHz > 928.0) {
        Serial.println("[ERROR] Frequency out of range! Must be between 300 MHz and 928 MHz, Using default 433.92 MHz.");
        frequency = 433.92;
    } else {
        frequency = freqMHz;
    }
    ELECHOUSE_cc1101.setMHZ(frequency);
}

// https://github.com/BruceDevices/firmware/blob/main/src/modules/rf/rf_utils.cpp
void RFModules::main() {
    ELECHOUSE_cc1101.setSpiPin(
        espatsettings.spiSckPin,
        espatsettings.spiMisoPin,
        espatsettings.spiMosiPin,
        espatsettings.cc1101CsPin
    );
    ELECHOUSE_cc1101.setGDO0(espatsettings.cc1101Gdo0Pin);
    ELECHOUSE_cc1101.Init();
 
    if (!ELECHOUSE_cc1101.getCC1101()) {
        Serial.println("[ERROR] CC1101 Initialization Failed!");
        cc1101_ready = false;
        return;
    }
 
    cc1101_ready = true;
 
    if (startup) {
        Serial.println("[INFO] CC1101 Initialized Successfully!");
        startup = false;
    }
 
    // Cấu hình mặc định — sẽ bị ghi đè bởi sendCommand() nếu cần
    ELECHOUSE_cc1101.setRxBW(270.0);
    ELECHOUSE_cc1101.setDeviation(0);
    ELECHOUSE_cc1101.setPA(12);
    ELECHOUSE_cc1101.setModulation(2);
    setFrequency(frequency); // dùng this->frequency (đã được set trước đó)
}

void RFModules::shutdownCC1101() {
    if (cc1101_ready) {
        ELECHOUSE_cc1101.setSidle();
        cc1101_ready = false;
    }
    digitalWrite(espatsettings.cc1101Gdo0Pin, LOW);
    digitalWrite(espatsettings.cc1101CsPin, HIGH);
    Serial.println("[INFO] Shutdown CC1101 Successfully!");
}

bool RFModules::getCC1101() {
    return ELECHOUSE_cc1101.getCC1101();
}

void RFModules::configureMode(int mode) {
    switch (mode) {
        case RF_RECEIVER_MODE:
            ELECHOUSE_cc1101.SetRx();
            rcSwitch.enableReceive(espatsettings.cc1101Gdo0Pin);
            rcSwitch.resetAvailable();
            Serial.println("[INFO] Set CC1101 to Rx Mode");
            break; 
 
        case RF_TRANSMITTER_MODE:
            ELECHOUSE_cc1101.setSyncMode(0);
            ELECHOUSE_cc1101.setCrc(0);
            ELECHOUSE_cc1101.setPktFormat(3);
            ELECHOUSE_cc1101.SetTx();
            pinMode(espatsettings.cc1101Gdo0Pin, OUTPUT);
            digitalWrite(espatsettings.cc1101Gdo0Pin, LOW);
            Serial.println("[INFO] Set CC1101 to Tx Mode");
            break;
        
        case RF_FREQUENCY_ANALYZER_MODE:
            ELECHOUSE_cc1101.setRxBW(812);         // bandwidth rộng để bắt được tín hiệu
            ELECHOUSE_cc1101.setPA(10);          // TX power (không quan trọng khi chỉ RX)
            ELECHOUSE_cc1101.SetRx();            // bật chế độ nhận
            ELECHOUSE_cc1101.setSyncMode(0);     // tắt sync word → raw mode
            ELECHOUSE_cc1101.setCCMode(0);       // ASK/OOK compatible mode
            Serial.println("[INFO] Set CC1101 to Analyzer Mode");
            break;

        default:
            Serial.println("[WARN] configureMode: unknown mode");
            break;
    }
}

void RFModules::stepFrequency(int step) {
    frequencyIndex = (frequencyIndex + step + GET_SIZE(subghz_frequency_list)) % GET_SIZE(subghz_frequency_list);
    keyData.frequency = frequency = subghz_frequency_list[frequencyIndex];
    setFrequency(subghz_frequency_list[frequencyIndex]);
    Serial.print("[INFO] Frequency set to: ");
    Serial.print(frequency);
    Serial.println(" MHz");
}

float RFModules::getFrequency() {
    return frequency;
}

RfCodes RFModules::getCurrentData() {
    return keyData;
}

float RFModules::getFrequencyAnalyzer() {
    return bestFreq;
}

void RFModules::stepRSSIThreshold(int step) {
    RSSI_threshold = RSSI_threshold + step;
    if (RSSI_threshold > -20) {
        RSSI_threshold = RSSI_MIN_VALID;
        return;
    } else if (RSSI_threshold < RSSI_MIN_VALID) {
        RSSI_threshold = -20;
        return;
    }
}

int8_t RFModules::getFreqAnalyzerRssiThreshold() {
    return RSSI_threshold;
}


void RFModules::resetKeyDetect() { keyDetected = false; }
    
bool RFModules::getKeyDetect() { return keyDetected; }

bool RfCodes::keeloq_check_decrypt(uint32_t decrypt) {
    uint16_t end_serial = serial & 0xFF;

    if ((decrypt >> 28 == btn) && (((((uint16_t)(decrypt >> 16)) & 0xFF) == end_serial) ||
                                   ((((uint16_t)(decrypt >> 16)) & 0xFF) == 0))) {
        cnt = decrypt & 0xFFFF;

        return true;
    }

    return false;
}

bool RfCodes::keeloq_check_decrypt_centurion(uint32_t decrypt) {
    if ((decrypt >> 28 == btn) && ((((uint16_t)(decrypt >> 16)) & 0x3FF) == 0x1CE)) {
        cnt = decrypt & 0xFFFF;

        return true;
    }

    return false;
}

void RfCodes::keeloq_step(uint16_t step) {
    cnt += step;

    hop = btn << 28 | (serial & 0x3FF) << 16 | cnt;

    if (mf_name == "Aprimatic") {
        uint32_t apri_serial = serial;
        uint8_t apr1 = 0;

        for (uint16_t i = 1; i != 0b10000000000; i <<= 1) {
            if (apri_serial & i) apr1++;
        }

        apri_serial &= 0b00001111111111;

        if (apr1 % 2 == 0) { apri_serial |= 0b110000000000; }

        hop = btn << 28 | (apri_serial & 0xFFF) << 16 | cnt;
    } else if (mf_name == "DTM_Neo" || mf_name == "FAAC_RC,XT" || mf_name == "Mutanco_Mutancode" ||
               mf_name == "Came_Space" || mf_name == "Genius_Bravo" || mf_name == "GSN" ||
               mf_name == "Rosh" || mf_name == "Rossi" || mf_name == "Peccinin" || mf_name == "Steelmate" ||
               mf_name == "Cardin_S449") {
        hop = btn << 28 | (serial & 0xFFF) << 16 | cnt;
    } else if (mf_name == "NICE_Smilo" || mf_name == "NICE_MHOUSE" || mf_name == "JCM_Tech") {
        hop = btn << 28 | (serial & 0xFF) << 16 | cnt;
    } else if (mf_name == "Merlin") {
        hop = btn << 28 | (0x000) << 16 | cnt;
    } else if (mf_name == "Centurion") {
        hop = btn << 28 | (0x1CE) << 16 | cnt;
    } else if (mf_name == "Monarch") {
        hop = btn << 28 | (0x100) << 16 | cnt;
    } else if (mf_name == "Dea_Mio") {
        uint8_t first_disc_num = (serial >> 8) & 0xF;
        uint8_t result_disc = (0xC + (first_disc_num % 4));

        uint32_t dea_serial = (serial & 0xFF) | (((uint32_t)result_disc) << 8);

        hop = btn << 28 | (dea_serial & 0xFFF) << 16 | cnt;
    }

    KeeloqKeystore keystore{&LittleFS};

    KeeloqKey current_key;

    for (const auto &key : keystore.get_keys()) {
        if (key.mf_name == mf_name) { current_key = key; }
    }

    switch (current_key.type) {
        case KEELOQ_SIMPLE_LEARNING: {
            encrypted = keeloq_encrypt(hop, current_key.key);

            break;
        }
        case KEELOQ_NORMAL_LEARNING: {
            uint64_t man = keeloq_normal_learning(hop, current_key.key);

            encrypted = keeloq_encrypt(hop, man);

            break;
        }
    }

    key = reverse_bits(encrypted, 32) << 32 | reverse_bits(fix, 32);
}

std::vector<String> split_string(String str, char c) {
    std::vector<String> cols{};
    size_t start = 0;

    while (start < str.length()) {
        auto it = str.indexOf(c, start);

        if (it == -1) break;

        cols.emplace_back(&str[start], it - start);
        start = it + 1;
    }

    if (start <= str.length() && !str.isEmpty()) cols.emplace_back(&str[start], str.length() - start);

    return cols;
}

KeeloqKeystore::KeeloqKeystore(FS *fs) {
    File keystore = fs->open("/mfcodes");

    if (!keystore) { return; }

    String line = keystore.readStringUntil('\n');

    for (; line != ""; line = keystore.readStringUntil('\n')) {
        auto cols = split_string(line, ';');

        if (cols.size() != 3) { return; }

        KeeloqKey key{cols[0], std::strtoull(cols[1].c_str(), NULL, 16), (uint8_t)cols[2].toInt()};

        keys.push_back(key);
    }
}

const std::vector<KeeloqKey> &KeeloqKeystore::get_keys() { return keys; }

uint32_t keeloq_encrypt(const uint32_t data, const uint64_t key) {
    uint32_t x = data, r;

    for (r = 0; r < 528; r++)
        x = (x >> 1) ^ ((bitAt(x, 0) ^ bitAt(x, 16) ^ (uint32_t)bitAt(key, r & 63) ^
                         bitAt(KEELOQ_NLF, g5(x, 1, 9, 20, 26, 31)))
                        << 31);

    return x;
}

uint32_t keeloq_decrypt(const uint32_t data, const uint64_t key) {
    uint32_t x = data, r;

    for (r = 0; r < 528; r++)
        x = (x << 1) ^ bitAt(x, 31) ^ bitAt(x, 15) ^ (uint32_t)bitAt(key, (15 - r) & 63) ^
            bitAt(KEELOQ_NLF, g5(x, 0, 8, 19, 25, 30));

    return x;
}

uint64_t keeloq_normal_learning(uint32_t data, const uint64_t key) {
    uint32_t k1, k2;

    data &= 0x0FFFFFFF;
    data |= 0x20000000;
    k1 = keeloq_decrypt(data, key);

    data &= 0x0FFFFFFF;
    data |= 0x60000000;
    k2 = keeloq_decrypt(data, key);

    return ((uint64_t)k2 << 32) | k1;
}

void keeloq_identify(RfCodes &instance) {
    
    KeeloqKeystore keystore{&LittleFS};

    for (const auto &key : keystore.get_keys()) {
        switch (key.type) {
            case KEELOQ_SIMPLE_LEARNING: {
                uint64_t decrypt = keeloq_decrypt(instance.encrypted, key.key);

                if (instance.keeloq_check_decrypt(decrypt)) {
                    instance.mf_name = key.mf_name;
                    instance.hop = decrypt;

                    return;
                }

                break;
            }

            case KEELOQ_NORMAL_LEARNING: {
                uint64_t man = keeloq_normal_learning(instance.fix, key.key);
                uint64_t decrypt = keeloq_decrypt(instance.encrypted, man);

                if (instance.mf_name == "Centurion") {
                    if (instance.keeloq_check_decrypt_centurion(decrypt)) {
                        instance.hop = decrypt;

                        return;
                    }
                }

                if (instance.keeloq_check_decrypt(decrypt)) {
                    instance.mf_name = key.mf_name;
                    instance.hop = decrypt;

                    return;
                }

                break;
            }
        }
    }
}
