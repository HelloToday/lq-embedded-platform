#ifndef __FUN_H
#define __FUN_H
/* Includes ------------------------------------------------------------------*/
#include "headfile.h"

typedef uint8_t u8;
typedef uint32_t u32;

#define LONG_TIME 1000
void led_show(u8 led, u8 mode);
void lcd_show(void);
void key_scan(void);
#endif
