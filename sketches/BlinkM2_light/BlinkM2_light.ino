/*
 * BlinkM2_light
 *
 * - Uses "light_ws2812" from https://github.com/cpldcpu/light_ws2812
 * - Uses brightness algorithm from https://github.com/adafruit/Adafruit_NeoPixel
 * - Uses HSVtoRGB algo and many other concepts from https://github.com/FastLED/
 *
 *
 * Light Script playback logic
 * - scripts have the properties: id, length, pattern_lines
 * - script playback has the arguments: id, start, len, reps
 * - if play from boot or i2c, clear playstate_last
 * - if play from script, save playstate_last (or offer goto vs gosub)
 */

#include <avr/power.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "led_utils.h"
#include "patterns.h"

//#include "TinyWireS.h"
// be sure to modify TinyWireS/utility/usiTwiSlave.h
// and set TWI_RX_BUFFER_SIZE to 16

// playing values: 0=off, 1=normal, 2=playing from powerup playing=3 direct led addressing FIXME:
enum playmode {
    PLAY_OFF = 0,
    PLAY_ON,
    PLAY_POWERUP,
    PLAY_DIRECT,  // FIXME: what is this?
};

typedef struct {
    uint8_t id;
    uint8_t pos;
    uint8_t start;
    uint8_t end;
    uint8_t count;
} playstate_t;

#define PATT_MAX 48
#define DEFAULT_I2C_ADDR      0x09
#define DEFAULT_BOOT_MODE     PLAY_ON
#define DEFAULT_SCRIPT_ID     0x00
#define DEFAULT_SCRIPT_LEN    0x00
#define DEFAULT_SCRIPT_REPS   0x00
#define DEFAULT_ISPROGRAMMED  0xDC

// eeprom begin: muncha buncha eeprom
uint8_t  ee_i2c_addr         EEMEM = DEFAULT_I2C_ADDR;
uint8_t  ee_boot_mode        EEMEM = DEFAULT_BOOT_MODE; // FIXME: BOOT_PLAY_SCRIPT;
uint8_t  ee_script_id        EEMEM = DEFAULT_SCRIPT_ID;
uint8_t  ee_script_len       EEMEM = DEFAULT_SCRIPT_LEN;
uint8_t  ee_script_reps      EEMEM = DEFAULT_SCRIPT_REPS;
//uint8_t  ee_boot_fadespeed   EEMEM = DEFAUL;
//uint8_t  ee_boot_timeadj     EEMEM = 0x00;
uint8_t  ee_isprogrammed     EEMEM = DEFAULT_ISPROGRAMMED;

// NOTE: can't declare EEPROM statically because Arduino loader doesn't send eeprom
patternline_t patternlines_ee[PATT_MAX] EEMEM;

rgb_t leds[ NUM_LEDS ];
ledvector_t ledvectors[ NUM_LEDS ];
fader_t fader;

uint32_t led_update_next;
//uint32_t play_update_next; // debug toy
uint32_t pattern_update_next;

patternline_t pltmp;  // temp pattern holder

uint16_t fade_millis;
uint16_t ttmp;   // temp time holder
uint8_t ntmp;    // temp ledn holder

rgb_t ctmp;  // temp color holdor

byte cmd;
byte cmdargs[8];  // for i2c transactions


playstate_t playstate;
playstate_t playstate_last;

uint8_t playing;
/*
//uint8_t play_id_last = 255;  // 255 = no last id
uint8_t play_id;
uint8_t playpos;        // current play position
uint8_t playstart;      // start play position
uint8_t playend;        // = PATT_MAX; // end play position
uint8_t playcount;      // number of times to play loop, or 0=infinite
*/

#if DEBUG
//
void dbgblink( uint8_t cnt, uint16_t hi, uint16_t lo )
{
    for( int i=0; i<cnt; i++ ) { 
        digitalWrite( dbgpin, HIGH );
        delay( hi );
        digitalWrite( dbgpin, LOW ) ;
        delay( lo );
    }
}
//
#define dbg_playstate( p ) {  \
    dbg(F(" playstate: id:")); dbg( p.id );  \
    dbg(F(", start:"));  dbg( p.start ); \
    dbg(F(", end:"));    dbg( p.end ); \
    dbg(F(", cnt:"));    dbg( p.count ); \
    dbg(F(", pos:"));    dbg( p.pos ); \
}
//
void dbg_ledstate()
{
  dbgln(F("ledstate:"));
  for( int i=0; i<NUM_LEDS; i++ ) {
    dbg(i); dbg(F(": ")); 
    dbg(leds[i].r); dbg(','); dbg(leds[i].g); dbg(','); dbg(leds[i].b);
    dbg(F("\t:: "));
    dbg(ledvectors[i].dest.r); dbg(',');
    dbg(ledvectors[i].dest.g); dbg(',');
    dbg(ledvectors[i].dest.b); dbg("\t<--");
    dbg(ledvectors[i].last.r); dbg(',');
    dbg(ledvectors[i].last.g); dbg(',');
    dbg(ledvectors[i].last.b); dbgln();
  }
}

