/*
 *
 *
 *
 */

#include "led_utils.h"
#include "light_ws2812.h"

// memory-intensive LED globals defined in main
extern rgb_t leds[];
extern ledvector_t ledvectors[];
extern fader_t fader;

#if ENABLE_BRIGHTNESS 
static uint8_t led_brightness;
#endif

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
#if ENABLE_BRIGHTNESS
    if( led_brightness ) {
        r = scale8( r, led_brightness);
        g = scale8( g, led_brightness);
        b = scale8( b, led_brightness);
    }
#endif
    leds[n].r = r;
    leds[n].g = g;
    leds[n].b = b;   
}

//
//
//
void led_setAll( uint8_t r, uint8_t g, uint8_t b)
{
    for(uint8_t i=0; i<NUM_LEDS; i++) {
        led_setN( i, r,g,b );
    }
}

uint8_t led_get_brightness() {
    return led_brightness - 1;
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
void led_set_brightness(uint8_t b)
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
// blend between a start color and a destination color
// by an 8-bit "fractional" amount ranging from 0-255 (0=start, 255=dest)
// return true when at top of fader
// idea stolen from FastLED
//
bool led_blend( rgb_t* curr, const rgb_t* start, const rgb_t* dest, fract8 blend_amount )
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
//
bool ledfader_update()
{
    uint8_t pos = fader.pos / 256;  // FIXME: document this descale
    // we're done if our current position is within reaching distance of max
    bool donefading = (fader.pos > (FADERPOS_MAX - fader.posinc));
    if( !donefading ) 
        fader.pos += fader.posinc;  // move fader a bit more forwards
    
    for( uint8_t i=0; i< NUM_LEDS; i++ ) {
        //dbgln("led:");
        bool shouldSet = led_should_set( i, fader.ledn );
        if( !shouldSet ) continue;
        rgb_t* start = &(ledvectors[i].last);
        rgb_t* end   = &(ledvectors[i].dest);
        rgb_t* curc  = &leds[i];
        
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
#define ledfader_compute_faderposinc( m ) \
    (( (uint32_t)FADERPOS_MAX * led_update_millis) / (m))

//
void ledfader_set_dest( rgb_t* newc, uint16_t dmillis, uint8_t ledn )
{
    for( uint8_t i = 0; i< NUM_LEDS; i++ ) {
        
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
#if ENABLE_BRIGHTNESS
        if( led_brightness ) { 
            lv->dest.r = scale8( lv->dest.r ,led_brightness);
            lv->dest.g = scale8( lv->dest.g ,led_brightness);
            lv->dest.b = scale8( lv->dest.b ,led_brightness);
        }
#endif
    }
}

//
void ledfader_rotateLEDs( uint8_t amount, uint8_t start, uint8_t end, uint8_t mode )
{
        // need to:
        // - copy old dest to last foreach ledfader
        // - copy new dest from next ledfader
        // - set ledn
        // - reset faderpos
        // FIXME: set led appropriately
        // FIXME: handle negative rotation
    
    // mode == 0 : rotate w/ carry
    // mode == 1 : rotate w/ drop
    // mode == 2 : bounce
    // if start < end then rotate to right
    // if start > end then rotate to left
    for( uint8_t i=0; i<amount; i++ ) {  // foreach rotation
        // rotate by one to the right, with loop
        rgb_t olddest = ledvectors[end].dest; 
        for( uint8_t j=end; j>start; j--) {
            ledvectors[j].last = leds[j]; // old curr becomes last
            ledvectors[j].dest = ledvectors[j-1].dest; // new dest is next led dest
        }
        ledvectors[start].last = leds[0];
        ledvectors[start].dest = olddest;
    }
    
}

// ---------------------------------------------------------------------------

//
// possible values of 'ledn'
//   0        : all LEDs
//   1 -  127 : the LED specified 
//  bank of patterns:
//                     11 1111 1111 2222 2222 2233
//   led   0123 4567 8901 2345 6789 0123 4567 8901
//   -------------------------|-------------------
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
//
// other patterns we need to support
//
// 4-LED w/ 0-overlap (8 options)
//   xx0 : 1111 0000 0000 0000 0000 0000 0000 0000
//   xx1 : 0000 1111 0000 0000 0000 0000 0000 0000
//   xx2 : 0000 0000 1111 0000 0000 0000 0000 0000
//   xx3 : 0000 0000 0000 1111 0000 0000 0000 0000
//   xx4 : 0000 0000 0000 0000 1111 0000 0000 0000
//   xx5 : 0000 0000 0000 0000 0000 1111 0000 0000
//   xx6 : 0000 0000 0000 0000 0000 0000 1111 0000
//   xx7 : 0000 0000 0000 0000 0000 0000 0000 1111
//
// 4-LED w/ 2-overlap (16 options)
//   xx0 : 1111 0000 0000 0000 0000 0000 0000 0000
//   xx1 : 0011 1100 0000 0000 0000 0000 0000 0000
//   xx2 : 0000 1111 0000 0000 0000 0000 0000 0000
//   xx3 : 0000 0011 1100 0000 0000 0000 0000 0000
//   xx4 : 0000 0000 1111 0000 0000 0000 0000 0000
//   xx5 : 0000 0000 0011 1100 0000 0000 0000 0000
//   xx6 : 0000 0000 0000 1111 0000 0000 0000 0000
//   xx7 : 0000 0000 0000 0011 1100 0000 0000 0000
//   xx8 : 0000 0000 0000 0000 1111 0000 0000 0000
//   xx9 : 0000 0000 0000 0000 0011 1100 0000 0000
//   xx0 : 0000 0000 0000 0000 0000 1111 0000 0000
//   x11 : 0000 0000 0000 0000 0000 0011 1100 0000
//   x12 : 0000 0000 0000 0000 0000 0000 1111 0000
//   x13 : 0000 0000 0000 0000 0000 0000 0011 1100
//   x14 : 0000 0000 0000 0000 0000 0000 0000 1111
//   x15 : 1100 0000 0000 0000 0000 0000 0000 0011
//
// 8-LED w/ 4-overlap (8 options)
//   xx0 : 1111 1111 0000 0000 0000 0000 0000 0000
//   xx1 : 0000 1111 1111 0000 0000 0000 0000 0000
//   xx2 : 0000 0000 1111 1111 0000 0000 0000 0000
//   xx3 : 0000 0000 0000 1111 1111 0000 0000 0000
//   xx4 : 0000 0000 0000 0000 1111 1111 0000 0000
//   xx5 : 0000 0000 0000 0000 0000 1111 1111 0000
//   xx6 : 0000 0000 0000 0000 0000 0000 1111 1111
//   xx7 : 1111 0000 0000 0000 0000 0000 0000 1111
//
// 2-LED  w/ 1-overlap (16 options)
//   xx0 : 1100 0000 0000 0000 1100 0000 0000 0000  
//   xx1 : 0110 0000 0000 0000 0110 0000 0000 0000
//   xx2 : 0011 0000 0000 0000 0011 0000 0000 0000
//   xx3 : 0001 1000 0000 0000 0001 1000 0000 0000
//   xx4 : 0000 1100 0000 0000 0000 1100 0000 0000 
//   xx5 : 0000 0110 0000 0000 0000 0110 0000 0000
//   xx6 : 0000 0011 0000 0000 0000 0011 0000 0000 
//   xx7 : 0000 0001 1000 0000 0000 0001 1000 0000 
//   xx8 : 0000 0000 1100 0000 0000 0000 1100 0000 
//   xx9 : 0000 0000 0110 0000 0000 0000 0110 0000 
//   x10 : 0000 0000 0011 0000 0000 0000 0011 0000 
//   x11 : 0000 0000 0001 1000 0000 0000 0001 1000 
//   x12 : 0000 0000 0000 1100 0000 0000 0000 1100 
//   x13 : 0000 0000 0000 0110 0000 0000 0000 0110 
//   x14 : 0000 0000 0000 0011 0000 0000 0000 0011 
//   x15 : 1000 0000 0000 0001 1000 0000 0000 0001

//   xxx : 1111 0000 0000 0000 0000 0000 0000 0000
//   xxx : 1111 1111 0000 0000 0000 0000 0000 0000
//   xxx : 1111 1111 1111 0000 0000 0000 0000 0000
//   xxx : 1111 1111 1111 1111 0000 0000 0000 0000
//   xxx : 1111 1111 1111 1111 1111 0000 0000 0000
//   xxx : 1111 1111 1111 1111 1111 1111 0000 0000
//   xxx : 1111 1111 1111 1111 1111 1111 1111 0000
//
//   xxx : 0000 1111 1111 1111 1111 1111 1111 1111 
//   xxx : 1111 0000 1111 1111 1111 1111 1111 1111
//   xxx : 1111 1111 0000 1111 1111 1111 1111 1111
//   xxx : 1111 1111 1111 0000 1111 1111 1111 1111  
//   xxx : 1111 1111 1111 1111 0000 1111 1111 1111
//   xxx : 1111 1111 1111 1111 1111 0000 1111 1111
//   xxx : 1111 1111 1111 1111 1111 1111 0000 1111
//   xxx : 1111 1111 1111 1111 1111 1111 1111 0000
//   
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

// ---------------------------------------------------------------------------

//
// scale8: scale one byte by a second one, which is treated as
//         the numerator of a fraction whose denominator is 256
//         In other words, it computes i * (scale / 256)
// scale an 8-bit value by an 8-bit "percentage" (that ranges from 0-255)
// stolen from FastLED
//
uint8_t scale8( uint8_t i, fract8 scale)
{
    return ((uint16_t)i * (uint16_t)(scale) ) >> 8;
}

#if 0
// scale8: scale one byte by a second one, which is treated as
//         the numerator of a fraction whose denominator is 256
//         In other words, it computes i * (scale / 256)
//         4 clocks AVR with MUL, 2 clocks ARM
uint8_t scale8( uint8_t i, fract8 scale)
{
#if SCALE8_C == 1
    return ((uint16_t)i * (uint16_t)(scale) ) >> 8;
#elif SCALE8_AVRASM == 1
#if defined(LIB8_ATTINY)
    uint8_t work=0;
    uint8_t cnt=0x80;
    asm volatile(
        "LOOP_%=:                             \n\t"
        /*"  sbrc %[scale], 0             \n\t"
        "  add %[work], %[i]            \n\t"
        "  ror %[work]                  \n\t"
        "  lsr %[scale]                 \n\t"
        "  clc                          \n\t"*/
        "  sbrc %[scale], 0             \n\t"
        "  add %[work], %[i]            \n\t"
        "  ror %[work]                  \n\t"
        "  lsr %[scale]                 \n\t"
        "  lsr %[cnt]                   \n\t"
        "brcc LOOP_%="
        : [work] "+r" (work), [cnt] "+r" (cnt)
        : [scale] "r" (scale), [i] "r" (i)
        :
      );
    return work;
#else
    asm volatile(
         /* Multiply 8-bit i * 8-bit scale, giving 16-bit r1,r0 */
         "mul %0, %1          \n\t"
         /* Move the high 8-bits of the product (r1) back to i */
         "mov %0, r1          \n\t"
         /* Restore r1 to "0"; it's expected to always be that */
         "clr __zero_reg__    \n\t"

         : "+a" (i)      /* writes to i */
         : "a"  (scale)  /* uses scale */
         : "r0", "r1"    /* clobbers r0, r1 */ );

    /* Return the result */
    return i;
#endif
#else
#error "No implementation for scale8 available."
#endif
}
#endif

// ---------------------------------------------------------------------------

// X(n+1) = (2053 * X(n)) + 13849)
#define RAND16_2053  ((uint16_t)(2053))
#define RAND16_13849 ((uint16_t)(13849))
#define RAND16_SEED  1337
uint16_t rand16seed = RAND16_SEED;

// from fastled
uint8_t random8()
{
    rand16seed = (rand16seed * RAND16_2053) + RAND16_13849;
    // return the sum of the high and low bytes, for better
    //  mixing and non-sequential correlation
    return (uint8_t)(((uint8_t)(rand16seed & 0xFF)) +
                     ((uint8_t)(rand16seed >> 8)));
}
// from fastled
uint8_t random8( uint8_t lim )
{
    uint8_t r = random8();
    r = scale8( r, lim);
    return r;
}

// from fastled 
uint8_t dim8_lin( uint8_t x )
{
    if( x & 0x80 ) {
        x = scale8( x, x);
    } else {
        x += 1;
        x /= 2;
    }
    return x;
}
// from fastled
//  The "video" version of scale8 guarantees that the output will
//  be only be zero if one or both of the inputs are zero.  If both
//  inputs are non-zero, the output is guaranteed to be non-zero.
//  This makes for better 'video'/LED dimming, at the cost of
//  several additional cycles.
uint8_t scale8_video( uint8_t i, fract8 scale)
{
    uint8_t j = (((int)i * (int)scale) >> 8) + ((i&&scale)?1:0);
    // uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    // uint8_t j = (i == 0) ? 0 : (((int)i * (int)(scale) ) >> 8) + nonzeroscale;
    return j;
}
// from fastled
// nscale8x3: scale three one byte values by a fourth one, which is treated as
//         the numerator of a fraction whose demominator is 256
//         In other words, it computes r,g,b * (scale / 256)
//
//         THIS FUNCTION ALWAYS MODIFIES ITS ARGUMENTS IN PLACE
void nscale8x3_video( uint8_t* r, uint8_t* g, uint8_t* b, fract8 scale)
{
    uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    *r = (*r == 0) ? 0 : (((int)*r * (int)(scale) ) >> 8) + nonzeroscale;
    *g = (*g == 0) ? 0 : (((int)*g * (int)(scale) ) >> 8) + nonzeroscale;
    *b = (*b == 0) ? 0 : (((int)*b * (int)(scale) ) >> 8) + nonzeroscale;
}
#define scale8_video_LEAVING_R1_DIRTY( i, scale) scale8_video(i,scale)

// Sometimes the compiler will do clever things to reduce
// code size that result in a net slowdown, if it thinks that
// a variable is not used in a certain location.
// This macro does its best to convince the compiler that
// the variable is used in this location, to help control
// code motion and de-duplication that would result in a slowdown.
#define FORCE_REFERENCE(var)  asm volatile( "" : : "r" (var) )
#define K255 255
#define K171 171
#define K85  85

// from fastled
// or, for simpler but worse implementation,
//  see: http://web.mit.edu/storborg/Public/hsvtorgb.c
void hsv2rgb_rainbow( const rgb_t* hsv, rgb_t* rgb)
{
    // Yellow has a higher inherent brightness than
    // any other color; 'pure' yellow is perceived to
    // be 93% as bright as white.  In order to make
    // yellow appear the correct relative brightness,
    // it has to be rendered brighter than all other
    // colors.
    // Level Y1 is a moderate boost, the default.
    // Level Y2 is a strong boost.
    const uint8_t Y1 = 1;
    const uint8_t Y2 = 0;

    // G2: Whether to divide all greens by two.
    // Depends GREATLY on your particular LEDs
    const uint8_t G2 = 0;

    // Gscale: what to scale green down by.
    // Depends GREATLY on your particular LEDs
    const uint8_t Gscale = 0;

    // FIXME: we store h,s,v in rgb struct
    uint8_t hue = hsv->r;
    uint8_t sat = hsv->g;
    uint8_t val = hsv->b;

    uint8_t offset = hue & 0x1F; // 0..31

    // offset8 = offset * 8
    uint8_t offset8 = offset;
    {
        offset8 <<= 1;
        asm volatile("");
        offset8 <<= 1;
        asm volatile("");
        offset8 <<= 1;
    }

    uint8_t third = scale8( offset8, (256 / 3));

    uint8_t r, g, b;

    if( ! (hue & 0x80) ) {
        // 0XX
        if( ! (hue & 0x40) ) {
            // 00X
            //section 0-1
            if( ! (hue & 0x20) ) {
                // 000
                //case 0: // R -> O
                r = K255 - third;
                g = third;
                b = 0;
                FORCE_REFERENCE(b);
            } else {
                // 001
                //case 1: // O -> Y
                if( Y1 ) {
                    r = K171;
                    g = K85 + third ;
                    b = 0;
                    FORCE_REFERENCE(b);
                }
                if( Y2 ) {
                    r = K171 + third;
                    //uint8_t twothirds = (third << 1);
                    uint8_t twothirds = scale8( offset8, ((256 * 2) / 3));
                    g = K85 + twothirds;
                    b = 0;
                    FORCE_REFERENCE(b);
                }
            }
        } else {
            //01X
            // section 2-3
            if( !  (hue & 0x20) ) {
                // 010
                //case 2: // Y -> G
                if( Y1 ) {
                    //uint8_t twothirds = (third << 1);
                    uint8_t twothirds = scale8( offset8, ((256 * 2) / 3));
                    r = K171 - twothirds;
                    g = K171 + third;
                    b = 0;
                    FORCE_REFERENCE(b);
                }
                if( Y2 ) {
                    r = K255 - offset8;
                    g = K255;
                    b = 0;
                    FORCE_REFERENCE(b);
                }
            } else {
                // 011
                // case 3: // G -> A
                r = 0;
                FORCE_REFERENCE(r);
                g = K255 - third;
                b = third;
            }
        }
    } else {
        // section 4-7
        // 1XX
        if( ! (hue & 0x40) ) {
            // 10X
            if( ! ( hue & 0x20) ) {
                // 100
                //case 4: // A -> B
                r = 0;
                FORCE_REFERENCE(r);
                //uint8_t twothirds = (third << 1);
                uint8_t twothirds = scale8( offset8, ((256 * 2) / 3));
                g = K171 - twothirds;
                b = K85  + twothirds;

            } else {
                // 101
                //case 5: // B -> P
                r = third;
                g = 0;
                FORCE_REFERENCE(g);
                b = K255 - third;

            }
        } else {
            if( !  (hue & 0x20)  ) {
                // 110
                //case 6: // P -- K
                r = K85 + third;
                g = 0;
                FORCE_REFERENCE(g);
                b = K171 - third;

            } else {
                // 111
                //case 7: // K -> R
                r = K171 + third;
                g = 0;
                FORCE_REFERENCE(g);
                b = K85 - third;

            }
        }
    }

    // This is one of the good places to scale the green down,
    // although the client can scale green down as well.
    if( G2 ) g = g >> 1;
    if( Gscale ) g = scale8_video_LEAVING_R1_DIRTY( g, Gscale);

    // Scale down colors if we're desaturated at all
    // and add the brightness_floor to r, g, and b.
    if( sat != 255 ) {

        nscale8x3_video( &r, &g, &b, sat);

        uint8_t desat = 255 - sat;
        desat = scale8( desat, desat);

        uint8_t brightness_floor = desat;
        r += brightness_floor;
        g += brightness_floor;
        b += brightness_floor;
    }

    // Now scale everything down if we're at value < 255.
    if( val != 255 ) {

        val = scale8_video_LEAVING_R1_DIRTY( val, val);
        nscale8x3_video( &r, &g, &b, val);
    }

    // Here we have the old AVR "missing std X+n" problem again
    // It turns out that fixing it winds up costing more than
    // not fixing it.
    // To paraphrase Dr Bronner, profile! profile! profile!
    //asm volatile(  ""  :  :  : "r26", "r27" );
    //asm volatile (" movw r30, r26 \n" : : : "r30", "r31");
    rgb->r = r;
    rgb->g = g;
    rgb->b = b;
}

#define APPLY_DIMMING(X) (X)
#define HSV_SECTION_6 (0x20)
#define HSV_SECTION_3 (0x40)
#if 1
void hsv2rgb_raw_C (const rgb_t* hsv, rgb_t* rgb)
{
    // FIXME: shorting hsv in rgb struct
    uint8_t hue = hsv->r;
    uint8_t sat = hsv->g;
    uint8_t val = hsv->b;
    // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
    // "Dimming" is used on saturation and brightness to make
    // the output more visually linear.

    // Apply dimming curves
    uint8_t value = APPLY_DIMMING(val);
    uint8_t saturation = sat;

    // The brightness floor is minimum number that all of
    // R, G, and B will be set to.
    uint8_t invsat = APPLY_DIMMING( 255 - saturation);
    uint8_t brightness_floor = (value * invsat) / 256;

    // The color amplitude is the maximum amount of R, G, and B
    // that will be added on top of the brightness_floor to
    // create the specific hue desired.
    uint8_t color_amplitude = value - brightness_floor;

    // Figure out which section of the hue wheel we're in,
    // and how far offset we are withing that section
    uint8_t section = hue / HSV_SECTION_3; // 0..2
    uint8_t offset = hue % HSV_SECTION_3;  // 0..63

    uint8_t rampup = offset; // 0..63
    uint8_t rampdown = (HSV_SECTION_3 - 1) - offset; // 63..0

    // We now scale rampup and rampdown to a 0-255 range -- at least
    // in theory, but here's where architecture-specific decsions
    // come in to play:
    // To scale them up to 0-255, we'd want to multiply by 4.
    // But in the very next step, we multiply the ramps by other
    // values and then divide the resulting product by 256.
    // So which is faster?
    //   ((ramp * 4) * othervalue) / 256
    // or
    //   ((ramp    ) * othervalue) /  64
    // It depends on your processor architecture.
    // On 8-bit AVR, the "/ 256" is just a one-cycle register move,
    // but the "/ 64" might be a multicycle shift process. So on AVR
    // it's faster do multiply the ramp values by four, and then
    // divide by 256.
    // On ARM, the "/ 256" and "/ 64" are one cycle each, so it's
    // faster to NOT multiply the ramp values by four, and just to
    // divide the resulting product by 64 (instead of 256).
    // Moral of the story: trust your profiler, not your insticts.

    // Since there's an AVR assembly version elsewhere, we'll
    // assume what we're on an architecture where any number of
    // bit shifts has roughly the same cost, and we'll remove the
    // redundant math at the source level:

    //  // scale up to 255 range
    //  //rampup *= 4; // 0..252
    //  //rampdown *= 4; // 0..252

    // compute color-amplitude-scaled-down versions of rampup and rampdown
    uint8_t rampup_amp_adj   = (rampup   * color_amplitude) / (256 / 4);
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) / (256 / 4);

    // add brightness_floor offset to everything
    uint8_t rampup_adj_with_floor   = rampup_amp_adj   + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;


    if( section ) {
        if( section == 1) {
            // section 1: 0x40..0x7F
            rgb->r = brightness_floor;
            rgb->g = rampdown_adj_with_floor;
            rgb->b = rampup_adj_with_floor;
        } else {
            // section 2; 0x80..0xBF
            rgb->r = rampup_adj_with_floor;
            rgb->g = brightness_floor;
            rgb->b = rampdown_adj_with_floor;
        }
    } else {
        // section 0: 0x00..0x3F
        rgb->r = rampdown_adj_with_floor;
        rgb->g = rampup_adj_with_floor;
        rgb->b = brightness_floor;
    }
}


#endif




