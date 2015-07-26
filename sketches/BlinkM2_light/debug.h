#ifndef _DEBUG_H_
#define _DEBUG_H_

#if DEBUG 
  // can only use the below in main sketch,
  // not in C libraries
  #if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny84__)  //
    #define __BLINKM_BOARD__
    #define dbg(x)
    #define dbgln(x)
  #else
    // compile defines ARDUINO_AVR_UNO
    #define dbg(x) Serial.print(x)
    #define dbgln(x) Serial.println(x)
  #endif

#else // no debug

  #define dbg(x)
  #define dbgln(x)

#endif // DEBUG

// -----------------------------

#if DEBUG

uint16_t dbg_freeram () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

#define dbg_print_freeram() dbg(F("freeRam:")); dbgln( dbg_freeram() );

//
void dbgblink( uint8_t cnt, uint16_t hi, uint16_t lo )
{
    for( int i=0; i<cnt; i++ ) { 
        digitalWrite( dbgpin, HIGH );
        delay( hi );
        digitalWrite( dbgpin, LOW ) ;
        delay( lo );
    }
}

//
#define dbg_playstate( s, p ) {                             \
        dbg( s ); dbg(F(" playstate: id:")); dbg( p.id );   \
        dbg(F(", start:"));  dbg( p.start );                \
        dbg(F(", end:"));    dbg( p.end );                  \
        dbg(F(", cnt:"));    dbg( p.count );                \
        dbg(F(", pos:"));    dbg( p.pos );                  \
    }
//
void dbg_ledstate()
{
    dbgln(F("ledstate:"));
    for( int i=0; i<NUM_LEDS; i++ ) {
        dbg(i); dbg(F(": ")); 
        dbg(leds[i].r); dbg(','); dbg(leds[i].g); dbg(','); dbg(leds[i].b);
        dbg(F("\t:: "));
        dbg(ledvectors[i].dest.r); dbg(',');
        dbg(ledvectors[i].dest.g); dbg(',');
        dbg(ledvectors[i].dest.b); dbg("\t<--");
        dbg(ledvectors[i].last.r); dbg(',');
        dbg(ledvectors[i].last.g); dbg(',');
        dbg(ledvectors[i].last.b); dbgln();
    }
}
void dbg_setup()
{ 
    pinMode(dbgpin, OUTPUT);
    dbgblink( 6, 100, 100 );
}
//
void dbg_test1()
{ 
    dbgln("setting All LEDs");
    led_setAll( 0, 0, 155 );
    led_show();
    delay(300);
    led_setAll( 0, 155, 0 );
    led_show();
    delay(300);

    dbgln("set_brightness All LEDs");
    led_set_brightness(120);
    led_setAll( 0, 0, 155 );
    led_show();
    delay(300);
    led_setAll( 0, 155, 0 );
    led_show();
    delay(300);


    led_setAll( 0,0,0 );
    led_show();
    delay(500);
}

void dbg_pwrtst()
{
    delay(4000);
    ledpower_disable();

    delay(2000);
    clock_prescale_set(clock_div_128); // power reduce 4x
    delay(4000/128);
    clock_prescale_set(clock_div_1);
 
    watchdog_setup();
    
    sleep();
    while( 1 ) {
        if( watchdogActivated ) {
            //digitalWrite(dbgpin, HIGH); delay(100); digitalWrite(dbgpin,LOW);            
            watchdogActivated = false;
            // Increase the count of sleep iterations and take a sensor
            // reading once the max number of iterations has been hit.
            sleepIterations += 1;
            if (sleepIterations >= MAX_SLEEP_ITERATIONS) {
                // Reset the number of sleep iterations.
                sleepIterations = 0;
                dbgblink( 6, 50,50);
            }
        }
        sleep();
    }
    
    ledpower_enable();
    delay(2000);
}

#else

#define dbgblink( x,y,z )
#define dbg_playstate( s, p )
#define dbg_ledstate()
#define dbg_setup()
#define dbg_pwrtst()
#endif

#endif // _DEBUG_H_
