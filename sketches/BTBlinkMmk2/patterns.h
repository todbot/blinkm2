
#ifndef PATTERNS_H
#define PATTERNS_H

//#include "blinkm_types.h"  // for rgb_t


// can't declare these statically because Arduino loader doesn't send eeprom
//pattern_t pattern_ee EEMEM; 
uint8_t     ee_i2c_addr   EEMEM = 0x09; //I2C_ADDR;
uint8_t     ee_mode       EEMEM = 0x00; //BOOT_PLAY_SCRIPT;
patt_info_t ee_patt_info  EEMEM;
//uint8_t ee_patt_id    EEMEM = 0x00;
//uint8_t ee_reps       EEMEM = 0x00;
//uint8_t ee_playstart  EEMEM = 0x00;
//uint8_t ee_playend    EEMEM = 0x00;

patt_line_t ee_patt_lines[patt_max] EEMEM;

patt_line_t patt_lines_default[] PROGMEM  = {
    // cmd      R     G     B    fade ledn
    {{ 'c', { 0x11, 0x00, 0x00 }, 50, 0 }}, // 0  red A
    {{ 'c', { 0x73, 0x00, 0x00 }, 50, 0 }}, // 1  red B
    {{ 'c', { 0x00, 0x00, 0x00 }, 50, 0 }}, // 2  off both
    {{ 'c', { 0x00, 0x11, 0x00 }, 50, 0 }}, // 3  grn A
    {{ 'c', { 0x00, 0x73, 0x00 }, 50, 0 }}, // 4  grn B
    {{ 'c', { 0x00, 0x00, 0x00 }, 50, 0 }}, // 5  off both
    {{ 'c', { 0x00, 0x00, 0x11 }, 50, 0 }}, // 6  blu A
    {{ 'c', { 0x00, 0x00, 0x73 }, 50, 0 }}, // 7  blu B
    {{ 'c', { 0x00, 0x00, 0x00 },100, 0 }}, // 8  off both
    {{ 'c', { 0x10, 0x10, 0x10 }, 50, 0 }}, // 9  half-bright, both LEDs
    {{ 'c', { 0x00, 0x00, 0x00 }, 50, 0 }}, // 10 off both
    {{ 'c', { 0x11, 0x11, 0x11 }, 50, 0 }}, // 11 white A
    {{ 'c', { 0x20, 0x20, 0x20 }, 50, 0 }}, // 12 off A
    {{ 'c', { 0x30, 0x30, 0x30 }, 50, 0 }}, // 13 white B
    {{ 'c', { 0x40, 0x40, 0x40 }, 50, 0 }}, // 14 off B
    {{ 'c', { 0x00, 0x00, 0x00 }, 50, 0 }}, // 15 off everyone
  //{{ 'p', 3, 0, 4,  0,  0 }},
};

/*
patt_line_t patt_lines_default[] PROGMEM = {
    //    G     R     B    fade ledn
    { { 0x00, 0x11, 0x00 },  100, 0 }, // 0  red A
    { { 0x00, 0x11, 0x00 },  100, 0 }, // 1  red B
    { { 0x00, 0x00, 0x00 },  100, 0 }, // 2  off both
    { { 0x11, 0x00, 0x00 },  100, 0 }, // 3  grn A
    { { 0x11, 0x00, 0x00 },  100, 0 }, // 4  grn B
    { { 0x00, 0x00, 0x00 },  100, 0 }, // 5  off both
    { { 0x00, 0x00, 0x11 },  100, 0 }, // 6  blu A
    { { 0x00, 0x00, 0x11 },  100, 0 }, // 7  blu B
    { { 0x00, 0x00, 0x00 },  100, 0 }, // 8  off both
    { { 0x10, 0x10, 0x10 }, 100, 0 }, // 9  half-bright, both LEDs
    { { 0x00, 0x00, 0x00 }, 100, 0 }, // 10 off both
    { { 0x11, 0x11, 0x11 },  50, 0 }, // 11 white A
    { { 0x00, 0x00, 0x00 },  50, 0 }, // 12 off A
    { { 0x7f, 0x7f, 0x7f },  50, 0 }, // 13 white B
    { { 0x00, 0x00, 0x00 }, 100, 0 }, // 14 off B
    { { 0x00, 0x00, 0x00 }, 100, 0 }, // 15 off everyone
};
*/

patt_line_t patt_lines_rgb0[] PROGMEM = {
    {{ 'c', { 0x33, 0x00, 0x00 }, 100, 0b1001001001001001 }}, // 0  red all
    {{ 'c', { 0x00, 0x00, 0x00 }, 100, 0b1001001001001001 }}, // 0  red all
    {{ 'c', { 0x33, 0x00, 0x00 }, 100, 0b0010010010010010 }}, // 1  grn all
    {{ 'c', { 0x00, 0x00, 0x00 }, 100, 0b0010010010010010 }}, // 1  grn all
};

