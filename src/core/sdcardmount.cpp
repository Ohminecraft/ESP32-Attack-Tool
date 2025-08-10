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

bool SDCardModules::deleteFile(String path) {
    if (mounted) {
         return         
         SD.remove(path);
    }
}