#endif

//
void setup() 
{
#if defined(ARDUINO_AVR_UNO)
    Serial.begin(115200);
    dbgln("BlinkM2_light!");
#endif

    //args_t todtmp;
    //todtmp.r = 22;
    //todtmp.args[2] = 33;
    
#if DEBUG
    pinMode(dbgpin, OUTPUT);
    dbgblink( 6, 100, 100 );
              
    dbgln("setting All LEDs");
    led_setAll( 0, 0, 155 );
    led_show();
    delay(300);
    led_setAll( 0, 155, 0 );
    led_show();
    delay(300);

    dbgln("set_brightness All LEDs");
    led_set_brightness(120);
    led_setAll( 0, 0, 155 );
    led_show();
    delay(300);
    led_setAll( 0, 155, 0 );
    led_show();
    delay(300);

    led_setAll( 0,0,0 );
    led_show();
    delay(500);
#endif
    
#if 1
    uint8_t ee_set = eeprom_read_byte( &ee_isprogrammed );
    if( ee_set != DEFAULT_ISPROGRAMMED ) { // EEPROM no programmed
        dbgln(F("no EEPROM values detect, reloading from defaults"));
        // load up EEPROM from flash, 
        // to deal with fact that Arduino uploader doesn't upload EEPROM
        for( uint8_t i=0; i<PATT_MAX; i++ ) {
            memcpy_P( &pltmp, &(patternlines_default[i]), sizeof(patternline_t) ); 
            eeprom_write_block( &pltmp, &(patternlines_ee[i]), sizeof(patternline_t) );
        }
        eeprom_write_byte( &ee_i2c_addr,     DEFAULT_I2C_ADDR );
        eeprom_write_byte( &ee_boot_mode,    DEFAULT_BOOT_MODE );
        eeprom_write_byte( &ee_script_id,    DEFAULT_SCRIPT_ID );
        eeprom_write_byte( &ee_script_len,   DEFAULT_SCRIPT_LEN );
        eeprom_write_byte( &ee_script_reps,  DEFAULT_SCRIPT_REPS );
        eeprom_write_byte( &ee_isprogrammed, DEFAULT_ISPROGRAMMED );
    }
#endif

    // initialize i2c interface
    uint8_t i2c_addr = eeprom_read_byte( &ee_i2c_addr );
    //if( i2c_addr==0 || i2c_addr>0x7f) i2c_addr = I2C_ADDR;  // just in case
    //usiTwiSlaveInit( i2c_addr );

    playstate_last.id = 255;  // indicate no previous script: FIXME:
    
    //  FIXME: check for play on boot
    play_script( 7, 0, 0, 0 ); // id, reps, start, len
    
    pattern_update_next = millis(); // reset pattern clock
}


//
void loop()
{
    update_led_state();
    update_play_state();
}

//
void update_led_state()
{
    // update LEDs every led_update_millis
    if( (long)(millis() - led_update_next) > 0 ) {
        led_update_next += led_update_millis;
        bool done = ledfader_update();
#if 0
        dbg("fader: pos:"); dbg(fader.pos); dbg(", posinc:"); dbg(fader.posinc);
        dbg(", done:"); dbg( done ); dbg(" = ");
        dbg( fader.pos > (FADERPOS_MAX - fader.posinc) );
        dbgln('.');
#endif
        
        led_show();
    } // led_update_next    
}

//
// if playing, fetch a new pattern line
void update_play_state()
{
    // playing light pattern
    if( playing ) {
        if( (long)(millis() - pattern_update_next) > 0  ) { // time to get next line
        
            get_next_patternline();
            
            // prepare to go to next line
            playstate.pos++;
            if( playstate.pos == playstate.end ) {
                playstate.pos = playstate.start; // loop the pattern
                playstate.count--;
                if( playstate.count == 0 ) {
                    if( playstate_last.id == 255 )
                        playing = PLAY_OFF;
                    else {
                        dbg(F("RESTORING LAST PLAYSTATE: ")); dbgln(playstate_last.id);
                        memcpy( &playstate, &playstate_last, sizeof(playstate_t) );
                        playstate_last.id = 255;
                    }
                    //playing = PLAY_OFF; // done!
                } else if(playstate.count==255) {
                    playstate.count = 0; // infinite playing
                }
            }
            pattern_update_next += ttmp;

            dbg(F("update_play_state: "));
            dbg_playstate( playstate );
            dbg(F(", bright:")); dbgln(led_get_brightness());
        }

    } // playing

}
    
