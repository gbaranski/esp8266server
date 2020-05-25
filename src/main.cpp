#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#ifndef __ARDUINO_OTA_H
#include "OTA.h"
#endif

#ifndef STASSID
#define STASSID "Nigeria"
#define STAPSK "hondamsx125"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

const int RELAYPIN = 14;
bool isTimerOn;
unsigned long previousMillis = 0;
int remainingSeconds = 0;

ESP8266WebServer server(80);

void handleStartMixing()
{
  digitalWrite(RELAYPIN, 1);
  isTimerOn = true;
  remainingSeconds = 600;
  server.send(200, "text/plain", "OK");
}
void handleGetESPData()
{
  const String JSON =
      R"({"remainingSeconds":)" + String(remainingSeconds) +
      R"(,"isTimerOn":)" + String(isTimerOn) +
      "}";
  Serial.println(JSON);
  server.send(200, "application/json", JSON);
}

void handleTimer()
{
  remainingSeconds--;
  Serial.println(remainingSeconds);
}

void setupGPIO()
{
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, 0);
}

void setupWifi()
{
  setupGPIO();
  WiFi.mode(WIFI_STA);
  WiFi.hostname("ESP8266-Watermixer");
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  setupOTA();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting");
  setupWifi();

  server.on("/startMixing", handleStartMixing);
  server.on("/getESPData", handleGetESPData);
  server.begin();
}

void loop()
{
  server.handleClient();
  handleOTA();

  if (isTimerOn)
  {
    if (millis() - previousMillis >= 1000)
    {
      previousMillis = millis();
      if (remainingSeconds == 599)
      {
        digitalWrite(RELAYPIN, 0);
      }
      handleTimer();
      if (remainingSeconds == 0)
      {
        isTimerOn = false;
      }
    }
  }
}