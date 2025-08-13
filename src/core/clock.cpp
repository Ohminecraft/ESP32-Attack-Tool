#include "clockheader.h"

/*
    * clock.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 0);
ESP32Time rtc;

void TimeClock::main() {
        //timeClient.setTimeOffset();
    timeClient.begin();
    if (timeClient.forceUpdate() && wifi_connected) {
        rtc.setTime(timeClient.getEpochTime() + espatsettings.timeZone * 3600);
        update(rtc.getTimeStruct());
        Serial.println("[INFO] Get NTP Time successfully!");
    }      
}

void TimeClock::update(struct tm timeget) {
    snprintf(timechar, sizeof(timechar), "%02d:%02d:%02d", timeget.tm_hour, timeget.tm_min, timeget.tm_sec);
}

