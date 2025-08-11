#pragma once 

#ifndef SDCARDMOUNTHEADER_H
#define SDCARDMOUNTHEADER_H

#include <Arduino.h>
#include <Update.h>
#include <SD.h>
#include <SPI.h>

#include <LinkedList.h>

#include "core/utilsheader.h"
#include "configs.h"

extern LinkedList<String> *sdcard_buffer;

class SDCardModules {
public:
    bool mounted = false;
    void main();
    void close();
    bool deleteFile(String path);
    File getFile(String path, String mode);
    void addListFileToLinkedList(LinkedList<String> *file_names, String str_dir = "/", String ext = "");
    int8_t update();
    bool isExists(String path);
};

#endif // SDCARDMOUNTHEADER_H