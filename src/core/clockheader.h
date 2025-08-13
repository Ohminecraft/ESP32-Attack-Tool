#pragma once

#ifndef CLOCKHEADER_H
#define CLOCKHEADER_H

/*
    * clockheader.h
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

#include <Arduino.h>
#include <NTPClient.h>
#include <ESP32Time.h>
#include <WiFiUdp.h>

#include "core/settingheader.h"

extern ESP32ATSetting espatsettings;

extern ESP32Time rtc;

class TimeClock {
    public:
        char timechar[10];
        void main();
        void update(struct tm timeget);
}; 
#endif