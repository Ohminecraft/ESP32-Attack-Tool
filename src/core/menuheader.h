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
#include "core/displayheader.h"
#include "core/sdcardmountheader.h"
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
    SD_UPDATE_MENU,
    SD_DELETE_MENU,
    BADUSB_KEY_LAYOUT_MENU,
    BADUSB_RUNNING,
    IR_TV_B_GONE_REGION,
    IR_READ_RUNNING,
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
    BLE_SCAN_RUNNING,
    BLE_INFO_MENU_LIST,
    BLE_INFO_MENU_DETAIL,
    BLE_SPOOFER_MAIN_MENU,
    BLE_SPOOFER_APPLE_MENU,
    BLE_SPOOFER_SAMSUNG_MENU,
    BLE_SPOOFER_GOOGLE_MENU,
    BLE_SPOOFER_AD_TYPE_MENU,
    BLE_SPOOFER_RUNNING,
    BLE_ANALYZER_RUNNING,
    BLE_EXPLOIT_ATTACK_MENU,
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
    MAIN_DEEP_SLEEP,
    MAIN_REBOOT,
    MAIN_MENU_COUNT
};

// BLE menu items
enum BLEMenuItem {
    BLE_SCAN,
    BLE_ANALYZER,
    BLE_INFO,
    BLE_BADUSB,
    BLE_SPOOFER,
    BLE_EXPLOIT_ATTACK,
    BLE_BACK,
    BLE_MENU_COUNT
};

enum BLESpooferAdType {
    BLE_SPO_AD_TYPE_NON,
    BLE_SPO_AD_TYPE_IND, // in NimBLE this is define for DIR Ad Type
    BLE_SPO_AD_TYPE_SCAN, // in NimBLE this is define for UND Ad Type
    BLE_SPO_AD_TYPE_BACK,
    BLE_SPO_AD_TYPE_COUNT
};

// BLE Main Spoofer menu items
enum BLEMainSpooferItem {
    BLE_SPOOFER_APPLE,
    BLE_SPOOFER_SAMSUNG,
    BLE_SPOOFER_GOOGLE,
    BLE_SPOOFER_BACK,
    BLE_SPOOFER_COUNT
};

// BLE Apple Spoofer menu items
enum BLEAppleSpooferItem {
    BLE_SPO_APPLE_AIRPODS,
    BLE_SPO_APPLE_AIRPODS_PRO,
    BLE_SPO_APPLE_AIRPODS_MAX,
    BLE_SPO_APPLE_AIRPODS_GEN2,
    BLE_SPO_APPLE_AIRPODS_GEN3,
    BLE_SPO_APPLE_AIRPODS_PRO_GEN2,
    BLE_SPO_APPLE_POWERBEATS,
    BLE_SPO_APPLE_POWERBEATS_PRO,
    BLE_SPO_APPLE_BEATS_SOLO_PRO,
    BLE_SPO_APPLE_BEATS_STUDIO_BUDS,
    BLE_SPO_APPLE_BEATS_FLEX,
    BLE_SPO_APPLE_BEATS_X,
    BLE_SPO_APPLE_BEATS_SOLO3,
    BLE_SPO_APPLE_BEATS_STUDIO3,
    BLE_SPO_APPLE_BEATS_STUDIO_PRO,
    BLE_SPO_APPLE_BEATS_FIT_PRO,
    BLE_SPO_APPLE_BEATS_STUDIO_BUDS_PLUS,
    BLE_SPO_APPLE_BACK,
    BLE_SPO_APPLE_COUNT
};

