#include "rfheader.h"

/*
    * rfsend.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

void RFModules::keeloqLoopEmulate_pre(RfCodes &codes) {
    if (codes.serial != 0) {
        codes.fix = codes.btn << 28 | codes.serial;
        codes.Bit = 64;
        codes.keeloq_step(0);
    }
    keeloq_loop_emulate = true;
}

void RFModules::keeloqLoopEmulate_post() {
    if (prevPress) {
            keyList.clear();
            bitList.clear();
            keeloq_loop_emulate = false;
        }


    if (check(selPress)) {
        if (keyData.serial == 0) {
            for (int i = 0; uint64_t key : keyList) {
                keyData.Bit = bitList[i++];
                keyData.key = key;
                sendCommand(keyData);
            }
        } else {
            sendCommand(keyData);
            keyData.keeloq_step(num_keeloq_steps);
            keeloq_save(keyData);
        }
    }
}

void RFModules::transmittedCommand(RfCodes &codes) {
    int total = bitList.size() + bitRawList.size() + keyList.size() + rawDataList.size() > 0 ? 1 : 0;
    Serial.printf("[INFO] Total signals found: %d\n", total);

    if (codes.protocol != "" && codes.preset != "" && codes.frequency > 0) {
        for (int bit : bitList) {
            codes.Bit = bit;
            sendCommand(codes);
        }
        for (int bitRaw : bitRawList) {
            codes.Bit = bitRaw;
            sendCommand(codes);
        }
        for (uint64_t key : keyList) {
            codes.key = key;
            sendCommand(codes);
        }

        for (String rawData : rawDataList) {
            codes.data = rawData;
            sendCommand(codes);
        }
    }

    bitList.clear();
    bitRawList.clear();
    keyList.clear();
    rawDataList.clear();
}

// https://github.com/BruceDevices/firmware/blob/main/src/modules/rf/rf_send.cpp
void RFModules::sendCommand(const RfCodes& code) {
    uint32_t frequency  = code.frequency;
    String   protocol   = code.protocol;
    String   preset     = code.preset;
    String   data       = code.data;
 
    byte  modulation = 2;    // 0=2-FSK, 1=GFSK, 2=ASK/OOK, 3=4-FSK, 4=MSK
    float deviation  = 1.58;
    float rxBW       = 270;
    float dataRate   = 10;
    int   rcswitch_protocol_no = 1;
 
    // ── Giải mã preset ──────────────────────────────────────
    if (preset == "FuriHalSubGhzPresetOok270Async") {
        rcswitch_protocol_no = 1;
        modulation = 2;
        rxBW       = 270;
    } else if (preset == "FuriHalSubGhzPresetOok650Async") {
        rcswitch_protocol_no = 2;
        modulation = 2;
        rxBW       = 650;
    } else if (preset == "FuriHalSubGhzPreset2FSKDev238Async") {
        modulation = 0;
        deviation  = 2.380371;
        rxBW       = 238;
    } else if (preset == "FuriHalSubGhzPreset2FSKDev476Async") {
        modulation = 0;
        deviation  = 47.60742;
        rxBW       = 476;
    } else if (preset == "FuriHalSubGhzPresetMSK99_97KbAsync") {
        modulation = 4;
        deviation  = 47.60742;
        dataRate   = 99.97;
    } else if (preset == "FuriHalSubGhzPresetGFSK9_99KbAsync") {
        modulation = 1;
        deviation  = 19.042969;
        dataRate   = 9.996;
    } else {
        // FIX #6: bỏ loop thừa, dùng toInt() trực tiếp
        int n = preset.toInt();
        if (n > 0 && n <= 30) {
            rcswitch_protocol_no = n;
        } else {
            Serial.print("[WARN] Unsupported preset: ");
            Serial.println(preset);
            return;
        }
    }
 
    // ── Khởi tạo CC1101 đúng thứ tự ────────────────────────
    // FIX #1: set this->frequency TRƯỚC rồi mới gọi main(),
    //         main() sẽ dùng đúng tần số này.
    setFrequency(frequency / 1000000.0);
    main(); // Init SPI, getCC1101(), setFrequency(this->frequency)
    if (!cc1101_ready) {
        Serial.println("[ERROR] CC1101 not ready, aborting send.");
        return;
    }
 
    // Ghi đè các tham số theo preset
    ELECHOUSE_cc1101.setPA(12);
    ELECHOUSE_cc1101.setModulation(modulation);
    if (deviation > 0) ELECHOUSE_cc1101.setDeviation(deviation);
    if (rxBW > 0)      ELECHOUSE_cc1101.setRxBW(rxBW);
    if (dataRate > 0)  ELECHOUSE_cc1101.setDRate(dataRate);
 
    // FIX #1: configureMode KHÔNG gọi main() nữa → tần số không bị ghi đè
    configureMode(RF_TRANSMITTER_MODE);
 
    // ── Gửi theo protocol ───────────────────────────────────
    if (protocol == "RAW") {
        // FIX #4: đếm buff_size đúng
        int buff_size = 1;
        for (int i = 0; i < (int)data.length(); i++) {
            if (data[i] == ' ') buff_size++;
        }
 
        int *transmittimings = (int *)calloc(sizeof(int), buff_size + 1);
        if (!transmittimings) {
            Serial.println("[ERROR] calloc failed for RAW buffer");
            shutdownCC1101();
            return;
        }
 
        int startIndex = 0;
        for (int idx = 0; idx < buff_size; idx++) {
            int spaceIdx = data.indexOf(' ', startIndex);
            if (spaceIdx == -1) {
                transmittimings[idx] = data.substring(startIndex).toInt();
            } else {
                transmittimings[idx] = data.substring(startIndex, spaceIdx).toInt();
            }
            startIndex = spaceIdx + 1;
        }
        transmittimings[buff_size] = 0; // terminator
 
        sendRAW(transmittimings);
        free(transmittimings);
 
    } else if (protocol == "BinRAW") {
        // FIX (pass by value): dùng local copy thay vì sửa code
        RfCodes localCode = code;
        localCode.data = hexStrToBinStr(code.data);
        localCode.data.trim();
        sendRAWBit(localCode);
 
    } else if (protocol == "RcSwitch") {
        sendType(code.key, code.Bit, code.te, rcswitch_protocol_no, 6);
 
    } else if (protocol.startsWith("Princeton")) {
        sendType(code.key, code.Bit, 350, 1, 10);
 
    } else {
        Serial.print("[WARN] Unsupported protocol: ");
        Serial.print(protocol);
        Serial.println(" | Falling back to RcSwitch protocol 11");
        sendType(code.key, code.Bit, 270, 11, 10);
    }
 
    shutdownCC1101();
}


void RFModules::sendRAW(int *ptrtransmittimings) {
    if (!ptrtransmittimings) return;
 
    // Tính tổng thời gian để ước lượng số lần repeat
    bool hasNeg = false;
    unsigned long sum_us = 0;
    for (int i = 0; ptrtransmittimings[i] != 0; i++) {
        if (ptrtransmittimings[i] < 0) hasNeg = true;
        sum_us += (unsigned long)abs(ptrtransmittimings[i]);
    }
 
    int nRepeatTransmit = 1;
    if (sum_us > 0) {
        nRepeatTransmit = (int)(900000UL / sum_us);
        nRepeatTransmit = constrain(nRepeatTransmit, 1, 6);
    }
 
    for (int nRepeat = 0; nRepeat < nRepeatTransmit; nRepeat++) {
        bool level = true;
        for (int i = 0; ptrtransmittimings[i] != 0; i++) {
            int dur = ptrtransmittimings[i];
            bool  lvl;
            unsigned int t;
 
            if (hasNeg) {
                // signed: dương = HIGH, âm = LOW
                lvl = (dur >= 0);
                t   = (unsigned int)abs(dur);
            } else {
                // unsigned: xen kẽ HIGH/LOW
                lvl   = level;
                t     = (unsigned int)dur;
                level = !level;
            }
 
            digitalWrite(espatsettings.cc1101Gdo0Pin, lvl ? HIGH : LOW);
            delayMicroseconds(t);
        }
        // Gap cuối mỗi lần repeat
        digitalWrite(espatsettings.cc1101Gdo0Pin, LOW);
        delayMicroseconds(8000);
    }
}

void RFModules::sendRAWBit(const RfCodes& data) {
    if (data.data.length() == 0) return;
 
    // FIX #5: bắt đầu từ index hợp lệ (length-1), không phải length
    for (int nRepeat = 0; nRepeat < 1; nRepeat++) {
        for (int currentBit = (int)data.data.length() - 1; currentBit >= 0; currentBit--) {
            char c = data.data[currentBit];
            bool lvl;
            if (c == '1') {
                lvl = true;
            } else if (c == '0') {
                lvl = false;
            } else {
                Serial.printf("[WARN] sendRAWBit: invalid char '%c' at index %d\n", c, currentBit);
                continue;
            }
            digitalWrite(espatsettings.cc1101Gdo0Pin, lvl ? HIGH : LOW);
            delayMicroseconds(data.te);
        }
        digitalWrite(espatsettings.cc1101Gdo0Pin, LOW);
    }
}

void RFModules::sendType(uint64_t data, unsigned int bits, int pulse, int protocol, int repeat) {
    // ref: https://github.com/LSatan/SmartRC-CC1101-Driver-Lib/blob/master/examples/...
    RCSwitch mySwitch = RCSwitch();
    mySwitch.enableTransmit(espatsettings.cc1101Gdo0Pin);
    mySwitch.setProtocol(protocol);
    if (pulse > 0) mySwitch.setPulseLength(pulse);
    mySwitch.setRepeatTransmit(repeat > 0 ? repeat : 6);
    mySwitch.send(data, bits);
    mySwitch.disableTransmit();
}