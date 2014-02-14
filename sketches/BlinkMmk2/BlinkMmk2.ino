 /**
  * BlinkMmk2 --- 
  *
  * More on TinyWireS usage - see TinyWireS.h
  *
  * From: blinkm/variants/tiny14/pins_arduino.h  
  * (originally from: http://hlt.media.mit.edu/?p=1695 )
  * ATMEL ATTINY84 / ARDUINO
  *
  *                           +-\/-+
  *                     VCC  1|    |14  GND
  *             (D 10)  PB0  2|    |13  AREF (D  0)
  *             (D  9)  PB1  3|    |12  PA1  (D  1) 
  *                     PB3  4|    |11  PA2  (D  2) 
  *  PWM  INT0  (D  8)  PB2  5|    |10  PA3  (D  3) 
  *  PWM        (D  7)  PA7  6|    |9   PA4  (D  4) 
  *  PWM        (D  6)  PA6  7|    |8   PA5  (D  5)        PWM
  *                           +----+
  *
  */


//#include "TinyWireS.h"                // wrapper class for I2C slave routines
#include "FastLED.h"
#include "LEDFader.h"

#include <avr/pgmspace.h>    // for memcpy_P()
#include <avr/eeprom.h>  // FOR EEMEM

// How many leds in your strip?
const int NUM_LEDS = 4;
//const int NUM_LEDS = 1;

//const int DATA_PIN = 0;   // maxm A0
const int DATA_PIN = 7; // blinkmmk2

const int I2C_SLAVE_ADDR  = 0x09;     // default BlinkM addr

// BlinkM MaxM pins 
const int redPin = 8;   // PB2 OC0A
const int grnPin = 7;   // PA7 OC0B
const int bluPin = 5;   // PA5 OC1B
const int sdaPin = 6;   // PA6 
const int sclPin = 9;   // PA4
const int in0Pin = A0;  // PA0
const int in1Pin = A1;  // PA1
const int in2Pin = A2;  // PA2
const int in3Pin = A3;  // PA3


// Define the array of leds
CRGB leds[NUM_LEDS];
rgb_t ctmp;
uint16_t ttmp;   // temp time holder
uint8_t ntmp;    // temp ledn holder

const  uint32_t led_update_millis = 10;  // tick msec
static uint32_t led_update_next = 0;
static uint32_t pattern_update_next = 0;

const uint8_t patt_max=16;

uint8_t playpos   = 0; // current play position
uint8_t playstart = 0; // start play position
uint8_t playend   = patt_max; // end play position
uint8_t playlen   = patt_max;
uint8_t playcount = 0; // number of times to play loop, or 0=infinite
uint8_t playing = 1; // playing values: 0=off, 1=normal, 2==playing from powerup

// the light sequence patterns
#include "patterns.h"

patternline_t pltmp;  // temp pattern holder

rgbfader_t faders[NUM_LEDS];
LEDFader ledfader(leds, faders, NUM_LEDS ); 

//
void setup() 
{ 
  // load up EEPROM from flash, 
  // to deal with fact that Arduino uploader doesn't upload EEPROM
  // FIXME: need to check if we need to do this
  //ptmp.len = patt_max;
  //ptmp.lines = patternlines_ee;
  //eeprom_write_block( &pltmp, &pattern_ee, sizeof(pattern_t) );
  for( uint8_t i=0; i<patt_max; i++ ) {
    memcpy_P( &pltmp, &(patternlines_default[i]), sizeof(patternline_t) ); 
    eeprom_write_block( &pltmp, &(patternlines_ee[i]), sizeof(patternline_t) );
  }

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

#if 1
  for( int i=0; i< NUM_LEDS; i++ )
    leds[i] = 0x000080;
  FastLED.show();
  delay(1000);
  for( int i=0; i< NUM_LEDS; i++ )
    leds[i] = 0x008000;
  FastLED.show();
  delay(2000);
  for( int i=0; i< NUM_LEDS; i++ ) 
    leds[i] = 0x000000;
  FastLED.show();
  delay(3000);
#endif

  //TinyWireS.begin(I2C_SLAVE_ADDR);
  
}

//
void loop() 
{ 
  updateLEDs();
  delay(1);
}

//
void displayLEDs()
{
  FastLED.show();
}

//
void updateLEDs(void)
{
    uint32_t now = millis();

    // update LED for fading every led_update_millis
    if( (long)(now - led_update_next) > 0 ) { 
      led_update_next += led_update_millis;
      ledfader.update();
      displayLEDs();
    }
    
    // playing light pattern
    if( playing ) {
      if( (long)(now - pattern_update_next) > 0  ) { // time to get next line

        memcpy( &pltmp, &patternlines_mem[playpos], sizeof(patternline_t) );
      //eeprom_read_block(&pltmp,&patternlines_ee[playpos],sizeof(patternline_t));
      //memcpy_P(&pltmp,&(patternlines_default[playpos]),sizeof(patternline_t)); 
        ctmp = pltmp.color;
        ttmp = pltmp.dmillis;
        ntmp = pltmp.ledn;
        if( ttmp == 0 && ctmp.r==0 && ctmp.g==0 && ctmp.b==0 ) {
          // skip lines set to zero
        } else {
          ledfader.setDest( &ctmp, ttmp, ntmp );
        }
        playpos++;
        if( playpos == playend ) {
          playpos = playstart; // loop the pattern
          //playcount--;
          //if( playcount == 0 ) {
          //  playing=0; // done!
          //} else if(playcount==255) {
          //  playcount = 0; // infinite playing
          //}
        }
        pattern_update_next += ttmp*10;
      }
    } // playing

}


void play_pattern(uint8_t id, uint8_t reps, uint8_t startpos,uint8_t endpos)
{
  if( id==0 ) {
    //play_pattern_ee(reps, startpos, endpos);
    //playlen = ptmp.len;
       
  } else {
    //play_pattern_fl(id,reps,startpos,endpos);
    //memcpy_P( &ptmp, &patterns[id-1], sizeof(pattern_t) );
    //playlen = ptmp.len;
  }
    
}


/*inline void setLED( uint8_t i, uint8_t r, uint8_t g, uint8_t b )
{
  leds[i].setRGB( r,g,b );
  }*/



/*
#if 0
      if( i == 100 ) {            // 10*100 = 1000 msec
        ctmp = CRGB::Red;
        ledfader.setDest( &ctmp, 100  );
      }
      else if( i == 200 ) { 
        ctmp = CRGB::Green;
        ledfader.setDest( &ctmp, 100  );
      } 
      else if( i == 300 ) {
        ctmp = CRGB::Blue;
        ledfader.setDest( &ctmp, 100  );
      }
      else if( i == 400 ) {
        ctmp = CRGB::Black;
        ledfader.setDest( &ctmp, 500  ); // 5000 msec = 5sec
      }
      else if( i == 1000 ) { 
        i = 0;
      }
      i++;
#endif    


#if 0
    // Turn the LED on, then pause
    leds[i].r = random(200);
    leds[i].g = random(200);
    leds[i].b = random(200);
    FastLED.show();
    delay(100);

    //FastLED.setBrightness( random(100) ) ;
    FastLED.setBrightness( 64 ) ;

    // Now turn the LED off, then pause
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(100);

    i++;
    if( i == NUM_LEDS ) i = 0;
#endif
*/
