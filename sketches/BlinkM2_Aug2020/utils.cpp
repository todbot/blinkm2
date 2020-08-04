
#include "utils.h"

#include "BlinkMTypes.h"

//__attribute__ ((always_inline)) static inline uint8_t colorSlide( uint8_t curr, uint8_t dest, uint8_t step )
 uint8_t colorSlide( uint8_t curr, uint8_t dest, uint8_t step )
{
    int diff = curr - dest;
    if(diff < 0)  diff = -diff;

    if( diff <= step ) return dest;
    if( curr == dest ) return dest;
    else if( curr < dest ) return curr + step;
    else                   return curr - step;
}

/// (from FastLED)
/// scale three one byte values by a fourth one, which is treated as
///         the numerator of a fraction whose demominator is 256
///         In other words, it computes r,g,b * (scale / 256), ensuring
/// that non-zero values passed in remain non zero, no matter how low the scale
/// argument.
///
///         THIS FUNCTION ALWAYS MODIFIES ITS ARGUMENTS IN PLACE
 void fl_nscale8x3_video( uint8_t& r, uint8_t& g, uint8_t& b, uint8_t scale)
{
    uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    r = (r == 0) ? 0 : (((int)r * (int)(scale) ) >> 8) + nonzeroscale;
    g = (g == 0) ? 0 : (((int)g * (int)(scale) ) >> 8) + nonzeroscale;
    b = (b == 0) ? 0 : (((int)b * (int)(scale) ) >> 8) + nonzeroscale;
}

///
 uint8_t rand8bit(void)
{
    static int16_t rctx;
    if( rctx == 0 ) rctx = 1234; // 123467;
    int hi = (rctx<<1) / 123;
    int lo = (rctx<<1) % 123;
    int  x = 1687 * lo - 286 * hi;
    rctx = x;
    return x;
}

#define rand(x) rand8bit(x)

 uint8_t get_rand_range(uint8_t prev, uint8_t range)
{
    if( range == 0 ) return prev;
    if( range == 0xff ) return (uint8_t)rand();  // max range
    int n = (uint8_t)rand() % range;
    if( prev != 0 ) n -= (range/2);
    //int n = ((uint8_t)rand() % range) - (range/2);  // half high, half low
    //int n = prev - (range/2) + ((uint8_t)rand() % range);
    //int n = prev + ((uint8_t)rand() % range - (prev/2));
    n = prev + n;
    n = (n>255) ? 255 : ((n<0) ? 0 : n);
    return (uint8_t)n;
}


// NO, this is not linear interporlation. this is "next step" in a linear interp
// this should be called something like "nextstep"?
//
// liner interpolation (lerp) of curr to dest by step amount
// TODO: look into 'lerp8by8()' in FastLED/lib8tion.h
//uint8_t Player::colorSlide( uint8_t curr, uint8_t dest, uint8_t step )
//__attribute__ ((always_inline)) static inline uint8_t colorSlide( uint8_t curr, uint8_t dest, uint8_t step )
uint8_t colorSlideOld( uint8_t curr, uint8_t dest, uint8_t step )
{
    int diff = curr - dest;
    if(diff < 0)  diff = -diff;

    if( diff <= step ) return dest;
    if( curr == dest ) return dest;
    else if( curr < dest ) return curr + step;
    else                   return curr - step;
}

// from https://stackoverflow.com/a/22120275/221735
 void hsvToRgb(uint8_t oh, uint8_t os, uint8_t ov,
                     uint8_t* r, uint8_t* g, uint8_t* b)
{
    unsigned char region, p, q, t;
    unsigned int h, s, v, remainder;

    if (os == 0) {
        *r = ov;  *g = ov;  *b = ov;
        return;
    }

    // converting to 16 bit to prevent overflow
    h = oh;  s = os;   v = ov;

    region = h / 43;
    remainder = (h - (region * 43)) * 6;

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:
            *r = v;  *g = t;  *b = p;
            break;
        case 1:
            *r = q;  *g = v;  *b = p;
            break;
        case 2:
            *r = p;  *g = v;  *b = t;
            break;
        case 3:
            *r = p;  *g = q;  *b = v;
            break;
        case 4:
            *r = t;  *g = p;  *b = v;
            break;
        default:
            *r = v;  *g = p;  *b = q;
            break;
    }
}

#define APPLY_DIMMING(X) (X)
#define HSV_SECTION_6 (0x20)
#define HSV_SECTION_3 (0x40)

