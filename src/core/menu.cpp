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


// https://github.com/pr3y/Bruce/blob/main/src/main.cpp
void __attribute__((weak)) taskHandleInput(void *parameter) {
	auto timer = millis();
	while (true) {
		if (millis() - timer > 50) {
			nextPress = false;
			prevPress = false;
			selPress = false;
			handleInputs();
			timer = millis();
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void menuinit() {
	ESP32Encoder::useInternalWeakPullResistors = puType::up;
	encoder.attachHalfQuad(ENC_PIN_A, ENC_PIN_B);
	encoder.setCount(0);
   // Setup encoder button pin
	pinMode(ENC_BTN, INPUT_PULLUP);

	if (!display.main()) {
		Serial.println("[ERROR] Failed to initialize display!");
		while(1); // Halt if display fails
	}
	xTaskCreate(taskHandleInput, "HandleInput", 4096, NULL, 2, &xHandle);
	wifi.main();
	ble.main();
	eportal.setup();
	wifi.StartMode(WIFI_SCAN_OFF);
	Serial.println("[INFO] Menu system initialized");
	displayWelcome();
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	displayMainMenu();
}

void displayWelcome() {
	display.clearScreen();
	String title = "ESP32 Attack Tool";
	String version_num = ATTACK_TOOL_VERSION;
	int yTitle = SCR_HEIGHT / 2 - 5;
	int yVersion = SCR_HEIGHT / 2 + 5;
	display.drawingCenterString(title, yTitle);
	display.drawingCenterString(version_num, yVersion, true);
	vTaskDelay(1300 / portTICK_PERIOD_MS);
	display.clearScreen();
	String footer1 = "Code By";
	String footer2 = "@Ohminecraft";
	display.drawingCenterString(footer1, yTitle);
	display.drawingCenterString(footer2, yVersion, true);
	display.displayInvert(true);
	vTaskDelay(1300 / portTICK_PERIOD_MS);
	display.displayInvert(false);
	display.clearScreen();
	String readyText1 = "Welcome Hacker";
	String readyText2 = "Home!";
	display.drawingCenterString(readyText1, yTitle);
	display.drawingCenterString(readyText2, yVersion, true);
}

void displayMainMenu() {
	display.clearScreen();
	display.displayStringwithCoordinates("==== MAIN MENU ====", 0, 12);
	
	String items[MAIN_MENU_COUNT] = {
		"BLE",
		"WiFi",
		"NRF24",
		"Reboot"
	};
	
	// Display only MAX_SHOW_SECLECTION items at a time due to screen height
	uint8_t startIndex = (currentSelection >= MAX_SHOW_SECLECTION) ? currentSelection - 1 : 0;
	
	for(int i = 0; i < MAX_SHOW_SECLECTION && (startIndex + i) < MAIN_MENU_COUNT; i++) {
		String itemText = ((startIndex + i) == currentSelection) ? 
			"> " + items[startIndex + i] : "  " + items[startIndex + i];
		display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
	}
	display.sendDisplay();
}

void displayBLEScanMenu() {
	display.clearScreen();
	display.displayStringwithCoordinates("==== BLE SCAN =====", 0, 12, true);
	if (bleScanRunning) {
		if (bleScanInProgress) {
			display.displayStringwithCoordinates("Scanning...", 0, 24, true);
		} else {
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
	display.clearScreen();
	display.displayStringwithCoordinates("===== BLE MENU =====", 0, 12);

	String items[BLE_MENU_COUNT] = {
		"BLE Scan",
		"BLE Info",
		"BLE Spoofer",
		"BLE Exploit Atk",
		"< Back"
	};

	// Display only MAX_SHOW_SECLECTION items at a time due to screen height
	uint8_t startIndex = (currentSelection >= MAX_SHOW_SECLECTION) ? currentSelection - 1 : 0;
	
	for(int i = 0; i < MAX_SHOW_SECLECTION && (startIndex + i) < BLE_MENU_COUNT; i++) {
		String itemText = ((startIndex + i) == currentSelection) ? 
			"> " + items[startIndex + i] : "  " + items[startIndex + i];
		display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
	}
	display.sendDisplay();
}

void displayBLEInfoListMenu() {
	display.clearScreen();
	display.displayStringwithCoordinates("===== BLE INFO =====", 0, 12, true);

	if (!blescanres || blescanres->size() == 0) {
		display.displayStringwithCoordinates("No Devices found!", 0, 24);
		display.displayStringwithCoordinates("Scan first", 0, 36);
		
		// Show only back option if haven't AP exists
		String backText = (currentSelection == 0) ? "> Back" : "  Back";
		display.displayStringwithCoordinates(backText, 0, 48, true);
		return;
	}
	
	// show AP + option Back
	int totalItems = blescanres->size() + 1;
	
	if (currentSelection < blescanres->size()) {

		BLEScanResult bledevice = blescanres->get(currentSelection);
		String ble_title = bledevice.name;
		if (ble_title == "<no name>") ble_title = bledevice.addr.toString().c_str();
		// Truncate if too long
		if (ble_title.length() > 17) {
			ble_title = ble_title.substring(0, 18) + "...";
		}
		
		display.displayStringwithCoordinates("> " + ble_title, 0, 24, true);
	} else {
		display.displayStringwithCoordinates("> Back", 0, 48, true);
		if (blescanres->size() > 0) {
			display.displayStringwithCoordinates("Devices: " + String(blescanres->size()), 0, 24, true);
		}
	}
}

void displayBLEInfoDetail() {
	display.clearScreen();
	display.displayStringwithCoordinates("==== BLE DETAIL ====", 0, 12, true);

	if (!blescanres || currentSelection >= blescanres->size()) {
		display.displayStringwithCoordinates("Invalid device", 0, 24);
		return;
	}

	BLEScanResult res = blescanres->get(currentSelection);
	String nameStr = res.name;
	String addrStr = res.addr.toString().c_str();
	String rssiStr = "RSSI: " + String(res.rssi);

	display.displayStringwithCoordinates(nameStr, 0, 24);
	display.displayStringwithCoordinates("Addr:" + addrStr, 0, 36);
	display.displayStringwithCoordinates(rssiStr, 0, 48, true);
}

void displayMainSpooferMenu() {
	digitalWrite(STA_LED, LOW);
	display.clearScreen();
	display.displayStringwithCoordinates("== BLE SPOOFER ATK ==", 0, 12);

	String items[BLE_SPOOFER_COUNT] = {
		"Apple",
		"Samsung",
		"Google",
		"< Back"
	};

	// Display only MAX_SHOW_SECLECTION items at a time due to screen height
	uint8_t startIndex = (currentSelection >= MAX_SHOW_SECLECTION) ? currentSelection - 1 : 0;
	
	for(int i = 0; i < MAX_SHOW_SECLECTION && (startIndex + i) < BLE_SPOOFER_COUNT; i++) {
		String itemText = ((startIndex + i) == currentSelection) ? 
			"> " + items[startIndex + i] : "  " + items[startIndex + i];
		display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
	}
	display.sendDisplay();
}

void displayAppleSpooferMenu() {
	digitalWrite(STA_LED, LOW);
	display.clearScreen();
	display.displayStringwithCoordinates("= BLE APPLE SPO ATK =", 0, 12);

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

	uint8_t startIndex = (currentSelection >= MAX_SHOW_SECLECTION) ? currentSelection - 1 : 0;
	
	for(int i = 0; i < MAX_SHOW_SECLECTION && (startIndex + i) < BLE_SPO_APPLE_COUNT; i++) {
		String itemText = ((startIndex + i) == currentSelection) ? 
			"> " + items[startIndex + i] : "  " + items[startIndex + i];
		display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
	}
	display.sendDisplay();
}

void displaySamsungSpooferMenu() {
	digitalWrite(STA_LED, LOW);
	display.clearScreen();
	display.displayStringwithCoordinates("== BLE SAMSUNG SPO ==", 0, 12);

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

	uint8_t startIndex = (currentSelection >= MAX_SHOW_SECLECTION) ? currentSelection - 1 : 0;

	for(int i = 0; i < MAX_SHOW_SECLECTION && (startIndex + i) < BLE_SPO_SAMSUNG_COUNT; i++) {
		String itemText = ((startIndex + i) == currentSelection) ?
			"> " + items[startIndex + i] : "  " + items[startIndex + i];
		display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
	}

	display.sendDisplay();
}


void displayGoogleSpooferMenu() {
	digitalWrite(STA_LED, LOW);
	display.clearScreen();
	display.displayStringwithCoordinates("== BLE GOOGLE SPOO ==", 0, 12);

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
	uint8_t startIndex = (currentSelection >= MAX_SHOW_SECLECTION) ? currentSelection - 1 : 0;

	for(int i = 0; i < MAX_SHOW_SECLECTION && (startIndex + i) < BLE_SPO_GOOGLE_COUNT; i++) {
		String itemText = ((startIndex + i) == currentSelection) ?
			"> " + items[startIndex + i] : "  " + items[startIndex + i];
		display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
	}

	display.sendDisplay();
}


void displayAdTypeSpooferMenu() {
	digitalWrite(STA_LED, LOW);
	display.clearScreen();
	display.displayStringwithCoordinates("== BLE SELECT AD TYPE ==", 0, 12);

	String items[BLE_SPO_AD_TYPE_COUNT] = {
		"Type NON",
		"Type DIR",
		"Type UND",
		"< Back"
	};

	uint8_t startIndex = (currentSelection >= MAX_SHOW_SECLECTION) ? currentSelection - 1 : 0;
	
	for(int i = 0; i < MAX_SHOW_SECLECTION && (startIndex + i) < BLE_SPO_AD_TYPE_COUNT; i++) {
		String itemText = ((startIndex + i) == currentSelection) ? 
			"> " + items[startIndex + i] : "  " + items[startIndex + i];
		display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
	}
	display.sendDisplay();
}

void displaySpooferRunning() {
	digitalWrite(STA_LED, HIGH);
	display.clearScreen();
	display.displayStringwithCoordinates("ADVERTISING...", 0, 12, true);
}

void displayExploitAttackBLEMenu() {
	digitalWrite(STA_LED, LOW);
	display.clearScreen();
	display.displayStringwithCoordinates("== BLE EXPLOIT ATK ==", 0, 12);
	
	String items[BLE_ATK_MENU_COUNT] = {
		"Sour Apple",
		"Apple Juice",
		"Swiftpair MS", 
		"Samsung Spam",
		"Google Spam",
		"< Back"
	};
	
	// Display only MAX_SHOW_SECLECTION items at a time due to screen height
	uint8_t startIndex = (currentSelection >= MAX_SHOW_SECLECTION) ? currentSelection - 1 : 0;
	
	for(int i = 0; i < MAX_SHOW_SECLECTION && (startIndex + i) < BLE_ATK_MENU_COUNT; i++) {
		String itemText = ((startIndex + i) == currentSelection) ? 
			"> " + items[startIndex + i] : "  " + items[startIndex + i];
		display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
	}
	display.sendDisplay();
}

void displayWiFiMenu() {
	display.clearScreen();
	display.displayStringwithCoordinates("==== WIFI MENU =====", 0, 12);
	
	String items[WIFI_MENU_COUNT] = {
		"Scan WiFi",
		"Select WiFi",
		"Attack WiFi",
		"< Back"
	};
	
	// Display only MAX_SHOW_SECLECTION items at a time due to screen height
	uint8_t startIndex = (currentSelection >= MAX_SHOW_SECLECTION) ? currentSelection - 1 : 0;
	
	for(int i = 0; i < MAX_SHOW_SECLECTION && (startIndex + i) < WIFI_MENU_COUNT; i++) {
		String itemText = ((startIndex + i) == currentSelection) ? 
			"> " + items[startIndex + i] : "  " + items[startIndex + i];
		display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
	}
	display.sendDisplay();
}

void displayWiFiScanMenu() {
	display.clearScreen();
	display.displayStringwithCoordinates("==== WIFI SCAN =====", 0, 12, true);
	if (wifiScanRunning) {
		if (wifiScanInProgress) {
			display.displayStringwithCoordinates("Scanning...", 0, 24, true);
		} else {
				wifiScanDisplay = true;
				display.displayStringwithCoordinates("Scan complete", 0, 24);
				display.displayStringwithCoordinates("SELECT->back", 0, 36);
				display.displayStringwithCoordinates("Found: " + String(access_points ? access_points->size() : 0), 0, 48, true);
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
	display.clearScreen();
	display.displayStringwithCoordinates("==== WIFI SELECT ====", 0, 12, true);
	
	if (!access_points || access_points->size() == 0) {
		display.displayStringwithCoordinates("No APs found!", 0, 24);
		display.displayStringwithCoordinates("Scan first", 0, 36);
		
		// Show only back option if haven't AP exists
		String backText = (currentSelection == 0) ? "> Back" : "  Back";
		display.displayStringwithCoordinates(backText, 0, 48, true);
		return;
	}
	
	// show AP + option Back
	int totalItems = access_points->size() + 1;
	
	if (currentSelection < access_points->size()) {

		AccessPoint ap = access_points->get(currentSelection);
		String status = ap.selected ? "[*] " : "[ ] ";
		String apInfo = status + ap.essid;
		char bssidStr[18];

		snprintf(bssidStr, sizeof(bssidStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
		ap.bssid[0], ap.bssid[1], ap.bssid[2], 
		ap.bssid[3], ap.bssid[4], ap.bssid[5]);
		
		// Truncate if too long
		if (apInfo.length() > 14) {
			apInfo = apInfo.substring(0, 18) + "...";
		}
		
		display.displayStringwithCoordinates("> " + apInfo, 0, 24);
		display.displayStringwithCoordinates("Ch:" + String(ap.channel) + " R:" + String(ap.rssi), 0, 36);
		display.displayStringwithCoordinates("B:" + String(bssidStr), 0, 48);
		display.displayStringwithCoordinates("Enc:" + ap.wpastr, 0, 60, true);
	} else {
		display.displayStringwithCoordinates("> Back", 0, 48, true);
		if (access_points->size() > 0) {
			display.displayStringwithCoordinates("APs: " + String(access_points->size()), 0, 24, true);
		}
	}
}

void displayWiFiAttackMenu() {
	display.clearScreen();
	display.displayStringwithCoordinates("==== WIFI ATTACK ====", 0, 12, true);
	
	String items[WIFI_ATK_MENU_COUNT] = {
		"Deauth Tar Attack",
		"Deauth Flood",
		"Probe Attack",
		"Rickroll Beacon",
		"Stable SSID Beacon",
		"Random Beacon",
		"Evil Portal",
		"Evil Portal Deauth",
		"< Back"
	};
	
	// Display only MAX_SHOW_SECLECTION items at a time due to screen height
	uint8_t startIndex = (currentSelection >= MAX_SHOW_SECLECTION) ? currentSelection - 1 : 0;
	
	for(int i = 0; i < MAX_SHOW_SECLECTION && (startIndex + i) < WIFI_ATK_MENU_COUNT; i++) {
		String itemText = ((startIndex + i) == currentSelection) ? 
			"> " + items[startIndex + i] : "  " + items[startIndex + i];
		display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
	}
	display.sendDisplay();
}

void displayRebootConfirm() {
	display.clearScreen();
	display.displayStringwithCoordinates("====== REBOOT ======", 0, 12);
	display.displayStringwithCoordinates("Press SELECT to", 0, 24);
	display.displayStringwithCoordinates("confirm reboot", 0, 36);
	display.displayStringwithCoordinates("or LEFT to cancel", 0, 48, true);
}

void displayNRF24Menu() {
	display.clearScreen();
	display.displayStringwithCoordinates("==== NRF24 MENU ====", 0, 12, true);

	String items[NRF24_MENU_COUNT] = {
		"Analyzer",
		"Scanner",
		"Jammer",
		"< Back"
	};

	uint8_t startIndex = (currentSelection >= MAX_SHOW_SECLECTION) ? currentSelection - 1 : 0;
	
	for(int i = 0; i < MAX_SHOW_SECLECTION && (startIndex + i) < NRF24_MENU_COUNT; i++) {
		String itemText = ((startIndex + i) == currentSelection) ? 
			"> " + items[startIndex + i] : "  " + items[startIndex + i];
		display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
	}
	display.sendDisplay();
}

void displayNRF24JammerMenu() {
	display.clearScreen();
	display.displayStringwithCoordinates("=== NRF24 JAMMER ===", 0, 12, true);

	String items[NRF24_JAM_MENU_COUNT] = {
		"WiFi",
		"BLE",
		"Bluetooth",
		"Zigbee",
		"RC",
		"Video Transmitter",
		"USB Wireless",
		"Full Channel",
		"< Back"
	};

	uint8_t startIndex = (currentSelection >= MAX_SHOW_SECLECTION) ? currentSelection - 1 : 0;
	
	for(int i = 0; i < MAX_SHOW_SECLECTION && (startIndex + i) < NRF24_JAM_MENU_COUNT; i++) {
		String itemText = ((startIndex + i) == currentSelection) ? 
			"> " + items[startIndex + i] : "  " + items[startIndex + i];
		display.displayStringwithCoordinates(itemText, 0, 24 + (i * 12));
	}
	display.sendDisplay();
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
					display.displayStringwithCoordinates(access_points->get(i).essid, 0, 24, true);
					display.displayStringwithCoordinates("Enable Deauth Sel AP", 0, 48, true);
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

	size_t freeHeap = esp_get_free_heap_size();
	if (freeHeap <= MEM_LOWER_LIM + 2000) {
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
	display.clearScreen();
	String attackName = "";
	
	display.displayStringwithCoordinates("NRF24 JAMMER", 0, 12);
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
		case Full_Channel:
			attackName = "Full Channel";
			break;
	}
	
	display.displayStringwithCoordinates("Mode: " + attackName, 0, 36);
	
	// Hiển thị thông tin thêm
	display.displayStringwithCoordinates("Press SELECT to stop", 0, 48);
	
	// Hiển thị memory info nếu cần
	size_t freeHeap = esp_get_free_heap_size();
	if (freeHeap <= MEM_LOWER_LIM + 2000) {
		display.displayStringwithCoordinates("LOW MEM!", 80, 60);
	}
	
	display.sendDisplay();
}

void displayAttackStatus() {
	display.clearScreen();
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
		}
	}

	display.displayStringwithCoordinates("ATTACK RUNNING", 0, 12);
	display.displayStringwithCoordinates(attackName, 0, 24);
	
	if (currentState == WIFI_ATTACK_RUNNING) {
		String packetStr = "packet/sec: ";
		packetStr.concat(wifi.packet_sent);
		display.displayStringwithCoordinates(packetStr, 0, 36, true);
	} else if (currentState == BLE_ATTACK_RUNNING) {
		display.displayStringwithCoordinates("Advertising...", 0, 36, true);
	}
	
	// Display memory info
	size_t freeHeap = esp_get_free_heap_size();
	if (freeHeap <= MEM_LOWER_LIM + 2000) {
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
			display.clearScreen();
			display.displayStringwithCoordinates("ATTACK RUNNING", 0, 12);
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

	size_t freeHeap = esp_get_free_heap_size();
	if (freeHeap <= MEM_LOWER_LIM + 2000) {
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
		case WIFI_ATTACK_MENU:
			displayWiFiAttackMenu();
			break;
		case WIFI_SELECT_MENU:
			displayWiFiSelectMenu();
			break;
		case NRF24_MENU:
			displayNRF24Menu();
			break;
		case NRF24_JAMMER_MENU:
			displayNRF24JammerMenu();
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
		case WIFI_ATTACK_MENU:
			displayWiFiAttackMenu();
			break;
		case WIFI_SELECT_MENU:
			displayWiFiSelectMenu();
			break;
		case NRF24_MENU:
			displayNRF24Menu();
			break;
		case NRF24_JAMMER_MENU:
			displayNRF24JammerMenu();
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
			} else if (currentSelection == MAIN_REBOOT) {
				displayRebootConfirm();
				//delay(1000); // Give user time to see the message
				
				// Wait for user confirmation
				bool confirmed = false;
				bool cancelled = false;
				unsigned long startTime = millis();
				
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
					ble.initSpoofer();
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
			if (currentSelection == WIFI_SCAN) {
				currentState = WIFI_SCAN_RUNNING;
				displayWiFiScanMenu();
			} else if (currentSelection == WIFI_SELECT) {
				currentState = WIFI_SELECT_MENU;
				currentSelection = 0;
				if (access_points && access_points->size() > 0) {
					maxSelections = access_points->size() + 1;
				} else {
					maxSelections = 1;
				}
				displayWiFiSelectMenu();
			} else if (currentSelection == WIFI_ATTACK) {
				currentState = WIFI_ATTACK_MENU;
				currentSelection = 0;
				maxSelections = WIFI_ATK_MENU_COUNT;
				displayWiFiAttackMenu();
			} else if (currentSelection == WIFI_BACK) {
				goBack();
			}
			break;
			
		case WIFI_SCAN_RUNNING:
			if (!wifiScanRunning) {
				startWiFiScan();
			} else {
				wifiScanRunning = false;
			}
			break;
			
		case WIFI_SELECT_MENU:
			if (!access_points || access_points->size() == 0) {
				goBack();
			} else {
				int totalItems = access_points->size() + 1;
				if (currentSelection < access_points->size()) {
					AccessPoint ap = access_points->get(currentSelection);
					ap.selected = !ap.selected;
					access_points->set(currentSelection, ap);
					displayWiFiSelectMenu();
				} else {
					goBack();
				}
			}
				break;
			
		case WIFI_ATTACK_MENU:
			if (currentSelection == WIFI_ATK_BACK) {
				goBack();
			} else {
				// Check if we have selected APs for deauth, AP for probe and AP beacon attacks
				if ((currentSelection == 0 || currentSelection == 2 || currentSelection == 7) && 
					(!access_points || !hasSelectedAPs())) {
					display.clearScreen();
					display.displayStringwithCoordinates("NO APs SELECTED!", 0, 12);
					display.displayStringwithCoordinates("Select APs first", 0, 21, true);
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
				WiFiScanState attackTypes[] = {WIFI_ATTACK_DEAUTH, WIFI_ATTACK_DEAUTH_FLOOD, WIFI_ATTACK_AUTH, WIFI_ATTACK_RIC_BEACON, WIFI_ATTACK_STA_BEACON,
									   WIFI_ATTACK_RND_BEACON, WIFI_ATTACK_EVIL_PORTAL, WIFI_ATTACK_EVIL_PORTAL_DEAUTH};
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
											Usb_Wireless, Full_Channel};
				currentNRFJammerMode = listjammermode[currentSelection];
				displayNRFJammerStatus();
				startNRFJammer(currentNRFJammerMode);
				}
			break;

	}
	
}

void goBack() {
	switch(currentState) {
		case WIFI_MENU:
			currentState = MAIN_MENU;
			currentSelection = 0;
			maxSelections = MAIN_MENU_COUNT;
			displayMainMenu();
			break;
		case WIFI_SCAN_RUNNING:
			if (wifiScanRunning) {
				wifiScanRunning = false;
				wifiScanOneShot = false;
				wifiAttackOneShot = false;
				wifiScanDisplay = false;
			}
			currentState = WIFI_MENU;
			currentSelection = 0;
			maxSelections = WIFI_MENU_COUNT;
			displayWiFiMenu();
			break;
		case WIFI_ATTACK_MENU:
			currentState = WIFI_MENU;
			currentSelection = 0;
			maxSelections = WIFI_MENU_COUNT;
			displayWiFiMenu();
			break;
		case BLE_SCAN_RUNNING:
			if (bleScanRunning) {
				bleScanRunning = false;
				bleScanOneShot = false;
				bleScanDisplay = false;
			}
			currentState = BLE_MENU;
			currentSelection = 0;
			maxSelections = BLE_MENU_COUNT;
			displayBLEMenu();
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
			nrf.shutdownNRF();
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
			displayNRF24Menu();
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

	memset(values, 0, sizeof(uint8_t)*N);

	int n = 200;
	while (n--) {
		if (check(selPress)) {
			goBack();
			break;
		} 
		int i = N;
		while (i--) {
			if (check(selPress)) {
				goBack();
				return;
			} 
			nrf.setChannel(i);
			nrf.enableCE();
			delayMicroseconds(128);
			nrf.disableCE();
			if (nrf.carrierDetected()) {
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
	int barWidth = SCR_WIDTH / N;
	int x = 0;
	for (int i = 0; i < N; ++i) {
		int v = 63 - values[i] * 3;
		if (v < 0) {
			v = 0;
		}
		display.drawingVLine(x, v - 10, 64 - v);
		x += barWidth;
	}
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

	byte drawHeight = map(norm, 0, 64, 0, 64);

	for (byte count = 126; count > 0; count--) {
	  sensorArray[count] = sensorArray[count - 1];
	}
	sensorArray[0] = drawHeight;

	display.clearScreen();

	display.drawingLine(0, 0, 0, 63);
	display.drawingLine(127, 0, 127, 63);

	for (byte count = 0; count < 64; count += 10) {
	  display.drawingLine(127, count, 122, count);
	  display.drawingLine(0, count, 5, count);
	  if (check(selPress)) {
			goBack();
			return;
		}
	}

	for (byte count = 10; count < 127; count += 10) {
	  display.drawingPixel(count, 0);
	  display.drawingPixel(count, 63);
	  if (check(selPress)) {
			goBack();
			return;
		}
	}

	for (byte count = 0; count < 127; count++) {
	  display.drawingLine(127 - count, 63, 127 - count, 63 - sensorArray[count]);
	  if (check(selPress)) {
			goBack();
			return;
		}
	}

	display.setCursor(12, 12);
	display.printString("[" + String(norm) + "]");
	display.sendDisplay();
}

void nrfScanner() {
	if (check(selPress)) {
		Serial.println("[INFO] NRF24 Scanner stopped by user.");
		goBack();
		return;
	}
	nrf.scanChannel();
	if (check(selPress)) {
		Serial.println("[INFO] NRF24 Scanner stopped by user.");
		goBack();
		return;
	}
	nrfOutputScanChannel();
	if (millis() - lastSaveTime > saveInterval) {
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
	else if (attackType == WIFI_ATTACK_EVIL_PORTAL) {
		strmode = "Evil Portal";
	}
	else if (attackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH) {
		strmode = "Evil Portal Deauth";
	}
	Serial.println("[INFO] Starting WiFi attack: " + strmode);
	
	currentWiFiAttackType = attackType;
	currentState = WIFI_ATTACK_RUNNING;
	
	digitalWrite(STA_LED, HIGH);

	if (currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL ||
		currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH) displayEvilPortalInfo();
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
	else if (jammer_mode == Full_Channel) strmode = "Full Channel 1 . 100";

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
	ble.bleScan();
	bleScanOneShot = true;
	ble.ShutdownBLE();
	bleScanInProgress = false;

}

void startWiFiScan() {
	wifiScanRunning = true;
	wifiScanOneShot = false;
	
	// Start AP scan
	wifiScanInProgress = true;
	displayWiFiScanMenu();
	wifi.StartMode(WIFI_SCAN_AP);
	wifiScanOneShot = true;
	wifi.StartMode(WIFI_SCAN_OFF);
	wifiScanInProgress = false;
}

void stopCurrentAttack() {
	Serial.println("[INFO] Stopping current attack");
	 
	if (currentState == BLE_ATTACK_RUNNING) {
		ble.ShutdownBLE();
	} else if (currentState == WIFI_ATTACK_RUNNING) {
		if (currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL || currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH) {
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
	if (ble.ble_initialized) {
		ble.ShutdownBLE();
	}
	
	// Shutdown WiFi
	if (wifi.wifi_initialized) {
		wifi.StartMode(WIFI_SCAN_OFF);
	}
	
	// Display reboot message
	display.clearScreen();
	display.displayStringwithCoordinates("REBOOTING...", 0, 12);
	display.displayStringwithCoordinates("Please wait...", 0, 21, true);
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	
	// Restart ESP32
	ESP.restart();
}

void handleInput() {
	if (check(selPress)) {
		if ((wifiScanRunning && currentState == WIFI_SCAN_RUNNING) ||
			(bleScanRunning && currentState == BLE_SCAN_RUNNING) ||
			currentState == NRF24_ANALYZER_RUNNING ||
			currentState == NRF24_JAMMER_RUNNING ||
			currentState == NRF24_SCANNER_RUNNING ||
			currentState == WIFI_ATTACK_RUNNING ||
			currentState == BLE_ATTACK_RUNNING ||
			currentState == BLE_SPOOFER_RUNNING)
		{
			goBack();
		}
		else {
			selectCurrentItem();
		}
	}

	if (check(prevPress)) {
		if ((currentState == WIFI_SCAN_RUNNING && !wifiScanOneShot) ||
			(currentState == BLE_SCAN_RUNNING && !bleScanOneShot))
		{
			goBack();
		} 
		else {
			navigateUp();
		}
	}

	if (check(nextPress)) {
		navigateDown();
	}
}

void menuloop() {
	// Handle Input
	handleInput();

	// Check for critical low memory and auto-reboot
	static unsigned long lastMemoryCheck = 0;
	if (millis() - lastMemoryCheck > 3000) { // Check every 3 seconds
		size_t freeHeap = esp_get_free_heap_size();
		if (freeHeap < MEM_LOWER_LIM) { // Critical low memory threshold
			Serial.println("[SYSTEM_REBOOT] Critical low memory detected! Auto-rebooting...");
			display.clearScreen();
			display.displayStringwithCoordinates("CRITICAL LOW MEM!", 0, 12);
			display.displayStringwithCoordinates("AUTO REBOOTING...", 0, 21, true);
			vTaskDelay(2000 / portTICK_PERIOD_MS);
			wifi.StartMode(WIFI_SCAN_OFF);
			ble.ShutdownBLE();
			nrf.shutdownNRFJammer();
			nrf.shutdownNRF();
			// Ensure all tasks are stopped before rebooting
			ESP.restart();
		}
		lastMemoryCheck = millis();
	}
	
	// Handle WiFi scanning
	if (wifiScanRunning && currentState == WIFI_SCAN_RUNNING) {

		if (!wifiScanOneShot) startWiFiScan();

		if (!wifiScanDisplay) displayWiFiScanMenu();

	}

	else if (bleScanRunning && currentState == BLE_SCAN_RUNNING) {

		if (!bleScanOneShot) startBLEScan();

		if (!bleScanDisplay) displayBLEScanMenu();

	}

	else if (currentState == NRF24_ANALYZER_RUNNING) {
		if (!nrfAnalyzerSetupOneShot) {
			Serial.println("[INFO] Starting NRF Analyzer");
			nrf.analyzerSetup();
			nrfAnalyzerSetupOneShot = true;
		}
		nrfAnalyzer();
	}

	else if (currentState == NRF24_JAMMER_RUNNING) {
		if (!nrfJammerSetupOneShot) {
			Serial.println("[INFO] Starting NRF Jammer");
			nrf.jammerNRFRadioSetup();
			nrfJammerSetupOneShot = true;
		}
		nrf.jammerNRFRadioMain(currentNRFJammerMode);
	}

	else if (currentState == NRF24_SCANNER_RUNNING) {
		if (!nrfScannerSetupOneShot) {
			Serial.println("[INFO] Starting NRF Scanner");
			nrf.scannerSetup();
			nrfScannerSetupOneShot = true;
		}
		nrfScanner();
	}

	else if (currentState == BLE_SPOOFER_RUNNING) {
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
	else if (currentState == BLE_ATTACK_RUNNING || currentState == WIFI_ATTACK_RUNNING) {
		static unsigned long lastDisplayUpdate = 0;
		static unsigned long lastMemoryCheck = 0;
		
		// Memory check
		if (millis() - lastMemoryCheck > 5000) {
			if (!checkLeftMemory()) {
				Serial.println("[WARN] Low memory detected, stopping attack");
				stopCurrentAttack();
				return;
			}
			lastMemoryCheck = millis();
		}
		
		if (currentState == BLE_ATTACK_RUNNING) {
			// BLE attack handling...
			switch(currentBLEAttackType) {
				case BLE_ATTACK_SOUR_APPLE:
					ble.executeAppleSpam(SourApple);
					break;
				case BLE_ATTACK_APPLE_JUICE:
					ble.executeAppleSpam(AppleJuice);
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
		else if (currentState == WIFI_ATTACK_RUNNING) {
			// SỬA: Tách biệt xử lý Evil Portal và các WiFi attack khác
			if (currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL || currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH) {
				// Evil Portal handling

				if (!evilPortalOneShot) {
					bool _ap;
					Serial.println("[INFO] Starting Setup Portal");
					if (currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH) {
						_ap = eportal.apSetup(true);
					} else {
						_ap = eportal.apSetup(false);
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
		if (currentState == WIFI_ATTACK_RUNNING) { // Ble Attack no need redraw display
			if (currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL || currentWiFiAttackType == WIFI_ATTACK_EVIL_PORTAL_DEAUTH || currentWiFiAttackType == WIFI_ATTACK_DEAUTH_FLOOD) {
			// i can't using != for wtf reason, idk to fix this
			// Deauth Flood using different redraw
			} else {
				if (millis() - lastDisplayUpdate > 1000) {
					wifi.packet_sent = 0;
					displayAttackStatus();
					lastDisplayUpdate = millis();
				}
			}
		}
	}
}

#pragma GCC diagnostic pop