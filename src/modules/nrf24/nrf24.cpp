#include "nrf24header.h"

/*
    * nrf24.cpp (Based nRFBox by @ciferTech)
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

RF24 NRFRadio(rf24_gpio_pin_t(espatsettings.nrfCePin), rf24_gpio_pin_t(espatsettings.nrfCsPin));
SPIClass *NRFSPI;

uint8_t scan_channel[SCAN_CHANNELS];
int16_t sensorArray[SCR_WIDTH];
uint8_t values[SCR_WIDTH];

void NRF24Modules::main() {
    // Initialize the NRF24 radio
    NRFSPI = &SPI;
    NRFSPI->begin(espatsettings.spiSckPin,
                  espatsettings.spiMisoPin,
                  espatsettings.spiMosiPin,
                  espatsettings.nrfCsPin);
    NRFSPI->setDataMode(SPI_MODE0);
    NRFSPI->setFrequency(10000000);
    NRFSPI->setBitOrder(MSBFIRST);

    pinMode(espatsettings.nrfCsPin, OUTPUT);
    pinMode(espatsettings.nrfCePin, OUTPUT);
    digitalWrite(espatsettings.nrfCsPin, HIGH);
    digitalWrite(espatsettings.nrfCePin, LOW);

    if (!NRFRadio.begin(NRFSPI, rf24_gpio_pin_t(espatsettings.nrfCePin), rf24_gpio_pin_t(espatsettings.nrfCsPin))) {
        Serial.println("[ERROR] NRF Radio Initialization Failed!");
        return;
    }
    Serial.println("[INFO] NRF Radio Initialized Successfully!");

    this->initNRF(*NRFSPI);
}

byte NRF24Modules::getRegister(SPIClass &SPIIN, byte r) {
    byte c;
    
    digitalWrite(espatsettings.nrfCsPin, LOW);
    SPIIN.transfer(r & 0x1F);
    c = SPIIN.transfer(0);
    digitalWrite(espatsettings.nrfCsPin, HIGH);

    return c;
}

void NRF24Modules::setRegister(SPIClass &SPIIN, byte r, byte v) {
    digitalWrite(espatsettings.nrfCsPin, LOW);
    SPIIN.transfer((r & 0x1F) | 0x20);
    SPIIN.transfer(v);
    digitalWrite(espatsettings.nrfCsPin, HIGH);
}

void NRF24Modules::initNRF(SPIClass &SPIIN) {
    this->setRegister(SPIIN, NRF24_CONFIG, this->getRegister(SPIIN, NRF24_CONFIG) | 0x02);
    delayMicroseconds(130);
}

void NRF24Modules::shutdownNRF(SPIClass &SPIIN) {
    this->setRegister(SPIIN, NRF24_CONFIG, getRegister(SPIIN, NRF24_CONFIG) * ~0x02);
}
void NRF24Modules::setRX(SPIClass &SPIIN) {
    this->setRegister(SPIIN, NRF24_CONFIG, getRegister(SPIIN, NRF24_CONFIG) | 0x01);
    digitalWrite(espatsettings.nrfCePin, HIGH);
    delayMicroseconds(100);
}

void NRF24Modules::analyzerScanChannels() {
    digitalWrite(espatsettings.nrfCePin, LOW);
    for (int i = 0; i < ANA_CHANNELS; i++) {
        NRFRadio.setChannel((128 * i) / ANA_CHANNELS);
        this->setRX(*NRFSPI);
        delayMicroseconds(40);
        digitalWrite(espatsettings.nrfCePin, LOW);
    }
}

void NRF24Modules::writeRegister(SPIClass &SPIIN, uint8_t reg, uint8_t value) {
    digitalWrite(espatsettings.nrfCsPin, LOW);
    SPIIN.transfer(reg | 0x20);
    SPIIN.transfer(value);
    digitalWrite(espatsettings.nrfCsPin, HIGH);
}

uint8_t NRF24Modules::readRegister(SPIClass &SPIIN, uint8_t reg) {
    digitalWrite(espatsettings.nrfCsPin, LOW);
    SPIIN.transfer(reg & 0x1F);
    uint8_t res = SPIIN.transfer(0x00);
    digitalWrite(espatsettings.nrfCsPin, HIGH);
    return res;
}

void NRF24Modules::setChannel(SPIClass &SPIIN, uint8_t channel) {
    this->setRegister(SPIIN, NRF24_RF_CH, channel);
}

bool NRF24Modules::carrierDetected(SPIClass &SPIIN) {
    return this->readRegister(SPIIN, NRF24_RPD) & 0x01;
}

void NRF24Modules::analyzerSetup() {
    pinMode(espatsettings.nrfCePin, OUTPUT);
    pinMode(espatsettings.nrfCsPin, OUTPUT);

    NRFSPI = &SPI;
    NRFSPI->begin(espatsettings.spiSckPin,
                  espatsettings.spiMisoPin,
                  espatsettings.spiMosiPin,
                  espatsettings.nrfCsPin);
    NRFSPI->setDataMode(SPI_MODE0);
    NRFSPI->setFrequency(10000000);
    NRFSPI->setBitOrder(MSBFIRST);

    digitalWrite(espatsettings.nrfCsPin, HIGH);
    digitalWrite(espatsettings.nrfCePin, LOW);

    if (!NRFRadio.begin(NRFSPI, rf24_gpio_pin_t(espatsettings.nrfCePin), rf24_gpio_pin_t(espatsettings.nrfCsPin))) {
        Serial.println("[ERROR] NRF Radio Initialization Failed!");
        return;
    }

    digitalWrite(espatsettings.nrfCePin, LOW);

    this->initNRF(*NRFSPI);
    writeRegister(*NRFSPI, NRF24_EN_AA, 0x00);
    writeRegister(*NRFSPI, NRF24_RF_SETUP, 0x03);  
}

void NRF24Modules::jammerNRFRadioSetup() {
    pinMode(espatsettings.nrfCePin, OUTPUT);
    pinMode(espatsettings.nrfCsPin, OUTPUT);
    digitalWrite(espatsettings.nrfCePin, LOW);
    digitalWrite(espatsettings.nrfCsPin, HIGH);

    NRFSPI = &SPI;
    NRFSPI->begin(espatsettings.spiSckPin,
                  espatsettings.spiMisoPin,
                  espatsettings.spiMosiPin,
                  espatsettings.nrfCsPin);
    NRFSPI->setDataMode(SPI_MODE0);
    NRFSPI->setFrequency(16000000);
    NRFSPI->setBitOrder(MSBFIRST);
    // Initialize the NRF24 radio
    if (!NRFRadio.begin(NRFSPI, rf24_gpio_pin_t(espatsettings.nrfCePin), rf24_gpio_pin_t(espatsettings.nrfCsPin))) {
        Serial.println("[ERROR] NRF Radio Initialization Failed!");
        return;
    }
    NRFRadio.setAutoAck(false);
    NRFRadio.openWritingPipe(0xFFFFFFFFFF);
    NRFRadio.stopListening();
    NRFRadio.setRetries(0, 0);
    NRFRadio.setPALevel(RF24_PA_MAX);
    NRFRadio.setDataRate(RF24_2MBPS);
    NRFRadio.setPayloadSize(5);
    NRFRadio.setAddressWidth(3);
    NRFRadio.setCRCLength(RF24_CRC_DISABLED);
    NRFRadio.startConstCarrier(RF24_PA_MAX, 45);
}

void NRF24Modules::jammerNRFRadioMain(NRFJammerMode mode) {
    if (mode == Wifi) {
        //if (jammer_chan_hop >= (sizeof(WiFi_channels) / sizeof(WiFi_channels[0]))) jammer_chan_hop = 0;
        for (int i = 0; i < GET_SIZE(WiFi_channels); i++) { // This way is more effective than using jammer_chan_hop
            NRFRadio.setChannel(WiFi_channels[i]);
        }
        //NRFRadio.setChannel(WiFi_channels[jammer_chan_hop]);
    } else if (mode == BLE) {
        //if (jammer_chan_hop >= (sizeof(ble_channels) / sizeof(ble_channels[0]))) jammer_chan_hop = 0;
        //NRFRadio.setChannel(ble_channels[jammer_chan_hop]);
        for (int i = 0; i < GET_SIZE(ble_channels); i++) {
            NRFRadio.setChannel(ble_channels[i]);
        }
    } else if (mode == Bluetooth) {
        //if (jammer_chan_hop >= (sizeof(bluetooth_channels) / sizeof(bluetooth_channels[0]))) jammer_chan_hop = 0;
        //NRFRadio.setChannel(bluetooth_channels[jammer_chan_hop]);
        for (int i = 0; i < GET_SIZE(bluetooth_channels); i++) {
            NRFRadio.setChannel(bluetooth_channels[i]);
        }
    } else if (mode == Zigbee) {
        //if (jammer_chan_hop >= (sizeof(zigbee_channels) / sizeof(zigbee_channels[0]))) jammer_chan_hop = 0;
        //NRFRadio.setChannel(zigbee_channels[jammer_chan_hop]);
        for (int i = 0; i < GET_SIZE(zigbee_channels); i++) {
            NRFRadio.setChannel(zigbee_channels[i]);
        }
    } else if (mode == RC) {
        //if (jammer_chan_hop >= (sizeof(rc_channels) / sizeof(rc_channels[0]))) jammer_chan_hop = 0;
        //NRFRadio.setChannel(rc_channels[jammer_chan_hop]);
        for (int i = 0; i < GET_SIZE(rc_channels); i++) {
            NRFRadio.setChannel(rc_channels[i]);
        }
    } else if (mode == Video_Transmitter) {
        //if (jammer_chan_hop >= (sizeof(videoTransmitter_channels) / sizeof(videoTransmitter_channels[0]))) jammer_chan_hop = 0;
        //NRFRadio.setChannel(videoTransmitter_channels[jammer_chan_hop]);
        for (int i = 0; i < GET_SIZE(videoTransmitter_channels); i++) {
            NRFRadio.setChannel(videoTransmitter_channels[i]);
        }
    } else if (mode == Usb_Wireless) {
        //if (jammer_chan_hop >= (sizeof(usbWireless_channels) / sizeof(usbWireless_channels[0]))) jammer_chan_hop = 0;
        //NRFRadio.setChannel(usbWireless_channels[jammer_chan_hop]);
        for (int i = 0; i < GET_SIZE(usbWireless_channels); i++) {
            NRFRadio.setChannel(usbWireless_channels[i]);
        }
    } else if (mode == Drone) {
        //if (jammer_chan_hop >= (sizeof(drone) / sizeof(drone[0]))) jammer_chan_hop = 0;
        //NRFRadio.setChannel(drone[jammer_chan_hop]);
        for (int i = 0; i < GET_SIZE(drone); i++) {
            NRFRadio.setChannel(drone[i]);
        }
    } else if (mode == Full_Channel) {
        //if (jammer_chan_hop >= (sizeof(full_channels) / sizeof(full_channels[0]))) jammer_chan_hop = 0;
        //NRFRadio.setChannel(full_channels[jammer_chan_hop]);
        for (int i = 0; i < GET_SIZE(full_channels); i++) {
            NRFRadio.setChannel(full_channels[i]);
        }
    }
    //jammer_chan_hop++;
    //Serial.println("[INFO] Jamming Channel: " + String(jammer_chan_hop));
    if (selPress) {
        return;
    }
}

void NRF24Modules::shutdownNRFJammer() {
    NRFRadio.stopConstCarrier();
}

void NRF24Modules::scanChannel() {
   digitalWrite(espatsettings.nrfCePin, LOW);

    memset(scan_channel, 0, sizeof(scan_channel));

    const int samplesPerChannel = 50;

    for (int i = 0; i < SCAN_CHANNELS; i++) {
      NRFRadio.setChannel((128 * i) / SCAN_CHANNELS); // Use RF24 setChannel

      for (int j = 0; j < samplesPerChannel; j++) {
        // Check Select button in inner loop
        if (selPress) {
          return;
        }
        setRX(*NRFSPI);
        delayMicroseconds(100);
        digitalWrite(espatsettings.nrfCePin, LOW);
        scan_channel[i] += getRegister(*NRFSPI, NRF24_RPD);
      }

      scan_channel[i] = (scan_channel[i] * 100) / samplesPerChannel;
    }
}

void NRF24Modules::scannerSetup() {
    pinMode(espatsettings.nrfCePin, OUTPUT);
    pinMode(espatsettings.nrfCsPin, OUTPUT);
    digitalWrite(espatsettings.nrfCePin, LOW);
    digitalWrite(espatsettings.nrfCsPin, HIGH);

    NRFSPI = &SPI;
    NRFSPI->begin(espatsettings.spiSckPin,
                  espatsettings.spiMisoPin,
                  espatsettings.spiMosiPin,
                  espatsettings.nrfCsPin);
    NRFSPI->setDataMode(SPI_MODE0);
    NRFSPI->setFrequency(16000000);
    NRFSPI->setBitOrder(MSBFIRST);

    if (!NRFRadio.begin(NRFSPI, rf24_gpio_pin_t(espatsettings.nrfCePin), rf24_gpio_pin_t(espatsettings.nrfCsPin))) {
        Serial.println("[ERROR] NRF Radio Initialization Failed!");
        return;
    }

    digitalWrite(espatsettings.nrfCePin, LOW);

    this->initNRF(*NRFSPI);
    this->setRegister(*NRFSPI, NRF24_EN_AA, 0x00);
    this->setRegister(*NRFSPI, NRF24_RF_SETUP, 0x0F);

}