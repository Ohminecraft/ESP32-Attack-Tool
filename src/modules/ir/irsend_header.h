#pragma once 

#ifndef IRSEND_HEADER_H
#define IRSEND_HEADER_H

/*
    * irsend_header.h
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRSend.h>
#include "configs.h"
#include "WORLD_IR_CODES.h"
#include "core/utilsheader.h"

#define NOPP __asm__ __volatile__("nop")

enum TvBeGoneRegion {
    NA = 0,
    EU = 1
};

extern TvBeGoneRegion begoneregion;

class IRSendModules {
    private:
        IRsend irsend;
        uint32_t size_num_codes;
    public:
        IRSendModules();

        uint8_t begone_code_sended = 0;
        void main();
        void startTVBGone();
};

#endif