/**
 *
 * BlinkM2_Aug2020 - for use with "blinkm2neo" ATtiny85 board
 *
 * 2017-2020 @todbot / Tod E. Kurt
 *
 *
 * Libraries required:
 * - Light_WS2812_AVR - https://github.com/cpldcpu/light_ws2812
 * - TimerOne - https://github.com/PaulStoffregen/TimerOne (must use github version, not Library Manager)
 *
 *
 */

#include "blinkm_config.h"

#include <Wire.h>
#include <TimerOne.h>
#include <avr/eeprom.h>

#include "Player.h"
#include "LedScripts.h"

extern "C"{
#include "light_ws2812.h"
};

#include "eeprom_stuff.h"

const int sclPin  = SCL_PIN;
const int misoPin = MISO_PIN;
const int sdaPin  = SDA_PIN;
const int ledPin  = LED_PIN;

// data read from I2C
uint8_t cmdargs[8];
// data sent to I2C
uint8_t outvals[8];
uint8_t outcount = 0; // num bytes to send

// 
uint8_t inputs[2];

// 
rgb_t leds[NUM_LEDS];


// Does all the hard work
Player player(leds, NUM_LEDS);

//
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
    
    if( boot_id != EEPROM_BOOT_ID ) { // magic value
        dbgln("EEPROM NOT SET UP. Using defaults");
        eeprom_reset_vals();
    }

    // set up
    uint8_t i2c_addr       = eeprom_read_byte( &ee_i2c_addr ); 
    uint8_t boot_mode      = eeprom_read_byte( &ee_boot_mode );
    uint8_t boot_script_id = eeprom_read_byte( &ee_boot_script_id );
    uint8_t boot_reps      = eeprom_read_byte( &ee_boot_reps );
    uint8_t boot_fadespeed = eeprom_read_byte( &ee_boot_fadespeed );
    uint8_t boot_timeadj   = eeprom_read_byte( &ee_boot_timeadj );
    uint8_t boot_brightness= eeprom_read_byte( &ee_boot_brightness );

    
    // initialize i2c interface 
    if( i2c_addr==0 || i2c_addr>0x7f) { i2c_addr = I2C_ADDR_DEFAULT; }

    Wire.begin(i2c_addr);
    Wire.onReceive(i2cReceiveEvent);
    Wire.onRequest(i2cRequestEvent);

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

// read data sent from Master (Controller)
void i2cReceiveEvent(int howMany)
{
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
            //I2Cbegin( args[0] );
        }
        break;
    case('Z'):                // return protocol version
        // no args, returns 2 vals
        outvals[0] = BLINKM_PROTOCOL_VERSION_MAJOR;
        outvals[1] = BLINKM_PROTOCOL_VERSION_MINOR;
        outcount = 2;
        break;
    case('R'):                // read a script line, outputs 5: dur,cmd,arg1,arg2,arg3
        // command with 2 args, returns 5 vals
        script_line_t line;
        if(cmdargs[1] == 0 ) { // eeprom script
            eeprom_read_block( &line, &(ee_script_lines[cmdargs[2]]),
                               sizeof(script_line_t));
            outvals[0] = line.dur;
            outvals[1] = line.cmd;
            outvals[2] = line.args[0];
            outvals[3] = line.args[1];
            outvals[4] = line.args[2];
            outcount = 5;
        }
        break;
    case('B'):                 // set boot params: mode, script_id, reps, fadesp, timeadj
        //readI2CVals(5);
            
            /* eeprom_write_block( &(args[0]), // data starts from index 0 */
            /*                     &(ee_boot_mode), */
            /*                     5); */
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
    //strip.show();
}

void checkInputs()
{
    // inputs[0] = analogRead(a0Pin);
    // inputs[1] = analogRead(a1Pin);
    // inputs[2] = (digitalRead(misoPin) == HIGH) ? 255 : 0;
    // inputs[3] = (digitalRead(sdaPin) == HIGH) ? 255 : 0;
    // // FIXME: verify doesn't mess up i2c
}
