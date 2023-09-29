#include <WiFi.h>
#include <TFT_eSPI.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <vector>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "orb.h"
#include "frame.h"

TFT_eSPI tft = TFT_eSPI();

// const int pwmFreq = 5000;
// const int pwmResolution = 8;
// const int pwmLedChannelTFT = 0;

const char* ssid = "xxxxxx";      //edit
const char* password = "xxxxxx";  //edit

int timeZone = 2;                 //edit
#define gray 0x39C7
#define dblue 0x01A9
#define purple 0xF14F
#define green 0x2D51

String payload = "";
const String endpoint = "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=eur&include_last_updated_at=true&include_24hr_change=true";

double current = 0;
double last = 0;

double readings[20] = { 0 };
int n = 0;
int fromtop = 80;
int f = 0;  //frame in animation

double minimal;
double maximal;

int p[20] = { 0 };

unsigned long currTime = 0;
unsigned long refresh = 120000;

// int deb = 0;
// int brightnes[5] = { 40, 80, 120, 160, 200 };
// int b = 1;

int spe = 0;  //speed of animation/

StaticJsonDocument<6000> doc;

void setup() {
  // pinMode(35,INPUT_PULLUP);
  Serial.begin(9600);
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);

  // ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  // ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  // ledcWrite(pwmLedChannelTFT, brightnes[b]);

  WiFi.begin(ssid, password);
  tft.print("Connecting");


  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    tft.print(".");
  }
  tft.println("CONNECTED!");
  // delay(1000);
  tft.print("Retrieving data...");
  getData();
}

void loop() {
  if (millis() > currTime + refresh) {
    tft.drawString("Refreshing...", 244, 6);
    getData();
    currTime = millis();
  }

  if (spe > 8000) {
    tft.pushImage(4, 32, 38, 38, frame[f]);
    spe = 0;
    f++;
    if (f == 59)
      f = 0;
  }

  // if (digitalRead(35) == 0) {
  //   if (deb == 0) {
  //     deb = 1;
  //     b++;
  //     if (b == 6)
  //       b = 0;
  //     ledcWrite(pwmLedChannelTFT, brightnes[b]);
  //   }
  // } else deb = 0;

  spe++;
}

void getData() {
  if ((WiFi.status() == WL_CONNECTED)) {  //Check the current connection status
    HTTPClient http;
    http.begin(endpoint);       //Specify the URL
    int httpCode = http.GET();  //Make the request
    if (httpCode > 0) {         //Check for the returning code
      payload = http.getString();
      char inp[payload.length()];
      payload.toCharArray(inp, payload.length());
      deserializeJson(doc, inp);

      String v = doc["bitcoin"]["eur"];
      double c = doc["bitcoin"]["eur_24h_change"];
      String t = doc["bitcoin"]["last_updated_at"];
      Serial.print(t);
      unsigned long t1 = t.toInt() + (timeZone * 3600);
      //day(t), month(t), year(t), hour(t), minute(t), second(t)

      tft.fillScreen(TFT_BLACK);
      //tft.fillRect(200,126,4,4,TFT_GREEN);
      tft.fillRect(46, 40, 56, 28, dblue);

      //tft.fillRect(118,22,120,100,dblue);

      for (int i = 0; i < 20; i++)
        tft.drawLine(128 + (i * 10), 22, 128 + (i * 10), 152, gray);
      for (int i = 0; i < 13; i++)
        tft.drawLine(128, 22 + (i * 10), 318, 22 + (i * 10), gray);
      tft.drawLine(128, 22, 128, 152, TFT_WHITE);
      tft.drawLine(128, 152, 318, 152, TFT_WHITE);

      tft.setTextColor(TFT_WHITE, dblue);
      tft.drawString("Updated:", 50, 45);
      String leadingZeroMinute = "";
      if (minute(t1) < 10)
        leadingZeroMinute = "0";
      String leadingZeroHour = "";
      if (hour(t1) < 10)
        leadingZeroHour = "0";
      tft.drawString(leadingZeroHour + String(hour(t1)) + ":" + leadingZeroMinute + String(minute(t1)), 50, 55);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);

      current = v.toDouble();
      tft.drawString("PRICE (EUR):", 4, fromtop + 4, 2);
      tft.drawString("CHANGE (24H):", 4, fromtop + 42 + 8, 2);
      tft.setFreeFont(&Orbitron_Medium_16);
      tft.setTextColor(green, TFT_BLACK);
      tft.drawString(String(current), 4, fromtop + 20);
      tft.setTextColor(TFT_ORANGE, TFT_BLACK);
      tft.drawString("Bitcoin", 4, 0);
      tft.setTextColor(green, TFT_BLACK);

      tft.drawString(String(c), 4, fromtop + 56 + 10, 2);
      tft.setTextColor(0x0B52, TFT_BLACK);
      tft.setTextFont(1);
      tft.drawString("LAST 20 READINGS", 128, 6);
      tft.setTextColor(TFT_ORANGE, TFT_BLACK);
      tft.setTextFont(1);
      tft.drawString("MAX", 104, 16);
      tft.drawString("MIN", 104, 152, 1);
      last = current;

      if (n < 20) {
        readings[n] = current;
        n++;
      } else {
        for (int i = 1; i < 20; i++)
          readings[i - 1] = readings[i];
        readings[11] = current;
      }

      minimal = readings[0];
      maximal = readings[0];

      for (int i = 0; i < n; i++) {

        if (readings[i] < minimal)
          minimal = readings[i];
        if (readings[i] > maximal)
          maximal = readings[i];

        int mx = maximal / 2;
        int mi = minimal / 2;
        int re = readings[i] / 2;
        //tft.drawString(String(i)+"."+String(readings[i]),120,i*10);
      }
      int mx = maximal / 2;
      int mi = minimal / 2;

      for (int i = 0; i < n; i++) {
        int re = readings[i] / 2;
        p[i] = map(re, mi, mx, 0, 100);
        // tft.drawString(String(p[i]),190,i*10);
      }
      if (n >= 1)
        for (int i = 1; i < n; i++) {
          tft.drawLine(128 + ((i - 1) * 10), 152 - p[i - 1], 128 + ((i)*10), 152 - p[i], TFT_RED);
          tft.fillCircle(128 + ((i - 1) * 10), 152 - p[i - 1], 2, TFT_RED);
          tft.fillCircle(128 + ((i)*10), 152 - p[i], 2, TFT_RED);
        }
      //tft.drawString(String(minimal),190,10);
      //tft.drawString(String(maximal),190,20);
    }
  }
}
