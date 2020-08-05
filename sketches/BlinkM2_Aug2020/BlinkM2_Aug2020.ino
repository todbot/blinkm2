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
 * ATtiny85 static resource use:
 * - 20200731: 4686 bytes flash / 295 bytes RAM - basic functionalty, no eeprom
 * - 20200804: 5730 bytes flash / 304 bytes RAM - eeprom read/write, some rearch
 * - 20200804: 5712 bytes flash / 304 bytes RAM - eeprom config struct
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

blinkm_config EEMEM ee_config;

//script_line_t ee_script_lines[patt_max] EEMEM;
script_line_t EEMEM ee_script_lines[patt_max];

// NOTE: Declaring EEPROM statically doesn't do anyting because the Arduino
//        loader doesn't burn eeprom when flashing device

// This ID value must match ee_config.boot_id or EEPROM is invalid and will be reloaded
#define EEPROM_BOOT_ID 0xB2

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

// The buffer that is written to the LEDs
rgb_t leds[NUM_LEDS];  

// Player does all the hard work
Player player(leds, NUM_LEDS);


// Arduino setup
void setup()
{
    dbg_start();
#if defined(__BLINKM_DEV__)
    delay(1000);
    dbgln("BlinkM2!");
#endif
    
    pinMode(ledPin, OUTPUT);
    //pinMode(a0Pin, INPUT_PULLUP);
    //pinMode(a1Pin, INPUT_PULLUP);
    //pinMode(misoPin, INPUT_PULLUP);
   
    blinkm_config config;
    eeprom_load_config(&config);

    if( config.boot_id != EEPROM_BOOT_ID ) { // is magic value valid?
        dbgln("*** Resetting EEPROM to defaults ***");
        eeprom_reset_config();
        eeprom_reset_script();
    }
    
    // initialize i2c interface, ensuring good addr
    if( config.i2c_addr==0 || config.i2c_addr>0x7f) {
        config.i2c_addr = I2C_ADDR_DEFAULT;
    }

    i2cSetup( config.i2c_addr );
    
    // initialize Timer1 periodic timer for player
    Timer1.initialize(10000); // every 10 millis
    Timer1.attachInterrupt(updatePlayer);

    // initialize Player object
    player.setFadespeed(config.fadespeed);
    player.setBrightness(config.brightness);
    player.setInputs(inputs);
    
    if( config.boot_mode == BOOT_PLAY_SCRIPT ) {
        player.playScript( config.script_id, config.script_reps, config.script_pos ); // FIXME: script_len?
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
        //outvals[0] = eeprom_read_byte(&ee_i2c_addr);
        outvals[0] = eeprom_read_byte(&ee_config.i2c_addr);
        outcount = 1;
        break;
    case('A'):                // set i2c address
        // command with 4 args, changes I2C address
        // FIXME
        if( cmdargs[1] != 0 && cmdargs[1] == cmdargs[4] &&
            cmdargs[2] == 0xD0 && cmdargs[3] == 0x0D ) {  //
            eeprom_write_byte( &ee_config.i2c_addr, cmdargs[1] ); // write address
            i2cSetup( cmdargs[1] );
        }
        break;
    case('L'):            // set script length & repeats: script_id, len, reps
        // command with 3 args (script_id, len, reps)
        eeprom_write_byte( &ee_config.script_id,   cmdargs[1] );
        eeprom_write_byte( &ee_config.script_len,  cmdargs[2] );
        eeprom_write_byte( &ee_config.script_reps, cmdargs[3] );
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
            eeprom_read_block( &linetmp, (const void*)(&ee_script_lines[ cmdargs[2] ])  ,
                               sizeof(script_line_t));
            dbgln(linetmp.cmd);
            memcpy( &outvals, &linetmp, sizeof(script_line_t));
            //outvals[0] = linetmp.dur;
            //outvals[1] = linetmp.cmd;
            //outvals[2] = linetmp.args[0];
            //outvals[3] = linetmp.args[1];
            //outvals[4] = linetmp.args[2];
            outcount = 5;
        }
        else { // flash-based script
            
        }
        break;
    case('W'):                // write a script line: id, pos, dur, cmd,arg1,arg2,arg3
        // command with 7 args (script_id,pos,dur, cmd, arg1,arg2,arg3)
        if( cmdargs[1] == 0 ) { // eeprom script
            if( cmdargs[1] < EE_SCRIPT_LEN_MAX ) {
                eeprom_write_block( &cmdargs[2], 
                                    &ee_script_lines[ cmdargs[1] ],
                                    sizeof(script_line_t));
            } // else too big
        } else {                // flash-based script
        }
        break;
        
    case('B'):                 // set boot params: mode, script_id, reps, fadesp, timeadj
        // command with 5 args
        eeprom_write_block( &cmdargs[1], // data starts from arg index 1 
                            &ee_config.boot_mode, // start from boot_mode
                            5); // copy 5 bytes
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

// -------------------------------------------------------------


//
//
//
void eeprom_load_config(blinkm_config*config)
{
    eeprom_read_block( config, &ee_config, sizeof(blinkm_config));

    dbgln("eeprom config:");
    dbg(" i2c_addr:   "); dbgln(config->i2c_addr);
    dbg(" boot_id:    "); dbgln(config->boot_id);
    dbg(" boot_mode:  "); dbgln(config->boot_mode);
    dbg(" script_id:  "); dbgln(config->script_id);
    dbg(" script_reps:"); dbgln(config->script_reps);
    dbg(" fadespeed:  "); dbgln(config->fadespeed);
    dbg(" timeadj:    "); dbgln(config->timeadj);
    dbg(" brightness: "); dbgln(config->brightness);
    dbg(" script_len: "); dbgln(config->script_len);
    dbg(" script_pos: "); dbgln(config->script_pos);
}

//
// FIXME: magic values below
//
void eeprom_reset_config()
{
    const blinkm_config defaults =
        {
         .i2c_addr    = I2C_ADDR_DEFAULT,
         .boot_id     = EEPROM_BOOT_ID,
         .boot_mode   = BOOT_PLAY_SCRIPT,
         .script_id   = 0,
         .script_reps = 0,
         .fadespeed   = 6,
         .timeadj     = 0,
         .brightness  = 255,
         .script_len  = 5,
         .script_pos  = 0,
        };
    
    eeprom_write_block( &defaults, &ee_config, sizeof(blinkm_config));
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
    for( int i=script_len; i<EE_SCRIPT_LEN_MAX; i++ ) {
        eeprom_write_block( &linetmp, &ee_script_lines[i], sizeof(script_line_t) );
        dbg("  dur,cmd,args: "); dbg(linetmp.dur); dbg(','); dbg(linetmp.cmd); dbg(',');
        dbg(linetmp.args[0]); dbg(','); dbg(linetmp.args[1]); dbg(','); dbgln(linetmp.args[2]);
    }

}
