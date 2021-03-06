

#include "stdint.h"


// RGB triplet of 8-bit vals for input/output use
typedef union {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
    uint8_t raw[3];
} rgb_t;

// RGB triplet unsigned ints for internal use of 100x scale
// used instead of floating point
typedef union { 
    struct {
        int r;
        int g;
        int b;
    };
    int raw[3];
} rgbint_t;

typedef struct {
    rgb_t dest;  // the eventual destination color we want to hit
    rgbint_t m100x;  // slope of the amount of to move each tick
    int stepcnt;  // number of steps for our fade run
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
    void setCurr(rgb_t* newcolor, int ledn);
    void setDest(rgb_t* newcolor, int steps, int ledn);
    void update(void);
};

