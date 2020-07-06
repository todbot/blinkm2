/**
 *
 * BlinkM2_Jul2020 - for use with "blinkm_mk2sss" ATtiny84 board
 *   with two Analog inputs
 *
 * 2017-2020 Tod E. Kurt @todbot
 *
 *
 * Libraries required:
 * - TinyWires - https://github.com/rambo/TinyWire
 * - Light_WS2812_AVR - https://github.com/cpldcpu/light_ws2812
 *
 *
 *
 */

#include "blinkm_config.h"

#include "Player.h"
#include "LedScripts.h"

#include <avr/eeprom.h>

extern "C"{
#include "light_ws2812.h"
};

#if defined(__BLINKM_BOARD__)
#include "TinyWireS.h"                  // wrapper class for I2C slave routines
#define I2C           TinyWireS
#define I2Cread       TinyWireS.receive
#define I2Cupdater()  TinyWireS_stop_check()
#else
#include "Wire.h"
#define I2C            Wire
#define I2Cread        Wire.read
#define I2Cupdater()  
#endif

const int sclPin  = SCL_PIN;
const int misoPin = MISO_PIN;
const int sdaPin  = SDA_PIN;
const int ledPin  = LED_PIN;

// NOTE: can't declare EEPROM statically because Arduino loader doesn't send eeprom
// IDEA: can we "compress" the script into eeprom, decompress into RAM?

uint8_t  ee_i2c_addr         EEMEM = I2C_ADDR_DEFAULT;
uint8_t  ee_boot_id          EEMEM = 0xB1;
uint8_t  ee_boot_mode        EEMEM = 0x00; // FIXME: BOOT_PLAY_SCRIPT;
uint8_t  ee_boot_script_id   EEMEM = 0x00;
uint8_t  ee_boot_reps        EEMEM = 0x00;
uint8_t  ee_boot_fadespeed   EEMEM = 0x08;
uint8_t  ee_boot_timeadj     EEMEM = 0x00;
uint8_t  ee_boot_brightness  EEMEM = 0x00;
uint8_t  ee_unused2          EEMEM = 0xDA;

script_line_t ee_script_lines[patt_max] EEMEM;

uint8_t cmd;
uint8_t args[4];
uint8_t inputs[4];

rgb_t leds[NUM_LEDS];

Player player((rgb_t*) leds, NUM_LEDS);

void setup()
{
    dbg_start();

    pinMode(ledPin, OUTPUT);
    //pinMode(a0Pin, INPUT_PULLUP);
    //pinMode(a1Pin, INPUT_PULLUP);
    //pinMode(misoPin, INPUT_PULLUP);

    delay(2000);  // for debug
    dbgln("setup done!");

    // set up
    uint8_t i2c_addr       = eeprom_read_byte( &ee_i2c_addr ); 
    uint8_t boot_id        = eeprom_read_byte( &ee_boot_id );
    uint8_t boot_mode      = eeprom_read_byte( &ee_boot_mode );
    uint8_t boot_script_id = eeprom_read_byte( &ee_boot_script_id );
    uint8_t boot_reps      = eeprom_read_byte( &ee_boot_reps );
    uint8_t boot_fadespeed = eeprom_read_byte( &ee_boot_fadespeed );
    uint8_t boot_timeadj   = eeprom_read_byte( &ee_boot_timeadj );
    uint8_t boot_brightness= eeprom_read_byte( &ee_boot_brightness );

    if( boot_id != 0xB1 ) { // magic value
        dbgln("EEPROM NOT SET UP. Using defaults");
        boot_id = 0xB1;
        boot_mode = BOOT_PLAY_SCRIPT;
        boot_script_id = 1;
        boot_reps = 0;
        boot_fadespeed = 8;
        boot_timeadj = 0;
    }
    // initialize i2c interface 
    if( i2c_addr==0 || i2c_addr>0x7f) { i2c_addr = I2C_ADDR_DEFAULT; }  
    // TinyWireS.begin(i2c_addr);

    // verify BEGIN ws2812 library works
    leds[0].r = 255;
    ws2812_setleds((cRGB*)leds,NUM_LEDS);
    delay(500);                         // wait for 500ms.
    leds[0].b = 255;
    ws2812_setleds((cRGB*)leds,NUM_LEDS);
    delay(500);                         // wait for 500ms.
    leds[0] = rgb_t(0,0,0);
    delay(100);
    // verify END

    player.setFadespeed( boot_fadespeed );
    player.setInputs(inputs);

    if( boot_mode == BOOT_PLAY_SCRIPT ) {
        //play_script( boot_script_id, boot_reps, 0 );
        player.playScript( boot_script_id, boot_reps, 0 ); // FIXME: do real startup behavior
    }

}

