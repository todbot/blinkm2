#ifndef LEDSCRIPTS_H
#define LEDSCRIPTS_H

#include <Arduino.h>  // includes PROGMEM define

#include "BlinkMTypes.h"

// determine size of array statically
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// R,G,B,R,G,B,....
const script_line_t script_rgb[] PROGMEM = {
    { 50, {'c', {0xff,0x00,0x00} } },  //cmd, r,g,b
    { 50, {'c', {0x00,0xff,0x00} } },
    { 50, {'c', {0x00,0x00,0xff} } },
};

// white blink on & off
const script_line_t script_blink_white[] PROGMEM = {
     { 20, {'c', {0xff,0xff,0xff}}},
     { 20, {'c', {0x00,0x00,0x00}}},
};
// red blink on & off
const script_line_t script_blink_red[] PROGMEM = {
     { 20, {'c', {0xff,0x00,0x00}}},
     { 20, {'c', {0x00,0x00,0x00}}},
};
// green blink on & off
const script_line_t script_blink_green[] PROGMEM = {
     { 20, {'c', {0x00,0xff,0x00}}},
     { 20, {'c', {0x00,0x00,0x00}}},
};
// blue blink on & off
const script_line_t script_blink_blue[] PROGMEM = {
     { 20, {'c', {0x00,0x00,0xff}}},
     { 20, {'c', {0x00,0x00,0x00}}},
};
// cyan blink on & off
const script_line_t script_blink_cyan[] PROGMEM = {
     { 20, {'c', {0xff,0x00,0xff}}},
     { 20, {'c', {0x00,0x00,0x00}}},
};
// yellow blink on & off
const script_line_t script_blink_yellow[] PROGMEM = {
     { 20, {'c', {0xff,0xff,0x00}}},
     { 20, {'c', {0x00,0x00,0x00}}},
};
// black (off)
const script_line_t script_black[] PROGMEM = {
     { 20, {'c', {0x00,0x00,0x00}}},
};
// hue cycle
const script_line_t script_hue_cycle[] PROGMEM = {
     { 30, {'h', {0x00,0xff,0xff}}},  // red
     { 30, {'h', {0x2a,0xff,0xff}}},  // yellow 
     { 30, {'h', {0x54,0xff,0xff}}},  // green
     { 30, {'h', {0x7e,0xff,0xff}}},  // cyan
     { 30, {'h', {0xa8,0xff,0xff}}},  // blue
     { 30, {'h', {0xd2,0xff,0xff}}},  // magenta
     { 30, {'h', {0xff,0xff,0xff}}},  // red
};
// random color mood light
const script_line_t script_randmood[] PROGMEM = {
     {1, {'f', {3,0,0}}}, // slow fade time
     {50, {'C', {0x80,0x80,0x80}}}, // random fade to other hues
};
// virtual candle
const script_line_t script_candle[] PROGMEM = {
     {  1, {'f', {  10,0x00,0x00}}}, // set color_step (fade speed) 
     { 50, {'h', {0x10,0xff,0xff}}}, // set orange
     {  2, {'H', {0x00,0x00,0x30}}},
     { 27, {'H', {0x00,0x10,0x10}}},
     {  2, {'H', {0x00,0x10,0x10}}},
     {  7, {'H', {0x00,0x00,0x20}}},
     { 10, {'H', {0x00,0x00,0x40}}},
     { 10, {'H', {0x00,0x00,0x40}}},
     { 10, {'H', {0x00,0x00,0x20}}},
     { 50, {'h', {0x0a,0xff,0xff}}}, // set orange
     {  1, {'f', {  40,0x00,0x00}}}, // set color_step (fade speed) 
     {  5, {'H', {0x00,0x00,0xff}}},
     {  1, {'H', {0x00,0x00,0x40}}},
     {  1, {'H', {0x00,0x00,0x10}}},
     {  5, {'H', {0x00,0x00,0x40}}},
     {  5, {'H', {0x00,0x00,0x30}}},
};
// virtual water
const script_line_t script_water[] PROGMEM = { 
     {  1, {'f',   {10,0x00,0x00}}}, // set color_step (fade speed) 
     { 20, {'h', { 140,0xff,0xff}}}, // set blue
     {  2, {'H', {0x05,0x00,0x30}}},
     {  2, {'H', {0x05,0x00,0x10}}},
     {  2, {'H', {0x05,0x00,0x10}}},
     {  7, {'H', {0x05,0x00,0x20}}},
     { 10, {'H', {0x05,0x00,0x40}}},
     { 10, {'H', {0x15,0x00,0x40}}},
     { 10, {'H', {0x05,0x00,0x20}}},
     { 20, {'h', { 160,0xff,0xff}}}, // set blue
     {  1, {'f', {  20,0x00,0x00}}}, // set color_step (fade speed) 
     {  5, {'H', {0x05,0x00,0x40}}},
     {  1, {'H', {0x05,0x00,0x40}}},
     {  1, {'H', {0x05,0x00,0x10}}},
     {  5, {'H', {0x05,0x00,0x20}}},
     {  5, {'H', {0x05,0x00,0x30}}},
};

