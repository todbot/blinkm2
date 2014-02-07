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
  *  PWM        (D  7)  PA7  6|    |9   PA4  (D  4)  SCL 
  *  PWM  SDA   (D  6)  PA6  7|    |8   PA5  (D  5)        PWM
  *                           +----+
  *
  */


#include <avr/pgmspace.h>    // for memcpy_P()
#include <avr/eeprom.h>  // FOR EEMEM
#include <avr/delay.h>

#include <stdlib.h>

extern "C" {
  #include "usiTwiSlave.h"
  #include "light_ws2812.h"

//  #include "debugtools.h"
}

#include "blinkm_types.h"
//#include "FastLED.h"


// How many leds in your strip?
///const int nLEDs = 4;
const int nLEDs = 16;
const int wsnLEDs = 3*nLEDs;

const uint8_t patt_fudge_percent = 50;

#define BLINKM_PROTOCOL_VERSION_MAJOR 'c'
#define BLINKM_PROTOCOL_VERSION_MINOR 'f'
#define I2C_SLAVE_ADDR 0x09
//const int I2C_SLAVE_ADDR  = 0x09;     // default BlinkM addr

const int DATA_PIN = 0;   // maxm A0
//const int DATA_PIN = 7; // blinkmmk2

const uint8_t patt_max=16;

const  uint8_t  led_update_millis = 10;  // tick msec
static uint32_t led_update_next;
static uint32_t pattern_update_next;

// Define the array of leds
CRGB leds[nLEDs];
rgb_t ctmp;
uint16_t ttmp;   // temp time holder
uint8_t ntmp;    // temp ledn holder
int16_t tmpc;

uint8_t playpos = 0; // current play position
uint8_t playing = 1; // playing values: 0=off, 1=normal, 2==playing from powerup

rgbfader_t faders[nLEDs];

patt_info_t patt_info;
patt_line_t pattline;  // temp pattern holder

uint8_t inputs[4];


#include "debugtools.h"


//-----------------------------

// functions for doing smooth led fading
#include "ledfader_funcs.h"

// the light sequence patterns
#include "patterns.h"

//-----------------------------


//
void setup() 
{ 
    dbg_tx_init();
    dbg_tx_strP(PSTR("hello! this is 63 in hex: "));
    dbg_tx_hex( 63 );
    dbg_tx_str("\n");

    //if( i2c_addr==0 || i2c_addr>0x7f) i2c_addr = I2C_ADDR;  // just in case
    usiTwiSlaveInit( I2C_SLAVE_ADDR );

    // load up EEPROM from flash, 
    // to deal with fact that Arduino uploader doesn't upload EEPROM
    // FIXME: need to check if we need to do this
    //ptmp.len = patt_max;
    //ptmp.lines = patternlines_ee;
    //eeprom_write_block( &pattline, &pattern_ee, sizeof(pattern_t) );
    for( uint8_t i=0; i<patt_max; i++ ) {
        memcpy_P( &pattline, &(patt_lines_default[i]), sizeof(patt_line_t) ); 
        eeprom_write_block(&pattline, &(ee_patt_lines[i]), sizeof(patt_line_t));
    }
    
    ctmp.r = ctmp.g = ctmp.b = 0;
    ledfader_setCurr(ctmp, 0);

    //FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, nLEDs);
    //FastLED.show();

    pinMode( ws2812_pin, OUTPUT );
    ws2812_sendarray( (uint8_t*)leds, wsnLEDs );
    //ws2812_setleds( (uint8_t*)leds, nLEDs );

    debug_flash();

    led_update_next = millis();

    patt_info.id    = 4;
    patt_info.start = 0;
    patt_info.count = 0;
    patt_info.end   = 0;

    startPlaying();
}

//
void loop()  
{ 
    updateLEDs();
    //handleInputs();
    handleI2C();
}

