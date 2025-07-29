#include "nrf24header.h"

/*
    * nrf24.cpp (Based nRFBox by @ciferTech)
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

RF24 NRFRadio(NRF24_CE_PIN, NRF24_CSN_PIN, 16000000);

byte NRF24Modules::getRegister(byte r) {
    byte c;
    
    digitalWrite(NRF24_CSN_PIN, LOW);
    SPI.transfer(r & 0x1F);
    c = SPI.transfer(0);
    digitalWrite(NRF24_CSN_PIN, HIGH);

    return c;
}

void NRF24Modules::setRegister(byte r, byte v) {
    digitalWrite(NRF24_CSN_PIN, LOW);
    SPI.transfer((r & 0x1F) | 0x20);
    SPI.transfer(v);
    digitalWrite(NRF24_CSN_PIN, HIGH);
}

void NRF24Modules::initNRF() {
    this->setRegister(NRF24_CONFIG, this->getRegister(NRF24_CONFIG) | 0x02);
    vTaskDelay(5 / portTICK_PERIOD_MS);
}

void NRF24Modules::shutdownNRF() {
    this->setRegister(NRF24_CONFIG, getRegister(NRF24_CONFIG) * ~0x02);
}

void NRF24Modules::enableCE() {
    digitalWrite(NRF24_CE_PIN, HIGH);
}

void NRF24Modules::disableCE() {
    digitalWrite(NRF24_CE_PIN, LOW);
}

void NRF24Modules::setRX() {
    this->setRegister(NRF24_CONFIG, getRegister(NRF24_CONFIG) | 0x01);
    this->enableCE();
    delayMicroseconds(100);
}

void NRF24Modules::analyzerScanChannels() {
    this->disableCE();
    for (int i = 0; i < ANA_CHANNELS; i++) {
        NRFRadio.setChannel((128 * i) / ANA_CHANNELS);
        this->setRX();
        delayMicroseconds(40);
        this->disableCE();
        if (this->getRegister(NRF24_RPD) > 0) ana_channel[i]++;
    }
}

void NRF24Modules::writeRegister(uint8_t reg, uint8_t value) {
    digitalWrite(NRF24_CSN_PIN, LOW);
    SPI.transfer(reg | 0x20);
    SPI.transfer(value);
    digitalWrite(NRF24_CSN_PIN, HIGH);
}

uint8_t NRF24Modules::readRegister(uint8_t reg) {
    digitalWrite(NRF24_CSN_PIN, LOW);
    SPI.transfer(reg & 0x1F);
    uint8_t res = SPI.transfer(0x00);
    digitalWrite(NRF24_CSN_PIN, HIGH);
    return res;
}

void NRF24Modules::setChannel(uint8_t channel) {
    this->setRegister(NRF24_RF_CH, channel);
}

bool NRF24Modules::carrierDetected() {
    return this->readRegister(NRF24_RPD) & 0x01;
}

void NRF24Modules::analyzerSetup() {
    pinMode(NRF24_CE_PIN, OUTPUT);
    pinMode(NRF24_CSN_PIN, OUTPUT);

    SPI.begin(NRF24_SCK_PIN, NRF24_MISO_PIN, NRF24_MOSI_PIN, NRF24_CSN_PIN);
    SPI.setDataMode(SPI_MODE0);
    SPI.setFrequency(10000000);
    SPI.setBitOrder(MSBFIRST);

    digitalWrite(NRF24_CSN_PIN, HIGH);
    digitalWrite(NRF24_CE_PIN, LOW);

    NRFRadio.begin();
    this->disableCE();

    this->initNRF();
    writeRegister(NRF24_EN_AA, 0x00);
    writeRegister(NRF24_RF_SETUP, 0x03);  
}

void NRF24Modules::jammerNRFRadioSetup() {
    if (!NRFRadio.begin()) {
        Serial.println("[ERROR] NRF Radio Initialization Failed!");
        return;
    }
    NRFRadio.setAutoAck(false);
    NRFRadio.stopListening();
    NRFRadio.setRetries(0, 0);
    NRFRadio.setPALevel(RF24_PA_MAX);
    NRFRadio.setDataRate(RF24_2MBPS);
    NRFRadio.setCRCLength(RF24_CRC_DISABLED);
    NRFRadio.startConstCarrier(RF24_PA_HIGH, 45);
}

int jammer_chan_hop = 0;

void NRF24Modules::jammerNRFRadioMain(NRFJammerMode mode) {
    jammer_chan_hop++;
    if (mode == Wifi) {
        if (jammer_chan_hop >= sizeof(WiFi_channels)) jammer_chan_hop = 0;
        NRFRadio.setChannel(WiFi_channels[jammer_chan_hop]);
    } else if (mode == BLE) {
        if (jammer_chan_hop >= sizeof(ble_channels)) jammer_chan_hop = 0;
        NRFRadio.setChannel(ble_channels[jammer_chan_hop]);
    } else if (mode == Bluetooth) {
        if (jammer_chan_hop >= sizeof(bluetooth_channels)) jammer_chan_hop = 0;
        NRFRadio.setChannel(bluetooth_channels[jammer_chan_hop]);
    } else if (mode == Zigbee) {
        if (jammer_chan_hop >= sizeof(zigbee_channels)) jammer_chan_hop = 0;
        NRFRadio.setChannel(zigbee_channels[jammer_chan_hop]);
    } else if (mode == RC) {
        if (jammer_chan_hop >= sizeof(rc_channels)) jammer_chan_hop = 0;
        NRFRadio.setChannel(rc_channels[jammer_chan_hop]);
    } else if (mode == Video_Transmitter) {
        if (jammer_chan_hop >= sizeof(videoTransmitter_channels)) jammer_chan_hop = 0;
        NRFRadio.setChannel(videoTransmitter_channels[jammer_chan_hop]);
    } else if (mode == Usb_Wireless) {
        if (jammer_chan_hop >= sizeof(usbWireless_channels)) jammer_chan_hop = 0;
        NRFRadio.setChannel(usbWireless_channels[jammer_chan_hop]);
    } else if (mode == Full_Channel) {
        if (jammer_chan_hop >= sizeof(full_channels)) jammer_chan_hop = 0;
        NRFRadio.setChannel(full_channels[jammer_chan_hop]);
    }
}

void NRF24Modules::shutdownNRFJammer() {
    NRFRadio.stopConstCarrier();
}

void NRF24Modules::scanChannel() {
    this->disableCE();

    memset(scan_channel, 0, sizeof(scan_channel));

    const int samplesPerChannel = 50;

    for (int i = 0; i < SCAN_CHANNELS; i++) {
        NRFRadio.setChannel((128 * i) / SCAN_CHANNELS);

        for (int j = 0; j < samplesPerChannel; j++) {
            if (check(selPress)) {
                Serial.println("[INFO] NRF24 Scanner stopped by user.");
                return;
            }
            this->setRX();
            
            delayMicroseconds(100);
            this->disableCE();
            scan_channel[i] += getRegister(NRF24_RPD);
        }
        scan_channel[i] = (scan_channel[i] * 100) / samplesPerChannel;
    }
}

void NRF24Modules::loadPreviousGraph() {
    EEPROM.begin(128);
    for (byte i = 0; i < 128; i++) {
      sensorArray[i] = EEPROM.read(EEPROM_ADDRESS_SENSOR_ARRAY + i);
    }
    EEPROM.end();
}

void NRF24Modules::saveGraphtoEEPROM() {
    EEPROM.begin(128);
    for (byte i = 0; i < 128; i++) {
      EEPROM.write(EEPROM_ADDRESS_SENSOR_ARRAY + i, sensorArray[i]);
    }
    EEPROM.commit();
    EEPROM.end();
}

void NRF24Modules::scannerSetup() {
    for (byte count = 0; count <= 128; count++) {
      sensorArray[count] = 0;
    }

    SPI.begin(NRF24_SCK_PIN, NRF24_MISO_PIN, NRF24_MOSI_PIN, NRF24_CSN_PIN);
    SPI.setDataMode(SPI_MODE0);
    SPI.setFrequency(16000000);
    SPI.setBitOrder(MSBFIRST);

    NRFRadio.begin(); // Initialize RadioA
    this->disableCE();

    this->initNRF();
    this->setRegister(NRF24_EN_AA, 0x0);
    this->setRegister(NRF24_RF_SETUP, 0x0F);

    this->loadPreviousGraph();
}