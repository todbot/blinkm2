
#ifndef BLINKM_TYPES_H
#define BLINKM_TYPES_H


#include <inttypes.h>


// RGB triplet of 8-bit vals for input/output use
typedef union {
    struct {
        union { 
            uint8_t g;
            uint8_t s;
        };
        union { 
            uint8_t r;
            uint8_t h;
        };
        union { 
            uint8_t b;
            uint8_t v;
        };
    }; 
    uint8_t raw[3];
} rgb_t;

#ifndef __INC_FASTSPI_LED2_H
typedef rgb_t CRGB;
#endif

// RGB triplet unsigned ints for internal use of 100x scale
// used instead of floating point
typedef union {
    struct {
        int16_t g;
        int16_t r;
        int16_t b;
    };
    int16_t raw[3];
} rgbint_t;

typedef struct {
    rgb_t dest;  // the eventual destination color we want to hit
    rgbint_t m100x;  // slope of the amount of to move each tick
    uint16_t stepcnt;  // number of steps for our fade run
} rgbfader_t;


//--------------------

//struct patt_line_t {
typedef union { 
    struct { 
        uint8_t cmd;
        rgb_t color;
        uint16_t dmillis; // hundreths of a sec
        uint16_t ledn;     // number of led, or 0 for all
    };
    uint8_t args[8];
} patt_line_t;

/*
typedef struct { 
    uint8_t cmd;
    union { 
        rgb_t color;
        uint16_t dmillis; // hundreths of a sec
        uint16_t ledn;     // number of led, or 0 for all
    };
    uint8_t args[7];
} patt_line2_t;
*/

//uint8_t playstart = 0; // start play position
//uint8_t playend   = patt_max; // end play position
//uint8_t playlen   = patt_max;
//uint8_t playcount = 0; // number of times to play loop, or 0=infinite
struct patt_info_t { 
    uint8_t id;
    uint8_t start;
    uint8_t end;
    uint8_t count;
};


#endif
