/**
 *
 * BlinkM2_Aug2020 - for use with "blinkm2neo" ATtiny85 board
 *
 * 2017-2020 @todbot / Tod E. Kurt
 *
 * Works on:
 *  - ATtiny85 on ATtinyCore  - https://github.com/SpenceKonde/ATTinyCore/
 *  - Arduino ATmega32U4 Pro Micro / Sparkfun Pro Micro 
 *
 * Libraries required:
 * - Light_WS2812_AVR - https://github.com/cpldcpu/light_ws2812
 * - TimerOne - https://github.com/PaulStoffregen/TimerOne (must use github version, not Library Manager)
 *
 * ATtiny85 static resource use:
 * - 20200731 : 4686 bytes flash / 295 bytes RAM  
 *
 * ATtiny85 configuration:  Arduino IDE 1.8.3 / ATTinyCore 1.4.0 
 * - Board: ATtiny85 (no bootloader)
 * - Chip: ATtiny85
 * - Clock: 8 MHz (internal)
 * - B.O.D. Level: (B.O.D Disabled)
 * - Save EEPROM: EEPROM retained
 * - Timer 1 Clock: CPU (CPU frequency)
 * - LTO: Enabled
 * - millis()/micros(): Enabled
 *
 */

#include "blinkm_config.h" // config for diff BlinkM / dev types

#include <Wire.h>          // from ATtinyCore / Arduino core
#include <TimerOne.h>      // TimerOne third-party library
#include <avr/eeprom.h>    // avr-libc core

extern "C"{
#include "light_ws2812.h"  // light_ws2812 third-party library
};

#include "Player.h"
#include "LedScripts.h"

// Pins defined blinkm_config.h
const int sclPin  = SCL_PIN;
const int misoPin = MISO_PIN;
const int sdaPin  = SDA_PIN;
const int ledPin  = LED_PIN;

uint8_t ee_i2c_addr         EEMEM; // = I2C_ADDR_DEFAULT;
uint8_t ee_boot_id          EEMEM; // = EEPROM_BOOT_ID;
uint8_t ee_boot_mode        EEMEM; // = 0x00; // FIXME: BOOT_PLAY_SCRIPT;
uint8_t ee_boot_script_id   EEMEM; // = 0x00;
uint8_t ee_boot_reps        EEMEM; // = 0x00;
uint8_t ee_boot_fadespeed   EEMEM; // = 0x07;
uint8_t ee_boot_timeadj     EEMEM; // = 0x00;
uint8_t ee_boot_brightness  EEMEM; // = 0x25;
uint8_t ee_unused2          EEMEM; // = 0xDA;

//script_line_t ee_script_lines[patt_max] EEMEM;
script_line_t EEMEM ee_script_lines[patt_max];

// NOTE: Declaring EEPROM statically doesn't do anyting because the Arduino
//        loader doesn't burn eeprom when flashing device
// IDEA: Check EEPROM id and load EEPROM from flash if not set  DONE
// IDEA: Can we "compress" the script into eeprom, decompress into RAM?

#define EEPROM_BOOT_ID 0xB1

// FIXME: make this an enum
// possible values for boot_mode
#define BOOT_NOTHING     0
#define BOOT_PLAY_SCRIPT 1
#define BOOT_MODE_END    2


uint8_t cmdargs[8];    // data read from I2C
uint8_t outvals[8];    // data sent to I2C
uint8_t outcount = 0;  // num bytes to send

// 
uint8_t inputs[2];
script_line_t linetmp;

// 
rgb_t leds[NUM_LEDS];

// Player does all the hard work
Player player(leds, NUM_LEDS);


// Arduino setup
void setup()
{
    dbg_start();

    pinMode(ledPin, OUTPUT);
    //pinMode(a0Pin, INPUT_PULLUP);
    //pinMode(a1Pin, INPUT_PULLUP);
    //pinMode(misoPin, INPUT_PULLUP);

#if defined(__BLINKM_DEV__)
    delay(2000);  // for debug
    dbgln("in setup!");
#endif
    
    uint8_t boot_id        = eeprom_read_byte( &ee_boot_id );
    dbg("EEPROM boot id: "); dbgln(boot_id);
    
    //if( 1 ) {
    if( boot_id != EEPROM_BOOT_ID ) { // is magic value valid?
        dbgln("*** Resetting EEPROM to defaults ***");
        eeprom_reset_vals();
    }
    eeprom_reset_script();
    
    // set up
    uint8_t i2c_addr       = eeprom_read_byte( &ee_i2c_addr ); 
    uint8_t boot_mode      = eeprom_read_byte( &ee_boot_mode );
    uint8_t boot_script_id = eeprom_read_byte( &ee_boot_script_id );
    uint8_t boot_reps      = eeprom_read_byte( &ee_boot_reps );
    uint8_t boot_fadespeed = eeprom_read_byte( &ee_boot_fadespeed );
    uint8_t boot_timeadj   = eeprom_read_byte( &ee_boot_timeadj );
    uint8_t boot_brightness= eeprom_read_byte( &ee_boot_brightness );
    
    // initialize i2c interface 
    if( i2c_addr==0 || i2c_addr>0x7f) {
        i2c_addr = I2C_ADDR_DEFAULT;
    }

    i2cSetup( i2c_addr );
    
    // initialize Timer1 periodic timer for player
    Timer1.initialize(10000); // every 10 millis
    Timer1.attachInterrupt(updatePlayer);

    player.setFadespeed(boot_fadespeed);
    player.setInputs(inputs);
    player.setBrightness(boot_brightness);
    
    if( boot_mode == BOOT_PLAY_SCRIPT ) {
        player.playScript( boot_script_id, boot_reps, 0 ); // FIXME: do real startup behavior
    }
    
    dbgln("setup done");
}

