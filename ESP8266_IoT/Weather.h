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

#ifndef __WEATHER_H__
#define __WEATHER_H__

class Weather
{
  public:
    Weather();
    static void saveWeatherConfig(String ssid, String pass);
    void loadWeatherConfig(void);
    bool getLatestWeather(void);

  public:
    String weather;
    String icon;
    String rainAmount;
    String cityName;
    String tempMin;
    String tempMax;

  protected:
    unsigned long mLastMeasured;
    unsigned long mLastTried;
    String mCityId;
    String mAppId;

    bool parseResult(String& line, String& outString, const char* startStr, int startStrLen, const char* endStr, int startPos=0);
};

#endif // __WEATHER_H__
