#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <utility>

/* Time Section */
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200 + 3600; //gmt+8
const int   daylightOffset_sec = 3600;

char* timeRequest(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return "";
  }
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  char timeMin[3];
  strftime(timeMin, 3, "%M", &timeinfo);  
  char timeWeekDay[10];
  char timeHHMM[6]; 
  snprintf(timeHHMM, sizeof(timeHHMM), "%s:%s", timeHour, timeMin);
  char* returnTime = (char*)malloc(strlen(timeHHMM)+1);
  strcpy(returnTime, timeHHMM);

  return returnTime;
}

/* Weather API Call*/
struct weatherData {
    const char* area;
    const char* forecast;
};
std::vector<weatherData> weatherRequest() {
    std::vector<weatherData> weatherList;

    HTTPClient http;
    http.begin("https://api.data.gov.sg/v1/environment/2-hour-weather-forecast");
    int httpCode = http.GET();
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            DynamicJsonDocument doc(12288);
            DeserializationError error = deserializeJson(doc, payload);
            if (!error) {
                JsonObject items = doc["items"][0];
                JsonArray forecasts = items["forecasts"];
                for (JsonVariant forecast : forecasts) {
                    const char* area = forecast["area"];
                    const char* forecastStr = forecast["forecast"];
                    weatherList.push_back({area, forecastStr});
                }
            } else {
                Serial.print("JSON parsing error: ");
                Serial.println(error.c_str());
            }
        } else {
            Serial.printf("HTTP error code: %d\n", httpCode);
        }
    } else {
        Serial.println("HTTP request failed");
    }
    http.end();

    return weatherList;
}