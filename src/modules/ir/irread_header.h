#pragma once

#ifndef IRREAD_HEADER_H
#define IRREAD_HEADER_H

#include <Arduino.h>
#include <IRrecv.h>
#include "core/utilsheader.h"
#include "configs.h"

class IRReadModules {
    private:
        IRrecv irrecv = IRrecv(IR_RX_PIN, 4096 / 2, 50);;
        decode_results results;
    public:      
        void main();
};

#endif