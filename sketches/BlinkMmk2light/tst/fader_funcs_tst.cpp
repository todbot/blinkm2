

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "blinkm_types.h"

#define nLEDs 1
rgb_t leds[nLEDs];
rgbfader_t faders[nLEDs];

#include "ledfader_funcs_tst.h"

/*
patt_line_t foo[]  = {
    { 'c', { 0x33, 0x44, 0x55 }, 100, 1 },
};
patt_line_t patt_lines_blink_white[] = {
    //{ .cmd='c', { .r=0x33, .g=0x33, .b=0x33 },  50, 0 }, // 0  white all
    { 'c', {{ 0x00, 0x00, 0x00 }},  50, 0 }, // 1  off all
};
*/
/*
rgb_t myc = { 
    .r = 1, .g = 2, .b =3,
};
patt_line_t poop = { 
    .cmd = 'c', .color = {3,4,5}, .dmillis  = 1, .ledn=0 
};
*/
int ease8InOutCubic(int x) { 
    int i = 1;
    return 3*(x^i) - 2*(x^3);
}

uint8_t ease8InOutApprox( uint8_t i)
{
    if( i < 64) {
        // start with slope 0.5
        i /= 2;
    } else if( i > (255 - 64)) {
        // end with slope 0.5
        i = 255 - i;
        i /= 2;
        i = 255 - i;
    } else {
        // in the middle, use slope 192/128 = 1.5
        i -= 64;
        i += (i / 2);
        i += 32;
    }
    
    return i;
}

int main()
{
    printf("fader_funcs_tst\n");

    int led_start = 30;
    int led_end   = 60;
    int led_range = led_end - led_start;
    for( int i = 0; i< led_range; i++ ) { 
        int f = (256 * i) / led_range;  // scale range to 0-255
        int eased = ease8InOutApprox( f ) ; // get eased version
        int p = led_start + (eased * led_range)/256;
        printf("%d : %d : %d : %d\n", i, f, eased, p);
    }

    /*
    for( int p=ledpos_start; p < ledpos_end; p++ ) {
        int i = 256 * (p - ledpos_start)/ledpos_end;
        int easedledpos = p *  ease8InOutCubic( i ) / 256;
        printf("%d : %d : %d\n", i, p, easedledpos );
    }
    */
    exit(0);

    rgb_t ctmp;
    for( int i=0; i< 255; i++ ) {
        ctmp.h = i; ctmp.s=255; ctmp.v = 255;
        hsvToRgb( &ctmp );
        printf("%d: %d,%d,%d\n", i, ctmp.r, ctmp.g, ctmp.b );
    }

#if 0
    rgb_t ctmp; // = { 255,255,255 };
    //int ttmp = 15;
    //int ln = 5;
    int steps=10;

    ctmp.r=7; ctmp.g=100; ctmp.b=101;
    ledfader_setCurr( ctmp, 2);
    ctmp.r=203; ctmp.g=0; ctmp.b=0;
    steps = 1000;
    ledfader_setDest( ctmp, steps, 0 );
    for( int i=0; i< steps+1; i++) {
        ledfader_update();
    }
#endif

    /*
    ctmp.r=25; ctmp.g=250; ctmp.b=55;
    ledfader.setDest( &ctmp, 50, ln );

    for( int i=0; i< 55; i++) {
        ledfader.update();
    }

    ctmp.r=125; ctmp.g=2; ctmp.b=255;
    ledfader.setDest( &ctmp, 20, ln );

    for( int i=0; i< 25; i++) {
        ledfader.update();
    }

    ctmp.r=255; ctmp.g=200; ctmp.b=0;
    ledfader.setDest( &ctmp, 1000, ln );

    for( int i=0; i< 1010; i++) {
        ledfader.update();
    }
    */
}


// takes as setup: oldColor, newColor, steps
// curColor is current
void fadeBySteps(int oldColor, int newColor, int steps)
{
    printf("\nfadeBySteps: %d -> %d : %d\n", oldColor, newColor, steps);

    int curColor;

    // fadeBySteps setup
    float slope = ((float)newColor-oldColor)/steps;
    float err=0;
    curColor = oldColor;

    for( int i=0; i< steps; i++ ) { 
        //float tmpColor = (float) curColor + slope;
        float tmpColor = (float) curColor + slope + 0.5f; // 0.5 for rounding
        // state needed:
        //  newColor (1), slope (2), steps (2)
        //if( slope >= 0 ) 
            err = tmpColor - (newColor - (slope*(steps-i-1)));
        //else 
            //err = tmpColor - (-(slope*(steps-i-1)) + newColor);
        int flag=0;
        // hmmm, this is pretty basic error accumulation 
        //if( err >  slope ) { tmpColor -= slope; flag =  1; }
        //if( err < -slope ) { tmpColor += slope; flag = -1; }
        if ( (slope > 0 && err > slope) || (slope < 0 && err < slope) ) {
            tmpColor -= (err/slope)*slope; 
            flag = (err<slope) ? -(err/slope): (err/slope);
        }
        //curColor = tmpColor + 0.5f; // 0.5 for rounding
        curColor = tmpColor;
        printf("%3d: slope:%+3.2f cur:%3d err:%+3.2f   %3d\n",
               i, slope, curColor, err, flag);
    }
    curColor = newColor; // fixme: make err use newColor as 
}
