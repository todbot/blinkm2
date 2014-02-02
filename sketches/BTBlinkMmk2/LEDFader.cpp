

#include "LEDFader.h"

#include <stdio.h>
#include <stdarg.h>
#include <Arduino.h>
// printf_wrapper
void pp(char *fmt, ... ) 
{
  char tmp[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt );
  vsnprintf(tmp, 128, fmt, args);
  va_end (args);
  Serial.print(tmp);
}

//
void LEDFader::setCurr(rgb_t* newcolor, uint16_t ledn)
{ 
    uint8_t i=0;

    if (ledn > 0) i=ledn-1;
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
void LEDFader::setDestN(rgb_t newc, int steps, uint8_t ledn)
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
    /*
    if( ledn==0 ) {
        p("ledn: %d dest: %d,%d,%d  old: %d,%d,%d  m100x:%d,%d,%d\n", 
          ledn, f->dest.r, f->dest.g, f->dest.b, 
          old.r, old.g, old.b,
          f->m100x.r, f->m100x.g, f->m100x.b);
    }
    */
}

//
void LEDFader::setDest(rgb_t newcolor, int steps, uint16_t ledn)
{
    pp("setDest: colr:%d,%d,%d\n", newcolor.r,newcolor.g,newcolor.b);
    if( ledn==0 ) { 
        for (uint8_t i=0; i < nLEDs; i++) {
            setDestN( newcolor, steps, i);
        }  
    } 
    else { 
        uint8_t bitpos = 0;
        do { 
            uint8_t bit = (ledn>>bitpos) & 0x1;
            //pp("ledn:%4x bitpos:%x bit:%x\n", ledn,bitpos,bit );
            if( bit )  
                setDestN( newcolor, steps, bitpos );
        } while( ++bitpos != nLEDs );
    }
}

//__attribute__((always_inline)) inline uint8_t LEDFader::getNewVal( uint16_t stepcnt, int16_t m100x, uint8_t dest, uint8_t curr) 
uint8_t LEDFader::getNewVal( uint16_t stepcnt, int16_t m100x, 
                             uint8_t dest, uint8_t curr) 
{
    int dest100x = 100* dest;    // dest color
    // new color from slope, with rounding up (+50)
    int tmpc = (100* curr) + m100x + 0 ; 
    //err = -(tmpc - (dest100x - (m100x*(stepcnt-i))));
    int err = -(tmpc - (dest100x - (m100x*(stepcnt))));
    
    if((m100x > 0 && err>m100x) || (m100x < 0 && err<m100x) ) {
        tmpc += err;        //tmpc += m100x;
    }
            
    return ((tmpc + 50) / 100); // de-scale
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
            /*
            if( i==0 ) {
                pp("%d:st:%3d m100x:%d dest:%d \t led: %d\n",
                   i, f->stepcnt, f->m100x.r, f->dest.r, leds[i].r);
            }
            */
            leds[i].r = getNewVal(f->stepcnt, f->m100x.r, f->dest.r, leds[i].r);
            leds[i].g = getNewVal(f->stepcnt, f->m100x.g, f->dest.g, leds[i].g);
            leds[i].b = getNewVal(f->stepcnt, f->m100x.b, f->dest.b, leds[i].b);

        }
        else { 
            /*
            pp("!!BOP!!");
            leds[i].r = f->dest.r;
            leds[i].g = f->dest.g;
            leds[i].b = f->dest.b;
            */
        }
    }
}

/*
#if 0
//
void LEDFader::update0(void)
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
                    //tmpc += m100x;
                    tmpc += err;
                }
                leds[i].raw[j] = (uint8_t)((tmpc + 50) / 100); // de-scale
                if( j==0 && i==5 ) {
                    //p("%d:st:%3d err:%4d curr:%3d dest100x:%d\tledraw:%d\n",
                    //  i, f->stepcnt, err, curr, dest100x, leds[i].raw[j]);
                }
            }
            
            //leds[i].r = f->dest.r;
            //leds[i].g = f->dest.g;
            //leds[i].b = f->dest.b;
        } else { 
            //p("!!BOP!!");
            leds[i].r = f->dest.r;
            leds[i].g = f->dest.g;
            leds[i].b = f->dest.b;
        }

    }
}
#endif
*/
