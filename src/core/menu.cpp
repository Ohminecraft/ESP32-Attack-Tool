#include "menuheader.h"

/*
	* menu.cpp
	* /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
	* /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
	* Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
	* Licensed under the MIT License.
*/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"

WiFiModules wifi;
BLEModules ble;
DisplayModules display;
EvilPortalAddtional eportal;
NRF24Modules nrf;
IRSendModules irtx;
IRReadModules irrx;


// https://github.com/pr3y/Bruce/blob/main/src/main.cpp
void __attribute__((weak)) taskHandleInput(void *parameter) {
	auto timer = millis();
	while (true) {
		if (millis() - timer > 20) {
			handleInputs();
			//nextPress = false;
			//prevPress = false;
			//selPress = false;
			timer = millis();
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void menuinit() {
	#if defined(USING_ENCODER)
	encoder = new RotaryEncoder(ENC_PIN_A, ENC_PIN_B, RotaryEncoder::LatchMode::FOUR3);

    // register interrupt routine
    attachInterrupt(digitalPinToInterrupt(ENC_PIN_A), checkPosition, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_PIN_B), checkPosition, CHANGE);
	#elif defined(USING_BUTTON)
	pinMode(LEFT_BTN, INPUT_PULLUP);
	pinMode(RIGHT_BTN, INPUT_PULLUP);
	#endif
	pinMode(SEL_BTN, INPUT_PULLUP);
	#if defined(_SPI_SCREEN)
		pinMode(CS_PIN, OUTPUT);
		pinMode(NRF24_CSN_PIN, OUTPUT);
		digitalWrite(CS_PIN, HIGH);
		digitalWrite(NRF24_CSN_PIN, HIGH);
	#endif

	if (!display.main()) {
		Serial.println("[ERROR] Failed to initialize display!");
		while(1); // Halt if display fails
	}
	irtx.main();
	irrx.main();
	wifi.main();
	ble.main();
	eportal.setup();
	Serial.println("[INFO] Menu system initialized");
	Serial.printf("[INFO] Total heap: %d bytes\n", String(getHeap(GET_TOTAL_HEAP)).toInt());
	Serial.printf("[INFO] Free heap: %d bytes\n", String(getHeap(GET_FREE_HEAP)).toInt());
	Serial.printf("[INFO] Used heap: %d bytes\n", String(getHeap(GET_USED_HEAP)).toInt());
	Serial.printf("[INFO] Used: %d%%\n", String(getHeap(GET_USED_HEAP_PERCENT)).toInt());
	#if defined(_I2C_SCREEN) || defined(_SPI_SCREEN) // For CLI (Future)
		xTaskCreate(taskHandleInput, "HandleInput", 4096, NULL, 2, &xHandle);
	#endif
	displayWelcome();
	displayMainMenu();
}

void displayWelcome() {
	display.clearScreen();
	String title = "ESP32 Attack Tool";
	int yTitle = SCR_HEIGHT / 2 - 5;
	int yVersion = SCR_HEIGHT / 2 + 5;
	display.drawingCenterString(title, yTitle);
	display.drawBipmap((SCR_WIDTH / 2) - 12, yVersion - 4, 22, 22, bitmapicon[0], true);
	vTaskDelay(1300 / portTICK_PERIOD_MS);
	display.clearScreen();
	String footer1 = "Code By";
	String footer2 = "@Ohminecraft";
	String version_num = "v" + String(ATTACK_TOOL_VERSION);
	display.drawingCenterString(footer1, yTitle);
	display.drawingCenterString(footer2, yVersion);
	display.drawingCenterString(version_num, yVersion + 12, true);	
	display.displayInvert(true);
	vTaskDelay(1300 / portTICK_PERIOD_MS);
	display.displayInvert(false);
	//display.clearScreen();
	for (int i = 0; i < 9; i++) {
		for (int x = 0; x < startup_bitmap_allArray_LEN; x++) {
			if (selPress) break;
			display.clearScreen();
			display.drawBipmap(0, 0, 128, 64, startup_bitmap_allArray[x], true);
			vTaskDelay(1 / portTICK_PERIOD_MS);
		}
		if (check(selPress)) break;
	}
	//String readyText1 = "Welcome Hacker";
	//String readyText2 = "Home!";
	//display.drawingCenterString(readyText1, yTitle);
	//display.drawingCenterString(readyText2, yVersion, true);
}

void displayStatusBar(bool sendDisplay = false) {
	display.clearScreen();
	if (currentState == MAIN_MENU)
		display.displayStringwithCoordinates("Main Menu", 0, 12);
	else if (currentState == BLE_MENU)
		display.displayStringwithCoordinates("BLE Menu", 0, 12);
	else if (currentState == BLE_INFO_MENU_LIST)
		display.displayStringwithCoordinates("BLE Info", 0, 12);
	else if (currentState == BLE_INFO_MENU_DETAIL)
		display.displayStringwithCoordinates("BLE Detail", 0, 12);
	else if (currentState == BLE_SCAN_RUNNING)
		display.displayStringwithCoordinates("BLE Scan", 0, 12);
	else if (currentState == BLE_SPOOFER_MAIN_MENU)
		display.displayStringwithCoordinates("BLE Spoofer", 0, 12);
	else if (currentState == BLE_SPOOFER_APPLE_MENU)
		display.displayStringwithCoordinates("BLE Apple Spf", 0, 12);
	else if (currentState == BLE_SPOOFER_SAMSUNG_MENU)
		display.displayStringwithCoordinates("BLE Samsung Spf", 0, 12);
	else if (currentState == BLE_SPOOFER_GOOGLE_MENU)
		display.displayStringwithCoordinates("BLE Google Spf", 0, 12);
	else if (currentState == BLE_SPOOFER_AD_TYPE_MENU)
		display.displayStringwithCoordinates("BLE AdType", 0, 12);
	else if (currentState == BLE_EXPLOIT_ATTACK_MENU)
		display.displayStringwithCoordinates("BLE Exploit Atk", 0, 12);
	else if (currentState == BLE_ATTACK_RUNNING)
		display.displayStringwithCoordinates("BLE Attack", 0, 12);
	else if (currentState == WIFI_SELECT_MENU ||
		 	 currentState == WIFI_SELECT_STA_AP_MENU ||
			 currentState == WIFI_SELECT_STA_MENU ||
			 currentState == WIFI_SELECT_PROBE_REQ_SSIDS_MENU
	) 
		display.displayStringwithCoordinates("WiFi Sel Menu", 0, 12);
	else if (currentState == WIFI_GENERAL_MENU)
		display.displayStringwithCoordinates("WiFi Gen Menu", 0, 12);
	else if (currentState == WIFI_ATTACK_MENU)
		display.displayStringwithCoordinates("WiFi Atk Menu", 0, 12);
	else if (currentState == WIFI_SCAN_RUNNING)
		display.displayStringwithCoordinates("WiFi Scan", 0, 12);
	else if (currentState == WIFI_MENU)
		display.displayStringwithCoordinates("WiFi Menu", 0, 12);
	else if (currentState == WIFI_UTILS_MENU)
		display.displayStringwithCoordinates("WiFi Utils", 0, 12);
	else if (currentState == NRF24_MENU)
		display.displayStringwithCoordinates("NRF Menu", 0, 12);
	else if (currentState == NRF24_JAMMER_MENU)
		display.displayStringwithCoordinates("NRF Jam Menu", 0, 12);
	else if (currentState == WIFI_SCAN_SNIFFER_RUNNING)
		display.displayStringwithCoordinates("WiFi Sniffer", 0, 12);
	else if (currentState == NRF24_ANALYZER_RUNNING)
		display.displayStringwithCoordinates("NRF Analyzer", 0, 12);
	else if (currentState == NRF24_SCANNER_RUNNING)
		display.displayStringwithCoordinates("NRF Scanner", 0, 12);
	else if (currentState == NRF24_JAMMER_RUNNING)
		display.displayStringwithCoordinates("NRF Jammer", 0, 12);
	else if (currentState == WIFI_ATTACK_RUNNING)
		display.displayStringwithCoordinates("WiFi Attack", 0, 12);
	else if (currentState == IR_MENU)
		display.displayStringwithCoordinates("IR Menu", 0, 12);
	else if (currentState == IR_TV_B_GONE_REGION)
		display.displayStringwithCoordinates("IR Tv-B-Gone", 0, 12);
	else if (currentState == IR_SEND_RUNNING)
		display.displayStringwithCoordinates("IR Send", 0, 12);
	else
		display.displayStringwithCoordinates("Unknown State", 0, 12);
	
	String heapInfo = "H: ";
	heapInfo.concat(String(getHeap(GET_USED_HEAP_PERCENT)).toInt());
	heapInfo.concat("%");
	display.displayStringwithCoordinates(heapInfo, SCR_WIDTH - 38, 12);
	if (sendDisplay) display.sendDisplay();
}

void menuNode(String items[], int itemCount) {
	displayStatusBar();
	
	// Display only MAX_SHOW_SECLECTION items at a time due to screen height
	uint8_t startIndex = (currentSelection >= MAX_SHOW_SECLECTION) ? currentSelection - 1 : 0;
		
	for(int i = 0; i < MAX_SHOW_SECLECTION && (startIndex + i) < itemCount; i++) {
		String itemText = ((startIndex + i) == currentSelection) ? 
			"> " + items[startIndex + i] : "  " + items[startIndex + i];
		display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
	}
		
	display.sendDisplay();
}

void menuNode(String items, String info, String errortext[2], int itemCount, bool showBack = true) {
	itemCount = itemCount + 1; // Back Option
	if(currentSelection < itemCount - 1) {
		String itemText = "> " + items;
		display.displayStringwithCoordinates(itemText, 0, 24);
	}
	
	 else if (itemCount - 1 == 0) {
		display.displayStringwithCoordinates(errortext[0], 0, 24);
		display.displayStringwithCoordinates(errortext[1], 0, 36);
		display.displayStringwithCoordinates("> Back", 0, 48, true);
	} else {
		display.displayStringwithCoordinates(info + String(itemCount - 1), 0, 24, true);
		display.displayStringwithCoordinates("> Back", 0, 48, true);
	}
	
	display.sendDisplay();
}

void menuNode(String items[], size_t item_size, String info, String errortext[2], int itemCount) {
	if(currentSelection < itemCount) {
		String itemText;
		for (int i = 0; i < item_size; i++) {
			if (i == 0) itemText = "> " + items[i];
			else itemText = items[i];
			display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
		}
	} else if (itemCount == 0) {
		display.displayStringwithCoordinates(errortext[0], 0, 24);
		display.displayStringwithCoordinates(errortext[1], 0, 36);
		display.displayStringwithCoordinates("> Back", 0, 48);
	} else {
		display.displayStringwithCoordinates(info + String(itemCount), 0, 24);
		display.displayStringwithCoordinates("> Back", 0, 48);
	}
	
	display.sendDisplay();
}

void displayMainMenu() {
	String items[MAIN_MENU_COUNT] = {
		"BLE",
		"WiFi",
		"NRF24",
		"IR",
		"Deep Sleep",
		"Reboot"
	};
	
	menuNode(items, MAIN_MENU_COUNT);
}

void displayBLEScanMenu() {
	displayStatusBar(true);
	if (bleScanRunning) {
		if (!bleScanInProgress) {
			bleScanDisplay = true;
			display.displayStringwithCoordinates("Scan complete", 0, 24);
			display.displayStringwithCoordinates("SELECT->back", 0, 36);
			display.displayStringwithCoordinates("Found: " + String(blescanres ? blescanres->size() : 0), 0, 48, true);
		}
	} else {
		display.displayStringwithCoordinates("Press SELECT to", 0, 24);
		display.displayStringwithCoordinates("start scan", 0, 36);
		display.displayStringwithCoordinates("or LEFT to go back", 0, 48, true);
	}
}

void displayBLEMenu() {
	String items[BLE_MENU_COUNT] = {
		"BLE Scan",
		"BLE Info",
		"BLE Spoofer",
		"BLE Exploit Atk",
		"< Back"
	};

	menuNode(items, BLE_MENU_COUNT);
}

void displayBLEInfoListMenu() {
	displayStatusBar();
	String ble_title;
	if (currentSelection < blescanres->size()) {
		BLEScanResult bledevice = blescanres->get(currentSelection);
		ble_title = bledevice.name;
		#ifdef USE_NIMBLE
		if (ble_title == "<no name>") ble_title = bledevice.addr.toString().c_str();
		#else
		if (ble_title == "<no name>") ble_title = bledevice.addr;
		#endif
		// Truncate if too long
		if (ble_title.length() > 17)
			ble_title = ble_title.substring(0, 18) + "...";
	}
	String errortext[2] = {
		"No Devices Found!",
		"Scan First!"
	};
	menuNode(ble_title, "Devices: ", errortext, blescanres->size());
}

void displayBLEInfoDetail() {
	displayStatusBar();

	if (!blescanres || currentSelection >= blescanres->size()) {
		display.displayStringwithCoordinates("Invalid device", 0, 24);
		return;
	}

	BLEScanResult res = blescanres->get(currentSelection);
	String nameStr = res.name;
	#ifdef USE_NIMBLE
	String addrStr = res.addr.toString().c_str();
	#else
	String addrStr = res.addr;
	#endif
	String rssiStr = "RSSI: " + String(res.rssi);

	display.displayStringwithCoordinates(nameStr, 0, 24);
	display.displayStringwithCoordinates("Addr:" + addrStr, 0, 36);
	display.displayStringwithCoordinates(rssiStr, 0, 48, true);
}

void displayMainSpooferMenu() {
	digitalWrite(STA_LED, LOW);
	displayStatusBar();

	String items[BLE_SPOOFER_COUNT] = {
		"Apple",
		"Samsung",
		"Google",
		"< Back"
	};

	menuNode(items, BLE_SPOOFER_COUNT);
}

void displayAppleSpooferMenu() {
	digitalWrite(STA_LED, LOW);
	displayStatusBar();

	String items[BLE_SPO_APPLE_COUNT] = {
		"Airpods",
		"Airpods Pro",
		"Airpods Max",
		"Airpods Gen 2",
		"Airpods Gen 3",
		"Airpods Pro Gen 2",
		"PowerBeats",
		"PowerBeats Pro",
		"Beats Solo Pro",
		"Beats Studio Buds",
		"Beats Flex",
		"Beats X",
		"Beats Solo 3",
		"Beats Studio 3",
		"Beats Studio Pro",
		"Beats Fit Pro",
		"Beats Studio Buds+",
		"< Back"
	};

	menuNode(items, BLE_SPO_APPLE_COUNT);
}

void displaySamsungSpooferMenu() {
	digitalWrite(STA_LED, LOW);
	displayStatusBar();

	String items[BLE_SPO_SAMSUNG_COUNT] = {
		"Fallback Watch",
		"White Watch4 Classic 44m",
		"Black Watch4 Classic 40m",
		"White Watch4 Classic 40m",
		"Black Watch4 44m",
		"Silver Watch4 44m",
		"Green Watch4 44m",
		"Black Watch4 40m",
		"White Watch4 40m",
		"Gold Watch4 40m",
		"French Watch4",
		"French Watch4 Classic",
		"Fox Watch5 44mm",
		"Black Watch5 44mm",
		"Sapphire Watch5 44mm",
		"Purpleish Watch5 40mm",
		"Gold Watch5 40mm",
		"Black Watch5 Pro 45mm",
		"Gray Watch5 Pro 45mm",
		"White Watch5 44mm",
		"White Black Watch5",
		"Black Watch6 Pink 40mm",
		"Gold Watch6 Gold 40mm",
		"Silver Watch6 Cyan 44mm",
		"Black Watch6 Classic 43m",
		"Green Watch6 Classic 43m",
		"< Back"
	};

	menuNode(items, BLE_SPO_SAMSUNG_COUNT);
}


void displayGoogleSpooferMenu() {
	digitalWrite(STA_LED, LOW);
	displayStatusBar();

	String items[BLE_SPO_GOOGLE_COUNT] = {
		"Bisto Dev Board",
		"Arduino 101",
		"Arduino 101 2",
		"AntiSpoof Test",
		"Google Gphones",
		"Test 00000D",
		"Android Auto",
		"Test Android TV",
		"Test Android TV 2",
		"Fast Pair Headphones",
		"LG HBS1110",
		"Smart Controller 1",
		"BLE Phone",
		"Goodyear",
		"Smart Setup",
		"Goodyear 2",
		"T10",
		"ATS2833 EVB",
		"Bose NC700",
		"Bose QC35 II",
		"Bose QC35 II 2",
		"JBL Flip 6",
		"JBL Buds Pro",
		"JBL Live 300TWS",
		"JBL Everest 110GA",
		"Pixel Buds",
		"Pixel Buds 2",
		"Sony XM5",
		"DENON AH-C830NCW",
		"JBL LIVE FLEX",
		"JBL REFLECT MINI NC",
		"Bose QC35 II Dup1",
		"Bose QC35 II Dup2",
		"JBL Everest 110GA 2",
		"JBL LIVE400BT",
		"JBL Everest 310GA",
		"LG HBS 1500",
		"JBL VIBE BEAM",
		"JBL WAVE BEAM",
		"Beoplay H4",
		"JBL TUNE 720BT",
		"WONDERBOOM 3",
		"Beoplay E6",
		"JBL LIVE220BT",
		"Sony WI-1000X",
		"JBL Everest 310GA 2",
		"LG HBS 1700",
		"SRS XB43",
		"WI-1000XM2",
		"Sony WF-SP700N",
		"DENON AH-C830NCW 2",
		"JBL TUNE125TWS",
		"JBL LIVE770NC",
		"JBL Everest 110GA GunMetal",
		"LG HBS 835",
		"LG HBS 2000",
		"Flipper Zero",
		"Free Robux",
		"Free VBucks",
		"Rickroll",
		"Animated Rickroll",
		"BLM",
		"Talking Sasquach",
		"Obama",
		"Ryanair",
		"FBI",
		"Tesla",
		"Ton Upgrade Netflix",
		"< Back"
	};
	menuNode(items, BLE_SPO_GOOGLE_COUNT);
}


void displayAdTypeSpooferMenu() {
	digitalWrite(STA_LED, LOW);
	displayStatusBar();

	String items[BLE_SPO_AD_TYPE_COUNT] = {
		#ifndef USE_NIMBLE
		"Type NON",
		"Type IND",
		"Type SCAN",
		#else
		"Type NON",
		"Type DIR",
		"Type UND",
		#endif
		"< Back"
	};

	menuNode(items, BLE_SPO_AD_TYPE_COUNT);
}

void displaySpooferRunning() {
	digitalWrite(STA_LED, HIGH);
	display.clearScreen();
	display.displayStringwithCoordinates("ADVERTISING...", 0, 12, true);
}

void displayExploitAttackBLEMenu() {
	digitalWrite(STA_LED, LOW);
	displayStatusBar();
	
	String items[BLE_ATK_MENU_COUNT] = {
		"Sour Apple",
		"Apple Juice",
		"Swiftpair MS", 
		"Samsung Spam",
		"Google Spam",
		"< Back"
	};
	
	menuNode(items, BLE_ATK_MENU_COUNT);
}

void displayWiFiMenu() {
	displayStatusBar();
	
	String items[WIFI_MENU_COUNT] = {
		"WiFi General",
		"WiFi Select",
		"WiFi Sta Select",
		"WiFi Probe SSIDs Sel",
		"WiFi Utils",
		"WiFi Attack",
		"< Back"
	};
	
	menuNode(items, WIFI_MENU_COUNT);
}

void displayWiFiUtilsMenu() {
	displayStatusBar();
	
	String items[WIFI_UTILS_MENU_COUNT] = {
		"Set AP MAC",
		"Set STA MAC",
		"Generate AP MAC",
		"Generate STA MAC",
		"< Back"
	};
	
	menuNode(items, WIFI_UTILS_MENU_COUNT);
}

void displayWiFiGeneralMenu() {
	displayStatusBar();

	String items[WIFI_GENERAL_MENU_COUNT] = {
		"Scan AP",
		"Scan AP (Old)",
		"Scan AP/STA",
		"Probe Req Scan",
		"Deauth Scan",
		"Beacon Scan",
		"EAPOL/PMKID Scan",
		"EAPOL.. Deauth Scan",
		"< Back"
	};

	menuNode(items, WIFI_GENERAL_MENU_COUNT);
}

void displayWiFiScanMenu(WiFiGeneralItem mode) {
	displayStatusBar(true);
	if (wifiScanRunning) {
		if (!wifiScanInProgress) {
			wifiScanDisplay = true;
			display.displayStringwithCoordinates("Scan complete", 0, 24);
			display.displayStringwithCoordinates("SELECT->back", 0, 36);
			if (mode == WIFI_GENERAL_AP_SCAN || mode == WIFI_GENERAL_AP_SCAN_OLD)
				display.displayStringwithCoordinates("Found: " + String(access_points ? access_points->size() : 0), 0, 48, true);
			else if (mode == WIFI_GENERAL_AP_STA_SCAN) {
				display.displayStringwithCoordinates("AP Found: " + String(access_points ? access_points->size() : 0), 0, 48);
				display.displayStringwithCoordinates("STA Found: " + String(device_station ? device_station->size() : 0), 0, 60, true);
			}
		} else {
			if (wifiSnifferMode == WIFI_GENERAL_AP_SCAN_OLD) {
				display.displayStringwithCoordinates("Scanning...", 0, 24, true);
			}
		}
	} else {
		display.displayStringwithCoordinates("Press SELECT to", 0, 24);
		display.displayStringwithCoordinates("start scan", 0, 36);
		display.displayStringwithCoordinates("or LEFT to go back", 0, 48, true);
	}
}

/*
void displayWiFiReScanMenu(uint32_t elapsedTime) {
	display.clearScreen();
	display.displayStringwithCoordinates("==== WIFI SCAN ====", 0, 12);
	display.displayStringwithCoordinates("ERROR!", 0, 24);
	display.displayStringwithCoordinates("Not Found Any AP", 0, 36);
	display.displayStringwithCoordinates("ReScan in: " + String(elapsedTime), 0, 48);
}
*/

void displayWiFiSelectMenu() {
	displayStatusBar();

	String items[4] = {};
	
	if (currentSelection < access_points->size()) {

		if (!set_mac) {
			AccessPoint ap = access_points->get(currentSelection);
			String status = ap.selected ? "[*] " : "[ ] ";
			String apInfo = status + ap.essid;
			char bssidStr[18];

			snprintf(bssidStr, sizeof(bssidStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
			ap.bssid[0], ap.bssid[1], ap.bssid[2], 
			ap.bssid[3], ap.bssid[4], ap.bssid[5]);
			

			for (int i = 0; i < 4; i++) {
				if (i == 0) items[i] = apInfo;
				else if (i == 1) items[i] = ("Ch:" + String(ap.channel) + " R:" + String(ap.rssi));
				else if (i == 2) items[i] = ("B:" + String(bssidStr));
				else if (i == 3) items[i] = ("Enc:" + ap.wpastr);
			}
		} else {
			AccessPoint ap = access_points->get(currentSelection);
			String apInfo = ap.essid;
			char bssidStr[18];

			snprintf(bssidStr, sizeof(bssidStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
			ap.bssid[0], ap.bssid[1], ap.bssid[2], 
			ap.bssid[3], ap.bssid[4], ap.bssid[5]);

			for (int i = 0; i < 3; i++) {
				if (i == 0) items[i] = apInfo;
				else if (i == 1) items[i] = ("B:" + String(bssidStr));
				else if (i == 2) items[i] = ("Mac:" + String(macToString(wifi.ap_mac)));
			}
		}
		
	}

	String errorText[2] = {
		"No APs found!",
		"Scan First!"
	};

	menuNode(items, sizeof(items) / sizeof(items[0]), "APs: ", errorText, access_points->size());
}

void displayWiFiSelectProbeReqSsidsMenu() {
	displayStatusBar();

	String items[3] = {};
	
	if (currentSelection < probe_req_ssids->size()) {

		ProbeReqSsid ssid = probe_req_ssids->get(currentSelection);
		String status = ssid.selected ? "[*] " : "[ ] ";
		String ssidInfo = status + ssid.essid;	

		for (int i = 0; i < 3; i++) {
			if (i == 0) items[i] = ssidInfo;
			else if (i == 1) items[i] = ("Ch:" + String(ssid.channel) + " R:" + String(ssid.rssi));
			else if (i == 2) items[i] = ("Requests:" + String(ssid.requests));
		}
	}

	String errorText[2] = {
		"No Ssid found!",
		"Scan First!"
	};

	menuNode(items, sizeof(items) / sizeof(items[0]), "Ssids: ", errorText, probe_req_ssids->size());
}

void displayWiFiSelectAptoSta() {
	displayStatusBar();

	String errorText[2] = {
		"No APs found!",
		"Scan First!"
	};

	menuNode(access_points->get(currentSelection).essid, "APs: ", errorText, access_points->size());
}

void displayWiFiSelectStaInAp() {
	displayStatusBar();
	
	String items[2] = {};
	String staInfo = "";
	AccessPoint ap = access_points->get(ap_index);

	if (currentSelection < ap.stations->size()) {

		if (!set_mac) {
			Station sta = device_station->get(ap.stations->get(currentSelection));
			String status = sta.selected ? "[*] " : "[ ] ";
			staInfo = status + macToString(sta.mac);
			items[0] = staInfo;
		} else {
			// If set_mac is true, we just show the MAC address
			// of the selected station
			Station sta = device_station->get(ap.stations->get(currentSelection));
			staInfo =  macToString(sta.mac);
			for (int i = 0; i < 2; i++) {
				if (i == 0) {
					items[i] = staInfo;
				} else if (i == 1) {
					items[i] = "Mac: " + String(macToString(wifi.sta_mac));
				}
			}
		}

	}

	String errorText[2] = {
		"No STAs found in AP!",
		"Scan First!"
	};

	menuNode(items, sizeof(items) / sizeof(items[0]), "STA in AP: ", errorText, ap.stations->size());
}

void displayWiFiAttackMenu() {
	displayStatusBar();
	
	String items[WIFI_ATK_MENU_COUNT] = {
		"Deauth Tar Attack",
		"Deauth Station Attack",
		"Deauth Flood",
		"Probe Attack",
		"Rickroll Beacon",
		"Stable SSID Beacon",
		"Random Beacon",
		"AP Beacon",
		"Evil Portal",
		"Evil Portal Deauth",
		"Karma",
		"Target Bad Msg",
		"Bad Msg",
		"< Back"
	};
	
	menuNode(items, WIFI_ATK_MENU_COUNT);
}

void displayRebootConfirm() {
	display.clearScreen();
	display.displayStringwithCoordinates("====== REBOOT ======", 0, 12);
	display.displayStringwithCoordinates("Press SELECT to", 0, 24);
	display.displayStringwithCoordinates("confirm reboot", 0, 36);
	display.displayStringwithCoordinates("or LEFT to cancel", 0, 48, true);
}

void displayDeepSleepConfirm() {
	display.clearScreen();
	display.displayStringwithCoordinates("===== DeepSleep =====", 0, 12);
	display.displayStringwithCoordinates("Press SELECT to", 0, 24);
	display.displayStringwithCoordinates("confirm deepsleep", 0, 36);
	display.displayStringwithCoordinates("or LEFT to cancel", 0, 48, true);
}

void displayNRF24Menu() {
	displayStatusBar();

	String items[NRF24_MENU_COUNT] = {
		"Analyzer",
		"Scanner",
		"Jammer",
		"< Back"
	};

	menuNode(items, NRF24_MENU_COUNT);
}

void displayNRF24JammerMenu() {
	displayStatusBar();

	String items[NRF24_JAM_MENU_COUNT] = {
		"WiFi",
		"BLE",
		"Bluetooth",
		"Zigbee",
		"RC",
		"Video Transmitter",
		"USB Wireless",
		"Drone",
		"Full Channel",
		"< Back"
	};

	menuNode(items, NRF24_JAM_MENU_COUNT);
}

void displayEvilPortalInfo() {
	display.clearScreen();
	if (!evilPortalOneShot) {
		if (currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL) {
			if (!access_points || access_points->size() == 0) {
				display.displayStringwithCoordinates("No AP Found!", 0, 12);
				display.displayStringwithCoordinates("Using Default AP:", 0, 24);
				display.displayStringwithCoordinates("'ESP32 Attack Tool'", 0, 36, true);
			}
			else {
				bool foundSelectedAP = false;
				for (int i = 0; i < access_points->size(); i++) {
					if (access_points->get(i).selected) {
						display.displayStringwithCoordinates("Selected AP:", 0, 12);
						display.displayStringwithCoordinates(access_points->get(i).essid, 0, 24, true);
						foundSelectedAP = true;
						break;
					}
				}
				if (!foundSelectedAP) {
					display.displayStringwithCoordinates("No AP Selected!", 0, 12);
					display.displayStringwithCoordinates("Using Default AP:", 0, 24);
					display.displayStringwithCoordinates("'ESP32 Attack Tool'", 0, 36, true);
				}
			}
			
		} else if (currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH) {
			for (int i = 0; i < access_points->size(); i++) {
				if (access_points->get(i).selected) {
					display.displayStringwithCoordinates("Selected AP:", 0, 12);
					display.displayStringwithCoordinates(access_points->get(i).essid, 0, 24);
					display.displayStringwithCoordinates("Enable Deauth Sel AP", 0, 48, true);
					break;
				}
			}
		} else if (currentWiFiAttackType == WIFI_ATTACK_KARMA) {
			for (int i = 0; i < probe_req_ssids->size(); i++) {
				if (probe_req_ssids->get(i).selected) {
					display.displayStringwithCoordinates("Selected Probe Ssid:", 0, 12);
					display.displayStringwithCoordinates(probe_req_ssids->get(i).essid, 0, 24);
					display.displayStringwithCoordinates("Enable Karma", 0, 48, true);
					break;
				}
			}
		}

		if (eportal.runServer) {
			display.clearScreen();
			display.displayStringwithCoordinates("Evil Portal Is", 0, 12);
			display.displayStringwithCoordinates("READY!", 0, 24, true);
			if (currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH) {
				display.displayStringwithCoordinates("Enable Deauth Sel AP", 0, 48, true);
			}
		}
	}
	vTaskDelay(1500 / portTICK_PERIOD_MS);
	if (eportal.runServer) {
		display.clearScreen();
	}

	if (getHeap(GET_FREE_HEAP) <= MEM_LOWER_LIM + 2000) {
		display.displayStringwithCoordinates("LOW MEM!", 80, 36, true);
	}
	/*
	display.clearScreen();
	display.displayString("This Feature Is", true);
	display.displayString("DEPRECATED", true);
	display.displayString("Or will return in", true);
	display.displayString("FUTURE?");
	*/
}

void displayNRFJammerStatus() {
	String attackName = "";
	
	displayStatusBar();
	display.displayStringwithCoordinates("RUNNING", 0, 24);
	
	switch(currentNRFJammerMode) {
		case Wifi:
			attackName = "WiFi";
			break;
		case BLE:
			attackName = "BLE";
			break;
		case Bluetooth:
			attackName = "Bluetooth";
			break;
		case Zigbee:
			attackName = "Zigbee";
			break;
		case RC:
			attackName = "RC";
			break;
		case Video_Transmitter:
			attackName = "Video TX";
			break;
		case Usb_Wireless:
			attackName = "USB Wireless";
			break;
		case Drone:
			attackName = "Drone";
			break;
		case Full_Channel:
			attackName = "Full Channel";
			break;
	}
	
	display.displayStringwithCoordinates("Mode: " + attackName, 0, 36);
	
	// Hiển thị thông tin thêm
	display.displayStringwithCoordinates("Press SELECT to stop", 0, 48);
	
	// Hiển thị memory info nếu cần
	if (getHeap(GET_FREE_HEAP) <= MEM_LOWER_LIM + 2000) {
		display.displayStringwithCoordinates("LOW MEM!", 80, 60);
	}
	
	display.sendDisplay();
}

void displayIRMenu() {
	displayStatusBar();

	String items[IR_MENU_COUNT] = {
		"Tv-B-Gone",
		"< Back"
	};

	menuNode(items, IR_MENU_COUNT);
}

void displayIRTvBGoneRegionMenu() {
	displayStatusBar();

	String items[IR_TV_B_GONE_REGION_COUNT] = {
		"NA",
		"EU",
		"< Back"
	};

	menuNode(items, IR_TV_B_GONE_REGION_COUNT);
}

void displayAttackStatus() {
	String attackName = "";
	if (currentState == BLE_ATTACK_RUNNING) {
	   switch(currentBLEAttackType) {
			case BLE_ATTACK_SOUR_APPLE:
				attackName = "Sour Apple";
				break;
			case BLE_ATTACK_APPLE_JUICE:
				attackName = "Apple Juice";
				break;
			case BLE_ATTACK_MICROSOFT:
				attackName = "Swiftpair MS";
				break;
			case BLE_ATTACK_SAMSUNG:
				attackName = "Samsung Spam";
				break;
			case BLE_ATTACK_GOOGLE:
				attackName = "Google Spam";
				break;
		}
	} else if (currentState == WIFI_ATTACK_RUNNING) {
		switch(currentWiFiAttackType) {
			case WIFI_ATTACK_DEAUTH:
				attackName = "Deauth Attack";
				break;
			case WIFI_ATTACK_STA_DEAUTH:
				attackName = "Station Deauth Attack";
				break;
			case WIFI_ATTACK_AUTH:
				attackName = "Probe Attack";
				break;
			case WIFI_ATTACK_RIC_BEACON:
				attackName = "Rickroll Beacon";
				break;
			case WIFI_ATTACK_STA_BEACON:
				attackName = "Stable SSID Beacon";
				break;
			case WIFI_ATTACK_RND_BEACON:
				attackName = "Random Beacon";
				break;
			case WIFI_ATTACK_AP_BEACON:
				attackName = "AP Beacon";
				break;
			case WIFI_ATTACK_BAD_MSG:
				attackName = "Target Bad Msg";
				break;
			case WIFI_ATTACK_BAD_MSG_ALL:
				attackName = "Bad Msg";
				break;
		}
	}

	displayStatusBar();
	display.displayStringwithCoordinates(attackName, 0, 24);
	
	if (currentState == WIFI_ATTACK_RUNNING) {
		String packetStr = "packet/sec: ";
		packetStr.concat(String(wifi.packet_sent).toInt());
		display.displayStringwithCoordinates(packetStr, 0, 36, true);
	} else if (currentState == BLE_ATTACK_RUNNING) {
		display.displayStringwithCoordinates("Advertising...", 0, 36, true);
	}
	
	// Display memory info
	if (getHeap(GET_FREE_HEAP) <= MEM_LOWER_LIM + 2000) {
		display.displayStringwithCoordinates("LOW MEM!", 80, 48, true);
	}
}

void displayDeauthFloodInfo() {
	if (!wifi.deauth_flood_scan_one_shot) {
		display.clearScreen();
		display.displayStringwithCoordinates("SCANNING...", 0, 12, true);
	}
	if (wifi.deauth_flood_found_ap) {
		if (wifi.deauth_flood_redraw) {
			displayStatusBar();
			display.displayStringwithCoordinates("Deauth Flood", 0, 24);
			display.displayStringwithCoordinates("Target:", 0, 36);
			display.displayStringwithCoordinates(wifi.deauth_flood_target, 0, 48, true);
			wifi.deauth_flood_redraw = false;
		}
	} else {
		if (!fixDeauthFloodDisplayLoop) {
			display.clearScreen();
			display.displayStringwithCoordinates("No Ap Found!", 0, 12);
			display.displayStringwithCoordinates("Please Retry!", 0, 24);
			display.displayStringwithCoordinates("SELECT->Exit", 0, 36, true);
			fixDeauthFloodDisplayLoop = true;
		}
	}

	if (getHeap(GET_FREE_HEAP) <= MEM_LOWER_LIM + 2000) {
		display.displayStringwithCoordinates("LOW MEM!", 80, 48, true);
	}
}

void navigateUp() {
	if (maxSelections <= 1) {
		return;
	}
	
	if (currentSelection > 0) {
		currentSelection--;
	} else {
		currentSelection = maxSelections - 1;
	}
	
	// Redraw menu
   
	switch(currentState) {
		case MAIN_MENU:
			displayMainMenu();
			break;
		case BLE_MENU:
			displayBLEMenu();
			break;
		case BLE_INFO_MENU_LIST:
			displayBLEInfoListMenu();
			break;
		case BLE_INFO_MENU_DETAIL:
			// Không cuộn trong detail
			break;
		case BLE_SPOOFER_MAIN_MENU:
			displayMainSpooferMenu();
			break;
		case BLE_SPOOFER_APPLE_MENU:
			displayAppleSpooferMenu();
			break;
		case BLE_SPOOFER_SAMSUNG_MENU:
			displaySamsungSpooferMenu();
			break;
		case BLE_SPOOFER_GOOGLE_MENU:
			displayGoogleSpooferMenu();
			break;
		case BLE_SPOOFER_AD_TYPE_MENU:
			displayAdTypeSpooferMenu();
			break;
		case BLE_EXPLOIT_ATTACK_MENU:
			displayExploitAttackBLEMenu();
			break;
		case WIFI_MENU:
			displayWiFiMenu();
			break;
		case WIFI_GENERAL_MENU:
			displayWiFiGeneralMenu();
			break;
		case WIFI_UTILS_MENU:
			displayWiFiUtilsMenu();
			break;
		case WIFI_ATTACK_MENU:
			displayWiFiAttackMenu();
			break;
		case WIFI_SELECT_MENU:
			displayWiFiSelectMenu();
			break;
		case WIFI_SELECT_PROBE_REQ_SSIDS_MENU:
			displayWiFiSelectProbeReqSsidsMenu();
			break;
		case WIFI_SELECT_STA_AP_MENU:
			displayWiFiSelectAptoSta();
			break;
		case WIFI_SELECT_STA_MENU:
			displayWiFiSelectStaInAp();
			break;
		case NRF24_MENU:
			displayNRF24Menu();
			break;
		case NRF24_JAMMER_MENU:
			displayNRF24JammerMenu();
			break;
		case IR_MENU:
			displayIRMenu();
			break;
		case IR_TV_B_GONE_REGION:
			displayIRTvBGoneRegionMenu();
			break;
	}
}

void navigateDown() {
	if (maxSelections <= 1) {
		return;
	}

	currentSelection = (currentSelection + 1) % maxSelections;
	
	// Redraw menu
   
	switch(currentState) {
		case MAIN_MENU:
			displayMainMenu();
			break;
		case BLE_MENU:
			displayBLEMenu();
			break;
		case BLE_INFO_MENU_LIST:
			displayBLEInfoListMenu();
			break;
		case BLE_INFO_MENU_DETAIL:
			// Không cuộn trong detail
			break;
		case BLE_SPOOFER_MAIN_MENU:
			displayMainSpooferMenu();
			break;
		case BLE_SPOOFER_APPLE_MENU:
			displayAppleSpooferMenu();
			break;
		case BLE_SPOOFER_SAMSUNG_MENU:
			displaySamsungSpooferMenu();
			break;
		case BLE_SPOOFER_GOOGLE_MENU:
			displayGoogleSpooferMenu();
			break;
		case BLE_SPOOFER_AD_TYPE_MENU:
			displayAdTypeSpooferMenu();
			break;
		case BLE_EXPLOIT_ATTACK_MENU:
			displayExploitAttackBLEMenu();
			break;
		case WIFI_MENU:
			displayWiFiMenu();
			break;
		case WIFI_GENERAL_MENU:
			displayWiFiGeneralMenu();
			break;
		case WIFI_UTILS_MENU:
			displayWiFiUtilsMenu();
			break;
		case WIFI_ATTACK_MENU:
			displayWiFiAttackMenu();
			break;
		case WIFI_SELECT_MENU:
			displayWiFiSelectMenu();
			break;
		case WIFI_SELECT_PROBE_REQ_SSIDS_MENU:
			displayWiFiSelectProbeReqSsidsMenu();
			break;
		case WIFI_SELECT_STA_AP_MENU:
			displayWiFiSelectAptoSta();
			break;
		case WIFI_SELECT_STA_MENU:
			displayWiFiSelectStaInAp();
			break;
		case NRF24_MENU:
			displayNRF24Menu();
			break;
		case NRF24_JAMMER_MENU:
			displayNRF24JammerMenu();
			break;
		case IR_MENU:
			displayIRMenu();
			break;
		case IR_TV_B_GONE_REGION:
			displayIRTvBGoneRegionMenu();
			break;
	}
	
}

void selectCurrentItem() {
	switch(currentState) {
		case MAIN_MENU:
			if (currentSelection == MAIN_BLE) {
				currentState = BLE_MENU;
				currentSelection = 0;
				maxSelections = BLE_MENU_COUNT;
				displayBLEMenu();
			} else if (currentSelection == MAIN_WIFI) {
				currentState = WIFI_MENU;
				currentSelection = 0;
				maxSelections = WIFI_MENU_COUNT;
				displayWiFiMenu();
			} else if (currentSelection == MAIN_NRF24) {
				currentState = NRF24_MENU;
				currentSelection = 0;
				maxSelections = NRF24_MENU_COUNT;
				displayNRF24Menu();
			} else if (currentSelection == MAIN_IR) {
				currentState = IR_MENU;
				currentSelection = 0;
				maxSelections = IR_MENU_COUNT;
				displayIRMenu();
			} else if (currentSelection == MAIN_REBOOT) {
				displayRebootConfirm();
				//delay(1000); // Give user time to see the message
				
				// Wait for user confirmation
				bool confirmed = false;
				bool cancelled = false;
				//unsigned long startTime = millis();
				
				while (!confirmed && !cancelled /*&& (millis() - startTime < 10000)*/) {

					if (check(selPress)) {
						confirmed = true;
					} else if (check(prevPress)) {
						cancelled = true;
					}
					vTaskDelay(10 / portTICK_PERIOD_MS);
				}
				
				if (confirmed) {
					performReboot();
				} else {
					// Return to main menu
					displayMainMenu();
				}
			} else if (currentSelection == MAIN_DEEP_SLEEP) {
				displayDeepSleepConfirm();

				bool confirmed = false;
				bool cancelled = false;
				//unsigned long startTime = millis();
				
				while (!confirmed && !cancelled /*&& (millis() - startTime < 10000)*/) {

					if (check(selPress)) {
						confirmed = true;
					} else if (check(prevPress)) {
						cancelled = true;
					}
					vTaskDelay(10 / portTICK_PERIOD_MS);
				}
				
				if (confirmed) {
					performDeepSleep();
				} else {
					// Return to main menu
					displayMainMenu();
				}
			}
			break;
			
		case BLE_MENU:
			if (currentSelection == BLE_BACK) {
				goBack();
			} else {
				if (currentSelection == BLE_EXPLOIT_ATTACK) {
					currentState = BLE_EXPLOIT_ATTACK_MENU;
					currentSelection = 0;
					maxSelections = BLE_ATK_MENU_COUNT;
					ble.initSpam();
					displayExploitAttackBLEMenu();
				} else if (currentSelection == BLE_INFO) {
					currentState = BLE_INFO_MENU_LIST;
					currentSelection = 0;
					maxSelections = blescanres ? blescanres->size() + 1 : 1;
					displayBLEInfoListMenu();
				} else if (currentSelection == BLE_SPOOFER) {
					currentState = BLE_SPOOFER_MAIN_MENU;
					currentSelection = 0;
					maxSelections = BLE_SPOOFER_COUNT;
					ble.initSpoofer();
					displayMainSpooferMenu();
				} else if (currentSelection == BLE_SCAN) {
					currentState = BLE_SCAN_RUNNING;
					displayBLEScanMenu();
				}
			}
			break;
		case BLE_SCAN_RUNNING:
			if (!bleScanRunning) {
				startBLEScan();
			} else bleScanRunning = false;
			break;
		case BLE_INFO_MENU_LIST:
			if (!blescanres || blescanres->size() == 0 || currentSelection == blescanres->size()) {
				goBack();
			} else {
				currentState = BLE_INFO_MENU_DETAIL;
				displayBLEInfoDetail();
			}
			break;
		case BLE_INFO_MENU_DETAIL:
			currentState = BLE_INFO_MENU_LIST;
			displayBLEInfoListMenu();
			break;
		case BLE_SPOOFER_MAIN_MENU:
			if (currentSelection == BLE_SPOOFER_BACK) {
				goBack();
			}
			else {
				if (currentSelection == BLE_SPOOFER_APPLE) {
					currentState = BLE_SPOOFER_APPLE_MENU;
					currentSelection = 0;
					maxSelections = BLE_SPO_APPLE_COUNT;
					bleSpooferBrandType = BLE_SPO_BRAND_APPLE;
					displayAppleSpooferMenu();
					
				} else if (currentSelection == BLE_SPOOFER_SAMSUNG) {
					currentState = BLE_SPOOFER_SAMSUNG_MENU;
					currentSelection = 0;
					maxSelections = BLE_SPO_SAMSUNG_COUNT;
					bleSpooferBrandType = BLE_SPO_BRAND_SAMSUNG;
					displaySamsungSpooferMenu();
		
				} else if (currentSelection == BLE_SPOOFER_GOOGLE) {
					currentState = BLE_SPOOFER_GOOGLE_MENU;
					currentSelection = 0;
					maxSelections = BLE_SPO_GOOGLE_COUNT;
					bleSpooferBrandType = BLE_SPO_BRAND_GOOGLE;
					displayGoogleSpooferMenu();
				}
			}
			break;

		case BLE_SPOOFER_APPLE_MENU:
			if (currentSelection == BLE_SPO_APPLE_BACK) {
				goBack();
			} else {
				ble_spoofer_device = currentSelection;
				currentState = BLE_SPOOFER_AD_TYPE_MENU;
				currentSelection = 0;
				maxSelections = BLE_SPO_AD_TYPE_COUNT;
				displayAdTypeSpooferMenu();
			}
			break;
		
		case BLE_SPOOFER_SAMSUNG_MENU:
			if (currentSelection == BLE_SPO_SAMSUNG_BACK) {
				goBack();
			} else {
				ble_spoofer_device = currentSelection;
				currentState = BLE_SPOOFER_AD_TYPE_MENU;
				currentSelection = 0;
				maxSelections = BLE_SPO_AD_TYPE_COUNT;
				displayAdTypeSpooferMenu();
			}
			break;
		
		case BLE_SPOOFER_GOOGLE_MENU:
			if (currentSelection == BLE_SPO_GOOGLE_BACK) {
				goBack();
			} else {
				ble_spoofer_device = currentSelection;
				currentState = BLE_SPOOFER_AD_TYPE_MENU;
				currentSelection = 0;
				maxSelections = BLE_SPO_AD_TYPE_COUNT;
				displayAdTypeSpooferMenu();
			}
			break;
		
		case BLE_SPOOFER_AD_TYPE_MENU:
			if (currentSelection == BLE_SPO_AD_TYPE_BACK) {
				goBack();
			} else {
				ble_spoofer_ad_type = currentSelection;
				currentState = BLE_SPOOFER_RUNNING;
			}
			break;
		case BLE_EXPLOIT_ATTACK_MENU:
			if (currentSelection == BLE_ATK_BACK) {
				goBack();
			}
			else {
				// Check memory before starting attack
				if (!checkLeftMemory()) {
					display.clearScreen();
					display.displayStringwithCoordinates("LOW MEMORY!", 0, 12);
					display.displayStringwithCoordinates("Cannot start", 0, 21);
					display.displayStringwithCoordinates("attack", 0, 31, true);
					vTaskDelay(2000 / portTICK_PERIOD_MS);
					displayBLEMenu();
					return;
				}
				
				// Start BLE attack
				BLEScanState attackTypes[] = {BLE_ATTACK_SOUR_APPLE, BLE_ATTACK_APPLE_JUICE, BLE_ATTACK_MICROSOFT, 
									   BLE_ATTACK_SAMSUNG, BLE_ATTACK_GOOGLE};
				startBLEAttack(attackTypes[currentSelection]);
				}
			break;
			
		case WIFI_MENU:
			if (currentSelection == WIFI_GENERAL) {
				currentState = WIFI_GENERAL_MENU;
				currentSelection = 0;
				maxSelections = WIFI_GENERAL_MENU_COUNT;
				displayWiFiGeneralMenu();
			} else if (currentSelection == WIFI_UTILS) {
				currentState = WIFI_UTILS_MENU;
				currentSelection = 0;
				maxSelections = WIFI_UTILS_MENU_COUNT;
				displayWiFiUtilsMenu();
			} else if (currentSelection == WIFI_SELECT) {
				currentState = WIFI_SELECT_MENU;
				currentSelection = 0;
				if (access_points && access_points->size() > 0) {
					maxSelections = access_points->size() + 1;
				} else {
					maxSelections = 1;
				}
				displayWiFiSelectMenu();
			} else if (currentSelection == WIFI_PROBE_REQ_SSIDS_SELECT) {
				currentState = WIFI_SELECT_PROBE_REQ_SSIDS_MENU;
				currentSelection = 0;
				if (probe_req_ssids && probe_req_ssids->size() > 0) {
					maxSelections = probe_req_ssids->size() + 1;
				} else {
					maxSelections = 1;
				}
				displayWiFiSelectProbeReqSsidsMenu();
			} else if (currentSelection == WIFI_STA_SELECT) {
				currentState = WIFI_SELECT_STA_AP_MENU;
				currentSelection = 0;
				if (access_points && access_points->size() > 0) {
					maxSelections = access_points->size() + 1;
				} else {
					maxSelections = 1;
				}
				displayWiFiSelectAptoSta();
			}
			 else if (currentSelection == WIFI_ATTACK) {
				currentState = WIFI_ATTACK_MENU;
				currentSelection = 0;
				maxSelections = WIFI_ATK_MENU_COUNT;
				displayWiFiAttackMenu();
			} else if (currentSelection == WIFI_BACK) {
				goBack();
			}
			break;
		case WIFI_UTILS_MENU:
			if (currentSelection == WIFI_UTILS_GENERATE_AP_MAC) {
				uint8_t mac[6];
				generateRandomMac(mac);
				for (int i = 0; i < 6; i++) {
					wifi.ap_mac[i] = mac[i];
				}
				display.clearScreen();
				display.displayStringwithCoordinates("AP MAC Generated", 0, 12);
				display.displayStringwithCoordinates("MAC: " + macToString(wifi.ap_mac), 0, 24);
				display.displayStringwithCoordinates("Press SELECT to", 0, 36);
				display.displayStringwithCoordinates("go Back", 0, 48, true);
				while(!check(selPress)) {
					vTaskDelay(10 / portTICK_PERIOD_MS);
				}
				displayWiFiUtilsMenu();
			} else if (currentSelection == WIFI_UTILS_GENERATE_STA_MAC) {
				uint8_t mac[6];
				generateRandomMac(mac);
				for (int i = 0; i < 6; i++) {
					wifi.sta_mac[i] = mac[i];
				}
				display.clearScreen();
				display.displayStringwithCoordinates("STA MAC Generated", 0, 12);
				display.displayStringwithCoordinates("MAC:" + macToString(wifi.sta_mac), 0, 24);
				display.displayStringwithCoordinates("Press SELECT to", 0, 36);
				display.displayStringwithCoordinates("go Back", 0, 48, true);
				while(!check(selPress)) {
					vTaskDelay(10 / portTICK_PERIOD_MS);
				}
				displayWiFiUtilsMenu();
			}
			else if (currentSelection == WIFI_UTILS_SET_AP_MAC) {
				set_mac = true;
				currentState = WIFI_SELECT_MENU;
				currentSelection = 0;
				if (access_points && access_points->size() > 0) {
					maxSelections = access_points->size() + 1;
				} else {
					maxSelections = 1;
				}
				displayWiFiSelectMenu();
			}
			else if (currentSelection == WIFI_UTILS_SET_STA_MAC) {
				set_mac = true;
				currentState = WIFI_SELECT_STA_AP_MENU;
				currentSelection = 0;
				if (access_points && access_points->size() > 0) {
					maxSelections = access_points->size() + 1;
				} else {
					maxSelections = 1;
				}
				displayWiFiSelectAptoSta();
			} else if (currentSelection == WIFI_UTILS_BACK) {
				goBack();
			}
			break;
		case WIFI_GENERAL_MENU:
			if (currentSelection == WIFI_GENERAL_BACK) {
				goBack();
			}
			else if (currentSelection == WIFI_GENERAL_AP_SCAN) {
				currentState = WIFI_SCAN_RUNNING;
				displayWiFiScanMenu(WIFI_GENERAL_AP_SCAN);
				//startWiFiScan(WIFI_GENERAL_AP_SCAN);
			}
			else if (currentSelection == WIFI_GENERAL_AP_SCAN_OLD) {
				currentState = WIFI_SCAN_RUNNING;
				displayWiFiScanMenu(WIFI_GENERAL_AP_SCAN_OLD);
			}
			else if (currentSelection == WIFI_GENERAL_AP_STA_SCAN) {
				currentState = WIFI_SCAN_RUNNING;
				displayWiFiScanMenu(WIFI_GENERAL_AP_STA_SCAN);
				//startWiFiScan(WIFI_GENERAL_AP_STA_SCAN);
			}
			else if (currentSelection == WIFI_GENERAL_PROBE_REQ_SCAN) {
				currentState = WIFI_SCAN_SNIFFER_RUNNING;
				displayStatusBar(true);
				startSnifferScan(WIFI_GENERAL_PROBE_REQ_SCAN);
			} else if (currentSelection == WIFI_GENERAL_DEAUTH_SCAN) {
				currentState = WIFI_SCAN_SNIFFER_RUNNING;
				displayStatusBar(true);
				startSnifferScan(WIFI_GENERAL_DEAUTH_SCAN);
			} else if (currentSelection == WIFI_GENERAL_BEACON_SCAN) {
				currentState = WIFI_SCAN_SNIFFER_RUNNING;
				displayStatusBar(true);
				startSnifferScan(WIFI_GENERAL_BEACON_SCAN);
			} else if (currentSelection == WIFI_GENERAL_EAPOL_SCAN) {
				currentState = WIFI_SCAN_SNIFFER_RUNNING;
				displayStatusBar(true);
				startSnifferScan(WIFI_GENERAL_EAPOL_SCAN);
			} else if (currentSelection == WIFI_GENERAL_EAPOL_DEAUTH_SCAN) {
				currentState = WIFI_SCAN_SNIFFER_RUNNING;
				displayStatusBar(true);
				startSnifferScan(WIFI_GENERAL_EAPOL_DEAUTH_SCAN);
			}
			wifiSnifferMode = currentSelection;
			break;
		case WIFI_SCAN_RUNNING:
			if (!wifiScanRunning) {
				if (currentSelection == WIFI_GENERAL_AP_SCAN) {
					startWiFiScan(WIFI_GENERAL_AP_SCAN);
				}
				else if (currentSelection == WIFI_GENERAL_AP_SCAN_OLD) {
					startWiFiScan(WIFI_GENERAL_AP_SCAN_OLD);
				}
				else if (currentSelection == WIFI_GENERAL_AP_STA_SCAN) {
					startWiFiScan(WIFI_GENERAL_AP_STA_SCAN);
				}
			} else wifiScanRunning = false;
			break;
		case WIFI_SELECT_MENU:
			if (!access_points || access_points->size() == 0) {
				goBack();
			} else {
				if (currentSelection < access_points->size()) {
					if (!set_mac) {
						AccessPoint ap = access_points->get(currentSelection);
						ap.selected = !ap.selected;
						access_points->set(currentSelection, ap);
						displayWiFiSelectMenu();
					} else {
						AccessPoint ap = access_points->get(currentSelection);
						for (int i = 0; i < 6; i++) {
							wifi.ap_mac[i] = ap.bssid[i];
						}
						display.clearScreen();
						display.displayStringwithCoordinates("AP MAC Set To", 0, 12);
						display.displayStringwithCoordinates(macToString(wifi.ap_mac), 0, 24, true);
						vTaskDelay(2000 / portTICK_PERIOD_MS);
						goBack();
					}	
				} else {
					goBack();
				}
			}
			break;
		case WIFI_SELECT_PROBE_REQ_SSIDS_MENU:
			if (!probe_req_ssids || probe_req_ssids->size() == 0) {
				goBack();
			} else {
				if (currentSelection < probe_req_ssids->size()) {
					ProbeReqSsid ssid = probe_req_ssids->get(currentSelection);
					ssid.selected = !ssid.selected;
					probe_req_ssids->set(currentSelection, ssid);
					displayWiFiSelectProbeReqSsidsMenu();
				} else {
					goBack();
				}
			}
			break;
		case WIFI_SELECT_STA_AP_MENU:
			if (!access_points || access_points->size() == 0) {
				goBack();
			} else {
				if (currentSelection < access_points->size()) {
					ap_index = currentSelection;
					currentState = WIFI_SELECT_STA_MENU;
					currentSelection = 0;
					maxSelections = access_points->get(ap_index).stations->size() + 1;
					displayWiFiSelectStaInAp();
				} else {
					goBack();
				}
			}
			break;
		case WIFI_SELECT_STA_MENU:
			if (!device_station || access_points->get(ap_index).stations->size() == 0) {
				goBack();
			} else {
				if (!set_mac) {
					if (currentSelection < access_points->get(ap_index).stations->size()) {
						int sta = access_points->get(ap_index).stations->get(currentSelection);
						Station new_sta = device_station->get(sta);
						new_sta.selected = !new_sta.selected;
						device_station->set(sta, new_sta);
						displayWiFiSelectStaInAp();
					} else {
						goBack();
					}
				} else {
					if (currentSelection < access_points->get(ap_index).stations->size()) {
						int sta = access_points->get(ap_index).stations->get(currentSelection);
						Station new_sta = device_station->get(sta);
						for (int i = 0; i < 6; i++) {
							wifi.sta_mac[i] = new_sta.mac[i];
						}
						display.clearScreen();
						display.displayStringwithCoordinates("STA MAC Set To", 0, 12);
						display.displayStringwithCoordinates(macToString(wifi.sta_mac), 0, 24, true);
						vTaskDelay(2000 / portTICK_PERIOD_MS);
						goBack();
					} else {
						goBack();
					}
				}
			}
			break;
		case WIFI_ATTACK_MENU:
			if (currentSelection == WIFI_ATK_BACK) {
				goBack();
			} else {
				// Check if we have selected APs for deauth, AP for probe and AP beacon attacks
				if ((currentSelection == WIFI_ATK_DEAUTH ||
					 currentSelection == WIFI_ATK_STA_DEAUTH ||
					 currentSelection == WIFI_ATK_AUTH ||
					 currentSelection == WIFI_ATK_AP_BEACON ||
					 currentSelection == WIFI_ATK_EVIL_PORTAL_DEAUTH ||
					 currentSelection == WIFI_ATK_BAD_MSG ||
					 currentSelection == WIFI_ATK_BAD_MSG_ALL) && 
					(!access_points || !hasSelectedAPs())) {
					display.clearScreen();
					display.displayStringwithCoordinates("NO AP SELECTED!", 0, 12);
					display.displayStringwithCoordinates("Select AP first", 0, 21, true);
					vTaskDelay(2000 / portTICK_PERIOD_MS);
					displayWiFiAttackMenu();
					return;
				}

				if ((currentSelection == WIFI_ATK_STA_DEAUTH ||
				 	 currentSelection == WIFI_ATK_BAD_MSG) &&
					(!device_station || !hasSelectedSTAs())) {
					display.clearScreen();
					display.displayStringwithCoordinates("NO STA SELECTED!", 0, 12);
					display.displayStringwithCoordinates("Select STA first", 0, 21, true);
					vTaskDelay(2000 / portTICK_PERIOD_MS);
					displayWiFiAttackMenu();
				}

				if (currentSelection == WIFI_ATK_KARMA &&
					(!probe_req_ssids || !hasSelectedProbeReqSSID())) {
					display.clearScreen();
					display.displayStringwithCoordinates("NO SSID SELECTED!", 0, 12);
					display.displayStringwithCoordinates("Select SSID first", 0, 21, true);
					vTaskDelay(2000 / portTICK_PERIOD_MS);
					displayWiFiAttackMenu();
					return;
				}
				
				// Check memory before starting attack
				if (!checkLeftMemory()) {
					display.clearScreen();
					display.displayStringwithCoordinates("LOW MEMORY!", 0, 12);
					display.displayStringwithCoordinates("Cannot start", 0, 21);
					display.displayStringwithCoordinates("attack", 0, 31, true);
					vTaskDelay(2000 / portTICK_PERIOD_MS);
					displayWiFiAttackMenu();
					return;
				}
				
				// Start WiFi attack
				WiFiScanState attackTypes[] = {WIFI_ATTACK_DEAUTH, WIFI_ATTACK_STA_DEAUTH, WIFI_ATTACK_DEAUTH_FLOOD, WIFI_ATTACK_AUTH, WIFI_ATTACK_RIC_BEACON, WIFI_ATTACK_STA_BEACON,
									   WIFI_ATTACK_RND_BEACON, WIFI_ATTACK_AP_BEACON, WIFI_ATTACK_EVIL_PORTAL, WIFI_ATTACK_EVIL_PORTAL_DEAUTH, WIFI_ATTACK_KARMA, WIFI_ATTACK_BAD_MSG, WIFI_ATTACK_BAD_MSG_ALL};
				startWiFiAttack(attackTypes[currentSelection]);
			}
			break;
		
		case NRF24_MENU:
			if (currentSelection == NRF24_BACK) {
				goBack();
			} else {
				if (currentSelection == NRF24_ANALYZER) {
					currentState = NRF24_ANALYZER_RUNNING;
					display.clearScreen();
					nrfAnalyzerSetupOneShot = false;
				}
				else if (currentSelection == NRF24_SCANNER) {
					currentState = NRF24_SCANNER_RUNNING;
					display.clearScreen();
					nrfScannerSetupOneShot = false;
				}
				else if (currentSelection == NRF24_JAMMER) {
					currentState = NRF24_JAMMER_MENU;
					currentSelection = 0;
					maxSelections = NRF24_JAM_MENU_COUNT;
					displayNRF24JammerMenu();
				}
			}
			break;
		case NRF24_JAMMER_MENU:
			if (currentSelection == NRF24_JAM_BACK){
				goBack();
			} else {
				if (!checkLeftMemory()) {
					display.clearScreen();
					display.displayStringwithCoordinates("LOW MEMORY!", 0, 12);
					display.displayStringwithCoordinates("Cannot start", 0, 21);
					display.displayStringwithCoordinates("attack", 0, 31, true);
					vTaskDelay(2000 / portTICK_PERIOD_MS);
					displayNRF24JammerMenu();
					return;
				}
				
				NRFJammerMode listjammermode[] = {Wifi, BLE, Bluetooth, Zigbee, RC, Video_Transmitter, 
											Usb_Wireless, Drone, Full_Channel};
				currentNRFJammerMode = listjammermode[currentSelection];
				startNRFJammer(currentNRFJammerMode);
				displayNRFJammerStatus();
				}
			break;
		case IR_MENU:
			if (currentSelection == IR_TV_B_GONE) {
				currentState = IR_TV_B_GONE_REGION;
				currentSelection = 0;
				maxSelections = IR_TV_B_GONE_REGION_COUNT;
				displayIRTvBGoneRegionMenu();
			} else if (currentSelection == IR_BACK) {
				goBack();
			}
			break;
		case IR_TV_B_GONE_REGION:
			if (currentSelection == IR_TV_B_GONE_BACK) {
				goBack();
			} else {
				// Check memory before starting attack
				if (!checkLeftMemory()) {
					display.clearScreen();
					display.displayStringwithCoordinates("LOW MEMORY!", 0, 12);
					display.displayStringwithCoordinates("Cannot start", 0, 21);
					display.displayStringwithCoordinates("attack", 0, 31, true);
					vTaskDelay(2000 / portTICK_PERIOD_MS);
					displayIRTvBGoneRegionMenu();
					return;
				}
				if (currentSelection == IR_TV_B_GONE_NA) {
					irTvBGoneRegion = NA;
				} else {
					irTvBGoneRegion = EU;
				}
				currentState = IR_SEND_RUNNING;
				starttvbgone = true;
			}
			break;
	}
	
}

void goBack() {
	switch(currentState) {
		case BLE_MENU:
		case WIFI_MENU:
			currentState = MAIN_MENU;
			currentSelection = 0;
			maxSelections = MAIN_MENU_COUNT;
			displayMainMenu();
			break;
		case WIFI_GENERAL_MENU:
			currentState = WIFI_MENU;
			currentSelection = 0;
			maxSelections = WIFI_MENU_COUNT;
			displayWiFiMenu();
			break;
		case WIFI_SCAN_RUNNING:
			if (wifiSnifferMode == WIFI_GENERAL_AP_SCAN || wifiSnifferMode == WIFI_GENERAL_AP_STA_SCAN) {
				if (wifiScanRunning && wifiScanInProgress) {
					wifiScanInProgress = false;
					wifi.StartMode(WIFI_SCAN_OFF);
					if (currentSelection == WIFI_GENERAL_AP_SCAN) {
						Serial.println("[INFO] Wifi Scan completed successfully! AP in list: " + String(access_points->size()));
						displayWiFiScanMenu(WIFI_GENERAL_AP_SCAN);
					} else if (currentSelection == WIFI_GENERAL_AP_STA_SCAN) {
						Serial.println("[INFO] Wifi Scan completed successfully! AP in list: " + String(access_points->size()) + " Station in list: " + String(device_station->size()));
						displayWiFiScanMenu(WIFI_GENERAL_AP_STA_SCAN);
					}
					display.clearBuffer();
				}
				else if (!wifiScanRunning) {
					currentState = WIFI_GENERAL_MENU;
					currentSelection = 0;
					maxSelections = WIFI_GENERAL_MENU_COUNT;
					displayWiFiGeneralMenu();
				}
				else if (wifiScanRunning && wifiScanOneShot &&wifiScanDisplay)  {
					wifiScanRunning = false;
					wifiScanOneShot = false;
					wifiScanDisplay = false;
					currentState = WIFI_GENERAL_MENU;
					currentSelection = 0;
					maxSelections = WIFI_GENERAL_MENU_COUNT;
					wifiSnifferMode = -1;
					displayWiFiGeneralMenu();
				}
			} else {
				if (wifiScanRunning) {
					wifiScanRunning = false;
					wifiScanOneShot = false;
					wifiScanDisplay = false;
				}
				currentState = WIFI_GENERAL_MENU;
				currentSelection = 0;
				maxSelections = WIFI_GENERAL_MENU_COUNT;
				wifiSnifferMode = -1;
				displayWiFiGeneralMenu();
			}
			break;
		case WIFI_UTILS_MENU:
			currentState = WIFI_MENU;
			currentSelection = 0;
			maxSelections = WIFI_MENU_COUNT;
			displayWiFiMenu();
			break;
		case WIFI_SCAN_SNIFFER_RUNNING:
			currentState = WIFI_GENERAL_MENU;
			currentSelection = 0;
			maxSelections = WIFI_GENERAL_MENU_COUNT;
			wifiSnifferMode = -1;
			wifi.StartMode(WIFI_SCAN_OFF);
			displayWiFiGeneralMenu();
			break;
		case WIFI_SELECT_MENU:
			if (!set_mac) {
				currentState = WIFI_MENU;
				currentSelection = 0;
				maxSelections = WIFI_MENU_COUNT;
				displayWiFiMenu();
			} else {
				set_mac = false;
				currentState = WIFI_UTILS_MENU;
				currentSelection = 0;
				maxSelections = WIFI_UTILS_MENU_COUNT;
				displayWiFiUtilsMenu();
			}
			break;
		case WIFI_SELECT_PROBE_REQ_SSIDS_MENU:
		case WIFI_SELECT_STA_AP_MENU:
		case WIFI_ATTACK_MENU:
			currentState = WIFI_MENU;
			currentSelection = 0;
			maxSelections = WIFI_MENU_COUNT;
			displayWiFiMenu();
			break;
		case WIFI_SELECT_STA_MENU:
			if (!set_mac) {
				currentState = WIFI_SELECT_STA_AP_MENU;
				currentSelection = 0;
				maxSelections = access_points->size() + 1;
				displayWiFiSelectAptoSta();
			} else {
				set_mac = false;
				currentState = WIFI_UTILS_MENU;
				currentSelection = 0;
				maxSelections = WIFI_UTILS_MENU_COUNT;
				displayWiFiUtilsMenu();
			}
			break;
		case BLE_SCAN_RUNNING:
			if (bleScanRunning && bleScanInProgress) {
				bleScanInProgress = false;
				ble.ShutdownBLE();
				Serial.println("[INFO] BLE Scan completed successfully! Devices in list: " + String(blescanres->size()));
				displayBLEScanMenu();
				display.clearBuffer();
			}
			else if (!bleScanRunning) {
				currentState = BLE_MENU;
				currentSelection = 0;
				maxSelections = BLE_MENU_COUNT;
				displayBLEMenu();
			}
			else if (bleScanRunning && bleScanOneShot && bleScanDisplay) {
				bleScanRunning = false;
				bleScanOneShot = false;
				bleScanDisplay = false;
				currentState = BLE_MENU;
				currentSelection = 0;
				maxSelections = BLE_MENU_COUNT;
				displayBLEMenu();
			}
			break;
		case BLE_EXPLOIT_ATTACK_MENU:
			currentState = BLE_MENU;
			currentSelection = 0;
			maxSelections = BLE_MENU_COUNT;
			ble.ShutdownBLE();
			displayBLEMenu();
			break;
		case BLE_INFO_MENU_DETAIL:
			currentState = BLE_INFO_MENU_LIST;
			displayBLEInfoListMenu();
			break;
		case BLE_INFO_MENU_LIST:
			currentState = BLE_MENU;
			currentSelection = 0;
			maxSelections = BLE_MENU_COUNT;
			displayBLEMenu();
			break;
		case BLE_ATTACK_RUNNING:
			stopCurrentAttack();
			break;
		case BLE_SPOOFER_MAIN_MENU:
			currentState = BLE_MENU;
			currentSelection = 0;
			maxSelections = BLE_MENU_COUNT;
			ble.ShutdownBLE();
			displayBLEMenu();
			break;
		case BLE_SPOOFER_APPLE_MENU:
			currentState = BLE_SPOOFER_MAIN_MENU;
			currentSelection = 0;
			maxSelections = BLE_SPOOFER_COUNT;
			bleSpooferBrandType = NONE;
			displayMainSpooferMenu();
			break;
		case BLE_SPOOFER_SAMSUNG_MENU:
			currentState = BLE_SPOOFER_MAIN_MENU;
			currentSelection = 0;
			maxSelections = BLE_SPOOFER_COUNT;
			bleSpooferBrandType = NONE;
			displayMainSpooferMenu();
			break;
		case BLE_SPOOFER_GOOGLE_MENU:
			currentState = BLE_SPOOFER_MAIN_MENU;
			currentSelection = 0;
			maxSelections = BLE_SPOOFER_COUNT;
			bleSpooferBrandType = NONE;
			displayMainSpooferMenu();
			break;
		case BLE_SPOOFER_AD_TYPE_MENU:
			if (bleSpooferBrandType == BLE_SPO_BRAND_APPLE) {
				currentState = BLE_SPOOFER_APPLE_MENU;
				currentSelection = 0;
				maxSelections = BLE_SPO_APPLE_COUNT;
				ble_spoofer_device = 0;
				displayAppleSpooferMenu();
			} 
			else if (bleSpooferBrandType == BLE_SPO_BRAND_SAMSUNG) {
				currentState = BLE_SPOOFER_SAMSUNG_MENU;
				currentSelection = 0;
				maxSelections = BLE_SPO_SAMSUNG_COUNT;
				ble_spoofer_device = 0;
				displaySamsungSpooferMenu();
			} 
			else if (bleSpooferBrandType == BLE_SPO_BRAND_GOOGLE) {
				currentState = BLE_SPOOFER_GOOGLE_MENU;
				currentSelection = 0;
				maxSelections = BLE_SPO_GOOGLE_COUNT;
				ble_spoofer_device = 0;
				displayGoogleSpooferMenu();
			} 
			break;
		case BLE_SPOOFER_RUNNING:
			bleSpooferDone = false;
			ble_spoofer_device = 0;
			ble_spoofer_ad_type = 0;
			if (bleSpooferBrandType == BLE_SPO_BRAND_APPLE) {
				currentState = BLE_SPOOFER_APPLE_MENU;
				currentSelection = 0;
				maxSelections = BLE_SPO_APPLE_COUNT;
				displayAppleSpooferMenu();
			} 
			else if (bleSpooferBrandType == BLE_SPO_BRAND_SAMSUNG) {
				currentState = BLE_SPOOFER_SAMSUNG_MENU;
				currentSelection = 0;
				maxSelections = BLE_SPO_SAMSUNG_COUNT;
				displaySamsungSpooferMenu();
			} 
			else if (bleSpooferBrandType == BLE_SPO_BRAND_GOOGLE) {
				currentState = BLE_SPOOFER_GOOGLE_MENU;
				currentSelection = 0;
				maxSelections = BLE_SPO_GOOGLE_COUNT;
				displayGoogleSpooferMenu();
			}
			ble.stopSpoofer();
			break;
		case WIFI_ATTACK_RUNNING:
			stopCurrentAttack();
			break;
		case NRF24_MENU:
			currentState = MAIN_MENU;
			currentSelection = 0;
			maxSelections = MAIN_MENU_COUNT;
			displayMainMenu();
			break;
		case NRF24_ANALYZER_RUNNING:
			currentState = NRF24_MENU;
			currentSelection = 0;
			maxSelections = NRF24_MENU_COUNT;
			nrfAnalyzerSetupOneShot = false;
			nrf.shutdownNRF(*NRFSPI);
			displayNRF24Menu();
			break;
		case NRF24_JAMMER_MENU:
			currentState = NRF24_MENU;
			currentSelection = 0;
			maxSelections = NRF24_MENU_COUNT;
			displayNRF24Menu();
			break;
		case NRF24_JAMMER_RUNNING:
			currentState = NRF24_JAMMER_MENU;
			currentSelection = 0;
			maxSelections = NRF24_JAM_MENU_COUNT;
			nrfJammerSetupOneShot = false;
			digitalWrite(STA_LED, LOW);
			nrf.shutdownNRFJammer();
			displayNRF24JammerMenu();
			break;
		case NRF24_SCANNER_RUNNING:
			currentState = NRF24_MENU;
			currentSelection = 0;
			maxSelections = NRF24_MENU_COUNT;
			nrfScannerSetupOneShot = false;
			display.setCursor(0, 0);
			nrf.shutdownNRF(*NRFSPI);
			displayNRF24Menu();
			break;
		case IR_MENU:
			currentState = MAIN_MENU;
			currentSelection = 0;
			maxSelections = MAIN_MENU_COUNT;
			displayMainMenu();
			break;
		case IR_TV_B_GONE_REGION:
			currentState = IR_MENU;
			currentSelection = 0;
			maxSelections = IR_MENU_COUNT;
			displayIRMenu();
			break;
		case IR_SEND_RUNNING:
			currentState = IR_MENU;
			currentSelection = 0;
			maxSelections = IR_MENU_COUNT;
			displayIRMenu();
			break;
	}
	
}

// https://github.com/cifertech/nRFBox/blob/main/nRFBox/ism.cpp
void nrfAnalyzer() {
	nrf.analyzerScanChannels();

	if(check(selPress)) {
		Serial.println("[INFO] NRF24 Analyzer stopped by user.");
		goBack();
		return;
	}

	memset(values, 0, sizeof(uint8_t)*SCR_WIDTH);

	int n = 200;
	while (n--) {
		if (check(selPress)) {
			Serial.println("[INFO] NRF24 Analyzer stopped by user.");
			goBack();
			break;
		} 
		int i = SCR_WIDTH;
		while (i--) {
			if (check(selPress)) {
				Serial.println("[INFO] NRF24 Analyzer stopped by user.");
				goBack();
				return;
			}
			nrf.setChannel(*NRFSPI, i);
			digitalWrite(NRF24_CE_PIN, HIGH);
			delayMicroseconds(128);
			digitalWrite(NRF24_CE_PIN, LOW);
			if (nrf.carrierDetected(*NRFSPI))
			{
				++values[i];
			}
		}
	}

	if (check(selPress)) {
		Serial.println("[INFO] NRF24 Analyzer stopped by user.");
		goBack();
		return;
	}

	display.clearScreen();
	int x = 0;
	int peak = 0;
	for (int i = 0; i < SCR_WIDTH; i++) {
		if (values[i] > peak) {
			peak = values[i];
		}
		int v = 63 - values[i] * 3;
		if (v < 0) {
			v = 0;
		}
		display.drawingVLine(x, v - 10, 64 - v);
		x += 1;
	}
	Serial.println("[INFO] NRF24 Analyzer | Peak: " + String(peak));
	//display.setFont(u8g2_font_ncenB08_tr);
	display.displayStringwithCoordinates("1...5...10...25..50...80...128", 0, 64);
	display.sendDisplay();
}

void nrfOutputScanChannel() {
	int norm = 0;

    for (int i = 0; i < SCAN_CHANNELS; i++) {
      if (scan_channel[i] > norm) {
        norm = scan_channel[i];
      }
    }

	Serial.println("[INFO] NRF24 Scanner | Data: " + String(norm));

    byte drawHeight = (norm > SCR_HEIGHT) ? SCR_HEIGHT - 1 : norm;

    for (byte count = 126; count > 0; count--) {
      sensorArray[count] = sensorArray[count - 1];
    }
    sensorArray[0] = drawHeight;

    display.clearScreen();

    display.drawingLine(0, 0, 0, 63);
    display.drawingLine(SCR_WIDTH - 1, 0, SCR_WIDTH - 1, 63);

    for (byte count = 0; count < 64; count += 10) {
		display.drawingLine(SCR_WIDTH - 1, count, 122, count);
		display.drawingLine(0, count, 5, count);
    }

    for (byte count = 10; count < SCR_WIDTH - 1; count += 10) {
		display.drawingPixel(count, 0);
		display.drawingPixel(count, 63);
    }

    for (byte count = 0; count < SCR_WIDTH - 1; count++) {
		display.drawingLine(SCR_WIDTH - 1 - count, 63, SCR_WIDTH - 1 - count, 63 - sensorArray[count]);
    }

    display.setCursor(12, 12);
    display.printString("[" + String(norm) + "]");

    display.sendDisplay();
}        

void nrfScanner() {
	if (selPress) {
		return;
	}

	nrf.scanChannel();

	if (selPress) {
		return;
	}

	nrfOutputScanChannel();

	if (selPress) {
		return;
	}
	static unsigned long lastSaveTime = 0;
	if (millis() - lastSaveTime > 5000) {
		nrf.saveGraphtoEEPROM();
		lastSaveTime = millis();
	} 
}

void startBLEAttack(BLEScanState attackType) {
	String strmode = "";
	if (attackType == BLE_ATTACK_SOUR_APPLE) strmode = "Sour Apple";
	else if (attackType == BLE_ATTACK_APPLE_JUICE) strmode = "Apple Juice";
	else if (attackType == BLE_ATTACK_MICROSOFT) strmode = "Swiftpair";
	else if (attackType == BLE_ATTACK_GOOGLE) strmode = "Android (Google)";
	else if (attackType == BLE_ATTACK_SAMSUNG) strmode = "Samsung";
	Serial.println("[INFO] Starting BLE attack: " + strmode);
	
	currentBLEAttackType = attackType;
	attackStartTime = millis();
	currentState = BLE_ATTACK_RUNNING;
	
	digitalWrite(STA_LED, HIGH);
	displayAttackStatus();
}

void startWiFiAttack(WiFiScanState attackType) {
	String strmode;
	if (attackType == WIFI_ATTACK_DEAUTH) {	
		strmode = "Deauth";
	}
	else if (attackType == WIFI_ATTACK_STA_DEAUTH) {
		strmode = "Deauth Station";
	}
	else if (attackType == WIFI_ATTACK_DEAUTH_FLOOD) {
		strmode = "Deauth Flood";
	}
	else if (attackType == WIFI_ATTACK_AUTH) {	
		strmode = "Probe";
	}
	else if (attackType == WIFI_ATTACK_RND_BEACON) {	
		strmode = "Random Beacon";
	}
	else if (attackType == WIFI_ATTACK_STA_BEACON) {	
		strmode = "Static Beacon";
	}
	else if (attackType == WIFI_ATTACK_RIC_BEACON) {
		strmode = "Rick Roll Beacon";
	}
	else if (attackType == WIFI_ATTACK_AP_BEACON) {
		strmode = "AP Beacon";
	}
	else if (attackType == WIFI_ATTACK_EVIL_PORTAL) {
		strmode = "Evil Portal";
	}
	else if (attackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH) {
		strmode = "Evil Portal Deauth";
	}
	else if (attackType == WIFI_ATTACK_KARMA) {
		strmode = "Karma";
	}
	else if (attackType == WIFI_ATTACK_BAD_MSG) {
		strmode = "Target Bad Msg";
	}
	else if (attackType == WIFI_ATTACK_BAD_MSG_ALL) {
		strmode = "Bad Msg";
	}
	Serial.println("[INFO] Starting WiFi attack: " + strmode);
	
	currentWiFiAttackType = attackType;
	currentState = WIFI_ATTACK_RUNNING;
	
	digitalWrite(STA_LED, HIGH);

	if (currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL ||
		currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH ||
		currentWiFiAttackType == WIFI_ATTACK_KARMA) displayEvilPortalInfo();
	else if (currentWiFiAttackType == WIFI_ATTACK_DEAUTH_FLOOD) displayDeauthFloodInfo();
	else displayAttackStatus();
}

void startNRFJammer(NRFJammerMode jammer_mode) {
	String strmode;
	if (jammer_mode == Wifi) strmode = "Wifi";
	else if (jammer_mode == BLE) strmode = "BLE";
	else if (jammer_mode == Bluetooth) strmode = "Bluetooth";
	else if (jammer_mode == Zigbee) strmode = "Zigbee";
	else if (jammer_mode == RC) strmode = "RC";
	else if (jammer_mode == Video_Transmitter) strmode = "Video Transmitter";
	else if (jammer_mode == Usb_Wireless) strmode = "USB Wireless";
	else if (jammer_mode == Drone) strmode = "Drone";
	else if (jammer_mode == Full_Channel) strmode = "Full Channel 1 -> 100";

	Serial.println("[INFO] Starting Jamming | Mode: " + strmode);

	nrfJammerSetupOneShot = false;
	currentState = NRF24_JAMMER_RUNNING;

	digitalWrite(STA_LED, HIGH); 
}

void startBLEScan() {
	bleScanRunning = true;
	bleScanOneShot = false;

	bleScanInProgress = true;
	displayBLEScanMenu();
	bleScanOneShot = true;
	display.clearBuffer();
	ble.bleScan();
	//displayStatusBar(true);
	//ble.ShutdownBLE();
	//bleScanInProgress = false;

}

void startWiFiScan(WiFiGeneralItem mode) {
	wifiScanRunning = true;
	wifiScanOneShot = false;
	
	// Start AP scan
	if (mode == WIFI_GENERAL_AP_SCAN || mode == WIFI_GENERAL_AP_STA_SCAN) {
		wifiScanInProgress = true;
		display.clearBuffer();
		wifiScanOneShot = true;
		if (mode == WIFI_GENERAL_AP_SCAN) {
			displayWiFiScanMenu(WIFI_GENERAL_AP_SCAN);
			wifi.StartMode(WIFI_SCAN_AP);
		}
		else if (mode == WIFI_GENERAL_AP_STA_SCAN) {
			displayWiFiScanMenu(WIFI_GENERAL_AP_STA_SCAN);
			wifi.StartMode(WIFI_SCAN_AP_STA);
		}
	} else {
		wifiScanInProgress = true;
		displayWiFiScanMenu(WIFI_GENERAL_AP_SCAN_OLD);
		wifi.StartMode(WIFI_SCAN_AP_OLD);
		wifiScanOneShot = true;
		wifi.StartMode(WIFI_SCAN_OFF);
		wifiScanInProgress = false;
	}
	
	//wifi.StartMode(WIFI_SCAN_OFF);
	//wifiScanInProgress = false;
}

void startSnifferScan(WiFiGeneralItem sniffer_mode) {
	display.clearBuffer();
	if (sniffer_mode == WIFI_GENERAL_PROBE_REQ_SCAN) {
		wifi.StartMode(WIFI_SCAN_PROBE_REQ);
	}
	else if (sniffer_mode == WIFI_GENERAL_DEAUTH_SCAN) {
		wifi.StartMode(WIFI_SCAN_DEAUTH);
	}
	else if (sniffer_mode == WIFI_GENERAL_BEACON_SCAN) {
		wifi.StartMode(WIFI_SCAN_BEACON);
	}
	else if (sniffer_mode == WIFI_GENERAL_EAPOL_SCAN) {
		wifi.StartMode(WIFI_SCAN_EAPOL);
	}
	else if (sniffer_mode == WIFI_GENERAL_EAPOL_DEAUTH_SCAN) {
		wifi.StartMode(WIFI_SCAN_EAPOL_DEAUTH);
	}

}

void stopCurrentAttack() {
	Serial.println("[INFO] Stopping current attack");
	 
	if (currentState == BLE_ATTACK_RUNNING) {
		ble.ShutdownBLE();
	} else if (currentState == WIFI_ATTACK_RUNNING) {
		if (currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL ||
			currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH ||
			currentWiFiAttackType == WIFI_ATTACK_KARMA) {
			eportal.shutdownServer();
			evilPortalOneShot = false;
		}
		else if (currentWiFiAttackType == WIFI_ATTACK_DEAUTH_FLOOD) {
			wifi.deauth_flood_found_ap = false;
			wifi.deauth_flood_redraw = false;
			fixDeauthFloodDisplayLoop = false;
			wifi.deauth_flood_scan_one_shot = false;
			wifi.deauth_flood_target = "";
			delete deauth_flood_ap;
			deauth_flood_ap = new LinkedList<AccessPoint>(); // Free Memory
		}
		else {
			wifiAttackOneShot = false;
		}
		display.setFont(u8g2_font_ncenB08_tr);
		wifi.StartMode(WIFI_SCAN_OFF);
	}
		
	digitalWrite(STA_LED, LOW);
		
	// Add delay to ensure proper cleanup
	vTaskDelay(200 / portTICK_PERIOD_MS);
		
		// Return to appropriate menu
	if (currentState == BLE_ATTACK_RUNNING) {
		currentState = BLE_EXPLOIT_ATTACK_MENU;
		maxSelections = BLE_ATK_MENU_COUNT;
		displayExploitAttackBLEMenu();
	} else if (currentState == WIFI_ATTACK_RUNNING) {
		currentState = WIFI_ATTACK_MENU;
		maxSelections = WIFI_ATK_MENU_COUNT;
		displayWiFiAttackMenu();
	}
}

bool hasSelectedAPs() {
	if (!access_points || access_points->size() == 0) {
		return false;
	}
	
	for (int i = 0; i < access_points->size(); i++) {
		if (access_points->get(i).selected) {
			return true;
		}
	}
	return false;
}

bool hasSelectedSTAs() {
	if (!device_station || device_station->size() == 0) {
		return false;
	}
	
	for (int i = 0; i < access_points->size(); i++) {
		if (access_points->get(i).selected) {
			for (int x = 0; x < access_points->get(i).stations->size(); x++) {
				if (device_station->get(access_points->get(i).stations->get(x)).selected) {
					return true;
				}
			}
		}
	}
	return false;
}

bool hasSelectedProbeReqSSID() {
	if (!probe_req_ssids || probe_req_ssids->size() == 0) {
		return false;
	}
	
	for (int i = 0; i < probe_req_ssids->size(); i++) {
		if (probe_req_ssids->get(i).selected) {
			return true;
		}
	}
	return false;
}


void performReboot() {
	Serial.println("[SYSTEM_REBOOT] Performing system reboot...");
	
	// Stop any running attacks or scans
	if (wifiScanRunning) {
		wifiScanRunning = false;
	}

	if (bleScanRunning) {
		bleScanRunning = false;
	}
	
	// Shutdown BLE
	if (ble_initialized) {
		ble.ShutdownBLE();

	}
	
	// Shutdown WiFi
	if (wifi_initialized) {
		wifi.StartMode(WIFI_SCAN_OFF);
	}
	
	currentState = MAIN_MENU;

	// Display reboot message
	display.clearScreen();
	display.sendDisplay();
	display.displayStringwithCoordinates("REBOOTING...", 0, 12);
	display.displayStringwithCoordinates("Please wait...", 0, 21, true);
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	
	// Restart ESP32
	ESP.restart();
}

void performDeepSleep() {
	if (autoSleep)
	Serial.println("[SYSTEM_REBOOT] Forcing Performing Deep Sleep...");
	else 
	Serial.println("[SYSTEM_REBOOT] User Performing Deep Sleep...");
	
	// Stop any running attacks or scans
	if (wifiScanRunning) {
		wifiScanRunning = false;
	}

	if (bleScanRunning) {
		bleScanRunning = false;
	}
	
	// Shutdown BLE
	if (ble_initialized) {
		ble.ShutdownBLE();
	}
	
	// Shutdown WiFi
	if (wifi_initialized) {
		wifi.StartMode(WIFI_SCAN_OFF);
	}

	currentState = MAIN_MENU;
	
	// Display reboot message
	display.clearScreen();
	display.displayStringwithCoordinates("Deep Sleep", 0, 12);

	display.displayStringwithCoordinates("Tip: Press Sel", 0, 24);
	display.displayStringwithCoordinates("to wake up", 0, 36);
	display.displayStringwithCoordinates("device", 0, 48, true);
	if (autoSleep) display.displayStringwithCoordinates("Forcing by System", 0, 60, true);
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	display.clearScreen();
	display.sendDisplay();
	// Restart ESP32
	esp_deep_sleep_enable_gpio_wakeup(1 << ENC_BTN, ESP_GPIO_WAKEUP_GPIO_LOW);
	esp_deep_sleep_start();
}

void handleInput(MenuState handle_state) {
	// Handle Input
	if (check(selPress)) {
		if ((wifiScanRunning && handle_state == WIFI_SCAN_RUNNING) ||
			handle_state == WIFI_SCAN_SNIFFER_RUNNING ||
			(bleScanRunning && handle_state == BLE_SCAN_RUNNING) ||
			handle_state == NRF24_ANALYZER_RUNNING ||
			handle_state == NRF24_JAMMER_RUNNING ||
			handle_state == NRF24_SCANNER_RUNNING ||
			handle_state == WIFI_ATTACK_RUNNING ||
			handle_state == BLE_ATTACK_RUNNING ||
			handle_state == BLE_SPOOFER_RUNNING ||
			handle_state == IR_SEND_RUNNING)
		{
			goBack();
		}
		else {
			selectCurrentItem();
		}
	}

	if (check(prevPress)) {
		if ((handle_state == WIFI_SCAN_RUNNING && !wifiScanOneShot) ||
			(handle_state == BLE_SCAN_RUNNING && !bleScanOneShot))
		{
			goBack();
		}
		else if (wifiSnifferMode == WIFI_GENERAL_EAPOL_SCAN || wifiSnifferMode == WIFI_GENERAL_EAPOL_DEAUTH_SCAN) {
			if (wifi.set_channel < 2) wifi.set_channel = 14;
			else wifi.set_channel = wifi.set_channel - 1;
			wifi.changeChannel();
			display_buffer->add("Change Ch to: " + String(wifi.set_channel));
			Serial.println("[INFO] Manually change channel to " + String(wifi.set_channel));
			wifiScanRedraw = true;
		}
		else {
			if (handleStateRunningCheck) navigateUp();
		}
	}

	if (check(nextPress)) {
		if (wifiSnifferMode == WIFI_GENERAL_EAPOL_SCAN || wifiSnifferMode == WIFI_GENERAL_EAPOL_DEAUTH_SCAN) {
			if (wifi.set_channel > 13) wifi.set_channel = 1;
			else wifi.set_channel = wifi.set_channel + 1;
			wifi.changeChannel();
			display_buffer->add("Change Ch to: " + String(wifi.set_channel));
			Serial.println("[INFO] Manually change channel to " + String(wifi.set_channel));
			wifiScanRedraw = true;
		}
		else {
			if (handleStateRunningCheck) navigateDown();
		}
	}
}

void handleTasks(MenuState handle_state) {
	// Handle WiFi scanning
	if (wifiScanRunning && handle_state == WIFI_SCAN_RUNNING) {

		if (!wifiScanOneShot) {
			if (wifiSnifferMode == WIFI_GENERAL_AP_SCAN) {
				startWiFiScan(WIFI_GENERAL_AP_SCAN);
			}
			else {
				startWiFiScan(WIFI_GENERAL_AP_STA_SCAN);
			}
		}
		if (wifiSnifferMode == WIFI_GENERAL_AP_SCAN || wifiSnifferMode == WIFI_GENERAL_AP_SCAN_OLD) {
			if (!wifiScanDisplay && !wifiScanInProgress) displayWiFiScanMenu(WIFI_GENERAL_AP_SCAN);
		}
		else {
			if (!wifiScanDisplay && !wifiScanInProgress) displayWiFiScanMenu(WIFI_GENERAL_AP_STA_SCAN);
		}

		if (wifiScanInProgress && wifiSnifferMode != WIFI_GENERAL_AP_SCAN_OLD) {
			if (wifiScanRedraw) {
				wifiScanRedraw = false;
				display.clearScreen();
				displayStatusBar();
				display.displayBuffer();
			}
		}

		if (wifiSnifferMode != WIFI_GENERAL_AP_SCAN_OLD) {
			static unsigned long channelhoptimer = 0;
			if (wifiScanRunning && wifiScanInProgress) {
				if (millis() - channelhoptimer > 500) {
					wifi.channelHop();
					channelhoptimer = millis();
				}
			}
		}
	}

	else if (handle_state == WIFI_SCAN_SNIFFER_RUNNING) {
		if (wifiScanRedraw) {
			wifiScanRedraw = false;
			display.clearScreen();
			displayStatusBar();
			display.displayBuffer();
		}

		if (wifiSnifferMode != WIFI_GENERAL_EAPOL_SCAN && wifiSnifferMode != WIFI_GENERAL_EAPOL_DEAUTH_SCAN) {
			static unsigned long channelhoptimer = 0;
			if (millis() - channelhoptimer > 500) {
				wifi.channelHop();
				channelhoptimer = millis();
			}
		}
	}

	else if (bleScanRunning && handle_state == BLE_SCAN_RUNNING) {

		if (!bleScanOneShot) startBLEScan();

		if (!bleScanDisplay && !bleScanInProgress) displayBLEScanMenu();

		if (bleScanInProgress) {
			if (bleScanRedraw) {
				bleScanRedraw = false;
				display.clearScreen();
				displayStatusBar();
				display.displayBuffer();
			}
		}
	}

	else if (handle_state == NRF24_ANALYZER_RUNNING) {
		if (!nrfAnalyzerSetupOneShot) {
			Serial.println("[INFO] Starting NRF Analyzer");
			display.clearScreen();
			display.sendDisplay();
			nrf.analyzerSetup();
			nrfAnalyzerSetupOneShot = true;
		}
		nrfAnalyzer();
	}

	else if (handle_state == NRF24_JAMMER_RUNNING) {
		if (!nrfJammerSetupOneShot) {
			Serial.println("[INFO] Starting NRF Jammer");
			nrf.jammerNRFRadioSetup();
			nrfJammerSetupOneShot = true;
		}
		//while(!check(selPress)) {
			nrf.jammerNRFRadioMain(currentNRFJammerMode);
		//}
		if (check(selPress)) {
			Serial.println("[INFO] NRF24 Jammer stopped by user.");
			goBack();
		}
	}

	else if (handle_state == NRF24_SCANNER_RUNNING) {
		if (!nrfScannerSetupOneShot) {
			Serial.println("[INFO] Starting NRF Scanner");
			display.clearScreen();
			display.sendDisplay();
			nrf.scannerSetup();
			nrfScannerSetupOneShot = true;
		}
		//while(!check(selPress)) {
			nrfScanner();
		//}
		if (check(selPress)) {
			Serial.println("[INFO] NRF24 Scanner stopped by user.");
			goBack();
		}
	}

	else if (handle_state == IR_SEND_RUNNING) {
		if (starttvbgone) {
			Serial.println("[INFO] Starting IR TV-B-Gone");
			String region;
			if (irTvBGoneRegion == NA) {
				region = "NA";
				begoneregion = NA;
			}
			else {
				region = "EU";
				begoneregion = EU;
			}
			display.clearScreen();
			displayStatusBar();
			display.displayStringwithCoordinates("TV-B-Gone Mode", 0, 24);
			display.displayStringwithCoordinates("Region:" + region, 0,36);
			display.displayStringwithCoordinates("Press Sel to stop", 0, 48, true);
			irtx.startTVBGone();
			display.clearScreen();
			displayStatusBar();
			display.displayStringwithCoordinates("All Codes", 0, 24);
			display.displayStringwithCoordinates("Sended", 0, 36);
			display.displayStringwithCoordinates("Total:" + String(irtx.begone_code_sended),0, 48);
			display.displayStringwithCoordinates("Press Sel to exit", 0, 60, true);
			while(!check(selPress)) yield();
			Serial.println("[INFO] IR TV-B-Gone Done! Total: " + String(irtx.begone_code_sended) + " codes sended.");
			starttvbgone = false;
			irtx.begone_code_sended = 0;
			goBack();
		}
	}

	else if (handle_state == BLE_SPOOFER_RUNNING) {
		if (!bleSpooferDone) {
			if (bleSpooferBrandType == BLE_SPO_BRAND_APPLE)
				ble.startSpoofer(ble_spoofer_device, BLE_SPOOFER_DEVICE_BRAND_APPLE, ble_spoofer_ad_type);
			else if (bleSpooferBrandType == BLE_SPO_BRAND_SAMSUNG)
				ble.startSpoofer(ble_spoofer_device, BLE_SPOOFER_DEVICE_BRAND_SAMSUNG, ble_spoofer_ad_type);
			else if (bleSpooferBrandType == BLE_SPO_BRAND_GOOGLE)
				ble.startSpoofer(ble_spoofer_device, BLE_SPOOFER_DEVICE_BRAND_GOOGLE, ble_spoofer_ad_type);
			displaySpooferRunning();
			bleSpooferDone = true;
		}
	}
	
	// If attack is running, execute attack periodically and update display
	else if (handle_state == BLE_ATTACK_RUNNING || handle_state == WIFI_ATTACK_RUNNING) {
		static unsigned long lastDisplayUpdate = 0;
		static unsigned long lastMemoryCheck = 0;

		if (handle_state == BLE_ATTACK_RUNNING) {
			// BLE attack handling...
			switch(currentBLEAttackType) {
				case BLE_ATTACK_SOUR_APPLE:
					ble.executeSwiftpair(SourApple);
					break;
				case BLE_ATTACK_APPLE_JUICE:
					ble.executeSwiftpair(AppleJuice);
					break;
				case BLE_ATTACK_MICROSOFT:
					ble.executeSwiftpair(Microsoft);
					break;
				case BLE_ATTACK_SAMSUNG:
					ble.executeSwiftpair(Samsung);
					break;
				case BLE_ATTACK_GOOGLE:
					ble.executeSwiftpair(Google);
					break;
			}
		} 
		else if (handle_state == WIFI_ATTACK_RUNNING) {
			// SỬA: Tách biệt xử lý Evil Portal và các WiFi attack khác
			if (currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL ||
				currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH || 
				currentWiFiAttackType == WIFI_ATTACK_KARMA) {
				// Evil Portal handling

				if (!evilPortalOneShot) {
					bool _ap = false;
					Serial.println("[INFO] Starting Setup Portal");
					if (currentWiFiAttackType == WIFI_ATTACK_KARMA) {
						for (int i = 0; i < probe_req_ssids->size(); i++) {
							if (probe_req_ssids->get(i).selected) {
								_ap = eportal.apSetup(probe_req_ssids->get(i).essid, false);
								Serial.println("[INFO] Karma Mode Enable");
								break;
							}
						}
					} else {
						if (currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH) {
							_ap = eportal.apSetup("", true);
						} else {
							_ap = eportal.apSetup("", false);
						}
					}
					
					bool _html = eportal.htmlSetup();
					vTaskDelay(1000 / portTICK_PERIOD_MS);
					if (_ap && _html) {
						eportal.apStart();
						displayEvilPortalInfo();
						display.setFont(u8g2_font_6x10_tf);
						display.displayEvilPortalText(eportal.get_user_name(), eportal.get_password());
					}
					else {
						display.displayString("Failed To Init", true);
						display.displayString("Initialized Portal", true, true);
					}
					evilPortalOneShot = true;
				}

				if (eportal.password_received && eportal.name_received){
					display.displayEvilPortalText(eportal.get_user_name(), eportal.get_password());
				}
				eportal.loop();
				
				// Deauth handling for Evil Portal + Deauth
				if (currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH) {
					static unsigned long lastDeauthTime = 0;
					if (millis() - lastDeauthTime > 250) {
						esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame, 26, false);
						vTaskDelay(1 / portTICK_RATE_MS);
						esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame, 26, false);
						vTaskDelay(1 / portTICK_RATE_MS);
						esp_wifi_80211_tx(WIFI_IF_AP, deauth_frame, 26, false);
						vTaskDelay(1 / portTICK_RATE_MS);
						lastDeauthTime = millis();
					}
				}
			}
			else if (currentWiFiAttackType == WIFI_ATTACK_DEAUTH_FLOOD) {
				displayDeauthFloodInfo();
				wifi.StartDeauthFlood();
			}
			else {
				// Regular WiFi attacks
				if (!wifiAttackOneShot) {
					wifi.StartMode(currentWiFiAttackType);
					wifiAttackOneShot = true;
				}
				wifi.mainAttackLoop(currentWiFiAttackType);
			}
		}

		// Update display every 1 second (not using with evil portal)
		if (currentWiFiAttackType != WIFI_ATTACK_EVIL_PORTAL &&
			currentWiFiAttackType != WIFI_ATTACK_EVIL_PORTAL_DEAUTH &&
			currentWiFiAttackType != WIFI_ATTACK_DEAUTH_FLOOD &&
			currentWiFiAttackType != WIFI_ATTACK_KARMA) {
			if (millis() - lastDisplayUpdate > 2000) {
				displayAttackStatus();
				wifi.packet_sent = 0;
				lastDisplayUpdate = millis();
			}
		}
	}
}

static unsigned long autoSleepTimer = 0;

void redrawTasks() {
	switch(currentState) {
		case MAIN_MENU:
			displayMainMenu();
			break;
		case BLE_MENU:
			displayBLEMenu();
			break;
		case BLE_INFO_MENU_LIST:
			displayBLEInfoListMenu();
			break;
		case BLE_INFO_MENU_DETAIL:
			displayBLEInfoDetail();
			break;
		case BLE_SPOOFER_MAIN_MENU:
			displayMainSpooferMenu();
			break;
		case BLE_SPOOFER_APPLE_MENU:
			displayAppleSpooferMenu();
			break;
		case BLE_SPOOFER_SAMSUNG_MENU:
			displaySamsungSpooferMenu();
			break;
		case BLE_SPOOFER_GOOGLE_MENU:
			displayGoogleSpooferMenu();
			break;
		case BLE_SPOOFER_AD_TYPE_MENU:
			displayAdTypeSpooferMenu();
			break;
		case BLE_EXPLOIT_ATTACK_MENU:
			displayExploitAttackBLEMenu();
			break;
		case WIFI_MENU:
			displayWiFiMenu();
			break;
		case WIFI_GENERAL_MENU:
			displayWiFiGeneralMenu();
			break;
		case WIFI_UTILS_MENU:
			displayWiFiUtilsMenu();
			break;
		case WIFI_ATTACK_MENU:
			displayWiFiAttackMenu();
			break;
		case WIFI_SELECT_MENU:
			displayWiFiSelectMenu();
			break;
		case WIFI_SELECT_PROBE_REQ_SSIDS_MENU:
			displayWiFiSelectProbeReqSsidsMenu();
			break;
		case WIFI_SELECT_STA_AP_MENU:
			displayWiFiSelectAptoSta();
			break;
		case WIFI_SELECT_STA_MENU:
			displayWiFiSelectStaInAp();
			break;
		case NRF24_MENU:
			displayNRF24Menu();
			break;
		case NRF24_JAMMER_MENU:
			displayNRF24JammerMenu();
			break;
		case IR_MENU:
			displayIRMenu();
			break;
		case IR_TV_B_GONE_REGION:
			displayIRTvBGoneRegionMenu();
			break;
	}
}

void autoSleepCheck() {
	if (handleStateRunningCheck)  // prevent into deep sleep mode when in IR send mode
	{
		if (!selPress && !prevPress && !nextPress) { // auto deep sleep
			if (autoSleepTimer == 0) { 
				autoSleepTimer = millis();
			}
		} else {
			autoSleepTimer = 0;
		}
		if (autoSleepTimer > 0 && (millis() - autoSleepTimer) > 300000) { // 5 mins
			autoSleep = true;
			if (standby) standby = false;
			performDeepSleep();
		}
		if (autoSleepTimer > 0 && (millis() - autoSleepTimer) > 30000) { // 30 secs
			if (!standby) standby = true;
		}
	} else {
		autoSleepTimer = millis();
	}
}

void menuloop() {
	handleStateRunningCheck = !(currentState == WIFI_SCAN_RUNNING) && // prevent into deep sleep mode when in attack mode 
							!(currentState == WIFI_SCAN_SNIFFER_RUNNING) &&
							!(currentState == BLE_SCAN_RUNNING) &&
							!(currentState == NRF24_ANALYZER_RUNNING) &&
							!(currentState == NRF24_JAMMER_RUNNING) &&
							!(currentState == NRF24_SCANNER_RUNNING) &&
							!(currentState == WIFI_ATTACK_RUNNING) &&
							!(currentState == BLE_ATTACK_RUNNING) &&
							!(currentState == BLE_SPOOFER_RUNNING) &&
							!(currentState == IR_SEND_RUNNING);
	autoSleepCheck();
	handleInput(currentState);
	handleTasks(currentState);
	if (standby) {
		for (int i = 0; i < standby_bitmap_allArray_LEN; i++) {
			if (check(selPress) || check(prevPress) || check(nextPress)) { // auto deep sleep
				if (standby) standby = false;
				autoSleepTimer = millis();
				redrawTasks();
				break;
			}
			display.clearScreen();
			display.drawBipmap(0, 0, 128, 64, standby_bitmap_allArray[i], true);
			vTaskDelay(20 / portTICK_PERIOD_MS);
		}
	}
	// Check for critical low memory and auto-reboot
	if (getHeap(GET_FREE_HEAP) < MEM_LOWER_LIM) { // Critical low memory threshold
		if (!low_memory_warning)
		Serial.println("[SYSTEM_REBOOT] Critical low memory detected! Stop add to buffer...");
		low_memory_warning = true;
			//display.clearScreen();
			//display.displayStringwithCoordinates("CRITICAL LOW MEM!", 0, 12);
			//display.displayStringwithCoordinates("AUTO REBOOTING...", 0, 21, true);
			//vTaskDelay(2000 / portTICK_PERIOD_MS);
			//wifi.StartMode(WIFI_SCAN_OFF);
			//ble.ShutdownBLE();
			//nrf.shutdownNRFJammer();
			//nrf.shutdownNRF();
			// Ensure all tasks are stopped before rebooting
			//ESP.restart();
	} else {
		low_memory_warning = false;
	}
	
}

#pragma GCC diagnostic pop