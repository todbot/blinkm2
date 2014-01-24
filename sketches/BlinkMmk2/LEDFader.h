

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

typedef struct {
    rgb_t color;
    uint16_t dmillis; // hundreths of a sec
    uint8_t ledn;     // number of led, or 0 for all
} patternline_t;

typedef void (*UpdateHandler)(void);


class LEDFader
{
    
 private:
    //UpdateHandler  updateHandler;
    int nLEDs;
    CRGB* leds;
    rgbfader_t* faders;

    void setDestN(CRGB* newcolor, int steps, int ledn);
    
 public:
    LEDFader( int nleds, CRGB* leds, rgbfader_t* faders);
    
    //void setUpdateHandler( UpdateHandler handler );
    
    void setCurr(CRGB* newcolor, int steps, int ledn);
    void setDest(CRGB* newcolor, int steps, int ledn);
    void update(void);
};

#endif
