#pragma once

#ifndef DUCKY_USB_HEADER_H
#define DUCKY_USB_HEADER_H

#include <Arduino.h>
#include <USB.h>

#include <USBHIDKeyboard.h>
#include "libs/Bad_Usb_Lib/BleKeyboard.h"

#include "core/utilsheader.h"
#include "core/sdcardmountheader.h"
#include "configs.h"

#define DEF_DELAY 100

enum KeyBoardLayout {
    Layout_en_US,
    Layout_pt_BR,
    Layout_pt_PT,
    Layout_fr_FR,
    Layout_es_ES,
    Layout_it_IT,
    Layout_en_UK,
    Layout_de_DE,
    Layout_sv_SE,
    Layout_da_DK,
    Layout_hu_HU,
    Layout_tr_TR,
    Layout_si_SI
};

extern HIDInterface *hid_usb;
extern HIDInterface *hid_ble;
extern uint8_t keyboardLayout;

extern SDCardModules sdcard;

class BadUSBModules {
    public:
        void beginKB(HIDInterface *&hid, const uint8_t *layout, bool usingble);
        void beginBadUSB(HIDInterface *&hid, bool usingble = false);
        bool isConnected(HIDInterface *&hid);
        void launchBadUSB(String badusbScript, HIDInterface *&hid);

};

#endif