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

#define DUAL_BAND_CHANNELS 38

static uint8_t dual_band_channels[DUAL_BAND_CHANNELS] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 149, 153, 157, 161, 165};

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

void rtl_ap_sniffer_callback(uint8_t *packet, uint length, void* param);
void rtl_ap_sta_sniffer_callback(uint8_t *packet, uint length, void* param);
void channelHop();

//extern uint8_t channel;
extern bool band5ghz_scan;

#endif