patt_line_t patt_lines_rgb[] PROGMEM = {
    {{ 'C', { 0x33, 0x00, 0x00 }, 100, 0b1111111100000000 }}, // 0  red all
    {{ 'n', { 0x00, 0x33, 0x00 }, 100, 0b0000000010101010 }}, // 1  grn all
    {{ 'n', { 0x00, 0x00, 0x33 }, 100, 0b1111111100000000 }}, // 2  blu all
    {{ 'n', { 0x33, 0x33, 0x33 }, 100, 0b0000000010101010 }}, // 2  wht all
    {{ 'c', { 0x00, 0x00, 0x00 }, 100, 0b1111111111111111 }}, // 1  off all
};

/*
patt_line_t patt_lines_blink_white[] PROGMEM = {
    { { 0x33, 0x33, 0x33 },  50, 0, 'c' }, // 0  white all
    { { 0x00, 0x00, 0x00 },  50, 0, 'c' }, // 1  off all
};

patt_line_t patt_lines_stoplight[] PROGMEM = {
    { { 0x00, 0x33, 0x00 },  50, 0, 'c' }, // 0  red
    { { 0xdd, 0x11, 0x00 },  50, 0, 'c' }, // 1  yellow
    { { 0x00, 0x33, 0x11 },  50, 0, 'c' }, // 1  greenblue
};
*/

patt_line_t* patterns[] PROGMEM = {
    (patt_line_t*) &patt_lines_default,
    (patt_line_t*) &patt_lines_rgb,
    //(patt_line_t*) &patt_lines_blink_white,
    //(patt_line_t*) &patt_lines_stoplight,
};

// this is so lame, but can't create a flexible array of patt_lines in a struct
int patt_lens[] PROGMEM = {
    16,
    3,
    2,
    3,
};


#endif

/*
#if 0
patt_line_t patt_lines_default[] PROGMEM = {
    //    G     R     B    fade ledn
    { { 0x00, 0x11, 0x00 }, 100, 1, 'c' }, // 0  red A
    { { 0x00, 0x11, 0x00 }, 100, 2, 'c' }, // 1  red B
    { { 0x00, 0x00, 0x00 }, 100, 0, 'c' }, // 2  off both
    { { 0x11, 0x00, 0x00 }, 100, 1, 'c' }, // 3  grn A
    { { 0x11, 0x00, 0x00 }, 100, 2, 'c' }, // 4  grn B
    { { 0x00, 0x00, 0x00 }, 100, 0, 'c' }, // 5  off both
    { { 0x00, 0x00, 0x11 }, 100, 1, 'c' }, // 6  blu A
    { { 0x00, 0x00, 0x11 }, 100, 2, 'c' }, // 7  blu B
    { { 0x00, 0x00, 0x00 }, 100, 0, 'c' }, // 8  off both
    { { 0x10, 0x10, 0x10 }, 100, 0, 'c' }, // 9  half-bright, both LEDs
    { { 0x00, 0x00, 0x00 }, 100, 0, 'c' }, // 10 off both
    { { 0x11, 0x11, 0x11 },  50, 1, 'c' }, // 11 white A
    { { 0x00, 0x00, 0x00 },  50, 1, 'c' }, // 12 off A
    { { 0x7f, 0x7f, 0x7f },  50, 2, 'c' }, // 13 white B
    { { 0x00, 0x00, 0x00 }, 100, 2, 'c' }, // 14 off B
    { { 0x00, 0x00, 0x00 }, 100, 0, 'c' }, // 15 off everyone
};
#endif
*/
/*
// this does not work
struct pattern_t {
    uint8_t len;  // number of script lines, 0 == blank script, not playing
    patternline_t lines[];
};
static const pattern_t pattern_default PROGMEM  = { 
    3, 
    //(patternline_t[3])
    {
        { { 0x00, 0xff, 0x00 },  50, 0 }, // 0  red all
        { { 0xff, 0x00, 0x00 },  50, 0 }, // 1  grn all
        { { 0x00, 0x00, 0xff },  50, 0 }, // 2  blu all
    }
};
*/


/*
typedef struct {
    uint8_t len;  // number of script lines, 0 == blank script, not playing
    patternline_t lines[];
} pattern2_t;

pattern_t patterntoo PROGMEM = { 
    3, 
    (patternline_t[3]) 
    {
        { { 0x00, 0xff, 0x00 },  50, 0 }, // 0  red all
        { { 0xff, 0x00, 0x00 },  50, 0 }, // 1  grn all
        { { 0x00, 0x00, 0xff },  50, 0 }, // 2  blu all
    },
};
*/
