

#include "Fader.h"


//
void Fader::setCurr(rgb_t* newcolor, int ledn)
{ 
    uint8_t i=0;

    if (ledn > 0)  i=ledn-1;
    else        ledn=nLEDs;
 
    // either do all or just one
    for(; i < ledn; i++) {
        rgbfader_t* f = &faders[i];
        f->dest.r = newcolor->r;
        f->dest.g = newcolor->g;
        f->dest.b = newcolor->b;

        f->stepcnt = 0;

        leds[i].r = newcolor->r;
        leds[i].g = newcolor->g;
        leds[i].b = newcolor->b;
    }
}

//
void Fader::setDestN(rgb_t* newcolor, int steps, int ledn)
{ 
    rgbfader_t* f = &faders[ledn];
    f->stepcnt = steps + 1;

    f->dest.r = newcolor->r; 
    f->dest.g = newcolor->g; 
    f->dest.b = newcolor->b;

    rgb_t old = leds[ledn];

    f->m100x.r = 100* ((int)f->dest.r - old.r) / steps;
    f->m100x.g = 100* ((int)f->dest.g - old.g) / steps;
    f->m100x.b = 100* ((int)f->dest.b - old.b) / steps;

    printf("setDestN:%d:st:%2d cur:%d %d %d -> dest:%d %d %d :slope:%2.2f %2.2f %2.2f\n",
           ledn, steps,
           old.r,      old.g,      old.b,
           f->dest.r,  f->dest.g,  f->dest.b,
           f->m100x.r/100.0, f->m100x.g/100.0, f->m100x.b/100.0
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
        
        rgbint_t errs;
        rgbint_t flags;
        f->stepcnt--;
        if( f->stepcnt ) {
            for( uint8_t j=0; j<3; j++) { // operate on each r,g,b indep.
                int m100x = f->m100x.raw[j];    // slope
                int dest100x = 100* f->dest.raw[j];      // dest color
                uint8_t curr = leds[i].raw[j];  // curr color
                // new color from slope, with rounding up (+50)
                int tmpc = (100* curr) + m100x + 0 ; 
                int err = -(tmpc - (dest100x - (m100x*(f->stepcnt-i))));
                int flag = 0;

                //printf("m100x:%d err:%d tmpc:%d", m100x, err, tmpc);
                if((m100x > 0 && err>m100x) || (m100x < 0 && err<m100x) ) {
                    tmpc += m100x;
                    flag = (err<m100x) ? -(err/m100x): (err/m100x);
                    //printf(" --> %d",tmpc);
                    //tmpc -= (err/m100x)*m100x;
                }
                //printf("\n");
                errs.raw[j] = err;
                flags.raw[j] = flag;
                leds[i].raw[j] = (tmpc + 50) / 100; // de-scale
            }
        } else { 
            leds[i].r = f->dest.r;
            leds[i].g = f->dest.g;
            leds[i].b = f->dest.b;
        }
        
        printf("\tupdate:%4d cur:%3d,%3d,%3d : dest:%d,%d,%d : err:%3.2f %3.2f %3.2f: %d,%d,%d\n\n", 
               f->stepcnt,
               leds[i].r,  leds[i].g,  leds[i].b,
               f->dest.r,  f->dest.g,  f->dest.b,
               errs.r/100.0, errs.g/100.0, errs.b/100.0,
               flags.r,    flags.g,    flags.b
               );

        //leds[i].r = f->curr100x.r / 100;
        //leds[i].g = f->curr100x.g / 100;
        //leds[i].b = f->curr100x.b / 100;

    }
}
