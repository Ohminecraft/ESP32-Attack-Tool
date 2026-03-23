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

int8_t RFModules::readRSSI() {
  uint8_t raw = ELECHOUSE_cc1101.SpiReadStatus(CC1101_RSSI);
  int16_t rssi_dbm;
  if (raw >= 128) {
    rssi_dbm = ((int16_t)raw - 256) / 2 - 74;
  } else {
    rssi_dbm = (int16_t)raw / 2 - 74;
  }
  return (int8_t)constrain(rssi_dbm, -120, 0);
}

int8_t RFModules::measureRSSI(float freqMHz) {
  ELECHOUSE_cc1101.setMHZ(freqMHz);
  ELECHOUSE_cc1101.SetRx();
  delayMicroseconds(COARSE_SETTLE_US);

  int16_t sum = 0;
  for (int i = 0; i < RSSI_SAMPLES; i++) {
    sum += readRSSI();
    delayMicroseconds(RSSI_SAMPLE_DELAY_US);
  }
  return (int8_t)(sum / RSSI_SAMPLES);
}

float RFModules::fineScan(float freqCenter, int8_t &bestRssiOut) {
  float fineStart = freqCenter - FINE_RANGE_MHZ;
  float fineEnd   = freqCenter + FINE_RANGE_MHZ;

  // Clamp vào dải hợp lệ của CC1101
  fineStart = constrain(fineStart, 300.0, 928.0);
  fineEnd   = constrain(fineEnd, 300.0, 928.0);

  float   peakFreq = freqCenter;
  int8_t  peakRSSI = RSSI_MIN_VALID;

  for (float f = fineStart; f <= fineEnd; f += FINE_STEP_MHZ) {
    int8_t rssi = measureRSSI(f);
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
    int8_t coarseRSSI = measureRSSI(coarseFreq);

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