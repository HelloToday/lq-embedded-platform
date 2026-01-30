#ifndef __FUN_H
#define __FUN_H

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
void uart_prepare(void);
void uart_ctrl(void);
#endif

