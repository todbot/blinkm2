/*
 *
 *
 */

#ifndef LEDFADER_FUNCS_H
#define LEDFADER_FUNCS_H

// requirements before inclusion: 
// -- nLEDs  - number of RGB LEDs
// -- leds[] - array of rgb_t or CRGB structs, of length nLEDs
// -- faders[] - array of rgbfader_t structs, of length nLEDs

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

//inline
__attribute__((always_inline)) inline 
void ledfader_setDestN(rgb_t newc, int steps, uint8_t ledn)
{ 
    rgbfader_t* f = &faders[ledn];
    faders[ledn].stepcnt = steps + 1;
    f->dest.r = newc.r;
    f->dest.g = newc.g; 
    f->dest.b = newc.b;

    CRGB old = leds[ledn];

    f->m100x.r = 100* ((int16_t)newc.r - old.r) / steps;
    f->m100x.g = 100* ((int16_t)newc.g - old.g) / steps;
    f->m100x.b = 100* ((int16_t)newc.b - old.b) / steps;
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

/*
//__attribute__((always_inline)) inline 
uint8_t ledfader_getNewVal( uint16_t stepcnt, int16_t m100x, 
                            uint8_t dest, uint8_t curr) 
{
    int16_t dest100x = 100* dest;    // dest color at 100x
    // new color from slope, working backwards from dest
    int16_t tmpc = dest100x - (m100x*(stepcnt));
    return ((tmpc + 50) / 100); // de-scale, with rounding
}
*/

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
            tmpc = (100* (int16_t)f->dest.r) - (f->m100x.r * (f->stepcnt));
            tmpc = (tmpc + 50) / 100;
            leds[i].r = tmpc;

            tmpc = (100* (int16_t)f->dest.g) - (f->m100x.g * (f->stepcnt));
            tmpc = (tmpc + 50) / 100;
            leds[i].g = tmpc;

            tmpc = (100* (int16_t)f->dest.b) - (f->m100x.b * (f->stepcnt));
            tmpc = (tmpc + 50) / 100;
            leds[i].b = tmpc;
        }
        else { 
            leds[i].r = f->dest.r;
            leds[i].g = f->dest.g;
            leds[i].b = f->dest.b;
        }
#if 0
        if( i==0 ) {
            dbg_tx_strP(PSTR("up: "));
            dbg_tx_hex(f->stepcnt);
            dbg_tx_putc(', cur:');
            dbg_tx_hex(leds[i].r);
            dbg_tx_putc('\n');
        }
#endif
    }
}

/*
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
            leds[i].g = ledfader_getNewVal(f->stepcnt, f->m100x.g, f->dest.g, leds[i].g);
            leds[i].b = ledfader_getNewVal(f->stepcnt, f->m100x.b, f->dest.b, leds[i].b);
        }
        else { 
            leds[i].r = f->dest.r;
            leds[i].g = f->dest.g;
            leds[i].b = f->dest.b;
        }
#if 1
            if( i==0 ) {
                dbg_tx_strP(PSTR("up: "));
                dbg_tx_hex(f->stepcnt);
                dbg_tx_putc(', cur:');
                dbg_tx_hex(leds[i].r);
                dbg_tx_putc('\n');
            }
#endif
    }
}
*/



#endif
