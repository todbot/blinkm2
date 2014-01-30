

#include "TestClass.h"

#include "StructTest.h"

#include <avr/pgmspace.h>


TestClass<5> tester;

patternline_t pltmp; 
pattern_t ptmp;

void setup()
{
    tester.getSize();
    tester.setAt( 3, 100 );

    tester.setAt(3, 100);
    tester.setAt(4, 200);
    tester.setAt(5, 300);
    
    for( int i=0; i<tester.getSize(); i++ ) { 
        PORTA = tester.elementAt(i);
    }

    for(int i=0; i<patt_max; i++) { 
        pltmp = pattern_default[i];
        rgb_t colr = pltmp.color;
        PORTA = colr.r;
    }

    //pattern* p = patterns[0];
    memcpy_P( &ptmp, &patterns[0], sizeof(pattern_t) );
    int psize = ptmp.len;
    for( int i=0; i< psize; i++) { 
        memcpy_P( &pltmp, &ptmp.lines[i], sizeof(patternline_t) );
        rgb_t colr = pltmp.color;
    }

}

void loop()
{

}