void loop()
{
    checkI2C();
    checkInputs();
    updateState();
    updateStrip();
    player.update();
}

void updateStrip()
{
    ws2812_setleds((struct cRGB*)leds,NUM_LEDS);
}

void updateState()
{
    // FIXME what goes here?
}

void checkInputs()
{
    // inputs[0] = analogRead(a0Pin);
    // inputs[1] = analogRead(a1Pin);
    // inputs[2] = (digitalRead(misoPin) == HIGH) ? 255 : 0;
    // inputs[3] = (digitalRead(sdaPin) == HIGH) ? 255 : 0;
    // // FIXME: verify doesn't mess up i2c
}

// 
void checkI2C()
{
    cmd = 0; // /get command

    I2Cupdater();
    if( I2C.available() ) {
      cmd = I2Cread();
    }
      
    switch(cmd) {
        case('o'):  // stop playback
            player.stop();
            break;
        case('f'):  // set fade speed
        case('t'):  // set time adjust
        case('l'):  // set ledn
            // read ONE i2c val
            readI2CVals(1);
            player.handleCmd(cmd, args[0], 0, 0);
            break;
        case('n'):
        case('c'):
        case('C'):
        case('h'):
        case('H'):
        case('p'):
            // read THREE i2c vals
            readI2CVals(3);
            player.handleCmd(cmd, args[0],args[1],args[2]); 
            break;
        default:
            break;
    }
}

void readI2CVals(uint8_t cnt)
{
    for( int i=0; i<cnt; i++) {
        args[i] = I2Cread();
    }
}


// void demo_neopixel()
// {
//   for(int i=0;i< NUM_LEDS;i++){
//     // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
//     strip.setPixelColor(i, strip.Color(0,150,0)); // Moderately bright green color.
//
//     strip.show(); // This sends the updated pixel color to the hardware.
//
//     delay(100); // Delay for a period of time (in milliseconds).
//   }
// }
//
// void demo_lightws2812_rgb_t()
// {
//     for( int i=0; i< NUM_LEDS; i++) {
//         leds[i] = rgb_t(255,0,0); // red
//         updateStrip();
//         delay(200);                         // wait for 500ms.
//
//         leds[i] = rgb_t(0,255,0); // green
//         updateStrip();
//         delay(200);
//
//         leds[i] = rgb_t(0,0,255); // blue
//         updateStrip();
//         delay(200);
//
//         leds[i] = rgb_t(0,0,0);
//         updateStrip();
//         delay(100);
//     }
// }

// void demo_lightws2812()
// {
//     for( int i=0; i< NUM_LEDS; i++) {
//         // leds[i] = rgb(255,0,0);
//         leds[i].r=255;leds[0].g=00;leds[0].b=0;    // Write red to array
//         ws2812_setleds(leds,NUM_LEDS);
//         _delay_ms(200);                         // wait for 500ms.
//
//         leds[i].r=0;leds[i].g=255;leds[i].b=0;			// green
//         ws2812_setleds(leds,NUM_LEDS);
//         _delay_ms(200);
//
//         leds[i].r=0;leds[i].g=00;leds[i].b=255;		// blue
//         ws2812_setleds(leds,NUM_LEDS);
//         _delay_ms(200);
//
//         leds[i].r=0;leds[i].g=0;leds[i].b=0;		// off
//         ws2812_setleds(leds,NUM_LEDS);
//         _delay_ms(100);
//     }
// }
