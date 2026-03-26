#pragma once

#ifndef RFHEADER_H
#define RFHEADER_H

#include <Arduino.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <RCSwitch.h>

#include <driver/rmt_rx.h>
#include <driver/rmt_tx.h>

#include "core/settingheader.h"
#include "core/utilsheader.h"
#include "core/sdcardmountheader.h"
#include "core/displayheader.h"

extern ESP32ATSetting espatsettings;
extern SDCardModules sdcard;
extern DisplayModules display;

#define KEELOQ_NLF 0x3A5C742E

#define RF_RECEIVER_MODE 0
#define RF_TRANSMITTER_MODE 1
#define RF_FREQUENCY_ANALYZER_MODE 2

#define KEELOQ_SIMPLE_LEARNING 1
#define KEELOQ_NORMAL_LEARNING 2

#define RSSI_THRESHOLD        -85   // dBm — ignore weaker signal (default)
#define RSSI_MIN_VALID        -120  // dBm — limit signal strength
#define FINE_STEP_MHZ         0.05  // fine scan step (50 kHz)
#define FINE_RANGE_MHZ        1.0   // ±1 MHz scan around scanned signal

#define RMT_RESOLUTION_HZ   1000000UL
#define RMT_1MS_TICKS       1000UL

struct KeeloqKey {
    String mf_name{};
    uint64_t key = 0;
    uint32_t type = 0;
};

class KeeloqKeystore {
public:
    KeeloqKeystore(FS *fs);

    const std::vector<KeeloqKey> &get_keys();

private:
    std::vector<KeeloqKey> keys{};
};

const float subghz_frequency_list[] = {
    /* 300 - 348 MHz Frequency Range */
    300.000f,
    302.757f,
    303.875f,
    303.900f,
    304.250f,
    307.000f,
    307.500f,
    307.800f,
    309.000f,
    310.000f,
    312.000f,
    312.100f,
    312.200f,
    313.000f,
    313.850f,
    314.000f,
    314.350f,
    314.980f,
    315.000f,
    318.000f,
    330.000f,
    345.000f,
    348.000f,
    350.000f,

    /* 387 - 464 MHz Frequency Range */
    387.000f,
    390.000f,
    418.000f,
    430.000f,
    430.500f,
    431.000f,
    431.500f,
    433.075f,
    433.220f,
    433.420f,
    433.657f,
    433.889f,
    433.920f,
    434.075f,
    434.177f,
    434.190f,
    434.390f,
    434.420f,
    434.620f,
    434.775f,
    438.900f,
    440.175f,
    464.000f,
    467.750f,

    /* 779 - 928 MHz Frequency Range */
    779.000f,
    868.350f,
    868.400f,
    868.800f,
    868.950f,
    906.400f,
    915.000f,
    925.000f,
    928.000f
};

const float freq_analyzer_list_frequency[] = {
  // Band 300-348 MHz
  300.0, 303.0, 303.875, 304.25,
  307.0, 307.5, 308.0,
  310.0, 312.0, 312.0, 313.0, 314.0,
  314.85, 315.0,
  318.0,
  // Band 387-464 MHz  
  390.0,
  418.0,
  430.0, 430.5, 431.0, 431.5,
  433.075, 433.92, 434.0, 434.42, 434.775,
  438.9,
  440.0,
  446.0,
  447.0, 
  // Band 779-928 MHz
  779.0,
  868.0, 868.35,
  915.0,
  925.0,
  928.0
};

const int keeloq_steps[] = {
    -50,
    -10,
    -1,
    1,
    10,
    50
};

struct RfCodes {
    uint32_t frequency = 0;
    uint32_t serial = 0;
    uint64_t key = 0;
    uint16_t cnt = 0;
    uint32_t fix = 0;
    uint32_t hop = 0;
    uint32_t encrypted = 0;
    uint8_t btn = 0;
    String mf_name = "Unknown";
    String protocol = "";
    String preset = "";
    String data = "";
    int te = 0;
    std::vector<int> indexed_durations;
    int Bit = 0;
    int BitRAW = 0;

    bool keeloq_check_decrypt(uint32_t decrypt);
    bool keeloq_check_decrypt_centurion(uint32_t decrypt);

    void keeloq_step(uint16_t step);
};

struct RawRecording {
    float frequency;
    std::vector<rmt_symbol_word_t *> codes;
    std::vector<uint16_t> codeLengths;
    std::vector<uint16_t> gaps;
};

struct FreqFound {
    float freq;
    int rssi;
};

struct RawRecordingStatus {
    float frequency = 0.f;
    int rssiCount = 0;  // Counter for the number of RSSI readings
    int latestRssi = 0; // Store the latest RSSI value
    bool recordingStarted = false;
    bool recordingFinished = false;
    unsigned long firstSignalTime = 0; // Store the time of the latest signal
    unsigned long lastSignalTime = 0;  // Store the time of the latest signal
    unsigned long lastRssiUpdate = 0;
};

enum class RfRawState : uint8_t {
    IDLE,
    FREQ_SCANNING,    
    WAITING_SIGNAL, 
    RECORDING,       
    RECORDING_DONE,  
    EMITTING,         
    SAVING,     
    ERROR,
};

