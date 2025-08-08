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
extern uint8_t num_NAcodes, num_EUcodes;

uint8_t bitsleft_r = 0;
uint8_t bits_r=0;
uint8_t code_ptr;
volatile const IrCode * beGoneCode;
bool ending_early = false;

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

uint16_t ontime, offtime;
uint8_t i, num_codes;
TvBeGoneRegion begoneregion;

// https://github.com/pr3y/Bruce/blob/main/src/modules/ir/TV-B-Gone.cpp
void IRSendModules::startTVBGone() {
    if (begoneregion == NA) size_num_codes = num_NAcodes;
    else size_num_codes = num_EUcodes;

    uint16_t rawData[300];

    for (int i = 0; i < size_num_codes; i++) {
        if (check(selPress)) {
            ending_early = true;
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

        code_ptr = 0;
        for (uint8_t x = 0; x < numpairs; x++) {
            if (selPress) {
                break; // Stop sending codes if the user presses the button
            }
            uint16_t ti = (read_bits(bitcompression)) * 2;
            offtime = beGoneCode->times[ti];    // read word 1 - ontime
            ontime = beGoneCode->times[ti + 1]; // read word 2 - offtime
            rawData[x * 2] = offtime * 10;
            rawData[(x * 2) + 1] = ontime * 10;
        }
        irsend.sendRaw(rawData, (numpairs * 2), freq);
        yield(); // Allow other tasks to run
        begone_code_sended += 1;
        bitsleft_r = 0;
        digitalWrite(IR_PIN, LOW);
        delay_ten_us(3000);
        digitalWrite(IR_PIN, HIGH);
        delay_ten_us(20500);
    }
    if (ending_early) {
        delay_ten_us(50000); //500ms delay 
        for (int i = 0; i < 4; i++) {
            digitalWrite(IR_PIN, LOW);
            delay_ten_us(3000);
            digitalWrite(IR_PIN, HIGH);
        }
        digitalWrite(IR_PIN, LOW);
        delay_ten_us(65535);
        delay_ten_us(65535);
        ending_early = false; // Reset the flag
        Serial.println("[INFO] TV-B-Gone Stopped by user");
    } else {
        for (int i = 0; i < 8; i++) {
            digitalWrite(IR_PIN, LOW);
            delay_ten_us(3000);
            digitalWrite(IR_PIN, HIGH);
        }
        digitalWrite(IR_PIN, LOW);
        Serial.println("[INFO] TV-B-Gone Codes Sended: " + String(begone_code_sended));
        if (begone_code_sended == num_NAcodes || begone_code_sended == num_EUcodes) {
            Serial.println("[INFO] TV-B-Gone All Code Sended Successfully");
        } else {
            Serial.println("[ERROR] TV-B-Gone Finished with Failure");
        }
    }
}