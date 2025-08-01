#pragma once

#ifndef DISPLAYHEADER_H
#define DISPLAYHEADER_H

/*
    * displayheader.h Version 2.0 (with Claude, ChatGPT Help)
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * This file contains the declaration of the DisplayModules class which handles the initialization and management of an SSD1306 display.
    * It includes methods for initializing the display, clearing the screen, displaying strings, and managing a display buffer.
    * The display is used to show information and messages in the ESP32 Attack Tool.
*/

#include <Arduino.h>
#include <U8g2lib.h>
#include <LinkedList.h>
#include <SPI.h>
#include <Wire.h>

#include "core/utilsheader.h"
#include "configs.h"


// Define color constants for U8g2 compatibility
#define WHITE 1
#define BLACK 0
extern LinkedList<String>* display_buffer;

class DisplayModules
{
    private:
    #ifdef _SPI_SCREEN
        U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2;
    #elif defined(_I2C_SCREEN)
        U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
    #endif
        bool screenInitialized = false;
        
    public:
        DisplayModules();
        ~DisplayModules();

        bool main();

        void clearScreen();
        void drawingRect(int x, int y, int w, int h, bool fill_rect = true, int color = WHITE);
        void printString(String msg);
        void displayString(String msg, bool ln = false, bool senddisplay = false, int color = WHITE);
        void drawingCenterString(String msg, int y, bool senddisplay = false, int color = WHITE);
        void displayInvert(bool invert);
        void displayStringwithCoordinates(String msg, int x, int y, bool senddisplay = false, int color = WHITE);
        String wrapText(String input, int lineWidth = 21);
        void displayBuffer();
        void clearBuffer();
        void simpleStrikethrough(String text, int x, int y);
        void displayEvilPortalText(String username, String password);
        void drawingVLine(int x, int y, int h, int color = WHITE);
        void drawingLine(int x1, int y1, int x2, int y2, bool sendDisplay = false);
        void drawingPixel(int x, int y, bool sendDisplay = false);
        void sendDisplay();
        //void addToBuffer(String msg);
        void drawBipmap(int x, int y, int w, int h, const uint8_t* icon, bool senddisplay = false);
        
        // Additional U8g2 specific methods you might want to add
        void setCursor(int x, int y);
        void setFont(const uint8_t *font);
        int getStrWidth(const char *s);
        void drawMultiLineText(String text, int x, int y, int lineHeight = 12);
        int drawWrappedText(String text, int x, int y, int charsPerLine = 21);
};

#endif // DISPLAYHEADER_H