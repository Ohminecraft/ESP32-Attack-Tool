#include "irread_header.h"

/*
    * irread.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

bool irrecvRedraw = false;

#define IR_FREQUENCY 38000
#define DUTY_CYCLE 0.330000

void IRReadModules::main(bool startup) {
    irrecv.enableIRIn(); // Start the IR receiver
    pinMode(espatsettings.irRxPin, INPUT); // Set the IR RX pin as input
    if (startup) irrecv.disableIRIn();
    _read = false;
    Serial.println("[INFO] IR Read Module Initialized");
}

void IRReadModules::shutdown() {
    irrecv.disableIRIn(); // Stop the IR receiver
    Serial.println("[INFO] IR Read Module Shutdown");
}

void IRReadModules::begin() {
    display_buffer->clear();
    display_buffer->add("Waiting for IR");
    display_buffer->add("signal...");
    display_buffer->add("Press SELECT to exit");
}

String IRReadModules::parse_raw_signal() {
    rawcode = resultToRawArray(&results);
    raw_data_len = getCorrectedRawLength(&results);

    String signal_code = "";

    for (uint16_t i = 0; i < raw_data_len; i++) { signal_code += String(rawcode[i]) + " "; }

    delete[] rawcode;
    rawcode = nullptr;
    signal_code.trim();

    return signal_code;
}

String IRReadModules::parse_state_signal() {
    String r = "";
    uint16_t state_len = (results.bits) / 8;
    for (uint16_t i = 0; i < state_len; i++) {
        // r += uint64ToString(results.state[i], 16) + " ";
        r += ((results.state[i] < 0x10) ? "0" : ""); // adds 0 padding if necessary
        r += String(results.state[i], HEX) + " ";
    }
    r.toUpperCase();
    return r;
}

void IRReadModules::readSignal() {
    if (!_read && irrecv.decode(&results)) {
        _read = true;
        raw_data = true; // Always switch to RAW data mode

        String irraw = parse_raw_signal();

        display_buffer->add("IR Signal Detected!");
        display_buffer->add(irraw.substring(0, 21) + (irraw.length() > 21 ? "..." : ""));
        if (_read) {
            display_buffer->add("Press RIGHT to save");
            display_buffer->add("Press LEFT to discard");
        }
        irrecvRedraw = true;

        //irrecv.resume(); // Resume the IR receiver for the next signal
    }
}

void IRReadModules::discard_code() {
    if (_read) {
        irrecv.resume(); // Resume the IR receiver
        _read = false;
        raw_data = false;
        begin();
        irrecvRedraw = true;
    }
}

void IRReadModules::save_code() {
    if (_read) {
        code_content = "";
        display_buffer->clear();
        processSignalToContent();
        if (sdcard.isMounted()) {
            if (!saveintoSD()) {
                display.clearScreen();
                display.displayStringwithCoordinates("Error saving IR code", 0, 12);
                display.displayStringwithCoordinates("to SD card!", 0, 24, true);
                vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for 1 second
            } else {
                display.clearScreen();
                display.displayStringwithCoordinates("IR code saved to SD", 0, 12);
                display.displayStringwithCoordinates("card successfully!", 0, 24);
                display.displayStringwithCoordinates("Name: " + fileName, 0, 36, true);
                vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for 1 second
            }
        } else {
            display.clearScreen();
            display.displayStringwithCoordinates("SD Card not mounted", 0, 12, true);
            vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for 1 second
        }
        discard_code();
    }
    
}

void IRReadModules::processSignalToContent() {
    
    if (raw_data) {
        code_content += "type: raw\n";
        code_content += "frequency: " + String(IR_FREQUENCY) + "\n";
        code_content += "duty_cycle: " + String(DUTY_CYCLE) + "\n";
        code_content += "data: " + parse_raw_signal() + "\n";
    } else {
        // parsed signal  https://github.com/jamisonderek/flipper-zero-tutorials/wiki/Infrared
        code_content += "type: parsed\n";
        switch (results.decode_type) {
            case decode_type_t::RC5: {
                if (results.command > 0x3F) code_content += "protocol: RC5X\n";
                else code_content += "protocol: RC5\n";
                break;
            }
            case decode_type_t::RC6: {
                code_content += "protocol: RC6\n";
                break;
            }
            case decode_type_t::SAMSUNG: {
                code_content += "protocol: Samsung32\n";
                break;
            }
            case decode_type_t::SONY: {
                // check address and command ranges to find the exact protocol
                if (results.address > 0xFF) code_content += "protocol: SIRC20\n";
                else if (results.address > 0x1F) code_content += "protocol: SIRC15\n";
                else code_content += "protocol: SIRC\n";
                break;
            }
            case decode_type_t::NEC: {
                // check address and command ranges to find the exact protocol
                if (results.address > 0xFFFF) code_content += "protocol: NEC42ext\n";
                else if (results.address > 0xFF1F) code_content += "protocol: NECext\n";
                else if (results.address > 0xFF) code_content += "protocol: NEC42\n";
                else code_content += "protocol: NEC\n";
                break;
            }
            case decode_type_t::UNKNOWN: {
                Serial.println("[WARN] unknown protocol, try raw mode");
                return;
            }
            default: {
                code_content += "protocol: " + typeToString(results.decode_type, results.repeat) + "\n";
                break;
            }
        }

        code_content += "address: " + uint32ToString(results.address) + "\n";
        code_content += "command: " + uint32ToString(results.command) + "\n";

        // extra fields not supported on flipper
        code_content += "bits: " + String(results.bits) + "\n";
        if (hasACState(results.decode_type)) code_content += "state: " + parse_state_signal() + "\n";
        else if (results.bits > 32)
            code_content += "value: " + uint32ToString(results.value) + " " +
                                uint32ToString(results.value >> 32) + "\n"; // MEMO: from uint64_t
        else code_content += "value: " + uint32ToStringInverted(results.value) + "\n";
    }
    code_content += "#\n";
}

bool IRReadModules::saveintoSD() {
    String file_name = "Code_";
    int i = 1;

    while(sdcard.isExists("/" + file_name + String(i) + ".ir")) i++;
    file_name += String(i) ;
    file_name += ".ir";

    fileName = file_name; // Save the file name for later use

    File file = sdcard.getFile("/" + file_name, "w");

    if (!file) { return false; }

    file.print("Filetype: ESP32 Attack Tool - Ir File\n#\n#\n");
    file.print(code_content);
    file.close();
    return true;
}