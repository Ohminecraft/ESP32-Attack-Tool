#pragma once

#ifndef CONFIGS_H
#define CONFIGS_H

// BOARD CONFIGURATION //

//#define BOARD_ESP32_C3_MINI
#define BOARD_XIAO_ESP32_C3

// BLE Configuration //

#define SOUR_APPLE_SPAM_DELAY 20
#define APPLE_JUICE_SPAM_DELAY 400
#define SWIFTPAIR_SPAM_DELAY 20

#define ATTACK_TOOL_VERSION "2.8.1"

// SCREEN CONFIGURATION //

#define MAX_SHOW_SECLECTION 4
#define SCR_WIDTH 128
#define SCR_HEIGHT 64
#define GRAPH_VERTICAL_LINE_LIM (SCR_HEIGHT / 2) + 15

#if defined(BOARD_ESP32)
    #define SDA_PIN 21
    #define SCL_PIN 22
    #define RST_PIN -1
#elif defined(BOARD_ESP32_C3_MINI)
    #define SDA_PIN 8
    #define SCL_PIN 9
    #define RST_PIN -1
#elif defined(BOARD_XIAO_ESP32_C3)
    #define SDA_PIN 6
    #define SCL_PIN 7
    #define RST_PIN -1
#endif

// LED CONFIGURATION //

#if defined(BOARD_ESP32_C3_MINI)
    #define STA_LED 29 // Not Using
#elif defined(BOARD_XIAO_ESP32_C3)
    #define STA_LED 29
#elif defined(BOARD_ESP32)
    #define STA_LED 2
#endif


// INPUT CONFIGURATION //

#define USING_ENCODER
//#define USING_BUTTON

#define ENC_PIN_A 3
#define ENC_PIN_B 4
#define ENC_BTN 2

#define LEFT_BTN 1
#define RIGHT_BTN 2

#if defined(USING_ENCODER)
    #define SEL_BTN ENC_BTN
#else
    #define SEL_BTN 0
#endif

// SPI Global Configuration //

#ifdef BOARD_ESP32
    #define SPI_MOSI_PIN 23
    #define SPI_MISO_PIN 19
    #define SPI_SCK_PIN 18
#elif defined(BOARD_ESP32_C3_MINI)
    #define SPI_MOSI_PIN 6
    #define SPI_MISO_PIN 5
    #define SPI_SCK_PIN 4
#elif defined(BOARD_XIAO_ESP32_C3)
    #define SPI_MOSI_PIN 10
    #define SPI_MISO_PIN 9
    #define SPI_SCK_PIN 8
#endif
// SD Card Configuration //

#ifdef BOARD_ESP32
    #define SD_CS_PIN 5
#elif defined(BOARD_ESP32_C3_MINI)
    #define SD_CS_PIN 10
#elif defined(BOARD_XIAO_ESP32_C3)
    #define SD_CS_PIN 5
#endif

// NRF24 CONFIGURATION //

#if defined(BOARD_ESP32)
    #define NRF24_CE_PIN 8
    #define NRF24_CSN_PIN 17
#elif defined(BOARD_ESP32_C3_MINI)
    #define NRF24_CE_PIN 3
    #define NRF24_CSN_PIN 7
#elif defined(BOARD_XIAO_ESP32_C3)
    #define NRF24_CE_PIN 99
    #define NRF24_CSN_PIN 99
#endif

// IR Configuration //

#define IR_REPEAT 0

#ifdef BOARD_ESP32
    #define IR_PIN 15
    #define IR_RX_PIN 16 // RX PIN
#elif defined(BOARD_ESP32_C3_MINI)
    #define IR_PIN 20 // RX PIN in Esp32-C3-MINI
    #define IR_RX_PIN 21 // TX PIN in Esp32-C3-MINI
#elif defined(BOARD_XIAO_ESP32_C3)
    #define IR_PIN 99
    #define IR_RX_PIN 99
#endif

// Serial Configuration (for RTL8720DN) //

#ifdef BOARD_XIAO_ESP32_C3
    #define RTL_SERIAL_BAUD 115200
    #define SERIAL_RX_PIN 20
    #define SERIAL_TX_PIN 21
#endif
#endif // CONFIGS_H