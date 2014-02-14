
#ifndef PATTERNS_H
#define PATTERNS_H

/**
 * ideas for new commands:
 * - parameterized color ("blink_white", "blink_red", etc. become one)
 * - "rotate leds" 
 * - ""
 */

// can't declare these statically because Arduino loader doesn't send eeprom
//pattern_t pattern_ee EEMEM; 
uint8_t     ee_i2c_addr   EEMEM = 0x09; //I2C_ADDR;
uint8_t     ee_mode       EEMEM = 0x00; //BOOT_PLAY_SCRIPT;
patt_info_t ee_patt_info  EEMEM;
rgb_t       ee_color      EEMEM;

//uint8_t ee_patt_id    EEMEM = 0x00;
//uint8_t ee_reps       EEMEM = 0x00;
//uint8_t ee_playstart  EEMEM = 0x00;
//uint8_t ee_playend    EEMEM = 0x00;

patt_line_t ee_patt_lines[patt_max] EEMEM;


patt_line_t patt_lines_rgb[] PROGMEM = {
    //          G     R     B
    {{ 'c', { 0x00, 0x33, 0x00 }, 100, 0 }}, // red
    {{ 'c', { 0x33, 0x00, 0x00 }, 100, 0 }}, // grn
    {{ 'c', { 0x00, 0x00, 0x33 }, 100, 0 }}, // blu
    {{ 'c', { 0x00, 0x00, 0x00 }, 100, 0 }}, // off
};

patt_line_t patt_lines_rgb0[] PROGMEM = {
    {{ 'c', { 0x33, 0x00, 0x00 }, 100, 0b1111111100000010 }}, // 0  red all
    {{ 'n', { 0x00, 0x33, 0x00 }, 100, 0b0000000010101010 }}, // 1  grn all
    {{ 'n', { 0x00, 0x00, 0x33 }, 100, 0b1111111100001100 }}, // 2  blu all
    {{ 'n', { 0x33, 0x33, 0x33 }, 100, 0b0000000010101111 }}, // 2  wht all
    {{ 'c', { 0x00, 0x00, 0x00 }, 100, 0b1111111111111111 }}, // 1  off all
};

patt_line_t patt_lines_hsv[] PROGMEM = {
    {{ 'c', { 0x00, 0xff, 0xff }, 100, 0 }}, // 
    {{ 'c', { 0x33, 0xff, 0xff }, 100, 0 }}, // 
    {{ 'c', { 0x66, 0xff, 0xff }, 100, 0 }}, // 
    {{ 'c', { 0x99, 0xff, 0xff }, 100, 0 }}, // 
};

patt_line_t patt_lines_huecycle[] PROGMEM = { 
    //          H    S    V
    { 'h', { 0xff,0x00,0x33 }, 200, 0 },  // white
    { 'h', { 0x00,0xff,0xff }, 100, 0 },  // red
    { 'h', { 0x2a,0xff,0xff }, 100, 0 },  // yellow 
    { 'h', { 0x54,0xff,0xff }, 100, 0 },  // green
    { 'h', { 0x7e,0xff,0xff }, 100, 0 },  // cyan
    { 'h', { 0xa8,0xff,0xff }, 100, 0 },  // blue
    { 'h', { 0xd2,0xff,0xff }, 100, 0 },  // magenta
    { 'h', { 0xff,0xff,0xff }, 100, 0 },  // red
};

patt_line_t patt_lines_blink_white[] PROGMEM = {
    { 'c', { 0x33, 0x33, 0x33 },  50, 0 }, // 0  white all
    { 'c', { 0x00, 0x00, 0x00 },  50, 0 }, // 1  off all
};

// Random color mood light
patt_line_t patt_lines_randmood[] PROGMEM = {
    {'H', {0x80,0x00,0x00}, 50, 0 }, // random fade to other hues
};


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
patt_line_t patt_lines_stoplight[] PROGMEM = {
    { { 0x00, 0x33, 0x00 },  50, 0, 'c' }, // 0  red
    { { 0xdd, 0x11, 0x00 },  50, 0, 'c' }, // 1  yellow
    { { 0x00, 0x33, 0x11 },  50, 0, 'c' }, // 1  greenblue
};
*/

patt_line_t* patterns[] PROGMEM = {
    (patt_line_t*) &patt_lines_rgb,          // 1
    (patt_line_t*) &patt_lines_rgb0,         // 2
    (patt_line_t*) &patt_lines_hsv,          // 3
    (patt_line_t*) &patt_lines_huecycle,     // 4
    (patt_line_t*) &patt_lines_blink_white,  // 5
    (patt_line_t*) &patt_lines_randmood,      // 6
    (patt_line_t*) &patt_lines_default,      // 7
   
    //(patt_line_t*) &patt_lines_blink_white,
    //(patt_line_t*) &patt_lines_stoplight,
};

// this is so lame, but can't create a flexible array of patt_lines in a struct
uint8_t patt_lens[] PROGMEM = {
    4,
    5,
    4,
    8,
    2,
    1,
    patt_max,
};


#endif

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