// must fill out patt_info struct before calling this func
void startPlaying(void)
{
    if( patt_info.end == 0 ) {  // fetch default len
        if( patt_info.id == 0 ) { 
            patt_info.end = patt_max;
        } else { 
            patt_info.end = pgm_read_byte( &(patt_lens[ patt_info.id-1 ]) );
        }
    }
        
    playpos = patt_info.start;
    pattern_update_next = millis(); 
    playing = 1;
}


//
inline
void handleInputs(void)
{
    inputs[0] = analogRead( 0 );
    inputs[1] = analogRead( 1 );
    inputs[2] = analogRead( 5 );
    inputs[3] = analogRead( 6 );
}

#if 1

//
inline
void handlePattLine(void)
{
    rgb_t    ctmp = pattline.color;
    int      ttmp = pattline.dmillis;
    uint16_t ntmp = pattline.ledn;

    ttmp -= (ttmp * patt_fudge_percent / 100); 

    if( ttmp < 1 ) ttmp = 1;

    if( pattline.cmd==0 && ttmp == 0 ) {
        return;
    }

    switch( pattline.cmd ) {
    case('n'): // go to rgb immediately
        ttmp = 1;  // and fall thru to 'c' case
    case('c'): // fade to rgb
        ledfader_setDest( ctmp, ttmp, ntmp );
        break;
    case('C'):  // random rgb    // FIXME
        ctmp.r = randRange(ctmp.r, leds[0].r );
        ctmp.g = randRange(ctmp.g, leds[0].g ); ///gamma(random(255));
        ctmp.b = randRange(ctmp.b, leds[0].b );
        ledfader_setDest( ctmp, ttmp, ntmp );
        break;
    case('h'):  // fade to hsv color
        hsvToRgb(&ctmp);
        ledfader_setDest( ctmp, ttmp, ntmp ); 
        break;
    case('H'):
        ctmp.h = randRange(ctmp.h, leds[0].h );
        ctmp.s = randRange(ctmp.s, leds[0].s );
        ctmp.v = randRange(ctmp.v, leds[0].v );
        hsvToRgb(&ctmp);
        ledfader_setDest( ctmp, ttmp, ntmp );
        break;
    case('i'):
        ttmp = analogRead( pattline.args[1] );
        break;
    case('o'):
        playing = 0;
        break;
    case('p'):
        patt_info.id    = pattline.args[1];
        patt_info.count = 0; //pattline.args[2];
        patt_info.start = 0; //pattline.args[3];
        patt_info.end   = 4; //pattline.args[4];
        startPlaying();
        break;
    case('T'):     // random time delay   // FIXME: test this
        pattline.dmillis = randRange( pattline.dmillis, pattline.args[0] );
        break;
    default:
        break;
    }

    dbg_tx_strP(PSTR("freeRam:"));
    dbg_tx_hex( freeRam() );
    dbg_tx_str("\n");
}

//__attribute__((always_inline)) inline void readI2Cvals(uint8_t cnt)
void readI2Cvals(uint8_t cnt)
{
    for(uint8_t i=0;i<cnt;i++) 
        pattline.args[1+i] = usiTwiReceiveByte();
}
void readI2Ccolor(void)
{
    pattline.color.r = usiTwiReceiveByte();
    pattline.color.g = usiTwiReceiveByte();
    pattline.color.b = usiTwiReceiveByte();
}

