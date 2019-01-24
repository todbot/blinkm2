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

#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/sleep.h>

#include "config.h"
#include "led_utils.h"
#include "patterns.h"

//#define boo

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

struct playstate_t {
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

// eeprom begin: 
uint8_t  ee_i2c_addr         EEMEM = DEFAULT_I2C_ADDR;
uint8_t  ee_boot_mode        EEMEM = DEFAULT_BOOT_MODE; // FIXME: BOOT_PLAY_SCRIPT;
uint8_t  ee_script_id        EEMEM = DEFAULT_SCRIPT_ID;
uint8_t  ee_script_len       EEMEM = DEFAULT_SCRIPT_LEN;
uint8_t  ee_script_reps      EEMEM = DEFAULT_SCRIPT_REPS;
uint8_t  ee_isprogrammed     EEMEM = DEFAULT_ISPROGRAMMED;
// eeprom end:

// NOTE: can't declare EEPROM statically because Arduino loader doesn't send eeprom
patternline_t patternlines_ee[PATT_MAX] EEMEM;

rgb_t leds[ NUM_LEDS ];
ledvector_t ledvectors[ NUM_LEDS ];
fader_t fader;

uint32_t led_update_next;
uint32_t pattern_update_next;

patternline_t pltmp;  // temp pattern holder

fract8 fade_scale = 128;
uint16_t fade_millis;
uint16_t ttmp;   // temp time holder
uint8_t ntmp;    // temp ledn holder

rgb_t ctmp;  // temp color holdor

byte cmd;
byte cmdargs[8];  // for i2c transactions

struct playstate_t playstate; // current player state
struct playstate_t playstate_last;
#define PLAYSTATE_NONE 255

uint8_t playing;

#define MAX_SLEEP_ITERATIONS 5
int sleepIterations = 0;
volatile bool watchdogActivated = false;

#include "powermgmt.h" // FIXME: .h includes functions

// can't include this because we use some globals 
#include "debug.h"

//
void setup() 
{
#if defined(ARDUINO_AVR_UNO)
    Serial.begin(115200);
    dbgln("BlinkM2_light!");
#endif
    dbg_setup();
    
    ledpower_setup();
    ledpower_enable();

    //dbg_pwrtst();
    
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

    playstate_last.id = PLAYSTATE_NONE;  // indicate no previous script
    
    //  FIXME: check for play on boot
    play_script( 10, 0, 0, 0 ); // id, reps, start, len
    
    //pattern_update_next = millis(); // reset pattern clock
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
    if( playing ) {
        if( (long)(millis() - pattern_update_next) > 0  ) { // time to get next line
        
            get_next_patternline(); // load pattern line
            
            dbg_playstate( F("update_play_state: "), playstate );
            dbg(F(", bright:")); dbgln(led_get_brightness());
            
            // prepare to go to next line
            playstate.pos++;
            if( playstate.pos == playstate.end ) {
                playstate.pos = playstate.start; // loop the pattern
                playstate.count--;
                if( playstate.count == 0 ) { // done playing?
                    if( playstate_last.id == PLAYSTATE_NONE )
                        playing = PLAY_OFF;
                    else { // we were gosub'd
                        dbg(F("update_play_state: RESTORING: ")); dbgln(playstate_last.id);
                        memcpy( &playstate, &playstate_last, sizeof(playstate_t) );
                        playstate_last.id = PLAYSTATE_NONE;
                        playstate.pos++;  
                        if( playstate.pos == playstate.end )
                            playstate.pos = playstate.start;
                        // FIXME: the above is a hack
                    }
                } else if(playstate.count==255) { // decremented past zero, so was zero before
                    playstate.count = 0; // infinite playing
                }
            }
            //dbg(F("\tttmp:")); dbgln(ttmp);
            pattern_update_next += ttmp;  // set next pattern time

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
    dbg(F("    get_next_patternline: ")); dbg(playstate.pos); dbg(F(", cmd:"));
    dbg((char)cmd); dbg(','); dbg(ctmp.r);dbg(','); dbg(ctmp.g);dbg(','); dbg(ctmp.b);
    dbg(F(",t:")); dbg(ttmp); dbg(F(", l:")); dbgln(ntmp);
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
void play_script(uint8_t scrid, uint8_t reps, uint8_t start, uint8_t len )
{
    // save previous playstate
    memcpy( &playstate_last, &playstate, sizeof(playstate_t) );
    
    playstate.id = scrid;
    //if( play_id > MAX_PLAY_ID ) { play_id = 0; } // FIXME: add this
    
    if( scrid == 0 ) { // id 0 is eeprom
        if( len==0 ) {
            len = eeprom_read_byte( &ee_script_len );
        }
        //count = eeprom_read_byte( &ee_script_reps );
    }
    else {            // id 1-N is flash
        if( len==0 ) {
            len = pgm_read_byte( &(pattern_lens[playstate.id-1]) );
        }
    }

    playstate.start = start;
    playstate.end   = start + len;
    playstate.pos   = playstate.start;
    playstate.count = reps;
    playing  = PLAY_ON;  

    dbg_playstate( F("play_script:"), playstate );  dbgln('.');

    pattern_update_next = millis();
    get_next_patternline();
}

//
// uses globals "cmd", "ctmp", "ntmp", "ttmp".
//
void handle_script_cmd()
{
    // cmd: set RGB color immediately
    // args: r,g,b, 0, ledn
    if( cmd == 'n' ) {
        fade_millis = 0;
        ledfader_set_dest( &ctmp, fade_millis, ntmp );
    }
    // cmd: fade to RGB color
    // args: r,g,b, fade_millis, ledn
    else if( cmd == 'c' ) {
        // FIXME: mapping of ttmp to fade_millis as a parameter?
        fade_millis = ttmp/2; // scale8( ttmp, fade_scale);  // fading only half the time, at color other half
        ledfader_set_dest( &ctmp, fade_millis, ntmp );
    }
    // cmd: fade to random RGB color
    // args: r,g,b random amount, fade_millis, ledn
    else if( cmd == 'C' ) {
        // not this one, can result in no red, all green
        //ctmp.r = random8() % ctmp.r; 
        //ctmp.g = random8() % ctmp.g;
        //ctmp.b = random8() % ctmp.b;
        /*        
        uint16_t dr = ctmp.r;
        uint16_t dg = ctmp.g;
        uint16_t db = ctmp.b;

        // FIXME: using ttmp for "amount" of random, which is also used for pattern_next_millis

        ctmp.r = (dr!=0) ? dr + (random8() % ttmp ) - (ttmp/2) : 0; // random around prev color
        ctmp.g = (dg!=0) ? dg + (random8() % ttmp ) - (ttmp/2) : 0; // random around prev color
        ctmp.b = (db!=0) ? db + (random8() % ttmp ) - (ttmp/2) : 0; // random around prev color
        ledfader_set_dest( &ctmp, fade_millis, ntmp );
        */

        uint8_t dr = ctmp.r;
        uint8_t dg = ctmp.g;
        uint8_t db = ctmp.b;
        ctmp = leds[ ntmp ]; // FIXME: need to range-check ntmp 
        ctmp.r = ctmp.r + (random8() % dr ) - (dr/2); // random around prev color
        ctmp.g = ctmp.g + (random8() % dg ) - (dg/2); // random around prev color
        ctmp.b = ctmp.b + (random8() % db ) - (db/2); // random around prev color
        fade_millis = ttmp/2;
        ledfader_set_dest( &ctmp, fade_millis, ntmp );

    }
    // cmd: fade to HSV color
    // args: h,s,v, fade_millis, ledn
    else if( cmd == 'h' ) {
        //hsv2rgb_raw_C( &ctmp, &ctmp );
        hsv2rgb_rainbow( &ctmp, &ctmp ); // FIXME: test
        dbg("hsv2rgb: r:"); dbg(ctmp.r); dbg(",g:"); dbg(ctmp.g); dbg(",b:"); dbgln(ctmp.b);
        fade_millis = ttmp / 2;  // FIXME: ttmp to fade_millis mapping
        ledfader_set_dest( &ctmp, fade_millis, ntmp );
    }
    // cmd: hue random
    else if( cmd == 'H' ) {

    }
    // cmd: set brightness.
    // args: arg0 = set absolute, arg1 = inc/dec
    else if( cmd == 'B' ) { 
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
    // - "time" based functions?j  ("jump if time > x"?)
    
    // cmd: rotate leds
    // args: arg0 = rotation amount (+/-)
    else if( cmd == 'R' ) {
        int8_t rot = ctmp.arg0;
        //ledfader_rotate( ctmp.arg0, ctmp.arg1, ctmp.arg2, ledn );
            
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

    // cmd: swap two LEDS (direct or fade-swap)  FIXME: not finished
    // args: arg0 = pos1, arg1 = pos2
    else if( cmd == 'X'  ) { 
        uint8_t p0 = ctmp.arg0;
        uint8_t p1 = ctmp.arg1;
        // FIXME: range check on p0,p1
        rgb_t p0dest = ledvectors[p0].dest;
        ledvectors[p0].dest = ledvectors[p1].dest;
        ledvectors[p1].dest = p0dest;
        //leds[p1] = ctmp;
    }
    // cmd: play a pattern
    // args: playid, reps, start, len
    else if( cmd == 'p' ) {
        uint8_t playid = ctmp.arg0;
        uint8_t reps   = ctmp.arg1;
        uint8_t start  = ctmp.arg2;
        uint8_t len    = ntmp;
        play_script( playid, reps, start, len );
    }
    // cmd: stop playing script
    // args: arg0
    // FIXME: add sleep, off, etc options
    else if( cmd == 's' ) {
        playing = 0;
    }
    // cmd: random time delay around midpoint
    // args: ttmp is min time, arg0 is additive max (maybe arg1 is low byte?)
    else if( cmd == 'T' ) {
        //if (ctmp.arg0 > pltmp.dmillis) ctmp.arg0 = pltmp.dmillis/2; // FIXME: verify this works
        //ttmp = ttmp - ((10*ctmp.arg0)/2) + (10 * (random8(ctmp.arg0)); // FIXME: gah
        //ttmp = ttmp + 10 * (random8() % ctmp.arg0);
        ttmp = ttmp + (10 * (random8() % (ctmp.arg0+1))); 
        dbg(F("\thandle_script: T ttmp:")); dbgln(ttmp);
    }
    // cmd: set fadespeed       // map to fade_millis slope adjust?
    // arg0: fade_scale, where 127 = std slope
    else if( cmd == 'f' ) {
        fade_scale = ctmp.arg0;
    }
    // cmd: set time adjustment  // prob remove
    else if( cmd == 't' ) {
        /// NOTE: time as low-byte (0-127) for short duration, high-bit as long duration flag?
    }
    // cmd: jump
    // args: arg0 = jump amount (+/-)
    else if( cmd == 'j' ) {
      int8_t newpos = playstate.pos + ctmp.arg0 - 1;
      if( newpos < playstate.start ) playstate.pos = 0;
      else if( newpos > playstate.end ) playstate.pos = playstate.end;
      else playstate.pos = newpos;
    }
    // cmd: input relative
    else if( cmd == 'i' ) {

    }
    // cmd: input absolute
    else if( cmd == 'I' ) {

    }
    // cmd: knob rgb
    else if( cmd == 'k' ) {

    }
    // cmd: knob hsv
    else if( cmd == 'K' ) { 

    }
    
}

