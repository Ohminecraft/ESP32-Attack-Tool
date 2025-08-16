#pragma once

#ifndef IRREAD_HEADER_H
#define IRREAD_HEADER_H

#include <Arduino.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <vector>
#include "core/sdcardmountheader.h"
#include "core/settingheader.h"
#include "core/utilsheader.h"
#include "core/displayheader.h"
#include "configs.h"

extern bool irrecvRedraw;
extern bool quickRemoteTV;
extern uint8_t button_pos;
extern SDCardModules sdcard;
extern DisplayModules display;
extern ESP32ATSetting espatsettings;

class IRReadModules {
    private:
        IRrecv irrecv = IRrecv(espatsettings.irRxPin, 4096 / 2, 50);
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
        void processSignalToContent(String btn_name);

        std::vector<String> quickremoteTV =  {"POWER", "UP",   "DOWN", "LEFT",  "RIGHT", "OK",       "SOURCES",
                                              "VOL+",  "VOL-", "CHA+", "CHA-",  "MUTE",  "SETTINGS", "NETFLIX",
                                              "HOME",  "BACK", "EXIT", "SMART"};
};

#endif