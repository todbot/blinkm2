

#include "Fader.h"

#include <stdio.h>

//
void Fader::setCurr(rgb_t* newcolor, int steps, int ledn)
{ 
    rgbfader_t* f = &faders[ledn];
    f->curr100x.r = newcolor->r * 100;
    f->curr100x.g = newcolor->g * 100;
    f->curr100x.b = newcolor->b * 100;
    
    f->dest.r = newcolor->r;
    f->dest.g = newcolor->g;
    f->dest.b = newcolor->b;

    f->stepcnt = 0;
}

//
void Fader::setDestN(rgb_t* newcolor, int steps, int ledn)
{ 
    rgbfader_t* f = &faders[ledn];
    f->dest.r = newcolor->r; 
    f->dest.g = newcolor->g; 
    f->dest.b = newcolor->b;
    
    f->stepcnt = steps + 1;

    /*
    // not a large enough range
    int16_t d;
    d = (f->dest.r*100 - f->curr100x.r) / steps;
    //if( d < 127 ) { d = 127;   // INT8_MAX
    //else if( d < -128 ) d = -128;  // INT8_MIN
    f->step100x.r = d;

    d = (f->dest.g*100 - f->curr100x.g) / steps;
    //if( d > 127 ) d = 127;   // INT8_MAX
    //else if( d < -128 ) d = -128;  // INT8_MIN
    f->step100x.g = d;

    d = (f->dest.b*100 - f->curr100x.b) / steps;
    //if( d > 127 ) d = 127;   // INT8_MAX
    //else if( d < -128 ) d = -128;  // INT8_MIN
    f->step100x.b = d;
    */

    // naive impl
    f->step100x.r = (f->dest.r*100 - f->curr100x.r) / steps;
    f->step100x.g = (f->dest.g*100 - f->curr100x.g) / steps;
    f->step100x.b = (f->dest.b*100 - f->curr100x.b) / steps;

    printf("setDestN:steps:%2d cur:%d %d %d -> dest:%d %d %d :step:%d %d %d\n",
           steps,
           f->curr100x.r, f->curr100x.g, f->curr100x.b,
           f->dest.r,    f->dest.g,    f->dest.b,
           f->step100x.r, f->step100x.g, f->step100x.b
           );
}

//
void Fader::setDest(rgb_t* newcolor, int steps, int ledn)
{
    if (ledn > 0) {
        setDestN(newcolor, steps, ledn - 1);
    } else {
        for (uint8_t i = 0; i < nLEDs; i++) {
            setDestN( newcolor, steps, i);
        }
    }
}


//
void Fader::update(void)
{
    for( uint8_t i=0; i<nLEDs; i++ ) {
        rgbfader_t* f = &faders[i];
        if( !f->stepcnt ) {
            continue;
        }

        printf("\tupdate:%4d  cur:%3d %3d %3d : dest:%3d %3d %3d\n", 
               f->stepcnt,
               f->curr100x.r/100, f->curr100x.g/100, f->curr100x.b/100,
               f->dest.r,     f->dest.g,     f->dest.b
               );

        f->stepcnt--;
        if( f->stepcnt ) {
            //if( f->step100x.r == 127 ) f->curr100x.r++;
            //else if( f->step100x.r == -128 ) f->curr100x.r--;
            //else 
            f->curr100x.r += f->step100x.r;
            f->curr100x.g += f->step100x.g;
            f->curr100x.b += f->step100x.b;
        } else {
            f->curr100x.r = f->dest.r * 100;
            f->curr100x.g = f->dest.g * 100;
            f->curr100x.b = f->dest.b * 100;
        }
        
        leds[i].r = f->curr100x.r / 100;
        leds[i].g = f->curr100x.g / 100;
        leds[i].b = f->curr100x.b / 100;

    }
}