// old neon
const script_line_t script_oldneon[] PROGMEM = { 
     {  1, {'f', {  10,0x00,0x00}}}, // set color_step (fade speed) 
     { 20, {'h', {  10,0xff,0xff}}}, // set reddish orange
     {  2, {'H', {0x05,0x00,0x20}}},
     {  2, {'H', {0x05,0x00,0x10}}},
     {  2, {'H', {0x05,0x00,0x10}}},
     {  7, {'H', {0x05,0x00,0x20}}},
     { 10, {'H', {0x05,0x00,0x40}}},
     { 10, {'H', {0x15,0x00,0x40}}},
     { 10, {'H', {0x05,0x00,0x20}}},
     { 20, {'h', {  14,0xff,0xff}}}, // set reddish orange
     {  1, {'f', {  30,0x00,0x00}}}, // set color_step (fade speed) 
     {  5, {'H', {0x05,0x00,0xff}}},
     {  1, {'H', {0x05,0x00,0x40}}},
     {  1, {'H', {0x05,0x00,0x10}}},
     {  5, {'H', {0x05,0x00,0x20}}},
     {  5, {'H', {0x05,0x00,0x30}}},
};

// "the seasons" (cycle)
const script_line_t script_seasons[] PROGMEM = {
     {  1, {'f', {  4,0x00,0x00}}}, // set color_step (fade speed)
     {100, {'h', { 70,0xff,0xff}}}, // set green/yellow
     { 50, {'H', { 10,0x00,0x00}}}, // set green/yellow
     {100, {'h', {128,0xff,0xff}}}, // set blue/green
     { 50, {'H', { 10,0x00,0x00}}}, // set blue/green
     {100, {'h', { 20,0xff,0xff}}}, // set orange/red
     { 50, {'H', { 10,0x00,0x00}}}, // set orange/red
     {100, {'h', {200,0x40,0xff}}}, // set white/blue
     { 50, {'H', { 10,0x00,0x00}}}, // set white
};

// "thunderstom"  (blues & purples, flashes of white)
const script_line_t script_thunderstorm[] PROGMEM = {
     {  1, {'f', {  1,0x00,0x00}}}, // set color_step (fade speed) 
     {100, {'h', {180,0xff,0x20}}}, //
     { 20, {'H', {  0,0x10,0x10}}}, // randomize a bit
     {100, {'h', {175,0xff,0x20}}}, // set dark blueish purple
     {  1, {'f', {200,0x00,0x00}}}, // set fast fade speed 
     {  2, {'h', {188,0x00,0xff}}}, // white (no saturation)
     {  2, {'h', {178,0x00,0x00}}}, // black (no brightness)
     {  4, {'h', {188,0x00,0xff}}}, // white (no saturation)
     {  1, {'f', { 30,0x00,0x00}}}, // 
     { 40, {'h', {172,0xff,0x10}}}, // 
};

// stop light
const script_line_t script_stoplight[] PROGMEM = { 
     {  1, {'f', {100,0x00,0x00}}},  // set color_step (fade speed) 
     {100, {'h', {  0,0xff,0xff}}},  // set red
     {100, {'h', { 90,0xff,0xff}}},  // set 'green' (really teal)
     { 30, {'h', { 48,0xff,0xff}}},  // set yellow
};





