#include "rfheader.h"

/*
    * rfread.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

void RFModules::parseReceivedData() {
    keyData.fix = 0;
    keyData.hop = 0;
    keyData.btn = 0;
    keyData.cnt = 0;
    keyData.mf_name = "Unknown";
    keyData.encrypted = 0;
    uint32_t receivedValue = rcSwitch.getReceivedValue();
    if (receivedValue) {
        keyData.frequency = long(frequency * 1000000);
        keyData.key = receivedValue;
        keyData.preset = String(rcSwitch.getReceivedProtocol());
        keyData.protocol = "RcSwitch";
        keyData.te = rcSwitch.getReceivedDelay();
        keyData.Bit = rcSwitch.getReceivedBitlength();
        keyData.data = "";

        if (rcSwitch.getReceivedProtocol() == 23) {
            uint64_t yek = reverse_bits(receivedValue, 64);

            keyData.fix = yek >> 32;
            keyData.btn = keyData.fix >> 28;
            keyData.encrypted = yek & 0xFFFFFFFF;
            keyData.serial = (yek >> 32) & 0xFFFFFFF;

            keeloq_identify(keyData);
        }
        keyDetected = true;
    }
    rcSwitch.resetAvailable();
}