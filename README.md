# ESP8266_WeatherForecast

Weather Forecast Screen with LCD ILI9341 on ESP8266.

# Dependent libraries

Thank you so much for very useful library!

## General steps to include zip library

On Arduino IDE,

1. Sketch
2. Include library
3. Install Zip library
4. Specify the following zip-ed libraries.

## Time library (by PaulStoffregen-san)

```
$ git clone https://github.com/PaulStoffregen/Time.git Time
$ cd Time
$ git archive HEAD --output=../Time.zip
```

## NTP library (by exabugs-san)

```
$ git clone https://github.com/exabugs/sketchLibraryNTP NTP
$ cd NTP
$ git archive HEAD --output=../NTP.zip
```

## Adafruit_Graphics

https://github.com/adafruit/Adafruit-GFX-Library

## Adafruit_ILI9341

https://github.com/adafruit/Adafruit_ILI9341

# Steps to configure

## Get API Key at openweathermap

Go https://www.openweathermap.org/ and signup to get API Key(appId).

## Check city Id

http://openweathermap.org/help/city_list.txt

## Configure SSID/Password & cityId/AppId

Please access to ESP8266's Mac Address with "1234567890" and open browser at 192.168.4.1 to configure them.

And please restart your device!

# Appendix

## My board config : ESP8266 - ILI9341

| ESP8266 | ILI9341 |
| :--- | :--- |
| GPIO4 | LCD DC/RS |
| GPIO5 | LCD RESET + LCD_EN (LED) |
| GPIO12 | LCD HSPI_MISO |
| GPIO13 | LCD HSPI_MOSI |
| GPIO14 | LCD HSPI_CLK |
| GPIO15 | LCD HSPI_CS |

## config.h

```
#define TFT_CS 15
#define TFT_RST 5
#define TFT_DC 4
```
