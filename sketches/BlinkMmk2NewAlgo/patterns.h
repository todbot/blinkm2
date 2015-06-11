
#ifndef PATTERNS_H
#define PATTERNS_H

#include "led_fader_types.h"  // for rgb_t

struct patternline_t {
    uint8_t cmd;
    rgb_t color;
    uint16_t dmillis; // hundreths of a sec
    uint8_t ledn;     // number of led, or 0 for all
};

// doesn't work on avr-gcc 4.8?
//#define rgb(ar,ag,ab) { .r=ar, .g=ag, .b=ab }
// note: order is g,r,b
#define rgb(ar,ag,ab) { ag, ar, ab }

const patternline_t patternlines_default[] PROGMEM = {
    { 'c', rgb( 0x7f, 0x7f, 0x7f ),  100, 0 }, // 13 white B
    { 'c', rgb( 0x22, 0x00, 0x00 ),  100, 0 }, // 0  red A
    { 'c', rgb( 0x00, 0x22, 0x00 ),  100, 0 }, // 4  grn A
    { 'c', rgb( 0x00, 0x00, 0x22 ),  100, 0 }, // 6  blu A
    { 'c', rgb( 0x00, 0x00, 0x00 ),  100, 0 }, // 14 off B
};

const patternline_t patternlines_rgb[] PROGMEM = {
    { 'c', rgb( 0x33, 0x00, 0x00 ),  200, 0 }, // 0  red all
    { 'c', rgb( 0x00, 0x33, 0x00 ),  200, 0 }, // 1  grn all
    { 'c', rgb( 0x00, 0x00, 0x33 ),  200, 0 }, // 2  blu all
};

const patternline_t patternlines_blink_white[] PROGMEM = {
    { 'c', rgb( 0x11, 0x11, 0x11 ),  50, 0 }, // 0  white all
    { 'c', rgb( 0x00, 0x00, 0x00 ),  50, 0 }, // 1  off all
};

const patternline_t patternlines_stoplight[] PROGMEM = {
    { 'c', rgb( 0x33, 0x00, 0x00 ), 100, 0 }, // 0  red
    { 'c', rgb( 0x33, 0x55, 0x00 ),  50, 0 }, // 1  yellow
    { 'c', rgb( 0x00, 0x33, 0x33 ),  50, 0 }, // 1  greenblue
};

const patternline_t patternlines_hues[] PROGMEM = {
    { 'h', rgb( 0x00, 0xff, 0xff ),  100, 0 }, 
    { 'h', rgb( 0x40, 0xff, 0xff ),  100, 0x60 }, 
    { 'h', rgb( 0x80, 0xff, 0xff ),  100, 0x61 }, 
    { 'h', rgb( 0xc0, 0xff, 0xff ),  100, 0x62 }, 
};

const patternline_t patternlines_rgbmulti[] PROGMEM = {
    { 'c', rgb( 0x33, 0x00, 0x00 ),  50, 1 }, 
    { 'c', rgb( 0x00, 0x33, 0x00 ),  50, 1 }, 
    { 'c', rgb( 0x00, 0x00, 0x33 ),  50, 1 }, 
    { 'c', rgb( 0x33, 0x00, 0x00 ),  100, 2 }, 
    { 'c', rgb( 0x00, 0x33, 0x00 ),  100, 2 }, 
    { 'c', rgb( 0x00, 0x00, 0x33 ),  100, 2 }, 
    { 'c', rgb( 0x44, 0x44, 0x44 ),  100, 0 }, 
    { 'c', rgb( 0x00, 0x00, 0x00 ),  100, 0  }, 
    { 'c', rgb( 0x00, 0x44, 0x44 ),  200, 64 + 0  }, 
    { 'c', rgb( 0x44, 0x00, 0x44 ),  200, 64 + 1 }, 
    { 'c', rgb( 0x00, 0x00, 0x00 ),  50,  0  }, 
};

const patternline_t* const patterns[] PROGMEM = {
    patternlines_default,
    patternlines_rgb,
    patternlines_blink_white,
    patternlines_stoplight,
    patternlines_hues,
    patternlines_rgbmulti,
};

// this is so lame, but can't create a flexible array of patternlines in a struct
const int pattern_lens[] PROGMEM = {
    5,
    3,
    2,
    3,
    4,
    11,
};


