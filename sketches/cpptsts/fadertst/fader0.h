

#include "stdint.h"


// RGB triplet of 8-bit vals for input/output use
typedef struct {
    uint8_t g;
    uint8_t r;
    uint8_t b;
} rgb_t;

typedef struct {
    int8_t g;
    int8_t r;
    int8_t b;
} rgbint8_t;

// RGB triplet unsigned ints for internal use of 100x scale
// used instead of floating point
typedef struct {
    int g;
    int r;
    int b;
} rgbint_t;

typedef struct {
    rgb_t dest;  // the eventual destination color we want to hit
    rgbint_t curr100x;  // current color times 100 (to lessen int trunc issue)
    rgbint_t step100x;  // the amount of to move each tick
    int stepcnt;
} rgbfader_t; 


class Fader
{
    uint8_t nLEDs;
    rgb_t* leds;
    rgbfader_t* faders;

    void setDestN(rgb_t* newcolor, int steps, int ledn);
    
 public:
    Fader(rgb_t* nleds,rgbfader_t* nfaders, int nnleds) 
        { leds = nleds; faders = nfaders; nLEDs = nnleds; }
    void setCurr(rgb_t* newcolor, int steps, int ledn);
    void setDest(rgb_t* newcolor, int steps, int ledn);
    void update(void);
};

/*

diff , steps 

*/
