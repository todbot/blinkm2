//
//
// from: http://playground.arduino.cc/Main/Printf
//
//

#ifndef PRINTF_WRAPPER_H
#define PRINTF_WRAPPER_H

#include <stdio.h>
#include <stdarg.h>
#include <Arduino.h>

void p(char *fmt, ... ) 
{
        char tmp[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(tmp, 128, fmt, args);
        va_end (args);
        Serial.print(tmp);
}

#endif
