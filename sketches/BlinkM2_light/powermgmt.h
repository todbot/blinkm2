
#ifndef _POWERMGMT_H_
#define _POWERMGMT_H_

// Put the Arduino to sleep.
void sleep() {
  // Set sleep to full power down.  Only external interrupts or 
  // the watchdog timer can wake the CPU!
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // Turn off the ADC while asleep.
  power_adc_disable();

  // Enable sleep and enter sleep mode.
  sleep_mode();

  // CPU is now asleep and program execution completely halts!
  // Once awake, execution will resume at this point.
  
  // When awake, disable sleep mode and turn on all devices.
  sleep_disable();
  power_all_enable();
}

//
void watchdog_setup() {
    // This next section of code is timing critical, so interrupts are disabled.
    // See more details of how to change the watchdog in the ATmega328P datasheet
    // around page 50, Watchdog Timer.
    noInterrupts();
    // Set the watchdog reset bit in the MCU status register to 0.
    MCUSR &= ~(1<<WDRF);
    // Set WDCE and WDE bits in the watchdog control register.
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    // Set watchdog clock prescaler bits to a value of 8 seconds.
    WDTCSR = (1<<WDP0) | (1<<WDP3);
    // Enable watchdog as interrupt only (no reset).
    WDTCSR |= (1<<WDIE);
    // Enable interrupts again.
    interrupts();
}
//
ISR(WDT_vect)
{
    watchdogActivated = true;
}

// "ledenpin" is connected to p-mosfet control Vcc of LEDs
//
inline void ledpower_setup() {
    pinMode( ledenpin, OUTPUT );
}
//
inline void ledpower_enable() {
    digitalWrite( ledenpin, LOW );  // LOW to turn ON LEDS
}
//
inline void ledpower_disable() {
    digitalWrite( ledenpin, HIGH ); // HIGH to turn OFF LEDS
}

// 4-LED power consumption w/ 8MHz ATtiny84
//
// 8.5mA - LEDs + normal speed
// 5.3mA - LEDs off + normal speed
// 1.3mA - LEDs off + 128 speed => per-off-LED = 1mA
// 0.28mA - sleep mode
//
// notes:
// - using power_*_disable() features doesn't have big effect at mA scale


#endif
