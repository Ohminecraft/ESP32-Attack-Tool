#include "nrf24header.h"

/*
    * nrf24.cpp (Based nRFBox by @ciferTech)
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

RF24 NRFRadio(NRF24_CE_PIN, NRF24_CSN_PIN, 16000000);
SPIClass *NRFSPI = nullptr;

byte NRF24Modules::getRegister(SPIClass &SPIIN, byte r) {
    byte c;
    
    digitalWrite(NRF24_CSN_PIN, LOW);
    SPIIN.transfer(r & 0x1F);
    c = SPIIN.transfer(0);
    digitalWrite(NRF24_CSN_PIN, HIGH);

    return c;
}

void NRF24Modules::setRegister(SPIClass &SPIIN, byte r, byte v) {
    digitalWrite(NRF24_CSN_PIN, LOW);
    SPIIN.transfer((r & 0x1F) | 0x20);
    SPIIN.transfer(v);
    digitalWrite(NRF24_CSN_PIN, HIGH);
}

void NRF24Modules::initNRF() {
    this->setRegister(*NRFSPI, NRF24_CONFIG, this->getRegister(*NRFSPI, NRF24_CONFIG) | 0x02);
    vTaskDelay(5 / portTICK_PERIOD_MS);
}

void NRF24Modules::shutdownNRF() {
    this->setRegister(*NRFSPI, NRF24_CONFIG, getRegister(*NRFSPI, NRF24_CONFIG) * ~0x02);
}
void NRF24Modules::setRX() {
    this->setRegister(*NRFSPI, NRF24_CONFIG, getRegister(*NRFSPI, NRF24_CONFIG) | 0x01);
    digitalWrite(NRF24_CE_PIN, HIGH);
    delayMicroseconds(100);
}

void NRF24Modules::analyzerScanChannels() {
    digitalWrite(NRF24_CE_PIN, LOW);
    for (int i = 0; i < ANA_CHANNELS; i++) {
        NRFRadio.setChannel((128 * i) / ANA_CHANNELS);
        this->setRX();
        delayMicroseconds(40);
        digitalWrite(NRF24_CE_PIN, LOW);
        if (this->getRegister(*NRFSPI, NRF24_RPD) > 0) ana_channel[i]++;
    }
}

void NRF24Modules::writeRegister(SPIClass &SPIIN, uint8_t reg, uint8_t value) {
    digitalWrite(NRF24_CSN_PIN, LOW);
    SPIIN.transfer(reg | 0x20);
    SPIIN.transfer(value);
    digitalWrite(NRF24_CSN_PIN, HIGH);
}

uint8_t NRF24Modules::readRegister(SPIClass &SPIIN, uint8_t reg) {
    digitalWrite(NRF24_CSN_PIN, LOW);
    SPIIN.transfer(reg & 0x1F);
    uint8_t res = SPIIN.transfer(0x00);
    digitalWrite(NRF24_CSN_PIN, HIGH);
    return res;
}

void NRF24Modules::setChannel(uint8_t channel) {
    this->setRegister(*NRFSPI, NRF24_RF_CH, channel);
}

bool NRF24Modules::carrierDetected() {
    return this->readRegister(*NRFSPI, NRF24_RPD) & 0x01;
}

void NRF24Modules::analyzerSetup() {
    pinMode(NRF24_CE_PIN, OUTPUT);
    pinMode(NRF24_CSN_PIN, OUTPUT);

    NRFSPI = new SPIClass(FSPI);
    NRFSPI->begin(NRF24_SCK_PIN, NRF24_MISO_PIN, NRF24_MOSI_PIN, NRF24_CSN_PIN);
    NRFSPI->setDataMode(SPI_MODE0);
    NRFSPI->setFrequency(16000000);
    NRFSPI->setBitOrder(MSBFIRST);

    digitalWrite(NRF24_CSN_PIN, HIGH);
    digitalWrite(NRF24_CE_PIN, LOW);

    NRFRadio.begin(NRFSPI, rf24_gpio_pin_t(NRF24_CE_PIN), rf24_gpio_pin_t(NRF24_CSN_PIN));

    digitalWrite(NRF24_CE_PIN, LOW);

    this->initNRF();
    writeRegister(*NRFSPI, NRF24_EN_AA, 0x00);
    writeRegister(*NRFSPI, NRF24_RF_SETUP, 0x03);  
}

void NRF24Modules::jammerNRFRadioSetup() {
    pinMode(NRF24_CE_PIN, OUTPUT);
    pinMode(NRF24_CSN_PIN, OUTPUT);
    digitalWrite(NRF24_CE_PIN, LOW);
    digitalWrite(NRF24_CSN_PIN, HIGH);
    NRFSPI = new SPIClass(FSPI); // Create a new SPIClass instance for NRF24
    NRFSPI->begin(NRF24_SCK_PIN, NRF24_MISO_PIN, NRF24_MOSI_PIN, NRF24_CSN_PIN);
    NRFSPI->setDataMode(SPI_MODE0);
    NRFSPI->setFrequency(16000000);
    NRFSPI->setBitOrder(MSBFIRST);
    // Initialize the NRF24 radio
    if (!NRFRadio.begin(NRFSPI, rf24_gpio_pin_t(NRF24_CE_PIN), rf24_gpio_pin_t(NRF24_CSN_PIN))) {
        Serial.println("[ERROR] NRF Radio Initialization Failed!");
        return;
    }
    NRFRadio.setAutoAck(false);
    NRFRadio.stopListening();
    NRFRadio.setRetries(0, 0);
    NRFRadio.setPALevel(RF24_PA_MAX);
    NRFRadio.setDataRate(RF24_2MBPS);
    NRFRadio.setCRCLength(RF24_CRC_DISABLED);
    NRFRadio.startConstCarrier(RF24_PA_MAX, 45);
}

int jammer_chan_hop = 0;

void NRF24Modules::jammerNRFRadioMain(NRFJammerMode mode) {
    jammer_chan_hop++;
    if (mode == Wifi) {
        if (jammer_chan_hop >= (sizeof(WiFi_channels) / sizeof(WiFi_channels[0]))) jammer_chan_hop = 0;
        NRFRadio.setChannel(WiFi_channels[jammer_chan_hop]);
        NRFRadio.writeFast(&write_text, sizeof(write_text));
    } else if (mode == BLE) {
        if (jammer_chan_hop >= (sizeof(ble_channels) / sizeof(ble_channels[0]))) jammer_chan_hop = 0;
        NRFRadio.setChannel(ble_channels[jammer_chan_hop]);
        NRFRadio.writeFast(&write_text, sizeof(write_text));
    } else if (mode == Bluetooth) {
        if (jammer_chan_hop >= (sizeof(bluetooth_channels) / sizeof(bluetooth_channels[0]))) jammer_chan_hop = 0;
        NRFRadio.setChannel(bluetooth_channels[jammer_chan_hop]);
        NRFRadio.writeFast(&write_text, sizeof(write_text));
    } else if (mode == Zigbee) {
        if (jammer_chan_hop >= (sizeof(zigbee_channels) / sizeof(zigbee_channels[0]))) jammer_chan_hop = 0;
        NRFRadio.setChannel(zigbee_channels[jammer_chan_hop]);
        NRFRadio.writeFast(&write_text, sizeof(write_text));
    } else if (mode == RC) {
        if (jammer_chan_hop >= (sizeof(rc_channels) / sizeof(rc_channels[0]))) jammer_chan_hop = 0;
        NRFRadio.setChannel(rc_channels[jammer_chan_hop]);
    } else if (mode == Video_Transmitter) {
        if (jammer_chan_hop >= (sizeof(videoTransmitter_channels) / sizeof(videoTransmitter_channels[0]))) jammer_chan_hop = 0;
        NRFRadio.setChannel(videoTransmitter_channels[jammer_chan_hop]);
    } else if (mode == Usb_Wireless) {
        if (jammer_chan_hop >= (sizeof(usbWireless_channels) / sizeof(usbWireless_channels[0]))) jammer_chan_hop = 0;
        NRFRadio.setChannel(usbWireless_channels[jammer_chan_hop]);
    } else if (mode == Full_Channel) {
        if (jammer_chan_hop >= (sizeof(full_channels) / sizeof(full_channels[0]))) jammer_chan_hop = 0;
        NRFRadio.setChannel(full_channels[jammer_chan_hop]);
    }
    if (selPress) {
        Serial.println("[INFO] Jammer Stopped by user");
        return;
    }
}

void NRF24Modules::shutdownNRFJammer() {
    NRFRadio.stopConstCarrier();
}

void NRF24Modules::scanChannel() {
    digitalWrite(NRF24_CE_PIN, LOW);

    memset(scan_channel, 0, sizeof(scan_channel));

    const int samplesPerChannel = 50;

    for (int i = 0; i < SCAN_CHANNELS; i++) {
        NRFRadio.setChannel((128 * i) / SCAN_CHANNELS);

        for (int j = 0; j < samplesPerChannel; j++) {
            if (selPress) {
                return;
            }
            this->setRX();
            
            delayMicroseconds(100);
            digitalWrite(NRF24_CE_PIN, LOW);
            scan_channel[i] += getRegister(*NRFSPI, NRF24_RPD);
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

    pinMode(NRF24_CE_PIN, OUTPUT);
    pinMode(NRF24_CSN_PIN, OUTPUT);
    NRFSPI = new SPIClass(FSPI); // Create a new SPIClass instance for NRF24
    NRFSPI->begin(NRF24_SCK_PIN, NRF24_MISO_PIN, NRF24_MOSI_PIN, NRF24_CSN_PIN);
    NRFSPI->setDataMode(SPI_MODE0);
    NRFSPI->setFrequency(16000000);
    NRFSPI->setBitOrder(MSBFIRST);

    NRFRadio.begin(NRFSPI, rf24_gpio_pin_t(NRF24_CE_PIN), rf24_gpio_pin_t(NRF24_CSN_PIN));

    digitalWrite(NRF24_CE_PIN, LOW);

    this->initNRF();
    this->setRegister(*NRFSPI, NRF24_EN_AA, 0x0);
    this->setRegister(*NRFSPI, NRF24_RF_SETUP, 0x0F);

    this->loadPreviousGraph();
}