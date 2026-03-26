#include "rfheader.h"

/*
    * rfrawrecord.cpp
    * RF Raw Record — scan tần số + ghi tín hiệu (non-blocking)
    * Mỗi hàm tick() chỉ xử lý 1 bước, gọi mỗi frame từ handleTasks().
    * Không có while(), không check nút — handleInput() lo toàn bộ input.
    *
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/


static rmt_receive_config_t s_rmtRxConfig = {
    .signal_range_min_ns =    3000,   // bỏ qua xung < 3µs (nhiễu)
    .signal_range_max_ns = 12000000,  // tín hiệu tối đa 12ms
};
static rmt_symbol_word_t s_rmtItems[64];

bool RFModules::_rmt_rx_done_cb(rmt_channel_t *ch,
                                 const rmt_rx_done_event_data_t *edata,
                                 void *user_data) {
    BaseType_t woken = pdFALSE;
    xQueueSendFromISR((QueueHandle_t)user_data, edata, &woken);
    return woken == pdTRUE;
}


#define RAW_SCAN_MAX_TRIES   5
#define RAW_SCAN_RSSI_THR   -65   // dBm

void RFModules::_raw_scan_begin(uint8_t range) {
    setFrequency(433.92f);
    main();
    if (!cc1101_ready) {
        Serial.println("[ERROR] raw_scan_begin: CC1101 not ready");
        return;
    }
    configureMode(RF_RECEIVER_MODE);
    _rawScan.reset(range);
    Serial.println("[INFO] RF Raw Scan Begin — range: " + String(_rawScan.rangeLabel()));
}

void RFModules::_raw_scan_set_range(uint8_t range) {
    uint8_t newRange = range % RAW_SCAN_RANGE_COUNT;
    _rawScan.reset(newRange);
    Serial.println("[INFO] RF Raw Scan Range → " + String(_rawScan.rangeLabel()));
}

float RFModules::raw_scan_tick() {
    static unsigned long _lastSetMhz = 0;
    if (millis() - _lastSetMhz < 5) return 0.0f;
    if (_rawScan.idx < _rawScan.idxStart || _rawScan.idx > _rawScan.idxEnd)
        _rawScan.idx = _rawScan.idxStart;

    float checkFreq = subghz_frequency_list[_rawScan.idx];

    ELECHOUSE_cc1101.setMHZ(checkFreq);
    _lastSetMhz = millis();
    int rssi = ELECHOUSE_cc1101.getRssi();

    if (rssi > RAW_SCAN_RSSI_THR) {
        _rawScan.best[_rawScan.attempt].freq = checkFreq;
        _rawScan.best[_rawScan.attempt].rssi = rssi;
        _rawScan.attempt++;

        if (_rawScan.attempt >= RAW_SCAN_MAX_TRIES) {
            int best = 0;
            for (int i = 1; i < RAW_SCAN_MAX_TRIES; i++) {
                if (_rawScan.best[i].rssi > _rawScan.best[best].rssi) best = i;
            }
            float found = _rawScan.best[best].freq;
            Serial.println("[INFO] RF Raw Scan Found: " + String(found) + " MHz");
            setFrequency(found);
            main();
            configureMode(RF_RECEIVER_MODE);
            return found;
        }
    }

    // Tăng index, wrap trong range
    _rawScan.idx++;
    if (_rawScan.idx > _rawScan.idxEnd) {
        _rawScan.idx     = _rawScan.idxStart;
        _rawScan.attempt = 0; // reset attempt khi hết 1 vòng
    }

    return 0.0f;
}

bool RFModules::_raw_record_begin(RawRecordState &rs) {
    rs.status       = RawRecordingStatus{};
    rs.fakeRssi     = false;
    rs.rssiFeature  = cc1101_ready;

    if (rs.rx_ch != NULL) {
        rmt_disable(rs.rx_ch);
        rmt_del_channel(rs.rx_ch);
        rs.rx_ch = NULL;
    }
    if (rs.receiveQueue != NULL) {
        vQueueDelete(rs.receiveQueue);
        rs.receiveQueue = NULL;
    }

    rs.receiveQueue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
    if (!rs.receiveQueue) {
        Serial.println("[ERROR] raw_record_begin: xQueueCreate failed");
        return false;
    }

    rmt_rx_channel_config_t rxChanCfg = {};
    rxChanCfg.gpio_num          = (gpio_num_t)espatsettings.cc1101Gdo0Pin;
    rxChanCfg.clk_src           = RMT_CLK_SRC_DEFAULT;
    rxChanCfg.resolution_hz     = RMT_RESOLUTION_HZ;
    rxChanCfg.mem_block_symbols = 64;
    rxChanCfg.flags.invert_in   = 0;
    rxChanCfg.flags.with_dma    = 0;

    esp_err_t err = rmt_new_rx_channel(&rxChanCfg, &rs.rx_ch);
    if (err != ESP_OK || rs.rx_ch == NULL) {
        Serial.printf("[ERROR] raw_record_begin: rmt_new_rx_channel err=0x%x\n", err);
        vQueueDelete(rs.receiveQueue);
        rs.receiveQueue = NULL;
        return false;
    }

    rmt_rx_event_callbacks_t cbs = {};
    cbs.on_recv_done = RFModules::_rmt_rx_done_cb;
    err = rmt_rx_register_event_callbacks(rs.rx_ch, &cbs, rs.receiveQueue);
    if (err != ESP_OK) {
        Serial.printf("[ERROR] raw_record_begin: register cb err=0x%x\n", err);
        rmt_del_channel(rs.rx_ch);
        rs.rx_ch = NULL;
        vQueueDelete(rs.receiveQueue);
        rs.receiveQueue = NULL;
        return false;
    }

    err = rmt_enable(rs.rx_ch);
    if (err != ESP_OK) {
        Serial.printf("[ERROR] raw_record_begin: rmt_enable err=0x%x\n", err);
        rmt_del_channel(rs.rx_ch);
        rs.rx_ch = NULL;
        vQueueDelete(rs.receiveQueue);
        rs.receiveQueue = NULL;
        return false;
    }

    err = rmt_receive(rs.rx_ch, s_rmtItems, sizeof(s_rmtItems), &s_rmtRxConfig);
    if (err != ESP_OK) {
        Serial.printf("[ERROR] raw_record_begin: rmt_receive err=0x%x\n", err);
        rmt_disable(rs.rx_ch);
        rmt_del_channel(rs.rx_ch);
        rs.rx_ch = NULL;
        vQueueDelete(rs.receiveQueue);
        rs.receiveQueue = NULL;
        return false;
    }

    Serial.println("[INFO] RF Raw Record Begin OK");
    return true;
}

RfRawState RFModules::raw_record_tick(RawRecordState &rs, bool stopRequested) {
    if (stopRequested && rs.status.recordingStarted) {
        rs.status.recordingFinished = true;
    }

    if (!rs.status.recordingFinished) {
        rmt_rx_done_event_data_t rxData;
        if (xQueueReceive(rs.receiveQueue, &rxData, 0) == pdPASS) {
            size_t rxSize = rxData.num_symbols;

            if (rxSize >= 5) {
                rs.fakeRssi = true;

                rmt_symbol_word_t *code =
                    (rmt_symbol_word_t *)malloc(rxSize * sizeof(rmt_symbol_word_t));
                if (!code) {
                    Serial.println("[ERROR] raw_record_tick: malloc failed");
                } else {
                    unsigned long      now          = millis();
                    unsigned long long signalDurUs  = 0;
                    for (size_t i = 0; i < rxSize; i++) {
                        code[i]      = rxData.received_symbols[i];
                        signalDurUs += code[i].duration0 + code[i].duration1;
                    }

                    rs.recording.codes.push_back(code);
                    rs.recording.codeLengths.push_back((uint16_t)rxSize);

                    if (rs.status.lastSignalTime != 0) {
                        unsigned long durMs = signalDurUs / RMT_1MS_TICKS;
                        uint16_t gap = (uint16_t)(now - rs.status.lastSignalTime - durMs - 5);
                        rs.recording.gaps.push_back(gap);
                    } else {
                        rs.status.firstSignalTime  = now;
                        rs.status.recordingStarted = true;
                    }
                    rs.status.lastSignalTime = now;
                }
            }

            rmt_receive(rs.rx_ch, s_rmtItems, sizeof(s_rmtItems), &s_rmtRxConfig);
        }

        if (rs.status.recordingStarted) {
            unsigned long now = millis();
            if (rs.status.lastRssiUpdate == 0 || now - rs.status.lastRssiUpdate >= 100) {
                if (rs.fakeRssi) {
                    rs.status.latestRssi = -45;
                } else {
                    rs.status.latestRssi = -90;
                }
                rs.fakeRssi = false;

                if (rs.rssiFeature && cc1101_ready) {
                    rs.status.latestRssi = ELECHOUSE_cc1101.getRssi();
                }

                rs.status.rssiCount++;
                rs.status.lastRssiUpdate = now;
            }

            if (millis() - rs.status.firstSignalTime >= 20000) {
                rs.status.recordingFinished = true;
            }
        }
    }

    if (rs.status.recordingFinished) {
        rmt_disable(rs.rx_ch);
        rmt_del_channel(rs.rx_ch);
        rs.rx_ch = NULL;
        vQueueDelete(rs.receiveQueue);
        rs.receiveQueue = NULL;
        shutdownCC1101();
        Serial.println("[INFO] RF Raw Record Done. Codes: " + String(rs.recording.codes.size()));
        return RfRawState::RECORDING_DONE;
    }

    return RfRawState::RECORDING;
}

bool RFModules::raw_save(RawRecording &rec) {
    if (rec.codes.empty()) {
        Serial.println("[WARN] raw_save: nothing to save");
        return false;
    }
    // Tìm tên file không trùng
    String filename = display.keyboard();
    if (filename == "\x01") {
        return false;
    }
    int idx = 0;
    do {
        filename += "_%d", idx++;
    } while (sdcard.isExists(filename));

    File file = sdcard.getFile(filename, FILE_WRITE);
    if (!file) {
        Serial.println("[ERROR] raw_save: cannot create file");
        return false;
    }

    file.print("Filetype: ESP32 Attack Tool - SubGhz RAW File\n");
    file.print("Version: 1\n");

    char lineBuf[32];
    snprintf(lineBuf, sizeof(lineBuf), "Frequency: %d\n", (int)(rec.frequency * 1000000));
    file.print(lineBuf);

    file.print("Preset: FuriHalSubGhzPresetOok270Async\n");
    file.print("Protocol: RAW\n");
    file.print("RAW_Data:");

    uint16_t valCount = 0;
    for (size_t i = 0; i < rec.codes.size(); i++) {
        for (size_t j = 0; j < rec.codeLengths[i]; j++) {
            auto &sym = rec.codes[i][j];

            // duration0
            if (sym.duration0 > 0) {
                if (sym.level0 != 1) file.print(" -");
                else                  file.print(" ");
                snprintf(lineBuf, sizeof(lineBuf), "%lu", (unsigned long)sym.duration0);
                file.print(lineBuf);
                valCount++;
                if (valCount % 512 == 0) file.print("\nRAW_Data:");
            }
            // duration1
            if (sym.duration1 > 0) {
                if (sym.level1 != 1) file.print(" -");
                else                  file.print(" ");
                snprintf(lineBuf, sizeof(lineBuf), "%lu", (unsigned long)sym.duration1);
                file.print(lineBuf);
                valCount++;
                if (valCount % 512 == 0) file.print("\nRAW_Data:");
            }
        }
        // Gap giữa các code block (âm = khoảng lặng)
        if (i < rec.codes.size() - 1) {
            snprintf(lineBuf, sizeof(lineBuf), " -%d", (int)(rec.gaps[i] * 1000));
            file.print(lineBuf);
            valCount++;
            if (valCount % 512 == 0) file.print("\nRAW_Data:");
        }
        file.flush();
    }

    file.print("\n");
    file.close();

    Serial.println("[INFO] RF Raw saved to: " + String(filename));
    return true;
}
