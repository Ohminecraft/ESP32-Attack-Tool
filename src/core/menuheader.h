#pragma once

#ifndef MENUHEADER_H
#define MENUHEADER_H

/*
    * menuheader.h Version 2.0 (with Claude, ChatGPT Help)
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * This file contains the declaration of the MenuSystem class which handles the main menu, BLE menu, and WiFi menu.
    * It includes button handling, menu navigation, and attack execution.
*/

#include <Arduino.h>
#include "core/assets.h"
#include "core/settingheader.h"
#include "core/utilsheader.h"
#include "core/logutilsheader.h"
#include "core/displayheader.h"
#include "core/sdcardmountheader.h"
#include "core/clockheader.h"
#include "core/communicationheader.h"
#include "modules/ble/bleheader.h"
#include "modules/wifi/wifiheader.h"
#include "modules/wifi/evilportalheader.h"
#include "modules/nrf24/nrf24header.h"
#include "modules/ir/irsend_header.h"
#include "modules/ir/irread_header.h"
#include "modules/badusb/ducky_usb_header.h"

#include "configs.h"

#ifdef WIFI_SCAN_RUNNING
#undef WIFI_SCAN_RUNNING
#endif

// Menu states
enum MenuState {
    MAIN_MENU,
    BLE_MENU,
    WIFI_MENU,
    NRF24_MENU,
    IR_MENU,
    SD_MENU,
    CLOCK_MENU,
    SD_UPDATE_MENU,
    SD_DELETE_MENU,
    BADUSB_KEY_LAYOUT_MENU,
    BADUSB_RUNNING,
    IR_TV_B_GONE_REGION,
    IR_READ_MENU,
    IR_READ_RUNNING,
    IR_CODE_SELECT,
    IR_SEND_RUNNING,
    NRF24_ANALYZER_RUNNING,
    NRF24_SCANNER_RUNNING,
    NRF24_JAMMER_RUNNING,
    NRF24_JAMMER_MENU,
    WIFI_UTILS_MENU,
    WIFI_UTILS_SET_MAC_MENU,
    WIFI_GENERAL_MENU,
    WIFI_SCAN_RUNNING,
    WIFI_SCAN_SNIFFER_RUNNING,
    WIFI_SELECT_MENU,
    WIFI_SELECT_PROBE_REQ_SSIDS_MENU,
    WIFI_SELECT_STA_AP_MENU,
    WIFI_SELECT_STA_MENU,
    WIFI_ATTACK_MENU,
    WIFI_DUAL_BAND_MENU,
    WIFI_DUAL_BAND_GENERAL_MENU,
    WIFI_DUAL_BAND_SELECT_MENU,
    WIFI_DUAL_BAND_ATTACK_MENU,
    BLE_SCAN_RUNNING,
    BLE_INFO_MENU_LIST,
    BLE_SPOOFER_MAIN_MENU,
    BLE_SPOOFER_APPLE_MENU,
    BLE_SPOOFER_APPLE_DEVICE_COLOR_MENU,
    BLE_SPOOFER_SAMSUNG_MENU,
    BLE_SPOOFER_SAMSUNG_BUDS_MENU,
    BLE_SPOOFER_SAMSUNG_WATCHS_MENU,
    BLE_SPOOFER_CONN_MODE_MENU,
    BLE_SPOOFER_DISC_MODE_MENU,
    BLE_SPOOFER_RUNNING,
    BLE_ANALYZER_RUNNING,
    BLE_EXPLOIT_ATTACK_MENU,
    BLE_MEDIA_MENU,
    BLE_KEYMOTE_MENU,
    BLE_TT_SCROLL_MENU,
    BLE_ATTACK_RUNNING,
    WIFI_ATTACK_RUNNING
};

// Main menu items
enum MainMenuItem {
    MAIN_BLE,
    MAIN_WIFI,
    MAIN_NRF24,
    MAIN_IR,
    MAIN_SD,
    MAIN_CLOCK,
    MAIN_DEEP_SLEEP,
    MAIN_REBOOT,
    MAIN_MENU_COUNT
};

// BLE menu items
enum BLEMenuItem {
    BLE_SCAN,
    BLE_ANALYZER,
    BLE_INFO,
    BLE_MEDIA_CMD,
    BLE_KEYMOTE,
    BLE_TT_SCROLL,
    BLE_BADUSB,
    BLE_SPOOFER,
    BLE_EXPLOIT_ATTACK,
    BLE_BACK,
    BLE_MENU_COUNT
};

enum BLESpooferConnMode {
    BLE_SPO_CONN_MODE_NON,
    BLE_SPO_CONN_MODE_DIR,
    BLE_SPO_CONN_MODE_UND,
    BLE_SPO_CONN_MODE_BACK,
    BLE_SPO_CONN_MODE_COUNT
};

enum BLESpooferDiscMode {
    BLE_SPO_DISC_MODE_NON,
    BLE_SPO_DISC_MODE_LTD,
    BLE_SPO_DISC_MODE_GEN,
    BLE_SPO_DISC_MODE_BACK,
    BLE_SPO_DISC_MODE_COUNT
};

