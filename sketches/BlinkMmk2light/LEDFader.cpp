

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
        f->dest100x.r = newcolor->r * 100;
        f->dest100x.g = newcolor->g * 100;
        f->dest100x.b = newcolor->b * 100;

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
    f->dest100x.r = newcolor->r * 100; 
    f->dest100x.g = newcolor->g * 100; 
    f->dest100x.b = newcolor->b * 100;

    CRGB old = leds[ledn];

    f->step100x.r = (f->dest100x.r - f->curr100x.r) / steps;
    f->step100x.g = (f->dest100x.r - f->curr100x.r) / steps;
    f->step100x.b = (f->dest100x.r - f->curr100x.r) / steps;
}

//
void LEDFader::setDest(rgb_t* newcolor, int steps, int ledn)
{
    if (ledn > 0) {
        setDestN(newcolor, steps, ledn-1);
    } else {
        for (uint8_t i=0; i < nLEDs; i++) {
            setDestN( newcolor, steps, i);
        }
    }
}

//
void LEDFader::update(void)
{
    for( uint8_t i=0; i<nLEDs; i++ ) {
        rgbfader_t* f = &faders[i];
        if( !f->stepcnt ) {
            continue;
        }

        f->stepcnt--;
        if( f->stepcnt ) {
            f->curr100x.r += f->step100x.r;
            f->curr100x.g += f->step100x.g;
            f->curr100x.b += f->step100x.b;
        } else {
            f->curr100x.r = f->dest100x.r;
            f->curr100x.g = f->dest100x.g;
            f->curr100x.b = f->dest100x.b;
        }
        
        leds[i].r = f->curr100x.r/100;
        leds[i].g = f->curr100x.r/100;
        leds[i].b = f->curr100x.r/100;
        //leds[i].setRGB(f->curr100x.r/100,f->curr100x.g/100,f->curr100x.b/100);
        //setLED( i, f->curr100x.r/100, f->curr100x.g/100, f->curr100x.b/100 );
    }
}

