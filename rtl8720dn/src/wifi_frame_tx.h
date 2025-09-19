#pragma once

#ifndef WIFI_FRAME_TX_H
#define WIFI_FRAME_TX_H

#include <Arduino.h>
#include "wifi_conf.h"

// RTL8720DN

static uint8_t deauth_frame_packet[26] = {
    0xC0, 0x00,                         // type, subtype c0: deauth (a0: disassociate)
    0x3A, 0x01,                         // duration (SDK takes care of that)
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // reciever (target)
    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // source (ap)
    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // BSSID (ap)
    0x00, 0x00,                         // fragment & squence number
    0x01, 0x00                          // reason code (1 = unspecified reason)
};

static uint8_t disassoc_frame_packet[26] = {
    0xA0, 0x00,                         // Frame Control (only first byte different)
    0x3A, 0x01,                         // Duration
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination addr
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source addr
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID
    0x00, 0x00,                         // Sequence number
    0x07, 0x00                          // Reason code
};

static uint8_t probe_frame_packet[68] = {
    0x40, 0x00,                                                            // Type: Probe Request
    0x00, 0x00,                                                            // Duration: 0 microseconds
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,                                    // Destination: Broadcast
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,                                    // Source: random MAC
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,                                    // BSS Id: Broadcast
    0x00, 0x00,                                                            // Sequence number (will be replaced by the SDK)
    0x00, 0x20,                                                            // Tag: Set SSID length, Tag length: 32
    0x20, 0x20,         0x20,               0x20,                          // SSID
    0x20,               0x20,               0x20,               0x20,
    0x20,               0x20,               0x20,               0x20,
    0x20,               0x20,               0x20,               0x20,
    0x20,               0x20,               0x20,               0x20,
    0x20,               0x20,               0x20,               0x20,
    0x20,               0x20,               0x20,               0x20,
    0x20,               0x20,               0x20,               0x20,
    0x01, 0x08,                                                             // Tag Number: Supported Rates (1), Tag length: 8
    0x82,                                                                   // 1(B)
    0x84,                                                                   // 2(B)
    0x8b,                                                                   // 5.5(B)
    0x96,                                                                   // 11(B)
    0x24,                                                                   // 18
    0x30,                                                                   // 24
    0x48,                                                                   // 36
    0x6c                                                                    // 54
};


extern uint8_t* rltk_wlan_info;
extern "C" void* alloc_mgtxmitframe(void* ptr);
extern "C" void update_mgntframe_attrib(void* ptr, void* frame_control);
extern "C" int dump_mgntframe(void* ptr, void* frame_control);

void rtl_tx_raw_frame(void* frame, size_t length);
void sendDualBandDeauthFrame(uint8_t bssid[6], uint8_t channel, uint8_t sta_mac[6]);
void sendDualBandDeauthFrame(uint8_t bssid[6], uint8_t channel);
void sendDualBandProbeReqFrame(String ssid, uint8_t channel);

#endif