const script_line_t  script_rgb_test[] PROGMEM =
    {
     { 1,  {'b', {0x2f,0x00,0x00} } },  //
     { 26, {'c', {0xff,0x00,0x00} } },  //cmd, r,g,b
     { 27, {'c', {0x00,0xff,0x00} } },
     { 28, {'c', {0x00,0x00,0xff} } },
     { 29, {'c', {0xff,0xff,0xff} } },
     { 10, {'b', {0xa0,0x00,0x00} } },  //
     { 10, {'b', {0x80,0x00,0x00} } },  //
     { 10, {'b', {0x60,0x00,0x00} } },  //
     { 10, {'b', {0x40,0x00,0x00} } },  //
     { 10, {'b', {0x10,0x00,0x00} } },  //
     { 50, {'c', {0xff,0x00,0x00} } },  //cmd, r,g,b
     { 50, {'c', {0x00,0xff,0x00} } },
     { 50, {'c', {0x00,0x00,0xff} } },
     //  { 50, {'n', {0xff,0xff,0xff} } },
     { 50, {'n', {0x01,0x01,0x01} } },
     { 50, {'.', {1,0,0         } } }, // set ledn to 1
     { 50, {'c', {0xff,0x00,0x00} } },  //cmd, r,g,b
     { 50, {'c', {0x00,0xff,0x00} } },
     { 50, {'c', {0x00,0x00,0xff} } },
     { 50, {'.', {2,0,0}}},              // set ledn to 2
     { 50, {'c', {0xff,0x00,0x00} } },  //cmd, r,g,b
     { 50, {'c', {0x00,0xff,0x00} } },
     { 50, {'c', {0x00,0x00,0xff} } },
     { 1,  {'.', {0,0,0         } } },
     
     // { 50, {'c', .args.r=0xff, .args.g=0x00, .args.b=0x00}}
     // { 50, {'c', .args= {0xff,0x00, 0x00} } }
     // { 50, {'c', rgb(0xff,0x00,0x00) } }
     // { 50, {'c', {.colr.r=0xff, .colr.g=0x00, .colr.b=0x00} } }
     // { 50, {'c', {.r=0xff, .g=0x00, .b=0x00} } }
    };



const script_line_t script_blinkm_default[] PROGMEM =
    {
     // dur, cmd,  arg1,arg2,arg3
     //    {   1, {'f', {  10,0x00,0x00}}}, // set color_step (fade speed) to 15
     { 51,  {'c', {200, 200,200 }}},
     { 52,  {'c', {201,   0,  0 }}},
     { 53,  {'c', {  0, 202,  0 }}},
     { 54,  {'c', {  0,   0,203 }}},
     { 55,  {'c', {  2,   4,  6 }}},
    };

const script_line_t script_blinkm_default_real[] PROGMEM =
    {
     // dur, cmd,  arg1,arg2,arg3
     //    {   1, {'f', {  10,0x00,0x00}}}, // set color_step (fade speed) to 15
     { 100, {'c', {0xff,0xff,0xff}}},
     { 50,  {'c', {0xff,0x00,0x00}}},
     { 50,  {'c', {0x00,0xff,0x00}}},
     { 50,  {'c', {0x00,0x00,0xff}}},
     { 50,  {'c', {0x00,0x00,0x00}}}
    };


// PUT THESE IN RAM FOR NOW
// const script_line_t* const scripts[] PROGMEM = {
const script_line_t* const scripts[] PROGMEM = {
    script_rgb,             // 1
    script_blink_white,     // 2
    script_blink_red,       // 3
    script_blink_green,     // 4
    script_blink_blue,      // 5
    script_blink_cyan,      // 6
    script_blink_yellow,    // 7

    script_black,           // 8
    script_hue_cycle,       // 9
    script_randmood,        // 10
    script_candle,          // 11
    script_water,           // 12
    script_oldneon,         // 13
    script_seasons,         // 14
    script_thunderstorm,    // 15
    script_stoplight,       // 16
    
    script_rgb_test,        // 17
    script_blinkm_default,  // 18
};

// this is so lame, but can't create a flexible array of patternlines in a struct in C++
const uint8_t script_lengths[] PROGMEM =  {
     ARRAY_SIZE( script_rgb ),           // 1
     ARRAY_SIZE( script_blink_white ),   // 2
     ARRAY_SIZE( script_blink_red ),     // 3
     ARRAY_SIZE( script_blink_green ),   // 4
     ARRAY_SIZE( script_blink_blue ),    // 5
     ARRAY_SIZE( script_blink_cyan ),    // 6
     ARRAY_SIZE( script_blink_yellow ),  // 7
     
     ARRAY_SIZE( script_black ),         // 8
     ARRAY_SIZE( script_hue_cycle ),     // 9
     ARRAY_SIZE( script_randmood ),      // 10
     ARRAY_SIZE( script_candle ),        // 11
     ARRAY_SIZE( script_water ),         // 12
     ARRAY_SIZE( script_oldneon ),       // 13
     ARRAY_SIZE( script_seasons ),       // 14
     ARRAY_SIZE( script_thunderstorm ),  // 15
     ARRAY_SIZE( script_stoplight ),     // 16
     
     ARRAY_SIZE( script_rgb_test ),      // 17
     ARRAY_SIZE( script_blinkm_default ),// 18
};

#define SCRIPT_DEFAULT_EE 18 // must match last line above

#endif
