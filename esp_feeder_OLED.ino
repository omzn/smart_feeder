/*
   ESP-WROOM-02 remote feeder
   MPU:  ESP-WROOM-02
*/

#include "esp_feeder.h" // Configuration parameters

#include <pgmspace.h>
#include <Wire.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <RTClib.h>
#include <FS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Fonts/FreeSans9pt7b.h>
//#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

#include "feeder.h"
#include "ntp.h"

#define OLED_RESET 0

const uint8_t PROGMEM aq[2][128] = {{
    0x00, 0x00, 0x00, 0x08, 0x00, 0x0f, 0xfc, 0xfc, 0x00, 0x1f, 0x03, 0xfe, 0x00, 0xff, 0x87, 0xfe,
    0x01, 0xbf, 0xc1, 0xfe, 0x01, 0x07, 0xc0, 0xfe, 0x02, 0x03, 0xf0, 0x7c, 0x02, 0x00, 0xf8, 0x3c,
    0x04, 0x00, 0x7c, 0x02, 0x09, 0x00, 0x3f, 0x06, 0x08, 0x00, 0x0f, 0x82, 0x0f, 0xfc, 0x07, 0xe2,
    0x0f, 0xff, 0xc1, 0xf2, 0x0f, 0xff, 0xe0, 0xfc, 0x03, 0xf3, 0xf8, 0x7c, 0x03, 0xf3, 0xfe, 0x18,
    0x03, 0xf3, 0xff, 0x10, 0x03, 0xf3, 0xff, 0xf0, 0x03, 0xfc, 0xff, 0xf0, 0x00, 0x3c, 0xff, 0xe0,
    0x03, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0x80, 0x00, 0xff, 0xff, 0x00, 0x00, 0x7f, 0xff, 0x00,
    0x00, 0x47, 0xf3, 0xe0, 0x00, 0x47, 0xf3, 0xf0, 0x00, 0x20, 0xff, 0xfe, 0x00, 0x30, 0xff, 0xff,
    0x00, 0x0f, 0xf0, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  }, {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x1f, 0xf9, 0xf8, 0x00, 0x3e, 0x07, 0xfc,
    0x01, 0xff, 0x0f, 0xfc, 0x03, 0x7f, 0x83, 0xfc, 0x02, 0x0f, 0x81, 0xfc, 0x04, 0x07, 0xe0, 0xf8,
    0x04, 0x01, 0xf0, 0x78, 0x08, 0x00, 0xf8, 0x04, 0x12, 0x00, 0x7e, 0x0c, 0x10, 0x00, 0x1f, 0x04,
    0x1f, 0xfe, 0x0f, 0xc4, 0x1f, 0xff, 0x87, 0xe4, 0x1f, 0xff, 0xe0, 0xf8, 0x07, 0xe7, 0xf8, 0x78,
    0x07, 0xe7, 0xfc, 0x30, 0x07, 0xe7, 0xff, 0x20, 0x07, 0xe7, 0xff, 0xe0, 0x07, 0xf9, 0xff, 0xe0,
    0x00, 0x39, 0xff, 0xc0, 0x00, 0x7f, 0xff, 0x80, 0x03, 0xff, 0xff, 0x00, 0x01, 0xff, 0xfe, 0x00,
    0x00, 0xff, 0xfe, 0x06, 0x00, 0x8f, 0xe7, 0xfc, 0x00, 0x8f, 0xe7, 0xf8, 0x00, 0x41, 0xff, 0xf0,
    0x00, 0x61, 0xff, 0x80, 0x00, 0xff, 0xfc, 0x00, 0x00, 0xff, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00,
  }
};

const String website_name  = "espfeeder";
const char* apSSID         = "WIFI_FEED_TAN";
String sitename;
boolean settingMode;
String ssidList;

String oled_contents[4];

uint32_t timer_count = 0;
int last_reset_day = 0;

uint32_t p_millis;
int16_t text_x = 128;

DNSServer dnsServer;
MDNSResponder mdns;
const IPAddress apIP(192, 168, 1, 1);
ESP8266WebServer webServer(80);

RTC_Millis rtc;

NTP ntp("ntp.nict.jp");
Adafruit_SSD1306 display(OLED_RESET);

drumFeeder feeder(DRV8830_1_ADDRESS, PIN_FEED_SW);

/* Setup and loop */

