#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>
#include <SettingsESP.h>
#include <WiFiConnector.h>
#include <Adafruit_NeoPixel.h>
#include <GTimer.h>
#define LED_PIN 2
#define MOSFET_PIN 5
#define NUM_LEDS 8
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

GyverDBFile db(&LittleFS, "/data.db");
GyverDB db_ram;
SettingsESP sett("Smart Lamp 💡", &db_ram);
//sets::Logger logger(500);
GTimer<millis> tmr;

bool wifi_setting_status;
int mode_status, red_, green_, blue_;

DB_KEYS(
    kk,
    wifi_ssid,
    wifi_pass,
    dynamic_flag,
    ssids,
    ssids_index,
    mode,
    brightness,
    red,
    green,
    blue,
    apply);

void scan_wifi() {
  if (db_ram[kk::dynamic_flag] == 1) {
    int scans_networks = WiFi.scanNetworks();
    db_ram[kk::ssids] = "";
    db_ram[kk::ssids_index] = 0;
    for (int i = 0; i < scans_networks; i++) {
      db_ram[kk::ssids] += String(WiFi.SSID(i) + ";");
    }
    db_ram[kk::dynamic_flag] = 2;
  }
}

void build(sets::Builder& b) {
  if (wifi_setting_status == true) {
    if (b.Button("Settings lamp")) {
      wifi_setting_status = false;
      sett.attachDB(&db_ram);
      b.reload();
    }
    if (b.beginGroup("Wi-Fi")) {
      if (db_ram[kk::dynamic_flag] == 2) b.Select(kk::ssids_index, "Found SSIDS:", db_ram[kk::ssids]);
      else b.Input(kk::wifi_ssid, "Manual input SSID:");
      b.Pass(kk::wifi_pass, "Password:");
      if (db_ram[kk::dynamic_flag] > 0) {
        if (b.Button("Cancel", sets::Colors::Red)) {
          db_ram[kk::dynamic_flag] = 0;
          db_ram[kk::wifi_pass] = "";
          sett.attachDB(&db);
          b.reload();
        }
      } else {
        if (b.Button("Find networks", sets::Colors::Blue)) {
          db_ram[kk::dynamic_flag] = 1;
          sett.attachDB(&db_ram);
          b.reload();
        }
      }
      if (b.Button("Connect")) {
        if (db_ram[kk::dynamic_flag] == 2) {
          db_ram[kk::wifi_ssid] = WiFi.SSID(db_ram[kk::ssids_index]);
          db[kk::wifi_ssid] = String(db_ram[kk::wifi_ssid]);
          db[kk::wifi_pass] = String(db_ram[kk::wifi_pass]);
          db_ram[kk::dynamic_flag] = 0;
          db_ram[kk::wifi_pass] = "";
        }
        Serial.println("Connect to: \n" + String(db[kk::wifi_ssid]) + "\n"+ String(db[kk::wifi_pass]) + "\n");
        WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);
        db.update();
        sett.attachDB(&db);
        b.reload();
      }
      b.endGroup();
    }
  } else {
    if (b.Button("Settings Wi-Fi", sets::Colors::Blue)) {
      wifi_setting_status = true;
      sett.attachDB(&db);
      b.reload();
    }
    if (b.beginGroup("LED")) {
      //b.Input(kk::led_num, "Led nums:");
      b.Slider(kk::brightness, "Brightness 💡", 0, 255, 1);
      b.Select(kk::mode, "Mode", "Rainbow;Fire;Random color;Change color;Confetti;Custom");
      if (b.Button("Update", sets::Colors::Aqua)) {
        strip.setBrightness(db_ram[kk::brightness]);
        mode_status = db_ram[kk::mode];
        red_ = db_ram[kk::red];
        green_ = db_ram[kk::green];
        blue_ = db_ram[kk::blue];
        tmr.start();
        //logger.println("tmr.start"); 
        b.reload();
      }
      b.endGroup();
    }
    if (db_ram[kk::mode] == 5) {
      if (b.beginGroup("Color")) {
        b.Slider(kk::red, "Red", 0, 255, 1);
        b.Slider(kk::green, "Green", 0, 255, 1);
        b.Slider(kk::blue, "Blue", 0, 255, 1);
        b.endGroup();
      }
    }
  }
  //b.Log(H(log), logger);
}

