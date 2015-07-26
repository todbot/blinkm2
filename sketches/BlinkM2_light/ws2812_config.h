/*
 * light_ws2812_config.h
 *
 * Created: 18.01.2014 09:58:15
 *
 * User Configuration file for the light_ws2812_lib
 *
 */ 


#ifndef WS2812_CONFIG_H_
#define WS2812_CONFIG_H_


///////////////////////////////////////////////////////////////////////
// Define I/O pin
///////////////////////////////////////////////////////////////////////

#if defined(__AVR_ATtiny84__) 

// debug board on attiny84 board
//#define ws2812_port B     // Data port 
//#define ws2812_pin  2     // Data out pin

// production attiny84 board 
#define ws2812_port A      // Data port 
#define ws2812_pin  7      // Data out pin

#elif defined(__AVR_ATtiny85__)

#define ws2812_port B      // Data port 
#define ws2812_pin  1      // Data out pin

#else

// Arduino UNO pins 0-7 is port D
#define ws2812_port D
#define ws2812_pin  3

#endif

#endif /* WS2812_CONFIG_H_ */
