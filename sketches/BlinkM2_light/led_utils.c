/*
 *
 *
 *
 */

#include "led_utils.h"

// memory-intensive LED globals defined in main
extern rgb_t leds[];
extern ledvector_t ledvectors[];
extern fader_t fader;

static uint8_t led_brightness;

//
void led_show()
{
    ws2812_setleds( (struct cRGB*)leds, NUM_LEDS);
}

//
//
//
void led_setN(  uint8_t n, uint8_t r, uint8_t g, uint8_t b )
{
    leds[n].r = r;
    leds[n].g = g;
    leds[n].b = b;
    /*
    if(n < NUM_LEDS) {
        if( led_brightness ) { // See notes in setBrightness()
            r = (r * led_brightness) >> 8; // scale by brightness value
            g = (g * led_brightness) >> 8;
            b = (b * led_brightness) >> 8;
        }
        leds[n].r = r;
        leds[n].g = g;
        leds[n].b = b;
    }
    */
}

//
//
//
void led_setAll( uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t i;
    for(i=0; i<NUM_LEDS; i++) {
        led_setN( i, r,g,b );
    }
}

// stolen from Adafruit_NeoPixel
// Adjust output brightness; 0=darkest (off), 255=brightest.  This does
// NOT immediately affect what's currently displayed on the LEDs.  The
// next call to show() will refresh the LEDs at this level.  However,
// this process is potentially "lossy," especially when increasing
// brightness.  The tight timing in the WS2811/WS2812 code means there
// aren't enough free cycles to perform this scaling on the fly as data
// is issued.  So we make a pass through the existing color data in RAM
// and scale it (subsequent graphics commands also work at this
// brightness level).  If there's a significant step up in brightness,
// the limited number of steps (quantization) in the old data will be
// quite visible in the re-scaled version.  For a non-destructive
// change, you'll need to re-render the full strip data.  C'est la vie.
void led_setBrightness(uint8_t b)
{
    // Stored brightness value is different than what's passed.
    // This simplifies the actual scaling math later, allowing a fast
    // 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
    // adding 1 here may (intentionally) roll over...so 0 = max brightness
    // (color values are interpreted literally; no scaling), 1 = min
    // brightness (off), 255 = just below max brightness.
    uint8_t newBrightness = b + 1;
    if(newBrightness != led_brightness) { // Compare against prior value
        // Brightness has changed -- re-scale existing data in RAM
        uint8_t  c,
               *ptr          = (uint8_t*) leds,
               oldBrightness = led_brightness - 1; // De-wrap old brightness value
        uint16_t scale;
        if(oldBrightness == 0) scale = 0; // Avoid /0
        else if(b == 255) scale = 65535 / oldBrightness;
        else scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
        uint16_t i;
        for(i=0; i< (NUM_LEDS*3); i++) {
            c      = *ptr;
            *ptr++ = (c * scale) >> 8;
        }
        led_brightness = newBrightness;
    }
}

// ----------------------------------------------------------------------------

//
// scale an 8-bit value by an 8-bit "percentage" (that ranges from 0-255)
// stolen from FastLED
//
uint8_t scale8( uint8_t i, fract8 scale)
{
    return ((uint16_t)i * (uint16_t)(scale) ) >> 8;

}

//
// blend between a start color and a destination color
// by an 8-bit "fractional" amount ranging from 0-255 (0=start, 255=dest)
// return true when at top of fader
// idea stolen from FastLED
//
bool led_blend( rgb_t* curr, rgb_t* start, rgb_t* dest, fract8 blend_amount )
{
    if( blend_amount == 0) {
        //memcpy( curr, start, 3 );
        curr->r = start->r;
        curr->g = start->g;
        curr->b = start->b;
    }
    else if( blend_amount == 255) {
        //memcpy( curr, dest, 3 );
        curr->r = dest->r;
        curr->g = dest->g;
        curr->b = dest->b;
       return true;
    }
    else {
        fract8 keep_amount = 256 - blend_amount;
        
        curr->r = scale8( start->r,  keep_amount) +
                  scale8( dest->r,   blend_amount);

        curr->g = scale8( start->g,  keep_amount) +
                  scale8( dest->g,   blend_amount);

        curr->b = scale8( start->b,  keep_amount) +
                  scale8( dest->b,   blend_amount);
    }
    return false;
}


//
// Update the state of all faders and state of current LEDs
// Should be called every led_update_millis()
// normally lives in led_fader.c, but can't use dbg() there
//
int ledfader_update()
{
    uint8_t pos = fader.pos / 256;  // FIXME: document this descale
    // we're done if our current position is within reaching distance of max
    bool donefading = (fader.pos > (FADERPOS_MAX - fader.posinc));
    if( !donefading ) 
        fader.pos += fader.posinc;  // move fader a bit more forwards
    
    uint8_t i;
    for( i=0; i< NUM_LEDS; i++ ) {
      //dbgln("led:");
        bool shouldSet = led_should_set( i, fader.ledn );
        if( !shouldSet ) continue;
        rgb_t* curc  = &leds[i];
        rgb_t* start = &(ledvectors[i].last);
        rgb_t* end   = &(ledvectors[i].dest);
        
        if( !donefading ) {
            led_blend( curc, start, end, pos );  // do next increment of fade
        } else { 
            curc->r = end->r;
            curc->g = end->g;
            curc->b = end->b;
        }
    }
    return donefading;
}

//
// compute the amount the fader will move ever led_update_millis
//
#define ledfader_compute_faderposinc( m ) (( (uint32_t)FADERPOS_MAX * led_update_millis) / (m))

