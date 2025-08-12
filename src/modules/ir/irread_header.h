#pragma once

#ifndef IRREAD_HEADER_H
#define IRREAD_HEADER_H

#include <Arduino.h>
#include <IRrecv.h>
#include <IRutils.h>
#include "core/sdcardmountheader.h"
#include "core/utilsheader.h"
#include "core/displayheader.h"
#include "configs.h"

extern bool irrecvRedraw;
extern SDCardModules sdcard;
extern DisplayModules display;

class IRReadModules {
    private:
        IRrecv irrecv = IRrecv(IR_RX_PIN, 4096 / 2, 50);
        decode_results results;
        bool _read = false;
        bool raw_data = false;
        uint16_t *rawcode;
        uint16_t raw_data_len;
        String code_content;
        String fileName;
    public:      
        void main(bool startup = false);
        void shutdown();
        void begin();
        void readSignal();
        String parse_raw_signal();
        String parse_state_signal();
        void discard_code();
        void save_code();
        bool saveintoSD();
        void processSignalToContent();
};

#endif