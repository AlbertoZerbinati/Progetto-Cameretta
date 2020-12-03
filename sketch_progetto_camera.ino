#include <FastLED.h>
#include <Ultrasonic.h>
#include <LiquidCrystal.h>
#include "rotary_encoder.h"

#define NUM_LEDS 144
#define LED_DATA_PIN 13
#define RASPIN 6

#define NUM_MODES 7
#define OFF 0
#define RGBMODE 1
#define WHITE 2
#define COLD 3
#define WARM 4
#define PICK 5
#define TEMP 6

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
CRGB leds[NUM_LEDS];
RotaryEncoder re;

//per colori RGB
int red = 255, green = 0, blue = 0;

//per switch mode
int mode = OFF;
int active_mode = OFF;
int prev_active_mode = active_mode;
String modes[NUM_MODES] = {"Off", "RGB", "White", "Cold", "Warm",  "Pick a color!", "Temperature"};

//INPUT (encoder rotativo)
const int pinKey = 7;
const int pinS1 = 8;
const int pinS2 = 9;

//INPUT analog potenziometro
const int pinPot = A0;

//INPUT tmp sensor (TMP36)
const int pinTmp = A5;

//colorpicker
int previousVal = 0;

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(35);
  off();

  pinMode(RASPIN, INPUT);
  pinMode(pinPot, INPUT);
  pinMode(pinTmp, INPUT);
  //analogReference(INTERNAL);

  re = RotaryEncoder(pinS1, pinS2, pinKey);

  lcd.begin(16, 2);
  lcd.clear();
}

void loop() {
  setMode();
  if (active_mode != prev_active_mode) {
    if (active_mode == OFF) {
      off();
      setMode();
      prev_active_mode = active_mode;
    }
    if (active_mode == RGBMODE) {
      prev_active_mode = active_mode; //NON POSSO AVERE QUESTA RIGA DOPO LA SUCCESSIVA
                                      //PERCHE' SIGNIFICHEREBBE USCIRE DA rgbmode(), SETTARE
                                      //prev_active_mode ALLA NUOVA MODALITA' E COSI' NON
                                      //POTREI PIU' ENTRARE NELL'if PRINCIPALE PERCHE'
                                      //prev_active_mode == active_mode     !!!
      rgbmode();
      setMode();
    }
    if (active_mode == WHITE) {
      white();
      setMode();
      prev_active_mode = active_mode;
    }
    if (active_mode == COLD) {
      cold();
      setMode();
      prev_active_mode = active_mode;
    }
    if (active_mode == WARM) {
      warm();
      setMode();
      prev_active_mode = active_mode;
    }
    if (active_mode == PICK) {
      prev_active_mode = active_mode;   //STESSA COSA DI rgbmode()
      pick();
      setMode();
    }
    if (active_mode == TEMP) {
      temp();
      setMode();
      int prev = prev_active_mode;   
      prev_active_mode = active_mode; //QUA DEVE STARE DOPO PERCHE' LA USO IN temp()
      active_mode = prev;             //ritorno alla modalita' di prima
    }
  }

  delay(10);
}

void off() {
  for (int i = 0; i <= NUM_LEDS / 2; i++) {
    leds[i] = CRGB(0, 0, 0);
    leds[NUM_LEDS - i] = CRGB(0, 0, 0);
    FastLED.show();
  }
}

void rgbmode() {
  while (active_mode == RGBMODE) {
    while (red > 0 && active_mode == RGBMODE) {
      setMode(); //controllo sempre se sono in modalità giusta
      red--;
      green++;
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(red, green, blue);
      }
      FastLED.show();
    }

    while (green > 0 && active_mode == RGBMODE) {
      setMode(); //controllo sempre se sono in modalità giusta
      green--;
      blue++;
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(red, green, blue);
      }
      FastLED.show();
    }

    while (blue > 0 && active_mode == RGBMODE) {
      setMode(); //controllo sempre se sono in modalità giusta
      blue--;
      red++;
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(red, green, blue);
      }
      FastLED.show();
    }
  }
}

void white() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(255, 255, 255);
  }
  FastLED.show();
}

void cold() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(80, 150, 255);
  }
  FastLED.show();
}

void warm() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(255, 150, 80);
  }
  FastLED.show();
}

void pick() {
  //analogReference(DEFAULT);
  while (active_mode == PICK) {
    setMode();
    int val = analogRead(pinPot);
    if (abs(val - previousVal) < 12) { //stabilizzazione colore
      val = previousVal;
    }
    float h = map(val, 0, 1023, 370, 0);
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(h, 255, 255);
    }
    FastLED.show();
    previousVal = val;
  }
}


void temp() {
  //temperatura media (aritmetica) su 100 letture
  double mean_temp = 0;

  for (int i = 0; i < 100; i++) {
    int val = analogRead(pinTmp);
    double volts = (val / 1024.0); //shoud be *5 but there is non-constant voltage on Vcc so I have to switch... :'(((
    switch (prev_active_mode) {
      case OFF: volts *= 5; break;
      case RGBMODE: volts *= (4.65 * 5 / 4.87); break;
      case WHITE: volts *= (4.4 * 5 / 4.87); break;
      case COLD: volts *= (4.4 * 5 / 4.87); break;
      case WARM: volts *= (4.4 * 5 / 4.87); break;
      case PICK: volts *= (4.65 * 5 / 4.87); break;
      default: volts *= 5; break;
    }
    double temp = (volts - 0.5) * 100;
    mean_temp += temp;
  }
  mean_temp /= 100;
  
  //Serial.println(mean_temp);
  
  lcd.setCursor(11, 0);
  lcd.print(mean_temp);
}

void setMode() {
  String dir = re.turnDirection();
  if (dir == "dx") {
    mode++;
    lcd.clear();
  }
  else if (dir == "sx") {
    mode--;
    lcd.clear();
  }
  mode = mode % NUM_MODES;
  if (mode < 0)
    mode = NUM_MODES - 1;
  if (digitalRead(RASPIN) == LOW)
    active_mode = OFF;
  if (re.isButtonPressed() && digitalRead(RASPIN) == HIGH)
    active_mode = mode;

  lcd.setCursor(0, 0);
  lcd.print("Mode   ");
  lcd.print(mode);
  lcd.setCursor(7 - (modes[mode].length() / 2), 1);
  lcd.print(modes[mode]);
}
