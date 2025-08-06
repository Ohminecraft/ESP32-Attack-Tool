#include "irsend_header.h"

/*
    * irsend.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

IRSendModules::IRSendModules() : irsend(IR_PIN)
{
    // Constructor for IR send module
    // Initialize IR send object with default parameters
}

extern const IrCode* const NApowerCodes[];
extern const IrCode* const EUpowerCodes[];
extern const uint8_t num_NAcodes;
extern const uint8_t num_EUcodes;
volatile const IrCode *beGoneCode;

void IRSendModules::main() {
    // Initialize IR send module
    irsend.begin();
    pinMode(IR_PIN, OUTPUT); // Set the IR pin as output
    Serial.println("[INFO] IR Send Module Initialized");
}

void delay_ten_us(uint16_t us) {
    uint8_t timer;
    while (us != 0) {
        for (timer = 0; timer <= 25; timer++) {
            NOPP;
            NOPP;
        }
        NOPP;
        us--;
    }
}

uint8_t read_bits(uint8_t count) {
    uint8_t i;
    uint8_t tmp = 0;
    for (i = 0; i < count; i++) {
        if (bitsleft_r == 0) {
            bits_r = beGoneCode->codes[code_ptr++];
            bitsleft_r = 8;
        }
        bitsleft_r--;
        tmp |= (((bits_r >> (bitsleft_r)) & 1) << (count - 1 - i));
    }
    return tmp;
}

TvBeGoneRegion begoneregion;

void IRSendModules::startTVBeGone() {
    if (begoneregion == NA) size_num_codes = num_NAcodes;
    else size_num_codes = num_EUcodes;

    uint16_t rawData[300];

    for (int i = 0; i < size_num_codes; i++) {
        if (check(selPress)) {
            Serial.println("[INFO] TV-B-Gone Stopped by user");
            break; // Stop sending codes if the user presses the button
        }
        if (begoneregion == NA) {
            beGoneCode = NApowerCodes[i];
        } else {
            beGoneCode = EUpowerCodes[i];
        }

        const uint8_t freq = beGoneCode->timer_val;
        const uint8_t numpairs = beGoneCode->numpairs;
        const uint8_t bitcompression = beGoneCode->bitcompression;

        for (int x = 0; x < numpairs; x++) {
            if (selPress) {
                break; // Stop sending codes if the user presses the button
            }
            uint16_t ti;
            ti = (read_bits(bitcompression)) * 2;
            offtime = beGoneCode->times[ti];    // read word 1 - ontime
            ontime = beGoneCode->times[ti + 1]; // read word 2 - offtime
            rawData[x * 2] = offtime * 10;
            rawData[(x * 2) + 1] = ontime * 10;
        }
        irsend.sendRaw(rawData, (numpairs * 2), freq);
        begone_code_sended += 1;
        delay_ten_us(20500);
    }
}