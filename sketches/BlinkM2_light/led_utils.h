


#ifndef _LED_UTILS_H_
#define _LED_UTILS_H_

#include <stdint.h>
#include <stdbool.h>

#include "config.h"
#include "led_types.h"

// now in config.h
//#define led_update_millis 10

//
uint8_t led_get_brightness();
void led_set_brightness(uint8_t b);

//
void led_setN(  uint8_t r, uint8_t g, uint8_t b, uint8_t n );

//
void led_setAll( uint8_t r, uint8_t g, uint8_t b);

//
void led_show();

//
bool ledfader_update();

//
void ledfader_set_dest( rgb_t* newc, uint16_t dmillis, uint8_t ledn );

//
bool led_should_set( uint8_t pos, uint8_t ledn );

bool led_blend( rgb_t* curr, const rgb_t* start, const rgb_t* dest, fract8 blend_amount );


uint8_t scale8( uint8_t i, fract8 scale);
uint8_t scale8_video( uint8_t i, fract8 scale);

uint8_t random8();

//void hsv2rgb_rainbow( const rgb_t* hsv, rgb_t* rgb);
void hsv2rgb_rainbow( const rgb_t* hsv, rgb_t* rgb);
void hsv2rgb_raw_C (const rgb_t* hsv, rgb_t* rgb);


#endif /* _LED_UTILS_H_ */

