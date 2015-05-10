 /**
  *
  * NOTE: Pullup resistors should be used on SDA & SCL lines
  * 
  * Thanks to BroHogan for TinyWireS: http://playground.arduino.cc/Code/USIi2c
  * Credit and thanks to Don Blake for his usiTwiSlave code. 
  * More on TinyWireS usage - see TinyWireS.h
  *
  * 2015, Tod E. Kurt, http://todbot.com/blog/ http://thingm.com/
  *
  */

#include "TinyWireS.h"                  // wrapper class for I2C slave routines
#include <Adafruit_NeoPixel.h>

#include "led_utils.h"

#define patt_max 32  // FIXME:

#include "patterns.h"  // the light sequence patterns


// BlinkM / BlinkM MinM pins
const int redPin = 3;  // 
const int grnPin = 4;  //
const int bluPin = 1;  // PWM, will blink when programming
const int sdaPin = 0;  // PWM, 'd' pin, can be digital I/O
const int sclPin = 2;  // A/D, 'c' pin, can be digital I/O, or analog input
/*
//
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
//
*/


#define I2C_SLAVE_ADDR  0x09     // default BlinkM addr
#define NUM_LEDS 8
#define LED_PIN 1  // blue pin

#define patt_max 32  // FIXME:


patternline_t pltmp;  // temp pattern holder

byte cmd;
//byte r,g,b;

uint8_t script_id;
uint8_t playpos   = 0; // current play position
uint8_t playstart = 0; // start play position
uint8_t playend   = patt_max; // end play position
uint8_t playcount = 0; // number of times to play loop, or 0=infinite
uint8_t playing; // playing values: 0 = off, 1 = normal, 2 == playing from powerup playing=3 direct led addressing FIXME: 

uint16_t fade_millis;

uint16_t ttmp;   // temp time holder
uint8_t ntmp;    // temp ledn holder

CRGB curr;
CRGB dest;
CRGB ctmp;    // temporary color
fract16 overlay_amount;
fract16 overlay_inc;
fract16 overlay_amounts[NUM_LEDS];
fract16 overlay_incs[NUM_LEDS];

const uint32_t led_update_millis = 10;  // tick msec
uint32_t led_update_next;
uint32_t pattern_update_next;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t redtoggle;

void setup()
{
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(128);

  TinyWireS.begin(I2C_SLAVE_ADDR);

  //playing = 1;
  //playend = 3;
  //overlay_amount = 0;
  
  // load up EEPROM from flash, 
  // to deal with fact that Arduino uploader doesn't upload EEPROM
  // FIXME: need to check if we need to do this
  //pltmp.len = 5; //patt_max;
  //pltmp.lines = patternlines_ee;
  //eeprom_write_block( &pltmp, &pattern_ee, sizeof(pattern_t) );

  for( uint8_t i=0; i<patt_max; i++ ) {
    memcpy_P( &pltmp, &(patternlines_default[i]), sizeof(patternline_t) ); 
    eeprom_write_block( &pltmp, &(patternlines_ee[i]), sizeof(patternline_t) );
  }

  //  FIXME: if play on boot
  script_id = 2;
  play_script();
  
  //digitalWrite(grnPin,HIGH);
}

//
void loop()
{
  update_led_state();
  //check_i2c();
}


// compute the overlay_inc
// this is the amount overlay_amount is incremented every led_update_millis
//
// (led_update_millis / fade_millis) = fractional amount to do every tick
// e.g. (10 / 500) = 0.02; (10 / 1000) = 0.01
//
// (led_update_millis / fade_millis) * 256  = fract8 amount to add to overlay
// (led_update_millis / fade_millis) * 65536  = fract16 amount to add to overlay
// 
// => overlay_inc = 65536 * led_update_millis / fade_millis 
//
void calculate_overlay_inc()
{
  //overlay_inc = ((uint32_t)65536 * led_update_millis) / fade_millis;
  overlay_inc = ((uint16_t)256 * led_update_millis) / fade_millis;
}

//
void get_next_patternline()
{
  if( script_id == 0 ) { // eeprom
    eeprom_read_block(&pltmp, &patternlines_ee[playpos],sizeof(patternline_t));
  } else {
    memcpy_P(&pltmp, &patterns[ script_id ][playpos], sizeof(patternline_t));
    //const patternline_t* patt = patterns[ 1 ];
    //memcpy_P(&pltmp, &patt[playpos], sizeof(patternline_t));
    //memcpy_P(&pltmp,&(patternlines_default[playpos]),sizeof(patternline_t));
    //memcpy_P(&pltmp,  &(patternlines_rgb[playpos]),sizeof(patternline_t));
  }
  
  ctmp = pltmp.color;
  ttmp = pltmp.dmillis * 10;
  ntmp = pltmp.ledn;
  
}