enum BLESamsungSpooferItem {
    BLE_SPO_SAMSUNG_FALLBACK_WATCH,
    BLE_SPO_SAMSUNG_WHITE_WATCH4_CLASSIC_44M,
    BLE_SPO_SAMSUNG_BLACK_WATCH4_CLASSIC_40M,
    BLE_SPO_SAMSUNG_WHITE_WATCH4_CLASSIC_40M,
    BLE_SPO_SAMSUNG_BLACK_WATCH4_44M,
    BLE_SPO_SAMSUNG_SILVER_WATCH4_44M,
    BLE_SPO_SAMSUNG_GREEN_WATCH4_44M,
    BLE_SPO_SAMSUNG_BLACK_WATCH4_40M,
    BLE_SPO_SAMSUNG_WHITE_WATCH4_40M,
    BLE_SPO_SAMSUNG_GOLD_WATCH4_40M,
    BLE_SPO_SAMSUNG_FRENCH_WATCH4,
    BLE_SPO_SAMSUNG_FRENCH_WATCH4_CLASSIC,
    BLE_SPO_SAMSUNG_FOX_WATCH5_44MM,
    BLE_SPO_SAMSUNG_BLACK_WATCH5_44MM,
    BLE_SPO_SAMSUNG_SAPPHIRE_WATCH5_44MM,
    BLE_SPO_SAMSUNG_PURPLEISH_WATCH5_40MM,
    BLE_SPO_SAMSUNG_GOLD_WATCH5_40MM,
    BLE_SPO_SAMSUNG_BLACK_WATCH5_PRO_45MM,
    BLE_SPO_SAMSUNG_GRAY_WATCH5_PRO_45MM,
    BLE_SPO_SAMSUNG_WHITE_WATCH5_44MM,
    BLE_SPO_SAMSUNG_WHITE_BLACK_WATCH5,
    BLE_SPO_SAMSUNG_BLACK_WATCH6_PINK_40MM,
    BLE_SPO_SAMSUNG_GOLD_WATCH6_GOLD_40MM,
    BLE_SPO_SAMSUNG_SILVER_WATCH6_CYAN_44MM,
    BLE_SPO_SAMSUNG_BLACK_WATCH6_CLASSIC_43M,
    BLE_SPO_SAMSUNG_GREEN_WATCH6_CLASSIC_43M,
    BLE_SPO_SAMSUNG_BACK,
    BLE_SPO_SAMSUNG_COUNT
};


enum BLEGoogleSpooferItem {
    BLE_SPO_GOOGLE_BISTO_DEV_BOARD,
    BLE_SPO_GOOGLE_ARDUINO_101,
    BLE_SPO_GOOGLE_ARDUINO_101_2,
    BLE_SPO_GOOGLE_ANTISPOOF_TEST,
    BLE_SPO_GOOGLE_GOOGLE_GPHONES,
    BLE_SPO_GOOGLE_TEST_00000D,
    BLE_SPO_GOOGLE_ANDROID_AUTO,
    BLE_SPO_GOOGLE_TEST_ANDROID_TV,
    BLE_SPO_GOOGLE_TEST_ANDROID_TV_2,
    BLE_SPO_GOOGLE_FAST_PAIR_HEADPHONES,
    BLE_SPO_GOOGLE_LG_HBS1110,
    BLE_SPO_GOOGLE_SMART_CONTROLLER_1,
    BLE_SPO_GOOGLE_BLE_PHONE,
    BLE_SPO_GOOGLE_GOODYEAR,
    BLE_SPO_GOOGLE_SMART_SETUP,
    BLE_SPO_GOOGLE_GOODYEAR_2,
    BLE_SPO_GOOGLE_T10,
    BLE_SPO_GOOGLE_ATS2833_EVB,
    BLE_SPO_GOOGLE_BOSE_NC700,
    BLE_SPO_GOOGLE_BOSE_QC35_II,
    BLE_SPO_GOOGLE_BOSE_QC35_II_2,
    BLE_SPO_GOOGLE_JBL_FLIP_6,
    BLE_SPO_GOOGLE_JBL_BUDS_PRO,
    BLE_SPO_GOOGLE_JBL_LIVE_300TWS,
    BLE_SPO_GOOGLE_JBL_EVEREST_110GA,
    BLE_SPO_GOOGLE_PIXEL_BUDS,
    BLE_SPO_GOOGLE_PIXEL_BUDS_2,
    BLE_SPO_GOOGLE_SONY_XM5,
    BLE_SPO_GOOGLE_DENON_AH_C830NCW,
    BLE_SPO_GOOGLE_JBL_LIVE_FLEX,
    BLE_SPO_GOOGLE_JBL_REFLECT_MINI_NC,
    BLE_SPO_GOOGLE_BOSE_QC35_II_DUP1,
    BLE_SPO_GOOGLE_BOSE_QC35_II_DUP2,
    BLE_SPO_GOOGLE_JBL_EVEREST_110GA_2,
    BLE_SPO_GOOGLE_JBL_LIVE400BT,
    BLE_SPO_GOOGLE_JBL_EVEREST_310GA,
    BLE_SPO_GOOGLE_LG_HBS_1500,
    BLE_SPO_GOOGLE_JBL_VIBE_BEAM,
    BLE_SPO_GOOGLE_JBL_WAVE_BEAM,
    BLE_SPO_GOOGLE_BEOPLAY_H4,
    BLE_SPO_GOOGLE_JBL_TUNE_720BT,
    BLE_SPO_GOOGLE_WONDERBOOM_3,
    BLE_SPO_GOOGLE_BEOPLAY_E6,
    BLE_SPO_GOOGLE_JBL_LIVE220BT,
    BLE_SPO_GOOGLE_SONY_WI_1000X,
    BLE_SPO_GOOGLE_JBL_EVEREST_310GA_2,
    BLE_SPO_GOOGLE_LG_HBS_1700,
    BLE_SPO_GOOGLE_SRS_XB43,
    BLE_SPO_GOOGLE_WI_1000XM2,
    BLE_SPO_GOOGLE_SONY_WF_SP700N,
    BLE_SPO_GOOGLE_DENON_AH_C830NCW_2,
    BLE_SPO_GOOGLE_JBL_TUNE125TWS,
    BLE_SPO_GOOGLE_JBL_LIVE770NC,
    BLE_SPO_GOOGLE_JBL_EVEREST_110GA_GUNMETAL,
    BLE_SPO_GOOGLE_LG_HBS_835,
    BLE_SPO_GOOGLE_LG_HBS_2000,
    BLE_SPO_GOOGLE_FLIPPER_ZERO,
    BLE_SPO_GOOGLE_FREE_ROBUX,
    BLE_SPO_GOOGLE_FREE_VBUCKS,
    BLE_SPO_GOOGLE_RICKROLL,
    BLE_SPO_GOOGLE_ANIMATED_RICKROLL,
    BLE_SPO_GOOGLE_BLM,
    BLE_SPO_GOOGLE_TALKING_SASQUACH,
    BLE_SPO_GOOGLE_OBAMA,
    BLE_SPO_GOOGLE_RYANAIR,
    BLE_SPO_GOOGLE_FBI,
    BLE_SPO_GOOGLE_TESLA,
    BLE_SPO_GOOGLE_TON_UPGRADE_NETFLIX,
    BLE_SPO_GOOGLE_BACK,
    BLE_SPO_GOOGLE_COUNT
};

