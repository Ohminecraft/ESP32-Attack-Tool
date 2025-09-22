#include "wififunc.h"


uint8_t getSecurityType(uint8_t* payload, uint16_t len) {
    const uint8_t* frame = payload;
	const uint8_t* ies = payload + 36; // Start of tagged parameters
	uint16_t ies_len = len - 36;
  
	bool hasRSN = false;
	bool hasWPA = false;
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

void getMAC(char *addr, uint8_t* data, uint16_t offset) {
    sprintf(addr, "%02x:%02x:%02x:%02x:%02x:%02x", 
            data[offset+0], data[offset+1], data[offset+2], 
            data[offset+3], data[offset+4], data[offset+5]);
}

#define HOP_INTERVAL 100 // milliseconds
uint32_t lastHop = 0;

void WiFiCallback::start_rtl_ap_scan_callback(bool sta_scan_enable) {
    wifi_on(RTW_MODE_STA);

    wifi_enter_promisc_mode();
    if (sta_scan_enable) wifi_set_promisc(RTW_PROMISC_ENABLE_2, rtl_ap_sta_sniffer_callback, 1);
    else wifi_set_promisc(RTW_PROMISC_ENABLE_2, rtl_ap_sniffer_callback, 1);
}

void WiFiCallback::rtl_ap_sniffer_callback(uint8_t *packet, uint length, void* param) {
    if (packet == NULL || length < 24) return;
    
    ieee80211_frame_info_t* frame_info = (ieee80211_frame_info_t *)param;
    
    uint8_t frame_type = (packet[0] >> 2) & 0x3;
    uint8_t frame_subtype = (packet[0] >> 4) & 0xF;
    
    // Management frames - Beacon processing
    if (frame_type == 0 && frame_subtype == 8) {  // Beacon frame
        // Check if AP already in list
        String essid = "";
        uint16_t channel_from_beacon = 0;
        
        // Parse Information Elements
        if (length > 36) {
            uint32_t ie_offset = 36;
            
            while (ie_offset + 2 < length) {
                uint8_t ie_type = packet[ie_offset];
                uint8_t ie_length = packet[ie_offset + 1];
                
                if (ie_offset + 2 + ie_length > length) break;
                
                if (ie_type == 0) {  // SSID IE
                    for (int i = 0; i < ie_length && i < 32; i++) {
                        if (packet[ie_offset + 2 + i] != 0) {
                            essid += (char)packet[ie_offset + 2 + i];
                        }
                    }
                }
                else if (ie_type == 3 && ie_length >= 1) {  // DS Parameter Set
                    channel_from_beacon = packet[ie_offset + 2];
                }
                
                ie_offset += 2 + ie_length;
            }
        }
        
        if (essid == "") {
            char bssid_str[18];
            getMAC(bssid_str, packet, 16);
            essid = String(bssid_str);
        }
        
        uint8_t security_type = getSecurityType(packet, length);
        
        // Format: NETWORK:SSID,RSSI,Channel,Band,BSSID,Security
        Serial.println("NETWORK:" + essid + "," + String(frame_info->rssi) + "," + 
                       String(channel_from_beacon) + ((channel_from_beacon > 13) ? ",5G," : ",2.4G,") + 
                       macToString(&packet[16]) + "," + String(security_type));
    }
}

void WiFiCallback::rtl_ap_sta_sniffer_callback(uint8_t *packet, uint length, void* param) {
    if (packet == NULL || length < 24) return;
    
    ieee80211_frame_info_t* frame_info = (ieee80211_frame_info_t *)param;
    
    uint8_t frame_type = (packet[0] >> 2) & 0x3;
    uint8_t frame_subtype = (packet[0] >> 4) & 0xF;
    
    // Management frames - Beacon processing
    if (frame_type == 0 && frame_subtype == 8) {  // Beacon frame
        // Check if AP already in list
        String essid = "";
        uint16_t channel_from_beacon = 0;
        
        // Parse Information Elements
        if (length > 36) {
            uint32_t ie_offset = 36;
            
            while (ie_offset + 2 < length) {
                uint8_t ie_type = packet[ie_offset];
                uint8_t ie_length = packet[ie_offset + 1];
                
                if (ie_offset + 2 + ie_length > length) break;
                
                if (ie_type == 0) {  // SSID IE
                    for (int i = 0; i < ie_length && i < 32; i++) {
                        if (packet[ie_offset + 2 + i] != 0) {
                            essid += (char)packet[ie_offset + 2 + i];
                        }
                    }
                }
                else if (ie_type == 3 && ie_length >= 1) {  // DS Parameter Set
                    channel_from_beacon = packet[ie_offset + 2];
                }
                
                ie_offset += 2 + ie_length;
            }
        }
        
        if (essid == "") {
            char bssid_str[18];
            getMAC(bssid_str, packet, 16);
            essid = String(bssid_str);
        }
        
        uint8_t security_type = getSecurityType(packet, length);
        
        // Format: NETWORK:SSID,RSSI,Channel,Band,BSSID,Security
        Serial.println("NETWORK:" + essid + "," + String(frame_info->rssi) + "," + 
                       String(channel_from_beacon) + ((channel_from_beacon > 13) ? ",5G," : ",2.4G,") + 
                       macToString(&packet[16]) + "," + String(security_type));
    }
    
    // Data frames - Station detection
    else if (frame_type == 2) {  // Data frame
        if (length < 24) return;

        // Format: STATION:SRC_BSSID,DST_BSSID
        Serial.println("STATION:" + macToString(&packet[10]) + "," + macToString(&packet[4]));
    }
}