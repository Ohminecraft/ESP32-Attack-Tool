#include "rfheader.h"

/*
    * rfread.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

void RFModules::parseReceivedData() {
    keyData.fix = 0;
    keyData.hop = 0;
    keyData.btn = 0;
    keyData.cnt = 0;
    keyData.mf_name = "Unknown";
    keyData.encrypted = 0;
    uint32_t receivedValue = rcSwitch.getReceivedValue();
    if (receivedValue) {
        keyData.frequency = long(frequency * 1000000);
        keyData.key = receivedValue;
        keyData.preset = String(rcSwitch.getReceivedProtocol());
        keyData.protocol = "RcSwitch";
        keyData.te = rcSwitch.getReceivedDelay();
        keyData.Bit = rcSwitch.getReceivedBitlength();
        keyData.data = "";

        if (rcSwitch.getReceivedProtocol() == 23) {
            uint64_t yek = reverse_bits(receivedValue, 64);

            keyData.fix = yek >> 32;
            keyData.btn = keyData.fix >> 28;
            keyData.encrypted = yek & 0xFFFFFFFF;
            keyData.serial = (yek >> 32) & 0xFFFFFFF;

            keeloq_identify(keyData);
        }
        keyDetected = true;
    }
    rcSwitch.resetAvailable();
}

float RFModules::fineScan(float freqCenter, int8_t &bestRssiOut) {
  float fineStart = freqCenter - FINE_RANGE_MHZ;
  float fineEnd   = freqCenter + FINE_RANGE_MHZ;

  fineStart = constrain(fineStart, 300.0, 928.0);
  fineEnd   = constrain(fineEnd, 300.0, 928.0);

  float   peakFreq = freqCenter;
  int8_t  peakRSSI = RSSI_MIN_VALID;

  for (float f = fineStart; f <= fineEnd; f += FINE_STEP_MHZ) {
    ELECHOUSE_cc1101.setMHZ(f);
    int8_t rssi = ELECHOUSE_cc1101.getRssi();
    if (rssi > peakRSSI) {
      peakRSSI = rssi;
      peakFreq = f;
    }
  }

  bestRssiOut = peakRSSI;
  return peakFreq;
}

void RFModules::frequencyAnalyzerLoop() {
    static int coarseIdx = 0;

    float coarseFreq = subghz_frequency_list[coarseIdx];
    ELECHOUSE_cc1101.setMHZ(coarseFreq);
    int8_t coarseRSSI = ELECHOUSE_cc1101.getRssi();

    if (coarseRSSI > RSSI_threshold) {

        int8_t fineRSSI = RSSI_MIN_VALID;
        float  fineFreq = fineScan(coarseFreq, fineRSSI);

        if (fineRSSI > RSSI_threshold) {

            if (fineRSSI > bestRSSI) {
                bestRSSI = fineRSSI;
                bestFreq = fineFreq;
                redraw = true;
                Serial.println("[INFO] Found New Best Frequency: " + (String)bestFreq);
            }
        }
    }

    coarseIdx = (coarseIdx + 1) % GET_SIZE(subghz_frequency_list);

    if (coarseIdx == 0) {
        static uint8_t sweepCount = 0;
        sweepCount++;
        if (sweepCount >= 5) {
        sweepCount = 0;
        bestFreq   = 0.0;
        bestRSSI   = RSSI_MIN_VALID;
        }
    }
}