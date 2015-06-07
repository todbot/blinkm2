#ifndef _LED_FADER_H_
#define _LED_FADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

//#define NUM_LEDS 32 

//  fract8   range is 0 to 0.99609375
//                 in steps of 0.00390625
typedef uint8_t   fract8;   // ANSI: unsigned short _Fract

// rgb_t conntains info for a single RGB color
typedef struct {
    uint8_t g;  // order is important: bytes are sent G,R,B to WS2812
    uint8_t r;
    uint8_t b;
} rgb_t;

// fader_t contains info needed to fade from start color to end color
typedef struct {
    rgb_t last;           // last / starting color
    rgb_t dest;           // destination/ending color
    uint16_t faderpos;    // position of fade, ranges from 0-65535 (0=last,65535=dest)
    uint16_t faderposinc; // amount of to increment fader_val by each tick 
} fader_t;


#define rgb_set( c, ar, ag, ab ) {c.r=ar; c.g=ag; c.b=ab; }


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

#ifdef __cplusplus
}
#endif

#endif
