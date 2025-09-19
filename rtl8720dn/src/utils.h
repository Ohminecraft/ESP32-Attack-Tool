#pragma once 

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <vector>

bool stringToMac(const String& macStr, uint8_t macAddr[6]);
String macToString(uint8_t macAddr[6]);
int splitStringToVector(String str, char delimiter, std::vector<String>& result);

#endif