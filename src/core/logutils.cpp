#include "core/logutilsheader.h"

LogUtils::LogUtils() {}

LogUtils::~LogUtils() {
	//if (bufA) free(bufA);
	//if (bufB) free(bufB);
    if (bufA) delete[] bufA;
	if (bufB) delete[] bufB;
}

void LogUtils::begin() {
	//if (!bufA) bufA = (uint8_t*)malloc(BUF_SIZE);
    //if (!bufB) bufB = (uint8_t*)malloc(BUF_SIZE);
	if (!bufA) bufA = new uint8_t[BUF_SIZE]; // i think it's better to use new/delete in cpp
	if (!bufB) bufB = new uint8_t[BUF_SIZE];
	if (!bufA || !bufB) {
		Serial.println("[ERROR] LogUtils malloc failed");
	}
	bufSizeA = bufSizeB = 0;
	useA = true;
}

void LogUtils::createFile(String file_name, bool is_pcap) {
	file_path = "";
	int i = 0;

	file_path += "/";
	file_path += file_name;
	file_path += "_";

	if (is_pcap) {
		while (sdcard.isExists("/" + file_name + "_" + String(i) + ".pcap")) i++;
		file_path += String(i);
		file_path += ".pcap";
	} else {
		while (sdcard.isExists("/" + file_name + "_" + String(i) + ".log")) i++;
		file_path += String(i);
		file_path += ".log";
	}

	if (espatsettings.savepcap) {
		File temp = sdcard.getFile(file_path, FILE_WRITE);
		if (!temp) {
			Serial.println("[WARN] Failed to create file or card not mounted");
			return;     
		}
		temp.close();
		if (is_pcap)
			Serial.println("[INFO] Created .pcap File");
		else
			Serial.println("[INFO] Created .log File");

		bufSizeA = 0;
		bufSizeB = 0;
		useA = true;

		if (is_pcap) {
		    // header PCAP
			write(uint32_t(0xa1b2c3d4));
			write(uint16_t(2));
			write(uint16_t(4));
		    write(int32_t(0));
			write(uint32_t(0));
			write(uint32_t(4096));          // snaplen
			write(uint32_t(105));           // DLT_IEEE802_11
		}
		save();
	}
}

void LogUtils::pcapAppend(wifi_promiscuous_pkt_t *packet, int len) {
	if (espatsettings.savepcap) add(packet->payload, len, true);
}
	
void LogUtils::logAppend(String log) {
	if (espatsettings.savepcap) add((const uint8_t*)log.c_str(), log.length(), false);
}

void LogUtils::add(const uint8_t* buf, uint32_t len, bool is_pcap){ 
	// buffer is full -> drop packet
	if((useA && bufSizeA + len >= BUF_SIZE && bufSizeB > 0) || (!useA && bufSizeB + len >= BUF_SIZE && bufSizeA > 0)){
		return;
	}
	
	if(useA && bufSizeA + len + 16 >= BUF_SIZE && bufSizeB == 0){
		useA = false;
	}
	else if(!useA && bufSizeB + len + 16 >= BUF_SIZE && bufSizeA == 0){
		useA = true;
	}
	
	uint32_t microSeconds = micros(); // e.g. 45200400 => 45s 200ms 400us
	uint32_t seconds = (microSeconds/1000)/1000; // e.g. 45200400/1000/1000 = 45200 / 1000 = 45s
	
	microSeconds -= seconds*1000*1000; // e.g. 45200400 - 45*1000*1000 = 45200400 - 45000000 = 400us (because we only need the offset)
	
	if (is_pcap) {
		write(seconds); // ts_sec
		write(microSeconds); // ts_usec
		write(len); // incl_len
		write(len); // orig_len
	}
	
	write(buf, len); // packet payload
	}

void LogUtils::save() {

		if ((bufSizeA + bufSizeB) == 0) return;

		bool writeA;
		uint32_t szA, szB;

		writeA = useA;
		useA = !useA;

		szA = bufSizeA;
		szB = bufSizeB;
		bufSizeA = 0;
		bufSizeB = 0;
	
		File file = sdcard.getFile(file_path, FILE_APPEND);
		if (!file) return;
	
		if (writeA) {
			if (szA) file.write(bufA, szA);
			if (szB) file.write(bufB, szB);
		} else {
			if (szB) file.write(bufB, szB);
			if (szA) file.write(bufA, szA);
		}
	
		file.close();
}


// ESP32 Marauder
void LogUtils::write(const uint8_t* buf, uint32_t len) {	
	if(useA) {
		memcpy(&bufA[bufSizeA], buf, len);
		bufSizeA += len;
	} else {
		memcpy(&bufB[bufSizeB], buf, len);
		bufSizeB += len;
	}
}

void LogUtils::write(int32_t n) {
	uint8_t buf[4];
	buf[0] = n;
	buf[1] = n >> 8;
	buf[2] = n >> 16;
	buf[3] = n >> 24;
	write(buf,4);
}
	
void LogUtils::write(uint32_t n) {
	uint8_t buf[4];
	buf[0] = n;
	buf[1] = n >> 8;
	buf[2] = n >> 16;
	buf[3] = n >> 24;
	write(buf,4);
}
	
void LogUtils::write(uint16_t n) {
	uint8_t buf[2];
	buf[0] = n;
	buf[1] = n >> 8;
	write(buf,2);
}