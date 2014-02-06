
#ifndef DEBUGTOOLS_H
#define DEBUGTOOLS_H

/*
int freeRam() 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
*/


int freeRam() 
//int get_free_memory()
{
extern int __bss_end;
extern int *__brkval;
  int free_memory;

  if((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);

  return free_memory;
}

#define DBG_TX
#ifdef DBG_TX
extern "C" { 
#include "dbg_putchar.h"
}
void dbg_tx_str(const char* s) {
    while( *s ) dbg_putchar( *s++);
}
void dbg_tx_strP(const char* s) {
    char c;
    while ((c = pgm_read_byte(s++)))
        dbg_putchar(c);
}
void dbg_tx_hex( int d ) { 
    static char str[5];
    dbg_tx_str( itoa(d, str, 16) );
}
#define dbg_tx_putc(x) dbg_putchar(x)
#else
#define dbg_tx_init()
#define dbg_tx_putchar(x)
#define dbg_tx_str(x)
#define dbg_tx_hex(x)
#endif

#endif
