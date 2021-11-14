#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "wifi_creds.h"

WiFiClient client;

const int httpPort = 80;

#define PIN_EN 5
#define PIN_RS 17
#define PIN_4 18
#define PIN_5 19
#define PIN_6 22
#define PIN_7 21

#define PIN_LED 15
#define PIN_5V_ENA 13

#define CONTRAST_PIN 9
#define BACKLIGHT_PIN 7
#define CONTRAST 110

LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_4, PIN_5, PIN_6, PIN_7, 23, POSITIVE);

unsigned long lastRequest = 0;
StaticJsonDocument<512> json;
StaticJsonDocument<512> getWhois();
String requestWhois();
void connect();

void setup()
{
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_5V_ENA, OUTPUT);

  digitalWrite(PIN_LED, HIGH);
  digitalWrite(PIN_5V_ENA, HIGH);

  lcd.backlight();
  lcd.begin(16, 2); // initialize the lcd

  lcd.home(); // go home

  connect();
}

void loop()
{
  if(WiFi.status() != WL_CONNECTED) connect();

  if (millis() - lastRequest >= 60 * 1000)
  {
    json = getWhois();
    lastRequest = millis();
  }

  static int counter = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("People: ");
  lcd.print(json["headcount"].as<long>());
  if (json["users"].size() > 0)
  {
    int index = (counter++) % (json["users"].size());
    lcd.setCursor(0, 1);
    lcd.print(json["users"][index].as<String>());
  }
  else
  {
    lcd.setCursor(0, 1);
    lcd.print("Pool's Closed");
  }
  delay(1000);
}

StaticJsonDocument<512> getWhois()
{
  StaticJsonDocument<512> json;
  DeserializationError error = deserializeJson(json, requestWhois());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Parsing");
  if (error != DeserializationError::Ok)
  {
    lcd.setCursor(0, 1);
    lcd.print(error.c_str());
    delay(1000);
  }
  return json;
}

String requestWhois()
{
  String payload = "";
  const char *root_ca = \
    "-----BEGIN CERTIFICATE-----"
    "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"
    "-----END CERTIFICATE-----";

  HTTPClient http;
  http.begin("https://whois.at.hsp.sh/api/now", root_ca); //Specify the URL and certificate
  int httpCode = http.GET();                              //Make the request

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Requesting");
  if (httpCode > 0)
  { //Check for the returning code

    payload = http.getString();
    http.end(); //Free the resources
    lcd.setCursor(0, 1);
    lcd.print(httpCode + " " + payload);
  }
  else
  {
    Serial.println(F("Error on HTTP request"));
    lcd.print(httpCode + " error");
    delay(1000);
    return "";
  }
  http.end(); //Free the resources
  return payload;
}

void connect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWD);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for WiFi");
  lcd.setCursor(0, 1); // go to the next line
  
  uint8_t cursor = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (cursor > 16)
    {
      cursor = 0;
      lcd.setCursor(cursor, 1);
      lcd.print("                ");
      lcd.setCursor(cursor, 1);
    };
    delay(100);
    lcd.print("*");
    ++cursor;
  }

  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(1000);

  json = getWhois();
}