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
const int NUM_LEDS = 16;  // at most 16
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
  handleCmds();
}

void handleCmds() 
{
  

}

//
void displayLEDs()
{
  FastLED.show();
}

void handlePattLine(void)
{
    rgb_t    ctmp = pltmp.color;
    int      ttmp = pltmp.dmillis;
    uint16_t ntmp = pltmp.ledn;
    uint8_t  cmd  = pltmp.cmd;

    if( cmd==0 && ttmp == 0 ) {
        return;
    }

    switch(cmd) {
    case('n'): // go to rgb immediately
        ttmp = 1;  // and fall thru to 'c' case
    case('c'): // fade to rgb
        ledfader.setDest( ctmp, ttmp, ntmp );
        break;
    case('C'):  // random rgb
        ctmp.r = gamma(random(255));
        ctmp.g = gamma(random(255));
        ctmp.b = gamma(random(255));
        ledfader.setDest( ctmp, ttmp, ntmp );
        break;
    case('o'):
        playing = 0;
        break;
    case('p'):
        patt_info.id    = ntmp;
        patt_info.count = pltmp.args[1];
        patt_info.start = pltmp.args[2];
        patt_info.end   = pltmp.args[3];
        playing = 1;
        break;
    default:
        break;
    }
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
        } 
        else {                  // flash
          patt_line_t* pp;
          memcpy_P(&pp, &patterns[ patt_info.id ], sizeof(patt_line_t*));
          memcpy_P(&pltmp, &pp[playpos],sizeof(patt_line_t));
        }

        pattern_update_next += pltmp.dmillis*10;

        handlePattLine();
    
        playpos++;
        if( playpos > patt_info.end ) {
          playpos = patt_info.start;
          patt_info.count--;
          if( patt_info.count == 0 ) {
            playing = 0; // done!
          } else if( patt_info.count==255 ) {
            patt_info.count = 0; // infinite playing
          }
        }
        
        p("patt:%d cnt:%d start:%d stop:%d pos:%d ttmp:%d\n", 
          patt_info.id, patt_info.count, patt_info.start, patt_info.end, 
          playpos, pltmp.dmillis*10);
        
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
  patt_info.end   = 4;
  //patt_info.start = 0;
  //patt_info.end = patt_max;
  patt_info.count = 0;
  // if( patt_info.id == 0 ) { 
  //  playpos = eeprom_read_byte( 

  playpos = patt_info.start;

  playing = 1;
}

uint8_t GammaE[] = { 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11,
11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
19, 19, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 27, 28,
29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40,
40, 41, 42, 43, 44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54,
55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 88, 89,
90, 91, 93, 94, 95, 96, 98, 99,100,102,103,104,106,107,109,110,
111,113,114,116,117,119,120,121,123,124,126,128,129,131,132,134,
135,137,138,140,142,143,145,146,148,150,151,153,155,157,158,160,
162,163,165,167,169,170,172,174,176,178,179,181,183,185,187,189,
191,193,194,196,198,200,202,204,206,208,210,212,214,216,218,220,
222,224,227,229,231,233,235,237,239,241,244,246,248,250,252,255};
//
static uint8_t gamma( uint8_t n ) 
{
    return GammaE[n];
}






