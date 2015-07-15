/*
 * BlinkM2_light
 *
 *
 *
 * Light Script playback logic
 * - scripts have the properties: id, length, pattern_lines
 * - script playback has the arguments: id, start, len, reps
 * 
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
byte cmdargs[8];

uint8_t playing;
uint8_t play_id;
uint8_t playpos;        // current play position
uint8_t playstart;      // start play position
uint8_t playend;        // = PATT_MAX; // end play position
uint8_t playcount;      // number of times to play loop, or 0=infinite

//uint8_t script_id;

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
void setup() 
{
#if defined(ARDUINO_AVR_UNO)
    Serial.begin(115200);
    dbgln("BlinkM2_light!");
#endif

#if 1
    pinMode(dbgpin, OUTPUT);
    dbgblink( 6, 100, 100 );
              
    dbgln("setting All LEDs");
    //led_setBrightness(0);
    led_setAll( 0, 0, 155 );
    led_show();
    delay(1000);
    led_setAll( 0, 155, 0 );
    led_show();
    delay(1000);
    
    led_setAll( 0,0,0 );
    led_show();
    delay(1000);
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

    //  FIXME: check for play on boot
    play_script( 5, 0, 0, 0 ); // id, reps, start, len

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
        int done = ledfader_update();
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
            playpos++;
            if( playpos == playend ) {
                playpos = playstart; // loop the pattern
                playcount--;
                if( playcount == 0 ) {
                    playing = PLAY_OFF; // done!
                } else if(playcount==255) {
                    playcount = 0; // infinite playing
                }
            }
            pattern_update_next += ttmp;

            dbg(F("update_play_state: ")); dbg(playpos);
            dbg(F(", start:")); dbg(playstart);
            dbg(F(", end:")); dbg(playend);
            dbg(F(", cnt:")); dbg(playcount);
            dbgln('.');

        }

    } // playing

}
    
//
void get_next_patternline()
{
    if( play_id == 0 ) { // eeprom
        eeprom_read_block(&pltmp, &patternlines_ee[playpos],sizeof(patternline_t));
    } else { // flash
        patternline_t* p;
        // read in start addr of patternline set
        memcpy_P( &p, &patterns[play_id-1], sizeof(patternline_t*) ); 
        memcpy_P(&pltmp, &p[playpos], sizeof(patternline_t));  // read in patternline
    }
    cmd  = pltmp.cmd;
    ctmp.r = pltmp.color.r; ctmp.g = pltmp.color.g; ctmp.b = pltmp.color.b;
    ttmp = pltmp.dmillis * 10;
    ntmp = pltmp.ledn;
#if 1
    dbg(F("get_next_patternline: ")); dbg(playpos); dbg(F(", cmd:"));
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
    play_id   = scrid;
    //if( play_id > MAX_PLAY_ID ) { play_id = 0; } // FIXME: add this
    
    if( scrid == 0 ) {
        if( len==0 ) {
            len = eeprom_read_byte( &ee_script_len );
        }
        //count = eeprom_read_byte( &ee_script_reps );
    }
    else {
        if( len==0 ) {
            
            len = pgm_read_byte( &(pattern_lens[play_id-1]) );
        }
    }

    playstart = start;
    playend   = start + len;
    playcount = reps;
    playing   = PLAY_ON;  
    
    dbg(F("play_script: id:")); dbg( play_id );
    dbg(F(", start:"));  dbg( playstart );
    dbg(F(", end:"));    dbg( playend );
    dbg(F(", cnt:"));    dbg( playcount );
    dbgln('.');

    get_next_patternline();
}


//
// uses globals "cmd", "ctmp", "ntmp", "ttmp".
//
void handle_script_cmd()
{
    uint8_t ctmp2;
    //dbg("handle_script_cmd: "); dbgln((char)cmd);
    if( cmd == 'n' ) {
        //dest = ctmp;
        //curr = ctmp;
    }
    else if( cmd == 'c' ) { 
        fade_millis = ttmp / 2;    // fading only half the time, at color other half
        ledfader_set_dest( &ctmp, fade_millis, ntmp );
        //calculate_overlay_inc();
    }
    else if( cmd == 'C' ) {  // FIXME: this uses cmdargs should use ctmp
        uint8_t dr = cmdargs[0];
        uint8_t dg = cmdargs[1];
        uint8_t db = cmdargs[2];
        ctmp = leds[ ntmp ]; // FIXME: need to parse ntmp
        ctmp.r = ctmp.r + (random8() % dr ) - (dr/2); // random around prev color
        ctmp.g = ctmp.g + (random8() % dg ) - (dg/2); // random around prev color
        ctmp.b = ctmp.b + (random8() % db ) - (db/2); // random around prev color
        ledfader_set_dest( &ctmp, fade_millis, ntmp );
        //curr.r = curr.r + (random8() % dr ) - (dr/2);  // random around previsous color
        //curr.g = curr.g + (random8() % dg ) - (dg/2);
        //curr.b = curr.b + (random8() % db ) - (db/2);
        //dest = curr;
    }
    else if( cmd == 'h' ) { 
        //hsv2rgb_raw_C( &ctmp, &ctmp );
        hsv2rgb_rainbow( &ctmp, &ctmp ); // FIXME: test
        dbg("hsv2rgb: r:"); dbg(ctmp.r); dbg(",g:"); dbg(ctmp.g); dbg(",b:"); dbgln(ctmp.b);
        fade_millis = ttmp / 2;
        ledfader_set_dest( &ctmp, fade_millis, ntmp );
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