//
void play_script()
{
  if( script_id == 0 ) {
    playend = 5;  // FIXME:
  }
  else { 
    playend = pgm_read_byte( script_id );
  }
  playpos = 0;
  playcount = 0;
  playing = 1;
  overlay_amount = 0;

  get_next_patternline();
}

//
void update_leds()
{
  if( overlay_amount < 250 ) { // 97.6% arbitrary
    ctmp = blend( curr, dest, (overlay_amount) );
    //ctmp = blend( curr, dest, (overlay_amount>>8) );
    //ctmp = dest;
    overlay_amount += overlay_inc;
  }
  else {
    ctmp = dest;
  }

  // for now set all pixels to same color
  for( uint8_t i=0; i< NUM_LEDS; i++) {
    strip.setPixelColor( i, ctmp.r, ctmp.g, ctmp.b );
  }
  strip.show();

}


//
// updateLEDState() is the main user-land function that:
// - periodically calls the rgb fader code to fade any actively moving colors
// - controls sequencing of a light pattern, if playing
// - ///triggers pattern playing on USB disconnect
//
void update_led_state()
{
    uint32_t now = millis();
    
    // update LEDs every led_update_millis
    if( (long)(now - led_update_next) > 0 ) {
      led_update_next += led_update_millis;
      
      update_leds();
      
      /*      
      // check for non-computer power up
      if( !usbHasBeenSetup ) {
        if( !playing && now > 500 ) {  // 500 msec wait
          playing = 2;
          startPlaying();
        }
      }
      else {  // else usb is setup...
        if( playing == 2 ) { // ...but we started a powerup play, so reset
          off();
        }
      }
      */
      
    } // led_update_next

    // playing light pattern
    if( playing ) {
      if( (long)(now - pattern_update_next) > 0  ) { // time to get next line
        digitalWrite(grnPin, (redtoggle++)%2 );
        
        get_next_patternline();
        
        if( ttmp == 0 && ctmp.r==0 && ctmp.g==0 && ctmp.b==0 ) {
          // skip lines set to zero
        } else {
          curr = dest;
          dest = ctmp;
          //fade_millis = ttmp;
          fade_millis = ttmp / 2;
          overlay_amount = 0;
          calculate_overlay_inc();
        }
        // prepare to go to next line
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
        pattern_update_next += ttmp;
      }
    } // playing
    
}


//
void check_i2c()
{

  if( TinyWireS.available() ) { 
    cmd = TinyWireS.receive();    // first byte is command
    if( cmd == 'c' ) {
      dest.r = TinyWireS.receive();
      dest.g = TinyWireS.receive();
      dest.b = TinyWireS.receive();
      overlay_amount = 0;
      calculate_overlay_inc();
      
    }
    else if( cmd == 'n' ) {  // "fade to color" & "set color now" cmds
      curr.r = TinyWireS.receive();
      curr.g = TinyWireS.receive();
      curr.b = TinyWireS.receive();
      dest = curr;

      // update leds
      
    }
    else if( cmd=='f' ) {
    }
    else if( cmd == 'C' ) { // "random rgb color" cmd
      byte dr = TinyWireS.receive();
      byte dg = TinyWireS.receive();
      byte db = TinyWireS.receive();
      curr.r = curr.r + (rand() % dr ) - (dr/2);  // random around previsous color
      curr.g = curr.g + (rand() % dg ) - (dg/2);
      curr.b = curr.b + (rand() % db ) - (db/2);
      dest = curr;
      
      // update leds
      
    }
    // else if( cmd == 'h' ) { // "fade to HSB color" cmd
    //}
    else {
      // unknown cmd
    }
  }
}

/*
//
void updateLEDsNew()
{
  for( int i = 0; i<NUM_LEDS; i++ ) {
    overlay_amount = overlay_amounts[i];
    overlay_inc = overlay_incs[i];
    if( overlay_amount < 250 ) { // 97.6% arbitrary
      ctmp = blend( curr, dest, (overlay_amount) );
      //ctmp = blend( curr, dest, (overlay_amount>>8) );
      //ctmp = dest;
      overlay_amount += overlay_inc;
    }
    else {
      ctmp = dest;
    }

    strip.setPixelColor( i, ctmp.r, ctmp.g, ctmp.b );
  }
  strip.show();

}
*/




