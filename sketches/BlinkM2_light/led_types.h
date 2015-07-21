#ifndef _LED_TYPES_
#define _LED_TYPES_

#include <stdint.h>

//  fract8   range is 0 to 0.99609375
//                 in steps of 0.00390625
typedef uint8_t   fract8;   // ANSI: unsigned short _Fract

// rgb_t conntains info for a single RGB color
// order is important: bytes are sent G,R,B to WS2812
typedef struct {
    union { 
        struct { 
            uint8_t g;
            uint8_t r;
            uint8_t b;
        };
        struct {
            uint8_t arg1;
            uint8_t arg0;
            uint8_t arg2;
        };
        uint8_t raw[3];
    };
} rgb_t;

/*
typedef struct {
    union {
        //rgb_t; // rgb;
        struct { 
            uint8_t g;
            uint8_t r;
            uint8_t b;
        };
        uint8_t args[3];
    };
} args_t;
*/
/*
typedef struct {
    union { uint8_t g;  uint8_t hue; uint8_t h; };
    union { uint8_t r;  uint8_t sat; uint8_t s; };
    union { uint8_t b;  uint8_t val; uint8_t v; };
} rgb2_t;
*/

// ONLY NEED A SINGLE FADER FOR CURRENT FADE IN PROCESS
// So ends up being like:
typedef struct {
    rgb_t last;           // last / starting color
    rgb_t dest;           // destination/ending color
} ledvector_t;  // was ledline_t

typedef struct {
    uint16_t pos;    // position of fade, ranges from 0-65535 (0=last,65535=dest)
    uint16_t posinc; // amount of to increment fader_val by each tick
    uint8_t ledn;  // which LEDs to fade
} fader_t;

#define FADERPOS_MAX ((uint16_t)0xffff)



#endif  // _LED_TYPES_
