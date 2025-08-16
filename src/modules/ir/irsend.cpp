#include "irsend_header.h"

/*
    * irsend.cpp
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

IRSendModules::IRSendModules() : irsend(espatsettings.irTxPin)
{
    // Constructor for IR send module
    // Initialize IR send object with default parameters
}

extern const IrCode* const NApowerCodes[];
extern const IrCode* const EUpowerCodes[];
extern uint8_t num_NAcodes, num_EUcodes;

LinkedList<IRCode>* ir_codes;

uint8_t bitsleft_r = 0;
uint8_t bits_r=0;
uint8_t code_ptr;
volatile const IrCode * beGoneCode;
bool ending_early = false;

void IRSendModules::main() {
    // Initialize IR send module
    irsend.begin();
    ir_codes = new LinkedList<IRCode>();
    pinMode(espatsettings.irTxPin, OUTPUT); // Set the IR pin as output
    Serial.println("[INFO] IR Send Module Initialized");
}

void delay_ten_us(uint16_t us) {
    uint8_t timer;
    while (us != 0) {
        for (timer = 0; timer <= 25; timer++) {
            NOPP;
            NOPP;
        }
        NOPP;
        us--;
    }
}

uint8_t read_bits(uint8_t count) {
    uint8_t i;
    uint8_t tmp = 0;
    for (i = 0; i < count; i++) {
        if (bitsleft_r == 0) {
            bits_r = beGoneCode->codes[code_ptr++];
            bitsleft_r = 8;
        }
        bitsleft_r--;
        tmp |= (((bits_r >> (bitsleft_r)) & 1) << (count - 1 - i));
    }
    return tmp;
}

uint16_t ontime, offtime;
uint8_t i, num_codes;
TvBeGoneRegion begoneregion;

// https://github.com/pr3y/Bruce/blob/main/src/modules/ir/TV-B-Gone.cpp
void IRSendModules::startTVBGone() {
    if (begoneregion == NA) size_num_codes = num_NAcodes;
    else size_num_codes = num_EUcodes;

    uint16_t rawData[300];

    for (int i = 0; i < size_num_codes; i++) {
        if (check(selPress)) {
            ending_early = true;
            break; // Stop sending codes if the user presses the button
        }
        if (begoneregion == NA) {
            beGoneCode = NApowerCodes[i];
        } else {
            beGoneCode = EUpowerCodes[i];
        }

        const uint8_t freq = beGoneCode->timer_val;
        const uint8_t numpairs = beGoneCode->numpairs;
        const uint8_t bitcompression = beGoneCode->bitcompression;

        code_ptr = 0;
        for (uint8_t x = 0; x < numpairs; x++) {
            if (selPress) {
                break; // Stop sending codes if the user presses the button
            }
            uint16_t ti = (read_bits(bitcompression)) * 2;
            offtime = beGoneCode->times[ti];    // read word 1 - ontime
            ontime = beGoneCode->times[ti + 1]; // read word 2 - offtime
            rawData[x * 2] = offtime * 10;
            rawData[(x * 2) + 1] = ontime * 10;
        }
        irsend.sendRaw(rawData, (numpairs * 2), freq);
        yield(); // Allow other tasks to run
        begone_code_sended += 1;
        bitsleft_r = 0;
        digitalWrite(espatsettings.irTxPin, LOW);
        delay_ten_us(3000);
        digitalWrite(espatsettings.irTxPin, HIGH);
        delay_ten_us(20500);
    }
    if (ending_early) {
        delay_ten_us(50000); //500ms delay 
        for (int i = 0; i < 4; i++) {
            digitalWrite(espatsettings.irTxPin, LOW);
            delay_ten_us(3000);
            digitalWrite(espatsettings.irTxPin, HIGH);
        }
        digitalWrite(espatsettings.irTxPin, LOW);
        delay_ten_us(65535);
        delay_ten_us(65535);
        ending_early = false; // Reset the flag
        Serial.println("[INFO] TV-B-Gone Stopped by user");
    } else {
        for (int i = 0; i < 8; i++) {
            digitalWrite(espatsettings.irTxPin, LOW);
            delay_ten_us(3000);
            digitalWrite(espatsettings.irTxPin, HIGH);
        }
        digitalWrite(espatsettings.irTxPin, LOW);
        Serial.println("[INFO] TV-B-Gone Codes Sended: " + String(begone_code_sended));
        if (begone_code_sended == num_NAcodes || begone_code_sended == num_EUcodes) {
            Serial.println("[INFO] TV-B-Gone All Code Sended Successfully");
        } else {
            Serial.println("[ERROR] TV-B-Gone Finished with Failure");
        }
    }
}

void IRSendModules::sendIRTx(String filename) {
    int total_codes = 0;
    String line;

    File databaseFile = sdcard.getFile(filename, FILE_READ);

    pinMode(espatsettings.irTxPin, OUTPUT);

    if (!databaseFile) {
        Serial.println("[ERROR] Failed to open database file: " + filename);
        return;
    }

    Serial.println("[INFO] Opened database file: " + filename);

    uint16_t frequency = 0;
    String rawData = "";
    String protocol = "";
    String address = "";
    String command = "";
    String value = "";
    uint8_t bits = 32;
    
    while (databaseFile.available()) {
        line = databaseFile.readStringUntil('\n');
        if (line.endsWith("\r")) line.remove(line.length() - 1);

        if (line.startsWith("type:")) {
            String type = line.substring(5);
            type.trim();
            if (type == "raw") {
                while (databaseFile.available()) {
                    line = databaseFile.readStringUntil('\n');
                    if (line.endsWith("\r")) line.remove(line.length() - 1);

                    if (line.startsWith("frequency:")) {
                        line = line.substring(10);
                        line.trim();
                        frequency = line.toInt();
                    } else if (line.startsWith("data:")) {
                        rawData = line.substring(5);
                        rawData.trim();
                    } else if ((frequency != 0 && rawData != "") || line.startsWith("#")) {
                        IRCode code;
                        code.type = "raw";
                        code.data = rawData;
                        code.frequency = frequency;
                        sendIRCommand(&code);

                        rawData = "";
                        frequency = 0;
                        type = "";
                        line = "";
                        break;
                    }
                }
            } else if (type == "parsed") {
                while (databaseFile.available()) {
                    line = databaseFile.readStringUntil('\n');
                    if (line.endsWith("\r")) line.remove(line.length() - 1);

                    if (line.startsWith("protocol:")) {
                        protocol = line.substring(9);
                        protocol.trim();
                    } else if (line.startsWith("address:")) {
                        address = line.substring(8);
                        address.trim();
                    } else if (line.startsWith("command:")) {
                        command = line.substring(8);
                        command.trim();
                    } else if (line.startsWith("value:") || line.startsWith("state:")) {
                        value = line.substring(6);
                        value.trim();
                    } else if (line.startsWith("bits:")) {
                        bits = line.substring(strlen("bits:")).toInt();
                    } else if (line.indexOf("#") != -1) { // TODO: also detect EOF
                        IRCode code(protocol, address, command, value, bits);
                        sendIRCommand(&code);

                        protocol = "";
                        address = "";
                        command = "";
                        value = "";
                        bits = 32;
                        type = "";
                        line = "";
                        break;
                    }
                }
            }
        }
    } // end while file has lines to process
    databaseFile.close();
    digitalWrite(espatsettings.irTxPin, LOW);
    return;
}

void IRSendModules::getCodesToSendIR(String filename) {
    String line;
    String txt;
    IRCode codes;
    bool codeReady = false;

    File databaseFile = sdcard.getFile(filename, FILE_READ);
    if (!databaseFile) {
        Serial.println("[ERROR] Failed to open database file: " + filename);
        return;
    }

    Serial.println("[INFO] Opened database file: " + filename);

    while (databaseFile.available() && ir_codes->size() < 100) {
        line = databaseFile.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        txt = line.substring(line.indexOf(":") + 1);
        txt.trim();

        if (line.startsWith("name:")) {
            if (codeReady) {
                ir_codes->add(codes);
                codes = IRCode();
            }
            codes.name = txt;
            codes.filepath = txt + " " + filename.substring(1 + filename.lastIndexOf("/"));
            codeReady = true;
        }
        else if (line.startsWith("type:")) codes.type = txt;
        else if (line.startsWith("protocol:")) codes.protocol = txt;
        else if (line.startsWith("address:")) codes.address = txt;
        else if (line.startsWith("frequency:")) codes.frequency = txt.toInt();
        else if (line.startsWith("bits:")) codes.bits = txt.toInt();
        else if (line.startsWith("command:")) codes.command = txt;
        else if (line.startsWith("data:") || line.startsWith("value:") || line.startsWith("state:"))
            codes.data = txt;
    }

    if (codeReady) {
        ir_codes->add(codes);
    }

    databaseFile.close();
}

void IRSendModules::sendIRCommand(IRCode *code) {
    // https://developer.flipper.net/flipperzero/doxygen/infrared_file_format.html
    if (code->type.equalsIgnoreCase("raw")) sendRawCommand(code->frequency, code->data);
    else if (code->protocol.equalsIgnoreCase("NEC")) sendNECCommand(code->address, code->command);
    else if (code->protocol.equalsIgnoreCase("NECext")) sendNECextCommand(code->address, code->command);
    else if (code->protocol.equalsIgnoreCase("RC5") || code->protocol.equalsIgnoreCase("RC5X"))
        sendRC5Command(code->address, code->command);
    else if (code->protocol.equalsIgnoreCase("RC6")) sendRC6Command(code->address, code->command);
    else if (code->protocol.equalsIgnoreCase("Samsung32")) sendSamsungCommand(code->address, code->command);
    else if (code->protocol.equalsIgnoreCase("SIRC")) sendSonyCommand(code->address, code->command, 12);
    else if (code->protocol.equalsIgnoreCase("SIRC15")) sendSonyCommand(code->address, code->command, 15);
    else if (code->protocol.equalsIgnoreCase("SIRC20")) sendSonyCommand(code->address, code->command, 20);
    else if (code->protocol.equalsIgnoreCase("Kaseikyo")) sendKaseikyoCommand(code->address, code->command);
    // Others protocols of IRRemoteESP8266, not related to Flipper Zero IR File Format
    else if (code->protocol != "" && code->data != "" &&
             strToDecodeType(code->protocol.c_str()) != decode_type_t::UNKNOWN)
        sendDecodedCommand(code->protocol, code->data, code->bits);
}

void IRSendModules::sendNECCommand(String address, String command) {
    IRsend irsend(espatsettings.irTxPin); // Set the GPIO to be used to sending the message.
    irsend.begin();
    uint16_t addressValue = strtoul(address.substring(0, 2).c_str(), nullptr, 16);
    uint16_t commandValue = strtoul(command.substring(0, 2).c_str(), nullptr, 16);
    uint64_t data = irsend.encodeNEC(addressValue, commandValue);
    irsend.sendNEC(data, 32);

    if (espatsettings.irRepeat > 0) {
        for (uint8_t i = 1; i <= espatsettings.irRepeat; i++) { irsend.sendNEC(data, 32); }
    }

    Serial.println(
        "[INFO] Ir Sent NEC Command" +
        (espatsettings.irRepeat > 0 ? " (1 initial + " + String(espatsettings.irRepeat) + " repeats)" : "")
    );

    digitalWrite(espatsettings.irTxPin, LOW);
}

void IRSendModules::sendNECextCommand(String address, String command) {
    IRsend irsend(espatsettings.irTxPin); // Set the GPIO to be used to sending the message.
    irsend.begin();

    int first_zero_byte_pos = address.indexOf("00", 2);
    if (first_zero_byte_pos != -1) address = address.substring(0, first_zero_byte_pos);
    first_zero_byte_pos = command.indexOf("00", 2);
    if (first_zero_byte_pos != -1) command = command.substring(0, first_zero_byte_pos);

    address.replace(" ", "");
    command.replace(" ", "");

    uint16_t addressValue = strtoul(address.c_str(), nullptr, 16);
    uint16_t commandValue = strtoul(command.c_str(), nullptr, 16);

    // Invert Endianness
    uint16_t newAddress = (addressValue >> 8) | (addressValue << 8);
    uint16_t newCommand = (commandValue >> 8) | (commandValue << 8);

    // NEC protocol bit order is LSB first
    uint16_t lsbAddress = reverseBits(newAddress, 16);
    uint16_t lsbCommand = reverseBits(newCommand, 16);

    uint32_t data = ((uint32_t)lsbAddress << 16) | lsbCommand;
    irsend.sendNEC(data, 32); // Sends MSB first

    if (espatsettings.irRepeat > 0) {
        for (uint8_t i = 1; i <= espatsettings.irRepeat; i++) { irsend.sendNEC(data, 32); }
    }

    Serial.println(
        "[INFO] Ir Sent NECext Command" +
        (espatsettings.irRepeat > 0 ? " (1 initial + " + String(espatsettings.irRepeat) + " repeats)" : "")
    );
    digitalWrite(espatsettings.irTxPin, LOW);
}

void IRSendModules::sendRC5Command(String address, String command) {
    IRsend irsend(espatsettings.irTxPin, true); // Set the GPIO to be used to sending the message.
    irsend.begin();

    uint8_t addressValue = strtoul(address.substring(0, 2).c_str(), nullptr, 16);
    uint8_t commandValue = strtoul(command.substring(0, 2).c_str(), nullptr, 16);
    uint16_t data = irsend.encodeRC5(addressValue, commandValue);
    irsend.sendRC5(data, 13);

    if (espatsettings.irRepeat > 0) {
        for (uint8_t i = 1; i <= espatsettings.irRepeat; i++) { irsend.sendRC5(data, 13); }
    }
    Serial.println(
        "[INFO] Ir Sent RC5 Command" +
        (espatsettings.irRepeat > 0 ? " (1 initial + " + String(espatsettings.irRepeat) + " repeats)" : "")
    );
    digitalWrite(espatsettings.irTxPin, LOW);
}

void IRSendModules::sendRC6Command(String address, String command) {
    IRsend irsend(espatsettings.irTxPin, true); // Set the GPIO to be used to sending the message.
    irsend.begin();

    address.replace(" ", "");
    command.replace(" ", "");
    uint32_t addressValue = strtoul(address.substring(0, 2).c_str(), nullptr, 16);
    uint32_t commandValue = strtoul(command.substring(0, 2).c_str(), nullptr, 16);
    uint64_t data = irsend.encodeRC6(addressValue, commandValue);

    irsend.sendRC6(data, 20);

    if (espatsettings.irRepeat > 0) {
        for (uint8_t i = 1; i <= espatsettings.irRepeat; i++) { irsend.sendRC6(data, 20); }
    }

    Serial.println(
        "[INFO] Ir Sent RC6 Command" +
        (espatsettings.irRepeat > 0 ? " (1 initial + " + String(espatsettings.irRepeat) + " repeats)" : "")
    );
    digitalWrite(espatsettings.irTxPin, LOW);
}

void IRSendModules::sendSamsungCommand(String address, String command) {
    IRsend irsend(espatsettings.irTxPin); // Set the GPIO to be used to sending the message.
    irsend.begin();;
    uint8_t addressValue = strtoul(address.substring(0, 2).c_str(), nullptr, 16);
    uint8_t commandValue = strtoul(command.substring(0, 2).c_str(), nullptr, 16);
    uint64_t data = irsend.encodeSAMSUNG(addressValue, commandValue);

    irsend.sendSAMSUNG(data, 32);

    if (espatsettings.irRepeat > 0) {
        for (uint8_t i = 1; i <= espatsettings.irRepeat; i++) { irsend.sendSAMSUNG(data, 32); }
    }

    Serial.println(
        "[INFO] Ir Sent Samsung Command" +
        (espatsettings.irRepeat > 0 ? " (1 initial + " + String(espatsettings.irRepeat) + " repeats)" : "")
    );
    digitalWrite(espatsettings.irTxPin, LOW);
}

void IRSendModules::sendSonyCommand(String address, String command, uint8_t nbits) {
    IRsend irsend(espatsettings.irTxPin); // Set the GPIO to be used to sending the message.
    irsend.begin();

    address.replace(" ", "");
    command.replace(" ", "");

    uint32_t addressValue = strtoul(address.c_str(), nullptr, 16);
    uint32_t commandValue = strtoul(command.c_str(), nullptr, 16);

    uint16_t swappedAddr = static_cast<uint16_t>(swap32(addressValue));
    uint8_t swappedCmd = static_cast<uint8_t>(swap32(commandValue));

    uint32_t data;

    if (nbits == 12) {
        // SIRC (12 bits)
        data = ((swappedAddr & 0x1F) << 7) | (swappedCmd & 0x7F);
    } else if (nbits == 15) {
        // SIRC15 (15 bits)
        data = ((swappedAddr & 0xFF) << 7) | (swappedCmd & 0x7F);
    } else if (nbits == 20) {
        // SIRC20 (20 bits)
        data = ((swappedAddr & 0x1FFF) << 7) | (swappedCmd & 0x7F);
    } else {
        Serial.println("Invalid Sony (SIRC) protocol bit size.");
        return;
    }

    // SIRC protocol bit order is LSB First
    data = reverseBits(data, nbits);

    // 1 initial + 2 repeat
    irsend.sendSony(data, nbits, 2); // Sends MSB First

    if (espatsettings.irRepeat > 0) {
        for (uint8_t i = 1; i <= espatsettings.irRepeat; i++) { irsend.sendSony(data, nbits, 2); }
    }

    Serial.println(
        "[INFO] Ir Sent Sony Command" +
        (espatsettings.irRepeat > 0 ? " (1 initial + " + String(espatsettings.irRepeat) + " repeats)" : "")
    );
    digitalWrite(espatsettings.irTxPin, LOW);
}

void IRSendModules::sendKaseikyoCommand(String address, String command) {
    IRsend irsend(espatsettings.irTxPin); // Set the GPIO to be used to sending the message.
    irsend.begin();

    address.replace(" ", "");
    command.replace(" ", "");

    uint32_t addressValue = strtoul(address.c_str(), nullptr, 16);
    uint32_t commandValue = strtoul(command.c_str(), nullptr, 16);

    uint32_t newAddress = swap32(addressValue);
    uint16_t newCommand = static_cast<uint16_t>(swap32(commandValue));

    uint8_t id = (newAddress >> 24) & 0xFF;
    uint16_t vendor_id = (newAddress >> 8) & 0xFFFF;
    uint8_t genre1 = (newAddress >> 4) & 0x0F;
    uint8_t genre2 = newAddress & 0x0F;

    uint16_t data = newCommand & 0x3FF;

    byte bytes[6];
    bytes[0] = vendor_id & 0xFF;
    bytes[1] = (vendor_id >> 8) & 0xFF;

    uint8_t vendor_parity = bytes[0] ^ bytes[1];
    vendor_parity = (vendor_parity & 0xF) ^ (vendor_parity >> 4);

    bytes[2] = (genre1 << 4) | (vendor_parity & 0x0F);
    bytes[3] = ((data & 0x0F) << 4) | genre2;
    bytes[4] = ((id & 0x03) << 6) | ((data >> 4) & 0x3F);

    bytes[5] = bytes[2] ^ bytes[3] ^ bytes[4];

    uint64_t lsb_data = 0;
    for (int i = 0; i < 6; i++) { lsb_data |= (uint64_t)bytes[i] << (8 * i); }

    // LSB First --> MSB First
    uint64_t msb_data = reverseBits(lsb_data, 48);

    irsend.sendPanasonic64(msb_data, 48); // Sends MSB First

    if (espatsettings.irRepeat > 0) {
        for (uint8_t i = 1; i <= espatsettings.irRepeat; i++) { irsend.sendPanasonic64(msb_data, 48); }
    }

    Serial.println(
        "[INFO] Ir Sent Kaseikyo Command" +
        (espatsettings.irRepeat > 0 ? " (1 initial + " + String(espatsettings.irRepeat) + " repeats)" : "")
    );
    digitalWrite(espatsettings.irTxPin, LOW);
}

bool IRSendModules::sendDecodedCommand(String protocol, String value, uint8_t bits) {
    decode_type_t type = strToDecodeType(protocol.c_str());
    if (type == decode_type_t::UNKNOWN) return false;

    IRsend irsend(espatsettings.irTxPin); // Set the GPIO to be used to sending the message.
    irsend.begin();
    bool success = false;

    if (hasACState(type)) {
        // need to send the state (still passed from value)
        uint8_t state[bits / 8] = {0};
        uint16_t state_pos = 0;
        for (uint16_t i = 0; i < value.length(); i += 3) {
            // parse  value -> state
            uint8_t highNibble = hexCharToDecimal(value[i]);
            uint8_t lowNibble = hexCharToDecimal(value[i + 1]);
            state[state_pos] = (highNibble << 4) | lowNibble;
            state_pos++;
        }
        // success = irsend.send(type, state, bits / 8);
        success = irsend.send(type, state, state_pos); // safer

        if (espatsettings.irRepeat > 0) {
            for (uint8_t i = 1; i <= espatsettings.irRepeat; i++) { irsend.send(type, state, state_pos); }
        }

    } else {
        value.replace(" ", "");
        uint64_t value_int = strtoull(value.c_str(), nullptr, 16);

        success =
            irsend.send(type, value_int, bits); // bool send(const decode_type_t type, const uint64_t data,
                                                // const uint16_t nbits, const uint16_t repeat = kNoRepeat);

        if (espatsettings.irRepeat > 0) {
            for (uint8_t i = 1; i <= espatsettings.irRepeat; i++) { irsend.send(type, value_int, bits); }
        }
    }

    delay(20);
    Serial.println(
        "[INFO] Ir Sent Decoded Command" +
        (espatsettings.irRepeat > 0 ? " (1 initial + " + String(espatsettings.irRepeat) + " repeats)" : "")
    );
    digitalWrite(espatsettings.irTxPin, LOW);
    return success;
}

void IRSendModules::sendRawCommand(uint16_t frequency, String rawData) {
    IRsend irsend(espatsettings.irTxPin); // Set the GPIO to be used to sending the message.
    irsend.begin();

    uint16_t dataBufferSize = 1;
    for (int i = 0; i < rawData.length(); i++) {
        if (rawData[i] == ' ') dataBufferSize += 1;
    }
    uint16_t *dataBuffer = (uint16_t *)malloc((dataBufferSize) * sizeof(uint16_t));

    uint16_t count = 0;
    // Parse raw data string
    while (rawData.length() > 0 && count < dataBufferSize) {
        int delimiterIndex = rawData.indexOf(' ');
        if (delimiterIndex == -1) { delimiterIndex = rawData.length(); }
        String dataChunk = rawData.substring(0, delimiterIndex);
        rawData.remove(0, delimiterIndex + 1);
        dataBuffer[count++] = (dataChunk.toInt());
    }

    Serial.println("[INFO] Parsing raw data complete.");
    // Serial.println(count);
    // Serial.println(dataBuffer[count-1]);
    // Serial.println(dataBuffer[0]);

    // Send raw command
    irsend.sendRaw(dataBuffer, count, frequency);

    if (espatsettings.irRepeat > 0) {
        for (uint8_t i = 1; i <= espatsettings.irRepeat; i++) {
            irsend.sendRaw(dataBuffer, count, frequency);
        }
    }

    free(dataBuffer);

    Serial.println(
        "[INFO] Ir Sent Raw Command" +
        (espatsettings.irRepeat > 0 ? " (1 initial + " + String(espatsettings.irRepeat) + " repeats)" : "")
    );
    digitalWrite(espatsettings.irTxPin, LOW);
}
