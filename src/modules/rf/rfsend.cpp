#include "rfheader.h"

/*
    * rfsend.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/


// https://github.com/BruceDevices/firmware/blob/main/src/modules/rf/rf_send.cpp
void RFModules::sendCommand(struct RfCodes code) {
    uint32_t frequency = code.frequency;
    String protocol = code.protocol;
    String preset = code.preset;
    String data = code.data;
    uint64_t key = code.key;
    byte modulation = 2; // possible values for CC1101: 0 = 2-FSK, 1 =GFSK, 2=ASK, 3 = 4-FSK, 4 = MSK
    float deviation = 1.58;
    float rxBW = 270.83; // Receive bandwidth
    float dataRate = 10;

    int rcswitch_protocol_no = 1;
    if (preset == "FuriHalSubGhzPresetOok270Async") {
        rcswitch_protocol_no = 1;
        //  pulseLength , syncFactor , zero , one, invertedSignal
        // rcswitch_protocol = { 350, {  1, 31 }, {  1,  3 }, {  3,  1 }, false };
        modulation = 2;
        rxBW = 270;
    } else if (preset == "FuriHalSubGhzPresetOok650Async") {
        rcswitch_protocol_no = 2;
        // rcswitch_protocol = { 650, {  1, 10 }, {  1,  2 }, {  2,  1 }, false };
        modulation = 2;
        rxBW = 650;
    } else if (preset == "FuriHalSubGhzPreset2FSKDev238Async") {
        modulation = 0;
        deviation = 2.380371;
        rxBW = 238;
    } else if (preset == "FuriHalSubGhzPreset2FSKDev476Async") {
        modulation = 0;
        deviation = 47.60742;
        rxBW = 476;
    } else if (preset == "FuriHalSubGhzPresetMSK99_97KbAsync") {
        modulation = 4;
        deviation = 47.60742;
        dataRate = 99.97;
    } else if (preset == "FuriHalSubGhzPresetGFSK9_99KbAsync") {
        modulation = 1;
        deviation = 19.042969;
        dataRate = 9.996;
    } else {
        bool found = false;
        for (int p = 0; p < 30; p++) {
            if (preset == String(p)) {
                rcswitch_protocol_no = preset.toInt();
                found = true;
            }
        }
        if (!found) {
            Serial.print("[WARN] Unsupported preset: ");
            Serial.println(preset);
            return;
        }
    }

    main();
    setFrequency(frequency / 1000000.0);
    ELECHOUSE_cc1101.setPA(12);
    ELECHOUSE_cc1101.setModulation(modulation);
    if (deviation) ELECHOUSE_cc1101.setDeviation(deviation);
    if (rxBW) ELECHOUSE_cc1101.setRxBW(rxBW); // Set the Receive Bandwidth in kHz. Value from 58.03 to 812.50. Default is 812.50 kHz.
    if (dataRate) ELECHOUSE_cc1101.setDRate(dataRate);
    configureMode(RF_TRANSMITTER_MODE);
    //ELECHOUSE_cc1101.SetTx();
    if (protocol == "RAW") {
        // count the number of elements of RAW_Data
        int buff_size = 0;
        int index = 0;
        while (index >= 0) {
            index = data.indexOf(' ', index + 1);
            buff_size++;
        }
        // alloc buffer for transmittimings
        int *transmittimings =
            (int *)calloc(sizeof(int), buff_size + 1); // should be smaller the data.length()
        size_t transmittimings_idx = 0;

        // split data into words, convert to int, and store them in transmittimings
        int startIndex = 0;
        index = 0;
        for (transmittimings_idx = 0; transmittimings_idx < buff_size; transmittimings_idx++) {
            index = data.indexOf(' ', startIndex);
            if (index == -1) {
                transmittimings[transmittimings_idx] = data.substring(startIndex).toInt();
            } else {
                transmittimings[transmittimings_idx] = data.substring(startIndex, index).toInt();
            }
            startIndex = index + 1;
        }
        transmittimings[transmittimings_idx] = 0; // termination

        // send rf command
        sendRAW(transmittimings);
        free(transmittimings);
    } else if (protocol == "BinRAW") {
        // transform from "00 01 02 ... FF" into "00000000 00000001 00000010 .... 11111111"
        code.data = hexStrToBinStr(code.data);
        // Serial.println(rfcode.data);
        code.data.trim();
        sendRAWBit(code);
    }

    else if (protocol == "RcSwitch") {
        data.replace(" ", "");
        uint64_t data_val = code.key;
        int bits = code.Bit;
        int pulse = code.te;
        int repeat = 6;

        sendType(data_val, bits, pulse, rcswitch_protocol_no, repeat);
    } else if (protocol.startsWith("Princeton")) {
        sendType(code.key, code.Bit, 350, 1, 10);
    } else {
        Serial.print("[WARN] Unsupported protocol: ");
        Serial.print(protocol);
        Serial.println(" | Sending RcSwitch 11 protocol");
        sendType(code.key, code.Bit, 270, 11, 10);

        return;
    }

    shutdownCC1101();
}


void RFModules::sendRAW(int *ptrtransmittimings) {
    if (!ptrtransmittimings) return;

    bool hasNeg = false;
    unsigned long sum_us = 0;
    for (int i = 0; ptrtransmittimings[i]; ++i) {
        if (ptrtransmittimings[i] < 0) hasNeg = true;
        int v = ptrtransmittimings[i] >= 0 ? ptrtransmittimings[i] : -ptrtransmittimings[i];
        sum_us += (unsigned long)v;
    }
    int nRepeatTransmit = 1;
    if (sum_us > 0) {
        nRepeatTransmit = (int)(900000UL / sum_us);
        if (nRepeatTransmit < 1) nRepeatTransmit = 1;
        if (nRepeatTransmit > 6) nRepeatTransmit = 6;
    }

    for (int nRepeat = 0; nRepeat < nRepeatTransmit; nRepeat++) {
        unsigned int currenttiming = 0;
        bool level = true;
        while (ptrtransmittimings[currenttiming]) {
            int dur = ptrtransmittimings[currenttiming];
            bool lvl;
            unsigned int t;
            if (hasNeg) {
                lvl = (dur >= 0);
                t = (unsigned int)(dur >= 0 ? dur : -dur);
            } else {
                lvl = level;
                t = (unsigned int)(dur >= 0 ? dur : -dur);
                level = !level;
            }
            digitalWrite(espatsettings.cc1101Gdo0Pin, lvl ? HIGH : LOW);
            delayMicroseconds(t);
            currenttiming++;
        }
        digitalWrite(espatsettings.cc1101Gdo0Pin, LOW);
        delayMicroseconds(8000);
    } // end for
}

void RFModules::sendRAWBit(RfCodes data) {
    if (data.data == "") return;
    bool currentlogiclevel = false;
    int nRepeatTransmit = 1;
    for (int nRepeat = 0; nRepeat < nRepeatTransmit; nRepeat++) {
        int currentBit = data.data.length();
        while (currentBit >= 0) { // Starts from the end of the string until the max number of bits to send
            char c = data.data[currentBit];
            if (c == '1') {
                currentlogiclevel = true;
            } else if (c == '0') {
                currentlogiclevel = false;
            } else {
                Serial.println("Invalid data");
                currentBit--;
                continue;
                // return;
            }

            digitalWrite(espatsettings.cc1101Gdo0Pin, currentlogiclevel ? HIGH : LOW);
            delayMicroseconds(data.te);
            currentBit--;
        }
        digitalWrite(espatsettings.cc1101Gdo0Pin, LOW);
    }
}

void RFModules::sendType(uint64_t data, unsigned int bits, int pulse, int protocol, int repeat) {
    // derived from
    // https://github.com/LSatan/SmartRC-CC1101-Driver-Lib/blob/master/examples/Rc-Switch%20examples%20cc1101/SendDemo_cc1101/SendDemo_cc1101.ino

    RCSwitch mySwitch = RCSwitch();

    mySwitch.enableTransmit(espatsettings.cc1101Gdo0Pin);

    mySwitch.setProtocol(protocol); // override
    if (pulse) { mySwitch.setPulseLength(pulse); }
    mySwitch.setRepeatTransmit(repeat > 0 ? repeat : 6);
    mySwitch.send(data, bits);

    mySwitch.disableTransmit();

    shutdownCC1101();
}