#pragma once

#ifndef COMMUNICATIONHEADER_H
#define COMMUNICATIONHEADER_H


#include <Arduino.h>
#include <HardwareSerial.h>

#include "core/utilsheader.h"
#include "modules/wifi/wifiheader.h"
#include "configs.h"


class RTL8720DNCommunication {
    private:
        HardwareSerial* RTLSerial;
        String response = "";
        QueueHandle_t responseQueue;
        TaskHandle_t readTaskHandle;
        
        static void readResponseTask(void* parameter); // Static task function
    public:
        RTL8720DNCommunication();
        ~RTL8720DNCommunication();
        
        void begin();
        void isReady();
        void sendCommand(const char* cmd);
        void sendCommand(const String cmd);
        String filterRTLData(String mixedLine);
        String waitForResponse(uint32_t timeout_ms = 5000);
        void parseAPScanResponse(String response);
        void parseAPSTAScanResponse(String response);
        void stopReading();
};


#endif // COMMUNICATIONHEADER_H