void i2cSetup(uint8_t addr)
{
    Wire.end();
    Wire.begin(addr);
    Wire.onReceive(i2cReceiveEvent);
    Wire.onRequest(i2cRequestEvent);    
}

// read data sent from Master (Controller)
void i2cReceiveEvent(int howMany)
{
    dbg("i2cReceiveEvent:"); dbgln(howMany);
    //if( howMany == 0 ) return;
    cmdargs[0] = 0; // zero out, just in case
    for( int i=0; i<howMany; i++ ) {
        cmdargs[i] = Wire.read();
    }
    handleI2C();
}

// send data back to Master (Controller) when requested
void i2cRequestEvent()
{
    for( int i=0; i<outcount; i++ ) { 
        Wire.write(outvals[i]);
    }
}

// called by i2cReceiveEvent
void handleI2C()
{
    int t;
    dbg("handleI2c:");
    dbg("  cmd:"); dbg((char)cmdargs[0]); dbg(" args:");
    dbg(cmdargs[1]); dbg(","); dbg(cmdargs[2]); dbg(","); dbg(cmdargs[3]);
    dbgln();
    
    outcount = 0;
    switch(cmdargs[0]) {
    case('o'):               // stop playback
        player.stop();
        break;
    case('b'):               // set brightness
    case('f'):               // set fade speed
    case('t'):               // set time adjust
    case('.'):               // set ledn
        // commands with 1 arg
        player.handleCmd(cmdargs[0], cmdargs[1], 0,0);
        break;
    case('n'):
    case('c'):
    case('C'):
    case('h'):
    case('H'):
    case('p'):
        // commands with 3 args
        player.handleCmd(cmdargs[0], cmdargs[1], cmdargs[2], cmdargs[3]); 
        break;
    case('g'):                // get current RGB color
        // no args, returns 3 vals
        outvals[0] = leds[0].r;
        outvals[1] = leds[0].g;
        outvals[2] = leds[0].b;
        outcount = 3;
        break;
    case('a'):                // get i2c address
        // no args, returns 1 val
        outvals[0] = eeprom_read_byte(&ee_i2c_addr);
        outcount = 1;
        break;
    case('A'):                // set i2c address
        // command with 4 args, changes I2C address
        // FIXME
        if( cmdargs[1] != 0 && cmdargs[1] == cmdargs[4] &&
            cmdargs[2] == 0xD0 && cmdargs[3] == 0x0D ) {  //
            eeprom_write_byte( &ee_i2c_addr, cmdargs[1] ); // write address
            i2cSetup( cmdargs[1] );
        }
        break;
    case('Z'):                // return protocol version
        // no args, returns 2 vals
        outvals[0] = BLINKM_PROTOCOL_VERSION_MAJOR;
        outvals[1] = BLINKM_PROTOCOL_VERSION_MINOR;
        outcount = 2;
        break;
    case('R'):                // read a script line, outputs 5: dur,cmd,arg1,arg2,arg3
        // command with 2 args (scriptid, pos), returns 5 vals
        if(cmdargs[1] == 0 ) { // eeprom script
            //eeprom_read_block( &line, (void*)ee_script_lines+0, sizeof(script_line_t) );
            eeprom_read_block( &linetmp, (const void*)(&ee_script_lines[cmdargs[2]])  ,
                               sizeof(script_line_t));
            dbgln(linetmp.cmd);
            outvals[0] = linetmp.dur;
            outvals[1] = linetmp.cmd;
            outvals[2] = linetmp.args[0];
            outvals[3] = linetmp.args[1];
            outvals[4] = linetmp.args[2];
            outcount = 5;
        }
        break;
    case('W'):
        break;
        
    case('B'):                 // set boot params: mode, script_id, reps, fadesp, timeadj
        //readI2CVals(5);
            
            /* eeprom_write_block( &(args[0]), // data starts from index 0 */
            /*                     &(ee_boot_mode), */
            /*                     5); */
        break;
        
    case('@'):                 // return free memory
        t = freeMemory();
        outvals[0] = (t >> 0) & 0xff;
        outvals[1] = (t >> 8) & 0xff;
        outcount = 2;
        break;
        
    default:
        break;
    }
}

