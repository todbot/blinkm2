
#ifndef _CONFIG_H_
#define _CONFIG_H_

//#define NUM_LEDS 32
#define NUM_LEDS 8

#define DEBUG 1

#define led_update_millis 10

const int dbgpin     = 5;
const int ledenpin   = 2;
const int leddatapin = 7; // not used

#define ENABLE_BRIGHTNESS 1


// for potential scale8 improvements
#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
#define LIB8_ATTINY 1
#define SCALE8_AVRASM  1
#define SCALE8_C 0
#else
#define SCALE8_C 1
#define SCALE8_AVRASM  1
#endif


#endif
