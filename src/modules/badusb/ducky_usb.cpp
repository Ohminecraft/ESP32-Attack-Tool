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

uint8_t keyboardLayout;

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

void BadUSBModules::beginKB(HIDInterface *&hid, const uint8_t *layout, bool usingble, uint8_t mode) {
    if (usingble) {
        if (hid == nullptr) hid = new BleKeyboard(espatsettings.bleName.c_str(), "ESP32AttackTool", 100);
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
        if (!hid->isConnected()) hid->begin(layout, mode);
    } else {
        #if !defined(BOARD_ESP32_C3_MINI)
        #endif
    }
}

void BadUSBModules::beginLayout(HIDInterface *&hid, bool usingble) {
    if (hid == nullptr) {
        if (keyboardLayout == Layout_en_US) {
            beginKB(hid, KeyboardLayout_en_US, usingble);
        } else if (keyboardLayout == Layout_pt_BR) {
            beginKB(hid, KeyboardLayout_pt_BR, usingble);
        } else if (keyboardLayout == Layout_pt_PT) {
            beginKB(hid, KeyboardLayout_pt_PT, usingble);
        } else if (keyboardLayout == Layout_fr_FR) {
            beginKB(hid, KeyboardLayout_fr_FR, usingble);
        } else if (keyboardLayout == Layout_es_ES) {
            beginKB(hid, KeyboardLayout_es_ES, usingble);
        } else if (keyboardLayout == Layout_it_IT) {
            beginKB(hid, KeyboardLayout_it_IT, usingble);
        } else if (keyboardLayout == Layout_en_UK) {
            beginKB(hid, KeyboardLayout_en_UK, usingble);
        } else if (keyboardLayout == Layout_de_DE) {
            beginKB(hid, KeyboardLayout_de_DE, usingble);
        } else if (keyboardLayout == Layout_sv_SE) {
            beginKB(hid, KeyboardLayout_sv_SE, usingble);
        } else if (keyboardLayout == Layout_da_DK) {
            beginKB(hid, KeyboardLayout_da_DK, usingble);
        } else if (keyboardLayout == Layout_hu_HU) {
            beginKB(hid, KeyboardLayout_hu_HU, usingble);
        } else if (keyboardLayout == Layout_tr_TR) {
            beginKB(hid, KeyboardLayout_tr_TR, usingble);
        } else if (keyboardLayout == Layout_si_SI) {
            beginKB(hid, KeyboardLayout_si_SI, usingble);
        }
       
    }
}

bool BadUSBModules::isConnected(HIDInterface *&hid) {
    return hid->isConnected();
}

void BadUSBModules::launchBadUSB(String badusbScript, HIDInterface *&hid) {
    if (!sdcard.isExists(badusbScript) || badusbScript == "") return;
    File payloadFile = sdcard.getFile(badusbScript, "r");
    if (!payloadFile) {
        Serial.println("[ERROR] Failed to open payload file: " + badusbScript);
        return;
    }
    Serial.println("[INFO] Launching BadUSB script: " + badusbScript);
    String lineContent = "";
    String Command = "";
    char Cmd[15];
    String Argument = "";
    String RepeatTmp = "";
    char ArgChar = '\0';
    bool ArgIsCmd; // Verifies if the Argument is DELETE, TAB or F1-F12

    hid->releaseAll();

    while (payloadFile.available()) {
        // CRLF is a combination of two control characters: the "Carriage Return" represented by
        // the character "\r" and the "Line Feed" represented by the character "\n".
        lineContent = payloadFile.readStringUntil('\n');
        if (lineContent.endsWith("\r")) lineContent.remove(lineContent.length() - 1);

        RepeatTmp = lineContent.substring(0, lineContent.indexOf(' '));
        RepeatTmp = RepeatTmp.c_str();
        if (RepeatTmp == "REPEAT") {
            if (lineContent.indexOf(' ') > 0) {
                // how many times it will repeat, using .toInt() conversion;
                RepeatTmp = lineContent.substring(lineContent.indexOf(' ') + 1);
                if (RepeatTmp.toInt() == 0) {
                    RepeatTmp = "1";
                    Serial.println("[INFO] Do 'REPEAT' command argument NaN, repeating once");
                }
            } else {
                RepeatTmp = "1";
                Serial.println("[INFO] Do 'REPEAT' command without argument, repeating once");
            }
        } else {
            Command = lineContent.substring(0, lineContent.indexOf(' ')); // get the Command
            strcpy(Cmd, Command.c_str());                                 // get the cmd
            if (lineContent.indexOf(' ') > 0)
                Argument = lineContent.substring(lineContent.indexOf(' ') + 1); // get the argument
            else Argument = "";
            RepeatTmp = "1";
        }
        uint16_t i;
        ArgIsCmd = false;
        Argument = Argument.c_str();
        ArgChar = Argument.charAt(0);
        for (i = 0; i < RepeatTmp.toInt(); i++) {
            DuckyCommand *ArgCmd = nullptr;
            DuckyCommand *PriCmd = nullptr;
            ArgIsCmd = false;
            for (auto cmds : duckyCmds) {
                if (strcmp(Cmd, cmds.command) == 0) {
                    PriCmd = &cmds;
                    // STRING and STRINGLN are processed here
                    if (cmds.type == DuckyCommandType_Print) {
                        hid->print(Argument);
                        if (strcmp(cmds.command, "STRINGLN") == 0) hid->println();
                        break;
                    }
                    // DELAY and DEFAULTDELAY are processed here
                    else if (cmds.type == DuckyCommandType_Delay) {
                        if ((int)cmds.key > 0) delay(DEF_DELAY); // Default delay is 100ms
                        else if (Argument.toInt() > 0) delay(Argument.toInt());
                        else delay(DEF_DELAY);
                        break;
                    }
                    // Comment line is porocessed Here
                    else if (cmds.type == DuckyCommandType_Comment) {
                        yield(); // do nothing, just wait for the next line
                        break;
                    }
                    // Normal commands are processed here
                    else if (cmds.type == DuckyCommandType_Cmd) {
                        hid->press(cmds.key);
                        ArgIsCmd = true;
                    }
                    // Combinations are processed here
                    else if (cmds.type == DuckyCommandType_Combination) {
                        for (auto comb : duckyComb) {
                            if (strcmp(Cmd, comb.command) == 0) {
                                hid->press(comb.key1);
                                hid->press(comb.key2);
                                if (comb.key3 != 0) hid->press(comb.key3);
                                ArgIsCmd = true;
                            }
                        }
                    }
                }
                // check if the Argument contains a command
                if (strcmp(Argument.c_str(), cmds.command) == 0) { ArgCmd = &cmds; }
            }

            if (ArgCmd != nullptr && PriCmd != nullptr) {
                if (ArgCmd->type == DuckyCommandType_Cmd) { hid->press(ArgCmd->key); }
            } else if (ArgIsCmd && PriCmd != nullptr) {
                if (ArgChar != '\0') hid->press(ArgChar);
            }
            hid->releaseAll();

            if (PriCmd == nullptr) {
                Serial.println("[INFO] Do '" + Command + "' command, but not supported, running as STRINGLN");
                if (Argument != "") {
                    hid->println(Command + " " + Argument);
                } else {
                    hid->println(Command);
                }
            } else {
                Serial.print("[INFO] Do '" + Command + "' command with argument: ");
                if (Argument != "") {
                    Serial.println(Argument);
                } else {
                    Serial.println();
                }
            }
        }
    }
    payloadFile.close();
    hid->releaseAll();
}

