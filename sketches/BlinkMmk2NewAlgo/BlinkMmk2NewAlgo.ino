 /**
  *
  * BlinkM mk2 testing
  *
  * Thanks to David Mellis and High/Low Tech Group for ATtiny84/85 support
  * http://highlowtech.org/?p=1695
  *
  * Thanks to BroHogan for TinyWireS: http://playground.arduino.cc/Code/USIi2c
  * Credit and thanks to Don Blake for his usiTwiSlave code. 
  * More on TinyWireS usage - see TinyWireS.h
  *
  * NOTE: Pullup resistors should be used on SDA & SCL lines
  * 
  *
  * 2015, Tod E. Kurt, http://todbot.com/blog/ http://thingm.com/
  *
  */
/*
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
*/


// configuration for multple boards and debug helpers
#include "config.h"

// note: Arduino IDE doesn't parse correctly inside ifdefs
#if defined(__BLINKM_BOARD__)
//#include "TinyWireS.h" // must comment out when in UNO debug
#else
#include "Wire.h"        // must comment out when in production
#endif

#include <Adafruit_NeoPixel.h>

#include "led_fader_types.h"

// light sequence patterns, needs struct from led_fader_funcs.h
#include "patterns.h"


// eeprom begin: muncha buncha eeprom
uint8_t  ee_i2c_addr         EEMEM = I2C_ADDR;
uint8_t  ee_boot_mode        EEMEM = 0x00; // FIXME: BOOT_PLAY_SCRIPT;
uint8_t  ee_boot_script_id   EEMEM = 0x00;
uint8_t  ee_boot_reps        EEMEM = 0x00;
uint8_t  ee_boot_fadespeed   EEMEM = 0x08;
uint8_t  ee_boot_timeadj     EEMEM = 0x00;
uint8_t  ee_unused2          EEMEM = 0xDA;

// NOTE: can't declare EEPROM statically because Arduino loader doesn't send eeprom
patternline_t patternlines_ee[PATT_MAX] EEMEM;

// NOTE: we will make as many variables as possible global
//       so we can track RAM usage at compile time

byte cmd;
byte cmdargs[8];

uint8_t script_id;
uint8_t playpos   = 0;        // current play position
uint8_t playstart = 0;        // start play position
uint8_t playend   = PATT_MAX; // end play position
uint8_t playcount = 0;        // number of times to play loop, or 0=infinite
uint8_t playing; // playing values: 0=off, 1=normal, 2=playing from powerup playing=3 direct led addressing FIXME: 

uint16_t fade_millis;
uint32_t now;

patternline_t pltmp;  // temp pattern holder
uint16_t ttmp;   // temp time holder
uint8_t ntmp;    // temp ledn holder

rgb_t ctmp;  // temp color holdor

uint32_t led_update_next;
uint32_t pattern_update_next;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// we need an leds array, so pull it from neopixel library
rgb_t* leds = (rgb_t*)strip.getPixels(); 
fader_t faders[NUM_LEDS];


// note: we have funcs in header file because of Arduino compile limitations
#include "led_fader_funcs.h"


uint8_t ledtoggle;  // temp debug

void play_script();
void get_next_patternline();
void update_led_state();
void check_i2c();
void handle_script_cmd();

//
void setup()
{
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    strip.setBrightness(128);
    
#if defined(__BLINKM_BOARD__)
    TinyWireS.begin(I2C_ADDR);
#else
    Wire.begin(I2C_ADDR);
    Serial.begin(115200);
    dbgln("BlinkMmk2!");
#endif

#if 0
    // load up EEPROM from flash, 
    // to deal with fact that Arduino uploader doesn't upload EEPROM
    for( uint8_t i=0; i<PATT_MAX; i++ ) {
    memcpy_P( &pltmp, &(patternlines_default[i]), sizeof(patternline_t) ); 
    eeprom_write_block( &pltmp, &(patternlines_ee[i]), sizeof(patternline_t) );
    }
#endif
    
    //  FIXME: check for play on boot
    script_id = 5;
    play_script();
    
}

//
void loop()
{
    update_led_state();
    check_i2c();
}


