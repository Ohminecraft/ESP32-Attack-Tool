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
#include <IRutils.h>
#include "configs.h"
#include "WORLD_IR_CODES.h"

#include "core/sdcardmountheader.h"
#include "core/utilsheader.h"

#define NOPP __asm__ __volatile__("nop")

enum TvBeGoneRegion {
    NA = 0,
    EU = 1
};

extern TvBeGoneRegion begoneregion;
extern SDCardModules sdcard;

struct IRCode {
    IRCode(
        String protocol = "", String address = "", String command = "", String data = "", uint8_t bits = 32
    )
        : protocol(protocol), address(address), command(command), data(data), bits(bits) {}

    IRCode(IRCode *code) {
        name = String(code->name);
        type = String(code->type);
        protocol = String(code->protocol);
        address = String(code->address);
        command = String(code->command);
        frequency = code->frequency;
        bits = code->bits;
        data = String(code->data);
        filepath = String(code->filepath);
    }

    String protocol = "";
    String address = "";
    String command = "";
    String data = "";
    uint8_t bits = 32;
    String name = "";
    String type = "";
    uint16_t frequency = 0;
    String filepath = "";
};

class IRSendModules {
    private:
        IRsend irsend;
        uint32_t size_num_codes;
    public:
        IRSendModules();

        uint8_t begone_code_sended = 0;
        void main();
        void startTVBGone();
        void sendIRTx(String filename);
        void sendIRCommand(IRCode *code);
        void sendNECCommand(String address, String command);
        void sendNECextCommand(String address, String command);
        void sendRC5Command(String address, String command);
        void sendRC6Command(String address, String command);
        void sendSamsungCommand(String address, String command);
        void sendSonyCommand(String address, String command, uint8_t nbits);
        void sendKaseikyoCommand(String address, String command);
        void sendRawCommand(uint16_t frequency, String rawData);
        bool sendDecodedCommand(String protocol, String value, uint8_t bits = 32);

};

#endif