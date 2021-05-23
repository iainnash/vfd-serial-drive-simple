#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "config.private.h"

// WiFi Parameters
const char* ssid = WIFI_NETWORK;
const char* password = WIFI_PASSWORD;

void setup() {
  delay(1000);
  Serial.begin(9600);
  Serial2.begin(9600);
  WiFi.begin(ssid, password);
 
  Serial.println("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  Serial.println("connected");
}

const char fingerprint[] PROGMEM = FINGERPRINT_DATA;

void loop() {
  // Check WiFi Status
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("f");
    // WiFiClientSecure client;
    // client.setInsecure(); //the magic line, use with caution
    // client.connect(FETCH_URL, 443);

    HTTPClient http;
    http.setTimeout(15000);
    delay(100);
    http.begin(FETCH_URL);
    int httpCode = http.GET();
    //Check the returning code                                                                  
    if (httpCode > 0) {
      // Parsing
      const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(http.getString());
      // Parameters
      int low = root["safeLow"];
      int average = root["average"];
      String lowStr = String(low/10);
      String averageStr = String(average/10);
      Serial.print(http.getString());
      Serial2.print("r");
      delay(100);
      Serial2.print(" ");
      delay(100);
      Serial2.print(lowStr);
      delay(100);
      Serial2.print(" ");
      delay(100);
      Serial2.print(averageStr);
      delay(100);
      Serial2.print(' ');
      delay(100);
    }
    http.end();   //Close connection
  }
  int tm = 0;
  while (tm < 60000) {
    tm+=1;
    delay(10);
    if (Serial.available()) {
      Serial2.write(Serial.read());
    }
  }
  // Delay
  // delay(60000);
}