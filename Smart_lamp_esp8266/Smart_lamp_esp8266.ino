#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>
#include <SettingsESP.h>
#include <WiFiConnector.h>
#include <Adafruit_NeoPixel.h>
#define LED_PIN 2
#define MOSFET_PIN 5
#define NUM_LEDS 8
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

GyverDBFile db(&LittleFS, "/data.db");
SettingsESP sett("Лампа", &db);

DB_KEYS(
    kk,
    wifi_ssid,
    wifi_pass,
    //led_num,
    mode,
    brightness,
    red,
    green,
    blue,
    apply);

void build(sets::Builder& b) {
  if (b.beginGroup("Wi-Fi")) {
    b.Input(kk::wifi_ssid, "Имя Wi-Fi сети");
    b.Pass(kk::wifi_pass, "Пароль");
    if (b.Button(kk::apply, "Подключить")) {
      db.update();
      WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);
    }
    b.endGroup();
  }
  if (b.beginGroup("LED")) {
    //b.Input(kk::led_num, "Количество светодиодов");
    b.Slider(kk::brightness, "Яркость 💡", 0, 255, 1);
    b.Select(kk::mode, "Режим", "Радуга;Огонь;Случайный цвет;Смена цвета;Конфетти;Свой цвет");
    db.update();
    strip.setBrightness(db[kk::brightness]);
    b.reload();
    b.endGroup();
  }
  if (db[kk::mode] == 5) {
    if (b.beginGroup("Параметры цвета")) {
      b.Slider(kk::red, "Red", 0, 255, 1);
      b.Slider(kk::green, "Green", 0, 255, 1);
      b.Slider(kk::blue, "Blue", 0, 255, 1);
      db.update();
      b.endGroup();
    }
  }
}

void setup() {
    Serial.begin(115200);
    Serial.println();

#ifdef ESP32
    LittleFS.begin(true);
#else
    LittleFS.begin();
#endif
    db.begin();
    db.init(kk::wifi_ssid, "");
    db.init(kk::wifi_pass, "");
    //db.init(kk::led_num, 1);
    db.init(kk::mode, 4);
    db.init(kk::brightness, 123);
    db.init(kk::red, 100);
    db.init(kk::green, 100);
    db.init(kk::blue, 100);

    WiFiConnector.onConnect([]() {
        Serial.print("Connected! ");
        Serial.println(WiFi.localIP());
    });
    WiFiConnector.onError([]() {
        Serial.print("Error! start AP ");
        Serial.println(WiFi.softAPIP());
    });

    WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);

    sett.begin();
    sett.onBuild(build);
    delay(1000);
    pinMode(MOSFET_PIN, OUTPUT);
    digitalWrite(MOSFET_PIN, HIGH);
    strip.begin();
    strip.show();
    strip.setBrightness(db[kk::brightness]);
}

void loop() {
    WiFiConnector.tick();
    sett.tick();
    if (db[kk::mode] == 0) rainbow();
    if (db[kk::mode] == 1) fire();
    if (db[kk::mode] == 2) random_color();
    if (db[kk::mode] == 3) color_switch();
    if (db[kk::mode] == 4) confettiEffect();
    //if (db[kk::mode] == 5) pulseEffect(db[kk::red], db[kk::green], db[kk::blue]);
    if (db[kk::mode] == 5) custom();
}

void rainbow() {
  static uint16_t hue = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.ColorHSV(hue + (i * 65536 / NUM_LEDS), 255, 255));
  }
  strip.show();
  hue += 256;  // Изменяем оттенок для плавного перехода
  delay(20);
}

/*
void rainbow() {
  for (int j = 0; j < 256; j++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(20);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
*/
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
    strip.setPixelColor(i, strip.Color(db[kk::red], db[kk::green], db[kk::blue]));
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
/*
void pulseEffect(uint8_t r, uint8_t g, uint8_t b) {
  for (int brightness = 0; brightness < 256; brightness++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color(r * brightness / 255, g * brightness / 255, b * brightness / 255));
    }
    strip.show();
    delay(10);
  }
  for (int brightness = 255; brightness >= 0; brightness--) {
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color(r * brightness / 255, g * brightness / 255, b * brightness / 255));
    }
    strip.show();
    delay(10);
  }
}
*/