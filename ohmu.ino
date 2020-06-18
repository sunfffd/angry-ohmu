// HARDWARE:
// WS2813 / WS2812 RGB LED Strip: 14 LEDS used
// KY002 vibration sensor x 1
// Arudino Micro : x 1
// 


#include <FastLED.h>

#define NUM_LEDS 14
#define DATA_PIN 6
#define CLOCK_PIN 13
#define LED_TYPE    WS2813
#define BRIGHTNESS  70 //0..255
#define COLOR_ORDER GRB

// Define the array of leds
CRGB leds[NUM_LEDS];

int shockPin = 7; // Use Pin 10 as our Input
int shockVal = HIGH; // This is where we record our shock measurement
boolean bAlarm = false;

unsigned long lastShockTime; // Record the time that we measured a shock

int delayMS = 3;
int breatheDelay = 10; //Rate of breathing
int shockAlarmTime = delayMS * 255 + 4000; // Number of milli seconds to keep the shock alarm high

int isBreathing = 1; //used for interrption
int lastBrightness = 0; //Remember the brightness before interuption
int val; //loop
int wakeDelay = 200;
void setup ()
{
  Serial.begin(9600);
  pinMode (shockPin, INPUT) ; // input from the KY-002

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
  .setCorrection(TypicalLEDStrip)
  .setDither(BRIGHTNESS < 255);
  FastLED.setBrightness(BRIGHTNESS); //Temporal Dithering

  //Start up sequence
  black();
  blue();
  delay(wakeDelay);
  black();
  delay(wakeDelay);
  red();
  delay(wakeDelay);
  black();
  delay(wakeDelay);

  //First breathe
  for (int brightness = 0; brightness<50; brightness++){
    for ( int i = 0; i < NUM_LEDS; i++ )
      {
        leds[i] = CRGB(0, 0, 255);
        leds[i].maximizeBrightness(brightness); 
      }
    FastLED.show();
    delay(breatheDelay);
  }

  attachInterrupt(digitalPinToInterrupt(shockPin), shockPinChanged, HIGH); //Interuption
}//end setup


void shockPinChanged(){
  isBreathing = 0;
}

void black(){
  for (int i; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
    FastLED.show();
  }
}
void blue(){
  for (int i; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 255);
    FastLED.show();
  }
}

void red(){
  for (int i; i < NUM_LEDS; i++) {
    leds[i] = CRGB(255, 0, 0);
    FastLED.show();
  }
}

void loop ()
{
  shockVal = digitalRead (shockPin) ; // read the value from our sensor
  
  //breathe
  if (!bAlarm ){
    
    for (int brightness = 50; brightness<255; brightness++){
      if(isBreathing){
        for ( int i = 0; i < NUM_LEDS; i++ )
          {
            leds[i] = CRGB(0, 0, 255);
            leds[i].maximizeBrightness(brightness); 
            lastBrightness = brightness;
          }
        FastLED.show();
        delay(breatheDelay);
      }
    }
    
    for (int brightness = 255; brightness >= 50; brightness--){
       if(isBreathing){
          for ( int i = 0; i < NUM_LEDS; i++ )
            {
              leds[i] = CRGB(0, 0, 255);
              leds[i].maximizeBrightness(brightness); 
              lastBrightness = brightness;
            }
          FastLED.show();
          delay(breatheDelay);
       }
    }
    
  }//end breathe cycle

  //ANGRY!!!!!!!!!!
  if (shockVal == 1) // If we're in an alarm state
  {
    lastShockTime = millis(); // record the time of the shock
    if (!bAlarm) {
      Serial.println("Angry");
      //smooth out brightness
      if (lastBrightness != 255){
        for (int brightness = lastBrightness; brightness<255; brightness++){
          for ( int i = 0; i < NUM_LEDS; i++ )
            {
              leds[i].maximizeBrightness(brightness); 
            }
          FastLED.show();
          delay(2);
        }
      }
      for (val = 0; val <= 255; val++)
      {
        for (int i = 0; i < NUM_LEDS; i++) {  
          leds[i] = CRGB(val, 0, 255 - val);
        }
        FastLED.show();
        delay(delayMS);
      }
      bAlarm = true;
    }
  }
  else
  {
    if ( (millis() - lastShockTime) > shockAlarmTime  &&  bAlarm) {
      Serial.println("Not angry");
      for (val = 0; val < 255; val++)
      {

        for (int i = 0; i < NUM_LEDS; i++) {
          leds[i] = CRGB(255 - val, 0, val);
        }
        FastLED.show();
        delay(delayMS);
      }

      //smooth out to low brightness
      for (int brightness = 255; brightness >= 50; brightness--){
        for ( int i = 0; i < NUM_LEDS; i++ )
          {
            leds[i].maximizeBrightness(brightness); 
          }
        FastLED.show();
        delay(2);
      }

      bAlarm = false;
      isBreathing = 1;
    }
  }
}
