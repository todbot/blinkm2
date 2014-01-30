
#ifndef StructTest_h
#define StructTest_h

#include <inttypes.h>


// RGB triplet of 8-bit vals for input/output use
typedef struct {
    uint8_t g;
    uint8_t r;
    uint8_t b;
} rgb_t;

// RGB triplet unsigned ints for internal use of 100x scale
// used instead of floating point
typedef struct {
    int g;
    int r;
    int b;
} rgbint_t;

typedef struct {
    rgbint_t dest100x;  // the eventual destination color we want to hit
    rgbint_t step100x;  // the amount of to move each tick
    rgbint_t curr100x;  // the current color, times 10 (to lessen int trunc issue)
    int stepcnt;
} rgbfader_t;

typedef struct {
    rgb_t color;
    uint16_t dmillis; // hundreths of a sec
    uint8_t ledn;     // number of led, or 0 for all
} patternline_t;

typedef struct {
    uint8_t len;  // number of script lines, 0 == blank script, not playing
    patternline_t* lines;  // can't have flexible array in c++ it seems
} pattern_t;


typedef struct { 
    uint8_t len;
    patternline_t lines[];
} pattern2_t;


static pattern2_t patterntoo = {
    3, 
    { 
        { { 0x00, 0xff, 0x00 },  50, 0 }, // 0  red all
        { { 0xff, 0x00, 0x00 },  50, 0 }, // 1  grn all
        { { 0x00, 0x00, 0xff },  50, 0 }, // 2  blu all
    }
};    


// hack to make this compile on commandline
#ifndef PROGMEM
#define PROGMEM
#endif

const int patt_max = 16;

patternline_t patternlines_default[patt_max] =
{
    //    G     R     B    fade ledn
    { { 0x00, 0xff, 0x00 },  50, 1 }, // 0  red A
    { { 0x00, 0xff, 0x00 },  50, 2 }, // 1  red B
    { { 0x00, 0x00, 0x00 },  50, 0 }, // 2  off both
    { { 0xff, 0x00, 0x00 },  50, 1 }, // 3  grn A
    { { 0xff, 0x00, 0x00 },  50, 2 }, // 4  grn B
    { { 0x00, 0x00, 0x00 },  50, 0 }, // 5  off both
    { { 0x00, 0x00, 0xff },  50, 1 }, // 6  blu A
    { { 0x00, 0x00, 0xff },  50, 2 }, // 7  blu B
    { { 0x00, 0x00, 0x00 },  50, 0 }, // 8  off both
    { { 0x80, 0x80, 0x80 }, 100, 0 }, // 9  half-bright, both LEDs
    { { 0x00, 0x00, 0x00 }, 100, 0 }, // 10 off both
    { { 0xff, 0xff, 0xff },  50, 1 }, // 11 white A
    { { 0x00, 0x00, 0x00 },  50, 1 }, // 12 off A
    { { 0xff, 0xff, 0xff },  50, 2 }, // 13 white B
    { { 0x00, 0x00, 0x00 }, 100, 2 }, // 14 off B
    { { 0x00, 0x00, 0x00 }, 100, 0 }, // 15 off everyone
};

patternline_t patternlines_rgb[] = 
{
    { { 0x00, 0xff, 0x00 },  50, 0 }, // 0  red all
    { { 0xff, 0x00, 0x00 },  50, 0 }, // 1  grn all
    { { 0x00, 0x00, 0xff },  50, 0 }, // 2  blu all
};

pattern_t pattern_default PROGMEM = { 16, patternlines_default };
pattern_t pattern_rgb PROGMEM     = {  3, patternlines_rgb     };

pattern_t* const patterns[] PROGMEM = 
{
    &pattern_default,
    &pattern_rgb,
};

#endif
