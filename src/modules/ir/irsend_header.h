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
#include <IRSend.h>
#include "configs.h"
#include "modules/ir/tv_be_gone_code.h"
#include "core/utilsheader.h"
#define NOPP __asm__ __volatile__("nop")

uint8_t bitsleft_r = 0;
uint8_t bits_r = 0;
uint8_t code_ptr;
uint16_t ontime, offtime;

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
        void startTVBeGone();
};

#endif