/*
patternline_t patternlines_mem[]  = {
    //    G     R     B    fade ledn
    { { 0x00, 0x11, 0x00 }, 100, 0 }, // 0  red A
    { { 0x00, 0x33, 0x00 }, 100, 0 }, // 1  red B
    { { 0x00, 0x00, 0x00 }, 100, 0 }, // 2  off both
    { { 0x11, 0x00, 0x00 }, 100, 0 }, // 3  grn A
    { { 0x33, 0x00, 0x00 }, 100, 0 }, // 4  grn B
    { { 0x00, 0x00, 0x00 }, 100, 0 }, // 5  off both
    { { 0x00, 0x00, 0x11 }, 100, 0 }, // 6  blu A
    { { 0x00, 0x00, 0x33 }, 100, 0 }, // 7  blu B
    { { 0x00, 0x00, 0x00 }, 100, 0 }, // 8  off both
    { { 0x10, 0x10, 0x10 }, 100, 0 }, // 9  half-bright, both LEDs
    { { 0x00, 0x00, 0x00 }, 100, 0 }, // 10 off both
    { { 0x11, 0x11, 0x11 }, 100, 0 }, // 11 white A
    { { 0x20, 0x20, 0x20 }, 100, 0 }, // 12 off A
    { { 0x30, 0x30, 0x30 }, 100, 0 }, // 13 white B
    { { 0x40, 0x40, 0x40 }, 100, 0 }, // 14 off B
    { { 0x00, 0x00, 0x00 }, 100, 0 }, // 15 off everyone
};
*/
/*
#if 0
patternline_t patternlines_default[] PROGMEM = {
    //    G     R     B    fade ledn
    { { 0x00, 0x11, 0x00 },  100, 1 }, // 0  red A
    { { 0x00, 0x11, 0x00 },  100, 2 }, // 1  red B
    { { 0x00, 0x00, 0x00 },  100, 0 }, // 2  off both
    { { 0x11, 0x00, 0x00 },  100, 1 }, // 3  grn A
    { { 0x11, 0x00, 0x00 },  100, 2 }, // 4  grn B
    { { 0x00, 0x00, 0x00 },  100, 0 }, // 5  off both
    { { 0x00, 0x00, 0x11 },  100, 1 }, // 6  blu A
    { { 0x00, 0x00, 0x11 },  100, 2 }, // 7  blu B
    { { 0x00, 0x00, 0x00 },  100, 0 }, // 8  off both
    { { 0x10, 0x10, 0x10 }, 100, 0 }, // 9  half-bright, both LEDs
    { { 0x00, 0x00, 0x00 }, 100, 0 }, // 10 off both
    { { 0x11, 0x11, 0x11 },  50, 1 }, // 11 white A
    { { 0x00, 0x00, 0x00 },  50, 1 }, // 12 off A
    { { 0x7f, 0x7f, 0x7f },  50, 2 }, // 13 white B
    { { 0x00, 0x00, 0x00 }, 100, 2 }, // 14 off B
    { { 0x00, 0x00, 0x00 }, 100, 0 }, // 15 off everyone
};
#endif
*/

#endif

/*
// this does not work
struct pattern_t {
    uint8_t len;  // number of script lines, 0 == blank script, not playing
    patternline_t lines[];
};
static const pattern_t pattern_default PROGMEM  = { 
    3, 
    //(patternline_t[3])
    {
        { { 0x00, 0xff, 0x00 },  50, 0 }, // 0  red all
        { { 0xff, 0x00, 0x00 },  50, 0 }, // 1  grn all
        { { 0x00, 0x00, 0xff },  50, 0 }, // 2  blu all
    }
};
*/


/*
typedef struct {
    uint8_t len;  // number of script lines, 0 == blank script, not playing
    patternline_t lines[];
} pattern2_t;

pattern_t patterntoo PROGMEM = { 
    3, 
    (patternline_t[3]) 
    {
        { { 0x00, 0xff, 0x00 },  50, 0 }, // 0  red all
        { { 0xff, 0x00, 0x00 },  50, 0 }, // 1  grn all
        { { 0x00, 0x00, 0xff },  50, 0 }, // 2  blu all
    },
};
*/
