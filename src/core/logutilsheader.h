#pragma once

#ifndef LOGUTILSHEADER_H
#define LOGUTILSHEADER_H

/*
    * logutilsheader.h
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License
*/

#include "core/settingheader.h"
#include "core/utilsheader.h"
#include "core/sdcardmountheader.h"

extern ESP32ATSetting espatsettings;
extern SDCardModules sdcard;

#define BUF_SIZE 8192

class LogUtils {
    private:
        uint8_t* bufA = nullptr;
        uint8_t* bufB = nullptr;
        volatile uint32_t bufSizeA = 0;
        volatile uint32_t bufSizeB = 0;
        volatile bool useA = true;
        String file_path;

        void add(const uint8_t* buf, uint32_t len, bool is_pcap);
        void write(const uint8_t* buf, uint32_t len);
        void write(int32_t n);
        void write(uint32_t n);
        void write(uint16_t n);

    public:
        LogUtils();
        ~LogUtils();
        void begin();
        void save();
        void createFile(String filename, bool is_pcap);
        void pcapAppend(wifi_promiscuous_pkt_t *packet, int len);
        void logAppend(String log);
};

#endif