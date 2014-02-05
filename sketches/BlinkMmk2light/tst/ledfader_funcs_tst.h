/*
 *
 *
 */

#ifndef LEDFADER_FUNCS_H
#define LEDFADER_FUNCS_H

// requirements before inclusion: 
// -- nLEDs  - number of RGB LEDs
// -- leds[] - array of rgb_t or CRGB structs, length nLEDs
// -- faders[] - array of fader struct, length nLEDs

// public functions
void ledfader_setCurr(rgb_t color, uint16_t ledn);
void ledfader_setDest(rgb_t color, int steps, uint16_t ledn);
void ledfader_update(void);

// private functions
void ledfader_setDestN(rgb_t newcolor, int steps, uint8_t ledn);
uint8_t ledfader_getNewVal(uint16_t stepcnt, int16_t m100x, 
                           uint8_t dest, uint8_t curr);


void ledfader_setCurr(rgb_t newcolor, uint16_t ledn)
{ 
    uint8_t i=0;

    if (ledn > 0) i=ledn-1;
    else       ledn=nLEDs;

    // either do all or just one
    for (; i < ledn; i++) {
        rgbfader_t* f = &faders[ledn];
        f->stepcnt = 0;
        f->dest.r = newcolor.r;
        f->dest.g = newcolor.g;
        f->dest.b = newcolor.b;

        leds[i].r = newcolor.r;
        leds[i].g = newcolor.g;
        leds[i].b = newcolor.b;
    }
}

void ledfader_setDestN(rgb_t newc, int steps, uint8_t ledn)
{ 
    rgbfader_t* f = &faders[ledn];
    f->stepcnt = steps + 1;
    f->dest.r = newc.r;
    f->dest.g = newc.g; 
    f->dest.b = newc.b;

    CRGB old = leds[ledn];

    f->m100x.r = 100* ((int)newc.r - old.r) / steps;
    f->m100x.g = 100* ((int)newc.g - old.g) / steps;
    f->m100x.b = 100* ((int)newc.b - old.b) / steps;
}

//
void ledfader_setDest(rgb_t newcolor, int steps, uint16_t ledn)
{
    if( ledn==0 ) { 
        for (uint8_t i=0; i < nLEDs; i++) {
            ledfader_setDestN( newcolor, steps, i);
        }  
    } 
    else { 
        uint8_t bitpos = 0;
        do { 
            uint8_t bit = (ledn>>bitpos) & 0x1;
            if( bit )  
                ledfader_setDestN( newcolor, steps, bitpos );
        } while( ++bitpos != nLEDs );
    }
}

__attribute__((always_inline)) inline 
uint8_t ledfader_getNewVal( uint16_t stepcnt, int16_t m100x, 
                            uint8_t dest, uint8_t curr) 
{
    int16_t dest100x = 100* dest;    // dest color at 100x
    // new color from slope, working backwards from dest
    int16_t tmpc = dest100x - (m100x*(stepcnt));
    return ((tmpc + 50) / 100); // de-scale, with rounding
}

//
void ledfader_update(void)
{
    for( uint8_t i=0; i<nLEDs; i++ ) {
        rgbfader_t* f = &faders[i];
        if( !f->stepcnt ) {
            continue;
        }

        f->stepcnt--;
        if( f->stepcnt ) {
            leds[i].r = ledfader_getNewVal(f->stepcnt, f->m100x.r, f->dest.r, leds[i].r);
            //leds[i].g = ledfader_getNewVal(f->stepcnt, f->m100x.g, f->dest.g, leds[i].g);
            //leds[i].b = ledfader_getNewVal(f->stepcnt, f->m100x.b, f->dest.b, leds[i].b);
        }
        else { 
            leds[i].r = f->dest.r;
            leds[i].g = f->dest.g;
            leds[i].b = f->dest.b;
        }
#if 1
        if( i==0 ) { 
            printf("update0: %3d, %3d, %3d, %2.2f\n", 
                   f->stepcnt, f->dest.r, leds[i].r, f->m100x.r/100.0);
        }
#endif
    }
}

#endif


/*
__attribute__((always_inline)) inline 
uint8_t ledfader_getNewVal( uint16_t stepcnt, int16_t m100x, 
                            uint8_t dest, uint8_t curr) 
{
    // dest color at 100x
    int16_t dest100x = 100* dest;
    // new color from slope, with rounding up (+50)
    //int16_t tmpc = (100* curr) + m100x + 0 ; 
    //err = -(tmpc - (dest100x - (m100x*(stepcnt-i))));
    int16_t tmpc = dest100x - (m100x*(stepcnt));
    //int16_t err = (tmpc - (dest100x - (m100x*(stepcnt))));
    int16_t err = 0;
    char bop = ' ';
#if 0    
    if((m100x > 0 && err>m100x) || (m100x < 0 && err<m100x) ) {
        bop = '*';
        tmpc -= err;
        //tmpc += err;
        //tmpc -= m100x;
    }
#endif
    //if( (m100x > 0 && err>m100x) ) { 
    //    tmpc -=m100x;
    //}

#if 0
    printf("\t\t\tnewval: m100x:%5d err:%5d tmpc:%5d %c\n",
           m100x, err, tmpc, bop);
#endif
    return ((tmpc + 50) / 100); // de-scale
}
*/
