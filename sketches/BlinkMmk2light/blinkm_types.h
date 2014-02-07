
#ifndef BLINKM_TYPES_H
#define BLINKM_TYPES_H


#include <inttypes.h>


// RGB triplet of 8-bit vals for input/output use
typedef union {
    struct {
        union {
            uint8_t g;
            uint8_t h;
        };
        union { 
            uint8_t r;
            uint8_t s;
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

typedef struct { 
    uint8_t cmd;
    union { 
        rgb_t color;
        uint16_t dmillis; // hundreths of a sec
        uint16_t ledn;     // number of led, or 0 for all
    };
    uint8_t args[7];
} patt_line2_t;

//uint8_t playstart = 0; // start play position
//uint8_t playend   = patt_max; // end play position
//uint8_t playlen   = patt_max;
//uint8_t playcount = 0; // number of times to play loop, or 0=infinite
struct patt_info_t { 
    uint8_t id;
    uint8_t reps;
    uint8_t start;
    uint8_t end;
    uint8_t count;
};


// --------------------------------


// from: http://web.mit.edu/storborg/Public/hsvtorgb.c
//
//
//void hsvToRgb(unsigned char *r, unsigned char *g, unsigned char *b, 
//              unsigned char h, unsigned char s, unsigned char v)
void hsvToRgb(rgb_t* c)
{
    uint8_t h = c->h;
    uint8_t s = c->s;
    uint8_t v = c->v;
    uint8_t r,g,b;
    uint8_t region, fpart, p, q, t;
    
    if(s == 0) { // color is grayscale 
        c->r = c->g = c->b = v;
        return;
    }
    
    // make hue 0-5 
    region = h / 43;
    // find remainder part, make it from 0-255 
    fpart = (h - (region * 43)) * 6;
    
    // calculate temp vars, doing integer multiplication 
    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * fpart) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - fpart)) >> 8))) >> 8;
        
    // assign temp vars based on color cone region 
    switch(region) {
        case 0:  r = v; g = t; b = p; break;
        case 1:  r = q; g = v; b = p; break;
        case 2:  r = p; g = v; b = t; break;
        case 3:  r = p; g = q; b = v; break;
        case 4:  r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
   
    c->r = r;  c->g = g;  c->b = b;
}


// from:
// http://qscribble.blogspot.com/2008/06/integer-conversion-from-hsl-to-rgb.html
//
//void HSL_to_RGB(int hue, int sat, int lum, int* r, int* g, int* b)
void hsvToRgb0(rgb_t* c)
{
    int hue = c->h;
    int sat = c->s;
    int lum = c->v;
    uint8_t r,g,b;

    int v;

    //v = (lum < 128) ? (lum * (256 + sat)) >> 8 :
    //      (((lum + sat) << 8) - lum * sat) >> 8;
    v = (lum < 128) ? (long int)(lum * (256 + sat)) >> 8 :
        (((long int)(lum + sat) << 8) - (long int)lum * sat) >> 8; 
 
   if (v <= 0) {
        r = g = b = 0;
    } else {
        int m;
        int sextant;
        int fract, vsf, mid1, mid2;

        m = lum + lum - v;
        hue *= 6;
        sextant = hue >> 8;
        fract = hue - (sextant << 8);
        vsf = v * fract * (v - m) / v >> 8;
        mid1 = m + vsf;
        mid2 = v - vsf;
        switch (sextant) {
           case 0: r = v;    g = mid1; b = m;    break;
           case 1: r = mid2; g = v;    b = m;    break;
           case 2: r = m;    g = v;    b = mid1; break;
           case 3: r = m;    g = mid2; b = v;    break;
           case 4: r = mid1; g = m;    b = v;    break;
           case 5: r = v;    g = m;    b = mid2; break;
        }
    }
    c->r = r;  c->g = g;  c->b = b;
}

#endif