// BLE Main Spoofer menu items
enum BLEMainSpooferItem {
    BLE_SPOOFER_APPLE,
    BLE_SPOOFER_SAMSUNG,
    BLE_SPOOFER_BACK,
    BLE_SPOOFER_COUNT
};

enum BLESamsungSpooferItem {
    BLE_SPO_SAMSUNG_BUDS,
    BLE_SPO_SAMSUNG_WATCH,
    BLE_SPO_SAMSUNG_BACK,
    BLE_SPO_SAMSUNG_COUNT
};

enum BLESpooferBrandType {
    BLE_SPO_BRAND_APPLE,
    BLE_SPO_BRAND_SAMSUNG,
    NONE
};

// BLE Exploit Attack menu items
enum BLEExploitAttackMenuItem {
    BLE_ATK_SOUR_APPLE,
    BLE_ATK_APPLE_JUICE,
    BLE_ATK_SWIFTPAIR_MS,
    BLE_ATK_SAMSUNG_SPAM,
    BLE_ATK_GOOGLE_SPAM,
    BLE_ATK_NAME_FLOOD_SPAM,
    BLE_ATK_SPAM_ALL,
    BLE_ATK_BACK,
    BLE_ATK_MENU_COUNT
};

enum BLEMediaCtrlItem {
    BLE_MEDIA_SCREENSHOT,
    BLE_MEDIA_PLAYPAUSE,
    BLE_MEDIA_STOP,
    BLE_MEDIA_NEXT_TRACK,
    BLE_MEDIA_PREV_TRACK,
    BLE_MEDIA_VOL_UP,
    BLE_MEDIA_VOL_DOWN,
    BLE_MEDIA_MUTE,
    BLE_MEDIA_BACK,
    BLE_MEDIA_MENU_COUNT
};

enum BLEKeymote {
    BLE_KEYMOTE_UP,
    BLE_KEYMOTE_DOWN,
    BLE_KEYMOTE_LEFT,
    BLE_KEYMOTE_RIGHT,
    BLE_KEYMOTE_BACK,
    BLE_KEYMOTE_ITEM_COUNT
};

enum TiktokScrollItem {
    BLE_TT_SCROLL_UP,
    BLE_TT_SCROLL_DOWN,
    BLE_TT_LIKE_VIDEO,
    BLE_TT_BACK,
    BLE_TT_ITEM_COUNT
};

// WiFi menu items
enum WiFiMenuItem {
    WIFI_GENERAL,
    WIFI_SELECT,
    WIFI_STA_SELECT,
    WIFI_PROBE_REQ_SSIDS_SELECT,
    WIFI_UTILS,
    WIFI_ATTACK,
    WIFI_DUAL_BAND,
    WIFI_BACK,
    WIFI_MENU_COUNT
};

enum WiFiUtils {
    WIFI_UTILS_SET_AP_MAC,
    WIFI_UTILS_SET_STA_MAC,
    WIFI_UTILS_GENERATE_AP_MAC,
    WIFI_UTILS_GENERATE_STA_MAC,
    WIFI_UTILS_SET_EVIL_PORTAL_HTML,
    WIFI_UTILS_BACK,
    WIFI_UTILS_MENU_COUNT
};

enum WiFiGeneralItem {
    WIFI_GENERAL_AP_SCAN,
    WIFI_GENERAL_AP_SCAN_OLD,
    WIFI_GENERAL_AP_STA_SCAN,
    WIFI_GENERAL_PROBE_REQ_SCAN,
    WIFI_GENERAL_DEAUTH_SCAN,
    WIFI_GENERAL_BEACON_SCAN,
    WIFI_GENERAL_EAPOL_SCAN,
    WIFI_GENERAL_EAPOL_DEAUTH_SCAN,
    WIFI_GENERAL_CH_ANALYZER,
    WIFI_GENERAL_BACK,
    WIFI_GENERAL_MENU_COUNT
};

// WiFi Attack menu items
enum WiFiAttackMenuItem {
    WIFI_ATK_DEAUTH,
    WIFI_ATK_STA_DEAUTH,
    WIFI_ATK_DEAUTH_FLOOD,
    WIFI_ATK_AUTH,
    WIFI_ATK_RIC_BEACON,
    WIFI_ATK_STA_BEACON,
    WIFI_ATK_RND_BEACON,
    WIFI_ATK_AP_BEACON,
    WIFI_ATK_EVIL_PORTAL,
    WIFI_ATK_EVIL_PORTAL_DEAUTH,
    WIFI_ATK_KARMA,
    WIFI_ATK_BAD_MSG,
    WIFI_ATK_BAD_MSG_ALL,
    WIFI_ATK_BACK,
    WIFI_ATK_MENU_COUNT
};

enum NRFMenuItem {
    NRF24_ANALYZER,
    NRF24_SCANNER,
    NRF24_JAMMER,
    NRF24_BACK,
    NRF24_MENU_COUNT
};

