#ifndef BLINKMTYPES_H
#define BLINKMTYPES_H

#include <stdint.h>

      
//__attribute__ ((always_inline)) static inline uint8_t colorSlide( uint8_t curr, uint8_t dest, uint8_t step )
static uint8_t colorSlide( uint8_t curr, uint8_t dest, uint8_t step )
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
static void fl_nscale8x3_video( uint8_t& r, uint8_t& g, uint8_t& b, uint8_t scale)
{
    uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    r = (r == 0) ? 0 : (((int)r * (int)(scale) ) >> 8) + nonzeroscale;
    g = (g == 0) ? 0 : (((int)g * (int)(scale) ) >> 8) + nonzeroscale;
    b = (b == 0) ? 0 : (((int)b * (int)(scale) ) >> 8) + nonzeroscale;
}

/// Representation of an HSV pixel (hue, saturation, value (aka brightness)).
struct hsv_t {
    union {
		struct {
		    union {
		        uint8_t hue;
		        uint8_t h; };
		    union {
		        uint8_t saturation;
		        uint8_t sat;
		        uint8_t s; };
		    union {
		        uint8_t value;
		        uint8_t val;
		        uint8_t v; };
		};
		uint8_t raw[3];
	};

    /// default values are UNITIALIZED
    inline hsv_t() __attribute__((always_inline))
    {
    }

    /// allow construction from H, S, V
    inline hsv_t( uint8_t ih, uint8_t is, uint8_t iv) __attribute__((always_inline))
        : h(ih), s(is), v(iv)
    {
    }

    /// allow copy construction
    inline hsv_t(const hsv_t& rhs) __attribute__((always_inline))
    {
        h = rhs.h;
        s = rhs.s;
        v = rhs.v;
    }
};


// RGB triplet of 8-bit vals for input/output use
struct rgb_t {
    union {
        struct {
            uint8_t g;  // must be in GRB format for light_ws2812 library
            uint8_t r;
            uint8_t b;
        };
        uint8_t raw[3];
    };
    // default values are UNINITIALIZED
    inline rgb_t() __attribute__((always_inline))
    {
    }
    // allow construction from R, G, B
    inline rgb_t( uint8_t ir, uint8_t ig, uint8_t ib)  __attribute__((always_inline))
        : g(ig), r(ir), b(ib)
    {
    }
    /// allow construction from HSV color
	inline rgb_t(const hsv_t& rhs) __attribute__((always_inline))
    {
        //hsv2rgb_rainbow( rhs, *this);
    }

    // allow copy construction
	inline rgb_t(const rgb_t& rhs) __attribute__((always_inline))
    {
        r = rhs.r;
        g = rhs.g;
        b = rhs.b;
    }
    // allow assignment from one RGB struct to another
    inline rgb_t& operator= (const rgb_t& rhs) __attribute__((always_inline))
    {
        r = rhs.r;
        g = rhs.g;
        b = rhs.b;
        return *this;
    }
    
        
    /* // allow equality checking */
    /* inline bool operator==(const rgb_t& rhs) __attribute__((always_inline)) */
    /* { */
    /*    return r == rhs.r && g == rhs.g && b == rhs.b; */
    /* } */
    /* // allow inequality checking */
    /* inline bool operator!=(const rgb_t& rhs) __attribute__((always_inline)) */
    /* { */
    /*      return r != rhs.r || g != rhs.g || b != rhs.b; */
    /* } */

    /// (from FastLED)
    /// scale down a RGB to N 256ths of it's current brightness, using
    /// 'video' dimming rules, which means that unless the scale factor is ZERO
    /// each channel is guaranteed NOT to dim down to zero.  If it's already
    /// nonzero, it'll stay nonzero, even if that means the hue shifts a little
    /// at low brightness levels.
    // inline rgb_t& nscale8_video (uint8_t scaledown )
    // {
    //     nscale8x3_video( r, g, b, scaledown);
    //     return *this;
    // }
    inline rgb_t& dim(uint8_t scale)
    {
        fl_nscale8x3_video(r,g,b, scale);
        return *this;
    }
    
    inline rgb_t& slideTowards(const rgb_t& rhs, const uint8_t step) __attribute__((always_inline))
    {
         r = colorSlide(r, rhs.r, step);
         g = colorSlide(g, rhs.g, step);
         b = colorSlide(b, rhs.b, step);
    }
};

struct script_line_t {
    uint8_t dur; // in ticks of 1/30sec
    struct {
        uint8_t cmd;
        union {
            struct { uint8_t r; uint8_t g; uint8_t b; }; // as color triplet
            uint8_t args[3];  // as unsigned array
            struct { int8_t a0; int8_t a1; int8_t a2; }; // if arbitrary args
            int8_t iargs[3];  // as signed array    FIXME all this is overkill
        };
    };
};

inline __attribute__((always_inline)) bool operator== (const rgb_t& lhs, const rgb_t& rhs)
{
    return (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b);
}

inline __attribute__((always_inline)) bool operator!= (const rgb_t& lhs, const rgb_t& rhs)
{
    return !(lhs == rhs);
}
    


// don't need this anymore?
#define rgb(ar,ag,ab) { .r=ar, .g=ag, .b=ab }

// create a callback_t type
typedef void(* callback_t )(void);


#endif
