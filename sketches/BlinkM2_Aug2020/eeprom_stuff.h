#ifndef eeprom_stuff_h
#define eeprom_stuff_h

// NOTE: Declaring EEPROM statically doesn't do anyting because the Arduino
//        loader doesn't burn eeprom when flashing device
// IDEA: Check EEPROM id and load EEPROM from flash if not set
// IDEA: Can we "compress" the script into eeprom, decompress into RAM?

#define EEPROM_BOOT_ID 0xB1

// FIXME: make this an enum
// possible values for boot_mode
#define BOOT_NOTHING     0
#define BOOT_PLAY_SCRIPT 1
#define BOOT_MODE_END    2

struct setupvals {
    uint8_t i2c_addr;
    uint8_t id;
    uint8_t mode;
    uint8_t script_id;
    uint8_t reps;
    uint8_t fadespeed;
    uint8_t timeadj;
    uint8_t brightness;
};
setupvals bootvals EEMEM;

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

void eeprom_reset_vals()
{
    eeprom_write_byte( &ee_i2c_addr, I2C_ADDR_DEFAULT ); // write address
    eeprom_write_byte( &ee_boot_id, EEPROM_BOOT_ID );
    eeprom_write_byte( &ee_boot_mode, BOOT_PLAY_SCRIPT );
    eeprom_write_byte( &ee_boot_script_id, 18 ); //17; // FIXME TESTING
    eeprom_write_byte( &ee_boot_reps, 0 );
    eeprom_write_byte( &ee_boot_fadespeed, 2 ); //8;
    eeprom_write_byte( &ee_boot_timeadj, 0 );
    eeprom_write_byte( &ee_boot_brightness, 255); //25; // 255

}

#endif