struct RawScanRangeLimits { int start; int end; const char *label; };
static const RawScanRangeLimits RAW_SCAN_RANGES[] = {
    {  0, 23, "300-348 MHz" },
    { 24, 47, "387-464 MHz" },
    { 48, 56, "779-928 MHz" },
    {  0, 56, "All ranges"  },
};
#define RAW_SCAN_RANGE_COUNT 4
 
struct FreqRawScanState {
    int     idx          = 0;
    int     idxStart     = 0;  
    int     idxEnd       = 56; 
    uint8_t rangeSelect  = 3;  
    uint8_t attempt      = 0;
    int     rssiThreshold = -65;
    FreqFound best[5];

    void reset(uint8_t range = 3) {
        rangeSelect = range < RAW_SCAN_RANGE_COUNT ? range : 3;
        idxStart    = RAW_SCAN_RANGES[rangeSelect].start;
        idxEnd      = RAW_SCAN_RANGES[rangeSelect].end;
        idx         = idxStart;
        attempt     = 0;
        for (int i = 0; i < 5; i++) { best[i].freq = 433.92f; best[i].rssi = -75; }
    }

    const char *rangeLabel() const {
        return RAW_SCAN_RANGES[rangeSelect < RAW_SCAN_RANGE_COUNT ? rangeSelect : 3].label;
    }
};
 
 
struct RawRecordState {
    RawRecording        recording;
    RawRecordingStatus  status;
    rmt_channel_handle_t rx_ch        = NULL;
    QueueHandle_t        receiveQueue = NULL;
    bool                 fakeRssi     = false;
    bool                 rssiFeature  = false;
    bool                 returnToMenu = false;
};

struct RawEmitState {
    size_t   codeIdx    = 0;
    int      symbolIdx  = 0;
    uint32_t gapStart   = 0;
    bool     inGap      = false;
    bool     done       = false;
    bool     returnToMenu = false;
    gpio_num_t txPin;
};


void keeloq_identify(RfCodes &instance);
uint32_t keeloq_encrypt(const uint32_t data, const uint64_t key);
uint64_t keeloq_normal_learning(uint32_t data, const uint64_t key);

class RFModules {
    private:

        uint8_t frequencyIndex = 36; // Default index for 433.92 MHz
        float frequency = subghz_frequency_list[frequencyIndex]; // Default frequency in MHz
        bool startup = true;
        bool cc1101_ready = false;

        // Receiver
        bool keyDetected = false;
        String filenametosave = "";

        // Frequenzy Analyzer
        int8_t RSSI_threshold = RSSI_THRESHOLD;

        // Transmitter

        String txfilename = "";
        int keeloq_steps_index = 3;

        float  bestFreq      = 0.0;
        int8_t bestRSSI      = RSSI_MIN_VALID;

        float fineScan(float freqCenter, int8_t &bestRssiOut);

        void setFrequency(float freqMHz);
        void sendRAW(int *ptrtransmittimings);
        void sendRAWBit(const RfCodes& data);
        void sendType(uint64_t data, unsigned int bits, int pulse, int protocol, int repeat);

        bool saveSignal(float frequency, RfCodes codes, char *key);


        // ISR callback cho RMT (phải static)
        static bool _rmt_rx_done_cb(rmt_channel_t *ch,
                                    const rmt_rx_done_event_data_t *edata,
                                    void *user_data);
    public:
        std::vector<int> bitList;
        std::vector<int> bitRawList;
        std::vector<uint64_t> keyList;
        std::vector<String> rawDataList;
        bool redraw = false;
        bool keeloq_loop_emulate = false;
        int num_keeloq_steps = keeloq_steps[keeloq_steps_index];

        bool signalsaved = false;

        RCSwitch rcSwitch = RCSwitch();
        RfCodes keyData;
        void main();
        bool getCC1101();
        void configureMode(int mode);
        void stepFrequency(int step);

        float getFrequency();
        float getFrequencyAnalyzer();
        RfCodes getCurrentData();
        String getSavedFileName();
        bool getKeyDetect();

        void selectedSubFile(String filename);
        void transmittedCommand(RfCodes &codes);
        void keeloqLoopEmulate_pre(RfCodes &codes);
        void stepKeeLoqStep(int step);
        void keeloq_save(RfCodes data);

        void parseReceivedData();
        void save();
        void readSubFile(String filename, RfCodes &data);
        void sendCommand(const RfCodes& code);
        void resetKeyDetect();
        void shutdownCC1101();

        void frequencyAnalyzerLoop();
        void stepRSSIThreshold(int step);
        int8_t getFreqAnalyzerRssiThreshold();

        FreqRawScanState _rawScan;

        void        _raw_scan_begin(uint8_t range = 3);
        void        _raw_scan_set_range(uint8_t range);
        float       raw_scan_tick();                          // 0=scanning, >0=found, -1=error
        bool        _raw_record_begin(RawRecordState &rs);
        RfRawState  raw_record_tick(RawRecordState &rs, bool stopRequested);
        bool        _raw_emit_begin(RawEmitState &es, RawRecording &rec);
        RfRawState  raw_emit_tick(RawEmitState &es, RawRecording &rec, bool stopRequested);
        bool        raw_save(RawRecording &rec);
};
#endif