#ifndef __KEYS_H__
#define __KEYS_H__
#include <Arduino.h>
//  Low level key report: up to 6 keys and shift, ctrl etc at once
typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
} KeyReport;

// Supported keyboard layouts
extern const uint8_t KeyboardLayout_de_DE[];
extern const uint8_t KeyboardLayout_en_US[];
extern const uint8_t KeyboardLayout_en_UK[];
extern const uint8_t KeyboardLayout_es_ES[];
extern const uint8_t KeyboardLayout_fr_FR[];
extern const uint8_t KeyboardLayout_it_IT[];
extern const uint8_t KeyboardLayout_pt_PT[];
extern const uint8_t KeyboardLayout_pt_BR[];
extern const uint8_t KeyboardLayout_sv_SE[];
extern const uint8_t KeyboardLayout_da_DK[];
extern const uint8_t KeyboardLayout_hu_HU[];
extern const uint8_t KeyboardLayout_tr_TR[];
extern const uint8_t KeyboardLayout_si_SI[];

#define KEY_LEFT_CTRL 0x80
#define KEY_LEFT_SHIFT 0x81
#define KEY_LEFT_ALT 0x82
#define KEY_LEFT_GUI 0x83
#define KEY_RIGHT_CTRL 0x84
#define KEY_RIGHT_SHIFT 0x85
#define KEY_RIGHT_ALT 0x86
#define KEY_RIGHT_GUI 0x87

#define KEY_UP_ARROW 0xDA
#define KEY_DOWN_ARROW 0xD9
#define KEY_LEFT_ARROW 0xD8
#define KEY_RIGHT_ARROW 0xD7
#define KEYBACKSPACE 0xB2 // changed from KEY_BACKSPACE due to compatibility to cardputer keyboard
#define KEYTAB 0xB3
#define KEY_RETURN 0xB0
#define KEY_ESC 0xB1
#define KEY_PRINT_SCREEN 0xCE
#define KEY_SCROLL_LOCK 0xCF
#define KEY_PAUSE 0xD0
#define KEY_INSERT 0xD1
#define KEY_DELETE 0xD4
#define KEY_PAGE_UP 0xD3
#define KEY_PAGE_DOWN 0xD6
#define KEY_HOME 0xD2
#define KEY_END 0xD5
#define KEY_MENU 0xED
#define KEY_CAPS_LOCK 0xC1
#define KEY_F1 0xC2
#define KEY_F2 0xC3
#define KEY_F3 0xC4
#define KEY_F4 0xC5
#define KEY_F5 0xC6
#define KEY_F6 0xC7
#define KEY_F7 0xC8
#define KEY_F8 0xC9
#define KEY_F9 0xCA
#define KEY_F10 0xCB
#define KEY_F11 0xCC
#define KEY_F12 0xCD
#define KEY_F13 0xF0
#define KEY_F14 0xF1
#define KEY_F15 0xF2
#define KEY_F16 0xF3
#define KEY_F17 0xF4
#define KEY_F18 0xF5
#define KEY_F19 0xF6
#define KEY_F20 0xF7
#define KEY_F21 0xF8
#define KEY_F22 0xF9
#define KEY_F23 0xFA
#define KEY_F24 0xFB

#define LED_NUMLOCK 0x01
#define LED_CAPSLOCK 0x02
#define LED_SCROLLLOCK 0x04
#define LED_COMPOSE 0x08
#define LED_KANA 0x10
#define KEY_SPACE 0x2c

