#include "communicationheader.h"

RTL8720DNCommunication::RTL8720DNCommunication() {
    this->RTLSerial = new HardwareSerial(1);
    this->responseQueue = xQueueCreate(10, sizeof(String*));
    this->readTaskHandle = NULL;
}

RTL8720DNCommunication::~RTL8720DNCommunication() {
    stopReading();
    if (responseQueue) {
        vQueueDelete(responseQueue);
    }
    if (RTLSerial) {
        delete RTLSerial;
    }
}


/*  Arguments:
        -am <MAC_ADDRESS> : Access Point MAC Address
        -sm <MAC_ADDRESS> : Station MAC Address 
        -s <SSID> : SSID Name
        -c <CHANNEL> : Channel Number (1-14<2.4G>, 36-165<5G>)
    Available command:
        RTL_READY // Check if RTL8720DN is ready
        RTL_START_AP_SCAN // Start Access Point Scan
        RTL_START_AP_STA_SCAN // Start Station Scan

        RTL_DEAUTH -am <MAC_ADDRESS>  -c <CHANNEL> // Deauthenticate a Station
        RTL_DEAUTH_STA -sm <STA_MAC_ADDRESS> -am <AP_MAC_ADDRESS> -c <CHANNEL> // Deauthenticate a Station from an Access Point
        RTL_START_DEAUTH_FLOOD // Start Deauthentication Flood
        RTL_START_AUTH_ATTACK -s <SSID> -c <CHANNEL> // Start Probe Attack with specified SSID

        RTL_STOP_SCAN // Stop any Scanning/Attacking

        // Work in Progress
        RTL_START_BEACON_SNIFFER // Start Beacon Sniffer
        RTL_START_PROBE_REQ_SNIFFER // Start Probe Request Sniffer
        RTL_START_EAPOL_SNIFFER // Start EAPOL Sniffer
        RTL_START_EAPOL_DEAUTH_SNIFFER // Start EAPOL Sniffer with Deauthentication
        RTL_START_DEAUTH_SNIFFER // Start Deauthentication Sniffer
        ///////////////////////////////////////
*/

void RTL8720DNCommunication::begin() {
    this->RTLSerial->begin(RTL_SERIAL_BAUD, SERIAL_8N1, SERIAL_RX_PIN, SERIAL_TX_PIN);
    
    xTaskCreate(
        readResponseTask,     // Task function
        "RTL_Read_Task",      // Task name
        4096,                 // Stack size
        this,                 // Parameter (this pointer)
        1,                    // Priority
        &readTaskHandle       // Task handle
    );
}

