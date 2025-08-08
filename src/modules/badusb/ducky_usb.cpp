#include "ducky_usb_header.h"

/*
    * ducky_usb.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

HIDInterface *hid_usb = nullptr;
HIDInterface *hid_ble = nullptr;

enum DuckyCommandType {
    DuckyCommandType_Unknown,
    DuckyCommandType_Cmd,
    DuckyCommandType_Print,
    DuckyCommandType_Delay,
    DuckyCommandType_Comment,
    DuckyCommandType_Loop,
    DuckyCommandType_Combination
};

struct DuckyCommand {
    const char *command;
    char key;
    DuckyCommandType type;
};

struct DuckyCombination {
    const char *command;
    char key1;
    char key2;
    char key3;
};
const DuckyCombination duckyComb[]{
    {"CTRL-ALT",       KEY_LEFT_CTRL, KEY_LEFT_ALT,   0             },
    {"CTRL-SHIFT",     KEY_LEFT_CTRL, KEY_LEFT_SHIFT, 0             },
    {"CTRL-GUI",       KEY_LEFT_CTRL, KEY_LEFT_GUI,   0             },
    {"CTRL-ESCAPE",    KEY_LEFT_CTRL, KEY_ESC,        0             },
    {"ALT-SHIFT",      KEY_LEFT_ALT,  KEY_LEFT_SHIFT, 0             },
    {"ALT-GUI",        KEY_LEFT_ALT,  KEY_LEFT_GUI,   0             },
    {"GUI-SHIFT",      KEY_LEFT_GUI,  KEY_LEFT_SHIFT, 0             },
    {"GUI-SPACE",      KEY_LEFT_GUI,  KEY_SPACE,      0             },
    {"CTRL-ALT-SHIFT", KEY_LEFT_CTRL, KEY_LEFT_ALT,   KEY_LEFT_SHIFT},
    {"CTRL-ALT-GUI",   KEY_LEFT_CTRL, KEY_LEFT_ALT,   KEY_LEFT_GUI  },
    {"ALT-SHIFT-GUI",  KEY_LEFT_ALT,  KEY_LEFT_SHIFT, KEY_LEFT_GUI  },
    {"CTRL-SHIFT-GUI", KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_LEFT_GUI  }
};

const DuckyCommand duckyCmds[]{
    {"STRING",         0,                DuckyCommandType_Print      },
    {"STRINGLN",       0,                DuckyCommandType_Print      },
    {"REM",            0,                DuckyCommandType_Comment    },
    {"DELAY",          0,                DuckyCommandType_Delay      },
    {"DEFAULTDELAY",   DEF_DELAY,        DuckyCommandType_Delay      },
    {"REPEAT",         0,                DuckyCommandType_Loop       },
    {"CTRL-ALT",       0,                DuckyCommandType_Combination},
    {"CTRL-SHIFT",     0,                DuckyCommandType_Combination},
    {"CTRL-GUI",       0,                DuckyCommandType_Combination},
    {"CTRL-ESCAPE",    0,                DuckyCommandType_Combination},
    {"ALT-SHIFT",      0,                DuckyCommandType_Combination},
    {"ALT-GUI",        0,                DuckyCommandType_Combination},
    {"GUI-SHIFT",      0,                DuckyCommandType_Combination},
    {"GUI-SPACE",      0,                DuckyCommandType_Combination},
    {"CTRL-ALT-SHIFT", 0,                DuckyCommandType_Combination},
    {"CTRL-ALT-GUI",   0,                DuckyCommandType_Combination},
    {"ALT-SHIFT-GUI",  0,                DuckyCommandType_Combination},
    {"CTRL-SHIFT-GUI", 0,                DuckyCommandType_Combination},
    {"BACKSPACE",      KEYBACKSPACE,     DuckyCommandType_Cmd        },
    {"DELETE",         KEY_DELETE,       DuckyCommandType_Cmd        },
    {"ALT",            KEY_LEFT_ALT,     DuckyCommandType_Cmd        },
    {"CTRL",           KEY_LEFT_CTRL,    DuckyCommandType_Cmd        },
    {"GUI",            KEY_LEFT_GUI,     DuckyCommandType_Cmd        },
    {"SHIFT",          KEY_LEFT_SHIFT,   DuckyCommandType_Cmd        },
    {"ESCAPE",         KEY_ESC,          DuckyCommandType_Cmd        },
    {"TAB",            KEYTAB,           DuckyCommandType_Cmd        },
    {"ENTER",          KEY_RETURN,       DuckyCommandType_Cmd        },
    {"DOWNARROW",      KEY_DOWN_ARROW,   DuckyCommandType_Cmd        },
    {"DOWN",           KEY_DOWN_ARROW,   DuckyCommandType_Cmd        },
    {"LEFTARROW",      KEY_LEFT_ARROW,   DuckyCommandType_Cmd        },
    {"LEFT",           KEY_LEFT_ARROW,   DuckyCommandType_Cmd        },
    {"RIGHTARROW",     KEY_RIGHT_ARROW,  DuckyCommandType_Cmd        },
    {"RIGHT",          KEY_RIGHT_ARROW,  DuckyCommandType_Cmd        },
    {"UPARROW",        KEY_UP_ARROW,     DuckyCommandType_Cmd        },
    {"UP",             KEY_UP_ARROW,     DuckyCommandType_Cmd        },
    {"BREAK",          KEY_PAUSE,        DuckyCommandType_Cmd        },
    {"CAPSLOCK",       KEY_CAPS_LOCK,    DuckyCommandType_Cmd        },
    {"PAUSE",          KEY_PAUSE,        DuckyCommandType_Cmd        },
    {"END",            KEY_END,          DuckyCommandType_Cmd        },
    {"HOME",           KEY_HOME,         DuckyCommandType_Cmd        },
    {"INSERT",         KEY_INSERT,       DuckyCommandType_Cmd        },
    {"NUMLOCK",        LED_NUMLOCK,      DuckyCommandType_Cmd        },
    {"PAGEUP",         KEY_PAGE_UP,      DuckyCommandType_Cmd        },
    {"PAGEDOWN",       KEY_PAGE_DOWN,    DuckyCommandType_Cmd        },
    {"PRINTSCREEN",    KEY_PRINT_SCREEN, DuckyCommandType_Cmd        },
    {"SCROLLOCK",      KEY_SCROLL_LOCK,  DuckyCommandType_Cmd        },
    {"MENU",           KEY_MENU,         DuckyCommandType_Cmd        },
    {"F1",             KEY_F1,           DuckyCommandType_Cmd        },
    {"F2",             KEY_F2,           DuckyCommandType_Cmd        },
    {"F3",             KEY_F3,           DuckyCommandType_Cmd        },
    {"F4",             KEY_F4,           DuckyCommandType_Cmd        },
    {"F5",             KEY_F5,           DuckyCommandType_Cmd        },
    {"F6",             KEY_F6,           DuckyCommandType_Cmd        },
    {"F7",             KEY_F7,           DuckyCommandType_Cmd        },
    {"F8",             KEY_F8,           DuckyCommandType_Cmd        },
    {"F9",             KEY_F9,           DuckyCommandType_Cmd        },
    {"F10",            KEY_F10,          DuckyCommandType_Cmd        },
    {"F11",            KEY_F11,          DuckyCommandType_Cmd        },
    {"F12",            KEY_F12,          DuckyCommandType_Cmd        },
    {"SPACE",          KEY_SPACE,        DuckyCommandType_Cmd        }
};

void BadUSBModules::beginKB(HIDInterface *&hid, const uint8_t *layout, bool usingble) {
    if (usingble) {
        hid = new BleKeyboard("ESP32 Attack Tool - BadUSB", "ESP32AttackTool", 100);
    } else {
        #if !defined(BOARD_ESP32_C3_MINI)
        hid = new USBHIDKeyboard();
        USB.begin();
        #endif
    }

    if (usingble) {
        if (hid->isConnected()) {
            hid->setLayout(layout);
            Serial.println("[INFO] Set Layout for BadUSB (BLE) Successfully");
            return;
        }
        hid->begin(layout);
    } else {
        #if !defined(BOARD_ESP32_C3_MINI)
        hid->begin(layout);
        #endif
    }
}

void BadUSBModules::launchBadUSB(HIDInterface *&hid, const uint8_t* layout, bool usingble) {
    if (hid == nullptr) {
        beginKB(hid, layout, usingble);
    }
    // (WIP)
}
