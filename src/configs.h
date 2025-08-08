#ifndef CONFIGS_H
#define CONFIGS_H

// BOARD CONFIGURATION //
#define BOARD_ESP32_C3_MINI

// BLE Configuration //
/*
    Using NimBLE Arduino Library For Better Life Time And Efficiency
    If You Want To Use ESP32 BLE Library, Just Comment This Line
    Use Original ESP32 BLE Library For Better Spoofer Experience
*/
#define USE_NIMBLE 

// SCREEN CONFIGURATION //
#define SCR_WIDTH 128
#define SCR_HEIGHT 64
#define _I2C_SCREEN

#ifdef _SPI_SCREEN
    #if defined(BOARD_ESP32)
        #define SPI_MOSI 23
        #define SPI_MISO 19
        #define SPI_SCK 18
        #define CS_PIN 5
        #define DC_PIN 4
        #define RST_PIN 16
    #elif defined(BOARD_ESP32_C3_MINI)
        #define SPI_MOSI 6
        #define SPI_MISO 5
        #define SPI_SCK 4
        #define CS_PIN 10
        #define DC_PIN 8
        #define RST_PIN 9
    #endif
#elif defined(_I2C_SCREEN)
    #if defined(BOARD_ESP32)
        #define SDA_PIN 21
        #define SCL_PIN 22
        #define RST_PIN -1
    #elif defined(BOARD_ESP32_C3_MINI)
        #define SDA_PIN 8
        #define SCL_PIN 9
        #define RST_PIN -1
    #endif
#endif
// LED CONFIGURATION //
#ifdef BOARD_ESP32_C3_MINI
    #define STA_LED 21 // Not Using
#elif BOARD_ESP32
    #define STA_LED 2
#endif


// ENCODER CONFIGURATION //
#define USING_ENCODER

#define ENC_PIN_A 2
#define ENC_PIN_B 1
#define ENC_BTN 0

// BUTTON CONFIGURATION //
//#define USING_BUTTON

#define LEFT_BTN 1
#define RIGHT_BTN 2

#if defined(USING_ENCODER)
    #define SEL_BTN ENC_BTN
#else
    #define SEL_BTN 0
#endif

// NRF24 CONFIGURATION //
#if defined(BOARD_ESP32)
    #define NRF24_CE_PIN 8
    #define NRF24_CSN_PIN 17
    #define NRF24_MOSI_PIN 23
    #define NRF24_MISO_PIN 19
    #define NRF24_SCK_PIN 18
#elif defined(BOARD_ESP32_C3_MINI)
    #define NRF24_CE_PIN 3
    #define NRF24_CSN_PIN 7
    #define NRF24_MOSI_PIN 6
    #define NRF24_MISO_PIN 5
    #define NRF24_SCK_PIN 4
#endif

// IR Configuration //
#ifdef BOARD_ESP32
    #define IR_PIN 15
    #define IR_RX_PIN 16 // RX PIN
#elif defined(BOARD_ESP32_C3_MINI)
    #define IR_PIN 20 // RX PIN in Esp32-C3-MINI
    #define IR_RX_PIN 21 // TX PIN in Esp32-C3-MINI
#endif
#endif