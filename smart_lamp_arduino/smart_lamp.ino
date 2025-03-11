// Натискання для зміни яскравості
// Утримування для зміни режиму світіння
// Натисність два рази щоб вимкнути датчик світла
#include <FastLED.h>    //бібліотека для зручного керування світлодіодами
#include <EncButton.h> //бібліотека для підключення кнопки
#include <TimerMs.h>

#define BTN_PIN 2       //пін кнопки
#define LED_PIN 4       //пін керування світлодіодами
#define PHOTORES_PIN 3  //пін датчику освітленості
#define COLOR_ORDER GRB
#define CHIPSET WS2811  //модель контролеру управляюмих світлодіодів
#define NUM_LEDS 8      //кількіть управляюмих світлодіодів
#define FRAMES_PER_SECOND 60
#define COOLING  55
#define SPARKING 120
//змінні та іх назви в скетчі
bool photores_status, gReverseDirection = false;
byte baza = 0, BRIGHTNESS = 120; //початкова яскравітсть
int8_t mode = 1, power_mode = 1, counter = 5;
CRGB leds[NUM_LEDS];
Button btn(BTN_PIN, INPUT, HIGH);
TimerMs tmr(1000, 1, 1);

void setup() {
  delay(3000);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  pinMode(PHOTORES_PIN, INPUT);
  //Serial.begin(9600);
  //Serial.print("Power mode - ");
  //Serial.println(power_mode);
  //Serial.print("Mode - ");
  //Serial.println(mode);
  //Serial.print("Brightness - ");
  //Serial.println(BRIGHTNESS);
  tmr.setPeriodMode();
  photores_status = digitalRead(PHOTORES_PIN);
}

void loop() {
  btn.tick();
  if (btn.hasClicks(2)) {
    power_mode += 1;
    if (power_mode > 3) power_mode = 1;
    //Serial.print("Power mode - ");
    //Serial.println(power_mode);
  }
  if (btn.click()) {
    delay(100);
    BRIGHTNESS += 60;
    if ((BRIGHTNESS > 240) || (BRIGHTNESS == 44)) BRIGHTNESS = 60;
    //Serial.print("Brightness - ");
    //Serial.println(BRIGHTNESS);
  }
  if (btn.hold()) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Blue;
    }
    FastLED.show();
    delay(1000);
    mode += 1;
    if (mode > 7) mode = 1;
    //Serial.print("Mode - ");
    //Serial.println(mode);
  }
  check_photores();
  if (((photores_status == true) && (power_mode == 1)) || (power_mode == 2)) {
    FastLED.setBrightness(BRIGHTNESS);
    if (mode == 1) {
      rainbow();
    } else if (mode == 2) {
      rainbow_2(); 
    } else if (mode == 3) {
      fire();
    } else if (mode == 4) {
      fire_2();
    } else if (mode == 5) {
      cyclone();
    } else if (mode == 6) {
      focus();
    } else if (mode == 7) {
      confetti();
    }
  }
  if (((photores_status == false) && (power_mode == 1)) || (power_mode == 3)) {
    FastLED.setBrightness(0);
    FastLED.show();
    delay(20);
    //delay(1000);
  }
}
// фільтр від самоактивації датчику
void check_photores() {
  bool status;
  if (tmr.tick()) {
    status = digitalRead(PHOTORES_PIN);
    if (status == true) {
      counter += 1;
      //Serial.print("Counter - ");
      //Serial.println(counter);
    } else {
      counter -= 1;
      //Serial.print("Counter - ");
      //Serial.println(counter);
    }
  }
  if (counter == 10) {
    photores_status = true;
    counter = 5;
  } else if (counter == 0){
    photores_status = false;
    counter = 5;
  }
}

//режим вогню
void fire()
{
  static byte heat[NUM_LEDS];
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }
    for( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}
//режим веселки
 void rainbow()
 {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(baza+ i * 5, 255, 255);
    }
    baza++;
    FastLED.show();
    delay(20);
 }

 void rainbow_2()
 {
   for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(baza+ i * 5, 255, 255);
    }
    baza++;
    FastLED.show();
    delay(20);
 }
//режим вогників
 void fire_2()
 {
    fadeToBlackBy(leds, NUM_LEDS, 2);
    int pos = beatsin16(13, 0, NUM_LEDS - 1);
    leds[pos] += CHSV(baza++, 255, 192);
    FastLED.show();
    delay(20);
 }
//режим циклону
void cyclone()
 {
   for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].nscale8(250);
      }
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(baza++, 255, 255);
    FastLED.show();
    delay(20);
    }
 }
//режим для фокусування
 void focus()
 {
    fadeToBlackBy(leds, NUM_LEDS, 2);
    for (int i = 0; i < 8; i++) {
      leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(baza+=16, 200, 255);
    }
    FastLED.show();
    delay(20);
 }
 //режим конфеті
 void confetti()
 {
   fadeToBlackBy(leds, NUM_LEDS, 2);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV(baza++ + random8(64), 200, 255);
    FastLED.show();
    delay(20);
 }
