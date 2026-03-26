#include "sdcardmountheader.h"

/*
    * sdcardmount.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

SPIClass *SDCardSPI;

void SDCardModules::main() {
    SDCardSPI = &SPI;
    SDCardSPI->begin(espatsettings.spiSckPin,
                     espatsettings.spiMisoPin,
                     espatsettings.spiMosiPin);
    if (!SD.begin(espatsettings.sdcardCsPin, *SDCardSPI)) {
        Serial.println("[ERROR] SD Card Mount Failed! | Fallback to LittleFS");
        if(LittleFS.begin(true)) {
            Serial.println("[INFO] Successfully mount LittleFS");
            littlefsmounted = true;
        }
        return;
    } else mounted = true;
    if (mounted) {
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
}

File SDCardModules::getFile(String path, String mode, bool create) {
    if (mounted) {
        File file = SD.open("/ESP32AttackTool" + path, mode.c_str(), create);
        if (file) {
            //Serial.println("[INFO] File opened/created successfully: " + path);
            return file;
        } else {
            Serial.println("[ERROR] Failed to open/created file: " + path);
            return File(); // Return an empty File object if opening failed
        }
    } else {
        if (littlefsmounted) {
            File file = LittleFS.open("/" + path, mode.c_str(), create);
            if (file) {
            //Serial.println("[INFO] File opened/created successfully: " + path);
            return file;
            } else {
                Serial.println("[ERROR] Failed to open/created file: " + path);
                return File(); // Return an empty File object if opening failed
            }
        }
        //Serial.println("[WARN] SD Card is not mounted!");
    }
    return File(); // Return an empty File object if SD card is not mounted
}

void SDCardModules::close() {
    if (mounted) {
        SD.end();
        mounted = false;
        Serial.println("[INFO] SD Card Unmounted Successfully!");
    } else {
        if (littlefsmounted) {
            LittleFS.end();
            littlefsmounted = false;
            Serial.println("[INFO] LittleFS Unmounted Successfully!");
        }
    }
}
bool SDCardModules::isMounted() {
    if (mounted) return mounted;
    else if (littlefsmounted) return littlefsmounted;
    return false;
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
    } else {
        if (littlefsmounted) {
            if(!LittleFS.remove("/" + path)) {
                Serial.println("[ERROR] Failed to delete file: " + path);
                return false;
            } else {
                Serial.println("[INFO] File deleted successfully: " + path);
                return true;
            }
        }
    }
    return true;
}

bool SDCardModules::isExists(String path) {
    if (mounted) {
        if (SD.exists("/ESP32AttackTool" + path)) {
            return true;
        } else {
            return false;
        }
    } else {
        if (littlefsmounted) {
            if (LittleFS.exists("/" + path)) {
                return true;
            } else {
                return false;
            }
        }
    }
    return false; // Default return if SD card is not mounted
}

void SDCardModules::addListFileToLinkedList(LinkedList<String> *file_names, String str_dir, String ext) {
    File dir;
    if (mounted) {
        if (str_dir == "/") {
            dir = SD.open("/ESP32AttackTool");
        } else {
            dir = SD.open("/ESP32AttackTool" + str_dir);
        }
    } else if (littlefsmounted) {
        if (str_dir == "/") {
            dir = LittleFS.open("/");
        } else {
            dir = LittleFS.open("/" + str_dir);
        }
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
    return true;
}