// Static task function để đọc serial liên tục
void RTL8720DNCommunication::readResponseTask(void* parameter) {
    RTL8720DNCommunication* comm = static_cast<RTL8720DNCommunication*>(parameter);
    String* responsePtr;
    
    while (true) {
        if (comm->RTLSerial->available()) {
            String receivedData = comm->RTLSerial->readStringUntil('\n');
            receivedData.trim();
            
            if (receivedData.length() > 0) {
                responsePtr = new String(receivedData);
                
                if (xQueueSend(comm->responseQueue, &responsePtr, 0) != pdTRUE) {
                    delete responsePtr;
                    Serial.println("[WARN] Response queue full, dropping message");
                }
            }
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void RTL8720DNCommunication::sendCommand(const char* cmd) {
    this->RTLSerial->println(cmd);
    Serial.println("[DEBUG] Sent: " + String(cmd));
}

void RTL8720DNCommunication::sendCommand(const String cmd) {
    this->RTLSerial->println(cmd);
    Serial.println("[DEBUG] Sent: " + cmd);
}

// Chờ response với timeout
String RTL8720DNCommunication::waitForResponse(uint32_t timeout_ms) {
    String* responsePtr = nullptr;
    String result = "";
    
    TickType_t timeout_ticks = timeout_ms / portTICK_PERIOD_MS;
    
    if (xQueueReceive(responseQueue, &responsePtr, timeout_ticks) == pdTRUE) {
        if (responsePtr) {
            result = *responsePtr;
            delete responsePtr;
        }
    }
    
    return result;
}

void RTL8720DNCommunication::isReady() {
    this->sendCommand("RTL_READY");
    
    String response = waitForResponse(800); // Wait 0.8 seconds
    
    if (response == "RTL_READY") {
        rtl8720dn_ready = true;
        Serial.println("[INFO] RTL8720DN is ready.");
    } else {
        rtl8720dn_ready = false;
        Serial.println("[ERROR] RTL8720DN is not responding. Got: " + response);
    }
}

void RTL8720DNCommunication::stopReading() {
    if (readTaskHandle != NULL) {
        vTaskDelete(readTaskHandle);
        readTaskHandle = NULL;
    }
}

String RTL8720DNCommunication::filterRTLData(String mixedLine) {
    // Tìm vị trí bắt đầu của NETWORK
    int rtlStart = 0;
    rtlStart = mixedLine.indexOf("NETWORK:");
    if (rtlStart == -1) {
        rtlStart = mixedLine.indexOf("STATION:");
        if (rtlStart == -1) {
            return ""; // Không tìm thấy NETWORK hoặc STATION
        }
    }
    
    // Tìm vị trí kết thúc (trước error message hoặc cuối dòng)
    int rtlEnd = mixedLine.length();
    
    // Các pattern error thường gặp
    String errorPatterns[] = {
        "ioctl[RTKIOCSIWFREQ] error"
    };
    
    for (int i = 0; i < GET_SIZE(errorPatterns); i++) {
        int errorPos = mixedLine.indexOf(errorPatterns[i], rtlStart);
        if (errorPos != -1 && errorPos < rtlEnd) {
            rtlEnd = errorPos;
        }
    }
    
    String rtlData = mixedLine.substring(rtlStart, rtlEnd);
    rtlData.trim();

    return rtlData;
}

void RTL8720DNCommunication::parseAPScanResponse(String response) {
    // Format: NETWORK:SSID,RSSI,Channel,Band,BSSID,Security
    response = response.substring(8); // Remove "NETWORK:"
        
    int pos1 = response.indexOf(',');
    int pos2 = response.indexOf(',', pos1 + 1);
    int pos3 = response.indexOf(',', pos2 + 1);
    int pos4 = response.indexOf(',', pos3 + 1);
    int pos5 = response.indexOf(',', pos4 + 1);
        
    if (pos1 > 0 && pos2 > 0 && pos3 > 0 && pos4 > 0 && pos5 > 0) {
        String ssid = response.substring(0, pos1);
        int8_t rssi = response.substring(pos1 + 1, pos2).toInt();
        uint8_t channel = response.substring(pos2 + 1, pos3).toInt();
        String band = response.substring(pos3 + 1, pos4);
        String bssid = response.substring(pos4 + 1, pos5);
        int security = response.substring(pos5 + 1).toInt();

        uint8_t bssid_mac[6];
        stringToMac(bssid, bssid_mac);

        bool mac_match = true;
        bool in_list = false;
        for (int i = 0; i < access_points->size(); i++) {
            mac_match = true;

            for (int x = 0; x < 6; x++) {
                if (bssid_mac[x] != access_points->get(i).bssid[x]) {
                    mac_match = false;
                    break;
                }
            }
            if (mac_match) {
                in_list = true;
                break;
            }
        }

        if (!in_list) {
            AccessPoint ap;
            ap.essid = ssid;
            ap.rssi = rssi;
            ap.channel = channel;
            memcpy(ap.bssid, bssid_mac, 6);
            ap.selected = false;
    
            String wpastr = "Unknown";
    
            switch(security) {
                case WIFI_SECURITY_OPEN: wpastr = "Open"; break;
                case WIFI_SECURITY_WEP: wpastr = "WEP"; break;
                case WIFI_SECURITY_WPA: wpastr = "WPA"; break;
                case WIFI_SECURITY_WPA2: wpastr = "WPA2"; break;
                case WIFI_SECURITY_WPA2_ENTERPRISE: wpastr = "WPA2/Enterprise"; break;
                case WIFI_SECURITY_WPA3: wpastr = "WPA3"; break;
                case WIFI_SECURITY_WPA_WPA2_MIXED: wpastr = "WPA/WPA2 Mixed"; break;
                case WIFI_SECURITY_WAPI: wpastr = "WAPI"; break;
            }
    
            ap.wpa = security;
            ap.wpastr = wpastr;
            ap.band = (band == "2.4G") ? WIFI_BAND_2_4G : WIFI_BAND_5G;
            ap.stations = new LinkedList<uint16_t>();
            
            if (!low_memory_warning)
				display_buffer->add("Ch:" + String(channel) + " " + ssid);
			else
				display_buffer->add("Low Mem! Ignore!");
            wifiScanRedraw = true;

            if (!low_memory_warning) {
				access_points->add(ap);
                Serial.println("[INFO] Added: " + ssid + " (Ch: " + String(channel) + ")" + " (BSSID: " + bssid \
                    + ")" + " (RSSI: " + String(rssi) + ") (Band: " + band + ") (Security: " + wpastr + ")");
			} else {
				Serial.println("[WARN] Low Memory! Ignore AP " + ssid + " (Ch: " + String(channel) + ")" + " (BSSID: " + bssid \
				+ ")" + " (RSSI: " + String(rssi) + ")" + " (Security: " + wpastr + ") - Not added to list");
			}
        }
    }
}

void RTL8720DNCommunication::parseAPSTAScanResponse(String response) {
    // Format: NETWORK:SSID,RSSI,Channel,Band,BSSID,Security
    // Format: STATION:SRC_BSSID,DST_BSSID

    if (response.startsWith("NETWORK:")) {
        parseAPScanResponse(response);
    } else if (response.startsWith("STATION:")) {
        response = response.substring(8); // Remove "STATION:"
        
        int pos1 = response.indexOf(',');
        
        if (pos1 > 0) {
            String src_bssid = response.substring(0, pos1);
            String dst_bssid = response.substring(pos1 + 1);

            uint8_t src_mac[6];
            uint8_t dst_mac[6];
            stringToMac(src_bssid, src_mac);
            stringToMac(dst_bssid, dst_mac);

            bool matched_ap = false;
            bool mac_match = true;
            bool ap_is_src = false;
            bool in_list = false;
            int ap_index = 0;
            uint8_t _temp_mac[6];

            for (int y = 0; y < 2; y++) {
                (y == 0) ? memcpy(_temp_mac, src_mac, 6) : memcpy(_temp_mac, dst_mac, 6);
                for (int i = 0; i < access_points->size(); i++) {
                    mac_match = true;
                    
                    for (int x = 0; x < 6; x++) {
                        if (_temp_mac[x] != access_points->get(i).bssid[x]) {
                            mac_match = false;
                            break;
                        }
                    }
                    if (mac_match) {
                        matched_ap = true;
                        if (y == 0) ap_is_src = true;
                        else ap_is_src = false;
                        ap_index = i;
                        break;
                    }
                }
                if (matched_ap) break;
            }

            if (!matched_ap) {
                return; // Drop frame if AP not found
            }

            for (int i = 0; i < device_station->size(); i++) {
                mac_match = true;

                for (int x = 0; x < 6; x++) {
                    if (ap_is_src) {
                        if (dst_mac[x] != device_station->get(i).mac[x]) {
                            mac_match = false;
                            break;
                        }
                    } else {
                        if (src_mac[x] != device_station->get(i).mac[x]) {
                            mac_match = false;
                            break;
                        }
                    }
                }

                if (mac_match) {
                    in_list = true;
                    break;
                }
            }
            char dst_addr[] = "00:00:00:00:00:00";
            getMAC(dst_addr, dst_mac, 0);

            if (in_list || (strcmp(dst_addr, "ff:ff:ff:ff:ff:ff") == 0)) {
                return; // Drop if station already in list or broadcast
            }

            Station sta;
            if (ap_is_src) {
                memcpy(sta.mac, dst_mac, 6);
            } else {
                memcpy(sta.mac, src_mac, 6);
            }

            sta.selected = false;

            if (!low_memory_warning) device_station->add(sta);
            char sta_addr[] = "00:00:00:00:00:00";
            if (ap_is_src) {
                getMAC(sta_addr, dst_mac, 0);
            } else {
                getMAC(sta_addr, src_mac, 0);
            }

            if (!low_memory_warning) {
                Serial.println("[INFO] Added Station " + String(sta_addr)  +" -> Ap:" + access_points->get(ap_index).essid);
                display_buffer->add(String(sta_addr));
                display_buffer->add("->" + access_points->get(ap_index).essid);
            } else {
                Serial.println("[WARN] Low Memory! Ignore Station " + String(sta_addr) + " -> Ap:" + access_points->get(ap_index).essid + " not added to display buffer!");
                display_buffer->add("Low Mem! Ignore!");
            }
            
            wifiScanRedraw = true;
    
            if (!low_memory_warning) {
                AccessPoint ap = access_points->get(ap_index);
                ap.stations->add(device_station->size() - 1);
                
                access_points->set(ap_index, ap);
            }
        }
    }
}