//
void get_next_patternline()
{
    if( playstate.id == 0 ) { // eeprom
        eeprom_read_block(&pltmp, &patternlines_ee[playstate.pos],sizeof(patternline_t));
    } else { // flash
        patternline_t* p;
        // read in start addr of patternline set
        memcpy_P( &p, &patterns[playstate.id-1], sizeof(patternline_t*) ); 
        memcpy_P(&pltmp, &p[playstate.pos], sizeof(patternline_t));  // read in patternline
    }
    
    cmd    = pltmp.cmd;
    ctmp.r = pltmp.color.r;  // FIXME: for all of this?
    ctmp.g = pltmp.color.g;
    ctmp.b = pltmp.color.b;
    ttmp   = pltmp.dmillis * 10;
    ntmp   = pltmp.ledn;
#if 1
    dbg(F("get_next_patternline: ")); dbg(playstate.pos); dbg(F(", cmd:"));
    dbg((char)cmd); dbg(','); dbg(ctmp.r);dbg(','); dbg(ctmp.g);dbg(','); dbg(ctmp.b);
    dbg(F(", l:")); dbgln(ntmp);
#endif
    if( ttmp == 0 && ctmp.r==0 && ctmp.g==0 && ctmp.b==0 ) {
        // skip lines set to zero
    }
    else {
        handle_script_cmd();
    }
}

    
//
// start a script playing
//
//
void play_script(uint8_t scrid, uint8_t reps, uint8_t start, uint8_t len )
{
    // save previous playstate
    memcpy( &playstate_last, &playstate, sizeof(playstate_t) );
    
    playstate.id = scrid;
    //if( play_id > MAX_PLAY_ID ) { play_id = 0; } // FIXME: add this
    
    if( scrid == 0 ) {
        if( len==0 ) {
            len = eeprom_read_byte( &ee_script_len );
        }
        //count = eeprom_read_byte( &ee_script_reps );
    }
    else {
        if( len==0 ) {
            len = pgm_read_byte( &(pattern_lens[playstate.id-1]) );
        }
    }

    playstate.start = start;
    playstate.end   = start + len;
    playstate.count = reps;
    playing   = PLAY_ON;  

    dbg_playstate( playstate );  dbgln('.');

    get_next_patternline();
}


