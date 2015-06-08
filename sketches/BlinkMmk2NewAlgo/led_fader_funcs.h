#ifndef _LED_FADER_FUNCS_H_
#define _LED_FADER_FUNCS_H_

#include <stdbool.h>

#include "led_fader_types.h"


#define rgb_set( c, ar, ag, ab ) {c.r=ar; c.g=ag; c.b=ab; }

// stolen from FastLED
uint8_t scale8( uint8_t i, fract8 scale)
{
    return ((uint16_t)i * (uint16_t)(scale) ) >> 8;
}

// blend between a start color and a destination color
// by an 8-bit "fractional" amount ranging from 0-255 (0=start, 255=dest)
// return true when at top of fader
// idea stolen from FastLED 
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
void led_set_dest( rgb_t* newc, uint16_t dmillis, uint8_t ledn )
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
        f->faderposinc = ((uint32_t)65535 * 10) / dmillis ; ////zzled_compute_faderposinc( 1, dmillis );
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

//
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
        uint8_t faderpos = faders[i].faderpos/256;
        if( i==0 ) { // debug
            dbg(curc->r); dbg(','); dbg(curc->g); dbg(','); dbg(curc->g);
            dbgln(" = cur");
        }
        
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


/*
//
void led_set_dest( rgb_t* newc, uint16_t dmillis, uint8_t ledn );

//
void led_update_faders();

//
uint8_t scale8( uint8_t i, fract8 scale);

//
bool led_blend( rgb_t* curr, rgb_t* start, rgb_t* dest, fract8 blend_amount );

//
uint16_t led_compute_faderposinc( uint8_t millistep, uint16_t dmillis );
//uint16_t compute_faderposinc( uint16_t dmillis );

*/


#endif
