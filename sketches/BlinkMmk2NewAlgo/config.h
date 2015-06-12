#ifndef _CONFIG_H_
#define _CONFIG_H_

#define NUM_LEDS 8
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



#if defined(__AVR_ATtiny85__)
// BlinkM pins
const int redPin = 3;  // 
const int grnPin = 4;  //
const int bluPin = 1;  // PWM, will blink when programming
const int sdaPin = 0;  // PWM, 'd' pin, can be digital I/O
const int sclPin = 2;  // A/D, 'c' pin, can be digital I/O, or analog input
const int LED_PIN = bluPin;
#elif defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny44__)
// BlinkM MaxM pins 
const int redPin = 8;   // PB2 OC0A
const int grnPin = 7;   // PA7 OC0B
const int bluPin = 5;   // PA5 OC1B 'i' pin
const int sdaPin = 6;   // PA6 
const int sclPin = 9;   // PA4
const int in0Pin = A0;  // PA0
const int in1Pin = A1;  // PA1
const int in2Pin = A2;  // PA2
const int in3Pin = A3;  // PA3
const int LED_PIN = redPin;
#else
#warning "unknown board type, assuming UNO"
const int redPin = 9;
const int grnPin = 10;
const int bluPin = 11;
//const int LED_PIN = 6;
const int LED_PIN = 7;
#endif


#endif
