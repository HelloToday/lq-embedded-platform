/* Define to prevent recursive inclusion -------------------------------------*/
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

extern u8 rx_data;

void led_show(u8 led, u8 mode);
void while_fun(void);
void key_scan(void);
void pass_add(u8 flag);
void pass_sub(u8 flag);
#endif

