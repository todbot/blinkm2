#ifndef eeprom_stuff_h
#define eeprom_stuff_h

// NOTE: Declaring EEPROM statically doesn't do anyting because the Arduino
//        loader doesn't burn eeprom when flashing device
// IDEA: Check EEPROM id and load EEPROM from flash if not set
// IDEA: Can we "compress" the script into eeprom, decompress into RAM?

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


#endif