void setup() {
  p_millis = millis();
#ifdef DEBUG
  Serial.begin(115200);
#endif
  EEPROM.begin(512);
  pinMode(PIN_FEED_SW, INPUT_PULLUP);
  Wire.begin(PIN_SDA, PIN_SCL);
  delay(10);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  //display.begin(SSD1306_EXTERNALVCC, 0x3c);
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(0x5F);
  display.display();
  display.clearDisplay();
  display.setTextWrap(false);
  SPIFFS.begin();
  rtc.begin(DateTime(2017, 1, 1, 0, 0, 0));
#ifdef DEBUG
  Serial.println("RTC began");
#endif
  sitename = website_name;

  //  analogWrite(PIN_LIGHT,512);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  if (restoreConfig()) {
    if (checkConnection()) {
      if (mdns.begin(sitename.c_str(), WiFi.localIP())) {
#ifdef DEBUG
        Serial.println("MDNS responder started.");
#endif
      }
      settingMode = false;

      ntp.begin();

      startWebServer_normal();
      return;
    } else {
      settingMode = true;
    }
  } else {
    settingMode = true;
  }

  if (settingMode == true) {
#ifdef DEBUG
    Serial.println("Setting mode");
#endif
    //WiFi.mode(WIFI_STA);
    //WiFi.disconnect();
#ifdef DEBUG
    Serial.println("Wifi disconnected");
#endif
    delay(100);
    WiFi.mode(WIFI_AP);
#ifdef DEBUG
    Serial.println("Wifi AP mode");
#endif
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(apSSID);
    dnsServer.start(53, "*", apIP);
#ifdef DEBUG
    Serial.println("Wifi AP configured");
#endif
    startWebServer_setting();
#ifdef DEBUG
    Serial.print("Starting Access Point at \"");
    Serial.print(apSSID);
    Serial.println("\"");
#endif
    //    display.setFont(&FreeSans9pt7b);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Connect to SSID:");
    display.println(apSSID);
    display.display();
  }
  ESP.wdtEnable(100);
}



void loop() {
  int pause = 0;
  ESP.wdtFeed();
  if (settingMode) {
    dnsServer.processNextRequest();
  }
  webServer.handleClient();
  // 100ms秒毎に実行
  if (millis() > p_millis + 100) {
    p_millis = millis();
    if (timer_count % (3600 * 10) == 0) {
      uint32_t epoch = ntp.getTime();
      if (epoch > 0) {
        rtc.adjust(DateTime(epoch + SECONDS_UTC_TO_JST ));
      }
#ifdef DEBUG
      Serial.print("epoch:");
      Serial.println(epoch);
#endif
    }
    DateTime now = rtc.now();
    if (now.day() != last_reset_day && feeder.fed() > 0) {
      feeder.reset();
      last_reset_day = now.day();
      EEPROM.write(EEPROM_DAYRESET_ADDR, char(now.day()));
      EEPROM.commit();
    }

    display.clearDisplay();
    display.setTextColor(WHITE);

    if (timer_count % 10 > 5) {
      pause = 1;
    } else {
      pause = 0;
    }

    //oled_contents[1] = timestamp_YYDDMMhhmm();
    oled_contents[0] = "# Fed:" + (String)feeder.fed() +
                       " Next:" + feeder.nextFeedTime();
    text_x -= 2;
    if (text_x < -255) {
      text_x = 128;
    }
    display.setFont(&FreeSans12pt7b);
    display.drawBitmap(text_x, 0,  aq[pause], 32, 32, WHITE);
    display.setCursor(text_x + 35, 27);
    display.print(oled_contents[0]);
    display.display();
    delay(1);

    feeder.control(now.hour(), now.minute());
    timer_count++;
    timer_count %= (86400UL) * 10;
  }
  delay(1);
}

/***************************************************************
   EEPROM restoring functions
 ***************************************************************/

