 /**
  * MaxMStripContorl -- 
  *
  * Some code take from TinyWireM/examples
  *
  * More on TinyWireS usage - see TinyWireS.h
  */

#include "TinyWireS.h"                  // wrapper class for I2C slave routines

#include "light_ws2812.h"


// BlinkM MaxM pins (see below for where this came from)
const int redPin = 8;   // PB2 OC0A
const int grnPin = 7;   // PA7 OC0B
const int bluPin = 5;   // PA5 OC1B
const int sdaPin = 6;   // PA6 
const int sclPin = 9;   // PA4
const int in0Pin = A0;  // PA0
const int in1Pin = A1;  // PA1
const int in2Pin = A2;  // PA2
const int in3Pin = A3;  // PA3

// From: blinkm/variants/tiny14/pins_arduino.h  
// (originally from: http://hlt.media.mit.edu/?p=1695 )
// ATMEL ATTINY84 / ARDUINO
//
//                           +-\/-+
//                     VCC  1|    |14  GND
//             (D 10)  PB0  2|    |13  AREF (D  0)
//             (D  9)  PB1  3|    |12  PA1  (D  1) 
//                     PB3  4|    |11  PA2  (D  2) 
//  PWM  INT0  (D  8)  PB2  5|    |10  PA3  (D  3) 
//  PWM        (D  7)  PA7  6|    |9   PA4  (D  4) 
//  PWM        (D  6)  PA6  7|    |8   PA5  (D  5)        PWM
//                           +----+

const int I2C_SLAVE_ADDR  = 0x09;     // default BlinkM addr

const int NUM_LEDS = 8;

typedef struct CRGB_ { uint8_t g; uint8_t r; uint8_t b; } CRGB;
CRGB leds[1]; //NUM_LEDS];


void setup()
{
  TinyWireS.begin(I2C_SLAVE_ADDR); 
  
  leds[0].r=32; leds[0].g=32; leds[0].b=32;

  ws2812_sendarray( (uint8_t*)leds, 3 );
  delay(500);
  
  pinMode( redPin, OUTPUT);   
  pinMode( grnPin, OUTPUT);  
  pinMode( bluPin, OUTPUT);  
  blink( redPin,1);
  blink( grnPin,1);
  blink( bluPin,1);

}

void loop()
{
  byte cmd;
  byte r,g,b;

  if( TinyWireS.available() ) { 

    cmd = TinyWireS.receive();    // first byte is command
    if( cmd == 'c' ) { 
      r = TinyWireS.receive();
      g = TinyWireS.receive();
      b = TinyWireS.receive();
      analogWrite( redPin, r );
      analogWrite( grnPin, g );
      analogWrite( bluPin, b );

    }
    // else if( cmd == 'h' ) { 
    //}
    else {
    }
  }
}


//
void blink(byte pin, byte times)
{ 
  for (byte i=0; i< times; i++){
    digitalWrite(pin,HIGH);
    delay (200);
    digitalWrite(pin,LOW);
    delay (100);
  }
}





