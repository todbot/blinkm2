

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define led_update_millis 10

const int dbgpin = 5;

#define NUM_LEDS 8

#define DEBUG 1

#if DEBUG 
// can only use the below in main sketch,
// not in C libraries
#if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny84__)  //
#define __BLINKM_BOARD__
#define dbg(x)
#define dbgln(x)
#else
// compile defines ARDUINO_AVR_UNO
#define dbg(x) Serial.print(x)
#define dbgln(x) Serial.println(x)
#endif

#else // no debug
#define dbg(x)
#define dbgln(x)
#endif

#endif