boolean restoreConfig() {
#ifdef DEBUG
  Serial.println("Reading EEPROM...");
#endif
  String ssid = "";
  String pass = "";

  // Initialize on first boot
  if (EEPROM.read(0) == 255) {
#ifdef DEBUG
    Serial.println("Initialize EEPROM...");
#endif
    for (int i = 0; i < EEPROM_LAST_ADDR; ++i) {
      EEPROM.write(i, 0);
    }
#ifdef DEBUG
    Serial.println("Erasing EEPROM...");
#endif
    EEPROM.commit();
  }

#ifdef DEBUG
  Serial.println("Reading EEPROM(2)...");
#endif

  int e_schedule  = EEPROM.read(EEPROM_SCHEDULE_ADDR) == 1 ? 1 : 0;
  feeder.schedule(e_schedule == 0 ? 0 : 1);

  for (int i = 0; i < 3; i++) {
    int e_ft_h  = EEPROM.read(EEPROM_SCHEDULE_ADDR + i * 3 + 1);
    int e_ft_m  = EEPROM.read(EEPROM_SCHEDULE_ADDR + i * 3 + 2);
    int e_ft_r  = EEPROM.read(EEPROM_SCHEDULE_ADDR + i * 3 + 3);
    feeder.schedule(i, e_ft_h, e_ft_m, e_ft_r);
  }

  int r  = EEPROM.read(EEPROM_FEEDER_ADDR);
  int f  = EEPROM.read(EEPROM_FEEDER_ADDR + 1);
#ifdef DEBUG
  Serial.print("rotated:");
  Serial.println(r);
  Serial.print("fed:");
  Serial.println(f);
#endif
  feeder.rotated(r);
  feeder.fed(f);

  last_reset_day  = EEPROM.read(EEPROM_DAYRESET_ADDR);

  if (EEPROM.read(EEPROM_SSID_ADDR) != 0) {
    for (int i = EEPROM_SSID_ADDR; i < EEPROM_SSID_ADDR + 32; ++i) {
      ssid += char(EEPROM.read(i));
    }
    for (int i = EEPROM_PASS_ADDR; i < EEPROM_PASS_ADDR + 64; ++i) {
      pass += char(EEPROM.read(i));
    }
#ifdef DEBUG
    Serial.print("ssid:");
    Serial.println(ssid);
    Serial.print("pass:");
    Serial.println(pass);
#endif
    delay(100);
    WiFi.begin(ssid.c_str(), pass.c_str());
#ifdef DEBUG
    Serial.println("WiFi started");
#endif
    delay(100);
    if (EEPROM.read(EEPROM_MDNS_ADDR) != 0) {
      sitename = "";
      for (int i = 0; i < 32; ++i) {
        byte c = EEPROM.read(EEPROM_MDNS_ADDR + i);
        if (c == 0) {
          break;
        }
        sitename += char(c);
      }
#ifdef DEBUG
      Serial.print("sitename:");
      Serial.println(sitename);
#endif
    }
    return true;
  } else {
#ifdef DEBUG
    Serial.println("restore config fails...");
#endif
    return false;
  }
}

/***************************************************************
   Network functions
 ***************************************************************/

boolean checkConnection() {
  int count = 0;
  while ( count < 60 ) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
#ifdef DEBUG
    Serial.print(".");
#endif
    count++;
  }
#ifdef DEBUG
  Serial.println("Timed out.");
#endif
  return false;
}

/***********************************************************
   WiFi Client functions

 ***********************************************************/

/***************************************************************
   Web server functions
 ***************************************************************/

