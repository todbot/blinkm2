#ifndef PLAYERTYPES_H
#define PLAYERTYPES_H

#include <stdint.h>

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))


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
    // allow equality checking
    inline bool operator==(const rgb_t& rhs) __attribute__((always_inline))
    {
       return r == rhs.r && g == rhs.g && b == rhs.b;
    }
    // allow inequality checking
    inline bool operator!=(const rgb_t& rhs) __attribute__((always_inline))
    {
       return r != rhs.r || g != rhs.g || b != rhs.b;
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


// don't need this anymore?
#define rgb(ar,ag,ab) { .r=ar, .g=ag, .b=ab }

typedef void(*callback_t)(void);


#endif