// Arduino loop
void loop()
{
    // I2C checked with Wire events
    // player state updated by Timer1 event
    
    checkInputs();
    updateStrip();
}

// Called by Timer1 interrupt event
void updatePlayer()
{
    player.update();
}

// Called frequently to update LEDs FIXME
void updateStrip()
{
    ws2812_setleds((struct cRGB*)leds,NUM_LEDS);
}

//
void checkInputs()
{
    // inputs[0] = analogRead(a0Pin);
    // inputs[1] = analogRead(a1Pin);
    // inputs[2] = (digitalRead(misoPin) == HIGH) ? 255 : 0;
    // inputs[3] = (digitalRead(sdaPin) == HIGH) ? 255 : 0;
    // // FIXME: verify doesn't mess up i2c
}


//
// FIXME: magic values below
//
void eeprom_reset_vals()
{
    eeprom_write_byte( &ee_i2c_addr,        I2C_ADDR_DEFAULT ); // write address
    eeprom_write_byte( &ee_boot_id,         EEPROM_BOOT_ID );
    eeprom_write_byte( &ee_boot_mode,       BOOT_PLAY_SCRIPT );
    eeprom_write_byte( &ee_boot_script_id,  0 ); //17; // FIXME TESTING
    eeprom_write_byte( &ee_boot_reps,       0 );
    eeprom_write_byte( &ee_boot_fadespeed,  6 ); //8;
    eeprom_write_byte( &ee_boot_timeadj,    0 );
    eeprom_write_byte( &ee_boot_brightness, 255); //25; // 255
}

//
//
//
void eeprom_reset_script()
{
    uint8_t scriptId = 18; // FIXME
    uint8_t script_len = pgm_read_byte( &script_lengths[scriptId-1] );
    dbg("*** eeprom_reset_script: ");
    dbg("scriptId: "); dbg(scriptId); dbg(" script_len:"); dbgln(script_len);
    
    const script_line_t* sl = pgm_read_ptr( &scripts[scriptId-1] );
    for( int i = 0; i< script_len; i++ ) { 
        // read script_line to RAM
        memcpy_P( &linetmp, sl+i, sizeof(script_line_t));
        // write RAM to EEPROM
        eeprom_write_block( &linetmp, &ee_script_lines[i], sizeof(script_line_t) );

        dbg("  dur,cmd,args: "); dbg(linetmp.dur); dbg(','); dbg((char)linetmp.cmd); dbg(',');
        dbg(linetmp.args[0]); dbg(','); dbg(linetmp.args[1]); dbg(','); dbgln(linetmp.args[2]);
    }

    memset( &linetmp, 0, sizeof(script_line_t) );
    for( int i=script_len; i<patt_max; i++ ) {
        eeprom_write_block( &linetmp, &ee_script_lines[i], sizeof(script_line_t) );
        dbg("  dur,cmd,args: "); dbg(linetmp.dur); dbg(','); dbg(linetmp.cmd); dbg(',');
        dbg(linetmp.args[0]); dbg(','); dbg(linetmp.args[1]); dbg(','); dbgln(linetmp.args[2]);
    }

}

/*
//
//
//
void eeprom_reset_script_bad()
{
    dbgln("*** eeprom_reset_script *** ");
    uint8_t scriptId = 18; // FIXME
    uint8_t script_len = pgm_read_byte( &script_lengths[scriptId-1] );
    dbg("script_len:"); dbgln(script_len);
    
    #define sr linetmp
    script_line_t* sl = pgm_read_ptr( &scripts[scriptId-1] );
    // first get pointer to scriptline set
    dbg("sl: dur,cmd,a0: "); dbg(sl->dur); dbg(','); dbg(sl->cmd); dbg(','); dbgln(sl->args[0]);
    for( int i = 0; i< script_len; i++ ) { 

        //memcpy_P( &linetmp, sl+i, sizeof(script_line_t));

        //dbg("sr: dur,cmd,args: "); dbg(sr.dur); dbg(','); dbg(sr.cmd); dbg(',');
        //dbg(sr.args[0]); dbg(','); dbg(sr.args[1]); dbg(','); dbg(sr.args[2]); dbgln();
        
        eeprom_write_block( sl+i, &ee_script_lines[i], sizeof(script_line_t) );
    }

}
*/