#define hsv2rgb_rainbow(a,b) hsv2rgb_raw_C(a,b)

 void hsv2rgb_raw_C (const struct hsv_t & hsv, struct rgb_t & rgb)
{
    // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
    // "Dimming" is used on saturation and brightness to make
    // the output more visually linear.

    // Apply dimming curves
    uint8_t value = APPLY_DIMMING( hsv.val);
    uint8_t saturation = hsv.sat;

    // The brightness floor is minimum number that all of
    // R, G, and B will be set to.
    uint8_t invsat = APPLY_DIMMING( 255 - saturation);
    uint8_t brightness_floor = (value * invsat) / 256;

    // The color amplitude is the maximum amount of R, G, and B
    // that will be added on top of the brightness_floor to
    // create the specific hue desired.
    uint8_t color_amplitude = value - brightness_floor;

    // Figure out which section of the hue wheel we're in,
    // and how far offset we are withing that section
    uint8_t section = hsv.hue / HSV_SECTION_3; // 0..2
    uint8_t offset = hsv.hue % HSV_SECTION_3;  // 0..63

    uint8_t rampup = offset; // 0..63
    uint8_t rampdown = (HSV_SECTION_3 - 1) - offset; // 63..0

    // We now scale rampup and rampdown to a 0-255 range -- at least
    // in theory, but here's where architecture-specific decsions
    // come in to play:
    // To scale them up to 0-255, we'd want to multiply by 4.
    // But in the very next step, we multiply the ramps by other
    // values and then divide the resulting product by 256.
    // So which is faster?
    //   ((ramp * 4) * othervalue) / 256
    // or
    //   ((ramp    ) * othervalue) /  64
    // It depends on your processor architecture.
    // On 8-bit AVR, the "/ 256" is just a one-cycle register move,
    // but the "/ 64" might be a multicycle shift process. So on AVR
    // it's faster do multiply the ramp values by four, and then
    // divide by 256.
    // On ARM, the "/ 256" and "/ 64" are one cycle each, so it's
    // faster to NOT multiply the ramp values by four, and just to
    // divide the resulting product by 64 (instead of 256).
    // Moral of the story: trust your profiler, not your insticts.

    // Since there's an AVR assembly version elsewhere, we'll
    // assume what we're on an architecture where any number of
    // bit shifts has roughly the same cost, and we'll remove the
    // redundant math at the source level:

    //  // scale up to 255 range
    //  //rampup *= 4; // 0..252
    //  //rampdown *= 4; // 0..252

    // compute color-amplitude-scaled-down versions of rampup and rampdown
    uint8_t rampup_amp_adj   = (rampup   * color_amplitude) / (256 / 4);
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) / (256 / 4);

    // add brightness_floor offset to everything
    uint8_t rampup_adj_with_floor   = rampup_amp_adj   + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;


    if( section ) {
        if( section == 1) {
            // section 1: 0x40..0x7F
            rgb.r = brightness_floor;
            rgb.g = rampdown_adj_with_floor;
            rgb.b = rampup_adj_with_floor;
        } else {
            // section 2; 0x80..0xBF
            rgb.r = rampup_adj_with_floor;
            rgb.g = brightness_floor;
            rgb.b = rampdown_adj_with_floor;
        }
    } else {
        // section 0: 0x00..0x3F
        rgb.r = rampdown_adj_with_floor;
        rgb.g = rampup_adj_with_floor;
        rgb.b = brightness_floor;
    }
}


// from: https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
extern char *__brkval;
int freeMemory()
{
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}


/*
#define FASTLED_SCALE8_FIXED 1
typedef uint8_t   fract8;   ///< ANSI: unsigned short _Fract
///  scale one byte by a second one, which is treated as
///  the numerator of a fraction whose denominator is 256
///  In other words, it computes i * (scale / 256)
static inline uint8_t scale8( uint8_t i, fract8 scale)
{
#if (FASTLED_SCALE8_FIXED == 1)
    return (((uint16_t)i) * (1+(uint16_t)(scale))) >> 8;
#else
    return ((uint16_t)i * (uint16_t)(scale) ) >> 8;
#endif
}
/// linear interpolation between two unsigned 8-bit values,
/// with 8-bit fraction
static inline uint8_t lerp8by8( uint8_t a, uint8_t b, fract8 frac)
{
    uint8_t result;
    if( b > a) {
        uint8_t delta = b - a;
        uint8_t scaled = scale8( delta, frac);
        result = a + scaled;
    } else {
        uint8_t delta = a - b;
        uint8_t scaled = scale8( delta, frac);
        result = a - scaled;
    }
    return result;
}
*/