//
void ledfader_set_dest( rgb_t* newc, uint16_t dmillis, uint8_t ledn )
{
    uint8_t i;
    for( i = 0; i< NUM_LEDS; i++ ) {
        
        bool shouldSet = led_should_set( i, ledn );
        if( !shouldSet ) continue;
        
        rgb_t* curc = &leds[i];   // current led rgb value
        ledvector_t* lv = &ledvectors[i];
        // reset fader position & inc amount
        fader.pos = 0;
        fader.posinc = ledfader_compute_faderposinc( dmillis );
        fader.ledn = ledn;;
        
        // make current color the new start color
        lv->last.r = curc->r;
        lv->last.g = curc->g;
        lv->last.b = curc->b;
        // make new color the new destination
        lv->dest.r = newc->r;
        lv->dest.g = newc->g;
        lv->dest.b = newc->b;
    }
}


//
// possible values of 'ledn'
//   0        : all LEDs
//   1 -  127 : the LED specified 
//  bank of patterns:
//   128 : 1010 1010 1010 1010 1010 1010 1010 1010
//   129 : 0101 0101 0101 0101 0101 0101 0101 0101
//   130 : 1001 0010 0100 1001 0010 0100 1001 0010
//   131 : 0100 1001 0010 0100 1001 0010 0100 1001
//   132 : 0010 0100 1001 0010 0100 1001 0010 0100
//   133 : 1000 1000 1000 1000 1000 1000 1000 1000
//   134 : 0100 0100 0100 0100 0100 0100 0100 0100
//   135 : 0010 0010 0010 0010 0010 0010 0010 0010
//   136 : 0001 0001 0001 0001 0001 0001 0001 0001
//   137 : 1000 0100 0010 0001 0000 1000 0100 0010
//   138 : 0100 0010 0001 0000 1000 0100 0010 0001
//   139 : 0010 0001 0000 1000 0100 0010 0001 0000
//   140 : 0001 0000 1000 0100 0010 0001 0000 1000
//   141 : 0000 1000 0100 0010 0001 0000 1000 0100
//   142 : 1000 0010 0000 1000 0010 0000 1000 0010
//   143 : 0100 0001 0000 0100 0001 0000 0100 0001
//   144 : 0010 0000 1000 0010 0000 1000 0010 0000
//   145 : 0001 0000 0100 0001 0000 0100 0001 0000
//   146 : 0000 1000 0010 0000 1000 0010 0000 1000
//   147 : 0000 0100 0001 0000 0100 0001 0000 0100
//   148 : 1000 0001 0000 0010 0000 0100 0000 1000
//   149 : 0100 0000 1000 0001 0000 0010 0000 0100
//   150 : 0010 0000 0100 0000 1000 0001 0000 0010
//   151 : 0001 0000 0010 0000 0100 0000 1000 0001
//   152 : 0000 1000 0001 0000 0010 0000 0100 0000
//   153 : 0000 0100 0000 1000 0001 0000 0010 0000
//   154 : 0000 0010 0000 0100 0000 1000 0001 0000
//   155 : 1000 0000 1000 0000 1000 0000 1000 0000
//   156 : 0100 0000 0100 0000 0100 0000 0100 0000
//   157 : 0010 0000 0010 0000 0010 0000 0010 0000
//   158 : 0001 0000 0001 0000 0001 0000 0001 0000
//   159 : 0000 1000 0000 1000 0000 1000 0000 1000
//   160 : 0000 0100 0000 0100 0000 0100 0000 0100
//   161 : 0000 0010 0000 0010 0000 0010 0000 0010
//   162 : 0000 0001 0000 0001 0000 0001 0000 0001

//  pattern3  = 100100100...         3
//  pattern4  = 010010010...         3
//  pattern5  = 001001001...         3
//  pattern6  = 1000100010001...      4
//  pattern7  = 01000100010001...     4
//  pattern8  = 001000100010001...    4
//  pattern9  = 0001000100010001...   4
//  pattern10 = 1000010000100001...     5
//  pattern11 = 01000010000100001...    5
//  pattern12 = 001000010000100001...   5
//  pattern13 = 0001000010000100001...  5
//  pattern14 = 00001000010000100001... 5
//
static uint8_t led_set_state;
bool led_should_set( uint8_t pos, uint8_t ledn )
{
    if( pos==0 ) led_set_state = 0;  // initialize statevar
    
    if( ledn==0 ) { // 0 == all leds
        return true;
    }
    if( pos == ledn-1 ) // direct LED 1-N -> 0-(N-1)
        return true;
    else { 
        // patterns
        switch(ledn) {
        case 0x80:
        case 0x81:
            return (led_set_state++ % 2 == (ledn-0x80));
        case 0x82:
        case 0x83:
        case 0x84:
            return (led_set_state++ % 3 == (ledn-0x82));
        case 0x85:
        case 0x86:
        case 0x87:
        case 0x88:
            return (led_set_state++ % 4 == (ledn-0x85));
        case 0x89:
        case 0x8a:
        case 0x8b:
        case 0x8c:
        case 0x8d:
            return (led_set_state++ % 5 == (ledn-0x89));
        case 0x8e:
        case 0x8f:
        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
            return (led_set_state++ % 6 == (ledn-0x8e));
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0x99:
        case 0x9a:
            return (led_set_state++ % 7 == (ledn-0x94));
        case 0x9b:
        case 0x9c:
        case 0x9d:
        case 0x9e:
        case 0x9f:
        case 0xa0:
        case 0xa1:
        case 0xa2:
            return (led_set_state++ % 8 == (ledn-0x9b));
        }
    }
    return false;
}

