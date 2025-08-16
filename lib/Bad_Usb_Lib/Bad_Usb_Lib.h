#ifndef BAD_USB_LIB_H
#define BAD_USB_LIB_H

#include "keys.h"
#include <Arduino.h>
class HIDInterface : public Print {
public:
    virtual void begin(Stream &stream, const uint8_t *layout) {};
    virtual void begin(const uint8_t *layout, uint8_t mode) {};
    virtual void begin(const uint8_t *layout) {};
    virtual void end(void) {};
    virtual int getReportData(uint8_t *buffer, size_t size) { return 0; };
    virtual size_t write(uint8_t c) { return 0; } ;
    virtual size_t write(const MediaKeyReport c) { return 0; };
    virtual size_t write(const uint8_t *buffer, size_t size) { return 0; };
    virtual size_t press(uint8_t k) { return 0; };
    virtual size_t pressRaw(uint8_t k) { return 0; };
    virtual size_t press(const MediaKeyReport k) { return 0; };
    virtual size_t pressMouse(const uint16_t b = MOUSE_LEFT) { return 0; };
    virtual size_t release(uint8_t k) { return 0; };
    virtual size_t releaseMouse(const uint16_t b = MOUSE_LEFT) { return 0; };
    virtual size_t releaseRaw(uint8_t k) { return 0; };
    virtual void resetButtons() {};
    virtual void setAxes(int16_t x, int16_t y, int16_t a1, int16_t a2, int16_t a3, int16_t a4, int16_t a5, int16_t a6, signed char hat1, signed char hat2, signed char hat3, signed char hat4) {};
    virtual size_t pressButton(uint8_t b) { return 0; };
    virtual size_t releaseButton(uint8_t b) { return 0; };
    virtual bool isPressedButton(uint8_t b) { return false; };
    virtual void click(const uint16_t b = MOUSE_LEFT) {};
    virtual void move(signed char x, signed char y, signed char wheel = 0, signed char hWheel = 0) {};
    virtual void wheel(signed char wheel = 0, signed char hWheel = 0) {};
    virtual bool isPressed(const uint16_t b) { return false; };
    virtual void releaseAll(void) {};
    
    virtual bool isConnected() { return false; };
    virtual void setLayout(const uint8_t *layout) {};
};
#endif
