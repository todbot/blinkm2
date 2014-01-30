

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fader.h"

#define NUM_LEDS 14

rgb_t leds[NUM_LEDS];
rgbfader_t faders[NUM_LEDS];
Fader ledfader(leds, faders, NUM_LEDS ); 

void fadeBySteps(int, int,int);

int main()
{
    printf("hello\n");

    /*
    fadeBySteps(7, 100, 50);
    fadeBySteps(100, 7, 50);
    fadeBySteps(101, 3, 21);
    fadeBySteps(0, 101, 23);
    fadeBySteps(23, 231, 100);
    exit(0);
    */

    rgb_t ctmp; // = { 255,255,255 };
    int ttmp = 15;
    int ln = 5;
    int steps=10;

    ctmp.r=7; ctmp.g=100; ctmp.b=101;
    ledfader.setCurr( &ctmp, 2);
    ctmp.r=100; ctmp.g=7; ctmp.b=3;
    steps = 200;
    ledfader.setDest( &ctmp, steps, 2 );
    for( int i=0; i< steps+1; i++) {
        ledfader.update();
    }


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
