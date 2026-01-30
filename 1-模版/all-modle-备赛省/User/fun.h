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
extern u16 r37_value;
extern float r37_volt;
extern float r38_volt;
extern float mcp_volt;
void eepom_ctrl(void);
void led_show(u8 led, u8 mode);
void while_fun(void);
void key_scan(void);
void key_scan_2(void);
#endif
