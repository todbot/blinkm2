
#ifndef LED_UTILS_H
#define LED_UTILS_H

//
// functions stolen from FastLED 3.1
//

//  fract8   range is 0 to 0.99609375
//                 in steps of 0.00390625
typedef uint8_t   fract8;   // ANSI: unsigned short _Fract
//  fract16  range is 0 to 0.99998474121
//                 in steps of 0.00001525878
typedef uint16_t  fract16;  // ANSI: unsigned       _Fract
//  sfract15 range is -0.99996948242 to 0.99996948242
//                 in steps of 0.00003051757
typedef int16_t   sfract15; // ANSI: signed         _Fract

// RGB triplet of 8-bit vals for input/output use
struct CRGB {
	union {
		struct {
            union {
                uint8_t r;
                uint8_t red;
            };
            union {
                uint8_t g;
                uint8_t green;
            };
            union {
                uint8_t b;
                uint8_t blue;
            };
        };
		uint8_t raw[3];
	};
    // allow assignment from one RGB struct to another
	inline CRGB& operator= (const CRGB& rhs) __attribute__((always_inline))
    {
        r = rhs.r;
        g = rhs.g;
        b = rhs.b;
        return *this;
    }
};
inline __attribute__((always_inline)) bool operator== (const CRGB& lhs, const CRGB& rhs)
{
    return (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b);
}

inline __attribute__((always_inline)) bool operator!= (const CRGB& lhs, const CRGB& rhs)
{
    return !(lhs == rhs);
}
    
//
uint8_t scale8( uint8_t i, fract8 scale)
{
    return ((uint16_t)i * (uint16_t)(scale) ) >> 8;
}

uint16_t scale16( uint16_t i, fract16 scale )
{
    uint16_t result;
    result = ((uint32_t)(i) * (uint32_t)(scale)) / 65536;
    return result;
}

/*
CRGB& nblend( CRGB& existing, const CRGB& overlay, fract8 amountOfOverlay)
{
    if( amountOfOverlay == 0)
        return existing;
    
    if( amountOfOverlay == 255) {
        existing = overlay;
        return existing;
    }
    return existing;
}
*/
CRGB& nblend( CRGB& existing, const CRGB& overlay, fract8 amountOfOverlay )
{
    if( amountOfOverlay == 0)
        return existing;
    
    if( amountOfOverlay == 255) {
        existing = overlay;
        return existing;
    }

    fract8 amountOfKeep = 256 - amountOfOverlay;

    existing.red   = scale8( existing.red,   amountOfKeep) +
                     scale8( overlay.red,    amountOfOverlay);
    existing.green = scale8( existing.green, amountOfKeep) + 
                     scale8( overlay.green,  amountOfOverlay);
    existing.blue  = scale8( existing.blue,  amountOfKeep) +
                     scale8( overlay.blue,   amountOfOverlay);

    return existing;
}

CRGB blend( const CRGB& p1, const CRGB& p2, fract8 amountOfP2 )
{
    CRGB nu(p1);
    nblend( nu, p2, amountOfP2);
    return nu;
}





#endif
