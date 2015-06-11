#ifndef _LED_FADER_FUNCS_H_
#define _LED_FADER_FUNCS_H_

#include <stdbool.h>


//
// Note: normally we would do proper .c/.h code structure
// but the current Arduino IDE compilation process doesn't
// use the same compiler flags & default includes on .c files
// as it does for .ino files.  This means that it's difficult
// to use Arduino libraries within .c files.
//
// To get around this, we create two .h files that are both included
// at the top of the .ino sketch file.
//
#include "config.h"
#include "led_fader_types.h"

// set the values of an rgb struct
#define rgb_set( c, ar, ag, ab ) {c.r=ar; c.g=ag; c.b=ab; }

uint8_t scale8( uint8_t i, fract8 scale);
bool led_blend( rgb_t* curr, rgb_t* start, rgb_t* dest, fract8 blend_amount );
void led_set_dest( rgb_t* newc, uint16_t dmillis, uint8_t ledn );
void led_update_faders();
int led_get_state(int m, int i, int st);

//
// compute the amount the fader will move ever led_update_millis
//
#define led_compute_faderposinc( m ) (( (uint32_t)65535 * led_update_millis) / (m))

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
        curr->r = start->r;
        curr->g = start->g;
        curr->b = start->b;
    }
    else if( blend_amount == 255) {
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
// Set the destination color for a set of LEDs
// @param new new led color
// @param dmillis time to fade to new color in "deci-millis"  (10msec ticks)
// @param ledn, which led to change, 0==all leds
//
void led_set_dest_old( rgb_t* newc, uint16_t dmillis, uint8_t ledn )
{
    if( ledn > NUM_LEDS ) { ledn = NUM_LEDS;  }
    uint8_t st  = ledn;
    uint8_t end = ledn;
    if( st==0 ) { end = NUM_LEDS; }
    else { st--;  } // adjust for 1..n => 0..(n-1)

    for( uint8_t i = st; i< end; i++ ) {
        rgb_t* curc = &leds[i];   // current led rgb value
        fader_t* f = &faders[i];  // fader for this led
        // reset fader position & inc amount
        f->faderpos = 0;
        f->faderposinc = led_compute_faderposinc( dmillis );
        //f->faderposinc = ((uint32_t)65535 * 10) / dmillis ; ////zzled_compute_faderposinc( 1, dmillis );
        if( i==0 ) { // debug
            dbg(curc->r); dbg(curc->g); dbg(curc->b);  dbgln("=set");
        }
        // make current color the new start color
        f->last.r = curc->r;
        f->last.g = curc->g;
        f->last.b = curc->b;
        // make new color the new destination
        f->dest.r = newc->r;
        f->dest.g = newc->g;
        f->dest.b = newc->b;
    }
}

void led_set_dest( rgb_t* newc, uint16_t dmillis, uint8_t ledn )
{
    uint8_t last_st=0;
    for( uint8_t i = 0; i< NUM_LEDS; i++ ) {
        // foreach LED figure out if we should update it
        bool shouldUpdate = false;
        if( ledn==0 || i == ledn ) // 0 == all leds
            shouldUpdate = true;
        else if( ledn >= 64 ) { 
            last_st = led_get_state( ledn, i, last_st );
            if( last_st ) shouldUpdate = true;
        }
        if( !shouldUpdate ) continue;
        
        rgb_t* curc = &leds[i];   // current led rgb value
        fader_t* f = &faders[i];  // fader for this led
        // reset fader position & inc amount
        f->faderpos = 0;
        f->faderposinc = led_compute_faderposinc( dmillis );
        //f->faderposinc = ((uint32_t)65535 * 10) / dmillis ; ////zzled_compute_faderposinc( 1, dmillis );
        dbg("set_dest:"); dbg(i); dbg(':');
        dbg(curc->r); dbg(','); dbg(curc->g); dbg(','); dbg(curc->b); dbgln('.');

        // make current color the new start color
        f->last.r = curc->r;
        f->last.g = curc->g;
        f->last.b = curc->b;
        // make new color the new destination
        f->dest.r = newc->r;
        f->dest.g = newc->g;
        f->dest.b = newc->b;
    }
}

//
// Update the state of all faders and state of current LEDs
// Should be called every led_update_millis()
// normally lives in led_fader.c, but can't use dbg() there
//
void led_update_faders()
{
    uint8_t i;
    for( i=0; i< NUM_LEDS; i++ ) {
      //dbgln("led:");
        rgb_t* curc  = &leds[i];
        rgb_t* start = &(faders[i].last);
        rgb_t* end   = &(faders[i].dest);
        uint8_t faderpos = faders[i].faderpos / 256;  // FIXME: document this divide
#if 0
        if( i==0 ) { // debug
            dbg(curc->r); dbg(','); dbg(curc->g); dbg(','); dbg(curc->g);
            dbgln(" = cur");
        }
#endif   
        bool donefading = led_blend( curc, start, end, faderpos );  // do next increment of fade
        if( !donefading ) 
            faders[i].faderpos += faders[i].faderposinc;
        else {
            curc->r = end->r;
            curc->g = end->g;
            curc->b = end->b;
        }
    }
}

// led patt         1         2         3         4         5         6
//   m    0123456789012345678901234567890123456789012345678901234567890123
//   00 - 1100000000000000000000000000000000000000000000000000000000000000
//   01 - 0011000000000000000000000000000000000000000000000000000000000000
//   02 - 0000110000000000000000000000000000000000000000000000000000000000
//   03 - 0000001100000000000000000000000000000000000000000000000000000000
//   04 - 0000000011000000000000000000000000000000000000000000000000000000
//   05 - 0000000000110000000000000000000000000000000000000000000000000000
//   06 - 0000000000001100000000000000000000000000000000000000000000000000
//   07 - 0000000000000011000000000000000000000000000000000000000000000000
//   08 - 0000000000000000110000000000000000000000000000000000000000000000
//   09 - 0000000000000000001100000000000000000000000000000000000000000000
//   0A - 0000000000000000000011000000000000000000000000000000000000000000
//   0B - 0000000000000000000000110000000000000000000000000000000000000000
//   0C - 0000000000000000000000001100000000000000000000000000000000000000
//   0D - 0000000000000000000000000011000000000000000000000000000000000000
//   0E - 0000000000000000000000000000110000000000000000000000000000000000
//   0F - 0000000000000000000000000000001100000000000000000000000000000000
//   10 - 0000000000000000000000000000000011000000000000000000000000000000
//   11 - 0000000000000000000000000000000000110000000000000000000000000000
//   12 - 0000000000000000000000000000000000001100000000000000000000000000
//   13 - 0000000000000000000000000000000000000011000000000000000000000000
//   14 - 0000000000000000000000000000000000000000110000000000000000000000
//   15 - 0000000000000000000000000000000000000000001100000000000000000000
//   16 - 0000000000000000000000000000000000000000000011000000000000000000
//   17 - 0000000000000000000000000000000000000000000000110000000000000000
//   18 - 0000000000000000000000000000000000000000000000001100000000000000
//   19 - 0000000000000000000000000000000000000000000000000011000000000000
//   1A - 0000000000000000000000000000000000000000000000000000110000000000
//   1B - 0000000000000000000000000000000000000000000000000000001100000000
//   1C - 0000000000000000000000000000000000000000000000000000000011000000
//   1D - 0000000000000000000000000000000000000000000000000000000000110000
//   1E - 0000000000000000000000000000000000000000000000000000000000001100
//   1F - 0000000000000000000000000000000000000000000000000000000000000011
//                  1         2         3         4         5         6
//        0123456789012345678901234567890123456789012345678901234567890123
//   20 - 1111000000000000000000000000000000000000000000000000000000000000
//   21 - 0000111100000000000000000000000000000000000000000000000000000000
//   22 - 0000000011110000000000000000000000000000000000000000000000000000
//   23 - 0000000000001111000000000000000000000000000000000000000000000000
//   24 - 0000000000000000111100000000000000000000000000000000000000000000
//   25 - 0000000000000000000011110000000000000000000000000000000000000000
//   26 - 0000000000000000000000001111000000000000000000000000000000000000
//   27 - 0000000000000000000000000000111100000000000000000000000000000000
//   28 - 0000000000000000000000000000000011110000000000000000000000000000
//   29 - 0000000000000000000000000000000000001111000000000000000000000000
//   2A - 0000000000000000000000000000000000000000111100000000000000000000
//   2B - 0000000000000000000000000000000000000000000011110000000000000000
//   2C - 0000000000000000000000000000000000000000000000001111000000000000
//   2D - 0000000000000000000000000000000000000000000000000000111100000000
//   2E - 0000000000000000000000000000000000000000000000000000000011110000
//   2F - 0000000000000000000000000000000000000000000000000000000000001111
//                  1         2         3         4         5         6
//        0123456789012345678901234567890123456789012345678901234567890123
//   30 - 1111111100000000000000000000000000000000000000000000000000000000
//   31 - 0000000011111111000000000000000000000000000000000000000000000000
//   32 - 0000000000000000111111110000000000000000000000000000000000000000
//   33 - 0000000000000000000000001111111100000000000000000000000000000000
//   34 - 0000000000000000000000000000000011111111000000000000000000000000
//   35 - 0000000000000000000000000000000000000000111111110000000000000000
//   36 - 0000000000000000000000000000000000000000000000001111111100000000
//   37 - 0000000000000000000000000000000000000000000000000000000011111111
//                  1         2         3         4         5         6
//        0123456789012345678901234567890123456789012345678901234567890123
//   38 - 1111111111111111000000000000000000000000000000000000000000000000
//   39 - 0000000000000000111111111111111100000000000000000000000000000000
//   3A - 0000000000000000000000000000000011111111111111110000000000000000
//   3B - 0000000000000000000000000000000000000000000000001111111111111111
//                  1         2         3         4         5         6
//        0123456789012345678901234567890123456789012345678901234567890123
//   3C - 0000000000000000000000000000000000000000000000000000000000000000
//   3D - 1111111111111111111111111111111100000000000000000000000000000000
//   3E - 0000000000000000000000000000000011111111111111111111111111111111
//   3F - 1111111111111111111111111111111111111111111111111111111111111111
//
// m = led pattern
// i = which led to use
// st = last led state
// returns the state of whether the specified LED should be affected
int led_get_state(int m, int i, int st)
{
    dbg("get_state:"); dbg(m);dbg(',');dbg(i);dbg(',');dbg(st);dbgln(':');
    //if( m < 0x40 ) {
    //    return m;
    //}
    m -= 0x40;
    
    if(      m < 0x20 ) {
        if( !st )  st = ((m-0x00)*2 == i) ? 2 : st;
        else st--;
    }
    else if( m < 0x30 ) {
        if( !st )  st = ((m-0x20)*4 == i) ? 4 : st;
        else st--;
    }
    else if( m < 0x38 ) {
        if( !st )  st = ((m-0x30)*8 == i) ? 8 : st;
        else st--;
    }
    else if( m < 0x3C ) {
        if( !st )  st = ((m-0x38)*16 == i) ? 16 : st;
        else st--;
    }
    else if( m < 0x3E ) {
        if( !st )  st = ((m-0x3C)*32 == i) ? 32 : st;
        else st--;
    }
    else {
        if( !st )  st = ((m-0x3F)*64 == i) ? 64 : st;
        else st--;
    }
    return st;
}


/*
  if( overlay_amount < 250 ) { // 97.6% arbitrary
    ctmp = blend( curr, dest, overlay_amount );
    overlay_amount += overlay_inc;
  }
  else {
    ctmp = dest;
  }

  uint8_t m = ntmp;
  if( m >= 64 )  {
    m -= 64;
    uint8_t st = 0;
    for( uint8_t i = 0; i< NUM_LEDS; i++ ) {
      st = get_led_state( m, i, st );
      if( st ) strip.setPixelColor( i, ctmp.r, ctmp.g, ctmp.b );
    }
  }
  else if( m == 0 ) {
    for( uint8_t i = 0; i< NUM_LEDS; i++ ) {
      strip.setPixelColor( i, ctmp.r, ctmp.g, ctmp.b );
    }
  }
  else {
    strip.setPixelColor( m, ctmp.r, ctmp.g, ctmp.b );
    dests[m] = ctmp;
  }
  */
  
  /*
  if( ntmp == 0 ) {          // set all LEDs to same color
    for( uint8_t i=0; i< NUM_LEDS; i++) {
      strip.setPixelColor( i, ctmp.r, ctmp.g, ctmp.b );
    }
  } else if( ntmp < 128 ) {  // set a single LED
    strip.setPixelColor( ntmp, ctmp.r, ctmp.g, ctmp.b );
  } else {                   // set a range, 6-bits = start, 2-bits = range
    uint8_t start = ntmp & 0b;
    uint8_t len   = (ntmp >> 4);
    strip.setPixelColor( ntmp, ctmp.r, ctmp.g, ctmp.b );
  }
  */

// X(n+1) = (2053 * X(n)) + 13849)
#define RAND16_2053  ((uint16_t)(2053))
#define RAND16_13849 ((uint16_t)(13849))
#define RAND16_SEED  1337
uint16_t rand16seed = RAND16_SEED;

//
static uint8_t random8()
{
    rand16seed = (rand16seed * RAND16_2053) + RAND16_13849;
    // return the sum of the high and low bytes, for better
    //  mixing and non-sequential correlation
    return (uint8_t)(((uint8_t)(rand16seed & 0xFF)) +
                     ((uint8_t)(rand16seed >> 8)));
}

//  The "video" version of scale8 guarantees that the output will
//  be only be zero if one or both of the inputs are zero.  If both
//  inputs are non-zero, the output is guaranteed to be non-zero.
//  This makes for better 'video'/LED dimming, at the cost of
//  several additional cycles.
static uint8_t scale8_video( uint8_t i, fract8 scale)
{
    uint8_t j = (((int)i * (int)scale) >> 8) + ((i&&scale)?1:0);
    // uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    // uint8_t j = (i == 0) ? 0 : (((int)i * (int)(scale) ) >> 8) + nonzeroscale;
    return j;
}
// nscale8x3: scale three one byte values by a fourth one, which is treated as
//         the numerator of a fraction whose demominator is 256
//         In other words, it computes r,g,b * (scale / 256)
//
//         THIS FUNCTION ALWAYS MODIFIES ITS ARGUMENTS IN PLACE
static void nscale8x3_video( uint8_t& r, uint8_t& g, uint8_t& b, fract8 scale)
{
    uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    r = (r == 0) ? 0 : (((int)r * (int)(scale) ) >> 8) + nonzeroscale;
    g = (g == 0) ? 0 : (((int)g * (int)(scale) ) >> 8) + nonzeroscale;
    b = (b == 0) ? 0 : (((int)b * (int)(scale) ) >> 8) + nonzeroscale;
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

static void hsv2rgb_rainbow( const rgb_t* hsv, rgb_t* rgb)
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

        nscale8x3_video( r, g, b, sat);

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
        nscale8x3_video( r, g, b, val);
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




#endif