enum NRFJammerItem {
    NRF24_JAM_WIFI,
    NRF24_JAM_BLE,
    NRF24_JAM_BLUE,
    NRF24_JAM_ZIGBEE,
    NRF24_JAM_RC,
    NRF24_JAM_VIDEO_TRANS,
    NRF24_JAM_USB_WIRELESS,
    NRF24_JAM_DRONE,
    NRF24_JAM_FULL_CHAN,
    NRF24_JAM_BACK,
    NRF24_JAM_MENU_COUNT
};

enum IRMenuItem {
    IR_READ,
    IR_SEND,
    IR_TV_B_GONE,
    IR_BACK,
    IR_MENU_COUNT
};

enum IRReadMenuItem {
    QUICK_TV,
    CUSTOM_READ,
    IR_READ_BACK,
    IR_READ_MENU_COUNT
};

enum IRTVBGoneRegion {
    IR_TV_B_GONE_NA,
    IR_TV_B_GONE_EU,
    IR_TV_B_GONE_BACK,
    IR_TV_B_GONE_REGION_COUNT
};

enum SDMenuItem {
    SD_UPDATE,
    SD_DELETE,
    SD_BACK,
    SD_MENU_COUNT
};

enum BadUSBKeyLayout {
    BADUSB_LAYOUT_EN_US,
    BADUSB_LAYOUT_PT_BR,
    BADUSB_LAYOUT_PT_PT,
    BADUSB_LAYOUT_FR_FR,
    BADUSB_LAYOUT_ES_ES,
    BADUSB_LAYOUT_IT_IT,
    BADUSB_LAYOUT_EN_UK,
    BADUSB_LAYOUT_DE_DE,
    BADUSB_LAYOUT_SV_SE,
    BADUSB_LAYOUT_DA_DK,
    BADUSB_LAYOUT_HU_HU,
    BADUSB_LAYOUT_TR_TR,
    BADUSB_LAYOUT_SI_SI,
    BADUSB_LAYOUT_BACK,
    BADUSB_LAYOUT_COUNT
};

MenuState currentState = MAIN_MENU;
uint16_t currentSelection = 0;
uint16_t maxSelections = MAIN_MENU_COUNT;

// Attack State
unsigned long attackStartTime;
BLEScanState currentBLEAttackType = BLE_SCAN_OFF;
WiFiScanState currentWiFiAttackType = WIFI_SCAN_OFF;
NRFJammerMode currentNRFJammerMode;

// BLE Scan State
bool bleScanRunning = false;
bool bleScanOneShot = false;
bool bleScanInProgress = false;
bool bleScanDisplay = false;

// BLE Spoofer State
bool bleSpooferDone = false;
BLESpooferBrandType bleSpooferBrandType = NONE;
uint8_t apple_device_type = -1;

// WiFi Scan State
bool wifiScanRunning = false;
bool wifiScanOneShot = false;
bool wifiScanInProgress = false;
bool wifiScanDisplay = false;

bool wifiSnifferInProgress = false;
uint8_t wifiSnifferMode;
bool analyzerChangedChannel = false;

int ap_index = 0;
bool set_mac = false;

// WiFi Attack State
bool wifiAttackOneShot = false;
bool fixDeauthFloodDisplayLoop = false;

// Evil Portal
bool evilPortalOneShot = false;
bool selectforevilportal = false;

// NRF24
bool nrfAnalyzerSetupOneShot = false;
bool nrfJammerSetupOneShot = false;
bool nrfScannerSetupOneShot = false;

// IRSend/Recv

TvBeGoneRegion irTvBGoneRegion;
bool starttvbgone = false;

//bool irtxRedraw = false;
bool send_select_code = false;
bool selectforirtx = false;
String irSendFile;

bool irreadOneShot = false;

// BadUSB
bool selectforbadusb = false;
bool badble = false;
String badusbFile;

bool need_restart = false;

// System
bool autoSleep = false;
bool standby = false;
bool handleStateRunningCheck = false; // Used to check if the handle state is running

void displayWelcome();
void displayMainMenu();
void redrawTasks();
        
// Menu navigation
void navigateUp();
void navigateDown();
void selectCurrentItem();
void goBack();
    
// Attack functions
void startBLEAttack(BLEScanState attackType);
void startWiFiAttack(WiFiScanState attackType);
void startNRFJammer(NRFJammerMode jammerMode);
void stopCurrentAttack();
        
// WiFi scan functions
void startWiFiScan(WiFiGeneralItem mode);
void startSnifferScan(WiFiGeneralItem sniffer_mode);
//void stopWiFiScan();

// BLE scan functions
void startBLEScan();

// NRF24 functions
void nrfAnalyzer();
void nrfOutputScanChannel();
void nrfScanner();
        
// Helper functions
bool hasSelectedAPs();
bool hasSelectedSTAs();
bool hasSelectedProbeReqSSID();
        
// System functions
void performReboot();
void performDeepSleep();

void menuinit();
void menuloop();

#endif // MENUHEADER_H