//
// uses globals "cmd", "ctmp", "ntmp", "ttmp".
//
void handle_script_cmd()
{
    //dbg_ledstate();
    if( cmd == 'n' ) {                 // set RGB color immediately
        fade_millis = 0;
        ledfader_set_dest( &ctmp, fade_millis, ntmp );
    }
    else if( cmd == 'c' ) {             // fade to RGB color
        // FIXME: mapping of ttmp to fade_millis as a parameter?
        fade_millis = ttmp / 2;    // fading only half the time, at color other half
        ledfader_set_dest( &ctmp, fade_millis, ntmp );
    }
    else if( cmd == 'C' ) {             // fade to random RGB color
        uint8_t dr = ctmp.r;
        uint8_t dg = ctmp.g;
        uint8_t db = ctmp.b;
        ctmp = leds[ ntmp ]; // FIXME: need to range-check ntmp
        ctmp.r = ctmp.r + (random8() % dr ) - (dr/2); // random around prev color
        ctmp.g = ctmp.g + (random8() % dg ) - (dg/2); // random around prev color
        ctmp.b = ctmp.b + (random8() % db ) - (db/2); // random around prev color
        ledfader_set_dest( &ctmp, fade_millis, ntmp );
        //curr.r = curr.r + (random8() % dr ) - (dr/2);  // random around previsous color
        //curr.g = curr.g + (random8() % dg ) - (dg/2);
        //curr.b = curr.b + (random8() % db ) - (db/2);
        //dest = curr;
    }
    else if( cmd == 'h' ) {             // fade to HSV color
        //hsv2rgb_raw_C( &ctmp, &ctmp );
        hsv2rgb_rainbow( &ctmp, &ctmp ); // FIXME: test
        dbg("hsv2rgb: r:"); dbg(ctmp.r); dbg(",g:"); dbg(ctmp.g); dbg(",b:"); dbgln(ctmp.b);
        fade_millis = ttmp / 2;  // FIXME: ttmp to fade_millis mapping
        ledfader_set_dest( &ctmp, fade_millis, ntmp );
    }
    else if( cmd == 'B' ) {            // set brightness. arg0 = set absolute, arg1 = inc/dec
        // FIXME: make arg0 be 'mode', arg1 = val
        // FIXME: support ledn?
        if( ctmp.arg0 ) {
            led_set_brightness( ctmp.arg0 ); // FIXME: arg naming
        }
        else if( ctmp.arg1 ) {
            int16_t b = led_get_brightness();
            int8_t d = ctmp.arg1;  // treat as signed
            b += d;
            b = (b > 255) ? 255 : (b<0) ? 0 : b; // if above 255 turn off brightness, if 
            led_set_brightness( b );
        }
    }
    // potential other generative commands:
    // - move to random
    // - move to specific
    // - swap
    // - "increment parameter" (like ledn, jump, color etc)
    else if( cmd == 'R' ) {             // rotate leds
        // need to:
        // - copy old dest to last foreach ledfader
        // - copy new dest from next ledfader
        // - set ledn
        // - reset faderpos
        // - FIXME: set led appropriately
        int8_t rot = ctmp.arg0;
        // FIXME: handle negative rotation
        for( uint8_t i=0; i<rot; i++ ) {  // foreach rotation
            // rotate by one
            rgb_t olddest = ledvectors[NUM_LEDS-1].dest;
            for( uint8_t j=NUM_LEDS-1; j>0; j--) {
                ledvectors[j].last = leds[j]; // old curr becomes last
                ledvectors[j].dest = ledvectors[j-1].dest; // new dest is next led dest
            }
            ledvectors[0].last = leds[0];
            ledvectors[0].dest = olddest;
        }
        fader.pos = 0;  // reset fader
        fader.ledn = ntmp;
    }

    else if( cmd == 'X'  ) {  // swap two LEDS (direct or fade-swap)  FIXME: not finished
        uint8_t p0 = ctmp.arg0;
        uint8_t p1 = ctmp.arg1;
        // FIXME: range check on p0,p1
        rgb_t p0dest = ledvectors[p0].dest;
        ledvectors[p0].dest = ledvectors[p1].dest;
        ledvectors[p1].dest = p0dest;
        //leds[p1] = ctmp;
    }
    else if( cmd == 'H' ) {           // hue random
    }
    else if( cmd == 'p' ) {           // play a pattern
        uint8_t playid = ctmp.arg0;
        uint8_t reps   = ctmp.arg1;
        uint8_t start  = ctmp.arg2;
        uint8_t len    = ntmp;
        play_script( playid, reps, start, len );
    }
    else if( cmd == 'T' ) {  // random time delay
        
    }
    else if( cmd == 'f' ) {  // set fadespeed
    }
    else if( cmd == 't' ) {  // set time adjustment
    }
    else if( cmd == 'F' ) {
    }
    else if( cmd == 'j' ) {
      int8_t newpos = playstate.pos + ctmp.arg0 - 1;
      if( newpos < playstate.start ) playstate.pos = 0;
      else if( newpos > playstate.end ) playstate.pos = playstate.end;
      else playstate.pos = newpos;
    }
    else if( cmd == 'i' ) {   // input relative
    }
    else if( cmd == 'I' ) {   // input absolute
    }
    else if( cmd == 'k' ) {   // knob rgb
    }
    else if( cmd == 'K' ) {   // knob hsv
    }
    
}




//----------------------

#if 0  
    if( (long)(millis() - play_update_next) > 0 ) {
        play_update_next += 2000; // every 5 secs
        dbgln("new play light");
        rgb_t c = color_list[ cntr ];
        cntr++;
        if( cntr == 5 ) cntr = 0;
        ledfader_set_dest( &c, 1000, ntmp % NUM_LEDS+1 ) ;
        ntmp++ ;
        
        //ctmp = { 0, 0, 0 };
        //ledfader_set_dest( &ctmp, 10, 0 ); // go dark
        /*
        led_setN( 0, 0,0,0 );
        uint8_t r = random() % 128;
        uint8_t g = random() % 128;
        uint8_t b = random() % 128;
        ctmp = { r,g,b };
        //uint8_t n = random() % NUM_LEDS + 1;
        ntmp++ ;
        ledfader_set_dest( &ctmp, 500, ntmp % NUM_LEDS +1 );
        */
    }
#endif