//void update(sets::Updater& upd) {
//  upd.update(H(log), logger);
//}

void rainbow() {
  static uint16_t hue = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.ColorHSV(hue + (i * 65536 / NUM_LEDS), 255, 255));
  }
  strip.show();
  hue += 256;
  delay(20);
}

void fire() {
  for (int i = 0; i < NUM_LEDS; i++) {
    int flicker = random(0, 150);
    int r = 255 - flicker;
    int g = 100 - flicker / 2;
    int b = 0;
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
  delay(random(50, 150));
}

void random_color() {
  int index = random(NUM_LEDS);
  strip.setPixelColor(index, strip.Color(random(255), random(255), random(255)));
  strip.show();
  delay(100);
}

void color_switch() {
  static uint16_t hue = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.ColorHSV(hue, 255, 255));
  }
  strip.show();
  hue += 256;
  delay(50);
}

void custom() {
 for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(red_, green_, blue_));
  }
  strip.show();
  delay(10);
}

void confettiEffect() {
  int pos = random(NUM_LEDS);
  int r = random(256);
  int g = random(256);
  int b = random(256);
  strip.setPixelColor(pos, strip.Color(r, g, b));
  strip.show();
  delay(50);

  for (int i = 0; i < NUM_LEDS; i++) {
    uint32_t color = strip.getPixelColor(i);
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    if (r > 10) r -= 10; else r = 0;
    if (g > 10) g -= 10; else g = 0;
    if (b > 10) b -= 10; else b = 0;
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
}

void colorExplosion() {
  int pos = random(NUM_LEDS);
  int r = random(256);
  int g = random(256);
  int b = random(256);
  strip.setPixelColor(pos, strip.Color(r, g, b));
  strip.show();
  delay(50);
  for (int i = 0; i < NUM_LEDS; i++) {
    uint32_t color = strip.getPixelColor(i);
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    if (r > 10) r -= 10; else r = 0;
    if (g > 10) g -= 10; else g = 0;
    if (b > 10) b -= 10; else b = 0;
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
}

void db_update() {
  if (tmr) {
    db.set(kk::mode, db_ram[kk::mode].toInt());
    db.set(kk::brightness, db_ram[kk::brightness].toInt());
    db.set(kk::red, db_ram[kk::red].toInt());
    db.set(kk::green, db_ram[kk::green].toInt());
    db.set(kk::blue, db_ram[kk::blue].toInt());
    db.update();  
    tmr.stop();
  }
}

void setup() {
  LittleFS.begin();
  db.begin();
  db.init(kk::wifi_ssid, "");
  db.init(kk::wifi_pass, "");
  db.init(kk::mode, 5);
  db.init(kk::brightness, 100);
  db.init(kk::red, 100);
  db.init(kk::green, 100);
  db.init(kk::blue, 100);
  db_ram.set(kk::mode, db[kk::mode].toInt());
  db_ram.set(kk::brightness, db[kk::brightness].toInt());
  db_ram.set(kk::red, db[kk::red].toInt());
  db_ram.set(kk::green, db[kk::green].toInt());
  db_ram.set(kk::blue, db[kk::blue].toInt());
  db_ram.init(kk::wifi_ssid, "");
  db_ram.init(kk::wifi_pass, "");
  db_ram.init(kk::dynamic_flag, false);
  db_ram.init(kk::ssids, "");
  db_ram.init(kk::ssids_index, 0);
  WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);
  sett.begin();
  sett.onBuild(build);
  //sett.onUpdate(update);
  delay(1000);
  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, HIGH);
  strip.begin();
  strip.show();
  strip.setBrightness(db_ram[kk::brightness]);
  mode_status = db_ram[kk::mode];
  tmr.setMode(GTMode::Overflow);
  tmr.setTime(300000);
  tmr.stop();
}

void loop() {
    WiFiConnector.tick();
    sett.tick();
    scan_wifi();
    db_update();
    switch (mode_status) {
      case 0:
        rainbow();
        break;
      case 1:
        fire();
        break;
      case 2:
        random_color();
        break;
      case 3:
        color_switch();
        break;
      case 4:
        confettiEffect();
        break;
      case 5:
        custom();
        break;
    }
}