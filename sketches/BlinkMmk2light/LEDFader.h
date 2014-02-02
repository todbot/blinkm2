

#ifndef LEDFader_h
#define LEDFader_h

#include "inttypes.h"

//#include "pixeltypes.h"
#define CRGB rgb_t

// RGB triplet of 8-bit vals for input/output use
typedef union {
    struct {
        uint8_t g;
        uint8_t r;
        uint8_t b;
    }; 
    uint8_t raw[3];
} rgb_t;

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
    rgbint_t dest100x;  // the eventual destination color we want to hit
    rgbint_t step100x;  // slope of the amount of to move each tick
    rgbint_t curr100x;
    int stepcnt;  // number of steps for our fade run
} rgbfader_t;


class LEDFader
{
    uint8_t nLEDs;
    rgbfader_t* faders;
    CRGB* leds;

    void setDestN(rgb_t* newcolor, int steps, int ledn);
    
 public:
    LEDFader(CRGB* nleds, rgbfader_t* nfaders, uint8_t nnLEDs) 
        { leds = nleds; faders = nfaders; nLEDs = nnLEDs; }

    void setCurr(rgb_t* color, int ledn);
    void setDest(rgb_t* color, int steps, int ledn);
    void update(void);
};



#endif

