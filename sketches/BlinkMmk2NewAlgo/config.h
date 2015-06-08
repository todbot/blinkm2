#ifndef _CONFIG_H_
#define _CONFIG_H_

#define NUM_LEDS 7
#define I2C_ADDR 0x09
#define PATT_MAX 16  // FIXME:
// tick msec
//#define foo 10
#define led_update_millis 10
//#define led_update_millis 10

#if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny84__)  //
#define __BLINKM_BOARD__
#define dbg(x)
#define dbgln(x)
#else
#define dbg(x) Serial.print(x)
#define dbgln(x) Serial.println(x)
#endif


#endif
