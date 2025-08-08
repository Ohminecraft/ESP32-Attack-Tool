#pragma once

#ifndef DUCKY_USB_HEADER_H
#define DUCKY_USB_HEADER_H

#include <Arduino.h>
#include <USB.h>

#include <USBHIDKeyboard.h>
#include <BLEKeyboard.h>

#include "core/utilsheader.h"
#include "configs.h"

#define DEF_DELAY 100


class BadUSBModules {
    public:
        void beginKB(HIDInterface *&hid, const uint8_t *layout, bool usingble);
        void launchBadUSB(HIDInterface *&hid, const uint8_t* layout, bool usingble = false);
};

#endif