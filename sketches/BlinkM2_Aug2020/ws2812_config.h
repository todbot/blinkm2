/*
 * light_ws2812_config.h
 *
 * v2.4 - Nov 27, 2016
 *
 * User Configuration file for the light_ws2812_lib
 *
 */

#ifndef WS2812_CONFIG_H_
#define WS2812_CONFIG_H_

#include "blinkm_config.h"

///////////////////////////////////////////////////////////////////////
// Define Reset time in µs.
//
// This is the time the library spends waiting after writing the data.
//
// WS2813 needs 300 µs reset time
// WS2812 and clones only need 50 µs
//
///////////////////////////////////////////////////////////////////////

#define ws2812_resettime  300

///////////////////////////////////////////////////////////////////////
// Define I/O pin
///////////////////////////////////////////////////////////////////////


#if defined(__BLINKM_BOARD__)
//#warning "--- BLINKM BOARD ---"
#elif defined(__BLINKM_DEV__)
//#warning "--- BLINKM DEV ---"
#else
#error "--- NO BLINKM DEFINED ---"
#endif

// Arduino pro micro on pin 10
//#define ws2812_port B     // Data port
//#define ws2812_pin  6     // Data out pin

// ATTiny85 BlinkM2neo board
// #define ws2812_port B     // Data port
// #define ws2812_pin  2     // Data out pin

#endif /* WS2812_CONFIG_H_ */
