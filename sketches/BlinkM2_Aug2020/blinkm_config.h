#ifndef BLINKM_CONFIG_H
#define BLINKM_CONFIG_H


#define BLINKM_PROTOCOL_VERSION_MAJOR 'c'
#define BLINKM_PROTOCOL_VERSION_MINOR 'a'


const int NUM_LEDS = 32;
const int I2C_ADDR_DEFAULT = 0x19;
const int EE_SCRIPT_LEN_MAX = 50 ;
// EEPROM size = 512 bytes
// ee_blinkm_config = 10 bytes
// ee_script_lines = 5 bytes per line * 50 = 250 bytes 

// This ID value must match ee_config.boot_id or EEPROM is invalid and will be reloaded
#define EEPROM_BOOT_ID 0xB2

// FIXME: make this an enum
// possible values for ee_config.boot_mode
#define BOOT_NOTHING     0
#define BOOT_PLAY_SCRIPT 1
#define BOOT_MODE_END    2


#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__)  //
#define __BLINKM_BOARD__
#else
#define __BLINKM_DEV__
#endif

#if defined(__BLINKM_BOARD__)
#define dbg_start()
#define dbg(x)
#define dbgln(x)
#else
#warning "building for BLINKM DEVELOPMENT"
#define dbg_start() do {Serial.begin(115200);Serial.println("dbg_start");} while(0)
#define dbg(x) Serial.print(x)
#define dbgln(x) Serial.println(x)
#endif

// Arduino pin numbers 
#if defined( __BLINKM_BOARD__ )
#define SCL_PIN   2
#define MISO_PIN  1
#define SDA_PIN   0
#define LED_PIN   3  // PB3? yes PB3, old BlinkM datasheet schematic is wrong (pin2 on SOIC)
#else
// Arduino ProMicro pinout
#define SCL_PIN   3
#define MISO_PIN  17
#define SDA_PIN   2
#define LED_PIN   10
#endif

// WS2812 pin number settings for light_ws2812
#if defined(__BLINKM_BOARD__)
// ATtiny85 
#define ws2812_port B     // Data port
#define ws2812_pin  3     // Data out pin
#else
// Arduino pro micro on pin 10
#define ws2812_port B     // Data port
#define ws2812_pin  6     // Data out pin, Arduino pin 10
#endif

#endif  // BLINKM_CONFIG_H
