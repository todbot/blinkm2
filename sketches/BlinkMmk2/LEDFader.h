

#ifndef LEDFader_h
#define LEDFader_h

#include "inttypes.h"

#include "pixeltypes.h"

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


class LEDFader
{
    uint8_t nLEDs;
    rgbfader_t* faders;
    CRGB* leds;

    void setDestN(rgb_t* newcolor, int steps, int ledn);
    
 public:
    LEDFader(CRGB* nleds, rgbfader_t* nfaders, uint8_t nnLEDs) 
        { leds = nleds; faders = nfaders; nLEDs = nnLEDs; }

    void setCurr(rgb_t* newcolor, int steps, int ledn);
    void setDest(rgb_t* newcolor, int steps, int ledn);
    void update(void);
};



#endif