void startWebServer_setting() {
#ifdef DEBUG
  Serial.print("Starting Web Server at ");
  Serial.println(WiFi.softAPIP());
#endif
  webServer.on("/pure.css", handleCss);
  webServer.on("/setap", []() {
#ifdef DEBUG
    Serial.print("Set AP ");
    Serial.println(WiFi.softAPIP());
#endif
    for (int i = 0; i < EEPROM_MDNS_ADDR; ++i) {
      EEPROM.write(i, 0);
    }
    String ssid = urlDecode(webServer.arg("ssid"));
    String pass = urlDecode(webServer.arg("pass"));
    String site = urlDecode(webServer.arg("site"));
    for (int i = 0; i < ssid.length(); ++i) {
      EEPROM.write(EEPROM_SSID_ADDR + i, ssid[i]);
    }
    for (int i = 0; i < pass.length(); ++i) {
      EEPROM.write(EEPROM_PASS_ADDR + i, pass[i]);
    }
    if (site != "") {
      for (int i = EEPROM_MDNS_ADDR; i < EEPROM_MDNS_ADDR + 32; ++i) {
        EEPROM.write(i, 0);
      }
      for (int i = 0; i < site.length(); ++i) {
        EEPROM.write(EEPROM_MDNS_ADDR + i, site[i]);
      }
    }
    EEPROM.commit();
    String s = "<h2>Setup complete</h2><p>Device will be connected to \"";
    s += ssid;
    s += "\" after the restart.</p><p>Your computer also need to re-connect to \"";
    s += ssid;
    s += "\".</p><p><button class=\"pure-button\" onclick=\"return quitBox();\">Close</button></p>";
    s += "<script>function quitBox() { open(location, '_self').close();return false;};setTimeout(\"quitBox()\",10000);</script>";
    webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    timer_count = 0;
  });
  webServer.onNotFound([]() {
#ifdef DEBUG
    Serial.println("captive webpage ");
#endif
    int n = WiFi.scanNetworks();
    delay(100);
    ssidList = "";
    for (int i = 0; i < n; ++i) {
      ssidList += "<option value=\"";
      ssidList += WiFi.SSID(i);
      ssidList += "\">";
      ssidList += WiFi.SSID(i);
      ssidList += "</option>";
    }
    String s = R"=====(
<div class="l-content">
<div class="l-box">
<h3 class="if-head">WiFi Setting</h3>
<p>Please enter your password by selecting the SSID.<br />
You can specify site name for accessing a name like http://aquamonitor.local/</p>
<form class="pure-form pure-form-stacked" method="get" action="setap" name="tm"><label for="ssid">SSID: </label>
<select id="ssid" name="ssid">
)=====";
    s += ssidList;
    s += R"=====(
</select>
<label for="pass">Password: </label><input id="pass" name="pass" length=64 type="password">
<label for="site" >Site name: </label><input id="site" name="site" length=32 type="text" placeholder="Site name">
<button class="pure-button pure-button-primary" type="submit">Submit</button></form>
</div>
</div>
)=====";
  webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
  });
  webServer.begin();
}

/*
 * Web server for normal operation
 */
void startWebServer_normal() {
#ifdef DEBUG
  Serial.print("Starting Web Server at ");
  Serial.println(WiFi.localIP());
#endif
  webServer.on("/reset", []() {
    for (int i = 0; i < EEPROM_LAST_ADDR; ++i) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();
    String s = "<h3 class=\"if-head\">Reset ALL</h3><p>Cleared all settings. Please reset device.</p>";
    s += "<p><button class=\"pure-button\" onclick=\"return quitBox();\">Close</button></p>";
    s += "<script>function quitBox() { open(location, '_self').close();return false;};</script>";
    webServer.send(200, "text/html", makePage("Reset ALL Settings", s));
    timer_count = 0;
  });
  webServer.on("/wifireset", []() {
    for (int i = 0; i < EEPROM_MDNS_ADDR; ++i) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();
    String s = "<h3 class=\"if-head\">Reset WiFi</h3><p>Cleared WiFi settings. Please reset device.</p>";
    s += "<p><button class=\"pure-button\" onclick=\"return quitBox();\">Close</button></p>";
    s += "<script>function quitBox() { open(location, '_self').close();return false;}</script>";
    webServer.send(200, "text/html", makePage("Reset WiFi Settings", s));
    timer_count = 0;
  });
  webServer.on("/", handleRoot);
  webServer.on("/pure.css", handleCss);
  webServer.on("/reboot", handleReboot);
  webServer.on("/feed", handleActionFeed);
  webServer.on("/feedreset", handleActionFeedReset);
  webServer.on("/status", handleStatus);
  webServer.on("/schedule", handleSchedule);
  webServer.begin();
}

void handleRoot() {
  send_fs("/index.html","text/html");  
}

void handleCss() {
  send_fs("/pure.css","text/css");  
}

void handleReboot() {
  String message;
  message = "{reboot:\"done\"}";
  webServer.send(200, "application/json", message);
  ESP.restart();
}

void handleActionFeed() {
  String message;

#ifdef DEBUG
  Serial.println("feed command");
#endif
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  int num  = webServer.arg("rotate").toInt();
  if (num < 1) {
    num = 1;
  }
  int success = feeder.doFeed(num);

  json["success"] = num;
  json["fed"] = feeder.fed();
  json["rotated"] = feeder.rotated();
  json["timestamp"] = timestamp();  
  json.printTo(message);
  webServer.send(200, "application/json", message);
}

