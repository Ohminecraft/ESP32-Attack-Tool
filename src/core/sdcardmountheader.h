#pragma once 

#ifndef SDCARDMOUNTHEADER_H
#define SDCARDMOUNTHEADER_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#include <LinkedList.h>

#include "core/utilsheader.h"
#include "configs.h"

class SDCardModules {
public:
    bool mounted = false;
    void main();
    void close();
    bool deleteFile(String path);
};

#endif // SDCARDMOUNTHEADER_H