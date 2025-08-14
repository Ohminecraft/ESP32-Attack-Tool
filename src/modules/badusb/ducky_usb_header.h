#pragma once

#ifndef DUCKY_USB_HEADER_H
#define DUCKY_USB_HEADER_H

#include <Arduino.h>
#include <USB.h>

#include <USBHIDKeyboard.h>
#include <BleKeyboard.h>
#include <BleMouse.h>

#include "core/utilsheader.h"
#include "core/sdcardmountheader.h"
#include "core/settingheader.h"
#include "configs.h"

extern ESP32ATSetting espatsettings;

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

enum MediaCommand {
    MEDIA_SCREENSHOT,
    MEDIA_PLAY_PAUSE,
    MEDIA_STOP,
    MEDIA_NEXT_TRACK,
    MEDIA_PREV_TRACK,
    MEDIA_VOL_UP,
    MEDIA_VOL_DOWN,
    MEDIA_MUTE
};

enum KeymoteCommand {
    KEYMOTE_UP,
    KEYMOTE_DOWN,
    KEYMOTE_LEFT,
    KEYMOTE_RIGHT
};

enum TikTokScrollCommand {
    SCROLL_UP,
    SCROLL_DOWN,
    LIKE_VIDEO
};

extern HIDInterface *hid_usb;
extern HIDInterface *hid_ble;
extern BleMouse *hid_mouse;
extern uint8_t keyboardLayout;

extern SDCardModules sdcard;

class BadUSBModules {
    public:
        void beginKB(HIDInterface *&hid, const uint8_t *layout, bool usingble);
        void beginMouse(BleMouse *&hid_mouse, bool usingble);
        void beginLayout(HIDInterface *&hid, bool usingble = false);
        bool isConnected(HIDInterface *&hid);
        void launchBadUSB(String badusbScript, HIDInterface *&hid);
        void mediaController(HIDInterface *&hid, MediaCommand command);
        void Keymote(HIDInterface *&hid, KeymoteCommand key);
        void tiktokScroll(BleMouse *&hid_mouse, TikTokScrollCommand cmd);
        bool isMouseConnected(BleMouse *&hid_mouse);
};

#endif