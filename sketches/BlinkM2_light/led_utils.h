


#ifndef _LED_UTILS_H_
#define _LED_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "config.h"
#include "light_ws2812.h"
#include "led_types.h"

#define led_update_millis 10

//
void led_setBrightness(uint8_t b);

//
void led_setN(  uint8_t r, uint8_t g, uint8_t b, uint8_t n );

//
void led_setAll( uint8_t r, uint8_t g, uint8_t b);

//
void led_show();

//
int ledfader_update();

//
void ledfader_set_dest( rgb_t* newc, uint16_t dmillis, uint8_t ledn );

//
bool led_should_set( uint8_t pos, uint8_t ledn );

bool led_blend( rgb_t* curr, rgb_t* start, rgb_t* dest, fract8 blend_amount );


#ifdef __cplusplus
}
#endif

#endif /* _LED_UTILS_H_ */

