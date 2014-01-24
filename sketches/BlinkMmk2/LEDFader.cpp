


extern "C" {
//  #include <inttypes.h>
}

#include "LEDFader.h"


// Constructors ////////////////////////////////////////////////////////////////


LEDFader::LEDFader(int nnumleds, CRGB* nleds, rgbfader_t* nfaders)
{
    nLEDs  = nnumleds;
    leds   = nleds;
    faders = nfaders;
}

// Public Methods //////////////////////////////////////////////////////////////

//
//void LEDFader::setUpdateHandler( UpdateHandler ahandler )
//{
//    updateHandler = ahandler;
//}

//
void LEDFader::setCurr(CRGB* newcolor, int steps, int ledn)
{ 
    rgbfader_t* f = &faders[ledn];
    f->curr100x.r = newcolor->r * 100;
    f->curr100x.g = newcolor->g * 100;
    f->curr100x.b = newcolor->b * 100;
    
    f->dest100x.r = f->curr100x.r;
    f->dest100x.g = f->curr100x.g;
    f->dest100x.b = f->curr100x.b;
    f->stepcnt = 0;
}

//
void LEDFader::setDestN(CRGB* newcolor, int steps, int ledn)
{ 
    rgbfader_t* f = &faders[ledn];
    f->dest100x.r = newcolor->r * 100;
    f->dest100x.g = newcolor->g * 100;
    f->dest100x.b = newcolor->b * 100;
    
    f->stepcnt = steps + 1;
    
    f->step100x.r = (f->dest100x.r - f->curr100x.r) / steps;
    f->step100x.g = (f->dest100x.g - f->curr100x.g) / steps;
    f->step100x.b = (f->dest100x.b - f->curr100x.b) / steps;
}

//
void LEDFader::setDest(CRGB* newcolor, int steps, int ledn)
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
void LEDFader::update(void)
{
    for( uint8_t i=0; i<nLEDs; i++ ) {
        //rgbfader_t f = faders[i];
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
        
        leds[i].setRGB( f->curr100x.r/100, f->curr100x.g/100, f->curr100x.b/100 );
    }
}

