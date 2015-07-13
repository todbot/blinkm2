/*
 * BlinkM2_light
 *
 * 
 */

#include <avr/power.h>

#include "config.h"
#include "led_utils.h"

//#include "TinyWireS.h"
// be sure to modify TinyWireS/utility/usiTwiSlave.h
// and set TWI_RX_BUFFER_SIZE to 16

rgb_t leds[ NUM_LEDS ];
ledvector_t ledvectors[ NUM_LEDS ];
fader_t fader;

uint32_t led_update_next;
uint32_t play_update_next; // debug toy

rgb_t ctmp;
uint8_t ntmp;


rgb_t color_list[] = {
    { 55,  0,  0 },
    { 0,  55,  0 },
    { 0,   0, 55 },
    { 33, 33, 33 },
    { 0,   0,  0 }
};
int cntr;

/*
// unused
void init_timer()
{
    // set up periodic timer
    TCCR1A = 0;
    TCCR1B =  _BV( CS12 ) | _BV( CS11 ) | _BV( CS10 ); 
    TIFR1  |= _BV( TOV1 );
    TIMSK1 |= _BV( TOIE1 );
}
*/
/*
volatile byte interruptval;

ISR(PCINT0_vect)
{
    interruptval++;
}
*/

void dbgblink( uint8_t cnt, uint16_t hi, uint16_t lo ) {
    for( int i=0; i<cnt; i++ ) { 
        digitalWrite( dbgpin, HIGH );
        delay( hi );
        digitalWrite( dbgpin, LOW ) ;
        delay( lo );
    }
}
                                       
//
void setup() 
{
    /*
    // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
    #if defined (__AVR_ATtiny85__) ||  defined (__AVR_ATtiny84__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
    #endif
    // End of trinket special code
    */
#if defined(ARDUINO_AVR_UNO)
    Serial.begin(115200);
    dbgln("BlinkM2_light!");
#endif
    
    pinMode(dbgpin, OUTPUT);
    dbgblink( 6, 100, 100 );
              
    dbgln("setting All LEDs");
    //led_setBrightness(0);
    led_setAll( 0, 0, 155 );
    led_show();
    delay(1000);
    led_setAll( 0, 155, 0 );
    led_show();
    delay(1000);
    
    led_setAll( 0,0,0 );
    led_show();
    delay(1000);
    /*    
    dbgln("setting N LEDs");
    dbgblink( 1, 100, 300 );
    
    for( int i=0; i< 10; i++ ) {
        uint8_t n = i % NUM_LEDS;
        uint8_t r = random();
        uint8_t g = random();
        uint8_t b = random();
        //ctmp = { r,g,b };
        //ledfader_set_dest( &ctmp, 10, n );
        led_setN( n, r,g,b );
        led_show();
        delay(100);
    }
    */
    /*
#if 0
    // enable pin change interrupts on the inputs
    GIMSK = _BV( PCIE0 );  // enable pin-change interrupt
    PCMSK0 = _BV(PCINT0);  // PCINT0 = PA0
    PCMSK1 = _BV(PCINT8);  // PCINT8 = PB0
#endif
    */    
}


//
void loop()
{
    update_led_state();

    if( (long)(millis() - play_update_next) > 0 ) {
        play_update_next += 2000; // every 5 secs
        dbgln("new play light");
        rgb_t c = color_list[ cntr ];
        cntr++;
        if( cntr == 5 ) cntr = 0;
        ledfader_set_dest( &c, 1000, ntmp % NUM_LEDS+1 ) ;
        ntmp++ ;
        
        //ctmp = { 0, 0, 0 };
        //ledfader_set_dest( &ctmp, 10, 0 ); // go dark
        /*
        led_setN( 0, 0,0,0 );
        uint8_t r = random() % 128;
        uint8_t g = random() % 128;
        uint8_t b = random() % 128;
        ctmp = { r,g,b };
        //uint8_t n = random() % NUM_LEDS + 1;
        ntmp++ ;
        ledfader_set_dest( &ctmp, 500, ntmp % NUM_LEDS +1 );
        */
    }
}

//
void update_led_state()
{
    // update LEDs every led_update_millis
    if( (long)(millis() - led_update_next) > 0 ) {
        led_update_next += led_update_millis;
        int done = ledfader_update();

        dbg("fader: pos:"); dbg(fader.pos); dbg(", posinc:"); dbg(fader.posinc);
        dbg(", done:"); dbg( done ); dbg(" = ");
        dbg( fader.pos > (FADERPOS_MAX - fader.posinc) );
        //dbg(':');
        //bg(curc->r); dbg(','); dbg(curc->g); dbg(','); dbg(curc->b);
        dbgln('.');


        led_show();
    } // led_update_next
    
}