#define BUTTON_1      0x1
#define BUTTON_2      0x2
#define BUTTON_3      0x3
#define BUTTON_4      0x4
#define BUTTON_5      0x5
#define BUTTON_6      0x6
#define BUTTON_7      0x7
#define BUTTON_8      0x8
#define BUTTON_9      0x9
#define BUTTON_10     0xa
#define BUTTON_11     0xb
#define BUTTON_12     0xc
#define BUTTON_13     0xd
#define BUTTON_14     0xe
#define BUTTON_15     0xf
#define BUTTON_16    0x10
#define BUTTON_17    0x11
#define BUTTON_18    0x12
#define BUTTON_19    0x13
#define BUTTON_20    0x14
#define BUTTON_21    0x15
#define BUTTON_22    0x16
#define BUTTON_23    0x17
#define BUTTON_24    0x18
#define BUTTON_25    0x19
#define BUTTON_26    0x1a
#define BUTTON_27    0x1b
#define BUTTON_28    0x1c
#define BUTTON_29    0x1d
#define BUTTON_30    0x1e
#define BUTTON_31    0x1f
#define BUTTON_32    0x20
#define BUTTON_33    0x21
#define BUTTON_34    0x22
#define BUTTON_35    0x23
#define BUTTON_36    0x24
#define BUTTON_37    0x25
#define BUTTON_38    0x26
#define BUTTON_39    0x27
#define BUTTON_40    0x28
#define BUTTON_41    0x29
#define BUTTON_42    0x2a
#define BUTTON_43    0x2b
#define BUTTON_44    0x2c
#define BUTTON_45    0x2d
#define BUTTON_46    0x2e
#define BUTTON_47    0x2f
#define BUTTON_48    0x30
#define BUTTON_49    0x31
#define BUTTON_50    0x32
#define BUTTON_51    0x33
#define BUTTON_52    0x34
#define BUTTON_53    0x35
#define BUTTON_54    0x36
#define BUTTON_55    0x37
#define BUTTON_56    0x38
#define BUTTON_57    0x39
#define BUTTON_58    0x3a
#define BUTTON_59    0x3b
#define BUTTON_60    0x3c
#define BUTTON_61    0x3d
#define BUTTON_62    0x3e
#define BUTTON_63    0x3f
#define BUTTON_64    0x40
#define BUTTON_65    0x41
#define BUTTON_66    0x42
#define BUTTON_67    0x43
#define BUTTON_68    0x44
#define BUTTON_69    0x45
#define BUTTON_70    0x46
#define BUTTON_71    0x47
#define BUTTON_72    0x48
#define BUTTON_73    0x49
#define BUTTON_74    0x4a
#define BUTTON_75    0x4b
#define BUTTON_76    0x4c
#define BUTTON_77    0x4d
#define BUTTON_78    0x4e
#define BUTTON_79    0x4f
#define BUTTON_80    0x50
#define BUTTON_81    0x51
#define BUTTON_82    0x52
#define BUTTON_83    0x53
#define BUTTON_84    0x54
#define BUTTON_85    0x55
#define BUTTON_86    0x56
#define BUTTON_87    0x57
#define BUTTON_88    0x58
#define BUTTON_89    0x59
#define BUTTON_90    0x5a
#define BUTTON_91    0x5b
#define BUTTON_92    0x5c
#define BUTTON_93    0x5d
#define BUTTON_94    0x5e
#define BUTTON_95    0x5f
#define BUTTON_96    0x60
#define BUTTON_97    0x61
#define BUTTON_98    0x62
#define BUTTON_99    0x63
#define BUTTON_100   0x64
#define BUTTON_101   0x65
#define BUTTON_102   0x66
#define BUTTON_103   0x67
#define BUTTON_104   0x68
#define BUTTON_105   0x69
#define BUTTON_106   0x6a
#define BUTTON_107   0x6b
#define BUTTON_108   0x6c
#define BUTTON_109   0x6d
#define BUTTON_110   0x6e
#define BUTTON_111   0x6f
#define BUTTON_112   0x70
#define BUTTON_113   0x71
#define BUTTON_114   0x72
#define BUTTON_115   0x73
#define BUTTON_116   0x74
#define BUTTON_117   0x75
#define BUTTON_118   0x76
#define BUTTON_119   0x77
#define BUTTON_120   0x78
#define BUTTON_121   0x79
#define BUTTON_122   0x7a
#define BUTTON_123   0x7b
#define BUTTON_124   0x7c
#define BUTTON_125   0x7d
#define BUTTON_126   0x7e
#define BUTTON_127   0x7f
#define BUTTON_128   0x80

#define DPAD_CENTERED 	0
#define DPAD_UP 	    	1
#define DPAD_UP_RIGHT 	2
#define DPAD_RIGHT 		  3
#define DPAD_DOWN_RIGHT 4
#define DPAD_DOWN 	   	5
#define DPAD_DOWN_LEFT 	6
#define DPAD_LEFT 		  7
#define DPAD_UP_LEFT 	  8

#define MOUSE_LEFT 1
#define MOUSE_RIGHT 2
#define MOUSE_MIDDLE 4
#define MOUSE_BACK 8
#define MOUSE_FORWARD 16
#define MOUSE_ALL (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)
#define MOUSE_ALL_ALL (MOUSE_ALL | MOUSE_BACK | MOUSE_FORWARD)

typedef uint8_t MediaKeyReport[2];

const MediaKeyReport KEY_MEDIA_NEXT_TRACK = {1, 0};
const MediaKeyReport KEY_MEDIA_PREVIOUS_TRACK = {2, 0};
const MediaKeyReport KEY_MEDIA_STOP = {4, 0};
const MediaKeyReport KEY_MEDIA_PLAY_PAUSE = {8, 0};
const MediaKeyReport KEY_MEDIA_MUTE = {16, 0};
const MediaKeyReport KEY_MEDIA_VOLUME_UP = {32, 0};
const MediaKeyReport KEY_MEDIA_VOLUME_DOWN = {64, 0};
const MediaKeyReport KEY_MEDIA_WWW_HOME = {128, 0};
const MediaKeyReport KEY_MEDIA_LOCAL_MACHINE_BROWSER = {0, 1}; // Opens "My Computer" on Windows
const MediaKeyReport KEY_MEDIA_CALCULATOR = {0, 2};
const MediaKeyReport KEY_MEDIA_WWW_BOOKMARKS = {0, 4};
const MediaKeyReport KEY_MEDIA_WWW_SEARCH = {0, 8};
const MediaKeyReport KEY_MEDIA_WWW_STOP = {0, 16};
const MediaKeyReport KEY_MEDIA_WWW_BACK = {0, 32};
const MediaKeyReport KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION = {0, 64}; // Media Selection
const MediaKeyReport KEY_MEDIA_EMAIL_READER = {0, 128};

#endif
