
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
    // rotate: args( mode|amount, start, end )
    { 'R', args( 1, 0, 0),           0, 20 },
    //{ 'R', args( 1, 0, 0),           128, 20 },
    //{ 'B', args( 0, 10, 0),          0, 0 },
//    { 'j', args(((uint8_t)-2, 0, 0),           0, 0 },
};

const patternline_t patternlines_player[] PROGMEM = {
    { 'c', rgb( 44,44,44 ), 0, 100 },
    { 'p', args( 8, 1, 4 ), 2, 0 },
    { 'p', args( 8, 1, 1 ), 2, 0 },
    { 'c', rgb(0,0,0),      0, 100 },
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

const patternline_t patternlines_fire1[] PROGMEM = {
    { 'c', rgb( 0xff,0x44,0x00 ), 0, 10 },
    { 'C', rgb( 0xff,0x44,0x00 ), 1, 30 },
    { 'T', args(20,0,0),  0, 1 },
    { 'R', args(1,0,0),  0, 10 },
//    { 'j', args(-3,0,0), 0, 0 },
};

const patternline_t patternlines_random1[] PROGMEM = {
    { 'C', rgb( 155,155,155), 1, 30 },
    { 'R', args(1,0,0),  0, 30 },
    //{ 'T', args(100,0,0 ), 0,  1 },  //
    //{ 'c', rgb(0,0,0),      0, 3},
    //{ 'T', args( 3,0,0 ), 0, 3 },
};

const patternline_t patternlines_rotate2[] PROGMEM = {
    { 'c', rgb( 44,44,44 ), 0, 50 },
    { 'R', args( 1, 3,5), 1, 50 }, // ledn is mode?  1=bounce, 0=1-way
//    { 'R', args( -1, 3,5), 0, 50 }, // ledn is mode?  1=bounce, 0=1-way
//    { 'j', args(-1,0,0), 0,0},
};

const patternline_t* const patterns[] PROGMEM = {
    patternlines_default,     // 1
    patternlines_rgb,         // 2
    patternlines_blink_white, // 3
    patternlines_stoplight,   // 4 
    patternlines_hues,        // 5
    patternlines_rotate,      // 6
    patternlines_player,      // 7
    patternlines_rgbmulti,    // 8
    patternlines_fire1,       // 9
    patternlines_random1,     // 10
    // be sure to add pattern to pattern_lens below too
};

#define PATTLEN(x) (sizeof(x) / sizeof(patternline_t))
// this is so lame, but can't create a flexible array of patternlines in a struct
const uint8_t pattern_lens[] PROGMEM = {
    PATTLEN( patternlines_default ),     //1
    PATTLEN( patternlines_rgb ),         //2
    PATTLEN( patternlines_blink_white ), //3
    PATTLEN( patternlines_stoplight ),   //4
    PATTLEN( patternlines_hues ),        //5
    PATTLEN( patternlines_rotate ),      //6
    PATTLEN( patternlines_player ),      //7
    PATTLEN( patternlines_rgbmulti ),    //8
    PATTLEN( patternlines_fire1 ),       //9 
    PATTLEN( patternlines_random1 ),     //10
};

#endif
