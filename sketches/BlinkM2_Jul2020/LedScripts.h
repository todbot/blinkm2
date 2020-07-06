#ifndef LEDSCRIPTS_H
#define LEDSCRIPTS_H

#include "PlayerTypes.h"
#include "Arduino.h"  // includes PROGMEM define

// FIXME: make this an enum
// possible values for boot_mode
#define BOOT_NOTHING     0
#define BOOT_PLAY_SCRIPT 1
#define BOOT_MODE_END    2

// R,G,B,R,G,B,....
const script_line_t script_rgb[] PROGMEM = {
    { 50, {'c', {0xff,0x00,0x00} } },  //cmd, r,g,b
    { 51, {'c', {0x00,0xff,0x00} } },
    { 52, {'c', {0x00,0x00,0xff} } },
    { 10, {'l', {1,0,0}}}
};

const script_line_t  script_rgb2[] PROGMEM = {
    { 30, {'c', {0xff,0x00,0x00} } },  //cmd, r,g,b
    { 31, {'c', {0x00,0xff,0x00} } },
    { 51, {'n', {0x00,0x00,0xff } } },
    { 32, {'c', {0xff,0xff,0xff} } }
    // { 50, {'c', .args.r=0xff, .args.g=0x00, .args.b=0x00}}
    // { 50, {'c', .args= {0xff,0x00, 0x00} } }
    // { 50, {'c', rgb(0xff,0x00,0x00) } }
    // { 50, {'c', {.colr.r=0xff, .colr.g=0x00, .colr.b=0x00} } }
    // { 50, {'c', {.r=0xff, .g=0x00, .b=0x00} } }
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

const script_line_t script_blinkm_default[] PROGMEM = {
      // dur, cmd,  arg1,arg2,arg3
      {   1, {'f', {  10,0x00,0x00}}}, // set color_step (fade speed) to 15
      { 100, {'c', {0xff,0xff,0xff}}},
      { 50,  {'c', {0xff,0x00,0x00}}},
      { 50,  {'c', {0x00,0xff,0x00}}},
      { 50,  {'c', {0x00,0x00,0xff}}},
      { 50,  {'c', {0x00,0x00,0x00}}}
};

// PUT THESE IN RAM FOR NOW
// const script_line_t* const scripts[] PROGMEM = {
const script_line_t* const scripts[] = {
    script_rgb,          // 1
    script_rgb2,         // 2
    script_blink_white,  // 3
    script_blink_red,    // 4
    script_blink_green,  // 5
    script_blink_blue,   // 6
    script_blinkm_default, // 7
};

// this is so lame, but can't create a flexible array of patternlines in a struct in C++
const int script_lengths[] = {
     ARRAY_SIZE( script_rgb ), 
     ARRAY_SIZE( script_rgb2 ),
     ARRAY_SIZE( script_blink_white ),
     ARRAY_SIZE( script_blink_red ),
     ARRAY_SIZE( script_blink_green ),
     ARRAY_SIZE( script_blink_blue ),
     ARRAY_SIZE( script_blinkm_default ),
};

#endif
