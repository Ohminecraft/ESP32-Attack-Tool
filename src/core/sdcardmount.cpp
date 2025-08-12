#include "sdcardmountheader.h"

/*
    * sdcardmount.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

SPIClass *SDCardSPI;
LinkedList<String> *sdcard_buffer;

void SDCardModules::main() {
    SDCardSPI = &SPI;
    sdcard_buffer = new LinkedList<String>();
    SDCardSPI->begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SD_CS_PIN);
    if (!SD.begin(SD_CS_PIN, *SDCardSPI)) {
        Serial.println("[ERROR] SD Card Mount Failed!");
        return;
    }
    mounted = true;
    Serial.println("[INFO] SD Card Mounted Successfully!");
    Serial.print("[INFO] SD Card Size: ");
    Serial.print(SD.cardSize() / (1024 * 1024));
    Serial.println(" MB");

    if(!SD.exists("/ESP32AttackTool")) {
        Serial.println("[INFO] Main Directory does not exist. Creating...");
        SD.mkdir("/ESP32AttackTool");
        Serial.println("[INFO] Main Directory created successfully.");
    }
}

File SDCardModules::getFile(String path, String mode) {
    if (mounted) {
        File file = SD.open("/ESP32AttackTool" + path, mode.c_str());
        if (file) {
            Serial.println("[INFO] File opened successfully: " + path);
            return file;
        } else {
            Serial.println("[ERROR] Failed to open file: " + path);
            return File(); // Return an empty File object if opening failed
        }
    } else {
        Serial.println("[WARN] SD Card is not mounted!");
    }
    return File(); // Return an empty File object if SD card is not mounted
}

void SDCardModules::close() {
    if (mounted) {
        SD.end();
        mounted = false;
        Serial.println("[INFO] SD Card Unmounted Successfully!");
    } else {
        Serial.println("[WARN] SD Card is not mounted!");
    }
}

bool SDCardModules::isMounted() {
    return mounted;
}

bool SDCardModules::deleteFile(String path) {
    if (mounted) {
        if(!SD.remove("/ESP32AttackTool" + path)) {
            Serial.println("[ERROR] Failed to delete file: " + path);
            return false;
        } else {
            Serial.println("[INFO] File deleted successfully: " + path);
            return true;
        }
    }
}

bool SDCardModules::isExists(String path) {
    if (mounted) {
        if (SD.exists("/ESP32AttackTool" + path)) {
            return true;
        } else {
            return false;
        }
    } else {
        Serial.println("[WARN] SD Card is not mounted!");
    }
    return false; // Default return if SD card is not mounted
}

void SDCardModules::addListFileToLinkedList(LinkedList<String> *file_names, String str_dir, String ext) {
    if (mounted) {
        File dir;
        if (str_dir == "/") {
            dir = SD.open("/ESP32AttackTool");
        } else {
            dir = SD.open("/ESP32AttackTool" + str_dir);
        }
        while (true) {
            File entry = dir.openNextFile();
            if (!entry) {
                break;
            }
            if (entry.isDirectory()) continue;

            String file_name = entry.name();
            if (ext != "") {
                if (file_name.endsWith(ext)) {
                    file_names->add(file_name);
                }
            } else {
                file_names->add(file_name);
            }
        }
    } else {
        Serial.println("[WARN] SD Card is not mounted!");
    }
}

int8_t SDCardModules::update() { // 0 for fail open file, -1 for fail begin update, -2 for fail write stream, -3 for fail end update
    if (mounted) {
        File new_version_firmware = SD.open("/ESP32AttackTool/firmware.bin");
        size_t update_size = new_version_firmware.size();
        if (update_size > 0) {
            Serial.println("[INFO] Starting firmware update. Please wait...");
            if (Update.begin(update_size)) {
                size_t written = Update.writeStream(new_version_firmware);
                if (written == update_size) {
                    Serial.println("[INFO] Firmware update successful. Rebooting...");
                    if (Update.end()) {
                        ESP.restart();
                    } else {
                        Serial.println("[ERROR] Update not finished properly!");
                        return -3; // Fail to end update
                    }
                } else {
                    Serial.println("[ERROR] Written only: " + String(written) + "/" + String(update_size));
                    Serial.println("[ERROR] Retry the update process!");
                    return -2; // Fail to write stream
                }
            } else {
                Serial.println("[ERROR] Update begin failed!");
                return -1; // Fail to begin update
            }
            new_version_firmware.close();
        } else {
            Serial.println("[ERROR] Update file is empty or does not exist.");
            return 0; // Fail to open file
        }
    } else {
        Serial.println("[WARN] SD Card is not mounted!");
    }
}