void BadUSBModules::mediaController(HIDInterface *&hid, MediaCommand command) {
    if (command == MEDIA_SCREENSHOT) hid->press(KEY_PRINT_SCREEN);
    else if (command == MEDIA_PLAY_PAUSE) hid->press(KEY_MEDIA_PLAY_PAUSE);
    else if (command == MEDIA_STOP) hid->press(KEY_MEDIA_STOP);
    else if (command == MEDIA_NEXT_TRACK) hid->press(KEY_MEDIA_NEXT_TRACK);
    else if (command == MEDIA_PREV_TRACK) hid->press(KEY_MEDIA_PREVIOUS_TRACK);
    else if (command == MEDIA_VOL_UP) hid->press(KEY_MEDIA_VOLUME_UP);
    else if (command == MEDIA_VOL_DOWN) hid->press(KEY_MEDIA_VOLUME_DOWN);
    else if (command == MEDIA_MUTE) hid->press(KEY_MEDIA_MUTE);
    hid->releaseAll();
}   

void BadUSBModules::Keymote(HIDInterface *&hid, KeymoteCommand key) {
    if (key == KEYMOTE_UP) hid->press(KEY_UP_ARROW);
    if (key == KEYMOTE_DOWN) hid->press(KEY_DOWN_ARROW);
    if (key == KEYMOTE_LEFT) hid->press(KEY_LEFT_ARROW);
    if (key == KEYMOTE_RIGHT) hid->press(KEY_RIGHT_ARROW);
    hid->releaseAll();
}

void BadUSBModules::tiktokScroll(HIDInterface *&hid, TikTokScrollCommand cmd) {
    hid->move(0, 0);
    if (cmd == SCROLL_DOWN) {
        hid->pressMouse(MOUSE_LEFT);   // Giữ
        for (int i = 0; i < 5; i++) { // Vuốt dài hơn
            hid->move(0, -30); // Mỗi lần dịch 30 pixel
            vTaskDelay(12 / portTICK_PERIOD_MS);
        }
        hid->releaseMouse(MOUSE_LEFT); // Thả
        for (int i = 0; i < 5; i++) { // Vuốt dài hơn
            hid->move(0, 30); // Mỗi lần dịch 30 pixel
            vTaskDelay(12 / portTICK_PERIOD_MS);
        }
    }
    else if (cmd == SCROLL_UP) {
        hid->pressMouse(MOUSE_LEFT);   // Giữ
        for (int i = 0; i < 5; i++) { // Vuốt dài hơn
            hid->move(0, 30); // Mỗi lần dịch 30 pixel
            vTaskDelay(12 / portTICK_PERIOD_MS);
        }
        hid->releaseMouse(MOUSE_LEFT); // Thả
        for (int i = 0; i < 5; i++) { // Vuốt dài hơn
            hid->move(0, -30); // Mỗi lần dịch 30 pixel
            vTaskDelay(12 / portTICK_PERIOD_MS);
        }
    }
    else if (cmd == LIKE_VIDEO) {
        hid->click();
        hid->releaseMouse();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        hid->click();
    }
    hid->releaseMouse();
}

void BadUSBModules::endHID(HIDInterface *&hid) {
    hid->end();
}
