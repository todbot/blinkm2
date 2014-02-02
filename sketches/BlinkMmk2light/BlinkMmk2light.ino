 /**
  * BlinkMmk2light --- a potential "lightweight" version of BlinkMmk2 sketch
  *
  * Note: this sketch compiles to >4kB, which because of a GCC bug results in a
  * "relocation truncated to fit: R_AVR_13_PCREL" error.  To fix this bug,
  * replace the "ld" program using this:
  *  https://github.com/TCWORLD/ATTinyCore/tree/master/PCREL%20Patch%20for%20GCC
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


//#include "TinyWireS.h"               // wrapper class for I2C slave routines

#include <avr/pgmspace.h>    // for memcpy_P()
#include <avr/eeprom.h>  // FOR EEMEM

#include "FastLED.h"
//#include "light_ws2812_tod.h"

#include "blinkm_types.h"


// How many leds in your strip?
//const int NUM_LEDS = 4;
const int nLEDs = 15;

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
CRGB leds[nLEDs];
rgb_t ctmp;
uint16_t ttmp;   // temp time holder
uint8_t ntmp;    // temp ledn holder

const  uint32_t led_update_millis = 10;  // tick msec
static uint32_t led_update_next;
static uint32_t pattern_update_next;

const uint8_t patt_max=16;

uint8_t playpos   = 0; // current play position
//uint8_t playstart = 0; // start play position
//uint8_t playend   = patt_max; // end play position
//uint8_t playlen   = patt_max;
//uint8_t playcount = 0; // number of times to play loop, or 0=infinite
uint8_t playing = 1; // playing values: 0=off, 1=normal, 2==playing from powerup


rgbfader_t faders[nLEDs];

patt_info_t patt_info;

patt_line_t pltmp;  // temp pattern holder


//-----------------------------
#include "ledfader_funcs.h"

// the light sequence patterns
#include "patterns.h"

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
      memcpy_P( &pltmp, &(patt_lines_default[i]), sizeof(patt_line_t) ); 
      eeprom_write_block( &pltmp, &(ee_patt_lines[i]), sizeof(patt_line_t) );
  }
  
  ledfader_setCurr(0, 0);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, nLEDs);

  /*
  DDRA |= _BV(PA7);
  //DDRB | _BV(PORTB7);

  leds[0].r=32; leds[0].g=32; leds[0].b=32;
  leds[3].r=64; leds[3].g=0; leds[3].b=64;
  ws2812_sendarray((uint8_t*)leds, nLEDs*3);
  delay(1500);
  leds[0].r=32; leds[0].g=0; leds[0].b=0;
  ws2812_sendarray((uint8_t*)leds, nLEDs*3);
  delay(3000);
  */

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
static void displayLEDs()
{
    FastLED.show();
    //ws2812_sendarray((uint8_t*)leds, nLEDs*3);
}


void start_playing(void)
{

  if( patt_info.id == 0 ) {   // eeprom
  } 
  else { 
  }

  patt_info.id    = 1;

  patt_info.start = 0;
  patt_info.end   = 3;
  //patt_info.start = 0;
  //patt_info.end = patt_max;
  patt_info.count = 0;
  // if( patt_info.id == 0 ) { 
  //  playpos = eeprom_read_byte( 

  playpos = patt_info.start;

  playing = 1;
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
        ledfader_setDest( ctmp, ttmp, ntmp );
        break;
    case('C'):  // random rgb
        ctmp.r = gamma(random(255));
        ctmp.g = gamma(random(255));
        ctmp.b = gamma(random(255));
        ledfader_setDest( ctmp, ttmp, ntmp );
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
      ledfader_update();
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
        
        //p("patt:%d cnt:%d start:%d stop:%d pos:%d ttmp:%d\n", 
        //  patt_info.id, patt_info.count, patt_info.start, patt_info.end, 
        //  playpos, pltmp.dmillis*10);
        
      }

    } // playing
    else {
        //p("stopped");
    }

}

uint8_t GammaE[] PROGMEM = { 
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
    return n; //pgm_read_byte( &GammaE[n] );
}
