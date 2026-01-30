#ifndef __FUN_H
#define __FUN_H

/* Includes ------------------------------------------------------------------*/
#include "headfile.h"

typedef int32_t  s32;
typedef int16_t s16;
typedef int8_t  s8;
typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

void led_show(u8 led, u8 mode);
void key_scan(void);
void lcd_show(void);
void while_fun(void);
void volt_adc_get(void);
#endif
