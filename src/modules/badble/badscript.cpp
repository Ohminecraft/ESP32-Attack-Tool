#include "badscript_header.h"

/*
    * badscript.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

HIDInterface *hid_ble = nullptr;
uint8_t currentkbmode = 0;

uint8_t keyboardLayout;

enum DuckyCommandType {
    DuckyCommandType_Cmd,
    DuckyCommandType_Print,
    DuckyCommandType_Delay,
    DuckyCommandType_Comment,
    DuckyCommandType_Repeat,
    DuckyCommandType_Combination,
    DuckyCommandType_WaitForButtonPress,
    DuckyCommandType_AltChar,
    DuckyCommandType_AltString,
    DuckyCommandType_StringDelay,
    DuckyCommandType_DefaultStringDelay
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
    {"CTRL-ALT",       KEY_LEFT_CTRL, KEY_LEFT_ALT,     0             },
    {"CTRL-SHIFT",     KEY_LEFT_CTRL, KEY_LEFT_SHIFT,   0             },
    {"CTRL-GUI",       KEY_LEFT_CTRL, KEY_LEFT_GUI,     0             },
    {"CTRL-ESCAPE",    KEY_LEFT_CTRL, KEY_ESC,          0             },
    {"ALT-SHIFT",      KEY_LEFT_ALT,  KEY_LEFT_SHIFT,   0             },
    {"ALT-GUI",        KEY_LEFT_ALT,  KEY_LEFT_GUI,     0             },
    {"GUI-SHIFT",      KEY_LEFT_GUI,  KEY_LEFT_SHIFT,   0             },
    {"GUI-SPACE",      KEY_LEFT_GUI,  KEY_SPACE,        0             },
    {"CTRL-ALT-SHIFT", KEY_LEFT_CTRL, KEY_LEFT_ALT,     KEY_LEFT_SHIFT},
    {"CTRL-ALT-GUI",   KEY_LEFT_CTRL, KEY_LEFT_ALT,     KEY_LEFT_GUI  },
    {"ALT-SHIFT-GUI",  KEY_LEFT_ALT,  KEY_LEFT_SHIFT,   KEY_LEFT_GUI  },
    {"CTRL-SHIFT-GUI", KEY_LEFT_CTRL, KEY_LEFT_SHIFT,   KEY_LEFT_GUI  },
    {"SYSREQ",         KEY_LEFT_ALT,  KEY_PRINT_SCREEN, 0             }
};

const DuckyCommand duckyCmds[]{
    {"REM",                   0,                DuckyCommandType_Comment           },
    {"//",                    0,                DuckyCommandType_Comment           },
    {"STRING",                0,                DuckyCommandType_Print             },
    {"STRINGLN",              0,                DuckyCommandType_Print             },
    {"DELAY",                 0,                DuckyCommandType_Delay             },
    {"DEFAULTDELAY",          DEF_DELAY,        DuckyCommandType_Delay             },
    {"DEFAULT_DELAY",         DEF_DELAY,        DuckyCommandType_Delay             },
    {"STRING_DELAY",          0,                DuckyCommandType_StringDelay       },
    {"STRINGDELAY",           0,                DuckyCommandType_StringDelay       },
    {"DEFAULT_STRING_DELAY",  0,                DuckyCommandType_DefaultStringDelay},
    {"DEFAULTSTRINGDELAY",    0,                DuckyCommandType_DefaultStringDelay},
    {"REPEAT",                0,                DuckyCommandType_Repeat            },
    {"WAIT_FOR_BUTTON_PRESS", 0,                DuckyCommandType_WaitForButtonPress},
    {"ALTCHAR",               0,                DuckyCommandType_AltChar           },
    {"ALTSTRING",             0,                DuckyCommandType_AltString         },
    {"ALTCODE",               0,                DuckyCommandType_AltString         },
    {"CTRL-ALT",              0,                DuckyCommandType_Combination       },
    {"CTRL-SHIFT",            0,                DuckyCommandType_Combination       },
    {"CTRL-GUI",              0,                DuckyCommandType_Combination       },
    {"CTRL-ESCAPE",           0,                DuckyCommandType_Combination       },
    {"ALT-SHIFT",             0,                DuckyCommandType_Combination       },
    {"ALT-GUI",               0,                DuckyCommandType_Combination       },
    {"GUI-SHIFT",             0,                DuckyCommandType_Combination       },
    {"GUI-SPACE",             0,                DuckyCommandType_Combination       },
    {"CTRL-ALT-SHIFT",        0,                DuckyCommandType_Combination       },
    {"CTRL-ALT-GUI",          0,                DuckyCommandType_Combination       },
    {"ALT-SHIFT-GUI",         0,                DuckyCommandType_Combination       },
    {"CTRL-SHIFT-GUI",        0,                DuckyCommandType_Combination       },
    {"SYSREQ",                0,                DuckyCommandType_Combination       },
    {"BACKSPACE",             KEYBACKSPACE,     DuckyCommandType_Cmd               },
    {"DELETE",                KEY_DELETE,       DuckyCommandType_Cmd               },
    {"ALT",                   KEY_LEFT_ALT,     DuckyCommandType_Cmd               },
    {"CTRL",                  KEY_LEFT_CTRL,    DuckyCommandType_Cmd               },
    {"CONTROL",               KEY_LEFT_CTRL,    DuckyCommandType_Cmd               },
    {"GUI",                   KEY_LEFT_GUI,     DuckyCommandType_Cmd               },
    {"WINDOWS",               KEY_LEFT_GUI,     DuckyCommandType_Cmd               },
    {"SHIFT",                 KEY_LEFT_SHIFT,   DuckyCommandType_Cmd               },
    {"ESCAPE",                KEY_ESC,          DuckyCommandType_Cmd               },
    {"ESC",                   KEY_ESC,          DuckyCommandType_Cmd               },
    {"TAB",                   KEYTAB,           DuckyCommandType_Cmd               },
    {"ENTER",                 KEY_RETURN,       DuckyCommandType_Cmd               },
    {"DOWNARROW",             KEY_DOWN_ARROW,   DuckyCommandType_Cmd               },
    {"DOWN",                  KEY_DOWN_ARROW,   DuckyCommandType_Cmd               },
    {"LEFTARROW",             KEY_LEFT_ARROW,   DuckyCommandType_Cmd               },
    {"LEFT",                  KEY_LEFT_ARROW,   DuckyCommandType_Cmd               },
    {"RIGHTARROW",            KEY_RIGHT_ARROW,  DuckyCommandType_Cmd               },
    {"RIGHT",                 KEY_RIGHT_ARROW,  DuckyCommandType_Cmd               },
    {"UPARROW",               KEY_UP_ARROW,     DuckyCommandType_Cmd               },
    {"UP",                    KEY_UP_ARROW,     DuckyCommandType_Cmd               },
    {"BREAK",                 KEY_PAUSE,        DuckyCommandType_Cmd               },
    {"PAUSE",                 KEY_PAUSE,        DuckyCommandType_Cmd               },
    {"CAPSLOCK",              KEY_CAPS_LOCK,    DuckyCommandType_Cmd               },
    {"END",                   KEY_END,          DuckyCommandType_Cmd               },
    {"HOME",                  KEY_HOME,         DuckyCommandType_Cmd               },
    {"INSERT",                KEY_INSERT,       DuckyCommandType_Cmd               },
    {"NUMLOCK",               LED_NUMLOCK,      DuckyCommandType_Cmd               },
    {"PAGEUP",                KEY_PAGE_UP,      DuckyCommandType_Cmd               },
    {"PAGEDOWN",              KEY_PAGE_DOWN,    DuckyCommandType_Cmd               },
    {"PRINTSCREEN",           KEY_PRINT_SCREEN, DuckyCommandType_Cmd               },
    {"SCROLLOCK",             KEY_SCROLL_LOCK,  DuckyCommandType_Cmd               },
    {"MENU",                  KEY_MENU,         DuckyCommandType_Cmd               },
    {"APP",                   KEY_MENU,         DuckyCommandType_Cmd               },
    {"F1",                    KEY_F1,           DuckyCommandType_Cmd               },
    {"F2",                    KEY_F2,           DuckyCommandType_Cmd               },
    {"F3",                    KEY_F3,           DuckyCommandType_Cmd               },
    {"F4",                    KEY_F4,           DuckyCommandType_Cmd               },
    {"F5",                    KEY_F5,           DuckyCommandType_Cmd               },
    {"F6",                    KEY_F6,           DuckyCommandType_Cmd               },
    {"F7",                    KEY_F7,           DuckyCommandType_Cmd               },
    {"F8",                    KEY_F8,           DuckyCommandType_Cmd               },
    {"F9",                    KEY_F9,           DuckyCommandType_Cmd               },
    {"F10",                   KEY_F10,          DuckyCommandType_Cmd               },
    {"F11",                   KEY_F11,          DuckyCommandType_Cmd               },
    {"F12",                   KEY_F12,          DuckyCommandType_Cmd               },
    {"F13",                   KEY_F13,          DuckyCommandType_Cmd               },
    {"F14",                   KEY_F14,          DuckyCommandType_Cmd               },
    {"F15",                   KEY_F15,          DuckyCommandType_Cmd               },
    {"F16",                   KEY_F16,          DuckyCommandType_Cmd               },
    {"F17",                   KEY_F17,          DuckyCommandType_Cmd               },
    {"F18",                   KEY_F18,          DuckyCommandType_Cmd               },
    {"F19",                   KEY_F19,          DuckyCommandType_Cmd               },
    {"F20",                   KEY_F20,          DuckyCommandType_Cmd               },
    {"F21",                   KEY_F21,          DuckyCommandType_Cmd               },
    {"F22",                   KEY_F22,          DuckyCommandType_Cmd               },
    {"F23",                   KEY_F23,          DuckyCommandType_Cmd               },
    {"F24",                   KEY_F24,          DuckyCommandType_Cmd               },
    {"SPACE",                 KEY_SPACE,        DuckyCommandType_Cmd               },
    {"FN",                    KEYFN,            DuckyCommandType_Cmd               },
    {"GLOBE",                 KEYFN,            DuckyCommandType_Cmd               },
};

DuckyCommand *findDuckyCommand(const char *cmd) {
    for (auto &cmds : duckyCmds) {
        if (strcmp(cmd, cmds.command) == 0) { return const_cast<DuckyCommand *>(&cmds); }
    }
    return nullptr;
}

DuckyCombination *findDuckyCombination(const char *cmd) {
    for (auto &comb : duckyComb) {
        if (strcmp(cmd, comb.command) == 0) { return const_cast<DuckyCombination *>(&comb); }
    }
    return nullptr;
}

void sendAltChar(HIDInterface *hid, uint8_t charCode) {
    // Hold ALT key
    hid->press(KEY_LEFT_ALT);
    delay(espatsettings.badscriptKeyDelay);

    // Convert char code to 3-digit padded string (standard ALT code format)
    String codeStr = String(charCode);
    if (codeStr.length() < 3) {
        // Pad with leading zeros for proper ALT codes (e.g., 065 instead of 65)
        while (codeStr.length() < 3) { codeStr = "0" + codeStr; }
    }

    // Send each digit using numpad keys
    for (int i = 0; i < codeStr.length(); i++) {
        char digit = codeStr[i];
        uint8_t numpadKey = 0;

        switch (digit) {
            case '0': numpadKey = KEY_KP_0; break;
            case '1': numpadKey = KEY_KP_1; break;
            case '2': numpadKey = KEY_KP_2; break;
            case '3': numpadKey = KEY_KP_3; break;
            case '4': numpadKey = KEY_KP_4; break;
            case '5': numpadKey = KEY_KP_5; break;
            case '6': numpadKey = KEY_KP_6; break;
            case '7': numpadKey = KEY_KP_7; break;
            case '8': numpadKey = KEY_KP_8; break;
            case '9': numpadKey = KEY_KP_9; break;
            default: continue; // Skip invalid characters
        }

        hid->press(numpadKey);
        delay(espatsettings.badscriptKeyDelay);
        hid->release(numpadKey);
        delay(espatsettings.badscriptKeyDelay);
    }

    // Release ALT key (this triggers the character input)
    hid->release(KEY_LEFT_ALT);
    delay(espatsettings.badscriptKeyDelay);
}

void sendAltString(HIDInterface *hid, const String &text) {
    for (int i = 0; i < text.length(); i++) {
        uint8_t charCode = (uint8_t)text[i];
        sendAltChar(hid, charCode);
        delay(espatsettings.badscriptKeyDelay);
    }
}

void BadScriptModules::beginKB(HIDInterface *&hid, const uint8_t *layout, uint8_t mode) {
    if (hid == nullptr) hid = new BleKeyboard(espatsettings.bleName, "ESP32AttackTool", 100, espatsettings.usingSwiftpairForBLEUtilty);
    if (hid->isConnected()) {
        hid->setLayout(layout);
        hid->setDelay(50); // Set a default delay for commands, can be adjusted with STRING_DELAY and DEFAULT_STRING_DELAY
        return;
    }
    hid->begin(layout, mode);  currentkbmode = mode;
}

void BadScriptModules::beginLayout(HIDInterface *&hid) {
    if (hid == nullptr) {
        if (keyboardLayout == Layout_en_US) {
            beginKB(hid, KeyboardLayout_en_US);
        } else if (keyboardLayout == Layout_pt_BR) {
            beginKB(hid, KeyboardLayout_pt_BR);
        } else if (keyboardLayout == Layout_pt_PT) {
            beginKB(hid, KeyboardLayout_pt_PT);
        } else if (keyboardLayout == Layout_fr_FR) {
            beginKB(hid, KeyboardLayout_fr_FR);
        } else if (keyboardLayout == Layout_es_ES) {
            beginKB(hid, KeyboardLayout_es_ES);
        } else if (keyboardLayout == Layout_it_IT) {
            beginKB(hid, KeyboardLayout_it_IT);
        } else if (keyboardLayout == Layout_en_UK) {
            beginKB(hid, KeyboardLayout_en_UK);
        } else if (keyboardLayout == Layout_de_DE) {
            beginKB(hid, KeyboardLayout_de_DE);
        } else if (keyboardLayout == Layout_sv_SE) {
            beginKB(hid, KeyboardLayout_sv_SE);
        } else if (keyboardLayout == Layout_da_DK) {
            beginKB(hid, KeyboardLayout_da_DK);
        } else if (keyboardLayout == Layout_hu_HU) {
            beginKB(hid, KeyboardLayout_hu_HU);
        } else if (keyboardLayout == Layout_tr_TR) {
            beginKB(hid, KeyboardLayout_tr_TR);
        } else if (keyboardLayout == Layout_si_SI) {
            beginKB(hid, KeyboardLayout_si_SI);
        }
       
    }
}

bool BadScriptModules::isConnected(HIDInterface *&hid) {
    if (hid == nullptr) return false;
    else return hid->isConnected();
}

void BadScriptModules::launchBadScript(String badscriptScript, HIDInterface *&hid) {
    if (!sdcard.isExists(badscriptScript) || badscriptScript == "") return;
    File payloadFile = sdcard.getFile(badscriptScript, "r");
    if (!payloadFile) {
        Serial.println("[ERROR] Failed to open payload file: " + badscriptScript);
        return;
    }
    Serial.println("[INFO] Launching BadScript script: " + badscriptScript);
    String lineContent = "";
    String Command = "";
    char Cmd[25];
    String Argument = "";
    String RepeatTmp = "";

    // String delay variables
    static int nextStringDelay = -1;
    static int defaultStringDelay = espatsettings.badscriptKeyDelay;

    hid->releaseAll();

    uint32_t startMillisBADUSBBLE = millis();

    while (payloadFile.available()) {
        // CRLF is a combination of two control characters: the "Carriage Return" represented by
        // the character "\r" and the "Line Feed" represented by the character "\n".
        lineContent = payloadFile.readStringUntil('\n');
        if (lineContent.endsWith("\r")) lineContent.remove(lineContent.length() - 1);

        if (lineContent.length() == 0) continue; // skip empty lines

        int spaceIndex = lineContent.indexOf(' ');

        // Check if this is a REPEAT command
        if (spaceIndex > 0 && lineContent.substring(0, spaceIndex) == "REPEAT") {
            RepeatTmp = lineContent.substring(spaceIndex + 1);
            if (RepeatTmp.toInt() <= 0) {
                RepeatTmp = "1";
                Serial.println("[INFO] Do 'REPEAT' command argument NaN, repeating once");
            }
        } else if (spaceIndex == -1 && lineContent == "REPEAT") {
            RepeatTmp = "1";
            Serial.println("[WARN] Do 'REPEAT' command without argument, repeating once");
        } else {
            if (spaceIndex > 0) {
                Command = lineContent.substring(0, spaceIndex);
                Argument = lineContent.substring(spaceIndex + 1);
            } else {
                Command = lineContent;
                Argument = "";
            }
            strcpy(Cmd, Command.c_str());
            RepeatTmp = "1";
        }

        uint16_t i;
        uint16_t repeatCount = RepeatTmp.toInt();
        for (i = 0; i < repeatCount; i++) {
            DuckyCommand *PriCmd = findDuckyCommand(Cmd);
            DuckyCommand *ArgCmd = findDuckyCommand(Argument.c_str());

            if (PriCmd != nullptr) {
                // REM comment lines are processed here
                if (PriCmd->type == DuckyCommandType_Comment) {
                    // Do nothing for comments
                }
                // STRING and STRINGLN are processed here
                else if (PriCmd->type == DuckyCommandType_Print) {
                    // Set appropriate delay for this STRING command
                    int currentDelay = (nextStringDelay >= 0) ? nextStringDelay : defaultStringDelay;
                    hid->setDelay(currentDelay);

                    hid->print(Argument);
                    if (strcmp(PriCmd->command, "STRINGLN") == 0) hid->println();

                    // Reset one-time delay after use
                    if (nextStringDelay >= 0) { nextStringDelay = -1; }
                }
                // DELAY and DEFAULTDELAY are processed here
                else if (PriCmd->type == DuckyCommandType_Delay) {
                    if ((int)PriCmd->key > 0) delay(DEF_DELAY); // Default delay is 10ms
                    else {
                        int delayTime = Argument.toInt();
                        if (delayTime > 0) delay(delayTime);
                        else delay(DEF_DELAY);
                    }
                }
                // ALTCHAR command is processed here
                else if (PriCmd->type == DuckyCommandType_AltChar) {
                    int charCode = Argument.toInt();
                    if (charCode > 0 && charCode <= 255) { sendAltChar(hid, (uint8_t)charCode); }
                }
                // ALTSTRING and ALTCODE commands are processed here
                else if (PriCmd->type == DuckyCommandType_AltString) {
                    sendAltString(hid, Argument);
                }
                // STRING_DELAY and STRINGDELAY commands are processed here
                else if (PriCmd->type == DuckyCommandType_StringDelay) {
                    int delayValue = Argument.toInt();
                    if (delayValue >= 0) { nextStringDelay = delayValue; }
                }
                // DEFAULT_STRING_DELAY and DEFAULTSTRINGDELAY commands are processed here
                else if (PriCmd->type == DuckyCommandType_DefaultStringDelay) {
                    int delayValue = Argument.toInt();
                    if (delayValue >= 0) { defaultStringDelay = delayValue; }
                }
                // Normal commands are processed here
                else if (PriCmd->type == DuckyCommandType_Cmd) {
                    hid->press(PriCmd->key);
                }
                // Combinations are processed here
                else if (PriCmd->type == DuckyCommandType_Combination) {
                    DuckyCombination *comb = findDuckyCombination(Cmd);
                    if (comb != nullptr) {
                        hid->press(comb->key1);
                        hid->press(comb->key2);
                        if (comb->key3 != 0) hid->press(comb->key3);
                    }
                }

                // Send keys
                if (PriCmd->type != DuckyCommandType_Comment) {
                    if (ArgCmd != nullptr && PriCmd != nullptr && ArgCmd->type == DuckyCommandType_Cmd &&
                        PriCmd->type == DuckyCommandType_Cmd) {
                        hid->press(ArgCmd->key);
                    } else if (PriCmd != nullptr && PriCmd->type == DuckyCommandType_Cmd &&
                               Argument.length() > 0) {
                        for (int idx = 0; idx < Argument.length(); idx++) {
                            hid->press(Argument.charAt(idx));
                        }
                    }
                    hid->releaseAll();
                }
            }
        }
    }
}

void BadScriptModules::mediaController(HIDInterface *&hid, MediaCommand command) {
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

void BadScriptModules::Keymote(HIDInterface *&hid, KeymoteCommand key) {
    if (key == KEYMOTE_UP) hid->press(KEY_UP_ARROW);
    if (key == KEYMOTE_DOWN) hid->press(KEY_DOWN_ARROW);
    if (key == KEYMOTE_LEFT) hid->press(KEY_LEFT_ARROW);
    if (key == KEYMOTE_RIGHT) hid->press(KEY_RIGHT_ARROW);
    hid->releaseAll();
}

void BadScriptModules::tiktokScroll(HIDInterface *&hid, TikTokScrollCommand cmd) {
    if (cmd == SCROLL_DOWN) { // Finally Same Flipper Zero :) Support for IOS (below 17 :/)
        hid->wheel(-24);
        hid->wheel(-38);
        hid->wheel(-24);
    }
    else if (cmd == SCROLL_UP) {
        hid->wheel(24);
        hid->wheel(38);
        hid->wheel(24);
    }
    else if (cmd == LIKE_VIDEO) {
        hid->click();
        hid->releaseMouse();
        vTaskDelay(75 / portTICK_PERIOD_MS);
        hid->click();
        hid->releaseMouse();
    }
}