void handleActionFeedReset() {
  String message;

#ifdef DEBUG
  Serial.println("feed reset command");
#endif
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  feeder.reset();

  json["fed"] = feeder.fed();
  json["rotated"] = feeder.rotated();
  json["timestamp"] = timestamp();  
  json.printTo(message);
  webServer.send(200, "application/json", message);
}

void handleSchedule() {
  String message,argname,argv;
  int enable;
  int ft1_h = -1, ft1_m, ft1_r;
  int ft2_h = -1, ft2_m, ft2_r;
  int ft3_h = -1, ft3_m, ft3_r;

#ifdef DEBUG
  Serial.println("schedule command");
#endif
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  for (int i = 0; i < webServer.args(); i++) {
    argname = webServer.argName(i);
    argv = webServer.arg(i);
#ifdef DEBUG
    Serial.print("argname:");
    Serial.print(argname);
    Serial.print(" = ");
    Serial.println(argv);
#endif
    if (argname == "enable") {
       enable = argv ==  "true" ? 1 : 0;
    } else if (argname == "ft1_h") {
       ft1_h  = argv.toInt();
    } else if (argname == "ft1_m") {
       ft1_m  = argv.toInt();
    } else if (argname == "ft1_r") {
       ft1_r  = argv.toInt();
    } else if (argname == "ft2_h") {
       ft2_h  = argv.toInt();
    } else if (argname == "ft2_m") {
       ft2_m  = argv.toInt();
    } else if (argname == "ft2_r") {
       ft2_r  = argv.toInt();
    } else if (argname == "ft3_h") {
       ft3_h  = argv.toInt();
    } else if (argname == "ft3_m") {
       ft3_m  = argv.toInt();
    } else if (argname == "ft3_r") {
       ft3_r  = argv.toInt();
    }
  }
  feeder.schedule(enable);
  EEPROM.write(EEPROM_SCHEDULE_ADDR, char(feeder.schedule()));
  for (int i = 0; i < 3; i++) {
    if (i == 0 && ft1_h >= 0) {
      feeder.schedule(0,ft1_h,ft1_m,ft1_r);
    } else if (i == 1 && ft2_h >= 0) {  
      feeder.schedule(1,ft2_h,ft2_m,ft2_r);
    } else if (i == 2 && ft3_h >= 0) {  
      feeder.schedule(2,ft3_h,ft3_m,ft3_r);
    }
    EEPROM.write(EEPROM_SCHEDULE_ADDR + i*3+1, char(feeder.feedtime_h(i)));
    EEPROM.write(EEPROM_SCHEDULE_ADDR + i*3+2, char(feeder.feedtime_m(i)));
    EEPROM.write(EEPROM_SCHEDULE_ADDR + i*3+3, char(feeder.feedtime_r(i)));
  }
  EEPROM.commit();

  json["enable_schedule"] = feeder.schedule();
  json["ft1_h"] = feeder.feedtime_h(0);
  json["ft1_m"] = feeder.feedtime_m(0);
  json["ft1_r"] = feeder.feedtime_r(0);
  json["ft2_h"] = feeder.feedtime_h(1);
  json["ft2_m"] = feeder.feedtime_m(1);
  json["ft2_r"] = feeder.feedtime_r(1);
  json["ft3_h"] = feeder.feedtime_h(2);
  json["ft3_m"] = feeder.feedtime_m(2);
  json["ft3_r"] = feeder.feedtime_r(2);
  json["timestamp"] = timestamp();  
  json.printTo(message);
  webServer.send(200, "application/json", message);
}

void handleStatus() {
  String message;
#ifdef DEBUG
  Serial.println("status command");
#endif
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["fed"] = feeder.fed();
  json["rotated"] = feeder.rotated();
  json["enable_schedule"] = feeder.schedule();
  json["ft1_h"] = feeder.feedtime_h(0);
  json["ft1_m"] = feeder.feedtime_m(0);
  json["ft1_r"] = feeder.feedtime_r(0);
  json["ft2_h"] = feeder.feedtime_h(1);
  json["ft2_m"] = feeder.feedtime_m(1);
  json["ft2_r"] = feeder.feedtime_r(1);
  json["ft3_h"] = feeder.feedtime_h(2);
  json["ft3_m"] = feeder.feedtime_m(2);
  json["ft3_r"] = feeder.feedtime_r(2);
  json["timestamp"] = timestamp();  
  json.printTo(message);
  webServer.send(200, "application/json", message);
}

