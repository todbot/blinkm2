
#ifndef PATTERNS_H
#define PATTERNS_H

#include "led_types.h"  // for rgb_t
#include <avr/pgmspace.h>

struct patternline_t {
    uint8_t cmd;
    rgb_t color;
    uint8_t ledn;     // number of led, or 0 for all
    uint16_t dmillis; // hundreths of a sec
};
/*
struct patternline_t {
    uint8_t cmd;
    union {
        struct { 
            rgb_t color;
            uint16_t dmillis; // hundreths of a sec
            uint8_t ledn;     // number of led, or 0 for all
        };
        uint8_t args[6];
    };
};
*/
// doesn't work on avr-gcc 4.8?
//#define rgb(ar,ag,ab) { .r=ar, .g=ag, .b=ab }

// note: order is g,r,b
#define rgb(ar,ag,ab) { ag, ar, ab }
#define hsv(ar,ag,ab) { ag, ar, ab }
#define args(ar,ag,ab) { ag, ar, ab }

const patternline_t patternlines_default[] PROGMEM = {
    { 'c', rgb( 0x7f, 0x7f, 0x7f ),  0, 100 }, // 13 white B
    { 'c', rgb( 0x22, 0x00, 0x00 ),  0, 100 }, // 0  red A
    { 'c', rgb( 0x00, 0x22, 0x00 ),  0, 100 }, // 4  grn A
    { 'c', rgb( 0x00, 0x00, 0x22 ),  0, 100 }, // 6  blu A
    { 'c', rgb( 0x00, 0x00, 0x00 ),  0, 100 }, // 14 off B
};

const patternline_t patternlines_rgb[] PROGMEM = {
    { 'c', rgb( 0x33, 0x00, 0x00 ),  0, 200 }, // 0  red all
    { 'c', rgb( 0x00, 0x33, 0x00 ),  0, 200 }, // 1  grn all
    { 'c', rgb( 0x00, 0x00, 0x33 ),  0, 200 }, // 2  blu all
};

const patternline_t patternlines_blink_white[] PROGMEM = {
    { 'c', rgb( 0x11, 0x11, 0x11 ),  0, 50 }, // 0  white all
    { 'c', rgb( 0x00, 0x00, 0x00 ),  0, 50 }, // 1  off all
};

const patternline_t patternlines_stoplight[] PROGMEM = {
    { 'c', rgb( 0x33, 0x00, 0x00 ),  0, 100 }, // 0  red
    { 'c', rgb( 0x33, 0x55, 0x00 ),  0, 50 }, // 1  yellow
    { 'c', rgb( 0x00, 0x33, 0x33 ),  0, 50 }, // 1  greenblue
};

const patternline_t patternlines_hues[] PROGMEM = {
    { 'h', rgb( 0x00, 0xff, 0xff ),  128+1, 50 }, 
    { 'h', rgb( 0x40, 0xff, 0xff ),  128+0, 50 }, 
    { 'h', rgb( 0x80, 0xff, 0xff ),  128+1, 50 }, 
    { 'h', rgb( 0xc0, 0xff, 0xff ),  128+0, 50 },
    { 'R', args( 1, 0, 0),         0,  0 },
};

const patternline_t patternlines_rotate[] PROGMEM = {
    //{ 'B', args(1, 0, 0),            0, 0 },
    //{ 'c', rgb( 0x00, 0xff, 0x00 ),  0, 0 }, 
    //{ 'c', rgb( 0x00, 0x00, 0xff ),  128+1, 0 }, 
    { 'c', rgb( 0xff, 0x00, 0x00 ),  1, 50 }, 
    { 'R', args( 1, 0, 0),           128, 20 },
    { 'B', args( 0, 10, 0),          0, 0 },
    { 'j', args(-2, 0, 0),           0, 0 },
};

const patternline_t patternlines_player[] PROGMEM = {
    { 'c', rgb( 44,44,44 ), 0, 100 },
    { 'p', args( 8, 1, 4 ), 2, 0 },
    { 'p', args( 8, 1, 1 ), 2, 0 },
    { 'c', rgb(0,0,0),      0, 300 },
};

const patternline_t patternlines_rgbmulti[] PROGMEM = {
    { 'c', rgb( 0x33, 0x00, 0x00 ),  1, 50 }, 
    { 'c', rgb( 0x00, 0x33, 0x00 ),  1, 50 }, 
    { 'c', rgb( 0x00, 0x00, 0x33 ),  1, 50 }, 
    { 'c', rgb( 0x33, 0x00, 0x00 ),  2, 50 }, 
    { 'c', rgb( 0x00, 0x33, 0x00 ),  2, 50 }, 
    { 'c', rgb( 0x00, 0x00, 0x33 ),  2, 50 }, 
    { 'c', rgb( 0x11, 0x11, 0x11 ),  0, 100 }, 
    { 'c', rgb( 0x00, 0x00, 0x00 ),  0, 100 }, 
    { 'c', rgb( 0x88, 0x00, 0x88 ),  128+0, 100 }, 
    { 'c', rgb( 0x22, 0x55, 0x22 ),  128+1, 100 }, 
    { 'c', rgb( 0x44, 0x00, 0x00 ),  128+2, 100 }, 
    { 'c', rgb( 0x00, 0x00, 0x44 ),  128+3, 100 }, 
    { 'c', rgb( 0x00, 0x33, 0x00 ),  128+4, 100 }, 
    { 'c', rgb( 0x00, 0x00, 0x00 ),  0, 50 }, 
};

const patternline_t* const patterns[] PROGMEM = {
    patternlines_default,
    patternlines_rgb,
    patternlines_blink_white,
    patternlines_stoplight,
    patternlines_hues,
    patternlines_rotate,
    patternlines_player,
    patternlines_rgbmulti,
};

#define PATTLEN(x) (sizeof(x) / sizeof(patternline_t))
// this is so lame, but can't create a flexible array of patternlines in a struct
const uint8_t pattern_lens[] PROGMEM = {
    PATTLEN( patternlines_default ),     //5,
    PATTLEN( patternlines_rgb ),         //3,
    PATTLEN( patternlines_blink_white ), //2,
    PATTLEN( patternlines_stoplight ),   //3,
    PATTLEN( patternlines_hues ),        //5,
    PATTLEN( patternlines_rotate ),      //4,
    PATTLEN( patternlines_player ),  
    PATTLEN( patternlines_rgbmulti ),    //14,
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
