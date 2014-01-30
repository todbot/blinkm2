

#include <stdio.h>
#include <string.h>

#include "TestClass.h"

#include "StructTest.h"



TestClass<10> tester;

pattern_t ptmp;
patternline_t pltmp; 


int main()
{
    printf("hello\n");

    printf("size: %d\n", tester.getSize() );

    printf("setting pos 3,4,5 to 100,200,300 \n");
    tester.setAt(3, 100);
    tester.setAt(4, 200);
    tester.setAt(5, 300);
    
    for( int i=0; i<tester.getSize(); i++ ) { 
        printf("%d : %d\n", i, tester.elementAt(i));
    }

    printf("--------------\n");
    for(int i=0; i<patt_max; i++) { 
        patternline_t* pl = &patternlines_default[i];
        rgb_t colr = pl->color;
        printf("%2d: r,g,b:%2.2x%2.2x%2.2x  %d\n", i, 
               colr.r,colr.g,colr.b, pl->ledn );
    }

    /*
    printf("--------------\n");
    const pattern_t* p = patterns[0];
    int psize = p->len;
    for( int i=0; i< psize; i++) { 
        patternline_t* pl = &p->lines[i];
        rgb_t colr = pl->color;
        printf("  %2d: r,g,b:%2.2x%2.2x%2.2x  %d\n", i, 
               colr.r,colr.g,colr.b, pl->ledn );
    }
    */

    int lnum = 1;
    memcpy( &ptmp, patterns[lnum], sizeof(pattern_t) );
    int psize = ptmp.len;
    printf("psize: %d\n",psize);
    for( int i=0; i< psize; i++) { 
        memcpy( &pltmp, &ptmp.lines[i], sizeof(patternline_t) );
        rgb_t colr = pltmp.color;
        printf("  %2d: r,g,b:%2.2x%2.2x%2.2x  %d\n", i, 
               colr.r,colr.g,colr.b, pltmp.ledn );
    }
    

}
