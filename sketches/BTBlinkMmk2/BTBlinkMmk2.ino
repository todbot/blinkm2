 /**
  * BTBlinkMmk2 --- 
  *
  * BlinkM algo tests on BlinkyTape
  *
  */


#include "FastLED.h"
#include "LEDFader.h"

#include <avr/pgmspace.h>    // for memcpy_P()
#include <avr/eeprom.h>  // FOR EEMEM
#include <avr/delay.h>
#include <stdio.h>
#include <stdarg.h>

// How many leds in your strip?
const int NUM_LEDS = 30;
//const int NUM_LEDS = 1;

//const int DATA_PIN = 0;   // maxm A0
//const int DATA_PIN = 7; // blinkmmk2
const int DATA_PIN = 13; // blinkytape

// Define the array of leds
CRGB leds[NUM_LEDS];
//rgb_t ctmp;
//uint16_t ttmp;   // temp time holder
//uint8_t ntmp;    // temp ledn holder

const  uint32_t led_update_millis = 10;  // tick msec
static uint32_t led_update_next = 0;
static uint32_t pattern_update_next = 0;

const uint8_t patt_max=16;

// the light sequence patterns
#include "patterns.h"

patt_info_t patt_info;

/*
uint8_t patt_id   = 0;
uint8_t playstart = 0; // start play position
uint8_t playend   = patt_max; // end play position
*/

uint8_t playcount = 0; // number of times to play loop, or 0=infinite
uint8_t playpos   = 0; // current play position
uint8_t playlen   = patt_max; // gets decremented

uint8_t playing = 0; // playing values: 0=off, 1=normal, 2==playing from powerup

patt_line_t pltmp;  // temp pattern holder

rgbfader_t faders[NUM_LEDS];
LEDFader ledfader(leds, faders, NUM_LEDS ); 

// printf_wrapper
void p(char *fmt, ... ) 
{
  char tmp[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(tmp, 128, fmt, args);
  va_end (args);
  Serial.print(tmp);
}

//
void setup() 
{ 
  Serial.begin(57600);
  delay(2000);
  Serial.println("BTBlinkMmk2!");
  delay(1500);
  Serial.println("here we go...");

  // load up EEPROM from flash, 
  // to deal with fact that Arduino uploader doesn't upload EEPROM
  // FIXME: need to check if we need to do this
  //ptmp.len = patt_max;
  //ptmp.lines = ee_patt_lines;

  for( uint8_t i=0; i<patt_max; i++ ) {
    memcpy_P( &pltmp, &(patt_lines_default[i]), sizeof(patt_line_t) ); 
    eeprom_write_block( &pltmp, &(ee_patt_lines[i]), sizeof(patt_line_t) );
  }

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

#if 1
  for( int i=0; i< NUM_LEDS; i++ )  leds[i] = 0x000020;
  FastLED.show();
  delay(500);
  for( int i=0; i< NUM_LEDS; i++ )  leds[i] = 0x002000;
  FastLED.show();
  delay(500);
  for( int i=0; i< NUM_LEDS; i++ )  leds[i] = 0x000000;
  FastLED.show();
  delay(500);
#endif

  led_update_next = millis();
  pattern_update_next = millis();

  start_playing();
}

//
void loop() 
{ 
  updateLEDs();
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
        
        if( patt_info.id == 0 ) { // eeprom
          eeprom_read_block(&pltmp,&ee_patt_lines[playpos],sizeof(patt_line_t));
        } else {                  // flash
          patt_line_t* pp;
          memcpy_P(&pp, &patterns[ patt_info.id ], sizeof(patt_line_t*));
          memcpy_P(&pltmp, &pp[playpos],sizeof(patt_line_t));
        }
        
        rgb_t   ctmp = pltmp.color;
        int     ttmp = pltmp.dmillis;
        uint8_t ntmp = pltmp.ledn;

        pattern_update_next += ttmp*10;

        if( ttmp == 0 && ctmp.r==0 && ctmp.g==0 && ctmp.b==0 ) {
          // skip lines set to zero
        } else {
          ledfader.setDest( ctmp, ttmp, ntmp );
        }

        playpos++;
        if( playpos == patt_info.end ) {
          playpos = patt_info.start; //pgm_read_byte( &patt_lens[patt_info.id]);
          patt_info.count--;
          if( patt_info.count == 0 ) {
            playing = 0; // done!
          } else if( patt_info.count==255 ) {
            patt_info.count = 0; // infinite playing
          }
        }

        p("patt:%d cnt:%d start:%d stop:%d pos:%d ttmp:%d\n", 
          patt_info.id, patt_info.count, patt_info.start, patt_info.end, 
          playpos, ttmp);
        
      }

    } // playing
    else {
      p("stopped");
    }

}

void start_playing(void)
{

  if( patt_info.id == 0 ) {   // eeprom
  } 
  else { 
  }

  patt_info.id    = 1;

  patt_info.start = 0;
  patt_info.end   = 2;
  //patt_info.start = 0;
  //patt_info.end = patt_max;
  patt_info.count = 0;
  // if( patt_info.id == 0 ) { 
  //  playpos = eeprom_read_byte( 

  playpos = patt_info.start;

  playing = 1;
}






