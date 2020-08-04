#ifndef BLINKM_UTILS_H
#define BLINKM_UTILS_H

#include <stdint.h>

uint8_t colorSlide( uint8_t curr, uint8_t dest, uint8_t step );

void fl_nscale8x3_video( uint8_t& r, uint8_t& g, uint8_t& b, uint8_t scale);

uint8_t get_rand_range(uint8_t prev, uint8_t range);

void hsvToRgb(uint8_t oh, uint8_t os, uint8_t ov,
                     uint8_t* r, uint8_t* g, uint8_t* b);

int freeMemory();

#endif // BLINKM_UTILS
