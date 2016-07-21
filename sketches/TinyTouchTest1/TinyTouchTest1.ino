
#define ledPin 0
//#define touchPin 5
#define touchPin 0

extern "C" {
#include "TinyTouchLib.h"
}

void blink(uint8_t cnt, uint16_t msec) {
    while( cnt-- ) {
        digitalWrite(ledPin, HIGH);
        delay(msec);
        digitalWrite(ledPin, LOW);
        delay(msec);
    }
}

void setup() {

  tinytouch_init();

  pinMode(ledPin, OUTPUT);

  blink(10, 50);
}

void loop() {
//  if (tinytouch_sense() == tt_push) {
  if (tinytouch_sense() == tt_on) {
    digitalWrite( ledPin, HIGH );
    //PORTB ^= _BV(PB4); // Toggle LED on touch event
  }
  else {
    digitalWrite( ledPin, LOW );
  }

}