String timestamp() {
  String ts;
  DateTime now = rtc.now();
  char str[20];
  sprintf(str,"%04d-%02d-%02d %02d:%02d:%02d",now.year(),now.month(),now.day(),now.hour(),now.minute(),now.second());
  ts = str;
  return ts; 
}

String timestamp_YYDDMMhhmm() {
  String ts;
  DateTime now = rtc.now();
  char str[20];
  sprintf(str,"%04d-%02d-%02d %02d:%02d",now.year(),now.month(),now.day(),now.hour(),now.minute());
  ts = str;
  return ts; 
}

void send_fs (String path,String contentType) {
  if(SPIFFS.exists(path)){
    File file = SPIFFS.open(path, "r");
    size_t sent = webServer.streamFile(file, contentType);
    file.close();
  } else{
    webServer.send(500, "text/plain", "BAD PATH");
  }  
}

String makePage(String title, String contents) {
  String s = R"=====(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<link rel="stylesheet" href="/pure.css">
)=====";  
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += R"=====(
<div class="footer l-box">
<p>WiFi Aquatan-Light by @omzn 2017 / All rights researved</p>
</div>
)=====";
  s += "</body></html>";
  return s;
}

String urlDecode(String input) {
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}

/*
   NTP related functions
*/
/*
// send an NTP request to the time server at the given address
void sendNTPpacket(const char* address) {
  //  Serial.print("sendNTPpacket : ");
  //  Serial.println(address);

  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0]  = 0b11100011;   // LI, Version, Mode
  packetBuffer[1]  = 0;     // Stratum, or type of clock
  packetBuffer[2]  = 6;     // Polling Interval
  packetBuffer[3]  = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

uint32_t readNTPpacket() {
  //  Serial.println("Receive NTP Response");
  udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
  unsigned long secsSince1900 = 0;
  // convert four bytes starting at location 40 to a long integer
  secsSince1900 |= (unsigned long)packetBuffer[40] << 24;
  secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
  secsSince1900 |= (unsigned long)packetBuffer[42] <<  8;
  secsSince1900 |= (unsigned long)packetBuffer[43] <<  0;
  return secsSince1900 - 2208988800UL; // seconds since 1970
}

uint32_t getNTPtime() {
  while (udp.parsePacket() > 0) ; // discard any previously received packets
#ifdef DEBUG
  Serial.println("Transmit NTP Request");
#endif
  sendNTPpacket(timeServer);

  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      return readNTPpacket();
    }
  }

  return 0; // return 0 if unable to get the time
}

void lcd_init(int contrast) {
  delay(100);
  lcd_cmd(0x34);
  delay(5);
  lcd_cmd(0x34);
  delay(5);
  lcd_cmd(0x34);
  delay(40);

  Wire.beginTransmission(LCD_ADDR);
  Wire.write(0x00); // CO = 0,RS = 0
  Wire.write(0x35);
  Wire.write(0x41);
  Wire.write(0x80 | contrast);
  Wire.write(0xC0 | contrast);
  Wire.write(0x34);
  Wire.endTransmission();

  lcd_cmd(0x01);
  delay(400);

  lcd_cmd(0x0C);
  lcd_cmd(0x06);

  delay(500);
}

void lcd_cmd(unsigned char x) {
  Wire.beginTransmission(LCD_ADDR);
  Wire.write(0b00000000); // CO = 0,RS = 0
  Wire.write(x);
  Wire.endTransmission();
}

void lcd_data(unsigned char x){
  Wire.beginTransmission(LCD_ADDR);
  Wire.write(0b01000000); // CO = 0, RS = 1
  Wire.write(x ^ 0x80);
  Wire.endTransmission();
}

// 文字の表示
void lcd_printStr(const char *s) {
  Wire.beginTransmission(LCD_ADDR);
  while (*s) {
    if (*(s + 1)) {
      Wire.write(0b11000000); // CO = 1, RS = 1
      Wire.write(*s ^ 0x80);
    } else {
      Wire.write(0b01000000); // CO = 0, RS = 1
      Wire.write(*s ^ 0x80);
    }
    s++;
  }
  Wire.endTransmission();
}

// 表示位置の指定
void lcd_setCursor(unsigned char x, unsigned char y) {
  lcd_cmd(0x80 | (y * 0x40 + x));
}
*/

