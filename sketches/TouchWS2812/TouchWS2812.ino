
extern "C" {
#include "captouch.h"
#include "light_ws2812.h"
}

#define ledPin 0
//#define touchPin 5
#define touchPin 0 // fixme: adc chan not input, unused
#define neoPin 1

#define NUM_LEDS 64

struct cRGB led[8];


void setup() {

  tinytouch_init();

  pinMode(ledPin, OUTPUT);


  debugblink(10, 50);
}

void loop() {
  //  if (tinytouch_sense() == tt_push) {
  if (tinytouch_sense() == tt_on) {
    digitalWrite( ledPin, HIGH );
    led[0].r=255;led[0].g=00;led[0].b=255;    // Write red to array
    // strip.setPixelColor(1, 255,0,255);
  }
  else {
    digitalWrite( ledPin, LOW );
    led[0].r=0;led[0].g=00;led[0].b=0;    // Write red to array
    // strip.setPixelColor(1, 0,0,0);
  }
  // send data to LED strip
  ws2812_setleds(led,8);
  // strip.show();
}

void debugblink(uint8_t cnt, uint16_t msec) {
    while( cnt-- ) {
        digitalWrite(ledPin, HIGH);
        delay(msec);
        digitalWrite(ledPin, LOW);
        delay(msec);
    }
}
