#ifndef _LED_FADER_TYPES_
#define _LED_FADER_TYPES_

#include <stdint.h>
#include <stdbool.h>

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


#endif
