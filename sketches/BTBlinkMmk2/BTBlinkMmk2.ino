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

// How many leds in your strip?
const int NUM_LEDS = 8;
//const int NUM_LEDS = 1;

//const int DATA_PIN = 0;   // maxm A0
//const int DATA_PIN = 7; // blinkmmk2
const int DATA_PIN = 13; // blinkytape



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
  Serial.begin(57600);
  while(!Serial);
  Serial.println("BTBlinkMmk2!");
  delay(1000);
  Serial.println("here we go...");

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
  delay(1000);
  for( int i=0; i< NUM_LEDS; i++ ) 
    leds[i] = 0x000000;
  FastLED.show();
  delay(1000);
#endif

  led_update_next = millis();
  pattern_update_next = millis();
}

//
void loop() 
{ 
  updateLEDs();
}

//
inline void displayLEDs()
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
        Serial.print("patt:"); Serial.print(playpos); Serial.print(" ctmp:");
        Serial.print(ctmp.r);Serial.print(",");
        Serial.print(ctmp.g);Serial.print(",");
        Serial.print(ctmp.b);Serial.print(": ttmp:"); Serial.println(ttmp);

        /*
        CRGB led0 = leds[0];
        Serial.print("led0:");
        Serial.print(led0.r);  Serial.print(",");
        Serial.print(led0.g);  Serial.print(",");
        Serial.print(led0.b);  Serial.print(", ");
        Serial.print(led0.raw[0]);  Serial.print(",");
        Serial.print(led0.raw[1]);  Serial.print(",");
        Serial.print(led0.raw[2]);  Serial.print(", ");
        */

        if( ttmp == 0 && ctmp.r==0 && ctmp.g==0 && ctmp.b==0 ) {
          // skip lines set to zero
        } else {
          ledfader.setDest( ctmp, ttmp, ntmp );
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


/*
// fakey deleta-sigma attempt, does not work, just flickers
void updateLEDs(void)
{
  CRGB colr1 = 0xff00ff;
  CRGB colr2 = 0x000000;
  // rgb_t c2 = 0xff00ff; does not yet work

  while( 1 ) { 
    for( int i=0; i< NUM_LEDS; i++) { 
      leds[i] = 0xff00ff;
    }
    displayLEDs();
    _delay_ms(10);

    for( int i=3; i< NUM_LEDS; i++) { 
      leds[i] = 0x000000;
    }
    displayLEDs();
    _delay_ms(10);
  }

}
*/


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