//
void handleI2C(void)
{
    if( !usiTwiDataInReceiveBuffer() ) {
        return;
    }
    
    pattline.cmd  = usiTwiReceiveByte();
    
    switch(pattline.cmd) { 
        
    case('o'):         // stop playback
        //stopPlaying();
        playing=0;
        break;
    case('T'):         // script cmd: set time adjustment
        readI2Cvals(1);
        handlePattLine();
        break;
    case('f'):         // script cmd: set fade speed
    case('t'):         // script cmd: set time adjustment
        readI2Cvals(2);
        handlePattLine();
        break;
    case('n'):         // script cmd: set rgb color now
    case('c'):         // script cmd: fade to rgb color
    case('C'):         // script cmd: fade to random rgb color
    case('h'):         // script cmd: fade to hsv color
    case('H'):         // script cmd: fade to random hsv color
        //readI2Cvals(3);  // read r,g,b (or h,s,v)
        readI2Ccolor();   // read r,g,b (or h,s,v)
        
        pattline.dmillis = 30;  // FIXME:
        pattline.ledn = 0;      // FIXME:
        
        handlePattLine();
        break;
    case('p'):         // script cmd: play script
        readI2Cvals(3);
        handlePattLine();
        break;
    case('g'):         // get current RGB color
        usiTwiTransmitByte( leds[0].r );
        usiTwiTransmitByte( leds[0].g );
        usiTwiTransmitByte( leds[0].b );
        break;
    case('a'):         // get address 
        // FIXME: does this take too long for I2C?
        usiTwiTransmitByte( 0x09 ); //eeprom_read_byte(&ee_i2c_addr) );
        break;
    case('A'):
        readI2Cvals(4);
        eeprom_write_byte( &ee_i2c_addr, pattline.args[1] ); // write address
        // FIXME:
        usiTwiSlaveInit( pattline.args[1] );                 // re-init

        delay(5);    // wait a bit so the USI can reset
        break;
    case('Z'):        // return protocol version
        usiTwiTransmitByte( BLINKM_PROTOCOL_VERSION_MAJOR );
        usiTwiTransmitByte( BLINKM_PROTOCOL_VERSION_MINOR );
        break;
    case('R'):    // read a script line, outputs 5: dur,cmd,arg1,arg2,arg3
        readI2Cvals(2);   // num, pos
        if( pattline.args[1] == 0 ) { // eeprom script
            eeprom_read_block( &pattline, &ee_patt_lines[pattline.args[2]],
                               sizeof(patt_line_t));
            usiTwiTransmitByte( pattline.args[0] );
            usiTwiTransmitByte( pattline.args[1] );
            usiTwiTransmitByte( pattline.args[2] );
            usiTwiTransmitByte( pattline.args[3] );
            usiTwiTransmitByte( pattline.args[4] );
        }
        else {                 // flash-based scripts
        }
        break;
    case('i'):         // return current input values
        usiTwiTransmitByte( inputs[0] );
        usiTwiTransmitByte( inputs[1] );
        usiTwiTransmitByte( inputs[2] );
        usiTwiTransmitByte( inputs[3] );
        break;
    }
}
#endif

//
inline void displayLEDs()
{
    //FastLED.show();
    ws2812_sendarray( (uint8_t*)leds, wsnLEDs );
}

//
//
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
                eeprom_read_block( &pattline, &ee_patt_lines[playpos],
                                   sizeof(patt_line_t));
            } 
            else {                  // flash
                patt_line_t* pp;
                memcpy_P(&pp, &patterns[patt_info.id-1], sizeof(patt_line_t*));
                memcpy_P(&pattline, &pp[playpos],sizeof(patt_line_t));
            }

            pattern_update_next += pattline.dmillis*10;
            pattern_update_next += (ttmp * patt_fudge_percent / 100);

            handlePattLine();
    
            playpos++;
            if( playpos == patt_info.end ) {
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
            //  playpos, pattline.dmillis*10);
        }

    } // playing
    else {
        //p("stopped");
    }

}

// 
//
uint8_t randRange(uint8_t prev, uint8_t range)
{
    if( range == 0 ) return prev;
    if( range == 0xff ) return (uint8_t)rand();  // max range
    int n = (uint8_t)rand() % range;
    if( prev != 0 ) n -= (range/2);
    //int n = ((uint8_t)rand() % range) - (range/2);  // half high, half low
    //int n = prev - (range/2) + ((uint8_t)rand() % range);
    //int n = prev + ((uint8_t)rand() % range - (prev/2));
    n = prev + n;  
    n = (n>255) ? 255 : ((n<0) ? 0 : n);
    return (uint8_t)n;
}



/*
// from: http://rgb-123.com/ws2812-color-output/
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
static inline uint8_t gamma( uint8_t n ) 
{
    return pgm_read_byte( &GammaE[n] );
}
*/