enum BLESpooferBrandType {
    BLE_SPO_BRAND_APPLE,
    BLE_SPO_BRAND_SAMSUNG,
    BLE_SPO_BRAND_GOOGLE,
    NONE
};

// BLE Exploit Attack menu items
enum BLEExploitAttackMenuItem {
    BLE_ATK_SOUR_APPLE,
    BLE_ATK_APPLE_JUICE,
    BLE_ATK_SWIFTPAIR_MS,
    BLE_ATK_SAMSUNG_SPAM,
    BLE_ATK_GOOGLE_SPAM,
    BLE_ATK_BACK,
    BLE_ATK_MENU_COUNT
};

// WiFi menu items
enum WiFiMenuItem {
    WIFI_GENERAL,
    WIFI_SELECT,
    WIFI_STA_SELECT,
    WIFI_PROBE_REQ_SSIDS_SELECT,
    WIFI_UTILS,
    WIFI_ATTACK,
    WIFI_BACK,
    WIFI_MENU_COUNT
};

enum WiFiUtils {
    WIFI_UTILS_SET_AP_MAC,
    WIFI_UTILS_SET_STA_MAC,
    WIFI_UTILS_GENERATE_AP_MAC,
    WIFI_UTILS_GENERATE_STA_MAC,
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

// NRF24
bool nrfAnalyzerSetupOneShot = false;
bool nrfJammerSetupOneShot = false;
bool nrfScannerSetupOneShot = false;

// IRSend/Recv

TvBeGoneRegion irTvBGoneRegion;
bool starttvbgone = false;

bool selectforirtx = false;
String irSendFile;

bool irreadOneShot = false;

// BadUSB
bool selectforbadusb = false;
bool badble = false;
String badusbFile;

// System
bool autoSleep = false;
bool standby = false;
bool handleStateRunningCheck = false; // Used to check if the handle state is running

// Menu display functions
void displayWelcome();
void displayMainMenu();
void displayBLEMenu();
void displayBLEScanMenu();
void displayBLEInfoListMenu();
void displayBLEInfoDetail();
void displayMainSpooferMenu();
void displayAppleSpooferMenu();
void displaySamsungSpooferMenu();
void displayGoogleSpooferMenu();
void displayAdTypeSpooferMenu();
void displaySpooferRunning();
void displayExploitAttackBLEMenu();
void displayWiFiMenu();
void displayWiFiScanMenu(WiFiGeneralItem mode);
//void displayWiFiReScanMenu(uint32_t elapsedTime);
void displayWiFiSelectMenu();
void displayWiFiAttackMenu();
void displayAttackStatus();
void displayDeauthFloodInfo();
void displayNRF24Menu();
void displayNRF24JammerMenu();
void displayNRFJammerStatus();
void displayEvilPortalInfo();
void displayRebootConfirm();
        
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