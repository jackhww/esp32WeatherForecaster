#include <Arduino.h>
#include <icons.h>
#include "time.h"
#include "credentials.h"
#include "httpRequests.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

char timeHHMM[6];
// unsigned long previousMillis = 0;
// const long interval = 10000;

U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 18, /* data=*/ 23, /* cs=*/ 5, /* dc=*/ 17, /* reset=*/ 16);  
// U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 18, /* data=*/ 23, /* cs=*/ 5, /* dc=*/ 17, /* reset=*/ 16);

void drawWeatherSymbol(int x, int y, const uint8_t* symbol) {
 u8g2.drawXBMP(x,y,8,8,symbol);
}

void weatherOverview(std::vector <weatherData> & weatherEntries){
  u8g2.drawXBMP( 47, 52, 24, 24, image_droplet_3xw_bits);
  u8g2.drawXBMP( 3, 0, 128, 64, image_draw_7rdmeyebacslmgkwbay_bits);
  /* To change: call timeRequest only on startup, then update using internal RTC*/
  char* currentTime = timeRequest();
  u8g2.setFont(u8g2_font_haxrcorp4089_tr);
  u8g2.drawStr(101, 63, currentTime);

  // u8g2.drawXBMP( 58, 17, 8, 8, image_droplet_bits);
  u8g2.drawXBMP( 0, 57, 10, 10, image_ir_10px_bits);
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(30, 9, "Weather Overview");
  //to add: if timeHH < 18 set imageSun else set imageMoon
  u8g2.drawXBMP( 93, 56, 8, 8, imageSun);
  u8g2.drawXBMP( 86, 55, 8, 8, imageMoon);

  /* Forecast Areas */
  struct location{
    const char* name;
    int x;
    int y;
  };

  location locations[] = {
    {"Woodlands", 59, 14},
    {"Mandai", 93, 33},
    {"Jurong", 38,22},
    {"Central Water Catchment", 32,36},
    {"Queenstown", 63,42},
    {"Punggol", 76,30},  
    {"Tampines", 54,30}
  };

  //iterate through the list retrieved from the gov api, if name match with the locations[], strcmp forecast, draw to the x andy coordinates respectively and the bitmap to use
  /* To add more states: Sunny, thunderstorm, Light Drizzle.. etc*/
  for (const auto& location : locations) {
          for (const auto& entry : weatherEntries) {
              if (strcmp(entry.area, location.name) == 0) {
                  if (strcmp(entry.forecast, "Cloudy") == 0) {
                      drawWeatherSymbol(location.x, location.y, imageCloudy);
                      Serial.printf("%s Cloudy\n", location.name);
                  } else {
                      drawWeatherSymbol(location.x, location.y, imageRain);
                  }
                  break; //stop searching for location when
              }
          }
      }
  u8g2.sendBuffer();
}



void connectWifi(){
  u8g2.clearBuffer();
  u8g2.drawXBMP( 8, 15, 36, 36, image_ArrowC_1_36x36_bits);
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(50, 31, "Connecting To");
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(71, 42, "Wifi...");
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(75, 53, ssid);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("WiFi connected.");
  u8g2.sendBuffer();
}

void connectedScreen(){
  u8g2.clearBuffer();
  u8g2.drawXBMP( 0, 57, 10, 10, image_ir_10px_bits);
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(37, 29, "Connected!");
  u8g2.sendBuffer();
}

void setup(void) {
  Serial.begin(115200);
  u8g2.begin();  
  u8g2.enableUTF8Print();
  u8g2.setBitmapMode(1);
  while(WiFi.status() != WL_CONNECTED)
  {
  connectWifi();
  delay(100);
  }
  connectedScreen();
  // delay(300);
  char* currentTime = timeRequest();
  if (strlen(currentTime) > 0) {
    strcpy(timeHHMM, currentTime);
    Serial.print("Initial Time: ");
    Serial.println(currentTime);
    u8g2.setFont(u8g2_font_haxrcorp4089_tr);
    u8g2.drawStr(101, 63, timeHHMM);
    u8g2.sendBuffer();
  }
  free(currentTime);
  weatherRequest();
 
}


void loop(void) {
  u8g2.clearBuffer();
  // unsigned long currentMillis = millis();
  // if (currentMillis - previousMillis >= interval){
  //   previousMillis = currentMillis;

  //   unsigned long secondsSinceUpdate = currentMillis / 1000;
  //   unsigned int seconds = secondsSinceUpdate % 60;
  //   unsigned int minutes = (secondsSinceUpdate/60) % 60;
  //   unsigned int hours = (secondsSinceUpdate / 3600) %24;

  //   snprintf(currentTime, sizeof(timeHHMM), "%02u:%02u", hours, minutes);
  //   u8g2.setFont(u8g2_font_haxrcorp4089_tr);
  //   u8g2.drawStr(101, 63, timeHHMM);
  //   }
    std::vector<weatherData> weatherEntries = weatherRequest();
    // Debugging
    for (const auto& entry : weatherEntries) {
        Serial.print("Area: ");
        Serial.println(entry.area);
        Serial.print("Forecast: ");
        Serial.println(entry.forecast);
    }
    // End

  weatherOverview(weatherEntries);
  delay(60000);
  u8g2.sendBuffer();
}

/* To include WDT for reliability */