//
// updateLEDState() is the main user-land function that:
// - periodically calls the rgb fader code to fade any actively moving colors
// - controls sequencing of a light pattern, if playing
// - ///triggers pattern playing on USB disconnect
//
void update_led_state()
{
    now = millis();
    
    // update LEDs every led_update_millis
    if( (long)(now - led_update_next) > 0 ) {
      led_update_next += led_update_millis;
      led_update_faders();
      strip.show();
      
    } // led_update_next

    // playing light pattern
    if( playing ) {
      if( (long)(now - pattern_update_next) > 0  ) { // time to get next line
        digitalWrite(grnPin, (ledtoggle++)%2 );
        
        get_next_patternline();
        
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
void play_script()
{
    if( script_id == 0 ) {
        playend = 5;  // FIXME:
        playcount = 0;  // FIXME: infinite
    }
    else { 
        playend = pgm_read_byte( &pattern_lens[script_id] ); 
        playcount = 0;
    }
    
    playpos = 0;
    playing = 1;
    
    get_next_patternline();
}

//
void get_next_patternline()
{
    if( script_id == 0 ) { // eeprom
        eeprom_read_block(&pltmp, &patternlines_ee[playpos],sizeof(patternline_t));
    } else {
        patternline_t* p;
        memcpy_P( &p, &patterns[script_id], sizeof(patternline_t*) ); // read in start addr of patternline set
        memcpy_P(&pltmp, &p[playpos], sizeof(patternline_t));  // read in patternline
    }
    cmd  = pltmp.cmd;
    //ctmp = pltmp.color;
    ctmp.r = pltmp.color.r; ctmp.g = pltmp.color.g; ctmp.b = pltmp.color.b;
    ttmp = pltmp.dmillis * 10;
    ntmp = pltmp.ledn;
    dbg(F("\n---line: ")); dbg(playpos); dbg(':'); dbg((char)cmd); dbg(" - "); dbgln(ntmp);
    
    if( ttmp == 0 && ctmp.r==0 && ctmp.g==0 && ctmp.b==0 ) {
        // skip lines set to zero
    }
    else {
        handle_script_cmd();
    }
}


//
void handle_script_cmd()
{
    //dbg("handle_script_cmd: "); dbgln((char)cmd);
    if( cmd == 'n' ) {
        //dest = ctmp;
        //curr = ctmp;
    }
    else if( cmd == 'c' ) { 
        //curr = dest;
        //dest = ctmp;
        fade_millis = ttmp / 2;    // fading only half the time, at color other half
        led_set_dest( &ctmp, fade_millis, ntmp );
        //calculate_overlay_inc();
    }
    else if( cmd == 'C' ) {
        byte dr = cmdargs[0];
        byte dg = cmdargs[1];
        byte db = cmdargs[2];
        //curr.r = curr.r + (random8() % dr ) - (dr/2);  // random around previsous color
        //curr.g = curr.g + (random8() % dg ) - (dg/2);
        //curr.b = curr.b + (random8() % db ) - (db/2);
        //dest = curr;
    }
    else if( cmd == 'h' ) {
        //curr = dest;
        hsv2rgb_rainbow( &ctmp, &ctmp ); // FIXME: test
        //dbg("h:"); dbg(cmdargs[0]); dbg("r:"); dbg(dest.r);
        fade_millis = ttmp / 2;
        led_set_dest( &ctmp, fade_millis, ntmp );
        //calculate_overlay_inc();
    }
    else if( cmd == 'H' ) {
    }
    else if( cmd == 'p' ) {
    }
    else if( cmd == 'f' ) {
    }
    else if( cmd == 't' ) {
    }
    else if( cmd == 'F' ) {
    }
    else if( cmd == 'j' ) {
    }
    else if( cmd == 'i' ) {
    }
    else if( cmd == 'I' ) {
    }
    else if( cmd == 'k' ) {
    }
    else if( cmd == 'K' ) {
    }
    
}

// --------------------------------------------------------

//
void read_i2c_vals(uint8_t num, uint8_t*buf)
{
#if defined(__BLINKM_BOARD__)
    for( uint8_t i=0; i<num; i++)
        buf[i] = TinyWireS.receive();
#else
    for( uint8_t i=0; i<num; i++)
        buf[i] = Wire.read();
#endif
}

//
void check_i2c()
{
#if defined(__BLINKM_BOARD__)
    if( TinyWireS.available() ) { 
        cmd = TinyWireS.receive();    // first byte is command
#else
    if( Wire.available() ) { 
        cmd = Wire.read();    // first byte is command
#endif
            
        if( cmd == 'n' ||
            cmd == 'c' ||
            cmd == 'C' ||
            cmd == 'h' ||
            cmd == 'H' ||
            cmd == 'p' ) {
            read_i2c_vals( 3, cmdargs );
            handle_script_cmd();
        }
        else if( cmd == 'f' ) {
            
        }
        // else if( cmd == 'h' ) { // "fade to HSB color" cmd
        //}
        else {
            // unknown cmd
        }
        
    } // if available
}
    
