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
#include "Weather.h"
#include <FS.h>
#include <WiFiClient.h>

Weather::Weather():mLastMeasured(0),mLastTried(0)
{
}

void Weather::saveWeatherConfig(String cityId, String appId){
  if( (cityId!="") && (appId!="") ){
    if ( SPIFFS.exists(WEATHER_CONFIG) ) {
      SPIFFS.remove(WEATHER_CONFIG);
    }

    File f = SPIFFS.open(WEATHER_CONFIG, "w");
    f.println(cityId);
    f.println(appId);
    f.close();
  }
}

void Weather::loadWeatherConfig(void){
  File f = SPIFFS.open(WEATHER_CONFIG, "r");
  mCityId = f.readStringUntil('\n');
  mAppId = f.readStringUntil('\n');
  f.close();

  mCityId.trim();
  mAppId.trim();
}

const char* WEATHER_API_HOST = "api.openweathermap.org";
const char* WEATHER_API_URL = "/data/2.5/forecast/daily?id=";
const char* WEATHER_API_ARG = "&units=metric&cnt=1";

const char* WEATHER_API_RESULT = "{\"city\":{\"id\":";
const int   WEATHER_API_RESULT_LEN = strlen(WEATHER_API_RESULT);
const char* WEATHER_API_RESULT_WEATHER = "weather\":[{\"id\":";
const int   WEATHER_API_RESULT_WEATHER_LEN = strlen(WEATHER_API_RESULT_WEATHER);

const char* WEATHER_API_RESULT_WEATHER_MAIN = "main\":\"";
const int   WEATHER_API_RESULT_WEATHER_MAIN_LEN = strlen(WEATHER_API_RESULT_WEATHER_MAIN);

const char* WEATHER_API_RESULT_WEATHER_ICON = "icon\":\"";
const int   WEATHER_API_RESULT_WEATHER_ICON_LEN = strlen(WEATHER_API_RESULT_WEATHER_ICON);

const char* WEATHER_API_RESULT_WEATHER_RAIN = "rain\":";
const int   WEATHER_API_RESULT_WEATHER_RAIN_LEN = strlen(WEATHER_API_RESULT_WEATHER_RAIN);
const char* WEATHER_API_RESULT_WEATHER_RAIN_END = "}";

const char* WEATHER_API_RESULT_CITY = "name\":\"";
const int   WEATHER_API_RESULT_CITY_LEN = strlen(WEATHER_API_RESULT_CITY);

const char* WEATHER_API_RESULT_TEMP = "temp\":{\"";
const int   WEATHER_API_RESULT_TEMP_LEN = strlen(WEATHER_API_RESULT_TEMP);
const char* WEATHER_API_RESULT_TEMP_MIN = "min\":";
const int   WEATHER_API_RESULT_TEMP_MIN_LEN = strlen(WEATHER_API_RESULT_TEMP_MIN);
const char* WEATHER_API_RESULT_TEMP_MAX = "max\":";
const int   WEATHER_API_RESULT_TEMP_MAX_LEN = strlen(WEATHER_API_RESULT_TEMP_MAX);
const char* WEATHER_API_RESULT_TEMP_END = ",";

const char* WEATHER_API_RESULT_WEATHER_END = "\"";

bool Weather::parseResult(String& line, String& outString, const char* startStr, int startStrLen, const char* endStr, int startPos)
{
  int pos2 = StringFind( line, startStr, startPos );
  if( pos2!=-1 ) {
    int pos3 = StringFind( line, endStr, pos2+startStrLen+1 );
    if( pos3!=-1 ){
      outString = line.substring(pos2+startStrLen, pos3);
      DEBUG_PRINTLN( outString );
      return true;
    }
  }
  return false;
}

bool Weather::getLatestWeather(void)
{
  if( (mCityId=="") && (mAppId=="") ){
    loadWeatherConfig();
  }

  if( (mCityId!="") && (mAppId!="") ){
    WiFiClient client;

    unsigned long n = millis();
    if( (mLastMeasured!=0) && (n - mLastMeasured) < 3600000 ) return false; // within 1000msec barrier
    if( (n - mLastTried) < 5000 ) return false; // within 5000msec barrier
    mLastTried = n;

    DEBUG_PRINTLN(mCityId);
    DEBUG_PRINTLN(mAppId);

    if( !client.connect( WEATHER_API_HOST, 80 ) ){
      DEBUG_PRINTLN("Connection failed to openweathermap.");
      return false;
    }

    client.print(
      String("GET ") + WEATHER_API_URL + mCityId + "&appid=" + mAppId + WEATHER_API_ARG + " HTTP/1.1\r\n" +
      "Host: " + WEATHER_API_HOST + "\r\n" + 
      "Connection: close\r\n\r\n"
    );
    DEBUG_PRINTLN("WEATHER:posted");
  
    delay(10);

    bool bFound = false;
        
    while(client.available()){
      String line = client.readStringUntil('\r');
      line.trim();
      DEBUG_PRINTLN(line);
      int lineLength = line.length();

      if( line.startsWith( WEATHER_API_RESULT) ){
        // weather
        int pos = StringFind( line, WEATHER_API_RESULT_WEATHER );
        if( pos!=-1 ){
          bFound = true;
          pos = pos+WEATHER_API_RESULT_WEATHER_LEN;
          parseResult(line, weather,     WEATHER_API_RESULT_WEATHER_MAIN, WEATHER_API_RESULT_WEATHER_MAIN_LEN, WEATHER_API_RESULT_WEATHER_END, pos);
          parseResult(line, icon,        WEATHER_API_RESULT_WEATHER_ICON, WEATHER_API_RESULT_WEATHER_ICON_LEN, WEATHER_API_RESULT_WEATHER_END, pos);
          parseResult(line, rainAmount,  WEATHER_API_RESULT_WEATHER_RAIN, WEATHER_API_RESULT_WEATHER_RAIN_LEN, WEATHER_API_RESULT_WEATHER_RAIN_END, pos);
        }

        // temperature
        pos = StringFind( line, WEATHER_API_RESULT_TEMP );
        if( pos!=-1 ){
          bFound = true;
          pos = pos+WEATHER_API_RESULT_TEMP_LEN;
          parseResult(line, tempMin,  WEATHER_API_RESULT_TEMP_MIN, WEATHER_API_RESULT_TEMP_MIN_LEN, WEATHER_API_RESULT_TEMP_END, pos);
          parseResult(line, tempMax,  WEATHER_API_RESULT_TEMP_MAX, WEATHER_API_RESULT_TEMP_MAX_LEN, WEATHER_API_RESULT_TEMP_END, pos);
        }

        // cityname
        parseResult(line, cityName,  WEATHER_API_RESULT_CITY, WEATHER_API_RESULT_CITY_LEN, WEATHER_API_RESULT_WEATHER_END, WEATHER_API_RESULT_LEN);
      }
    }

    if( bFound ) {
      mLastMeasured = n; // update
    }

    return bFound;
  }
}

