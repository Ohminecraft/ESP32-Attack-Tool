#pragma once

#ifndef WIFIFUNC_H
#define WIFIFUNC_H

#include <Arduino.h>
#include "utils.h"
#include "wifi_drv.h"
#include "wifi_util.h"
#include "wifi_structures.h"
#include "wifi_conf.h"
#include <LinkedList.h>

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

struct BssidToDeauth {
    uint8_t bssid[6];
    uint8_t channel;
};

struct Station {
    uint8_t mac[6];
};

struct BssidToDeauthWithStaion {
    uint8_t bssid[6];
    uint8_t channel;
    LinkedList<Station>* stations;
};

struct StringToProbeReq {
    String ssid;
    uint8_t channel;
};

class WiFiCallback {
    public:
        void start_rtl_ap_scan_callback(bool sta_scan_enable);

        static void rtl_ap_sniffer_callback(uint8_t *packet, uint length, void* param);
        static void rtl_ap_sta_sniffer_callback(uint8_t *packet, uint length, void* param);
};

#endif