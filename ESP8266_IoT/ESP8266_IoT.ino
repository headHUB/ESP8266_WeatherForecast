/* 
 Copyright (C) 2016 hidenorly

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#include "base.h"
#include "config.h"

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Adafruit_ILI9341.h>

#include <ESP8266WiFi.h>
#include "WiFiUtil.h"
#include "WebConfig.h"
#include "NtpUtil.h"
#include "LooperThreadTicker.h"

#include <FS.h>
#include <Time.h>
#include <TimeLib.h>
#include <NTP.h>

#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)

#include "Weather.h"

// --- mode changer
bool initializeProperMode(){
  if( (digitalRead(MODE_PIN) == 0) || (!SPIFFS.exists(WIFI_CONFIG)) ){
    // setup because WiFi AP mode is specified or WIFI_CONFIG is not found.
    setupWiFiAP();
    setup_httpd();
    return false;
  } else {
    setupWiFiClient();
    setup_httpd();  // comment this out if you don't need to have httpd on WiFi client mode
  }
  return true;
}

// --- handler for WiFi client enabled
void onWiFiClientConnected(){
  DEBUG_PRINTLN("WiFi connected.");
  DEBUG_PRINT("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
  start_NTP(); // socket related is need to be executed in main loop.
}

// Initialize TFT LCD as global
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


class Poller:public LooperThreadTicker
{
  public:
    Poller(int dutyMSec=0):LooperThreadTicker(NULL, NULL, dutyMSec)
    {
    }
    virtual void doCallback(void)
    {
      char s[30];
      time_t n = now();
      sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d", year(n), month(n), day(n), hour(n), minute(n), second(n));
    
      DEBUG_PRINT("UTC : ");
      DEBUG_PRINTLN(s);

      static Weather weather;
      if( weather.getLatestWeather() ) {
        tft.setRotation(3);
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(0, 0);
        tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(4);
        tft.println(weather.cityName);
        tft.println(weather.weather);
        tft.println(weather.rainAmount + "[mm]");
        tft.println(weather.tempMin+"/" + weather.tempMax+" C");
//      tft.println(weather.icon);

        tft.setTextSize(4);
        time_t t = localtime(n, 9);
        sprintf(s, "%02d/%02d %02d:%02d", month(t), day(t), hour(t), minute(t));
        tft.println(s);
      }
  }
};


// --- General setup() function
void setup() {
  // Initialize GPIO
  initializeGPIO();
  
  // Initialize SerialPort
  Serial.begin(115200);

  // Initialize SPI File System
  SPIFFS.begin();

  // Check mode
  delay(1000);
  if(initializeProperMode()){
    static Poller* sPoll=new Poller(1000);
    g_LooperThreadManager.add(sPoll);
  }

  // Initialize LCD
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, LOW);
  delay(10);
  digitalWrite(TFT_RST, HIGH);

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
}

void loop() {
  // put your main code here, to run repeatedly:
  handleWiFiClientStatus();
  handleWebServer();
  g_LooperThreadManager.handleLooperThread();
}
