#include "irread_header.h"

/*
    * irread.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

void IRReadModules::main() {
    irrecv.enableIRIn(); // Start the IR receiver
    pinMode(IR_RX_PIN, INPUT); // Set the IR RX pin as input
    Serial.println("[INFO] IR Read Module Initialized");
}

// (WIP) Because im lazy :)