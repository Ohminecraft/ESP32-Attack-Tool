// uncomment the following line to use NimBLE library
//#define USE_NIMBLE

#ifndef ESP32_BLE_KEYBOARD_H
#define ESP32_BLE_KEYBOARD_H
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "NimBLECharacteristic.h"
#include "NimBLEHIDDevice.h"
#include "NimBLEServer.h"

#include "Bad_Usb_Lib.h"
#include "Print.h"
#include "keys.h"

#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C2) ||                              \
    defined(CONFIG_IDF_TARGET_ESP32S3)
#define MAX_TX_POWER ESP_PWR_LVL_P21 // ESP32C3 ESP32C2 ESP32S3
#elif defined(CONFIG_IDF_TARGET_ESP32H2) || defined(CONFIG_IDF_TARGET_ESP32C6)
#define MAX_TX_POWER ESP_PWR_LVL_P20 // ESP32H2 ESP32C6
#else
#define MAX_TX_POWER ESP_PWR_LVL_P9 // Default
#endif

#define BLE_KEYBOARD_VERSION "0.0.4"
#define BLE_KEYBOARD_VERSION_MAJOR 0
#define BLE_KEYBOARD_VERSION_MINOR 0
#define BLE_KEYBOARD_VERSION_REVISION 4


#define BLE_KEYBOARD_MODE_KEYBOARD 1
#define BLE_KEYBOARD_MODE_MOUSE 2
#define BLE_KEYBOARD_MODE_GAMEPAD 3
#define BLE_KEYBOARD_MODE_ALL 4

class BleKeyboard : public NimBLEServerCallbacks, public NimBLECharacteristicCallbacks, public HIDInterface
{
private:
  uint8_t _buttons;
  uint8_t _buttonsGamepad[16];
  NimBLEHIDDevice *hid;
  NimBLECharacteristic *inputKeyboard;
  NimBLECharacteristic *outputKeyboard;
  NimBLECharacteristic *inputMediaKeys;
  NimBLECharacteristic *inputMouse;
  NimBLECharacteristic *inputGamepad;
  NimBLECharacteristic *outputGamepad;

  NimBLEAdvertising *advertising;
  KeyReport _keyReport;
  MediaKeyReport _mediaKeyReport;
  std::string deviceName;
  std::string deviceManufacturer;
  uint8_t batteryLevel;
  bool connected = false;
  uint32_t _delay_ms = 7;
  void delay_ms(uint64_t ms);
  void buttons(const uint16_t b);

  uint16_t vid = 0x05ac;
  uint16_t pid = 0x820a;
  uint16_t version = 0x0210;

  uint16_t appearance = 0x03C1;

  const uint8_t *_asciimap;

public:
  BleKeyboard(std::string deviceName = "ESP32 Keyboard", std::string deviceManufacturer = "Espressif", uint8_t batteryLevel = 100);
  void begin(const uint8_t *layout = KeyboardLayout_en_US, uint8_t mode = BLE_KEYBOARD_MODE_ALL) override { begin(layout, HID_KEYBOARD, mode); };
  void begin(const uint8_t *layout, uint16_t showAs, uint8_t mode);
  void end(void);
  void setLayout(const uint8_t *layout = KeyboardLayout_en_US) { _asciimap = layout; }
  void sendReport(KeyReport* keys);
  void sendReport(MediaKeyReport* keys);
  size_t press(uint8_t k);
  size_t press(const MediaKeyReport k);
  size_t pressMouse(const uint16_t b = MOUSE_LEFT);
  size_t release(uint8_t k);
  size_t release(const MediaKeyReport k);
  size_t releaseMouse(const uint16_t b = MOUSE_LEFT);
  size_t write(uint8_t c);
  size_t write(const MediaKeyReport c);
  size_t write(const uint8_t *buffer, size_t size);
  void releaseAll(void);

  // keypad magic
  void resetButtons();
  void setAxes(int16_t x, int16_t y, int16_t a1, int16_t a2, int16_t a3, int16_t a4, int16_t a5, int16_t a6, signed char hat1, signed char hat2, signed char hat3, signed char hat4);
  size_t pressButton(uint8_t b);
  size_t releaseButton(uint8_t b);
  bool isPressedButton(uint8_t b);

  void click(const uint16_t b = MOUSE_LEFT );
  void move(signed char x, signed char y, signed char wheel = 0, signed char hWheel = 0);
  void wheel(signed char wheel = 0, signed char hWheel = 0);
  bool isPressed(const uint16_t b);

  bool isConnected(void);
  void setBatteryLevel(uint8_t level);
  void setAppearence(uint16_t v) { appearance = v; }
  void setRandomUUID(void) { _randUUID = !_randUUID; };
  uint16_t getAppearence() { return appearance; }
  void setName(std::string deviceName);  
  void setDelay(uint32_t ms);

  void set_vendor_id(uint16_t vid);
  void set_product_id(uint16_t pid);
  void set_version(uint16_t version);
protected:
  bool _randUUID = false;
  virtual void onStarted(NimBLEServer *pServer) { };
  //virtual void onConnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo) override;
  virtual void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo& connInfo, int reason) override;
  virtual void onWrite(NimBLECharacteristic* me, NimBLEConnInfo& connInfo) override;
  virtual void onAuthenticationComplete(NimBLEConnInfo& connInfo) override;
  virtual void onSubscribe(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo, uint16_t subValue) override;

};

#endif // CONFIG_BT_ENABLED
#endif // ESP32_BLE_KEYBOARD_H