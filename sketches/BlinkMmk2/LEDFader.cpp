

#include "LEDFader.h"

//
void LEDFader::setCurr(rgb_t* newcolor, int ledn)
{ 
    uint8_t i=0;

    if (ledn > 0) i=ledn;
    else       ledn=nLEDs;
    
    // either do all or just one
    for (; i < ledn; i++) {
        rgbfader_t* f = &faders[ledn];
        f->stepcnt = 0;
        f->dest.r = newcolor->r;
        f->dest.g = newcolor->g;
        f->dest.b = newcolor->b;

        leds[i].r = newcolor->r;
        leds[i].g = newcolor->g;
        leds[i].b = newcolor->b;
    }
}

//
void LEDFader::setDestN(rgb_t* newcolor, int steps, int ledn)
{ 
    rgbfader_t* f = &faders[ledn];
    f->stepcnt = steps + 1;
    f->dest.r = newcolor->r; 
    f->dest.g = newcolor->g; 
    f->dest.b = newcolor->b;

    CRGB old = leds[ledn];

    f->m100x.r = 100* ((int)f->dest.r - old.r) / steps;
    f->m100x.g = 100* ((int)f->dest.g - old.g) / steps;
    f->m100x.b = 100* ((int)f->dest.b - old.b) / steps;
}

//
void LEDFader::setDest(rgb_t* newcolor, int steps, int ledn)
{
    if (ledn > 0) {
        setDestN( newcolor, steps, ledn-1);
    } else {
        for (uint8_t i=0; i < nLEDs; i++) {
            setDestN( newcolor, steps, i);
        }
    }
}

//
void LEDFader::update(void)
{
    int16_t m100x, dest100x, tmpc, err;
    uint8_t curr;

    for( uint8_t i=0; i<nLEDs; i++ ) {
        rgbfader_t* f = &faders[i];
        if( !f->stepcnt ) {
            continue;
        }

        f->stepcnt--;
        if( f->stepcnt ) {

            for( uint8_t j=0; j<3; j++) { // operate on each r,g,b indep.
                m100x = f->m100x.raw[j];    // slope
                dest100x = 100* f->dest.raw[j];      // dest color
                curr = leds[i].raw[j];  // curr color
                // new color from slope, with rounding up (+50)
                tmpc = (100* curr) + m100x + 0 ; 
                err = -(tmpc - (dest100x - (m100x*(f->stepcnt-i))));

                if((m100x > 0 && err>m100x) || (m100x < 0 && err<m100x) ) {
                    tmpc += m100x;
                }
                leds[i].raw[j] = (tmpc + 50) / 100; // de-scale
            }

        } else { 
            leds[i].r = f->dest.r;
            leds[i].g = f->dest.g;
            leds[i].b = f->dest.b;
        }

    }
}
