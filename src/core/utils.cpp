#include "utilsheader.h"
#include "configs.h"

/*
	* utils.cpp Version 2.0 (with Claude Help)
	* /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
	* Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
	* Licensed under the MIT License.
*/

String generateRandomString() {
	int len = rand() % 10 + 1; // Limit length to 1-10 characters
	String randomName = "";
	
	// Reserve memory upfront to prevent fragmentation
	randomName.reserve(len + 1);
	
	for (int i = 0; i < len; i++) {
		randomName += alfa[rand() % alfa.length()];
	}
	
	return randomName;
}

uint64_t reverse_bits(uint64_t num, uint8_t bits) {
    uint64_t res = 0;

    for (uint8_t i = 0; i < bits; ++i) {
        res <<= 1;
        res |= bitAt(num, i);
    }

    return res;
}

// Function to compute CRC-64-ECMA
uint64_t crc64_ecma(const std::vector<int> &data) {
    uint64_t crc = CRC64_ECMA_INIT;

    for (int value : data) {
        crc ^= (uint64_t)value << 56; // Use the value as the high byte
        for (int i = 0; i < 8; i++) {
            if (crc & 0x8000000000000000) {
                crc = (crc << 1) ^ CRC64_ECMA_POLY;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

char *dec2binWzerofill(uint64_t Dec, unsigned int bitLength) {
    // Allocate memory dynamically for safety
    char *bin = (char *)malloc(bitLength + 1);
    if (!bin) return NULL; // Handle allocation failure

    bin[bitLength] = '\0'; // Null-terminate string

    for (int i = bitLength - 1; i >= 0; i--) {
        bin[i] = (Dec & 1) ? '1' : '0';
        Dec >>= 1;
    }

    return bin;
}

void generateRandomString(char* buffer, size_t length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    size_t charsetSize = sizeof(charset) - 1; // Exclude null terminator

    for (size_t i = 0; i < length - 1; i++) {
        buffer[i] = charset[random(0, charsetSize)];
    }
    buffer[length - 1] = '\0'; // Null-terminate the string
}

uint32_t getHeap(uint8_t type) {
	if (type == GET_TOTAL_HEAP) return ESP.getHeapSize();
	else if (type == GET_FREE_HEAP) return ESP.getFreeHeap();
	else if (type == GET_USED_HEAP) return (ESP.getHeapSize() - ESP.getFreeHeap());
	else if (type == GET_USED_HEAP_PERCENT) {
		size_t freeHeap = ESP.getFreeHeap();
		size_t totalHeap = ESP.getHeapSize();
		if (totalHeap == 0) return 0; // Avoid division by zero
		return ((totalHeap - freeHeap) * 100) / totalHeap; // Return used heap percentage
	}
	else return 0; // Invalid type
}

bool checkLeftMemory() {
	Serial.printf("[INFO] Free heap: %d bytes\n", String(getHeap(GET_FREE_HEAP)).toInt());
	Serial.printf("[INFO] Used heap: %d bytes\n", String(getHeap(GET_USED_HEAP)).toInt());
	Serial.printf("[INFO] Used: %d%%\n", String(getHeap(GET_USED_HEAP_PERCENT)).toInt());
	
	if (getHeap(GET_FREE_HEAP) <= MEM_LOWER_LIM + 2000) {
		Serial.println("[WARN] Warning: Low memory!");
		Serial.println("[WARN] Please restart the device or free up memory.");
		return true;
	}
	else if (getHeap(GET_FREE_HEAP) <= MEM_LOWER_LIM) {
		Serial.println("[WARN] Warning: Critical low memory!");
		return false;
	}
	return true;
}

void getMAC(char *addr, uint8_t* data, uint16_t offset) {
	sprintf(addr, "%02x:%02x:%02x:%02x:%02x:%02x", data[offset+0], data[offset+1], data[offset+2], data[offset+3], data[offset+4], data[offset+5]);
}

void getMAC(uint8_t* mac, const uint8_t* data, uint16_t offset) {
  for (int i = 0; i < 6; i++)
    mac[i] = data[offset + i];
}

String hexDump(const uint8_t *buf, size_t len) {
  String out;
  out.reserve(len * 3);  // "FF " per byte (approx)

  for (size_t i = 0; i < len; i++) {
    if (buf[i] < 0x10) {
      out += '0';
    }
    out += String(buf[i], HEX);

    if (i < len - 1) {
      out += ' ';
    }
  }

  out.toUpperCase();
  return out;
}

void generateRandomMac(uint8_t* mac) {
	// Set the locally administered bit and unicast bit for the first byte
	mac[0] = 0x02; // The locally administered bit is the second least significant bit
  
	// Generate the rest of the MAC address
	for (int i = 1; i < 6; i++) {
	  mac[i] = random(0, 255);
	}
}

void stringToMac(const String& macStr, uint8_t macAddr[6]) {
    // Ensure the input string is in the format "XX:XX:XX:XX:XX:XX"
    if (macStr.length() != 17) {
        Serial.println("[ERROR] Invalid MAC address format");
        return;
    }

    // Parse the MAC address string and fill the uint8_t array
    for (int i = 0; i < 6; i++) {
        macAddr[i] = (uint8_t)strtol(macStr.substring(i * 3, i * 3 + 2).c_str(), nullptr, 16);
    }
}

String macToString(uint8_t macAddr[6]) {
	char macStr[18]; // 17 characters for "XX:XX:XX:XX:XX:XX" + 1 null terminator
	snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
	  macAddr[0], macAddr[1], macAddr[2], 
	  macAddr[3], macAddr[4], macAddr[5]);
	return String(macStr);
}

uint8_t stringToHex(const String& hex_str) {
	if (hex_str.length() != 2) {
		Serial.println("[ERROR] Invalid hex string format");
		return 0;
	}
	return (uint8_t)strtol(hex_str.c_str(), nullptr, 16);
}


	
void setBaseMacAddress(uint8_t macAddr[6]) {
	esp_err_t err = esp_base_mac_addr_set(macAddr);
	
	// Check for success or handle errors
	if (err == ESP_OK) {
		//Serial.println("MAC address set successfully");
		return;
	} else if (err == ESP_ERR_INVALID_ARG) {
		Serial.println("[ERROR] Error: Invalid MAC address argument.");
	} else {
		Serial.printf("[ERROR] Error: Failed to set MAC address. Code: %d\n", err);
	}
}

int splitStringToVector(String str, char delimiter, std::vector<String>& result) {
    result.clear();
    int count = 0;
    int pos = 0;
    
    if (str.length() == 0) {
        return 0;
    }
    
    while (pos < str.length()) {
        int nextDelim = str.indexOf(delimiter, pos);
        String element;
        
        if (nextDelim == -1) {
            // Last element
            element = str.substring(pos);
        } else {
            // Regular element  
            element = str.substring(pos, nextDelim);
        }
        
        element.trim();
        if (element.length() > 0) {
            result.push_back(element);
            count++;
            //Serial.println("DEBUG: Split element[" + String(count-1) + "] = '" + element + "'");
        }
        
        if (nextDelim == -1) break;
        pos = nextDelim + 1;
    }
    
    return count;
}

String hexStrToBinStr(const String &hexStr) {
    String binStr = "";
    String hexByte = "";

    // Variables for decimal value
    int value;

    for (int i = 0; i < hexStr.length(); i++) {
        char c = hexStr.charAt(i);

        // Check if the character is a hexadecimal digit
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
            hexByte += c;
            if (hexByte.length() == 2) {
                // Convert the hexadecimal pair to a decimal value
                value = strtol(hexByte.c_str(), NULL, 16);

                // Convert the decimal value to binary and add to the binary string
                for (int j = 7; j >= 0; j--) { binStr += (value & (1 << j)) ? '1' : '0'; }
                // binStr += ' ';

                // Clear the hexByte string for the next byte
                hexByte = "";
            }
        }
    }

    // Remove the extra trailing space, if any
    if (binStr.length() > 0 && binStr.charAt(binStr.length() - 1) == ' ') {
        binStr.remove(binStr.length() - 1);
    }

    return binStr;
}

void decimalToHexString(uint64_t decimal, char *output) {
    char hexDigits[] = "0123456789ABCDEF";
    char temp[65];
    int index = 15;

    // Initialize tem string with zeros
    for (int i = 0; i < 64; i++) { temp[i] = '0'; }
    temp[65] = '\0';

    // Convert decimal to hexadecimal
    while (decimal > 0) {
        temp[index--] = hexDigits[decimal % 16];
        decimal /= 16;
    }

    // Format string with spaces
    int outputIndex = 0;
    for (int i = 0; i < 16; i++) {
        output[outputIndex++] = temp[i];
        if ((i % 2) == 1 && i != 15) { output[outputIndex++] = ' '; }
    }
    output[outputIndex] = '\0';
}

uint32_t hexStringToDecimal(const char *hexString) {
    uint32_t decimal = 0;
    int length = strlen(hexString);

    for (int i = 0; i < length; i += 3) {
        decimal <<= 8; // Shift left to accommodate next byte

        // Converts two characters hex to a single byte
        uint8_t highNibble = hexCharToDecimal(hexString[i]);
        uint8_t lowNibble = hexCharToDecimal(hexString[i + 1]);
        decimal |= (highNibble << 4) | lowNibble;
    }

    return decimal;
}

String uint32ToString(uint32_t value) {
    char buffer[12] = {0}; // 8 hex digits + 3 spaces + 1 null terminator
    snprintf(
        buffer,
        sizeof(buffer),
        "%02X %02X %02X %02X",
        value & 0xFF,
        (value >> 8) & 0xFF,
        (value >> 16) & 0xFF,
        (value >> 24) & 0xFF
    );
    return String(buffer);
}

String uint32ToStringInverted(uint32_t value) {
    char buffer[12] = {0}; // 8 hex digits + 3 spaces + 1 null terminator
    snprintf(
        buffer,
        sizeof(buffer),
        "%02X %02X %02X %02X",
        (value >> 24) & 0xFF,
        (value >> 16) & 0xFF,
        (value >> 8) & 0xFF,
        value & 0xFF
    );
    return String(buffer);
}

uint32_t swap32(uint32_t value) {
    return ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) | ((value & 0x00FF0000) >> 8) |
           ((value & 0xFF000000) >> 24);
}

uint8_t hexCharToDecimal(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return 0;
}

bool getNextLine(const String &src, int &index, String &line) {
    if (index >= src.length()) return false;

    int next = src.indexOf('\n', index);
    if (next == -1) {
        line = src.substring(index);
        index = src.length();
    } else {
        line = src.substring(index, next);
        index = next + 1;
    }

    if (line.endsWith("\r")) {
        line.remove(line.length() - 1);
    }
    return true;
}

volatile bool nextPress = false;
volatile bool prevPress = false;
volatile bool selPress = false;
volatile bool anykeyPress = false;
bool ble_initialized = false; // BLE Initialized Flag
bool wifi_initialized = false; // WiFi Initialized Flag
bool wifi_connected = false;
bool first_scan = true;
bool rtl8720dn_ready = false; // RTL8720DN Ready Flag
bool low_memory_warning = false; // Low Memory Warning Flag

// Encoder Object
RotaryEncoder *encoder = nullptr;

// Selection list
LinkedList<String> *selection_list;

IRAM_ATTR void checkPosition() {
    encoder->tick(); // just call tick() to check the state.
}

TaskHandle_t xHandle;

SPIClass *SDCardSPI;